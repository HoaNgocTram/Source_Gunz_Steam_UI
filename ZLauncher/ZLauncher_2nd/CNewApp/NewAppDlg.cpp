#include "stdafx.h"
#include "ZGlobal.h"
#include "NewAppDlg.h"
#include "NewAppAboutBox.h"
#include "NewAppLicenseConfirmDlg.h"
#include "NewAppUpdateConfirm.h"
#include "NewAppUpdateCompleteDlg.h"
#include "NewAppErrorDlg.h"
#include "NewAppWarnVista.h"
#include "CGLEncription.h"
#include <math.h>
#include <Mmsystem.h>
#include <stdio.h>
#include <windows.h>
#include <winternl.h> // Để sử dụng RtlGetVersion

#include <WebView2.h>
#include <wrl.h>
#include <wrl/client.h>
#include <shlobj.h>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ole32.lib")

#define _INTRO 1
#define _WEBVIEWER2 1

#define STR_GUNZ_PROCESS_NAME				"Gunz"
#define STR_GUNZ_EXECUTE_FILENAME			"Gunz.exe"
#define STR_LAUNCHER_EXECUTE_FILENAME		"GunzLauncher.exe"
#define STR_PATCHINFO_FILENAME				"patch.xml"
#define STR_DIRECTORY_REAL					"update_gunz_new"
#define STR_DIRECTORY_ALPHA					""
#ifdef _WEBVIEWER2
#define STR_WEBPAGE_URL						L"https://gunz.vn/launchergunz"
#else
#define STR_WEBPAGE_URL						"https://gunz.vn/launchergunz"
#endif
CString ConnectionHost =					XorStr<0x41, 15, 0x8B10AF84>("\x34\x32\x27\x25\x31\x23\x69\x2F\x3C\x24\x31\x62\x3B\x20" + 0x8B10AF84).s;

#ifdef _DEBUG
#define STR_EXT_UPDATER_FILENAME			"updaterd.exe"
#else
#define STR_EXT_UPDATER_FILENAME			"updater.exe"
#endif // _DEBUG 

#ifdef _WEBVIEWER2
using namespace Microsoft::WRL;

ComPtr<ICoreWebView2Controller> m_webViewController;
ComPtr<ICoreWebView2> m_webView;
bool m_bWebViewReady = false;

void CNewAppDlg::InitWebView2()
{
	// 🧩 Lấy đường dẫn Local AppData của Windows (luôn có quyền ghi)
	char szPath[MAX_PATH];
	std::wstring wCachePath;

	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)))
	{
		// Tạo đường dẫn: %LOCALAPPDATA%\GunzTheDuel\LauncherCache
		strcat_s(szPath, "\\GunzTheDuel");
		CreateDirectoryA(szPath, NULL); // Tạo folder cha
		strcat_s(szPath, "\\LauncherCache");
		CreateDirectoryA(szPath, NULL); // Tạo folder cache

		// Chuyển sang wide string cho WebView2
		int nLen = MultiByteToWideChar(CP_ACP, 0, szPath, -1, NULL, 0);
		wchar_t* wPath = new wchar_t[nLen];
		MultiByteToWideChar(CP_ACP, 0, szPath, -1, wPath, nLen);
		wCachePath = wPath;
		delete[] wPath;
	}
	else
	{
		// Fallback: Nếu không lấy được AppData thì mới dùng thư mục tạm (Temp)
		wCachePath = L"C:\\Windows\\Temp\\GunzWebView2";
	}

	// Khởi tạo WebView2 với đường dẫn an toàn
	CreateCoreWebView2EnvironmentWithOptions(
		nullptr,
		wCachePath.c_str(),  // ⚙️ Đã được đổi sang vùng có quyền ghi
		nullptr,
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT

			{
				if (FAILED(result)) return result;

				env->CreateCoreWebView2Controller(m_hWnd,
					Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
						[this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
						{
							if (controller)
							{
								m_webViewController = controller;
								m_webViewController->get_CoreWebView2(&m_webView);
								m_bWebViewReady = true;

								// Disable context menu
								ComPtr<ICoreWebView2Settings> settings;
								if (SUCCEEDED(m_webView->get_Settings(&settings)))
								{
									settings->put_AreDefaultContextMenusEnabled(FALSE);
									settings->put_AreDevToolsEnabled(FALSE);
								}

								// Load trang launcher
								m_webView->Navigate(STR_WEBPAGE_URL);
							}
							return S_OK;
						}).Get());
				return S_OK;
			}).Get());
}
#endif

UINT ThreadUpdate(LPVOID pParam);

void CNewAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDDBDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STARTGAME, m_cStartGame);
	DDX_Control(pDX, IDC_STOP, m_cStop);
	DDX_Control(pDX, IDC_EXPLORER, m_cExplorer);
	DDX_Control(pDX, IDC_ANIMATION, m_cAnimation);
}

bool CNewAppDlg::IsOpen(char* ip, int port)
{
	try
	{
		WSADATA wsadata;
		int err = WSAStartup(MAKEWORD(2, 2), &wsadata);
		if (err != 0)
		{
			return false;
		}
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		struct hostent* host = gethostbyname(ip);
		struct sockaddr_in sin;
		memset(&sin, 0, sizeof sin);
		sin.sin_family = AF_INET;
		if (!host) return false;
		sin.sin_addr.s_addr = ((struct in_addr *)(host->h_addr))->s_addr;
		sin.sin_port = htons(port);
		if (connect(sock, (struct sockaddr *)&sin, sizeof sin) == SOCKET_ERROR)
		{
			closesocket(sock);
			WSACleanup();
			return false;
		}
		closesocket(sock);
		WSACleanup();
		return true;
	}
	catch (...)
	{
		return false;
	}
}

