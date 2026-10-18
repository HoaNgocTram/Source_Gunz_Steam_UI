#ifndef ZGUARD_H
#define ZGUARD_H

#pragma once
#include "CriticalSection.h"


#ifndef _ABHI_SPLASH_H_
#define _ABHI_SPLASH_H_

#include "windows.h"

class CSplash
{
public:

	CSplash();
	CSplash(LPCTSTR lpszFileName, COLORREF colTrans);
	virtual ~CSplash();
	void ShowSplash();
	int DoLoop();
	int CloseSplash();
	DWORD SetBitmap(LPCTSTR lpszFileName);
	DWORD SetBitmap(HBITMAP hBitmap);
	bool SetTransparentColor(COLORREF col);
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HWND m_hwnd;

private:
	void Init();
	void  OnPaint(HWND hwnd);
	bool MakeTransparent();
	HWND RegAndCreateWindow();
	COLORREF m_colTrans;
	DWORD m_dwWidth;
	DWORD m_dwHeight;
	void FreeResources();
	HBITMAP m_hBitmap;
	LPCTSTR m_lpszClassName;
};
void SplashShow();

#endif 

#define IS_VALID_HANDLE(handle)          (handle&& handle != INVALID_HANDLE_VALUE)
#ifdef _ANTIHACK
class MemoryManagement {
public:
	MemoryManagement(HMODULE baseAddress, size_t addressSize) {
#if !defined(_CRITICALSECTION)
		_cs = new CriticalSection();
#endif
		if (baseAddress != NULL && addressSize > 0)
		{
			this->m_hBaseAddress = baseAddress;
			this->m_nAddressSize = addressSize;
			this->m_pMemory = new BYTE[addressSize];
			memcpy(this->m_pMemory, (LPVOID)baseAddress, addressSize);
		}
		else
		{
			this->m_hBaseAddress = NULL;
			this->m_nAddressSize = 0;
			this->m_pMemory = NULL;
		}
	}

	~MemoryManagement() {
		if (m_pMemory) {
			delete[] m_pMemory;
			m_pMemory = NULL;
		}
#if !defined(_CRITICALSECTION)
		if (_cs) {
			delete _cs;
			_cs = NULL;
		}
#endif
	}

	bool MemoryEdited() {
		if (!m_pMemory || !m_hBaseAddress) return false;
#if !defined(_CRITICALSECTION)
		_cs->Enter();
#endif
		bool bRet = memcmp(this->m_pMemory, (LPVOID)this->m_hBaseAddress, this->m_nAddressSize) != 0;
#if !defined(_CRITICALSECTION)
		_cs->Exit();
#endif
		return bRet;
	}

	void ModifyBuffer(uintptr_t address, PBYTE value, size_t valueSize) {
		if (!m_pMemory || !m_hBaseAddress) return;
#if !defined(_CRITICALSECTION)
		_cs->Enter();
#endif
		SIZE_T bytesWrote = 0;
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)address, value, valueSize, &bytesWrote);

		uintptr_t offset = address - (uintptr_t)this->m_hBaseAddress;

		for (size_t i = 0; i < valueSize; ++i)
		{
			this->m_pMemory[offset + i] = value[i];
		}
#if !defined(_CRITICALSECTION)
		_cs->Exit();
#endif
	}

private:
	PBYTE m_pMemory;
	HMODULE m_hBaseAddress;
	size_t m_nAddressSize;
#if !defined(_CRITICALSECTION)
	CriticalSection* _cs;
#endif
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// MEMORY SCAN
// Đọc .text section size từ PE header runtime — hoạt động cả x86 và x64
inline size_t GetTextSectionSize(HMODULE hModule) {
	if (!hModule) return 0;
	BYTE* pBase = (BYTE*)hModule;
	IMAGE_DOS_HEADER* pDos = (IMAGE_DOS_HEADER*)pBase;
	if (pDos->e_magic != IMAGE_DOS_SIGNATURE) return 0;
	IMAGE_NT_HEADERS* pNt = (IMAGE_NT_HEADERS*)(pBase + pDos->e_lfanew);
	if (pNt->Signature != IMAGE_NT_SIGNATURE) return 0;
	// Section đầu tiên (.text) nằm ngay sau Optional Header
	IMAGE_SECTION_HEADER* pSection = IMAGE_FIRST_SECTION(pNt);
	// Trả về kích thước thực tế của section .text
	return pSection->Misc.VirtualSize;
}

