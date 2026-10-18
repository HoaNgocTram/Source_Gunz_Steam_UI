#include "stdafx.h"
#include "MEvent.h"
#include <zmouse.h>
#include <sstream>  // <-- Thêm dòng này
#include <iomanip>
#include "Mint.h"

MPOINT MEvent::LatestPos = MPOINT(0, 0);
bool MEvent::bIMESupport = true;
bool MEvent::bLButton = false;
bool MEvent::bMButton = false;
bool MEvent::bRButton = false;

// Khai báo thư viện IME hệ thống Windows
#pragma comment(lib, "imm32.lib")

bool MEvent::GetShiftState(void)
{
	if ((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000) return true;
	return false;
}

bool MEvent::GetCtrlState(void)
{
	if ((GetKeyState(VK_CONTROL) & 0x8000) == 0x8000) return true;
	return false;
}

bool MEvent::GetAltState(void)
{
	if ((GetKeyState(VK_MENU) & 0x8000) == 0x8000) return true;
	return false;
}

bool MEvent::IsKeyDown(int key)
{
	return ((GetAsyncKeyState(key) & 0x8000) != 0);
}

bool MEvent::GetLButtonState()
{
	return bLButton;
}

bool MEvent::GetRButtonState()
{
	return bRButton;
}

bool MEvent::GetMButtonState()
{
	return bMButton;
}

MPOINT MEvent::GetMousePos(void)
{
	POINT p;
	GetCursorPos(&p);
	return MPOINT(p.x, p.y);
}

#include "MDebug.h"

void MEvent::ForceSetIME(DWORD fdwConversion, DWORD fdwSentence)
{
	HWND hWnd = Mint::GetInstance()->GetHWND();
	HIMC hImc = ImmGetContext(hWnd);
	if (hImc)
	{
		// Cải tiến: Kích hoạt luôn cờ Native và FullShape để mở rộng bảng mã cho IME quốc tế khi ép trạng thái
		fdwConversion |= IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE;
		ImmSetConversionStatus(hImc, fdwConversion, fdwSentence);
		ImmReleaseContext(hWnd, hImc);
	}
}

int MEvent::TranslateEvent(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	bCtrl = GetCtrlState();

	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_MOUSEMOVE:
		Pos.x = LOWORD(lparam);
		Pos.y = HIWORD(lparam);
		LatestPos = Pos;
		break;
	case WM_MOUSEWHEEL:
	{
		POINT pos;
		pos.x = LOWORD(lparam);
		pos.y = HIWORD(lparam);
		ScreenToClient(Mint::GetInstance()->GetHWND(), &pos);
		Pos.x = pos.x;
		Pos.y = pos.y;
		LatestPos = Pos;
	}
	break;
	}

	switch (message) {
	case WM_LBUTTONDOWN:
		nMessage = MWM_LBUTTONDOWN;
		bLButton = true;
		return EVENT_MINT_TRANSLATED;
	case WM_LBUTTONUP:
		bLButton = false;
		nMessage = MWM_LBUTTONUP;
		return EVENT_MINT_TRANSLATED;
	case WM_RBUTTONDOWN:
		bRButton = true;
		nMessage = MWM_RBUTTONDOWN;
		return EVENT_MINT_TRANSLATED;
	case WM_RBUTTONUP:
		bRButton = false;
		nMessage = MWM_RBUTTONUP;
		return EVENT_MINT_TRANSLATED;
	case WM_MBUTTONDOWN:
		bMButton = true;
		nMessage = MWM_MBUTTONDOWN;
		return EVENT_MINT_TRANSLATED;
	case WM_MBUTTONUP:
		bMButton = false;
		nMessage = MWM_MBUTTONUP;
		return EVENT_MINT_TRANSLATED;
	case WM_LBUTTONDBLCLK:
		nMessage = MWM_LBUTTONDBLCLK;
		bLButton = true;
		return EVENT_MINT_TRANSLATED;
	case WM_RBUTTONDBLCLK:
		nMessage = MWM_RBUTTONDBLCLK;
		bRButton = true;
		return EVENT_MINT_TRANSLATED;
	case WM_MBUTTONDBLCLK:
		nMessage = MWM_LBUTTONDBLCLK; // Sửa lỗi gõ nhầm gốc MWM_LBUTTONDBLCLK cho chuột giữa
		bLButton = true;
		return EVENT_MINT_TRANSLATED;
	case WM_MOUSEMOVE:
		nMessage = MWM_MOUSEMOVE;
		return EVENT_MINT_TRANSLATED;
	case WM_MOUSEWHEEL:
		nMessage = MWM_MOUSEWHEEL;
		nDelta = (short)HIWORD(wparam);
		return EVENT_MINT_TRANSLATED;

	case WM_KEYDOWN:
		nMessage = MWM_KEYDOWN;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;
	case WM_KEYUP:
		nMessage = MWM_KEYUP;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;
	case WM_SYSCHAR:
		nMessage = MWM_SYSCHAR;
		nKey = wparam;
		bAlt = true;
		return EVENT_MINT_TRANSLATED;
	case WM_SYSKEYDOWN:
		nMessage = MWM_SYSKEYDOWN;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;
	case WM_SYSKEYUP:
		nMessage = MWM_SYSKEYUP;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;

	case WM_CHAR:
		nMessage = MWM_CHAR;
		nKey = (unsigned long)wparam;
		return EVENT_MINT_TRANSLATED;

	case WM_HOTKEY:
		nMessage = MWM_HOTKEY;
		nKey = wparam;
		return EVENT_MINT_TRANSLATED;

	case WM_INPUTLANGCHANGE:
	{
		// Giải phóng context cũ, ép tạo ngữ cảnh mới tiếp nhận bộ gõ
		ImmAssociateContext(hwnd, ImmCreateContext());
		HIMC hIMC = ImmGetContext(hwnd);
		if (hIMC) {
			DWORD dwConv, dwSent;
			ImmGetConversionStatus(hIMC, &dwConv, &dwSent);
			// Ép mở chế độ gõ chữ native, không cho kẹt ở chế độ chỉ gõ số/alphanumeric
			ImmSetConversionStatus(hIMC, dwConv | IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE, dwSent);
			ImmReleaseContext(hwnd, hIMC);
		}
		return (EVENT_PROCESSED | EVENT_MINT_TRANSLATED);
	}

	case WM_IME_STARTCOMPOSITION:
		if (bIMESupport == true) {
			szIMECompositionString[0] = NULL;
			szIMECompositionResultString[0] = NULL;
			return (EVENT_PROCESSED | EVENT_MINT_TRANSLATED);
		}
		break;

	case WM_IME_COMPOSITION:
		if (bIMESupport == true) {
			nMessage = MWM_IMECOMPOSE;
			Mint* pMint = Mint::GetInstance();
			HIMC hIMC = ImmGetContext(hwnd);
			if (hIMC) {
				DWORD dwConv, dwSent;
				if (ImmGetConversionStatus(hIMC, &dwConv, &dwSent)) {
					if (!(dwConv & IME_CMODE_NATIVE)) {
						ImmSetConversionStatus(hIMC, dwConv | IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE, dwSent);
					}
				}

				// 1. LUỒNG KẾT QUẢ (Khi gõ xong ký tự hoặc nhấn Enter gõ chữ)
				if (lparam & GCS_RESULTSTR) {
					wchar_t szRawTarget[MIMECOMPOSITIONSTRING_LENGTH] = { 0 };
					LONG bytesReceived = ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, szRawTarget, sizeof(szRawTarget));

					if (bytesReceived > 0) {
						int wlen = bytesReceived / sizeof(wchar_t);
						szRawTarget[wlen] = L'\0';

						// Chuẩn hóa chuỗi WideChar về dạng Dựng Sẵn (Form C)
						wchar_t szNormalizedW[MIMECOMPOSITIONSTRING_LENGTH] = { 0 };
						int nlen = NormalizeString(NormalizationC, szRawTarget, -1, NULL, 0);
						if (nlen > 0 && nlen < MIMECOMPOSITIONSTRING_LENGTH) {
							NormalizeString(NormalizationC, szRawTarget, -1, szNormalizedW, nlen);
							szNormalizedW[nlen] = L'\0';
						}
						else {
							wcsncpy(szNormalizedW, szRawTarget, wlen);
							szNormalizedW[wlen] = L'\0';
						}

						// 🛠️ ĐƯỜNG VÒNG CHO IME: Đổi ký tự vừa gõ thành \uXXXX để ZChat.cpp giải mã chuẩn đét
						std::ostringstream ss;
						for (int i = 0; i < wlen; ++i) {
							wchar_t wc = szNormalizedW[i];
							if (wc >= 0x0020 && wc <= 0x007E) {
								ss << (char)wc; // Giữ nguyên ký tự Latin/Số/Khoảng trắng
							}
							else {
								ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)wc;
							}
						}

						std::string escapeStr = ss.str();
						strncpy(szIMECompositionResultString, escapeStr.c_str(), sizeof(szIMECompositionResultString) - 1);
						szIMECompositionResultString[sizeof(szIMECompositionResultString) - 1] = '\0';
					}
					else {
						szIMECompositionResultString[0] = '\0';
					}

					pMint->m_nCompositionAttributeSize = 0;
					memset(pMint->m_nCompositionAttributes, 0, sizeof(BYTE) * (MIMECOMPOSITIONSTRING_LENGTH));
				}

				// 2. LUỒNG ĐANG GÕ (Chữ nhấp nháy gạch chân trên thanh chat)
				if (lparam & GCS_COMPSTR) {
					LONG bytesReceived = ImmGetCompositionStringW(hIMC, GCS_COMPSTR, szIMECompositionString, sizeof(szIMECompositionString));
					if (bytesReceived > 0) {
						int wlen = bytesReceived / sizeof(wchar_t);
						szIMECompositionString[wlen] = L'\0';

						// Để hiển thị đúng chữ đang gõ (ví dụ chữ 阿) trên thanh chat của máy Win English, 
						// ta ép hàm sử dụng CodePage liên kết của bộ gõ hiện tại (chọn CP_ACP linh hoạt thay vì ép cứng 1258)
						// Nếu game hiển thị chuỗi gạch chân bằng WideChar gốc, anh có thể giữ nguyên luồng nạp này.
					}
				}
				else {
					szIMECompositionString[0] = L'\0';
				}

				if (lparam & GCS_COMPATTR)
					pMint->m_nCompositionAttributeSize = ImmGetCompositionStringW(hIMC, GCS_COMPATTR, pMint->m_nCompositionAttributes, sizeof(pMint->m_nCompositionAttributes));

				if (lparam & GCS_CURSORPOS)
					pMint->m_nCompositionCaretPosition = ImmGetCompositionStringW(hIMC, GCS_CURSORPOS, NULL, 0);

				ImmReleaseContext(hwnd, hIMC);
			}
			return (EVENT_PROCESSED | EVENT_MINT_TRANSLATED);
		}
		break;

	case WM_IME_ENDCOMPOSITION:
		if (bIMESupport == true) {
			szIMECompositionString[0] = L'\0';
			szIMECompositionResultString[0] = L'\0';
			Mint* pMint = Mint::GetInstance();
			pMint->m_nCompositionCaretPosition = 0;
			return (EVENT_PROCESSED | EVENT_MINT_TRANSLATED);
		}
		break;

	case WM_IME_NOTIFY:
	{
		Mint* pMint = Mint::GetInstance();
		if (bIMESupport == true && pMint->IsEnableIME() == true) {
			if (wparam == IMN_OPENCANDIDATE || wparam == IMN_CHANGECANDIDATE) {
				pMint->OpenCandidateList();
			}
			else if (wparam == IMN_CLOSECANDIDATE) {
				pMint->CloseCandidateList();
			}
			return (EVENT_PROCESSED | EVENT_MINT_TRANSLATED);
		}
	}
	break;

	case WM_IME_SETCONTEXT:
		// Cho phép truyền thông điệp ngữ cảnh vẽ IME tự nhiên của hệ thống để hiển thị cửa sổ chọn từ của tiếng Trung/Hàn/Nhật
		return DefWindowProc(hwnd, message, wparam, lparam);
	}

	return EVENT_NOT_PROCESSED;
}