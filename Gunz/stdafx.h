#pragma once
//#include "vld.h"
#include <sdkddkver.h>
#include <afxdb.h>
#include <afxtempl.h>
#include <afxdtctl.h>
#include <winsock2.h>
#include <mswsock.h>
#include <crtdbg.h>
#include <windows.h>
#include <mmsystem.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>
#include <cstddef>
#include <comutil.h>
#include <stdio.h>
#include <intrin.h>
//#include <IPHlpApi.h>
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "rapidxml_print.hpp"
//#pragma comment(lib, "Iphlpapi.lib")
#include "../CSCommon/MFeatureDefine.h"

#include "detours.h"

#define _WIN32_WINNT 0x0501
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS
#define _CRTDBG_MAP_ALLOC
#define    POINTER_64   __ptr64
#define WIN32_LEAN_AND_MEAN	

#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define NO_WARN_MBCS_MFC_DEPRECATION

/////////////////////
//  Toggle system  //
/////////////////////

// Custom: Hệ thống khí hậu
//#define _RAINSYSTEM 1

// Custom: System License Key
//#define _LICENSE 1	//1 dạng check files SHA1
#define _DISCORD 1

//////////CHATSYSTEM//////
#define _NEW_CHAT 1
#define _CHATBACKGROUND 1
/////////////////////////
//#define _DEVMODE 1
#define _RADAR 1
#define _PICKOFF 1
#define _MACROTIME 1
#define _FIRSTBLOOD 1
//#define _NEWLOAD 1	//loading mới 
#define _HITSCOUNT 1
#define _SPRINTSCREEN 1
#define _HIDETHINGLOADING 1
//#define _RADIO 1	// Radio trong game /ra
//#define  WAVEIN 1 //(Windows WaveIn), nén bằng Opus, và gửi frame đi qua mạng theo thời gian thực.
#define _FSM_ALLOWUNKNOWN 1
#define _BLASTCHALLENGENPC 1
#define _SCREENSHOT_FILTER_CHAT 1
//#define _CHARACTERLOBBY 1	//hình ảnh nhân vật thay vì thông tin text trong lobby
#define _LAUNCHER 1
#define _FONTSIZE 1
#define _FLAHS 1  // cảnh báo ứng dụng khi bị mất focus
#define _NAMEAPP "Gunz"
#define _NAMEAPPFOLDER "Gunz The Duel"
#define _NAMEAPPFOLDER_BANDICAM "\\Gunz The Duel"
#define _EXTFILEMRS_E  ".mrs"
#define _LOGINPING 1
#define _DRAWHPAP 1
#define _MAIET_LOGO 1
#define _LOGINORI 1	//login gốc nếu ẩn đi sẽ nhanh
//#define _CHURCH 1	//login nhà thờ (đã bỏ)
#define _FIXGUNMASIVE 1
#define _CHARGEMASSIVESTYLE 1
#define _LADDERDUEL 1
#define _FONTNEW 1
#define _CAMERADISTANCE 1
#define _OGGTOMP3 1	//sử dụng nhạc mp3 thay vì ogg
//#define _LOTTIE 1	//interface animations
#define _TRAILCOLOR 1
#define _BULLETBAR 1
#define _MACOLOR 1
#define _BARNPC 1
#define _NPCBAR 1
#define _FREELOOK 1
#define _CROSS_SIZE_BOX 1
//#define _DEATHEFFECT 1  // thần chết trên đầu
//#define _CROSS_TRASP 1	//độ mờ của crosshair (chưa có option.xml cho nó)
#define _SYSINTERNEW 1
//#define _INPUTFPS 1
//#define _MOVINGFLOOR 1
#define _GLOBALANNOUNCE 1
//#define _SIMNAMEKOR 1	//kiểm tra tên sai phạm theo bảng hàn quốc
#define _ZGUARD 1
//#define _MAP_CACHING 1
//#define _ICONVIP 1	//nhưng sẽ ko hiển thị icon rank normal channel game room
#define _TAGVIP 1
#define _LADDERUPDATE 1
//#define _NEWCOMMANDER 1
//#define _ZPEERANTIHACK 1
//#define _GAMBLEITEM_TEST_LOG	1
//#define _REPLAY_TEST_LOG 1
//#define _LOCATOR 1
//#define _AFKSYSTEM 1 //kick khỏi room khi afk 15 phút
#define _UTF8 1  // bên cml
//#define _EAC 1  // Easy Anti Cheat
#ifdef _EAC
#include "eos_sdk.h"
#include "eos_metrics.h"
#include "eos_metrics_types.h"

