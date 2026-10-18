#include "stdafx.h"
//#include "../MatchServer/vld/vld.h"

#ifdef _HSHIELD
#include "HShield/HShield.h"
#endif

#ifdef _XTRAP
#include "./XTrap/Xtrap_C_Interface.h"						// update sgk 0702 start
#include "./XTrap/XTrap4Launcher.h"
#pragma comment (lib, "./XTrap/XTrap4Launcher_mt.lib")
#pragma comment (lib, "./XTrap/XTrap4Client_mt.lib")	// update sgk 0702 end
#endif

#include "ZPrerequisites.h"
#include "ZConfiguration.h"
#include "ZGameClient.h"
#include <windows.h>
#include <wingdi.h>
#include <mmsystem.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <Psapi.h>
#pragma comment (lib, "Psapi.lib")

#include "DxErr.h"


#include "main.h"
#include "resource.h"
#include "VersionNo.h"

#include "Mint4R2.h"
#include "ZApplication.h"
#include "MDebug.h"
#include "ZMessages.h"
#include "MMatchNotify.h"
#include "RealSpace2.h"
#include "Mint.h"
#include "ZGameInterface.h"
#include "RFrameWork.h"
#include "ZButton.h"
#include "ZDirectInput.h"
#include "ZActionDef.h"
#include "MRegistry.h"
#include "ZInitialLoading.h"
#include "MDebug.h"
#include "MCrashDump.h"
#include "ZEffectFlashBang.h"
#include "ZMsgBox.h"
#include "ZSecurity.h"
#include "ZStencilLight.h"
#include "ZReplay.h"
#include "ZUtil.h"
#include "ZOptionInterface.h"
#include "HMAC_SHA1.h"
#ifdef _ANTIHACK
#include "detours.h"
#include "ZAntiHack.h"
#include "ZGuard.h"
#include "AntiCheat.h"
//#include "AntiInject.h"
//#include "AntiInject2.h"
#include "aSB.h"
#include "ZAntiProcess.h"
#include "ZAntiHack28.h"
#endif

#ifdef USING_VERTEX_SHADER
#include "RShaderMgr.h"
#endif

//#include "mempool.h"
#include "RLenzFlare.h"
#include "ZLocale.h"
#include "MSysInfo.h"

#include "MTraceMemory.h"
#include "ZInput.h"
#include "Mint4Gunz.h"
#include "SecurityTest.h"
#include "CheckReturnCallStack.h"
#include "MMD5.h"
#include "RGGlobal.h"
#ifndef KKLF_ACTIVATE
#define KKLF_ACTIVATE       0x00000001
#endif

#ifndef KKLF_SETFORPROCESS
#define KKLF_SETFORPROCESS  0x00000100
#endif
#ifdef _EAC
#include <thread>
#include "eos_platform_prereqs.h"
#include "eos_sdk.h"
#include "eos_common.h"
#include "eos_anticheatclient.h"
#include "eos_anticheatclient_types.h"
#include "eos_anticheatcommon_types.h"
#include "eos_auth.h"
#include "eos_auth_types.h"
#endif
#ifdef _EAC
EOS_HPlatform g_hPlatform = NULL;
EOS_HMetrics  g_hMetrics = NULL;
#endif

#ifdef _LICENSE
#include "ZLicense.h"
#endif
#ifdef _SERIALCUSTOM
#include "ZSerialKey.h"
#endif
#include <stdlib.h>  
#include <crtdbg.h> 
#include <map>
#include <dshow.h>
#include "MZFileSystem.h"
#pragma comment(lib, "strmiids.lib")
#include <dshow.h>

static IGraphBuilder* g_pDSGraph = NULL;
static IMediaControl* g_pDSControl = NULL;
static IMediaEvent* g_pDSEvent = NULL;
static IVideoWindow* g_pDSVideoWnd = NULL;
static wchar_t        g_szTempVideoPath[MAX_PATH] = { 0 }; // Lưu đường dẫn file Temp để xóa sau

// Biến hỗ trợ Thread nạp Data ngầm
static bool g_bAsyncLoadDone = false;
static bool g_bAsyncLoadSuccess = false;

static LARGE_INTEGER g_QPCFreq = { 0 };
static std::map<WPARAM, LARGE_INTEGER> g_KeyTimerQPC;
static std::map<int, LARGE_INTEGER>    g_MouseTimerQPC;
static std::map<WPARAM, int>           g_KeyChainMap;
static int                             g_nMouseChain[2] = { 0, 0 };
static LARGE_INTEGER g_liLastLButtonClick = { 0 };
static int g_nMouseWComboChain = 0;
static LARGE_INTEGER g_liLastKeyTime = { 0 };
static WPARAM        g_lastWParam = 0;
//#include <vld.h>
// Export các biến này để Driver NVIDIA và AMD tự động nhận diện game cần hiệu năng cao
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

HMODULE	g_hUser32;
DWORD g_ThreadLastUpdateTime = 0;
DWORD g_BanLastUpdateTime = 0;
#ifdef _DEBUG
//jintriple3 ¸Þ¸ð¸® ¸¯ vld
//#include "vld.h"
#endif

#include "RGMain.h"

#ifdef _DEBUG
RMODEPARAMS	g_ModeParams={640,480,false,D3DFMT_R5G6B5};
//RMODEPARAMS	g_ModeParams={1024,768,false,RPIXELFORMAT_565};
#else
RMODEPARAMS	g_ModeParams={800,600,true,D3DFMT_R5G6B5};
#endif

#ifndef _DEBUG
#define SUPPORT_EXCEPTIONHANDLING
#endif

// Custom: Disable NHN auth
//#ifdef LOCALE_NHNUSA
//#include "ZNHN_USA.h"
//#include "ZNHN_USA_Report.h"
//#include "ZNHN_USA_Poll.h"
//#endif

#ifdef _GAMEGUARD
#include "ZGameguard.h"
#endif

bool g_bPlayMusicInBackground = false;

RRESULT RenderScene(void *pParam);

#define RD_STRING_LENGTH 512
char cstrReleaseDate[512];// = "ReleaseDate : 12/22/2003";

ZApplication	g_App;
MDrawContextR2* g_pDC = NULL;
MFontR2*		g_pDefFont = NULL;
ZDirectInput	g_DInput;
ZInput*			g_pInput = NULL;
Mint4Gunz		g_Mint;

// Antihack stuff
bool			g_bThreadChecker = true;
DWORD			g_dwThreadId = 0;
HANDLE			g_hIntegrityThread = NULL;
DWORD g_ShouldBanPlayer = 0;
DWORD g_ShouldUpdateCrcTimer = 0;

#ifdef _EAC
bool IsEACServiceRunningA()
{
	SC_HANDLE scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
	if (!scm) return false;

	SC_HANDLE svc = OpenServiceA(scm, "EasyAntiCheat_EOS", SERVICE_QUERY_STATUS);
	if (!svc) {
		CloseServiceHandle(scm);
		return false;
	}

	SERVICE_STATUS_PROCESS ssp;
	DWORD bytes = 0;

	BOOL ok = QueryServiceStatusEx(
		svc,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&ssp,
		sizeof(ssp),
		&bytes);

	CloseServiceHandle(svc);
	CloseServiceHandle(scm);

	return ok && ssp.dwCurrentState == SERVICE_RUNNING;
}

void KillGameWithMessage(const char* reason)
{
	MessageBoxA(NULL, reason, "Easy Anti-Cheat", MB_OK | MB_ICONERROR | MB_TOPMOST);

	// Cho log có thời gian flush nếu bạn muốn
	Sleep(300);

	// Thoát game an toàn (không gắt như TerminateProcess)
	//ExitProcess(0);
	TerminateProcess(GetCurrentProcess(), 0);
}

void WatchdogThread()
{
	Sleep(2000); // đợi EAC init

	while (true)
	{
		if (!IsEACServiceRunningA())
		{
			TerminateProcess(GetCurrentProcess(), 0); //tắt nhanh ko thông báo
			/*KillGameWithMessage(
				"Easy Anti-Cheat has been disabled or encountered a problem.\n"
				"Please restart the game."
			);*/
			return;
		}
		Sleep(800);
	}
}

DWORD WINAPI WatchdogThreadProc(LPVOID lpParam)
{
	WatchdogThread();
	return 0;
}
#endif

void IntegrityThread();

HRESULT GetDirectXVersionViaDxDiag( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, TCHAR* pcDirectXVersionLetter );

void zexit(int returnCode)
{
	// °ÔÀÓ°¡µå´Â Á¦´EÎ deleteµÇ¾ûÚß ¿À·ù¹ß»ý½Ã ÀÚÃ¼ ·Î±×¸¦ ¿Ã¹Ù¸£°Ô ³²±E¼EÀÖ´Ù.
	// ±×³É exit()ÇØµµ ZGameGuard¸¦ ½Ì±ÛÅÏÀ¸·Î ¸¸µé¾ú±E¶§¹®¿¡ ¼Ò¸EÚ¿¡¼­ °ÔÀÓ°¡µå°¡ deleteµÇÁö¸¸ ¾ûÞ°¼­ÀÎÁE±×¶§ Å©·¡½Ã°¡ ÀÏ¾ûÏ­´Ù.
	// exit()ÇÏ±EÀE¡ °ÔÀÓ°¡µå¸¦ ¼öµ¿À¸·Î ÇØÁ¦ÇÏ¸E±×·± ¹®Á¦°¡ ÀÏ¾ûÏªÁE¾Ê´Â´Ù.
	// ÇØÅ· °ËÃEµûÜÇ ÀÌÀ¯·Î Å¬¶óÀÌ¾ðÆ® Á¾·á½Ã exitÇÏÁö¸»°Ezexit¸¦ ¾²ÀÚ.
#ifdef _GAMEGUARD
	GetZGameguard().Release();
#endif
	exit(returnCode);
}

void CrcFailExitApp() {
	// Disabled for x64 compatibility - CRC values differ due to struct size changes
	return;
}

void _ZChangeGameState(int nIndex)
{
	GunzState state = GunzState(nIndex);

	if (ZApplication::GetGameInterface())
	{
		ZApplication::GetGameInterface()->SetState(state);
	}
}
#ifdef _LOTTIE 1
#include <rlottie.h>
#endif
#ifdef _ANTIHACK
cDetour* GetRawInputDataDetour = nullptr;
void AntiSB() {
#define USER32 XorStr<0x49,11,0x0AD073C2>("\x3C\x39\x2E\x3E\x7E\x7C\x61\x34\x3D\x3E"+0x0AD073C2).s
#define GETRAWINPUTDATA XorStr<0x82,16,0xF9AAEBB0>("\xC5\xE6\xF0\xD7\xE7\xF0\xC1\xE7\xFA\xFE\xF8\xC9\xEF\xFB\xF1"+0xF9AAEBB0).s
	void* address = (void*)GetProcAddress(LoadLibraryA(USER32), GETRAWINPUTDATA);
	GetRawInputDataDetour = new cDetour(address, GetRawInputDataHook);
}

//Custom: Clear files...
__forceinline void ClearFiles() {
	DeleteFile("lastchar.dat");
	DeleteFile("profile.txt");
	DeleteFile("patch.xml");
}
#endif

#ifdef _SERIALCUSTOM
__forceinline void ClearPatch() 
{
	DeleteFile("update");
}
#endif

struct CUSTOMVERTEX {
	FLOAT	x, y, z, rhw;
	FLOAT	tu, tv;
};
//#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_TEX1)
//#define NUM_BAR_BITMAP	1
//#define IL_TEXT_BUFFER_SIZE 512
//#define ADJUST_SIZE				0.5f
//#define ADJUST_SIZE2			0.0f
//#define FADEIN_NUM_DRAWING		120
//#define FADEOUT_NUM_DRAWING		120
//#define BLENDING_NUM_DRAWING	240
//void ZInitialLoading::DrawIntro(const char* szFileName, int nTimeMS)
//{
//	MBitmapR2 pIntroBmp;
//	// Sử dụng đúng cấu trúc Create(Tên, Device, Đường dẫn)
//	if (!pIntroBmp.Create("IntroTemp", RGetDevice(), szFileName))
//	{
//		mlog(">>> INTRO: Khong load duoc file %s - Check duong dan ngay!\n", szFileName);
//		return;
//	}
//
//	unsigned long nStartTime = timeGetTime();
//	float screenW = (float)RGetScreenWidth();
//	float screenH = (float)RGetScreenHeight();
//
//	// Lấy kích thước ảnh gốc
//	float imgW = (float)pIntroBmp.GetWidth();
//	float imgH = (float)pIntroBmp.GetHeight();
//
//	while (true)
//	{
//		unsigned long nNow = timeGetTime();
//		if (nNow - nStartTime > (unsigned long)nTimeMS) break;
//
//		if (RGetDevice()->BeginScene() == D3D_OK)
//		{
//			// Xóa nền đen
//			RGetDevice()->Clear(0, NULL, D3DCLEAR_TARGET, 0xFF000000, 1.0f, 0);
//
//			// --- THIẾT LẬP RENDER STATE GIỐNG HÀM DRAW GỐC---
//			RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
//			RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
//			RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
//
//			// Thiết lập để lấy Alpha từ Texture Factor (Dùng để Fade In/Out)
//			RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
//			RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
//			RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
//			RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
//
//			// Tính toán Alpha (Fade in 500ms, Fade out 500ms)
//			int nElapsed = nNow - nStartTime;
//			float fAlpha = 1.0f;
//			if (nElapsed < 500) fAlpha = (float)nElapsed / 500.0f;
//			else if (nElapsed > nTimeMS - 500) fAlpha = (float)(nTimeMS - nElapsed) / 500.0f;
//
//			// Gán Alpha vào Texture Factor (Cú pháp chuẩn)
//			DWORD dwAlpha = (DWORD)((BYTE)(0xFF * fAlpha)) << 24;
//			RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, dwAlpha);
//
//			// Vẽ Quad bằng Vertex có sẵn trong code
//			CUSTOMVERTEX Sprite[4] = {
//				{ 0 - ADJUST_SIZE      , 0 - ADJUST_SIZE       , 0, 1.0f, 0, 0 },
//				{ screenW - ADJUST_SIZE2, 0 - ADJUST_SIZE       , 0, 1.0f, 1, 0 },
//				{ screenW - ADJUST_SIZE2, screenH - ADJUST_SIZE2, 0, 1.0f, 1, 1 },
//				{ 0 - ADJUST_SIZE      , screenH - ADJUST_SIZE2, 0, 1.0f, 0, 1 },
//			};
//
//			RGetDevice()->SetFVF(D3DFVF_CUSTOMVERTEX);
//			RGetDevice()->SetTexture(0, pIntroBmp.m_pTexture->GetTexture());
//			RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Sprite, sizeof(CUSTOMVERTEX));
//
//			RGetDevice()->EndScene();
//			RGetDevice()->Present(NULL, NULL, NULL, NULL);
//		}
//
//		// Chống treo app
//		MSG msg;
//		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
//			TranslateMessage(&msg); DispatchMessage(&msg);
//		}
//		Sleep(1);
//	}
//	pIntroBmp.Destroy();
//}
// Khhai báo pointer IBasicAudio toàn cục (đặt cùng chỗ với g_pDSGraph, g_pDSControl...)
IBasicAudio* g_pDSBasicAudio = NULL;
IVMRWindowlessControl* g_pDSVMRControl = NULL;

