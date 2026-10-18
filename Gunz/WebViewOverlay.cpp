#include "stdafx.h"
#include <windows.h>
#include <wrl.h>
#include <WebView2.h>
#include <string>
#include <shlobj.h>
#pragma comment(lib, "shell32.lib")

using namespace Microsoft::WRL;

HWND g_hWebWnd = nullptr;
ComPtr<ICoreWebView2Controller> g_webController;
ComPtr<ICoreWebView2> g_webView;

std::wstring ToWide(const char* str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    std::wstring wstr(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str, -1, &wstr[0], len);
    return wstr;
}

// Thêm biến lưu vị trí Widget
LRESULT CALLBACK WebProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SETCURSOR:
    {
        // Bắt buộc Set Cursor Mũi tên chuẩn Win32
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        while (ShowCursor(TRUE) < 0);
        return TRUE; // Trả về TRUE để báo Windows không chuyển giao cho Parent Handle
    }

    case WM_MOUSEACTIVATE:
    {
        // Khi nhấp chuột vào WebView, kích hoạt Focus Win32 ngay lập tức
        return MA_ACTIVATE;
    }

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    {
        // Cho phép gán Focus vào WebView2 để thao tác phím/chuột trên Web
        SetFocus(hWnd);
        if (g_webController)
        {
            g_webController->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
        }
        break;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_DESTROY:
    {
        if (g_webController) g_webController->Close();
        g_webController = nullptr;
        g_webView = nullptr;
        g_hWebWnd = nullptr;
        return 0;
    }
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Hàm khởi tạo WebView2 gắn trực tiếp vào Widget
void ZEmbedWebPageToWidget(const char* url, int x, int y, int width, int height)
{
    HWND hGunzMainWnd = g_hWnd;

    if (g_hWebWnd && IsWindow(g_hWebWnd))
    {
        // Ép Z-Order luôn nằm trên cùng (TOPMOST) của DirectX Surface
        SetWindowPos(g_hWebWnd, HWND_TOPMOST, x, y, width, height, SWP_SHOWWINDOW | SWP_NOACTIVATE);
        ShowWindow(g_hWebWnd, SW_SHOW);

        if (g_webController) {
            RECT bounds = { 0, 0, width, height };
            g_webController->put_Bounds(bounds);
        }
        if (g_webView) {
            g_webView->Navigate(ToWide(url).c_str());
        }
        return;
    }

    HINSTANCE hInst = GetModuleHandle(NULL);
    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WebProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "GUNZ_WEBVIEW_WIDGET";
    RegisterClassA(&wc);

    // THÊM WS_EX_TOPMOST ở tham số đầu tiên để đè lên Fullscreen
    g_hWebWnd = CreateWindowExA(
        WS_EX_TOPMOST,
        wc.lpszClassName,
        "",
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
        x, y, width, height,
        hGunzMainWnd,
        NULL, hInst, NULL);

    if (!g_hWebWnd) return;

    // Thiết lập Cache WebView2
    char szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)))
    {
        strcat_s(szPath, "\\GunzTheDuel\\WebView2_Cache");
        CreateDirectoryA(szPath, NULL);
    }

    int nLen = MultiByteToWideChar(CP_ACP, 0, szPath, -1, NULL, 0);
    std::wstring wCachePath(nLen, L'\0');
    MultiByteToWideChar(CP_ACP, 0, szPath, -1, &wCachePath[0], nLen);

    std::wstring wurl = ToWide(url);

    CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        wCachePath.c_str(),
        nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [wurl, width, height](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
            {
                if (!env) return E_FAIL;
                env->CreateCoreWebView2Controller(g_hWebWnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [wurl, width, height](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                        {
                            if (!controller) return E_FAIL;

                            g_webController = controller;
                            g_webController->get_CoreWebView2(&g_webView);

                            // -----------------------------------------------------------------
                            // TẬP TRUNG CHÚ Ý: ĐẢM BẢO MÁY TRẠM XỬ LÝ CURSOR CHUẨN TRÊN WEBVIEW2
                            // -----------------------------------------------------------------
                            // 1. Ép Controller nhận Focus & Mouse Capture chuẩn từ Win32
                            g_webController->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);

                            // 2. Ép hiển thị Cửa sổ Controller
                            g_webController->put_IsVisible(TRUE);

                            ComPtr<ICoreWebView2Settings> settings;
                            if (SUCCEEDED(g_webView->get_Settings(&settings)))
                            {
                                settings->put_AreDefaultContextMenusEnabled(FALSE);
                                settings->put_AreDevToolsEnabled(FALSE);
                                // Cho phép JavaScript thao tác trỏ chuột nếu cần
                                settings->put_IsScriptEnabled(TRUE);
                            }

                            RECT bounds = { 0, 0, width, height };
                            g_webController->put_Bounds(bounds);

                            g_webView->Navigate(wurl.c_str());
                            return S_OK;
                        }).Get());
                return S_OK;
            }).Get());
}

// Hàm ẩn WebView khi đóng Widget hoặc chuyển Tab
void ZHideWebWidget()
{
    if (g_hWebWnd)
    {
        ShowWindow(g_hWebWnd, SW_HIDE);
    }
}