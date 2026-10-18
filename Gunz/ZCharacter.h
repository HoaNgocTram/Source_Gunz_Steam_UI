#ifndef _ZCHARACTER_H
#define _ZCHARACTER_H

//#pragma	once

#include "MRTTI.h"
#include "ZCharacterObject.h"
//#include "ZActor.h"
#include "MUID.h"
#include "RTypes.h"
#include "RPathFinder.h"
#include "RVisualMeshMgr.h"

#include "MObjectTypes.h"
//#include "MObjectCharacter.h"
#include "ZItem.h"
#include "ZCharacterItem.h"
#include "ZCharacterBuff.h"

#include "MMatchObject.h"
#include "RCharCloth.h"
#include "ZFile.h"
#include "Mempool.h"

#include "ZModule_HPAP.h"
#include "MMatchSpyMode.h"
#include <list>
#include <string>


using namespace std;

_USING_NAMESPACE_REALSPACE2

#define MAX_SPEED			1000.f			// ÃÖ´EÓµµ..			

#ifdef _LOBBYSET
#define RUN_SPEED			ZGetGameClient()->GetMatchStageSetting()->GetSpeedValue()
#define BACK_SPEED			(ZGetGameClient()->GetMatchStageSetting()->GetSpeedValue() - 180.f)
#else
#define RUN_SPEED			630.f			
#define BACK_SPEED			450.f
#endif

#define ACCEL_SPEED			7000.f			// °¡¼Óµµ
#define STOP_SPEED			3000.f			// ¾Æ¹«Å°µµ ¾È´­·¶À»¶§ °¨¼Óµµ..
#define STOP_FORMAX_SPEED	7100.f			// ´Þ¸®´Â ¼Óµµ ÀÌ»óÀ¸·Î ¿Ã¶ó°¬À»¶§ »¡¸® °¨¼ÓÇÏ´Â ¼Óµµ

#define CHARACTER_RADIUS	35.f		// Ä³¸¯ÅÍ Ãæµ¹ ¹ÝÁö¸§
#define CHARACTER_HEIGHT	180.0f		// Ä³¸¯ÅÍ Ãæµ¹ ³ôÀÌ

#define ARRIVAL_TOLER		5.f

class ZShadow;

struct ZANIMATIONINFO {
	char *Name;
	bool bEnableCancel;		// Äµ½½ °¡´ÉÇÑÁE
	bool bLoop;				// ¹Ýº¹ µÇ´Â µ¿ÀÛ
	bool bMove;				// ¿òÁ÷ÀÓÀÌ Æ÷ÇÔµÈ ¾Ö´Ï¸ÞÀÌ¼Ç
	bool bContinuos;		// Æ÷ÇÔµÈ ¿òÁ÷ÀÓÀÌ ½ÃÀÛºÎÅÍ ¿¬°áµÇ¾ûÜÖ´ÂÁE
};

struct ZFACECOSTUME
{
	char* szMaleMeshName;
	char* szFemaleMeshName;
};


enum ZC_SKILL {

	ZC_SKILL_NONE = 0,

	ZC_SKILL_UPPERCUT,
	ZC_SKILL_SPLASHSHOT,
	ZC_SKILL_DASH,
	ZC_SKILL_CHARGEDSHOT,

	ZC_SKILL_END
};


enum ZC_DIE_ACTION
{
	ZC_DIE_ACTION_RIFLE = 0,
	ZC_DIE_ACTION_KNIFE,
	ZC_DIE_ACTION_SHOTGUN,
	ZC_DIE_ACTION_ROCKET,

	ZC_DIE_ACTION_END
};

enum ZC_SPMOTION_TYPE {

	ZC_SPMOTION_TAUNT = 0,
	ZC_SPMOTION_BOW ,
	ZC_SPMOTION_WAVE ,
	ZC_SPMOTION_LAUGH ,
	ZC_SPMOTION_CRY ,
	ZC_SPMOTION_DANCE ,

	ZC_SPMOTION_END
};

enum ZC_WEAPON_SLOT_TYPE {

	ZC_SLOT_MELEE_WEAPON = 0,
	ZC_SLOT_PRIMARY_WEAPON,
	ZC_SLOT_SECONDARY_WEAPON,
	ZC_SLOT_ITEM1,
	ZC_SLOT_ITEM2,

	ZC_SLOT_END,
};

enum ZC_SHOT_SP_TYPE {
	ZC_WEAPON_SP_NONE = 0,

	// grenade type
	ZC_WEAPON_SP_GRENADE,
	ZC_WEAPON_SP_ROCKET,
	ZC_WEAPON_SP_FLASHBANG,
	ZC_WEAPON_SP_SMOKE,
	ZC_WEAPON_SP_TEAR_GAS,

	ZC_WEAPON_SP_ITEMKIT,	// medikit, repairkit, bulletkit µûÑE

	ZC_WEAPON_SP_POTION,
	ZC_WEAPON_SP_TRAP,
	ZC_WEAPON_SP_DYNAMITE,

	ZC_WEAPON_SPY_STUNGRENADE,
	ZC_WEAPON_SP_FLASHBANG_SPY,
	ZC_WEAPON_SP_SMOKE_SPY,
	ZC_WEAPON_SP_TRAP_SPY,
#ifdef _PORTALGUN 1
	ZC_WEAPON_SP_PORTAL_GUN,
	ZC_WEAPON_SP_PORTAL_GUN_RED,
#endif
	ZC_WEAPON_SP_END,
};

enum ZSTUNTYPE {
	ZST_NONE	=	-1,
	ZST_DAMAGE1	=	0,
	ZST_DAMAGE2,
	ZST_SLASH,			// °­º£±E½ºÅÏ
	ZST_BLOCKED,		// Ä® ¸·ÇûÀ»¶§ ½ºÅÏ
	ZST_LIGHTNING,		// ÀÎÃ¦Æ®Áß Lightning
	ZST_LOOP,			// ½ºÅ³Áß root ¼Ó¼º
};


class ZSlot {
public:
	ZSlot() {
		m_WeaponID = 0;
	}

	int m_WeaponID;
};