bool PlayIntroVideoAVI(HWND hWnd, const char* szZipFileName)
{
	// 1. Mở file AVI từ MRS
	MZFile mfile;
	if (!mfile.Open(szZipFileName, ZApplication::GetFileSystem()))
	{
		mlog("Intro Video: Failed to open %s from MRS\n", szZipFileName);
		return false;
	}

	int nSize = mfile.GetLength();
	if (nSize <= 0) return false;

	char* pBuffer = new char[nSize];
	mfile.Read(pBuffer, nSize);
	mfile.Close();

	// 2. Tạo file Temp ngoài ổ cứng
	char szTempPathA[MAX_PATH];
	char szTempFileA[MAX_PATH];
	GetTempPathA(MAX_PATH, szTempPathA);
	GetTempFileNameA(szTempPathA, "gzv", 0, szTempFileA);

	FILE* fp = fopen(szTempFileA, "wb");
	if (fp)
	{
		fwrite(pBuffer, 1, nSize, fp);
		fclose(fp);
	}
	delete[] pBuffer;

	MultiByteToWideChar(CP_ACP, 0, szTempFileA, -1, g_szTempVideoPath, MAX_PATH);

	// 3. Khởi tạo DirectShow Graph Builder
	CoInitialize(NULL);
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void**)&g_pDSGraph);
	if (FAILED(hr)) return false;

	// =========================================================================
	// 🔥 TẠO VÀ THÊM VMR-7/9 FILTER VÀO GRAPH ĐỂ PHÁT WINDOWLESS (SỬA LỖI ĐEN FULLSCREEN) 🔥
	// =========================================================================
	IBaseFilter* pVMR = NULL;
	hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&pVMR);

	if (SUCCEEDED(hr))
	{
		g_pDSGraph->AddFilter(pVMR, L"Video Mixing Renderer");

		// Chuyển VMR sang chế độ Windowless (Không tạo cửa sổ đè)
		IVMRFilterConfig* pConfig = NULL;
		pVMR->QueryInterface(IID_IVMRFilterConfig, (void**)&pConfig);
		if (pConfig)
		{
			pConfig->SetRenderingMode(VMRMode_Windowless);
			pConfig->Release();
		}

		// Lấy VMR Control để gắn Cửa sổ Game
		pVMR->QueryInterface(IID_IVMRWindowlessControl, (void**)&g_pDSVMRControl);
		if (g_pDSVMRControl)
		{
			g_pDSVMRControl->SetVideoClippingWindow(hWnd);

			// Ép tỷ lệ hiển thị vừa khít cửa sổ Game
			RECT rc;
			GetClientRect(hWnd, &rc);
			g_pDSVMRControl->SetVideoPosition(NULL, &rc);
		}
		pVMR->Release();
	}
	// =========================================================================

	g_pDSGraph->QueryInterface(IID_IMediaControl, (void**)&g_pDSControl);
	g_pDSGraph->QueryInterface(IID_IMediaEvent, (void**)&g_pDSEvent);

	// 4. Render file AVI
	hr = g_pDSGraph->RenderFile(g_szTempVideoPath, NULL);
	if (FAILED(hr))
	{
		mlog("DirectShow: RenderFile failed\n");
		return false;
	}

	// 5. Tắt / Chỉnh Âm lượng
	if (SUCCEEDED(g_pDSGraph->QueryInterface(IID_IBasicAudio, (void**)&g_pDSBasicAudio)))
	{
		if (g_pDSBasicAudio)
		{
			g_pDSBasicAudio->put_Volume(-10000); // 50% Volume (hoặc -10000 để Mute)
		}
	}

	g_pDSControl->Run();
	return true;
}

// Function dừng Video & Xóa sạch File Temp
void StopIntroVideoAVI()
{
	if (g_pDSControl) g_pDSControl->Stop();

	if (g_pDSVMRControl) { g_pDSVMRControl->Release(); g_pDSVMRControl = NULL; }
	if (g_pDSBasicAudio) { g_pDSBasicAudio->Release(); g_pDSBasicAudio = NULL; }
	if (g_pDSEvent) { g_pDSEvent->Release(); g_pDSEvent = NULL; }
	if (g_pDSControl) { g_pDSControl->Release(); g_pDSControl = NULL; }
	if (g_pDSGraph) { g_pDSGraph->Release(); g_pDSGraph = NULL; }
	CoUninitialize();

	if (wcslen(g_szTempVideoPath) > 0)
	{
		DeleteFileW(g_szTempVideoPath);
		g_szTempVideoPath[0] = L'\0';
	}
}

// Thread chạy ngầm nạp Game Data
DWORD WINAPI Thread_AppOnCreate(LPVOID lpParam)
{
	ZLoadingProgress appLoading("application");
	g_bAsyncLoadSuccess = g_App.OnCreate(&appLoading);
	g_bAsyncLoadDone = true; // Báo hiệu đã nạp xong!
	return 0;
}

