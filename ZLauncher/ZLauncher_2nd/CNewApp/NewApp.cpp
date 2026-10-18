#include <stdafx.h>
#include "afxwin.h"
#include "NewApp.h"
#include "NewAppDlg.h"
#include <Shlwapi.h>
#include <string>

#pragma comment(lib, "Shlwapi.lib")
#define _EAC 1
CNewApp theApp;

// Constructor must be defined
#ifdef _EAC
CNewApp::CNewApp()
{
}
BEGIN_MESSAGE_MAP(CNewApp, CWinApp)
END_MESSAGE_MAP()

HANDLE g_hMutex = NULL;

bool InstallEOS()
{
	STARTUPINFOA si{};
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi{};

	char cmdLine[MAX_PATH] =
		"\"EasyAntiCheat\\EasyAntiCheat_EOS_Setup.exe\" install a91729130eb949eaa22ef2e3bd7ea5cb";

	BOOL ok = CreateProcessA(
		NULL,
		cmdLine,
		NULL,
		NULL,
		FALSE,
		CREATE_NO_WINDOW,
		NULL,
		NULL,
		&si,
		&pi
	);

	if (!ok)
		return false;

	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
}

bool IsEOSInstalled()
{
	HKEY hKey;
	char value[128];
	DWORD size = sizeof(value);

	if (RegOpenKeyExA(
		HKEY_LOCAL_MACHINE,
		"SOFTWARE\\WOW6432Node\\EasyAntiCheat_EOS",
		0,
		KEY_READ,
		&hKey) != ERROR_SUCCESS)
		return false;

	bool ok =
		RegQueryValueExA(
			hKey,
			"ProductsInstalled",
			nullptr,
			nullptr,
			(LPBYTE)value,
			&size
		) == ERROR_SUCCESS
		&& strstr(value, "a91729130eb949eaa22ef2e3bd7ea5cb") != nullptr;

	RegCloseKey(hKey);
	return ok;
}

BOOL CNewApp::InitInstance()
{
	BOOL bFound = FALSE;
	g_hMutex = ::CreateMutex(NULL, TRUE, "ZGunzLauncherMutex");

	if (::GetLastError() == ERROR_ALREADY_EXISTS)
		bFound = TRUE;

	if (g_hMutex)
		::ReleaseMutex(g_hMutex);

	if (bFound)
		return FALSE;

	AfxEnableControlContainer();
	AfxInitRichEdit();
	
	// 🔥 EOS CHECK TRƯỚC KHI HIỆN UI
	if (!IsEOSInstalled())
	{
		/*int ret = MessageBoxA(
			NULL,
			"Epic Online Services is required to run this game.\n\nInstall now?",
			"Easy Anti-Cheat",
			MB_YESNO | MB_ICONQUESTION
		);

		if (ret != IDYES)
			return FALSE;*/

		if (!InstallEOS())
		{
			MessageBoxA(
				NULL,
				"Failed to install Epic Online Services.",
				"Error",
				MB_OK | MB_ICONERROR
			);
			return FALSE;
		}
	}

	CNewAppDlg dlg;
	dlg.DoModal();

	return false;
}

//BOOL CNewApp::InitInstance()
//{
//    // Tạo mutex với tên độc đáo
//    const wchar_t* mutexName = L"ZGunzLauncher_UniqueMutex_2025";
//    g_hMutex = ::CreateMutexW(NULL, TRUE, mutexName);
//    DWORD lastError = ::GetLastError();
//    BOOL bFound = FALSE;
//    if (lastError == ERROR_ALREADY_EXISTS)
//    {
//        ::CloseHandle(g_hMutex);
//        g_hMutex = NULL;
//        //MessageBoxW(NULL, L"An instance of the application is already running.", L"Application Error", MB_OK | MB_ICONERROR);
//        return FALSE;
//    }
//    else if (g_hMutex == NULL)
//    {
//        MessageBoxW(NULL, L"Failed to initialize application mutex.", L"Application Error", MB_OK | MB_ICONERROR);
//        return FALSE;
//    }
//
//    // Lấy đường dẫn ứng dụng
//    wchar_t exePath[MAX_PATH];
//    if (GetModuleFileNameW(NULL, exePath, MAX_PATH) == 0)
//    {
//        MessageBoxW(NULL, L"Failed to retrieve application path.", L"Application Error", MB_OK | MB_ICONERROR);
//        ::CloseHandle(g_hMutex);
//        g_hMutex = NULL;
//        return FALSE;
//    }
//
//    PathRemoveFileSpecW(exePath);
//    std::wstring helperExe = std::wstring(exePath) + L"\\start_protected_game.exe";
//
//    // Kiểm tra sự tồn tại của file
//    if (!PathFileExistsW(helperExe.c_str()))
//    {
//        MessageBoxW(NULL, L"Required component 'start_protected_game.exe' not found.", L"Easy Anti-Cheat", MB_OK | MB_ICONERROR);
//        ::CloseHandle(g_hMutex);
//        g_hMutex = NULL;
//        return FALSE;
//    }
//
//    // Khởi chạy tiến trình
//    STARTUPINFOW si = { sizeof(si) };
//    PROCESS_INFORMATION pi = { 0 };
//    std::wstring cmdLine = L"\"" + helperExe + L"\"";
//
//    if (!CreateProcessW(helperExe.c_str(), &cmdLine[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
//    {
//        MessageBoxW(NULL, L"Failed to launch 'start_protected_game.exe'.", L"Easy Anti-Cheat", MB_OK | MB_ICONERROR);
//        ::CloseHandle(g_hMutex);
//        g_hMutex = NULL;
//        return FALSE;
//    }
//
//    // Chờ tiến trình hoàn tất
//    WaitForSingleObject(pi.hProcess, INFINITE);
//
//    DWORD exitCode = 1;
//    GetExitCodeProcess(pi.hProcess, &exitCode);
//    CloseHandle(pi.hThread);
//    CloseHandle(pi.hProcess);
//
//    if (exitCode != 0)
//    {
//        MessageBoxW(NULL, L"Anti-cheat verification failed.", L"Easy Anti-Cheat", MB_OK | MB_ICONERROR);
//        ::CloseHandle(g_hMutex);
//        g_hMutex = NULL;
//        return FALSE;
//    }
//
//    // Tiếp tục khởi tạo ứng dụng
//    AfxEnableControlContainer();
//    AfxInitRichEdit();
//
//    CNewAppDlg dlg;
//    dlg.DoModal();
//
//    // Đóng mutex trước khi thoát
//    if (g_hMutex)
//    {
//        ::CloseHandle(g_hMutex);
//        g_hMutex = NULL;
//    }
//
//    return FALSE;
//}
#else
CNewApp::CNewApp()
{
}
BEGIN_MESSAGE_MAP(CNewApp, CWinApp)
END_MESSAGE_MAP()

HANDLE g_hMutex = NULL;

BOOL CNewApp::InitInstance()
{
	BOOL bFound = FALSE;
	g_hMutex = ::CreateMutex(NULL, TRUE, "ZGunzLauncherMutex");

	if (::GetLastError() == ERROR_ALREADY_EXISTS)
		bFound = TRUE;

	if (g_hMutex)
		::ReleaseMutex(g_hMutex);

	if (bFound)
		return FALSE;

	AfxEnableControlContainer();
	AfxInitRichEdit();

	CNewAppDlg dlg;
	dlg.DoModal();

	return false;
}
#endif