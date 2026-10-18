// MatchServer.cpp : ÀÀ¿ë ÇÁ·Î±×·¥¿¡ ´ëÇÑ Å¬·¡½º µ¿ÀÛÀ» Á¤ÀÇÇÕ´Ï´Ù.
//

#include "stdafx.h"
#include "MatchServer.h"
#include "MainFrm.h"

#include <shlwapi.h>

#include "ChildFrm.h"
#include "MatchServerDoc.h"
#include "MatchServerView.h"
#include "OutputView.h"
#include "CommandLogView.h"
#include "MRegistry.h"
#include "matchserver.h"
#include "MBMatchServer.h"
#include "MDebug.h"
#include "MSync.h"
#include "MMatchConfig.h"
#include "MTraceMemory.h"
#include "MMatchCheckLoopTime.h"
#include "MMatchStatus.h"

#define _LICENSE
#ifdef _LICENSE
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <vector>
#pragma comment(lib, "winhttp.lib")
#endif
#ifdef _DEBUG
#define new DEBUG_NEW

// ÀÌ°ÍÀº Å×½ºÆ®¿ë..bird
//#define _FETCH_112

#endif


#ifdef _FETCH_112
	#include "MInet.h"
	MHttp g_Http;
#endif

#define APPLICATION_NAME	"MatchServer"

// À¯ÀÏÇÑ CMatchServerApp °³Ã¼ÀÔ´Ï´Ù.
CMatchServerApp			theApp;
// Custom: MatchServer Mutex name changed
//MSingleRunController	g_SingleRunController("FGMatchServer"); 

// CMatchServerApp

BEGIN_MESSAGE_MAP(CMatchServerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Ç¥ÁØ ÆÄÀÏÀ» ±âÃÊ·Î ÇÏ´Â ¹®¼­ ¸í·ÉÀÔ´Ï´Ù.
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Ç¥ÁØ ÀÎ¼â ¼³Á¤ ¸í·ÉÀÔ´Ï´Ù.
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND(ID_ViewServerStatus, OnViewServerStatus)
	ON_COMMAND(ID_MESSAGE_EXIT, OnMessageExit)
	ON_COMMAND(ID_SHOW_CMD_LOG, OnShowCmdLog)
	ON_COMMAND(ID_USE_COUNTRY_FILTER, OnSetUseCountryFilter)
	ON_COMMAND(ID_ACCEPT_INVAILD_IP, OnSetAccetpInvalidIP)
	ON_COMMAND(ID_UPDATE_IPtoCOUNTRY, OnUpdateIPtoCountry)
	ON_COMMAND(ID_UPDATE_BLOCK_COUNTRY_CODE, OnUpdateBlockCountryCode)
	ON_COMMAND(ID_UPDATE_CUSTOM_IP, OnUpdateCustomIP)
	ON_UPDATE_COMMAND_UI(ID_SHOW_CMD_LOG, OnUpdateShowCmdLog)
	ON_UPDATE_COMMAND_UI(ID_USE_COUNTRY_FILTER, OnUseCountryFilter)
	ON_UPDATE_COMMAND_UI(ID_ACCEPT_INVAILD_IP, OnAcceptInvalidIP)
	ON_UPDATE_COMMAND_UI(ID_UPDATE_IPtoCOUNTRY, OnEnableUpdateIPtoCountry)
	ON_UPDATE_COMMAND_UI(ID_UPDATE_BLOCK_COUNTRY_CODE, OnEnableUpdateBlockCountryCode)
	ON_UPDATE_COMMAND_UI(ID_UPDATE_CUSTOM_IP, OnEnableUpdateCustomIP)
	ON_COMMAND(ID_TOOL_TEST, OnToolTest)
	// Custom: Reload Config via server
	ON_COMMAND(ID_RELOADCONFIG, OnReloadConfig)
END_MESSAGE_MAP()

#ifdef _LICENSE
std::string GetHWID() {
	DWORD serial = 0;
	GetVolumeInformationA("C:\\", NULL, 0, &serial, NULL, NULL, NULL, 0);
	char buf[64];
	sprintf(buf, "%08X", serial);
	return std::string(buf);
}

// Tự định nghĩa LicStr riêng để không đụng hàng với XorStr của Source
template <unsigned int XORSTART, unsigned int BUFLEN, unsigned int XREFKILLER>
class LicStr
{
public:
	char s[BUFLEN];

	LicStr(const char* xs) {
		auto xvalue = XORSTART;
		for (auto i = 0; i < (BUFLEN - 1); i++) {
			// Logic giống y hệt tool PHP
			s[i] = xs[i] ^ xvalue;
			xvalue = (xvalue + 1) % 256;
		}
		s[BUFLEN - 1] = 0;
	}

	// Hàm trả về string thường
	std::string str() { return std::string(s); }

	// Hàm trả về wstring cho WinHttp
	std::wstring wstr() {
		std::string ts(s);
		return std::wstring(ts.begin(), ts.end());
	}

	~LicStr() {
		for (auto i = 0; i < BUFLEN; i++) s[i] = 0; // Xóa dấu vết bộ nhớ
	}
private:
	LicStr();
};

bool VerifyLicense(const std::string& hwid, const std::string& licenseKey, const std::string& appName) {
	bool ok = false;
	HINTERNET hSession = WinHttpOpen(L"LicenseCheck/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) return false;

	HINTERNET hConnect = WinHttpConnect(hSession, L"gunz.vn", INTERNET_DEFAULT_HTTPS_PORT, 0);
	if (hConnect) {
		std::wstring req = L"/license/api/verify.php?key=" +
			std::wstring(licenseKey.begin(), licenseKey.end()) +
			L"&hwid=" + std::wstring(hwid.begin(), hwid.end()) +
			L"&app=" + std::wstring(appName.begin(), appName.end());

		HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", req.c_str(), NULL,
			WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

		if (hRequest) {
			if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
				WINHTTP_NO_REQUEST_DATA, 0, 0, 0) && WinHttpReceiveResponse(hRequest, NULL)) {

				DWORD dwSize = 0;
				WinHttpQueryDataAvailable(hRequest, &dwSize);
				if (dwSize > 0) {
					std::string resp(dwSize, 0);
					DWORD dwRead = 0;
					WinHttpReadData(hRequest, &resp[0], dwSize, &dwRead);

					std::string lower = resp;
					std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

					if (lower.find("\"status\":\"ok\"") != std::string::npos)
						ok = true;
					else if (lower.find("banned") != std::string::npos)
						std::cout << "License banned.\n";
					else if (lower.find("expired") != std::string::npos)
						std::cout << "License expired.\n";
					else if (lower.find("hwid mismatch") != std::string::npos)
						std::cout << "HWID mismatch.\n";
					else if (lower.find("already activated") != std::string::npos)
						std::cout << "This machine already activated with another key.\n";
					else
						std::cout << "Invalid or server error: " << resp << "\n";
				}
			}
			WinHttpCloseHandle(hRequest);
		}
		WinHttpCloseHandle(hConnect);
	}
	WinHttpCloseHandle(hSession);
	return ok;
}
#endif

