#include "stdafx.h"
#include "WebviewTokenHelper.h"
#include "ZCharacter.h"
#include "ZCharacterManager.h"
#include "ZGameClient.h"
#include "sha256.h"
#include <sstream>
#include <iomanip>
#include <cctype>
#include <string>
#include <cstdio>
std::string sha256(const std::string& data)
{
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    BYTE hash[32];
    DWORD hashLen = 32;

    CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
    CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash);
    CryptHashData(hHash, (BYTE*)data.c_str(), data.size(), 0);
    CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0);
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    std::ostringstream oss;
    for (int i = 0; i < 32; i++)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];

    return oss.str();
}

static const char* CLIENT_SECRET = "MY_SECRET_CID_KEY_123";  // phải trùng PHP

// URL encode
std::string UrlEncode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (unsigned char c : value) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        }
        else {
            escaped << '%' << std::uppercase << std::setw(2) << int(c);
            escaped << std::nouppercase;
        }
    }
    return escaped.str();
}

// ===============================
// 🔵 Tạo token theo CharName
// ===============================
std::string MakeCharNameToken(const std::string& name)
{
    std::string raw = name + "|" + CLIENT_SECRET;
    std::string hash = sha256(raw);
    return name + ":" + hash;
}

// ===============================
// 🔵 Lấy CharName người chơi đang điều khiển
// ===============================
const char* GetMyCharName()
{
    // 1) Use MyInfo (safe in lobby & ingame)
    if (ZGetMyInfo() && ZGetMyInfo()->GetCharName())
        return ZGetMyInfo()->GetCharName();

    // 2) Fallback: ingame ZCharacter
    ZCharacter* me = ZGetCharacterManager()->Find(ZGetMyUID());
    if (me && me->GetProperty())
        return me->GetProperty()->GetName();

    return "Unknown";
}

