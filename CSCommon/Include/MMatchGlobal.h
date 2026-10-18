#ifndef _MMATCHGLOBAL_H
#define _MMATCHGLOBAL_H

#include "MBaseLocale.h"
#include "MUID.h"

constexpr auto MAX_CHAR_COUNT		=		4;		


constexpr auto CYCLE_STAGE_UPDATECHECKSUM	= 500;


constexpr auto NUM_APPLYED_TEAMBONUS_TEAM_PLAYERS = 3;		
constexpr auto RESPAWN_DELAYTIME_AFTER_DYING = 5000;	
constexpr auto RESPAWN_DELAYTIME_AFTER_DYING_MIN = 2000;	
constexpr auto RESPAWN_DELAYTIME_AFTER_DYING_MAX = 20000;	

constexpr auto MAX_XP_BONUS_RATIO = 2.0f;	
constexpr auto MAX_BP_BONUS_RATIO = 2.0f;	

constexpr auto TRANS_STAGELIST_NODE_COUNT =	8;	
constexpr auto TRANS_STANDBY_CLANLIST_NODE_COUNT = 4;	


constexpr auto MAX_REPLIER = 16;			

#ifndef _2PCLAN
constexpr auto CLAN_SPONSORS_COUNT = 1;		
#else
constexpr auto CLAN_SPONSORS_COUNT = 4;		
#endif 

constexpr auto CLAN_CREATING_NEED_BOUNTY = 2000; 
constexpr auto CLAN_CREATING_NEED_LEVEL = 10;		


#ifdef _LVL200
constexpr auto MAX_CHAR_LEVEL = 99;
#else
constexpr auto MAX_CHAR_LEVEL = 999;
#endif
constexpr auto MATCH_SIMPLE_DESC_LENGTH = 64;


constexpr auto ACTIONLEAGUE_TEAM_MEMBER_COUNT = 4;		
constexpr auto MAX_LADDER_TEAM_MEMBER = 4;		
constexpr auto MAX_CLANBATTLE_TEAM_MEMBER = 8;		

#define CLAN_BATTLE					

enum LadderWar
{
	MPLADDERWARSTYPE_NORMAL_1VS1 = 0,
	MPLADDERWARSTYPE_NORMAL_2VS2,
	MPLADDERWARSTYPE_NORMAL_3VS3,
	MPLADDERWARSTYPE_NORMAL_4VS4,
	MPLADDERWARSTYPE_NORMAL_5VS5,
	MPLADDERWARSTYPE_NORMAL_6VS6,
	MPLADDERWARSTYPE_NORMAL_7VS7,
	MPLADDERWARSTYPE_NORMAL_8VS8,
	MPLADDERWARSTYPE_MAX
};

enum MMatchTeam
{
	MMT_ALL			= 0,
	MMT_SPECTATOR	= 1,
	MMT_RED			= 2,
	MMT_BLUE		= 3,
	MMT_END
};

enum MMatchServerMode
{
	MSM_NORMAL1		= 0,		
	MSM_CLAN		= 1,		
	MSM_LADDER		= 2,		
	MSM_EVENT		= 3,		
	MSM_TEST		= 4,		
	MSM_MAX,

	MSM_ALL			= 100,		
};

enum MMatchProposalMode
{
	MPROPOSAL_NONE = 0,				
	MPROPOSAL_LADDER_INVITE,		
	MPROPOSAL_CLAN_INVITE,			
	MPROPOSAL_END
};


enum MLADDERTYPE
{
	MLADDERTYPE_NORMAL_1VS1 = 0, // Custom 1 vs 1
	MLADDERTYPE_NORMAL_2VS2,
	MLADDERTYPE_NORMAL_3VS3,
	MLADDERTYPE_NORMAL_4VS4,
	MLADDERTYPE_NORMAL_5VS5, // Custom 5vs5
	MLADDERTYPE_MAX
};
const int g_nNeedLadderMemberCount[MLADDERTYPE_MAX] =
{
	1,  // Custom 1 vs 1
	2,
	3,
	4,
	5 // Custom 5vs5
};



