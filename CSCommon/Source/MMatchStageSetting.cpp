#include "stdafx.h"
#include "MMatchStageSetting.h"
#include "MMatchServer.h"
#include "MMatchObject.h"
#include "MMatchSpyMap.h"

MMatchStageSetting::MMatchStageSetting()
{
	Clear();
}


MMatchStageSetting::~MMatchStageSetting()
{
	Clear();
}

unsigned long MMatchStageSetting::GetChecksum()
{
	return (m_StageSetting.Ref().nMapIndex + m_StageSetting.Ref().nGameType + m_StageSetting.Ref().nMaxPlayers);
}

void MMatchStageSetting::SetDefault()
{
	m_StageSetting.Ref().nMapIndex = 0;
	strcpy(m_StageSetting.Ref().szMapName, MMATCH_DEFAULT_STAGESETTING_MAPNAME);
	m_StageSetting.Ref().nGameType = MMATCH_DEFAULT_STAGESETTING_GAMETYPE;
	m_StageSetting.Ref().bTeamKillEnabled = MMATCH_DEFAULT_STAGESETTING_TEAMKILL;
	m_StageSetting.Ref().bTeamWinThePoint = MMATCH_DEFAULT_STAGESETTING_TEAM_WINTHEPOINT;
	m_StageSetting.Ref().bForcedEntryEnabled = MMATCH_DEFAULT_STAGESETTING_FORCEDENTRY;
	m_StageSetting.Ref().nLimitTime = MMATCH_DEFAULT_STAGESETTING_LIMITTIME;
	m_StageSetting.Ref().nMaxPlayers = MMATCH_DEFAULT_STAGESETTING_MAXPLAYERS;
	m_StageSetting.Ref().nRoundMax = MMATCH_DEFAULT_STAGESETTING_ROUNDMAX;
	m_StageSetting.Ref().nLimitLevel = MMATCH_DEFAULT_STAGESETTING_LIMITLEVEL;
	m_StageSetting.Ref().bAutoTeamBalancing = MMATCH_DEFAULT_STAGESETTING_AUTOTEAMBALANCING;
	m_StageSetting.Ref().uidStage = MUID(0,0);
	m_StageSetting.Ref().bIsRelayMap = false;
	m_StageSetting.Ref().bIsStartRelayMap = false;
	m_StageSetting.Ref().nRelayMapListCount = 0;
#ifdef _BOXLEAD
	m_StageSetting.Ref().bIsLeading = false;
#endif
	m_StageSetting.Ref().nPingLimitHigh = 0;
	m_StageSetting.Ref().nPingLimitLow = 0;
	m_StageSetting.Ref().nPingStrikes = 0;
	m_StageSetting.Ref().nPingType = true;
	for (int i=0; i<MAX_RELAYMAP_LIST_COUNT; ++i)
		m_StageSetting.Ref().MapList[i].nMapID = -1;
	m_StageSetting.Ref().nRelayMapType = RELAY_MAP_TURN;
	m_StageSetting.Ref().nRelayMapRepeatCount = RELAY_MAP_3REPEAT;
	strcpy(m_StageSetting.Ref().szStageBestTime, "");
#ifdef _VOTESETTING
	m_StageSetting.Ref().bVoteEnabled = true;
#endif

#ifdef _SPEC 1
	m_StageSetting.Ref().bObserverEnabled = MMATCH_DEFAULT_STAGESETTING_SPEC;
#endif

#ifdef _TYPENET
	m_StageSetting.Ref().Netcode = NetcodeType::P2PAntilead;
#endif

#ifdef _LOBBYSET
	m_StageSetting.Ref().m_nGravityValue = 2500.f;
	m_StageSetting.Ref().m_nJumpValue = 900.f;
	m_StageSetting.Ref().m_nSpeedValue = 630.f;
	m_StageSetting.Ref().m_nDMGType = 0;
#endif
	//Custom: Lightmaps Time Code
	m_StageSetting.Ref().lightMapIndex = 0;
#ifdef _ROTATION
	m_StageSetting.Ref().bTeamAlternateSpawns = MMATCH_DEFAULT_STAGESETTING_ROTATION;
#endif

#ifdef _LIGHTURNOFF
	m_StageSetting.Ref().bLightMapTurn = MMATCH_DEFAULT_STAGESETTING_FORCELIGHT;
#endif
	m_StageSetting.Ref().bNokStyle = MMATCH_DEFAULT_STAGESETTING_NOKSTYLE;
	m_StageSetting.Ref().bXmas = MMATCH_DEFAULT_STAGESETTING_FOG;
	m_StageSetting.Ref().bFog = MMATCH_DEFAULT_STAGESETTING_RAIN;
	m_StageSetting.Ref().bHall = MMATCH_DEFAULT_STAGESETTING_HALL;
	m_StageSetting.Ref().bPrivate = false;
	m_StageSetting.Ref().nRoomModCount = 0;
	memset(m_StageSetting.Ref().RoomMods, 0, sizeof(m_StageSetting.Ref().RoomMods));
	m_StageSetting.MakeCrc();
}

