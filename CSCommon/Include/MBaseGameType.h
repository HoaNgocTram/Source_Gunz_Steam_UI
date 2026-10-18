#ifndef _MBASEGAMETYPE_H
#define _MBASEGAMETYPE_H

#include <crtdbg.h>
#include <map>
#include <set>
using namespace std;

    // Custom: Game modes old and classic.
enum MMATCH_GAMETYPE {
	MMATCH_GAMETYPE_DEATHMATCH_SOLO		=0,			///< Deathmatch
	MMATCH_GAMETYPE_DEATHMATCH_TEAM		=1,			///< Deathmatch team
	MMATCH_GAMETYPE_GLADIATOR_SOLO		=2,			///< Gladiator
	MMATCH_GAMETYPE_GLADIATOR_TEAM		=3,			///< Gladiator team
	MMATCH_GAMETYPE_ASSASSINATE			=4,			///< Assassinate
	MMATCH_GAMETYPE_TRAINING			=5,			///< Training

	MMATCH_GAMETYPE_SURVIVAL			=6,			///< Survival
	MMATCH_GAMETYPE_QUEST				=7,			///< Quest

	MMATCH_GAMETYPE_BERSERKER			=8,			///< Berserker
	MMATCH_GAMETYPE_DEATHMATCH_TEAM2	=9,			///< Team Extreme
	MMATCH_GAMETYPE_DUEL				=10,		///< Duel Match
	MMATCH_GAMETYPE_DUELTOURNAMENT		=11,		///< Duel Tournament
	MMATCH_GAMETYPE_QUEST_CHALLENGE		=12,		///< Challenge Quest

	// Custom: Game modes new.
	MMATCH_GAMETYPE_TEAM_TRAINING		=13,        ///< Training Team Event
	MMATCH_GAMETYPE_CTF					=14,        ///< Capture The Flag
	MMATCH_GAMETYPE_INFECTED			=15,        ///< Infected
	MMATCH_GAMETYPE_GUNGAME				=16,        ///< GunGame
	MMATCH_GAMETYPE_SPY				    =17,        ///< Spy Mode
	MMATCH_GAMETYPE_TURBO               =18,		///< Turbo

	MMATCH_GAMETYPE_VAMPIRE             =19,		///< Vampire

	MMATCH_GAMETYPE_SKILLMAP            =20,        ///< Skill map
	MMATCH_GAMETYPE_DROPMAGIC           =21,        ///< MagicBox
	MMATCH_GAMETYPE_PAINTBALL_SOLO     = 22,        ///< Paint Solo
	MMATCH_GAMETYPE_PAINTBALL_TEAM     = 23,        ///< Paint Team

	MMATCH_GAMETYPE_CLASSIC_SOLO       = 24,	    ///< Classic	
	MMATCH_GAMETYPE_MODE_STAFF         = 25,		///< Mode Staff
	MMATCH_GAMETYPE_CLASSIC_TEAM       = 26,		///< Classic Team
	MMATCH_GAMETYPE_SNIPERMODE         = 27,		///< Sniper Mode
	MMATCH_GAMETYPE_GHOST				= 28,       ///< Ghost Mode	// Custom: Ghost Mode - Desperate

	MMATCH_GAMETYPE_MAX,
};

extern const char* MMatchGameTypeAcronym[MMATCH_GAMETYPE_MAX];

#define		MAX_RELAYMAP_LIST_COUNT			20
struct RelayMap
{
	int				nMapID;
};

enum RELAY_MAP_TYPE
{
	RELAY_MAP_TURN			= 0,
	RELAY_MAP_RANDOM,

	RELAY_MAP_MAX_TYPE_COUNT
};

enum RELAY_MAP_REPEAT_COUNT
{
	RELAY_MAP_1REPEAT			= 0,
	RELAY_MAP_2REPEAT,
	RELAY_MAP_3REPEAT,
	RELAY_MAP_4REPEAT,
	RELAY_MAP_5REPEAT,

	RELAYMAP_MAX_REPEAT_COUNT,
};

const MMATCH_GAMETYPE MMATCH_GAMETYPE_DEFAULT = MMATCH_GAMETYPE_DEATHMATCH_SOLO;

struct MMatchGameTypeInfo
{
	MMATCH_GAMETYPE		nGameTypeID;			
	char				szGameTypeStr[24];		
	float				fGameExpRatio;			
	float				fTeamMyExpRatio;		
	float				fTeamBonusExpRatio;		
	set<int>			MapSet;					
	void Set(const MMATCH_GAMETYPE a_nGameTypeID, const char* a_szGameTypeStr, const float a_fGameExpRatio,
		     const float a_fTeamMyExpRatio, const float a_fTeamBonusExpRatio);
	void AddMap(int nMapID);
	void AddAllMap();
};


