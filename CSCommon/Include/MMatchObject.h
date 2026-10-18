#ifndef MMATCHOBJECT_H
#define MMATCHOBJECT_H

#include <vector>
#include <map>
#include <algorithm>
#include <string>
using namespace std;
#include "MMatchItem.h"
#include "MUID.h"
#include "MObject.h"
#include "MMatchGlobal.h"
#include "MMatchFriendInfo.h"
#include "MMatchClan.h"
#include "MMatchChannel.h"
#include "MSmartRefreshImpl.h"
#include "MQuestItem.h"
#include "MMatchAntiHack.h"
#include "MMatchHShield.h"
#include "MMatchGambleItem.h"
#include "MHackingTypes.h"
#include "MMatchObjectCharBuffInfo.h"

#include "MMatchAccountPenaltyInfo.h"

#include "MMatchCharBRInfo.h"

// 주의 - 이것은 디비의 UserGrade테이블과 싱크가 맞아야 한다.
enum MMatchUserGradeID
{
	MMUG_FREE			= 0,	// 무료계정
	MMUG_REGULAR		= 1,	// 정액 유무
	MMUG_STAR			= 2,	// 스타유무(게임참여)

#ifdef _VIPGRADES
	MMUG_VIP1           = 3, //Cyan
	MMUG_VIP2           = 4, //Blue
	MMUG_VIP3           = 5, //Yellow
	MMUG_VIP4           = 6, //Rosa
	MMUG_VIP5           = 7, //Gris
	MMUG_VIP6           = 8, //Morado
	MMUG_VIP7           = 9, //Super VIP
#endif

#ifdef _EVENTGRD
	MMUG_EVENT2         = 10, //Agua
	MMUG_EVENT3         = 11, //Brown
	MMUG_EVENT4         = 12, //Black
	MMUG_EVENT1         = 13,  //Fiucha
	MMUG_BOOSTER        = 14,  //Server Boster
	MMUG_WINTOUR        = 15,  //Winner Torunament
#endif

	MMUG_CRIMINAL		= 100,	// 관리자
	MMUG_WARNING_1		= 101,	// 1차경고
	MMUG_WARNING_2		= 102,	// 2차경고
	MMUG_WARNING_3		= 103,	// 3차경고
	MMUG_CHAT_LIMITED	= 104,  // 채팅 금지
	MMUG_PENALTY		= 105,	// 기간 정지
#ifdef _NEWGRADE
	MMUG_HIDE_ADMIN           = 32,   // Staff Hide??
#endif

	MMUG_MANAGER		= 250,	// Custom: Manager Administrator
	MMUG_EVENTTEAM		= 251,	// Custom: Event Team
	MMUG_EVENTMASTER	= 256,	// Event Master
	MMUG_BLOCKED		= 253,	// Banned
	MMUG_DEVELOPER		= 254,	// Developer
	MMUG_GAMEMASTER     = 252,  // GameMaster (GM)
	MMUG_ADMIN			= 255	// Owner
};



enum MMatchDisconnectStatus
{
	MMDS_CONNECTED = 1,
	MMDS_DISCONN_WAIT,
	MMDS_DISCONNECT,

	MMDS_END,
};



// 유료이용자 권한 - 지금은 DB의 PGradeID를 읽지않고 단지 PC방별 보너스 용도로만 사용한다.
enum MMatchPremiumGradeID
{
	MMPG_FREE			= 0,	// 무료
	MMPG_PREMIUM_IP		= 1		// 넷마블 PC방 보너스
};

// 성별 - 이것은 디비의 성별정보와 싱크가 맞아야 한다.
enum MMatchSex
{
	MMS_MALE = 0,		// 남자
	MMS_FEMALE = 1,		// 여자
#ifdef _DYNAMIC
	MMS_END
#endif
};

/// °èÁ¤ Á¤º¸
struct MMatchAccountInfo
{
	int						m_nAID;
	char					m_szUserID[ MAX_USERID_STRING_LEN ];	// °èÁ¤ID
	MMatchUserGradeID		m_nUGrade;								// µûÍÞ
	MMatchPremiumGradeID	m_nPGrade;								// À¯·EÀÌ¿EÚ ±ÇÇÑ
	int						m_nCountryFlag;
	int						m_nCash;
	int						m_nEvent;

	MMatchHackingType		m_HackingType;
	SYSTEMTIME				m_EndBlockDate;							// ÇØÅ· À¯ÀúºúÓ°ÀÌ À¯ÁöµÇ´Â ½Ã°£.
	DWORD					m_dwHackingBlockEndTimeMS;

	bool					m_bIsPowerLevelingHacker;
	int						m_nPowerLevelingRegTimeMin;

	int						m_nCCode;
	char					m_szHWID[128];
	unsigned long			m_nInviteWar;

	MMatchAccountInfo() : m_nAID(-1), m_nUGrade(MMUG_FREE), m_nPGrade(MMPG_FREE), m_nCountryFlag(0), m_nCCode(0), m_nCash(0), m_nEvent(0)
	{
		m_HackingType				= MMHT_NO;
		m_dwHackingBlockEndTimeMS	= 0;
		m_bIsPowerLevelingHacker	= false;
		m_nPowerLevelingRegTimeMin	= 0;
		memset(m_szUserID, 0, MAX_USERID_STRING_LEN);
		memset(m_szHWID, 0, 128);
		m_nInviteWar = 0;
	}
};


struct MAccountItemNode
{
	int					nAIID;
	unsigned long int	nItemID;
	int					nRentMinutePeriodRemainder;
	int					nCount;
};

// ÇÃ·¹ÀÌ¾ûÌ¡ ÇöÀEÀ§Ä¡ÇÏ°EÀÖ´Â °E
enum MMatchPlace
{
	MMP_OUTSIDE	= 0,
	MMP_LOBBY = 1,
	MMP_STAGE	= 2,
	MMP_BATTLE	= 3,
	MMP_END
};

enum MMatchObjectStageState
{
	MOSS_NONREADY	= 0,
	MOSS_READY		= 1,
	MOSS_SHOP		= 2,
	MOSS_EQUIPMENT	= 3,
	MOSS_END
};

enum MMatchObjectClass
{
	MOC_NONE = 0,
	MOC_HUNTER = 1,
	MOC_SLAUGHTER = 2,
	MOC_TRICKSTER = 3,
	MOC_GLADIATOR = 4,
	MOC_DUELIST = 5,
	MOC_INCINERATOR = 6,
	MOC_COMBATOFFICER = 7,
	MOC_ASSASSIN = 8,
	MOC_TERRORIST = 9,
	MOC_END
};

#define DEFAULT_CHAR_HP				100
#define DEFAULT_CHAR_AP				0   

#define DBCACHING_REQUEST_POINT			40
#define DBCACHING_REQUEST_TIME_POINT	60 * 5	// 5ºÐ¸¶´Ù ¾÷µ¥ÀÌÆ®~!

struct DBCharCachingData
{
	int	nAddedXP;
	int	nAddedBP;
	int	nAddedLC;
	int	nAddedKillCount;
	int	nAddedDeathCount;
	int nAddedPlayTime;

	void Reset()
	{
		nAddedXP = 0;
		nAddedBP = 0;
		nAddedLC = 0;
		nAddedKillCount = 0;
		nAddedDeathCount = 0;
		nAddedPlayTime = 0;
	}
	bool IsRequestUpdate()
	{
		if ((nAddedKillCount > DBCACHING_REQUEST_POINT) || (nAddedDeathCount > DBCACHING_REQUEST_POINT)
			|| nAddedPlayTime > DBCACHING_REQUEST_TIME_POINT)
			return true;

		return false;
	}
};

struct MGamePlayInfo
{
	int nPlayTime;
	int	nKillCount;
	int nDeathCount;
	int	nXP;
	int	nBP;
	int nLC;

	void Reset()
	{
		nPlayTime = 0;
		nKillCount = 0;
		nDeathCount = 0;
		nXP = 0;
		nBP = 0;
		nLC = 0;
	}
};

// Ä³¸¯ÅÍÀÇ Å¬·£Á¤º¸
struct MMatchCharClanInfo
{
	int					m_nClanID;							// db»óÀÇ Å¬·£ ID
	char				m_szClanName[ CLAN_NAME_LENGTH ];	// Å¬·£ ÀÌ¸§
	MMatchClanGrade		m_nGrade;							// Å¬·£¿¡¼­ÀÇ ±ÇÇÑ
	int					m_nContPoint;						// Å¬·£ ±â¿©µµ
	string				m_strDeleteDate;


