#include "stdafx.h"
#include "NewChat.h"
#include "RGMain.h"
#include "ZCharacterManager.h"
#include "ZInput.h"
#include "Config.h"
#include "defer.h"
#include "MClipboard.h"
#include "CodePageConversion.h"
#include <windows.h>
#include <string>
#include "EmojiManager.h"
#include "AnimatedGifTexture.h"
#include "RealSpace2.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "ZIDLResource.h"
#include "ZEmojiPicker.h"
#include "ZCombatChat.h"

inline std::string WideToUTF8(const std::wstring& wstr)
{
	if (wstr.empty()) return std::string();

	int size = WideCharToMultiByte(
		CP_UTF8, 0,
		wstr.c_str(), -1,
		nullptr, 0,
		nullptr, nullptr
	);

	std::string result(size - 1, 0);
	WideCharToMultiByte(
		CP_UTF8, 0,
		wstr.c_str(), -1,
		&result[0], size,
		nullptr, nullptr
	);

	return result;
}

void CopyToClipboard(const std::wstring& text) {
	if (text.empty()) return;

	if (OpenClipboard(NULL)) {
		EmptyClipboard();
		HGLOBAL hClipboardData = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(wchar_t));
		if (hClipboardData) {
			wchar_t* pchData = (wchar_t*)GlobalLock(hClipboardData);
			if (pchData) {
				wcscpy_s(pchData, text.length() + 1, text.c_str());
				GlobalUnlock(hClipboardData);
				SetClipboardData(CF_UNICODETEXT, hClipboardData);
			}
		}
		CloseClipboard();
	}
}

static std::wstring CodePageConversionWide(const char* str, UINT codePage = CP_ACP)
{
	if (!str) return L"";

	int len = MultiByteToWideChar(codePage, 0, str, -1, nullptr, 0);
	std::wstring wstr(len, 0);
	MultiByteToWideChar(codePage, 0, str, -1, &wstr[0], len);

	// Xóa ký tự null dư thừa cuối chuỗi
	if (!wstr.empty() && wstr.back() == L'\0')
		wstr.pop_back();

	return wstr;
}

namespace ResizeFlagsType
{
	enum
	{
		X1 = 1 << 0,
		Y1 = 1 << 1,
		X2 = 1 << 2,
		Y2 = 1 << 3,
	};
}

// Note that while Wrap and Linebreak both act as linebreaks,
// the former is placed by the line-wrapping mechanism and
// the latter is explicitly placed by the message creator.
enum class FormatSpecifierType {
	Unknown = -1,
	Wrap,
	Linebreak,
	Color,
	Default,
	Bold,
	Italic,
	BoldItalic,
	Underline,
	Strikethrough,
};

struct FormatSpecifier {
	int nStartPos;
	FormatSpecifierType ft;
	D3DCOLOR Color;

	FormatSpecifier(int nStart, D3DCOLOR c) : nStartPos(nStart), ft(FormatSpecifierType::Color), Color(c) {}
	FormatSpecifier(int nStart, FormatSpecifierType type) : nStartPos(nStart), ft(type) {}
};

struct ChatMessage {
	float Time{};
	std::wstring Msg;
	u32 DefaultColor;
	std::vector<FormatSpecifier> FormatSpecifiers;
	int Lines{};
	int nEmojiSet = 0; // emoji set from sender prefix

	void SubstituteFormatSpecifiers();

	int GetLines() const {
		return Lines;
	}

	void ClearWrappingLineBreaks() {
		erase_remove_if(FormatSpecifiers, [&](auto&& x) { return x.ft == FormatSpecifierType::Wrap; });
	}

	const FormatSpecifier* GetLineBreak(int n) const {
		int i = 0;
		for (auto it = FormatSpecifiers.begin(); it != FormatSpecifiers.end(); it++) {
			if (it->ft == FormatSpecifierType::Wrap || it->ft == FormatSpecifierType::Linebreak) {
				if (i == n)
					return &*it;

				i++;
			}
		}

		return 0;
	}

	// Returns an iterator to the format specifier that was inserted.
	auto AddWrappingLineBreak(int n) {
		assert(n >= 0);
		if (n < 0)
			n = 0;

		if (FormatSpecifiers.empty()) {
			FormatSpecifiers.emplace_back(n, FormatSpecifierType::Wrap);
			// Return the last iterator, since we appended to the end.
			return std::prev(FormatSpecifiers.end());
		}

		for (auto it = FormatSpecifiers.rbegin(); it != FormatSpecifiers.rend(); it++) {
			if (it->nStartPos < n) {
				// it.base() is AFTER it (in terms of normal order, not reversed),
				// and insert inserts BEFORE the passed iterator, so this inserts
				// after the current iterator, which is correct since the
				// desired index n is after the current format specifier.
				return FormatSpecifiers.insert(it.base(), FormatSpecifier(n, FormatSpecifierType::Wrap));
			}
		}

		// The loop was unable to find a format specifier that precedes
		// the position, so we must add it at the start.
		return FormatSpecifiers.insert(FormatSpecifiers.begin(), FormatSpecifier(n, FormatSpecifierType::Wrap));
	}
};

namespace EmphasisType
{
	enum
	{
		Default = 0,
		Italic = 1 << 0,
		Bold = 1 << 1,
		Underline = 1 << 2,
		Strikethrough = 1 << 3,
	};
}

// A substring of a line to be displayed.
// The substring may be the entire line, but cannot span more than one line.
struct LineSegmentInfo
{
	// Index into Chat::vMsgs.
	int ChatMessageIndex;
	// Offset into ChatMessage::Msg at which the substring to be displayed begins.
	u16 Offset;
	// Length of the substring, in characters.
	u16 LengthInCharacters;
	// Pixel offset on the X axis at which this segment starts.
	u16 PixelOffsetX;
	struct {
		// Is this the start of the line?
		u16 IsStartOfLine : 1;
		// Emphasis, i.e. italic, bold, etc.
		// This is a bitmask; it can hold a combination of multiple emphases.
		u16 Emphasis : 15;
	};
	u32 TextColor;
};

// Stuff crashes if this is increased
static constexpr int MAX_INPUT_LENGTH = 230;

void ChatMessage::SubstituteFormatSpecifiers()
{
	// TODO: Properly handle multiple emphases at once, e.g. both italic and underlined,
	// and remove only one when one is ended.
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		auto CharToFT = [&](char c) {
			switch (c) {
			case 'b': return FormatSpecifierType::Bold;
			case 'i': return FormatSpecifierType::Italic;
			case 's': return FormatSpecifierType::Strikethrough;
			case 'u': return FormatSpecifierType::Underline;
				//case 'n': return FormatSpecifierType::Linebreak;
			default:  return FormatSpecifierType::Unknown;
			};
			};

		const auto npos = std::wstring::npos;

		// Scan entire message for emoji set prefix \x1F + digit (may be after "PlayerName : ")
		for (size_t p = 0; p + 1 < Msg.size(); p++)
		{
			if (Msg[p] == (wchar_t)EmojiManager::EMOJI_PREFIX && Msg[p + 1] >= L'0' && Msg[p + 1] <= L'9')
			{
				nEmojiSet = Msg[p + 1] - L'0';
				Msg.erase(p, 2);
				break;
			}
		}

		// Fallback: if no prefix found, use local active set
		if (nEmojiSet == 0)
			nEmojiSet = ZGetEmojiManager().GetActiveSet();

		bool Erased = false;

		for (auto Pos = Msg.find_first_of(L"^[", 0);
			Pos != npos && Pos <= Msg.length() - 2;
			Pos = Pos < Msg.length() ? Msg.find_first_of(L"^[", Erased ? Pos : Pos + 1) : npos)
		{
			Erased = false;

			auto Erase = [&](std::wstring::size_type Count) {
				Msg.erase(Pos, Count);

				Erased = true;
				};

			auto RemainingLength = Msg.length() - Pos;
			auto CurrentChar = Msg[Pos];

			if (CurrentChar == '^')
			{
				// Handles color specifiers, like "Normal text ^1Red text"
				auto NextChar = Msg[Pos + 1];
				if (isdigit(NextChar))
				{
					// Simple specifier, e.g. "^1Red text"

					FormatSpecifiers.emplace_back(Pos, MMColorSet[NextChar - '0']);
					Erase(2);
				}
				else if (NextChar == '#')
				{
					// Elaborate specifier, e.g. "^#80FF0000Transparent red text"

					auto ishexdigit = [&](auto c) {
						c = tolower(c);
						return isdigit(c) || (c >= 'a' && c <= 'f');
						};

					auto ColorStart = Pos + 2;
					auto ColorEnd = ColorStart;
					while (ColorEnd < Msg.length() &&
						ColorEnd - ColorStart < 8 &&
						ishexdigit(Msg[ColorEnd])) {
						++ColorEnd;
					}

					auto Distance = ColorEnd - ColorStart;

					// Must be 8 digits
					if (Distance != 8)
						continue;

					wchar_t ColorString[32];
					strncpy_safe(ColorString, &Msg[ColorStart], Distance);

					wchar_t* endptr;
					auto Color = static_cast<D3DCOLOR>(wcstoul(ColorString, &endptr, 16));
					assert(endptr == ColorString + Distance);

					FormatSpecifiers.emplace_back(Pos, Color);
					Erase(ColorEnd - Pos);
				}
			}
			else if (CurrentChar == '[')
			{
				// Handles specifiers like "Normal text [b]Bold text[/b]"
				auto EndBracket = Msg.find_first_of(L"]", Pos + 1);

				if (EndBracket == npos)
					continue; // Malformed specifier

				auto Distance = EndBracket - Pos;

				if (Msg[Pos + 1] == '/' && (Distance == 2 || Distance == 3))
				{
					// End of sequence
					// Matches e.g. [/], [/b], [/i]

					// Go back to default text
					FormatSpecifiers.emplace_back(Pos, FormatSpecifierType::Default);
				}
				else
				{
					// Beginning of sequence
					// Matches e.g. [b], [i]
					auto ft = CharToFT(Msg[Pos + 1]);
					if (ft == FormatSpecifierType::Unknown)
						continue;

					FormatSpecifiers.emplace_back(Pos, ft);
				}

				Erase(Distance + 1);
			}
		}
	}
}

