#include "stdafx.h"
#include "MMatchRuleQuestChallenge.h"
#include "MNewQuestNpcManager.h"
#include "MNewQuestPlayerManager.h"
#include "MSharedCommandTable.h"
#include "MMatchWorldItemDesc.h"
#include "MMatchFormula.h"
#include "MMatchStage.h"
#define FILENAME_NPC2_XML		"NewQuest/npc2.xml"
#define FILENAME_SCENARIO2_XML	"NewQuest/scenario2.xml"
MActorDefManager MMatchRuleQuestChallenge::ms_actorDefMgr;
MNewQuestScenarioManager MMatchRuleQuestChallenge::ms_scenarioMgr;

void MMatchRuleQuestChallenge::InitActorDefMgr()
{
	if (!ms_actorDefMgr.ReadXml(NULL, FILENAME_NPC2_XML))
	{
		mlog("Read new quest file %s Failed\n", FILENAME_NPC2_XML);
		_ASSERT(0); return;
	}
}

void MMatchRuleQuestChallenge::InitScenarioMgr()
{
	if (!ms_scenarioMgr.ReadXml(NULL, FILENAME_SCENARIO2_XML, MMatchServer::GetInstance()->GetQuest()->GetDropTable()))
	{
		mlog("Read new quest file %s Failed\n", FILENAME_SCENARIO2_XML);
		_ASSERT(0); return;
	}
}

MNewQuestScenarioManager* MMatchRuleQuestChallenge::GetScenarioMgr()
{
	return &ms_scenarioMgr;
}


MMatchRuleQuestChallenge::MMatchRuleQuestChallenge(MMatchStage* pStage)
: MMatchRule(pStage)
, m_pPlayerMgr(NULL)
, m_pNpcMgr(NULL)
, m_pScenario(NULL)
, m_nCurrSector(0)
{

}

MMatchRuleQuestChallenge::~MMatchRuleQuestChallenge()
{
}

// 모든 라운드가 끝나면 false 반환
bool MMatchRuleQuestChallenge::RoundCount()
{
	if (!m_pScenario)
	{
		_ASSERT(0);
		return false;
	}

	if (m_nCurrSector < m_pScenario->GetNumSector())
	{
		return true;
	}

	return false;
}

void MMatchRuleQuestChallenge::OnBegin()
{
	if (m_pPlayerMgr)
	{
		delete m_pPlayerMgr;
		m_pPlayerMgr = 0;
	}
	m_pPlayerMgr = new MNewQuestPlayerManager;
	if (m_pNpcMgr)
	{
		delete m_pNpcMgr;
		m_pNpcMgr = 0;
	}
	m_pNpcMgr = new MNewQuestNpcManager;
	m_nCurrSector = 0;

	//todok 플레이어 접속 종료 및 방 나갈때 제거해줘야 한다

	// 시나리오 세팅
	m_pScenario = ms_scenarioMgr.GetScenario( m_pStage->GetMapName());
	if (!m_pScenario)
		m_pScenario = ms_scenarioMgr.GetScenario( ms_scenarioMgr.GetDefaultScenarioName());

	// 게임에 존재하는 플레이어 추가
	for (MUIDRefCache::iterator it=m_pStage->GetObjBegin(); it!=m_pStage->GetObjEnd(); ++it)
	{
		MUID uidChar = it->first;

		MMatchObject* pObj = MMatchServer::GetInstance()->GetObject(uidChar);
		if (IsAdminGrade(pObj)  && pObj->GetAccountInfo()->m_nUGrade == MMUG_GAMEMASTER
			&& pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide)) continue;

		m_pPlayerMgr->AddPlayer(pObj);
	}
	MGetMatchServer()->OnStartCQ(m_pStage, m_pScenario->GetNumSector());
	CollectStartingQuestChallengeGameLogInfo();
//	m_dwCurrTime = MMatchServer::GetInstance()->GetTickTime();
}

