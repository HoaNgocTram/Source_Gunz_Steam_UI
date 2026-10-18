#include "stdafx.h"
#include <windows.h>
#include "MDebug.h"
#include "RealSpace2.h"
#include "RParticleSystem.h"
#include "RFont.h"
#include "RMeshUtil.h"
#include <imm.h>
#pragma comment(lib, "imm32.lib")
#include "../Mint2/Include/MWidget.h"
#include "../Mint2/Include/MEdit.h"
#define WM_MINT_IMECOMPOSE 0xBF00
#pragma comment(lib,"winmm.lib")

// BGM background play - set from game code
extern bool g_bPlayMusicInBackground;

#define RTOOLTIP_GAP 700
static DWORD g_last_mouse_move_time = 0;
static bool g_tool_tip = false;

#ifdef _INPUTFPS
static double lastUpdateTime;
static double lastRenderTime;
#endif

bool IsToolTipEnable() {
	return g_tool_tip;
}

_NAMESPACE_REALSPACE2_BEGIN

//RMODEPARAMS g_ModeParams={ 640,480,false,RPIXELFORMAT_565 };

extern HWND g_hWnd;

bool g_bActive;
extern bool g_bFixedResolution;

RECT g_rcWindowBounds;
WNDPROC	g_WinProc = NULL;
RFFUNCTION g_pFunctions[RF_ENDOFRFUNCTIONTYPE] = { NULL, };
//LPD3DXFONT g_lpFont=NULL;

//extern LPDIRECT3DTEXTURE9 g_lpTexture ;
//extern LPDIRECT3DSURFACE9 g_lpSurface ;

extern int gNumTrisRendered;

#ifdef _USE_GDIPLUS		// GDI Plus 
#include "unknwn.h"
#include "gdiplus.h"

Gdiplus::GdiplusStartupInput	g_gdiplusStartupInput;
ULONG_PTR 						g_gdiplusToken = NULL;
#endif

void RSetFunction(RFUNCTIONTYPE ft, RFFUNCTION pfunc)
{
	g_pFunctions[ft] = pfunc;
}

bool RIsActive()
{
	return GetActiveWindow() == g_hWnd;
	//	return g_bActive;
}

void RFrame_Create()
{
#ifdef _USE_GDIPLUS
	Gdiplus::GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, NULL);
#endif
	GetWindowRect(g_hWnd, &g_rcWindowBounds);
}

/*
void RFrame_InitFont()
{
	LOGFONT lFont;

	ZeroMemory(&lFont, sizeof(LOGFONT));
	lFont.lfHeight = 16;
	lFont.lfWidth = 0;
	lFont.lfWeight = FW_BOLD;
	lFont.lfCharSet = SHIFTJIS_CHARSET;
	lFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lFont.lfQuality = PROOF_QUALITY;
	lFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
	strcpy(lFont.lfFaceName, "FONTa9");

	SAFE_RELEASE(g_lpFont);
	HRESULT hr=D3DXCreateFontIndirect(RGetDevice(), &lFont, &g_lpFont);
	_ASSERT(hr==D3D_OK);
	mlog("font restored.\n");
}
*/

void RFrame_Init()
{
	//	RFrame_InitFont();
}

void RFrame_Restore()
{
	//if( IsFixedResolution() )
	//	FixedResolutionRenderStart();

//	RFrame_InitFont();
	RParticleSystem::Restore();
	if (g_pFunctions[RF_RESTORE])
		g_pFunctions[RF_RESTORE](NULL);
}

void RFrame_Destroy()
{
	//if( IsFixedResolution() )
	//{
	//	FixedResolutionRenderEnd();
	//	FixedResolutionRenderInvalidate();
	//}

//	SAFE_RELEASE(g_lpFont);
	if (g_pFunctions[RF_DESTROY])
		g_pFunctions[RF_DESTROY](NULL);

	mlog("Rframe_destory::closeDisplay\n");
	RCloseDisplay();

#ifdef _USE_GDIPLUS
	Gdiplus::GdiplusShutdown(g_gdiplusToken);
#endif
}