void Chat::UpdateBorderFromOldChat(int screenW, int screenH)
{
	// Priority:
	// 1. "NewChatFrame" widget in XML (dedicated config for NewChat)
	// 2. "CombatChatOutput" widget (align with old chat)
	// 3. Fallback: hardcoded percentage

	if (ZGetGameInterface() && ZGetGameInterface()->GetIDLResource())
	{
		ZIDLResource* pRes = ZGetGameInterface()->GetIDLResource();

		// Priority 1: dedicated NewChatFrame widget
		MWidget* pNewChatFrame = pRes->FindWidget("NewChatFrame");
		if (pNewChatFrame)
		{
			MRECT rc = pNewChatFrame->GetScreenRect();
			Border.x1 = rc.x;
			Border.y1 = rc.y;
			Border.x2 = rc.x + rc.w;
			Border.y2 = rc.y + rc.h;
			return;
		}

		// Priority 2: align with old chat widgets
		MWidget* pOutput = pRes->FindWidget("CombatChatOutput");
		if (pOutput)
		{
			MRECT rc = pOutput->GetScreenRect();
			Border.x1 = rc.x;
			Border.y1 = rc.y;
			Border.x2 = rc.x + rc.w;

			MWidget* pInput = pRes->FindWidget("CombatChatInput");
			if (pInput)
			{
				MRECT rcInput = pInput->GetScreenRect();
				Border.y2 = rcInput.y;
			}
			else
			{
				Border.y2 = rc.y + rc.h;
			}
			return;
		}
	}

	// Priority 3: fallback from XML baseline (800x600)
	double scaleX = (double)screenW / 800.0;
	double scaleY = (double)screenH / 600.0;
	int frameX = (int)(10 * scaleX);
	int frameY = screenH - (int)(250 * scaleY);

	Border.x1 = frameX + (int)(7 * scaleX);
	Border.y1 = frameY + (int)(112 * scaleY);
	Border.x2 = frameX + (int)(487 * scaleX);
	Border.y2 = frameY + (int)(215 * scaleY);
}

Chat::Chat(const std::string& FontName, bool BoldFont, int FontSize)
	: FontName{ FontName }, BoldFont{ BoldFont }, FontSize{ FontSize }
{
	const auto ScreenWidth = RGetScreenWidth();
	const auto ScreenHeight = RGetScreenHeight();

	UpdateBorderFromOldChat(ScreenWidth, ScreenHeight);

	Cursor.x = ScreenWidth / 2;
	Cursor.y = ScreenHeight / 2;

	const auto Scale = 1.f;
	DefaultFont.Create("NewChatFont", FontName.c_str(),
		int(float(FontSize) / 1080 * RGetScreenHeight() + 0.5), Scale, BoldFont);
	ItalicFont.Create("NewChatItalicFont", FontName.c_str(),
		int(float(FontSize) / 1080 * RGetScreenHeight() + 0.5), Scale, BoldFont, true);

	FontHeight = DefaultFont.GetHeight();

	m_pTabPlayerList = nullptr;
}

//Chat::~Chat() = default;
Chat::~Chat()
{
	if (m_pTabPlayerList) {
		delete m_pTabPlayerList;
		m_pTabPlayerList = nullptr;
	}
}

//void Chat::InsertTextAtCaret(const char* szText)
//{
//	std::wstring wp = CodePageConversionWide(szText);
//	int insertPos = (CaretPos >= 0) ? CaretPos + 1 : (int)InputField.size();
//	if (insertPos > (int)InputField.size())
//		insertPos = (int)InputField.size();
//	InputField.insert(insertPos, wp);
//	CaretPos = insertPos + (int)wp.size() - 1;
//}

void Chat::InsertTextAtCaret(const char* szText)
{
	if (!szText) return;

	// Chuyển đổi chuẩn từ ANSI sang Wide
	std::wstring wp = CodePageConversionWide(szText);

	// Xóa trắng cái phần đang gõ dở sau dấu cách cuối cùng
	size_t lastSpace = InputField.rfind(L' ');
	if (lastSpace == std::wstring::npos) {
		InputField = wp;
	}
	else {
		InputField = InputField.substr(0, lastSpace + 1) + wp;
	}

	InputField += L" "; // Thêm dấu cách

	// Đặt con trỏ ở cuối dòng
	CaretPos = (int)InputField.size();

	// Đảm bảo InputEnabled vẫn đang là true để UI nó vẽ lại cái InputField
	InputEnabled = true;
}

void Chat::EnableInput(bool Enable, bool ToTeam)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		auto&& Cfg = *ZGetConfiguration();

		InputEnabled = Enable;
		TeamChat = ToTeam;

		// BẬT hoặc TẮT kết nối IME hệ thống theo trạng thái đóng mở ô chat
		if (Mint::GetInstance()) {
			Mint::GetInstance()->EnableIME(Enable);
		}

		if (Enable) {
			InputField.clear();
			CaretPos = -1;
		}
		else {
			SelectionState = SelectionStateType{};

			// Khi đóng chat, hủy luôn bảng chọn tên TabPlayerList đang hiện nếu có
			if (m_pTabPlayerList && m_pTabPlayerList->IsVisible()) {
				m_pTabPlayerList->Show(false);
			}
		}

		POINT pt = { RGetScreenWidth() / 2, RGetScreenHeight() / 2 };
		ClientToScreen(g_hWnd, &pt);
		SetCursorPos(pt.x, pt.y);

		ZGetInput()->OffActionKeys();
		ZGetGameInterface()->SetCursorEnable(Enable);

		ZPostPeerChatIcon(Enable);
	}
}

void Chat::OutputChatMsg(const char* Msg)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		OutputChatMsg(Msg, TextColor);
	}

}

void Chat::OutputChatMsg(const char* szMsg, u32 dwColor)
{
	wchar_t WideMsg[4096];
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bCodeChat)
	{
		auto ret = CodePageConversion<CP_SYMBOL>(WideMsg, szMsg);
		if (ret == ConversionError)
		{
			MLog("Chat::OutputChatMsg -- Conversion error\n");
			assert(false);
			return;
		}
	}
	else
	{
		auto ret = CodePageConversion<CP_UTF8>(WideMsg, szMsg);
		if (ret == ConversionError)
		{
			MLog("Chat::OutputChatMsg -- Conversion error\n");
			assert(false);
			return;
		}
	}
	if (Msgs.size() > 10) {
		Msgs.erase(Msgs.begin());
		// Sau khi xóa tin đầu, phải cập nhật lại LineSegments để tránh lệch dòng
		LineSegments.clear();
		for (int i = 0; i < (int)Msgs.size(); ++i)
			DivideIntoLines(i, std::back_inserter(LineSegments));
	}

	Msgs.emplace_back();
	auto&& Msg = Msgs.back();
	Msg.Time = ZGetGame()->GetTime();
	Msg.Msg = WideMsg;
	Msg.DefaultColor = dwColor;

	Msg.SubstituteFormatSpecifiers();
	DivideIntoLines(Msgs.size() - 1, std::back_inserter(LineSegments));

	NumNewlyAddedLines += Msg.GetLines();
	if (ChatLinesPixelOffsetY <= 0)
		ChatLinesPixelOffsetY = FontHeight;
}

void Chat::Scale(double WidthRatio, double HeightRatio)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		UpdateBorderFromOldChat(RGetScreenWidth(), RGetScreenHeight());

		ResetFonts();
	}
}

void Chat::Resize(int nWidth, int nHeight)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		UpdateBorderFromOldChat(RGetScreenWidth(), RGetScreenHeight());

		ResetFonts();
	}
}

void Chat::ClearHistory()
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		Msgs.clear();
		LineSegments.clear();
		NumNewlyAddedLines = 0;
		ChatLinesPixelOffsetY = 0;
	}
}

bool Chat::CursorInRange(int x1, int y1, int x2, int y2)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		return Cursor.x > x1 && Cursor.x < x2 && Cursor.y > y1 && Cursor.y < y2;
	}
}

int Chat::GetTextLength(MFontR2& Font, const wchar_t* Format, ...)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		wchar_t buf[1024];
		va_list va;
		va_start(va, Format);
		vswprintf_safe(buf, Format, va);
		va_end(va);
		return Font.GetWidth(buf);
	}
}

struct CaretType
{
	int TotalTextHeight;
	v2i CaretPos;
};
static CaretType GetCaretPos(MFontR2& Font, const wchar_t* Text, int CaretPos, int Width)
{
	CaretType ret{ 1, { 0, 1 } };
	v2i Cursor{ 0, 1 };
	for (auto c = Text; *c != 0; ++c)
	{
		auto CharWidth = Font.GetWidth(c, 1);

		Cursor.x += CharWidth;
		if (Cursor.x > Width)
		{
			++Cursor.y;
			Cursor.x = CharWidth;
		}

		auto Distance = c - Text;
		if (Distance == CaretPos)
			ret.CaretPos = Cursor;
	}
	ret.TotalTextHeight = Cursor.y;
	return ret;
}