constexpr auto DEFAULT_CLAN_POINT = 1000;
constexpr auto DAY_OF_DELETE_CLAN = 7;
constexpr auto MAX_WAIT_CLAN_DELETE_HOUR = 24;
constexpr auto UNDEFINE_DELETE_HOUR = 2000000000;

enum MMatchClanDeleteState
{
	MMCDS_NORMAL = 1,
	MMCDS_WAIT,
	MMCDS_DELETE,

	MMCDS_END,
};



enum MBITFLAG_USEROPTION {
	MBITFLAG_USEROPTION_REJECT_WHISPER	= 1,
	MBITFLAG_USEROPTION_REJECT_INVITE	= 1<<1
};

enum WarpHackType
{
	WHT_NONE,
	WHT_MOUSE,
	WHT_KEYBOARD,
	WHT_KEYMOUSE64,
	WHT_INTEGRITYTHREAD,
	WHT_MEMORY,
	WHT_MACRO,
	WHT_CHEATENGINE,
	WHT_RAWINPUTHOOK,
	WHT_TIME_GETTICKCOUNT,
	WHT_TIME_TIMEGETTIME,
	WHT_TIME_QPC,
	WHT_D3D_ENDSCENE,
	WHT_D3D_DRAWINDEXEDPRIMITIVE,
	WHT_D3D_RESET,
	WHT_HITBOX,
	WHT_PICKLESDASKID,
	WHT_MAIETCRC,
	WHT_MEMCMP,
	WHT_CQHACK,
	WHT_UNDERCLOCK,
	WHT_HOOKS,
	WHT_PROGRAMS,
	WHT_BULLET,
	WHT_RANGE,
	WHT_HPAP,
	WHT_MOUSEMACRO,
	WHT_INJECT,
	WHT_SUDASH,
	WHT_SPAWN,
	WHT_ESP,
	WHT_DIEPEER,
	WHT_RELOADFAST,
	WHT_END
};



constexpr auto MAX_QUEST_MAP_SECTOR_COUNT = 16;

#ifdef _DEBUG_QUEST
constexpr auto MAX_QUEST_NPC_INFO_COUNT = 100;
#else
constexpr auto MAX_QUEST_NPC_INFO_COUNT = 14;
#endif



constexpr auto ALL_PLAYER_NOT_READY = 1;
constexpr auto QUEST_START_FAILED_BY_SACRIFICE_SLOT = 2;
constexpr auto INVALID_TACKET_USER = 3;
constexpr auto INVALID_MAP = 4;

constexpr auto MIN_QUESTITEM_ID = 200001;
constexpr auto MAX_QUESTITEM_ID = 299999;

enum KMS_SCHEDULE_TYPE
{
	KMST_NO = 0,
	KMST_REPEAT,
	KMST_COUNT,
	KMST_ONCE,

	KMS_SCHEDULE_TYPE_END,
};

enum KMS_COMMAND_TYPE
{
	KMSC_NO = 0,
	KMSC_ANNOUNCE,
	KMSC_STOP_SERVER,
	KMSC_RESTART_SERVER,
	
	KMS_COMMAND_TYPE_END,
};

enum SERVER_STATE_KIND
{
	SSK_OPENDB = 0,

	SSK_END,
};

enum SERVER_ERR_STATE
{
	SES_NO = 0,
	SES_ERR_DB,
    
	SES_END,
};

enum SERVER_TYPE
{
	ST_NULL = 0,
	ST_DEBUG,
	ST_NORMAL,
	ST_CLAN,
	ST_QUEST,
	ST_EVENT,
};


enum MMatchBlockLevel
{
	MMBL_NO = 0,
	MMBL_ACCOUNT,
	MMBL_LOGONLY,

	MMBL_END,
};

inline MMatchTeam NegativeTeam(MMatchTeam nTeam)
{
	if (nTeam == MMT_RED) return MMT_BLUE;
	else if (nTeam == MMT_BLUE) return MMT_RED;
	return nTeam;
}

