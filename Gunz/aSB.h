#pragma once
#include <Windows.h>
#include "detours.h"

class cDetour
{
public:
	void* Tramp;
	void* Det;
	void* Target;

	cDetour(void* From, void* To)
	{
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttachEx(&From, To, (DETOUR_TRAMPOLINE**)&Tramp, &Target, &Det);
		DetourTransactionCommit();
	}
};

// Dùng đúng prototype của GetRawInputData — UINT WINAPI
// Trên x86: WINAPI = __stdcall. Trên x64: WINAPI = __fastcall (compiler tự xử lý)
typedef UINT(WINAPI* pfnGetRawInputData)(HRAWINPUT, UINT, LPVOID, PUINT, UINT);

extern cDetour* GetRawInputDataDetour;

inline UINT WINAPI GetRawInputDataHook(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader)
{
	UINT RetVal = ((pfnGetRawInputData)GetRawInputDataDetour->Tramp)(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);

	if (RetVal && pData)
	{
		if (((RAWINPUT*)pData)->header.hDevice == NULL)
			return 0;
	}

	return RetVal;
}