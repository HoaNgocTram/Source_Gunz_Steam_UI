#include "stdafx.h"
#include "NewAppDlg.h"
#include "Log.h"
#include "MZip.h"
#include "FileInfo.h"
#include "MMD5.h"
#include <stack>
#include <string>
#define _LINCENSECHECK
#ifdef _LINCENSECHECK
#include <windows.h>
#include <winhttp.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#pragma comment(lib, "winhttp.lib")
#endif
using namespace std;

#ifdef _LINCENSECHECK
// --- HWID generator ---
std::string GetHWID() {
    DWORD serial = 0;
    GetVolumeInformationA("C:\\", NULL, 0, &serial, NULL, NULL, NULL, 0);
    char buf[64];
    sprintf(buf, "%08X", serial);
    return std::string(buf);
}

enum class LicenseStatus {
    OK,
    EXPIRED,
    BANNED,
    HWID_MISMATCH,
    HWID_CONFLICT,
    INVALID,
    NETWORK_ERROR,
    UNKNOWN
};

struct LicenseResult {
    LicenseStatus status;
    std::string message;
};

bool VerifyLicense(const std::string& hwid, const std::string& licenseKey)
{
    bool isValid = false;
    HINTERNET hSession = WinHttpOpen(L"LicenseCheck/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

    if (hSession) {
        HINTERNET hConnect = WinHttpConnect(hSession, L"gunz.vn", INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (hConnect) {
            std::wstring req = L"tool/license/api/verify.php?key=" +
                std::wstring(licenseKey.begin(), licenseKey.end()) +
                L"&hwid=" + std::wstring(hwid.begin(), hwid.end()) +
                L"&app=ZPatchBuilder";

            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", req.c_str(), NULL,
                WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

            if (hRequest) {
                if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
                    if (WinHttpReceiveResponse(hRequest, NULL)) {
                        DWORD dwSize = 0;
                        WinHttpQueryDataAvailable(hRequest, &dwSize);
                        if (dwSize > 0) {
                            std::string response(dwSize, 0);
                            DWORD dwRead = 0;
                            WinHttpReadData(hRequest, &response[0], dwSize, &dwRead);

                            // check basic statuses
                            if (response.find("\"status\":\"ok\"") != std::string::npos)
                                isValid = true;
                            else if (response.find("banned") != std::string::npos)
                                MessageBoxA(NULL, "License banned.", "License Error", MB_ICONERROR);
                            else if (response.find("expired") != std::string::npos)
                                MessageBoxA(NULL, "License expired.", "License Error", MB_ICONERROR);
                            else if (response.find("HWID mismatch") != std::string::npos)
                                MessageBoxA(NULL, "HWID mismatch — this key belongs to another PC.", "License Error", MB_ICONERROR);
                            else if (response.find("already activated") != std::string::npos)
                                MessageBoxA(NULL, "This PC already activated with another key.", "License Error", MB_ICONERROR);
                            else
                                MessageBoxA(NULL, ("License invalid or server response:\n" + response).c_str(), "License Error", MB_ICONERROR);
                        }
                    }
                }
                WinHttpCloseHandle(hRequest);
            }
            WinHttpCloseHandle(hConnect);
        }
        WinHttpCloseHandle(hSession);
    }

    return isValid;
}
#endif

// DoDataExchange
void CNewAppDlg::DoDataExchange(CDataExchange* pDX)
{
    CDDBDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STARTGAME, m_cStartGame);
}

// Message map
BEGIN_MESSAGE_MAP(CNewAppDlg, CDDBDialog)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_STARTGAME, OnBnClickedStartgame)
END_MESSAGE_MAP()

/************************************************************************************
  Dialog
*************************************************************************************/
CNewAppDlg::CNewAppDlg(CWnd* pParent) : CDDBDialog(IDD_DIALOG_MAINDLG, pParent)
{
    CDDBDLGINFO DlgInfo;
    DlgInfo.nWidth = 640;
    DlgInfo.nHeight = 240;
    DlgInfo.IDSkinResource = IDR_BMP_MAINDLG;
    DlgInfo.IDIconResource = IDR_ICO_MAINFRAME;
    DlgInfo.IDWaveResource = IDR_WAV_PUSHBUTTON;
    DlgInfo.bUseQuitButton = true;
    DlgInfo.bEnableMoveDlg = true;
    DlgInfo.bUseTitlebar = true;
    DlgInfo.nTextColor = RGB(38, 38, 38);
    DlgInfo.nTextBkColor = RGB(182, 182, 182);
    SetDialogInfo(DlgInfo);
}

CNewAppDlg::~CNewAppDlg()
{
    while (m_PatchList.begin() != m_PatchList.end())
    {
        delete (*m_PatchList.begin());
        m_PatchList.pop_front();
    }
}