void MMatchRuleQuestChallenge::OnEnd()
{

	for (MUIDRefCache::iterator i = m_pStage->GetObjBegin(); i != m_pStage->GetObjEnd(); i++)
	{
		MMatchObject* pObj = NULL;

		pObj = (MMatchObject*)(*i).second;
		if (IsEnabledObject(pObj))
			MGetMatchServer()->ResponseCharacterItemList(pObj->GetUID());
	}
	//requires user to successfully complete the game itself
	if (m_nCurrSector == m_pScenario->GetNumSector() && m_pNpcMgr->GetNumNpc() == 0)
	{
		PostInsertQuestChallengeGameLogAsyncJob();
	}

	if (m_pNpcMgr)
	{
		delete m_pNpcMgr;
		m_pNpcMgr = 0;
	}
	if (m_pPlayerMgr)
	{
		delete m_pPlayerMgr;
		m_pPlayerMgr = 0;
	}
}

void MMatchRuleQuestChallenge::OnRoundBegin()
{
	// PLAY 상태로 넘어갈때 전부 스폰 시킨다
	ProcessNpcSpawning();

	MMatchRule::OnRoundBegin();
}

void MMatchRuleQuestChallenge::OnRoundEnd()
{
	if (m_pStage)
		m_pStage->m_WorldItemManager.CleanUpCQItems();
	// 마지막 섹터가 아니라면 다음 섹터로 이동하게 한다
	if (m_nCurrSector < m_pScenario->GetNumSector())
	{
		RouteExpToPlayers();
		++m_nCurrSector;
		RouteMoveToNextSector(m_nCurrSector);
	}

	MMatchRule::OnRoundEnd();
}

void MMatchRuleQuestChallenge::OnQuestEnterBattle(MUID& uidChar)
{
}

bool MMatchRuleQuestChallenge::CheckPlayersAlive()
{
	int nAliveCount = 0;
	MMatchObject* pObj;
	for (MUIDRefCache::iterator i=m_pStage->GetObjBegin(); i!=m_pStage->GetObjEnd(); i++) 
	{
		pObj = (MMatchObject*)(*i).second;
		if (pObj->GetEnterBattle() == false) continue;	// 배틀참가하고 있는 플레이어만 체크
		if (IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade ==
			MMUG_GAMEMASTER && pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide)) continue;

		if (pObj->CheckAlive()==true)
		{
			++nAliveCount;
		}
	}

	// 모두 죽었으면 리셋
	if (nAliveCount == 0) return false;

	return true;
}

//Moved the iterator to OnCheckEnableBattleCondtion, using this for what it was intended
bool MMatchRuleQuestChallenge::OnRun()
{
	DWORD nClock = MMatchServer::GetInstance()->GetGlobalClockCount();

	switch (GetRoundState())
	{
		case MMATCH_ROUNDSTATE_PREPARE:
		{
			if (GetStage()->CheckBattleEntry() == true)
			{
				if (OnCheckEnableBattleCondition())
				{
					SetRoundState(MMATCH_ROUNDSTATE_COUNTDOWN);
				}
				else
				{
					SetRoundState(MMATCH_ROUNDSTATE_FREE);
				}
			}
			return true;
		}
		case MMATCH_ROUNDSTATE_COUNTDOWN:
		{
			if (nClock - GetRoundStateTimer() > 3*1000)
			{
				SetRoundState(MMATCH_ROUNDSTATE_PLAY);
			}
			return true;
		}
		case MMATCH_ROUNDSTATE_PLAY:
		{
			if (!OnCheckEnableBattleCondition())
			{
				SetRoundState(MMATCH_ROUNDSTATE_FREE);
			}
			if (OnCheckRoundFinish())
			{
				SetRoundState(MMATCH_ROUNDSTATE_FINISH);
			}
			if (!CheckPlayersAlive())
			{
				SetRoundState(MMATCH_ROUNDSTATE_EXIT);
			}
			return true;
		}
		case MMATCH_ROUNDSTATE_FINISH:
		{
			if (nClock - GetRoundStateTimer() > 1000)
			{
				if (m_nCurrSector < m_pScenario->GetNumSector())
				{
					if (nClock - GetRoundStateTimer() > 6000)
					{
						SetRoundState(MMATCH_ROUNDSTATE_PREPARE);
					}
				}
				else
				{
					SetRoundState(MMATCH_ROUNDSTATE_EXIT);
				}
			}
			return true;
		}
		case MMATCH_ROUNDSTATE_FREE:
		{
			if (OnCheckEnableBattleCondition())
			{
				SetRoundState(MMATCH_ROUNDSTATE_PREPARE);
			}
			return true;
		}
		case MMATCH_ROUNDSTATE_EXIT:
		{
			return false;
		}
	}
	return false;
}