void RFrame_Invalidate()
{
	//if( IsFixedResolution() )
	//{
	//	FixedResolutionRenderEnd();
	//	FixedResolutionRenderInvalidate();
	//}
//	SAFE_RELEASE(g_lpFont);

//	RFontTexture::m_dwStateBlock=NULL;
	RGetShaderMgr()->Release();
	RParticleSystem::Invalidate();
	if (g_pFunctions[RF_INVALIDATE])
		g_pFunctions[RF_INVALIDATE](NULL);
}

void RFrame_Update()
{
	if (g_pFunctions[RF_UPDATE])
		g_pFunctions[RF_UPDATE](NULL);
}


void RFrame_Render()
{
	if (!RIsActive() && RGetScreenType() == 0) return;

	RRESULT isOK = RIsReadyToRender();
	if (isOK == R_NOTREADY)
		return;
	else
		if (isOK == R_RESTORED)
		{
			RMODEPARAMS ModeParams = { RGetScreenWidth(),RGetScreenHeight(),RGetScreenType(),RGetPixelFormat() };
			RResetDevice(&ModeParams);
			mlog("devices Restored. \n");
		}

	if (timeGetTime() > g_last_mouse_move_time + RTOOLTIP_GAP)
		g_tool_tip = true;

	if (g_pFunctions[RF_RENDER])
		g_pFunctions[RF_RENDER](NULL);

	RGetDevice()->SetStreamSource(0, NULL, 0, 0);
	RGetDevice()->SetIndices(0);
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);


	//	Draw FPS

	//	60fps °¡ 100Á¡

		/*
		char buf[256];
		float fMs = 1000.f/g_fFPS;
		float fScore = 100-(fMs-(1000.f/60.f))*2;

		sprintf(buf, "FPS : %3.3f , %d triangles, %4.1f ms,score %4.1f Á¡",g_fFPS,gNumTrisRendered,fMs,fScore);
		RECT drawRect;
		SetRect(&drawRect, 0, 0, RGetScreenWidth(), RGetScreenHeight());
		g_lpFont->DrawText(buf, -1, &drawRect, DT_LEFT | DT_TOP, D3DCOLOR_RGBA(255, 255, 255, 255));
	//
	*/


	/*
		for(int i=0;i<MGetLogHistoryCount();i++)
		{
			drawRect.top=(i+1)*20;
			g_lpFont->DrawText(MGetLogHistory(i), -1, &drawRect, DT_LEFT | DT_TOP, D3DCOLOR_RGBA(255, 255, 255, 255));
		}

	*/
}

void RFrame_ToggleFullScreen()
{
	RMODEPARAMS ModeParams = { RGetScreenWidth(),RGetScreenHeight(),RGetScreenType(),RGetPixelFormat() };

	if (ModeParams.nScreenType > 0)									// À©µµ¿ì -> Ç®½ºÅ©¸°ÀÏ¶§ ÀúÀåÇÏ°í..
		GetWindowRect(g_hWnd, &g_rcWindowBounds);
	if (ModeParams.nScreenType == 0)
		ModeParams.nScreenType = 1;
	else if (ModeParams.nScreenType == 1)
		ModeParams.nScreenType = 0;
	else if (ModeParams.nScreenType == 2)
		ModeParams.nScreenType = 0;
	RResetDevice(&ModeParams);

	if (ModeParams.nScreenType > 0)									// Ç®½ºÅ©¸° -> À©µµ¿ì·Î °¥¶§ º¹±¸ÇÑ´Ù.
	{
		// Custom: Window style fix
		//SetWindowLong( g_hWnd, GWL_STYLE, WS_POPUP | WS_CAPTION | WS_SYSMENU );

		SetWindowPos(g_hWnd, HWND_NOTOPMOST,
			g_rcWindowBounds.left, g_rcWindowBounds.top,
			(g_rcWindowBounds.right - g_rcWindowBounds.left),
			(g_rcWindowBounds.bottom - g_rcWindowBounds.top),
			SWP_SHOWWINDOW /*| SWP_FRAMECHANGED*/); // added frame change
	}
	else
		SetWindowLong(g_hWnd, GWL_STYLE, WS_POPUP | WS_SYSMENU);
}