std::pair<bool, v2i> Chat::GetPos(const ChatMessage& c, unsigned long Pos)
{
	std::pair<bool, v2i> ret{ false, {0, 0} };
	if (Pos > c.Msg.length())
		return ret;

	D3DRECT Output = GetOutputRect();

	int Limit = (Output.y2 - Output.y1 - 10) / FontHeight;

	int nLines = 0;

	for (int i = Msgs.size() - 1; nLines < Limit && i >= 0; i--) {
		auto& cl = Msgs.at(i);

		if (&c == &cl) {
			int nOffset = 0;

			if (c.GetLines() == 1) {
				ret.second.y = Output.y2 - 5 - (nLines)*FontHeight - FontHeight * .5;
			}
			else {
				int nLine = 0;

				for (int i = 0; i < c.GetLines() - 1; i++) {
					auto* lb = c.GetLineBreak(i);
					if (!lb || int(Pos) < lb->nStartPos)
						break;

					nLine++;
				}

				ret.second.y = Output.y2 - 5 - (nLines - nLine) * FontHeight - FontHeight * .5;

				if (nLine > 0) {
					auto* lb = c.GetLineBreak(nLine - 1);
					if (lb) nOffset = lb->nStartPos;
				}
			}

			ret.second.x = Output.x1 + 5 + GetTextLength(DefaultFont, L"%.*s_", Pos - nOffset,
				&c.Msg.at(nOffset)) - GetTextLength(DefaultFont, L"_");

			ret.first = true;
			return ret;
		}

		nLines += cl.GetLines();
	}

	return ret;
}

std::wstring g_wImeGlobalLockStr = L"";
bool g_bLastInputState = false;
DWORD g_dwImeLastInsertTime = 0;
bool Chat::OnEvent(MEvent* pEvent) {
	static bool bWasEnabled = false;
	bool bNowEnabled = IsInputEnabled();

	// 🚀 PHÁT HIỆN KHOẢNH KHẮC MỞ CHAT (Transition)
	if (bNowEnabled && !bWasEnabled)
	{
		// 1. Ép sạch dữ liệu cũ
		InputField = L"";
		CaretPos = -1;
		g_wImeGlobalLockStr = L"";
		g_dwImeLastInsertTime = 0;

		// 2. ⚡ QUAN TRỌNG: Xóa đệm IME ở mức hệ thống ngay lúc vừa mở
		HIMC hImc = ImmGetContext(g_hWnd);
		if (hImc) {
			// Hủy bỏ mọi chuỗi đang soạn thảo dở dang trong đệm Windows
			ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);

			// Xóa sạch các kết quả cũ còn lưu trong IME Context
			// Nếu vẫn bị, hãy thêm lệnh này để Reset Context:
			ImmSetCompositionStringW(hImc, SCS_SETSTR, NULL, 0, NULL, 0);

			ImmReleaseContext(g_hWnd, hImc);
		}
	}
	bWasEnabled = bNowEnabled;

	if (ZGetGame() && !ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		return false;
	}

	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		auto&& Cfg = *ZGetConfiguration();

		static DWORD dwLastToggleTime = 0;
		DWORD dwCurrentTime = timeGetTime();

		// =========================================================================
		// 🛠️ XỬ LÝ BỘ GÕ WINDOWS TELEX MẶC ĐỊNH (FIX KHÔNG HIỆN CHỮ CÓ DẤU)
		// =========================================================================
		// 🎯 TỰ ĐỘNG RESET: Nếu phát hiện thanh chat vừa được bật lên, clear sạch vân tay cũ để không bị đơ
		if (IsInputEnabled() != g_bLastInputState)
		{
			g_bLastInputState = IsInputEnabled();
			g_wImeGlobalLockStr = L""; // Xóa sạch ký ức chữ ngoài Lobby

			// Xóa luôn chuỗi kết quả IME cũ trong Windows để tránh tự động nhảy chữ cũ vào trận
			HIMC hImc = ImmGetContext(g_hWnd);
			if (hImc)
			{
				LONG dwSize = ImmGetCompositionStringW(hImc, GCS_RESULTSTR, NULL, 0);
				if (dwSize > 0)
				{
					std::wstring wBuf(dwSize / sizeof(wchar_t), 0);
					ImmGetCompositionStringW(hImc, GCS_RESULTSTR, &wBuf[0], dwSize);

					if (!wBuf.empty() && wBuf.back() == L'\0') wBuf.pop_back();

					if (!wBuf.empty())
					{
						DWORD dwNow = timeGetTime();

						// 🎯 LOGIC MỚI: Chỉ chặn nếu thời gian gõ quá nhanh (< 80ms) - BẤT KỂ NỘI DUNG CHỮ LÀ GÌ.
						// Điều này cho phép gõ "哈哈" (haha) hay bất kỳ từ trùng nhau nào liền tù tì mà không cần space!
						bool bIsSpam = (dwNow - g_dwImeLastInsertTime < 80);

						if (!bIsSpam)
						{
							if (InputField.length() + wBuf.length() <= MAX_INPUT_LENGTH)
							{
								// Cập nhật mốc thời gian mới
								g_dwImeLastInsertTime = dwNow;
								// Lưu vân tay (vẫn lưu để đề phòng spam cực đoan)
								g_wImeGlobalLockStr = wBuf;

								// Chèn chuỗi Unicode
								size_t insertPos = min((size_t)(CaretPos + 1), InputField.length());
								InputField.insert(insertPos, wBuf);
								CaretPos += (int)wBuf.length();

								// Vẽ lại con trỏ UI
								auto ret = GetCaretPos(DefaultFont, InputField.c_str(), CaretPos, Border.x2 - (Border.x1 + 5));
								InputHeight = ret.TotalTextHeight;
								CaretCoord = ret.CaretPos;
							}
						}

						// Dù là spam hay chữ hợp lệ, ta luôn trả về true để nuốt sự kiện IME của Windows, 
						// tránh nó dội dấu '?' xuống ô chat phía dưới.
						ImmReleaseContext(g_hWnd, hImc);
						return true;
					}
				}
				ImmReleaseContext(g_hWnd, hImc);
			}
		}

		if (IsInputEnabled())
		{
			// Khối xử lý phím tắt hệ thống (như ESCAPE để đóng chat nhanh không thông qua bộ lọc)
			if (pEvent->nMessage == MWM_CHAR && pEvent->nKey == VK_ESCAPE) {
				if (ZGetEmojiPicker().IsVisible()) { ZGetEmojiPicker().Hide(); return true; }
				EnableInput(false, false);
				return true;
			}

			// =========================================================================
			// 🚀 BẮT CHỮ IME TRỰC TIẾP TỪ WINDOWS RAM - BẢN FIX ĐƠ CỨNG KHUNG CHAT
			// =========================================================================
			HIMC hImc = ImmGetContext(g_hWnd);
			if (hImc)
			{
				LONG dwSize = ImmGetCompositionStringW(hImc, GCS_RESULTSTR, NULL, 0);
				if (dwSize > 0)
				{
					std::wstring wBuf(dwSize / sizeof(wchar_t), 0);
					ImmGetCompositionStringW(hImc, GCS_RESULTSTR, &wBuf[0], dwSize);

					if (!wBuf.empty() && wBuf.back() == L'\0') wBuf.pop_back();

					// CHỈ XỬ LÝ KHI CHỮ NÀY KHÁC VỚI CHỮ VỪA ĐƯỢC NẠP TRƯỚC ĐÓ
					if (!wBuf.empty() && wBuf != g_wImeGlobalLockStr)
					{
						if (InputField.length() + wBuf.length() <= MAX_INPUT_LENGTH)
						{
							g_wImeGlobalLockStr = wBuf; // Cập nhật vân tay chữ mới

							// Chèn chuỗi Unicode sạch vào vị trí con trỏ ô chat
							size_t insertPos = min((size_t)(CaretPos + 1), InputField.length());
							InputField.insert(insertPos, wBuf);
							CaretPos += (int)wBuf.length();

							// Vẽ lại con trỏ UI ngay lập tức
							auto ret = GetCaretPos(DefaultFont, InputField.c_str(), CaretPos, Border.x2 - (Border.x1 + 5));
							InputHeight = ret.TotalTextHeight;
							CaretCoord = ret.CaretPos;
						}

						ImmReleaseContext(g_hWnd, hImc);
						return true; // CHỈ return true khi thực sự có chữ mới được nạp để chặn spam
					}
				}
				ImmReleaseContext(g_hWnd, hImc);
			}
			// =========================================================================
		}
		// =========================================================================

		const auto ActionPressed = pEvent->nMessage == MWM_ACTIONPRESSED;
		const auto CharMessage = pEvent->nMessage == MWM_CHAR;

		bool ChatPressed = false;
		{
			static bool IgnoreNextChatActionKey = false;
			auto&& Key = ZGetConfiguration()->GetKeyboard()->ActionKeys[ZACTION_CHAT];

			if (InputEnabled)
			{
				// Khi đang mở chat: Chỉ nhận Enter phát ra từ thông điệp ký tự cửa sổ để gửi tin
				if (CharMessage && pEvent->nKey == VK_RETURN)
				{
					ChatPressed = true;
				}

				if (Key.nVirtualKey == DIK_RETURN || Key.nVirtualKeyAlt == DIK_RETURN ||
					Key.nVirtualKey == DIK_NUMPADENTER || Key.nVirtualKeyAlt == DIK_NUMPADENTER)
				{
					HideAlways = false;
					IgnoreNextChatActionKey = true;
				}
			}
			else
			{
				if (ZIsActionKeyPressed(ZACTION_SHOW_FULL_CHAT))
					HideAlways = false;

				// Khi đang đóng chat: CHỈ cho phép mở thông qua cổng chuẩn ZACTION_CHAT của game
				auto ChatActionKeyPressed = ActionPressed && pEvent->nKey == ZACTION_CHAT;
				if (IgnoreNextChatActionKey && ChatActionKeyPressed)
				{
					IgnoreNextChatActionKey = false;
				}
				else if (ChatActionKeyPressed)
				{
					ChatPressed = true;
				}
			}
		}

		const auto TeamChatPressed = !InputEnabled && ActionPressed && pEvent->nKey == ZACTION_TEAMCHAT;

		if (ChatPressed || TeamChatPressed)
		{
			// 🛑 KHÓA DEBOUNCE AN TOÀN: Nếu khoảng cách bấm Enter giữa 2 trạng thái < 150ms -> Huỷ lệnh dội phím
			if (dwCurrentTime - dwLastToggleTime < 150)
			{
				return true;
			}
			dwLastToggleTime = dwCurrentTime; // Cập nhật ngay mốc thời gian hợp lệ

			if (InputEnabled && ChatPressed && !InputField.empty())
			{
				char MultiByteString[1024];
				if (ZGetGame() && ZGetConfiguration()->GetEtc()->bCodeChat)
				{
					CodePageConversion<CP_SYMBOL>(MultiByteString, InputField.c_str());
				}
				else
				{
					CodePageConversion<CP_UTF8>(MultiByteString, InputField.c_str());
				}

				ZGetGameInterface()->GetChat()->Input(MultiByteString);

				InputHistory.push_back(InputField);
				CurInputHistoryEntry = InputHistory.size();

				InputField.clear();
				CaretPos = -1;
			}

			// Đổi trạng thái Ẩn/Hiện của thanh chat
			EnableInput(!InputEnabled, TeamChatPressed);
			return true;
		}

		if (pEvent->nMessage == MWM_KEYDOWN) {
			bool bCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
			switch (pEvent->nKey) {
			case 'C':
			case 'c':
				if (bCtrl && IsInputEnabled())
				{
					if (!InputField.empty()) {
						CopyToClipboard(InputField);
					}
					return true;
				}
				break;

			case VK_HOME:
				if (IsInputEnabled()) CaretPos = -1;
				break;

			case VK_END:
				if (IsInputEnabled()) CaretPos = (int)InputField.length() - 1;
				break;

			case VK_TAB:
				if (IsInputEnabled())
				{
					if (InputField.size() > 0)
					{
						int cp = (CaretPos >= 0 && CaretPos < (int)InputField.size()) ? CaretPos : (int)InputField.size() - 1;
						if (InputField[cp] == L':')
						{
							InputField.erase(cp, 1);
							if (CaretPos > 0) CaretPos--;
							auto ir = GetInputRect();
							ZGetEmojiPicker().SetAnchor(ir.x1, ir.y1, ir.x2 - ir.x1);
							ZGetEmojiPicker().SetInsertCallback([](const char* p) {
								GetRGMain().GetChat().InsertTextAtCaret(p);
								});
							ZGetEmojiPicker().Toggle();
							break;
						}
					}

					if (m_pTabPlayerList && m_pTabPlayerList->IsVisible())
					{
						m_pTabPlayerList->OnPickPlayer();
						return true;
					}

					if (m_pTabPlayerList == nullptr)
					{
						m_pTabPlayerList = new ZTabPlayerList("TabPlayerList", ZGetGameInterface(), ZGetGameInterface());
						if (m_pTabPlayerList) {
							m_pTabPlayerList->SetFont(MFontManager::Get("Default"));
							m_pTabPlayerList->Show(false);
						}
					}

					if (m_pTabPlayerList)
					{
						m_pTabPlayerList->OnShow();
						D3DRECT rect = GetInputRect();
						m_pTabPlayerList->SetBounds(rect.x1, rect.y1 - 125, 150, 120);
						m_pTabPlayerList->Show(true);
						m_pTabPlayerList->SetFocus();
					}
					return true;
				}
				break;

			case VK_UP:
				if (IsInputEnabled()) {
					if (m_pTabPlayerList && m_pTabPlayerList->IsVisible()) {
						m_pTabPlayerList->OnEvent(pEvent, m_pTabPlayerList->GetListener());
						return true;
					}
					if (CurInputHistoryEntry > 0) {
						CurInputHistoryEntry--;
						InputField.assign(InputHistory.at(CurInputHistoryEntry));
						CaretPos = (int)InputHistory.at(CurInputHistoryEntry).length() - 1;
					}
				}
				break;

			case VK_DOWN:
				if (IsInputEnabled()) {
					if (m_pTabPlayerList && m_pTabPlayerList->IsVisible()) {
						m_pTabPlayerList->OnEvent(pEvent, m_pTabPlayerList->GetListener());
						return true;
					}
					if (CurInputHistoryEntry < int(InputHistory.size()) - 1) {
						CurInputHistoryEntry++;
						auto&& strEntry = InputHistory.at(CurInputHistoryEntry);
						InputField.assign(strEntry);
						CaretPos = (int)strEntry.length() - 1;
					}
					else {
						InputField.clear();
						CaretPos = -1;
					}
				}
				break;

			case VK_LEFT:
				if (IsInputEnabled() && CaretPos >= 0)
					CaretPos--;
				break;

			case VK_RIGHT:
				if (IsInputEnabled() && CaretPos < int(InputField.length()) - 1)
					CaretPos++;
				break;

			case 'V':
			case 'v':
				if (bCtrl && IsInputEnabled())
				{
					wchar_t Clipboard[256];
					MClipboard::Get(g_hWnd, Clipboard, std::size(Clipboard));
					if (InputField.length() + wcslen(Clipboard) > MAX_INPUT_LENGTH)
					{
						InputField.append(Clipboard, Clipboard + MAX_INPUT_LENGTH - InputField.length());
					}
					else
					{
						InputField += Clipboard;
					}
					CaretPos = (int)InputField.length() - 1;
					return true;
				}
				break;
			};
		}
		else if (pEvent->nMessage == MWM_CHAR && IsInputEnabled()) {
			switch (pEvent->nKey) {
			case VK_TAB:
				break;

			case VK_RETURN:
				ChatPressed = true;        // Thông luồng Enter, bắn tin lên chatlog luôn!
				g_wImeGlobalLockStr = L""; // Gửi xong thì reset luôn vân tay để sẵn sàng cho câu sau
				break;

			case VK_BACK:
				// Nút xóa Backspace hoạt động tự do, không bị cấn vân tay IME cũ
				if (CaretPos >= 0 && !InputField.empty()) {
					size_t erasePos = min((size_t)CaretPos, InputField.length() - 1);
					InputField.erase(erasePos, 1);
					CaretPos--;

					auto ret = GetCaretPos(DefaultFont, InputField.c_str(), CaretPos, Border.x2 - (Border.x1 + 5));
					InputHeight = ret.TotalTextHeight;
					CaretCoord = ret.CaretPos;
				}
				break;

			default:
				// Chặn phím rác '?' sinh ra do Telex giải dấu
				if (ImmIsIME(GetKeyboardLayout(0)))
				{
					if (pEvent->nKey == L'?' || pEvent->nKey > 127)
					{
						break;
					}
				}

				// Gõ phím thường tiếng Anh / Số
				if (InputField.length() < MAX_INPUT_LENGTH) {
					if (pEvent->nKey < 32)
						break;

					size_t insertPos = min((size_t)(CaretPos + 1), InputField.length());
					InputField.insert(insertPos, 1, pEvent->nKey);

					auto SlashR = L"/r ";
					auto SlashWhisper = L"/whisper ";
					if (iequals(InputField, SlashR))
					{
						wchar_t LastSenderWide[512];
						auto* LastSender = ZGetGameInterface()->GetChat()->m_szWhisperLastSender;
						auto len = CodePageConversion<CP_ACP>(LastSenderWide, LastSender);
						if (len != ConversionError)
						{
							InputField = SlashWhisper;
							InputField += LastSenderWide;
							InputField += ' ';
							CaretPos = (int)InputField.length() - 1;
						}
					}
					else
					{
						CaretPos++;
					}
				}
			};
		}

		auto ret = GetCaretPos(DefaultFont, InputField.c_str(), CaretPos, Border.x2 - (Border.x1 + 5));
		InputHeight = ret.TotalTextHeight;
		CaretCoord = ret.CaretPos;

		return true;
	}

	return false;
}

