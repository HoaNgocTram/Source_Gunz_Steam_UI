#include "StdAfx.h"
#include "MDebug.h"
#include "d3d9.h"
#include <winternl.h>
#include <intrin.h>

void MSysInfoLog_CPU()
{
    LARGE_INTEGER ulFreq, ulTicks, ulValue, ulStartCounter, ulEAX_EDX, ulResult;

    QueryPerformanceFrequency(&ulFreq);
    QueryPerformanceCounter(&ulTicks);
    ulValue.QuadPart = ulTicks.QuadPart + ulFreq.QuadPart;

#ifndef _M_X64
    _asm
    {
        rdtsc
        mov ulEAX_EDX.LowPart, EAX
        mov ulEAX_EDX.HighPart, EDX
    }
#else
    ulEAX_EDX.QuadPart = __rdtsc();
#endif

    ulStartCounter.QuadPart = ulEAX_EDX.QuadPart;
    do
    {
        QueryPerformanceCounter(&ulTicks);
    } while (ulTicks.QuadPart <= ulValue.QuadPart);

#ifndef _M_X64
    _asm
    {
        rdtsc
        mov ulEAX_EDX.LowPart, EAX
        mov ulEAX_EDX.HighPart, EDX
    }
#else
ulEAX_EDX.QuadPart = __rdtsc();
#endif

    ulResult.QuadPart = ulEAX_EDX.QuadPart - ulStartCounter.QuadPart;
    int nCPUClock = int(ulResult.QuadPart / 1000000);

    DWORD nCPUFamily, nCPUModel, nCPUStepping;
    char pszCPUType[49] = { 0 };

#ifndef _M_X64
    _asm
    {
        mov eax, 0x80000002
        cpuid
        mov pszCPUType[0], al
        mov pszCPUType[1], ah
        shr eax, 16
        mov pszCPUType[2], al
        mov pszCPUType[3], ah
        mov pszCPUType[4], bl
        mov pszCPUType[5], bh
        shr ebx, 16
        mov pszCPUType[6], bl
        mov pszCPUType[7], bh
        mov pszCPUType[8], cl
        mov pszCPUType[9], ch
        shr ecx, 16
        mov pszCPUType[10], cl
        mov pszCPUType[11], ch
        mov pszCPUType[12], dl
        mov pszCPUType[13], dh
        shr edx, 16
        mov pszCPUType[14], dl
        mov pszCPUType[15], dh

        mov eax, 0x80000003
        cpuid
        mov pszCPUType[16], al
        mov pszCPUType[17], ah
        shr eax, 16
        mov pszCPUType[18], al
        mov pszCPUType[19], ah
        mov pszCPUType[20], bl
        mov pszCPUType[21], bh
        shr ebx, 16
        mov pszCPUType[22], bl
        mov pszCPUType[23], bh
        mov pszCPUType[24], cl
        mov pszCPUType[25], ch
        shr ecx, 16
        mov pszCPUType[26], cl
        mov pszCPUType[27], ch
        mov pszCPUType[28], dl
        mov pszCPUType[29], dh
        shr edx, 16
        mov pszCPUType[30], dl
        mov pszCPUType[31], dh

        mov eax, 0x80000004
        cpuid
        mov pszCPUType[32], al
        mov pszCPUType[33], ah
        shr eax, 16
        mov pszCPUType[34], al
        mov pszCPUType[35], ah
        mov pszCPUType[36], bl
        mov pszCPUType[37], bh
        shr ebx, 16
        mov pszCPUType[38], bl
        mov pszCPUType[39], bh
        mov pszCPUType[40], cl
        mov pszCPUType[41], ch
        shr ecx, 16
        mov pszCPUType[42], cl
        mov pszCPUType[43], ch
        mov pszCPUType[44], dl
        mov pszCPUType[45], dh
        shr edx, 16
        mov pszCPUType[46], dl
        mov pszCPUType[47], dh

        mov eax, 1
        cpuid
        mov nCPUFamily, eax
        mov nCPUModel, eax
        mov nCPUStepping, eax
    }
#else
    {
        int cpuInfo[4];
        __cpuid(cpuInfo, 0x80000002);
        memcpy(pszCPUType + 0, cpuInfo, 16);
        __cpuid(cpuInfo, 0x80000003);
        memcpy(pszCPUType + 16, cpuInfo, 16);
        __cpuid(cpuInfo, 0x80000004);
        memcpy(pszCPUType + 32, cpuInfo, 16);
    }
    {
        int cpuInfo1[4];
        __cpuid(cpuInfo1, 1);
        nCPUFamily = cpuInfo1[0];
        nCPUModel = cpuInfo1[0];
        nCPUStepping = cpuInfo1[0];
    }
#endif

    nCPUFamily = ((nCPUFamily >> 8) & 0xF) + ((nCPUFamily >> 20) & 0xFF);
    nCPUModel = ((nCPUModel >> 4) & 0xF) + ((nCPUModel >> 12) & 0xF0);
    nCPUStepping = (nCPUStepping & 0xF);

    for (int i = 0; i < 48; i++) {
        if (pszCPUType[i] == '\0') break;
        if (pszCPUType[i] < 32 || pszCPUType[i] > 126) pszCPUType[i] = ' ';
    }

    char szDesc[512] = "";
    /*sprintf(szDesc, "CPU ID = %s (family = %d, model = %d, stepping = %d) @ %d MHz\n",
        pszCPUType, nCPUFamily, nCPUModel, nCPUStepping, nCPUClock);*/
    sprintf(szDesc, "CPU ID = %s @ %d MHz\n",
        pszCPUType, nCPUClock);
    mlog(szDesc);
}