BOOL CNewAppDlg::OnInitDialog()
{
#ifdef _LINCENSECHECK
    // --- Mở console để nhập license trước khi khởi tạo GUI ---
    AllocConsole();

    // redirect stdout, stdin to console
    FILE* pCout = nullptr;
    FILE* pCin = nullptr;
    freopen_s(&pCout, "CONOUT$", "w", stdout);
    freopen_s(&pCin, "CONIN$", "r", stdin);

    // Optional: set console title
    SetConsoleTitleA("ZPatchBuilder - License Check (Console)");

    // Print HWID and ask for key
    std::string hwid = GetHWID();
    std::cout << "================ ZPatchBuilder License Check ================\n";
    std::cout << "HWID: " << hwid << "\n\n";
    std::cout << "Paste/Type your license key and press ENTER:\n> ";

    std::string key;
    std::getline(std::cin, key);

    // Trim trailing CR/LF
    while (!key.empty() && (key.back() == '\r' || key.back() == '\n')) key.pop_back();

    // Nếu người dùng không nhập gì, thoát
    if (key.empty()) {
        std::cout << "\nNo key entered. Exiting...\nPress ENTER to close.";
        std::cin.get();
        FreeConsole();
        exit(0);
    }

    // --- Gọi verify ---
    bool ok = VerifyLicense(hwid, key.c_str());

    if (!ok) {
        std::cout << "\n❌ License check failed or invalid.\n";
        std::cout << "Press ENTER to exit.";
        std::cin.get();
        FreeConsole();
        exit(1);
    }

    // Nếu hợp lệ
    std::cout << "\n✅ License verified successfully. Press ENTER to continue...";
    std::cin.get();

    // Đóng console và tiếp tục khởi tạo GUI
    FreeConsole();

    // =======================================================
    // ========== PHẦN KHỞI TẠO GUI GỐC CỦA APP ============
    // =======================================================
#endif

    CDDBDialog::OnInitDialog();

    ((CComboBox*)GetDlgItem(IDC_COUNTRY))->SetCurSel(0);

    SetWindowText("Gunz Patch Builder");

    // --- NEW: Delete old patch.xml if exists ---
    if (GetFileAttributes("patch.xml") != INVALID_FILE_ATTRIBUTES)
    {
        DeleteFile("patch.xml");
    }

    CRect rect;
    GetWindowRect(&rect);
    int nWidth = rect.Width(), nHeight = rect.Height();
    rect.left = AfxGetApp()->GetProfileInt("Window Position", "x", 50);
    rect.top = AfxGetApp()->GetProfileInt("Window Position", "y", 50);
    rect.right = rect.left + nWidth;
    rect.bottom = rect.top + nHeight;
    MoveWindow(rect, true);

    return true;
}

void CNewAppDlg::OnBnClickedStartgame()
{
    GetDlgItem(IDC_STARTGAME)->EnableWindow(FALSE);

    if (((CComboBox*)GetDlgItem(IDC_COUNTRY))->GetCurSel() == 3)
    {
        CopyFile("Gunz.exe.manifest", "theduel.exe.manifest", FALSE);
        DeleteFile("Gunz.exe.manifest");
    }
    // --- NEW: Delete old patch.xml if exists ---
    if (GetFileAttributes("patch.xml") != INVALID_FILE_ATTRIBUTES)
    {
        DeleteFile("patch.xml");
    }

    ScanDir(".");

    Build();
}

void CNewAppDlg::ScanDir(const char* szDir)
{
    while (m_PatchList.begin() != m_PatchList.end())
    {
        delete (*m_PatchList.begin());
        m_PatchList.pop_front();
    }

    WIN32_FIND_DATA FindData;
    HANDLE hFind;

    stack<string> ScanDir;
    ScanDir.push(szDir);

    while (!ScanDir.empty())
    {
        string strDir = ScanDir.top();
        ScanDir.pop();

        char szFind[_MAX_DIR];
        sprintf(szFind, "%s/*", strDir.c_str());

        if ((hFind = FindFirstFile(szFind, &FindData)) == INVALID_HANDLE_VALUE)
        {
            _ASSERT(FALSE);
            continue;
        }
        else
        {
            do
            {
                if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if ((strcmp(FindData.cFileName, ".") == 0) || (strcmp(FindData.cFileName, "..") == 0))
                    {
                    }
                    else
                    {
                        char szPath[_MAX_DIR];
                        sprintf(szPath, "%s/%s", strDir.c_str(), FindData.cFileName);
                        ScanDir.push(szPath);
                    }
                }
                else
                {
                    char szFilePath[_MAX_DIR];
                    sprintf(szFilePath, "%s/%s", strDir.c_str(), FindData.cFileName);

                    for (char* p = szFilePath; *p; p++) {
                        if (*p == '\\') *p = '/';
                    }

                    char szFileExtName[25];
                    strcpy(szFileExtName, FindData.cFileName + (strlen(FindData.cFileName) - 3));

                    MPatchNode* pNode = new MPatchNode(szFilePath, FindData.nFileSizeLow, FindData.ftLastWriteTime, "");
                    pNode->MakeChecksum();
                    string sMd5 = pNode->GetChecksum();

                    char szMsg[512];
                    sprintf(szMsg, "[ZPatchBuilder] Checksum for %s: %s", szFilePath, sMd5.c_str());
                    PutLog(szMsg);

                    m_PatchList.push_back(pNode);
                }
            } while (FindNextFile(hFind, &FindData));

            FindClose(hFind);
        }
    }
}

