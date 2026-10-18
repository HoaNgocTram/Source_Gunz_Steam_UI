#ifndef _MMATCHTRANSDATATYPE_H
#define _MMATCHTRANSDATATYPE_H

#include "MMatchObject.h"
#include "MMatchRule.h"		// MMATCH_GAMETYPE, MMATCH_ROUNDSTATE, MMATCH_ROUNDRESULT ÂE¶ -> Á¤¸®¿ä¸Á
#include "MMatchStageSetting.h"
#include "MMatchGameType.h"
#include "MMatchGlobal.h"

#pragma pack(push, old)
#pragma pack(1)

struct MTD_LadderWarsCounterInfo
{
	int Count[MPLADDERWARSTYPE_MAX];
};
struct MTD_AccountCharInfo
{
	char				szName[MATCHOBJECT_NAME_LENGTH];
	char				nCharNum;
	unsigned char		nLevel;

	int					nLC;     // Medal / Ladder Coins
	int					nBP;     // Bounty Points
	unsigned long int	nXP;     // Experience Points
	int					nCash;   // Coins / Cash trong tài khoản
	int					nRankScore;    // Điểm Rank (Score)
	int					nWins;         // Số trận thắng
	int					nLosses;       // Số trận thua
	int					nCustomKill;
	int					nCustomDeath;
};

#ifdef _PLAYERINO
struct MTD_Information
{
	int nLevel;
	int nBounty;
	int nLC;
	int nBP;
	int nEXP;
	int nFace;
	int nHair;
	int nSex;
	int nKill;
	int nDeath;
	char szName[128];
	char szClan[128];
	int nClanEmblem;
	MUID uidTarget;
	int nHP;
	int nAP;
	unsigned long int nEquipedItemID[MMCIP_END];
};
struct MTD_InformationClothes
{
	int nFace;
	int nHair;
	int nSex;
	unsigned long int nEquipedItemID[MMCIP_END];
};
#endif

struct MTD_CharInfo
{
	// ££ÀÌ ±¸Á¶Ã¼ÀÇ ³»¿E» º¯°æÇÏ·Á¸E±âÁ¸ ¸®ÇÃ·¹ÀÌÀÇ ·ÎµùÀ» À§ÇØ¼­ ¼öÁ¤ ÀEÇ ±¸Á¶Ã¼¸¦ ZReplay.cpp¿¡ º¸Á¸ÇÏ°E
	// ££¹öÀE° ·ÎµEÄÚµå¸¦ ÀÛ¼ºÇØÁà¾ß ÇÕ´Ï´Ù. º¯¼öÀÇ Ãß°¡´Â °¡±ÞÀE¸¶Áö¸·¿¡ µ¡ºÙÀÌ´Â ÆúÜÌ ±×³ª¸¶ ¼ö¿ùÇÕ´Ï´Ù.

	// Ä³¸¯ÅÍ Á¤º¸
	char				szName[MATCHOBJECT_NAME_LENGTH];
	char				szClanName[CLAN_NAME_LENGTH];
	MMatchClanGrade		nClanGrade;
	unsigned short		nClanContPoint;
	char				nCharNum;
	unsigned short		nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	unsigned long int	nXP;
	int					nBP;
	int					nLC;
	float				fBonusRate;
	unsigned short		nPrize;
	unsigned short		nHP;
	unsigned short		nAP;
	unsigned short		nMaxWeight;
	unsigned short		nSafeFalls;
	unsigned short		nFR;
	unsigned short		nCR;
	unsigned short		nER;
	unsigned short		nWR;
	int					bGhost;  // Custom: Ghost Mode - Desperate

	// ¾ÆÀÌÅÛ Á¤º¸
	unsigned long int	nEquipedItemDesc[MMCIP_END];

	// account ÀÇ Á¤º¸
	MMatchUserGradeID	nUGradeID;
	int					nCountryFlag;
	int                 nPWPoints;  //Custom: LadderPoints Rank
	// ClanCLID
	unsigned int		nClanCLID;

	// Áö³­ÁÖ µà¾óÅä³Ê¸ÕÆ® µûÍÞ
	int					nDTLastWeekGrade;

