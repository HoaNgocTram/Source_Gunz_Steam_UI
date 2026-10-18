#pragma once
#include "stdafx.h"
#include "ZFunctions.h"
#include <Windows.h>
#define ALLOW_EAC 1
// định nghĩa ở top của file hoặc trong settings compile: #define ALLOW_EAC 1

#ifdef ALLOW_EAC
bool IsAllowedModule(const char* name)
{
	if (!name) return false;
	// EAC / EOS modules whitelist
	if (_stricmp(name, "start_protected_game.exe") == 0) return true;
	if (_stricmp(name, "Gunz.exe.local") == 0) return true;
	if (_stricmp(name, "easydrv.sys") == 0) return true;
	if (_stricmp(name, "EOSSDK-Win32-Shipping.dll") == 0) return true;
	if (_stricmp(name, "EOSSDK-Win64-Shipping.dll") == 0) return true;
	// EAC runtime DLLs
	if (_strnicmp(name, "EasyAntiCheat", 13) == 0) return true;
	return false;
}
#endif

#ifdef _ANTIHACK
bool IsAddressHooked(uintptr_t address)
{
	if (!address) return false;
	BYTE* pByte = (BYTE*)address;
#ifdef _M_X64
	if (pByte[0] == 0xFF && pByte[1] == 0x25) return true;   // jmp [rip+disp32]
	if (pByte[0] == 0x48 && pByte[1] == 0xB8) return true;   // mov rax, imm64 (trampoline)
	if (pByte[0] == 0xE9) return true;                        // jmp rel32
	if (pByte[0] == 0xE8) return true;                        // call rel32
#else
	if (*pByte == 0xE8 || *pByte == 0xE9 || *pByte == 0x7E || *pByte == 0x74 || *pByte == 0xFF) return true;
#endif
	return false;
}