constexpr auto ANNOUNCE_STRING_LEN = 512;
constexpr auto MSG_STRING_LEN = 64;
constexpr auto CHAT_STRING_LEN = 64;
constexpr auto VOTE_DISCUSS_STRING_LEN = 64;
constexpr auto VOTE_ARG_STRING_LEN = 64;
constexpr auto STAGENAME_LENGTH = 64;
constexpr auto STAGEPASSWD_LENGTH = 8;
constexpr auto STAGE_QUEST_MAX_PLAYER = 4;
constexpr auto STAGE_MAX_PLAYERCOUNT = 64;
constexpr auto STAGE__MAX_ROUND = 500;
constexpr auto CLAN_NAME_LENGTH = 16;
constexpr auto MIN_CLANNAME = 3;
constexpr auto MAX_CLANNAME = 12;
constexpr auto MIN_CHARNAME = 4;
constexpr auto MAX_CHARNAME = 12;
constexpr auto MATCHOBJECT_NAME_LENGTH = 32;
constexpr auto MAX_CHARNAME_LENGTH = 24;
constexpr auto MAX_CHATROOMNAME_STRING_LEN = 64;
constexpr auto MAX_USERID_STRING_LEN = 21;
constexpr auto MAX_USER_PASSWORD_STRING_LEN = 20;
constexpr auto USERNAME_STRING_LEN = 50;
constexpr auto CHANNELNAME_LEN = 64;
constexpr auto CHANNELRULE_LEN = 64;
constexpr auto MAPNAME_LENGTH = 32;
//Custom: Lightmaps Time Code
constexpr auto MAX_LIGHTMAP = 4;
constexpr auto NHN_GAMEID = "u_gunz";
constexpr auto NHN_AUTH_LENGTH = 4096;
constexpr auto NHN_OUTBUFF_LENGTH = 1024;
constexpr auto MAX_ACCOUNT_ITEM = 1000;
constexpr auto MAX_EXPIRED_ACCOUNT_ITEM = 100;
constexpr auto MAX_ITEM_COUNT = 500;
constexpr auto MAX_QUEST_REWARD_ITEM_COUNT = 500;
constexpr auto MAX_SPENDABLE_ITEM_COUNT = 999;
constexpr auto MAX_GAMBLEITEMNAME_LEN = (65);
constexpr auto MAX_GAMBLEITEMDESC_LEN = (65);
constexpr auto MAX_BUYGAMBLEITEM_ELAPSEDTIME_MIN = (5);
constexpr auto MIN_REQUEST_STAGESTART_TIME = (1 * 1000);
constexpr auto MIN_REQUEST_SUICIDE_TIME = (1000 * 60 * 3);
constexpr auto MAX_MD5LENGH = (16);
constexpr auto MAX_HWID_ENGH = (12);
constexpr auto MAX_SURVIVAL_SCENARIO_COUNT = 3;
constexpr auto MAX_SURVIVAL_RANKING_LIST = 10;

typedef struct _RankingInfo
{
	char szCharName[MATCHOBJECT_NAME_LENGTH];
	DWORD dwCID;
	DWORD dwRanking;
	DWORD dwRankingPoint;
} RANKINGINFO;

constexpr auto DUELTOURNAMENT_TIMESTAMP_MAX_LENGTH = 8;

enum MDUELTOURNAMENTTYPE
{
	MDUELTOURNAMENTTYPE_FINAL				= 0,		
	MDUELTOURNAMENTTYPE_SEMIFINAL,						
	MDUELTOURNAMENTTYPE_QUATERFINAL,					
	MDUELTOURNAMENTTYPE_MAX	
};

enum MDUELTOURNAMENTROUNDSTATE {
	MDUELTOURNAMENTROUNDSTATE_FINAL = 0,	
	MDUELTOURNAMENTROUNDSTATE_SEMIFINAL,
	MDUELTOURNAMENTROUNDSTATE_QUATERFINAL,
	MDUELTOURNAMENTROUNDSTATE_MAX
};

enum MDUELTOURNAMENTMATCHMAKINGFACTOR			
{
	MDUELTOURNAMENTMATCHMAKINGFACTOR_TPGAP = 0,			
	MDUELTOURNAMENTMATCHMAKINGFACTOR_OVERWAIT,			
};

#define DUELTOURNAMENT_PRECOUNTDOWN_WINLOSE_SHOWTIME	4000
#define DUELTOURNAMENT_PRECOUNTDOWN_NEXTMATCH_SHOWTIME	6000