int Chat::GetTextLen(ChatMessage& cl, int Pos, int Count) {
	return GetTextLength(DefaultFont, L"_%.*s_", Count, &cl.Msg.at(Pos)) - GetTextLength(DefaultFont, L"__");
}

int Chat::GetTextLen(const char* Msg, int Count) {
	return GetTextLength(DefaultFont, L"_%.*s_", Count, Msg) - GetTextLength(DefaultFont, L"__");
}

void Chat::OnUpdate(float TimeDelta)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		auto&& Cfg = *ZGetConfiguration();

		UpdateNewMessagesAnimation(TimeDelta);

		if (!IsInputEnabled())
			return;

		auto PrevCursorPos = Cursor;
		Cursor = MEvent::LatestPos;

		D3DXVECTOR2 MinimumSize{ 192.f * RGetScreenWidth() / 1920.f, 108.f * RGetScreenHeight() / 1080.f };

		if (ResizeFlags) {
			if (ResizeFlags & ResizeFlagsType::X1 &&
				Border.x1 + Cursor.x - PrevCursorPos.x < Border.x2 - MinimumSize.x) {
				Border.x1 += Cursor.x - PrevCursorPos.x;
			}
			if (ResizeFlags & ResizeFlagsType::X2 &&
				Border.x2 + Cursor.x - PrevCursorPos.x > Border.x1 + MinimumSize.x) {
				Border.x2 += Cursor.x - PrevCursorPos.x;
			}
			if (ResizeFlags & ResizeFlagsType::Y1 &&
				Border.y1 + Cursor.y - PrevCursorPos.y < Border.y2 - MinimumSize.y) {
				Border.y1 += Cursor.y - PrevCursorPos.y;
			}
			if (ResizeFlags & ResizeFlagsType::Y2 &&
				Border.y2 + Cursor.y - PrevCursorPos.y > Border.y1 + MinimumSize.y) {
				Border.y2 += Cursor.y - PrevCursorPos.y;
			}

			LineSegments.clear();
			for (int i = 0; i < int(Msgs.size()); ++i)
				DivideIntoLines(i, std::back_inserter(LineSegments));
		}

		if (Action == ChatWindowAction::Moving) {
			Border.x1 += Cursor.x - PrevCursorPos.x;
			Border.y1 += Cursor.y - PrevCursorPos.y;
			Border.x2 += Cursor.x - PrevCursorPos.x;
			Border.y2 += Cursor.y - PrevCursorPos.y;
		}

		if (SelectionState.FromMsg && SelectionState.ToMsg &&
			MEvent::IsKeyDown(VK_CONTROL) && MEvent::IsKeyDown('C')) {
			if (OpenClipboard(g_hWnd)) {
				EmptyClipboard();

				if (SelectionState.FromMsg == SelectionState.ToMsg) {
					auto index = min(SelectionState.FromPos, SelectionState.ToPos);
					auto str = SelectionState.FromMsg->Msg.substr(index);
					MClipboard::Set(g_hWnd, str);
				}
				else {
					std::wstring str;

					bool FirstFound = false;

					for (auto it = Msgs.begin(); it != Msgs.end(); it++) {
						auto* pcl = &*it;

						if (pcl == SelectionState.FromMsg || pcl == SelectionState.ToMsg) {
							if (!FirstFound) {
								auto nPos = pcl == SelectionState.FromMsg ?
									SelectionState.FromPos : SelectionState.ToPos;
								str.append(&pcl->Msg.at(nPos));

								FirstFound = true;
								continue;
							}
							else {
								auto nPos = pcl == SelectionState.FromMsg ?
									SelectionState.FromPos : SelectionState.ToPos;
								str.append(L"\n");
								str.append(pcl->Msg.c_str(), nPos + 2);

								break;
							}
						}

						if (FirstFound) {
							str.append(L"\n");
							str.append(pcl->Msg.c_str());
						}
					}

					if (FirstFound) {
						MClipboard::Set(g_hWnd, str);
					}
				}

				CloseClipboard();
			}
		}

		const int nBorderWidth = 5;

		// TODO: Move to OnEvent
		if (MEvent::IsKeyDown(VK_LBUTTON)) {
			if (Action == ChatWindowAction::None) {
				D3DRECT tr = GetTotalRect();

				if (CursorInRange(tr.x1 - nBorderWidth, tr.y1 - nBorderWidth,
					tr.x1 + nBorderWidth, tr.y2 + nBorderWidth)) {
					ResizeFlags |= ResizeFlagsType::X1;
				}
				if (CursorInRange(tr.x1 - nBorderWidth, tr.y1 - nBorderWidth,
					tr.x2 + nBorderWidth, tr.y1 + nBorderWidth)) {
					ResizeFlags |= ResizeFlagsType::Y1;
				}
				if (CursorInRange(tr.x2 - nBorderWidth, tr.y1 - nBorderWidth,
					tr.x2 + nBorderWidth, tr.y2 + nBorderWidth)) {
					ResizeFlags |= ResizeFlagsType::X2;
				}
				if (CursorInRange(tr.x1 - nBorderWidth, tr.y2 - nBorderWidth,
					tr.x2 + nBorderWidth, tr.y2 + nBorderWidth)) {
					ResizeFlags |= ResizeFlagsType::Y2;
				}

				if (ResizeFlags)
					Action = ChatWindowAction::Resizing;
			}

			if (CursorInRange(Border.x2 - 15, Border.y1 - 18, Border.x2 - 15 + 12, Border.y1 - 18 + FontHeight) &&
				Action == ChatWindowAction::None) {
				UpdateBorderFromOldChat(RGetScreenWidth(), RGetScreenHeight());
			}
			else if (CursorInRange(Border.x1 + 5, Border.y1 + 5, Border.x2 - 5, Border.y2 - 5)) {
				if (Action != ChatWindowAction::Selecting) {
					auto&& Output = GetOutputRect();

					int Limit = (Output.y2 - Output.y1 - 10) / FontHeight;
					int Line = Limit - ((Output.y2 - 5) - Cursor.y) / FontHeight;

					int i = Msgs.size() - 1;
					int CurLine = Limit + 1;

					while (i >= 0) {
						auto&& cl = Msgs[i];

						if (CurLine - cl.GetLines() <= Line) {
							SelectionState.FromMsg = &cl;
							Action = ChatWindowAction::Selecting;

							int Pos = 0;
							if (CurLine - cl.GetLines() != Line) {
								auto* lb = cl.GetLineBreak(Line - (CurLine - cl.GetLines()) - 1);
								if (lb) Pos = lb->nStartPos;
							}
							int x = Cursor.x - (Output.x1 + 5);
							int Len = 0;

							while (x > Len && Pos < int(cl.Msg.length())) {
								Len += GetTextLen(cl, Pos, 1);
								Pos++;
							}

							Pos--;

							if (Len - GetTextLen(cl, Pos, 1) / 2 > x)
								SelectionState.FromPos = Pos - 1;
							else
								SelectionState.FromPos = Pos;

							break;
						}

						CurLine -= cl.GetLines();
						i--;
					}

					if (i < 0) {
						SelectionState.FromMsg = 0;
						SelectionState.ToMsg = 0;
					}
				}
				else {
					auto&& Output = GetOutputRect();

					int Limit = (Output.y2 - Output.y1 - 10) / FontHeight;
					int Line = Limit - ((Output.y2 - 5) - Cursor.y) / FontHeight;

					int i = Msgs.size() - 1;
					int CurLine = Limit + 1;

					while (i >= 0) {
						auto&& cl = Msgs.at(i);

						if (CurLine - cl.GetLines() <= Line || i == 0) {
							SelectionState.ToMsg = &cl;

							int Pos;

							if (CurLine - cl.GetLines() <= Line) {
								Pos = 0;
								if (CurLine - cl.GetLines() != Line) {
									auto* lb = cl.GetLineBreak(Line - (CurLine - cl.GetLines()) - 1);
									if (lb) Pos = lb->nStartPos;
								}
							}
							else
								Pos = 0;

							int x = Cursor.x - (Output.x1 + 5);
							int nLen = 0;

							while (x > nLen && Pos < int(cl.Msg.length())) {
								nLen += GetTextLen(cl, Pos, 1);
								Pos++;
							}

							Pos--;

							if (nLen - GetTextLen(cl, Pos, 1) / 2 > x)
								SelectionState.ToPos = Pos - 1;
							else
								SelectionState.ToPos = Pos;

							break;
						}

						CurLine -= cl.GetLines();
						i--;
					}
				}
			}
			else if (Action != ChatWindowAction::Selecting) {
				SelectionState.FromMsg = 0;
				SelectionState.ToMsg = 0;
			}

			if (Action == ChatWindowAction::None &&
				CursorInRange(Border.x1, Border.y1 - 20, Border.x2 + 1, Border.y1))
				Action = ChatWindowAction::Moving;
		}
		else
		{
			Action = ChatWindowAction::None;
			ResizeFlags = 0;
		}
	}
}