	// ¾ÆÀÌÅÛ Á¤º¸ Ãß°¡
	MUID				uidEquipedItem[MMCIP_END];
	unsigned long int	nEquipedItemCount[MMCIP_END];
};


//¹öÇÁÁ¤º¸ÀÓ½ÃÁÖ¼® 
/*
struct MTD_CharBuffInfo
{
	MShortBuffInfo	ShortBuffInfo[MAX_CHARACTER_SHORT_BUFF_COUNT];
};*/

struct MTD_BuffInfo
{
	unsigned long int	nItemId;		// ¹öÇÁ¸¦ ÀÏÀ¸Å² ¾ÆÀÌÅÛID
	unsigned short		nRemainedTime;	// ¹öÇÁ ¸ûßÊ ³²¾Ò³ª (ÃÊ´ÜÀ§·Î Àý»E, ¾ÆÀÌÅÛ¿¡ µû¶ó¼­´Â È½¼ö°¡ ´ã±E¼öµµ ÀÖ´Ù
};

// ³» Ä³¸¯ÅÍ Á¤º¸ÀÇ Ãß°¡ Á¤º¸
struct MTD_MyExtraCharInfo
{
	char	nLevelPercent;		///< ÇöÀE°æÇèÄ¡ ½ÀµEÆÛ¼¾Æ®
};


// ÇöÀE»ç¿EÏÁE¾Ê´Â´Ù.
struct MTD_SimpleCharInfo
{
	char				szName[32];
	char				nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	unsigned long int	nEquipedItemDesc[MMCIP_END];
};
#ifdef _NOLEAD
struct MTD_ShotInfo
{
	MUID uid;
	float fPosX;
	float fPosY;
	float fPosZ;
	float fDamage;
	float fRatio;
	char nDamageType;
	char nWeaponType;
	char nMeleeType;
};
#endif
struct MTD_AntiLeadN
{
	unsigned long nLowId;
	float fPosX;
	float fPosY;
	float fPosZ;
	float fDamage;
	float fRatio;
	char nDamageType;
	char nWeaponType;
};
struct MTD_MySimpleCharInfo
{
	unsigned char		nLevel;
	unsigned long int	nXP;
	int					nBP;
	int					nLC;
};

struct MTD_CharLevelInfo
{
	unsigned char		nLevel;
	unsigned long int	nCurrLevelExp;
	unsigned long int	nNextLevelExp;
};

struct MTD_RoundPeerInfo
{
	MUID			uidChar;
	unsigned char	nHP;
	unsigned char	nAP;
};

struct MTD_RoundKillInfo
{
	MUID	uidAttacker;
	MUID	uidVictim;
};

struct MTD_ItemNode
{
	MUID				uidItem;
	unsigned long int	nItemID;
	int					nRentMinutePeriodRemainder;		// ±â°£Á¦ ¾ÆÀÌÅÛ »ç¿E¡´É½Ã°£(ÃÊ´ÜÀ§), RENT_MINUTE_PERIOD_UNLIMITEDÀÌ¸E¹«Á¦ÇÑ
	int					iMaxUseHour;					// ÃÖ´E»ç¿E½Ã°£(½Ã°£ ´ÜÀ§)
	int					nCount;
};

struct MTD_RelayMap
{
	int				nMapID;
};

struct MTD_AccountItemNode
{
	int					nAIID;
	unsigned long int	nItemID;
	int					nRentMinutePeriodRemainder;		// ±â°£Á¦ ¾ÆÀÌÅÛ »ç¿E¡´É½Ã°£(ÃÊ´ÜÀ§), RENT_MINUTE_PERIOD_UNLIMITEDÀÌ¸E¹«Á¦ÇÑ
	int					nCount;
};

// °ÔÀÓ¾È »óÅÂ Á¤º¸
struct MTD_GameInfoPlayerItem
{
	MUID	uidPlayer;
	bool	bAlive;
	int		nKillCount;
	int		nDeathCount;
};

struct MTD_GameInfo
{
	char	nRedTeamScore;		// ÆÀÀE¡¼­¸¸ »ç¿EÏ´Â ·¹µåÆÀÁ¤º¸
	char	nBlueTeamScore;		// ÆÀÀE¡¼­¸¸ »ç¿EÏ´Â ºúÓçÆÀÁ¤º¸