void MMatchStageSetting::SetMapName(char* pszName)
{
	if (pszName == NULL) return;
	m_StageSetting.CheckCrc();
	if (strlen(pszName) < MAPNAME_LENGTH)
	{
		memset(m_StageSetting.Ref().szMapName, 0, MAPNAME_LENGTH);

		strcpy(m_StageSetting.Ref().szMapName, pszName); 

		// MapIndex
		m_StageSetting.Ref().nMapIndex = 0;
		for (int i = 0; i < MMATCH_MAP_MAX; i++)
		{
			if (!stricmp(MGetMapDescMgr()->GetMapName(i), pszName))
			{
				m_StageSetting.Ref().nMapIndex = MGetMapDescMgr()->GetMapID(i);
				if (MGetMapDescMgr()->IsLightMap(MGetMapDescMgr()->GetMapID(i)))
				{
					//mlog("Founded LightMap\n");
					m_StageSetting.Ref().bLightMapTurn = true;
				}
				else
				{
					//mlog("Not founed LightMap\n");
					m_StageSetting.Ref().bLightMapTurn = false;
				}
				break;
			}
		}
		//mlog("ForceLights %d\n", m_StageSetting.Ref().bLightMapTurn);
	}
	m_StageSetting.MakeCrc();
}

void MMatchStageSetting::SetStageBestTime(const char* szBestTime)
{
	if (szBestTime == NULL) return;
	m_StageSetting.CheckCrc();
	if (strlen(szBestTime) < 3)
	{
		memset(m_StageSetting.Ref().szStageBestTime, 0, strlen(szBestTime));

		strcpy(m_StageSetting.Ref().szStageBestTime, szBestTime); // -- by SungE 2007-04-02
	}
	m_StageSetting.MakeCrc();
}

//Custom: Lightmaps Time Code
void MMatchStageSetting::SetMapLightMapIndex(int const& lightIndex)
{
	m_StageSetting.CheckCrc();

	m_StageSetting.Ref().lightMapIndex = lightIndex;

	m_StageSetting.MakeCrc();
}

void MMatchStageSetting::SetMapIndex(int nMapIndex)
{
	m_StageSetting.CheckCrc();
	m_StageSetting.Ref().nMapIndex = nMapIndex; 

	// MapName까지 함께 세팅해준다.
	if ( MGetMapDescMgr()->MIsCorrectMap(nMapIndex))
	{
		strcpy(m_StageSetting.Ref().szMapName, MGetMapDescMgr()->GetMapName(nMapIndex)); 
	}
	m_StageSetting.MakeCrc();
}

void MMatchStageSetting::Clear()
{
	SetDefault();
	m_CharSettingList.DeleteAll();
	m_uidMaster = MUID(0,0);
	m_nStageState = STAGE_STATE_STANDBY;
	m_bIsCheckTicket = false;
	m_RoomMods.clear(); // Custom: Clear room modifiers
	m_vecSpyBanMapList.reserve(MMATCH_SPY_MAP_MAX - 1);
}

MSTAGE_CHAR_SETTING_NODE* MMatchStageSetting::FindCharSetting(const MUID& uid)
{
	for (MStageCharSettingList::iterator i=m_CharSettingList.begin();i!=m_CharSettingList.end();i++) {
		if (uid == (*i)->uidChar) return (*i);
	}
	return NULL;
}