void Chat::UpdateNewMessagesAnimation(float TimeDelta)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		if (ChatLinesPixelOffsetY <= 0) {
			return;
		}

		constexpr auto LinesPerSecond = 4;

		auto PixelDelta = TimeDelta * FontHeight * LinesPerSecond;
		ChatLinesPixelOffsetY -= PixelDelta;

		if (ChatLinesPixelOffsetY <= 0)
		{
			NumNewlyAddedLines--;
			ChatLinesPixelOffsetY = NumNewlyAddedLines > 0 ? FontHeight + ChatLinesPixelOffsetY : 0;
		}
	}
}

D3DRECT Chat::GetOutputRect() {
	D3DRECT r = { Border.x1, Border.y1, Border.x2, Border.y2 - GetEffectiveLineHeight() };
	return r;
}

D3DRECT Chat::GetInputRect() {
	int effH = GetEffectiveLineHeight();
	D3DRECT r = { Border.x1, Border.y2 - effH, Border.x2, Border.y2 + (InputHeight - 1) * effH };
	return r;
}

D3DRECT Chat::GetTotalRect() {
	D3DRECT r = { Border.x1, Border.y1 - 20, Border.x2, Border.y2 };
	return r;
}

// Converts a D3DRECT, which is specified in terms of the coordinates of each corner,
// to an MRECT, which is specified in terms of the top left coordinate and the extents.
static MRECT MakeMRECT(const D3DRECT& src)
{
	return{
		src.x1,
		src.y1,
		src.x2 - src.x1,
		src.y2 - src.y1,
	};
}

void Chat::OnDraw(MDrawContext* pDC)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		if (HideAlways || (HideDuringReplays && ZGetGame()->IsReplay()))
			return;

		bool ShowAll = ZIsActionKeyPressed(ZACTION_SHOW_FULL_CHAT) && !InputEnabled;
		auto&& Output = GetOutputRect();

		int CeiledLimit, FlooredLimit;
		int effectiveH = GetEffectiveLineHeight();
		if (ShowAll)
		{
			CeiledLimit = FlooredLimit = (Output.y2 - 5) / effectiveH;
		}
		else
		{
			auto Limit = float(Output.y2 - Output.y1 - 10) / effectiveH;
			FlooredLimit = int(Limit);
			CeiledLimit = int(ceil(Limit));
		}

		auto Time = ZGetGame()->GetTime();

		DrawBackground(pDC, Time, NumNewlyAddedLines > 0 ? CeiledLimit : FlooredLimit, ShowAll);
		DrawChatLines(pDC, Time, InputEnabled ? CeiledLimit : FlooredLimit, ShowAll);
		DrawSelection(pDC);

		if (IsInputEnabled()) {
			DrawFrame(pDC, Time);
		}
	}
}