void MSysInfoLog_Display()
{
    HMODULE hD3DLibrary = NULL;
    LPDIRECT3D9 pD3D = NULL;
    LPDIRECT3DDEVICE9 pLPDIRECT3DDEVICE9 = NULL;
    D3DADAPTER_IDENTIFIER9 deviceID;

    pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (pD3D == nullptr)
    {
        mlog("Error creating device.\n");
        return;
    }

    pD3D->GetAdapterIdentifier(0, 0, &deviceID);
    pD3D->Release();

#ifndef _PUBLISH
    mlog("D3D_SDK_VERSION = %d \n", D3D_SDK_VERSION);
#endif

    /*mlog("Display Device = %s (vendor=%x device=%x subsys=%x revision=%x)\n",
        deviceID.Description, deviceID.VendorId, deviceID.DeviceId, deviceID.SubSysId, deviceID.Revision);*/
    mlog("Display Device = %s \n",
        deviceID.Description);

    mlog("Display Driver Version = %d.%d.%04d.%04d\n",
        deviceID.DriverVersion.HighPart >> 16, deviceID.DriverVersion.HighPart & 0xffff,
        deviceID.DriverVersion.LowPart >> 16, deviceID.DriverVersion.LowPart & 0xffff);
}

typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
LPFN_ISWOW64PROCESS fnIsWow64Process;
BOOL MSysInfo_IsWow64()
{
    BOOL bIsWow64 = FALSE;

    int nIsWow64Proc[] = { 0xC9, 0xF3, 0xD7, 0xEF, 0xF7, 0xB6, 0xB4, 0xD0, 0xF2, 0xEF, 0xE3, 0xE5, 0xF3, 0xF3, 0x80 };
    int nKernel32Dll[] = { 0xEB, 0xE5, 0xF2, 0xEE, 0xE5, 0xEC, 0xB3, 0xB2, 0xAE, 0xE4, 0xEC, 0xEC, 0x80 };

    char szIsWow64Proc[128];
    memset(szIsWow64Proc, 0, sizeof(szIsWow64Proc));
    char szKernel32Dll[128];
    memset(szKernel32Dll, 0, sizeof(szKernel32Dll));

    for (int i = 0; i < _countof(nIsWow64Proc); ++i)
        szIsWow64Proc[i] = nIsWow64Proc[i] ^ 0x80;

    for (int i = 0; i < _countof(nKernel32Dll); ++i)
        szKernel32Dll[i] = nKernel32Dll[i] ^ 0x80;

    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT(szKernel32Dll)), szIsWow64Proc);

    if (NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
        {
            return FALSE;
        }
    }

    return bIsWow64;
}

void MSysInfoLog_OS()
{
    typedef NTSTATUS(NTAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

    HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtDll) {
        mlog("Failed to get ntdll.dll\n");
        return;
    }

    RtlGetVersionPtr pRtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hNtDll, "RtlGetVersion");
    if (!pRtlGetVersion) {
        mlog("Failed to get address of RtlGetVersion\n");
        return;
    }

    RTL_OSVERSIONINFOW os = { 0 };
    os.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
    pRtlGetVersion(&os);

    MEMORYSTATUSEX ms = { 0 };
    ms.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&ms);
    ULONGLONG dwPhysicalMemory = ms.ullTotalPhys / (1024 * 1024);

    DWORD dwProductType = 0;
    GetProductInfo(os.dwMajorVersion, os.dwMinorVersion, 0, 0, &dwProductType);
    bool isServer = (dwProductType == PRODUCT_STANDARD_SERVER || dwProductType == PRODUCT_DATACENTER_SERVER ||
        dwProductType == PRODUCT_ENTERPRISE_SERVER || dwProductType == PRODUCT_WEB_SERVER);
    bool isWorkstation = !isServer;

    const char* szWindowsName = "Unknown Windows";
    if (os.dwMajorVersion == 10 && os.dwMinorVersion == 0) {
        if (os.dwBuildNumber >= 22000)
            szWindowsName = isWorkstation ? "Windows 11" : "Windows Server 2022";
        else
            szWindowsName = isWorkstation ? "Windows 10" : "Windows Server 2016/2019";
    }
    else if (os.dwMajorVersion == 6) {
        if (os.dwMinorVersion == 3) szWindowsName = isWorkstation ? "Windows 8.1" : "Windows Server 2012 R2";
        else if (os.dwMinorVersion == 2) szWindowsName = isWorkstation ? "Windows 8" : "Windows Server 2012";
        else if (os.dwMinorVersion == 1) szWindowsName = isWorkstation ? "Windows 7" : "Windows Server 2008 R2";
        else if (os.dwMinorVersion == 0) szWindowsName = isWorkstation ? "Windows Vista" : "Windows Server 2008";
    }
    else if (os.dwMajorVersion == 5) {
        if (os.dwMinorVersion == 2) szWindowsName = "Windows Server 2003";
        else if (os.dwMinorVersion == 1) szWindowsName = "Windows XP";
        else if (os.dwMinorVersion == 0) szWindowsName = "Windows 2000";
    }

#ifdef _M_X64
    const char* szArch = "64-bit";
#else
    const char* szArch = "32-bit";
#endif

    char szDesc[512];
    sprintf(szDesc, "Windows = %s (%s) Build %d (%llu MB)\n ===============System Information===============\n\n",
        szWindowsName, szArch, os.dwBuildNumber, dwPhysicalMemory);

    mlog(szDesc);
}

void MSysInfoLog()
{
    MSysInfoLog_CPU();
    MSysInfoLog_Display();
    MSysInfoLog_OS();
}