bool MMatchRuleQuestChallenge::OnCheckRoundFinish()
{
	// 전부 소탕하면 라운드를 끝낸다
	if (m_pNpcMgr->GetNumNpc() == 0)
	{
		for (MUIDRefCache::iterator i = m_pStage->GetObjBegin(); i != m_pStage->GetObjEnd(); i++)
		{
			MUID uidChar = i->first;
			if (!m_pPlayerMgr->IsPlayerInMap(uidChar))
			{
				MMatchObject* pObj = MMatchServer::GetInstance()->GetObject(uidChar);
				if (!IsEnabledObject(pObj)) continue;
				if (pObj->GetPlace() == MMP_BATTLE) //Custom: Make sure the user is actually in the game
				{
					m_pPlayerMgr->AddPlayer(pObj);
				}
			}
		}
		return true;
	}
	return false;
}

bool MMatchRuleQuestChallenge::OnCheckEnableBattleCondition()
{
	for (MUIDRefCache::iterator i = m_pStage->GetObjBegin(); i != m_pStage->GetObjEnd(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (!IsEnabledObject(pObj)) continue;
		if (pObj->GetEnterBattle() == false) continue;
		if (IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade == MMUG_GAMEMASTER
			&& pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide)) continue;

		MUID uidChar = i->first;
		if (m_pPlayerMgr && !m_pPlayerMgr->IsPlayerInMap(uidChar))
		{
			m_pPlayerMgr->AddPlayer(pObj);
			if (m_pStage && m_pScenario)
				MGetMatchServer()->OnStartCQForSingle(pObj->GetUID(), m_pStage, m_pScenario->GetNumSector());

			RouteMoveToNextSectorSingle(pObj->GetUID(), m_nCurrSector);
			SpawnNpcsForSingle(pObj->GetUID());
			pObj->SetAlive(false);
		}
	}
	return true;
}


void MMatchRuleQuestChallenge::ProcessNpcSpawning()
{
	MNewQuestSector* pCurrSector = NULL;
	if (m_pScenario)
	{
		MNewQuestSector* pCurrSector = m_pScenario->GetSector(m_nCurrSector);
		if (pCurrSector)
		{
			int numSpawnType = pCurrSector->GetNumSpawnType();
			for (int i = 0; i < numSpawnType; ++i)
			{
				MNewQuestSpawn* pSpawn = pCurrSector->GetSpawnByIndex(i);
				if (pSpawn)
				{
					for (int k = 0; k < pSpawn->GetNum(); ++k)
					{
						MQuestDropItem item;
						MMatchServer::GetInstance()->GetQuest()->GetDropTable()->Roll(item, pSpawn->GetDropTableId(), 0);

						SpawnNpc(pSpawn->GetActor(), i, k, item.nID, pSpawn->GetBoss());
					}
				}
			}
		}
	}
}

void MMatchRuleQuestChallenge::SpawnNpcsForSingle(MUID playerUid)
{
	MNewQuestSector* pCurrSector = NULL;
	if (m_pScenario)
	{
		MNewQuestSector* pCurrSector = m_pScenario->GetSector(m_nCurrSector);
		if (pCurrSector)
		{
			if (m_pNpcMgr)
			{
				ItorNpc it = m_pNpcMgr->m_mapNpc.begin();
				for (ItorNpc it = m_pNpcMgr->m_mapNpc.begin(); it != m_pNpcMgr->m_mapNpc.end(); ++it)
				{
					if ((*it).second == NULL)
						continue;

					RouteSpawnNpcSingle(playerUid, (*it).second->GetUID(), (*it).second->GetController(), (*it).second->GetActorType().c_str(), (*it).second->GetCustomIndex(), (*it).second->GetIndex(), (*it).second->IsBoss());
				}
			}
		}
	}
}