	MMatchCharClanInfo() {  Clear(); }
	void Clear()
	{
		m_nClanID = 0; 
		// m_szClanName[0] = 0; 
		m_nGrade=MCG_NONE;
		m_nContPoint = 0;
		m_strDeleteDate.clear();

		memset( m_szClanName, 0, CLAN_NAME_LENGTH );
	}
	bool IsJoined() { return (m_nClanID == 0) ? false : true; }
public:
	int GetClanID() { return m_nClanID; }
};


#define DEFAULT_CHARINFO_MAXWEIGHT		100
#define DEFAULT_CHARINFO_SAFEFALLS		0
#define DEFAULT_CHARINFO_BONUSRATE		0.0f
#define DEFAULT_CHARINFO_PRIZE			0




/// Ä³¸¯ÅÍ Á¤º¸
class MMatchCharInfo
{
public:
	unsigned long int	m_nCID;
	int					m_nCharNum;
	char				m_szName[MATCHOBJECT_NAME_LENGTH];
	int					m_nLevel;
	MMatchSex			m_nSex;			// ¼ºº°
	int					m_nHair;		// ¸Ó¸®
	int					m_nFace;		// ¾ó±¼
	unsigned long int	m_nXP;
	int					m_nBP;
	int					m_nLC;
	float				m_fBonusRate;
	int					m_nPrize;
	int					m_nHP;
	int					m_nAP;
	int					m_nMaxWeight;
	int					m_nSafeFalls;
	int					m_nFR;
	int					m_nCR;
	int					m_nER;
	int					m_nWR;
	unsigned long int	m_nEquipedItemCIID[MMCIP_END];
	MMatchItemMap		m_ItemList;			// ¾ÆÀÌÅÛ Á¤º¸
	MMatchEquipedItem	m_EquipedItem;		// ÀåºñÇÏ°EÀÖ´Â ¾ÆÀÌÅÛ Á¤º¸
	MMatchCharClanInfo	m_ClanInfo;			// Å¬·£ Á¤º¸

	// Äù½ºÆ® ¾ÆÀÌÅÛ.
	MQuestItemMap		m_QuestItemList;
	DBQuestCachingData	m_DBQuestCachingData;
	DBQuestCachingData& GetDBQuestCachingData() { return m_DBQuestCachingData; }


	// ¸ó½ºÅÍ ¹ÙÀÌºE
	MQuestMonsterBible	m_QMonsterBible;

	// gamble item.
	MMatchGambleItemManager m_GambleItemManager;

	unsigned long int	m_nTotalPlayTimeSec;		// ÄÉ¸¯ÅÍÀÇ ÃÑ ÇÃ·¹ÀÌ ½Ã°£
	unsigned long int	m_nPlayTimeSec;				// Á¢¼ÓÇØ¼­ ÇÃ·¹ÀÌÇÑ ½Ã°£
	unsigned long int	m_nConnTime;				// Á¢¼ÓÇÑ ½Ã°£(1ÃÊ = 1000)
	unsigned long int	m_nBattleStartTime;			// ¹èÆ² ½ÃÀÛ½Ã°£

	unsigned long int	m_nTotalKillCount;			// ÀE¼ Å³¼E
	unsigned long int	m_nTotalDeathCount;			// ÀE¼ µ¥¾²¼E
	unsigned long int	m_nConnKillCount;			// Á¢¼ÓÀÌÈÄ·Î ´©ÀûµÈ Å³¼E
	unsigned long int	m_nConnDeathCount;			// Á¢¼ÓÀÌÈÄ·Î ´©ÀûµÈ µ¥¾²¼E
	unsigned long int   m_nConnXP;					// Á¢¼ÓÀÌÈÄ·Î ´©ÀûµÈ °æÇèÄ¡

	unsigned long int   m_nBattleStartXP;			// ¹èÆ² ½ÃÀÛ °æÇèÄ¡

	bool				m_IsSendMyItemListByRequestClient;

	unsigned int		m_nRank;

	bool				m_bIsLadderMatching;		// Custom: Clan war glitch fix

protected:
	DBCharCachingData	m_DBCachingData;
public:
	MMatchCharInfo() : m_nCID(0), m_nCharNum(0), m_nLevel(0), m_nSex(MMS_MALE), m_nFace(0),
		               m_nHair(0), m_nXP(0), m_nBP(0), m_nLC(0), m_fBonusRate(DEFAULT_CHARINFO_BONUSRATE), m_nPrize(DEFAULT_CHARINFO_PRIZE), m_nHP(0),
					   m_nAP(0), m_nMaxWeight(DEFAULT_CHARINFO_MAXWEIGHT), m_nSafeFalls(DEFAULT_CHARINFO_SAFEFALLS),
					   m_nFR(0), m_nCR(0), m_nER(0), m_nWR(0),
					   m_nConnTime(0), m_nTotalKillCount(0), m_nTotalDeathCount(0), m_nConnKillCount(0), m_nConnDeathCount(0), 
					   m_nConnXP(0), m_nRank(0), m_nBattleStartTime(0), m_nPlayTimeSec(0), m_nTotalPlayTimeSec(0), m_nBattleStartXP(0)
	{
		memset(m_szName, 0, MATCHOBJECT_NAME_LENGTH);
		memset(m_nEquipedItemCIID, 0, sizeof(m_nEquipedItemCIID));
		memset(&m_DBCachingData, 0, sizeof(m_DBCachingData));
		memset(&m_QMonsterBible, 0, sizeof(MQuestMonsterBible) );

		m_QuestItemList.Clear();
		m_DBQuestCachingData.Reset();
		m_CharGamePlayInfo.Reset();

		m_IsSendMyItemListByRequestClient = false;
		m_bIsLadderMatching = false;

		m_EquipedItem.SetOwner( this );
	}
	bool EquipFromItemList();
	void ClearItems();
	void Clear();
	void GetTotalWeight(int* poutWeight, int* poutMaxWeight);


	// db caching±ûÝEÇÔ²² ´õÇØÁØ´Ù.
	void IncKill()
	{ 
		m_nTotalKillCount += 1;
		m_nConnKillCount += 1;
		m_DBCachingData.nAddedKillCount += 1;
		m_CharGamePlayInfo.nKillCount +=1;
	}
	void IncDeath()
	{ 
		m_nTotalDeathCount += 1;
		m_nConnDeathCount += 1;
		m_DBCachingData.nAddedDeathCount += 1;
		m_CharGamePlayInfo.nDeathCount += 1;
	}
	void IncBP(int nAddedBP)		
	{ 
		m_nBP += nAddedBP;
		m_DBCachingData.nAddedBP += nAddedBP;
		m_CharGamePlayInfo.nBP += nAddedBP;
	}
	void DecBP(int nDecBP)
	{ 
		m_nBP -= nDecBP;
		m_DBCachingData.nAddedBP -= nDecBP;
		m_CharGamePlayInfo.nBP -= nDecBP;
	}
	void IncLC(int nAddedLC)
	{
		m_nLC += nAddedLC;
		m_DBCachingData.nAddedLC += nAddedLC;
		m_CharGamePlayInfo.nLC += nAddedLC;
	}
	void DecLC(int nDecLC)
	{
		m_nLC -= nDecLC;
		m_DBCachingData.nAddedLC -= nDecLC;
		m_CharGamePlayInfo.nLC -= nDecLC;
	}
	void IncXP(int nAddedXP)
	{ 
		m_nConnXP += nAddedXP;
		m_nXP += nAddedXP;
		m_DBCachingData.nAddedXP += nAddedXP;
		m_CharGamePlayInfo.nXP += nAddedXP;
	}
	void DecXP(int nDecXP)
	{ 
		m_nConnXP -= nDecXP; 
		m_nXP -= nDecXP; 
		m_DBCachingData.nAddedXP -= nDecXP; 
		m_CharGamePlayInfo.nXP -= nDecXP;
	}

	void UpdatePlayTime(int nPlayTime)
	{
		int nAddedPlayTime = nPlayTime - m_nPlayTimeSec;

		m_nPlayTimeSec = nPlayTime;
		m_nTotalPlayTimeSec += nAddedPlayTime;
		m_DBCachingData.nAddedPlayTime += nAddedPlayTime;
	}

	DBCharCachingData* GetDBCachingData() { return &m_DBCachingData; }

protected:
	MMatchCharBattleTimeRewardInfoMap m_CharBRInfoMap;