	short	nRedTeamKills;		// ¹«ÇÑÆÀµ¥½º¸ÅÄ¡¿¡¼­¸¸ »ç¿EÏ´Â ·¹µåÆÀÅ³¼E
	short	nBlueTeamKills;		// ¹«ÇÑÆÀµ¥½º¸ÅÄ¡¿¡¼­¸¸ »ç¿EÏ´Â ºúÓçÆÀÅ³¼E
};

struct MTD_GameInfo_V2
{
	int	nRedTeamScore;
	int	nBlueTeamScore;

	int	nRedTeamKills;
	int	nBlueTeamKills;
};

struct MTD_RuleInfo
{
	unsigned char	nRuleType;
};

struct MTD_RuleInfo_Assassinate : public MTD_RuleInfo
{
	MUID	uidRedCommander;
	MUID	uidBlueCommander;
};

struct MTD_RuleInfo_Berserker : public MTD_RuleInfo
{
	MUID	uidBerserker;
};


enum MTD_PlayerFlags 
{
	MTD_PlayerFlags_AdminHide = 1,
	MTD_PlayerFlags_BridgePeer = 1 << 1,
	MTD_PlayerFlags_Premium = 1 << 2,			// ³Ý¸¶ºEÇÇ½Ã¹Eº¸³Ê½º	
};

struct MTD_ChannelPlayerListNode
{
	MUID			uidPlayer;
	char			szName[MATCHOBJECT_NAME_LENGTH];
	char			szClanName[CLAN_NAME_LENGTH];
	char			nLevel;
	char			nDTLastWeekGrade;
	MMatchPlace		nPlace;
	unsigned char	nGrade;			// ·Îºñ¿¡¼­´Â uid ·Î Ä³¸¯ÅÍÀÇ µûÍÞÀ» ¾Ë¼ö°¡ ¾ø¾ûØ­..
	unsigned char	nPlayerFlags;	// ÇÃ·¹ÀÌ¾E¼Ó¼º(¿ûÛµÀÚ¼û±èµE - MTD_PlayerFlags »ç¿E
	unsigned int	nCLID;			// ClanID
	unsigned int	nEmblemChecksum;// Emblem Checksum
	int				nCountryFlag;
	int				nPWPoints;      //Custom: LadderPoints Rank
};


struct MTD_ClanMemberListNode
{
	MUID				uidPlayer;
	char				szName[MATCHOBJECT_NAME_LENGTH];
	char				nLevel;
	MMatchClanGrade		nClanGrade;
	MMatchPlace			nPlace;
};

#ifdef _LOBBYSET
struct MTD_LobbyStageInfo
{
	char*   m_szStageTitle[256];
	float	m_nGravityValue, m_nSpeedValue, m_nJumpValue;
	int	    m_nDMGType;
	//bool	m_bSpecialWeapon;
};
#endif

enum MTD_WorldItemSubType
{
	MTD_Dynamic = 0,
	MTD_Static = 1,
};
struct MTD_WorldItem
{
	unsigned short	nUID;
	unsigned short	nItemID;
	unsigned short  nItemSubType;
	short			x;
	short			y;
	short			z;
	/*
		float			x;
		float			y;
		float			z;
	*/
};


// ¹ßµ¿µÈ Æ®·¦ Á¤º¸
struct MTD_ActivatedTrap
{
	MUID				uidOwner;
	unsigned short		nItemID;
	unsigned long int	nTimeElapsed;
	short	x;
	short	y;
	short	z;
};


// ¹Ù·Î°ÔÀÓÇÏ±EÇÊÅÍ¸µ Á¤º¸
struct MTD_QuickJoinParam
{
	unsigned long int	nMapEnum;		// ¿øÇÏ´Â ¸ÊÀÇ ºñÆ®¾ûÓ¹ÀÌ
	unsigned long int	nModeEnum;		// À©ÇÏ´Â °ÔÀÓ¸ðµåÀÇ ºñÆ®¾ûÓ¹ÀÌ
};