void MMatchRuleQuestChallenge::SpawnNpc(const char* szActorDef, int nCustomSpawnTypeIndex, int nSpawnIndex, int nDropItemId, bool bIsBoss)
{
	if (!m_pNpcMgr || !m_pPlayerMgr)
		return;

	// 적당한 컨트롤러를 찾는다. 컨트롤러할 사람이 없으면 스폰을 취소
	MUID uidController = m_pPlayerMgr->FindSuitableController();
	if (uidController.IsInvalid())
		return;

	// npc 매니저에 실제 npc 생성 요청. 아까 찾은 컨트롤러 uid를 넘겨준다
	MUID uidNpc = MMatchServer::GetInstance()->UseUID();
	m_pNpcMgr->AddNpcObject(uidNpc, uidController, nCustomSpawnTypeIndex, nSpawnIndex, bIsBoss, szActorDef, nDropItemId);
	m_pPlayerMgr->IncreaseNpcControl(uidController, uidNpc);

	//todok szActorDef를 이름이 아닌 인덱스로 보내도록 수정하자
	RouteSpawnNpc(uidNpc, uidController, szActorDef, nCustomSpawnTypeIndex, nSpawnIndex, bIsBoss);
}

void MMatchRuleQuestChallenge::SpawnNpcSummon(const char* szActorDef, int num, MShortVector nSpawnPos, MShortVector nSpawnDir)
{
	if (!m_pNpcMgr || !m_pPlayerMgr)
		return;

	// 적당한 컨트롤러를 찾는다. 컨트롤러할 사람이 없으면 스폰을 취소
	MUID uidController = m_pPlayerMgr->FindSuitableController();
	if (uidController.IsInvalid())
		return;

	// npc 매니저에 실제 npc 생성 요청. 아까 찾은 컨트롤러 uid를 넘겨준다
	MUID uidNpc = MMatchServer::GetInstance()->UseUID();
	m_pNpcMgr->AddNpcObject(uidNpc, uidController, 0, 0, false, szActorDef,0); //change this back later
	m_pPlayerMgr->IncreaseNpcControl(uidController, uidNpc);

	//todok szActorDef를 이름이 아닌 인덱스로 보내도록 수정하자
	RouteSpawnNpcSummon(uidNpc, uidController, num, szActorDef, nSpawnPos,nSpawnDir);
}

void MMatchRuleQuestChallenge::RouteSpawnNpc(MUID uidNPC, MUID uidController, const char* szNpcDefName, int nCustomSpawnTypeIndex, int nSpawnIndex, bool bIsBoss)
{
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_NEWQUEST_NPC_SPAWN, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidController));
	pCmd->AddParameter(new MCmdParamUID(uidNPC));
	pCmd->AddParameter(new MCmdParamStr(szNpcDefName));
	pCmd->AddParameter(new MCmdParamUChar(nCustomSpawnTypeIndex));
	pCmd->AddParameter(new MCmdParamUChar(nSpawnIndex));
	pCmd->AddParameter(new MCmdParamBool(bIsBoss));
	MMatchServer::GetInstance()->RouteToBattle(m_pStage->GetUID(), pCmd);
}

void MMatchRuleQuestChallenge::RouteSpawnNpcSummon(MUID uidNpc, MUID uidController, int Num, const char* szNpcDefName, MShortVector pos,MShortVector dir)
{
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_NEWQUEST_REQUEST_NPC_SPAWN, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUID(uidController));
	pCmd->AddParameter(new MCmdParamUID(uidNpc));
	pCmd->AddParameter(new MCmdParamInt(0));
	pCmd->AddParameter(new MCmdParamInt(0));
	pCmd->AddParameter(new MCmdParamStr(szNpcDefName));
	pCmd->AddParameter(new MCmdParamShortVector(pos.x, pos.y, pos.z));
	pCmd->AddParameter(new MCmdParamShortVector(dir.x, dir.y, dir.z));
	MMatchServer::GetInstance()->RouteToBattle(m_pStage->GetUID(), pCmd);
}

void MMatchRuleQuestChallenge::RouteSpawnNpcSingle(MUID listener, MUID uidNPC, MUID uidController, const char* szNpcDefName, int nCustomSpawnTypeIndex, int nSpawnIndex, bool bIsBoss)
{

	MMatchObject* pObj = MMatchServer::GetInstance()->GetObject(listener);
	if (!IsEnabledObject(pObj)) return;

	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_NEWQUEST_NPC_SPAWN, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUID(uidController));
	pCmd->AddParameter(new MCmdParamUID(uidNPC));
	pCmd->AddParameter(new MCmdParamStr(szNpcDefName));
	pCmd->AddParameter(new MCmdParamUChar(nCustomSpawnTypeIndex));
	pCmd->AddParameter(new MCmdParamUChar(nSpawnIndex));
	pCmd->AddParameter(new MCmdParamBool(bIsBoss));
	MMatchServer::GetInstance()->RouteToListener(pObj, pCmd);
}