// CMatchServerApp »ý¼º
CMatchServerApp::CMatchServerApp()
{
	// TODO: ¿©±â¿¡ »ý¼º ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.
	// InitInstance¿¡ ¸ðµç Áß¿äÇÑ ÃÊ±âÈ­ ÀÛ¾÷À» ¹èÄ¡ÇÕ´Ï´Ù.

	m_bTodayRankingRequestDone = false;
}

CMatchServerApp::~CMatchServerApp()
{
#ifdef _MTRACEMEMORY
	MShutdownTraceMemory();
#endif

	if (m_pDocTemplateCmdLogView)
	{
		delete m_pDocTemplateCmdLogView;
	}
}

// CMatchServerApp ÃÊ±âÈ­
BOOL CMatchServerApp::InitInstance()
{
#ifdef _LICENSE
	// ================= LICENSE CHECK ==================
	AllocConsole();
	FILE* pCout = nullptr; FILE* pCin = nullptr;
	freopen_s(&pCout, "CONOUT$", "w", stdout);
	freopen_s(&pCin, "CONIN$", "r", stdin);
	SetConsoleTitleA("MatchServer License Check");

	std::string hwid = GetHWID();
	std::cout << "================ MatchServer License Check ================\n";
	std::cout << "HWID: " << hwid << "\n\n";
	std::cout << "Enter your license key: ";

	std::string key;
	std::getline(std::cin, key);
	while (!key.empty() && (key.back() == '\r' || key.back() == '\n')) key.pop_back();

	if (key.empty()) {
		std::cout << "No key entered. Exiting...\n";
		std::cin.get();
		FreeConsole();
		exit(0);
	}

	bool ok = VerifyLicense(hwid, key, "MatchServer");

	if (!ok) {
		std::cout << "\n License invalid or expired.\n";
		std::cout << "Press ENTER to exit.";
		std::cin.get();
		FreeConsole();
		exit(1);
	}

	std::cout << "\n License verified successfully! Press ENTER to start server...";
	std::cin.get();
	FreeConsole();
	// ==================================================
#endif
#ifdef _MTRACEMEMORY
	MInitTraceMemory();
#endif
//	_CrtSetBreakAlloc(206319);

	m_bOutputLog = 0;

//	MNewMemories::Init();

#ifdef _FETCH_112
	g_Http.Create();
#endif


	//if (g_SingleRunController.Create(true) == false)
	//	return FALSE;


	MRegistry::szApplicationName=APPLICATION_NAME;

	if(m_ZFS.Create(".")==false){
		AfxMessageBox("MAIET Zip File System Initialize Error");
		return FALSE;
	}

	// ÀÀ¿ë ÇÁ·Î±×·¥ ¸Å´ÏÆä½ºÆ®°¡ ComCtl32.dll ¹öÀü 6 ÀÌ»óÀ» »ç¿ëÇÏ¿© ºñÁÖ¾ó ½ºÅ¸ÀÏÀ»
	// »ç¿ëÇÏµµ·Ï ÁöÁ¤ÇÏ´Â °æ¿ì, Windows XP »ó¿¡¼­ ¹Ýµå½Ã InitCommonControls()°¡ ÇÊ¿äÇÕ´Ï´Ù. 
	// InitCommonControls()¸¦ »ç¿ëÇÏÁö ¾ÊÀ¸¸é Ã¢À» ¸¸µé ¼ö ¾ø½À´Ï´Ù.
	InitCommonControls();

	CWinApp::InitInstance();

	// OLE ¶óÀÌºê·¯¸®¸¦ ÃÊ±âÈ­ÇÕ´Ï´Ù.
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Ç¥ÁØ ÃÊ±âÈ­
	// ÀÌµé ±â´ÉÀ» »ç¿ëÇÏÁö ¾Ê°í ÃÖÁ¾ ½ÇÇà ÆÄÀÏÀÇ Å©±â¸¦ ÁÙÀÌ·Á¸é
	// ¾Æ·¡¿¡¼­ ÇÊ¿ä ¾ø´Â Æ¯Á¤ ÃÊ±âÈ­ ·çÆ¾À» Á¦°ÅÇØ¾ß ÇÕ´Ï´Ù.
	// ÇØ´ç ¼³Á¤ÀÌ ÀúÀåµÈ ·¹Áö½ºÆ®¸® Å°¸¦ º¯°æÇÏ½Ê½Ã¿À.
	// TODO: ÀÌ ¹®ÀÚ¿­À» È¸»ç ¶Ç´Â Á¶Á÷ÀÇ ÀÌ¸§°ú °°Àº
	// ÀûÀýÇÑ ³»¿ëÀ¸·Î ¼öÁ¤ÇØ¾ß ÇÕ´Ï´Ù.
	// Custom: Our own registry key
	SetRegistryKey(_T("MAIET entertainment"));
	//SetRegistryKey(_T("·ÎÄÃ ÀÀ¿ë ÇÁ·Î±×·¥ ¸¶¹ý»ç¿¡¼­ »ý¼ºµÈ ÀÀ¿ë ÇÁ·Î±×·¥"));
	LoadStdProfileSettings(4);  // MRU¸¦ Æ÷ÇÔÇÏ¿© Ç¥ÁØ INI ÆÄÀÏ ¿É¼ÇÀ» ·ÎµåÇÕ´Ï´Ù.
	// ÀÀ¿ë ÇÁ·Î±×·¥ÀÇ ¹®¼­ ÅÛÇÃ¸´À» µî·ÏÇÕ´Ï´Ù. ¹®¼­ ÅÛÇÃ¸´Àº
	// ¹®¼­, ÇÁ·¹ÀÓ Ã¢ ¹× ºä »çÀÌÀÇ ¿¬°á ¿ªÇÒÀ» ÇÕ´Ï´Ù.
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_MatchServerTYPE,
		RUNTIME_CLASS(CMatchServerDoc),
		RUNTIME_CLASS(CChildFrame), // »ç¿ëÀÚ ÁöÁ¤ MDI ÀÚ½Ä ÇÁ·¹ÀÓÀÔ´Ï´Ù.
		RUNTIME_CLASS(COutputView));
	AddDocTemplate(pDocTemplate);

	m_pDocTemplateOutput = pDocTemplate;

	// Template
	m_pDocTemplateOutput = pDocTemplate;
	m_pDocTemplateCmdLogView = new CMultiDocTemplate(IDR_MatchServerTYPE,
		RUNTIME_CLASS(CMatchServerDoc),
		RUNTIME_CLASS(CChildFrame), // Custom MDI child frame
		RUNTIME_CLASS(CCommandLogView));


	// ÁÖ MDI ÇÁ·¹ÀÓ Ã¢À» ¸¸µì´Ï´Ù.
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;
	// Á¢¹Ì»ç°¡ ÀÖÀ» °æ¿ì¿¡¸¸ DragAcceptFiles¸¦ È£ÃâÇÕ´Ï´Ù.
	// MDI ÀÀ¿ë ÇÁ·Î±×·¥¿¡¼­´Â m_pMainWnd¸¦ ¼³Á¤ÇÑ ÈÄ ¹Ù·Î ÀÌ·¯ÇÑ È£ÃâÀÌ ¹ß»ýÇØ¾ß ÇÕ´Ï´Ù.
	// Ç¥ÁØ ¼Ð ¸í·É, DDE, ÆÄÀÏ ¿­±â¿¡ ´ëÇÑ ¸í·ÉÁÙÀ» ±¸¹® ºÐ¼®ÇÕ´Ï´Ù.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// ¸í·ÉÁÙ¿¡ ÁöÁ¤µÈ ¸í·ÉÀ» µð½ºÆÐÄ¡ÇÕ´Ï´Ù. ÀÀ¿ë ÇÁ·Î±×·¥ÀÌ /RegServer, /Register, /Unregserver ¶Ç´Â /Unregister·Î ½ÃÀÛµÈ °æ¿ì FALSE¸¦ ¹ÝÈ¯ÇÕ´Ï´Ù.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// ÁÖ Ã¢ÀÌ ÃÊ±âÈ­µÇ¾úÀ¸¹Ç·Î ÀÌ¸¦ Ç¥½ÃÇÏ°í ¾÷µ¥ÀÌÆ®ÇÕ´Ï´Ù.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	//pMainFrame->m_wndConsoleBar.GetDlgItem(IDC_COMBO_COMMAND)->SetFocus();
	//pMainFrame->m_wndConsoleBar.ShowWindow(SW_HIDE);


	// Translation: A floating window in debug mode is annoying, so hide it.
	// Custom: Disable the hide window function.
	// µð¹ö±× ¸ðµåÀÏ¶§´Â ¶ß´Â Ã¢ÀÌ ±ÍÂúÀ¸¹Ç·Î ¾Èº¸ÀÌ°Ô ÇÑ´Ù.