bool MMatchStageSetting::IsTeamPlay()
{
	return MGetGameTypeMgr()->IsTeamGame(m_StageSetting.Ref().nGameType);
}
bool MMatchStageSetting::IsSoloPlay()
{
	return MGetGameTypeMgr()->IsSoloGame(m_StageSetting.Ref().nGameType);
}
bool MMatchStageSetting::IsWaitforRoundEnd()
{
	return MGetGameTypeMgr()->IsWaitForRoundEnd(m_StageSetting.Ref().nGameType);
}

bool MMatchStageSetting::IsQuestDrived()
{
	return MGetGameTypeMgr()->IsQuestDerived(m_StageSetting.Ref().nGameType);
}

void MMatchStageSetting::UpdateStageSetting(MSTAGE_SETTING_NODE* pSetting)
{
	// move to MMatchServer::OnStageSetting(...) - by SungE 2007-05-14
	//if( STAGE_BASIC_MAX_PLAYERCOUNT < pSetting->nMaxPlayers )
	//{
	//	if( QuestTestServer() && MGetGameTypeMgr()->IsQuestDerived(pSetting->nGameType) )
	//	{
	//		pSetting->nMaxPlayers = STAGE_QUEST_MAX_PLAYER;
	//	}
	//	else
	//	{
	//		pSetting->nMaxPlayers = STAGE_BASIC_MAX_PLAYERCOUNT;
	//	}
	//}

	m_StageSetting.Set_CheckCrc(*pSetting);
	/*m_StageSetting.CheckCrc();
	memcpy(&m_StageSetting.Ref(), pSetting, sizeof(MSTAGE_SETTING_NODE));
	m_StageSetting.MakeCrc();*/
}


void MMatchStageSetting::UpdateCharSetting(const MUID& uid, unsigned int nTeam, MMatchObjectStageState nStageState)
{
	MSTAGE_CHAR_SETTING_NODE* pNode = FindCharSetting(uid);
	if (pNode) {
		pNode->nTeam = nTeam;
		pNode->nState = nStageState;
	} else {
		MSTAGE_CHAR_SETTING_NODE* pNew = new MSTAGE_CHAR_SETTING_NODE;
		pNew->uidChar = uid;
		pNew->nTeam = nTeam;
		pNew->nState = nStageState;
		m_CharSettingList.push_back(pNew);
	}			
}



const MMatchGameTypeInfo* MMatchStageSetting::GetCurrGameTypeInfo()
{ 
	return MGetGameTypeMgr()->GetInfo(m_StageSetting.Ref().nGameType); 
}

void MMatchStageSetting::SetRelayMapList(RelayMap* pValue)
{ 
	m_StageSetting.CheckCrc();
	memcpy(m_StageSetting.Ref().MapList, pValue, sizeof(m_StageSetting.Ref().MapList));
	m_StageSetting.MakeCrc();
}

// Custom: Room Modifiers
void MMatchStageSetting::InsertRoomModifier( ROOM_MOD mod, int nValue )
{
	MMatchRoomModifier roomMod;
	roomMod.mod = mod;
	roomMod.nValue = nValue;

	m_RoomMods.push_back( roomMod );
}

void MMatchStageSetting::StoreRoomModsToNode()
{
	m_StageSetting.CheckCrc();

	int nCount = (int)m_RoomMods.size();
	if (nCount > MMOD_END) nCount = MMOD_END;

	for (int i = 0; i < nCount; ++i)
		m_StageSetting.Ref().RoomMods[i] = m_RoomMods[i];

	m_StageSetting.Ref().nRoomModCount = nCount;

	m_StageSetting.MakeCrc();
}

void MMatchStageSetting::LoadRoomModsFromNode()
{
	m_RoomMods.clear();

	int nCount = m_StageSetting.Ref().nRoomModCount;
	if (nCount < 0) nCount = 0;
	if (nCount > MMOD_END) nCount = MMOD_END;

	for (int i = 0; i < nCount; ++i)
		m_RoomMods.push_back(m_StageSetting.Ref().RoomMods[i]);
}