class MWidgetHelper : public MWidget {
public:
	static MWidget* GetFocusedWidget() { return m_pFocusedWidget; }
};
// Custom: Changed ancient definition of RealSpace2 init
//long FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// 🚀 ĐÓN ĐẦU UNICODE VÀ ĐỔI SANG UTF-8 CHUẨN ĐÉT NHƯ COPY-PASTE
	if (message == WM_IME_COMPOSITION)
	{
		if (lParam & GCS_RESULTSTR) // Khi người chơi chốt chữ từ bảng gợi ý
		{
			HIMC hIMC = ImmGetContext(hWnd);
			if (hIMC)
			{
				// 1. Lấy độ dài tính bằng BYTE của chuỗi UTF-16 trong RAM IME
				LONG nByteLen = ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, NULL, 0);
				if (nByteLen > 0)
				{
					// 2. Cấp phát mảng động theo đúng số byte hệ thống trả về
					int nChars = nByteLen / 2;
					wchar_t* wszRes = new wchar_t[nChars + 1];

					// Nạp chính xác chuỗi Wide-Char (Unicode gốc) vào mảng
					ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, wszRes, nByteLen);
					wszRes[nChars] = L'\0'; // Chốt hạ chuỗi Unicode

					// 3. CHUYỂN ĐỔI SANG UTF-8 (Cơ chế giống hệt Windows xử lý Clipboard khi Paste)
					// Lấy trước độ dài cần thiết cho chuỗi UTF-8
					int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0, wszRes, -1, NULL, 0, NULL, NULL);
					if (nUtf8Len > 0)
					{
						char* szUtf8Result = new char[nUtf8Len + 1];
						WideCharToMultiByte(CP_UTF8, 0, wszRes, -1, szUtf8Result, nUtf8Len, NULL, NULL);
						szUtf8Result[nUtf8Len] = '\0';

						// 4. Lấy ô nhập liệu đang Focus và bơm chuỗi UTF-8 sạch vào
						MWidget* pFocused = MWidgetHelper::GetFocusedWidget();
						if (pFocused != NULL && (strcmp(pFocused->GetClassName(), "Edit") == 0 || strcmp(pFocused->GetClassName(), "MEdit") == 0))
						{
							MEdit* pEdit = (MEdit*)pFocused;

							// Bơm thẳng chuỗi UTF-8 sạch vào ô chat, bỏ qua bộ lọc phím đơn byte của WndProc
							pEdit->AddText(szUtf8Result);

							// Bắn lệnh ép UI của GunZ render cập nhật lại khung chat
							MListener* pListener = pEdit->GetListener();
							if (pListener != NULL) {
								pListener->OnCommand(pEdit, "changed");
							}
						}
						delete[] szUtf8Result;
					}
					delete[] wszRes;
				}
				ImmReleaseContext(hWnd, hIMC);
			}
			return 0; // Chặn đứng hoàn toàn tin nhắn gốc của Windows để không bị sinh dấu '?' hay ô vuông rác
		}
	}

	// Nếu Windows cố tình bắn tin nhắn WM_CHAR rác (dấu '?') vào khi đang mở IME, đấm chết luôn tại cửa!
	//if (message == WM_CHAR && ImmIsIME(GetKeyboardLayout(0)))
	//{
	//	//if (wParam == '?') return 0;
	//}

	// Handle messages
	switch (message)
	{

	case WM_INPUTLANGCHANGE:
	{
		HIMC hIMC = ImmGetContext(hWnd);
		if (hIMC) {
			ImmAssociateContext(hWnd, hIMC);
			ImmReleaseContext(hWnd, hIMC);
		}
		break; // Cứ để nó chạy tiếp xuống g_WinProc bên dưới
	}

#ifndef _PUBLISH
	case WM_SYSCHAR:
		if (wParam == VK_RETURN)
			RFrame_ToggleFullScreen();
		return 0;
#endif
	case WM_SYSCOMMAND:
	{
		switch (wParam)
		{
			// Trap ALT so it doesn't pause the app
		case SC_PREVWINDOW:
		case SC_NEXTWINDOW:
		case SC_KEYMENU:
		{
			return 0;
		}
		break;
		}
	}
	break;

	case WM_ACTIVATEAPP:
	{
		if (wParam == TRUE) {
			if (g_pFunctions[RF_ACTIVATE])
				g_pFunctions[RF_ACTIVATE](NULL);
			g_bActive = TRUE;
		}
		else {
			if (g_pFunctions[RF_DEACTIVATE])
				g_pFunctions[RF_DEACTIVATE](NULL);
			if (RGetScreenType() == 0 && !g_bPlayMusicInBackground) {
				ShowWindow(hWnd, SW_MINIMIZE);
				UpdateWindow(hWnd);
			}
			g_bActive = FALSE;
		}
	}
	break;

	case WM_MOUSEMOVE:
	{
		g_last_mouse_move_time = timeGetTime();
		g_tool_tip = false;
	}
	break;

	case WM_CLOSE:
	{
		RFrame_Destroy();
		PostQuitMessage(0);
		return 0;
	}
	break;
	}
	return g_WinProc(hWnd, message, wParam, lParam);
}

