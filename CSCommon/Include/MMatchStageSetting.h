#ifndef _MMATCHSTAGESETTING_H
#define _MMATCHSTAGESETTING_H

#include <list>
using namespace std;
#include "MUID.h"
#include "MMatchGlobal.h"
#include "MMatchGameType.h"
#include "MMatchObject.h"
#include "MMatchMap.h"

class MMatchObject;
class MMatchStage;

#define MMATCH_TEAM_MAX_COUNT		2

#define MMATCH_SPECTATOR_STR		"SPECTATOR"
#define MMATCH_TEAM1_NAME_STR		"RED TEAM"
#define MMATCH_TEAM2_NAME_STR		"BLUE TEAM"


inline const char* GetTeamNameStr(MMatchTeam nTeam)
{
	switch (nTeam)
	{
	case MMT_SPECTATOR:
		return MMATCH_SPECTATOR_STR;
	case MMT_RED:
		return MMATCH_TEAM1_NAME_STR;
	case MMT_BLUE:
		return MMATCH_TEAM2_NAME_STR;
	default:
		return "";
	}
	return "";
}


enum STAGE_STATE {
	STAGE_STATE_STANDBY = 0,
	STAGE_STATE_COUNTDOWN,
	STAGE_STATE_RUN,
	STAGE_STATE_CLOSE,

	STAGE_STATE_COUNT
};

#define MSTAGENODE_FLAG_FORCEDENTRY_ENABLED		1		// 난입
#define MSTAGENODE_FLAG_PRIVATE					2		// 비밀방
#define MSTAGENODE_FLAG_LIMITLEVEL				4		// 레벨제한


enum class NetcodeType : u8
{
	ServerBased,
	P2PAntilead,
	P2PLead,
};

enum ROOM_MOD
{
	MMOD_LEAD,
	MMOD_FASTRELOAD,
	MMOD_EXPLOSION,
	MMOD_GRAVITY,
	MMOD_SPEED,
	MMOD_JUMP,
	MMOD_FLIP,
	MMOD_FPS,
	MMOD_MASSIVES,
	MMOD_NINJA,
	MMOD_VANILLA,
	MMOD_MELEEONLY,
	MMOD_EVENTMODE,
	MMOD_SHOTGUNONLY,
	MMOD_INFINITE_AMMO,
	MMOD_DMG,
	MMOD_WIREFRAME,
	MMOD_NC,
	MMOD_NR,
	MMOD_NS,
	MMOD_NG,
	MMOD_NI,
	MMOD_ROLLTHEDICE,
	MMOD_REVONLY,
	MMOD_GLAD,
	MMOD_WIWEAPON,
	MMOD_PAINT,
	MMOD_IS,
	MMOD_NODAMAGE,
	MMOD_FORCEHP,
	MMOD_FORCEAP,
	MMOD_SNIPERONLY,
	MMOD_FORCEMELEEDELAY,
	MMOD_FORCEMELEEDMG,
	MMOD_NAME,
	MMOD_END
};

// for new UI: gom mod thanh nhom de hien icon
enum ROOM_MOD_GROUP
{
	RMG_WEAPON = 0,		// han che vu khi
	RMG_PHYSIC,			// the chat, di chuyen
	RMG_HPSET,			// ep HP/AP ve gia tri co dinh cho ca phong
	RMG_DAMAGE,			// thay doi sat thuong
	RMG_GEAR,			// han che trang bi
	RMG_ETC,			// con lai

	RMG_COUNT
};

struct MMatchRoomModifier
{
	ROOM_MOD mod;
	int nValue;
};

ROOM_MOD_GROUP	GetRoomModGroup(ROOM_MOD mod);
unsigned int	GetRoomModGroupFlags(const vector<MMatchRoomModifier>& mods);
void			BuildRoomModText(const vector<MMatchRoomModifier>& mods, char* szOut, int nMaxLen);