void MMatchStageSetting::SetRoomModsFromArray(const MMatchRoomModifier* pMods, int nCount)
{
	m_RoomMods.clear();

	if (pMods != NULL)
	{
		if (nCount < 0) nCount = 0;
		if (nCount > MMOD_END) nCount = MMOD_END;

		for (int i = 0; i < nCount; ++i)
			m_RoomMods.push_back(pMods[i]);
	}

	StoreRoomModsToNode();
}

bool MMatchStageSetting::IsUserExempt(MMatchUserGradeID uGrade)
{
	//TODO: Request a packet from the server confirming if the channel is event or not, I don't want to use string confirmation.
	//Since server stuff is in your hands I'll let you handle it unless you want me to. 
	switch(uGrade)
		{
			case 251:
			case 252:
			case 254:
			case 255:
			case 32:
				return true;
			default:
				return false;
		}
	//If a better idea occurs such as a #macro feel free to replace, I decided this would work for now. 
}

bool MMatchStageSetting::IsModifierUsed( ROOM_MOD nRoomMod )
{
	for (int i = 0; i < m_RoomMods.size(); ++i)
	{
		if (m_RoomMods[i].mod == nRoomMod)
			return true;
	}
	return false;
}

int MMatchStageSetting::GetModifierValue( ROOM_MOD nRoomMod )
{
	for( int i = 0; i < m_RoomMods.size(); ++i )
	{
		if( m_RoomMods[i].mod == nRoomMod )
			return m_RoomMods[i].nValue;
	}

	return -1;
}

bool MMatchStageSetting::IsModifierPercentage( ROOM_MOD nRoomMod )
{
	switch( nRoomMod )
	{
	case MMOD_DMG:
	case MMOD_FORCEHP:
	case MMOD_FORCEAP:
	case MMOD_FORCEMELEEDELAY:
	case MMOD_FORCEMELEEDMG:
		return false;
	}
	
	return true;
}

const char* MMatchStageSetting::GetModifierString(ROOM_MOD nRoomMod)
{
	switch( nRoomMod )
	{
		case MMOD_LEAD:
			return "Leading";
		case MMOD_FASTRELOAD:
			return "Fast Reload";
		case MMOD_EXPLOSION:
			return "Ignore Explosions / Flashbang / Smoke Grenade";
		case MMOD_GRAVITY:
			return "Gravity";
		case MMOD_SPEED:
			return "Speed";
		case MMOD_JUMP:
			return "Jump";
		case MMOD_FLIP:
			return "No Flips";
		case MMOD_FPS:
			return "FPS Mode";
		case MMOD_MASSIVES:
			return "No Massives";
		case MMOD_NINJA:
			return "Ninja";
		case MMOD_VANILLA:
			return "Vanilla Mode";
		case MMOD_EVENTMODE:
			return "No AP Mode";
		case MMOD_MELEEONLY:
			return "Melee Only";
		case MMOD_SHOTGUNONLY:
			return "Shotguns Only";
		case MMOD_REVONLY:
			return "Pistol/Revolver Only";
		case MMOD_SNIPERONLY:
			return "Sniper Only";
		case MMOD_FORCEMELEEDELAY:
			return "Force Melee Delay";
		case MMOD_FORCEMELEEDMG:
			return "Force Melee Damage";
		case MMOD_INFINITE_AMMO:
			return "Infinite Ammo";
		case MMOD_DMG:
			return "Damage Modifier";
		case MMOD_NC:
			return "No Clothes";
		case MMOD_NR:
			return "No Rings";
		case MMOD_NS:
			return "No Sword";
		case MMOD_NG:
			return "No Guns";
		case MMOD_NI:
			return "No Items";
		case MMOD_ROLLTHEDICE:
			return "Roll The Dice";
		case MMOD_GLAD:
			return "No Flips, No Guns, No Items";
		case MMOD_WIWEAPON:
			return "WorldItem weapon spawns";
		case MMOD_PAINT:
			return "Paint Mode";
		case MMOD_IS:
			return "Instant Respawn";
		case MMOD_NODAMAGE:
			return "No Damage";
		case MMOD_FORCEHP:
			return "Force HP";
		case MMOD_FORCEAP:
			return "Force AP";
		case MMOD_NAME:
			return "Show up Name";
		default:
			break;
	}

	return "Unknown";
}