RRESULT OnCreate(void* pParam)
{
#ifdef _DEBUG
	g_App.PreCheckArguments();
#endif

	if (g_hWnd)
	{
		// 1. Đăng ký danh sách Layout hợp pháp với Windows Subsystem (Hỗ trợ IME Tiếng Việt/Trung/Thái)
		typedef HKL(WINAPI* PFN_LoadKbdLayout)(LPCSTR, UINT);
		HMODULE hUser32 = GetModuleHandleA("user32.dll");
		if (hUser32) {
			PFN_LoadKbdLayout pLoadLayout = (PFN_LoadKbdLayout)GetProcAddress(hUser32, "LoadKeyboardLayoutA");
			//if (pLoadLayout) {
			//	pLoadLayout("0000042a", 0x00000001); // Tiếng Việt
			//	pLoadLayout("00000804", 0x00000001); // Tiếng Trung
			//	pLoadLayout("0000041e", 0x00000001); // Tiếng Thái
			//}
		}

		// 2. Tạo và liên kết Context
		HIMC hIMC = ImmGetContext(g_hWnd);
		if (!hIMC) {
			hIMC = ImmCreateContext();
			if (hIMC) {
				ImmAssociateContext(g_hWnd, hIMC);
			}
		}
		else {
			ImmAssociateContext(g_hWnd, hIMC);
		}

		// 3. Mở van bộ gõ
		if (hIMC) {
			ImmSetOpenStatus(hIMC, TRUE);
			ImmReleaseContext(g_hWnd, hIMC);
		}

		// 4. Đồng bộ thông điệp IME
		SendMessage(g_hWnd, WM_IME_SETCONTEXT, TRUE, ISC_SHOWUICOMPOSITIONWINDOW);
	}

	// Custom: Voice Chat Draw
	GetRGMain().OnCreateDevice();

	// Thiệt lập Graphics
	RAdvancedGraphics::bDepthBuffering = Z_VIDEO_STENCILBUFFER;
	RAdvancedGraphics::nMultiSampling = Z_VIDEO_MULTISAMPLING;
	if (RAdvancedGraphics::bDepthBuffering || RAdvancedGraphics::nMultiSampling > 0)
	{
		RMODEPARAMS ModeParams =
		{
			RGetScreenWidth(), RGetScreenHeight(), RGetScreenType(), RGetPixelFormat()
		};
		RResetDevice(&ModeParams);
	}

#ifdef _DYNAMIC
	if (Z_ETC_DYNAMIC == true)
	{
		RMesh::SetPartsMeshLoadingSkip(1);
	}
#endif

#ifdef _LOTTIE 1
	// Custom: load Lottie
	if (!rlottie::Animation::SetDX9Ptr(RGetDevice(), g_isDirect3D9ExEnabled ? rlottie::D3D9Ex : rlottie::D3D9))
	{
		mlog("Invalid Lottie DirectX Ptr");
		return R_ERROR_LOADING;
	}
	rlottie::Animation::StartElapsedTime();
#endif

	string strFileLenzFlare("System/LenzFlare.xml");
#ifndef _DEBUG
	strFileLenzFlare += "";
#endif
	RCreateLenzFlare(strFileLenzFlare.c_str());
	RGetLenzFlare()->Initialize();

	mlog("main : RGetLenzFlare()->Initialize() \n");

	RBspObject::CreateShadeMap("sfx/water_splash.bmp");

#ifdef _SPRINTSCREEN
	sprintf_safe(cstrReleaseDate, "Last Patch: " __DATE__ "");
#endif

	g_DInput.Create(g_hWnd, FALSE, FALSE);
	g_pInput = new ZInput(&g_DInput);

	RSetGammaRamp(Z_VIDEO_GAMMA_VALUE);
	RSetRenderFlags(RRENDER_CLEAR_BACKBUFFER);

	// Khởi tạo Loading Manager
	ZGetInitialLoading()->Initialize(1, 0, 0, RGetScreenWidth(), RGetScreenHeight(), 0, 0, 1024, 768);

#ifdef _ANTIHACK
#ifdef _THEMIDAGUNZ
	VM_START
#endif
		HANDLE hThreadAC[4];
	hThreadAC[0] = CreateThread(nullptr, 0, SearchandDestroy, nullptr, 0, nullptr);
	hThreadAC[1] = CreateThread(nullptr, 0, StreakGuard, nullptr, 0, nullptr);
	hThreadAC[2] = CreateThread(nullptr, 0, Scanner, nullptr, 0, nullptr);
	hThreadAC[3] = CreateThread(nullptr, 0, ZGuard::GuardInit, nullptr, 0, nullptr);

	AntiNoob = hThreadAC[3];

	for (int i = 0; i < 4; i++)
	{
		if (!IS_VALID_HANDLE(hThreadAC[i]))
		{
			MLog("[Anti-Cheat]: Fail Create Thread %d\n", i);
			return R_ERROR_LOADING;
		}
	}
#ifdef _THEMIDAGUNZ
	VM_END
#endif
#endif
		mlog("InitialLoading success.\n");

	// Load Font TTF
	struct _finddata_t c_file;
	intptr_t hFile;
	char szFileName[256];
#define FONT_DIR	"Font/"
#define FONT_EXT	"ttf"
	if ((hFile = _findfirst(FONT_DIR "*." FONT_EXT, &c_file)) != -1L) {
		do {
			strcpy(szFileName, FONT_DIR);
			strcat(szFileName, c_file.name);
			AddFontResource(szFileName);
		} while (_findnext(hFile, &c_file) == 0);
		_findclose(hFile);
	}

	g_pDefFont = new MFontR2;
	if (!g_pDefFont->Create("Default", Z_LOCALE_DEFAULT_FONT, DEFAULT_FONT_HEIGHT, 1.0f))
	{
		mlog("Fail to Create default font : MFontR2 / main.cpp.. onCreate\n");
		g_pDefFont->Destroy();
		SAFE_DELETE(g_pDefFont);
		g_pDefFont = NULL;
	}

	g_pDC = new MDrawContextR2(RGetDevice());

	// Khởi tạo Mint UI Engine
	g_Mint.Initialize(800, 600, g_pDC, g_pDefFont);
	Mint::GetInstance()->SetHWND(RealSpace2::g_hWnd);

	mlog("interface Initialize success\n");

	// =======================================================================
	// 🚀 FAST LAUNCH CHUẨN STEAM: INTRO JPG (FULL STATE MACHINE) -> VIDEO AVI
	// =======================================================================

	// -----------------------------------------------------------------------
	// PHẦN 1: CHẠY HẾT STATE MACHINE CỦA ẢNH JPG (INTRO 1 -> FADE -> INTRO 2 -> FADE)
	// -----------------------------------------------------------------------
	if (ZGetInitialLoading() && ZGetInitialLoading()->IsUseEnable())
	{
		ZGetInitialLoading()->InitIntros();

		// Vòng lặp Render chờ State Machine chạy đến STAGE_BACKGROUND (Global Enum)
		while (ZGetInitialLoading()->GetStage() != STAGE_BACKGROUND)
		{
			// Xử lý sự kiện Windows
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			// Nút Skip Intro JPG (ESC, Space, Enter, Click chuột) -> Nhảy thẳng sang Video
			//if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) ||
			//	(GetAsyncKeyState(VK_SPACE) & 0x8000) ||
			//	(GetAsyncKeyState(VK_RETURN) & 0x8000) ||
			//	(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
			//{
			//	break;
			//}

			// Ép DirectX Render Frame ảnh Intro
			if (RBeginScene())
			{
				// Dùng MODE_DEFAULT (Global Enum) thay vì ZInitialLoading::MODE_DEFAULT
				ZGetInitialLoading()->Draw(MODE_DEFAULT, -1, true, false);

				REndScene();
			}

			Sleep(16); // Khóa nhẹ FPS ở ~60 FPS
		}

		// Giải phóng Texture JPG ngay sau khi chuỗi Intro JPG kết thúc
		ZGetInitialLoading()->Release();
	}

	// -----------------------------------------------------------------------
// PHẦN 2: CHẠY INTRO VIDEO AVI VÀ NẠP DATA NGẦM
// -----------------------------------------------------------------------
	bool bVideoPlaying = PlayIntroVideoAVI(g_hWnd, "Media/Intro.avi");

	// Khởi tạo Thread nạp Data ngầm
	g_bAsyncLoadDone = false;
	g_bAsyncLoadSuccess = false;

	HANDLE hLoadThread = CreateThread(NULL, 0, Thread_AppOnCreate, NULL, 0, NULL);

	long evCode = 0;
	while (!g_bAsyncLoadDone)
	{
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Skip Video (nếu anh mở lại tính năng Skip)
		// if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) || ...) break;

		if (g_pDSEvent && SUCCEEDED(g_pDSEvent->WaitForCompletion(10, &evCode)))
		{
			break;
		}

		Sleep(10);
	}

	if (hLoadThread != NULL)
	{
		WaitForSingleObject(hLoadThread, INFINITE);
		CloseHandle(hLoadThread);
	}

	// =======================================================================
	// 🚀 FIX KHỰNG WINDOW BAR & MÀN HÌNH ĐEN FULLSCREEN
	// =======================================================================
	if (bVideoPlaying)
	{
		StopIntroVideoAVI();
	}

	// 1. Khôi phục Focus & Ép cửa sổ Borderless/Fullscreen nằm đè lên trên cùng lập tức (Không cho hiện Taskbar)
	if (g_hWnd)
	{
		// Đưa cửa sổ game lên Top, giữ nguyên vị trí/kích thước, không kích hoạt animation chuyển window
		SetWindowPos(g_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		SetFocus(g_hWnd);
		SetActiveWindow(g_hWnd);
	}

	// 2. Khôi phục lại DirectX Device State (Tránh bị đen thui ở Fullscreen)
	if (RGetDevice())
	{
		// Clear khung hình về màu đen để trả D3D Device về trạng thái sẵn sàng Render
		RGetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

		if (RBeginScene())
		{
			// Render 1 frame trống mồi để D3D SwapChain hoạt động bình thường
			REndScene();
			RFlip(); // Hoặc RGetDevice()->Present(NULL, NULL, NULL, NULL);
		}
	}
	// =======================================================================

	if (!g_bAsyncLoadSuccess)
	{
		return R_ERROR_LOADING;
	}

	// Config Sound & Controls
	ZGetSoundEngine()->SetEffectVolume(Z_AUDIO_EFFECT_VOLUME);
	ZGetSoundEngine()->SetMusicVolume(Z_AUDIO_BGM_VOLUME);
	g_bPlayMusicInBackground = Z_AUDIO_PLAY_IN_BACKGROUND;
	ZGetSoundEngine()->SetEffectMute(Z_AUDIO_EFFECT_MUTE);
	ZGetSoundEngine()->SetMusicMute(Z_AUDIO_BGM_MUTE);

	g_Mint.SetWorkspaceSize(g_ModeParams.nWidth, g_ModeParams.nHeight);
	g_Mint.GetMainFrame()->SetSize(g_ModeParams.nWidth, g_ModeParams.nHeight);
	ZGetOptionInterface()->Resize(g_ModeParams.nWidth, g_ModeParams.nHeight);

	for (int i = 0; i < ZACTION_COUNT; i++) {
		ZACTIONKEYDESCRIPTION& keyDesc = ZGetConfiguration()->GetKeyboard()->ActionKeys[i];
		g_pInput->RegisterActionKey(i, keyDesc.nVirtualKey);
		if (keyDesc.nVirtualKeyAlt != -1)
			g_pInput->RegisterActionKey(i, keyDesc.nVirtualKeyAlt);
	}

	// -----------------------------------------------------------------------
	// PHẦN 3: VÀO MÀN HÌNH CHÍNH (LOGIN / HIDEOUT)
	// -----------------------------------------------------------------------
	g_App.SetInitialState();

	ZGetFlashBangEffect()->SetDrawCopyScreen(true);
	mlog("main : OnCreate() done\n");

	// Nếu đang ở Windowed / Borderless (g_d3dpp.Windowed == TRUE)
	// Bỏ HWND_TOPMOST để trả lại Window Style bình thường cho Win32
	if (RealSpace2::g_d3dpp.Windowed)
	{
		SetWindowPos(g_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	}

	SetFocus(g_hWnd);

	return R_OK;
}


bool CheckDll(char* fileName, BYTE* SHA1_Value)
{
	BYTE digest[20];
	BYTE Key[GUNZ_HMAC_KEY_LENGTH];

	memset(Key, 0, 20);
	memcpy(Key, GUNZ_HMAC_KEY, strlen(GUNZ_HMAC_KEY));

	CHMAC_SHA1 HMAC_SHA1 ;
	HMAC_SHA1.HMAC_SHA1_file(fileName, Key, GUNZ_HMAC_KEY_LENGTH, digest) ;

	if(memcmp(digest, SHA1_Value, 20) ==0)
	{
		return true;
	}

	return false;
}



RRESULT OnDestroy(void *pParam)
{
	mlog("Destroy gunz\n");

	g_App.OnDestroy();

	SAFE_DELETE(g_pDefFont);

	g_Mint.Finalize();

	mlog("interface finalize.\n");

	SAFE_DELETE(g_pInput);
	g_DInput.Destroy();

	mlog("game input destroy.\n");

	RGetShaderMgr()->Release();

//	g_App.OnDestroy();

	// mlog("main : g_App.OnDestroy()\n");

	ZGetConfiguration()->Destroy();

	mlog("game configuration destroy.\n");

	if (g_pDC != NULL)
	{
		delete g_pDC;
		g_pDC = NULL;
	}

	struct _finddata_t c_file;
	intptr_t hFile;
	char szFileName[256];
#define FONT_DIR	"Font/"
#define FONT_EXT	"ttf"
	if( (hFile = _findfirst(FONT_DIR "*." FONT_EXT, &c_file )) != -1L ){
		do{
			strcpy(szFileName, FONT_DIR);
			strcat(szFileName, c_file.name);
			RemoveFontResource(szFileName);
		}while( _findnext( hFile, &c_file ) == 0 );
		_findclose(hFile);
	}

	MFontManager::Destroy();
	MBitmapManager::Destroy();
	MBitmapManager::DestroyAniBitmap();

	mlog("Bitmap manager destroy Animation bitmap.\n");

	/*
	for(list<HANDLE>::iterator i=g_FontMemHandles.begin(); i!=g_FontMemHandles.end(); i++){
		RemoveFontMemResourceEx(*i);
	}
	*/

	//ReleaseMemPool(RealSoundEffectPlay);
	//UninitMemPool(RealSoundEffectPlay);

	//ReleaseMemPool(RealSoundEffect);
	//UninitMemPool(RealSoundEffect);

	//ReleaseMemPool(RealSoundEffectFx);
	//UninitMemPool(RealSoundEffectFx);

	//mlog("main : UninitMemPool(RealSoundEffectFx)\n");

	// ¸Þ¸ð¸®Ç® ÇE¦
	ZBasicInfoItem::Release(); // ÇÒ´çµÇ¾EÀÖ´Â ¸Þ¸ð¸® ÇØÁ¦
//	ZHPInfoItem::Release();

	ZGetStencilLight()->Destroy();
	LightSource::Release();

	RBspObject::DestroyShadeMap();
	RDestroyLenzFlare();
	RAnimationFileMgr::GetInstance()->Destroy();
	
	ZStringResManager::ResetInstance();

	DestroyRGMain();

	mlog("destroy gunz finish.\n");

	return R_OK;
}

RRESULT OnUpdate(void* pParam)
{
	__BP(100, "main::OnUpdate");

	g_pInput->Update();

	g_App.OnUpdate();

#ifdef _EAC
	/*if (g_hPlatform) {
		EOS_Platform_Tick(g_hPlatform);
	}*/

	//if (g_hPlatform == nullptr) return;

	//static DWORD dwLastTick = 0;
	//DWORD dwCurrTime = timeGetTime();

	//// Chỉ thực thi mỗi 33ms (tương đương khoảng 30 FPS cho riêng EAC)
	//if (dwCurrTime - dwLastTick >= 33) {
	//	EOS_Platform_Tick(g_hPlatform);
	//	dwLastTick = dwCurrTime;
	//}
#endif

	const DWORD dwCurrUpdateTime = timeGetTime();

	__EP(100);

	return R_OK;
}

RRESULT OnRender(void *pParam)
{
	__BP(101, "main::OnRender");

	//Clip the cursor to the main screen
	if (GetFocus() == g_hWnd && RGetScreenType() == 0)
	{
		RECT rect;
		GetWindowRect(g_hWnd, &rect);
		ClipCursor(&rect);
	}
	//otherwise release the cursor
	else
	{
		ClipCursor(NULL);
	}
	//RUpdateMouseClip(); //điều khiển chuột mới bên Realspace2

	if( !RIsActive() && RGetScreenType() ==0 )
	{
		__EP(101);
		return R_NOTREADY;
	}


	g_App.OnDraw();

	// Custom: Fix bandicam support
	if (ZIsActionKeyPressed(ZACTION_SCREENSHOT))
	{
		try {
			g_App.GetGameInterface()->SaveScreenShot();
		}
		catch (...) {
			mlog("Screenshot exception caught\n");
		}
	}
	if (ZIsActionKeyPressed(ZACTION_MOVING_PICTURE))
	{
		if (g_App.GetGameInterface() && g_App.GetGameInterface()->GetBandiCapturer())
			g_App.GetGameInterface()->GetBandiCapturer()->ToggleStart();
	}

	if (ZGetGameInterface()->GetBandiCapturer() != NULL)
		ZGetGameInterface()->GetBandiCapturer()->DrawCapture(g_pDC);


#ifdef _SMOOTHLOOP
	Sleep(10);
#endif
	static char __buffer[256];
	float x = 10.f / 800.f;
	int screenx = x * MGetWorkspaceWidth();
	if (RGetIsWidthScreen() || RGetIsLongScreen())
	{
		x = (x * 800 + 80) / 590.f;
		screenx = x * MGetWorkspaceWidth();
	}
	if (g_pDefFont && ZGetConfiguration()->GetEtc()->bShowFPS && !ZIsActionKeyPressed(ZACTION_SCORE))
	{
		sprintf(__buffer, "FPS: %3.0f", g_fFPS);

		// 1. Lấy độ rộng của chuỗi text "FPS: XXX" để biết nó dài bao nhiêu pixel
		int nTextWidth = g_pDefFont->m_Font.GetTextWidth(__buffer);

		// 2. Tính toán vị trí X để bám lề phải:
		// Lấy toàn bộ chiều rộng màn hình (WorkspaceWidth) trừ đi (độ rộng text + khoảng cách đệm)
		// Để đệm 10 pixel cho nó đừng dính sát sạt vào mép màn hình nhìn cho đẹp.
		int nPosX = (int)MGetWorkspaceWidth() - nTextWidth - 10;

		// 3. Vẽ lên màn hình (Y = 5 hoặc 10 cho nó xuống dưới thanh tiêu đề tí)
		g_pDefFont->m_Font.DrawText(nPosX, 5, __buffer);
	}
	__EP(101);

	return R_OK;
}

RRESULT OnInvalidate(void *pParam)
{
	MBitmapR2::m_dwStateBlock=NULL;

	g_App.OnInvalidate();
	
	return R_OK;
}

RRESULT OnRestore(void *pParam)
{
	for(int i=0; i<MBitmapManager::GetCount(); i++){
		MBitmapR2* pBitmap = (MBitmapR2*)MBitmapManager::Get(i);
		pBitmap->OnLostDevice();
	}

	g_App.OnRestore();

	return R_OK;
}

RRESULT OnActivate(void *pParam)
{
	if (ZGetGameInterface() && ZGetGameClient() && Z_ETC_BOOST)
		ZGetGameClient()->PriorityBoost(true);

	// Restore volumes when returning to game
	if (ZGetSoundEngine())
	{
		ZGetSoundEngine()->SetMusicVolume(Z_AUDIO_BGM_VOLUME);
		ZGetSoundEngine()->SetEffectVolume(Z_AUDIO_EFFECT_VOLUME);
	}

	return R_OK;
}

RRESULT OnDeActivate(void *pParam)
{

	if (ZGetGameInterface() && ZGetGameClient())
		ZGetGameClient()->PriorityBoost(false);

	// Clear all action key states to prevent stuck keys (e.g. Tab scoreboard after Alt+Tab)
	if (ZGetInput())
		ZGetInput()->OffActionKeys();

	// Mute when alt-tabbed
	if (ZGetSoundEngine())
	{
		ZGetSoundEngine()->SetEffectVolume(0);
		if (!Z_AUDIO_PLAY_IN_BACKGROUND)
			ZGetSoundEngine()->SetMusicVolume(0);
	}

	return R_OK;
}

RRESULT OnError(void *pParam)
{
	mlog("RealSpace::OnError(%d) \n", RGetLastError());

	switch (RGetLastError())
	{
	case RERROR_INVALID_DEVICE:
		{
			D3DADAPTER_IDENTIFIER9 *ai=RGetAdapterID();
			char szLog[512];
			ZTransMsg( szLog, MSG_DONOTSUPPORT_GPCARD, 1, ai->Description);

			int ret=MessageBox(NULL, szLog, ZMsg( MSG_WARNING), MB_YESNO);
			if(ret!=IDYES)
				return R_UNKNOWN;
		}
		break;
	case RERROR_CANNOT_CREATE_D3D:
		{
			ShowCursor(TRUE);

			char szLog[512];
			sprintf(szLog, ZMsg( MSG_DIRECTX_NOT_INSTALL));

			int ret=MessageBox(NULL, szLog, ZMsg( MSG_WARNING), MB_YESNO);
			if(ret==IDYES)
			{
				ShellExecute(g_hWnd, "open", ZMsg( MSG_DIRECTX_DOWNLOAD_URL), NULL, NULL, SW_SHOWNORMAL); 
			}
		}
		break;

	};

	return R_OK;
}

void SetModeParams()
{
	g_ModeParams.nScreenType = ZGetConfiguration()->GetVideo()->nScreenType;

	// SỬA TẠI ĐÂY:
	// Cho phép Type 1 (Windowed) và Type 2 (Borderless) dùng Resolution từ Config
	if (g_ModeParams.nScreenType != 0) // Nếu không phải Fullscreen (Type 0)
	{
		// Lấy Width/Height từ file XML (ZGetConfiguration)
		g_ModeParams.nWidth = ZGetConfiguration()->GetVideo()->nWidth;
		g_ModeParams.nHeight = ZGetConfiguration()->GetVideo()->nHeight;

		// Backup đề phòng trường hợp file XML để 0x0
		if (g_ModeParams.nWidth <= 0) g_ModeParams.nWidth = 800;
		if (g_ModeParams.nHeight <= 0) g_ModeParams.nHeight = 600;
	}
	else
	{
		// Chỉ Fullscreen mới lấy Width/Height từ Config (hoặc xử lý riêng tùy source)
		g_ModeParams.nWidth = ZGetConfiguration()->GetVideo()->nWidth;
		g_ModeParams.nHeight = ZGetConfiguration()->GetVideo()->nHeight;
	}

	// Phần ColorBits giữ nguyên
	ZGetConfiguration()->GetVideo()->nColorBits == 32 ?
		g_ModeParams.PixelFormat = D3DFMT_X8R8G8B8 : g_ModeParams.PixelFormat = D3DFMT_R5G6B5;
}

void ResetAppResource()
{
	MLog("Language changed → restarting...\n");

	// Lưu config (đã chứa ngôn ngữ mới)
	ZGetConfiguration()->Destroy();
	ZGetConfiguration()->Load();
	ZGetConfiguration()->Load_StringResDependent();
	ZGetConfiguration()->SaveToFile(FILENAME_CONFIG, Z_LOCALE_XML_HEADER);

	// Restart game EXE, bỏ launcher
	ZApplication::GetInstance()->RestartGame();
}

int FindStringPos(char* str,char* word)
{
	if(!str || str[0]==0)	return -1;
	if(!word || word[0]==0)	return -1;

	int str_len = (int)strlen(str);
	int word_len = (int)strlen(word);

	char c;
	bool bCheck = false;

	for(int i=0;i<str_len;i++) {
		c = str[i];
		if(c == word[0]) {

			bCheck = true;

			for(int j=1;j<word_len;j++) {
				if(str[i+j]!=word[j]) {
					bCheck = false;
					break;
				}
			}

			if(bCheck) {
				return i;
			}
		}
	}
	return -1;
}

bool FindCrashFunc(char* pName)
{
//	Function Name
//	File Name 
	if(!pName) return false;

	FILE *fp;
	fp = fopen( "mlog.txt", "r" );
	if(fp==NULL)  return false;

	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	char* pBuffer = new char [size];

	fread(pBuffer,1,size,fp);

	fclose(fp);

	// ¿E® ½ûÙº¿¡¼­ Ã£´Â´Ù.
	int posSource = FindStringPos(pBuffer,"ublish");
	if(posSource==-1) return false;

	int posA = FindStringPos(pBuffer+posSource,"Function Name");
//	int posB = FindStringPos(pBuffer,"File Name");	
	// filename ÀÌ ¾ø´Â °æ¿Eµ ÀÖ¾ûØ­ ÀÌ·¸°Ô ¹Ù²å´Ù
	int posB = posA + FindStringPos(pBuffer+posSource+posA,"\n");

	if(posA==-1) return false;
	if(posB==-1) return false;

	posA += 16;

//	int memsize = posB-posA-6;
	int memsize = posB-posA;
	memcpy(pName,&pBuffer[posA+posSource],memsize);

	pName[memsize] = 0;

	delete [] pBuffer;

	for(int i=0;i<memsize;i++) {
		if(pName[i]==':') {
			pName[i] = '-';
		}
	}

	return true;
}

void HandleExceptionLog()
{
	/*#define ERROR_REPORT_FOLDER	"ReportError"

	extern char* logfilename;	// Instance on MDebug.cpp

	// ERROR_REPORT_FOLDER Á¸ÀçÇÏ´ÂÁE°Ë»çÇÏ°E ¾øÀ¸¸E»ý¼º
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile(ERROR_REPORT_FOLDER, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		if (!CreateDirectory("ReportError", NULL)) {
			MessageBox(g_hWnd, "ReportError Æú´õ¸¦ »ý¼ºÇÒ ¼E¾ø½À´Ï´Ù.", APPLICATION_NAME , MB_ICONERROR|MB_OK);
			return;
		}
	} else 	{
		FindClose(hFind);
	}*/


/* 2007³E2¿E13ÀÏ BAReport ´õÀÌ»E»ç¿E¸øÇÏ°Ô ¸·À½


	// mlog.txt ¸¦ ERROR_REPORT_FOLDER ·Î º¹»E

	//acesaga_0928_911_moanus_rslog.txt
	//USAGE_EX) BAReport app=acesaga;addr=moon.maiet.net;port=21;id=ftp;passwd=ftp@;gid=10;user=moanus;localfile=rslog.txt;remotefile=remote_rslog.txt;

//	if(ZGetCharacterManager()) {
//		ZGetCharacterManager()->OutputDebugString_CharacterState();
//	}


	ZGameClient* pClient = (ZGameClient*)ZGameClient::GetInstance();

	char* pszCharName = NULL;
	MUID uidChar;
	MMatchObjCache* pObj;
	char szPlayer[128];

	if( pClient ) {

		uidChar = pClient->GetPlayerUID();
		pObj = pClient->FindObjCache(uidChar);
		if (pObj)
			pszCharName = pObj->GetName();

		wsprintf(szPlayer, "%s(%d%d)", pszCharName?pszCharName:"Unknown", uidChar.High, uidChar.Low);
	}
	else { 
		wsprintf(szPlayer, "Unknown(-1.-1)");
	}


//	if (pClient) {

		time_t currtime;
		time(&currtime);
		struct tm* pTM = localtime(&currtime);

		char cFuncName[1024];

		if(FindCrashFunc(cFuncName)==false) {
			strcpy(cFuncName,"Unknown Error");
		}

		char szFileName[_MAX_DIR], szDumpFileName[_MAX_DIR];
		wsprintf(szFileName, "%s_%s_%.2d%.2d_%.2d%.2d_%s_%s", cFuncName,
				APPLICATION_NAME, pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, szPlayer, "mlog.txt");
		wsprintf(szDumpFileName, "%s.dmp", szFileName);

		char szFullFileName[_MAX_DIR], szDumpFullFileName[_MAX_DIR];
		wsprintf(szFullFileName, "%s/%s", ERROR_REPORT_FOLDER, szFileName);
		wsprintf(szDumpFullFileName, "%s/%s", ERROR_REPORT_FOLDER, szDumpFileName);

		if (CopyFile("mlog.txt", szFullFileName, TRUE))
		{
			CopyFile("Gunz.dmp", szDumpFullFileName, TRUE);

			// BAReport ½ÇÇE
			char szCmd[4048];
			char szRemoteFileName[_MAX_DIR], szRemoteDumpFileName[_MAX_DIR];
			wsprintf(szRemoteFileName, "%s/%s/%s", ZGetConfiguration()->GetBAReportDir(), "gunzlog", szFileName);
			wsprintf(szRemoteDumpFileName, "%s/%s/%s", ZGetConfiguration()->GetBAReportDir(), "gunzlog", szDumpFileName);

			wsprintf(szCmd, "BAReport app=%s;addr=%s;port=21;id=ftp;passwd=ftp@;user=%s;localfile=%s,%s;remotefile=%s,%s", 
				APPLICATION_NAME, ZGetConfiguration()->GetBAReportAddr(), szPlayer, szFullFileName, szDumpFullFileName, szRemoteFileName, szRemoteDumpFileName);

			WinExec(szCmd, SW_SHOW);

			FILE *file = fopen("bareportpara.txt","w+");
			fprintf(file,szCmd);
			fclose(file);
		}
//	}
*/
}

void ZGuard_Kick(const char* szReason) {
	mlog("Easy Anti-Cheat: Kicking player. Reason: %s\n", szReason);

	// Thêm MB_SERVICE_NOTIFICATION để nó đè lên mọi thứ
	MessageBox(NULL,
		"The system detects the use of HARDWARE MACRO.\nThe game will automatically exit to ensure fairness!.",
		"Easy Anti-Cheat",
		MB_OK | MB_ICONSTOP | MB_SERVICE_NOTIFICATION);

	TerminateProcess(GetCurrentProcess(), 0);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	if (g_QPCFreq.QuadPart == 0) QueryPerformanceFrequency(&g_QPCFreq);

	switch (message)
	{
#ifdef LOCALE_JAPAN
		case WM_COPYDATA:
			{
				ZBaseAuthInfo* pAuth = ZGetLocale()->GetAuthInfo();
				if( ((ZGameOnJPAuthInfo*)pAuth)->NewLogin(wParam, lParam) )
				{
					MessageBox(g_hWnd, "Same id accessing from a different PC", NULL, MB_OK);
					zexit(-1);
				}
			}
			break;
#endif
			// ======================== BÀN PHÍM ========================
		//case WM_LBUTTONDOWN:
		//{
		//	// 1. Lưu thời điểm Click chuột trái để check combo với phím W sau đó
		//	QueryPerformanceCounter(&g_liLastLButtonClick);

		//	// 2. Lưu thời điểm để tính Duration (nhấn-thả) của chuột trái
		//	QueryPerformanceCounter(&g_MouseTimerQPC[0]);
		//	break;
		//}

		//case WM_KEYDOWN:
		//{
		//	// ---------------------------------------------------------------------
		//	// [A] CHỨC NĂNG ANTI-MACRO MOVEMENT / COMBO (WASD & SINGLE KEY DOWN)
		//	// ---------------------------------------------------------------------
		//	LARGE_INTEGER liNow;
		//	QueryPerformanceCounter(&liNow);

		//	bool isWASD = (wParam == 0x57 || wParam == 0x41 || wParam == 0x53 || wParam == 0x44); // W, A, S, D
		//	bool isLastWASD = (g_lastWParam == 0x57 || g_lastWParam == 0x41 || g_lastWParam == 0x53 || g_lastWParam == 0x44);

		//	if (isWASD && g_liLastKeyTime.QuadPart != 0)
		//	{
		//		double dwInterval = (double)(liNow.QuadPart - g_liLastKeyTime.QuadPart) * 1000.0 / g_QPCFreq.QuadPart;

		//		bool isMacro = false;
		//		// 1. Bắt nhấp đúp siêu tốc cùng 1 phím WASD (W W, S S, A A, D D)
		//		if (wParam == g_lastWParam && dwInterval > 0.0 && dwInterval < 3.0) {
		//			isMacro = true;
		//		}
		//		// 2. Bắt phím khác WASD chuyển sang WASD siêu tốc (Space -> W, Z -> W,...)
		//		else if (!isLastWASD && dwInterval > 0.0 && dwInterval < 3.0) {
		//			isMacro = true;
		//		}

		//		if (isMacro)
		//		{
		//			if (dwInterval < 0.5) {
		//				g_nMouseWComboChain += 4; // Siêu tốc (< 0.5ms) phạt nặng
		//			}
		//			else {
		//				g_nMouseWComboChain += 1;
		//			}

		//			mlog("ZGuard: Macro Detect! [0x%X -> 0x%X] Interval: %.2f ms\n", g_lastWParam, wParam, dwInterval);

		//			if (g_nMouseWComboChain >= 12) {
		//				ZGuard_Kick("Movement/Combo Macro Detected");
		//			}
		//		}
		//		else if (dwInterval > 3.0) {
		//			g_nMouseWComboChain = 0; // Bấm tự nhiên (> 3ms) -> Reset chuỗi phạt
		//		}
		//	}

		//	// Cập nhật mốc thời gian phím vừa nhấn
		//	g_lastWParam = wParam;
		//	g_liLastKeyTime = liNow;

		//	// Lưu mốc thời gian DOWN cho kiểm tra Hold-Duration phím đơn (Single Key)
		//	if (g_KeyTimerQPC.find(wParam) == g_KeyTimerQPC.end()) {
		//		QueryPerformanceCounter(&g_KeyTimerQPC[wParam]);
		//	}

		//	// ---------------------------------------------------------------------
		//	// [B] CHỨC NĂNG HOTKEY CHAT PHÒNG CHỜ (STAGE CHATTING)
		//	// ---------------------------------------------------------------------
		//	ZGameInterface* pGI = ZApplication::GetGameInterface();
		//	if (pGI != NULL && pGI->GetState() == GUNZ_STAGE)
		//	{
		//		ZIDLResource* pRes = pGI->GetIDLResource();
		//		MWidget* pFrame = pRes->FindWidget("StageChattingFrame");
		//		MEdit* pInput = (MEdit*)pRes->FindWidget("StageChattingInput");

		//		// Chỉ xử lý phím tắt khi khung chat tồn tại và chưa được Focus
		//		if (pFrame != NULL && pInput != NULL && pInput->IsFocus() == false)
		//		{
		//			if (wParam == VK_RETURN) // Phím ENTER -> Mở khung chat & Focus
		//			{
		//				pFrame->Show(true);
		//				pInput->SetFocus();
		//				return 0; // Ngắt Message không cho truyền tiếp xuống Game UI
		//			}
		//			if (wParam == 'Z' || wParam == 'z') // Phím Z -> Bật/Tắt ẩn hiện chat
		//			{
		//				pFrame->Show(!pFrame->IsVisible());
		//				return 0; // Ngắt Message
		//			}
		//		}
		//	}

		//	break;
		//}

		//case WM_KEYUP:
		//{
		//	if (g_KeyTimerQPC.count(wParam)) {
		//		LARGE_INTEGER liNow;
		//		QueryPerformanceCounter(&liNow);
		//		double dwDuration = (double)(liNow.QuadPart - g_KeyTimerQPC[wParam].QuadPart) * 1000.0 / g_QPCFreq.QuadPart;

		//		// MỐC BẮT PHÍM LẺ: < 7ms 
		//		if (dwDuration > 0.0 && dwDuration < 0.1) {
		//			g_KeyChainMap[wParam]++;
		//			 mlog("ZGuard: [SINGLE KEY] 0x%X Chain [%d] - %.2f ms\n", wParam, g_KeyChainMap[wParam], dwDuration);

		//			if (g_KeyChainMap[wParam] >= 5) { // Phím đơn cho phép 5 lần (nới lỏng hơn combo)
		//				ZGuard_Kick("Single Key Macro Spam");
		//			}
		//		}
		//		else {
		//			// MỐC RESET PHÍM ĐƠN: Bấm phím đó chậm lại (>7ms) là reset
		//			g_KeyChainMap[wParam] = 0;
		//		}
		//		g_KeyTimerQPC.erase(wParam);
		//	}
		//	break;
		//}
		
		// for new UI: phim tat cho khung chat o phong cho
		case WM_KEYDOWN:
		{
			ZGameInterface* pGI = ZApplication::GetGameInterface();
			if (pGI != NULL && pGI->GetState() == GUNZ_STAGE)
			{
				ZIDLResource* pRes = pGI->GetIDLResource();
				MWidget* pFrame = pRes->FindWidget("StageChattingFrame");
				MEdit* pInput = (MEdit*)pRes->FindWidget("StageChattingInput");

				if (pFrame != NULL && pInput != NULL && pInput->IsFocus() == false)
				{
					if (wParam == VK_RETURN)
					{
						pFrame->Show(true);
						pInput->SetFocus();
						return 0;
					}
					if (wParam == 'Z')
					{
						pFrame->Show(!pFrame->IsVisible());
						return 0;
					}
				}
			}
		}
		break;

		//case WM_RBUTTONDOWN:
		//{
		//	QueryPerformanceCounter(&g_MouseTimerQPC[1]); // Chuột phải
		//	break;
		//}

		//case WM_LBUTTONUP:
		//case WM_RBUTTONUP:
		//{
		//	int btnId = (message == WM_LBUTTONUP) ? 0 : 1;
		//	if (g_MouseTimerQPC.count(btnId)) {
		//		LARGE_INTEGER liNow;
		//		QueryPerformanceCounter(&liNow);
		//		double dwDuration = (double)(liNow.QuadPart - g_MouseTimerQPC[btnId].QuadPart) * 1000.0 / g_QPCFreq.QuadPart;

		//		if (dwDuration > 0.0 && dwDuration < 11.0) {
		//			g_nMouseChain[btnId]++;
		//			if (g_nMouseChain[btnId] >= 3) {
		//				ZGuard_Kick(btnId == 0 ? "Mouse Left Macro" : "Mouse Right Macro");
		//			}
		//		}
		//		else {
		//			g_nMouseChain[btnId] = 0;
		//		}
		//		g_MouseTimerQPC.erase(btnId);
		//	}
		//	break;
		//}

		case WM_SYSCHAR:
			if(ZIsLaunchDevelop() && wParam==VK_RETURN)
			{
#ifndef _PUBLISH
				RFrame_ToggleFullScreen();
#endif
				return 0;
			}
			break;

		case WM_CREATE:
			if (strlen(Z_LOCALE_HOMEPAGE_TITLE) > 0)
			{
				ShowIExplorer(false, Z_LOCALE_HOMEPAGE_TITLE);
			}
			break;
		case WM_DESTROY:
			if (strlen(Z_LOCALE_HOMEPAGE_TITLE) > 0)
			{
				ShowIExplorer(true, Z_LOCALE_HOMEPAGE_TITLE);
			}
			break;
		case WM_SETCURSOR:
			if(ZApplication::GetGameInterface())
				ZApplication::GetGameInterface()->OnResetCursor();
			return TRUE; // prevent Windows from setting cursor to window class cursor

		case WM_ENTERIDLE:
			// ¸ð´Þ ´E­»óÀÚ°¡ ÄÚµå¸¦ ºúÓ°ÇÏ°EÀÖÀ» ¶§ ºÎ¸ð¿¡°Ô º¸³»´Â idle ÅEö¸Þ½ÃÁE
			// (ÀÏº» IME¿¡ ¸ð´Þ ´E­»óÀÚ°¡ ÀÖ¾ûØ­ ³Ö¾úÀ½)
			// ¸ð´Þ ´E­»óÀÚ·Î ¾÷µ¥ÀÌÆ® ·çÇÁ¸¦ ºúÓ°ÇØ¼­ ¹«ÀE¾ûÖäÁûÓÎ ¾Ç¿EÇ±E¶§¹®¿¡ ¿©±â¼­ ¾÷µ¥ÀÌÆ®¸¦ ½ÇÇàÇÑ´Ù
			RFrame_UpdateRender();
			break;

			/*
		case  WM_LBUTTONDOWN:
			SetCapture(hWnd);
			return TRUE;
		case WM_LBUTTONUP:
			ReleaseCapture();
			return TRUE;
			*/
		//case WM_KEYDOWN:
		//	{
		//		bool b = false;
		//	}
	}

	if(Mint::GetInstance()->ProcessEvent(hWnd, message, wParam, lParam)==true)
	{
		if (ZGetGameInterface() && ZGetGameInterface()->IsReservedResetApp())	// for language changing
		{
			ZGetGameInterface()->ReserveResetApp(false);
			ResetAppResource();
		}

		return 0;
	}

	// thread safeÇÏ±âÀ§ÇØ ³ÖÀ½
	if (message == WM_CHANGE_GAMESTATE)
	{
		_ZChangeGameState(wParam);
	}


	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
class mtrl {
public:

};

class node {
public:
	int		m_nIndex[5];
};


class _map{
public:
	mtrl* GetMtrl(node* node,int index) { return GetMtrl(node->m_nIndex[index]); }
	mtrl* GetMtrl(int id) { return m_pIndex[id]; }

	mtrl*	m_pIndex[5];
};

class game {
public:
	_map m_map;	
};

game _game;
game* g_game;
*/

bool CheckFileList()
{
	MZFileSystem *pfs=ZApplication::GetFileSystem();
	MZFile mzf;

	string strFileNameFillist(FILENAME_FILELIST);
#ifndef _DEBUG
	strFileNameFillist += ""; // MEF NULL
#endif

	if(!mzf.Open(strFileNameFillist.c_str() ,pfs))
		return false;

	char *buffer;
	buffer=new char[mzf.GetLength()+1];
	mzf.Read(buffer,mzf.GetLength());
	buffer[mzf.GetLength()]=0;

	MXmlDocument aXml;
	aXml.Create();
	if(!aXml.LoadFromMemory(buffer))
	{
		delete[] buffer;
		return false;
	}

	delete[] buffer;

	int iCount, i;
	MXmlElement		aParent, aChild;
	aParent = aXml.GetDocumentElement();
	iCount = aParent.GetChildNodeCount();

	char szTagName[256];
	for (i = 0; i < iCount; i++)
	{
		aChild = aParent.GetChildNode(i);
		aChild.GetTagName(szTagName);
		if(stricmp(szTagName,"FILE")==0)
		{
			char szContents[256],szCrc32[256];
			aChild.GetAttribute(szContents,"NAME");
			aChild.GetAttribute(szCrc32,"CRC32");

			if(stricmp(szContents,"config.xml")!=0)
			{
				unsigned int crc32_list = pfs->GetCRC32(szContents);
				unsigned int crc32_current;
				sscanf(szCrc32,"%x",&crc32_current);

#ifndef _DEBUG
				if(crc32_current!=crc32_list)
				{
					// mlog("crc error , file %s ( current = %x, original = %x ).\n",szContents,crc32_current,crc32_list);

					// ¸ðµEÆÄÀÏÀ» °Ë»ç´Â ÇÑ´Ù
					return false; 
				}
#endif
			}
		}
	}

	return true;
}


enum RBASE_FONT{
	RBASE_FONT_GULIM = 0,
	RBASE_FONT_BATANG = 1,

	RBASE_FONT_END
};

static int g_base_font[RBASE_FONT_END];
static char g_UserDefineFont[256];

bool _GetFileFontName(char* pUserDefineFont)
{
	if(pUserDefineFont==NULL) return false;

	FILE* fp = fopen("_Font", "rt");
	if (fp) {
		fgets(pUserDefineFont,256, fp);
		fclose(fp);
		return true;
	}
	return false;
}

bool CheckFont()
{
	char FontPath[MAX_PATH];
	char FontNames[MAX_PATH+100];

	::GetWindowsDirectory(FontPath, MAX_PATH);

	strcpy(FontNames,FontPath);
	strcat(FontNames, "\\Fonts\\gulim.ttc");

	if (_access(FontNames,0) != -1)	{ g_base_font[RBASE_FONT_GULIM] = 1; }
	else							{ g_base_font[RBASE_FONT_GULIM] = 0; }

	strcpy(FontNames,FontPath);
	strcat(FontNames, "\\Fonts\\batang.ttc");

	if (_access(FontNames,0) != -1)	{ g_base_font[RBASE_FONT_BATANG] = 1; }
	else							{ g_base_font[RBASE_FONT_BATANG] = 0; }

	//	strcpy(FontNames,FontPath);
	//	strcat(FontNames, "\\Fonts\\System.ttc");
	//	if (_access(FontNames,0) != -1)	{ g_font[RBASE_FONT_BATANG] = 1; }
	//	else							{ g_font[RBASE_FONT_BATANG] = 0; }

	if(g_base_font[RBASE_FONT_GULIM]==0 && g_base_font[RBASE_FONT_BATANG]==0) {//µÑ´Ù¾øÀ¸¸E.

		if( _access("_Font",0) != -1) { // ÀÌ¹Ì ±â·ÏµÇ¾EÀÖ´Ù¸E.
			_GetFileFontName( g_UserDefineFont );
		}
		else {

			int hr = IDOK;

			//hr = ::MessageBox(NULL,"±ÍÇÏÀÇ ÄÄÇ»ÅÍ¿¡´Â °ÇÁûÌ¡ »ç¿EÏ´Â (±¼¸²,µ¸¿E ÆùÆ®°¡ ¾ø´Â °Í °°½À´Ï´Ù.\n ´Ù¸¥ ÆùÆ®¸¦ ¼±ÅÃ ÇÏ½Ã°Ú½À´Ï±E","¾Ë¸²",MB_OKCANCEL);
			//hr = ::MessageBox(NULL,"±ÍÇÏÀÇ ÄÄÇ»ÅÍ¿¡´Â °ÇÁûÌ¡ »ç¿EÏ´Â (±¼¸²,µ¸¿E ÆùÆ®°¡ ¾ø´Â °Í °°½À´Ï´Ù.\n °è¼Ó ÁøÇEÇÏ½Ã°Ú½À´Ï±E","¾Ë¸²",MB_OKCANCEL);

			if(hr==IDOK) {
				/*			
				CFontDialog dlg;
				if(dlg.DoModal()==IDOK) {
				CString facename = dlg.GetFaceName();
				lstrcpy((LPSTR)g_UserDefineFont,(LPSTR)facename.operator const char*());

				hr = ::MessageBox(NULL,"¼±ÅÃÇÏ½Å ÆùÆ®¸¦ ÀúÀEÇÏ½Ã°Ú½À´Ï±E","¾Ë¸²",MB_OKCANCEL);

				if(hr==IDOK)
				_SetFileFontName(g_UserDefineFont);
				}
				*/
				return true;
			}
			else {
				return false;
			}
		}
	}
	return true;
}

#include "shlobj.h"

void CheckFileAssociation()
{
#define GUNZ_REPLAY_CLASS_NAME	"GunzReplay"

	// Ã¼Å©ÇØºÁ¼­ µûÓÏÀÌ ¾ÈµÇ¾ûÜÖÀ¸¸EµûÓÏÇÑ´Ù. »ç¿EÚ¿¡°Ô ¹°¾ûÖ¼¼öµµ ÀÖ°Ú´Ù.
	char szValue[256];
	if(!MRegistry::Read(HKEY_CLASSES_ROOT,"." GUNZ_REC_FILE_EXT,NULL,szValue))
	{
		MRegistry::Write(HKEY_CLASSES_ROOT,"." GUNZ_REC_FILE_EXT,NULL,GUNZ_REPLAY_CLASS_NAME);

		char szModuleFileName[_MAX_PATH] = {0,};
		GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);

		char szCommand[_MAX_PATH];
		sprintf(szCommand,"\"%s\" \"%%1\"",szModuleFileName);

		MRegistry::Write(HKEY_CLASSES_ROOT,GUNZ_REPLAY_CLASS_NAME"\\shell\\open\\command",NULL,szCommand);

		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, NULL, NULL);
	}
}

// ÇØ´EÅØ½ºÆ® ÆÄÀÏ¿¡ ÇØ´E±Û±Í°¡ ÀÖÀ¸¸EXTRAP Å×½ºÆ® ÄÚµå°¡ ¼öÇàµÈ´Ù. (µÞ±¸¸Û) //
bool CheckXTrapIsGetTestCode()														// add sgk 0621
{
	char szBuf[256] = "";
	FILE* fp = fopen("XTrapTest.txt", "rt");
	if (fp)
	{
		fgets(szBuf, 256, fp);
		mlog("XTrapTest.txt : \n");
		mlog(szBuf);
		mlog("\n");
		fclose(fp);

		if (stricmp(szBuf, "RUN_XTRAP_TEST_CODE") == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		mlog("fail to open XTrapTest.txt\n");
		return false;
	}
}

void OnGetXTrapRealCodeArgv(char* szTemp, bool* bPatchSkip)							// add sgk 0621
{
	/* LOCALE_KOREA */
	wsprintf(szTemp, "660970B478F9CB6395356D98440FE8629F86DDCD4DBB54B513680296C770DFCDB63DCCFE8DFE34B3A6B2BAA8CFB5C8C9761A2D3F1E491F9EFF6757489912B1AB0F7D04245246E409A061005FC9527B0823FF73E0B9F4C91EB67C40AC1A16EABB8FE52BDE25C76F6955E9E52A0812CE");

#ifdef LOCALE_JAPAN
	memset(szTemp, 0, 256);
	wsprintf(szTemp, "660970B45869CA6395356D98440FE8624C8FEA6EF181FD7D095D6CBA9911AFB0B5661B972C3C82BB0FF2D47A32DFB56D407CB146190E29B1EA46F49C1E86160F0F7D04245246E409A061005FC9527B086EF578A8BCFCC91FB67C51F65E05AAB85F7E306086BDFF03DF1BA46A66C605FFBC6263206088B68D6930514A");
#endif

#ifdef LOCALE_US
	memset(szTemp, 0, 256);
	wsprintf(szTemp, "660970B497F9CB6395356D98440FE8629AE854BDDBD13EDCE69AC1898F7A23CEF138AD2BF2758B368950133F1B021D0D218BFB058146B32450591F8B22CBE6A2");
	*bPatchSkip = true;
#endif

#ifdef LOCALE_INDIA
	memset(szTemp, 0, 256);
//	wsprintf(szTemp, "660970B47C69CB6795356D98440FE8625582AC40166A109C00E4D6A6056D18A02BBAC0A19DA6BEE6B4D43AD07CFB61697FD7FF586D98ECFF1DA11222DD82028D0F7D04245246E417A4610E569557620395165EECCBF7CD9008C4C0120CA7A0AD9D568C0DC8C7BD38C629B7EAAE5435B46105721F036F7C5BF0");
	wsprintf(szTemp, "660970B47C69CB6795356D98490FE862FEBC286C65D77538F80891D97D18B65B43E538B6EADB14290A04CF119B162DE7AA91984B54023E368FB4C25D4A91F68A0F7D04245246E417A4610E569557620395165EECCBF7CD9008C4C0120CA7A0AD9D568C0DC8C7BD38C629B7EAAE5435B46105721F036F7C5BF0");
#endif

#ifdef LOCALE_BRAZIL
	memset(szTemp, 0, 256);
	wsprintf(szTemp, "660970B448FBCB6395356D98440FE8621A6EADB8532B3C5F1949386F921C6C0970FEF0A168B5352668BE414ADF1375136173F493A8A2C075AC0F919AC7241A650F7D04245246E401B574195DD31E6305975703051B9F4F5CA2A8046A5FF3331AB0C8F040AFA98BB5CE3134520AC79D1328E836DF645FC479");
#endif
}

void OnGetXTrapTestCodeArgv(char* szTemp, bool* bPatchSkip)							// add sgk 0621
{
	/* LOCALE_KOREA */
	wsprintf(szTemp, "660970B478F9CB6395356D98440FE8629F86DDCD4DBB54B513680296C770DFCDB63DCCFE8DFE34B3A6B2BAA8CFB5C8C9761A2D3F1E491F9EFF6757489912B1AB0F7D04245246E409A061005FC9527B0823FF73E0B9F4C91EB67C40AC1A16EABB8FE52BDE25C76F6955E9E52A0812A88323D4");

#ifdef LOCALE_JAPAN
	memset(szTemp, 0, 256);
	wsprintf(szTemp, "660970B45869CA6395356D98440FE8624C8FEA6EF181FD7D095D6CBA9911AFB0B5661B972C3C82BB0FF2D47A32DFB56D407CB146190E29B1EA46F49C1E86160F0F7D04245246E409A061005FC9527B086EF578A8BCFCC91FB67C51F65E05AAB85F7E306086BDFF03DF1BA46A66C605FFBC6263206088B68D6930512C295649");
#endif

#ifdef LOCALE_US
	memset(szTemp, 0, 256);
	wsprintf(szTemp, "660970B497F9CB6395356D98440FE8629AE854BDDBD13EDCE69AC1898F7A23CEF138AD2BF2758B368950133F1B021D0D218BFB058146B32450591F8B22CBE6A2");
	*bPatchSkip = true;
#endif

#ifdef LOCALE_INDIA
	memset(szTemp, 0, 256);
//	wsprintf(szTemp, "660970B47C69CB6795356D98440FE8625582AC40166A109C00E4D6A6056D18A02BBAC0A19DA6BEE6B4D43AD07CFB61697FD7FF586D98ECFF1DA11222DD82028D0F7D04245246E417A4610E569557620395165EECCBF7CD9008C4C0120CA7A0AD9D568C0DC8C7BD38C629B7EAAE5435B46105721F036F7C5B962980B7");
	wsprintf(szTemp, "660970B47C69CB6795356D98490FE862FEBC286C65D77538F80891D97D18B65B43E538B6EADB14290A04CF119B162DE7AA91984B54023E368FB4C25D4A91F68A0F7D04245246E417A4610E569557620395165EECCBF7CD9008C4C0120CA7A0AD9D568C0DC8C7BD38C629B7EAAE5435B46105721F036F7C5B962980B7");
#endif

#ifdef LOCALE_BRAZIL
	memset(szTemp, 0, 256);
	wsprintf(szTemp, "660970B448FBCB6395356D98440FE8621A6EADB8532B3C5F1949386F921C6C0970FEF0A168B5352668BE414ADF1375136173F493A8A2C075AC0F919AC7241A650F7D04245246E401B574195DD31E6305975703051B9F4F5CA2A8046A5FF3331AB0C8F040AFA98BB5CE3134520AC79D1328E836DF645FC41F2B9A7E");
#endif
}
#ifdef _LAUNCHER
void UpgradeMrsFile()
{
	char temp_path[1024];
	sprintf(temp_path, "*");

	FFileList file_list;
	GetFindFileListWin(temp_path, _EXTFILEMRS_E, file_list);
	file_list.UpgradeMrs();
}
#endif
HANDLE Mutex = 0;

#ifdef _HSHIELD
int __stdcall AhnHS_Callback(long lCode, long lParamSize, void* pParam);
#endif

DWORD g_dwMainThreadID;


//------------------------------------------- nhn usa -------------------------------------------------------------
bool InitReport()
{
	return true;

	// Custom: Disable NHN Auth
	/*
#ifdef LOCALE_NHNUSA
	mlog( "Init report start\n" );
	if( !GetNHNUSAReport().InitReport(((ZNHN_USAAuthInfo*)(ZGetLocale()->GetAuthInfo()))->GetUserID().c_str(),
		((ZNHN_USAAuthInfo*)(ZGetLocale()->GetAuthInfo()))->GetGameStr()) )
	{
		mlog( "Init nhn report fail.\n" );
		return false;
	}
	GetNHNUSAReport().ReportStartGame();
	mlog( "Init report success.\n" );
#endif

	return true;
	*/
}

bool InitPoll()
{
	return true;

	// Custom: Disable NHN Auth
	/*
#ifdef LOCALE_NHNUSA
	mlog( "Init poll start\n" );

	((ZNHN_USAAuthInfo*)(ZGetLocale()->GetAuthInfo()))->ZUpdateGameString();

	if( !GetNHNUSAPoll().ZHanPollInitGameString( ((ZNHN_USAAuthInfo*)(ZGetLocale()->GetAuthInfo()))->GetGameStr()) )
		return false;
#endif

	return true;
	*/
}


bool CheckGameGuardHackToolUser()
{
#ifdef _GAMEGUARD

	string strUserID;
	ZBaseAuthInfo* pAuth = ZGetLocale()->GetAuthInfo();

	if ( pAuth == NULL)
		return true;


#ifdef LOCALE_NHNUSA

#ifdef _DEBUG_NHN_USA_AUTH
	return true;
#endif

	strUserID = ((ZNHN_USAAuthInfo*)pAuth)->GetUserID();

#elif LOCALE_JAPAN

//	strUserID = ((ZGameOnJPAuthInfo*)pAuth)->GetUserID();
	return true;

#endif


	if( !GetZGameguard().CheckHackToolUser( strUserID.c_str()) )
		return false;


#endif	// _GAMEGUARD

	return true;
}


//------------------------------------------- nhn usa end----------------------------------------------------------

BOOL SetDumpPrivileges()
{
	BOOL       fSuccess  = FALSE;
	HANDLE      TokenHandle = NULL;
	TOKEN_PRIVILEGES TokenPrivileges;

	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
		&TokenHandle))
	{
		mlog("Dmp token fail\n");
		//printf("Could not get the process token");
		goto Cleanup;
	}

	TokenPrivileges.PrivilegeCount = 1;

	if (!LookupPrivilegeValue(NULL,
		SE_DEBUG_NAME,
		&TokenPrivileges.Privileges[0].Luid))
	{
		mlog("Dmp lookup fail\n");
		//printf("Couldn't lookup SeDebugPrivilege name");
		goto Cleanup;
	}

	TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	//Add privileges here.
	if (!AdjustTokenPrivileges(TokenHandle,
		FALSE,
		&TokenPrivileges,
		sizeof(TokenPrivileges),
		NULL,
		NULL))
	{
		mlog("Dmp adjust fail\n");
		//printf("Could not revoke the debug privilege");
		goto Cleanup;
	}

	fSuccess = TRUE;

Cleanup:

	if (TokenHandle)
	{
		CloseHandle(TokenHandle);
	}

	return fSuccess;
}
int PASCAL WinMain(HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow)
{

	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	InitLog(MLOGSTYLE_DEBUGSTRING|MLOGSTYLE_FILE);
#ifdef _ANTIHACK
	AntiSB();
	ClearFiles();
#endif
#ifdef _EAC
	if (GetFileAttributesA("Gunz.exe.local") != INVALID_FILE_ATTRIBUTES)
	{
		mlog("Easy Anti-Cheat: this is the protected instance.\n");
	}
	else
	{
		mlog("EAC not active: launching start_protected_game.exe...\n");

		// Chặn trường hợp chạy trùng 2 Gunz.exe cùng lúc
		HANDLE hMutex = CreateMutexA(NULL, TRUE, "GUNZ_EAC_MUTEX");
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			mlog("Another Gunz.exe is already running, aborting this instance.\n");
			return 0;
		}

		// Đường dẫn tới start_protected_game.exe
		STARTUPINFOA si;
		PROCESS_INFORMATION procInfo;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&procInfo, sizeof(procInfo));
		si.cb = sizeof(si);

		char exePath[MAX_PATH];
		GetModuleFileNameA(NULL, exePath, MAX_PATH);
		PathRemoveFileSpecA(exePath);
		strcat_s(exePath, "\\start_protected_game.exe");

		if (GetFileAttributesA(exePath) == INVALID_FILE_ATTRIBUTES)
		{
			MessageBoxA(NULL, "Easy Anti-Cheat launcher not found.\nPlease reupdate the game.", "Easy Anti-Cheat", MB_OK | MB_ICONERROR);
			return 0;
		}
		LoadKeyboardLayoutA("0000042a", KKLF_ACTIVATE | KKLF_SETFORPROCESS); // Mồi Việt
		LoadKeyboardLayoutA("00000804", KKLF_ACTIVATE | KKLF_SETFORPROCESS); // Mồi Trung
		BOOL success = CreateProcessA(exePath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &procInfo);

		if (!success)
		{
			MessageBoxA(NULL, "Unable to launch Easy Anti-Cheat.", "Easy Anti-Cheat", MB_OK | MB_ICONERROR);
			return 0;
		}

		// Chờ EAC tạo file Gunz.exe.local (báo hiệu đã spawn bản được bảo vệ)
		DWORD exitCode = 0;
		DWORD startTime = GetTickCount();
		bool eacReady = false;

		while (true)
		{
			Sleep(500);

			if (!GetExitCodeProcess(procInfo.hProcess, &exitCode))
				break;

			if (exitCode != STILL_ACTIVE)
			{
				MessageBoxA(NULL, "Easy Anti-Cheat closed unexpectedly.\nUnable to continue launching the game.", "Easy Anti-Cheat", MB_OK | MB_ICONERROR);
				CloseHandle(procInfo.hProcess);
				CloseHandle(procInfo.hThread);
				return 0;
			}

			if (GetFileAttributesA("Gunz.exe.local") != INVALID_FILE_ATTRIBUTES)
			{
				eacReady = true;
				mlog("EAC ready, exiting launcher instance.\n");
				break;
			}
		}

		CloseHandle(procInfo.hProcess);
		CloseHandle(procInfo.hThread);

		if (eacReady)
		{
			// Khi EAC đã sẵn sàng, thoát bản gốc ngay để chỉ còn bản EAC spawn
			ExitProcess(0);
		}
	}
	// Khởi tạo SDK
	EOS_InitializeOptions sdkOptions = { 0 };
	sdkOptions.ApiVersion = EOS_INITIALIZE_API_LATEST;
	sdkOptions.AllocateMemoryFunction = nullptr;
	sdkOptions.ReallocateMemoryFunction = nullptr;
	sdkOptions.ReleaseMemoryFunction = nullptr;
	sdkOptions.ProductName = "GunzVN";            // Phải có
	sdkOptions.ProductVersion = "1.0";          // Phải có
	EOS_EResult initRes = EOS_Initialize(&sdkOptions);

	if (initRes != EOS_EResult::EOS_Success) {
		char msg[256];
		sprintf(msg, "Epic Online Services (EOS) Initialize failed: %s", EOS_EResult_ToString(initRes));
		MessageBoxA(NULL, msg, "EAC", MB_OK | MB_ICONERROR);
	}
	mlog("Epic Online Services (EOS) Initialize success.\n");

	// Khởi tạo Platform
	EOS_Platform_Options platformOptions = { 0 };
	platformOptions.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
	platformOptions.ProductId = "a91729130eb949eaa22ef2e3bd7ea5cb";
	platformOptions.SandboxId = "61afb5ef566148b0bfdda3bba8b80407";
	platformOptions.DeploymentId = "3e39e527bef64ad095d5ae52781d3c95";
	platformOptions.ClientCredentials.ClientId = "xyza7891guWoBMPDSNQp8Ceio1GLqTX3";
	platformOptions.ClientCredentials.ClientSecret = "j0ktCFEErPzohdU/dskYNj+vZncXhgH4bPcWV8VaiGI";
	platformOptions.bIsServer = EOS_FALSE;
	platformOptions.EncryptionKey = NULL;
	platformOptions.OverrideCountryCode = NULL;
	platformOptions.OverrideLocaleCode = NULL;
	platformOptions.CacheDirectory = NULL;
	platformOptions.TickBudgetInMilliseconds = 0;
	platformOptions.RTCOptions = NULL;
	platformOptions.IntegratedPlatformOptionsContainerHandle = NULL;
	platformOptions.SystemSpecificOptions = NULL;
	platformOptions.TaskNetworkTimeoutSeconds = NULL;

	g_hPlatform = EOS_Platform_Create(&platformOptions);
	if (!g_hPlatform) {
		MessageBoxA(NULL, "Epic Online Services (EOS) Platform Create failed - Check Dev Portal IDs", "Easy Anti-Cheat", MB_OK | MB_ICONERROR);
		EOS_Shutdown();
		return false;
	}
	mlog("Epic Online Services (EOS) Platform Create success.\n");

	g_hMetrics = EOS_Platform_GetMetricsInterface(g_hPlatform);
	if (g_hMetrics == NULL) {
		mlog("EOS Metrics interface not available (g_hMetrics == NULL)\n");
	}
	else {
		mlog("Epic Online Services (EOS) Metrics interface acquired.\n");
	}

	// Lấy interface AntiCheat Client
	EOS_HAntiCheatClient hAntiCheat = EOS_Platform_GetAntiCheatClientInterface(g_hPlatform);
	if (!hAntiCheat) {
		mlog("Failed to get AntiCheat Client interface. Platform handle: %p\n", (void*)g_hPlatform);
		MessageBoxA(NULL, "Failed to get AntiCheat Client interface - Check EAC setup or Dev Portal", "Easy Anti-Cheat", MB_OK | MB_ICONERROR);
		EOS_Platform_Release(g_hPlatform);
	}
	else {
		mlog("Epic Online Services (EOS) AntiCheat Client interface success. Handle: %p\n =============Easy Anti-Cheat (EAC)==============\n\n", (void*)g_hPlatform);
	}

	// 1. Khai báo Options (Đúng theo Header)
	EOS_AntiCheatClient_AddNotifyPeerActionRequiredOptions ActionOptions = {};
	ActionOptions.ApiVersion = EOS_ANTICHEATCLIENT_ADDNOTIFYPEERACTIONREQUIRED_API_LATEST;

	// 2. Đăng ký Callback
	EOS_AntiCheatClient_AddNotifyPeerActionRequired(hAntiCheat, &ActionOptions, nullptr,
		[](const EOS_AntiCheatCommon_OnClientActionRequiredCallbackInfo* Data) {

			if (Data == nullptr) return;

			// KIỂM TRA ACTION (Trong Header là ClientAction và ActionReasonCode)
			// Enum đúng là: EOS_EAntiCheatCommonClientAction::EOS_ACCCA_RemovePlayer
			if (Data->ClientAction == EOS_EAntiCheatCommonClientAction::EOS_ACCCA_RemovePlayer)
			{
				mlog("EAC: Violation detected! Reason: %s\n",
					(int)Data->ActionReasonCode,
					Data->ActionReasonDetailsString);

				// Hiển thị thông báo và đóng game
				MessageBoxA(NULL, Data->ActionReasonDetailsString, "Easy Anti-Cheat", MB_OK | MB_ICONSTOP);
				ExitProcess(0);
			}
		}
	);

	CreateThread(NULL, 0, WatchdogThreadProc, NULL, 0, NULL);
