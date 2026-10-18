#include "stdafx.h"
#include "MMatrix.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchItem.h"
#include "MAgentObject.h"
#include "MMatchNotify.h"
#include "Msg.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchTransDataType.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchShop.h"
#include "MMatchTransDataType.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MMatchStatus.h"
#include "MAsyncDBJob.h"
#include "MVoteDiscussImpl.h"
#include "MUtil.h"
#include "MMatchGameType.h"
#include "MMatchRuleBaseQuest.h"
#include "MMatchRuleQuest.h"
#include "MMatchRuleBerserker.h"
#include "MMatchRuleDuel.h"
#include "MCrashDump.h"
#include "MNewQuestScenario.h"
#include "MMatchRuleQuestChallenge.h"
#include "MAsyncDBJob_InsertGamePlayerLog.h"
#ifdef _MAGICBOX
#include "MMatchRuleDropGunGame.h"
#endif
#ifdef _SPYMODE
#include "MMatchSpyMode.h"
#include "MMatchRuleSpy.h"
#endif
#ifdef _PAINTMODE
#include "MMatchRuleSoloPaintball.h"
#endif
#include "RuleCustomGame/MMatchRuleCustomGame.h"
#ifdef _UPREWARDLEVEL
#include "MMatchLevelUpReward.h"
#endif

static bool StageShowInfo(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat);


MMatchStage* MMatchServer::FindStage(const MUID& uidStage)
{
	MMatchStageMap::iterator i = m_StageMap.find(uidStage);
	if(i==m_StageMap.end()) return NULL;

	MMatchStage* pStage = (*i).second;
	return pStage;
}

bool MMatchServer::StageAdd(MMatchChannel* pChannel, const char* pszStageName, bool bPrivate, const char* pszStagePassword, MUID* pAllocUID, bool bIsAllowNullChannel)
{
	// Å¬·£ÀEº pChannelÀÌ NULLÀÌ´Ù.

	MUID uidStage = m_StageMap.UseUID();
	
	MMatchStage* pStage= new MMatchStage;
#ifdef _ROOMHIDE
	if (pChannel && !pChannel->AddStage(pStage, bPrivate)) {
		delete pStage;
		return false;
	}
#else
	if (pChannel && !pChannel->AddStage(pStage)) {
		delete pStage;
		return false;
}
#endif 

	MMATCH_GAMETYPE GameType = MMATCH_GAMETYPE_DEFAULT;
	bool bIsCheckTicket = false;
	DWORD dwTicketID = 0;

	if ( (NULL != pChannel) && MGetServerConfig()->IsUseTicket()) {
		bIsCheckTicket = (pChannel != 0) && pChannel->IsUseTicket() && pChannel->IsTicketChannel();
		dwTicketID = pChannel->GetTicketItemID();

		// Æ¼ÄÏ ¼­¹ö¿¡¼­ »ç¼³ Ã¤³ÎÀº ¹«Á¶°Ç Æ¼ÄÏ °Ë»E- Æ¼ÄÏÀº Å¬·£ÀEÆ¼ÄÏ°Eµ¿ÀÏÇÏ´Ù.
		if ( pChannel->GetChannelType() == MCHANNEL_TYPE_USER) {
			bIsCheckTicket = true;
			dwTicketID = GetChannelMap()->GetClanChannelTicketInfo().m_dwTicketItemID;
		}
	}

	if (!pStage->Create( uidStage, pszStageName, bPrivate, pszStagePassword, bIsAllowNullChannel, GameType, bIsCheckTicket, dwTicketID) ) {
		if (pChannel) {
			pChannel->RemoveStage(pStage);
		}

		delete pStage;
		return false;
	}

	m_StageMap.Insert(uidStage, pStage);

	*pAllocUID = uidStage;

	return true;
}


bool MMatchServer::StageRemove(const MUID& uidStage, MMatchStageMap::iterator* pNextItor)
{
	MMatchStageMap::iterator i = m_StageMap.find(uidStage);
	if(i==m_StageMap.end()) {
		return false;
	}

	MMatchStage* pStage = (*i).second;

	MMatchChannel* pChannel = FindChannel(pStage->GetOwnerChannel());
	if (pChannel) {
		pChannel->RemoveStage(pStage);
	}

	pStage->Destroy();
	delete pStage;

	MMatchStageMap::iterator itorTemp = m_StageMap.erase(i);
	if (pNextItor) *pNextItor = itorTemp;

	return true;
}
bool MMatchServer::StageJoin(const MUID& uidPlayer, const MUID& uidStage, bool rejoin, MMatchTeam Team)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;

	if (pObj->GetStageUID() != MUID(0,0))
		StageLeave(pObj->GetUID());//, pObj->GetStageUID());

	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (pChannel == NULL) return false;
	if (pChannel->GetChannelType() == MCHANNEL_TYPE_DUELTOURNAMENT) return false;

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	int ret = ValidateStageJoin(uidPlayer, uidStage);
	if (ret != MOK) {
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, ret);
		return false;
	}
	pObj->OnStageJoin();

	// Cache Add
	MMatchObjectCacheBuilder CacheBuilder;
	CacheBuilder.AddObject(pObj);
	MCommand* pCmdCacheAdd = CacheBuilder.GetResultCmd(MATCHCACHEMODE_ADD, this);
	RouteToStage(pStage->GetUID(), pCmdCacheAdd);

	// Join
	pStage->AddObject(uidPlayer, pObj);
		// ÀÓ½ÃÄÚµE.. Àß¸øµÈ Å¬·£ID ¿Â´Ù¸EÃ¼Å©ÇÏ¿© Àâ±âÀ§ÇÔ...20090224 by kammir
	if(pObj->GetCharInfo()->m_ClanInfo.GetClanID() >= 9000000)
		LOG(LOG_FILE, "[UpdateCharClanInfo()] %s's ClanID:%d.", pObj->GetAccountName(), pObj->GetCharInfo()->m_ClanInfo.GetClanID());

	pObj->SetStageUID(uidStage);
	pObj->SetStageState(MOSS_NONREADY);

#ifdef _FORCE_EVENT_TEAM
	if (pChannel->IsUseEvent())
	{
		if (IsAdminGrade(pObj))
			pObj->SetTeam(MMT_RED);
		else
			pObj->SetTeam(MMT_BLUE);
	}
	else
#endif

	pObj->SetTeam(pStage->GetRecommandedTeam());

	if (rejoin == true && pStage->GetStageType() != MST_LADDER && pStage->GetStageType() != MST_LADDERWARS) rejoin = false;
	if (rejoin == false)
	{
		// Cast Join
		MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_JOIN), MUID(0, 0), m_This);
		pNew->AddParameter(new MCommandParameterUID(uidPlayer));
		pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
		pNew->AddParameter(new MCommandParameterUInt(pStage->GetIndex() + 1));
		pNew->AddParameter(new MCommandParameterString((char*)pStage->GetName()));

		if (pStage->GetState() == STAGE_STATE_STANDBY)  RouteToStage(pStage->GetUID(), pNew);
		else											RouteToListener(pObj, pNew);
	}
	else
	{
		MCommand* pCmd = CreateCommand(MC_MATCH_LADDER_PREPARE, uidPlayer);
		pCmd->AddParameter(new MCmdParamUID(uidStage));
		pCmd->AddParameter(new MCmdParamInt((int)pObj->GetTeam()));
		RouteToListener(pObj, pCmd);
	}


	// Cache Update
	CacheBuilder.Reset();
	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pScanObj = (MMatchObject*)GetObject(uidObj);
		if (pScanObj) {
			CacheBuilder.AddObject(pScanObj);
		} else {
			LOG(LOG_PROG, "MMatchServer::StageJoin - Invalid ObjectMUID(%u:%u) exist in Stage(%s)\n",
				uidObj.High, uidObj.Low, pStage->GetName());
			pStage->RemoveObject(uidObj);
			return false;
		}
	}
    MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
	RouteToListener(pObj, pCmdCacheUpdate);


	if (rejoin == false)
	{
		// Cast Master(¹æÀå)
		MUID uidMaster = pStage->GetMasterUID();
		MCommand* pMasterCmd = CreateCommand(MC_MATCH_STAGE_MASTER, MUID(0, 0));
		pMasterCmd->AddParameter(new MCommandParameterUID(uidStage));
		pMasterCmd->AddParameter(new MCommandParameterUID(uidMaster));
		RouteToListener(pObj, pMasterCmd);
	}


#ifdef _QUEST_ITEM
	// Custom: Quest/Clan Server
	//if (MGetServerConfig()->GetServerMode() == MSM_TEST)
	//{
		const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if( 0 == pNode )
		{
			mlog("MMatchServer::StageJoin - Couldn't find Setting.\n");
			return false;
		}

		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
			if( 0 == pRuleQuest )
			{
				mlog("MMatchServer::StageJoin - Couldn't find Quest Rule.\n");
				return false;
			}

			pRuleQuest->OnChangeCondition();
			//pRuleQuest->OnResponseQL_ToStage( pObj->GetStageUID() );
			// µ¿È¯¾¾²²¼­ Ã³À½ ½ºÅ×ÀÌÁEÁ¶ÀÎ½Ã´Â ÀÌÀE¡ ¼³Á¤ÀÌ Äù½ºÆ®·Î µÇÀÖ¾ûÑµ 
			//  Ã³À½ Á¶ÀÎÇÑ À¯Àú´Â Äù½ºÆ® Å¸ÀÔÀÎÁE¾Ë¼ö°¡ ¾ø±â¿¡,
			//	Å¬¶óÀÌ¾ðÆ®°¡ ½ºÅ×ÀÌÁEÅ¸ÀÔÀÌ Äù½ºÆ®ÀÎÁö¸¦ ÀÎ½ÄÇÏ´Â ½ÃÁ¡¿¡¼­
			//  ÀÌ Á¤º¸¸¦ ¿äÃ»À» ÇÏ´Â ¹æÇâÀ¸·Î ¼öÁ¤ÇÔ. - 05/04/14 by Ãß±³¼º.
			// pStage->GetRule()->OnResponseSacrificeSlotInfoToStage( uidPlayer );
		}
	//}
#endif

		if (rejoin == false)
		{
			// Cast Character Setting
			StageTeam(uidPlayer, uidStage, pObj->GetTeam());
			StagePlayerState(uidPlayer, uidStage, pObj->GetStageState());
		}
		else {
			pObj->SetForcedEntry(true);
			StageTeam(uidPlayer, uidStage, Team);
			StagePlayerState(uidPlayer, uidStage, MOSS_READY);
			MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
			RouteToListener(pObj, pCmd);
			pObj->SetLaunchedGame(true);
			pCmd = CreateCommand(MC_MATCH_LADDER_LAUNCH, uidPlayer);
			pCmd->AddParameter(new MCmdParamUID(uidStage));
			pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pStage->GetMapName())));
			pCmd->AddParameter(new MCmdParamBool((bool)(pStage->GetStageType() == MST_LADDERWARS)));
			RouteToListener(pObj, pCmd);
			StageEnterBattle(uidPlayer, uidStage);
		}


	return true;
}
bool MMatchServer::StageLeave(const MUID& uidPlayer)//, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject( uidPlayer );
	if( !IsEnabledObject(pObj) ) return false;
	// MMatchStage* pStage = FindStage(uidStage);

	//if(pObj->GetStageUID()!=uidStage)
	//	mlog(" stage leave hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return false;

	bool bLeaverMaster = false;
	if (uidPlayer == pStage->GetMasterUID()) bLeaverMaster = true;

#ifdef _QUEST_ITEM
	// Custom: Quest/Clan Server
	//if (MGetServerConfig()->GetServerMode() == MSM_TEST)
	//{
		const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if( 0 != pNode )
		{
			if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
			{
				MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
				if(pRuleQuest)
				{
					pRuleQuest->PreProcessLeaveStage( uidPlayer );
				} else {
					LOG(LOG_PROG, "StageLeave:: MMatchRule to MMatchRuleBaseQuest FAILED \n");
				}
			}
		}
	//}
#endif

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_LEAVE), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	// pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
	RouteToStage(pStage->GetUID(), pNew);

	pStage->RemoveObject(uidPlayer);

	//MMatchObject* pObj = GetObject(uidPlayer);
	//if (pObj)
	{
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.AddObject(pObj);
		MCommand* pCmdCache = CacheBuilder.GetResultCmd(MATCHCACHEMODE_REMOVE, this);
		RouteToStage(pStage->GetUID(), pCmdCache);
	}

	// cast Master
	if (bLeaverMaster) StageMaster(pStage->GetUID());

#ifdef _QUEST_ITEM
	// À¯Àú°¡ ½ºÅ×ÀÌÁö¿¡¼­ ³ª°£ÈÄ¿¡ QLÀ» ´Ù½Ã °è»EØ Áà¾ß ÇÔ.
	// Custom: Quest/Clan Server
	//if (MGetServerConfig()->GetServerMode() == MSM_TEST)
	{
		const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if( 0 == pNode )
		{
			LOG(LOG_PROG, "MMatchServer::StageLeave - MSTAGE_SETTING_NODE Failed.\n" );
			return false;
		}

		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
			if( 0 == pRuleQuest )
			{
				LOG(LOG_PROG, "MMatchServer::StageLeave - MMatchRuleBaseQuest Failed.\n" );
				return false;
			}

			if( STAGE_STATE_STANDBY == pStage->GetState() )
				pRuleQuest->OnChangeCondition();
				//pRuleQuest->OnResponseQL_ToStage( uidStage );
		}
	}
#endif

	return true;
}



DWORD StageEnterBattleExceptionHandler( PEXCEPTION_POINTERS ExceptionInfo )
{
	char szStageDumpFileName[ _MAX_DIR ]= {0,};
	SYSTEMTIME SystemTime;
	GetLocalTime( &SystemTime );
	sprintf( szStageDumpFileName, "Log/StageDump_%d-%d-%d_%d-%d-%d.dmp"
		, SystemTime.wYear
		, SystemTime.wMonth
		, SystemTime.wDay
		, SystemTime.wHour
		, SystemTime.wMinute
		, SystemTime.wSecond );

	return CrashExceptionDump( ExceptionInfo, szStageDumpFileName, true );
}

void InnerEnterBattle( MMatchObject* pObj, MMatchStage* pStage )
{
	pStage->EnterBattle( pObj );
}

void InnerEnterBattleException( MMatchObject* pObj, MMatchStage* pStage )
{
		mlog( "\nexception : stage enter battle =====================\n" );


		MMatchObject* pMaster = MGetMatchServer()->GetObject( pStage->GetMasterUID() );
		if( NULL != pMaster )  
		{
			if( NULL != pMaster->GetCharInfo() )
			{
				mlog( "stage master cid : %d\n", pMaster->GetCharInfo()->m_nCID );
			}
		}
		else
		{
			mlog( "stage master hass problem.\n" );				
		}
		
		
		if( NULL != pObj->GetCharInfo() )
		{
			mlog( "cmd sender cid : %d\n", pObj->GetCharInfo()->m_nCID );
		}
		else
		{
			mlog( "cmd send char info null point.\n" );
		}

		
		MMatchStageSetting*	pStageSetting = pStage->GetStageSetting();
		if( NULL != pStageSetting )
		{
			mlog( "stage state : %d\n", pStage->GetStageSetting()->GetStageState() );

			const MSTAGE_SETTING_NODE* pExStageSettingNode = pStageSetting->GetStageSetting();
			if( NULL != pExStageSettingNode )
			{
				mlog( "stage name : %s\n", pExStageSettingNode->szMapName );
				mlog( "stage game type : %d\n", pExStageSettingNode->nGameType );
				mlog( "stage max player : %d\n", pExStageSettingNode->nMaxPlayers );
				mlog( "stage current player : %d\n", pStage->GetPlayers() );
				mlog( "stage force entry enable : %d\n", pExStageSettingNode->bForcedEntryEnabled );
				mlog( "stage rule pointer : %x\n", pStage->GetRule() );
			}
		}

		MUIDRefCache::iterator itStage, endStage;
		endStage = pStage->GetObjEnd();
		itStage = pStage->GetObjBegin();
		MMatchObject* pObj2 = NULL;
		for( ; endStage != itStage; ++itStage )
		{
			pObj2 = MGetMatchServer()->GetObject( itStage->first );
			if( NULL == pObj2 )
			{
				mlog( "!!!!stage can't find player!!!!\n" );
				continue;
			}

			mlog( "stage player name : %s\n", pObj2->GetName() );
		}

		mlog( "=====================\n\n" );
}

bool ExceptionTraceStageEnterBattle( MMatchObject* pObj, MMatchStage* pStage )
{
	if( NULL == pObj )
	{
		return false;
	}

	if( NULL == pStage )
	{
		return false;
	}

	__try
	{
		InnerEnterBattle( pObj, pStage );
	}
	__except( StageEnterBattleExceptionHandler(GetExceptionInformation()) )
	{
		InnerEnterBattleException( pObj, pStage );

		return false;
	}

	return true;
}



bool MMatchServer::StageEnterBattle(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;
	// MMatchStage* pStage = FindStage(uidStage);
	
	if(pObj->GetStageUID()!=uidStage)
	{
		// Custom: Made it actually ignore the player by returning..
		mlog(" stage enter battle hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);
		return false;
	}

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return false;

	pObj->SetPlace(MMP_BATTLE);

	MCommand* pNew = CreateCommand(MC_MATCH_STAGE_ENTERBATTLE, MUID(0,0));
	//pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	//pNew->AddParameter(new MCommandParameterUID(uidStage));

	unsigned char nParam = MCEP_NORMAL;
	if (pObj->IsForcedEntried()) nParam = MCEP_FORCED;
	pNew->AddParameter(new MCommandParameterUChar(nParam));

	void* pPeerArray = MMakeBlobArray(sizeof(MTD_PeerListNode), 1);
	MTD_PeerListNode* pNode = (MTD_PeerListNode*)MGetBlobArrayElement(pPeerArray, 0);
	memset(pNode, 0, sizeof(MTD_PeerListNode));
	
	pNode->uidChar	= pObj->GetUID();
	// Custom: Don't distribute IP/Port in Clan Wars
	//if (pStage->GetStageSetting() != NULL && pStage->GetStageSetting()->IsTeamWinThePoint())
	//{
	//	// unconnectable, force relay
	//	pNode->dwIP = 0;
	//	pNode->nPort = 0;
	//}
	if (pObj->GetForcedNATOption() == true)
	{
		// There is a possibility that the GMs receive the non-staff user's IP and attempts to connect..?
		pNode->dwIP = 0;
		pNode->nPort = 0;
	}
	else
	{
		pNode->dwIP = pObj->GetIP();
		pNode->nPort = pObj->GetPort();
	}
	if (pStage->GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_SPY)
	{
		pStage->PlayerTeam(uidPlayer, MMT_BLUE);
		pNode->ExtendInfo.nTeam = (char)MMT_BLUE;
	}
	else if (pStage->GetStageSetting()->IsTeamPlay())
	{
		pNode->ExtendInfo.nTeam = (char)pObj->GetTeam();
	}
	else
	{
		pNode->ExtendInfo.nTeam = 0;
	}
	CopyCharInfoForTrans(&pNode->CharInfo, pObj->GetCharInfo(), pObj);
	//¹öÇÁÁ¤º¸ÀÓ½ÃÁÖ¼® 	CopyCharBuffInfoForTrans(&pNode->CharBuffInfo, pObj->GetCharInfo(), pObj);

	pNode->ExtendInfo.nPlayerFlags = pObj->GetPlayerFlags();
	if (pStage->GetStageSetting()->IsTeamPlay())	pNode->ExtendInfo.nTeam = (char)pObj->GetTeam();
	else											pNode->ExtendInfo.nTeam = 0;	

	pNew->AddParameter(new MCommandParameterBlob(pPeerArray, MGetBlobArraySize(pPeerArray)));
	MEraseBlobArray(pPeerArray);

	RouteToStage(uidStage, pNew);


	pObj->GetCharInfo()->m_nBattleStartTime = MMatchServer::GetInstance()->GetGlobalClockCount();
	pObj->GetCharInfo()->m_nBattleStartXP = pObj->GetCharInfo()->m_nXP;
	return ExceptionTraceStageEnterBattle(pObj, pStage);
}

bool MMatchServer::StageLeaveBattle(const MUID& uidPlayer, bool bGameFinishLeaveBattle, bool bForcedLeave)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;
	if (pObj->GetPlace() != MMP_BATTLE) { return false; }
	const MUID uidStage = pObj->GetStageUID();
	MMatchStage* pStage = FindStage(uidStage);

	if (pStage == NULL)
	{
		if (pObj->GetRelayPeer())
		{
			MAgentObject* pAgent = GetAgent(pObj->GetAgentUID());
			if (pAgent) {
				MCommand* pCmd = CreateCommand(MC_AGENT_PEER_UNBIND, pAgent->GetCommListener());
				pCmd->AddParameter(new MCmdParamUID(uidPlayer));
				Post(pCmd);
			}
		}

		UpdateCharDBCachingData(pObj);
		UpdateCharItemDBCachingData(pObj);
		ProcessCharPlayInfo(pObj);
		return false;
	}
	else
	{

		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.Reset();
		for (MUIDRefCache::iterator i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); i++) {
			MMatchObject* pScanObj = (MMatchObject*)(*i).second;
			CacheBuilder.AddObject(pScanObj);
		}
		MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
		RouteToListener(pObj, pCmdCacheUpdate);
	}

	pStage->LeaveBattle(pObj);
	pObj->SetPlace(MMP_STAGE);

#ifdef _PORTALGUN 1
	pObj->m_UIDItemBlue = 0;
	pObj->m_UIDItemRed = 0;
	pObj->m_UIDItemDated = 0;
	pObj->m_bMoveWorldItem = false;
#endif

	// ·¹º§¿¡ ¾È¸Â´Â Àåºñ¾ÆÀÌÅÛ Ã¼Å©