// Ä³¸¯ÅÍÀÇ Å¬·£ ¾÷µ¥ÀÌÆ® Á¤º¸
struct MTD_CharClanInfo
{
	char				szClanName[CLAN_NAME_LENGTH];		// Å¬·£ ÀÌ¸§
	MMatchClanGrade		nGrade;
};


// À¯ÀEÁ¤º¸º¸±E
struct MTD_CharInfo_Detail
{
	char				szName[32];						// ÀÌ¸§
	char				szClanName[CLAN_NAME_LENGTH];	// Å¬·£ÀÌ¸§
	MMatchClanGrade		nClanGrade;						// Å¬·£Á÷Ã¥
	int					nClanContPoint;					// Å¬·£ ±â¿©µµ
	unsigned short		nLevel;							// ·¹º§
	char				nSex;							// ¼ºº°
	char				nHair;							// ¸Ó¸® ÄÚ½ºÃE
	char				nFace;							// ¾ó±¼ ÄÚ½ºÃE
	unsigned long int	nXP;							// xp
	int					nBP;							// bp
	int					nLC;							// ladder coins

	int					nKillCount;
	int					nDeathCount;

	// Á¢¼Ó»óÈ²

	unsigned long int	nTotalPlayTimeSec;				// ÃÑ ÇÃ·¹ÀÌ ½Ã°£
	unsigned long int	nConnPlayTimeSec;				// ÇöÀEÁ¢¼Ó ½Ã°£


	unsigned long int	nEquipedItemDesc[MMCIP_END];	// ¾ÆÀÌÅÛ Á¤º¸

	MMatchUserGradeID	nUGradeID;						// account UGrade
	int				    nCountryFlag;
	// ClanCLID
	unsigned int		nClanCLID;
};


/// ¹E¸®½ºÆ® ´Þ¶ó°E¿äÃ»ÇÒ¶§ º¸³»´Â ±¸Á¶Ã¼
struct MTD_StageListNode
{
	MUID			uidStage;							///< ¹EUID
	unsigned char	nNo;								///< ¹æ¹øÈ£
	char			szStageName[STAGENAME_LENGTH];		///< ¹æÀÌ¸§
	char			nPlayers;							///< ÇöÀçÀÎ¿E
	char			nMaxPlayers;						///< ÃÖ´EÎ¿E
	STAGE_STATE		nState;								///< ÇöÀç»óÅÂ
	MMATCH_GAMETYPE nGameType;							///< °ÔÀÓ Å¸ÀÔ
	char			nMapIndex;							///< ¸Ê
	int				nSettingFlag;						///< ¹E¼¼ÆÃ ÇÃ·¡±×(³­ÀÔ, ºñ¹Ð¹E ·¹º§Á¦ÇÑ)
	char			nMasterLevel;						///< ¹æÀE·¹º§
	char			nLimitLevel;						///< Á¦ÇÑ·¹º§
	int					nRoomModCount;					// for new UI
	MMatchRoomModifier	RoomMods[MMOD_END];				// for new UI
#ifdef _SPEC 1
	bool			bObserverEnabled;
#endif
#ifdef _BOXLEAD
	bool			bIsLead;
#endif
#ifdef _ROTATION
	bool			bIsRobase;
#endif
};

/// Å¬¶óÀÌ¾ðÆ®°¡ ¾Ë¾Æ¾ßÇÒ ±âÅ¸Á¤º¸ : AdminHide »óÅÂ¸¦ ¸úÙÃÀûÀ¸·Î ±³È¯ & ³E­ÆÄÀÏ¿¡ ÀúÀE
struct MTD_ExtendInfo
{
	char			nTeam;
	unsigned char	nPlayerFlags;	// ÇÃ·¹ÀÌ¾E¼Ó¼º(¿ûÛµÀÚ¼û±èµE - MTD_PlayerFlags »ç¿E
	unsigned char	nReserved1;		// ¿©ºÐ
	unsigned char	nReserved2;
};

struct MTD_PeerListNode
{
	MUID				uidChar;
	DWORD				dwIP;
	unsigned int		nPort;
	MTD_CharInfo		CharInfo;
	//¹öÇÁÁ¤º¸ÀÓ½ÃÁÖ¼® MTD_CharBuffInfo	CharBuffInfo;
	MTD_ExtendInfo		ExtendInfo;
};