DWORD WINAPI StreakGuard(LPVOID lpParam)
{
	UNREFERENCED_PARAMETER(lpParam);

	uintptr_t getTickCount = (uintptr_t)GetProcAddress(GetModuleHandleA(XorStr<0xC2, 13, 0xC8D01D45>("\xA9\xA6\xB6\xAB\xA3\xAB\xFB\xFB\xE4\xAF\xA0\xA1" + 0xC8D01D45).s), XorStr<0x7E, 13, 0x39677FD3>("\x39\x1A\xF4\xD5\xEB\xE0\xEF\xC6\xE9\xF2\xE6\xFD" + 0x39677FD3).s);
	uintptr_t queryPerformanceCounter = (uintptr_t)GetProcAddress(GetModuleHandleA(XorStr<0xCB, 13, 0x78928F39>("\xA0\xA9\xBF\xA0\xAA\xBC\xE2\xE0\xFD\xB0\xB9\xBA" + 0x78928F39).s), XorStr<0x15, 24, 0x2E187D3F>("\x44\x63\x72\x6A\x60\x4A\x7E\x6E\x7B\x71\x6D\x4D\x40\x4C\x40\x41\x66\x49\x52\x46\x5D\x4F\x59" + 0x2E187D3F).s);
	
	while (true)
	{
		// --- Hook check (timing APIs) ---
		bool bHooked = IsAddressHooked(getTickCount) || IsAddressHooked(queryPerformanceCounter);

		// --- DLL inject check ---
		const char* dllChecks[] = {
			XorStr<0xB6, 10, 0x38B0BE6A>("\xC6\xC5\xD7\xC1\xC3\x95\xD8\xD1\xD2" + 0x38B0BE6A).s,
			XorStr<0xF5, 13, 0xE518A4D8>("\x9C\x98\x9D\x9D\x9A\x8E\x94\x8E\xD3\x9A\x93\x6C" + 0xE518A4D8).s,
			XorStr<0xDA, 12, 0x16342B7F>("\xB1\xB5\xEF\xB9\xAC\xB6\x90\xCF\x86\x8F\x88" + 0x16342B7F).s,
			XorStr<0x18, 12, 0xD90D718C>("\x6B\x71\x75\x6F\x7E\x72\x6A\x31\x44\x4D\x4E" + 0xD90D718C).s,
			XorStr<0x89, 24, 0x457ABD19>("\xCA\xF8\xEA\xE5\xEA\xA9\xFC\xB0\xB9\xD0\xF6\xF5\xE6\xE2\xBE\xB8\xDC\xC9\xCB\xB2\xF9\xF2\xF3" + 0x457ABD19).s,
			XorStr<0x63, 26, 0x7ED7050B>("\x2D\x2F\x37\x46\x32\x06\x00\x1C\x0E\x1E\x1E\x0F\x03\x5E\x22\x1A\x1C\x00\x01\x1F\x12\x56\x1D\x16\x17" + 0x7ED7050B).s,
			XorStr<0xB3, 13, 0x55ABBCDD>("\xC0\xDC\xD0\xDA\xDB\xD9\xC9\xD3\x95\xD8\xD1\xD2" + 0x55ABBCDD).s,
			XorStr<0x0A, 14, 0xD156CB07>("\x59\x7B\x69\x68\x6A\x47\x71\x72\x79\x3D\x70\x79\x7A" + 0xD156CB07).s,
			XorStr<0x97, 15, 0x396C08BB>("\xC0\xF9\xF5\xF6\xD3\xFD\xFE\xF5\xC0\x91\x8F\xC6\xCF\xC8" + 0x396C08BB).s,
			XorStr<0xBB, 15, 0x77CCCBB1>("\xEC\xDD\xD1\xD2\xF7\xA1\xA2\xA9\x9C\xF6\xEB\xA2\xAB\xA4" + 0x77CCCBB1).s,
			XorStr<0xAF, 15, 0x727D0191>("\xF8\xD1\xDD\xDE\xFB\xD5\xD6\xDD\xE8\x8B\x97\xDE\xD7\xD0" + 0x727D0191).s,
			XorStr<0x0D, 14, 0x4CD3BD22>("\x20\x23\x22\x3D\x3C\x3F\x3E\x39\x48\x38\x73\x74\x75" + 0x4CD3BD22).s,
			XorStr<0xFC, 14, 0x8D67A660>("\xD1\xD0\xD3\xD2\x2D\x2C\x2F\x2E\x29\x2B\x62\x6B\x64" + 0x8D67A660).s,
			XorStr<0x14, 13, 0x3898A07F>("\x7D\x7B\x7C\x72\x7B\x6D\x75\x69\x32\x79\x72\x73" + 0x3898A07F).s,
			XorStr<0x06, 15, 0x02E119B6>("\x51\x66\x64\x65\x42\x6A\x6F\x66\x51\x3C\x3E\x75\x7E\x7F" + 0x02E119B6).s,
			XorStr<0x8F, 15, 0x508908C7>("\xD8\xF1\xFD\xFE\xDB\xF5\xF6\xFD\xC8\xAA\xB7\xFE\xF7\xF0" + 0x508908C7).s,
			XorStr<0xE9, 15, 0xADAD13E0>("\xBE\x8B\x87\x80\xA5\x8F\x8C\x9B\xAE\xC3\xDD\x90\x99\x9A" + 0xADAD13E0).s,
			XorStr<0xB1, 13, 0x97E2A918>("\xC2\xDA\xD6\xD8\xD9\xD7\xC7\xD1\x97\xDE\xD7\xD0" + 0x97E2A918).s,
			XorStr<0xE1, 12, 0xB73926E8>("\x8A\x8C\xD0\x80\x97\x8F\x97\xC6\x8D\x86\x87" + 0xB73926E8).s,
			XorStr<0x5E, 11, 0x5E3A4723>("\x3A\x3E\x0E\x08\x07\x0F\x4A\x01\x0A\x0B" + 0x5E3A4723).s,
			XorStr<0x12, 9, 0x02C658DB>("\x7A\x7C\x7B\x7E\x38\x73\x74\x75" + 0x02C658DB).s,
		};

		bool bBadDll = false;
		for (int i = 0; i < _countof(dllChecks); i++)
		{
			HMODULE hMod = GetModuleHandleA(dllChecks[i]);
			if (hMod != NULL)
			{
#ifdef ALLOW_EAC
				// Lấy tên file từ module path để check whitelist
				char modPath[MAX_PATH] = { 0 };
				if (GetModuleFileNameA(hMod, modPath, MAX_PATH) > 0)
				{
					const char* modName = strrchr(modPath, '\\');
					modName = modName ? modName + 1 : modPath;
					if (IsAllowedModule(modName)) continue;
				}
#endif
				bBadDll = true;
				break;
			}
		}

		if (bHooked || bBadDll)
		{
			char guard[128];
			sprintf(guard, "[Anti-Cheat]: No use dll, most likely, you are automatically banned, avoid using third-party programs.");
			mlog("%s", guard);
#ifdef _HWID
			if (ZGetGameInterface()) 
			{
				ZPostBanMe(WHT_INJECT);
			}
			else 
			{
				char Log[64];
				sprintf(Log, EXIT_TEXT);
				mlog("%s\n", Log);
				ExitProcess(NULL);
			}
#else
			mlog("[AntiCheat-TRIGGER] StreakGuard: INJECT_DLL_CHECK\n");
			Sleep(500);
			ExitProcess(NULL);
#endif
		}
		Sleep(3000);
	}
	return 0;
}