#define LEGAL_ITEMLEVEL_DIFF		3
	bool bIsCorrect = true;
	for (int i = 0; i < MMCIP_END; i++) {
		if (CorrectEquipmentByLevel(pObj, MMatchCharItemParts(i), LEGAL_ITEMLEVEL_DIFF)) {
			bIsCorrect = false;
		}
	}

	if (!bIsCorrect) {
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_RESULT, MUID(0,0));
		pNewCmd->AddParameter(new MCommandParameterInt(MERR_TAKEOFF_ITEM_BY_LEVELDOWN));
		RouteToListener(pObj, pNewCmd);
	}
	
	CheckExpiredItems(pObj);		//< ±â°£ ¸¸·E¾ÆÀÌÅÛÀÌ ÀÖ´ÂÁEÃ¼Å©

	if (pObj->GetRelayPeer()) {
		MAgentObject* pAgent = GetAgent(pObj->GetAgentUID());
		if (pAgent) {
			MCommand* pCmd = CreateCommand(MC_AGENT_PEER_UNBIND, pAgent->GetCommListener());
			pCmd->AddParameter(new MCmdParamUID(uidPlayer));
			Post(pCmd);
		}
	}	

	// Ä³¸¯ÅÍ ÇÃ·¹ÀÌÇÑ Á¤º¸ ¾÷µ¥ÀÌÆ® 
	ProcessCharPlayInfo(pObj);

	//=======================================================================================================================================
	
	bool bIsLeaveAllBattle = true;
	
	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pAllObj = (MMatchObject*)GetObject(uidObj);
		if(NULL == pAllObj) continue;
		if(MMP_STAGE != pAllObj->GetPlace()) { 
			bIsLeaveAllBattle = false; 
			break; 
		}
	}


	if(pStage->IsRelayMap())
	{
		if(bGameFinishLeaveBattle)
		{	// ¸±·¹ÀÌ¸Ê, ¹èÆ² Á¾·á·Î ½ºÅ×ÀÌÁö·Î ³ª¿ÔÀ»¶§
			if(!pStage->m_bIsLastRelayMap)
			{	// ´ÙÀ½¸ÊÀÌ ÀÖ´Ù¸E¹Ù·Î ´ÙÀ½ ¸Ê½ÃÀÛ Ã³¸®		

				if( !bForcedLeave ) 
				{
					pObj->SetStageState(MOSS_READY);
				}

				if( bIsLeaveAllBattle ) 
				{					
					OnStageRelayStart(uidStage);
				} 

				MCommand* pNew = CreateCommand(MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT, MUID(0,0));
				pNew->AddParameter(new MCommandParameterUID(uidPlayer));
				pNew->AddParameter(new MCommandParameterBool(true));
				RouteToStage(uidStage, pNew);
			}
		}
		else
		{	///< ¸ÞÀÎ ¸Þ´º·Î ½ºÅ×ÀÌÁö¿¡ ³ª¿È		
			MCommand* pNew = CreateCommand(MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT, MUID(0,0));
			pNew->AddParameter(new MCommandParameterUID(uidPlayer));
			pNew->AddParameter(new MCommandParameterBool(false));
			RouteToStage(uidStage, pNew);			

			if(bIsLeaveAllBattle) 
			{	///< ¸ðµÎ ½ºÅ×ÀÌÁö¿¡ ÀÖ´Ù¸E¸±·¹ÀÌ¸Ê ¼¼ÆÃÀ» ´Ù½Ã ÇØÁØ´Ù.
				pStage->m_bIsLastRelayMap = true;//¸±·¹ÀÌ¸ÊÀ» ³¡³½´Ù
				pStage->GetStageSetting()->SetMapName(MMATCH_MAPNAME_RELAYMAP);
				pStage->SetRelayMapCurrList(pStage->GetRelayMapList());
				pStage->m_RelayMapRepeatCountRemained = pStage->GetRelayMapRepeatCount();
			}
		}
	} 
	else 
	{
		MCommand* pNew = CreateCommand(MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT, MUID(0,0));
		pNew->AddParameter(new MCommandParameterUID(uidPlayer));
		pNew->AddParameter(new MCommandParameterBool(false));
		RouteToStage(uidStage, pNew);
	}

	//=======================================================================================================================================

	// ¹æ¿¡¼­ ³ª°¡¸Enoready»óÅÂ·Î º¯°æµÈ´Ù. 
	// º¯°æµÈ Á¤º¸¸¦ ½ºÅ×ÀÌÁöÀÇ ¸ðµEÅ¬¶óÀÌ¾ðÆ®·Î º¸³»ÁÜ. - by SungE 2007-06-04
	StagePlayerState( uidPlayer, pStage->GetUID(), pObj->GetStageState() );	
	
	UpdateCharDBCachingData(pObj);		///< XP, BP, KillCount, DeathCount Ä³½³ ¾÷µ¥ÀÌÆ®
	UpdateCharItemDBCachingData(pObj);	///< Character Item¿¡¼­ ¾÷µ¥ÀÌÆ®°¡ ÇÊ¿äÇÑ °ÍµE¾÷µ¥ÀÌÆ®
	//CheckSpendableItemCounts(pObj);		///< Ç×»EUpdateCharItemDBCachingData µÚ¿¡ ÀÖ¾ûÚß ÇÕ´Ï´Ù.

	return true;
}

bool MMatchServer::StageChat(const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL)	return false;
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) return false;

	if (pObj->GetAccountInfo()->m_nUGrade == MMUG_CHAT_LIMITED) return false;

//	InsertChatDBLog(uidPlayer, pszChat);

	///< È«±âÁÖ(2009.08.04)
	///< ÇöÀEÇØ´E»ç¿EÚ°¡ ÀÖ´Â Stage¿Í º¸³»¿Â StageÀÇ UID°¡ ´Ù¸¦ °æ¿E
	///< ´Ù¸¥ Stageµé¿¡°Ôµµ Msg¸¦ º¸³¾ ¼EÀÖ´Â ¹®Á¦°¡ ÀÖÀ½ (ÇØÅ· ÇÁ·Î±×·¥ »ç¿EÃ)
	if( uidStage != pObj->GetStageUID() )
	{
		//LOG(LOG_FILE,"MMatchServer::StageChat - Different Stage(S:%d, P:%d)", uidStage, pObj->GetStageUID());
		return false;
	}
#ifdef _FORCE_EVENT_TEAM
	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (pChannel == NULL)
		return false;
#endif
	// Custom: Silence Room
	DWORD dwTime = timeGetTime();
	if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER && dwTime < pStage->GetSilenceEndTime())
	{
		char szMsg[128];
		memset(szMsg, 0, sizeof( szMsg ));
		sprintf_s( szMsg, "Room is silenced. You may chat in %d seconds.", (pStage->GetSilenceEndTime() - dwTime)/1000 );

		Announce(pObj, szMsg);
		return false;
	}

#ifdef _FORCE_EVENT_TEAM
	if (pChannel->IsUseEvent())
	{
		if (!IsAdminGrade(pObj))
			return false;
	}
	else

	{
		pObj->SetTeam(pStage->GetRecommandedTeam());
	}
#endif
	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_CHAT), MUID(0,0), m_This);
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterString(pszChat));
	RouteToStage(uidStage, pCmd);
	return true;
}

// Custom: in-game chat server sided.
bool MMatchServer::GameChat(const MUID& uidPlayer, const MUID& uidStage, char* szMsg, int nTeam)
{
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;
	if (pObj->GetAccountInfo()->m_nUGrade == MMUG_CHAT_LIMITED) return false;
	if (nTeam > 1 && nTeam != pObj->GetTeam()) return false;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL)	return false;
	if (uidStage != pObj->GetStageUID()) return false;

	// Custom: Silence Room
	DWORD dwTime = timeGetTime();
	if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER && dwTime < pStage->GetSilenceEndTime())
	{
		char szMsg[128];
		memset(szMsg, 0, sizeof(szMsg));
		sprintf_s(szMsg, "Room is silenced. You may chat in %d seconds.", (pStage->GetSilenceEndTime() - dwTime) / 1000);

		Announce(pObj, szMsg);
		return false;
	}

	// Send to everyone in the chat.
	if ((nTeam == MMT_ALL) || (nTeam == MMT_SPECTATOR))
	{
		MCommand* pCmd = CreateCommand(MC_MATCH_GAME_CHAT, MUID(0, 0));
		pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
		pCmd->AddParameter(new MCommandParameterUID(uidStage));
		pCmd->AddParameter(new MCommandParameterString(szMsg));
		pCmd->AddParameter(new MCommandParameterInt(nTeam));
		RouteToBattle(pObj->GetStageUID(), pCmd);
	}
	// Send to own team only.
	else if (nTeam == pObj->GetTeam())
	{
		for (MUIDRefCache::iterator itor = pStage->GetObjBegin(); itor != pStage->GetObjEnd(); itor++)
		{
			MMatchObject* pPlayer = (MMatchObject*)(*itor).second;

			if (!IsEnabledObject(pPlayer)) continue;
			if (!pPlayer->GetEnterBattle()) continue;
			if (pPlayer->GetTeam() != nTeam && !pPlayer->CheckPlayerFlags(MTD_PlayerFlags_AdminHide)) continue;

			MCommand* pCmd = CreateCommand(MC_MATCH_GAME_CHAT, MUID(0, 0));
			pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
			pCmd->AddParameter(new MCommandParameterUID(uidStage));
			pCmd->AddParameter(new MCommandParameterString(szMsg));
			pCmd->AddParameter(new MCommandParameterInt(nTeam));
			RouteToListener(pPlayer, pCmd);
		}
	}

	return true;
}

bool MMatchServer::StageTeam(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	pStage->PlayerTeam(uidPlayer, nTeam);

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_TEAM, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterUInt(nTeam));

	RouteToStageWaitRoom(uidStage, pCmd);
	return true;
}

bool MMatchServer::StagePlayerState(const MUID& uidPlayer, const MUID& uidStage, MMatchObjectStageState nStageState)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;
	// MMatchStage* pStage = FindStage(uidStage);
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return false;

	pStage->PlayerState(uidPlayer, nStageState);
	
	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_PLAYER_STATE, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterInt(nStageState));
	RouteToStage(uidStage, pCmd);
	return true;
}

bool MMatchServer::StageMaster(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	MUID uidMaster = pStage->GetMasterUID();

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_MASTER, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterUID(uidMaster));
	RouteToStage(uidStage, pCmd);

	return true;
}

void MMatchServer::StageLaunch(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	ReserveAgent(pStage);
		
	if (MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType()))
	{
		if (pStage->m_pRule)
		{
			pStage->m_pRule->OnQuestStageLaunch();
		}
	}


	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_LAUNCH, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidStage));
	pCmd->AddParameter(new MCmdParamStr( const_cast<char*>(pStage->GetMapName()) ));
	RouteToStage(uidStage, pCmd);
}

void MMatchServer::StageRelayLaunch(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	ReserveAgent(pStage);

	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pObj = (MMatchObject*)GetObject(uidObj);
		if (pObj) {
			if( pObj->GetStageState() == MOSS_READY) {
				MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_RELAY_LAUNCH, MUID(0,0));
				pCmd->AddParameter(new MCmdParamUID(uidStage));
				pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pStage->GetMapName())));
				pCmd->AddParameter(new MCmdParamBool(false));
				RouteToListener(pObj, pCmd);
			} else {
				MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_RELAY_LAUNCH, MUID(0,0));
				pCmd->AddParameter(new MCmdParamUID(uidStage));
				pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pStage->GetMapName())));
				pCmd->AddParameter(new MCmdParamBool(true));
				RouteToListener(pObj, pCmd);
			}
		} else {
			LOG(LOG_PROG, "WARNING(StageRelayLaunch) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
			i=pStage->RemoveObject(uidObj);
			LogObjectCommandHistory(uidObj);
		}
	}
}

void MMatchServer::StageFinishGame(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	bool bIsRelayMapUnFinish = true;

	if (pStage->GetStageType() == MST_LADDER || pStage->GetStageType() == MST_LADDERWARS)
	{
		ClanReDef::iterator it = ClanRejoiner.begin();
		while (it != ClanRejoiner.end())
		{
			if (it->second->StageUID == uidStage)
			{
				ClanRejoiner.erase(it++);
			}
			else
				++it;
		}
	}

	if(pStage->IsRelayMap())
	{ // ¸±·¹ÀÌ ¸ÊÀÏ¶§¿¡´Â ¹èÆ²À» ´Ù½Ã ½ÃÀÛÇØÁØ´Ù. 
		if((int)pStage->m_vecRelayMapsRemained.size() <= 0)
		{	// ³²Àº ¸ÊÀÌ ¾øÀ»¶§
			int nRepeatCount = (int)pStage->m_RelayMapRepeatCountRemained - 1;
			if(nRepeatCount < 0)
			{
				bIsRelayMapUnFinish = false;

				pStage->m_bIsLastRelayMap = true;//¸±·¹ÀÌ¸ÊÀ» ³¡³½´Ù				
				nRepeatCount = 0;
				pStage->GetStageSetting()->SetMapName(MMATCH_MAPNAME_RELAYMAP);	//"RelayMap" ¼¼ÆÃ
			}
			pStage->m_RelayMapRepeatCountRemained = (RELAY_MAP_REPEAT_COUNT)nRepeatCount;
			pStage->SetRelayMapCurrList(pStage->GetRelayMapList());
		}

		if(!pStage->m_bIsLastRelayMap) {
			// Ã³À½ ½ÃÀÛ½Ã, Flag¸¦ On½ÃÄÑÁØ´Ù. 
			if( pStage->IsStartRelayMap() == false ) {
				pStage->SetIsStartRelayMap(true);
			}			

			if((int)pStage->m_vecRelayMapsRemained.size() > 0) { // ´ÙÀ½¸ÊÀÌ ÀÖ´Ù¸E
				int nRelayMapIndex = 0;

				if(pStage->GetRelayMapType() == RELAY_MAP_TURN) {	//< ³²Àº °ÍÁß¿¡¼­ Ã¹ ¹øÂ°ºÎÅÍ ½ÃÀÛ(°¡µ¶¼º)
					nRelayMapIndex = 0; 
				} else if(pStage->GetRelayMapType() == RELAY_MAP_RANDOM) {
					nRelayMapIndex = rand() % (int)pStage->m_vecRelayMapsRemained.size();
				}

				if(nRelayMapIndex >= MAX_RELAYMAP_LIST_COUNT) { //< ¸Ê ±¸¼ºÀÌ 20°³
					mlog("StageFinishGame RelayMap Fail RelayMapList MIsCorrect MaxCount[%d] \n", (int)nRelayMapIndex);
					return;
				}

				char* szMapName = (char*)MGetMapDescMgr()->GetMapName(pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID);
				if (!szMapName)
				{
					mlog("RelayMapBattleStart Fail MapID[%d] \n", (int)pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID);
					return;
				}

				pStage->GetStageSetting()->SetMapName(szMapName);

				// ½ÇÇàÇÑ ¸±·¹ÀÌ¸ÊÀº »èÁ¦ÇØÁØ´Ù.
				vector<RelayMap>::iterator itor = pStage->m_vecRelayMapsRemained.begin();
				for(int i=0 ; nRelayMapIndex > i ; ++itor, ++i);// ÇØ´EÀÎµ¦½º±ûÝEÀÌµ¿
				pStage->m_vecRelayMapsRemained.erase(itor);
			} 
			else {
				mlog("MMatchServer::StageFinishGame::IsRelayMap() - m_vecRelayMapsRemained.size() == 0\n");
			}
		} else {
			pStage->SetIsStartRelayMap(false);
			bIsRelayMapUnFinish = false; // ¸±·¹ÀÌ¸Ê ÁøÇàÀÌ ³¡³µÀ½
		}
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_FINISH_GAME, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterBool(bIsRelayMapUnFinish));
	RouteToStage(uidStage, pCmd);

	return;
}

MCommand* MMatchServer::CreateCmdResponseStageSetting(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return NULL;

	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_STAGESETTING, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetUID()));

	MMatchStageSetting* pSetting = pStage->GetStageSetting();

	// Param 1 : Stage Settings
	void* pStageSettingArray = MMakeBlobArray(sizeof(MSTAGE_SETTING_NODE), 1);
	MSTAGE_SETTING_NODE* pNode = (MSTAGE_SETTING_NODE*)MGetBlobArrayElement(pStageSettingArray, 0);
	CopyMemory(pNode, pSetting->GetStageSetting(), sizeof(MSTAGE_SETTING_NODE));
	pCmd->AddParameter(new MCommandParameterBlob(pStageSettingArray, MGetBlobArraySize(pStageSettingArray)));
	MEraseBlobArray(pStageSettingArray);

	// Param 2 : Char Settings
	int nCharCount = (int)pStage->GetObjCount();
	void* pCharArray = MMakeBlobArray(sizeof(MSTAGE_CHAR_SETTING_NODE), nCharCount);
	int nIndex=0;
	for (MUIDRefCache::iterator itor=pStage->GetObjBegin(); itor!=pStage->GetObjEnd(); itor++) {
		MSTAGE_CHAR_SETTING_NODE* pCharNode = (MSTAGE_CHAR_SETTING_NODE*)MGetBlobArrayElement(pCharArray, nIndex++);
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		pCharNode->uidChar = pObj->GetUID();
		pCharNode->nTeam = pObj->GetTeam();
		pCharNode->nState = pObj->GetStageState();
	}
	pCmd->AddParameter(new MCommandParameterBlob(pCharArray, MGetBlobArraySize(pCharArray)));
	MEraseBlobArray(pCharArray);

	// Param 3 : Stage State
	pCmd->AddParameter(new MCommandParameterInt((int)pStage->GetState()));

	// Param 4 : Stage Master
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetMasterUID()));

	return pCmd;
}



void MMatchServer::OnStageCreate(const MUID& uidChar, char* pszStageName, bool bPrivate, char* pszStagePassword,
	void* pModBlob, int nModCount)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (pChannel == NULL) return;

	if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pChannel->GetChannelType() == MCHANNEL_TYPE_CLAN)
		&& (pChannel->GetChannelType() == MCHANNEL_TYPE_DUELTOURNAMENT || pChannel->GetChannelType() == MCHANNEL_TYPE_LADDERWAR)) {
		return;
	}

	// Custom: Event channels
	if ((!IsAdminGrade(pObj)) && (strstr(pChannel->GetName(), "Event") && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER))
	{
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_CREATE, MERR_CANNOT_CREATE_STAGE);
		return;
	}

	if (strlen(pszStageName) <= 0)
	{
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_CREATE, MERR_CANNOT_CREATE_STAGE);
		return;
	}
	
	MUID uidStage;

	if (!StageAdd(pChannel, pszStageName, bPrivate, pszStagePassword, &uidStage, false))
	{
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_CREATE, MERR_CANNOT_CREATE_STAGE);
		return;
	}
	MMatchStage* pStage = FindStage(uidStage);

	// mod phai co truoc khi nguoi dau tien vao phong, khong thi goi cai dat dau tien thieu mod
	if (pStage && pModBlob != NULL && nModCount > 0)
	{
		if (nModCount > MMOD_END) nModCount = MMOD_END;

		MMatchRoomModifier mods[MMOD_END];
		for (int i = 0; i < nModCount; ++i)
			mods[i] = *(MMatchRoomModifier*)MGetBlobArrayElement(pModBlob, i);

		pStage->GetStageSetting()->SetRoomModsFromArray(mods, nModCount);
	}

	StageJoin(uidChar, uidStage);

	if (pStage)
		pStage->SetFirstMasterName(pObj->GetCharInfo()->m_szName);
}


//void MMatchServer::OnStageJoin(const MUID& uidChar, const MUID& uidStage)
//{
//	MMatchObject* pObj = GetObject(uidChar);
//	if (pObj == NULL) return;
//
//	MMatchStage* pStage = NULL;
//
//	if (uidStage == MUID(0,0)) {
//		return;
//	} else {
//		pStage = FindStage(uidStage);
//	}
//
//	if (pStage == NULL) {
//		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_STAGE_NOT_EXIST);
//		return;
//	}
//
//	if ((IsAdminGrade(pObj) == false) && pStage->IsPrivate())
//	{
//		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_CANNOT_JOIN_STAGE_BY_PASSWORD);
//		return;
//	}
//
//	StageJoin(uidChar, pStage->GetUID());
//}

void MMatchServer::OnPrivateStageJoin(const MUID& uidPlayer, const MUID& uidStage, char* pszPassword)
{
	if (strlen(pszPassword) > STAGEPASSWD_LENGTH) return;

	MMatchStage* pStage = NULL;

	if (uidStage == MUID(0,0)) 
	{
		return;
	} 
	else 
	{
		pStage = FindStage(uidStage);
	}

	if (pStage == NULL) 
	{
		MMatchObject* pObj = GetObject(uidPlayer);
		if (pObj != NULL)
		{
			RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_STAGE_NOT_EXIST);
		}

		return;
	}

	// ¿µÀÚ³ª °³¹ßÀÚ¸E¹«½Ã..

	bool bSkipPassword = false;

	MMatchObject* pObj = GetObject(uidPlayer);

	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) 
		return;

	MMatchUserGradeID ugid = pObj->GetAccountInfo()->m_nUGrade;

	if (ugid == MMUG_DEVELOPER || ugid == MMUG_ADMIN || ugid == MMUG_GAMEMASTER)
		bSkipPassword = true;

	// ºñ¹Ð¹æÀÌ ¾Æ´Ï°Å³ª ÆÐ½º¿öµå°¡ Æ²¸®¸EÆÐ½º¿öµå°¡ Æ²·È´Ù°EÀÀ´äÇÑ´Ù.
	if(bSkipPassword==false) 
	{
		// Custom: Added check for non-GMs joining ladder/DT matches
		if ((!pStage->IsPrivate()) || (strcmp(pStage->GetPassword(), pszPassword)) || (pStage->GetStageType() == MST_LADDER) || (pStage->GetStageType() == MST_LADDERWARS) || (pStage->GetStageSetting() && pStage->GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUELTOURNAMENT))
		{
			MMatchObject* pObj = GetObject(uidPlayer);
			if (pObj != NULL)
			{
				RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_CANNOT_JOIN_STAGE_BY_PASSWORD);
			}

			return;
		}
	}

	StageJoin(uidPlayer, pStage->GetUID());
}

void MMatchServer::OnStageFollow(const MUID& uidPlayer, const char* pszTargetName)
{
	MMatchObject* pPlayerObj = GetObject(uidPlayer);
	if (pPlayerObj == NULL) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) return;

	// ÀÚ±EÀÚ½ÅÀ» µû¶E°¡·Á°EÇßÀ»°æ¿E°Ë»E
	if (pPlayerObj->GetUID() == pTargetObj->GetUID()) return;

	// ½ºÅ×ÀÌÆ®°¡ Àß¸øµÇ¾EÀÖ´ÂÁE°Ë»E
	if (!pPlayerObj->CheckEnableAction(MMatchObject::MMOA_STAGE_FOLLOW)) return;


	// ¼­·Î ´Ù¸¥ Ã¤³ÎÀÎÁE°Ë»E
	if (pPlayerObj->GetChannelUID() != pTargetObj->GetChannelUID()) {

#ifdef _VOTESETTING
		RouteResponseToListener( pPlayerObj, MC_MATCH_RESPONSE_STAGE_FOLLOW, MERR_CANNOT_FOLLOW );
#endif
		return;
	}

	// Custom: Made this message appear only when the user is not an admin/event team member
	if (!IsAdminGrade(pPlayerObj) && pPlayerObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER && (IsAdminGrade(pTargetObj) == true)) {
		NotifyMessage(pPlayerObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	MMatchStage* pStage = FindStage(pTargetObj->GetStageUID());
	if (pStage == NULL) return;

	// Custom: Allow admins to spectate ladder matches
	// Å¬·£ÀEÔÀÓÀº µû¶ó°¥ ¼E¾øÀ½
	//if (pStage->GetStageType() != MST_NORMAL) return;

	if (pStage->GetStageType() != MST_NORMAL)
	{
		if (!IsAdminGrade(pPlayerObj) && pPlayerObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER)
			return;

		// Hide player
		if (!pPlayerObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
			OnAdminHide(uidPlayer);

		StageJoin(uidPlayer, pTargetObj->GetStageUID());

		// Fix for being in game room
		OnRequestForcedEntry(pStage->GetUID(), uidPlayer);
		return;
	}

	if (pStage->IsPrivate()==false) {
		if ((pStage->GetStageSetting()->GetForcedEntry()==false) && pStage->GetState() != STAGE_STATE_STANDBY) {
			// Deny Join

#ifdef _VOTESETTING
			RouteResponseToListener( pPlayerObj, MC_MATCH_RESPONSE_STAGE_FOLLOW, MERR_CANNOT_FOLLOW );
#endif
		} else {
			StageJoin(uidPlayer, pTargetObj->GetStageUID());
		}
	}
	else {
		// µû¶ó°¡·Á´Â ¹æÀÌ ºñ¹Ð¹øÈ£¸¦ ÇÊ¿ä·Î ÇÒ°æ¿EÂ µû¶ó°¥¼E¾øÀ½.
		//RouteResponseToListener( pPlayerObj, MC_MATCH_RESPONSE_STAGE_FOLLOW, MERR_CANNOT_FOLLOW_BY_PASSWORD );

		// ÇØ´ç¹æÀÌ ºñ¹Ð¹æÀÌ¸Eºñ¹Ð¹øÈ£¸¦ ¿ä±¸ÇÑ´Ù.
		MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_REQUIRE_PASSWORD), MUID(0,0), m_This);
		pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
		pNew->AddParameter(new MCommandParameterString((char*)pStage->GetName()));
		RouteToListener(pPlayerObj, pNew);
	}
}