#define SCAN_START GetModuleHandle(NULL)
#define CODE_SIZE  GetTextSectionSize(SCAN_START)

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ANTI UC
#define KERNEL32 XorStr<0x14, 13, 0x1DBC6B28>("\x7F\x70\x64\x79\x7D\x75\x29\x29\x32\x79\x72\x73" + 0x1DBC6B28).s
#define GETTICKCOUNT XorStr<0x78, 13, 0x4D18A169>("\x3F\x1C\x0E\x2F\x15\x1E\x15\x3C\xEF\xF4\xEC\xF7" + 0x4D18A169).s
#define QUERYPERFORMANCECOUNTER XorStr<0x09, 24, 0xAB9D4BC8>("\x58\x7F\x6E\x7E\x74\x5E\x6A\x62\x77\x7D\x61\x79\x74\x78\x74\x7D\x5A\x75\x6E\x72\x69\x7B\x6D" + 0xAB9D4BC8).s
#define WINMM XorStr<0x10, 10, 0xDEFA4C41>("\x67\x78\x7C\x7E\x79\x3B\x72\x7B\x74" + 0xDEFA4C41).s
#define TIMEGETTIME XorStr<0x2A, 12, 0xB4EDE7B0>("\x5E\x42\x41\x48\x69\x4A\x44\x65\x5B\x5E\x51" + 0xB4EDE7B0).s


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// HOOKS
#define HOOKDLL XorStr<0xA9, 9, 0x487237BD>("\xC1\xC5\xC4\xC7\x83\xCA\xC3\xDC" + 0x487237BD).s
#define SOFTSPDLL XorStr<0x54,11,0xA47C9DA9>("\x27\x3A\x30\x23\x2B\x29\x74\x3F\x30\x31"+0xA47C9DA9).s
#define SPEEDHACKDLL XorStr<0x67,14,0xD72A6F4F>("\x34\x18\x0C\x0F\x0F\x24\x0C\x0D\x04\x5E\x15\x1E\x1F"+0xD72A6F4F).s
#define MYHOOKDLL XorStr<0xED,14,0xC3FDE9AD>("\xA0\x97\xA7\x9F\x9E\x99\xB7\x98\x99\xD8\x93\x94\x95"+0xC3FDE9AD).s
#define WIREFRAMEDLL XorStr<0xBD,12,0xB5B1C3F7>("\xD6\xD0\x8C\xA4\xB3\xAB\xB3\xEA\xA1\xAA\xAB"+0xB5B1C3F7).s

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// MEMORY VIEWER
#define MEMORYVIEWER XorStr<0x6A,14,0x97460719>("\x27\x0E\x01\x02\x1C\x16\x50\x27\x1B\x16\x03\x10\x04"+0x97460719).s
#define MACROLG	XorStr<0xFF, 15, 0xB452D525>("\xB3\x6F\x66\x6B\x77\x61\x66\x6E\x27\x4F\x29\x42\x5E\x4E" + 0xB452D525).s
#define MACRORZ XorStr<0xCB, 14, 0x62E5CFDC>("\x99\xAD\xB7\xAB\xBD\xF0\x82\xAB\xBD\xB5\xA5\xA5\xB2" + 0x62E5CFDC).s
#define MACROCS XorStr<0xE4, 5, 0x46035327>("\x8D\xA6\xB3\xA2" + 0x46035327).s

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROGRAMS
#define AUTOITV3 XorStr<0x0A,10,0x6787F189>("\x4B\x7E\x78\x62\x47\x7B\x30\x67\x21"+0x6787F189).s
#define THUNDERRT6FORMDC XorStr<0x3E,17,0xDFD8207D>("\x6A\x57\x35\x2F\x26\x26\x36\x17\x12\x71\x0E\x26\x38\x26\x08\x0E"+0xDFD8207D).s
#define DXKSAUTO XorStr<0xCE,11,0x0B5BDA83>("\xAA\x97\xBB\xF6\xA1\xF3\x95\xA0\xA2\xB8"+0x0B5BDA83).s
#define SBMUTEX XorStr<0x5E,19,0xB004D733>("\x06\x09\x32\x3E\x34\x36\x2A\x30\x39\x38\x3B\x2B\x35\x26\x39\x39\x2B\x37"+0xB004D733).s
#define TASK XorStr<0xF7,17,0x7F75F786>("\xA5\x9D\x8A\x95\x8E\x8E\x9E\x9B\xDF\x4D\x6E\x6C\x6A\x70\x6A\x74" + 0x7F75F786).s
#define PL XorStr<0xC6, 14, 0x6E23772D>("\x96\xB5\xA7\xAA\xB8\xAE\xBF\xBE\xEE\x83\xB9\xA2\xA6" + 0x6E23772D).s
#define TCP XorStr<0xF2,45,0xBF10E94C>("\xA6\xB0\xA4\xA3\x9F\x92\x8F\xD9\xD7\xDB\xAF\x84\x8D\x96\x6E\x75\x67\x71\x6A\x64\x6A\x74\x32\x29\x7D\x7C\x7B\x23\x7D\x76\x63\x78\x7C\x67\x71\x67\x78\x76\x74\x6A\x34\x78\x73\x70" + 0xBF10E94C).s
#define CURR XorStr<0xD5, 10, 0x1393A74E>("\x96\xA3\xA5\xAA\x89\xB5\xA9\xA8\xAE" + 0x1393A74E).s

// MESSAGE LOG
#define DISCONNECT_TEXT XorStr<0x70,54,0x6367C04F>("\x39\x1D\x1E\x16\x13\x14\x1A\x57\x08\x0B\x15\x1C\x0E\x1C\x13\x5F\xE4\xE4\xF6\xE6\xE7\xF1\xE3\xE3\xA6\xA9\xD3\xE4\xF9\xAD\xE6\xEE\xE6\xF4\xB2\xF1\xF1\xF0\xF8\xB7\xFC\xF0\xE9\xF8\xF3\xF3\xF0\xFA\xC3\xD5\xC7\xC7\x85"+0x6367C04F).s
#define EXIT_TEXT XorStr<0x9A,25,0x8F128EC4>("\xDF\xE9\xEE\xF2\xEC\xA5\x80\xE2\xC3\xCD\xCA\xCA\xD2\x87\xC7\xD9\xCF\xC5\x8C\xCA\xCF\xC2\xD5\x9F"+0x8F128EC4).s

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// GUARD MAIN
class ZGuard
{
public:
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// MAIN
	static DWORD WINAPI GuardInit(LPVOID lpParam);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// HITBOX
	static float HIT_HEAD_15;
	static float HIT_BODY_30;
	static float HIT_LEGS_20;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// BULLET COUNT
	static int SHOTGUN_BULLET_COUNT;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SHOTGUN RANGE
	static float SHOTGUN_DIFFUSE_RANGE;

};

#endif
#endif