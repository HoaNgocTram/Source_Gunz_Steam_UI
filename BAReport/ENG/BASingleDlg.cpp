// BASingleDlg.cpp : ±¸Çö ÆÄÀÏÀÔ´Ï´Ù.
//

#include "stdafx.h"
#include "BAReport.h"
#include "BASingleDlg.h"
#include "BASysInfo.h"
#include "FileTransfer.h"
#include "BAReportDlg.h"
#include <string>
#include <vector>
using namespace std;


// BASingleDlg ´ëÈ­ »óÀÚÀÔ´Ï´Ù.

IMPLEMENT_DYNAMIC(BASingleDlg, CDialog)
BASingleDlg::BASingleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(BASingleDlg::IDD, pParent)
{
}

BASingleDlg::~BASingleDlg()
{
}

void BASingleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_NAME, m_Name);
	DDX_Control(pDX, IDC_EDIT_SYMPTOM, m_Symptom);
}


BEGIN_MESSAGE_MAP(BASingleDlg, CDialog)
	ON_EN_CHANGE(IDC_EDIT_SYMPTOM, &BASingleDlg::OnEnChangeEditSymptom)
END_MESSAGE_MAP()

BOOL BASingleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 1. Ép Font Tahoma (để hiển thị được dấu)
	CFont* pFont = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	m_Symptom.SetFont(pFont);

	// 2. CHIÊU QUAN TRỌNG: Ép Rich Edit sử dụng chế độ Unicode
	// Gửi thông điệp EM_SETTEXTMODE để ép nó chạy chuẩn Unicode (TM_RICHTEXT | TM_UNICODE)
	::SendMessage(m_Symptom.GetSafeHwnd(), 0x0459 /*EM_SETTEXTMODE*/, 3 /*TM_RICHTEXT | TM_UNICODE*/, 0);

	// TODO:  ¿©±â¿¡ Ãß°¡ ÃÊ±âÈ­ ÀÛ¾÷À» Ãß°¡ÇÕ´Ï´Ù.

	m_Name.SetWindowText("NULL");
	m_Name.SetSel(0,-1);
	m_Name.SetFocus();
	return FALSE;  // return TRUE unless you set the focus to a control
	// ¿¹¿Ü: OCX ¼Ó¼º ÆäÀÌÁö´Â FALSE¸¦ ¹ÝÈ¯ÇØ¾ß ÇÕ´Ï´Ù.
}

static DWORD CALLBACK StreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb)
{
	std::vector<char>* pVec = (std::vector<char>*)dwCookie;
	pVec->insert(pVec->end(), pbBuff, pbBuff + cb);
	*pcb = cb;
	return 0;
}

void BASingleDlg::OnOK()
{
	CWnd* pStatic = GetDlgItem(IDC_STATIC_SINGLE);
	pStatic->SetWindowText("GunZ is now gathering your system information. Please wait.");

	time_t currtime;
	time(&currtime);
	struct tm* pTM = localtime(&currtime);

	CString strName;
	m_Name.GetWindowText(strName);

	char szRemoteFileName[256];
	wsprintf(szRemoteFileName, "incoming/info/_report_%s_%.2d%.2d_%.2d%.2d.txt",
		(LPCSTR)strName, pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min);

#define SYSINFO_FILENAME	"sysinfo.txt"

	// Sử dụng "wb" (Write Binary) để kiểm soát chính xác từng byte ghi xuống
	FILE* pFile = fopen(SYSINFO_FILENAME, "wb");
	if (pFile) {
		// 1. Ghi BOM UTF-8
		unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
		fwrite(bom, 1, 3, pFile);

		// 2. LẤY UNICODE TRỰC TIẾP (Bỏ qua CString strSymptom)
		HWND hWndRich = m_Symptom.GetSafeHwnd();
		int nLen = ::GetWindowTextLengthW(hWndRich);

		if (nLen > 0) {
			// Lấy chuỗi WideChar (Unicode 16-bit)
			std::vector<wchar_t> wbuf(nLen + 1);
			::GetWindowTextW(hWndRich, &wbuf[0], nLen + 1);

			// 3. CHUYỂN UNICODE SANG UTF-8
			int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0, &wbuf[0], -1, NULL, 0, NULL, NULL);
			if (nUtf8Len > 0) {
				std::vector<char> utf8buf(nUtf8Len);
				WideCharToMultiByte(CP_UTF8, 0, &wbuf[0], -1, &utf8buf[0], nUtf8Len, NULL, NULL);

				// Ghi dữ liệu UTF-8 vào file (loại bỏ ký tự NULL kết thúc chuỗi)
				fwrite(&utf8buf[0], 1, (DWORD)strlen(&utf8buf[0]), pFile);
			}
		}

		// Ghi dấu xuống dòng chuẩn
		const char* pNewLine = "\r\n";
		fwrite(pNewLine, 1, 2, pFile);
		fclose(pFile);

		// Tiếp tục quy trình thu thập thông tin hệ thống của MAIET
		BASysInfo(SYSINFO_FILENAME);

		// Gửi file lên server qua FTP
		g_pFileTransfer->PutFileQueue("update.gunz.vn", 21, "ftp", "ftp", SYSINFO_FILENAME, szRemoteFileName);

		CBAReportDlg* pDlg = (CBAReportDlg*)AfxGetMainWnd();
		pDlg->Upload();
		DestroyWindow();
	}
	else {
		// Thông báo lỗi nếu không mở được file (bằng tiếng Hàn gốc của anh hoặc tiếng Anh)
		AfxMessageBox("Failed to save report file.");
	}
}

void BASingleDlg::OnCancel()
{
	// TODO: ¿©±â¿¡ Æ¯¼öÈ­µÈ ÄÚµå¸¦ Ãß°¡ ¹×/¶Ç´Â ±âº» Å¬·¡½º¸¦ È£ÃâÇÕ´Ï´Ù.
	CBAReportDlg* pDlg = (CBAReportDlg*)AfxGetMainWnd();
	pDlg->DestroyWindow();

//	CDialog::OnCancel();
}

void BASingleDlg::OnEnChangeEditSymptom()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