	MGamePlayInfo m_CharGamePlayInfo;
public:
	MMatchCharBattleTimeRewardInfoMap& GetBRInfoMap() { return m_CharBRInfoMap; }
	MGamePlayInfo* GetCharGamePlayInfo() { return &m_CharGamePlayInfo; }

};

class MMatchTimeSyncInfo {
protected:
	int				m_nFoulCount;
	unsigned long	m_nLastSyncClock;
public:
	MMatchTimeSyncInfo()				{ m_nFoulCount=0; m_nLastSyncClock=0; }
	virtual ~MMatchTimeSyncInfo()		{}

	int GetFoulCount()					{ return m_nFoulCount; }
	void AddFoulCount()					{ m_nFoulCount++; }
	void ResetFoulCount()				{ m_nFoulCount = 0; }
	unsigned long GetLastSyncClock()	{ return m_nLastSyncClock; }
	void Update(unsigned long nClock)	{ m_nLastSyncClock = nClock; }
};

// MatchObject°¡ °ÔÀÓ¾È¿¡¼­ »ç¿EÏ´Â º¯¼öµE
struct MMatchObjectGameInfo
{
	bool		bJoinedGame;		// °ÔÀÓ¿¡ ÂE¡ÁßÀÎÁE¿©ºÎ - ÆÀÀE¡¼­ ³­ÀÔÇßÀ»¶§
};


class MMatchDisconnStatusInfo
{
public :
	MMatchDisconnStatusInfo() 
	{
		m_DisconnStatus						= MMDS_CONNECTED;
		m_dwLastDisconnStatusUpdatedTime	= timeGetTime();
		m_bIsSendDisconnMsg					= false;
		m_dwDisconnSetTime					= 0;
		m_bIsUpdateDB						= false;
	}

	~MMatchDisconnStatusInfo() {}

	const MMatchDisconnectStatus	GetStatus()				{ return m_DisconnStatus; }
	const DWORD						GetLastUpdatedTime()	{ return m_dwLastDisconnStatusUpdatedTime; }
	const DWORD						GetMsgID()				{ return m_dwMsgID; }
	const MMatchHackingType			GetHackingType()		{ return m_HackingType; } 
	const MMatchBlockLevel			GetBlockLevel()			{ return m_BlockLevel; }
	const string&					GetEndDate()			{ return m_strEndDate; }
	const string&					GetComment()			{ return m_strComment; }
		
	const bool	IsSendDisconnMsg()	{ return m_bIsSendDisconnMsg; }
	void		SendCompleted()		{ m_bIsSendDisconnMsg = false; }	// MMatchServer¿¡¼­ Ä¿¸ÇµEÃ³¸®¸¦ À§ÇØ¼­ »ç¿E..
																		// IsSendDisconnMsg·Î Á¢¼Ó Á¾·E¸Þ½ÃÁö¸¦ º¸³»¾ß ÇÏ´ÂÁE°Ë»çÈÄ,
																		// Ä¿¸Çµå¸¦ º¸³»ÈÄ¿¡´Â SendCompleted()¸¦ È£ÃâÇÏ¿© ´ÙÀ½¿¡ Áßº¹À¸·Î º¸³»´Â °ÍÀ» ¸·´Â´Ù.
																		// ´EÁÁÀº ¹æ¹ýÀ» »ý°¢ÇØ ºÁ¾ß ÇÔ. -by SungE. 2006-03-07.

	const bool IsUpdateDB()			{ return m_bIsUpdateDB; }
	void UpdateDataBaseCompleted()	{ m_bIsUpdateDB = false; } // UpdateµÇ¸éÀº ´ÙÀ½ BlockTypeÀÌ ¼³Á¤ÀEûÝEfalse·Î ¼³Á¤.

	const bool IsDisconnectable( const DWORD dwTime = timeGetTime() )
	{
		if( (MMDS_DISCONNECT == GetStatus()) && (MINTERVAL_DISCONNECT_STATUS_MIN < (dwTime - m_dwDisconnSetTime)) )
			return true;
		return false;
	}
	
	void SetStatus( const MMatchDisconnectStatus Status )	
	{
		m_DisconnStatus = Status; 
		if( MMDS_DISCONN_WAIT == Status )
			SendDisconnMsgPrepared();

		if( MMDS_DISCONNECT == Status )
			m_dwDisconnSetTime = (timeGetTime() - 2000);
		
	}
	void SetUpdateTime( const DWORD dwTime )					{ m_dwLastDisconnStatusUpdatedTime = dwTime; }
	void SetMsgID( const DWORD dwMsgID )						{ m_dwMsgID = dwMsgID; }
	void SetHackingType( const MMatchHackingType HackingType )	{ m_HackingType = HackingType; m_bIsUpdateDB = true; } // »õ·Î¿EBlockTypeÀÌ ¼³Á¤µÇ¸E
																													   //  DB¾÷µ¥ÀÌÆ® ÁØºñµµ °°ÀÌÇÔ.
	void SetBlockLevel( const MMatchBlockLevel BlockLevel )		{ m_BlockLevel = BlockLevel; }
	void SetEndDate( const string& strEndDate )					{ m_strEndDate = strEndDate; }
	void SetComment( const string& strComment )					{ m_strComment = strComment; }
	
	void Update( const DWORD dwTime )
	{
		if( (dwTime - GetLastUpdatedTime()) > MINTERVAL_DISCONNECT_STATUS_MIN ) 
		{
			if( MMDS_DISCONN_WAIT == GetStatus() ) {
				SetStatus( MMDS_DISCONNECT );		
			}
			
			SetUpdateTime( dwTime );
		}
	}

private :
	void SendDisconnMsgPrepared()	{ m_bIsSendDisconnMsg = true; }

private :
	MMatchDisconnectStatus	m_DisconnStatus;
	DWORD					m_dwLastDisconnStatusUpdatedTime;
	DWORD					m_dwDisconnSetTime;
	DWORD					m_dwMsgID;
	MMatchHackingType		m_HackingType;
	MMatchBlockLevel		m_BlockLevel;	// Level¿¡µû¶E°èÁ¤ ºúÓ°, ·Î±×ÀÛ¾÷¸¸ ÇÏ´ÂµûÜ¸·Î ³ª´²ÁE
	string					m_strEndDate;
	string					m_strComment;
	bool					m_bIsSendDisconnMsg;
	bool					m_bIsUpdateDB;

	const static DWORD MINTERVAL_DISCONNECT_STATUS_MIN;
};


struct MMatchObjectChannelInfo
{
	MUID			uidChannel;
	MUID			uidRecentChannel;
	bool			bChannelListTransfer;
	MCHANNEL_TYPE	nChannelListType;
	unsigned long	nChannelListChecksum;
	int				nTimeLastChannelListTrans;
	void Clear()
	{
		uidChannel = MUID(0,0);
		uidRecentChannel = MUID(0,0);
		bChannelListTransfer = false;
		nChannelListType = MCHANNEL_TYPE_PRESET;
		nChannelListChecksum = 0;
		nTimeLastChannelListTrans = 0;
	}
};

class MAsyncJob;
class MBMatchGameguard;


struct MASYNCJOBQ
{
	list<MAsyncJob*>	DBJobQ;
	bool				bIsRunningAsyncJob;
	int					nLastJobID;
};


#define MAX_CHAT_FLOOD_COUNT	5
#define MAX_BAN_CHAT_TICK		30 * 1000

class MMatchChatFloodInfo 
{
protected:
	DWORD			m_dwBanTick;
	unsigned short	m_dwBanCount;

	list<int>	m_ChatFloodList;
public:
	MMatchChatFloodInfo() : m_dwBanTick(0), m_dwBanCount(0)
	{}

	~MMatchChatFloodInfo()
	{
		m_dwBanTick = 0;
		m_dwBanCount = 0;
		
		m_ChatFloodList.clear();
	}

	bool PushChatTickCount()
	{
		DWORD dwFirstTick = 0;
		DWORD dwCurTickCount = GetTickCount();

		if( m_ChatFloodList.size() >= MAX_CHAT_FLOOD_COUNT )
		{
			dwFirstTick = m_ChatFloodList.front();
			m_ChatFloodList.pop_front();			
		}

		m_ChatFloodList.push_back(dwCurTickCount);

		// ¸Ç Ã³À½ÀÇ Tick°E»ðÀÔÇÏ·Á´Â Tick°úÀÇ Â÷ÀÌ°¡ 1ÃÊ ÀÌÇÏ¸E. FloodingÀÌ´Ù!
		return (dwCurTickCount - dwFirstTick < 1000)? true : false;
	}