/// Ä³¸¯ÅÍ ¼Ó¼º - ÀÌ °ªÀº º¯ÇÏÁE¾Ê´Â´Ù.
struct ZCharacterProperty_Old
{
	char		szName[MATCHOBJECT_NAME_LENGTH];
	char		szClanName[CLAN_NAME_LENGTH];
	MMatchSex	nSex;
	int			nHair;
	int			nFace;
	int			nLevel;
	float		fMaxHP;
	float		fMaxAP;
	int			nMoveSpeed;
	int			nWeight;
	int			nMaxWeight;
	int			nSafeFall;
	int			nGhost;				// Custom: Ghost Mode - Desperate
	ZCharacterProperty_Old() :	nSex(MMS_MALE),
							nHair(0),
							nFace(0),
							nLevel(1),
							fMaxHP(1000.f), 
							fMaxAP(1000.f), 
							nMoveSpeed(100), 
							nWeight(0), 
							nMaxWeight(100), 
							nSafeFall(100),
							nGhost(0)				// Custom: Ghost Mode - Desperate
							{ 
								szName[0] = 0;
								szClanName[0] = 0;
							}
	void SetName(const char* name) { strcpy(szName, name); }
	void SetClanName(const char* name) { strcpy(szClanName, name); }
};

/// ¸Þ¸ð¸®ÇÙ ¹æ¾ûÛE¸·Î »õ·Î ¸¸µEÄ³¸¯ÅÍ ¼Ó¼º - ±âÁ¸ ±¸Á¶Ã¼´Â ¸®ÇÃ·¹ÀÌ È£È¯¶§¹®¿¡ ³²°ÜµÎ¾úÀ½
// ±âÁ¸ ¼Ó¼º ±¸Á¶Ã¼¿¡¼­ ¾È¾²´Â ¸â¹öº¯¼ö´Â »©¹ö·È´Ù
struct ZCharacterProperty_CharClanName
{
	char		szName[MATCHOBJECT_NAME_LENGTH];
	char		szClanName[CLAN_NAME_LENGTH];
};
struct ZCharacterProperty
{
	MProtectValue<ZCharacterProperty_CharClanName> nameCharClan;
	MMatchSex	nSex;
	int			nHair;
	int			nFace;
	int			nLevel;
	MProtectValue<float>		fMaxHP;		// ÀÌ °ªÀ» ¹Ù²Ù°EÀÚ»E>¸®½ºÆùÇØ¼­ HP¸¦ »½Æ¢±âÇÏ´Â ¹æ½ÄÀÇ ÇØÅ·
	MProtectValue<float>		fMaxAP;
	MProtectValue<int>			nGhost;		// Custom: Ghost Mode - Desperate
	ZCharacterProperty() :	nSex(MMS_MALE),
		nHair(0),
		nFace(0),
		nLevel(1)
	{ 
		nameCharClan.Ref().szName[0] = 0;
		nameCharClan.Ref().szClanName[0] = 0;
		nameCharClan.MakeCrc();

		//fMaxHP.Set_MakeCrc(1000.f);
		//fMaxAP.Set_MakeCrc(1000.f);
		fMaxHP.Set_MakeCrc(DEFAULT_CHAR_HP);
		fMaxAP.Set_MakeCrc(DEFAULT_CHAR_AP);
		nGhost.Set_MakeCrc(0);				// Custom: Ghost Mode - Desperate
	}
	void SetName(const char* name)	   { nameCharClan.CheckCrc(); strcpy(nameCharClan.Ref().szName, name);	   nameCharClan.MakeCrc(); }
	void SetClanName(const char* name) { nameCharClan.CheckCrc(); strcpy(nameCharClan.Ref().szClanName, name); nameCharClan.MakeCrc(); }
	const char* GetName() { return nameCharClan.Ref().szName; }
	const char* GetClanName() { return nameCharClan.Ref().szClanName; }

	void CopyToOldStruct(ZCharacterProperty_Old& old)	// ¸®ÇÃ·¹ÀÌ ÀúÀå¿E
	{
		memcpy(old.szName, nameCharClan.Ref().szName, MATCHOBJECT_NAME_LENGTH);
		memcpy(old.szClanName, nameCharClan.Ref().szClanName, CLAN_NAME_LENGTH);
		
		old.nSex = nSex;
		old.nHair = nHair;
		old.nFace = nFace;
		old.nLevel = nLevel;
		old.fMaxHP = fMaxHP.Ref();		// TodoH(»E - ¸®ÇÃ·¹ÀÌ °EÃ..
		old.fMaxAP = fMaxAP.Ref();		// TodoH(»E - ¸®ÇÃ·¹ÀÌ °EÃ..
		old.nMoveSpeed = 100;
		old.nWeight = 0;
		old.nMaxWeight = 100;
		old.nSafeFall = 100;
		old.nGhost = nGhost.Ref();		// Custom: Ghost Mode - Desperate
	}

	void CopyFromOldStruct(ZCharacterProperty_Old& old)	// ¸®ÇÃ·¹ÀÌ Àç»ý¿E
	{
		memcpy(nameCharClan.Ref().szName, old.szName, MATCHOBJECT_NAME_LENGTH);
		memcpy(nameCharClan.Ref().szClanName, old.szClanName, CLAN_NAME_LENGTH);
		nameCharClan.MakeCrc();

		nSex = old.nSex;
		nHair = old.nHair;
		nFace = old.nFace;
		nLevel = old.nLevel;
		fMaxHP.Set_MakeCrc(old.fMaxHP);		// TodoH(상) - 리플레이 관련..
		fMaxAP.Set_MakeCrc(old.fMaxAP);		// TodoH(상) - 리플레이 관련..
		nGhost.Set_MakeCrc(old.nGhost);		// Custom: Ghost Mode - Desperate
	}
};

/// Ä³¸¯ÅÍ »óÅÂ°ª
struct ZCharacterStatus
{	// ¼öÁ¤½Ã ±âÁ¸ ¸®ÇÃ·¹ÀÌ ·ÎµE°úÓÁÇØ¾ßÇÕ´Ï´Ù
	int			nLife;
	int			nKills;
	int			nDeaths;
	int			nLoadingPercent;	// Ã³À½ ¹æ¿¡ µé¾ûÛÃ¶§ ·ÎµùÀÌ ´Ù µÇ¾ú´ÂÁöÀÇ ÆÛ¼¾Æ® 100ÀÌ µÇ¸E·ÎµùÀÌ ´ÙµÈ°Í
	int			nCombo;
	int			nMaxCombo;
	int			nAllKill;
	int			nExcellent;
	int			nFantastic;
	int			nHeadShot;
	int			nUnbelievable;
	int			nExp;
	int			nScore;
	int			nWinCount;
	int			nLossCount;
	int         isSpeed;
	int         isSuper;
	int         isJjang;
	int         isGodMode;
	int         isCam;
	int			isESP;
	int			nGivenDamage;
	int			nTakenDamage;
	int			nRoundTakenDamage;
	int			nRoundGivenDamage;
	int			nRoundGivenDamageOld;
	int			nLastShot;
	int			nRoundLastGivenDamage;
	int			nRoundLastTakenDamage;
	int         isNinja;
	int			bGhost;						// Custom: Ghost Mode - Desperate
#ifdef _POSTABINFO
	int			nHP;
	int			nAP;
#endif


#ifdef _BARNPC
	MUID		nNPCUID;
#endif