class MBaseGameTypeCatalogue
{
private:
	MMatchGameTypeInfo			m_GameTypeInfo[MMATCH_GAMETYPE_MAX];
public:
	MBaseGameTypeCatalogue();
	virtual ~MBaseGameTypeCatalogue();

	inline MMatchGameTypeInfo* GetInfo(MMATCH_GAMETYPE nGameType);
	inline const char* GetGameTypeStr(MMATCH_GAMETYPE nGameType);
	inline void SetGameTypeStr(MMATCH_GAMETYPE nGameType, const char* szName);
	inline bool IsCorrectGameType(const int nGameTypeID);	
	inline bool IsTeamGame(MMATCH_GAMETYPE nGameType);		
	inline bool IsSoloGame(MMATCH_GAMETYPE nGameType);
	inline bool IsTeamExtremeGame(MMATCH_GAMETYPE nGameType); 
	inline bool IsTeamLimitTime(MMATCH_GAMETYPE nGameType);
	inline bool IsSpyGame(MMATCH_GAMETYPE nGameType);
	inline bool IsMapSkill(MMATCH_GAMETYPE nGameType);
	inline bool IsMagicBox(MMATCH_GAMETYPE nGameType); 
	inline bool IsPaintballSolo(MMATCH_GAMETYPE nGameType);
	inline bool IsPaintballTeam(MMATCH_GAMETYPE nGameType);
	inline bool IsWaitForRoundEnd(MMATCH_GAMETYPE nGameType);		
	inline bool IsQuestOnly(MMATCH_GAMETYPE nGameType);	
	inline bool IsSurvivalOnly(MMATCH_GAMETYPE nGameType); 
	inline bool IsQuestChallengeOnly(MMATCH_GAMETYPE nGameType); // Custom: CQ fixes
	inline bool IsQuestDerived(MMATCH_GAMETYPE nGameType);	
	inline bool IsWorldItemSpawnEnable(MMATCH_GAMETYPE nGameType);	
};


//////////////////////////////////////////////////////////////////////////////////
inline bool MBaseGameTypeCatalogue::IsTeamGame(MMATCH_GAMETYPE nGameType)
{
	//Custom: Rule Custom Game
	if ((nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM2) ||
		(nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_ASSASSINATE) ||
		(nGameType == MMATCH_GAMETYPE_PAINTBALL_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_TEAM_TRAINING) ||
		(nGameType == MMATCH_GAMETYPE_CTF) ||
		(nGameType == MMATCH_GAMETYPE_INFECTED)||
		(nGameType == MMATCH_GAMETYPE_SNIPERMODE) ||
		(nGameType == MMATCH_GAMETYPE_CLASSIC_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_GHOST) ||				// Custom: Ghost Mode - Desperate
		(nGameType == MMATCH_GAMETYPE_SPY))
	{
		return true;
	}
	return false;
}
inline bool MBaseGameTypeCatalogue::IsSoloGame(MMATCH_GAMETYPE nGameType)
{
	//Custom: Rule Custom Game
	if ((nGameType == MMATCH_GAMETYPE_DEATHMATCH_SOLO) ||
		(nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO) ||
		(nGameType == MMATCH_GAMETYPE_TRAINING) ||
		(nGameType == MMATCH_GAMETYPE_PAINTBALL_SOLO) ||
		(nGameType == MMATCH_GAMETYPE_SKILLMAP) ||
		(nGameType == MMATCH_GAMETYPE_DROPMAGIC) ||
		(nGameType == MMATCH_GAMETYPE_GUNGAME) ||
		(nGameType == MMATCH_GAMETYPE_VAMPIRE) ||
		(nGameType == MMATCH_GAMETYPE_MODE_STAFF) ||
		(nGameType == MMATCH_GAMETYPE_CLASSIC_SOLO) ||
		(nGameType == MMATCH_GAMETYPE_TURBO))
	{
		return true;
	}
	return false;
}
inline bool MBaseGameTypeCatalogue::IsTeamLimitTime(MMATCH_GAMETYPE nGameType)
{
	//Custom: Rule Custom Game
	if ((nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_DUEL) ||
		(nGameType == MMATCH_GAMETYPE_PAINTBALL_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_ASSASSINATE) ||
		(nGameType == MMATCH_GAMETYPE_TEAM_TRAINING) ||
		(nGameType == MMATCH_GAMETYPE_INFECTED)||
		(nGameType == MMATCH_GAMETYPE_SNIPERMODE) ||
		(nGameType == MMATCH_GAMETYPE_CLASSIC_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_GHOST) ||				// Custom: Ghost Mode - Desperate
		(nGameType == MMATCH_GAMETYPE_SPY))
	{
		return true;
	}
	return false;
}

