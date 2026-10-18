#ifndef _MMATCHRULEQUESTCHALLENGE_H
#define _MMATCHRULEQUESTCHALLENGE_H

#include "MMatchRule.h"
#include "MActorDef.h"
#include "MNewQuestScenario.h"
#include "MMatchQuestGameLog.h"


class MNewQuestPlayerManager;
class MNewQuestNpcManager;

class MMatchRuleQuestChallenge : public MMatchRule
{
	static MActorDefManager ms_actorDefMgr;
	static MNewQuestScenarioManager ms_scenarioMgr;
public:
	static void InitActorDefMgr();
	static void InitScenarioMgr();
	static MNewQuestScenarioManager* GetScenarioMgr();

private:

	MNewQuestPlayerManager* m_pPlayerMgr;
	MNewQuestNpcManager* m_pNpcMgr;
	std::map<int, string> m_mapNPCBitFlags;

	MNewQuestScenario* m_pScenario;
	MMatchQuestChallengeGameLogInfoManager	m_QuestChallengeGameLogInfoMgr;
	int m_nCurrSector;		// 현재 진행중인 섹터

protected:
	virtual bool RoundCount();										///< 라운드 카운트. 모든 라운드가 끝나면 false를 반환한다.
	virtual void OnBegin();											///< 전체 게임 시작시 호출
	virtual void OnEnd();											///< 전체 게임 종료시 호출
	virtual void OnRoundBegin();									///< 라운드 시작할 때 호출
	virtual void OnRoundEnd();										///< 라운드 끝날 때 호출
	virtual bool OnRun();											///< 게임틱시 호출
	virtual bool CheckPlayersAlive();
	virtual void OnLeaveBattle(MUID& uidChar);
	virtual bool OnCheckEnableBattleCondition(); //Custom : Emulating maiets roundstates for CQ
	DWORD m_dwCurrTime; //Custom: Log the game time
	//virtual void ProcessNPCSpawn() {}						///< NPC 스폰작업
	//virtual bool CheckNPCSpawnEnable() { return true; }					///< NPC가 스폰 가능한지 여부
	//virtual void RouteGameInfo() {}							///< 클라이언트에 게임 정보 보내준다.
	//virtual void RouteStageGameInfo() {}					///< 대기중 스테이지에서 바뀐 게임 정보를 보내준다.

	////virtual void OnCompleted();					///< 퀘스트 성공시 호출된다.
	////virtual void OnFailed();					///< 퀘스트 실패시 호출된다.
	//virtual void RouteCompleted() {}			///< 퀘스트 성공 메시지를 보낸다. - 리워드까지 함께 보낸다.
	//virtual void RouteFailed() {}				///< 퀘스트 실패 메시지 보낸다.
	//virtual void DistributeReward() {}			///< 퀘스트 성공시 리워드 배분

	//virtual void OnRequestMovetoPortal(const MUID& uidPlayer) {}	/// 플레이어가 포탈로 이동했을 경우 호출된다.
	//virtual void OnReadyToNewSector(const MUID& uidPlayer) {}		/// 포탈로 이동하고 나서 이동이 완료되었을 경우 호출된다.

	//virtual void OnRequestTestSectorClear() {}
	//virtual void OnRequestTestFinish() {}

protected:
	virtual bool OnCheckRoundFinish();							///< 라운드가 끝났는지 체크

public:
	virtual void OnQuestEnterBattle(MUID& uidChar);
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_QUEST_CHALLENGE; }
	virtual void OnObtainWorldItem(MMatchObject* pObj, int nItemID, int* pnExtraValues);
	bool OnObtainWorldItemRandom(int nItemID, int* pnExtraValues);
	void RouteObtainZItem(MUID uidRef, unsigned long int nItemID);
	DWORD GetScenarioTime() { return m_dwCurrTime; };
	void PostCQGameLog();
	void PostInsertQuestChallengeGameLogAsyncJob();
	void CollectEndQuestChallengeGameLogInfo();
	void CollectStartingQuestChallengeGameLogInfo();
public:
	MMatchRuleQuestChallenge(MMatchStage* pStage);
	virtual ~MMatchRuleQuestChallenge();

	virtual void OnCommand(MCommand* pCommand);

private:
	void RouteSpawnNpc(MUID uidNPC, MUID uidController, const char* szNpcDefName, int nCustomSpawnTypeIndex, int nSpawnIndex, bool bIsBoss);
	void RouteSpawnNpcSummon(MUID uidNPC, MUID uidController, int num, const char* szNpcDefName, MShortVector sVec, MShortVector dir);
	void RouteSpawnNpcSingle(MUID listener, MUID uidNPC, MUID uidController, const char* szNpcDefName, int nCustomSpawnTypeIndex, int nSpawnIndex, bool bIsBoss);
	void RouteNpcDead(MUID uidNPC, MUID uidKiller);
	void RouteMoveToNextSector(int nSectorID);
	void RouteMoveToNextSectorSingle(MUID listener, int nSectorID);
	void RouteExpToPlayers();

	void ProcessNpcSpawning();
	void SpawnNpcsForSingle(MUID playerUid);
	void SpawnNpc(const char* szActorDef, int nCustomSpawnTypeIndex, int nSpawnIndex, int nDropItemId, bool bIsBoss);
	void SpawnNpcSummon(const char* szActorDef, int num, MShortVector nSpawnPos, MShortVector nSpawnDir);
	void DropItemByNpcDead(const MUID& uidKiller, int nWorldItemId, const MVector& pos);
};

#endif