	bool		nIsShoot;

#ifdef _BULLETBAR
	int         nMaxCurrBullet;
#endif

#ifdef _KILLFEED
	MMatchWeaponType LastWeapon;
#endif

#ifdef _ICONCHAT
	bool			isTalking;
#endif

#ifdef _KILLSTREAK
	int         nKillStreakCount;
#endif
	ZCharacterStatus() :	
							nLife(10),
							nKills(0),
							nDeaths(0),
							nLoadingPercent(0),
							nCombo(0),
							nMaxCombo(0),
							nAllKill(0),
							nExcellent(0),
							nFantastic(0),
							nHeadShot(0),
							nUnbelievable(0),
							nExp(0),
							nScore(0),
							nWinCount(0),
		                    isSpeed(0),
		                    isSuper(0),
		                    isJjang(0),
		                    isCam(0), 
		                    isESP(0),
	                    	nGivenDamage(0),
		                    isGodMode(0),
		                    nTakenDamage(0),
	                    	nRoundLastGivenDamage(0),
	                    	nRoundLastTakenDamage(0),
	                    	nRoundGivenDamage(0),
							nLastShot(0),
							nRoundGivenDamageOld(0),
	                     	nRoundTakenDamage(0),
		                    isNinja(0),
							bGhost(0),				// Custom: Ghost Mode - Desperate
#ifdef _POSTABINFO
		                    nHP(0),
		                    nAP(0),
#endif
#ifdef _BARNPC
							nNPCUID(0,0),
#endif

							nIsShoot(false),

#ifdef _BULLETBAR
		                    nMaxCurrBullet(0),
#endif

#ifdef _KILLFEED
		                    LastWeapon(MWT_NONE),
#endif

#ifdef _ICONCHAT
		                   isTalking(false),
#endif

#ifdef _KILLSTREAK
		nKillStreakCount(0),
#endif
							nLossCount(0)
							{  }

	void AddKills(int nAddedKills = 1) { nKills += nAddedKills; }
	void AddDeaths(int nAddedDeaths = 1) { nDeaths += nAddedDeaths;  }
	void AddExp(int _nExp=1) { nExp += _nExp; }

};

struct ZCharacterStatus_Old
{	// ¼öÁ¤½Ã ±âÁ¸ ¸®ÇÃ·¹ÀÌ ·ÎµE°úÓÁÇØ¾ßÇÕ´Ï´Ù
	int			nLife;
	int			nKills;
	int			nDeaths;
	int			nLoadingPercent;	// Ã³À½ ¹æ¿¡ µé¾ûÛÃ¶§ ·ÎµùÀÌ ´Ù µÇ¾ú´ÂÁöÀÇ ÆÛ¼¾Æ® 100ÀÌ µÇ¸E·ÎµùÀÌ ´ÙµÈ°Í
	int			nCombo;
	int			nMaxCombo;
	int			nAllKill;
	int			nExcellent;
	int			nFantastic;
	int			nHeadShot;
	int			nUnbelievable;
	int			nExp;

	ZCharacterStatus_Old() :
		nLife(10),
		nKills(0),
		nDeaths(0),
		nLoadingPercent(0),
		nCombo(0),
		nMaxCombo(0),
		nAllKill(0),
		nExcellent(0),
		nFantastic(0),
		nHeadShot(0),
		nUnbelievable(0),
		nExp(0)
	{  }

	void AddKills(int nAddedKills = 1) { nKills += nAddedKills; }
	void AddDeaths(int nAddedDeaths = 1) { nDeaths += nAddedDeaths; }
	void AddExp(int _nExp = 1) { nExp += _nExp; }
};

// ÀÌ°ÍÀº Ä³¸¯ÅÍ³¢¸® ÁÖ°úÕÞ´Â µ¥ÀÌÅÍ·Î ³ªÁß¿¡ ÅõÇ¥ ÆÇÁ¤ÀÇ ±Ù°Å°¡ µÈ´Ù.
/*
struct ZHPItem {
	MUID muid;
	float fHP;
};
*/

//struct ZHPInfoItem : public CMemPoolSm<ZHPInfoItem>{
//	ZHPInfoItem()	{ pHPTable=NULL; }
//	~ZHPInfoItem()	{ if(pHPTable) delete pHPTable; }
//	
//	int		nCount;
//	ZHPItem *pHPTable;
//};


//class ZHPInfoHistory : public list<ZHPInfoItem*> {
//};

#define CHARACTER_ICON_DELAY		2.f
#define CHARACTER_ICON_FADE_DELAY	.2f
#define CHARACTER_ICON_SIZE			32.f		// 아이콘 크기 (640x480기준)

class ZModule_HPAP;
class ZModule_QuestStatus;

// 1bit ÆÐÅ·
struct ZCharaterStatusBitPacking { 
	union {
		struct {
			bool	m_bLand:1;				// 지금 발을 땅에 대고있는지..
			bool	m_bWallJump:1;			// 벽점프 중인지
			bool	m_bJumpUp:1;			// 점프올라가는중
			bool	m_bJumpDown:1;			// 내려가는중
			bool	m_bWallJump2:1;			// 이건 walljump 후에 착지시 두번째 튕겨져 나오는 점프..
			bool	m_bTumble:1;			// 덤블링 중
			bool	m_bBlast:1;				// 띄워짐당할때 ( 올라갈때 )
			bool	m_bBlastFall:1;			// 띄워져서 떨어질때
			bool	m_bBlastDrop:1;			// 떨어지다 땅에 튕길때
			bool	m_bBlastStand:1;		// 일어날때
			bool	m_bBlastAirmove:1;		// 공중회전후 착지
			bool	m_bSpMotion:1;
			bool	m_bCommander:1;			///< 대장
		//	bool	m_bCharging:1;			// 힘모으고 있는중
		//	bool	m_bCharged:1;			// 힘모인상태
			bool	m_bLostConEffect:1;		// 네트웍 응답이 없을때 머리에 뜨는 이펙트가 나와야 하는지.
			bool	m_bChatEffect:1;		// 채팅시 머리에 뜨는 이펙트가 나와야 하는지.
			bool	m_bGhost : 1;

#ifdef _ICONCHAT
			bool	m_bChatVoice : 1;     
#endif

