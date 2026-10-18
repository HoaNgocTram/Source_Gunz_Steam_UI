#pragma once
#include "stdafx.h"
#include <string>
#include <windows.h>

// 1. Biến toàn cục (dùng extern để ZGameInterface truy cập được nếu cần)
extern HWND g_hWebWnd;

// 2. Các hàm Core điều khiển WebView2 Window
void ZEmbedWebPageToWidget(const char* url, int x, int y, int width, int height);
void ZHideWebWidget();

// 3. Các hàm Helper tạo Token & URL
std::string UrlEncode(const std::string& value);
std::string MakeCharNameToken(const std::string& name);
const char* GetMyCharName();

// 4. Các hàm mở tính năng Webview (đã có tham số vị trí x, y, w, h)
void OpenGachaInWebview(int x, int y, int w, int h);
void OpenBattlePassInWebview(int x, int y, int w, int h);
void OpenMarketInWebview(int x, int y, int w, int h);
void OpenLadderRewardInWebview(int x, int y, int w, int h);