void MMatchRuleQuestChallenge::RouteNpcDead(MUID uidNPC, MUID uidKiller)
{
	MCommand* pNew = MMatchServer::GetInstance()->CreateCommand(MC_NEWQUEST_NPC_DEAD, MUID(0,0));
	pNew->AddParameter(new MCmdParamUID(uidKiller));
	pNew->AddParameter(new MCmdParamUID(uidNPC));
	MMatchServer::GetInstance()->RouteToBattle(m_pStage->GetUID(), pNew);
}

void MMatchRuleQuestChallenge::RouteMoveToNextSector(int nSectorID)
{
	MCommand* pNew = MMatchServer::GetInstance()->CreateCommand(MC_NEWQUEST_MOVE_TO_NEXT_SECTOR, MUID(0,0));
	pNew->AddParameter(new MCmdParamInt(nSectorID));
	pNew->AddParameter(new MCmdParamBool(false));
	MMatchServer::GetInstance()->RouteToBattle(m_pStage->GetUID(), pNew);
}
	
void MMatchRuleQuestChallenge::RouteMoveToNextSectorSingle(MUID uidListener, int nSectorID)
{
	MMatchObject* pObj = MMatchServer::GetInstance()->GetObject(uidListener);
	if (!IsEnabledObject(pObj)) return;

	MCommand* pNew = MMatchServer::GetInstance()->CreateCommand(MC_NEWQUEST_MOVE_TO_NEXT_SECTOR, MUID(0, 0));
	pNew->AddParameter(new MCmdParamInt(nSectorID));
	pNew->AddParameter(new MCmdParamBool(true));
	MMatchServer::GetInstance()->RouteToListener(pObj, pNew);
}

void MMatchRuleQuestChallenge::RouteExpToPlayers()
{
	for (MUIDRefCache::iterator it=m_pStage->GetObjBegin(); it!=m_pStage->GetObjEnd(); ++it)
	{
		MUID uidChar = it->first;

		MMatchObject* pObj = MMatchServer::GetInstance()->GetObject(uidChar);
		if (IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade == MMUG_GAMEMASTER &&
			pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide)) continue;
		if ((!IsEnabledObject(pObj)) || (!pObj->CheckAlive())) continue;

		//EXP
		int nXP = 0;
		int nBP = 0;

		MNewQuestSector* pCurrSector = NULL;
		if (m_pScenario)
		{
			MNewQuestSector* pCurrSector = m_pScenario->GetSector(m_nCurrSector);
			if (pCurrSector)
			{
				nXP = pCurrSector->GetRewardXp();
				nBP = pCurrSector->GetRewardBp();
			}
		}

		const float fXPBonusRatio = MMatchFormula::CalcXPBonusRatio(pObj, MIBT_QUEST);
		const float fBPBonusRatio = MMatchFormula::CalcBPBounsRatio(pObj, MIBT_QUEST);


		int nExpBonus = (int)(nXP * fXPBonusRatio);
		nXP += nExpBonus;

		int nBPBonus = nBP * fBPBonusRatio;
		nBP += nBPBonus;

		MGetMatchServer()->ProcessPlayerXPBP(m_pStage, pObj, nXP, nBP);

		
		int nExpPercent = MMatchFormula::GetLevelPercent(pObj->GetCharInfo()->m_nXP, 
															pObj->GetCharInfo()->m_nLevel);

		MCommand* pNewCmd = MMatchServer::GetInstance()->CreateCommand(MC_QUEST_SECTOR_BONUS, MUID(0,0));
		pNewCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
		pNewCmd->AddParameter(new MCmdParamUInt(nXP));
		pNewCmd->AddParameter(new MCmdParamUInt(nBP));
		pNewCmd->AddParameter(new MCmdParamUInt(nExpPercent));
		MMatchServer::GetInstance()->RouteToListener( pObj, pNewCmd );
	}
}