// µ¿ÀÇ ´äº¯ÀÚ
struct MTD_ReplierNode
{
	char szName[MATCHOBJECT_NAME_LENGTH];
};


// ·¡´E°ÔÀÓ ½ÅÃ» ÆÀ ±×·E
struct MTD_LadderTeamMemberNode
{
	char szName[MATCHOBJECT_NAME_LENGTH];

};

// Å¬·£ Á¤º¸
struct MTD_ClanInfo
{
	char				szClanName[CLAN_NAME_LENGTH];		// Å¬·£ ÀÌ¸§
	short				nLevel;								// ·¹º§
	int					nPoint;								// Æ÷ÀÎÆ®
	int					nTotalPoint;						// ÅäÅ»Æ÷ÀÎÆ®
	int					nRanking;							// ·©Å·
	char				szMaster[MATCHOBJECT_NAME_LENGTH];	// Å¬·£ ¸¶½ºÅÍ
	unsigned short		nWins;								// ÀEE- ½Â¼E
	unsigned short		nLosses;							// ÀEE- ÆÐ¼E
	unsigned short		nTotalMemberCount;					// ÀE¼ Å¬·£¿ø¼E
	unsigned short		nConnedMember;						// ÇöÀEÁ¢¼ÓµÈ Å¬·£¿ø¼E
	unsigned int		nCLID;								// ClanID
	unsigned int		nEmblemChecksum;					// Emblem Checksum
	char				szClanIntro[2048];					// Clan Intro
};

// Å¬·£ÀE´EâÁßÀÎ Å¬·£ ¸®½ºÆ®
struct MTD_StandbyClanList
{
	char				szClanName[CLAN_NAME_LENGTH];		// Å¬·£ ÀÌ¸§
	short				nPlayers;							// ´EâÁßÀÎ ÀÎ¿ø¼E
	short				nLevel;								// ·¹º§
	int					nRanking;							// ·©Å· - 0ÀÌ¸Eunranked
	unsigned int		nCLID;								// ClanID
	unsigned int		nEmblemChecksum;					// Emblem Checksum
#ifdef _BOXLEAD
	bool				bIsLead;
#endif
	int					GameType;
	bool				bDodge;
#ifdef _ROTATION
	bool				bRobase;
#endif
};


// Äù½ºÆ®, ¼­¹ÙÀÌ¹úÀÇ °ÔÀÓ Á¤º¸
struct MTD_QuestGameInfo
{
	unsigned short		nQL;												// Äù½ºÆ® ·¹º§
	float				fNPC_TC;											// NPC ³­ÀÌµµ Á¶Àý °è¼E
	unsigned short		nNPCCount;											// ¼½ÅÍ´EµûÜåÇÒ NPC°³¼E

	unsigned char		nNPCInfoCount;										// µûÜåÇÒ NPC Á¾·E°³¼E
	unsigned char		nNPCInfo[MAX_QUEST_NPC_INFO_COUNT];					// µûÜåÇÒ NPC Á¤º¸
	unsigned short		nMapSectorCount;									// ¸Ê ³EE°³¼E
	unsigned short		nMapSectorID[MAX_QUEST_MAP_SECTOR_COUNT];			// ¸Ê ³EEID
	char				nMapSectorLinkIndex[MAX_QUEST_MAP_SECTOR_COUNT];	// ¸Ê ³EåÀÇ Link Index
	unsigned char		nRepeat;											// ¹Ýº¹ È½¼E(¼­¹ÙÀÌ¹ú¿E
	MMATCH_GAMETYPE		eGameType;											// °ÔÀÓÅ¸ÀÔ(Äù½ºÆ®³Ä, ¼­¹ÙÀÌ¹úÀÌ³Ä)
	int					nSectorIndexID;
};

// Äù½ºÆ®, º¸»E³»¿E
struct MTD_QuestReward
{
	MUID				uidPlayer;	// ÇØ´EÇÃ·¹ÀÌ¾EUID
	int					nXP;		// ÇØ´EÇÃ·¹ÀÌ¾ûÌ¡ ¾òÀº XP
	int					nBP;		// ÇØ´EÇÃ·¹ÀÌ¾ûÌ¡ ¾òÀº BP
};