BEGIN_MESSAGE_MAP(CNewAppDlg, CDDBDialog)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_STARTGAME, OnBnClickedStartgame)
	ON_BN_CLICKED(IDC_STOP, OnBnClickedStop)
	ON_BN_CLICKED(IDC_ABOUTBOX, OnAbout)
	ON_BN_CLICKED(IDC_QUIT, OnQuit)
	ON_MESSAGE(WM_FIND_PATCHINFO, OnFindPatchInfo)
	ON_MESSAGE(WM_UPDATE_COMPLETE, OnUpdateComplete)
	ON_MESSAGE(WM_UPDATE_ERROR, OnUpdateError)
END_MESSAGE_MAP()

CNewAppDlg::CNewAppDlg(CWnd* pParent) : CDDBDialog(IDD_DIALOG_MAINDLG, pParent)
{
	CDDBDLGINFO DlgInfo;
	DlgInfo.nWidth = 750;
	DlgInfo.nHeight = 541;
	DlgInfo.IDSkinResource = IDR_BMP_MAINDLG;
	DlgInfo.IDIconResource = IDR_ICO_MAINFRAME;
	DlgInfo.IDWaveResource = IDR_WAV_PUSHBUTTON;
	DlgInfo.bEnableMoveDlg = true;
	DlgInfo.bUseAboutboxButton = true;
	DlgInfo.bUseMinimizeButton = true;
	DlgInfo.bUseQuitButton = true;
	DlgInfo.bUseTitlebar = true;
	DlgInfo.bUseBmpTitle = true;
#ifdef _SKINBRZ
	DlgInfo.nTextColor = RGB(255, 255, 255);
	DlgInfo.nTextBkColor = RGB(150, 150, 150);
#else
	DlgInfo.nTextColor = RGB(150, 150, 150);
	DlgInfo.nTextBkColor = RGB(38, 38, 38);
#endif
	SetDialogInfo(DlgInfo);

	m_nStatus = STATUS_NONE;

	TCHAR path[MAX_PATH];
	TCHAR path2[MAX_PATH];
	SHGetSpecialFolderPath(0, path, CSIDL_PROGRAM_FILES, FALSE);
	sprintf(path2, "%s/Internet Explorer/iexplore.exe", path);
	if (!fopen(path2, "r"))
	{
		if (MessageBox("Remember that you need Internet Explorer installed to run GunZ \nIf you are seeing this warning, means launcher can not detect Internet Explorer installed on your pc\n Click OK to Start GunzLauncher and Cancel to Install Internet Explorer", "Warning GunzLauncher", MB_OKCANCEL | MB_ICONWARNING) == IDOK)
		{
		}
		else
		{
			ShellExecute(NULL, "open", "http://windows.microsoft.com/en-us/internet-explorer/download-ie", NULL, NULL, SW_SHOWNORMAL);
		}
	}

	INTERNET_CONNECTED_INFO ci;
	memset(&ci, 0, sizeof(ci));
	ci.dwConnectedState = INTERNET_STATE_CONNECTED;
	InternetSetOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &ci, sizeof(ci));

	m_pThreadUpdate = NULL;
	m_bThreadContinue = false;

	m_nMajorVersion = 0;

	m_bUpdateCanceled = false;

	m_bReservedQuit = false;

#ifdef _SKIP_UPDATE
	m_bSkipUpdate = true;
#else
	m_bSkipUpdate = false;
#endif // _SKIP_UPDATE

	m_bNavigationComplete = false;

	m_bUpdateComplete = false;
}

void SetIEFeatureMode() {
	HKEY hKey;
	const char* subkey = "SOFTWARE\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION";
	if (RegCreateKeyExA(HKEY_CURRENT_USER, subkey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS) {
		char exeName[MAX_PATH];
		GetModuleFileNameA(NULL, exeName, MAX_PATH);
		char* pExe = strrchr(exeName, '\\');
		if (pExe) pExe++; else pExe = exeName;

		DWORD ieMode = 11001; // 11001 = IE11 Edge mode
		RegSetValueExA(hKey, pExe, 0, REG_DWORD, (BYTE*)&ieMode, sizeof(DWORD));
		RegCloseKey(hKey);
	}
}

BOOL CNewAppDlg::OnInitDialog()
{
	CDDBDialog::OnInitDialog();
#ifdef _WEBVIEWER2
#else
	SetIEFeatureMode(); // Ép IE11
#endif

	HRSRC hRes = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_STRING), _T("TXT"));
	if (hRes != NULL)
	{
		HGLOBAL hData = ::LoadResource(AfxGetResourceHandle(), hRes);
		if (hData != NULL)
		{
			LPVOID lpData = LockResource(hData);

			STLoadFromMemory((char*)lpData);

			FreeResource(hData);
		}
	}

	InitLog();

	char szVersion[128];
	m_nMajorVersion = GetOSVersion(szVersion);

	CString strBuff;
	strBuff.Format("[APP] OS version : %s", szVersion);
	PutLog(strBuff);


// 	if (dlg.GetChecked())
// 		AfxGetApp()->WriteProfileInt("Flag", "DoNotDisplayVistaWarnning", 1);
// 	SetWindowText(ZGetString("STR_102"));

	CRect rect;
	GetWindowRect(&rect);
	auto nWidth = rect.Width(), nHeight = rect.Height();
	rect.left = AfxGetApp()->GetProfileInt("Window Position", "x", 50);
	rect.top = AfxGetApp()->GetProfileInt("Window Position", "y", 50);
	rect.right = rect.left + nWidth;
	rect.bottom = rect.top + nHeight;
	MoveWindow(rect, true);

	m_cExplorer.MoveWindow(1, 1, 0, 0, false);

	m_cStartGame.InitDDBButton(IDR_BMP_STARTBTN, IDR_WAV_PUSHBUTTON);
	m_cStartGame.MoveWindow(518, 461);
	m_cStartGame.EnableWindow(FALSE);

	m_cStop.InitDDBButton(IDR_BMP_BUTTON, IDR_WAV_PUSHBUTTON);
	m_cStop.MoveWindow(530, 510);
	m_cStop.ShowWindow(SW_HIDE);

	m_cAnimation.InitDDBAniBmp(IDR_BMP_ANIMATION, 6);
	m_cAnimation.SetTimerValue(200);
	m_cAnimation.MoveWindow(25, 470, 10, 10);
	m_cAnimation.EnableAnimate(FALSE);
	m_cAnimation.ShowScene(0);

	GetScreen()->PutBitmap(28, 20, IDR_BMP_GUNZBG, 0, 0, 694, 434);