void MMatchRuleQuestChallenge::OnCommand(MCommand* pCommand)
{
	switch (pCommand->GetID())
	{
		case MC_NEWQUEST_REQUEST_NPC_DEAD:
		{
			MUID uidSender = pCommand->GetSenderUID();
			MUID uidKiller, uidNPC;
			MShortVector s_pos;
			pCommand->GetParameter(&uidKiller, 0, MPT_UID);
			pCommand->GetParameter(&uidNPC, 1, MPT_UID);
			pCommand->GetParameter(&s_pos, 2, MPT_SVECTOR);
			MVector pos = MVector((float)s_pos.x, (float)s_pos.y, (float)s_pos.z);

			if (!m_pNpcMgr || !m_pPlayerMgr)
				break;

			if (GetRoundState() != MMATCH_ROUNDSTATE_PLAY)
				break;

			MNewQuestNpcObject* pNpc = m_pNpcMgr->GetNpc(uidNPC);
			if (!pNpc)
			{
				_ASSERT(0); break;
			}

			// 커맨드 보낸이는 반드시 해당 npc의 컨트롤러여야 한다. 아니라면 해킹이나 버그 상황
			if (pNpc->GetController() != uidSender)
			{
				_ASSERT(0); break;
			}

			// 드롭 아이템 생성
			DropItemByNpcDead(uidKiller, pNpc->GetDropItemId(), pos);

			m_pNpcMgr->DeleteNpcObject(uidNPC);
			m_pPlayerMgr->DecreaseNpcControl(uidSender);

			RouteNpcDead(uidNPC, uidKiller);
			break;
		}
		case MC_NEWQUEST_REQUEST_NPC_SPAWN:
		{
			MUID uidController;
			MUID uidNpc;
			char szActorDefName[128];
			unsigned char nCustomSpawnTypeIndex, nSpawnIndex;
			bool bIsBoss;
			int num, index;
			MShortVector s_pos,s_dir;

			if (!m_pNpcMgr || !m_pPlayerMgr)
				break;

			if (GetRoundState() != MMATCH_ROUNDSTATE_PLAY)
				break;

			pCommand->GetParameter(&uidController, 0, MPT_UID);
			pCommand->GetParameter(&uidNpc, 1, MPT_UID);
			pCommand->GetParameter(&num, 2, MPT_INT);
			pCommand->GetParameter(&index, 3, MPT_INT);
			pCommand->GetParameter(szActorDefName, 4, MPT_STR, sizeof(szActorDefName));
			pCommand->GetParameter(&s_pos, 5, MPT_SVECTOR);
			pCommand->GetParameter(&s_dir, 6, MPT_SVECTOR);

			MNewQuestNpcObject* pNpc = m_pNpcMgr->GetNpc(uidNpc);
			if (!pNpc)
			{
				mlog("not npc uid\n");
				break;
				//_ASSERT(0); break;
			}

			// 커맨드 보낸이는 반드시 해당 npc의 컨트롤러여야 한다. 아니라면 해킹이나 버그 상황
			if (pNpc->GetController() != pCommand->GetSenderUID())
			{
				//mlog("not same controller\n");
				/*_ASSERT(0);*/ break;
			}

			MNewQuestSector* pCurrSector = NULL;
			if (m_pScenario)
			{
				for (int k = 0; k < num; k++)
				{
					SpawnNpcSummon(szActorDefName, 1, s_pos,s_dir);
				}
			}
			break;
		}
	}
}

void MMatchRuleQuestChallenge::DropItemByNpcDead(const MUID& uidKiller, int nWorldItemId, const MVector& pos)
{
	// 월드아이템인지 검사하자
	if (!MGetMatchWorldItemDescMgr()->GetItemDesc(nWorldItemId))
	{
		MMatchObject* pPlayer = MMatchServer::GetInstance()->GetObject(uidKiller);
		if (!IsEnabledObject(pPlayer) || nWorldItemId == 0)
			return;
		int nSpawnItemID = QUEST_WORLDITEM_ITEMBOX_ID;
		int nQuestItemID = nWorldItemId;
		int nRentPeriodHour = RENT_MINUTE_PERIOD_UNLIMITED;

		int nWorldItemExtraValues[WORLDITEM_EXTRAVALUE_NUM];
		nWorldItemExtraValues[0] = nQuestItemID;
		nWorldItemExtraValues[1] = nRentPeriodHour;

		m_pStage->SpawnServerSideWorldItem(pPlayer, nSpawnItemID, pos.x, pos.y, pos.z, QUEST_DYNAMIC_WORLDITEM_LIFETIME, nWorldItemExtraValues);
	}
	else
	{
		MMatchObject* pPlayer = MMatchServer::GetInstance()->GetObject(uidKiller);
		if (!pPlayer) return;

		int nWorldItemExtraValues[WORLDITEM_EXTRAVALUE_NUM];
		for (int i = 0; i < WORLDITEM_EXTRAVALUE_NUM; i++)
			nWorldItemExtraValues[i] = -1;


		m_pStage->SpawnServerSideWorldItem(pPlayer, nWorldItemId, pos.x, pos.y, pos.z,
			QUEST_DYNAMIC_WORLDITEM_LIFETIME, nWorldItemExtraValues);
	}
}