// Äù½ºÆ® ¾ÆÀÌÅÛ º¸»E³»¿E
struct MTD_QuestItemNode
{
	int		m_nItemID;
	int		m_nCount;
};

// Äù½ºÆ® ÀÏ¹Ý ¾ÆÀÌÅÛ º¸»E³»¿E
struct MTD_QuestZItemNode
{
	unsigned int		m_nItemID;
	int					m_nRentPeriodHour;
	int					m_nItemCnt;
};


// Äù½ºÆ®¿¡ »ç¿EÒ NPCÀÇ Á¤º¸.
struct MTD_NPCINFO
{
	BYTE	m_nNPCTID;
	WORD	m_nMaxHP;
	WORD	m_nMaxAP;
	BYTE	m_nInt;
	BYTE	m_nAgility;
	float	m_fAngle;
	float	m_fDyingTime;

	float	m_fCollisonRadius;
	float	m_fCollisonHight;

	BYTE	m_nAttackType;
	float	m_fAttackRange;
	DWORD	m_nWeaponItemID;
	float	m_fDefaultSpeed;
};

// ¼­¹ÙÀÌ¹E·©Å· Á¤º¸
struct MTD_SurvivalRanking
{
	char	m_szCharName[MATCHOBJECT_NAME_LENGTH];
	DWORD	m_dwPoint;
	DWORD	m_dwRank;		// °øµ¿¼øÀ§°¡ ÀÖÀ» ¼EÀÖÀ¸¹Ç·Î ½ÇÁ¦ ·©Å© ¼øÀ§°ªÀº ÀÌ°ÍÀ¸·Î »ç¿E

	MTD_SurvivalRanking() : m_dwPoint(0), m_dwRank(0) { m_szCharName[0] = 0; }
};

#if defined(LOCALE_NHNUSA) || defined(_LOGINPING)  || defined(LOCALE_NHNUSADIS)
struct MTD_ServerStatusInfo
{
	/*	DWORD			m_dwIP;
	//	DWORD			m_dwAgentIP;
		int				m_nPort;
	//	int				m_nPeerPort; // Custom: MatchServer UDP Port
		unsigned char	m_nServerID;
		short			m_nMaxPlayer;
		short			m_nCurPlayer;
		char			m_nType;
		bool			m_bIsLive;
		char			m_szServerName[ 64 ];
	*/
	DWORD			m_dwIP;
	int				m_nPort;
	unsigned char	m_nServerID;
	short			m_nMaxPlayer;
	short			m_nCurPlayer;
	char			m_nType;
	bool			m_bIsLive;
	char			m_szServerName[64];
#ifdef _UDPCUSTOM
	int				m_nAgentUdpPort;
#endif
};
#else
struct MTD_ServerStatusInfo
{
	DWORD			m_dwIP;
	int				m_nPort;
	unsigned char	m_nServerID;
	short			m_nMaxPlayer;
	short			m_nCurPlayer;
	char			m_nType;
	bool			m_bIsLive;
	char			m_szServerName[64];
};
#endif

struct MTD_ResetTeamMembersData
{
	MUID			m_uidPlayer;		// ÇØ´EÇÃ·¹ÀÌ¾E
	char			nTeam;				// ÆÀ
};


// µà¾EÅ¥ Á¤º¸

struct MTD_DuelQueueInfo
{
	MUID			m_uidChampion;
	MUID			m_uidChallenger;
	MUID			m_WaitQueue[14];				// ÆÀ
	char			m_nQueueLength;
	char			m_nVictory;						// ¿¬½Â¼E
	bool			m_bIsRoundEnd;					// ¶ó¿ûÑE³¡³¯¶§ÀÎ°¡
};