#ifndef _SKINBRZ
#else
	GetScreen()->PutBitmap(0, 140, GetSkin()->GetDC(), 490, 0, 18, 165);
	GetScreen()->PutBitmap(732, 140, GetSkin()->GetDC(), 509, 0, 18, 165);
#endif

	m_nStatus = STATUS_NONE;

	UpdateInfo();

	GetRunInfo();

	GetFileVersion(STR_GUNZ_EXECUTE_FILENAME, m_szClientVersion);

	GetFileVersion(STR_LAUNCHER_EXECUTE_FILENAME, m_szLauncherVersion);

	PutLog("[APP] Window created successfully.");

	WIN32_FIND_DATA FindData;
	HANDLE hFind = FindFirstFile(STR_EXT_UPDATER_FILENAME, &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
		DeleteFile(STR_EXT_UPDATER_FILENAME);
	SetTimer(TIMER_UPDATE, 100, NULL);

	return true;
}

void CNewAppDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_UPDATE)
	{
		if (m_bReservedQuit)
		{
			switch (m_nStatus)
			{
			case STATUS_NONE:
			case STATUS_READY:
			case STATUS_CANNOT_FIND_PATCHINFO:
			case STATUS_FIND_NEW_FILE:
			case STATUS_UPDATE_FAIL:
			case STATUS_UPDATE_COMPLETE:
			case STATUS_STARTGAME:
				OnDestroyDialog();
				break;
			}
		}

		UpdateStartUI();

#ifdef _INTRO
		UpdateWebLoading();
#endif // _INTRO

		UpdateInfo();

		if (m_nStatus == STATUS_STARTGAME)
			StartGame();
	}

	CDDBDialog::OnTimer(nIDEvent);
}

