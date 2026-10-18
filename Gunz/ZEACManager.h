#include "stdafx.h"
#include <windows.h>
#include <string>
#include <thread>
#include "eos_platform_prereqs.h"
#include "eos_sdk.h"
#include "eos_common.h"
#include "eos_anticheatclient.h"
#include "eos_anticheatclient_types.h"
#include "eos_anticheatcommon_types.h"
#include "eos_auth.h"
#include "eos_auth_types.h"
#include "eos_metrics.h"
#include "eos_metrics_types.h"
#include "../sdk/xor/include/xorstr.h"

#define EAC_PRODUCT_ID XorStr<0x80,33,0x0BC2D684>("\xE1\xB8\xB3\xB4\xB6\xBC\xB7\xB4\xB8\xEC\xE8\xB2\xB8\xB4\xEB\xEE\xF1\xA3\xA0\xF6\xF2\xA7\xF3\xA4\xFA\xFD\xAD\xFE\xFD\xA8\xFD\xFD" + 0x0BC2D684).s
#define EAC_SANDBOX_ID XorStr<0xDE,33,0x1738D54E>("\xE8\xEE\x81\x87\x80\xD6\x81\x83\xD3\xD1\xDE\xD8\xDE\xD3\x8E\xDD\x8C\x89\x94\x95\x93\xC0\x96\x97\x97\xCF\x9A\xC1\xCA\xCF\xCC\xCA" + 0x1738D54E).s
#define EAC_DEPLOYMENT_ID XorStr<0x66,33,0x41E8732D>("\x55\x02\x5B\x50\x0F\x5E\x5E\x5A\x0C\x0A\x16\x47\x46\x12\x10\x45\x4F\x42\x1C\x4C\x1B\x1E\x49\x4F\x49\x47\xB1\xE5\xB1\xE0\xBD\xB0" + 0x41E8732D).s
#define EAC_CLIENT_ID   XorStr<0xC9,33,0x5503B46E>("\xB1\xB3\xB1\xAD\xFA\xF6\xF6\xE1\xB6\xA7\x84\xBB\x97\x9B\x87\x9C\x8A\x94\x8A\xAC\xE5\x9D\xBA\x89\x8E\xD3\xA4\xA8\x94\xB2\xBF\xDB" + 0x5503B46E).s
#define EAC_CLIENT_SECRET   XorStr<0x1D,44,0x21C94A77>("\x77\x2E\x74\x54\x62\x64\x66\x61\x57\x76\x5D\x47\x41\x4E\x7E\x03\x49\x5D\x44\x69\x7F\x58\x18\x42\x6F\x58\x54\x60\x51\x5D\x73\x08\x5F\x6E\x5C\x17\x17\x7A\x15\x25\x2C\x01\x0E" + 0x21C94A77).s

class ZEACManager {
public:
    static ZEACManager& GetInstance();

    // Khởi chạy Launcher (start_protected_game.exe)
    bool LaunchProtectedInstance();

    // Khởi tạo SDK (EOS)
    bool InitializeSDK();

    // Cập nhật trạng thái (gọi trong vòng lặp game)
    void Tick();

    void Shutdown();

private:
    EOS_HPlatform m_hPlatform = nullptr;
    EOS_HAntiCheatClient m_hAntiCheat = nullptr;

    ZEACManager() {}

    std::string DecryptString(const std::vector<unsigned char>& ciphertext, unsigned char key) {
        std::string plaintext = "";
        for (unsigned char b : ciphertext) {
            plaintext += (char)(b ^ key); // Đảo ngược phép XOR để lấy lại ký tự gốc
        }
        return plaintext;
    }
};