void MMatchServer::OnStageLeave(const MUID& uidPlayer)//, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject( uidPlayer );
	if( !IsEnabledObject(pObj) ) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if( !IsEnabledObject(GetObject(uidPlayer)) )
	{
		return;
	}

	StageLeave(uidPlayer);// , uidStage);
}

void MMatchServer::OnStageRequestPlayerList(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	// MMatchStage* pStage = FindStage(uidStage);
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// ¹æÀÎ¿E¸ñ·Ï
	MMatchObjectCacheBuilder CacheBuilder;
	CacheBuilder.Reset();
	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MMatchObject* pScanObj = (MMatchObject*)(*i).second;
		CacheBuilder.AddObject(pScanObj);
	}
    MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
	RouteToListener(pObj, pCmdCacheUpdate);

	// Cast Master(¹æÀE
	MUID uidMaster = pStage->GetMasterUID();
	MCommand* pMasterCmd = CreateCommand(MC_MATCH_STAGE_MASTER, MUID(0,0));
	pMasterCmd->AddParameter(new MCommandParameterUID(uidStage));
	pMasterCmd->AddParameter(new MCommandParameterUID(uidMaster));
	RouteToListener(pObj, pMasterCmd);

	// Cast Character Setting
	StageTeam(uidPlayer, uidStage, pObj->GetTeam());
	StagePlayerState(uidPlayer, uidStage, pObj->GetStageState());
}

void MMatchServer::OnStageEnterBattle(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	StageEnterBattle(uidPlayer, uidStage);
}

void MMatchServer::OnStageLeaveBattle(const MUID& uidPlayer, bool bGameFinishLeaveBattle)//, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	// Custom: if vote is going on, instantly kick the user.
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if (pStage->CheckUserWasVoted(uidPlayer)) {
		pStage->KickBanPlayer(pObj->GetCharInfo()->m_szName, true);
		return;
	}
	
	StageLeaveBattle(uidPlayer, bGameFinishLeaveBattle, false);//, uidStage);
}


#include "CMLexicalAnalyzer.h"
// °­ÅEÀÓ½ÃÄÚµE
bool StageKick(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	MMatchObject* pChar = pServer->GetObject(uidPlayer);
	if (pChar == NULL)	return false;
	MMatchStage* pStage = pServer->FindStage(uidStage);
	if (pStage == NULL) return false;
	if (uidPlayer != pStage->GetMasterUID()) return false;

	bool bResult = false;
	CMLexicalAnalyzer lex;
	lex.Create(pszChat);

	if (lex.GetCount() >= 1) {
		char* pszCmd = lex.GetByStr(0);
		if (pszCmd) {
			if (stricmp(pszCmd, "/kick") == 0) {
				if (lex.GetCount() >= 2) {
					char* pszTarget = lex.GetByStr(1);
					if (pszTarget) {
						for (MUIDRefCache::iterator itor = pStage->GetObjBegin(); 
							itor != pStage->GetObjEnd(); ++itor)
						{
							MMatchObject* pTarget = (MMatchObject*)((*itor).second);
							if (stricmp(pszTarget, pTarget->GetName()) == 0) {
								if (pTarget->GetPlace() != MMP_BATTLE) {
									pServer->StageLeave(pTarget->GetUID());//, uidStage);
									bResult = true;
								}
								// Custom: Kick player from stage while target is in battle, but the game must either be in Prepare/Finish/Exit
								else if ( pTarget->GetPlace() == MMP_BATTLE && pStage->GetRule() != NULL && pStage->GetRule()->GetRoundState() != MMATCH_ROUNDSTATE_PLAY && pStage->GetRule()->GetRoundState() != MMATCH_ROUNDSTATE_COUNTDOWN )
								{
									pServer->StageLeaveBattle(pTarget->GetUID(), true, true);
									pServer->StageLeave(pTarget->GetUID());
									bResult = true;
								}
								break;
							}
						}
					}
				}
			}	// Kick
		}
	}

	lex.Destroy();
	return bResult;
}

// ¹æÀåÈ®ÀÎ ÀÓ½ÃÄÚµE
bool StageShowInfo(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	MMatchObject* pChar = pServer->GetObject(uidPlayer);
	if (pChar == NULL)	return false;
	MMatchStage* pStage = pServer->FindStage(uidStage);
	if (pStage == NULL) return false;
	if (uidPlayer != pStage->GetMasterUID()) return false;

	bool bResult = false;
	CMLexicalAnalyzer lex;
	lex.Create(pszChat);

	if (lex.GetCount() >= 1) {
		char* pszCmd = lex.GetByStr(0);
		if (pszCmd) {
			if (stricmp(pszCmd, "/showinfo") == 0) {
				char szMsg[256]="";
				sprintf(szMsg, "FirstMaster : (%s)", pStage->GetFirstMasterName());
				pServer->Announce(pChar, szMsg);
				bResult = true;
			}	// ShowInfo
		}
	}

	lex.Destroy();
	return bResult;
}
void MMatchServer::OnStageChat(const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	// RAONHAJE : °­ÅEÀÓ½ÃÄÚµE
	if (pszChat[0] == '/') {
		if (StageKick(this, uidPlayer, uidStage, pszChat))
			return;
		if (StageShowInfo(this, uidPlayer, uidStage, pszChat))
			return;
	}

	StageChat(uidPlayer, uidStage, pszChat);
}

void MMatchServer::OnGameChat(const MUID& uidPlayer, const MUID& uidStage, char* szChat, int nTeam)
{
	GameChat(uidPlayer, uidStage, szChat, nTeam);
}

void MMatchServer::OnStageStart(const MUID& uidPlayer, const MUID& uidStage, int nCountdown)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetMasterUID() != uidPlayer) return;

	if (pStage->StartGame(MGetServerConfig()->IsUseResourceCRC32CacheCheck()) == true) {
		StageRelayMapBattleStart(uidPlayer, uidStage);

		MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_START), MUID(0,0), m_This);
		pNew->AddParameter(new MCommandParameterUID(uidPlayer));
		pNew->AddParameter(new MCommandParameterUID(uidStage));
		pNew->AddParameter(new MCommandParameterInt(min(nCountdown,3)));
		RouteToStage(uidStage, pNew);

		// µðºñ¿¡ ·Î±×¸¦ ³²±ä´Ù.
		SaveGameLog(uidStage);
	}
}

void MMatchServer::OnStageRelayStart(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	
	if (pStage->StartRelayGame(MGetServerConfig()->IsUseResourceCRC32CacheCheck()) == true) {
		// µðºñ¿¡ ·Î±×¸¦ ³²±ä´Ù.
		SaveGameLog(uidStage);
	}
}

void MMatchServer::OnStartStageList(const MUID& uidComm)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->SetStageListTransfer(true);
}

void MMatchServer::OnStopStageList(const MUID& uidComm)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->SetStageListTransfer(false);
}

void MMatchServer::OnStagePlayerState(const MUID& uidPlayer, const MUID& uidStage, MMatchObjectStageState nStageState)
{
	StagePlayerState(uidPlayer, uidStage, nStageState);
}


//void MMatchServer::OnStageTeam(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam)
//{
//	MMatchStage* pStage = FindStage(uidStage);
//	if (pStage == NULL) return;
//
//	MMatchObject* pChar = GetObject(uidPlayer);
//	if (pChar == NULL) return;
//	if (pStage->GetStageType() == MST_LADDER && !IsAdminGrade(pChar) && !pChar->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER && !pChar->GetAccountInfo()->m_nUGrade == MMUG_ADMIN || !IsAdminGrade(pChar) && !pChar->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER && !pChar->GetAccountInfo()->m_nUGrade == MMUG_ADMIN || pStage->GetStageType() == MST_LADDERWARS && !IsAdminGrade(pChar) && !pChar->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER && !pChar->GetAccountInfo()->m_nUGrade == MMUG_MANAGER) return;
//	StageTeam(uidPlayer, uidStage, nTeam);
//}
void MMatchServer::OnStageTeam(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	MMatchObject* pChar = GetObject(uidPlayer);
	if (pChar == NULL) return;

	StageTeam(uidPlayer, uidStage, nTeam);
}
void MMatchServer::OnStageMap(const MUID& uidStage, char* pszMapName, const MUID& uidPlayer)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;	// ´Eâ»óÅÂ¿¡¼­¸¸ ¹Ù²Ü¼EÀÖ´Ù
	if (strlen(pszMapName) < 2) return;

	pStage->SetMapName( pszMapName );
	pStage->SetIsRelayMap(strcmp(MMATCH_MAPNAME_RELAYMAP, pszMapName) == 0);
	
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_MAP), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterString(pStage->GetMapName()));

	if ( MGetGameTypeMgr()->IsQuestDerived( pStage->GetStageSetting()->GetGameType()))
	{
		MMatchRuleBaseQuest* pQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
		pQuest->RefreshStageGameInfo();
	}

    RouteToStage(uidStage, pNew);
}
//Custom: Lightmaps Time Code
void MMatchServer::OnStageMapLighting(MUID const& matchStage, int& lightingIndex, MUID const& sender)
{
	MMatchStage* pStage = FindStage(matchStage);
	if (pStage == NULL) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;	// ´ë±â»óÅÂ¿¡¼­¸¸ ¹Ù²Ü¼ö ÀÖ´Ù

	if (pStage->GetMasterUID() != sender)
	{
		return;
	}

	//Safety to prevent issues with crashes if fuckery happens
	if (lightingIndex > MGetMapDescMgr()->GetLightMapCount(pStage->GetStageSetting()->GetMapIndex()))
	{
		lightingIndex = 0;
	}

	pStage->GetStageSetting()->SetMapLightMapIndex(lightingIndex);

	MCommand* command = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_MAPTOD), MUID(0, 0), m_This);
	command->AddParameter(new MCommandParameterUID(matchStage));
	command->AddParameter(new MCommandParameterInt(lightingIndex));
	RouteToStage(matchStage, command);
}

void MMatchServer::StageRelayMapBattleStart(const MUID& uidPlayer, const MUID& uidStage)
{// ¸±·¹ÀÌ¸Ê ¼±ÅÃÇÏ°E°ÔÀÓ ½ÃÀÛ ¹öÆ° ´©¸£¸E´ÙÀ½À» ¼öÇàÇÑ´Ù
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetMasterUID() != uidPlayer) return;
	if(!pStage->IsRelayMap()) return;
	
	// °ÔÀÓ Ã³À½½ÃÀÛ½Ã ÃÊ±âÈ­ ÇØÁÖ±E
	pStage->InitCurrRelayMap();

	if (pStage->m_vecRelayMapsRemained.empty()) return;

	if((int)pStage->m_vecRelayMapsRemained.size() > MAX_RELAYMAP_LIST_COUNT)
	{// ¸Ê ±¸¼ºÀÌ 20°³ ÃÊ°úÇÏ¸E¿¡·¯
		mlog("RelayMapBattleStart Fail RelayMapList MIsCorrect OverCount[%d] \n", (int)pStage->m_vecRelayMapsRemained.size());
		return;
	}

	if (pStage->m_vecRelayMapsRemained.size() != pStage->GetRelayMapListCount())
	{
		mlog("m_vecRelayMapsRemained[%d] != GetRelayMapListCount[%d]\n", (int)pStage->m_vecRelayMapsRemained.size(), pStage->GetRelayMapListCount());
		return;
	}

	// Ã³À½ ½ÇÇàÇÒ ¸ÊÀ» Á¤ÇÑ´Ù
	int nRelayMapIndex = 0;
	if(pStage->GetRelayMapType() == RELAY_MAP_TURN )
		nRelayMapIndex = 0; // ³²Àº°ÍÁß¿¡¼­ Ã³À½¹øÂ° ºÎÅÍ ½ÃÀÛ(°¡µ¶¼º)
	else if(pStage->GetRelayMapType() == RELAY_MAP_RANDOM)
		nRelayMapIndex = rand() % int(pStage->m_vecRelayMapsRemained.size());

	if(MMATCH_MAP_RELAYMAP == pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID)
	{
		mlog("RelayMapBattleStart Fail Type[%d], RoundCount[Curr:%d][%d], ListCount[Curr:%d][%d] \n",  
			pStage->GetRelayMapType(), pStage->m_RelayMapRepeatCountRemained, pStage->GetRelayMapRepeatCount(), (int)pStage->m_vecRelayMapsRemained.size(), pStage->GetRelayMapListCount());
		return;
	}

	char* szMapName = (char*)MGetMapDescMgr()->GetMapName(pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID);
	if (!szMapName)
	{
		mlog("RelayMapBattleStart Fail MapID[%d] \n", (int)pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID);
		return;
	}

	pStage->GetStageSetting()->SetMapName(szMapName);

	// ½ÇÇàÇÑ ¸±·¹ÀÌ¸ÊÀº »èÁ¦ÇØÁØ´Ù.
	vector<RelayMap>::iterator itor = pStage->m_vecRelayMapsRemained.begin();
	for(int i=0 ; nRelayMapIndex > i ; ++itor, ++i);// ÇØ´EÀÎµ¦½º±ûÝEÀÌµ¿
	pStage->m_vecRelayMapsRemained.erase(itor);
}

void MMatchServer::OnStageRelayMapElementUpdate(const MUID& uidStage, int nType, int nRepeatCount)
{
	MMatchStage* pStage = FindStage(uidStage);

	if (pStage == NULL) return;
	if (!pStage->IsRelayMap()) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;

	pStage->SetRelayMapType((RELAY_MAP_TYPE)nType);
	pStage->SetRelayMapRepeatCount((RELAY_MAP_REPEAT_COUNT)nRepeatCount);

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_RELAY_MAP_ELEMENT_UPDATE), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapType()));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapRepeatCount()));
	RouteToStage(uidStage, pNew);
}

void MMatchServer::OnStageRelayMapListUpdate(const MUID& uidStage, int nRelayMapType, int nRelayMapRepeatCount, void* pRelayMapListBlob)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if(!pStage->IsRelayMap()) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;	// ´Eâ»óÅÂ¿¡¼­¸¸ ¹Ù²Ü¼EÀÖ´Ù

	// ¸±·¹ÀÌ¸Ê Á¤º¸¸¦ ¼­¹öÂÊ ½ºÅ×ÀÌÁö¸¦ °»½Å
	RelayMap relayMapList[MAX_RELAYMAP_LIST_COUNT];
	for (int i = 0; i < MAX_RELAYMAP_LIST_COUNT; i++)
		relayMapList[i].nMapID = -1;
	int nRelayMapListCount = MGetBlobArrayCount(pRelayMapListBlob);
	if(nRelayMapListCount > MAX_RELAYMAP_LIST_COUNT)
		nRelayMapListCount = MAX_RELAYMAP_LIST_COUNT;
	for (int i = 0; i < nRelayMapListCount; i++)
	{
		MTD_RelayMap* pRelayMap = (MTD_RelayMap*)MGetBlobArrayElement(pRelayMapListBlob, i);
		if(!MGetMapDescMgr()->MIsCorrectMap(pRelayMap->nMapID))
		{
			mlog("OnStageRelayMapListUpdate Fail MIsCorrectMap ID[%d] \n", (int)pRelayMap->nMapID);
			break;
		}
		relayMapList[i].nMapID = pRelayMap->nMapID;
	}

	pStage->SetRelayMapType((RELAY_MAP_TYPE)nRelayMapType);
	pStage->SetRelayMapRepeatCount((RELAY_MAP_REPEAT_COUNT)nRelayMapRepeatCount);
	pStage->SetRelayMapList(relayMapList);
	pStage->InitCurrRelayMap();


	// ºúÓ° ¸¸µé±E ¸Ê¸®½ºÆ® ¼¼ÆÃ
	pRelayMapListBlob = MMakeBlobArray(sizeof(MTD_RelayMap), pStage->GetRelayMapListCount());
	RelayMap RelayMapList[MAX_RELAYMAP_LIST_COUNT];
	memcpy(RelayMapList, pStage->GetRelayMapList(), sizeof(RelayMap)*MAX_RELAYMAP_LIST_COUNT);
	for (int i = 0; i < pStage->GetRelayMapListCount(); i++)
	{
		MTD_RelayMap* pRelayMapList = (MTD_RelayMap*)MGetBlobArrayElement(pRelayMapListBlob, i);
		pRelayMapList->nMapID = RelayMapList[i].nMapID;
	}

	// ¹æÀåÀÌ º¸³½ ¸±·¹ÀÌ¸Ê Á¤º¸¸¦ ¹æ¿øµé¿¡°Ô º¸³¿
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapType()));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapRepeatCount()));
	pNew->AddParameter(new MCommandParameterBlob(pRelayMapListBlob, MGetBlobArraySize(pRelayMapListBlob)));
	RouteToStage(uidStage, pNew);
}
void MMatchServer::OnStageRelayMapListInfo(const MUID& uidStage, const MUID& uidChar)
{
	MMatchStage* pStage = FindStage(uidStage);
	if(pStage == NULL) return;
	if(!pStage->IsRelayMap()) return;
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;
	// ´Eâ»óÅÂÀÏ¶§ ¹æÀåÀº Ã³¸® ¾ÈÇØÁÜ(¸±·¹ÀÌ¸Ê ÀÛ¼ºÁßÀÏ¼öµµ ÀÖÀ½)
	if(pStage->GetState() == STAGE_STATE_STANDBY && pStage->GetMasterUID() == uidChar) return;	

	// ºúÓ° ¸¸µé±E ¸Ê¸®½ºÆ® ¼¼ÆÃ
	void* pRelayMapListBlob = MMakeBlobArray(sizeof(MTD_RelayMap), pStage->GetRelayMapListCount());
	RelayMap RelayMapList[MAX_RELAYMAP_LIST_COUNT];
	memcpy(RelayMapList, pStage->GetRelayMapList(), sizeof(RelayMap)*MAX_RELAYMAP_LIST_COUNT);
	for (int i = 0; i < pStage->GetRelayMapListCount(); i++)
	{
		MTD_RelayMap* pRelayMapList = (MTD_RelayMap*)MGetBlobArrayElement(pRelayMapListBlob, i);
		pRelayMapList->nMapID = RelayMapList[i].nMapID;
	}
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapType()));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapRepeatCount()));
	pNew->AddParameter(new MCommandParameterBlob(pRelayMapListBlob, MGetBlobArraySize(pRelayMapListBlob)));
	MEraseBlobArray(pRelayMapListBlob);

	RouteToListener(pObj, pNew); // ¹æÀåÀÌ ¸±·¹ÀÌ¸Ê ¼³Á¤Áß¿¡ ¾÷µ¥ÀÌÆ®µÈ ¼³Á¤À¸·Î º¯°EµÉ¼ö°¡ ÀÖÀ½
}