extern EOS_HPlatform g_hPlatform;
extern EOS_HMetrics  g_hMetrics;
#endif

// === Launch Mode ===
#define _PUBLISH
#define _SERIALKEY
#define LOCALE_NHNUSA

// Custom: Anti Hack
#define _THEMIDAGUNZ 1
//#define _ANTIHACK 1	// tắt tạm thời để lấy mlog báo về bởi cái anti ngu này

//#define _SHA1  0x9e,0x66,0xa0,0xf2,0xda,0xf0,0x26,0xb9,0x3e,0xb6,0xd9,0x24,0xad,0xc8,0xf4,0x4b,0x8c,0x74,0xa0,0x23
//#define _SERIALCUSTOM 1

// Detours 4.x (from sdk/detours) auto-detects platform. Manual defines not needed.
//#ifdef _M_X64
//#define DETOURS_X64 1
//#define DETOURS_64BIT 1
//#else
//#define	DETOURS_X86 1
//#define	DETOURS_32BIT 1
//#endif

#if defined(LOCALE_NHNUSA)// || defined(_DEBUG)
#endif
#if defined(_DEBUG) || defined(_RELEASE) || defined(LOCALE_KOREA) || defined(LOCALE_NHNUSA) || defined(LOCALE_JAPAN) || defined(LOCALE_BRAZIL) || defined(LOCALE_VIETNAM)
#define _MULTILANGUAGE 1
#endif

#ifdef _DEBUG
// #define _XTRAP 
// #define _HSHIELD
// #define _XTRAP
#endif

// stl
#include <list>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <sstream>

// DirectX Includes
#include "d3d9.h"
#include "d3dx9math.h"

#include "fmod.h"

// cml
#include "MXml.h"
#include "MUtil.h"
#include "MDebug.h"
#include "MRTTI.h"
#include "MUID.h"
#include "MemPool.h"

// xor head
#include <xorstr.h>

// mint
#include "Mint.h"
#include "MWidget.h"
#include "MBitmap.h"
#include "MButton.h"
#include "MListBox.h"
#include "MTextArea.h"
#include "MTabCtrl.h"
#include "MComboBox.h"
#include "MFrame.h"
#include "MPopupMenu.h"

// realspace2
#include "rtypes.h"
#include "RNameSpace.h"
#include "RTypes.h"
#include "RealSpace2.h"
#include "RBspObject.h"
#include "RMeshMgr.h"
#include "RVisualMeshMgr.h"
#include "RMaterialList.h"
#include "RAnimationMgr.h"
#include "Mint4R2.h"

// cscommon
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchStage.h"
#include "MMatchItem.h"
#include "MMatchMap.h"
#include "MSafeUDP.h"
#include "MMatchClient.h"
#include "MGameClient.h"
#include "MMatchTransDataType.h"
#include "MErrorTable.h"
#include "Config.h"
#include "MSharedCommandTable.h"
#include "MObjectTypes.h"
#include "MMatchBuff.h"

// gunz global
#include "ZApplication.h"
#include "ZGlobal.h"
#include "ZMessages.h"
#include "ZStringResManager.h"
#include "ZGameInterface.h"
#include "ZCombatInterface.h"
#include "ZGame.h"
#include "ZGameClient.h"
#include "ZObject.h"
#include "ZIDLResource.h"
#include "ZInterfaceListener.h"
#include "ZColorTable.h"
#include "ZMyInfo.h"
#include "ZMyItemList.h"
#include "ZNetRepository.h"
#include "ZItem.h"
#include "ZItemDesc.h"
#include "ZPost.h"
#include "ZSoundEngine.h"
#include "ZSoundFMod.h"
#include "ZCamera.h"
#include "ZCharacter.h"
#include "ZActor.h"
#include "ThemidaSDK.h"
#include "RGGlobal.h"
#include "targetver.h"
