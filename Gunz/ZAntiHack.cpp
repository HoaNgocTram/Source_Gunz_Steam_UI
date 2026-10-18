#include "stdafx.h"

#ifdef _ANTIHACK

#include "ZAntiHack.h"
#include "ZFunctions.h"

using namespace std;
HANDLE SystemProcessScan();
bool ScanProcessMemory(HANDLE hProcessHandle);
typedef struct PROCESS_DUMP {
	uintptr_t m_aOffset;
	unsigned char m_aMemDump[MAX_DUMP_SIZE];
} *PPROCESS_DUMP;
extern PROCESS_DUMP MEMORY_PROCESS_DUMP[MAX_PROCESS_DUMP];


PROCESS_DUMP MEMORY_PROCESS_DUMP[MAX_PROCESS_DUMP] = {
{(uintptr_t)0x417770, {0xE8, 0xC4, 0xAF, 0x00, 0x00, 0xE9, 0x79, 0xFE, 0xFF, 0xFF, 0x8B, 0xFF, 0x55, 0x8B, 0xEC, 0x8B, 0xC1, 0x8B, 0x4D, 0x08, 0xC7, 0x00, 0x88, 0xDA, 0x47, 0x00, 0x8B, 0x09, 0x83, 0x60, 0x08, 0x00}}, //SHOTBOT.EXE + SHOTBOT.AU3
{(uintptr_t)0x462000, {0x00, 0x00, 0x00, 0x00, 0x07, 0x0A, 0x44, 0x00, 0xAE, 0x62, 0x45, 0x00, 0xBA, 0x62, 0x45, 0x00, 0xD0, 0x62, 0x45, 0x00, 0xDA, 0x62, 0x45, 0x00, 0x01, 0x63, 0x45, 0x00, 0x1F, 0x63, 0x45, 0x00}}, //GUNZ MOUSE REBINDER 1.19 - 1.20
{(uintptr_t)0x445CF4, {0x6A, 0x60, 0x68, 0x90, 0x36, 0x45, 0x00, 0xE8, 0x60, 0xF8, 0xFF, 0xFF, 0xBF, 0x94, 0x00, 0x00, 0x00, 0x8B, 0xC7, 0xE8, 0xE4, 0xED, 0xFF, 0xFF, 0x89, 0x65, 0xE8, 0x8B, 0xF4, 0x89, 0x3E, 0x56}} //GUNZ MOUSE REBINDER 1.18
};


void AutoitHax(){

	while(1){
		if(SystemProcessScan() != INVALID_HANDLE_VALUE)
		{
			ExitProcess(NULL);
		}
		Sleep(10000);
	}

}


HANDLE SystemProcessScan()
{
	DWORD dwProcesses[1024];
	DWORD dwTotal = 0;
	HANDLE hProcessHandle;
	if (!EnumProcesses(dwProcesses, sizeof(dwProcesses), &dwTotal))
		return INVALID_HANDLE_VALUE;

	DWORD dwCount = dwTotal / sizeof(DWORD);
	for (DWORD i = 0; i < dwCount; i++)
	{
		if (dwProcesses[i] == 0) continue;
		// PROCESS_VM_READ đủ cho ReadProcessMemory, không cần PROCESS_ALL_ACCESS
		hProcessHandle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, dwProcesses[i]);
		if (hProcessHandle == NULL || hProcessHandle == INVALID_HANDLE_VALUE)
			continue;
		if (ScanProcessMemory(hProcessHandle))
			return hProcessHandle;
		CloseHandle(hProcessHandle);
	}
	return INVALID_HANDLE_VALUE;
}


bool ScanProcessMemory(HANDLE hProcessHandle)
{
	for (int i = 0; i < MAX_PROCESS_DUMP; i++)
	{
		unsigned char aTmpBuffer[MAX_DUMP_SIZE] = { 0 };
		SIZE_T aBytesRead = 0;
		// Cast offset sang LPCVOID — trên x64 sẽ zero-extend tự động từ uintptr_t
		BOOL bOk = ReadProcessMemory(hProcessHandle, (LPCVOID)MEMORY_PROCESS_DUMP[i].m_aOffset, (LPVOID)aTmpBuffer, MAX_DUMP_SIZE, &aBytesRead);
		if (bOk && aBytesRead == MAX_DUMP_SIZE)
		{
			if (memcmp(aTmpBuffer, MEMORY_PROCESS_DUMP[i].m_aMemDump, MAX_DUMP_SIZE) == 0)
				return true;
		}
	}
	return false;
}
#endif