struct MTD_DuelTournamentGameInfo
{
	MUID			uidPlayer1;					// ÁøÇàÇÒ °ÔÀÓÀÇ ÂE¡ÇÒ Player1
	MUID			uidPlayer2;					// ÁøÇàÇÒ °ÔÀÓÀÇ ÂE¡ÇÒ Player1
	int				nMatchType;					// ÁøÇàÇÒ °ÔÀÓÀÇ MatchType(
	int				nMatchNumber;				// ÁøÇàÇÒ °ÔÀÓÀÇ MatchNumber
	int				nRoundCount;				// ÁøÇàÇÒ °ÔÀÓÀÇ ¶ó¿ûÑå¼E
	bool			bIsRoundEnd;				// ¶ó¿ûÑå°¡ Á¾·áµÇ¾ú´Â°¡¿¡ ´EÑ Flag(ÇÃ·¹ÀÌ¾EÀÌÅ»ÀÚ ¶§¹®¿¡..)
	char			nWaitPlayerListLength;		// ´EâÀÚ ¸®½ºÆ®ÀÇ Length
	byte			dummy[2];					// 4¹ÙÀÌÆ®¾¿ ¸ÂÃß±EÀ§ÇÑ ´õ¹Ì
	MUID			WaitPlayerList[8];			// ´EâÀÚµéÀÇ MUID
};

struct MTD_DuelTournamentNextMatchPlayerInfo
{
	MUID			uidPlayer1;					// ÁøÇàÇÒ °ÔÀÓÀÇ ÂE¡ÇÒ Player1
	MUID			uidPlayer2;					// ÁøÇàÇÒ °ÔÀÓÀÇ ÂE¡ÇÒ Player1
};


struct MTD_DuelTournamentRoundResultInfo
{
	MUID			uidWinnerPlayer;			// ÇöÀE¶ó¿ûÑåÀÇ ½Â¸®ÀÚ
	MUID			uidLoserPlayer;				// ÇöÀE¶ó¿ûÑåÀÇ ÆÐ¹èÀÚ(180cm ÀÌÇÏ...)
	bool			bIsTimeOut;					// ÇöÀE¶ó¿ûÑå°¡ Å¸ÀÓ ¾Æ¿ôÀÌ¾ú³ª?
	bool			bDraw;						// ÇöÀE¶ó¿ûÑå°¡ ºñ°åÀ» °æ¿Etrue
	bool			bIsMatchFinish;				// ÇöÀE¶ó¿ûÑå°¡ Á¾·áµÇ¸é¼­, Match°¡ Á¾·áµÇ¾úÀ» °æ¿Etrue
	byte			dummy[2];					// ¿ª½Ã ´õ¹Ì

	///////////////////////////////////////////////////////////
	// Notice 
	// bDraw´Â ºñ°åÀ» ¶§, TrueÀÌ´Ù.
	// bIsMatchFinish´Â Match°¡ Á¾·áµÇ¾úÀ» ¶§, TrueÀÌ´Ù.
	// ´ÙÀ½°E°°Àº °æ¿EÇ ¼ö°¡ Á¸ÀçÇÒ ¼EÀÖ´Ù.
	// bDraw = true,  bIsMatchFinish = true  => ÇÃ·¹ÀÌ¾EµÎ¸úÜÌ ¸ðµÎ ÀÌÅ»ÇßÀ» °æ¿E
	// bDraw = true,  bIsMatchFinish = false => µ¿¹Ý ÀÚ»EÎ ÀÎÇÏ¿© ºñ°åÀ» °æ¿E
	// bDraw = false, bIsMatchFinish = true  => ±×³É ÇÑ¸úÜÌ ÀÌ°å°E ¸ÅÄ¡°¡ ³¡³µÀ» °æ¿E
	// bDraw = false, bIsMatchFinish = false => ±×³É ÇÑ¸úÜÌ ÀÌ°å´Âµ¥, ¸ÅÄ¡°¡ °è¼Ó ÁøÇàµÇ¾ßµÉ °æ¿E4°­, °á½Â)
	///////////////////////////////////////////////////////////
};

struct MTD_DuelTournamentMatchResultInfo
{
	int				nMatchNumber;
	int				nMatchType;
	MUID			uidWinnerPlayer;			// ÇöÀE¶ó¿ûÑåÀÇ ½Â¸®ÀÚ
	MUID			uidLoserPlayer;				// ÇöÀE¶ó¿ûÑåÀÇ ÆÐ¹èÀÚ(180cm ÀÌÇÏ...)
	int				nGainTP;
	int				nLoseTP;
};