#ifdef _DEBUG
	//m_pMainWnd->ShowWindow(SW_HIDE);
#endif

	return TRUE;
}

// CMatchServerApp ¸Þ½ÃÁö Ã³¸®±â
int CMatchServerApp::ExitInstance()
{
#ifdef _FETCH_112
	g_Http.Destroy();
#endif

	// TODO: Add your specialized code here and/or call the base class
	return CWinApp::ExitInstance();
}

void CMatchServerApp::HeartBeat()
{
	POSITION p = GetFirstDocTemplatePosition(); 
	CDocTemplate* pTemplate = GetNextDocTemplate(p); 
	p = pTemplate->GetFirstDocPosition(); 
	CMatchServerDoc* pDoc = (CMatchServerDoc*)pTemplate->GetNextDoc(p); 
	if(pDoc!=NULL) pDoc->Run();
	Sleep(1);


#ifdef _FETCH_112
	unsigned long int nNowTime=timeGetTime();
	static unsigned long int nLastTime = 0;
	//if ((nNowTime - nLastTime) >= (1000 * 60 * 5))		// 5ºÐ¸¶´Ù ÇÑ¹ø¾¿ fetch
	if ((nNowTime - nLastTime) >= (1000 * 60  * 1))		// 5ºÐ¸¶´Ù ÇÑ¹ø¾¿ fetch
	{
		g_Http.Query("http://192.168.0.31:8080/112.html?mode=fetch");
		nLastTime = nNowTime;
	}
#endif

}