int Chat::DrawTextWordWrap(MFontR2& Font, const WStringView& Str, const D3DRECT& r, u32 Color)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		int Lines = 1;
		int StringLength = int(Str.size());
		int CurrentLineLength = 0;
		int MaxLineLength = r.x2 - r.x1;

		for (int i = 0; i < StringLength; i++)
		{
			int CharWidth = Font.GetWidth(&Str[i], 1);
			int CharHeight = Font.GetHeight();

			if (CurrentLineLength + CharWidth > MaxLineLength)
			{
				CurrentLineLength = 0;
				Lines++;
			}

			auto x = r.x1 + CurrentLineLength;
			auto y = r.y1 + (CharHeight + 1) * max(0, Lines - 1);
			Font.m_Font.DrawTextWSV(x, y, Str.substr(i, 1), Color);

			CurrentLineLength += CharWidth;
		}

		return Lines;
	}
}

void Chat::DrawTextN(MFontR2& pFont, const WStringView& Str, const D3DRECT& r, u32 Color, int nEmojiSet, MDrawContext* pDC)
{
	if (!ZGetGame() || !ZGetConfiguration()->GetEtc()->bNewChatEnable)
		return;

	bool bEmoji = pDC && (nEmojiSet > 0) && ZGetEmojiManager().IsSetLoaded(nEmojiSet);

	if (!bEmoji || Str.size() < 2)
	{
		pFont.m_Font.DrawTextWSV(r.x1, r.y1, Str, Color);
		return;
	}

	int x = r.x1;
	int y = r.y1;
	int emojiSize = ZGetEmojiManager().GetSize(true, nEmojiSet); // ingame size
	int fontH = pFont.GetHeight();
	size_t segStart = 0;

	// Calculate max emoji height in this line for vertical centering
	int maxEmojiH = fontH;
	for (size_t k = 0; k < Str.size(); k++)
	{
		EmojiMatchResult m = ZGetEmojiManager().FindEmojiW(Str.data(), (int)k, (int)Str.size(), nEmojiSet);
		if (m.pEntry)
		{
			int h_dummy, h_prescan;
			ZGetEmojiManager().GetDrawSize(m.pEntry, true, nEmojiSet, h_dummy, h_prescan);
			if (h_prescan > maxEmojiH) maxEmojiH = h_prescan;
			k += m.nPatternLen - 1;
		}
	}

	// Vertically center text within the tallest element
	int textOffsetY = (maxEmojiH > fontH) ? (maxEmojiH - fontH) / 2 : 0;

	for (size_t i = 0; i < Str.size(); i++)
	{
		EmojiMatchResult match = ZGetEmojiManager().FindEmojiW(Str.data(), (int)i, (int)Str.size(), nEmojiSet);
		if (match.pEntry)
		{
			// Draw text before emoji (vertically centered)
			if (i > segStart)
			{
				WStringView pre = Str.substr(segStart, i - segStart);
				pFont.m_Font.DrawTextWSV(x, y + textOffsetY, pre, Color);
				for (size_t j = segStart; j < i; j++)
					x += pFont.GetWidth(&Str[j], 1);
			}

			int w, h;
			ZGetEmojiManager().GetDrawSize(match.pEntry, true, nEmojiSet, w, h);

			// Center emoji vertically within line
			int emojiOffsetY = (maxEmojiH - h) / 2;

			pFont.m_Font.EndFont();

			if (match.pEntry->bAnimated)
			{
				// === ANIMATED GIF EMOJI ===
				AnimatedGifTexture* pAnimTex = ZGetEmojiManager().GetAnimatedTexture(match.pEntry->filename.c_str(), nEmojiSet);
				if (pAnimTex && pAnimTex->IsValid())
				{
					LPDIRECT3DTEXTURE9 pFrameTex = pAnimTex->GetCurrentFrameTexture();
					if (pFrameTex)
					{
						// Draw the current GIF frame directly via D3D
						// We need to use the device directly since MBitmap doesn't wrap raw textures
						LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();
						if (pd3dDevice)
						{
							// Save current render states
							DWORD dwPrevAlphaBlend, dwPrevSrcBlend, dwPrevDestBlend;
							DWORD dwPrevColorOp, dwPrevColorArg1, dwPrevAlphaOp, dwPrevAlphaArg1;
							pd3dDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwPrevAlphaBlend);
							pd3dDevice->GetRenderState(D3DRS_SRCBLEND, &dwPrevSrcBlend);
							pd3dDevice->GetRenderState(D3DRS_DESTBLEND, &dwPrevDestBlend);
							pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwPrevColorOp);
							pd3dDevice->GetTextureStageState(0, D3DTSS_COLORARG1, &dwPrevColorArg1);
							pd3dDevice->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwPrevAlphaOp);
							pd3dDevice->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwPrevAlphaArg1);

							// Setup alpha blending for transparent GIF
							pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
							pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
							pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
							pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
							pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
							pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
							pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

							pd3dDevice->SetTexture(0, pFrameTex);
							pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

							// Textured quad
							struct GIFVERTEX {
								float x, y, z, rhw;
								float u, v;
							};

							float fx = (float)x;
							float fy = (float)(y + emojiOffsetY);
							float fw = (float)w;
							float fh = (float)h;

							GIFVERTEX verts[4] = {
								{ fx,      fy,      0.f, 1.f, 0.f, 0.f },
								{ fx + fw, fy,      0.f, 1.f, 1.f, 0.f },
								{ fx + fw, fy + fh, 0.f, 1.f, 1.f, 1.f },
								{ fx,      fy + fh, 0.f, 1.f, 0.f, 1.f },
							};

							pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, verts, sizeof(GIFVERTEX));

							// Restore previous states
							pd3dDevice->SetTexture(0, nullptr);
							pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwPrevAlphaBlend);
							pd3dDevice->SetRenderState(D3DRS_SRCBLEND, dwPrevSrcBlend);
							pd3dDevice->SetRenderState(D3DRS_DESTBLEND, dwPrevDestBlend);
							pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwPrevColorOp);
							pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, dwPrevColorArg1);
							pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, dwPrevAlphaOp);
							pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwPrevAlphaArg1);
						}
					}
				}
			}
			else
			{
				// === STATIC PNG EMOJI (original behavior) ===
				MBitmap* pBmp = MBitmapManager::Get(match.pEntry->filename.c_str());
				if (pBmp)
				{
					pDC->SetBitmap(pBmp);
					pDC->Draw(x, y + emojiOffsetY, w, h);
				}
			}

			pFont.m_Font.BeginFont();

			x += w;
			i += match.nPatternLen - 1; // -1 because loop does i++
			segStart = i + 1;
			continue;
		}
	}

	// Draw remaining text (vertically centered)
	if (segStart < Str.size())
	{
		WStringView rest = Str.substr(segStart);
		pFont.m_Font.DrawTextWSV(x, y + textOffsetY, rest, Color);
	}
}

void Chat::DrawBorder(MDrawContext* pDC)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		auto rect = Border;
		rect.y2 += (InputHeight - 1) * GetEffectiveLineHeight();

		// Draw the box outline
		D3DXVECTOR2 vs[] = {
			{ float(rect.x1), float(rect.y1) },
			{ float(rect.x2), float(rect.y1) },
			{ float(rect.x2), float(rect.y2) },
			{ float(rect.x1), float(rect.y2) },
		};

		for (size_t i = 0; i < std::size(vs); i++)
		{
			auto a = vs[i];
			auto b = vs[(i + 1) % std::size(vs)];
			pDC->Line(a.x, a.y, b.x, b.y);
		}

		// Draw the line between the output and input
		rect.y2 -= 2;
		rect.y2 -= InputHeight * FontHeight;
		pDC->Line(rect.x1, rect.y2, rect.x2, rect.y2);
	}
}

void Chat::DrawBackground(MDrawContext* pDC, float Time, int Limit, bool ShowAll)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		if (BackgroundColor & 0xFF000000)
		{
			if (!InputEnabled)
			{
				// Need to store this value instead of calculating it every frame
				int Lines = -max(0, NumNewlyAddedLines - 1);
				// i needs to be signed since it terminates on -1
				for (int i = int(Msgs.size() - 1); Lines < Limit && i >= 0; i--)
				{
					auto&& cl = Msgs.at(i);

					if (cl.Time + FadeTime < Time && !ShowAll && !InputEnabled)
						break;

					Lines += cl.GetLines();
				}

				Lines = min(Lines, Limit);

				if (Lines > 0)
				{
					auto&& Output = GetOutputRect();
					D3DRECT Rect = {
						Output.x1,
						Output.y2 - 5 - Lines * GetEffectiveLineHeight(),
						Output.x2,
						Output.y2,
					};

					if (NumNewlyAddedLines > 0) {
						Rect.y1 += ChatLinesPixelOffsetY;
						if (!ShowAll) {
							Rect.y1 = max(Rect.y1, Output.y1);
						}
					}

					pDC->SetColor(BackgroundColor);
					pDC->FillRectangle(MakeMRECT(Rect));
				}
			}
			else
			{
				auto Rect = Border;
				Rect.y2 += (InputHeight - 1) * GetEffectiveLineHeight();

				pDC->SetColor(BackgroundColor);
				pDC->FillRectangle(MakeMRECT(Rect));
			}
		}
	}
}