void MMatchServer::OnStageSetting(const MUID& uidPlayer, const MUID& uidStage, void* pStageBlob, int nStageCount)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;	// ´Eâ»óÅÂ¿¡¼­¸¸ ¹Ù²Ü¼EÀÖ´Ù
	if (nStageCount <= 0) return;

	// validate
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) {
		mlog(" stage setting invalid object (%d, %d) ignore\n", uidPlayer.High, uidPlayer.Low);
		return;
	}

	if( pObj->GetStageUID()!=uidStage ||  nStageCount!=1 ||
		MGetBlobArraySize(pStageBlob) != (sizeof(MSTAGE_SETTING_NODE)+sizeof(int)*2) )
	{
		mlog(" stage setting hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);
		LogObjectCommandHistory( uidPlayer );
		return;
	}

	// ¹æÀåÀÌ°Å³ª ¿ûÛµÀÚ°¡ ¾Æ´Ñµ¥ ¼¼ÆÃÀ» ¹Ù²Ù¸E±×³É ¸®ÅÏ
	if (pStage->GetMasterUID() != uidPlayer)
	{
		MMatchObject* pObjMaster = GetObject(uidPlayer);
		if ((!IsAdminGrade(pObjMaster))) return;
	}


	MSTAGE_SETTING_NODE* pNode = (MSTAGE_SETTING_NODE*)MGetBlobArrayElement(pStageBlob, 0);

	// let's refactor
	if( (pNode->nGameType < MMATCH_GAMETYPE_DEATHMATCH_SOLO) || (pNode->nGameType >= MMATCH_GAMETYPE_MAX)) {
		mlog(" stage setting game mode hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);
		LogObjectCommandHistory( uidPlayer );

		// µðºñ¿¡ ³²±âÀÚ.
//		pObj->SetInvalidStageSettingDisconnectWaitInfo();
		pObj->DisconnectHacker( MMHT_INVALIDSTAGESETTING );

		return;
	}

	// ¼­¹ÙÀÌ¹úÀÌ ºñÈ°¼º ¼¼ÆÃÀÎµ¥ ¼­¹ÙÀÌ¹E¿äÃ»½Ã
	if( MGetServerConfig()->IsEnabledSurvivalMode()==false && pNode->nGameType==MMATCH_GAMETYPE_SURVIVAL) {
		mlog(" stage setting game mode hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);
		LogObjectCommandHistory( uidPlayer );
		pObj->DisconnectHacker( MMHT_INVALIDSTAGESETTING );
		return;
	}

	// ±âº»ÀûÀ¸·Î ÃÖ´EÀÎ¿øÀÌ STAGE_BASIC_MAX_PLAYERCOUNTÀÌ ³ÑÀ¸¸ESTAGE_BASIC_MAX_PLAYERCOUNT·Î ¸ÂÃçÁÜ.
	// ³²Àº ÀÛ¾÷À» ÁøÇàÇÏ¸éÀº °¢ °ÔÀÓ¿¡ ¸Â´Â ÀÎ¿øÀ¸·Î ¼ÂÆÃÀ» ÇÔ. - by SungE 2007-05-14
	if( STAGE_MAX_PLAYERCOUNT < pNode->nMaxPlayers )
		pNode->nMaxPlayers = STAGE_MAX_PLAYERCOUNT;

	// ÀÌ ÀÌ»óÀÇ ¶ó¿ûÑE¼ÂÆÃÀº ºÒ°¡´É ÇÏ´Ù. ¹«Á¶°Ç º¸Á¤ÇÑ´Ù. - By SungE 2007-11-07
	if( STAGE__MAX_ROUND < pNode->nRoundMax )
		pNode->nRoundMax = STAGE__MAX_ROUND;
#ifdef _TYPENET
	pNode->Netcode = NetcodeType::P2PAntilead;
#endif
	MMatchStageSetting* pSetting = pStage->GetStageSetting();
	MMatchChannel* pChannel = FindChannel(pStage->GetOwnerChannel());

	bool bCheckChannelRule = true;

	if (QuestTestServer())
	{
		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType) || pNode->nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE)
		{
			bCheckChannelRule = false;
		}
	}

	if ((pChannel) && (bCheckChannelRule))
	{
		// ¼¼ÆÃÇÒ ¼EÀÖ´Â ¸Ê, °ÔÀÓÅ¸ÀÔÀÎÁEÃ¼Å©
		MChannelRule* pRule = MGetChannelRuleMgr()->GetRule(pChannel->GetRuleType());
		if (pRule)
		{
			if (!pRule->CheckGameType(pNode->nGameType))
			{
				pNode->nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
			}

			bool bDuelMode = false;
			bool bCTFMode = false;

#ifdef _LIGHTURNOFF
			bool bLGHMode = false;
#endif

			if ( pNode->nGameType == MMATCH_GAMETYPE_DUEL)
				bDuelMode = true;

			if ( pNode->nGameType == MMATCH_GAMETYPE_CTF)
				bCTFMode = true;

#ifdef _LIGHTURNOFF
			//mlog("pRules\n");
			if (!pRule->CheckLightMap(pNode->szMapName))
			{
				//mlog("passing by\n");
				strcpy(pNode->szMapName, MGetMapDescMgr()->GetMapName(MMATCH_MAP_MANSION));
				pNode->nMapIndex = 0;
				pNode->bLightMapTurn = true;
			}
#endif

			if (bCTFMode)
			{
				// map index sucks (broken)
				if (!pRule->CheckCTFMap(pNode->szMapName))
				{
					strcpy(pNode->szMapName, MGetMapDescMgr()->GetMapName(MMATCH_MAP_MANSION));
					pNode->nMapIndex = 0;
				}
				else
				{
					strcpy(pNode->szMapName, pSetting->GetMapName());
					pNode->nMapIndex = pSetting->GetMapIndex();
				}
			}
			else
			{
				if (!pRule->CheckMap(pNode->nMapIndex, bDuelMode))
				{
					strcpy(pNode->szMapName, MGetMapDescMgr()->GetMapName(MMATCH_MAP_MANSION));
					pNode->nMapIndex = 0;
				}
				else
				{
					strcpy(pNode->szMapName, pSetting->GetMapName());
					pNode->nMapIndex = pSetting->GetMapIndex();
				}
			}
		}
	}

	// Custom: don't start quest games which contain roomtags. Not the most elegant solution but it works.
	if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType) || pNode->nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE || pNode->nGameType == MMATCH_GAMETYPE_SKILLMAP || pNode->nGameType == MMATCH_GAMETYPE_TURBO || pNode->nGameType == MMATCH_GAMETYPE_PAINTBALL_TEAM || pNode->nGameType == MMATCH_GAMETYPE_PAINTBALL_SOLO)
	{
		if (strstr(pStage->GetName(), "[") || strstr(pStage->GetName(), "]")) {
			MCommand* pCmd = CreateCommand(MC_MATCH_ANNOUNCE, MUID(0, 0));
			pCmd->AddParameter(new MCmdParamUInt(0));
			pCmd->AddParameter(new MCmdParamStr("Roomtags can not be used in these mode."));
			RouteToStage(pStage->GetUID(), pCmd);
			
			pNode->nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
		}
	}
	// Custom: No Teleport
	if (MGetGameTypeMgr()->IsTeamGame(pNode->nGameType) ||
		pNode->nGameType == MMATCH_GAMETYPE_DUEL || 
		pNode->nGameType == MMATCH_GAMETYPE_GUNGAME ||
		pNode->nGameType == MMATCH_GAMETYPE_DEATHMATCH_SOLO ||
		pNode->nGameType == MMATCH_GAMETYPE_BERSERKER ||
		pNode->nGameType == MMATCH_GAMETYPE_SKILLMAP ||
		pNode->nGameType == MMATCH_GAMETYPE_TURBO ||
		pNode->nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO ||
		pNode->nGameType == MMATCH_GAMETYPE_BERSERKER ||
		pNode->nGameType == MMATCH_GAMETYPE_SPY)
	{
		if (strstr(pStage->GetName(), "[tele]"))
		{
			MCommand* pCmd = CreateCommand(MC_MATCH_ANNOUNCE, MUID(0, 0));
			pCmd->AddParameter(new MCmdParamUInt(0));
			pCmd->AddParameter(new MCmdParamStr("This TAG is only available for Training!"));
			RouteToStage(pStage->GetUID(), pCmd);

			pNode->nGameType = MMATCH_GAMETYPE_TRAINING;
		}
		if (strstr(pStage->GetName(), "[ins]"))
		{
			MCommand* pCmd = CreateCommand(MC_MATCH_ANNOUNCE, MUID(0, 0));
			pCmd->AddParameter(new MCmdParamUInt(0));
			pCmd->AddParameter(new MCmdParamStr("This TAG is only available for DeathMatch!"));
			RouteToStage(pStage->GetUID(), pCmd);

			pNode->nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
		}
	}
	MMATCH_GAMETYPE nLastGameType = pSetting->GetGameType();

	//if (pNode->nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE)
	//{
	//	MNewQuestScenarioManager* pScenario = MMatchRuleQuestChallenge::GetScenarioMgr();
	//	MNewQuestScenario* pItem = pScenario->GetScenario(pScenario->GetDefaultScenarioName());
	//	if (pItem == nullptr) {
	//		mlog("error getting scenario %s\n", pScenario->GetDefaultScenarioName());
	//	}
	//	else
	//	{
	//		pNode->nMaxPlayers = pItem->GetMaxPlayers();
	//		//pNode->nLimitLevel = pItem->GetLevelLimit();
	//	}
	//}
    //Custom: Fixed Late Join Setting Stage
	//Custom: Late Join Setting
	if (MGetGameTypeMgr()->IsSpyGame(pNode->nGameType))
	{
#ifdef _ROTATION
		pNode->bTeamAlternateSpawns = false;
#endif
		pNode->bObserverEnabled = false;
		pNode->bAutoTeamBalancing = false;
		pNode->nMaxPlayers = 12;
#ifdef _SPEC 1
		pNode->bForcedEntryEnabled = true;
#endif
		pNode->Netcode = NetcodeType::P2PAntilead;
	}
	if (MGetGameTypeMgr()->IsTeamGame(pNode->nGameType))
	{
		pNode->bForcedEntryEnabled = MMATCH_DEFAULT_STAGESETTING_FORCEDENTRY;
#ifdef _SPEC 1
		pNode->bObserverEnabled = false;
#endif
	}
	if (MGetGameTypeMgr()->IsSoloGame(pNode->nGameType))
	{
		pNode->bForcedEntryEnabled = true;
		pNode->bTeamAlternateSpawns = false;
	}
	if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
	{
		pNode->bForcedEntryEnabled = false;
		pNode->nMaxPlayers = STAGE_QUEST_MAX_PLAYER;
		pNode->nLimitTime = STAGESETTING_LIMITTIME_UNLIMITED;
		pNode->bTeamAlternateSpawns = false;
		pNode->Netcode = NetcodeType::P2PLead;
#ifdef _SPEC 1
		pNode->bObserverEnabled = false;
#endif
		pNode->bIsLeading = true;
	}
	if (MGetGameTypeMgr()->IsQuestChallengeOnly(pNode->nGameType))
	{
		pNode->bForcedEntryEnabled = false;
		pNode->nMaxPlayers = STAGE_QUEST_MAX_PLAYER;
		pNode->nLimitTime = STAGESETTING_LIMITTIME_UNLIMITED;
		pNode->bTeamAlternateSpawns = false;
#ifdef _SPEC 1
		pNode->bObserverEnabled = false;
#endif
		pNode->bIsLeading = true;
		pNode->Netcode = NetcodeType::P2PLead;
	}
	pStage->SetIsRelayMap(strcmp(MMATCH_MAPNAME_RELAYMAP, pNode->szMapName) == 0);
	pStage->SetIsStartRelayMap(false);

	if(!pStage->IsRelayMap())
	{	// ¸±·¹ÀÌ¸ÊÀÌ ¾Æ´Ï¸E±âº»À¸·Î ÃÊ±âÈ­ ÇØÁØ´Ù.
		pNode->bIsRelayMap = pStage->IsRelayMap();
		pNode->bIsStartRelayMap = pStage->IsStartRelayMap();
		for (int i=0; i<MAX_RELAYMAP_LIST_COUNT; ++i)
			pNode->MapList[i].nMapID = -1;
		pNode->nRelayMapListCount = 0;
		pNode->nRelayMapType = RELAY_MAP_TURN;
		pNode->nRelayMapRepeatCount = RELAY_MAP_3REPEAT;
	}

	MMatchRoomModifier savedMods[MMOD_END];
	int nSavedModCount = pSetting->GetStageSetting()->nRoomModCount;
	memcpy(savedMods, pSetting->GetStageSetting()->RoomMods, sizeof(savedMods));

	pSetting->UpdateStageSetting(pNode);
	pSetting->SetPrivate(pStage->IsPrivate());	// co phong mat khau chi server quyet, client gui len khong tinh

	if (pNode->nRoomModCount <= 0)
		pSetting->SetRoomModsFromArray(savedMods, nSavedModCount);
	pStage->ChangeRule(pNode->nGameType);


	MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
	RouteToStage(uidStage, pCmd);


	// °ÔÀÓ ¸ðµå°¡ º¯°æµÇ¾úÀ»°æ¿E±âÁ¸ ¸ÊÀÌ ºÎÀûÀýÇÏ´Ù¸E¸ðµå¿¡ ¸Â´Â ¸ÊÀ¸·Î ¹Ù²ãÁØ´Ù
	if (nLastGameType != pSetting->GetGameType())
	{
		char szNewMap[ MAPNAME_LENGTH ] = {0};

		// Æ¯¼öÇÑ ¸ðµåµE(¸Ê ¸ñ·ÏÀÌ À¯º°³­ °ÍµE
		bool bLast_QuestOrSurvival = MGetGameTypeMgr()->IsQuestDerived( nLastGameType );
		bool bCurr_QuestOrSurvival = MGetGameTypeMgr()->IsQuestDerived( pSetting->GetGameType() );
		
		bool bLast_Duel = nLastGameType == MMATCH_GAMETYPE_DUEL;
		bool bCurr_Duel = pSetting->GetGameType() == MMATCH_GAMETYPE_DUEL;

		bool bLast_ChallengeQuest = nLastGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE;
		bool bCurr_ChallengeQuest = pSetting->GetGameType() == MMATCH_GAMETYPE_QUEST_CHALLENGE;

		// ±×¿Ü ÀÏ¹ÝÀûÀÎ ¸ðµåµE(µ¥½º¸ÅÄ¡ µE
		bool bLast_Other = !bLast_QuestOrSurvival && !bLast_Duel && !bLast_ChallengeQuest && nLastGameType != MMATCH_GAMETYPE_SKILLMAP;
		bool bCurr_Other = !bCurr_QuestOrSurvival && !bCurr_Duel && !bCurr_ChallengeQuest && pSetting->GetGameType() != MMATCH_GAMETYPE_SKILLMAP;

		bool bLast_SkillMap = nLastGameType == MMATCH_GAMETYPE_SKILLMAP;
		bool bCurr_SkillMap = pSetting->GetGameType() == MMATCH_GAMETYPE_SKILLMAP;
#ifdef _MAGICBOX
		bool bLast_Magic = nLastGameType == MMATCH_GAMETYPE_DROPMAGIC;
		bool bCurr_Magic = pSetting->GetGameType() == MMATCH_GAMETYPE_DROPMAGIC;
#endif

#ifdef _PAINTMODE
		bool bLast_Paint = nLastGameType == MMATCH_GAMETYPE_PAINTBALL_SOLO;
		bool bCurr_Paint = pSetting->GetGameType() == MMATCH_GAMETYPE_PAINTBALL_SOLO;

		bool bLast_PaintTeam = nLastGameType == MMATCH_GAMETYPE_PAINTBALL_TEAM;
		bool bCurr_PaintTeam = pSetting->GetGameType() == MMATCH_GAMETYPE_PAINTBALL_TEAM;
#endif

		// Äù½ºÆ®³ª ¼­¹ÙÀÌ¹ú¿¡ ¸Â´Â ¸ÊÀ¸·Î º¯°E
		if ( !bLast_QuestOrSurvival && bCurr_QuestOrSurvival)
		{
			OnStageMap(uidStage, MMATCH_DEFAULT_STAGESETTING_MAPNAME, uidPlayer);

			MMatchRuleBaseQuest* pQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule());
			pQuest->RefreshStageGameInfo();
		}
		// µà¾ó¿¡ ¸Â´Â ¸ÊÀ¸·Î º¯°E
		else if ( !bLast_Duel && bCurr_Duel)
		{
			strcpy( szNewMap, MGetMapDescMgr()->GetMapName( MMATCH_MAP_HALL));
			OnStageMap(uidStage, szNewMap, uidPlayer);
		}
		else if (!bLast_SkillMap && bCurr_SkillMap) {
			strcpy_s(szNewMap, MGetMapDescMgr()->GetMapName(MMATCH_MAP_SKILLMAP));
			OnStageMap(uidStage, szNewMap, uidPlayer);
		}
#ifdef _MAGICBOX
		else if (!bLast_Magic && bCurr_Magic) {
			strcpy_s(szNewMap, MGetMapDescMgr()->GetMapName(MMATCH_MAP_MANSION));
			OnStageMap(uidStage, szNewMap, uidPlayer);
		}
#endif

#ifdef _PAINTMODE
		else if (!bLast_Paint && bCurr_Paint) {
			strcpy_s(szNewMap, MGetMapDescMgr()->GetMapName(MMATCH_MAP_TOWN));
			OnStageMap(uidStage, szNewMap, uidPlayer);
		}

		else if (!bLast_PaintTeam && bCurr_PaintTeam) {
			strcpy_s(szNewMap, MGetMapDescMgr()->GetMapName(MMATCH_MAP_TOWN));
			OnStageMap(uidStage, szNewMap, uidPlayer);
		}

#endif	
		else if ( !bLast_ChallengeQuest && bCurr_ChallengeQuest)
		{
			MNewQuestScenarioManager* pScenarioMgr = MMatchRuleQuestChallenge::GetScenarioMgr();
			_ASSERT(pScenarioMgr);
			if (pScenarioMgr)
				OnStageMap(uidStage, (char*)pScenarioMgr->GetDefaultScenarioName(), uidPlayer);
		}
		// À§ÀÇ ¸ðµåµé¿¡¼­ ±âÅ¸ ¸ðµå·Î ¹Ù²E°æ¿EMansionÀ¸·Î ¼¼ÆÃ
		else if ( !bLast_Other && bCurr_Other)
		{
			strcpy( szNewMap, MGetMapDescMgr()->GetMapName( MMATCH_MAP_MANSION));
			OnStageMap(uidStage, szNewMap, uidPlayer);
		}

		//if (MGetGameTypeMgr()->IsQuestDerived( nLastGameType ) == false &&
		//	MGetGameTypeMgr()->IsQuestDerived( pSetting->GetGameType() ) == true)
		//{
		//	OnStageMap(uidStage, MMATCH_DEFAULT_STAGESETTING_MAPNAME);

		//	MMatchRuleBaseQuest* pQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule());
		//	pQuest->RefreshStageGameInfo();
		//}
		//else if ( (nLastGameType != MMATCH_GAMETYPE_DUEL) && ( pSetting->GetGameType() == MMATCH_GAMETYPE_DUEL))
		//{
		//	strcpy( szNewMap, MGetMapDescMgr()->GetMapName( MMATCH_MAP_HALL));
		//	OnStageMap(uidStage, szNewMap);
		//}
		//else if ( ((nLastGameType == MMATCH_GAMETYPE_QUEST) || (nLastGameType == MMATCH_GAMETYPE_SURVIVAL) || (nLastGameType == MMATCH_GAMETYPE_DUEL)) &&
		//	      ((pSetting->GetGameType() != MMATCH_GAMETYPE_QUEST) && (pSetting->GetGameType() != MMATCH_GAMETYPE_SURVIVAL) && ( pSetting->GetGameType() != MMATCH_GAMETYPE_DUEL)))
		//{
		//	strcpy( szNewMap, MGetMapDescMgr()->GetMapName( MMATCH_MAP_MANSION));
		//	OnStageMap(uidStage, szNewMap);
		//}
	}

}

void MMatchServer::OnRequestStageSetting(const MUID& uidComm, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
	pCmd->m_Receiver = uidComm;
	Post(pCmd);

	// ¸Ê ¼±ÅÃÀÌ ¸±·¹ÀÌ¸ÊÀÌ¸EÃ³¸®ÇØÁØ´Ù.
	OnStageRelayMapListInfo(uidStage, uidComm);

	MMatchObject* pChar = GetObject(uidComm);

	// Custom: Don't show firstmaster at all.
	//if (pChar && (MMUG_EVENTMASTER == pChar->GetAccountInfo()->m_nUGrade)) 	{
	//{
	//	// ÀÌº¥Æ® ¸¶½ºÅÍ¿¡°Ô Ã³À½ ¹æ¸¸µé¾ú´E»ç¶÷À» ¾Ë·ÁÁØ´Ù
	//	StageShowInfo(this, uidComm, uidStage, "/showinfo");
	//}
}

void MMatchServer::OnRequestPeerList(const MUID& uidChar, const MUID& uidStage)
{
	ResponsePeerList(uidChar, uidStage);
}

void MMatchServer::OnRequestGameInfo(const MUID& uidChar, const MUID& uidStage)
{
	ResponseGameInfo(uidChar, uidStage);
}

void MMatchServer::ResponseGameInfo(const MUID& uidChar, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage); if (pStage == NULL) return;
	MMatchObject* pObj = GetObject(uidChar); if (pObj == NULL) return;
	if (pStage->GetRule() == NULL) return;

	// Custom: Larger Game Info Packet, deprecate the old one
	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_GAME_INFO_V2, MUID(0,0));
	pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));

	void* pGameInfoArray = MMakeBlobArray(sizeof(MTD_GameInfo_V2), 1);
	MTD_GameInfo_V2* pGameItem = (MTD_GameInfo_V2*)MGetBlobArrayElement(pGameInfoArray, 0);
	memset(pGameItem, 0, sizeof(MTD_GameInfo_V2));
	
	if (pStage->GetStageSetting()->IsTeamPlay())
	{
		pGameItem->nRedTeamScore = pStage->GetTeamScore(MMT_RED);
		pGameItem->nBlueTeamScore = pStage->GetTeamScore(MMT_BLUE);

		pGameItem->nRedTeamKills = pStage->GetTeamKills(MMT_RED);
		pGameItem->nBlueTeamKills = pStage->GetTeamKills(MMT_BLUE);
	}

	// °ÔÀÓÁ¤º¸
	//void* pGameInfoArray = MMakeBlobArray(sizeof(MTD_GameInfo), 1);
	//MTD_GameInfo* pGameItem = (MTD_GameInfo*)MGetBlobArrayElement(pGameInfoArray, 0);
	//memset(pGameItem, 0, sizeof(MTD_GameInfo));
	//
	//if (pStage->GetStageSetting()->IsTeamPlay())
	//{
	//	pGameItem->nRedTeamScore = static_cast<char>(pStage->GetTeamScore(MMT_RED));
	//	pGameItem->nBlueTeamScore = static_cast<char>(pStage->GetTeamScore(MMT_BLUE));

	//	pGameItem->nRedTeamKills = static_cast<short>(pStage->GetTeamKills(MMT_RED));
	//	pGameItem->nBlueTeamKills = static_cast<short>(pStage->GetTeamKills(MMT_BLUE));
	//}

	pNew->AddParameter(new MCommandParameterBlob(pGameInfoArray, MGetBlobArraySize(pGameInfoArray)));
	MEraseBlobArray(pGameInfoArray);

	// ·E¤º¸
	void* pRuleInfoArray = NULL;
	if (pStage->GetRule())
		pRuleInfoArray = pStage->GetRule()->CreateRuleInfoBlob();
	if (pRuleInfoArray == NULL)
		pRuleInfoArray = MMakeBlobArray(0, 0);
	pNew->AddParameter(new MCommandParameterBlob(pRuleInfoArray, MGetBlobArraySize(pRuleInfoArray)));
	MEraseBlobArray(pRuleInfoArray);

	// Battle¿¡ µé¾ûÌ£ »ç¶÷¸¸ List¸¦ ¸¸µç´Ù.
	int nPlayerCount = pStage->GetObjInBattleCount();

	void* pPlayerItemArray = MMakeBlobArray(sizeof(MTD_GameInfoPlayerItem), nPlayerCount);
	int nIndex=0;
	for (MUIDRefCache::iterator itor=pStage->GetObjBegin(); itor!=pStage->GetObjEnd(); itor++) 
	{
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		if (pObj->GetEnterBattle() == false) continue;

		MTD_GameInfoPlayerItem* pPlayerItem = (MTD_GameInfoPlayerItem*)MGetBlobArrayElement(pPlayerItemArray, nIndex++);
		pPlayerItem->uidPlayer = pObj->GetUID();
		pPlayerItem->bAlive = pObj->CheckAlive();
		pPlayerItem->nKillCount = pObj->GetAllRoundKillCount();
		pPlayerItem->nDeathCount = pObj->GetAllRoundDeathCount();
	}
	pNew->AddParameter(new MCommandParameterBlob(pPlayerItemArray, MGetBlobArraySize(pPlayerItemArray)));
	MEraseBlobArray(pPlayerItemArray);

	RouteToListener(pObj, pNew);
}

void MMatchServer::OnMatchLoadingComplete(const MUID& uidPlayer, int nPercent)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_LOADING_COMPLETE, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidPlayer));
	pCmd->AddParameter(new MCmdParamInt(nPercent));
	RouteToStage(pObj->GetStageUID(), pCmd);	
}
#ifdef _SPEC 1
void MMatchServer::ProcessOnGameKill(MMatchStage* pStage, MMatchObject* pAttacker, MMatchObject* pVictim)
{
	bool bSuicide = false;
	if (pAttacker == pVictim) bSuicide = true;

	int nAttackerExp = 0;
	int nVictimExp = 0;
	int nAttackerLevel = pAttacker->GetCharInfo()->m_nLevel;
	int nVictimLevel = pVictim->GetCharInfo()->m_nLevel;
	MMATCH_GAMETYPE nGameType = pStage->GetStageSetting()->GetGameType();
	float fGameExpRatio = MGetGameTypeMgr()->GetInfo(nGameType)->fGameExpRatio;
	MUID uidStage = pAttacker->GetStageUID();

	CalcExpOnGameKill(pStage, pAttacker, pVictim, &nAttackerExp, &nVictimExp);

	PostGameDeadOnGameKill(uidStage, pAttacker, pVictim, nAttackerExp, nVictimExp);

	if (nGameType == MMATCH_GAMETYPE_TRAINING || nGameType == MMATCH_GAMETYPE_TEAM_TRAINING) return;

	pAttacker->GetCharInfo()->IncXP(nAttackerExp);
	pVictim->GetCharInfo()->DecXP(nVictimExp);

	int nNewAttackerLevel = -1, nNewVictimLevel = -1;
	if ((!bSuicide) && (pAttacker->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pAttacker->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nAttackerLevel)))
	{
		nNewAttackerLevel = MMatchFormula::GetLevelFromExp(pAttacker->GetCharInfo()->m_nXP);
		if (nNewAttackerLevel != pAttacker->GetCharInfo()->m_nLevel) pAttacker->GetCharInfo()->m_nLevel = nNewAttackerLevel;
	}

	if ((pVictim->GetCharInfo()->m_nLevel > 0) &&
		(pVictim->GetCharInfo()->m_nXP < MMatchFormula::GetNeedExp(nVictimLevel - 1)))
	{
		nNewVictimLevel = MMatchFormula::GetLevelFromExp(pVictim->GetCharInfo()->m_nXP);
		if (nNewVictimLevel != pVictim->GetCharInfo()->m_nLevel) pVictim->GetCharInfo()->m_nLevel = nNewVictimLevel;
	}

	if (!bSuicide)
	{
		pAttacker->GetCharInfo()->IncKill();

		const int nBPBonus = CalcBPonGameKill(pStage, pAttacker, nAttackerLevel, nVictimLevel);

		pAttacker->GetCharInfo()->IncBP(nBPBonus);
	}

	pVictim->GetCharInfo()->IncDeath();

	if (pAttacker->GetCharInfo()->GetDBCachingData()->IsRequestUpdate())
	{
		UpdateCharDBCachingData(pAttacker);
	}
	if (pVictim->GetCharInfo()->GetDBCachingData()->IsRequestUpdate())
	{
		UpdateCharDBCachingData(pAttacker);
	}

	if ((!bSuicide) && (nNewAttackerLevel >= 0) && (nNewAttackerLevel != nAttackerLevel))
	{
		UpdateCharDBCachingData(pAttacker);

		pAttacker->GetCharInfo()->m_nLevel = nNewAttackerLevel;
		if (!GetDBMgr()->UpdateCharLevel(pAttacker->GetCharInfo()->m_nCID,
			nNewAttackerLevel,
			pAttacker->GetCharInfo()->m_nBP,
			pAttacker->GetCharInfo()->m_nTotalKillCount,
			pAttacker->GetCharInfo()->m_nTotalDeathCount,
			pAttacker->GetCharInfo()->m_nTotalPlayTimeSec,
			true))
		{
			mlog("DB UpdateCharLevel Error : %s\n", pAttacker->GetCharInfo()->m_szName);
		}
	}
	if ((nNewVictimLevel >= 0) && (nNewVictimLevel != nVictimLevel))
	{
		UpdateCharDBCachingData(pVictim);

		pVictim->GetCharInfo()->m_nLevel = nNewVictimLevel;
		if (!GetDBMgr()->UpdateCharLevel(pVictim->GetCharInfo()->m_nCID,
			nNewVictimLevel,
			pVictim->GetCharInfo()->m_nBP,
			pVictim->GetCharInfo()->m_nTotalKillCount,
			pVictim->GetCharInfo()->m_nTotalDeathCount,
			pVictim->GetCharInfo()->m_nTotalPlayTimeSec,
			false
		))
		{
			mlog("DB UpdateCharLevel Error : %s\n", pVictim->GetCharInfo()->m_szName);
		}
	}

	if ((!bSuicide) && (nNewAttackerLevel >= 0) && (nNewAttackerLevel > nAttackerLevel))
	{
		MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_UP, MUID(0, 0));
		pCmd->AddParameter(new MCommandParameterUID(pAttacker->GetUID()));
		pCmd->AddParameter(new MCommandParameterInt(nNewAttackerLevel));
		RouteToBattle(uidStage, pCmd);

	}
	if ((nNewVictimLevel >= 0) && (nNewVictimLevel < nVictimLevel))
	{
		MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_DOWN, MUID(0, 0));
		pCmd->AddParameter(new MCommandParameterUID(pVictim->GetUID()));
		pCmd->AddParameter(new MCommandParameterInt(nNewVictimLevel));
		RouteToBattle(uidStage, pCmd);
	}
}
void MMatchServer::OnGameKill(const MUID& uidAttacker, const MUID& uidVictim)
{
	MMatchObject* pVictim = GetObject(uidVictim);
	MMatchObject* pAttacker = GetObject(uidAttacker);
	if (pVictim == NULL) return;

	MMatchStage* pStage = FindStage(pVictim->GetStageUID());
	if (pStage == NULL) return;

	if (pVictim->CheckAlive() == false) {
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SUICIDE, MUID(0, 0));
		int nResult = MOK;
		pNew->AddParameter(new MCommandParameterInt(nResult));
		pNew->AddParameter(new MCommandParameterUID(pVictim->GetUID()));
		RouteToBattle(pStage->GetUID(), pNew);
		return;
	}

	MUID TheAttackerUID = uidAttacker;

	if (pAttacker)
	{
		if (pAttacker->GetStageUID() != pVictim->GetStageUID())
		{
			pAttacker = NULL;
			TheAttackerUID = MUID(0, 0);
		}
	}

	pVictim->OnDead();

	if (TheAttackerUID != uidVictim)
	{
		if (pAttacker != NULL)
		{
			pAttacker->OnKill();
		}
	}

	if ((pAttacker != NULL) && (pVictim != NULL))
	{
		ProcessOnGameKill(pStage, pAttacker, pVictim);
	}

	pStage->OnGameKill(TheAttackerUID, uidVictim);
}
#endif
void MMatchServer::OnGameRoundState(const MUID& uidStage, int nState, int nRound)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	pStage->RoundStateFromClient(uidStage, nState, nRound);
}