inline int GetDTPlayerCount(MDUELTOURNAMENTTYPE nType)
{
	switch(nType) {
		case MDUELTOURNAMENTTYPE_QUATERFINAL :		return 8;
		case MDUELTOURNAMENTTYPE_SEMIFINAL :		return 4;
		case MDUELTOURNAMENTTYPE_FINAL :			return 2;			
	}

	return 0;
}

inline int GetDTRoundCount(MDUELTOURNAMENTROUNDSTATE nRoundState)
{
	switch(nRoundState) {
		case MDUELTOURNAMENTROUNDSTATE_QUATERFINAL :		return 4;
		case MDUELTOURNAMENTROUNDSTATE_SEMIFINAL :			return 2;
		case MDUELTOURNAMENTROUNDSTATE_FINAL :				return 1;			
	}

	return 0;
}

typedef struct _DTRankingInfo
{
	char m_szCharName[MATCHOBJECT_NAME_LENGTH];
	int m_nTP;
	int m_nWins;
	int m_nLoses;
	int m_nRanking;
	int m_nRankingIncrease;
	int m_nFinalWins;
	int m_nGrade;
} DTRankingInfo;

typedef struct _DTPlayerInfo
{
	char m_szCharName[MATCHOBJECT_NAME_LENGTH];
	MUID uidPlayer;
	int m_nTP;
} DTPlayerInfo;

constexpr auto ACCOUNTITEM_INCREASE_LOG_MOVE = 100;
constexpr auto ACCOUNTITEM_INCREASE_LOG_BUY = 101;
constexpr auto ACCOUNTITEM_INCREASE_LOG_REWARD_BY_GM = 102;

constexpr auto ACCOUNTITEM_DECREASE_LOG_MOVE = 200;
constexpr auto ACCOUNTITEM_DECREASE_LOG_SELL = 201;
constexpr auto ACCOUNTITEM_DECREASE_LOG_EXPIRE = 202;

constexpr auto CHARITEM_INCREASE_LOG_MOVE = 100;
constexpr auto CHARITEM_INCREASE_LOG_BUY = 101;
constexpr auto CHARITEM_INCREASE_LOG_REWARD = 102;

constexpr auto CHARITEM_DECREASE_LOG_MOVE = 200;
constexpr auto CHARITEM_DECREASE_LOG_SELL = 201;
constexpr auto CHARITEM_DECREASE_LOG_EXPIRE = 202;
constexpr auto CHARITEM_DECREASE_LOG_GAMBLE = 203;
constexpr auto CHARITEM_DECREASE_LOG_SPEND = 204;

constexpr auto	MAX_CHARACTER_SHORT_BUFF_COUNT = 2;

typedef struct _MLongBuffInfoInDB
{	
	int nCBID;
	int nBuffID;
	int nBuffSecondPeriod;
	int nStartPlayTime;	
} MLongBuffInfoInDB;

typedef struct _MLongBuffInfo
{	
	MUID uidBuff;

	int nCBID;
	int nBuffID;
	int nBuffPeriod;
	int nBuffPeriodRemainder;	
} MLongBuffInfo;

typedef struct _MShortBuffInfo
{	
	MUID uidBuff;

	int nBuffID;
	int nBuffPeriod;
	int nBuffPeriodRemainder;
} MShortBuffInfo;

struct LadderWarsCharInfo
{
	int Ranking;
	int Wins;
	int Losses;
	int Draws;
	int Score;
	void AddLoss() { Losses++; }
	void AddWin() { Wins++; }
	void AddDraw() { Draws++; }
	void AddScore(int score) { Score += score; }
	void MinusScore(int score) { Score -= score; if (Score <= -1) Score = 0; }
};
typedef struct _PWRankingInfo
{
	char m_szCharName[MATCHOBJECT_NAME_LENGTH];
	int m_nWins;
	int m_nLosses;
	int m_nScore; //Custom: LadderPoints Rank
	int m_nRanking;
} PWRankingInfo;

constexpr auto MAX_TRAP_THROWING_LIFE = 10.f;


#endif