template <typename T>
struct LineDivisionState
{
	T&& OutputIterator;
	LineSegmentInfo CurLineSegmentInfo;
	int ChatMessageIndex = 0;
	int MsgIndex = 0;
	int Lines = 0;
	int CurrentLinePixelLength = 0;
	u32 CurTextColor;
	u32 CurEmphasis = EmphasisType::Default;

	LineDivisionState(T&& OutputIterator, int ChatMessageIndex, u32 CurTextColor) :
		OutputIterator{ std::forward<T>(OutputIterator) },
		ChatMessageIndex{ ChatMessageIndex },
		CurTextColor{ CurTextColor }
	{
	}

	void AddSegment(bool IsEndOfLine)
	{
		// Compute the length from the distance from the current character index
		// to the one the substring started at.
		CurLineSegmentInfo.LengthInCharacters = MsgIndex - int(CurLineSegmentInfo.Offset);

		// Add this LineSegmentInfo to the vector.
		OutputIterator++ = CurLineSegmentInfo;

		if (IsEndOfLine)
		{
			CurrentLinePixelLength = 0;
			Lines++;
		}

		// Reset to zero-initialized LineSegmentInfo.
		CurLineSegmentInfo = LineSegmentInfo{};
		// Set data.
		CurLineSegmentInfo.ChatMessageIndex = ChatMessageIndex;
		CurLineSegmentInfo.Offset = MsgIndex;
		CurLineSegmentInfo.PixelOffsetX = CurrentLinePixelLength;
		CurLineSegmentInfo.IsStartOfLine = CurrentLinePixelLength == 0;
		CurLineSegmentInfo.TextColor = CurTextColor;
		CurLineSegmentInfo.Emphasis = CurEmphasis;
	}

	void HandleFormatSpecifier(FormatSpecifier& FormatSpec)
	{
		switch (FormatSpec.ft) {
		case FormatSpecifierType::Color:
			CurTextColor = FormatSpec.Color;
			break;

		case FormatSpecifierType::Default:
			CurEmphasis = EmphasisType::Default;
			break;

		case FormatSpecifierType::Bold:
			CurEmphasis |= EmphasisType::Bold;
			break;

		case FormatSpecifierType::Italic:
			CurEmphasis |= EmphasisType::Italic;
			break;

		case FormatSpecifierType::Underline:
			CurEmphasis |= EmphasisType::Underline;
			break;

		case FormatSpecifierType::Strikethrough:
			CurEmphasis |= EmphasisType::Strikethrough;
			break;

		case FormatSpecifierType::Linebreak:
			AddSegment(true);
			return;
		};

		if (MsgIndex - int(CurLineSegmentInfo.Offset) == 0)
		{
			// If MsgIndex - int(CurLineSegmentInfo.Offset) equals zero,
			// the substring would be empty if we were to add a line.
			// Instead, we want to add the modified text attributes to the current segment.
			CurLineSegmentInfo.TextColor = CurTextColor;
			CurLineSegmentInfo.Emphasis = CurEmphasis;
		}
		else
		{
			AddSegment(false);
		}
	}
};

template <typename T>
void Chat::DivideIntoLines(int ChatMessageIndex, T&& OutputIterator)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		auto&& cl = Msgs[ChatMessageIndex];
		// Clear the previous wrapping line breaks, since we're going to add new ones.
		cl.ClearWrappingLineBreaks();

		auto MaxLineLength = (Border.x2 - 5) - (Border.x1 + 5);

		LineDivisionState<T> State{ std::forward<T>(OutputIterator), ChatMessageIndex, cl.DefaultColor };

		// Initialize the first segment.
		State.CurLineSegmentInfo.ChatMessageIndex = ChatMessageIndex;
		State.CurLineSegmentInfo.Offset = 0;
		State.CurLineSegmentInfo.PixelOffsetX = 0;
		State.CurLineSegmentInfo.IsStartOfLine = true;
		State.CurLineSegmentInfo.TextColor = cl.DefaultColor;
		State.CurLineSegmentInfo.Emphasis = EmphasisType::Default;

		auto FormatIterator = cl.FormatSpecifiers.begin();
		for (State.MsgIndex = 0; State.MsgIndex < int(cl.Msg.length()); ++State.MsgIndex)
		{
			// Process all the format specifiers at this index.
			// There may be more than one, so we do a loop.
			while (FormatIterator != cl.FormatSpecifiers.end() &&
				FormatIterator->nStartPos == State.MsgIndex)
			{
				State.HandleFormatSpecifier(*FormatIterator);
				++FormatIterator;
			}

			auto CharWidth = DefaultFont.GetWidth(cl.Msg.data() + State.MsgIndex, 1);

			// If adding this character would make the line length exceed the max,
			// we add a new line for this character to go on.
			if (State.CurrentLinePixelLength + CharWidth > MaxLineLength)
			{
				// ChatMessage::AddWrappingLineBreak returns an iterator to the line break
				// that was inserted, so we want to set FormatIterator to the next one.
				// We do this since the current iterator may have been invalidated by the mutation.
				FormatIterator = std::next(cl.AddWrappingLineBreak(State.MsgIndex));
				State.AddSegment(true);
			}

			State.CurrentLinePixelLength += CharWidth;
		}

		// Add the final segment.
		State.AddSegment(true);

		cl.Lines = State.Lines;
	}
}

MFontR2& Chat::GetFont(u32 Emphasis)
{
	if (Emphasis & EmphasisType::Italic)
		return ItalicFont;

	return DefaultFont;
}

static auto GetDrawLinesRect(const D3DRECT& OutputRect, int LinesDrawn,
	v2i PixelOffset, int FontHeight)
{
	return D3DRECT{
		OutputRect.x1 + 5 + PixelOffset.x,
		OutputRect.y2 - 5 - ((LinesDrawn + 1) * FontHeight) + PixelOffset.y,
		OutputRect.x2 - 5,
		OutputRect.y2 - 5
	};
}

static u32 ScaleAlpha(u32 Color, float MessageTime, float CurrentTime,
	float BeginFadeTime, float EndFadeTime)
{
	auto Delta = CurrentTime - MessageTime;

	auto A = (Color & 0xFF000000) >> 24;
	auto RGB = Color & 0x00FFFFFF;

	if (Delta < BeginFadeTime)
		return Color; // 100% alpha
	if (Delta > EndFadeTime)
		return RGB;   // 0% alpha

	auto Scale = 1 - ((Delta - BeginFadeTime) / (EndFadeTime - BeginFadeTime));
	auto AS = static_cast<u8>(A * Scale);

	return (AS << 24) | RGB;
}

void Chat::DrawChatLines(MDrawContext* pDC, float Time, int Limit, bool ShowAll)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		auto Reverse = [&](auto&& Container, int Offset = 0) {
			return MakeRange(Container.rbegin() + Offset, Container.rend());
			};

		DefaultFont.m_Font.BeginFont();

		auto PrevClipRect = pDC->GetClipRect();
		DWORD dwPrevScissor = FALSE;
		RECT prevScissorRect = {};
		{
			auto ClipRect = GetOutputRect();
			if (ShowAll)
				ClipRect.y1 = 0;
			pDC->SetClipRect(MakeMRECT(ClipRect));

			// Also set D3D scissor rect for GIF emoji (DrawPrimitiveUP bypasses Mint2 clip)
			LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();
			RECT scissor = { ClipRect.x1, ClipRect.y1, ClipRect.x2, ClipRect.y2 };
			if (pd3dDevice)
			{
				pd3dDevice->GetRenderState(D3DRS_SCISSORTESTENABLE, &dwPrevScissor);
				pd3dDevice->GetScissorRect(&prevScissorRect);
				pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
				pd3dDevice->SetScissorRect(&scissor);
			}
		}

		if (ChatLinesPixelOffsetY > 0)
			Limit++;

		auto MessagesOffset = max(0, NumNewlyAddedLines - 1);

		int LinesDrawn = 0;
		int AccumulatedPixelHeight = 0; // track total pixel height of drawn lines
		for (auto&& LineSegment : Reverse(LineSegments, MessagesOffset))
		{
			if (LineSegment.ChatMessageIndex >= Msgs.size())
				continue;
			auto&& cl = Msgs[LineSegment.ChatMessageIndex];

			if (!ShowAll && !InputEnabled && Time > cl.Time + FadeTime)
				break;

			if (cl.Msg.empty() || LineSegment.Offset >= cl.Msg.size())
				continue;
			auto String = cl.Msg.data() + LineSegment.Offset;
			auto Length = LineSegment.LengthInCharacters;
			auto&& Font = GetFont(LineSegment.Emphasis);
			auto Color = LineSegment.TextColor;

			// Calculate actual line height considering emojis
			int lineH = FontHeight;
			if (Z_ETC_EMOTE && ZGetEmojiManager().IsLoaded())
				lineH = ZGetEmojiManager().GetLineHeightW(String, Length, FontHeight, true, cl.nEmojiSet);

			if (!ShowAll && !InputEnabled)
				Color = ScaleAlpha(Color, cl.Time, Time, FadeTime * 0.8f, FadeTime);

			// Position line using accumulated height instead of fixed FontHeight
			v2i PixelOffset{ LineSegment.PixelOffsetX, int(ChatLinesPixelOffsetY) };
			D3DRECT Rect = {
				GetOutputRect().x1 + 5 + PixelOffset.x,
				GetOutputRect().y2 - 5 - AccumulatedPixelHeight - lineH + PixelOffset.y,
				GetOutputRect().x2 - 5,
				GetOutputRect().y2 - 5
			};

			// Skip lines that are above the output area (out of bounds)
			if (!ShowAll && Rect.y1 < GetOutputRect().y1)
			{
				if (LineSegment.IsStartOfLine)
					break; // no more lines will fit
			}
			else
			{
				DrawTextN(Font, { String, Length }, Rect, Color, cl.nEmojiSet, pDC);
			}

			if (LineSegment.IsStartOfLine)
			{
				AccumulatedPixelHeight += lineH + 4; // +4 padding between all lines
				++LinesDrawn;
				if (LinesDrawn >= Limit)
					break;
			}
		}

		pDC->SetClipRect(PrevClipRect);

		// Restore D3D scissor rect
		{
			LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();
			if (pd3dDevice)
			{
				pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, dwPrevScissor);
				pd3dDevice->SetScissorRect(&prevScissorRect);
			}
		}

		DefaultFont.m_Font.EndFont();
	}
}