DWORD WINAPI Scanner(LPVOID lpParam)
{
	UNREFERENCED_PARAMETER(lpParam);

	CRITICAL_SECTION m_cs;
	InitializeCriticalSection(&m_cs);
	EnterCriticalSection(&m_cs);

	// Đọc .text section từ PE header — hoạt động cả x86 và x64
	HMODULE hModule = GetModuleHandle(NULL);
	BYTE* pBase = (BYTE*)hModule;
	IMAGE_DOS_HEADER* pDos = (IMAGE_DOS_HEADER*)pBase;
	IMAGE_NT_HEADERS* pNt = (IMAGE_NT_HEADERS*)(pBase + pDos->e_lfanew);
	IMAGE_SECTION_HEADER* pText = IMAGE_FIRST_SECTION(pNt);

	uintptr_t Start = (uintptr_t)hModule + pText->VirtualAddress;
	size_t textSize = pText->Misc.VirtualSize;

	// Tìm vùng code thực tế (bỏ padding 0x00 ở cuối)
	size_t EndAddy = textSize;
	int cnt2 = 0;
	for (size_t s = 0; s < textSize; s++)
	{
		unsigned char n = *((unsigned char*)(Start + s));
		if (n == 0x00) cnt2++; else cnt2 = 0;
		if (cnt2 >= 16)
		{
			EndAddy = s - 15; // lùi lại trước chuỗi 0x00
			break;
		}
	}

	if (EndAddy == 0)
	{
		LeaveCriticalSection(&m_cs);
		DeleteCriticalSection(&m_cs);
		return 0;
	}

	PBYTE Mem = new BYTE[EndAddy];
	memcpy(Mem, (LPVOID)Start, EndAddy);
	DWORD TimeTest1 = 0, TimeTest2 = 0;
	LeaveCriticalSection(&m_cs);

	while (true)
	{
		EnterCriticalSection(&m_cs);
		if (memcmp(Mem, (LPVOID)Start, EndAddy) != 0)
		{
			mlog("Memory is Different\n");
			LeaveCriticalSection(&m_cs);
			ExitProcess(0); // thay vì *(int*)0 = 0 gây crash dump không cần thiết
		}
		TimeTest1 = TimeTest2;
		TimeTest2 = timeGetTime();
		if (TimeTest1 != 0)
		{
			if ((TimeTest2 - TimeTest1) > 110000)
			{
				LeaveCriticalSection(&m_cs);
				ExitProcess(0);
			}
		}
		LeaveCriticalSection(&m_cs);
		Sleep(24000);
	}
	return 0;
}
#endif