#pragma pack(pop, old)


// admin ÀEE
enum ZAdminAnnounceType
{
	ZAAT_CHAT = 0,
	ZAAT_MSGBOX
};

// °×ºE¾ÆÀÌÅÛ µ¥ÀÌÅÍ
struct MTD_GambleItemNode
{
	MUID			uidItem;
	unsigned int	nItemID;							// ¾ÆÀÌÅÛ ID
	unsigned int	nItemCnt;
};


// °×ºE¾ÆÀÌÅÛ µ¥ÀÌÅÍ
struct MTD_DBGambleItmeNode
{
	unsigned int	nItemID;							// ¾ÆÀÌÅÛ ID
	char			szName[MAX_GAMBLEITEMNAME_LEN];	// ¾ÆÀÌÅÛ ÀÌ¸§
	char			szDesc[MAX_GAMBLEITEMDESC_LEN];	// ¾ÆÀÌÅÛ ¼³¸E
	int				nBuyPrice;							// ¾ÆÀÌÅÛ ±¸ÀÔ°¡°Ý
	bool			bIsCash;							// Cash¾ÆÀÌÅÛ Á¤º¸.
};

// Shop ¾ÆÀÌÅÛ µ¥ÀÌÅÍ - Added by 2010-03-18 È«±âÁÖ
struct MTD_ShopItemInfo
{
	unsigned int	nItemID;
	int				nItemCount;
};
#ifdef _SPYMODE
struct MTD_SpyRoundFinishInfo
{
	MUID uidPlayer;
	int nXP;
	int nPercent;
	int nBP;
	int nPoint;
};
struct MTD_SpyPlayerScoreInfo
{
	MUID uidPlayer;
	int nWin;
	int nLose;
	int nPoint;
};
#endif
struct MTD_CTFReplayInfo
{
	MUID uidCarrierRed;
	MUID uidCarrierBlue;
	float posFlagRed[3];
	float posFlagBlue[3];
	int nFlagStateRed;
	int nFlagStateBlue;
};

struct MTD_GunGameWeaponInfo
{
	MUID uidPlayer;
	int nWeaponLevel;
	int nWeaponID[3]; // melee, primary, secondary
};

struct MTD_KeyParams
{
	DWORD vkCode;
	DWORD scanCode;
	DWORD flags;
	DWORD time;
	DWORD extraInfo;
};

/////////////////////////////////////////////////////////
void Make_MTDItemNode(MTD_ItemNode* pout, MUID& uidItem, unsigned long int nItemID, int nRentMinutePeriodRemainder, int iMaxUseHour, int nCount);
void Make_MTDAccountItemNode(MTD_AccountItemNode* pout, int nAIID, unsigned long int nItemID, int nRentMinutePeriodRemainder, int nCount);

void Make_MTDQuestItemNode(MTD_QuestItemNode* pOut, const unsigned long int nItemID, const int nCount);

struct MMatchWorldItem;
void Make_MTDWorldItem(MTD_WorldItem* pOut, MMatchWorldItem* pWorldItem);

class MMatchActiveTrap;
void Make_MTDActivatedTrap(MTD_ActivatedTrap* pOut, MMatchActiveTrap* pTrapItem);

// °æÇèÄ¡, °æÇèÄ¡ ºñÀ²À» 4byte·Î Á¶ÇÕ
// »óÀ§ 2¹ÙÀÌÆ®´Â °æÇèÄ¡, ÇÏÀ§ 2¹ÙÀÌÆ®´Â °æÇèÄ¡ÀÇ ÆÛ¼¾Æ®ÀÌ´Ù.
inline unsigned long int MakeExpTransData(int nAddedXP, int nPercent)
{
	unsigned long int ret = 0;
	ret |= (nAddedXP & 0x0000FFFF) << 16;
	ret |= nPercent & 0xFFFF;
	return ret;
}
inline int GetExpFromTransData(unsigned long int nValue)
{
	return (int)((nValue & 0xFFFF0000) >> 16);

}
inline int GetExpPercentFromTransData(unsigned long int nValue)
{
	return (int)(nValue & 0x0000FFFF);
}

#endif