void MMatchServer::OnDuelSetObserver(const MUID& uidChar)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_SET_OBSERVER, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidChar));
	RouteToBattle(pObj->GetStageUID(), pCmd);
}

void MMatchServer::OnRequestRoll(const MUID& uidRoller)
{
	MMatchObject* pObj = GetObject(uidRoller);
	if (pObj == NULL) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// Don't give rolls when hiding
	if ((IsAdminGrade(pObj) || pObj->GetAccountInfo()->m_nUGrade == MMUG_GAMEMASTER) && pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
		return;

	char szStageNameCopy[256]; //Thanks Linear for this code. :> 
	strcpy( szStageNameCopy, pStage->GetName() );

	for( int i = 0; i < strlen( szStageNameCopy ); ++i )
		szStageNameCopy[i] = tolower(szStageNameCopy[i]);
	
	if(strstr(szStageNameCopy, "[rtd]"))
	{
		char szMsg[256];
		memset(szMsg, 0, sizeof(szMsg));

		int rolledDice = 0;
		srand(timeGetTime());
		rolledDice = rand() % 11 + 1;

		switch(rolledDice)
		{
		case 1:
			sprintf_s( szMsg, "'%s' rolled 1: Zoomed Out.", pObj->GetName());
			break;
		case 2:
			sprintf_s( szMsg, "'%s' rolled 2: Near Death.", pObj->GetName());
			break;
		case 3:
			sprintf_s( szMsg, "'%s' rolled 3: ESP.", pObj->GetName());
			break;
		case 4:
			sprintf_s( szMsg, "'%s' rolled 4: Super Tank.", pObj->GetName());
			break;
		case 5:
			sprintf_s( szMsg, "'%s' rolled 5: Shield Mode.", pObj->GetName());
			break;
		case 6:
			sprintf_s( szMsg, "'%s' rolled 6: Ninja Jump.", pObj->GetName());
			break;
		case 7:
			sprintf_s( szMsg, "'%s' rolled 7: Speed Demon.", pObj->GetName());
			break;
		case 8:
			sprintf_s( szMsg, "'%s' rolled 8: Slow Down.", pObj->GetName());
			break;
		case 9:
			sprintf_s( szMsg, "'%s' rolled 9: Bunny Mode.", pObj->GetName());
			break;
		case 10:
			sprintf_s( szMsg, "'%s' rolled 10: Stronger Legs.", pObj->GetName());
			break;
		case 11:
			sprintf_s( szMsg, "'%s' rolled 11: Wireframe.", pObj->GetName());
		}

		MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_ROLL, MUID(0,0));
			pCmd->AddParameter(new MCmdParamUID(uidRoller));
			pCmd->AddParameter(new MCmdParamInt(rolledDice));
			pCmd->AddParameter(new MCmdParamStr(szMsg));
		RouteToStage(pStage->GetUID(), pCmd);
	}
}
void MMatchServer::OnAdminSBTest(const MUID& uidComm, char* pszTargetName)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);

	if (pObj == NULL) return;
	if (pObj->GetCharInfo() == NULL) return;
	if (pObj->GetStageUID().IsInvalid()) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);

	if (pTargetObj == NULL) return;
	if (pTargetObj->GetCharInfo() == NULL) return;
	if (pTargetObj->GetStageUID().IsInvalid()) return;

	if (pObj->GetStageUID() != pTargetObj->GetStageUID())
		return;

	// Event Team support
	if (IsAdminGrade(pObj) == false && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER)
		return;

	if (pObj->GetPlace() != MMP_BATTLE || pTargetObj->GetPlace() != MMP_BATTLE)
		return;

	MCommand* pCmd = CreateCommand(MC_ADMIN_SBTEST, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
	pCmd->AddParameter(new MCmdParamStr(pTargetObj->GetName()));
	RouteToListener(pTargetObj, pCmd);

	char szBuf[128];
	sprintf(szBuf, "Tested SB for '%s'.", pszTargetName);

	Announce(pObj, szBuf);
}
void MMatchServer::OnRequestSpawn(const MUID& uidChar, const MVector& pos, const MVector& dir)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	// Do Not Spawn when AdminHiding
#ifdef _SPEC
	if ((IsAdminGrade(pObj) && pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide)) || pObj->GetTeam() == MMT_SPECTATOR) 
	{
		// Do not spawn when admin-hiding or spectating
		return;
	}
#else
	if (IsAdminGrade(pObj) && pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide)) return;
#endif

	// ¸¶Áö¸· Á×¾ú´E½Ã°£°E»õ·Î ¸®½ºÆùÀ» ¿äÃ»ÇÑ ½Ã°£ »çÀÌ¿¡ 2ÃÊ ÀÌ»óÀÇ ½Ã°£ÀÌ ÀÖ¾ú´ÂÁE°Ë»çÇÑ´Ù.
	DWORD dwTime = timeGetTime() - pObj->GetLastSpawnTime();	
	if ( dwTime < RESPAWN_DELAYTIME_AFTER_DYING_MIN) return;
	pObj->SetLastSpawnTime(timeGetTime());

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;
	if ( (pStage->GetRule()->GetRoundState() != MMATCH_ROUNDSTATE_PREPARE) && (!pObj->IsEnabledRespawnDeathTime(GetTickTime())) )
		 return;

	MMatchRule* pRule = pStage->GetRule();
	MMATCH_GAMETYPE gameType = pRule->GetGameType();
	if (gameType == MMATCH_GAMETYPE_DUEL)
	{
		MMatchRuleDuel* pDuel = (MMatchRuleDuel*)pRule;
		if (uidChar != pDuel->uidChampion && uidChar != pDuel->uidChallenger)
		{
			OnDuelSetObserver(uidChar);
			return;
		}
	}
#ifdef _SPYMODE
	else if (gameType == MMATCH_GAMETYPE_SPY)
	{
		MMatchRuleSpy* pSpy = (MMatchRuleSpy*)pRule;
		if (!pSpy->IsParticipant(uidChar))
		{
			OnDuelSetObserver(uidChar);
			return;
		}
	}
#endif
	else if (MGetGameTypeMgr()->IsQuestDerived((gameType)))
	{
		MMatchRuleBaseQuest* pQst = (MMatchRuleBaseQuest*)pRule;
		if (pQst && pQst->GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
		{
			OnDuelSetObserver(uidChar);
			return;
		}
	}
	pObj->ResetCustomItemUseCount();
	pObj->SetAlive(true);

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_RESPONSE_SPAWN, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidChar));
	pCmd->AddParameter(new MCmdParamShortVector(pos.x, pos.y, pos.z));
	pCmd->AddParameter(new MCmdParamShortVector(DirElementToShort(dir.x), DirElementToShort(dir.y), DirElementToShort(dir.z)));
	RouteToBattle(pObj->GetStageUID(), pCmd);
}

void MMatchServer::OnGameRequestTimeSync(const MUID& uidComm, unsigned long nLocalTimeStamp)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	MMatchTimeSyncInfo* pSync = pObj->GetSyncInfo();
	pSync->Update(GetGlobalClockCount());

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_RESPONSE_TIMESYNC, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUInt(nLocalTimeStamp));
	pCmd->AddParameter(new MCmdParamUInt(GetGlobalClockCount()));
	RouteToListener(pObj, pCmd);
}

void MMatchServer::OnGameReportTimeSync(const MUID& uidComm, unsigned long nLocalTimeStamp, unsigned int nDataChecksum)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->UpdateTickLastPacketRecved();	// Last Packet Timestamp

	if (pObj->GetEnterBattle() == false)
		return;

	//// SpeedHack Test ////
	MMatchTimeSyncInfo* pSync = pObj->GetSyncInfo();
	int nSyncDiff = nLocalTimeStamp - pSync->GetLastSyncClock();
	float fError = static_cast<float>(nSyncDiff) / static_cast<float>(MATCH_CYCLE_CHECK_SPEEDHACK);
	if (fError > 2.f) {	
		pSync->AddFoulCount();
		if (pSync->GetFoulCount() >= 3) {	// 3¿¬¼Ó ½ºÇÇµåÇÙ °ËÃE- 3Áø¾Æ¿E

			#ifndef _DEBUG		// µð¹ö±×ÇÒ¶§´Â »©³õ¾ÒÀ½
				NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GAME_SPEEDHACK);
				StageLeave(pObj->GetUID());//, pObj->GetStageUID());
				Disconnect(pObj->GetUID());
			#endif
			mlog("SPEEDHACK : User='%s', SyncRatio=%f (TimeDiff=%d) \n", 
				pObj->GetName(), fError, nSyncDiff);
			pSync->ResetFoulCount();
		}
	} else {
		pSync->ResetFoulCount();
	}
	pSync->Update(GetGlobalClockCount());

	//// MemoryHack Test ////
	if (nDataChecksum > 0) {	// ¼­¹ö°¡ Client MemoryChecksum ¸ð¸£¹Ç·Î ÀÏ´Ü Å¬¶óÀÌ¾ðÆ®°¡ ½Å°úãÏ´ÂÀÇ¹Ì·Î »ç¿EÑ´Ù.
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GAME_MEMORYHACK);
		StageLeave(pObj->GetUID());//, pObj->GetStageUID());
		Disconnect(pObj->GetUID());
		mlog("MEMORYHACK : User='%s', MemoryChecksum=%u \n", pObj->GetName(), nDataChecksum);
	}
}

void MMatchServer::OnUpdateFinishedRound(const MUID& uidStage, const MUID& uidChar, 
						   void* pPeerInfo, void* pKillInfo)
{

}

void MMatchServer::OnRequestForcedEntry(const MUID& uidStage, const MUID& uidChar)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchObject* pObj = GetObject(uidChar);	
	if (pObj == NULL) return;

	pObj->SetForcedEntry(true);

	// Custom: Allow admins to spectate ladder matches
	// probably the best temporary fix I could provide so far without updating everything
	if (pStage->GetStageType() == MST_LADDER && pStage->GetStageType() == MST_LADDERWARS)
	{
		if (!pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
		{
			RouteResponseToListener(pObj, MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY, MERR_CANNOT_JOIN_STAGE_BY_FORCEDENTRY);
			return;
		}

		MCommand* pCmd = CreateCommand(MC_MATCH_LADDER_LAUNCH, uidChar);
		pCmd->AddParameter(new MCmdParamUID(uidStage));
		pCmd->AddParameter(new MCmdParamStr( const_cast<char*>(pStage->GetMapName()) ));
		RouteToListener(pObj, pCmd);
	}

	if (MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType()) || MGetGameTypeMgr()->IsQuestChallengeOnly(pStage->GetStageSetting()->GetGameType()))
	{
		if (pStage->m_pRule)
		{
			MUID uidChar = pObj->GetUID();
			pStage->m_pRule->OnQuestEnterBattle(uidChar);
		}
	}

	RouteResponseToListener(pObj, MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY, MOK);
}

void MMatchServer::OnRequestSuicide(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	pStage->ReserveSuicide( uidPlayer, MGetMatchServer()->GetGlobalClockCount() );

	// OnGameKill(uidPlayer, uidPlayer);

	//MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SUICIDE, MUID(0,0));
	//int nResult = MOK;
	//pNew->AddParameter(new MCommandParameterInt(nResult));
	//pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	//RouteToBattle(pObj->GetStageUID(), pNew);
}

void MMatchServer::OnRequestMoveToGamePortal(const MUID& uidPlayer, const int nItemUID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	pStage->ObtainPortalItem(pObj, nItemUID);
}

void MMatchServer::OnRequestObtainWorldItem(const MUID& uidPlayer, const int nItemUID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	pStage->ObtainWorldItem(pObj, nItemUID);
}
// Custom Fix Spy Landmine
void MMatchServer::OnRequestSpawnWorldItem(const MUID& uidPlayer, const int nItemID, const float x, const float y, const float z, float fDropDelayTime)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if (!pObj->IsHaveCustomItem())
		return;
	if (pObj->IncreaseCustomItemUseCount())
	{
		pStage->RequestSpawnWorldItem(pObj, nItemID, x, y, z, fDropDelayTime);
	}
}
//void MMatchServer::OnRequestSpawnWorldItem(const MUID& uidPlayer, const int nItemID, const float x, const float y, const float z, float fDropDelayTime)
//{
//	MMatchObject* pObj = GetObject(uidPlayer);
//	if (pObj == NULL) return;
//	MMatchStage* pStage = FindStage(pObj->GetStageUID());
//	if (pStage == NULL) return;
//
//	if (pStage->GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_SPY && nItemID == MSPYMODE_WORLDITEM_LANDMINE_ID)
//	{
//		pStage->RequestSpawnWorldItem(pObj, nItemID, x, y, z, fDropDelayTime);
//		return;
//	}
//
//	if (!pObj->IsHaveCustomItem())
//		return;
//}
#ifdef _PORTALGUN 1
void MMatchServer::OnRequestSpawnGamePortal(const MUID& uidPlayer, const int nItemID, const float x, const float y, const float z)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if (nItemID == PORTAL_BLUE_ID || nItemID == PORTAL_RED_ID)
	{
		int nWorldItemExtraValues[WORLDITEM_EXTRAVALUE_NUM];
		for (int i = 0; i < WORLDITEM_EXTRAVALUE_NUM; i++) nWorldItemExtraValues[i] = 0;
		pStage->SpawnServerSideWorldItem(pObj, nItemID, x, y, z, -1, nWorldItemExtraValues);
	}
}
#endif
void MMatchServer::OnNotifyThrowTrapItem(const MUID& uidPlayer, const int nItemID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if (!pObj->IsEquipCustomItem(nItemID))
		return;

	pStage->OnNotifyThrowTrapItem(uidPlayer, nItemID);
}

void MMatchServer::OnNotifyActivatedTrapItem(const MUID& uidPlayer, const int nItemID, const MVector3& pos)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	pStage->OnNotifyActivatedTrapItem(uidPlayer, nItemID, pos);
}

float MMatchServer::GetDuelVictoryMultiflier(int nVictorty)
{
	return 1.0f;
}

float MMatchServer::GetDuelPlayersMultiflier(int nPlayerCount)
{
	return 1.0f;
}

void MMatchServer::CalcExpOnGameKill(MMatchStage* pStage, MMatchObject* pAttacker, MMatchObject* pVictim, 
					   int* poutAttackerExp, int* poutVictimExp)
{
	bool bSuicide = false;		// ÀÚ»E
	if (pAttacker == pVictim) bSuicide = true;		

	MMATCH_GAMETYPE nGameType = pStage->GetStageSetting()->GetGameType();
	float fGameExpRatio = MGetGameTypeMgr()->GetInfo(nGameType)->fGameExpRatio;

	// °ÔÀÓÅ¸ÀÔÀÌ TrainingÀÌ¸E¹Ù·Î 0¸®ÅÏ
	// Custom: Game modes
	if (nGameType == MMATCH_GAMETYPE_TRAINING || nGameType == MMATCH_GAMETYPE_TEAM_TRAINING)
	{
		*poutAttackerExp = 0;
		*poutVictimExp = 0;
		return;
	}
	// °ÔÀÓÅ¸ÀÔÀÌ ¹ö¼­Ä¿ÀÏ °æ¿E
	else if (nGameType == MMATCH_GAMETYPE_BERSERKER)
	{
		MMatchRuleBerserker* pRuleBerserker = (MMatchRuleBerserker*)pStage->GetRule();

		if (pRuleBerserker->GetBerserker() == pAttacker->GetUID())
		{
			if (pAttacker != pVictim)
			{
				// ¹ö¼­Ä¿´Â °æÇèÄ¡¸¦ 80%¸¸ È¹µæÇÑ´Ù.
				fGameExpRatio = fGameExpRatio * 0.8f;
			}
			else
			{
				// ¹ö¼­Ä¿´Â ÀÚ»E¶Ç´Â ÇÇ°¡ ÁÙ¾EÁ×´Â°æ¿E¼Õ½Ç °æÇèÄ¡´Â ¾øµµ·Ï ÇÑ´Ù.
				fGameExpRatio = 0.0f;
			}
		}
	}
	else if (nGameType == MMATCH_GAMETYPE_DUEL)
	{
		MMatchRuleDuel* pRuleDuel = (MMatchRuleDuel*)pStage->GetRule();
		if (pVictim->GetUID() == pRuleDuel->uidChallenger)
		{
			fGameExpRatio *= GetDuelVictoryMultiflier(pRuleDuel->GetVictory());
		}
		else
		{
			fGameExpRatio *= GetDuelVictoryMultiflier(pRuleDuel->GetVictory()) * GetDuelPlayersMultiflier(pStage->GetPlayers());

		}
//		if (pRuleDuel->GetVictory() <= 1)
//		{
//			fGameExpRatio = fGameExpRatio * GetDuelPlayersMultiflier(pStage->GetPlayers()) * GetDuelVictoryMultiflier()
//		}
	}
	else if (nGameType == MMATCH_GAMETYPE_CTF)
	{
		MMatchRuleTeamCTF* pRuleCTF = (MMatchRuleTeamCTF*)pStage->GetRule();
		if (pAttacker != pVictim)
		{
			if(pAttacker->GetTeam() != pVictim->GetTeam() && (pVictim->GetUID() == pRuleCTF->GetBlueCarrier() || pVictim->GetUID() == pRuleCTF->GetRedCarrier()))
				fGameExpRatio *= 2;
		}
		else
		{
			fGameExpRatio = 0.0f;
		}
	}
	else if (nGameType == MMATCH_GAMETYPE_INFECTED)
	{
		MMatchRuleTeamInfected* pRuleInfected = (MMatchRuleTeamInfected*)pStage->GetRule();
		if (pAttacker != pVictim)
		{
			if(pAttacker->GetTeam() != pVictim->GetTeam())
			{
				if (pVictim->GetTeam() == MMT_RED)
					fGameExpRatio *= 3;
				else if (pVictim->GetTeam() == MMT_BLUE)
					fGameExpRatio *= 1.5f;
			}
		}
		else
		{
			fGameExpRatio = 0.0f;
		}
	}

	// ¸Ê, °ÔÀÓÅ¸ÀÔ¿¡ ´EÑ °æÇèÄ¡ ºñÀ² Àû¿E
	int nMapIndex = pStage->GetStageSetting()->GetMapIndex();
	if ((nMapIndex >=0) && (nMapIndex < MMATCH_MAP_COUNT))
	{
		float fMapExpRatio = MGetMapDescMgr()->GetExpRatio(nMapIndex);
		fGameExpRatio = fGameExpRatio * fMapExpRatio;
	}

	int nAttackerLevel = pAttacker->GetCharInfo()->m_nLevel;
	int nVictimLevel = pVictim->GetCharInfo()->m_nLevel;

	// °æÇèÄ¡ °è»E
	int nAttackerExp = (int)(MMatchFormula::GetGettingExp(nAttackerLevel, nVictimLevel) * fGameExpRatio);
	int nVictimExp = (int)(MMatchFormula::CalcPanaltyEXP(nAttackerLevel, nVictimLevel) * fGameExpRatio);


#ifndef _EXPCHANNEL
	if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pStage->GetStageType() == MST_LADDER && pStage->GetStageType() == MST_LADDERWARS))
	{
		nAttackerExp = (int)((float)nAttackerExp * 1.5f);
		nVictimExp = 0;
	}
	MMatchChannel* pOwnerChannel = FindChannel(pStage->GetOwnerChannel());
	if (pOwnerChannel && strstr(pOwnerChannel->GetName(), "Event"))
	{
		*poutAttackerExp = 0;
		*poutVictimExp = 0;
		return;
	}
	MMatchChannel* pOwnerChannel = FindChannel(pStage->GetOwnerChannel());
	if ((pOwnerChannel) && (!bSuicide))
	{
		if ((pOwnerChannel->GetRuleType() == MCHANNEL_RULE_MASTERY) || 
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_ELITE) ||
#ifdef _CHANNELNEW
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_NEWS) ||
#endif
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_CHAMPION))
		{
			nVictimExp=0;
		}
	}
#else
	if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pStage->GetStageType() == MST_LADDER && pStage->GetStageType() == MST_LADDERWARS))
	{
		nAttackerExp = nAttackerExp * 6;
		nVictimExp = 0;
	}
	MMatchChannel* pOwnerChannel = FindChannel(pStage->GetOwnerChannel());
	if (pOwnerChannel && strstr(pOwnerChannel->GetName(), "Event"))
	{
		*poutAttackerExp = 0;
		*poutVictimExp = 0;
		return;
	}
	if ((pOwnerChannel) && (!bSuicide))
	{
		if ((pOwnerChannel->GetRuleType() == MCHANNEL_RULE_MASTERY) ||
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_ELITE) ||
#ifdef _CHANNELNEW
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_NEWS) ||
#endif
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_CHAMPION))
		{
			nVictimExp = 0;
		}
	}