#endif

	g_fpOnCrcFail = CrcFailExitApp;

#ifdef LOCALE_JAPAN
	ZGameOnJPAuthInfo::m_hLauncher = ::FindWindow( NULL, TITLE_PUBLAGENT );
#endif

	g_dwMainThreadID = GetCurrentThreadId();
	
#ifdef _MTRACEMEMORY
	MInitTraceMemory();
#endif

	char szModuleFileName[_MAX_DIR] = {0,};
	GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);
	PathRemoveFileSpec(szModuleFileName);
	SetCurrentDirectory(szModuleFileName);

//#define _MULTICLIENT 1

#if defined(_MULTICLIENT)
	if (!fopen("mutex", "r"))
		Mutex = CreateMutex(NULL, TRUE, "Gunz");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(g_hWnd, "Cannot Open Gunz.exe", NULL, MB_OK);
		mlog("Cannot Open Gunz.exe\n");
		//zexit(-1);
		ExitProcess(NULL);
		return 0;
	}
#endif															// update sgk 0702 end

#ifdef _LICENSE
	Checks::LicenseSystem();
#endif

#ifdef _SERIALCUSTOM
	Serial::SerialKey();
#endif

	srand( (unsigned)time( NULL ));

	// Custom: Added defined(LOCALE_NHNUSA)
