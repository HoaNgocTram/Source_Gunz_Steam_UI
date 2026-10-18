#include "stdafx.h"
#include "WindowUI.h"
std::string g_szLauncherStatus = "Connecting to server...";
std::string g_szLauncherPercent = "0%";
namespace gui
{
	WPARAM execute()
	{
		MSG message;
		while (GetMessage(&message, 0, 0, 0) > 0)
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		return message.wParam;
	}

	window::window() : widget(0, 0, true), paint_font(0)
	{}

	window::~window()
	{
		if (this->paint_font)
		{
			DeleteObject(this->paint_font);
		}
	}

	bool window::assemble(std::string const& class_name, std::string const& window_name, rectangle rect, const char* icon_name)
	{
		if (!this->create_class(class_name, icon_name))
		{
			return false;
		}

		if (!this->create_window(class_name, window_name, rect))
		{
			return false;
		}

		this->set_message_handlers();

		SetLayeredWindowAttributes(this->get_handle(), 0, 229, LWA_ALPHA);

		ShowWindow(this->get_handle(), SW_SHOWNORMAL);
		UpdateWindow(this->get_handle());

		this->progress = new progressbar(this->get_handle(), this->get_instance());
		this->progress->assemble(rectangle(-1, 216, 460, 12), 0, 100);

		return true;
	}

	bool window::create_class(std::string const& class_name, const char* icon_name)
	{
		WNDCLASSEXA WndClassEx;
		WndClassEx.cbSize = sizeof(WNDCLASSEXA);
		WndClassEx.style = 0;
		WndClassEx.lpfnWndProc = window::WndProc;
		WndClassEx.cbClsExtra = 0;
		WndClassEx.cbWndExtra = 0;
		WndClassEx.hInstance = this->get_instance();
		WndClassEx.hIcon = LoadIconA(this->get_instance(), icon_name);
		WndClassEx.hCursor = LoadCursorA(NULL, IDC_ARROW);
		WndClassEx.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));
		WndClassEx.lpszMenuName = NULL;
		WndClassEx.lpszClassName = class_name.c_str();
		WndClassEx.hIconSm = LoadIconA(this->get_instance(), icon_name);

		return (RegisterClassExA(&WndClassEx) != NULL);
	}

	bool window::create_window(std::string const& class_name, std::string const& window_name, rectangle& rect)
	{
		if (!rect.get_x() && !rect.get_y())
		{
			RECT rc;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);

			//center screen:
			rect.set_xy((rc.right / 2) - (rect.get_width() / 2), (rc.bottom / 2) - (rect.get_height() / 2));

			//left corner screen:
			//rect.set_xy(rc.right - rect.get_width(), rc.bottom - rect.get_height());
		}

		return this->create(class_name, window_name, rect, WS_POPUPWINDOW, WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TOPMOST, NULL);
	}
	
	void window::set_message_handlers()
	{
		this->add_message_handlers(4,
			message_pair(WM_CLOSE, [](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
				{
					DestroyWindow(hWnd);
					return 0;
				}),
			message_pair(WM_DESTROY, [](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
				{
					PostQuitMessage(0);
					return 0;
				}),
			message_pair(WM_NCHITTEST, [](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
				{
					return HTCAPTION;
				}),
			message_pair(WM_PAINT, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
				{
					this->paint_font = CreateFontA(12, 0, 0, 0, FW_DONTCARE, 0, 0, 0, ANSI_CHARSET,
						OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5, DEFAULT_PITCH | FF_DONTCARE, "Tahoma");

					this->background = LoadBitmap(this->get_instance(), MAKEINTRESOURCE(NM_BACKGROUND_IMAGE));

					PAINTSTRUCT paint_struct;

					HDC hdc_temp = BeginPaint(hWnd, &paint_struct);
					HDC hdc = CreateCompatibleDC(hdc_temp);

					SelectObject(hdc, this->background);
					SelectObject(hdc, this->paint_font);

					SetTextColor(hdc, RGB(255, 255, 255));
					SetBkMode(hdc, TRANSPARENT);

					// 1. Vẽ chữ trạng thái ở bên trái (X = 10)
					TextOutA(hdc, 10, 195, g_szLauncherStatus.c_str(), (int)g_szLauncherStatus.length());

					// 2. 🎯 VẼ CHỮ PHẦN TRĂM Ở BÊN PHẢI (X = 420, cùng độ cao Y = 195)
					TextOutA(hdc, 420, 195, g_szLauncherPercent.c_str(), (int)g_szLauncherPercent.length());

					BitBlt(hdc_temp, 0, 0, 460, 230, hdc, 0, 0, SRCCOPY);

					DeleteDC(hdc);
					EndPaint(hWnd, &paint_struct);
					return 0;
				})
		);
	}
}