#include "stdafx.h"
#include "ZTranslator.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>

ZTranslator::ZTranslator()
{
}

ZTranslator::~ZTranslator()
{
}

bool ZTranslator::IsTranslationRequest(const char* szMsg, std::string& fromLang, std::string& toLang, std::string& text)
{
	if (!szMsg || strlen(szMsg) < 5) return false;

	std::string msg(szMsg);

	size_t colonPos = msg.find(':');
	if (colonPos == std::string::npos || colonPos < 2) return false;

	size_t spacePos = msg.find(' ', colonPos);
	if (spacePos == std::string::npos || spacePos < 4) return false;

	if (colonPos != 2) return false;
	if (spacePos != 5) return false;

	fromLang = msg.substr(0, 2);
	toLang = msg.substr(3, 2);
	text = msg.substr(6);

	std::transform(fromLang.begin(), fromLang.end(), fromLang.begin(), ::tolower);
	std::transform(toLang.begin(), toLang.end(), toLang.begin(), ::tolower);

	size_t start = text.find_first_not_of(" \t");
	size_t end = text.find_last_not_of(" \t");
	if (start != std::string::npos && end != std::string::npos)
	{
		text = text.substr(start, end - start + 1);
	}

	return !text.empty();
}

std::string ZTranslator::UrlEncode(const std::string& str)
{
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (size_t i = 0; i < str.length(); i++)
	{
		unsigned char c = str[i];

		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
		{
			escaped << c;
		}
		else if (c == ' ')
		{
			escaped << '+';
		}
		else
		{
			escaped << '%' << std::uppercase;
			escaped << std::setw(2) << int((unsigned char)c);
			escaped << std::nouppercase;
		}
	}

	return escaped.str();
}

std::string ZTranslator::Utf8ToAnsi(const std::string& utf8)
{
	if (utf8.empty()) return "";

	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
	if (wchars_num == 0) return utf8;

	wchar_t* wstr = new wchar_t[wchars_num];
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wstr, wchars_num);

	int ansi_num = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	if (ansi_num == 0)
	{
		delete[] wstr;
		return utf8;
	}

	char* ansi = new char[ansi_num];
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, ansi, ansi_num, NULL, NULL);

	std::string result(ansi);
	delete[] wstr;
	delete[] ansi;

	return result;
}

// 🛠️ HÀM BỔ SUNG: Biến chuỗi UTF-8 sạch thành dạng mã hóa \uXXXX giống hệt MyMemory
std::string Utf8ToUnicodeEscape(const std::string& utf8Str)
{
	if (utf8Str.empty()) return "";

	// 1. Chuyển UTF-8 sang WideChar (Unicode UTF-16)
	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
	if (wchars_num <= 0) return utf8Str;

	std::wstring wstr(wchars_num, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wstr[0], wchars_num);

	// 2. Duyệt từng ký tự WideChar để ép thành chuỗi "\uXXXX"
	std::ostringstream ss;
	// wchars_num - 1 để bỏ qua ký tự kết thúc chuỗi '\0'
	for (int i = 0; i < wchars_num - 1; ++i)
	{
		wchar_t wc = wstr[i];
		// Nếu là ký tự ASCII chuẩn (chữ tiếng Anh, số, khoảng trắng) thì giữ nguyên
		if (wc >= 0x0020 && wc <= 0x007E)
		{
			ss << (char)wc;
		}
		else // Ký tự đặc biệt, tiếng Việt, Trung, Nhật, Hàn -> chuyển sang \uXXXX
		{
			ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)wc;
		}
	}

	return ss.str();
}

// 🛠️ HÀM CẢI TIẾN: Bóc tách chuỗi dịch từ API Google (Xử lý mảng chuỗi JSON thô)
std::string ZTranslator::ExtractTranslation(const std::string& jsonResponse)
{
	if (jsonResponse.empty()) return "";

	// Bóc tách mảng của Google API
	size_t startPos = jsonResponse.find("[[[\"");
	if (startPos == std::string::npos)
	{
		startPos = jsonResponse.find("\"");
		if (startPos == std::string::npos) return "";
		startPos += 1;
	}
	else
	{
		startPos += 4;
	}

	size_t endPos = jsonResponse.find("\"", startPos);
	if (endPos == std::string::npos) return "";

	std::string result = jsonResponse.substr(startPos, endPos - startPos);

	// Dọn dẹp ký tự escape mặc định của JSON
	size_t pos = 0;
	while ((pos = result.find("\\n", pos)) != std::string::npos) {
		result.replace(pos, 2, "\n");
		pos += 1;
	}
	pos = 0;
	while ((pos = result.find("\\\"", pos)) != std::string::npos) {
		result.replace(pos, 2, "\"");
		pos += 1;
	}

	// 🛠️ ĐI ĐƯỜNG VÒNG THẦN THÁNH: Biến "Xin chào" thành "Xin ch\u00e0o", "你好" thành "\u4f60\u597d"
	result = Utf8ToUnicodeEscape(result);

	return result;
}

bool ZTranslator::HttpGet(const std::string& url, std::string& response)
{
	HINTERNET hInternet = NULL;
	HINTERNET hConnect = NULL;
	bool success = false;

	hInternet = InternetOpenA("GunZ Translator/1.0",
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL, NULL, 0);

	if (!hInternet)
		return false;

	DWORD timeout = 2000;
	InternetSetOptionA(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
	InternetSetOptionA(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

	hConnect = InternetOpenUrlA(hInternet,
		url.c_str(),
		NULL, 0,
		INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE,
		0);

	if (hConnect)
	{
		char buffer[4096];
		DWORD bytesRead = 0;
		response.clear();

		while (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0)
		{
			buffer[bytesRead] = '\0';
			response += buffer;
		}

		success = !response.empty();
		InternetCloseHandle(hConnect);
	}

	InternetCloseHandle(hInternet);
	return success;
}

// 🛠️ HÀM CẢI TIẾN: Gọi API Google Dịch Không Giới Hạn
bool ZTranslator::TranslateWithAPI(const std::string& fromLang, const std::string& toLang,
	const std::string& text, std::string& outTranslated)
{
	std::string encodedText = UrlEncode(text);

	// Sử dụng Endpoint dạng single-page ổn định nhất của Google (Bypass mọi loại khóa captcha)
	std::string url = "https://translate.googleapis.com/translate_a/single?client=gtx&sl="
		+ fromLang + "&tl=" + toLang + "&dt=t&q=" + encodedText;

	std::string response;

	// Khởi tạo WinINet thủ công để chèn USER-AGENT giả lập trình duyệt Chrome xịn
	HINTERNET hInternet = InternetOpenA("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
		INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if (!hInternet) return false;

	DWORD timeout = 3000; // Tăng lên 3 giây cho chắc chắn nếu mạng lag
	InternetSetOptionA(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
	InternetSetOptionA(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

	HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);

	if (hConnect)
	{
		char buffer[4096];
		DWORD bytesRead = 0;
		response.clear();

		while (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0)
		{
			buffer[bytesRead] = '\0';
			response += buffer;
		}
		InternetCloseHandle(hConnect);
	}
	InternetCloseHandle(hInternet);

	if (response.empty()) return false;

	outTranslated = ExtractTranslation(response);
	return !outTranslated.empty();
}

void ZTranslator::FormatTranslatedMessage(const std::string& translated, char* outMsg, int maxLen)
{
	_snprintf(outMsg, maxLen, "[trans] %s", translated.c_str());
	outMsg[maxLen - 1] = '\0';
}