struct MSTAGE_SETTING_NODE {
	MUID				uidStage;
	char				szMapName[MAPNAME_LENGTH];	// 맵이름
	char				nMapIndex;					// 맵인덱스
	MMATCH_GAMETYPE		nGameType;					// 게임타입
	int					nRoundMax;					// 라운드
	int					nLimitTime;					// 제한시간(1 - 1분)
	int					nLimitLevel;				// 제한레벨
	int					nMaxPlayers;				// 최대인원
	bool				bTeamKillEnabled;			// 팀킬여부
	bool				bTeamWinThePoint;			// 선승제 여부
	bool				bForcedEntryEnabled;		// 게임중 참가 여부
	int					nStageType;                 // Custom: png mode
	bool				bPrivate;	// for new UI
	int					nRoomModCount;
	MMatchRoomModifier	RoomMods[MMOD_END];

	NetcodeType         Netcode;

#ifdef _LOBBYSET
	float	m_nGravityValue, m_nSpeedValue, m_nJumpValue;
	int		m_nDMGType;
	//bool   m_bSpecialWeapon;
#endif

	// 추가됨
	bool				bAutoTeamBalancing;			// 오토팀밸런스 - 팀플게임에서만 사용
#ifdef _ROTATION
	bool                bTeamAlternateSpawns;
#endif

#ifdef _SPEC 1
	bool				bObserverEnabled;			// 관전모드 여부
#endif

#ifdef _LIGHTURNOFF
	bool                bLightMapTurn;
#endif

	bool                bNokStyle;
	bool                bXmas;
	bool                bFog;
	bool                bHall;
	// 릴레이맵
	int					nPingLimitLow;
	int					nPingLimitHigh;
	int					nPingStrikes;
	bool				nPingType;

	bool				bIsRelayMap;				// 릴레이맵 유무
	bool				bIsStartRelayMap;			// 릴레이맵 시작한 유무
#ifdef _BOXLEAD
	bool				bIsLeading;
#endif
	int					nRelayMapListCount;			// 릴레이맵 리스트 갯수
	RelayMap			MapList[MAX_RELAYMAP_LIST_COUNT];	// 릴레이맵 리스트
	RELAY_MAP_TYPE		nRelayMapType;				// 릴레이맵 타입
	RELAY_MAP_REPEAT_COUNT	nRelayMapRepeatCount;	// 릴레이맵 회차	
	char				szStageBestTime[6]; // Custom: Stage Best Time
#ifdef _VOTESETTING
	bool				bVoteEnabled;				// 투표가능 여부
#endif

	//Custom: Lightmaps Time Code
	int					lightMapIndex;
};

// 방 처음 만들었을때 스테이지 세팅 초기값
#define MMATCH_DEFAULT_STAGESETTING_MAPNAME			"Mansion"

#define MMATCH_DEFAULT_STAGESETTING_GAMETYPE			MMATCH_GAMETYPE_DEATHMATCH_SOLO
#define MMATCH_DEFAULT_STAGESETTING_ROUNDMAX			50		// 50라운드
#define MMATCH_DEFAULT_STAGESETTING_LIMITTIME			(30 * 60 * 1000)
#define MMATCH_DEFAULT_STAGESETTING_LIMITLEVEL			0		// 무제한
#define MMATCH_DEFAULT_STAGESETTING_MAXPLAYERS			8		// 8명
#define MMATCH_DEFAULT_STAGESETTING_TEAMKILL			false	// 팀킬불가
#define MMATCH_DEFAULT_STAGESETTING_TEAM_WINTHEPOINT	false	// 선승제 여부
#define MMATCH_DEFAULT_STAGESETTING_FORCEDENTRY			true	// 난입가능
#define MMATCH_DEFAULT_STAGESETTING_AUTOTEAMBALANCING	true	// 오토팀밸런스