void CNewAppDlg::OnBnClickedStartgame()
{
	if ((m_bSkipUpdate == true) || (m_bUpdateComplete == true))
	{
#ifdef _LICENSE
		if (ShowLicenseDlg() == false)
			return;
#endif

		m_nStatus = STATUS_STARTGAME;

		StartGame();
	}
	else
	{
		if (!m_Update.IsCreated())
		{
			m_Update.Create(ConnectionHost.GetString(), 80, STR_DIRECTORY_REAL, "", "");
		}

		int i = 0;
		while (m_bThreadContinue == true)
		{
			if (i > 10)
			{
				PutLog("[APP] ERROR - The thread is not terminating.");

				m_nStatus = STATUS_READY;

				return;
			}

			Sleep(100);

			i++;
		}

		m_bThreadContinue = true;
		m_pThreadUpdate = AfxBeginThread(ThreadUpdate, (LPVOID*)this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
		m_pThreadUpdate->ResumeThread();

		m_cExplorer.SetFocus();
	}
}

UINT ThreadUpdate(LPVOID pParam)
{
	CNewAppDlg *pDlg = (CNewAppDlg*)pParam;

	//Sleep(4000);
	if ((pDlg->m_nStatus == STATUS_READY) || (pDlg->m_nStatus == STATUS_CANNOT_FIND_PATCHINFO) || (pDlg->m_nStatus == STATUS_UPDATE_FAIL))
	{
		PutLog("[APP] Download patch info file");

		pDlg->m_nStatus = STATUS_ANALYSE_PATCHINFO;

		CString strMessageSecond;
		strMessageSecond.Format("./%s", STR_PATCHINFO_FILENAME);

		bool bRetVal = pDlg->m_Update.CheckUpdate(strMessageSecond.GetBuffer());
		if (bRetVal == false)
		{
			PutLog("[APP] ERROR - Cannot download patch info file");
			pDlg->m_nStatus = STATUS_CANNOT_FIND_PATCHINFO;
			pDlg->PostMessage(WM_UPDATE_ERROR, 0, 0);
			pDlg->m_bThreadContinue = false;
			return true;
		}

		if (pDlg->m_Update.GetUpdateInfo().GetTotalPatchFileCount() > 0)
		{
			bool bRetVal = pDlg->m_Update.CheckUpdate(strMessageSecond.GetBuffer());
			if (bRetVal == false)
			{
				PutLog("[APP] ERROR - Cannot download patch info file");
				pDlg->m_nStatus = STATUS_CANNOT_FIND_PATCHINFO;
				pDlg->PostMessage(WM_UPDATE_ERROR, 0, 0);
				pDlg->m_bThreadContinue = false;
				return true;
			}
			if (pDlg->m_Update.GetUpdateInfo().GetTotalPatchFileCount() > 0)
			{
				PutLog("[APP] Exist patch file(s)");
				pDlg->m_nStatus = STATUS_FIND_NEW_FILE;
				pDlg->PostMessage(WM_FIND_PATCHINFO, 0, 0);
			}
		}

		else
		{
			pDlg->m_bUpdateComplete = true;
			pDlg->m_nStatus = STATUS_READY;
			pDlg->PostMessage(WM_COMMAND, MAKEWPARAM(IDC_STARTGAME, BN_CLICKED), 0);
		}
	}

	else if (pDlg->m_nStatus == STATUS_FIND_NEW_FILE)
	{
		PutLog("[APP] Start file updating...");

		pDlg->m_nStatus = STATUS_UPDATING;

		bool bRetVal = bRetVal = pDlg->m_Update.StartUpdate();

		if (bRetVal == false)
		{
			PutLog("[APP] ERROR - Update fail");

			pDlg->m_nStatus = STATUS_UPDATE_FAIL;

			pDlg->PostMessage(WM_UPDATE_ERROR, 0, 0);
		}

		else
		{
			PutLog("[APP] Update complete.");

			pDlg->m_nStatus = STATUS_UPDATE_COMPLETE;

			pDlg->PostMessage(WM_UPDATE_COMPLETE, 0, 0);
			pDlg->PostMessage(WM_COMMAND, MAKEWPARAM(IDC_STARTGAME, BN_CLICKED), 0);
		}
	}

	pDlg->m_bThreadContinue = false;

	return true;
}

void CNewAppDlg::OnBnClickedStop()
{
	PutLog("[APP] Update canceled...");

	m_bUpdateCanceled = true;

	m_Update.StopUpdate();

	m_cExplorer.SetFocus();
}

void CNewAppDlg::UpdateStartUI()
{
	static bool bUpdateStartUI = true;
	if (!bUpdateStartUI)
		return;

	static DWORD dwTime = timeGetTime();
	DWORD currTime = timeGetTime() - dwTime;

	if (currTime > 300)
	{
		m_nStatus = STATUS_READY;

		if (m_bSkipUpdate == true)
		{
			m_bUpdateComplete = true;

			m_cStartGame.EnableWindow(TRUE);
		}
		else
			OnBnClickedStartgame();

		bUpdateStartUI = false;
	}
}

#ifdef _WEBVIEWER2
void CNewAppDlg::UpdateWebLoading()
{
	static bool bCompleteOpenWeb = false;
	if (bCompleteOpenWeb)
		return;

	static bool bOpenWeb = false;
	static DWORD dwTime = timeGetTime() + 1000;

	if (!bOpenWeb && (timeGetTime() > dwTime))
	{
		InitWebView2(); // ✅ chỉ gọi 1 lần để tạo WebView2 trong khung
		bOpenWeb = true;
	}

	if (m_bWebViewReady)
	{
#define SIZE_WEBPAGE_X 720
#define SIZE_WEBPAGE_Y 425

		CRect rect;
		GetClientRect(&rect);

		RECT rcWeb{};
		rcWeb.left = 15;
		rcWeb.top = 25;
		rcWeb.right = rcWeb.left + SIZE_WEBPAGE_X;
		rcWeb.bottom = rcWeb.top + SIZE_WEBPAGE_Y;

		if (m_webViewController)
			m_webViewController->put_Bounds(rcWeb);

		bCompleteOpenWeb = true;
		return;
	}

	// 💫 Giữ animation loading y như cũ
	int x = 690, y = 415, r = 18;
	float gap = 6.28f / 10.0f;
	for (int i = 0; i < 10; i++)
	{
		float value = gap * (float)i;
		int dx = x + (int)((float)r * sin(value));
		int dy = y + (int)((float)r * cos(value));
		int color = (int)(70.0f * sin(value + (double)timeGetTime() / 300.0f) + 70.0f);
		GetScreen()->PutFillCircle(dx, dy, 3, RGB(color, color, color));
	}
	Rendering(x - r - 20, y - r - 5, r * 2 + 40, r * 2 + 10);
}
#else
void CNewAppDlg::UpdateWebLoading()
{
	static bool bCompleteOpenWeb = false;
	if (bCompleteOpenWeb)
		return;

	static bool bOpenWeb = false;

	static DWORD dwTime = timeGetTime() + 1000;
	if (!bOpenWeb && (timeGetTime() > dwTime))
	{
		m_cExplorer.Navigate(_T(STR_WEBPAGE_URL), NULL, NULL, NULL, NULL);
		bOpenWeb = true;
	}

	static bool bLoadingComplete = false;
	if (!bLoadingComplete && m_bNavigationComplete)
	{
		bLoadingComplete = true;
		dwTime = timeGetTime() + 1000;
	}

	if (!bCompleteOpenWeb && bOpenWeb && bLoadingComplete && (timeGetTime() > dwTime))
	{
#define SIZE_WEBPAGE_X		720
#define SIZE_WEBPAGE_Y		425

		CRect rect;
		m_cExplorer.GetClientRect(&rect);

		float size = (float)(SIZE_WEBPAGE_X - rect.Width()) * 0.7f + 0.5f;
		size = (size > 150.0f) ? 150.0f : size;

		int width = rect.Width() + (int)size;
		width = (width > SIZE_WEBPAGE_X) ? SIZE_WEBPAGE_X : width;

		int height = SIZE_WEBPAGE_Y;

		m_cExplorer.MoveWindow(15 + SIZE_WEBPAGE_X - width, 25, width, height, FALSE);

		if ((width == SIZE_WEBPAGE_X) && (height == SIZE_WEBPAGE_Y))
			bCompleteOpenWeb = true;

		return;
	}

	int x = 690, y = 415, r = 18;
	float gap = 6.28f / 10.0f;
	for (int i = 0; i < 10; i++)
	{
		float value = gap * (float)i;

		int dx = x + (int)((float)r * sin(value));
		int dy = y + (int)((float)r * cos(value));

		int color = (int)(70.0f * sin(value + (double)timeGetTime() / 300.0f) + 70.0f);

		GetScreen()->PutFillCircle(dx, dy, 3, RGB(color, color, color));
	}

	Rendering(x - r - 20, y - r - 5, r * 2 + 40, r * 2 + 10);
}
#endif

LRESULT CNewAppDlg::OnFindPatchInfo(WPARAM wParam, LPARAM lParam)
{
	if (m_bReservedQuit)
		return TRUE;

	OnBnClickedStartgame();

	return TRUE;
}

LRESULT CNewAppDlg::OnUpdateComplete(WPARAM wParam, LPARAM lParam)
{
	if (m_bReservedQuit)
		return TRUE;

	CNewAppUpdateCompleteDlg dlg;

	this->SetFocus();
	if (m_Update.IsReboot())
	{
		dlg.SetDialog(this, ZGetString("STR_105"), ZGetString("STR_106"), MB_OK);
		dlg.DoModal();

		RunExternalUpdater();

		OnQuit();
	}

	else
	{
		m_bUpdateComplete = true;

		m_nStatus = STATUS_READY;
	}

	return TRUE;
}

LRESULT CNewAppDlg::OnUpdateError(WPARAM wParam, LPARAM lParam)
{
	if (m_bReservedQuit)
		return TRUE;

	CString strLog;
	strLog.Format("[APP] Updater error message :\n\n%s", m_Update.GetErrorMsg()->GetBuffer());
	PutLog(strLog);

	strLog.Format("[App] File transfer error message :\n\n%s", m_Update.GetFileTransferError());
	PutLog(strLog);

	CNewAppErrorDlg dlg;
	dlg.SetDialog(this, ZGetString("STR_108"), ZGetString("STR_109"), m_Update.GetErrorMsg(), MB_OK);
	this->SetFocus();
	int nRetVal = dlg.DoModal();

	return TRUE;
}

void CNewAppDlg::StartGame()
{
	static DWORD dwStartTime = timeGetTime();
	DWORD curr_time = timeGetTime() - dwStartTime;

	static bool bRunGameProcess = false;
	if (!bRunGameProcess && (curr_time > 1000))
	{
		CGLEncription cEncription;
		cEncription.CreateSerialKey(0);

		char szCmdLine[4096] = { 0, };

		sprintf(szCmdLine, ".\\%s", STR_GUNZ_EXECUTE_FILENAME);

		WinExec(szCmdLine, SW_SHOW);

		bRunGameProcess = true;
		dwStartTime = timeGetTime();

		return;
	}

	static bool bQuit = false;
	if (bRunGameProcess && !bQuit && (curr_time > 15000))
	{
		OnQuit();

		bQuit = true;
	}
}

unsigned long int	m_nStartTime;
unsigned long LastSize;
void CNewAppDlg::UpdateInfo()
{
	ZUpdateUIInfo info = m_Update.GetUpdateInfo();
	CString strMessageFirst = "";
	CString strMessageSecond = "";
	float fPercent = 0.0f;

	switch (m_nStatus)
	{
	case STATUS_NONE:
	{
		strMessageFirst = ZGetString("STR_110");
		DWORD dwTime = (timeGetTime() / 120) % 15;
		for (int i = 0; i < (int)dwTime; i++)	strMessageFirst += ".";

		break;
	}

	case STATUS_READY:
	{
		m_cAnimation.EnableAnimate(FALSE);
		m_cAnimation.ShowScene(0);

		strMessageFirst = ZGetString("STR_111");

		if (m_bUpdateComplete == true)
		{
			m_cStartGame.SetBlanking(TRUE, 800);
			m_cStartGame.EnableWindow(TRUE);
		}
		else
		{
			m_cStartGame.SetBlanking(FALSE);
			m_cStartGame.EnableWindow(FALSE);
		}
		m_cStop.EnableWindow(FALSE);
		break;
	}

	case STATUS_ANALYSE_PATCHINFO:
	{
		m_cAnimation.EnableAnimate(TRUE);

		strMessageFirst = ZGetString("STR_114");
		DWORD dwTime = (timeGetTime() / 120) % 15;
		m_cStartGame.EnableWindow(FALSE);
		m_cStop.EnableWindow(FALSE);
		break;
	}

	case STATUS_CANNOT_FIND_PATCHINFO:
	{
		m_cAnimation.EnableAnimate(FALSE);
		m_cAnimation.ShowScene(0);

		strMessageFirst = ZGetString("STR_115");

		m_cStartGame.SetBlanking(FALSE);
		m_cStartGame.EnableWindow(TRUE);
		m_cStop.EnableWindow(FALSE);
		break;
	}

	case STATUS_FIND_NEW_FILE:
	{
		strMessageFirst = ZGetString("STR_116");

		m_cStartGame.EnableWindow(FALSE);
		m_cStop.EnableWindow(FALSE);
		break;
	}

	case STATUS_UPDATING:
	{
		unsigned long nCount = info.GetCurrPatchedFileCount() + 1;
		if (nCount > info.GetTotalPatchFileCount())
		{
			strMessageFirst = ZGetString("STR_117");
			strMessageSecond = ZGetString("STR_118");

			fPercent = 100.0f;
		}
		else
		{
			strMessageFirst.Format("[%d / %d] File : '%s'", nCount, info.GetTotalPatchFileCount(), info.GetCurrPatchFileName());
			strMessageSecond = ZGetString("STR_120");
			fPercent = (float)(info.GetCurrPatchedFileSize() + info.GetCurrDownloadSize()) / (float)info.GetTotalPatchFileSize() * 100.0f;
		}

		DWORD dwTime = (timeGetTime() / 500) % 5;
		m_cStartGame.EnableWindow(FALSE);
		m_cStop.EnableWindow(TRUE);
		break;
	}

	case STATUS_UPDATE_FAIL:
	{
		m_cAnimation.EnableAnimate(FALSE);
		m_cAnimation.ShowScene(0);

		strMessageFirst = ZGetString("STR_121");

		m_cStartGame.SetBlanking(FALSE);
		m_cStartGame.EnableWindow(TRUE);
		m_cStop.EnableWindow(FALSE);
		break;
	}

	case STATUS_UPDATE_COMPLETE:
	{
		m_cAnimation.EnableAnimate(FALSE);
		m_cAnimation.ShowScene(0);

		strMessageFirst = ZGetString("STR_122");
		strMessageSecond = ZGetString("STR_122");
		fPercent = 100.0f;

		m_cStartGame.EnableWindow(FALSE);
		m_cStop.EnableWindow(FALSE);
		break;
	}

	case STATUS_STARTGAME:
	{
		m_cAnimation.EnableAnimate(FALSE);
		m_cAnimation.ShowScene(0);

		strMessageFirst = ZGetString("STR_123");
		DWORD dwTime = (timeGetTime() / 500) % 5;
		for (int i = 0; i < (int)dwTime; i++)	strMessageFirst += ".";

		fPercent = 0.0f;

		m_cStartGame.EnableWindow(FALSE);
		m_cStop.EnableWindow(FALSE);
		break;
	}
	}

	if (strMessageSecond.GetLength() == 0)
	{
		if (m_bSkipUpdate)
			strMessageSecond += ZGetString("STR_112");
		strMessageSecond += "    ";
	}

	if (fPercent < 0.0f)		fPercent = 0.0f;
	if (fPercent > 100.0f)		fPercent = 100.0f;

	strMessageFirst = "     " + strMessageFirst;
	DrawPanel(10, 460, 500, 70, strMessageFirst);

#ifdef _SKINBRZ
	GetScreen()->PutText(20, 489, 15, "Arial", strMessageSecond, GetTextColor());
#else
	GetScreen()->PutText(23, 489, 15, "Arial", strMessageSecond, GetTextColor());
#endif

	float fGausse = 489.0f * (fPercent / 100.0f) + 1.5f * (float)sin((double)timeGetTime() / 100.0f);
	if ((fGausse < 0.0f) || (fPercent < 0.01f))		fGausse = 0.0f;
	if ((fGausse > 489.0f) || (fPercent > 99.99f)) 	fGausse = 489.0f;

	GetScreen()->PutBitmap(16, 506, GetSkin()->GetDC(), 0, 86, 489, 19);
	GetScreen()->PutBitmap(16, 506, GetSkin()->GetDC(), 0, 63, (int)fGausse, 19);

	if (fPercent > 0.0f)
	{
		CString strPercent;
		strPercent.Format("%.1f%%", fPercent);

#ifdef _SKINBRZ
		GetScreen()->PutText(16, 508, 489, 19, 15, "Arial", strPercent.GetBuffer(), GetTextColor(), TS_NORMAL, DT_CENTER);
#else
		GetScreen()->PutText(17, 507, 489, 19, 15, "Arial", strPercent.GetBuffer(), RGB(50, 50, 50), TS_NORMAL, DT_CENTER);
		GetScreen()->PutText(16, 506, 489, 19, 15, "Arial", strPercent.GetBuffer(), RGB(200, 200, 200), TS_NORMAL, DT_CENTER);
#endif
	}

	Rendering(10, 460, 500, 70);
}

void CNewAppDlg::RunExternalUpdater()
{
	HRSRC hRes = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_UPDATER), _T("EXE"));
	if (hRes == NULL)
		return;

	HGLOBAL hData = ::LoadResource(AfxGetResourceHandle(), hRes);
	if (hData == NULL)
		return;

	LPVOID lpData = LockResource(hData);

	DWORD dwSize = SizeofResource(AfxGetInstanceHandle(), hRes);
	DWORD dwWriteSize = 0;
	HANDLE hFile = ::CreateFile(STR_EXT_UPDATER_FILENAME, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		::WriteFile(hFile, lpData, dwSize, &dwWriteSize, NULL);
		::CloseHandle(hFile);

		Sleep(1000);

		if (dwSize == dwWriteSize)
		{
			PutLog("[APP] Run external updater");

			char szCmdLine[4096] = { 0, };

			sprintf(szCmdLine, ".\\%s", STR_EXT_UPDATER_FILENAME);

			WinExec(szCmdLine, SW_SHOW);
		}

		else
			PutLog("[APP] Invalid updater file");
	}

	else
		PutLog("[APP] Cannot create updater file");

	FreeResource(hData);
}