void MMatchRuleQuestChallenge::OnLeaveBattle(MUID& uidChar)
{
	if(m_pPlayerMgr == NULL || m_pNpcMgr == NULL) return;
	m_pPlayerMgr->RemovePlayer(uidChar);

	vector<MUID> npcList = m_pNpcMgr->GetNPCByController(uidChar);

	for(vector<MUID>::iterator it = npcList.begin(); it != npcList.end(); ++it)
	{
		MUID uidNpc = *it;
		MUID uidController = m_pPlayerMgr->FindSuitableController();
		if (uidController.IsInvalid())
		{
			SetRoundState(MMATCH_ROUNDSTATE_EXIT);
			return;
		}
		m_pNpcMgr->SetController(uidNpc, uidController);

		MCommand* pNew = MMatchServer::GetInstance()->CreateCommand(MC_NEWQUEST_NEW_CONTROL, MUID(0,0));
		pNew->AddParameter(new MCmdParamUID(uidController));
		pNew->AddParameter(new MCmdParamUID(uidNpc));
		MMatchServer::GetInstance()->RouteToBattle(m_pStage->GetUID(), pNew);
	}
		MGetMatchServer()->ResponseCharacterItemList(uidChar);

}

void MMatchRuleQuestChallenge::RouteObtainZItem(MUID uidRef, unsigned long int nItemID)
{
	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_NEWQUEST_OBTAIN_ZITEM, MUID(0, 0));
	pCmd->AddParameter(new MCmdParamUID(uidRef));
	pCmd->AddParameter(new MCmdParamUInt(nItemID));
	MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);
}

void MMatchRuleQuestChallenge::OnObtainWorldItem(MMatchObject* pObj, int nItemID, int* pnExtraValues)
{
	if(!IsEnabledObject(pObj))
		return;

	int nQuestItemID = pnExtraValues[0];
	int nRentPeriodHour = pnExtraValues[1];

	if (nQuestItemID == -1) //eat it in this case.
		return;

	if (pObj->GetCharInfo()->m_ItemList.GetCount() >= MAX_ITEM_COUNT)
	{
		char sztext[48];
		sprintf_s(sztext, 48, "You have too many items to receive a reward");
		MGetMatchServer()->Announce(pObj, sztext);
		OnObtainWorldItemRandom(nItemID, pnExtraValues);
		//still need to return here otherwise we fall thru
		return;
	}

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if ( pItemDesc == NULL ) return;

	if(pItemDesc->m_nResSex.Ref() != -1)
	{
		if(pObj->GetCharInfo() && (int)pObj->GetCharInfo()->m_nSex != pItemDesc->m_nResSex.Ref())
		{
			OnObtainWorldItemRandom(nItemID, pnExtraValues);
			return;
		}
		return;
	}

	if (MMatchServer::GetInstance()->DistributeZItem(pObj->GetUID(), nQuestItemID, true, RENT_MINUTE_PERIOD_UNLIMITED, 1))
	{
		RouteObtainZItem(pObj->GetUID(), unsigned long int(nQuestItemID));
	}
}