	bool IsBanUser()
	{
		DWORD dwCurTickCount = GetTickCount();

		if( dwCurTickCount - m_dwBanTick < MAX_BAN_CHAT_TICK) return true;

		m_dwBanTick = 0;
		return false;
	}

	void SetBanUser()				{ m_dwBanTick = GetTickCount(); m_dwBanCount++;}
};

class MMatchObjectDuelTournamentCharInfo
{
protected:
	bool m_bIsChallenge;
	bool m_bIsSettingData;

	int m_nTP;
	int m_nWins;
	int m_nLoses;
	int m_nRanking;
	int m_nRankingIncrease;
	int m_nQuaterFinalCount;
	int m_nQuaterFinalWins; 
	int m_nSemiFinalCount;
	int m_nSemiFinalWins;
	int m_nFinalCount;
	int m_nFinalWins;
	int	m_nLeaveCount;
	int	m_nLastWeekGrade;					// µà¾óÅä³Ê¸ÕÆ® Áö³­ÁÖ µûÍÞ ([1~10] 1ÀÌ ¿EöÇÑ µûÍÞ, 0Àº ¹«È¿°ª)

	char m_szTimeStamp[DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH + 1];

	list<DTRankingInfo*> m_SideRankingList;
public:
	MMatchObjectDuelTournamentCharInfo()
	{
		m_nTP = 0;
		m_nWins = 0;
		m_nLoses = 0;
		m_nRanking = 0;
		m_nRankingIncrease = 0;
		m_nQuaterFinalCount = 0;
		m_nQuaterFinalWins = 0;
		m_nSemiFinalCount = 0;
		m_nSemiFinalWins = 0;
		m_nFinalCount = 0;
		m_nFinalWins = 0;

		m_nLeaveCount = 0;
		m_nLastWeekGrade = 0;

		m_bIsChallenge = false;
		m_bIsSettingData = false;

		memset(m_szTimeStamp, 0, DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH + 1);
	}
	MMatchObjectDuelTournamentCharInfo(MMatchObjectDuelTournamentCharInfo *pDTCharInfo)
	{
		SetCharInfo(pDTCharInfo->GetTP()
			, pDTCharInfo->GetWins()
			, pDTCharInfo->GetLoses()
			, pDTCharInfo->GetRanking()
			, pDTCharInfo->GetRankingIncrease()
			, pDTCharInfo->GetFinalCount()
			, pDTCharInfo->GetFinalWins()
			, pDTCharInfo->GetSemiFinalCount()
			, pDTCharInfo->GetSemiFinalWins()
			, pDTCharInfo->GetQuaterFinalCount()
			, pDTCharInfo->GetQuaterFinalWins()
			, pDTCharInfo->GetLeaveCount()
			, pDTCharInfo->GetLastWeekGrade()
			, pDTCharInfo->GetTimeStamp());
	}

	~MMatchObjectDuelTournamentCharInfo()
	{
		RemoveSideRankingAll();
	}

	void SetCharInfo(int nTP, int nWins, int nLoses, int nRanking, int nRankingIncrease, 
		int nFinalCount, int nFinalWins, int nSemiFinalCount, int nSemiFinalWins, int nQuaterFinalCount, int nQuaterFinalWins, int nLeaveCount, int nLastWeekGrade, char* szTimeStamp)
	{
		m_nTP = nTP;
		m_nWins = nWins;
		m_nLoses = nLoses;
		m_nRanking = nRanking;
		m_nRankingIncrease = nRankingIncrease;
		m_nQuaterFinalCount = nQuaterFinalCount;
		m_nQuaterFinalWins = nQuaterFinalWins;
		m_nSemiFinalCount = nSemiFinalCount;
		m_nSemiFinalWins = nSemiFinalWins;
		m_nFinalCount = nFinalCount;
		m_nFinalWins = nFinalWins;

		m_nLeaveCount = nLeaveCount;
		m_nLastWeekGrade = nLastWeekGrade;

		m_bIsChallenge = false;
		m_bIsSettingData = true;

		memcpy(m_szTimeStamp, szTimeStamp, DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH + 1);
	}

	list<DTRankingInfo*>* GetSideRankingList()	{ return &m_SideRankingList;}

	void AddSideRankingInfo(DTRankingInfo* pRankingInfo)
	{
		DTRankingInfo *pNewInfo = new DTRankingInfo;
		memcpy(pNewInfo, pRankingInfo, sizeof(DTRankingInfo));
		m_SideRankingList.push_back(pNewInfo);
	}

	void RemoveSideRankingAll()
	{	
		list<DTRankingInfo*>::iterator iter = m_SideRankingList.begin();
		for(; iter != m_SideRankingList.end();){
			DTRankingInfo* pInfo = (*iter);
			iter = m_SideRankingList.erase(iter);
			delete pInfo;
		}
	}

	int GetTP()					{ return m_nTP; }
	int GetWins()				{ return m_nWins; }
	int GetLoses()				{ return m_nLoses; }
	int GetRanking()			{ return m_nRanking; }
	int GetRankingIncrease()	{ return m_nRankingIncrease; }
	int GetQuaterFinalCount()	{ return m_nQuaterFinalCount; }
	int GetQuaterFinalWins()	{ return m_nQuaterFinalWins; }
	int GetSemiFinalCount()		{ return m_nSemiFinalCount; }
	int GetSemiFinalWins()		{ return m_nSemiFinalWins; }
	int GetFinalCount()			{ return m_nFinalCount; }
	int GetFinalWins()			{ return m_nFinalWins; }
	int GetLeaveCount()			{ return m_nLeaveCount; }
	int GetLastWeekGrade()		{ return m_nLastWeekGrade; }
	char *GetTimeStamp()		{ return m_szTimeStamp; }
	
	void SetTP(int nTP)					{ m_nTP = nTP; }
	void SetLastWeekGrade(int grade)	{ m_nLastWeekGrade = grade; }

	void IncreaseWins()					{ ++m_nWins; }
	void IncreaseLoses()				{ ++m_nLoses; }
	void IncreaseQuaterFinalCount()		{ ++m_nQuaterFinalCount; }
	void IncreaseQuaterFinalWins()		{ ++m_nQuaterFinalWins; }
	void IncreaseSemiFinalCount()		{ ++m_nSemiFinalCount; }
	void IncreaseSemiFinalWins()		{ ++m_nSemiFinalWins; }
	void IncreaseFinalCount()			{ ++m_nFinalCount; }
	void IncreaseFinalWins()			{ ++m_nFinalWins; }
	void IncreaseLeaveCount()			{ ++m_nLeaveCount; }

	bool IsChallengeDuelTournament()	{ return m_bIsChallenge; }
	void SetChallengeDuelTournament(bool bValue) { m_bIsChallenge = bValue; }

	bool IsJoinDuelTournament()	{ return m_bIsChallenge; }
	void SetJoinDuelTournament(bool bValue) { m_bIsChallenge = bValue; }

	bool IsSettingData() { return m_bIsSettingData; }
};

class MMatchObject : public MObject {
protected:
	MMatchAccountInfo			m_AccountInfo;		// °èÁ¤ Á¤º¸
	MMatchCharInfo*				m_pCharInfo;		// Ä³¸¯ÅÍ Á¤º¸
	MMatchFriendInfo*			m_pFriendInfo;		// Ä£±¸Á¤º¸
	MMatchPlace					m_nPlace;			// À§Ä¡ Á¤º¸
	MMatchTimeSyncInfo			m_nTimeSyncInfo;	// ½ºÇÇµåÇÙ °¨½Ã	
	MMatchObjectChannelInfo		m_ChannelInfo;		// Ã¤³Î Á¤º¸
	MMatchObjectGameInfo		m_GameInfo;			// °ÔÀÓ¾È¿¡¼­ÀÇ Á¤º¸
	MMatchDisconnStatusInfo		m_DisconnStatusInfo;// ¿ÀºE§À¸ Á¢¼ÓÁ¾·E»óÅÂ Á¤º¸.
	MMatchObjectHShieldInfo		m_HSieldInfo;

	bool			m_bHacker;						// ÇÙÀÌ °ËÃâµÈ À¯ÀE
	bool			m_bBridgePeer;
	bool			m_bRelayPeer;
	MUID			m_uidAgent;

	DWORD			m_dwIP;
	char 			m_szIP[64];
	unsigned int	m_nPort;
	bool			m_bFreeLoginIP;					// ÀÎ¿øÁ¦ÇÑ »ó°EøÀÌ ·Î±×ÀÎ Çã¿EÈ Å¬¶óÀÌ¾ðÆ®