#ifdef _ROTATION
#define MMATCH_DEFAULT_STAGESETTING_ROTATION	false	
#endif
#ifdef _SPEC 1
#define MMATCH_DEFAULT_STAGESETTING_SPEC	    false	
#endif
#define MMATCH_DEFAULT_STAGESETTING_NOKSTYLE	false	
#define MMATCH_DEFAULT_STAGESETTING_FOG	        false	
#define MMATCH_DEFAULT_STAGESETTING_RAIN	    false
#define MMATCH_DEFAULT_STAGESETTING_HALL	    false
#ifdef _LIGHTURNOFF
#define MMATCH_DEFAULT_STAGESETTING_FORCELIGHT	true
#endif

#define STAGESETTING_LIMITTIME_UNLIMITED				0		// 제한시간이 무제한은 0


struct MSTAGE_CHAR_SETTING_NODE {
	MUID	uidChar;
	int		nTeam;
	MMatchObjectStageState	nState;
	MSTAGE_CHAR_SETTING_NODE() : uidChar(MUID(0, 0)), nTeam(0), nState(MOSS_NONREADY) {	}
};
class MStageCharSettingList : public list<MSTAGE_CHAR_SETTING_NODE*> {
public:
	void DeleteAll() {
		for (iterator i = begin(); i != end(); i++) {
			delete (*i);
		}
		clear();
	}
};

class MMatchStageSetting {
protected:
	MProtectValue<MSTAGE_SETTING_NODE>	m_StageSetting;
	MUID					m_uidMaster;	// 방장
	STAGE_STATE				m_nStageState;	// 현재 State (게임중,대기중,..)

	vector<MMatchRoomModifier>	m_RoomMods;		// Custom: Room modifiers

	bool					m_bIsCheckTicket;
	DWORD					m_dwTicketItemID;
public:
	MStageCharSettingList	m_CharSettingList;
public:
	MMatchStageSetting();
	virtual ~MMatchStageSetting();
	void Clear();
	void SetDefault();
	unsigned long GetChecksum();
	MSTAGE_CHAR_SETTING_NODE* FindCharSetting(const MUID& uid);

	// Get
	const char* GetMapName() { return m_StageSetting.Ref().szMapName; }
	int							GetMapIndex() { return m_StageSetting.Ref().nMapIndex; }
	int							GetRoundMax() { return m_StageSetting.Ref().nRoundMax; }
	int							GetLimitTime() { return m_StageSetting.Ref().nLimitTime; }
	int							GetLimitLevel() { return m_StageSetting.Ref().nLimitLevel; }
	MUID						GetMasterUID() { return m_uidMaster; }
	STAGE_STATE					GetStageState() { return m_nStageState; }
	int							GetStageType() { return m_StageSetting.Ref().nStageType; } // Custom: Stage Interface mode png
	MMATCH_GAMETYPE				GetGameType() { return m_StageSetting.Ref().nGameType; }
	int							GetMaxPlayers() { return m_StageSetting.Ref().nMaxPlayers; }
	bool						GetForcedEntry() { return m_StageSetting.Ref().bForcedEntryEnabled; }
	bool						IsPrivate() { return m_StageSetting.Ref().bPrivate; }	// for new UI
	bool						GetAutoTeamBalancing() { return m_StageSetting.Ref().bAutoTeamBalancing; }
#ifdef _ROTATION
	bool						GetTeamRotation() { return m_StageSetting.Ref().bTeamAlternateSpawns; }
#endif
#ifdef _SPEC 1
	bool						GetSpecMode() { return m_StageSetting.Ref().bObserverEnabled; }
#endif
	bool						GetNoKStyle() { return m_StageSetting.Ref().bNokStyle; }
	bool						GetXmas() { return m_StageSetting.Ref().bXmas; }
	bool						GetFog() { return m_StageSetting.Ref().bFog; }
	bool						GetHalloween() { return m_StageSetting.Ref().bHall; }
#ifdef _LIGHTURNOFF
	bool						GetLightMapStage() { return m_StageSetting.Ref().bLightMapTurn; }
#endif
	const MSTAGE_SETTING_NODE* GetStageSetting() { return &m_StageSetting.Ref(); }
	const MMatchGameTypeInfo* GetCurrGameTypeInfo();
	DWORD						GetTicketItemID() { return m_dwTicketItemID; }
	int							GetRelayMapListCount() { return m_StageSetting.Ref().nRelayMapListCount; }
	const RelayMap* GetRelayMapList() { return m_StageSetting.Ref().MapList; }
	RELAY_MAP_TYPE				GetRelayMapType() { return m_StageSetting.Ref().nRelayMapType; }
	RELAY_MAP_REPEAT_COUNT		GetRelayMapRepeatCount() { return m_StageSetting.Ref().nRelayMapRepeatCount; }