void CNewAppDlg::GetRunInfo()
{
	CFile file;
	CFileException ex;

	//if (m_bIsAlpha)
	//	PutLog("[APP] Set alpha mode");
}

void CNewAppDlg::OnAbout()
{
	CString strMsg;
	strMsg.Format("%s Launcher\nCopyright (C) GunZ, All rights reserved to Masangsoft Inc.\n\n%s Version : %s\nLauncher Version : %s",
		STR_GUNZ_PROCESS_NAME, STR_GUNZ_PROCESS_NAME, m_szClientVersion, m_szLauncherVersion);

	CNewAppAboutBox dlg;
	dlg.SetDialog(this, "About", strMsg, MB_OK);
	this->SetFocus();
	dlg.DoModal();
}

void CNewAppDlg::OnOK()
{
}

void CNewAppDlg::OnCancel()
{
	if (m_nStatus == STATUS_UPDATING)
		OnBnClickedStop();

	else
		OnQuit();
}

void CNewAppDlg::OnQuit()
{
	m_bReservedQuit = true;

	if (m_nStatus == STATUS_UPDATING)
		OnBnClickedStop();
}

void CNewAppDlg::OnDestroyDialog()
{
	m_Update.Destroy();

	CRect rect;
	GetWindowRect(rect);
	AfxGetApp()->WriteProfileInt("Window Position", "x", rect.left);
	AfxGetApp()->WriteProfileInt("Window Position", "y", rect.top);

	CDialog::OnOK();
}