int CMatchServerApp::Run()
{
	ASSERT_VALID(this);
	_AFX_THREAD_STATE* pState = AfxGetThreadState();

	// acquire and dispatch messages until a WM_QUIT message is received.
	for (;;)
	{
		MGetCheckLoopTimeInstance()->SetStartLoop();

		// phase1: check to see if we can do idle work
		if (::PeekMessage(&(pState->m_msgCur), NULL, NULL, NULL, PM_NOREMOVE))
		{
			if (!PumpMessage())
				return ExitInstance();
		}

		if (m_bShutdown == false)
			HeartBeat();
		else
			break;

		MGetCheckLoopTimeInstance()->SetEndLoop();

		RequestDBRankingList();

		if ((MGetServerConfig()->IsUseLoopLog() == true) && (MGetServerConfig()->GetLoopTimeGap() != 0))
		{
			if (MGetCheckLoopTimeInstance()->GetLoopTimeGap() > MGetServerConfig()->GetLoopTimeGap())
			{
				MGetCheckLoopTimeInstance()->SaveLoopLogFile();
			}
		}
	}
	return 0;
}

SYSTEMTIME g_systemTime;
void CMatchServerApp::RequestDBRankingList()
{
	// ÇÏ·ç¿¡ 1È¸ DB¿¡¼­ ¼­¹ÙÀÌ¹ú ·©Å· Á¤º¸¸¦ ¿äÃ»ÇÑ´Ù (DB¿¡¼­ ÇÏ·ç 1È¸ ·©Å· ¼ÒÆÃÀ» ÇÏ±â ¶§¹®)
	// ¿äÃ»	½Ã°¢
	int HOUR = MGetServerConfig()->GetSurvivalRankingDalyRequestTimeHour();
	int MIN = MGetServerConfig()->GetSurvivalRankingDalyRequestTimeMinute();

	// '¿À´ÃÀÇ ¿äÃ» ¿Ï·á' ÇÃ·¡±×¸¦ ²ø ½Ã°£À» ÃæºÐÈ÷ °®±â À§ÇØ 0½Ã 0ºÐÀ¸·Î ¼³Á¤µÇ¾î¼± ¾ÈµÈ´Ù
	// 0½Ã 5ºÐÀ¸·Î ´ÊÃçÁØ´Ù
	if (HOUR == 0 && MIN == 0) {
		MIN = 5;
	}

	::GetLocalTime(&g_systemTime);
	if (g_systemTime.wHour > HOUR || (g_systemTime.wHour==HOUR && g_systemTime.wMinute>=MIN))
	{
		// ¿À´ÃÀÇ ¾÷µ¥ÀÌÆ® ½Ã°¢ÀÌ µÆ´Ù¸é(È¤Àº Áö³µ´Ù¸é) ÇÑ¹ø¸¸ DB¿¡ ¿äÃ»ÇÔ
		if (!m_bTodayRankingRequestDone)
		{
			mlog("Daily Survival Ranking Request [month%d day%d hour%d min%d]\n", g_systemTime.wMonth, g_systemTime.wDay, g_systemTime.wHour, g_systemTime.wMinute);

			MMatchServer::GetInstance()->OnRequestSurvivalModeGroupRanking();
			m_bTodayRankingRequestDone = true;
		}
	}
	else
	{
		m_bTodayRankingRequestDone = false;
	}
}