#ifndef _PUBLISH

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

int RMain(const char* AppName, HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow, RMODEPARAMS* pModeParams, WNDPROC winproc, WORD nIconResID)
{
	g_WinProc = winproc ? winproc : DefWindowProc;

	// make a window
	WNDCLASS    wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(DWORD);
	wc.hInstance = this_inst;
	wc.hIcon = LoadIcon(this_inst, MAKEINTRESOURCE(nIconResID));
	wc.hCursor = 0;//LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "RealSpace2";
	if (!RegisterClass(&wc)) return FALSE;
	DWORD dwStyle;
	if (pModeParams->nScreenType == 0)
		dwStyle = WS_POPUP | WS_SYSMENU;
	else if (pModeParams->nScreenType == 1)
		dwStyle = WS_POPUP | WS_CAPTION | WS_SYSMENU;
	else
		dwStyle = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
	if (pModeParams->nScreenType != 2)
	{
		g_hWnd = CreateWindow("RealSpace2", AppName, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
			pModeParams->nWidth, pModeParams->nHeight, NULL, NULL, this_inst, NULL);
	}
	else
	{
		g_hWnd = CreateWindow("RealSpace2", AppName, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
			pModeParams->nWidth, pModeParams->nHeight - getTaskBarHeight() - GetSystemMetrics(SM_CYBORDER), NULL, NULL, this_inst, NULL);
	}

	// initialize realspace2

	RAdjustWindow(pModeParams);

	while (ShowCursor(FALSE) > 0);
	//	ShowCursor(TRUE);	// RAONHAJE Mouse Cursor HardwareDraw

	//	RFrame_Create();
	//
	//	ShowWindow(g_hWnd,SW_SHOW);
	//	if(!RInitDisplay(g_hWnd,pModeParams))
	//	{
	//		mlog("can't init display\n");
	//		return -1;
	//	}
	//
	//	//RBeginScene();
	//	RGetDevice()->Clear(0 , NULL, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0 );
	////	REndScene();
	//	RFlip();


	//	RGetDevice()->ShowCursor( TRUE );	// RAONHAJE Mouse Cursor HardwareDraw

	return 0;
}
#ifdef _INPUTFPS
using namespace std::chrono;
void RFrame_UpdateRender(double& lastUpdateTime, double& lastRenderTime)
{
	double thisTime = duration<double, std::ratio<1, 1000>>(high_resolution_clock::now().time_since_epoch()).count();

	if (g_nUpdateLimitValue != 0)
	{
		const double maxUpdatePeriod = 1000.0 / static_cast<double>(g_nUpdateLimitValue);
		double  deltaTime = duration<double>(thisTime - lastUpdateTime).count();

		if (deltaTime >= maxUpdatePeriod)
		{
			RFrame_Update();
			lastUpdateTime = duration<double, std::ratio<1, 1000>>(high_resolution_clock::now().time_since_epoch()).count();
		}
	}
	else
	{
		RFrame_Update();
	}

	if (g_nFrameLimitValue != 0)
	{

		thisTime = duration<double, std::ratio<1, 1000>>(high_resolution_clock::now().time_since_epoch()).count();
		const double maxRenderPeriod = 1000.0 / static_cast<double>(g_nFrameLimitValue);
		double deltaTime = duration<double>(thisTime - lastRenderTime).count();

		if (deltaTime >= maxRenderPeriod)
		{
			g_fFPS = deltaTime;
			lastRenderTime = duration<double, std::ratio<1, 1000>>(high_resolution_clock::now().time_since_epoch()).count();

			RFrame_Render();
			if (!RFlip())
			{
				RIsReadyToRender();
			}
		}
	}
	else
	{
		RFrame_Render();
		if (!RFlip())
		{
			RIsReadyToRender();
		}
	}
}
#endif
void RFrame_UpdateInput()
{
	if (g_pFunctions[RF_UPDATEINPUT])
		g_pFunctions[RF_UPDATEINPUT](NULL);
}