			bool	m_bBackMoving:1;		// 뒤로 이동할때

			bool	m_bAdminHide:1;					///< admin hide 되어있는지..
			bool	m_bDie:1;						///< 죽었는지 체크
			bool	m_bStylishShoted:1;				///< 마지막으로 쏜게 스타일리쉬 했는지 체크
			bool	m_bFallingToNarak:1;			///< 나락으로 떨어지고 있는지 여부
			bool	m_bStun:1;						///< stun ..움직일수없게된상태.
			bool	m_bDamaged:1;					///< 데미지 여부
				
			bool	m_bPlayDone:1;				// 애니메이션 플레이가 다 되었는지. 다음동작으로 넘어가는 기준
			bool	m_bPlayDone_upper:1;		// 상체 애니메이션 플레이가 다 되었는지. 다음동작으로 넘어가는 기준
			bool	m_bIsLowModel:1;
			bool	m_bTagger:1;					///< 술래
			bool	m_bSniping:1;				// Custom: Snipers
			bool	m_bFrozen:1;				// Custom: Frozen (refuse any sort of input + movement)
		};

		DWORD dwFlagsPublic;
	}; // 패킹 끝
};

struct ZUserAndClanName
{
	char m_szUserName[MATCHOBJECT_NAME_LENGTH];			///< 유져이름(운영자는 '운영자')
	// Custom: Char name + Clan name fix
	char m_szUserAndClanName[MATCHOBJECT_NAME_LENGTH + CLAN_NAME_LENGTH];  ///< 유저이름(클랜이름) 요걸 스캔해서 애들이 메모리 핵을 쓰니까 요걸 숨겨버리자..
};
struct ZShotInfo;


/// 캐릭터 클래스
class ZCharacter : public ZCharacterObject
{
	MDeclareRTTI;
	//friend class ZCharacterManager;
private:
	unsigned int	m_skillMapBestTime;
#ifdef _DYNAMIC
	bool            MeshesFinishedLoading;
#endif

public:
	std::vector<std::vector<int>> m_LadderWarUpgradeLevel;
	void DrawFriendOutlineXray();
protected:

	// 모듈들. 한번 생성되고 소멸될때 같이 지운다
	ZModule_HPAP			*m_pModule_HPAP;
	ZModule_QuestStatus		*m_pModule_QuestStatus;
	ZModule_Resistance		*m_pModule_Resistance;
	ZModule_FireDamage		*m_pModule_FireDamage;
	ZModule_ColdDamage		*m_pModule_ColdDamage;
	ZModule_PoisonDamage	*m_pModule_PoisonDamage;
	ZModule_LightningDamage	*m_pModule_LightningDamage;
	ZModule_HealOverTime	*m_pModule_HealOverTime;

	MProtectValue<MTD_CharInfo>			m_MInitialInfo;
	MUID								m_uidLastTarget;
	

	ZCharacterProperty					m_Property;		///< HP 등의 캐릭터 속성
	MProtectValue<ZCharacterStatus>		m_Status;		///< 캐릭터 초기정보
	int						m_nCountryFlag;


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 게임 안에서의 케릭터 버프 관련 정보 및 함수
protected:	
	///버프정보임시주석 ZCharacterBuff m_CharacterBuff;				///< 적용되고 있는 버프

	float m_fPreMaxHP;
	float m_fPreMaxAP;
	int   m_nGhost;					// Custom: Ghost Mode - Desperate

public:

#ifdef _ZGUARD
	int m_nKey;
	int m_nKeyAlt;
	unsigned long int st_nLastPeerReload;
	unsigned long int st_nLastPeerDash;
	unsigned long int st_nLastSlashTime;

	LARGE_INTEGER m_nLastQPCTime;      // Lưu nhịp thời gian QPC cuối cùng
	double m_fIntervalHistory[5];      // Mảng lưu 5 khoảng cách thời gian gần nhất (ms)
	int    m_nHistoryIdx;              // Chỉ số của mảng history
	int	   m_nMacroWarningCount;       // Bộ đếm số lần vi phạm Macro

	LARGE_INTEGER m_nLastDashQPCTime;    // QPC riêng cho Dash
	double m_fDashIntervalHistory[5];    // Lịch sử Dash
	int    m_nDashHistoryIdx;
	int    m_nDashMacroWarningCount;     // Bộ đếm vi phạm Dash
#endif
#ifdef _BARNPC
	int         nBossDamage;
#endif

#ifdef _HITSCOUNT
	int m_nHits;
#endif

#ifdef _ZGUARD
	int nType;

	void SetnType(int n)
	{
		nType = n;
	}
	int nGetnType()
	{
		return nType;
	}
	float nLastDashX;

	void SetLastDashX(float n)
	{
		nLastDashX = n;
	}

	float GetLastDashX()
	{
		return nLastDashX;
	}
#endif

	void ApplyBuffEffect();

	float GetMaxHP();
	float GetMaxAP();
	float GetHP();
	float GetAP();
	void InitAccumulationDamage();
	float GetAccumulationDamage();
	void EnableAccumulationDamage(bool bAccumulationDamage);

	__forceinline void SetMaxHP(float nMaxHP) { m_pModule_HPAP->SetMaxHP(nMaxHP); }
	__forceinline void SetMaxAP(float nMaxAP) { m_pModule_HPAP->SetMaxAP(nMaxAP); }