#include "MMatchStatus.h"
#include ".\matchserver.h"

void CMatchServerApp::OnViewServerStatus()
{
	// TODO: Add your command handler code here
	MMatchServer* pServer = MMatchServer::GetInstance();
	// Custom: Translated string
	if (pServer) pServer->Log(MCommandCommunicator::LOG_PROG, "Server Status View");

	POSITION p = GetFirstDocTemplatePosition(); 
	CDocTemplate* pTemplate = GetNextDocTemplate(p); 
	p = pTemplate->GetFirstDocPosition(); 
	CMatchServerDoc* pDoc = (CMatchServerDoc*)pTemplate->GetNextDoc(p); 

	if(pDoc!=NULL) 
	{
		pDoc->m_pMatchServer->OnViewServerStatus();
//		MNewMemories::Dump();
		MGetServerStatusSingleton()->Dump();

#ifdef _CMD_PROFILE
		pDoc->m_pMatchServer->m_CommandProfiler.Analysis();
#endif
	}
	
}

BOOL CMatchServerApp::PreTranslateMessage(MSG* pMsg)
{
	if(GetKeyState(17)<0)
	{
		if(pMsg->message==WM_KEYDOWN && pMsg->wParam=='P')
		{
			OnViewServerStatus();
			return TRUE;
		}
		if(pMsg->message==WM_KEYDOWN && pMsg->wParam=='C')	// For Crash Test
		{
			return TRUE;
		}
		if(pMsg->message==WM_KEYDOWN && pMsg->wParam=='D') // For UI Debug
		{
			MBMatchServer* pServer = (MBMatchServer*)MMatchServer::GetInstance();
			CRichEditCtrl& c = pServer->m_pView->GetRichEditCtrl();

			return TRUE;
		}
	}

	return CWinApp::PreTranslateMessage(pMsg);
}