void RFrame_UpdateRender()
{
	__BP(5006, "RMain::Run");

	RFrame_Update();
	RFrame_Render();

	__BP(5007, "RMain::RFlip");
	RFlip();
	__EP(5007);

	__EP(5006);
}

int RRun()
{
	if (g_pFunctions[RF_CREATE])
	{
		if (g_pFunctions[RF_CREATE](NULL) != R_OK)
		{
			RFrame_Destroy();
			return -1;
		}
	}

	RFrame_Init();

	// message loop
	// Now we're ready to recieve and process Windows messages.
	BOOL bGotMsg;
	MSG  msg;
	DWORD dwLastFrame = 0;
	//    PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );

	do
	{
		// Use PeekMessage() if the app is active, so we can use idle time to
		// render the scene. Else, use GetMessage() to avoid eating CPU time.
//        if( g_bActive )
		bGotMsg = PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE);
		//        else
		//            bGotMsg = GetMessage( &msg, NULL, 0U, 0U );

		if (bGotMsg)
		{
			// WM_USER ÀÌ»óÀÇ ¸Þ½ÃÁö´Â Ã³¸®ÇÏÁö ¾Ê´Â´Ù. ÀÌ°ÍÀº ÀÏº»¾î IMEÀÇ ÆË¾÷¸Þ´º È£ÃâÀ» ¸·±â À§ÇÑ Ã³¸®´Ù.
			// (ÆË¾÷¸Þ´º°¡ ÄÚµåÁøÇàÀ» ¸ØÃß°ÔÇÏ´Â Æ¯¼ºÀ» À¯ÀúµéÀÌ ¹«Àû¾îºäÁî·Î ¾Ç¿ëÇÏ±â ¶§¹®)
			// WM_USER+25´Â °ÇÁî³»ºÎ¿¡¼­ »ç¿ëÇÏ°í ÀÖÀ¸¹Ç·Î WM_USER+25°¡ ³Ñ¾î°£ ¸Þ½ÃÁöºÎÅÍ´Â ¹ö¸°´Ù.
			if (msg.message <= WM_USER + 25)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			RFrame_UpdateRender();
		}

		if (!g_bActive)
			Sleep(10);
	} while (WM_QUIT != msg.message);
	return (INT)msg.wParam;
}


int RInitD3D(RMODEPARAMS* pModeParams)
{
	RFrame_Create();

	// 1. Khởi tạo Display trước (bao gồm cả việc tạo Device và căn chỉnh cửa sổ)
	if (!RInitDisplay(g_hWnd, pModeParams))
	{
		mlog("can't init display\n");
		return -1;
	}

	// 2. Sau khi mọi thứ đã sẵn sàng (đúng kích thước, đúng vị trí), mới hiện cửa sổ lên
	// Lưu ý: Trong hàm RInitDisplay cuối cùng mình viết đã có ShowWindow rồi, 
	// nếu ở đó có rồi thì ở đây bạn có thể bỏ qua hoặc giữ lại để chắc chắn.
	ShowWindow(g_hWnd, SW_SHOW);
	UpdateWindow(g_hWnd);

	// 3. Xóa màn hình lần đầu để tránh rác hình ảnh (Black screen initial)
	if (RGetDevice())
	{
		RGetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0x00000000, 1.0f, 0);
		// Có thể thực hiện RFlip() ở đây để đẩy frame đen này lên màn hình ngay
		RFlip();
	}

	return 0;
}

_NAMESPACE_REALSPACE2_END