void CNewAppDlg::GetFileVersion(const char* pszFileName, char* pszVersion)
{
	strcpy(pszVersion, "N/A");

	DWORD dwHandle;
	DWORD dwLength = GetFileVersionInfoSize(pszFileName, &dwHandle);

	if (!dwLength)
		return;

	std::auto_ptr<BYTE> lpBlock(new BYTE[dwLength]);

	if (!GetFileVersionInfoA(pszFileName, dwHandle, dwLength, lpBlock.get()))
		return;

	LPVOID pVersion = NULL;
	DWORD uLength;
	if (!VerQueryValue(lpBlock.get(), "\\VarFileInfo\\Translation", (LPVOID*)&pVersion, (UINT*)&uLength))
		return;

	CString rVersion;
	if (uLength == 4)
	{
		DWORD langD;
		memcpy(&langD, pVersion, 4);

		rVersion.Format("\\StringFileInfo\\%02X%02X%02X%02X\\ProductVersion",
			(langD & 0xff00) >> 8,
			(langD & 0xff),
			(langD & 0xff000000) >> 24,
			(langD & 0xff0000) >> 16);
	}
	else
	{
		rVersion.Format("\\StringFileInfo\\%04X04B0\\ProductVersion", GetUserDefaultLangID());
	}

	if (VerQueryValue(lpBlock.get(), (LPSTR)(LPCTSTR)rVersion, (LPVOID*)&pVersion, (UINT *)&uLength) == 0)
		return;

	strcpy(pszVersion, (char*)pVersion);
}

