#include "stdafx.h"
#include "AntiInject.h"
#ifdef _ANTIHACK
bool IsAddressHookedGunz(unsigned long address) 
{
    BYTE* offsetValue = (BYTE*)address;
    return (*offsetValue == 0xE8 || *offsetValue == 0xE9 || *offsetValue == 0x7E || *offsetValue == 0x74 || *offsetValue == 0xC3);
}

void AntiInject(void*)
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
    DWORD getTickCount = (DWORD)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetTickCount");
    DWORD queryPerformanceCounter = (DWORD)GetProcAddress(GetModuleHandleA("kernel32.dll"), "QueryPerformanceCounter");
    while (1)
    {
        Sleep(RandomNumber(5000, 7000));
        if (IsAddressHookedGunz(getTickCount) || IsAddressHookedGunz(queryPerformanceCounter) || 
            GetModuleHandleA("proxy.dll") || 
            GetModuleHandleA("injector.dll") || 
            GetModuleHandleA("daniel.dll") || 
            GetModuleHandleA("kn3drip.dll") != NULL)
        {
            ExitProcess(NULL);
        }
    }
}
BOOLEAN BlockAPI(HANDLE hProcess, const CHAR* libName, const CHAR* apiName)
{
    CHAR pRet[] = { 0xC3 };
    HINSTANCE hLib = NULL;
    VOID* pAddr = NULL;
    BOOL bRet = FALSE;
    SIZE_T dwRet = 0; // Đổi từ DWORD sang SIZE_T ở đây

    hLib = LoadLibraryA(libName);
    if (hLib) {
        pAddr = (VOID*)GetProcAddress(hLib, apiName);
        if (pAddr) {
            // WriteProcessMemory bây giờ sẽ nhận &dwRet mà không báo lỗi
            if (WriteProcessMemory(hProcess,
                (LPVOID)pAddr,
                (LPVOID)pRet,
                sizeof(pRet),
                &dwRet)) {
                if (dwRet > 0) {
                    bRet = TRUE;
                }
            }
        }
        FreeLibrary(hLib);
    }
    return bRet;
}
// Custom: Metodo 2
void Patch3DRipper(void*)
{
    HANDLE hProc = FindWindow(0, "Gunz");
    while (TRUE) {
        BlockAPI(hProc, "kn3drip.dll", "LdrLoadDll"); // LdrLoadDll
        Sleep(100);
    }
}
#endif