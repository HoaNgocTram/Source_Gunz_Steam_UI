#include "stdafx.h"
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
#include "ZEACManager.h"
#include <processenv.h>
#include <winerror.h>
#include <string>
#include <vector>
#include <iostream>
#include <fileapi.h>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib") // Cần thiết cho PathRemoveFileSpecA
//EOS_HPlatform g_hPlatform = NULL;
//EOS_HMetrics  g_hMetrics = NULL;
#ifdef EAC
ZEACManager& ZEACManager::GetInstance() {
    static ZEACManager instance;
    return instance;
}

bool ZEACManager::LaunchProtectedInstance() {
    // 1. Kiểm tra nếu đã chạy qua EAC (dựa trên tham số dòng lệnh EAC truyền vào)
    if (strstr(GetCommandLineA(), "-eac_launcher") || strstr(GetCommandLineA(), "productid")) {
        return true;
    }

    // 2. Kiểm tra Mutex để chắc chắn chỉ có MỘT bản launcher đang chạy
    HANDLE hMutex = CreateMutexA(NULL, TRUE, "GunZ_EAC_Bootstrapper_Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // Nếu đã có một bản đang chờ EAC, thì bản này phải tự đóng ngay
        return false;
    }

    // 3. Nếu chưa có tham số EAC, nghĩa là người chơi vừa click tay vào Gunz.exe
    char szLauncherPath[MAX_PATH];
    GetModuleFileNameA(NULL, szLauncherPath, MAX_PATH);
    PathRemoveFileSpecA(szLauncherPath);
    strcat_s(szLauncherPath, "\\start_protected_game.exe");

    if (GetFileAttributesA(szLauncherPath) != INVALID_FILE_ATTRIBUTES) {
        // Chạy launcher và THOÁT NGAY LẬP TỨC bản gốc này
        ShellExecuteA(NULL, "open", szLauncherPath, NULL, NULL, SW_SHOWNORMAL);
        ExitProcess(0);
    }

    return true;
}

bool ZEACManager::InitializeSDK() {

    EOS_InitializeOptions initOptions = { EOS_INITIALIZE_API_LATEST };
    initOptions.ProductName = "GunzVN";
    initOptions.ProductVersion = "1.0";

    if (EOS_Initialize(&initOptions) != EOS_EResult::EOS_Success) return false;

    EOS_Platform_Options platOptions = { EOS_PLATFORM_OPTIONS_API_LATEST };
    platOptions.ProductId = EAC_PRODUCT_ID;
    platOptions.SandboxId = EAC_SANDBOX_ID;
    platOptions.DeploymentId = EAC_DEPLOYMENT_ID;
    platOptions.ClientCredentials.ClientId = EAC_CLIENT_ID;
    platOptions.ClientCredentials.ClientSecret = EAC_CLIENT_SECRET;
    platOptions.bIsServer = EOS_FALSE;
    platOptions.EncryptionKey = NULL;
    platOptions.OverrideCountryCode = NULL;
    platOptions.OverrideLocaleCode = NULL;
    platOptions.CacheDirectory = NULL;
    platOptions.TickBudgetInMilliseconds = 0;
    platOptions.RTCOptions = NULL;
    platOptions.IntegratedPlatformOptionsContainerHandle = NULL;
    platOptions.SystemSpecificOptions = NULL;
    platOptions.TaskNetworkTimeoutSeconds = NULL;

    m_hPlatform = EOS_Platform_Create(&platOptions);
    if (!m_hPlatform) return false;

    m_hAntiCheat = EOS_Platform_GetAntiCheatClientInterface(m_hPlatform);
    return (m_hAntiCheat != nullptr);

    g_hMetrics = EOS_Platform_GetMetricsInterface(g_hPlatform);
    if (g_hMetrics == NULL) {
        mlog("EOS Metrics interface not available (g_hMetrics == NULL)\n");
    }
    else {
        mlog("Epic Online Services (EOS) Metrics interface acquired.\n");
    }
}

void ZEACManager::Tick() {
    if (m_hPlatform == nullptr) return;

    static DWORD dwLastTick = 0;
    DWORD dwCurrTime = timeGetTime();

    // Chỉ thực thi mỗi 33ms (tương đương khoảng 30 FPS cho riêng EAC)
    if (dwCurrTime - dwLastTick >= 33) {
        EOS_Platform_Tick(m_hPlatform);
        dwLastTick = dwCurrTime;
    }
}

void ZEACManager::Shutdown() {
    if (m_hPlatform != nullptr) {
        EOS_Platform_Release(m_hPlatform);
        EOS_Shutdown();
        m_hPlatform = nullptr;
    }
}
#endif