	unsigned char	m_nPlayerFlags;					// ÈÖ¹ß¼º ¼Ó¼ºµE(MTD_PlayerFlags)
	unsigned long	m_nUserOptionFlags;				// ±Ó¸»,ÃÊ´E°ÅºÎµûÜÇ ¿É¼Ç

	MUID			m_uidStage;
	MUID			m_uidChatRoom;

	bool			m_bStageListTransfer;
	unsigned long	m_nStageListChecksum;
	unsigned long	m_nStageListLastChecksum;
	int				m_nTimeLastStageListTrans;
	int				m_nStageCursor;

	MRefreshClientChannelImpl		m_RefreshClientChannelImpl;
	MRefreshClientClanMemberImpl	m_RefreshClientClanMemberImpl;

	MMatchObjectStageState	m_nStageState;	// ´Eâ¹æ¿¡¼­ÀÇ »óÅÂÁ¤º¸
	MMatchTeam		m_nTeam;
	int				m_nLadderGroupID;
	bool			m_bLadderChallenging;	// Å¬·£ÀE»ó´EÀ ´EâÁßÀÎÁE¿©ºÎ

//	bool			m_bStageMaster;	
	bool			m_bEnterBattle;
	bool			m_bAlive;
	unsigned int	m_nDeadTime;

	bool			m_bNewbie;				// ÀÚ½ÅÀÇ Ä³¸¯ÅÍµéÀÇ ÃÖ°úÓ¹º§ÀÌ 10·¹º§ÀÌ»óÀÌ¸EÀÔ¹®Ã¤³Î¿¡ µé¾ûÌ¥ ¼E¾ø´Ù.
	bool			m_bForcedEntried;		// ³­ÀÔÇÑ ÇÃ·¹ÀÌ¾ûÜÎÁE¿©ºÎ.
	bool			m_bLaunchedGame;

	unsigned int			m_nKillCount;	// ÇÑ¶ó¿ûÑEÁ×ÀÎ¼E
	unsigned int			m_nDeathCount;	// ÇÑ¶ó¿ûÑEÁ×Àº¼E
	unsigned long int		m_nAllRoundKillCount;	// ÀE¼ ¶ó¿ûÑåÀÇ Å³¼E
	unsigned long int		m_nAllRoundDeathCount;	// ÀE¼ ¶ó¿ûÑåÀÇ µ¥¾²¼E
	unsigned long int		m_nAllRoundScoreCount;

	// Custom: GunGame WeaponData
	int				m_nGunGameWeaponLevel;

	// Custom: Last announce time
	DWORD			m_dwLastAnnounce;

	// Custom: Forced NAT
	bool			m_bForcedNAT;

	bool			m_bWasCallVote;		// ÇÑ °ÔÀÓ ¾È¿¡¼­ ÅõÇ¥°ÇÀÇ¸¦ Çß´Â°¡?

	bool			m_bDBFriendListRequested;		// Ä£±¸¸®½ºÆ® DB¿¡ Á¤º¸¿äÃ»À» Çß´ÂÁE¿©ºÎ
	unsigned long int	m_nTickLastPacketRecved;	// ÃÖ±Ù ÆÐÅ¶ ¹ÞÀº½Ã°£
	unsigned long int	m_nLastHShieldMsgRecved;
	bool			m_bHShieldMsgRecved;
	string				m_strCountryCode3; // filter¿¡¼­ ¼³Á¤µÊ.

	//DWORD	m_dwLastHackCheckedTime;
	//DWORD	m_dwLastRecvNewHashValueTime;
	//bool	m_bIsRequestNewHashValue;

	DWORD	m_dwLastSpawnTime;				// ¸¶Áö¸·À¸·Î ½ºÆEµÇ¾ú´E½Ã°£

	unsigned long int m_nLastPingTime;		// Time Ping
	unsigned long int m_nQuestLatency;		// Äù½ºÆ®¸ðµE

	bool				m_bIsLoginCompleted;

	unsigned long		m_dwLastSendSuicideCommand;	// ºñÁ¤»EÀ¯Àú´Â MC_MATCH_REQUEST_SUICIDE¸¦ ¹«ÇÑÀ¸·Î º¸³¾¼EÀÖ´Ù.	
													// ÀÌ°ÍÀ» ¸·±EÀ§ÇØ¼­, ÀÏÁ¤ ½Ã°£ÀÌ Áö³ª¾ß ÀÀ´äÀ» ÇÏ´Â ¹æ¹ýÀ¸·Î ¼öÁ¤ÇÏ¿´´Ù.

	DWORD				m_dwLastSendGambleItemTime;

	bool				m_IsSendFirstGameguardRequest;
	bool				m_IsRecvFirstGameguardResponse;

public :
	int LastVoteID;
	bool bMatching;
	unsigned long int LadderWarsIdentifier;
	unsigned long		m_nInviteWar;
	MASYNCJOBQ			m_DBJobQ;
#ifdef _SPMOTIONLAST
	unsigned long int st_nLastDance;
#endif


protected:
	void UpdateChannelListChecksum(unsigned long nChecksum)	{ m_ChannelInfo.nChannelListChecksum = nChecksum; }
	unsigned long GetChannelListChecksum()					{ return m_ChannelInfo.nChannelListChecksum; }

	void UpdateStageListChecksum(unsigned long nChecksum)	{ m_nStageListChecksum = nChecksum; }
	unsigned long GetStageListChecksum()					{ return m_nStageListChecksum; }
	MMatchObject() : MObject() 
	{
	}
	void DeathCount()				{ m_nDeathCount++; m_nAllRoundDeathCount++; }
	void KillCount()				{ m_nKillCount++; m_nAllRoundKillCount++; }

//	void CheckClientHashValue( const DWORD dwTime );
public:
	MMatchObject(const MUID& uid);
	virtual ~MMatchObject();

	char* GetName() { 
		if (m_pCharInfo)
			return m_pCharInfo->m_szName; 
		else
			return "Unknown";
	}
	char* GetAccountName()			{ return m_AccountInfo.m_szUserID; }

#ifdef _PORTALGUN 1
	int				m_UIDItemRed;
	int				m_UIDItemBlue;
	int				m_UIDItemDated;
	bool			m_bMoveWorldItem;
#endif

	bool IsLoginCompleted() { return m_bIsLoginCompleted; }
	void LoginCompleted() { m_bIsLoginCompleted = true; }
	void LoginNotCompleted() { m_bIsLoginCompleted = false; }

	bool GetBridgePeer()			{ return m_bBridgePeer; }
	void SetBridgePeer(bool bValue)	{ m_bBridgePeer = bValue; }
	bool GetRelayPeer()				{ return m_bRelayPeer; }
	void SetRelayPeer(bool bRelay)	{ m_bRelayPeer = bRelay; }
	const MUID& GetAgentUID()		{ return m_uidAgent; }
	void SetAgentUID(const MUID& uidAgent)	{ m_uidAgent = uidAgent; }

	void SetPeerAddr(DWORD dwIP, char* szIP, unsigned short nPort)	{ m_dwIP=dwIP; strcpy(m_szIP, szIP); m_nPort = nPort; }
	DWORD GetIP()					{ return m_dwIP; }
	char* GetIPString()				{ return m_szIP; }
	unsigned short GetPort()		{ return m_nPort; }
	bool GetFreeLoginIP()			{ return m_bFreeLoginIP; }
	void SetFreeLoginIP(bool bFree)	{ m_bFreeLoginIP = bFree; }

	void ResetPlayerFlags()						{ m_nPlayerFlags = 0; }
	unsigned char GetPlayerFlags()				{ return m_nPlayerFlags; }
	bool CheckPlayerFlags(unsigned char nFlag)	{ return (m_nPlayerFlags&nFlag?true:false); }
	void SetPlayerFlag(unsigned char nFlagIdx, bool bSet)	
	{ 
		if (bSet) m_nPlayerFlags |= nFlagIdx; 
		else m_nPlayerFlags &= (0xff ^ nFlagIdx);
	}

	void SetUserOption(unsigned long nFlags)	{ m_nUserOptionFlags = nFlags; }
	bool CheckUserOption(unsigned long nFlag)	{ return (m_nUserOptionFlags&nFlag?true:false); }

	MUID GetChannelUID()						{ return m_ChannelInfo.uidChannel; }
	void SetChannelUID(const MUID& uid)			{ SetRecentChannelUID(m_ChannelInfo.uidChannel); m_ChannelInfo.uidChannel = uid; }
	MUID GetRecentChannelUID()					{ return m_ChannelInfo.uidRecentChannel; }
	void SetRecentChannelUID(const MUID& uid)	{ m_ChannelInfo.uidRecentChannel = uid; }