#endif
	// Á×Àº»ç¶÷ÀÌ ¿ûÛµÀÚ, °³¹ßÀÚÀÏ °æ¿E°æÇèÄ¡ µÎ¹E
	if ((pVictim->GetAccountInfo()->m_nUGrade == MMUG_ADMIN) || 
		(pVictim->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER))
	{
		nAttackerExp = nAttackerExp * 2;
	}
	// Á×ÀÎ»ç¶÷ÀÌ ¿ûÛµÀÚ, °³¹ßÀÚÀÏ °æ¿E°æÄ¡´Ù¿E¾øÀ½
	if ((!bSuicide) &&
		((pAttacker->GetAccountInfo()->m_nUGrade == MMUG_ADMIN) || 
		(pAttacker->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER)))
	{
		nVictimExp = 0;
	}

	// ÀÚ»EÏ °æ¿E°æÇèÄ¡ ¼Õ½ÇÀÌ µÎ¹E
	if (bSuicide) 
	{
		nVictimExp = (int)(MMatchFormula::GetSuicidePanaltyEXP(nVictimLevel) * fGameExpRatio);
		nAttackerExp = 0;
	}

	// ÆÀÅ³ÀÎ°æ¿E°æÇèÄ¡ Á¦·Î
	if ((pStage->GetStageSetting()->IsTeamPlay()) && (pAttacker->GetTeam() == pVictim->GetTeam()))
	{
		nAttackerExp = 0;
	}


	// ÆÀÀEÏ °æ¿E°æÇèÄ¡ ¹èºÐ
	if (pStage->IsApplyTeamBonus())
	{
		int nTeamBonus = 0;
		if (pStage->GetRule() != NULL)
		{
			int nNewAttackerExp = nAttackerExp;
			pStage->GetRule()->CalcTeamBonus(pAttacker, pVictim, nAttackerExp, &nNewAttackerExp, &nTeamBonus);
			nAttackerExp = nNewAttackerExp;
		}

		// ÆÀ °æÇèÄ¡ Àû¸³
		pStage->AddTeamBonus(nTeamBonus, MMatchTeam(pAttacker->GetTeam()));
	}

	// xp º¸³Ê½º Àû¿E³Ý¸¶ºEPC¹E °æÇèÄ¡ ¹ÝÁE
	int nAttackerExpBonus = 0;
	if (nAttackerExp != 0)
	{
		//const float ta = float(atoi("15")) / 100.0f;
		//mlog( "test float : %f\n", ta * 100.0f );

		//MMatchItemBonusType nBonusType			= GetStageBonusType(pStage->GetStageSetting());
		//const double		dAttackerExp		= static_cast< double >( nAttackerExp );
		//const double		dXPBonusRatio		= static_cast< double >( MMatchFormula::CalcXPBonusRatio(pAttacker, nBonusType) );
		//const double		dAttackerExpBouns	= dAttackerExp * dXPBonusRatio;
		//const double		dSumAttackerExp		= dAttackerExp + dAttackerExpBouns;
		//
		//
		//nAttackerExpBonus = static_cast< int >( dAttackerExpBouns + 0.00001); 

		MMatchItemBonusType nBonusType = GetStageBonusType(pStage->GetStageSetting());
		const float fAttackerExpBonusRatio = MMatchFormula::CalcXPBonusRatio(pAttacker, nBonusType);
		 //ºÎµ¿¼Ò¼öÁ¡ ¿ÀÂ÷¶§¹®¿¡ °è»E¡ ¿µÇâÀ» ÁÖÁE¾Ê´Â ¹E§¿¡¼­ º¸Á¤À» ÇØÁØ´Ù.
		// ¸¸¾EÀÌºÎºÐ¿¡¼­ ´Ù½Ã ¹®Á¦°¡ ¹ß»ýÇÑ´Ù¸Eº¸Á¤ÀÌ ¾Æ´Ñ º£ÀÌ½ººÎÅÍ ¼öÁ¤ ÀÛ¾÷À» ÇØ Áà¾ß ÇÑ´Ù.
		 nAttackerExpBonus = (int)(nAttackerExp * (fAttackerExpBonusRatio + 0.00001f));
	}

	*poutAttackerExp = nAttackerExp + nAttackerExpBonus;

	*poutVictimExp = nVictimExp;
}


const int MMatchServer::CalcBPonGameKill( MMatchStage* pStage, MMatchObject* pAttacker, const int nAttackerLevel, const int nVictimLevel )
{
	if( (0 == pStage) || (0 == pAttacker) ) 
		return -1;

	const int	nAddedBP		= static_cast< int >( MMatchFormula::GetGettingBounty(nAttackerLevel, nVictimLevel) );
	const float fBPBonusRatio	= MMatchFormula::CalcBPBounsRatio( pAttacker, GetStageBonusType(pStage->GetStageSetting()) );
	const int	nBPBonus		= static_cast< int >( nAddedBP * fBPBonusRatio );

	return nAddedBP + nBPBonus;
}




void MMatchServer::ProcessPlayerXPBP(MMatchStage* pStage, MMatchObject* pPlayer, int nAddedXP, int nAddedBP)
{
	if (pStage == NULL) return;
	if (!IsEnabledObject(pPlayer)) return;

	/*
		°æÇèÄ¡ °è»ê
		Ä³¸¯ÅÍ¿¡ °æÇèÄ¡ Àû¿ë
		·¹º§ °è»ê
		DBÄ³½Ì ¾÷µ¥ÀÌÆ®
		·¹º§¾÷,´Ù¿î ¸Þ¼¼Áö Àü¼Û
	*/

	MUID uidStage = pPlayer->GetStageUID();
	int nPlayerLevel = pPlayer->GetCharInfo()->m_nLevel;

	// Ä³¸¯ÅÍ XP ¾÷µ¥ÀÌÆ®
	pPlayer->GetCharInfo()->IncXP(nAddedXP);

	// ·¹º§ °è»ê
	int nNewPlayerLevel = -1;
	if ((pPlayer->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pPlayer->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nPlayerLevel)))
	{
		nNewPlayerLevel = MMatchFormula::GetLevelFromExp(pPlayer->GetCharInfo()->m_nXP);
		if (nNewPlayerLevel != pPlayer->GetCharInfo()->m_nLevel) pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;
	}

	// ¹Ù¿îÆ¼ Ãß°¡ÇØÁØ´Ù
	pPlayer->GetCharInfo()->IncBP(nAddedBP);


	// DB Ä³½³ ¾÷µ¥ÀÌÆ®
	if (pPlayer->GetCharInfo()->GetDBCachingData()->IsRequestUpdate()) {
		UpdateCharDBCachingData(pPlayer);		///< XP, BP, KillCount, DeathCount Ä³½³ ¾÷µ¥ÀÌÆ®
	}

	// ¸¸¾à ·¹º§ÀÌ ¹Ù²î¸é µû·Î ·¹º§¾÷ÇÑ´Ù.
	if ((nNewPlayerLevel >= 0) && (nNewPlayerLevel != nPlayerLevel))
	{
		// ·¹º§ÀÌ ¹Ù²î¸é ¹Ù·Î Ä³½³ ¾÷µ¥ÀÌÆ®ÇÑ´Ù
		UpdateCharDBCachingData(pPlayer);

		pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;
		if (!m_MatchDBMgr.UpdateCharLevel(pPlayer->GetCharInfo()->m_nCID,
			nNewPlayerLevel,
			pPlayer->GetCharInfo()->m_nBP,
			pPlayer->GetCharInfo()->m_nTotalKillCount,
			pPlayer->GetCharInfo()->m_nTotalDeathCount,
			pPlayer->GetCharInfo()->m_nTotalPlayTimeSec,
			true))
		{
			mlog("DB UpdateCharLevel Error : %s\n", pPlayer->GetCharInfo()->m_szName);
		}
	}


	// ·¹º§¾÷, ·¹º§ ´Ù¿î ¸Þ¼¼Áö º¸³»±â
	if (nNewPlayerLevel > 0)
	{
		if (nNewPlayerLevel > nPlayerLevel)
		{
#ifdef _UPREWARDLEVEL
			MMatchLevelUpReward* pLvlUp = new MMatchLevelUpReward;
			if (pLvlUp == NULL)
			{
				mlog("Level Up Reward Failed\n");
				return;
			}
			pLvlUp->GiveReward(pPlayer, nNewPlayerLevel);
#endif

			MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_UP, MUID(0, 0));
			pCmd->AddParameter(new MCommandParameterUID(pPlayer->GetUID()));
			pCmd->AddParameter(new MCommandParameterInt(nNewPlayerLevel));
#ifdef _UPREWARDLEVEL
			pCmd->AddParameter(new MCommandParameterInt(pLvlUp->GetLevelUpRewardByLevel(nNewPlayerLevel)->GetBounty()));
			pCmd->AddParameter(new MCommandParameterInt(pLvlUp->GetLevelUpRewardByLevel(nNewPlayerLevel)->GetCash()));
			pCmd->AddParameter(new MCommandParameterInt(pLvlUp->GetLevelUpRewardByLevel(nNewPlayerLevel)->GetItemID()));
#endif
			RouteToBattle(uidStage, pCmd);
		}
		else if (nNewPlayerLevel < nPlayerLevel)
		{
			MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_DOWN, MUID(0, 0));
			pCmd->AddParameter(new MCommandParameterUID(pPlayer->GetUID()));
			pCmd->AddParameter(new MCommandParameterInt(nNewPlayerLevel));
			RouteToBattle(uidStage, pCmd);
		}
	}
}

// ÆÀ º¸³Ê½º Àû¿E
void MMatchServer::ApplyObjectTeamBonus(MMatchObject* pObject, int nAddedExp)
{
	if (!IsEnabledObject(pObject)) return;
	if (nAddedExp <= 0)
	{
		_ASSERT(0);
		return;
	}

	bool bIsLevelUp = false;

	// º¸³Ê½º Àû¿ë
	if (nAddedExp != 0)
	{
		int nExpBonus = (int)(nAddedExp * MMatchFormula::CalcXPBonusRatio(pObject, MIBT_TEAM));
		nAddedExp += nExpBonus;
	}


	// Ä³¸¯ÅÍ XP ¾÷µ¥ÀÌÆ®
	pObject->GetCharInfo()->IncXP(nAddedExp);

	// ·¹º§ °è»ê
	int nNewLevel = -1;
	int nCurrLevel = pObject->GetCharInfo()->m_nLevel;

	if (nNewLevel > nCurrLevel) bIsLevelUp = true;

	if ((pObject->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pObject->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nCurrLevel)))
	{
		nNewLevel = MMatchFormula::GetLevelFromExp(pObject->GetCharInfo()->m_nXP);
		if (nNewLevel != nCurrLevel) pObject->GetCharInfo()->m_nLevel = nNewLevel;
	}

	// DB Ä³½³ ¾÷µ¥ÀÌÆ®
	if (pObject->GetCharInfo()->GetDBCachingData()->IsRequestUpdate())
	{
		UpdateCharDBCachingData(pObject);
	}

	// ¸¸¾à ·¹º§ÀÌ ¹Ù²î¸é ¹Ù·Î ·¹º§¾÷ÇÑ´Ù.
	if ((nNewLevel >= 0) && (nNewLevel != nCurrLevel))
	{
		// ·¹º§ÀÌ ¹Ù²î¸é ¹Ù·Î Ä³½³ ¾÷µ¥ÀÌÆ®ÇÑ´Ù
		UpdateCharDBCachingData(pObject);
		pObject->GetCharInfo()->m_nLevel = nNewLevel;
		nCurrLevel = nNewLevel;

		if (!m_MatchDBMgr.UpdateCharLevel(pObject->GetCharInfo()->m_nCID,
			nNewLevel,
			pObject->GetCharInfo()->m_nBP,
			pObject->GetCharInfo()->m_nTotalKillCount,
			pObject->GetCharInfo()->m_nTotalDeathCount,
			pObject->GetCharInfo()->m_nTotalPlayTimeSec,
			bIsLevelUp
		))
		{
			mlog("DB UpdateCharLevel Error : %s\n", pObject->GetCharInfo()->m_szName);
		}
	}


	MUID uidStage = pObject->GetStageUID();

	unsigned long int nExpArg;
	unsigned long int nChrExp;
	int nPercent;

	nChrExp = pObject->GetCharInfo()->m_nXP;
	nPercent = MMatchFormula::GetLevelPercent(nChrExp, nCurrLevel);
	// »óÀ§ 2¹ÙÀÌÆ®´Â °æÇèÄ¡, ÇÏÀ§ 2¹ÙÀÌÆ®´Â °æÇèÄ¡ÀÇ ÆÛ¼¾Æ®ÀÌ´Ù.
	nExpArg = MakeExpTransData(nAddedExp, nPercent);


	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_TEAMBONUS, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterUID(pObject->GetUID()));
	pCmd->AddParameter(new MCommandParameterUInt(nExpArg));
	RouteToBattle(uidStage, pCmd);


	// ·¹º§¾÷ ¸Þ¼¼Áö º¸³»±â
	if ((nNewLevel >= 0) && (nNewLevel > nCurrLevel))
	{
#ifdef _UPREWARDLEVEL
		MMatchLevelUpReward* pLvlUp = new MMatchLevelUpReward;
		if (pLvlUp == NULL)
		{
			mlog("Level Up Reward Failed\n");
			return;
		}
		pLvlUp->GiveReward(pObject, nNewLevel);
#endif

		MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_UP, MUID(0, 0));
		pCmd->AddParameter(new MCommandParameterUID(pObject->GetUID()));
		pCmd->AddParameter(new MCommandParameterInt(nNewLevel));
#ifdef _UPREWARDLEVEL
		pCmd->AddParameter(new MCommandParameterInt(pLvlUp->GetLevelUpRewardByLevel(nNewLevel)->GetBounty()));
		pCmd->AddParameter(new MCommandParameterInt(pLvlUp->GetLevelUpRewardByLevel(nNewLevel)->GetCash()));
		pCmd->AddParameter(new MCommandParameterInt(pLvlUp->GetLevelUpRewardByLevel(nNewLevel)->GetItemID()));
#endif
		RouteToBattle(uidStage, pCmd);
	}
}

// ÇÃ·¹ÀÌ Áß Ä³¸¯ÅÍ Á¤º¸ ¾÷µ¥ÀÌÆ®
void MMatchServer::ProcessCharPlayInfo(MMatchObject* pPlayer)
{
	if (!IsEnabledObject(pPlayer)) return;

	/*
	¿øÇÒ¶§¸¶´Ù Ä³¸¯ÅÍ Á¤º¸¸¦ ¾÷µ¥ÀÌÆ® Àû¿E
	°æÇèÄ¡ °è»E
	Ä³¸¯ÅÍ¿¡ °æÇèÄ¡ Àû¿E
	·¹º§ °è»E
	·¹º§¾E´Ù¿E¸Þ¼¼ÁEÀEÛ
	¹Ù¿ûâ¼ Ãß°¡ÇØÁØ´Ù
	Á¢¼Ó½Ã°£, °ÔÀÓ ÁøÇà½Ã°£, ÇÃ·¹ÀÌ ½Ã°£
	*/

	MUID uidStage = pPlayer->GetStageUID();
	int nPlayerLevel = pPlayer->GetCharInfo()->m_nLevel;

	// ·¹º§ °è»E
	int nNewPlayerLevel = -1;
	if ((pPlayer->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pPlayer->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nPlayerLevel)))
	{
		nNewPlayerLevel = MMatchFormula::GetLevelFromExp(pPlayer->GetCharInfo()->m_nXP);
		if (nNewPlayerLevel != pPlayer->GetCharInfo()->m_nLevel) pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;
	}
	// ¸¸¾E·¹º§ÀÌ ¹Ù²ûÔEµû·Î ·¹º§¾÷ÇÑ´Ù.
	if ((nNewPlayerLevel >= 0) && (nNewPlayerLevel != nPlayerLevel))
		pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;

	// Á¢¼Ó½Ã°£, °ÔÀÓ ÁøÇà½Ã°£, ÇÃ·¹ÀÌ ½Ã°£
	unsigned long int nNowTime = MMatchServer::GetInstance()->GetGlobalClockCount();
	unsigned long int nBattlePlayingTimeSec = 0;
	if(pPlayer->GetCharInfo()->m_nBattleStartTime != 0)
	{
		nBattlePlayingTimeSec = MGetTimeDistance(pPlayer->GetCharInfo()->m_nBattleStartTime, nNowTime) / 1000;	// ¹èÆ²À» ÁøÇàÇÑ ½Ã°£
		
		/*
		// ¾Æ¹« Ã³¸®µµ ÇÏÁE¾Ê´Âµ¥, ¿Ö ·Î±×´Â ³²±â´Â°Ç°¡¿E ÀÏ´Ü ÁÖ¼® Ã³¸®ÇÕ´Ï´Ù. - carrot318
		if(nBattlePlayingTimeSec > 60*60)
		{// ÀÌ»óÀûÀ¸·Î °ªÀÌ ¼¼ÆÃµÅ¸E·Î±×¸¦ ³²±ä´Ù.
			CTime theTime = CTime::GetCurrentTime();
			CString szTime = theTime.Format( "[%c] " );

			// °ÔÀÓ ¸ðµE
			char buf[64]={0,};
			MMatchStage* pStage = FindStage(uidStage);

			if( pStage != NULL )
			{
				switch((int)pStage->GetStageSetting()->GetGameType())
				{
				case MMATCH_GAMETYPE_DEATHMATCH_SOLO:	{sprintf(buf, "DEATHMATCH_SOLO");	} break;		///< °³ÀÎ µ¥¾²¸ÅÄ¡
				case MMATCH_GAMETYPE_DEATHMATCH_TEAM:	{sprintf(buf, "DEATHMATCH_TEAM");	} break;		///< ÆÀ µ¥¾²¸ÅÄ¡
				case MMATCH_GAMETYPE_GLADIATOR_SOLO:	{sprintf(buf, "GLADIATOR_SOLO");	} break;		///< °³ÀÎ ±Û·¡µð¿¡ÀÌÅÍ
				case MMATCH_GAMETYPE_GLADIATOR_TEAM:	{sprintf(buf, "GLADIATOR_TEAM");	} break;		///< ÆÀ ±Û·¡µð¿¡ÀÌÅÍ
				case MMATCH_GAMETYPE_ASSASSINATE:		{sprintf(buf, "ASSASSINATE");		} break;		///< º¸½ºÀE
				case MMATCH_GAMETYPE_TRAINING:			{sprintf(buf, "TRAINING");			} break;		///< ¿¬½À

				case MMATCH_GAMETYPE_SURVIVAL:			{sprintf(buf, "SURVIVAL");			} break;		///< ¼­¹ÙÀÌ¹E
				case MMATCH_GAMETYPE_QUEST:				{sprintf(buf, "QUEST");				} break;		///< Äù½ºÆ®
				case MMATCH_GAMETYPE_QUEST_CHALLENGE:	{sprintf(buf, "QUEST_CHALLENGE");	} break;		///< Ã§¸°ÁEÄù½ºÆ®

				case MMATCH_GAMETYPE_BERSERKER:			{sprintf(buf, "BERSERKER");			} break;		
				case MMATCH_GAMETYPE_DEATHMATCH_TEAM2:	{sprintf(buf, "DEATHMATCH_TEAM2");	} break;		
				case MMATCH_GAMETYPE_DUEL:				{sprintf(buf, "DUEL");				} break;	
				default:								{sprintf(buf, "don't know");		} break;
				}
				mlog("%s BattlePlayT Error GameMode:%s, CID:%d, Name:%s, ServerCurrT:%u, BattleStartT:%u, PlayT:%d, PlayerConnectT:%u \n", szTime, buf, pPlayer->GetCharInfo()->m_nCID, pPlayer->GetCharInfo()->m_szName, nNowTime, pPlayer->GetCharInfo()->m_nBattleStartTime, nBattlePlayingTimeSec, pPlayer->GetCharInfo()->m_nConnTime);
			}
		}
		*/
		//pPlayer->GetCharInfo()->m_nBattleStartTime = 0;
	}
	unsigned long int nLoginTotalTimeSec = MGetTimeDistance(pPlayer->GetCharInfo()->m_nConnTime, nNowTime) / 1000;	// °ÔÀÓÀ» ÁøÇàÇÑ ½Ã°£

	// ÀÌ»óÀûÀ¸·Î °æÇèÄ¡°¡ È¹µæÇßÀ¸¸E·Î±×¸¦ ³²°ÜÁØ´Ù.
	// ¾Æ¹« Ã³¸®µµ ÇÏÁE¾Ê´Âµ¥, ¿Ö ·Î±×´Â ³²±â´Â°Ç°¡¿E ÀÏ´Ü ÁÖ¼® Ã³¸®ÇÕ´Ï´Ù. - carrot318
	/*
	long int nBattleEXPGained = pPlayer->GetCharInfo()->m_nXP - pPlayer->GetCharInfo()->m_nBattleStartXP;
	if(nBattleEXPGained < -150000 || 150000 < nBattleEXPGained)
	{
		CTime theTime = CTime::GetCurrentTime();
		CString szTime = theTime.Format( "[%c] " );
		mlog("%s BattleXPGained Error CID:%d, Name:%s, StartXP:%d, EXPGained:%d \n", szTime, pPlayer->GetCharInfo()->m_nCID, pPlayer->GetCharInfo()->m_szName, pPlayer->GetCharInfo()->m_nBattleStartXP, nBattleEXPGained);
	}
	*/

#ifdef LOCALE_NHNUSA
	if (!m_MatchDBMgr.UpdateCharPlayInfo(pPlayer->GetAccountInfo()->m_nAID
										, pPlayer->GetCharInfo()->m_nCID
										, pPlayer->GetCharInfo()->m_nXP
										, pPlayer->GetCharInfo()->m_nLevel
										, nBattlePlayingTimeSec										// ¹èÆ² ½Ã°£
										, nLoginTotalTimeSec										// ÃÑ°ÔÀÓÀ» ÁøÇàÇÑ ½Ã°£
										, pPlayer->GetCharInfo()->m_nTotalKillCount
										, pPlayer->GetCharInfo()->m_nTotalDeathCount
										, pPlayer->GetCharInfo()->m_nBP
		                                , pPlayer->GetCharInfo()->m_nLC
										, false))
	{
		mlog("DB UpdateCharPlayInfo Error : %s\n", pPlayer->GetCharInfo()->m_szName);
	}
#endif
}

void MMatchServer::PostGameDeadOnGameKill(MUID& uidStage, MMatchObject* pAttacker, MMatchObject* pVictim,
									int nAddedAttackerExp, int nSubedVictimExp)
{
	unsigned long int nAttackerArg = 0;
	unsigned long int nVictimArg =0;

	int nRealAttackerLevel = pAttacker->GetCharInfo()->m_nLevel;
	int nRealVictimLevel = pVictim->GetCharInfo()->m_nLevel;

	unsigned long int nChrExp;
	int nPercent;

	nChrExp = pAttacker->GetCharInfo()->m_nXP;
	nPercent = MMatchFormula::GetLevelPercent(nChrExp, nRealAttackerLevel);
	nAttackerArg = MakeExpTransData(nAddedAttackerExp, nPercent);

	nChrExp = pVictim->GetCharInfo()->m_nXP;
	nPercent = MMatchFormula::GetLevelPercent(nChrExp, nRealVictimLevel);
	nVictimArg = MakeExpTransData(nSubedVictimExp, nPercent);

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_DEAD, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pAttacker->GetUID()));
	pCmd->AddParameter(new MCommandParameterUInt(nAttackerArg));
	pCmd->AddParameter(new MCommandParameterUID(pVictim->GetUID()));
	pCmd->AddParameter(new MCommandParameterUInt(nVictimArg));
	RouteToBattle(uidStage, pCmd);	
}