inline bool MBaseGameTypeCatalogue::IsWaitForRoundEnd(MMATCH_GAMETYPE nGameType)
{
	//Custom: Rule Custom Game
	if ((nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_DUEL) ||
		(nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_ASSASSINATE) ||
		(nGameType == MMATCH_GAMETYPE_DUELTOURNAMENT) ||
		(nGameType == MMATCH_GAMETYPE_TEAM_TRAINING) ||
		(nGameType == MMATCH_GAMETYPE_INFECTED)||
		(nGameType == MMATCH_GAMETYPE_SNIPERMODE) ||
		(nGameType == MMATCH_GAMETYPE_CLASSIC_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_GHOST) ||				// Custom: Ghost Mode - Desperate
		(nGameType == MMATCH_GAMETYPE_SPY))
	{
		return true;
	}
	return false;
}

//Custom: Rule Custom Game
inline bool MBaseGameTypeCatalogue::IsQuestDerived(MMATCH_GAMETYPE nGameType)
{
	if ( (nGameType == MMATCH_GAMETYPE_SURVIVAL) ||(nGameType == MMATCH_GAMETYPE_QUEST) )
	{
		return true;
	}

	return false;
}

inline bool MBaseGameTypeCatalogue::IsQuestOnly(MMATCH_GAMETYPE nGameType)
{
	return nGameType == MMATCH_GAMETYPE_QUEST;
}

inline bool MBaseGameTypeCatalogue::IsSurvivalOnly(MMATCH_GAMETYPE nGameType)
{
	return nGameType == MMATCH_GAMETYPE_SURVIVAL;
}

inline bool MBaseGameTypeCatalogue::IsQuestChallengeOnly(MMATCH_GAMETYPE nGameType)
{
	if ((nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE))
		return true;

	return false;
}

inline const char* MBaseGameTypeCatalogue::GetGameTypeStr(MMATCH_GAMETYPE nGameType)
{
	return m_GameTypeInfo[nGameType].szGameTypeStr;
}

inline void MBaseGameTypeCatalogue::SetGameTypeStr(MMATCH_GAMETYPE nGameType, const char* szName)
{
	strcpy( m_GameTypeInfo[nGameType].szGameTypeStr, szName) ;
}

bool MBaseGameTypeCatalogue::IsCorrectGameType(const int nGameTypeID)
{
	if ((nGameTypeID < 0) || (nGameTypeID >= MMATCH_GAMETYPE_MAX)) return false;
	return true;
}

inline MMatchGameTypeInfo* MBaseGameTypeCatalogue::GetInfo(MMATCH_GAMETYPE nGameType)
{
	return &m_GameTypeInfo[nGameType];
}

inline bool MBaseGameTypeCatalogue::IsWorldItemSpawnEnable(MMATCH_GAMETYPE nGameType)
{
	if ( (nGameType == MMATCH_GAMETYPE_SURVIVAL) || (nGameType == MMATCH_GAMETYPE_QUEST) || (nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE))
	{
		return false;
	}

	return true;

}
inline bool MBaseGameTypeCatalogue::IsTeamExtremeGame(MMATCH_GAMETYPE nGameType)
{

	if ((nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM2) ||
		(nGameType == MMATCH_GAMETYPE_CTF))
	{
		return true;
	}
	return false;
}
inline bool MBaseGameTypeCatalogue::IsSpyGame(MMATCH_GAMETYPE nGameType)
{
	if (nGameType == MMATCH_GAMETYPE_SPY)
		return true;

	return false;
}
inline bool MBaseGameTypeCatalogue::IsMapSkill(MMATCH_GAMETYPE nGameType)
{
	if (nGameType == MMATCH_GAMETYPE_SKILLMAP)
		return true;

	return false;
}
inline bool MBaseGameTypeCatalogue::IsMagicBox(MMATCH_GAMETYPE nGameType)
{
	if (nGameType == MMATCH_GAMETYPE_DROPMAGIC)
		return true;

	return false;
}
inline bool MBaseGameTypeCatalogue::IsPaintballTeam(MMATCH_GAMETYPE nGameType)
{
	if (nGameType == MMATCH_GAMETYPE_PAINTBALL_TEAM)
		return true;

	return false;
}
inline bool MBaseGameTypeCatalogue::IsPaintballSolo(MMATCH_GAMETYPE nGameType)
{
	if (nGameType == MMATCH_GAMETYPE_PAINTBALL_SOLO)
		return true;

	return false;
}
#endif