	bool						IsRelayMap() { return m_StageSetting.Ref().bIsRelayMap; }
	bool						IsStartRelayMap() { return m_StageSetting.Ref().bIsStartRelayMap; }
#ifdef _BOXLEAD
	bool						IsLead()				{ return m_StageSetting.Ref().bIsLeading; }
#endif

	//Custom: Lightmaps Time Code
	int const					GetLightMapIndex() { return m_StageSetting.Ref().lightMapIndex; }

#ifdef _TYPENET
	auto GetNetcode() const { return m_StageSetting.Ref().Netcode; }
#endif

	bool						IsUserExempt(MMatchUserGradeID uGrade); // Custom: Staff Bypass to Room tags.
	bool						IsModifierUsed(ROOM_MOD nRoomMod); // Custom: Room Modifiers
	int							GetModifierCount() { return m_RoomMods.size(); } // Custom: Room Modifiers
	static const char* GetModifierString(ROOM_MOD nRoomMod);	// for new UI
	int							GetModifierValue(ROOM_MOD nRoomMod);
	static bool					IsModifierPercentage(ROOM_MOD nRoomMod);

	// Set
	void SetMasterUID(const MUID& uid) { m_uidMaster = uid; }
	void SetMapName(char* pszName);

	//Custom: Lightmaps Time Code
	void SetMapLightMapIndex(int const& lightIndex);

	void SetMapIndex(int nMapIndex);
	void SetRoundMax(int nRound) { MEMBER_SET_CHECKCRC(m_StageSetting, nRoundMax, nRound); }
	void SetLimitTime(int nTime) { MEMBER_SET_CHECKCRC(m_StageSetting, nLimitTime, nTime); }
	void SetGameType(MMATCH_GAMETYPE type) { MEMBER_SET_CHECKCRC(m_StageSetting, nGameType, type); }
	void SetStageState(STAGE_STATE nState) { m_nStageState = nState; }
	void SetTeamWinThePoint(bool bValue) { MEMBER_SET_CHECKCRC(m_StageSetting, bTeamWinThePoint, bValue); }
	void SetAutoTeamBalancing(bool bValue) { MEMBER_SET_CHECKCRC(m_StageSetting, bAutoTeamBalancing, bValue); }
	void SetIsCheckTicket(bool bIsCheck) { m_bIsCheckTicket = bIsCheck; }
	void SetTicketItemID(DWORD dwTicketItemID) { m_dwTicketItemID = dwTicketItemID; }
	void SetMaxPlayers(int nValue) { MEMBER_SET_CHECKCRC(m_StageSetting, nMaxPlayers, nValue); }
	void SetPrivate(bool bValue) { MEMBER_SET_CHECKCRC(m_StageSetting, bPrivate, bValue); }	// for new UI
#ifdef _TYPENET
	void SetNetcode(NetcodeType NewNetcode) { MEMBER_SET_CHECKCRC(m_StageSetting, Netcode, NewNetcode);; }
#endif

#ifdef _ROTATION
	void SetRoBase(int nValue) { MEMBER_SET_CHECKCRC(m_StageSetting, bTeamAlternateSpawns, nValue); }
#endif
#ifdef _BOXLEAD
	void SetLead(bool bValue) { MEMBER_SET_CHECKCRC(m_StageSetting, bIsLeading, bValue); }
#endif

#ifdef _SPEC 1
	void SetSpecMode(int nValue) { MEMBER_SET_CHECKCRC(m_StageSetting, bObserverEnabled, nValue); }
#endif

#ifdef _LOBBYSET
	float GetGravityValue() { return m_StageSetting.Ref().m_nGravityValue; }
	void SetGravityValue(float nValue) { MEMBER_SET_CHECKCRC(m_StageSetting, m_nGravityValue, nValue); }