void MMatchServer::StageList(const MUID& uidPlayer, int nStageStartIndex, bool bCacheUpdate)
{
	MMatchObject* pChar = GetObject(uidPlayer);
	if (pChar == NULL) return;
	MMatchChannel* pChannel = FindChannel(pChar->GetChannelUID());
	if (pChannel == NULL) return;

	// Å¬·£¼­¹öÀÎµ¥ Å¬·£Ã¤³ÎÀÏ °æ¿E¡´Â ¹E¸®½ºÆ®´EÅ ´EâÁß Å¬·£ ¸®½ºÆ®¸¦ º¸³½´Ù.
	if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pChannel->GetChannelType() == MCHANNEL_TYPE_CLAN))
	{
		StandbyClanList(uidPlayer, nStageStartIndex, bCacheUpdate);
		return;
	}


	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_LIST), MUID(0,0), m_This);

	int nPrevStageCount = -1, nNextStageCount = -1;
	int nNextStageIndex = pChannel->GetMaxPlayers()-1;


	// 2008.09.16 
	int nRealStageStartIndex = nStageStartIndex;
	int nStageCount = 0;
	for(int i = 0; i < pChannel->GetMaxPlayers(); i++)
	{
		// ¹æÀÌ ºñ¿öÀÕÀ¸¸EÃ³¸®¾ÈÇÑ´Ù
		if (pChannel->IsEmptyStage(i)) continue;
		// ¹æÀÌ ÀÖÀ¸¸EÃ³¸®
		if(nStageCount < nStageStartIndex) // ¾Õ¿¡ ÅÇ¿¡ Ã³¸®µÈ ¹æµE< ÇöÀEÅÇ¿¡¼­ ½ÃÀÛÇÒ ¹EIndex
			nStageCount++;
		else
		{
			nRealStageStartIndex = i;
			break;
		}
	}

	int nRealStageCount = 0;
	for (int i = /*nStageStartIndex*/nRealStageStartIndex; i < pChannel->GetMaxPlayers(); i++)
	{
		if (pChannel->IsEmptyStage(i)) continue;

		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;

		nRealStageCount++;
		if (nRealStageCount >= TRANS_STAGELIST_NODE_COUNT) 
		{
			nNextStageIndex = i;
			break;
		}
	}

	if (!bCacheUpdate)
	{
		nPrevStageCount = pChannel->GetPrevStageCount(nStageStartIndex);
		nNextStageCount = pChannel->GetNextStageCount(nNextStageIndex);
	}

	pNew->AddParameter(new MCommandParameterChar((char)nPrevStageCount));
	pNew->AddParameter(new MCommandParameterChar((char)nNextStageCount));


	void* pStageArray = MMakeBlobArray(sizeof(MTD_StageListNode), nRealStageCount);
	int nArrayIndex=0;

	for (int i = /*nStageStartIndex*/nRealStageStartIndex; i < pChannel->GetMaxPlayers(); i++)
	{
		if (pChannel->IsEmptyStage(i)) continue;
		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;
		
		if( pStage->GetState() < STAGE_STATE_STANDBY || pStage->GetState() > STAGE_STATE_COUNT )
		{
			LOG(LOG_FILE, "there is unavailable stages in %s channel. No:%d \n", pChannel->GetName(), i);
			continue;
		}


		if (nArrayIndex >= nRealStageCount) break;

		MTD_StageListNode* pNode = (MTD_StageListNode*)MGetBlobArrayElement(pStageArray, nArrayIndex++);
		pNode->uidStage = pStage->GetUID();
		strcpy(pNode->szStageName, pStage->GetName());
		pNode->nNo = (unsigned char)(pStage->GetIndex() + 1);	// »ç¿EÚ¿¡°Ô º¸¿©ÁÖ´Â ÀÎµ¦½º´Â 1ºÎÅÍ ½ÃÀÛÇÑ´Ù
		pNode->nPlayers = (char)pStage->GetPlayers();
		pNode->nMaxPlayers = pStage->GetStageSetting()->GetMaxPlayers();
		pNode->nState = pStage->GetState();
		pNode->nGameType = pStage->GetStageSetting()->GetGameType();
#ifdef _BOXLEAD
		pNode->bIsLead = pStage->GetStageSetting()->IsLead();
#endif
#ifdef _ROTATION
		pNode->bIsRobase = pStage->GetStageSetting()->GetTeamRotation();
#endif
		// ¸±·¹ÀÌ¸E·ÎºE¹æ¸®½ºÆ® ¹è³Ê¸¦ ¸±·¹ÀÌ¸ÊÀ¸·Î À¯ÁöÇØÁØ´Ù.
		if(pStage->IsRelayMap()) pNode->nMapIndex = MMATCH_MAP_RELAYMAP;
		else		 			 pNode->nMapIndex = pStage->GetStageSetting()->GetMapIndex();
		
		pNode->nSettingFlag = 0;
		// ³­ÀÔ
		if (pStage->GetStageSetting()->GetForcedEntry())
		{
			pNode->nSettingFlag |= MSTAGENODE_FLAG_FORCEDENTRY_ENABLED;
		}
		// ºñ¹Ð¹E
		if (pStage->IsPrivate())
		{
			pNode->nSettingFlag |= MSTAGENODE_FLAG_PRIVATE;
		}
		// ·¹º§Á¦ÇÑ
		pNode->nLimitLevel = pStage->GetStageSetting()->GetLimitLevel();
		pNode->nMasterLevel = 0;

		if (pNode->nLimitLevel != 0)
		{
			pNode->nSettingFlag |= MSTAGENODE_FLAG_LIMITLEVEL;

			;
			MMatchObject* pMaster = GetObject(pStage->GetMasterUID());
			if (pMaster)
			{
				if (pMaster->GetCharInfo())
				{
					pNode->nMasterLevel = pMaster->GetCharInfo()->m_nLevel;
				}
			}
		}

		// for new UI: mod cua phong di kem de lobby ve icon
		const MSTAGE_SETTING_NODE* pStageSet = pStage->GetStageSetting()->GetStageSetting();
		pNode->nRoomModCount = pStageSet->nRoomModCount;
		if (pNode->nRoomModCount < 0) pNode->nRoomModCount = 0;
		if (pNode->nRoomModCount > MMOD_END) pNode->nRoomModCount = MMOD_END;
		memcpy(pNode->RoomMods, pStageSet->RoomMods, sizeof(pNode->RoomMods));
	}

	pNew->AddParameter(new MCommandParameterBlob(pStageArray, MGetBlobArraySize(pStageArray)));
	MEraseBlobArray(pStageArray);
	RouteToListener(pChar, pNew);	
}


void MMatchServer::OnStageRequestStageList(const MUID& uidPlayer, const MUID& uidChannel, const int nStageCursor)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return;

	pObj->SetStageCursor(nStageCursor);
	StageList(pObj->GetUID(), nStageCursor, false);
}


void MMatchServer::OnRequestQuickJoin(const MUID& uidPlayer, void* pQuickJoinBlob)
{
	MTD_QuickJoinParam* pNode = (MTD_QuickJoinParam*)MGetBlobArrayElement(pQuickJoinBlob, 0);
	ResponseQuickJoin(uidPlayer, pNode);
}

void MMatchServer::ResponseQuickJoin(const MUID& uidPlayer, MTD_QuickJoinParam* pQuickJoinParam)
{
	if (pQuickJoinParam == NULL) return;

	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (pChannel == NULL) return;

	list<MUID>	recommended_stage_list;
	MUID uidRecommendedStage = MUID(0,0);
	int nQuickJoinResult = MOK;

	for (int i = 0; i < pChannel->GetMaxStages(); i++)
	{
		if (pChannel->IsEmptyStage(i)) continue;
		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;

		int ret = ValidateStageJoin(pObj->GetUID(), pStage->GetUID());
		if (ret == MOK)
		{
			if (pStage->IsPrivate()) continue;

			int nMapIndex = pStage->GetStageSetting()->GetMapIndex();
			int nGameType = pStage->GetStageSetting()->GetGameType();

			if (!CheckBitSet(pQuickJoinParam->nMapEnum, nMapIndex)) continue;
			if (!CheckBitSet(pQuickJoinParam->nModeEnum, nGameType)) continue;

			//if (((1 << nMapIndex) & (pQuickJoinParam->nMapEnum)) == 0) continue;
			//if (((1 << nGameType) & (pQuickJoinParam->nModeEnum)) == 0) continue;

			recommended_stage_list.push_back(pStage->GetUID());
		}
	}

	if (!recommended_stage_list.empty())
	{
		int nSize = (int)recommended_stage_list.size();
		int nIndex = rand() % nSize;

		int nCnt = 0;
		for (list<MUID>::iterator itor = recommended_stage_list.begin(); itor != recommended_stage_list.end(); ++itor)
		{
			if (nIndex == nCnt)
			{
				uidRecommendedStage = (*itor);
				break;
			}
			nCnt++;
		}
	}
	else
	{
		nQuickJoinResult = MERR_CANNOT_NO_STAGE;
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_RESPONSE_QUICKJOIN, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterInt(nQuickJoinResult));
	pCmd->AddParameter(new MCommandParameterUID(uidRecommendedStage));
	RouteToListener(pObj, pCmd);	
}

static int __cdecl _int_sortfunc(const void* a, const void* b)
{
	return *((int*)a) - *((int*)b);
}

int MMatchServer::GetLadderTeamIDFromDB(const int nTeamTableIndex, const int* pnMemberCIDArray, const int nMemberCount)
{
	if ((nMemberCount <= 0) || (nTeamTableIndex != nMemberCount))
	{
		_ASSERT(0);
		return 0;
	}

	// cid ¿À¸§Â÷¼øÀ¸·Î ¼ÒÆÃ - db»ó¿¡ ¼ÒÆÃµÇ¾Eµé¾ûÌ¡ÀÖ´Ù. 
	int* pnSortedCIDs = new int[nMemberCount];
	for (int i = 0; i < nMemberCount; i++)
	{
		pnSortedCIDs[i] = pnMemberCIDArray[i];
	}
	qsort(pnSortedCIDs, nMemberCount, sizeof(int), _int_sortfunc);

	int nTID = 0;
	if (pnSortedCIDs[0] != 0)
	{
		if (!m_MatchDBMgr.GetLadderTeamID(nTeamTableIndex, pnSortedCIDs, nMemberCount, &nTID))
		{
			nTID = 0;
		}
	}

	delete[] pnSortedCIDs;
	return nTID;
}

void MMatchServer::SaveLadderTeamPointToDB(const int nTeamTableIndex, const int nWinnerTeamID, const int nLoserTeamID, const bool bIsDrawGame)
{
	// Æ÷ÀÎÆ® °è»E- ¾×¼Ç¸®±× ÀEE
	int nWinnerPoint = 0, nLoserPoint = 0, nDrawPoint = 0;

	nLoserPoint = -1;
	switch (nTeamTableIndex)
	{
		case 2:	// 2´E
		{
			nWinnerPoint = 4;
			nDrawPoint = 1;
		} break;
		case 3:
		{
			nWinnerPoint = 6;
			nDrawPoint = 1;
		} break;
		case 4:
		{
			nWinnerPoint = 10;
			nDrawPoint = 2;
		} break;
	}

	if (!m_MatchDBMgr.LadderTeamWinTheGame(nTeamTableIndex, nWinnerTeamID, nLoserTeamID, bIsDrawGame, nWinnerPoint, nLoserPoint, nDrawPoint))
	{
		mlog("DB Query(SaveLadderTeamPointToDB) Failed\n");
	}
}


void MMatchServer::OnVoteCallVote(const MUID& uidPlayer, const char* pszDiscuss, const char* pszArg)
{
	char szMsg[256];
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	// Custom: Only instant kick outside of event channel.
	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
#ifdef _NEWGRADE
	if ((pObj->GetAccountInfo()->m_nUGrade == MMUG_GAMEMASTER || pObj->GetAccountInfo()->m_nUGrade == MMUG_GAMEMASTER || pObj->GetAccountInfo()->m_nUGrade == MMUG_HIDE_ADMIN || IsAdminGrade(pObj)) && (pChannel && pChannel->IsUseEvent())) {
#else
	if ((pObj->GetAccountInfo()->m_nUGrade == MMUG_GAMEMASTER || IsAdminGrade(pObj)) && (pChannel && pChannel->IsUseEvent())) {
#endif
		MMatchStage* pStage = FindStage(pObj->GetStageUID());
		if (pStage) {
			pStage->KickBanPlayer(pszArg, false);
		}
		return;
	}
#ifdef _NEWGRADE
	else if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER && pObj->GetAccountInfo()->m_nUGrade != MMUG_HIDE_ADMIN && pChannel->IsUseEvent())
#else
	else if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER && pChannel->IsUseEvent())
#endif
	{
		sprintf(szMsg,"%s", "Cannot vote in event channel");
		Announce(uidPlayer, szMsg);
		return;
	}
#ifdef _NEWGRADE
	if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER && pObj->GetAccountInfo()->m_nUGrade != MMUG_HIDE_ADMIN)
#else
	if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER)
#endif
	{
		MMatchObject* pTar = GetPlayerByName(pszArg);
		if (!pTar) return;

		if (pTar->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
		{
			sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE);
			Announce(uidPlayer, szMsg);
			return;
		}
#ifdef _NEWGRADE
		if (IsAdminGrade(pTar) || pTar->GetAccountInfo()->m_nUGrade == MMUG_GAMEMASTER || pTar->GetAccountInfo()->m_nUGrade == MMUG_GAMEMASTER || pTar->GetAccountInfo()->m_nUGrade == MMUG_HIDE_ADMIN)
#else
		if (IsAdminGrade(pTar) || pTar->GetAccountInfo()->m_nUGrade == MMUG_GAMEMASTER)
#endif
		{
			sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE);
			Announce(uidPlayer, szMsg);
			return;
		}
	}

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if (pObj->WasCallVote())
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE);
		Announce(uidPlayer, szMsg);
		return;
	}

	pObj->SetVoteState( true );

	if (pStage->GetStageType() == MST_LADDER || pStage->GetStageType() == MST_LADDERWARS)
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE_LADERGAME);
		Announce(uidPlayer, szMsg);
		return;
	}

	if (pStage->GetRule() && pStage->GetRule()->GetGameType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE);
		Announce(uidPlayer, szMsg);
		return;
	}

#ifdef _VOTESETTING
	if( !pStage->GetStageSetting()->bVoteEnabled ) {
		VoteAbort( uidPlayer );
		return;
	}

	if( pStage->WasCallVote() ) {
		VoteAbort( uidPlayer );
		return;
	}
	else {
		pStage->SetVoteState( true );
	}
#endif

	if (pStage->GetVoteMgr()->GetDiscuss())
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_VOTE_ALREADY_START);
		Announce(uidPlayer, szMsg);
		return;
	}

	MVoteDiscuss* pDiscuss = MVoteDiscussBuilder::Build(uidPlayer, pStage->GetUID(), pszDiscuss, pszArg);
	if (pDiscuss == NULL) return;

	if (pStage->GetVoteMgr()->CallVote(pDiscuss))
	{
		pDiscuss->Vote(uidPlayer, MVOTE_YES);

		MCommand* pCmd = CreateCommand(MC_MATCH_NOTIFY_CALLVOTE, MUID(0,0));
		pCmd->AddParameter(new MCmdParamStr(pszDiscuss));
		pCmd->AddParameter(new MCmdParamStr(pszArg));
		RouteToStage(pStage->GetUID(), pCmd);
		return;
	}
	else
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_VOTE_FAILED);
		Announce(uidPlayer, szMsg);
		return;
	}
}

void MMatchServer::OnVoteYes(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	MVoteDiscuss* pDiscuss = pStage->GetVoteMgr()->GetDiscuss();
    if (pDiscuss == NULL) return;

	pDiscuss->Vote(uidPlayer, MVOTE_YES);
}

void MMatchServer::OnVoteNo(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	MVoteDiscuss* pDiscuss = pStage->GetVoteMgr()->GetDiscuss();
    if (pDiscuss == NULL) return;

	pDiscuss->Vote(uidPlayer, MVOTE_NO);
}

void MMatchServer::VoteAbort( const MUID& uidPlayer )
{
#ifndef MERR_CANNOT_VOTE
#define MERR_CANNOT_VOTE 120000
#endif

	MMatchObject* pObj = GetObject( uidPlayer );
	if (!IsEnabledObject(pObj)) return;

	MCommand* pCmd = CreateCommand( MC_MATCH_VOTE_RESPONSE, MUID(0, 0) );
	if(0 == pCmd) return;

	pCmd->AddParameter( new MCommandParameterInt(MERR_CANNOT_VOTE) );
	RouteToListener( pObj, pCmd );
}

void MMatchServer::OnEventChangeMaster(const MUID& uidAdmin)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// Staff check
	if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER) return;
	if (pStage->GetMasterUID() == uidAdmin) return;

	// Custom: Event Team
	if (pObj->GetAccountInfo()->m_nUGrade == MMUG_GAMEMASTER || IsAdminGrade(pObj))
	{
		pStage->SetMasterUID(uidAdmin);
		StageMaster(pStage->GetUID());
	}
}

void MMatchServer::OnEventChangePassword(const MUID& uidAdmin, const char* pszPassword)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// Custom: Event Team + StageMaster
	if (pObj->GetUID() == pStage->GetMasterUID() || IsAdminGrade(pObj) || pObj->GetAccountInfo()->m_nUGrade == MMUG_GAMEMASTER)
	{
		pStage->SetPassword(pszPassword);
		pStage->SetPrivate(true);
	}
}

/*void MMatchServer::OnEventRequestJjang(const MUID& uidAdmin, const char* pszTargetName)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if( 0 == pObj )
		return;

	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// °ü¸®ÀÚ ±ÇÇÑÀ» °¡Áø »ç¶÷ÀÌ ¾Æ´Ï¸é ¹«½Ã
	if ((!IsAdminGrade(pObj))) 
		return;


	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) return;
	if ((IsAdminGrade(pTargetObj))) return;
	if (MMUG_STAR == pTargetObj->GetAccountInfo()->m_nUGrade) return;	// ÀÌ¹Ì Â¯

	pTargetObj->GetAccountInfo()->m_nUGrade = MMUG_STAR;

	if (m_MatchDBMgr.EventJjangUpdate(pTargetObj->GetAccountInfo()->m_nAID, true)) {
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.AddObject(pTargetObj);
		MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_REPLACE, this);
		RouteToStage(pStage->GetUID(), pCmdCacheUpdate);

		MCommand* pCmdUIUpdate = CreateCommand(MC_EVENT_UPDATE_JJANG, MUID(0,0));
		pCmdUIUpdate->AddParameter(new MCommandParameterUID(pTargetObj->GetUID()));
		pCmdUIUpdate->AddParameter(new MCommandParameterBool(true));
		RouteToStage(pStage->GetUID(), pCmdUIUpdate);

		FILE* pFile = fopen("GM_LOGS.txt", "a");
		if (pFile != NULL) {
			char szDateRun[128] = "";
			char szTimeRun[128] = "";
			_strdate(szDateRun);
			_strtime(szTimeRun);
			fprintf_s(pFile, "[%s %s](GIVE_JJANG) %s gave jjang to %s\n", szDateRun, szTimeRun, pObj->GetName(), pTargetObj->GetName());
			fclose(pFile);
		}
	}
}*/
void MMatchServer::OnAdminSendItem(const MUID& uidAdmin, const char* szPlayer, const int nItemId, const int nDays)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (pObj == NULL || !IsAdminGrade(pObj)) return;

	if (strlen(szPlayer) < 2) {
		Announce(pObj, "Player name must be longer.");
		return;
	}

	if (nDays < 0) {
		Announce(pObj, "RentDays must be 0 or more.");
		return;
	}

	MMatchObject* pTargetObj = GetPlayerByName(szPlayer);

	int nCID = 0;
	if (m_MatchDBMgr.GetCharCID(szPlayer, &nCID) == false)
	{
		Announce(pObj, "Could not find player.");
		return;
	}

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemId);
	if (pItemDesc == NULL)
	{
		Announce(pObj, "Could not find item.");
		return;
	}

	if (!pItemDesc->IsSpendableItem())
	{
		unsigned long int nNewCIID = 0;
		if (!m_MatchDBMgr.InsertDistributeItem(nCID, nItemId, nDays != 0, nDays * 24, &nNewCIID)) {
			Announce(pObj, "Could not send item.");
			return;
		}

		if (pTargetObj != NULL) {
			MUID uidNew = MMatchItemMap::UseUID();
			pTargetObj->GetCharInfo()->m_ItemList.CreateItem(uidNew, nNewCIID, nItemId, nDays != 0, nDays * 24 * 60, nDays * 24);

			UpdateCharItemDBCachingData(pTargetObj);

			ResponseCharacterItemList(pTargetObj->GetUID());

			Announce(pTargetObj, "You have received an item!");
		}
	}
	else {
		Announce(pObj, "Cannot send items of this type.");
		return;
	}

	Announce(pObj, "Sent item to player.");
}
/*void MMatchServer::OnEventRemoveJjang(const MUID& uidAdmin, const char* pszTargetName)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if( 0 == pObj )
		return;

	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// °ü¸®ÀÚ ±ÇÇÑÀ» °¡Áø »ç¶÷ÀÌ ¾Æ´Ï¸é ¿¬°áÀ» ²÷´Â´Ù.
	if ((!IsAdminGrade(pObj))) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) return;			// ¾îµå¹Î ´ë»óÀ¸·Î Â¯ºÒ°¡

	// Custom: Check if target is actually a jjang-ed user
	if( pTargetObj->GetAccountInfo()->m_nUGrade != MMUG_STAR ) return;

	pTargetObj->GetAccountInfo()->m_nUGrade = MMUG_FREE;

	if (m_MatchDBMgr.EventJjangUpdate(pTargetObj->GetAccountInfo()->m_nAID, false)) {
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.AddObject(pTargetObj);
		MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_REPLACE, this);
		RouteToStage(pStage->GetUID(), pCmdCacheUpdate);

		MCommand* pCmdUIUpdate = CreateCommand(MC_EVENT_UPDATE_JJANG, MUID(0,0));
		pCmdUIUpdate->AddParameter(new MCommandParameterUID(pTargetObj->GetUID()));
		pCmdUIUpdate->AddParameter(new MCommandParameterBool(false));
		RouteToStage(pStage->GetUID(), pCmdUIUpdate);
	}
	FILE* pFile = fopen("GM_LOGS.txt", "a");
	if (pFile != NULL) {
		char szDateRun[128] = "";
		char szTimeRun[128] = "";
		_strdate(szDateRun);
		_strtime(szTimeRun);
		fprintf_s(pFile, "[%s %s](REMOVE_JJANG) %s removed %s's Jjang\n", szDateRun, szTimeRun, pObj->GetName(), pTargetObj->GetName());
		fclose(pFile);
	}
}*/


void MMatchServer::OnStageGo(const MUID& uidPlayer, unsigned int nRoomNo)
{
	MMatchObject* pChar = GetObject(uidPlayer);
	if (!IsEnabledObject(pChar)) return;
	if (pChar->GetPlace() != MMP_LOBBY) return;

	MMatchChannel* pChannel = FindChannel(pChar->GetChannelUID());
	if (pChannel == NULL) return;

	MMatchStage* pStage = pChannel->GetStage(nRoomNo-1);
	if (pStage) {
		MCommand* pNew = CreateCommand(MC_MATCH_REQUEST_STAGE_JOIN, GetUID());
		pNew->SetSenderUID(uidPlayer);	// ÇÃ·¹ÀÌ¾ûÌ¡ º¸³½ ¸Þ½ÃÁöÀÎ °ÍÃ³·³ À§ÀE
		pNew->AddParameter(new MCommandParameterUID(uidPlayer));
		pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
		Post(pNew);
	}
}