void Chat::DrawSelection(MDrawContext* pDC)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		if (SelectionState.FromMsg && SelectionState.ToMsg)
		{
			auto ret = GetPos(*SelectionState.FromMsg, SelectionState.FromPos);
			if (!ret.first)
				return;
			auto From = ret.second;

			ret = GetPos(*SelectionState.ToMsg, SelectionState.ToPos);
			if (!ret.first)
				return;
			auto To = ret.second;

			auto ShouldSwap = From.y > To.y || From.y == To.y && From.x > To.x;

			std::pair<const ChatMessage*, int> Stuff;
			if (ShouldSwap)
			{
				std::swap(From, To);
				Stuff = { SelectionState.FromMsg, SelectionState.FromPos };
			}
			else
			{
				Stuff = { SelectionState.ToMsg, SelectionState.ToPos };
			}

			ret = GetPos(*Stuff.first, Stuff.second + 1);
			if (!ret.first)
				return;
			To = ret.second;

			auto Fill = [&](auto&&... Coords) {
				pDC->FillRectangle(MakeMRECT({ Coords... }));
				};

			pDC->SetColor(SelectionColor);
			// Get half of font, adjusted in each direction to compensate for odd font heights.
			// For instance, if the font height is 15, we want to subtract 8 and add 7,
			// since adjusting by a single integer value would make the rectangles we draw
			// overlap each other slightly. (The height would be even when it ought to be odd.)
			auto TopOffset = int(ceil(FontHeight / 2.f));
			auto BottomOffset = FontHeight / 2;
			if (From.y == To.y)
			{
				Fill(From.x,
					From.y - TopOffset,
					To.x,
					To.y + BottomOffset);
			}
			else {
				Fill(From.x,
					From.y - TopOffset,
					Border.x2 - 5,
					From.y + BottomOffset);

				for (int i = FontHeight; i < To.y - From.y; i += FontHeight) {
					Fill(Border.x1 + 5,
						From.y + i - TopOffset,
						Border.x2 - 5,
						From.y + i + BottomOffset);
				}
				Fill(Border.x1,
					To.y - TopOffset,
					To.x - 5,
					To.y + BottomOffset);
			}
		}
	}
}
// Custom: Text New Chat
void Chat::DrawFrame(MDrawContext* pDC, float Time)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		// 1. CHẶN KHÔNG CHO CỬA SỔ TRẠNG THÁI IME CỦA WINDOWS TỰ HIỆN LÊN GÂY RÁC CHỮ KO/EN
		HWND imehwnd = ImmGetDefaultIMEWnd(g_hWnd);
		if (imehwnd) {
			SendMessage(imehwnd, WM_IME_CONTROL, IMC_CLOSESTATUSWINDOW, 0);
		}

		// Draw top of border
		{
			D3DRECT Rect = {
				Border.x1,
				Border.y1 - 20,
				Border.x2 + 1,
				Border.y1
			};

			pDC->SetColor(InterfaceColor);
			pDC->FillRectangle(MakeMRECT(Rect));
		}

		DrawBorder(pDC);

		//-- Draw Restore Window Button text
		{
			D3DRECT Rect = {
				Border.x2 - 15,
				Border.y1 - 18,
				Border.x2 - 15 + 12,
				Border.y1 - 18 + FontHeight,
			};
			DrawTextN(DefaultFont, L"", Rect, TextColor);
		}

		D3DRECT Rect = {
			Border.x1 + 5,
			Border.y2 - 2 - FontHeight,
			Border.x2,
			Border.y2,
		};

		int x = Rect.x1 + CaretCoord.x;
		int y = Rect.y1 + (CaretCoord.y - 1) * FontHeight;

		// Alternate every 0.4 seconds
		auto Period = 0.4f;
		if (fmod(Time, Period * 2) > Period)
		{
			// Draw caret
			pDC->SetColor(TextColor);
			pDC->Line(x, y, x, y + FontHeight);
		}

		DrawTextWordWrap(DefaultFont, InputField.c_str(), Rect, TextColor);

		// =========================================================================
// 🛠️ TỰ ĐỘNG NHẬN DIỆN VÀ VẼ NHÃN ĐA NGÔN NGỮ CHUẨN ĐẸP Ơ GÓC THANH CHAT
// =========================================================================
		HKL currentHKL = GetKeyboardLayout(0);
		WORD langID = LOWORD(currentHKL); // Lấy ID Ngôn ngữ hiện tại

		D3DRECT LangRect = {
			Border.x2 - 35,             // Tọa độ X nằm lui về bên trái nút đóng/mở
			Border.y2 - 2 - FontHeight, // Tọa độ Y thẳng hàng với dòng nhập chat
			Border.x2 - 5,
			Border.y2
		};

		std::wstring strLangLabel = L"EN"; // Mặc định là EN
		DWORD labelColor = TextColor;      // Màu chữ mặc định

		// Bắt chính xác mã các ngôn ngữ Unicode trên thế giới
		switch (langID)
		{
		case 0x042A: // Vietnamese (Bộ gõ tiếng Việt Windows)
			strLangLabel = L"VN";
			labelColor = TextColor;
			break;
		case 0x0412: // Korean (Tiếng Hàn)
			strLangLabel = L"KR";
			labelColor = TextColor;
			break;
		case 0x0411: // Japanese (Tiếng Nhật)
			strLangLabel = L"JP";
			labelColor = TextColor;
			break;
		case 0x0804: // Chinese (Simplified - Tiếng Trung Giản Thể)
		case 0x0C04: // Chinese (Traditional - Tiếng Trung Phồn Thể)
			strLangLabel = L"CN";
			labelColor = TextColor;
			break;
		case 0x041E: // Thai (Tiếng Thái)
			strLangLabel = L"TH";
			labelColor = TextColor;
			break;
		case 0x0439: // Hindi (Tiếng Ấn Độ)
			strLangLabel = L"IN";
			labelColor = TextColor;
			break;
		default:
			// Các ngôn ngữ Latinh khác giữ nguyên EN hoặc tự nhận diện theo phân vùng
			strLangLabel = L"EN";
			labelColor = TextColor;
			break;
		}

		// Tiến hành vẽ nhãn ngôn ngữ linh hoạt lên giao diện NewChat
		DrawTextN(DefaultFont, strLangLabel.c_str(), LangRect, labelColor);
		// =========================================================================
	}
}

void Chat::ResetFonts()
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		DefaultFont.Destroy();
		ItalicFont.Destroy();

		const auto Scale = 1.f;
		DefaultFont.Create("NewChatFont", FontName.c_str(),
			int(float(FontSize) / 1080 * RGetScreenHeight() + 0.5), Scale, BoldFont);
		ItalicFont.Create("NewChatItalicFont", FontName.c_str(),
			int(float(FontSize) / 1080 * RGetScreenHeight() + 0.5), Scale, BoldFont, true);

		FontHeight = DefaultFont.GetHeight();
	}
}

int Chat::GetEffectiveLineHeight() const
{
	int h = FontHeight;
	if (Z_ETC_EMOTE && ZGetEmojiManager().IsLoaded())
	{
		int emojiH = ZGetEmojiManager().GetMaxSize(true); // max ingame size across all sets
		if (emojiH > h) h = emojiH;
	}
	return h;
}

//void Chat::SetBackgroundColor()
//{
//	switch(Z_CHAT_BACKGROUND_COLOR)
//	{ 
//		case 1:
//			BackgroundColor = CHAT_DEFAULT_BACKGROUND_COLOR;
//			break;
//		case 2:
//			BackgroundColor = CHAT_BACKGROUND_COLOR_T_BLACK;
//			break;
//		case 3:
//			BackgroundColor = CHAT_BACKGROUND_COLOR_T_WHITE;
//			break;
//		case 4:
//			BackgroundColor = CHAT_BACKGROUND_COLOR_O_BLACK;
//			break;
//		case 5:
//			BackgroundColor = CHAT_BACKGROUND_COLOR_O_WHITE;
//			break;
//		default:
//		{
//			BackgroundColor = CHAT_DEFAULT_BACKGROUND_COLOR;
//			break;
//		}
//	}
//}
//
//D3DCOLOR Chat::SetBGColor(int nVal)
//{
//	switch (nVal)
//	{
//	case 1:
//		BackgroundColor = CHAT_DEFAULT_BACKGROUND_COLOR;
//		break;
//	case 2:
//		BackgroundColor = CHAT_BACKGROUND_COLOR_T_BLACK;
//		break;
//	case 3:
//		BackgroundColor = CHAT_BACKGROUND_COLOR_T_WHITE;
//		break;
//	case 4:
//		BackgroundColor = CHAT_BACKGROUND_COLOR_O_BLACK;
//		break;
//	case 5:
//		BackgroundColor = CHAT_BACKGROUND_COLOR_O_WHITE;
//		break;
//	default:
//	{
//		BackgroundColor = CHAT_DEFAULT_BACKGROUND_COLOR;
//		mlog("Couldn't find your option for background color chat, setting default one.\n");
//		break;
//	}
//	}
//
//	return BackgroundColor;
//}