	MUID GetStageUID()					{ return m_uidStage; }
	void SetStageUID(const MUID& uid)	{ m_uidStage = uid; }
	MUID GetChatRoomUID()				{ return m_uidChatRoom; }
	void SetChatRoomUID(const MUID& uid){ m_uidChatRoom = uid; }

	bool CheckChannelListTransfer()	{ return m_ChannelInfo.bChannelListTransfer; }
	void SetChannelListTransfer(const bool bVal, const MCHANNEL_TYPE nChannelType=MCHANNEL_TYPE_PRESET);

	bool CheckStageListTransfer()	{ return m_bStageListTransfer; }
	void SetStageListTransfer(bool bVal)	{ m_bStageListTransfer = bVal; UpdateStageListChecksum(0); }

	MRefreshClientChannelImpl* GetRefreshClientChannelImplement()		{ return &m_RefreshClientChannelImpl; }
	MRefreshClientClanMemberImpl* GetRefreshClientClanMemberImplement()	{ return &m_RefreshClientClanMemberImpl; }

	MMatchTeam GetTeam()			{ return m_nTeam; }
	void SetTeam(MMatchTeam nTeam);
	MMatchObjectStageState GetStageState()	{ return m_nStageState; }
	void SetStageState(MMatchObjectStageState nStageState)	{ m_nStageState = nStageState; }
	bool GetEnterBattle()			{ return m_bEnterBattle; }
	void SetEnterBattle(bool bEnter){ m_bEnterBattle = bEnter; }
	bool CheckAlive()				{ return m_bAlive; }
	void SetAlive(bool bVal)		{ m_bAlive = bVal; }
	void SetKillCount(unsigned int nKillCount) { m_nKillCount = nKillCount; }
	unsigned int GetKillCount()		{ return m_nKillCount; }
	void SetDeathCount(unsigned int nDeathCount) { m_nDeathCount = nDeathCount; }
	unsigned int GetDeathCount()	{ return m_nDeathCount; }

	int GetGunGameWeaponLevel() { return m_nGunGameWeaponLevel; }
	void SetGunGameWeaponLevel(int nGunGameWeaponLevel) { m_nGunGameWeaponLevel = nGunGameWeaponLevel; }

	DWORD GetLastAnnounceTime() { return m_dwLastAnnounce; }
	void SetLastAnnounceTime(DWORD dwTime) { m_dwLastAnnounce = dwTime; }

	bool GetForcedNATOption() { return m_bForcedNAT; }
	void SetForcedNATOption(bool bEnable) { m_bForcedNAT = bEnable; }

	unsigned int GetAllRoundKillCount()	{ return m_nAllRoundKillCount; }
	unsigned int GetAllRoundDeathCount()	{ return m_nAllRoundDeathCount; }
	unsigned int GetAllRoundScoreCount() { return m_nAllRoundScoreCount; }
	void SetAllRoundKillCount(unsigned int nAllRoundKillCount) { m_nAllRoundKillCount = nAllRoundKillCount; }
	void SetAllRoundDeathCount(unsigned int nAllRoundDeathCount) { m_nAllRoundDeathCount = nAllRoundDeathCount; }
	void SetAllRoundScoreCount(unsigned int nAllRoundScoreCount) { m_nAllRoundScoreCount = nAllRoundScoreCount; }
	void FreeCharInfo();
	void FreeFriendInfo();
	void OnDead();
	void OnKill();
	bool IsEnabledRespawnDeathTime(unsigned int nNowTime);

	void SetForcedEntry(bool bVal) { m_bForcedEntried = bVal; }
	bool IsForcedEntried() { return m_bForcedEntried; }
	void SetLaunchedGame(bool bVal) { m_bLaunchedGame = bVal; }
	bool IsLaunchedGame() { return m_bLaunchedGame; }
	void CheckNewbie(int nCharMaxLevel);
	bool IsNewbie()					{ return m_bNewbie; }
	bool IsHacker()					{ return m_bHacker; }
	void SetHacker(bool bHacker)	{ m_bHacker = bHacker; }

	inline bool WasCallVote()						{ return m_bWasCallVote; }
	inline void SetVoteState( const bool bState )	{ m_bWasCallVote = bState; }

	inline unsigned long int	GetTickLastPacketRecved()		{ return m_nTickLastPacketRecved; }
	inline unsigned long int	GetLastHShieldMsgRecved()		{ return m_nLastHShieldMsgRecved; }
	inline bool				GetHShieldMsgRecved()			{ return m_bHShieldMsgRecved; }
	inline void				SetHShieldMsgRecved(bool bSet)	{ m_bHShieldMsgRecved = bSet; }


	void UpdateTickLastPacketRecved();
	void UpdateLastHShieldMsgRecved();

	//void SetLastRecvNewHashValueTime( const DWORD dwTime )	
	//{ 
	//	m_dwLastRecvNewHashValueTime = dwTime; 
	//	m_bIsRequestNewHashValue = false; 
	//}

	DWORD GetLastSpawnTime( void)					{ return m_dwLastSpawnTime;		}
	void SetLastSpawnTime( DWORD dwTime)			{ m_dwLastSpawnTime = dwTime;	}

	inline unsigned long int	GetQuestLatency();
	inline void					SetQuestLatency(unsigned long int l);
	inline void					SetPingTime(unsigned long int t);

	const DWORD GetLastSendSuicideCmdTime()							{ return m_dwLastSendSuicideCommand; }
	inline void SetLastSendSuicideCmdTime( const DWORD dwTime )		{ m_dwLastSendSuicideCommand = dwTime; }

	const DWORD GetLastSendGambleItemTime() const { return m_dwLastSendGambleItemTime; }
	void SetLastSendGambleItemTime( const DWORD dwCurTime )	{ m_dwLastSendGambleItemTime = dwCurTime; }
public:
	// Ladder °EÃ
	int GetLadderGroupID()			{ return m_nLadderGroupID; }
	void SetLadderGroupID(int nID)	{ m_nLadderGroupID = nID; }
	void SetLadderChallenging(bool bVal) { m_bLadderChallenging = bVal; }
	bool IsLadderChallenging()			{ return m_bLadderChallenging; }		// Å¬·£ÀE»ó´EÀ ´EâÁßÀÎÁE¿©ºÎ
public:
	MMatchAccountInfo* GetAccountInfo() { return &m_AccountInfo; }
	MMatchCharInfo* GetCharInfo()	{ return m_pCharInfo; }
	void SetCharInfo(MMatchCharInfo* pCharInfo);
	
	MMatchFriendInfo* GetFriendInfo()	{ return m_pFriendInfo; }
	void SetFriendInfo(MMatchFriendInfo* pFriendInfo);
	bool DBFriendListRequested()	{ return m_bDBFriendListRequested; }
	MMatchPlace GetPlace()			{ return m_nPlace; }
	void SetPlace(MMatchPlace nPlace);
	MMatchTimeSyncInfo* GetSyncInfo()	{ return &m_nTimeSyncInfo; }
//	MMatchObjectAntiHackInfo* GetAntiHackInfo()		{ return &m_AntiHackInfo; }
	MMatchDisconnStatusInfo& GetDisconnStatusInfo() { return  m_DisconnStatusInfo; }
	MMatchObjectHShieldInfo* GetHShieldInfo()		{ return &m_HSieldInfo; }

	/// Æ½ Ã³¸®
	virtual void Tick(unsigned long int nTime);

	void OnStageJoin();
	void OnEnterBattle();
	void OnLeaveBattle();
	void OnInitRound();

	void SetStageCursor(int nStageCursor);
	const MMatchObjectGameInfo* GetGameInfo() { return &m_GameInfo; }

	//filter
	void			SetCountryCode3( const string strCountryCode3 ) { m_strCountryCode3 = strCountryCode3; }
	const string&	GetCountryCode3() const							{ return m_strCountryCode3; }