void MMatchServer::OnDuelQueueInfo(const MUID& uidStage, const MTD_DuelQueueInfo& QueueInfo)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_DUEL_QUEUEINFO, MUID(0,0));
	pCmd->AddParameter(new MCmdParamBlob(&QueueInfo, sizeof(MTD_DuelQueueInfo)));
	RouteToBattle(uidStage, pCmd);
}

void MMatchServer::OnQuestSendPing(const MUID& uidStage, unsigned long int t)
{
	MCommand* pCmd = CreateCommand(MC_QUEST_PING, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUInt(t));
	RouteToBattle(uidStage, pCmd);
}

void MMatchServer::SaveGameLog(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	int nMapID = pStage->GetStageSetting()->GetMapIndex();
	int nGameType = (int)pStage->GetStageSetting()->GetGameType();

	if ((MGetMapDescMgr()->MIsCorrectMap(nMapID)) && (MGetGameTypeMgr()->IsCorrectGameType(nGameType)))
	{
		if (pStage->GetStageType() != MST_LADDER && pStage->GetStageType() != MST_LADDERWARS)
		{
			MMatchObject* pMaster = GetObject(pStage->GetMasterUID());

			MAsyncDBJob_InsertGameLog* pJob = new MAsyncDBJob_InsertGameLog(uidStage);
			pJob->Input(pMaster == NULL ? 0 : pMaster->GetCharInfo()->m_nCID,
				MGetMapDescMgr()->GetMapName(nMapID),
				MGetGameTypeMgr()->GetInfo(MMATCH_GAMETYPE(nGameType))->szGameTypeStr);
			PostAsyncJob(pJob);
		}
	}

}

void MMatchServer::SaveGamePlayerLog(MMatchObject* pObj, unsigned int nStageID)
{
	if (!IsEnabledObject(pObj)) return;
	if (nStageID == 0) return;

	MAsyncDBJob_InsertGamePlayerLog* pJob = new MAsyncDBJob_InsertGamePlayerLog;
	pJob->Input(
		nStageID, 
		pObj->GetCharInfo()->m_nCID,
		(GetGlobalClockCount() - pObj->GetCharInfo()->m_nBattleStartTime) / 1000,
		pObj->GetCharInfo()->GetCharGamePlayInfo()->nKillCount,
		pObj->GetCharInfo()->GetCharGamePlayInfo()->nDeathCount,
		pObj->GetCharInfo()->GetCharGamePlayInfo()->nXP,
		pObj->GetCharInfo()->GetCharGamePlayInfo()->nBP
	);
	PostAsyncJob(pJob);
}

void MMatchServer::OnAdminFreeze(const MUID& uidComm, char* pszTargetName)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (!IsEnabledObject(pObj)) return;
	if (pObj->GetStageUID().IsInvalid()) return;
	if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER) return;

	MMatchStage* m_pStage = FindStage(pObj->GetStageUID());
	if (stricmp(pszTargetName, "all") == 0)
	{
		for (MUIDRefCache::iterator itor = m_pStage->GetObjBegin(); itor != m_pStage->GetObjEnd(); ++itor)
		{
			MMatchObject* pPlayer = (MMatchObject*)(*itor).second;
			if (!IsEnabledObject(pPlayer)) continue;
			if (pObj->GetStageUID() != pPlayer->GetStageUID()) continue;
			if (pObj->GetUID() == pPlayer->GetUID()) continue;
			if (pObj->GetPlace() != MMP_BATTLE || pPlayer->GetPlace() != MMP_BATTLE) continue;

			MCommand* pCmd = CreateCommand(MC_ADMIN_FREEZE, MUID(0, 0));
			pCmd->AddParameter(new MCmdParamUID(pPlayer->GetUID()));
			pCmd->AddParameter(new MCmdParamStr(pPlayer->GetName()));
			RouteToListener(pPlayer, pCmd);
		}

		char szBuf[128];
		sprintf(szBuf, "Froze '%s'.", pszTargetName);
		Announce(pObj, szBuf);
	}
	else
	{
		MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
		if(!IsEnabledObject(pTargetObj)) return;
		if (pObj->GetStageUID() != pTargetObj->GetStageUID()) return;
		if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER) return;
		if (pObj->GetPlace() != MMP_BATTLE || pTargetObj->GetPlace() != MMP_BATTLE) return;

		MCommand* pCmd = CreateCommand(MC_ADMIN_FREEZE, MUID(0, 0));
		pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
		pCmd->AddParameter(new MCmdParamStr(pTargetObj->GetName()));
		RouteToListener(pTargetObj, pCmd);

		char szBuf[128];
		sprintf(szBuf, "Froze '%s'.", pszTargetName);
		Announce(pObj, szBuf);
	}
}

void MMatchServer::OnAdminSummon(const MUID& uidComm, char* pszTargetName)
{
	MMatchObject* pObj = GetPlayerByCommUID( uidComm );
	if (!IsEnabledObject(pObj)) return;
	if (pObj->GetStageUID().IsInvalid()) return;
	if (IsAdminGrade(pObj) == false && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER) return;

	MMatchStage* m_pStage = FindStage(pObj->GetStageUID());

	if (strcmp(pszTargetName, "all") == 0)
	{
		for (MUIDRefCache::iterator itor = m_pStage->GetObjBegin(); itor != m_pStage->GetObjEnd(); ++itor)
		{
			MMatchObject* pPlayer = (MMatchObject*)(*itor).second;
			if (!IsEnabledObject(pPlayer)) continue;
			if (pObj->GetStageUID() != pPlayer->GetStageUID()) continue;
			if (pObj->GetUID() == pPlayer->GetUID()) continue;
			if (pObj->GetPlace() != MMP_BATTLE || pPlayer->GetPlace() != MMP_BATTLE) continue;

			MCommand* pCmd = CreateCommand(MC_ADMIN_SUMMON, MUID(0,0));
			pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
			pCmd->AddParameter(new MCmdParamStr(pPlayer->GetName()));
			RouteToListener(pPlayer, pCmd);
		}
		char szBuf[128];
		sprintf( szBuf, "Summoned '%s'.", pszTargetName );
		Announce( pObj, szBuf );
	}
	else
	{	
		MMatchObject* pTargetObj = GetPlayerByName( pszTargetName );
		if (!IsEnabledObject(pTargetObj)) return;
		if (pTargetObj->GetStageUID().IsInvalid()) return;
		if (pObj->GetPlace() != MMP_BATTLE || pTargetObj->GetPlace() != MMP_BATTLE) return;
		if (pObj->GetStageUID() != pTargetObj->GetStageUID()) return;

		MCommand* pCmd = CreateCommand(MC_ADMIN_SUMMON, MUID(0,0));
		pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
		pCmd->AddParameter(new MCmdParamStr(pTargetObj->GetName()));
		RouteToListener(pTargetObj, pCmd);

		char szBuf[128];
		sprintf( szBuf, "Summoned '%s'.", pszTargetName );
		Announce( pObj, szBuf );
	}
}

void MMatchServer::OnAdminGoTo(const MUID& uidComm, char* pszTargetName)
{
	MMatchObject* pObj = GetPlayerByCommUID( uidComm );
	if (!IsEnabledObject(pObj)) return;
	if( pObj->GetStageUID().IsInvalid() ) return;

	MMatchObject* pTargetObj = GetPlayerByName( pszTargetName );
	if (!IsEnabledObject(pTargetObj)) return;
	if (pTargetObj->GetStageUID().IsInvalid()) return;
	if (pObj->GetStageUID() != pTargetObj->GetStageUID()) return;

	// Staff check
	if (IsAdminGrade(pObj) == false && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER) return;
	if (pObj->GetPlace() != MMP_BATTLE || pTargetObj->GetPlace() != MMP_BATTLE) return;

	MCommand* pCmd = CreateCommand(MC_ADMIN_GOTO, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
	pCmd->AddParameter(new MCmdParamStr(pTargetObj->GetName()));
	RouteToListener(pObj, pCmd);

	char szBuf[128];
	sprintf( szBuf, "Teleported to '%s'.", pszTargetName );
	Announce( pObj, szBuf );
}

void MMatchServer::OnAdminSlap(const MUID& uidComm, char* pszTargetName)
{
	MMatchObject* pObj = GetPlayerByCommUID( uidComm );
	MMatchStage* m_pStage = FindStage(pObj->GetStageUID());
	if (!IsEnabledObject(pObj)) return;
	if( pObj->GetStageUID().IsInvalid() ) return;

	// Staff check
	if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER) return;

	if (strcmp( pszTargetName, "all") == 0)
	{
		for (MUIDRefCache::iterator itor = m_pStage->GetObjBegin(); itor != m_pStage->GetObjEnd(); ++itor)
		{
			MMatchObject* pPlayer = (MMatchObject*)(*itor).second;

			if (!IsEnabledObject(pPlayer)) continue;
			if (pObj->GetStageUID() != pPlayer->GetStageUID()) continue;
			if (pObj->GetUID() == pPlayer->GetUID()) continue;
			if (pObj->GetPlace() != MMP_BATTLE || pPlayer->GetPlace() != MMP_BATTLE) continue;
			if (!pObj->CheckAlive()) continue;

			MCommand* pCmd = CreateCommand(MC_ADMIN_SLAP, MUID(0,0));
			pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
			pCmd->AddParameter(new MCmdParamStr(pPlayer->GetName()));
			RouteToListener(pPlayer, pCmd);
		}

		char szBuf[128];
		sprintf( szBuf, "Slapped everyone." );
		Announce( pObj, szBuf );
	}
	else
	{
		MMatchObject* pTargetObj = GetPlayerByName( pszTargetName );

		if (!IsEnabledObject(pTargetObj)) return;
		if (pTargetObj->GetStageUID().IsInvalid()) return;
		if (pObj->GetPlace() != MMP_BATTLE || pTargetObj->GetPlace() != MMP_BATTLE) return;
		if (pObj->GetStageUID() != pTargetObj->GetStageUID()) return;
		if (!pObj->CheckAlive()) return;

		MCommand* pCmd = CreateCommand(MC_ADMIN_SLAP, MUID(0,0));
		pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
		pCmd->AddParameter(new MCmdParamStr(pTargetObj->GetName()));
		RouteToListener(pTargetObj, pCmd);

		char szBuf[128];
		sprintf( szBuf, "Slapped '%s'.", pszTargetName );
		Announce( pObj, szBuf );
	}
}

void MMatchServer::OnAdminSpawn(const MUID& uidComm, char* pszTargetName)
{
	MMatchObject* pObj = GetPlayerByCommUID( uidComm );

	if (!IsEnabledObject(pObj)) return;
	if( pObj->GetStageUID().IsInvalid() ) return;
	if (IsAdminGrade(pObj) == false && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER) return;

	if (strcmp(pszTargetName, "all") == 0)
	{
		MMatchStage* m_pStage = FindStage(pObj->GetStageUID());

		for (MUIDRefCache::iterator itor = m_pStage->GetObjBegin(); itor != m_pStage->GetObjEnd(); ++itor)
		{
			MMatchObject* pTargetObj = (MMatchObject*)(*itor).second;

			if (!IsEnabledObject(pTargetObj)) continue;
			if (pTargetObj->GetStageUID().IsInvalid()) continue;
			if (pObj->GetStageUID() != pTargetObj->GetStageUID()) continue;
			if (pObj->GetPlace() != MMP_BATTLE || pTargetObj->GetPlace() != MMP_BATTLE) continue;
			if (IsAdminGrade(pTargetObj) || pTargetObj->GetAccountInfo()->m_nUGrade == MMUG_GAMEMASTER) continue;

			pTargetObj->SetLastSpawnTime(timeGetTime());
			pTargetObj->SetAlive(true);
			pTargetObj->ResetCustomItemUseCount();

			// Custom: New admin_spawn
			MCommand* pCmd = CreateCommand(MC_ADMIN_SPAWN_RESPONSE, MUID(0, 0));
			pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
			pCmd->AddParameter(new MCmdParamUID(pTargetObj->GetUID()));
			RouteToBattle(pObj->GetStageUID(), pCmd);
		}

		char szBuf[128];
		sprintf(szBuf, "Spawned everyone.");
		Announce(pObj, szBuf);
	}
	else
	{
		MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);

		if (!IsEnabledObject(pTargetObj)) return;
		if (pTargetObj->GetStageUID().IsInvalid()) return;
		if (pObj->GetStageUID() != pTargetObj->GetStageUID()) return;
		if (pObj->GetPlace() != MMP_BATTLE || pTargetObj->GetPlace() != MMP_BATTLE) return;

		pTargetObj->SetLastSpawnTime(timeGetTime());
		pTargetObj->SetAlive(true);
		pTargetObj->ResetCustomItemUseCount();

		// Custom: New admin_spawn
		MCommand* pCmd = CreateCommand(MC_ADMIN_SPAWN_RESPONSE, MUID(0, 0));
		pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
		pCmd->AddParameter(new MCmdParamUID(pTargetObj->GetUID()));
		RouteToBattle(pObj->GetStageUID(), pCmd);

		char szBuf[128];
		sprintf(szBuf, "Spawned '%s'.", pszTargetName);
		Announce(pObj, szBuf);
	}
}

void MMatchServer::OnAdminPKick(const MUID& uidComm, char* pszTargetName, bool bBan)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (!IsEnabledObject(pObj)) return;
	if (pObj->GetStageUID().IsInvalid()) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (!IsEnabledObject(pTargetObj)) return;
	if (bBan && pTargetObj->GetStageUID().IsInvalid()) return;
	if (bBan && pObj->GetStageUID() != pTargetObj->GetStageUID()) return;

	// Event Team support
	if (IsAdminGrade(pObj) == false && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER) return;
	if (pObj->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());

	MMatchServer::GetInstance()->Announce( pObj, "Action completed." );
}

void MMatchServer::OnAdminSilence(const MUID& uidComm, int nDurationMin)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (!IsEnabledObject(pObj)) return;
	if (pObj->GetStageUID().IsInvalid()) return;

	// Event Team support
	if (IsAdminGrade(pObj) == false && pObj->GetAccountInfo()->m_nUGrade != MMUG_GAMEMASTER) return;
	if (pObj->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER) return;

	if (nDurationMin < 0 || nDurationMin > 5)
	{
		Announce(pObj, "Invalid operation. Permitted silence duration: 1-5 minutes. (0 min to forcefully disable if enabled)");
		return;
	}

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (!pStage) return;

	char szMsg[256];
	memset(szMsg, 0, sizeof(szMsg));

	bool bSilence = true;
	DWORD dwTime = timeGetTime();

	if (nDurationMin != 0 && dwTime < pStage->GetSilenceEndTime())
	{
		sprintf_s(szMsg, "Room is already silenced. %d seconds remaining.", (pStage->GetSilenceEndTime() - dwTime) / 1000);
		Announce(pObj, szMsg);
		return;
	}

	if (nDurationMin == 0)
	{
		if (dwTime < pStage->GetSilenceEndTime())
		{
			bSilence = false;
			pStage->SetSilenceEndTime(0);
		}
		else
		{
			sprintf_s(szMsg, "Room is not silenced.");
			Announce(pObj, szMsg);
			return;
		}
	}
	else
	{
		pStage->SetSilenceEndTime(dwTime + (nDurationMin * 60 * 1000));
	}

	if (bSilence) {
		sprintf_s(szMsg, "Room has been silenced by '%s' for %d minutes.", pObj->GetName(), nDurationMin);
	} else {
		sprintf_s(szMsg, "Room has been unsilenced by '%s'.", pObj->GetName());
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_ANNOUNCE, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUInt(0));
	pCmd->AddParameter(new MCmdParamStr(szMsg));
	RouteToStage(pStage->GetUID(), pCmd);
}
#ifdef _NOLEAD
void MMatchServer::OnNoLead(const MUID& uidComm, const MUID& uidTarget, const MVector& pos, int damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);

	if (pObj == NULL) return;
	if (pObj->GetCharInfo() == NULL) return;
	if (pObj->GetStageUID().IsInvalid()) return;

	MMatchObject* pTarget = GetObject(uidTarget);

	if (pTarget == NULL) return;
	if (pTarget->GetCharInfo() == NULL) return;
	if (pTarget->GetStageUID().IsInvalid()) return;

	if (pObj->GetStageUID() != pTarget->GetStageUID())
		return;

	if (pObj->GetPlace() != MMP_BATTLE || pTarget->GetPlace() != MMP_BATTLE)
		return;

	MCommand* pCmd = CreateCommand(MC_PEER_NOLEAD, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
	pCmd->AddParameter(new MCmdParamUID(pTarget->GetUID()));
	pCmd->AddParameter(new MCmdParamPos(pos.x, pos.y, pos.z));
	pCmd->AddParameter(new MCmdParamInt(damageType));
	pCmd->AddParameter(new MCmdParamInt(weaponType));
	pCmd->AddParameter(new MCmdParamFloat(fDamage));
	pCmd->AddParameter(new MCmdParamFloat(fPiercingRatio));
	pCmd->AddParameter(new MCmdParamInt(nMeleeType));

	RouteToListener(pTarget, pCmd);
}
void MMatchServer::OnNoLeadBlob(const MUID& uidComm, void* pBlobArray)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);

	if (pObj == NULL) return;
	if (pObj->GetCharInfo() == NULL) return;
	if (pObj->GetStageUID().IsInvalid()) return;

	// Blob-peeking
	int nSize = MGetBlobArrayCount(pBlobArray);

	MCommand* pCmd = CreateCommand(MC_PEER_NOLEADBLOB, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUID(uidComm));
	pCmd->AddParameter(new MCmdParamBlob(pBlobArray, MGetBlobArraySize(pBlobArray)));

	// BATTLE ONLY!
	RouteToBattle(pObj->GetStageUID(), pCmd);
	/*
	for (int i = 0; i < nSize; ++i)
	{
		MTD_ShotInfo* pInfo = (MTD_ShotInfo*)MGetBlobArrayElement(pBlobArray, i);

		MMatchObject* pTarget = GetObject( pInfo->uid );

		if( pTarget == NULL ) return;
		if( pTarget->GetCharInfo() == NULL ) return;
		if( pTarget->GetStageUID().IsInvalid() ) return;

		if( pObj->GetStageUID() != pTarget->GetStageUID() )
			return;

		if( pObj->GetPlace() != MMP_BATTLE || pTarget->GetPlace() != MMP_BATTLE )
			return;

		MCommand* pCmd = CreateCommand(MC_PEER_NOLEADBLOB, MUID(0,0));
		pCmd->AddParameter(new MCmdParamUID(uidComm));
		pCmd->AddParameter(new MCmdParamBlob(pBlobArray, MGetBlobArraySize(pBlobArray)));

		RouteToListener(pTarget, pCmd);
	}
	*/
}
#endif
void MMatchServer::OnRequestFlagCap(const MUID& uidPlayer, const int nItemID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if(pStage->GetStageSetting())
	{
		if(pStage->GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_CTF)
		{
			MMatchRuleTeamCTF* pRule = (MMatchRuleTeamCTF*)pStage->GetRule();
			if(pRule)
			{
				pRule->OnObtainWorldItem( pObj, nItemID, NULL );
			}
		}
	}
}
#ifdef _MAGICBOX
void MMatchServer::OnDropGunGameRequestItem(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if (pStage->GetStageSetting())
	{
		if (pStage->GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_DROPMAGIC)
		{
			MMatchRuleDropGunGame* pRule = (MMatchRuleDropGunGame*)pStage->GetRule();
			if (pRule)
			{
				pRule->OnObtainWorldItem(pObj);
			}
		}
	}
}
#endif

//challengequest
void MMatchServer::OnStartCQ(MMatchStage* m_pStage, int nRounds)
{
	MMatchStageSetting* pSetting = m_pStage->GetStageSetting();
	pSetting->SetRoundMax(nRounds);
	MCommand* pCmd = CreateCmdResponseStageSetting(m_pStage->GetUID());
	RouteToStage(m_pStage->GetUID(), pCmd);
}

void MMatchServer::OnStartCQForSingle(MUID uidListener, MMatchStage* m_pStage, int nRounds)
{
	MMatchObject* pObj = GetObject(uidListener);
	if (!IsEnabledObject(pObj)) return;

	MMatchStageSetting* pSetting = m_pStage->GetStageSetting();
	pSetting->SetRoundMax(nRounds);
	MCommand* pCmd = CreateCmdResponseStageSetting(m_pStage->GetUID());
	RouteToListener(pObj, pCmd);
}

#include "SkillMap.h"
void MMatchServer::RequestUpdateSkillMapBestTime(const MUID& player, const int& mapID, const unsigned int bestTime)
{
	MMatchObject* playerObj = GetObject(player);
	if (!IsEnabledObject(playerObj))
		return;

	//if (!MGetSkillMap()->FindMap(mapID)) {
	//	mlog("Error finding mapName");
	//	return;
	//}

	GetDBMgr()->UpdateSkillMapBestTIme(playerObj->GetCharInfo()->m_nCID, MGetMapDescMgr()->GetMapID(mapID), bestTime);
}

void MMatchServer::ResponseSkillMapBestTime(const MUID& player, const char* mapName)
{
	MMatchObject* playerObj = GetObject(player);
	if (!IsEnabledObject(playerObj))
		return;

	MMatchStage* stage = FindStage(playerObj->GetStageUID());
	if (stage == nullptr)
		return;

	if (!MGetSkillMap()->FindMap(mapName))
		return;

	unsigned int* outBestTime;

	if (!GetDBMgr()->GetSkillMapBestTime(playerObj->GetCharInfo()->m_nCID, MGetMapDescMgr()->GetMapID(mapName), outBestTime))
		return;

	MCommand* command = CreateCommand(MC_MATCH_RESPONSE_SKILLMAP_BESTTIME, MUID(0, 0));
	command->AddParameter(new MCmdParamUID(playerObj->GetUID()));
	command->AddParameter(new MCmdParamUInt(*outBestTime));

	MGetMatchServer()->RouteToBattle(stage->GetUID(), command);
}

#ifdef _ROCKETGUIDED
MUID MMatchServer::SelectTarget(const MUID& uidStage, const MUID& owner)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return MUID(0, 0);

	if (pStage->GetObjCount() == 0)
		return MUID(0, 0);

	MMatchObject* pOwner = MMatchServer::GetInstance()->GetObject(owner);
	if (pOwner == NULL)
		return MUID(0, 0);

	vector<MUID> vtCandidateUIDs;

	for (MUIDRefCache::iterator itor = pStage->GetObjBegin(); itor != pStage->GetObjEnd(); itor++)
	{
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		//if (stricmp(pObj->GetName(), pOwner->GetName() != 0))
		//{
		if (pObj->GetEnterBattle() == false) continue;
		if (pObj->CheckAlive())
		{
			vtCandidateUIDs.push_back((*itor).first);
		}
		//}
	}
	if (vtCandidateUIDs.size() == 0) return MUID(0, 0);

	int nRand = rand() % vtCandidateUIDs.size();
	return vtCandidateUIDs[nRand];
}

void MMatchServer::OnGuidedMissle(MUID Owner)
{
	MUID m_TargetID;

	MMatchObject* pObj = GetObject(Owner);
	if (pObj == NULL)
		return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL)
		return;

	if (pStage->GetState() != STAGE_STATE_RUN) return;

	m_TargetID = SelectTarget(pStage->GetUID(), Owner);
	//mlog("OnGuidedMissle - MMatchServer_Stage.cpp - Target Name %s\n", pObj->GetName());

	MCommand* pNew = CreateCommand(MC_RESPONSE_GUIDEDMISSLE, MUID(0, 0));
	pNew->AddParameter(new MCommandParameterUID(Owner));
	pNew->AddParameter(new MCommandParameterUID(m_TargetID));
	RouteToStage(pStage->GetUID(), pNew);
}
#endif