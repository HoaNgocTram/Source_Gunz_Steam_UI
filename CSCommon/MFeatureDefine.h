#pragma once
#include "../sdk/xor/include/xorstr.h"
/////////////////////
//  Toggle system  //
/////////////////////
// Enabled Settings
#define _SPYMODE 1
#define _SPYLADDER 1
#define _COMMANDLOGS 1
//#define _PLAYERINO 1
//#define _LIMITMAP 1
#define _2PCLAN	1
#define _SYSTEMUPDATETIME (10 * 60 * 1000)
#define _SRVRPNG 1
#define _MULTILANGUAGE 1
#define _COUNTRYFLAG 1
#define _KILLSTREAK 1
#define _STAFFCHAT 1
#define _AVATAR_ENABLE 1
#define _QUEST 1
#define _QUEST_ITEM	1
#define _MONSTER_BIBLE 1
#define _DUELTOURNAMENT	1
#define _CW_VOTE 1
#define _LADDERGAME 1
#define _MIPING	1
#define _LADDERGAMETEST 1	//lệnh test ladder
#define _VIPGRADES 1
#define _EVENTGRD 1
#define _NEWGRADE 1
#define _SHOPFIXCANT 1
#define _REPORT 1
#define _AGENTPORT 1
#define _ANTISPAM 1
#define _GRADECHANGE 1
#define _EXPCHANNEL 1
#define _CHANNELNEW 1
#define _UDPCUSTOM 1		// custom cổng UDP cho Agent
#define _LIGHTURNOFF 1
#define _VOICE_CHAT 1
#define _KILLFEED 1
#define _ROTATION 1
#define _BOXLEAD 1
#define _SWORDCOLOR 1
#define _NOLEAD 1
#define _CALCDMG 1
#define _HWID 1
#define _CMD_ALL 1
#define _UPCHARCMD 1
#define _MAGICBOX 1	// thiếu database
#define _PAINTMODE 1
#define _DYNAMIC 1	// Dynamic không load elu quần áo
#define _ICONCHAT 1	// icon voice chat trên đầu nhân vật
#define _ITEMSTORAGE 1
#define _SYSTEMZITEM 1
#define _LADDERWARSPACKETS 1
#define _LOGIN_AUTH 1
#define _POSTABINFO 1
#define UPDATE_STAGE_CHARVIEWER 1
#define UPDATE_STAGE_EQUIP_LOOK 1
//#define _KOR_THINGS 1
#define _ROCKETGUIDED 1
#define _PORTALGUN 1
#define _TYPENET 1
#define _ASSITEN 1
//#define _SPMOTIONLAST 1		//motion dance slot item (điếu định nghĩa build lỗi chết mẹ)
//#define _LOBBYSET 1		//bẫy vcl đụng vào ăn cook
#define _SPEC 1
//#define _RECOMMANDEDTEAM 1
//#define _FLOATDMG 1
//#define _FLOATDMG2 1
//#define _FLOATDMGTEXT 1
#define _FLOATDMG_MODE 1 // Replaces _FLOATDMG, _FLOATDMG2, _FLOATDMGTEXT with a single ComboBox (0=Off, 1=Text, 2=Elu)
#define _PERFECT 1
#define _MEMFIX 1

// Common definitions
//#define _ANTILEECH 1
#define _SERVER_FALLBACK 1 //khôa cứng IP theo mã hoá (tắt để dùng nhiều Server list)

//Gunz IP /*103.15.222.108*/XorStr<0xFF,15,0x68641E43>("\xCE\x30\x32\x2C\x32\x31\x2B\x34\x35\x3A\x27\x3B\x3B\x34" + 0x68641E43).s
//IP Gunz VN XorStr<0x0B,15,0xD5BF779A>("\x7E\x7C\x69\x6F\x7B\x75\x3F\x75\x66\x7A\x6F\x38\x61\x76" + 0xD5BF779A).s
// sửa tất cả link trong webview2 và nhạc ogg. sửa thông tin discord

#define SERVER_IP		XorStr<0x41, 15, 0x8B10AF84>("\x34\x32\x27\x25\x31\x23\x69\x2F\x3C\x24\x31\x62\x3B\x20" + 0x8B10AF84).s
#define _AGENT_IP       XorStr<0x41, 15, 0x8B10AF84>("\x34\x32\x27\x25\x31\x23\x69\x2F\x3C\x24\x31\x62\x3B\x20" + 0x8B10AF84).s
#define SERVER_DOMAIN	XorStr<0x41, 15, 0x8B10AF84>("\x34\x32\x27\x25\x31\x23\x69\x2F\x3C\x24\x31\x62\x3B\x20" + 0x8B10AF84).s

//Gunz Local

//#define _RANDOMNOTICE 1
//#define _RANDOMNOTICETIME (30 * 60 * 1000)
//#define _RANDOMNOTICEMSG1 "System Ranking Update!"

//#define _FORCE_EVENT_TEAM 1
//#define _ROOMHIDE 1

//#define _EVENTCMD 1
//#define _EXPLOITCLIENT 1
//#define _LVL200 1  dùng được max 999 lv
#define _ANTISQL 1
//#define _RANK 1
//#define _UPREWARDLEVEL 1

#if defined(_DEBUG) || defined(_RELEASE) || defined(LOCALE_KOREA) || defined(LOCALE_NHNUSA)
#endif

//#if defined(_DEBUG) || defined(_RELEASE) || defined(LOCALE_KOREA)
#if 0
#	define _SELL_CASHITEM
#endif