	__forceinline void SetHP(float nHP)	{ m_pModule_HPAP->SetHP(nHP); }
	__forceinline void SetAP(float nAP) { m_pModule_HPAP->SetAP(nAP); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	MProtectValue<ZUserAndClanName>*  m_pMUserAndClanName;  ///< Ä³¸¯¸EÅ¬·£¸E

	struct KillInfo {
		int			m_nKillsThisRound;				///< ÀÌ¹ø¶ó¿ûÑå¿¡¼­ÀÇ kills ( unbelievable ÆÇÁ¤)
		float		m_fLastKillTime;				///< ¸¶Áö¸·¿¡ Á×ÀÎ ½Ã°£ (excellent)¸¦ Ç¥½ÃÇÏ±EÀ§ÇÔ
	};
	MProtectValue<KillInfo> m_killInfo;

	struct DamageInfo {
		DWORD			m_dwLastDamagedTime;		// ¸¶Áö¸·À¸·Î °ø°Ý ¹ÞÀº ½Ã°£
		ZSTUNTYPE		m_nStunType;				///< ¸Â´Â ¾Ö´Ï¸ÞÀÌ¼Ç Á¾·E. 2:¸¶Áö¸·Å¸°Ý,4:lightning,5:·çÇÁ
		ZDAMAGETYPE		m_LastDamageType;			///< ¸¶Áö¸· µ¥¹ÌÁEÅ¸ÀÔ		
		MMatchWeaponType m_LastDamageWeapon;		///< ¸¶Áö¸· µ¥¹ÌÁE¹«±E.
		rvector			m_LastDamageDir;			///< ¸¶Áö¸· µ¥¹ÌÁE¹æÇE( Á×´Â ¸ð¼ÇÀ» °áÁ¤ )
		float			m_LastDamageDot;
		float			m_LastDamageDistance;

		MUID			m_LastThrower;				///< ¸¶Áö¸· ¶ç¿E»ç¶E
		float			m_tmLastThrowClear;			///< ¸¶Áö¸· ¶ç¿E»ç¶EÀØ¾ûÑµ µÇ´Â½Ã°£
	};
	MProtectValue<DamageInfo> m_damageInfo;

	int	m_nWhichFootSound;	///< ¹ß¼Ò¸®¸¦ ¹ø°¥¾Æ ³»±âÀ§ÇØ ¾ûÐÀ ¹ßÀÎÁEÀúÀåÇÑ º¯¼E

	MProtectValue<DWORD>* m_pMdwInvincibleStartTime;		// ¹«ÀûÀÇ ½ÃÀÛ ½Ã°£
	MProtectValue<DWORD>* m_pMdwInvincibleDuration;		// ¹«ÀûÀÇ Áö¼Ó½Ã°£

	virtual void UpdateSound();
public:
	void InitMesh();	///< Ä³¸¯ÅÍ ÆÄÃEµûÜÇ ¸Þ½¬Á¤º¸ ¼¼ÆÃ. InitCharInfo¿¡¼­ È£ÃE
	void InitProperties();

protected:

//	float m_fIconStartTime[ZCI_END];	///< ¸Ó¸®À§¿¡ ¶ß´Â ¾ÆÀÌÄÜµE

	void CheckLostConn();
	virtual void OnLevelDown();
	virtual void OnLevelUp();
	virtual void OnDraw();
//	virtual void RegisterModules();
	virtual void	OnDie();

	void ShiftFugitiveValues();
public:
	float	m_fLastValidTime;		// Dead Reckoning¿¡ ÇÊ¿äÇÑ º¯¼E-> Áö±Ý ÄÚµå¿¡¼­ ÇÊ¿ä¾ø¾ûÖ¸ÀÓ
	DWORD		m_dwIsValidTime;	//µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ· ¹æ¾ûÔ¦ À§ÇÑ Å¸ÀÓ Ã¼Å©..°­º£±âÂÊ..

//	float	m_fDistToFloor;			// ¹Ù´Ú±ûÝöÀÇ °Å¸®
//	rplane	m_FloorPlane;			// ¹Ù´Ú Æò¸éÀÇ ¹æÁ¤½Ä
//	float	m_fFallHeight;			// ³«ÇÏ°¡ ½ÃÀÛµÈ ½ÃÁ¡

	MProtectValue<ZCharaterStatusBitPacking> m_dwStatusBitPackingValue;	// ¾EÂ ÀÎ°£ÀûÀ¸·Î crcÃ¼Å©±ûÝE¸øÇÏ°Ú´Ù...;;

	//mmemory proxy
	MProtectValue<bool>* m_bCharged;
	MProtectValue<bool>* m_bCharging;

	

	MProtectValue<float>	m_fChargedFreeTime;		// Èû¸ðÀÎ°Ô Ç®¸®´Â ½Ã°£
	MProtectValue<int>		m_nWallJumpDir;			// º®Á¡ÇÁÇÏ´Â ¹æÇE
	MProtectValue<int>		m_nBlastType;			// ´Üµµ°è¿­Ãß°¡~


	ZC_STATE_LOWER	m_SpMotion;

	
	/*
	bool	m_bClimb;				// ÅÎµûÛ¡ ¿Ã¶ó°¡°úÜÖ´Â °æ¿E
	rvector	m_ClimbDir;				// ¿Ã¶ó°¡´Â ¹æÇE
	float	m_fClimbZ;				// ¿Ã¶ó°¡±E½ÃÀÛÇÑ ³ôÀÌ
	rplane	m_ClimbPlane;
	*/

//	bool	m_bRendered;				///< ÀÌÀEÁ·¹ÀÓ¿¡¼­ È­¸é¿¡ ³ª¿Ô´ÂÁE

	/////// ³×Æ®¿÷¿¡¼­ ÀÓ½Ã·Î ¿Ê °¥¾Æ ÀÔ±EÀ§ÇÑ ÇöÀE¼±ÅÃµÈ ÆÄÃ÷Á¤º¸
	//int m_t_parts[6];	//³²ÀÚ
	//int m_t_parts2[6];	//¿©ÀÚ
	
	
	// rvector		m_vProxyPosition, m_vProxyDirection;	///< ÀÌµ¿°ªÀÌ ÀÖ´Â ¾Ö´Ï¸ÞÀÌ¼ÇÀÇ ·»´õ¸µ À§Ä¡¸¦ À§ÇÑ º¯¼E
	
//	ZHPInfoHistory		m_HPHistory;		///< ÅõÇ¥¸¦ À§ÇØ ¸ûßÊ°£ÀÇ µ¥ÀÌÅÍ¸¦ °¡Áö°úÜÖ´Â´Ù

	ZCharacter();
	virtual ~ZCharacter();

	virtual bool Create(MTD_CharInfo* pCharInfo/*, MTD_CharBuffInfo* pCharBuffInfo*/);//¹öÇÁÁ¤º¸ÀÓ½ÃÁÖ¼® 
	virtual void Destroy();
	
	void InitMeshParts();
	
	void EmptyHistory();

	rvector m_TargetDir;
	rvector m_DirectionLower,m_DirectionUpper;


private:
	//Custom: Character Posture
	v3 m_vProxyPosition{ 0, 0, 0 };
public:

	// ÀÌ º¯¼öµéÀº ÀÌµ¿¼Óµµ ÇØÅ·´EóÀÌ µÊ
	MProtectValue<rvector> m_RealPositionBefore;			// ¾Ö´Ï¸ÞÀÌ¼ÇÀÇ ¿òÁ÷ÀÓÀ» ÃßÀûÇÏ±EÀ§ÇÑ º¯¼E
	MProtectValue<rvector> m_AnimationPositionDiff;
	MProtectValue<rvector> m_Accel;


	MProtectValue<ZC_STATE_UPPER>	m_AniState_Upper;		// »óÃ¼ ¾Ö´Ï¸ÞÀÌ¼Ç »óÅÂ
	MProtectValue<ZC_STATE_LOWER>	m_AniState_Lower;		// ÇÏÃ¼ ¾Ö´Ï¸ÞÀÌ¼Ç »óÅÂ (±âº»)
	ZANIMATIONINFO *m_pAnimationInfo_Upper,*m_pAnimationInfo_Lower;

	void AddIcon(int nIcon);
//	float GetIconStartTime(int nIcon);

	MProtectValue<int>				m_nVMID;	// VisualMesh ID
	//MUID	m_UID;		// ¼­¹ö¿¡¼­ ºÎ¿©ÇÑ Ä³¸¯ÅÍÀÇ UID
	MProtectValue<MMatchTeam>		m_nTeamID;	// Team ID

	MProtectValue<MCharacterMoveMode>		m_nMoveMode;
	MProtectValue<MCharacterMode>			m_nMode;
	MProtectValue<MCharacterState>			m_nState;

//	RVisualMesh*			m_pVMesh;

//	float	m_fLastAdjustedTime;
	MProtectValue<float>	m_fAttack1Ratio;//Ä®ÁúµûÜÇ °æ¿EÃ¹¹øÂ°ºñÀ²À» ³ªÁßÅ¸¿¡µµ Àû¿EÑ´Ù..
//	rvector m_AdjustedNormal;

	/*
	bool	m_bAutoDir;

	bool	m_bLeftMoving,m_bRightMoving;
	bool	m_bForwardMoving;
	*/

//	float	m_fLastUpdateTime;
	float	m_fLastReceivedTime;	// basicinfo µ¥ÀÌÅÍ¸¦ ¸¶Áö¸· ¹ÞÀº ½Ã°£
	MProtectValue<float> m_fHitBoxRealVal;
	MProtectValue<float> m_fHitBoxRealVal2;
	float	m_fTimeOffset;				// ³ª¿Í ÀÌ Ä³¸¯ÅÍÀÇ ½Ã°£Â÷ÀÌ
	float	m_fAccumulatedTimeError;	// ÇöÀç½Ã°£ÀÇ ´©ÀûµÈ ¿ÀÂE
	int		m_nTimeErrorCount;			// ÇöÀç½Ã°£ÀÇ ¿ÀÂ÷°¡ ´©ÀûµÈ È¸¼E

	float	m_fGlobalHP;			// ´Ù¸¥»ç¶÷µéÀÌ º¼¶§ ÀÌÄ³¸¯ÅÍÀÇ HPÀÇ Æò±Õ.. ÅõÇ¥¸¦ À§ÇÔ.
	int		m_nReceiveHPCount;		// Æò±Õ³»±âÀ§ÇÑ...

	
	//float	m_fAveragePingTime;				// ÀÏÁ¤½Ã°£ Æò±Õ ³×Æ®¿EÁö¿¬ ½Ã°£

	// ±Û·Î¹E½Ã°£°úÀÇ Â÷ÀÌ¸¦ ´©ÀûÇÑ´Ù. ÀÌ ¼ýÀÚ°¡ Á¡Á¡ Ä¿Áö´Â À¯Àú´Â ½ºÇÇµåÇÙÀÌ ÀÇ½ÉµÈ´Ù.
	//#define TIME_ERROR_CORRECTION_PERIOD	20

	//int		m_nTimeErrorCount;
	//float	m_TimeErrors[TIME_ERROR_CORRECTION_PERIOD];
	//float	m_fAccumulatedTimeError;

	//list<float> m_PingData;			// ¸ûÌ³ÀÇ ÇÎ Å¸ÀÓÀ» °¡Áö°EÆò±ÕÀ» ³½´Ù.

//	DWORD m_dwBackUpTime;

	// ¹«±E¹ß»ç¼ÓµµÀÇ ÀÌ»óÀ¯¹«¸¦ °ËÃâÇÑ´Ù.
	int		m_nLastShotItemID;
	float	m_fLastShotTime;
	int		m_nDamageThisRound;
	int		GhostTime;     // Custom: Ghost Mode - Desperate
#ifdef _AFKSYSTEM
	//DWORD	m_nLastKeyTime;
#endif
	// Custom: Infected
	bool	m_bInfected;

	MProtectValue<float> m_fHitBox;
	MProtectValue<float> m_fHitBox2;

	// Custom: GunGame
	// Description: Properties derived from MatchServer for updating purposes
	int		m_nGunGameWeaponID[3];
	int		m_nGunGameWeaponLevel;

	float	m_fDamageCaused;

	bool	m_bSBTest;
	DWORD	m_dwSBTestStart;

	// Custom: NOLEAD basic info check
	DWORD	m_dwLastBasicInfoTime;

	void SetInvincibleTime(int nDuration);
	inline bool	IsInvincible();

	char	m_szSlashChargingName[24];
	char	m_szSlashChargedName[24];
	char	m_szSlashName[24];
	char	m_szSlashCustomSound[24];

	bool IsMan();
	void DrawOutline();

	virtual void  OnUpdate(float fDelta);

	//¹öÇÁÁ¤º¸ÀÓ½ÃÁÖ¼® virtual void  UpdateBuff();
	virtual void  UpdateSpeed();
	virtual float GetMoveSpeedRatio();

	virtual void UpdateVelocity(float fDelta);	
	virtual void UpdateHeight(float fDelta);

	//Custom: Character Posture
	virtual void UpdateMotion(float fDelta=0.f);	
	virtual void UpdateDirection(float fDelta, const v3& Direction);

	virtual void UpdateAnimation();

	int  GetSelectWeaponDelay(MMatchItemDesc* pSelectItemDesc);

	void UpdateLoadAnimation();

	void Stop();
	//void DrawForce(bool bDrawShadow);	

	void CheckDrawWeaponTrack();
	void UpdateSpWeapon();

	void SetAnimation(char *AnimationName,bool bEnableCancel,int tick);
	void SetAnimation(RAniMode mode,char *AnimationName,bool bEnableCancel,int tick);

	void SetAnimationLower(ZC_STATE_LOWER nAni);
	void SetAnimationUpper(ZC_STATE_UPPER nAni);
	
	ZC_STATE_LOWER GetStateLower() { return m_AniState_Lower.Ref(); }
	ZC_STATE_UPPER GetStateUpper() { return m_AniState_Upper.Ref(); }

	bool IsUpperPlayDone()	{ return m_dwStatusBitPackingValue.Ref().m_bPlayDone_upper; }

	bool IsMoveAnimation();		// 움직임이 포함된 애니메이션인가 ?
	bool IsMoveGhost();			// Custom: Ghost Mode - Desperate

//	bool IsRendered()		{ return m_bRendered; }

	bool IsTeam(ZCharacter* pChar);

	bool IsRunWall();
	bool IsMeleeWeapon();
	virtual bool IsCollideable();

//	void SetAnimationForce(ZC_STATE nState, ZC_STATE_SUB nStateSub) {}

	void SetTargetDir(rvector vDir); 

//	bool Pick(int x,int y,RPickInfo* pInfo);
//	bool Pick(int x,int y,rvector* v,float* f);
	virtual bool Pick(rvector& pos,rvector& dir, RPickInfo* pInfo = NULL);

//	bool Move(rvector &diff);

	void OnSetSlot(int nSlot,int WeaponID);
	void OnChangeSlot(int nSlot);

	virtual void OnChangeWeapon(char* WeaponModelName);
	void OnChangeParts(RMeshPartsType type,int PartsID);
	void OnAttack(int type,rvector& pos);
//	void OnHeal(ZCharacter* pAttacter,int type,int heal);
	void OnShot();

	void ChangeWeapon(MMatchCharItemParts nParts, bool bReSelect = false);

	int GetLastShotItemID()	{ return m_nLastShotItemID; }
	float GetLastShotTime()						{ return m_fLastShotTime; }
	bool CheckValidShotTime(int nItemID, float fTime, ZItem* pItem);
	void UpdateValidShotTime(int nItemID, float fTime)	
	{ 
		m_nLastShotItemID = nItemID;
		m_fLastShotTime = fTime;
	}

	bool IsDie() { return m_dwStatusBitPackingValue.Ref().m_bDie; }
	void ForceDie() 
	{
		SetHP(0); 
		m_dwStatusBitPackingValue.Ref().m_bDie = true; 
	}		// ÀÌ°ÍÀº ±×³É Á×Àº »óÅÂ·Î ¸¸µé±EÀ§ÇÒ¶§ »ç¿E

	void SetAccel(rvector& accel) { m_Accel.Set_CheckCrc(accel); }
	virtual void SetDirection(rvector& dir);

	struct SlotInfo
	{
		int		m_nSelectSlot;
		ZSlot	m_Slot[ZC_SLOT_END];
	};
	MProtectValue<SlotInfo>	m_slotInfo;

	MProtectValue<ZCharacterStatus>& GetStatus()	{ return m_Status; }

	// Character Property
	ZCharacterProperty* GetProperty() { return &m_Property; }

	MMatchUserGradeID GetUserGrade()	{ return m_MInitialInfo.Ref().nUGradeID; }
	unsigned int GetClanID()	{ return m_MInitialInfo.Ref().nClanCLID; }

	void SetName(const char* szName) { m_Property.SetName(szName); }

#undef GetUserName
	const char *GetUserName()			{ return m_pMUserAndClanName->Ref().m_szUserName;	}		// ¿ûÛµÀÚ´Â Ã³¸®µÊ
	const char* GetCharName()           { return m_pMUserAndClanName->Ref().m_szUserName; }
	const char *GetUserAndClanName()	{ return m_pMUserAndClanName->Ref().m_szUserAndClanName; }	// ¿ûÛµÀÚ´Â Å¬·£Ç¥½Ã ¾ÈÇÔ
	bool IsAdminName();
	bool IsGhost();				// Custom: Ghost Mode - Desperate

#ifdef _VIPGRADES
	bool IsVIP1Name();
	bool IsVIP2Name();
	bool IsVIP3Name();
	bool IsVIP4Name();
	bool IsVIP5Name();
	bool IsVIP6Name();
	bool IsVIP7Name();
#endif

#ifdef _EVENTGRD
	bool IsEvent1Name();
	bool IsEvent2Name();
	bool IsEvent3Name();
	bool IsEvent4Name();
	bool IsEvent5Name();
	bool IsEvent6Name();
	bool IsStaffName();
	bool Event();
#endif

	bool IsAdminHide()			{ return m_dwStatusBitPackingValue.Ref().m_bAdminHide;	}
	void SetAdminHide(bool bHide) { m_dwStatusBitPackingValue.Ref().m_bAdminHide = bHide; }
	
//	void SetMoveSpeed(int nMoveSpeed) { m_Property.nMoveSpeed = nMoveSpeed; }
//	void SetWeight(int nWeight) { m_Property.nWeight = nWeight; }
//	void SetMaxWeight(int nMaxWeight) { m_Property.nMaxWeight = nMaxWeight; }
//	void SetSafeFall(int nSafeFall) { m_Property.nSafeFall = nSafeFall; }

#ifdef _KILLFEED
	MMatchWeaponType WType;
	void SetWeaponDamaged(MMatchWeaponType wWeapon) { WType = wWeapon; }
	MMatchWeaponType GetWeaponDamaged() { return WType; }
	MMatchWeaponType GetLastW() { return GetStatus().Ref().LastWeapon; }
	MMatchWeaponType m_nKillShotWeapon;
#endif

	int GetKils() { return GetStatus().Ref().nKills; }

#ifdef _DEATHEFFECT
	string GetName() { return GetCharInfo()->szName; }
#endif

#ifdef _KILLSTREAK
	int GetKillStreaks() { return GetStatus().Ref().nKillStreakCount; }
#endif
	bool CheckDrawGrenade();

//	int GetWeaponEffectType();
//	float GetCurrentWeaponRange();
//	float GetMeleeWeaponRange();

	bool GetStylishShoted() { return m_dwStatusBitPackingValue.Ref().m_bStylishShoted; }
	void UpdateStylishShoted();
	
	MUID GetLastAttacker() { return m_pModule_HPAP->GetLastAttacker(); }
	void SetLastAttacker(MUID uid) { m_pModule_HPAP->SetLastAttacker(uid); }
	ZDAMAGETYPE GetLastDamageType() { return m_damageInfo.Ref().m_LastDamageType; }
	void SetLastDamageType(ZDAMAGETYPE type) { MEMBER_SET_CHECKCRC(m_damageInfo, m_LastDamageType, type); }

	bool DoingStylishMotion();
	
	bool IsObserverTarget();

	MMatchTeam GetTeamID() { return m_nTeamID.Ref(); }
	MMatchTeam GetTeamID() const { return m_nTeamID.Ref(); }

	void SetTeamID(MMatchTeam nTeamID) { m_nTeamID.Set_CheckCrc(nTeamID); }
	bool IsSameTeam(ZCharacter* pCharacter) 
	{ 
		if (pCharacter->GetTeamID() == -1) return false;
		if (pCharacter->GetTeamID() == GetTeamID()) return true; return false; 
	}

	bool IsTagger() { return m_dwStatusBitPackingValue.Ref().m_bTagger; }
	void SetTagger(bool bTagger) { m_dwStatusBitPackingValue.Ref().m_bTagger = bTagger; }

	void SetLastThrower(MUID uid, float fTime) { MEMBER_SET_CHECKCRC(m_damageInfo, m_LastThrower, uid); MEMBER_SET_CHECKCRC(m_damageInfo, m_tmLastThrowClear, fTime); }
	const MUID& GetLastThrower() { return m_damageInfo.Ref().m_LastThrower; }
	float GetLastThrowClearTime() { return m_damageInfo.Ref().m_tmLastThrowClear; }

	// µ¿ÀÛÀÌ³ª ÀÌº¥Æ®¿¡ °EÑ °ÍµE
	//void Damaged(ZCharacter* pAttacker, rvector& dir, RMeshPartsType partstype,MMatchCharItemParts wtype,int nCount=-1);
	//void DamagedGrenade(MUID uidOwner, rvector& dir, float fDamage,int nTeamID);
	//void DamagedFalling(float fDamage);
	//void DamagedKatanaSplash(ZCharacter* pAttacker,float fDamageRange);

	void Revival();
	void Die();
	void ActDead();
	__forceinline void InitHPAP();	
	virtual void InitStatus();
	virtual void InitRound();
	virtual void InitItemBullet();
	virtual void InitSlashEquipment();

	void TestChangePartsAll();
	void TestToggleCharacter();
	void InfectCharacter(bool bFirst);

	virtual void OutputDebugString_CharacterState();

	void ToggleClothSimulation();
	void ChangeLowPolyModel();
	bool IsFallingToNarak() { return m_dwStatusBitPackingValue.Ref().m_bFallingToNarak; }

	MMatchItemDesc* GetSelectItemDesc() {
		if(GetItems())
			if(GetItems()->GetSelectedWeapon())
				return GetItems()->GetSelectedWeapon()->GetDesc();
		return NULL;
	}

	void LevelUp();
	void LevelDown();

	bool Save(ZFile *file);
	bool Load(ZFile *file,int nVersion);	// ³ªÁß¿¡ MZFile * ·Î Æ÷ÆÃ
#ifdef _UPCHARCMD
	bool Load(MTD_CharInfo* pCharInfo);	
#endif

	RMesh *GetWeaponMesh(MMatchCharItemParts parts);

	virtual float ColTest(const rvector& pos, const rvector& vec, float radius, rplane* out=0);
	virtual bool IsAttackable();

	virtual bool IsGuard();
	virtual void OnMeleeGuardSuccess();
	virtual void OnDamagedAnimation(ZObject *pAttacker,int type);

	unsigned long int nHunter_LastWeapons;
	// ZObject¿¡ ¸Â°Ô ¸¸µEµ¿ÀÛÀÌ³ª ÀÌº¥Æ®¿¡ °EÑ °ÍµE
	virtual ZOBJECTHITTEST HitTest( const rvector& origin, const rvector& to, float fTime ,rvector *pOutPos=NULL );

	virtual void OnKnockback(rvector& dir, float fForce);
//	virtual void OnDamage(int damage, float fRatio = 1.0f);
	
	virtual void OnDamagedAPlayer(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio = 1.f, int nMeleeTpye = -1);
	virtual void OnDamagedAPlayer(vector<MTD_AntiLeadN*> vShots);
	
	virtual void OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio=1.f, int nMeleeType=-1);
	virtual void OnScream();
	void SetCountry(int nCountry) { m_nCountryFlag = nCountry; }