	void DisconnectHacker( MMatchHackingType eType );
// 	void SetXTrapHackerDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus = MMDS_DISCONN_WAIT );
// 	void SetHShieldHackerDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus = MMDS_DISCONN_WAIT );
// 	void SetBadFileCRCDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus = MMDS_DISCONN_WAIT );
// 	void SetBadUserDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus = MMDS_DISCONN_WAIT );
// 	void SetGameguardHackerDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus = MMDS_DISCONN_WAIT );
// 	void SetDllInjectionDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus = MMDS_DISCONN_WAIT );
// 	void SetInvalidStageSettingDisconnectWaitInfo( const MMatchDisconnectStatus DisStatus = MMDS_DISCONN_WAIT );

public:
	enum MMO_ACTION
	{
		MMOA_STAGE_FOLLOW,
		MMOA_MAX
	};
	enum LadderWarsInviteState
	{
		Invited,
		Inviter,
		NotInvited
	};
	bool CheckEnableAction(MMO_ACTION nAction);		// ÇØ´E¾×¼ÇÀÌ °¡´ÉÇÑÁE¿©ºÎ - ÇÊ¿äÇÒ¶§¸¶´Ù Ãß°¡ÇÑ´Ù.

	bool m_bQuestRecvPong;
	DWORD m_dwHShieldCheckCount;

	MUID LadderWarsFriend;
	LadderWarsInviteState LadderState;

public :
	void DeleteGameguard();

	const bool IsSendFirstGameguardRequest() const	{ return m_IsSendFirstGameguardRequest; }
	void FirstGameguardReqeustIsSent()				{ m_IsSendFirstGameguardRequest = true; }

	const bool IsRecvFirstGameguardResponse() const { return m_IsRecvFirstGameguardResponse; }
	void FirstGameguardResponseIsRecved()			{ m_IsRecvFirstGameguardResponse = true; }

	void ResetCustomItemUseCount();
	const bool IsHaveCustomItem();
	const bool IncreaseCustomItemUseCount();
#ifdef _PORTALGUN 1
	const bool IsHavePortalGunEquipped();
#endif
	bool IsEquipCustomItem(int nItemId);


///< Ã¤ÆÃÃ¢ µµ¹E¹× º¡¾ûÔ® ±â´É ±¸ÇE
///< È«±âÁÖ(2009.08.05)
protected :
	MMatchChatFloodInfo m_ChatFloodInfo;
#ifdef _ANTISPAM
	unsigned long LastChatTime;
	int ChatBlockCount, ChatBlockMins;
#endif
public:
	bool CheckChatFlooding()		{ return m_ChatFloodInfo.PushChatTickCount();}
#ifdef _ANTISPAM
	bool SetLastChatTime(unsigned long time) { if ((time - LastChatTime) < 2000) { ChatBlockCount++; return false; } else { LastChatTime = time; return true; } }
	int GetChatBlockCount() { return ChatBlockCount; }
	void SetChatBlockCount(int count) { ChatBlockCount = count; }
	int GetChatBlockMins() { ChatBlockMins += 10; return ChatBlockMins; }
#endif
	bool IsChatBanUser()			{ return m_ChatFloodInfo.IsBanUser();}
	void SetChatBanUser()			{ m_ChatFloodInfo.SetBanUser();}

///< µà¾EÅä³Ê¸ÕÆ® °EÃ
///< È«±âÁÖ(2009.09.25)

public:	
	LadderWarsCharInfo* m_pLadderWarsCharInfo;
	LadderWarsCharInfo* GetLadderWarsInfo() { return m_pLadderWarsCharInfo; }
	void SetLadderWarsCharInfo(LadderWarsCharInfo* info);

	MMatchObjectDuelTournamentCharInfo* m_pDuelTournamentCharInfo;
	MMatchObjectDuelTournamentCharInfo* GetDuelTournamentCharInfo()		{ return m_pDuelTournamentCharInfo; }
	void SetDuelTournamentCharInfo(MMatchObjectDuelTournamentCharInfo *pDTCharInfo);	
	void FreeDuelTournamentInfo();	
	void FreeLadderWarsInfo();
	bool IsChallengeDuelTournament() { 
		if( m_pDuelTournamentCharInfo == NULL ) return false;
		return m_pDuelTournamentCharInfo->IsChallengeDuelTournament(); 
	}

	void SetChallengeDuelTournament(bool bValue) { 
		if( m_pDuelTournamentCharInfo == NULL ) return;
		m_pDuelTournamentCharInfo->SetChallengeDuelTournament(bValue); 
	}

	bool IsJoinDuelTournament() { 
		if( m_pDuelTournamentCharInfo == NULL ) return false;
		return m_pDuelTournamentCharInfo->IsJoinDuelTournament(); 
	}

	void SetJoinDuelTournament(bool bValue) { 
		if( m_pDuelTournamentCharInfo == NULL ) return;
		m_pDuelTournamentCharInfo->SetJoinDuelTournament(bValue); 
	}

protected:
	MMatchObjectCharBuff	m_CharBuffInfo;

public:
	void FreeCharBuff()							{ m_CharBuffInfo.FreeCharBuffInfo(); }
	MMatchObjectCharBuff*	GetCharBuff()		{ return &m_CharBuffInfo; }
	
///< Punishment °EÃ
///< È«±âÁÖ(2010.08.09)
protected:
	MMatchAccountPenaltyInfo m_AccountPenaltyInfo;

public:
	MMatchAccountPenaltyInfo* GetAccountPenaltyInfo() { return &m_AccountPenaltyInfo; }

protected:
	unsigned int m_nLastCheckBattleTimeReward;

	void SetLastCheckBattleTimeReward(unsigned int nVal)	{ m_nLastCheckBattleTimeReward = nVal; }
	unsigned int GetLastCheckBattleTimeReward()				{ return m_nLastCheckBattleTimeReward; }

	void SetBattleTimeReward(bool bVal);
	void BattleTimeReward(unsigned int nTime);

protected:
	void ResetGamePlayInfo() { if (m_pCharInfo != NULL) m_pCharInfo->GetCharGamePlayInfo()->Reset(); }

};

class MMatchObjectList : public map<MUID, MMatchObject*>{};


// Ä³¸¯ÅÍ »ý¼ºÇÒ¶§ ÁÖ´Â ±âº» ¾ÆÀÌÅÛ
struct MINITIALCOSTUME
{
	// ¹«±E
	unsigned int nMeleeItemID;
	unsigned int nPrimaryItemID;
	unsigned int nSecondaryItemID;
	unsigned int nCustom1ItemID;
	unsigned int nCustom2ItemID;
	unsigned int nCustom3ItemID;

	// ÀåºE¾ÆÀÌÅÛ
	unsigned int nHeadItemID;
	unsigned int nChestItemID;
	unsigned int nHandsItemID;
	unsigned int nLegsItemID;
	unsigned int nFeetItemID;
};

#define MAX_COSTUME_TEMPLATE		1
const MINITIALCOSTUME g_InitialCostume[MAX_COSTUME_TEMPLATE][2] = 
{ 
	// Custom: No costumes
	//{{1, 5001, 4001, 30301, 0,     0, 21001, 0, 23001, 0},    {1, 5001, 4001, 30301, 0,     0, 21501, 0, 23501, 0}},
    //{{2, 5002, 0,    30301, 0,     0, 21001, 0, 23001, 0},    {2, 5002, 0,    30301, 0,     0, 21501, 0, 23501, 0}},
    //{{1, 4005, 5001, 30401, 0,     0, 21001, 0, 23001, 0},    {1, 4005, 5001, 30401, 0,     0, 21501, 0, 23501, 0}},
    //{{2, 4001, 0,    30401, 0,     0, 21001, 0, 23001, 0},    {2, 4001, 0,    30401, 0,     0, 21501, 0, 23501, 0}},
    //{{2, 4002, 0,    30401, 30001, 0, 21001, 0, 23001, 0},    {2, 4002, 0,    30401, 30001, 0, 21501, 0, 23501, 0}},
    //{{1, 4006, 0,    30101, 30001, 0, 21001, 0, 23001, 0},    {1, 4006, 4006, 30101, 30001, 0, 21501, 0, 23501, 0}}

	// Custom: Costumes
   {{2010031, 2070014, 0, 2200007, 0, 0,   0, 3010000, 3030000, 3020000, 3040000},    {2010031, 2070014, 0, 2200007, 0, 0, 0, 3110000, 3130007, 3120055, 3140036}},
   //{{2, 5001, 4001, 30301, 0,     0, 520008, 521003, 522003, 523003, 524003},    {1, 5001, 4001, 30301, 0,     0, 520508, 521503, 522503, 523503, 524503}},
   //{{1, 5002, 0,    30301, 0,     0, 520068, 521059, 522049, 523053, 524052},    {2, 5002, 0,    30301, 0,     0, 520568, 521559, 522549, 523553, 524552}},
   //{{2, 4005, 5001, 30401, 0,     0, 520047, 521038, 522030, 523033, 524033},    {1, 4005, 5001, 30401, 0,     0, 520547, 521538, 522530, 523533, 524533}},
   //{{1, 4001, 0,    30401, 0,     0, 520037, 521029, 522024, 523027, 524027},    {2, 4001, 0,    30401, 0,     0, 520537, 521529, 522524, 523527, 524527}},
   //{{1, 4002, 0,    30401, 30001, 0, 520057, 521049, 522040, 523043, 524043},    {2, 4002, 0,    30401, 30001, 0, 520557, 521549, 522540, 523543, 524543}},
   //{{1, 4006, 0,    30101, 30001, 0, 520069, 521060, 522050, 523054, 524053},    {1, 4006, 4006, 30101, 30001, 0, 520569, 521560, 522550, 523554, 524553}}
	/*
	{{1, 5001, 4001, 30301, 0,     21001, 0, 23001, 0},	{1, 5001, 4001, 30301, 0,     21501, 0, 23501, 0}},	// °Ç³ªÀÌÆ®
	{{2, 5002, 0,    30301, 0,     21001, 0, 23001, 0},	{2, 5002, 0,    30301, 0,     21501, 0, 23501, 0}},	// °ÇÆÄÀÌÅÍ
	{{1, 4005, 5001, 30401, 0,     21001, 0, 23001, 0},	{1, 4005, 5001, 30401, 0,     21501, 0, 23501, 0}},	// ¾Ö¼­½Å
	{{2, 4001, 0,    30401, 0,     21001, 0, 23001, 0},	{2, 4001, 0,    30401, 0,     21501, 0, 23501, 0}},	// ½ºÄ«¿E®
	{{2, 4002, 0,    30401, 30001, 21001, 0, 23001, 0},	{2, 4002, 0,    30401, 30001, 21501, 0, 23501, 0}},	// °ÇÇÁ¸®½ºÆ®
	{{1, 4006, 0,	 30101, 30001, 21001, 0, 23001, 0},	{1, 4006, 4006, 30101, 30001, 21501, 0, 23501, 0}}	// ´ÚÅÍ
	*/
};