bool MMatchRuleQuestChallenge::OnObtainWorldItemRandom(int nItemID, int* pnExtraValues)
{
	int nQuestItemID = pnExtraValues[0];
	int nRentPeriodHour = pnExtraValues[1];

	if (nQuestItemID == -1) //eat it in this case.
		return false;


	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if ( pItemDesc == NULL ) return false;

	std::vector<MMatchObject*> vObjects;

	for (MUIDRefCache::iterator i = m_pStage->GetObjBegin(); i != m_pStage->GetObjEnd(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (!IsEnabledObject(pObj)) continue;

		if (pObj->GetCharInfo()->m_ItemList.GetCount() >= MAX_ITEM_COUNT)
		{
			continue;
		}
		if (m_pPlayerMgr->IsPlayerInMap(pObj->GetUID()))
		{
			if(pItemDesc->m_nResSex.Ref() == -1 || pObj->GetCharInfo() && (int)pObj->GetCharInfo()->m_nSex == pItemDesc->m_nResSex.Ref())
				vObjects.push_back(pObj);
		}
	}

	if (vObjects.empty())
		return false;

	const unsigned long n = vObjects.size();

	if (n == 0)
		return false;


	int randomIndex = rand() % n;

	MMatchObject* pObj = NULL;
	if (randomIndex < vObjects.size())
		pObj = vObjects[randomIndex];

	if (!pObj)
		return false;

	if (MMatchServer::GetInstance()->DistributeZItem(pObj->GetUID(), nQuestItemID, true, RENT_MINUTE_PERIOD_UNLIMITED, 1))
	{
		RouteObtainZItem(pObj->GetUID(), unsigned long int(nQuestItemID));
	}
	return true;
}

//Log
void MMatchRuleQuestChallenge::PostCQGameLog()
{
	//m_dwCurrTime = MGetMatchServer()->GetTickTime() - 10000; //Corrects a 10 second error that occured
	//MUID uidChar;
	//char* szText;
	//unsigned int nObjCID = m_pStage->GetObj(uidChar)->GetCharInfo()->m_nCID;
	//sprintf(szText, "%d %d %d %d", nObjCID);

	////Make sure the time is less than good time
	//if ((m_dwCurrTime - GetStage()->GetStartTime()) / 1000 / 60 <
	//	(m_pScenario->GetGoodTime() / 60) && m_nCurrSector == m_pScenario->GetNumSector()) //Sector Num check
	//{
	//	//	Custom: Write information to the db on match ending
	//	MGetMatchServer()->GetDBMgr()->InsertCQTimeLog(pObj->GetCharInfo()->m_nCID, pObj->GetCharInfo()->m_nCID,
	//		pObj->GetCharInfo()->m_nCID, pObj->GetCharInfo()->m_nCID,
	//		((m_dwCurrTime - GetStage()->GetStartTime()) / 1000 / 60), ((m_dwCurrTime - GetStage()->GetStartTime()) / 1000 % 60),
	//		MGetStringResManager()->GetStringFromXml(m_pScenario->GetName()));
	//}

}

void MMatchRuleQuestChallenge::CollectEndQuestChallengeGameLogInfo()
{
	m_QuestChallengeGameLogInfoMgr.SetEndTime( timeGetTime() );
}

void MMatchRuleQuestChallenge::CollectStartingQuestChallengeGameLogInfo()
{
	// 수집하기전에 이전의 정보를 반드시 지워야 함.
	m_QuestChallengeGameLogInfoMgr.Clear();

	//if( QuestTestServer() ) 
	//{
	// Master CID
	MMatchObject* pMaster = MMatchServer::GetInstance()->GetObject( GetStage()->GetMasterUID() );
	if( IsEnabledObject(pMaster) )
		m_QuestChallengeGameLogInfoMgr.SetMasterCID( pMaster->GetCharInfo()->m_nCID );

	m_QuestChallengeGameLogInfoMgr.SetScenarioID(MGetStringResManager()->GetStringFromXml(m_pScenario->GetName()));

	// Stage name 저장.
//	m_QuestChallengeGameLogInfoMgr.SetStageName( GetStage()->GetName() );

	// 시작할때의 유저 정보를 저장함.
	for (MUIDRefCache::iterator itor = m_pStage->GetObjBegin(); itor != m_pStage->GetObjEnd(); ++itor)
	{
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		m_QuestChallengeGameLogInfoMgr.AddQuestPlayer( pObj->GetUID(),pObj);
	}

	m_QuestChallengeGameLogInfoMgr.SetStartTime( timeGetTime() );
	//}
}

void MMatchRuleQuestChallenge::PostInsertQuestChallengeGameLogAsyncJob()
{
	// Custom: More than 4 in quest.
	// Custom: Quest/Clan Server
	//if( MSM_TEST == MGetServerConfig()->GetServerMode() ) 
	//{
	CollectEndQuestChallengeGameLogInfo();
	m_QuestChallengeGameLogInfoMgr.PostInsertQuestChallengeGameLog();
	//}
}