// ÀÀ¿ë ÇÁ·Î±×·¥ Á¤º¸¿¡ »ç¿ëµÇ´Â CAboutDlg ´ëÈ­ »óÀÚÀÔ´Ï´Ù.
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ´ëÈ­ »óÀÚ µ¥ÀÌÅÍ
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV Áö¿ø

// ±¸Çö
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// ´ëÈ­ »óÀÚ¸¦ ½ÇÇàÇÏ±â À§ÇÑ ÀÀ¿ë ÇÁ·Î±×·¥ ¸í·ÉÀÔ´Ï´Ù.
void CMatchServerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}
void CMatchServerApp::OnMessageExit()
{
	// TODO: ¿©±â¿¡ ¸í·É Ã³¸®±â ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.
	MMatchServer* pServer = MMatchServer::GetInstance();
	if(pServer)	{
		pServer->Log(MCommandCommunicator::LOG_PROG, "OnMessageExit - Stop Server");
		pServer->OnAdminServerHalt();		
	}
}

void CMatchServerApp::OnShowCmdLog()
{
	// TODO: ¿©±â¿¡ ¸í·É Ã³¸®±â ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.
	m_bOutputLog = 1-(int)m_bOutputLog;
}

void CMatchServerApp::OnUpdateShowCmdLog(CCmdUI *pCmdUI)
{
	// TODO: ¿©±â¿¡ ¸í·É ¾÷µ¥ÀÌÆ® UI Ã³¸®±â ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.
	pCmdUI->SetCheck(m_bOutputLog);
}

void CMatchServerApp::OnUpdateIPtoCountry()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	if(pServer)	
		pServer->UpdateIPtoCountryList();
}


void CMatchServerApp::OnUpdateBlockCountryCode()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	if(pServer)	
		pServer->UpdateBlockCountryCodeLsit();
}


void CMatchServerApp::OnUpdateCustomIP()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	if(pServer)	
		pServer->UpdateCustomIPList();
}


void CMatchServerApp::OnUseCountryFilter(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( MGetServerConfig()->IsUseFilter() );
}


void CMatchServerApp::OnSetUseCountryFilter()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	if(pServer)	
		pServer->SetUseCountryFilter();
}


void CMatchServerApp::OnSetAccetpInvalidIP()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	if(pServer)	
		pServer->SetAccetpInvalidIP();
}


void CMatchServerApp::OnAcceptInvalidIP(CCmdUI* pCmdUI )
{
	// pCmdUI->Enable( MGetServerConfig()->IsUseFilter() );
	pCmdUI->SetCheck( MGetServerConfig()->IsAcceptInvalidIP() );
}


void CMatchServerApp::OnEnableUpdateIPtoCountry( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( MGetServerConfig()->IsUseFilter() );
}


void CMatchServerApp::OnEnableUpdateBlockCountryCode( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( MGetServerConfig()->IsUseFilter() );
}


void CMatchServerApp::OnEnableUpdateCustomIP( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( MGetServerConfig()->IsUseFilter() );
}
void CMatchServerApp::OnToolTest()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	if(pServer)	{
		//MAgentObject *pObj = pServer->FindFreeAgent();		
		//if( pObj == NULL ) return;
		//pServer->Disconnect(pObj->GetUID());
		//pServer->SetUID(MUID(0, 9741612));
	}
}

void CMatchServerApp::OnReloadConfig()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	if (pServer)
	{
		// derp
		pServer->Log(MCommandCommunicator::LOG_PROG, "Reloading configuration..");
		((MBMatchServer*)pServer)->m_ConfigReloader.ReloadObjMapRun();
		pServer->Log(MCommandCommunicator::LOG_PROG, "Configuration reloaded.");
	}
}