#if defined(LOCALE_BRAZIL) || defined(LOCALE_INDIA) || defined(LOCALE_US) || defined(LOCALE_KOREA) || defined(LOCALE_NHNUSA)
	//#ifndef _DEBUG
	#ifdef _PUBLISH
		
	#endif
#endif	// LOCALE_NHNUSA

	// ·Î±× ½ÃÀÛ
	//mlog("GUNZ " STRFILEVER " launched. build (" __DATE__ " " __TIME__ ") \n");
	//char szDateRun[128]="";
	//char szTimeRun[128]="";
	//_strdate( szDateRun );
	//_strtime( szTimeRun );
	//mlog("Log time (%s %s)\n", szDateRun, szTimeRun);
	//mlog("Coded By Desperate\n"); // Custom: My runnable string

#ifndef _PUBLISH
	mlog("cmdline = %s\n",cmdline);

#endif

#ifdef _LAUNCHER
	UpgradeMrsFile();
#endif

	MSysInfoLog();

#if !defined(_EAC)
#ifdef _ANTIHACK
#ifdef _THEMIDAGUNZ
	VM_START
#endif
	ZGetAntiCheat()->SetMainWnd(this_inst);
	ZGetAntiCheat()->OnLauncherInit();
#ifdef _THEMIDAGUNZ
	VM_END