/*
const unsigned int g_InitialHair[4][2] = 
{
	{10000, 10022},
	{10001, 10023},
	{10002, 10024},
	{10003, 10025}
};
*/

#define MAX_COSTUME_HAIR		1 //5
const string g_szHairMeshName[MAX_COSTUME_HAIR][2] = 
{
	//{"eq_head_01", "eq_head_pony"},
	//{"eq_head_02", "eq_head_hair001"},
	//{"eq_head_08", "eq_head_hair04"},
	//{"eq_head_05", "eq_head_hair006"},
	//{"eq_head_zzz", "eq_head_hair002"}		// ÀÌ°Ç ÇöÀE»ç¿EÈÇÔ - ³ªÁß¿¡ ´Ù¸¥ ¸ðµ¨·Î ´E¼ÇØµµ µÊ
	{"eq_head_000", "eq_head_000"},// ÀÌ°Ç ÇöÀE»ç¿EÈÇÔ - ³ªÁß¿¡ ´Ù¸¥ ¸ðµ¨·Î ´E¼ÇØµµ µÊ
};

#define MAX_COSTUME_FACE		1 //19
const string g_szFaceMeshName[MAX_COSTUME_FACE][2] = 
{
	// ±âº»
	{"eq_face_000", "eq_face_000"},
	/*{"eq_face_01", "eq_face_001"},
	{"eq_face_02", "eq_face_002"},
	{"eq_face_04", "eq_face_003"},
	{"eq_face_05", "eq_face_004"},

	{"eq_face_newface01", "eq_face_newface01"},
	{"eq_face_newface02", "eq_face_newface02"},
	{"eq_face_newface03", "eq_face_newface03"},
    {"eq_face_newface04", "eq_face_newface04"},
	{"eq_face_newface05", "eq_face_newface05"},
	{"eq_face_newface06", "eq_face_newface06"},
	{"eq_face_newface07", "eq_face_newface07"},
	{"eq_face_newface08", "eq_face_newface08"},
	{"eq_face_newface09", "eq_face_newface09"},
	{"eq_face_newface10", "eq_face_newface10"},
	{"eq_face_newface11", "eq_face_newface11"},
	{"eq_face_newface12", "eq_face_newface12"},
	{"eq_face_newface13", "eq_face_newface13"},
	{"eq_face_newface14", "eq_face_newface14"}, 
	{"eq_face_newface15", "eq_face_newface15"}, */
};
//³²ÀÚ´Â eq_face_newface01 - 13
//¿©ÀÚ´Â eq_face_newface01 - 15

// MC_MATCH_STAGE_ENTERBATTLE Ä¿¸Çµå¿¡¼­ ¾²ÀÌ´Â ÆÄ¶ó¸ÞÅ¸
enum MCmdEnterBattleParam
{
	MCEP_NORMAL = 0,
	MCEP_FORCED = 1,		// ³­ÀÔ
	MCEP_END
};

// ÀÔÀ» ¼EÀÖ´Â ¾ÆÀÌÅÛÀÎÁEÃ¼Å©
bool IsEquipableItem(unsigned long int nItemID, int nPlayerLevel, MMatchSex nPlayerSex);

inline bool IsEnabledObject(MMatchObject* pObject) 
{
	if ((pObject == NULL) || (pObject->GetCharInfo() == NULL)) return false;
	return true;
}

inline bool IsAdminGrade(MMatchUserGradeID nGrade)
{
	if ((nGrade == MMUG_EVENTMASTER) ||
		(nGrade == MMUG_ADMIN) ||
#ifdef _NEWGRADE
		(nGrade == MMUG_HIDE_ADMIN) ||
#endif
		(nGrade == MMUG_EVENTTEAM) ||
		(nGrade == MMUG_GAMEMASTER) ||
		(nGrade == MMUG_DEVELOPER) ||
		(nGrade == MMUG_MANAGER))
		return true;

	return false;
}
inline bool IsAdminGrade(MMatchObject* pObject) 
{
	if (pObject == NULL) return false;

	return IsAdminGrade(pObject->GetAccountInfo()->m_nUGrade);
}
#ifdef _VIPGRADES
inline bool IsVipGrade(MMatchUserGradeID nGrade)
{
	if ((nGrade == MMUG_VIP1) ||
		(nGrade == MMUG_VIP2) ||
		(nGrade == MMUG_VIP3) ||
		(nGrade == MMUG_VIP4) ||
		(nGrade == MMUG_VIP5) ||
		(nGrade == MMUG_VIP6) ||
		(nGrade == MMUG_VIP7))
		return true;

	return false;
}
inline bool IsVipGrade(MMatchObject* pObject)
{
	if (pObject == NULL) return false;

	return IsVipGrade(pObject->GetAccountInfo()->m_nUGrade);
}
#endif
#ifdef _EVENTGRD
inline bool IsEventGrade(MMatchUserGradeID nGrade)
{
	if ((nGrade == MMUG_EVENT1) ||
		(nGrade == MMUG_EVENT2) ||
		(nGrade == MMUG_EVENT3) ||
		(nGrade == MMUG_EVENT4) ||
		(nGrade == MMUG_BOOSTER) ||
		(nGrade == MMUG_WINTOUR) ||
		(nGrade == MMUG_STAR))
		return true;

	return false;
}
inline bool IsEventGrade(MMatchObject* pObject)
{
	if (pObject == NULL) return false;

	return IsEventGrade(pObject->GetAccountInfo()->m_nUGrade);
}
#endif
#ifdef _EVENTCMD
inline bool IsEventMasterGrade(MMatchUserGradeID nGrade)
{
	if ((nGrade == MMUG_GAMEMASTER) ||
		(nGrade == MMUG_MANAGER))
		return true;

	return false;
}

inline bool IsEventMasterGrade(MMatchObject* pObject)
{
	if (pObject == NULL) return false;

	return IsEventMasterGrade(pObject->GetAccountInfo()->m_nUGrade);
}
#endif
//////////////////////////////////////////////////////////////////////////


unsigned long int MMatchObject::GetQuestLatency()
{
	unsigned long int nowTime = timeGetTime();

	unsigned long int ret = nowTime - m_nLastPingTime;

	if (ret > m_nQuestLatency)
		m_nQuestLatency = ret;

	return m_nQuestLatency;
}

void MMatchObject::SetQuestLatency(unsigned long int l)
{
	m_nQuestLatency = l - m_nLastPingTime;
}

void MMatchObject::SetPingTime(unsigned long int t)
{
	m_nLastPingTime = t;
}

#endif