void CNewAppDlg::Build()
{
    CString str;
    ((CListBox*)GetDlgItem(IDC_OUTPUT))->ResetContent();

    m_Itr = m_PatchList.begin();

    SetTimer(0, 10, NULL);
}

void CNewAppDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 0)
    {
        FILE* pFP = fopen("patch.xml", "a");
        if (pFP != NULL)
        {
            MPatchNode* pNode = *m_Itr;

            if (m_Itr == m_PatchList.begin())
            {
                fprintf(pFP, "<XML>\n");
                fprintf(pFP, "<PATCHINFO>\n");
            }

            if (!CheckFilteredFile(pNode->GetName()))
            {
                CString str;
                str.Format("Get file info : %s (%u byte,  %s CRC)", pNode->GetName(), pNode->GetSize(), pNode->GetChecksum().c_str());
                ((CListBox*)GetDlgItem(IDC_OUTPUT))->AddString(str);
                ((CListBox*)GetDlgItem(IDC_OUTPUT))->SetCaretIndex(((CListBox*)GetDlgItem(IDC_OUTPUT))->GetCount());

                fprintf(pFP, "	<PATCHNODE file=\"%s\">\n", pNode->GetName());
                fprintf(pFP, "		<SIZE>%u</SIZE>\n", pNode->GetSize());
                fprintf(pFP, "		<CHECKSUM>%s</CHECKSUM>\n", pNode->GetChecksum().c_str());
                fprintf(pFP, "	</PATCHNODE>\n");
            }

            m_Itr++;

            if (m_Itr == m_PatchList.end())
            {
                fprintf(pFP, "</PATCHINFO>\n");
                fprintf(pFP, "</XML>\n");

                ((CListBox*)GetDlgItem(IDC_OUTPUT))->AddString("Complete.");
                ((CListBox*)GetDlgItem(IDC_OUTPUT))->SetCaretIndex(((CListBox*)GetDlgItem(IDC_OUTPUT))->GetCount());

                GetDlgItem(IDC_STARTGAME)->EnableWindow(TRUE);

                this->SetFocus();

                KillTimer(0);
            }

            fclose(pFP);
        }
    }

    CDDBDialog::OnTimer(nIDEvent);
}

bool CNewAppDlg::CheckFilteredFile(const char* szFileName)
{
    char* pFileName = _strlwr(_strdup(szFileName));

    bool bSkip = false;

    char drive[_MAX_PATH], dir[_MAX_PATH], fname[_MAX_PATH], ext[_MAX_PATH];
    _splitpath(pFileName, drive, dir, fname, ext);

    if (strcmp(ext, ".txt") == 0)           bSkip = false;
    else if (strcmp(ext, ".html") == 0)     bSkip = true;
    else if (strcmp(ext, ".htm") == 0)      bSkip = false;
    else if (strcmp(ext, ".dat") == 0)      bSkip = true;
    else if (strcmp(ext, ".xml") == 0)      bSkip = true;
    else if (strcmp(ext, ".nsi") == 0)      bSkip = true;
    else if (strcmp(ext, ".bak") == 0)      bSkip = true;
    else if (strcmp(ext, ".map") == 0)      bSkip = true;
    else if (strcmp(ext, ".pdb") == 0)      bSkip = true;
    else if (strcmp(ext, ".bk_") == 0)      bSkip = true;
    else if (strcmp(ext, ".cs3") == 0)      bSkip = true;
    else if (strcmp(ext, ".des") == 0)      bSkip = true;
    else if (strcmp(ext, ".bsc") == 0)      bSkip = true;
    else if (strcmp(ext, ".pdb") == 0)      bSkip = true;
    else if (strcmp(ext, ".config") == 0)   bSkip = true;
    else if (strcmp(ext, ".bat") == 0)      bSkip = true;
    else if (strcmp(ext, ".ilk") == 0)      bSkip = true;
    else if (strcmp(ext, ".iobj") == 0)     bSkip = true;
    else if (strcmp(ext, ".ipdb") == 0)     bSkip = true;
    else if (strcmp(ext, ".picasa.ini") == 0) bSkip = true;
    else if (strstr(pFileName, "zpatchbuilder.exe") != NULL)        bSkip = true;
    else if (strstr(pFileName, "zpatchbuilderd.exe") != NULL)       bSkip = true;
    else if (strstr(pFileName, "zpatchbuilderconsole.exe") != NULL) bSkip = true;
    else if (strstr(pFileName, "zpatchbuilderconsoled.exe") != NULL) bSkip = true;
    else if (strstr(pFileName, "Thumbs.db") != NULL)                bSkip = true;
    else if (strstr(pFileName, "patch.xml") != NULL)                bSkip = true;
    else if (strstr(pFileName, "xtrap") != NULL)                    bSkip = true;
    else if (strstr(pFileName, "xpva") != NULL)                     bSkip = true;

    delete pFileName;

    if (bSkip)
        return true;

    return false;
}