#endif
#endif
#endif

	// Custom: Force unsupported OS
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx( (OSVERSIONINFO*)&os );

	if (os.dwMajorVersion <= 4)
	{
		mlog("Unsupported OS... exiting.\n");
		return false;
	}

	bool bCompatibilityMode = false;
	char szModule[MAX_PATH] = { 0 };
	GetModuleFileName(GetModuleHandle(NULL), szModule, sizeof(szModule));
	
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", NULL, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		char szValue[MAX_PATH] = { 0 };
		
		DWORD nLen = _MAX_PATH;
		DWORD dwType = REG_SZ;

		if (RegQueryValueEx(hKey, szModule, NULL, &dwType, (unsigned char *)szValue, &nLen) == ERROR_SUCCESS)
		{
			if (strstr(szValue, "WINXPSP2") || strstr(szValue, "WINXPSP3") || strstr(szValue, "VISTARTM") || strstr(szValue, "VISTASP1") || strstr(szValue, "VISTASP2") || strstr(szValue, "WIN7RTM") || strstr(szValue, "WIN8RTM"))
			{
				bCompatibilityMode = true;
			}
		}

		RegCloseKey(hKey);
	}

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", NULL, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		char szValue[MAX_PATH] = { 0 };
		
		DWORD nLen = _MAX_PATH;
		DWORD dwType = REG_SZ;

		if (RegQueryValueEx(hKey, szModule, NULL, &dwType, (unsigned char *)szValue, &nLen) == ERROR_SUCCESS)
		{
			if (strstr(szValue, "WINXPSP2") || strstr(szValue, "WINXPSP3") || strstr(szValue, "VISTARTM") || strstr(szValue, "VISTASP1") || strstr(szValue, "VISTASP2") || strstr(szValue, "WIN7RTM") || strstr(szValue, "WIN8RTM"))
			{
				bCompatibilityMode = true;
			}
		}

		RegCloseKey(hKey);
	}

	if (bCompatibilityMode)
	{
		mlog("ERROR: Running under compatibility mode.\n");
		MessageBox(g_hWnd, "Failed to run GunZ in compatibility mode.\nPlease remove compatibility mode settings and launch the game again!", "GunZ", MB_OK | MB_ICONERROR);
		return false;
	}