#define SM_SERVERR2		89
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);

typedef NTSTATUS(WINAPI* PRTLGETVERSION)(PRTL_OSVERSIONINFOEXW);

// Hàm để lấy thông tin phiên bản hệ điều hành chính xác
BOOL GetOSVersionInfo(OSVERSIONINFOEX* pOsvi)
{
	// Sử dụng RtlGetVersion từ ntdll.dll để lấy thông tin phiên bản chính xác
	HMODULE hNtDll = GetModuleHandle(TEXT("ntdll.dll"));
	if (!hNtDll)
		return FALSE;

	PRTLGETVERSION pRtlGetVersion = (PRTLGETVERSION)GetProcAddress(hNtDll, "RtlGetVersion");
	if (!pRtlGetVersion)
		return FALSE;

	RTL_OSVERSIONINFOEXW versionInfo = { 0 };
	versionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
	NTSTATUS status = pRtlGetVersion(&versionInfo);
	if (status != 0)
		return FALSE;

	// Chuyển đổi từ RTL_OSVERSIONINFOEXW sang OSVERSIONINFOEX
	pOsvi->dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	pOsvi->dwMajorVersion = versionInfo.dwMajorVersion;
	pOsvi->dwMinorVersion = versionInfo.dwMinorVersion;
	pOsvi->dwBuildNumber = versionInfo.dwBuildNumber;
	pOsvi->dwPlatformId = versionInfo.dwPlatformId;
	//wcscpy_s(pOsvi->szCSDVersion, versionInfo.szCSDVersion);
	pOsvi->wServicePackMajor = versionInfo.wServicePackMajor;
	pOsvi->wServicePackMinor = versionInfo.wServicePackMinor;
	pOsvi->wSuiteMask = versionInfo.wSuiteMask;
	pOsvi->wProductType = versionInfo.wProductType;
	pOsvi->wReserved = versionInfo.wReserved;

	return TRUE;
}