	float GetJumpValue() { return m_StageSetting.Ref().m_nJumpValue; }
	void SetJumpValue(float nValue) { MEMBER_SET_CHECKCRC(m_StageSetting, m_nJumpValue, nValue); }

	float GetSpeedValue() { return m_StageSetting.Ref().m_nSpeedValue; }
	void SetSpeedValue(float nValue) { MEMBER_SET_CHECKCRC(m_StageSetting, m_nSpeedValue, nValue); }

	int	GetDMGType() { return m_StageSetting.Ref().m_nDMGType; }
	void SetDMGType(int nValue) { MEMBER_SET_CHECKCRC(m_StageSetting, m_nDMGType, nValue); }
#endif

	void SetIsRelayMap(bool bValue) { MEMBER_SET_CHECKCRC(m_StageSetting, bIsRelayMap, bValue); }
	void SetIsStartRelayMap(bool bValue) { MEMBER_SET_CHECKCRC(m_StageSetting, bIsStartRelayMap, bValue); }

	void SetRelayMapListCount(int nValue) { MEMBER_SET_CHECKCRC(m_StageSetting, nRelayMapListCount, nValue); }
	void SetRelayMapList(RelayMap* pValue);
	void SetRelayMapType(RELAY_MAP_TYPE bValue) { MEMBER_SET_CHECKCRC(m_StageSetting, nRelayMapType, bValue); }
	void SetRelayMapRepeatCount(RELAY_MAP_REPEAT_COUNT bValue) { MEMBER_SET_CHECKCRC(m_StageSetting, nRelayMapRepeatCount, bValue); }

	void UpdateStageSetting(MSTAGE_SETTING_NODE* pSetting);
	void UpdateCharSetting(const MUID& uid, unsigned int nTeam, MMatchObjectStageState nStageState);

	void ResetCharSetting() { m_CharSettingList.DeleteAll(); }
	bool IsTeamPlay();
	bool IsSoloPlay();
	bool IsWaitforRoundEnd();
	bool IsQuestDrived();
	bool IsTeamWinThePoint() { return m_StageSetting.Ref().bTeamWinThePoint; }		///< 선승제 여부

	bool	IsCheckTicket() { return m_bIsCheckTicket; }
	void	SetStageBestTime(const char* szValue);
	const char* GetStageBestTime() { return m_StageSetting.Ref().szStageBestTime; }

	void InsertRoomModifier(ROOM_MOD mod, int nValue = 0); // Custom: Room Modifiers
	void StoreRoomModsToNode();
	void SetRoomModsFromArray(const MMatchRoomModifier* pMods, int nCount);
	void LoadRoomModsFromNode();

	// 핵방어용 클라에서만 호출
	void AntiHack_ShiftHeapPos() { m_StageSetting.ShiftHeapPos(); }
	void AntiHack_CheckCrc() { m_StageSetting.CheckCrc(); }
#ifdef _SPYMODE
public:
	vector<int>		m_vecSpyBanMapList;

	void ExcludeSpyMap(int nMapID);
	void IncludeSpyMap(int nMapID);

	bool IsExcludedSpyMap(int nMapID);
	bool IsIncludedSpyMap(int nMapID);

	void UnbanAllSpyMap();
#endif
};












#endif