//	if (CheckVideoAdapterSupported() == false)
//		return 0;

	CheckFileAssociation();

	// Initialize MZFileSystem - MUpdate 
	MRegistry::szApplicationName=APPLICATION_NAME;

	g_App.InitFileSystem();

//	mlog("CheckSum: %u \n", ZApplication::GetFileSystem()->GetTotalCRC());

	//if(!InitializeMessage(ZApplication::GetFileSystem())) {
	//	MLog("Check Messages.xml\n");
	//	return 0;
	//}

//	³Ý¸¶ºE¹öÀEº ±¸ºÐÇØ¾ßÇÔ... ³Ý¸¶ºE¹öÀEº MZIPREADFLAG_MRS1 µµ ÀÐ¾ûÚßÇÔ...

#ifdef _PUBLISH
//	#ifndef NETMARBLE_VERSION
		//MZFile::SetReadMode( MZIPREADFLAG_MRS2 );
//	#endif
#endif


#ifdef LOCALE_NHNUSA
	// NHNUSA´Â Ä¿¸Çµå¶óÀÎÀ¸·Î ¾ð¾ûØ±ÅÃÀ» ¾Ë·ÁÁØ´Ù, ´Ù¸¥ Áö¿ªºôµå¶óµµ µðÆÄÀÎ ÇÊ¿ä¾øÀ½
	ZGetLanguageSetting_forNHNUSA()->SetLanguageIndexFromCmdLineStr(cmdline);
#endif

	CreateRGMain();

	ZGetConfiguration()->Load();

	/*if (os.dwMajorVersion >= 6 && Z_ETC_D3D9E)
	{
		g_isDirect3D9ExEnabled = true;
	}*/

	ZStringResManager::MakeInstance();
	if( !ZApplication::GetInstance()->InitLocale() )
	{
		MLog("Locale Init error !!!\n");
		return false;
	}

	ZGetConfiguration()->Load_StringResDependent();

	// ¿©±â¼­ ¸ÞÅ©·Î ÄÁ¹öÆÃ... ¸Õ°¡ ±¸¸®±¸¸®~~ -by SungE.
	if( !ZGetConfiguration()->LateStringConvert() )
	{
		MLog( "main.cpp - Late string convert fale.\n" );
		return false;
	}

	DWORD ver_major = 0;
	DWORD ver_minor = 0;
	TCHAR ver_letter = ' ';

	// ÀÇ¹Ì¾øÀ½ ... ¿ÜºÎ¿¡¼­ dll ÀÌ ¾ø´Ù°E¸ÕÀú¶E..

/*_
	bool DXCheck = false;

	if( SUCCEEDED( GetDirectXVersionViaDxDiag( &ver_major, &ver_minor, &ver_letter ) ) ) {
		if(ver_major >= 8)
			DXCheck = true;
	} // ¼º°E¸øÇÑ °æ¿E¾Ë¼ö¾øÀ¸¹Ç·Î ½ÇÆÐ~

	if(DXCheck==false) {
		::MessageBox(NULL,"DirectX 8.0 ÀÌ»óÀ» ¼³Ä¡ÇÏ°E´Ù½Ã ½ÇÇàÇØ ÁÖ½Ã±E¹Ù¶ø´Ï´Ù.","¾Ë¸²",MB_OK);
	}
*/

#ifdef SUPPORT_EXCEPTIONHANDLING
	if (SetDumpPrivileges())
		mlog(" =================Load XML Files=================\n\n");
	else
		mlog("Dmp - FAILED\n");

#ifdef _SERIALCUSTOM
	ClearPatch();