	int GetDTLastWeekGrade() { return m_MInitialInfo.Ref().nDTLastWeekGrade; }
	int	GetCountry()         { return m_MInitialInfo.Ref().nCountryFlag; }
	int	GetPWPoint() { return m_MInitialInfo.Ref().nPWPoints; } //Custom: LadderPoints Rank
	int GetGradeID() { return m_MInitialInfo.Ref().nUGradeID; }
#ifdef _UPCHARCMD
	MTD_CharInfo* GetCharInfo() const { return (MTD_CharInfo*)&m_MInitialInfo.Ref(); }
#else
	const MTD_CharInfo* GetCharInfo() const { return &m_MInitialInfo.Ref(); }
#endif

	MUID GetLastTarget() { return m_uidLastTarget; }
	void SetLastTarget(MUID uidTarget) { m_uidLastTarget = uidTarget; }

	unsigned int GetSkillMapBestTime() { return m_skillMapBestTime; }
	void SetSkillMapBestTime(unsigned int n) { m_skillMapBestTime = n; }

public:
	void SpyHealthBonus(int nHPAP);

	void InitSpyWeaponBullet();

	virtual void DistributeSpyItem(vector<MMatchSpyItem>& vt);
	void TakeoutSpyItem();

};

void ZChangeCharParts(RVisualMesh* pVMesh, MMatchSex nSex, int nHair, int nFace, const unsigned long int* pItemID);
void ZChangeCharPartsNoAvatar(RVisualMesh* pVMesh, MMatchSex nSex, int nHair, int nFace, const unsigned long int* pItemID);
void ZChangeCharWeaponMesh(RVisualMesh* pVMesh, unsigned long int nWeaponID);
bool CheckTeenVersionMesh(RMesh** ppMesh);

//dll-injectionÀ¸·Î È£ÃâÇÏÁE¸øÇÏµµ·Ï °­Á¦ ÀÎ¶óÀÌ´×
//__forceinline void ZCharacter::InitHPAP(); // Custom: Moved this to ZCharacter.cpp

//////////////////////////////////////////////////////////////////////////
//	ENUM
//////////////////////////////////////////////////////////////////////////
enum CHARACTER_LIGHT_TYPE
{
	GUN,
	SHOTGUN,
	CANNON,
	NUM_LIGHT_TYPE,
};

//////////////////////////////////////////////////////////////////////////
//	STRUCT
//////////////////////////////////////////////////////////////////////////
typedef struct
{
	int			iType;
	float		fLife;
	rvector		vLightColor;
	float		fRange;
}	sCharacterLight;




int gaepanEncode(int a, int depth);
int gaepanDecode(int a, int depth);

#endif