// for new UI: mod nao thuoc nhom nao
ROOM_MOD_GROUP GetRoomModGroup(ROOM_MOD mod)
{
	switch (mod)
	{
	case MMOD_MELEEONLY:
	case MMOD_SHOTGUNONLY:
	case MMOD_SNIPERONLY:
	case MMOD_REVONLY:
	case MMOD_NG:
	case MMOD_NS:
	case MMOD_WIWEAPON:
	case MMOD_INFINITE_AMMO:
		return RMG_WEAPON;

	case MMOD_GRAVITY:
	case MMOD_SPEED:
	case MMOD_JUMP:
	case MMOD_FLIP:
	case MMOD_NINJA:
	case MMOD_MASSIVES:
	case MMOD_FASTRELOAD:
		return RMG_PHYSIC;

	case MMOD_FORCEHP:
	case MMOD_FORCEAP:
		return RMG_HPSET;

	case MMOD_DMG:
	case MMOD_NODAMAGE:
	case MMOD_FORCEMELEEDMG:
	case MMOD_FORCEMELEEDELAY:
		return RMG_DAMAGE;

	case MMOD_NC:
	case MMOD_NR:
	case MMOD_NI:
	case MMOD_VANILLA:
	case MMOD_GLAD:
		return RMG_GEAR;

	default:
		break;
	}

	return RMG_ETC;
}

unsigned int GetRoomModGroupFlags(const vector<MMatchRoomModifier>& mods)
{
	unsigned int nFlags = 0;

	for (int i = 0; i < (int)mods.size(); ++i)
		nFlags |= (1 << GetRoomModGroup(mods[i].mod));

	return nFlags;
}

// chuoi cho tooltip, moi mod mot dong
void BuildRoomModText(const vector<MMatchRoomModifier>& mods, char* szOut, int nMaxLen)
{
	if (szOut == NULL || nMaxLen <= 0) return;
	szOut[0] = '\0';

	for (int i = 0; i < (int)mods.size(); ++i)
	{
		char szLine[128];
		const char* szName = MMatchStageSetting::GetModifierString(mods[i].mod);

		if (mods[i].nValue >= 0)
		{
			if (MMatchStageSetting::IsModifierPercentage(mods[i].mod))
				sprintf(szLine, "%s %d%%", szName, mods[i].nValue);
			else
				sprintf(szLine, "%s %d", szName, mods[i].nValue);
		}
		else
		{
			sprintf(szLine, "%s", szName);
		}

		if ((int)(strlen(szOut) + strlen(szLine) + 2) >= nMaxLen) break;

		if (szOut[0] != '\0') strcat(szOut, "\n");
		strcat(szOut, szLine);
	}
}

#ifdef _SPYMODE
void MMatchStageSetting::ExcludeSpyMap(int nMapID)
{
	vector<int>::iterator itBegin = m_vecSpyBanMapList.begin(), itEnd = m_vecSpyBanMapList.end();
	vector<int>::iterator itFound = std::find(itBegin, itEnd, nMapID);

	if (itFound != itEnd) return;
	m_vecSpyBanMapList.push_back(nMapID);
}

void MMatchStageSetting::IncludeSpyMap(int nMapID)
{
	vector<int>::iterator itBegin = m_vecSpyBanMapList.begin(), itEnd = m_vecSpyBanMapList.end();
	vector<int>::iterator itFound = std::find(itBegin, itEnd, nMapID);

	if (itFound == itEnd) return;
	m_vecSpyBanMapList.erase(itFound);
}

bool MMatchStageSetting::IsExcludedSpyMap(int nMapID)
{
	vector<int>::iterator itBegin = m_vecSpyBanMapList.begin(), itEnd = m_vecSpyBanMapList.end();
	vector<int>::iterator itFound = std::find(itBegin, itEnd, nMapID);

	return itFound != itEnd;
}

bool MMatchStageSetting::IsIncludedSpyMap(int nMapID)
{
	return !IsExcludedSpyMap(nMapID);
}

void MMatchStageSetting::UnbanAllSpyMap()
{
	m_vecSpyBanMapList.clear();

}
#endif