int CNewAppDlg::GetOSVersion(char* pszBuff)
{
	int nMajorID = -1;

	memset(pszBuff, 0, sizeof(char) * 128); // Đảm bảo kích thước phù hợp với pszBuff

	OSVERSIONINFOEX osvi;
	SYSTEM_INFO si;
	PGNSI pGNSI;
	BOOL bOsVersionInfoEx;

	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

	// Sử dụng RtlGetVersion để lấy thông tin phiên bản chính xác
	if (!GetOSVersionInfo(&osvi))
	{
		// Nếu RtlGetVersion thất bại, thử dùng GetVersionEx (phương pháp cũ)
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi)))
		{
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			if (!GetVersionEx((OSVERSIONINFO*)&osvi))
				return -1;
		}
	}
	else
	{
		bOsVersionInfoEx = TRUE;
	}

	pGNSI = (PGNSI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
	if (NULL != pGNSI)        pGNSI(&si);
	else                      GetSystemInfo(&si);

	nMajorID = osvi.dwMajorVersion;

	switch (osvi.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:
		// Windows 11 và Windows 10
		if (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0)
		{
			if (osvi.wProductType == VER_NT_WORKSTATION)
			{
				if (osvi.dwBuildNumber >= 22000)
					strcat(pszBuff, "Windows 11 ");
				else
					strcat(pszBuff, "Windows 10 ");
			}
			else
			{
				if (osvi.dwBuildNumber >= 22000)
					strcat(pszBuff, "Windows Server 2022 ");
				else
					strcat(pszBuff, "Windows Server 2016/2019 ");
			}
		}
		// Windows 8.1
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3)
		{
			if (osvi.wProductType == VER_NT_WORKSTATION)
				strcat(pszBuff, "Windows 8.1 ");
			else
				strcat(pszBuff, "Windows Server 2012 R2 ");
		}
		// Windows 8
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2)
		{
			if (osvi.wProductType == VER_NT_WORKSTATION)
				strcat(pszBuff, "Windows 8 ");
			else
				strcat(pszBuff, "Windows Server 2012 ");
		}
		// Windows 7
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
		{
			if (osvi.wProductType == VER_NT_WORKSTATION)
				strcat(pszBuff, "Windows 7 ");
			else
				strcat(pszBuff, "Windows Server 2008 R2 ");
		}
		// Windows Vista
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
		{
			if (osvi.wProductType == VER_NT_WORKSTATION)
				strcat(pszBuff, "Windows Vista ");
			else
				strcat(pszBuff, "Windows Server \"Longhorn\" ");
		}
		// Windows Server 2003
		else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
		{
			if (GetSystemMetrics(SM_SERVERR2))
				strcat(pszBuff, "Microsoft Windows Server 2003 \"R2\" ");
			else if (osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
				strcat(pszBuff, "Microsoft Windows XP Professional x64 Edition ");
			else
				strcat(pszBuff, "Microsoft Windows Server 2003, ");
		}
		// Windows XP
		else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
			strcat(pszBuff, "Microsoft Windows XP ");
		// Windows 2000
		else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
			strcat(pszBuff, "Microsoft Windows 2000 ");
		// Windows NT
		else if (osvi.dwMajorVersion <= 4)
			strcat(pszBuff, "Microsoft Windows NT ");

		if (bOsVersionInfoEx)
		{
			if (osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_AMD64)
			{
				if (osvi.dwMajorVersion == 4)
					strcat(pszBuff, "Workstation 4.0 ");
				else if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
					strcat(pszBuff, "Home Edition ");
				else
					strcat(pszBuff, "Professional ");
			}
			else if (osvi.wProductType == VER_NT_SERVER || osvi.wProductType == VER_NT_DOMAIN_CONTROLLER)
			{
				if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
				{
					if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
					{
						if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
							strcat(pszBuff, "Datacenter Edition for Itanium-based Systems ");
						else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
							strcat(pszBuff, "Enterprise Edition for Itanium-based Systems ");
					}
					else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
					{
						if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
							strcat(pszBuff, "Datacenter x64 Edition ");
						else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
							strcat(pszBuff, "Enterprise x64 Edition ");
						else
							strcat(pszBuff, "Standard x64 Edition ");
					}
					else
					{
						if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
							strcat(pszBuff, "Datacenter Edition ");
						else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
							strcat(pszBuff, "Enterprise Edition ");
						else if (osvi.wSuiteMask & VER_SUITE_BLADE)
							strcat(pszBuff, "Web Edition ");
						else
							strcat(pszBuff, "Standard Edition ");
					}
				}
				else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
				{
					if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
						strcat(pszBuff, "Datacenter Server ");
					else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						strcat(pszBuff, "Advanced Server ");
					else
						strcat(pszBuff, "Server ");
				}
				else if (osvi.dwMajorVersion <= 4)
				{
					if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						strcat(pszBuff, "Server 4.0, Enterprise Edition ");
					else
						strcat(pszBuff, "Server 4.0 ");
				}
			}
		}
		else
		{
#define BUFSIZE 80
			HKEY hKey;
			TCHAR szProductType[BUFSIZE];
			DWORD dwBufLen = BUFSIZE * sizeof(TCHAR);
			LONG lRet;

			lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"), 0, KEY_QUERY_VALUE, &hKey);
			if (lRet != ERROR_SUCCESS)
				return -1;

			lRet = RegQueryValueEx(hKey, TEXT("ProductType"), NULL, NULL, (LPBYTE)szProductType, &dwBufLen);
			RegCloseKey(hKey);

			if ((lRet != ERROR_SUCCESS) || (dwBufLen > BUFSIZE * sizeof(TCHAR)))
				return -1;

			if (lstrcmpi(TEXT("WINNT"), szProductType) == 0)
				strcat(pszBuff, "Workstation ");
			if (lstrcmpi(TEXT("LANMANNT"), szProductType) == 0)
				strcat(pszBuff, "Server ");
			if (lstrcmpi(TEXT("SERVERNT"), szProductType) == 0)
				strcat(pszBuff, "Advanced Server ");

			char szTmp[128];
			sprintf(szTmp, "%d.%d ", osvi.dwMajorVersion, osvi.dwMinorVersion);
			strcat(pszBuff, szTmp);
		}

		if (osvi.dwMajorVersion == 4 && lstrcmpi(osvi.szCSDVersion, TEXT("Service Pack 6")) == 0)
		{
			HKEY hKey;
			LONG lRet;

			lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009"), 0, KEY_QUERY_VALUE, &hKey);
			if (lRet == ERROR_SUCCESS)
			{
				char szTmp[128];
				sprintf(szTmp, "Service Pack 6a (Build %d)", osvi.dwBuildNumber & 0xFFFF);
				strcat(pszBuff, szTmp);
			}
			else
			{
				char szTmp[128];
				sprintf(szTmp, "%s (Build %d)", osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
				strcat(pszBuff, szTmp);
			}

			RegCloseKey(hKey);
		}
		else
		{
			char szTmp[128];
			sprintf(szTmp, "%s (Build %d)", osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
			strcat(pszBuff, szTmp);
		}

		break;

	case VER_PLATFORM_WIN32_WINDOWS:
		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
		{
			strcat(pszBuff, "Microsoft Windows 95 ");
			if (osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B')
				strcat(pszBuff, "OSR2");
		}
		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
		{
			strcat(pszBuff, "Microsoft Windows 98 ");
			if (osvi.szCSDVersion[1] == 'A' || osvi.szCSDVersion[1] == 'B')
				strcat(pszBuff, "SE");
		}
		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
			strcat(pszBuff, "Microsoft Windows Millennium Edition");
		break;

	case VER_PLATFORM_WIN32s:
		strcat(pszBuff, "Microsoft Win32s");
		break;
	}

	return nMajorID;
}

BEGIN_EVENTSINK_MAP(CNewAppDlg, CDDBDialog)
	ON_EVENT(CNewAppDlg, IDC_EXPLORER, 259, DocumentCompleteExplorer, VTS_DISPATCH VTS_PVARIANT)
END_EVENTSINK_MAP()

void CNewAppDlg::DocumentCompleteExplorer(LPDISPATCH pDisp, VARIANT* URL)
{
	HRESULT		hr;
	LPUNKNOWN	lpUnknown;
	LPUNKNOWN	lpUnknownWB = NULL;
	LPUNKNOWN	lpUnknownDC = NULL;

	lpUnknown = m_cExplorer.GetControlUnknown();
	if (lpUnknown)
	{
		hr = lpUnknown->QueryInterface(IID_IUnknown, (LPVOID*)&lpUnknownWB);
		if (FAILED(hr))
			return;

		hr = pDisp->QueryInterface(IID_IUnknown, (LPVOID*)&lpUnknownDC);
		if (FAILED(hr))
			return;

		if (lpUnknownWB == lpUnknownDC)
		{
			m_bNavigationComplete = true;
		}

		if (lpUnknownWB)
			lpUnknownWB->Release();

		if (lpUnknownDC)
			lpUnknownDC->Release();
	}
}

bool CNewAppDlg::ShowLicenseDlg()
{
	CNewAppLicenseConfirmDlg dlg;
	dlg.SetDialog(this, ZGetString("STR_149"), "EasyAntiCheat/license.htm", ZGetString("STR_150"), ZGetString("STR_151"), ZGetString("STR_152"));

	int nRetVal = dlg.DoModal();
	if (nRetVal == IDOK)
		return true;

	return false;
}