#endif

	char szDumpFileName[256];
	sprintf(szDumpFileName, "Gunz.dmp");
	__try{
#endif
	if (ZApplication::GetInstance()->ParseArguments(cmdline) == false)
	{
		// Korean or Japan Version
		if ((ZGetLocale()->GetCountry() == MC_KOREA) || (ZGetLocale()->GetCountry() == MC_JAPAN))
		{
			mlog("Routed to Website \n");

			ShellExecute(NULL, "open", ZGetConfiguration()->GetLocale()->szHomepageUrl, NULL, NULL, SW_SHOWNORMAL);

			char szMsgWarning[128]="";
			char szMsgCertFail[128]="";
			ZTransMsg(szMsgWarning,MSG_WARNING);
			ZTransMsg(szMsgCertFail,MSG_REROUTE_TO_WEBSITE);
//			MessageBox(g_hWnd, szMsgCertFail, szMsgWarning, MB_OK);

			mlog(szMsgWarning);
			mlog(" : ");
			mlog(szMsgCertFail);

			return 0;
		}
		else
		{
			return 0;
		}
	}

//#ifdef _PUBLISH
	// if(!CheckFileList()) {
		// Á¾·áÇÏ´Â°ÍÀº ÀÏ´Ü º¸·E
		// int ret=MessageBox(NULL, "ÆÄÀÏÀÌ ¼Õ»óµÇ¾ú½À´Ï´Ù.", "Áß¿E", MB_OK);
		// return 0;
	//}
//#endif

	// Custom: Disable NHN Auth
	/*
#ifdef LOCALE_NHNUSA
	BYTE SHA_HanAuthForClient[20] = {0x29,0xc0,0x7e,0x6b,0x8d,0x1d,0x30,0xd2,0xed,0xac,0xaf,0xea,0x78,0x16,0x51,0xf0,0x50,0x52,0x26,0x91};
	BYTE SHA_hanpollforclient[20] = {0x09,0x04,0x51,0x9d,0x95,0xbb,0x66,0x2a,0xfb,0x93,0x87,0x2d,0x21,0xa2,0x93,0x1d,0x6a,0xcb,0xa5,0x4f};
	BYTE SHA_HanReportForClient[20] = {0x4c,0x62,0xaf,0x4d,0x5b,0x54,0xb8,0x96,0x46,0x66,0x8f,0x1e,0x12,0xe7,0xf2,0xd7,0xe4,0x58,0x65,0xc9}; 
	if(!CheckDll("hanauthforclient.dll", SHA_HanAuthForClient) || 
	   !CheckDll("hanpollforclient.dll", SHA_hanpollforclient) ||
	   !CheckDll("hanreportforclient.dll", SHA_HanReportForClient) )
	{
		MessageBox(g_hWnd,"Dll Hacking detected",  NULL, MB_OK);
		return false;
	}

	if( !InitReport() ) 
		return 0;


	if ( !InitPoll())
		return 0;
#endif
	*/



#ifdef _EAC // Custom: Remove HASH DLL
	//BYTE SHA_basebin[20] = { 0xb1,0x91,0x94,0x78,0xfb,0x4a,0x65,0xd1,0x77,0x77,0x5a,0xf5,0x46,0x06,0x12,0x48,0xcd,0xa2,0x87,0x94 };
	BYTE SHA_basecer[20] = { 0x92,0xb9,0xf4,0x1b,0x63,0x31,0x3f,0xc5,0x6e,0x68,0x90,0x9d,0xe6,0x57,0xbb,0xe3,0x38,0xe2,0x05,0xa2 };
	BYTE SHA_runtime[20] = { 0x9b,0xf3,0xe1,0x71,0xf0,0x3f,0xea,0xdc,0x2a,0xf6,0xec,0x01,0x02,0xea,0xd0,0x16,0xd2,0xf6,0xed,0x48 };
	if(//!CheckDll("EasyAntiCheat\\Certificates\\base.bin", SHA_basebin) ||
		!CheckDll("EasyAntiCheat\\Certificates\\base.cer", SHA_basecer) ||
		!CheckDll("EasyAntiCheat\\Certificates\\runtime.conf", SHA_runtime))
	{
		MessageBox(g_hWnd,"Error validating EasyAntiCheat code signing certificate.", "Easy Anti-Cheat", MB_OK | MB_ICONERROR);
		return false;
	}

#endif

// Custom: Made hack check automatic (w/o debug) and enabled for LOCALE_NHNUSA
	if (ZCheckHackProcess() == true)
	{
		//		MessageBox(NULL,
		//			ZMsg(MSG_HACKING_DETECTED), ZMsg( MSG_WARNING), MB_OK);
	#ifdef _ANTIHACK
			mlog("EXIT: ZCheckHackProcess() == true at main.cpp startup check.\n");
			return 0;
	#else
			mlog("WARN: ZCheckHackProcess() == true at startup, but _ANTIHACK is off - continuing anyway.\n");
	#endif
	}



#ifdef _SHA1
#ifdef _THEMIDAGUNZ
	VM_START
#endif
	BYTE SHA_system[20] = { _SHA1 };
	if (!CheckDll("system.mrs", SHA_system))
	{
		//MessageBox(g_hWnd, "Please Update Client.", NULL, MB_OK);
		//return false;
		mlog("Error Initialization Client : Open Launcher to Update files.\n");
		//MessageBox(NULL, "The application could not start successfully (0xc012547b). Click OK to close it.", NULL, MB_OK | MB_ICONERROR);
		FatalExit(-1);
		return false;
	}
#ifdef _THEMIDAGUNZ
	VM_END
#endif
#endif
	if(!InitializeNotify(ZApplication::GetFileSystem())) 
	{
		MLog("Check notify.xml\n");
		return 0;
	}
	else 
	{
		mlog( "InitializeNotify ok.\n" );
	}



	if(CheckFont()==false) {
		MLog("ÆùÆ®°¡ ¾ø´Â À¯Àú°¡ ÆùÆ® ¼±ÅÃÀ» ÃEÒ\n");
		return 0;
	}

	RSetFunction(RF_CREATE	,	OnCreate);
	RSetFunction(RF_RENDER	,	OnRender);
	RSetFunction(RF_UPDATE	,	OnUpdate);
	RSetFunction(RF_DESTROY ,	OnDestroy);
	RSetFunction(RF_INVALIDATE,	OnInvalidate);
	RSetFunction(RF_RESTORE,	OnRestore);
	RSetFunction(RF_ACTIVATE,	OnActivate);
	RSetFunction(RF_DEACTIVATE,	OnDeActivate);
	RSetFunction(RF_ERROR,		OnError);

	SetModeParams();

//	while(ShowCursor(FALSE)>0);

	const int nRMainReturn = RMain(APPLICATION_NAME,this_inst,prev_inst,cmdline,cmdshow,&g_ModeParams,WndProc,IDI_ICON1);
	if( 0 != nRMainReturn )
		return nRMainReturn;


#ifdef _GAMEGUARD
	mlog("start gameguard\n");

	ZGMAEGUARD_MODE mode = ZGGM_END;
	char szArg[ 64] = "";
	ZBaseAuthInfo* pAuth = ZGetLocale()->GetAuthInfo();
	

#ifdef LOCALE_NHNUSA
	if ( ((ZNHN_USAAuthInfo*)pAuth)->IsReal())
	{
		mode = ZGGM_REAL;
		strcpy( szArg, "GunzUS");
	}
	else if ( ((ZNHN_USAAuthInfo*)pAuth)->IsAlpha())
	{
		mode = ZGGM_ALPHA;
		strcpy( szArg, "GunzUSTest");
	}
	else
	{
		mlog( "error in gameguard mode...\n" );
		zexit( -1);
		return 0;
	}

#elif LOCALE_JAPAN
	if ( ((ZGameOnJPAuthInfo*)pAuth)->IsReal())
	{
		mode = ZGGM_REAL;
		strcpy( szArg, "GUNZWEI");
	}
	else if ( ((ZGameOnJPAuthInfo*)pAuth)->IsAlpha())
	{
		mode = ZGGM_ALPHA;
		strcpy( szArg, "GUNZWEITest");
	}
	else
	{
		mlog( "error in gameguard mode...\n" );
		zexit( -1);
		return 0;
	}

	if( !((ZGameOnJPAuthInfo*)pAuth)->SendMsgToLauncher(GET_MSG_HWND_TERMINATE_PUBGAME) )
	{
		mlog("Can't find GameOn Agent");
	}
#endif	// LOCALE_JAPAN


	if( !GetZGameguard().Init( mode, szArg) )
	{
		mlog( "error init gameguard...\n" );
		zexit( -1 );
		return 0;
	}


	GetZGameguard().SetMainWnd( g_hWnd );

	if( !CheckGameGuardHackToolUser() )
		return 0;

#endif	// _GAMEGUARD


#ifdef _GAMEGUARD
	#ifdef _PUBLISH
		// Áßº¹ ½ÇÇE±ÝÁE
		Mutex = CreateMutex(NULL, TRUE, "Gunz");
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			zexit(-1);
			return 0;
		}
	#endif
#endif
#ifdef _INPUTFPS
	RSetUpdateLimitPerSecond(Z_ETC_INPUT);
#endif

#ifdef LOCALE_NHNUSA
	// Custom: Disable NHN related
	//GetNHNUSAReport().ReportInitGameGuard();
#endif

	if( 0 != RInitD3D(&g_ModeParams) )
	{
		MessageBox(g_hWnd, "fail to initialize DirectX", NULL, MB_OK);
		mlog( "error init RInitD3D\n" );
		return 0;
	}

	const int nRRunReturn = RRun();

	//Á¾·á½Ã °ÔÀÓ°¡µå°¡ XfireÀÇ ¸Þ¸ð¸®¾²±E¿¡·¯¸¦ À¯¹ßÇÏ´Âµ¥ ÀÌ¶§ ¿À·ùÃ¢ÀÌ Ç®½ºÅ©¸° µÚ¿¡ ¶ß´Â °Í ¹æÁöÇÏ±EÀ§ÇØ
	//Á¾·áÀE¡ °ÇÁûÔ¦ ÃÖ¼ÒÈ­/ºñÈ°¼ºÈ­ ½ÃÄÑ³õ´Â´Ù. xfireÀÇ ÁE¢ÀûÀÎ ¹®Á¦ ÇØ°áÀ» ±â´EÏ±E¾ûÓÁ¿EÇ·Î ÀÌ·¸°Ô Ã³¸®
	ShowWindow(g_hWnd, SW_MINIMIZE);

	g_bThreadChecker = false;
	//ahCleanup(43320);

#ifdef _GAMEGUARD
	GetZGameguard().Release();
#endif

#ifdef _MTRACEMEMORY
	MShutdownTraceMemory();
#endif

#ifdef _HSHIELD
	_AhnHS_StopService();
	_AhnHS_Uninitialize();		
#endif


#ifdef LOCALE_NHNUSA
	// Custom: Disable NHN poll
	//mlog( "Poll Process\n" );
	//int nRetPoll = GetNHNUSAPoll().ZHanPollProcess( NULL);
#endif

	ZStringResManager::FreeInstance();

	return nRRunReturn;

//	ShowCursor(TRUE);

#ifdef SUPPORT_EXCEPTIONHANDLING
	}

	//__except(MFilterException(GetExceptionInformation())){
	__except(CrashExceptionDump(GetExceptionInformation(), szDumpFileName, true))
	{
#ifdef LOCALE_NHNUSA
		// Custom: Disable NHN related
		//GetNHNUSAReport().ReportCrashedGame();
#endif

		HandleExceptionLog();
//		MessageBox(g_hWnd, "¿¹»óÄ¡ ¸øÇÑ ¿À·ù°¡ ¹ß»ýÇß½À´Ï´Ù.", APPLICATION_NAME , MB_ICONERROR|MB_OK);
	}
#endif

#ifdef _PUBLISH
	//if (Mutex != 0) CloseHandle(Mutex);
#endif

//	CoUninitialize();

	return 0;
}

#ifdef _HSHIELD
int __stdcall AhnHS_Callback(long lCode, long lParamSize, void* pParam)
{
//	TCHAR szTitle[256];

	switch(lCode)
	{
		//Engine Callback
	case AHNHS_ENGINE_DETECT_GAME_HACK:
		{
			TCHAR szMsg[255];
			wsprintf(szMsg, _T("´ÙÀ½ À§Ä¡¿¡¼­ ÇØÅ·ÅøÀÌ ¹ß°ßµÇ¾EÇÁ·Î±×·¥À» Á¾·á½ÃÄ×½À´Ï´Ù.\n%s"), (char*)pParam);
//			MessageBox(NULL, szMsg, szTitle, MB_OK);
			mlog(szMsg);
			PostThreadMessage(g_dwMainThreadID, WM_QUIT, 0, 0);
			break;
		}

		//ÀÏºÎ API°¡ ÀÌ¹Ì ÈÄÅ·µÇ¾EÀÖ´Â »óÅÂ
		//±×·¯³ª ½ÇÁ¦·Î´Â ÀÌ¸¦ Â÷´ÜÇÏ°EÀÖ±E¶§¹®¿¡ ´Ù¸¥ ÈÄÅ·½Ãµµ ÇÁ·Î±×·¥Àº µ¿ÀÛÇÏÁE¾Ê½À´Ï´Ù.
		//ÀÌ CallbackÀº ´ÜÁE°æ°E³»Áö´Â Á¤º¸Á¦°EÂ÷¿ø¿¡¼­ Á¦°øµÇ¹Ç·Î °ÔÀÓÀ» Á¾·áÇÒ ÇÊ¿ä°¡ ¾ø½À´Ï´Ù.
	case AHNHS_ACTAPC_DETECT_ALREADYHOOKED:
		{
			PACTAPCPARAM_DETECT_HOOKFUNCTION pHookFunction = (PACTAPCPARAM_DETECT_HOOKFUNCTION)pParam;
			TCHAR szMsg[255];
			wsprintf(szMsg, _T("[HACKSHIELD] Already Hooked\n- szFunctionName : %s\n- szModuleName : %s\n"), 
				pHookFunction->szFunctionName, pHookFunction->szModuleName);
			OutputDebugString(szMsg);
			break;
		}

		//Speed °EÃ
	case AHNHS_ACTAPC_DETECT_SPEEDHACK:
	case AHNHS_ACTAPC_DETECT_SPEEDHACK_APP:
		{
//			MessageBox(NULL, _T("ÇöÀEÀÌ PC¿¡¼­ SpeedHackÀ¸·Î ÀÇ½ÉµÇ´Â µ¿ÀÛÀÌ °¨ÁöµÇ¾ú½À´Ï´Ù."), szTitle, MB_OK);
			mlog("ÇöÀEÀÌ PC¿¡¼­ SpeedHackÀ¸·Î ÀÇ½ÉµÇ´Â µ¿ÀÛÀÌ °¨ÁöµÇ¾ú½À´Ï´Ù.");
			PostThreadMessage(g_dwMainThreadID, WM_QUIT, 0, 0);
			break;
		}

		//µð¹ö±E¹æÁE
	case AHNHS_ACTAPC_DETECT_KDTRACE:	
	case AHNHS_ACTAPC_DETECT_KDTRACE_CHANGED:
		{
			TCHAR szMsg[255];
			wsprintf(szMsg, _T("ÇÁ·Î±×·¥¿¡ ´EÏ¿© µð¹ö±E½Ãµµ°¡ ¹ß»ýÇÏ¿´½À´Ï´Ù. (Code = %x)\nÇÁ·Î±×·¥À» Á¾·áÇÕ´Ï´Ù."), lCode);
//			MessageBox(NULL, szMsg, szTitle, MB_OK);
			mlog(szMsg);
			PostThreadMessage(g_dwMainThreadID, WM_QUIT, 0, 0);
			break;
		}

		//±×¿Ü ÇØÅ· ¹æÁE±â´É ÀÌ»E
	case AHNHS_ACTAPC_DETECT_AUTOMOUSE:
	case AHNHS_ACTAPC_DETECT_DRIVERFAILED:
	case AHNHS_ACTAPC_DETECT_HOOKFUNCTION:
	case AHNHS_ACTAPC_DETECT_MESSAGEHOOK:
	case AHNHS_ACTAPC_DETECT_MODULE_CHANGE:
		{
			TCHAR szMsg[255];
			wsprintf(szMsg, _T("ÇØÅ· ¹æ¾E±â´É¿¡ ÀÌ»óÀÌ ¹ß»ýÇÏ¿´½À´Ï´Ù. (Code = %x)\nÇÁ·Î±×·¥À» Á¾·áÇÕ´Ï´Ù."), lCode);
//			MessageBox(NULL, szMsg, szTitle, MB_OK);
			mlog(szMsg);
			PostThreadMessage(g_dwMainThreadID, WM_QUIT, 0, 0);
			break;
		}
	}

	return 1;
}
#endif

void drawDebugText(int x, int y, const char * str)
{
    MFontR2* pFont = (MFontR2*)MFontManager::Get(NULL);
    pFont->m_Font.BeginFont();
    pFont->m_Font.DrawText(x, y, str);
    pFont->m_Font.EndFont();  
}
