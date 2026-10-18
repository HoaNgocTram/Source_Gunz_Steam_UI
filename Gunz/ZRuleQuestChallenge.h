#ifndef _ZRULE_QUESTCHALLENGE_H
#define _ZRULE_QUESTCHALLENGE_H

#include "ZRule.h"
#include "MNewQuestScenario.h"
#include "MMatchUtil.h"
class ZFSMManager;
class MActorDefManager;
class ZActorActionManager;
class MNewQuestScenario;


class ZRuleQuestChallenge : public ZRule
{
private:
	ZFSMManager* m_pFsmManager;
	MActorDefManager* m_pActorDefMgr;
	ZActorActionManager* m_pActorActionMgr;


	MNewQuestScenario* m_pScenario;
	float m_fElapsedTime;
	int m_nCurrSector;
	int m_nNPC;
	int m_nRewardXP;
	int m_nRewardBP;
	int m_nItemID;
	bool m_bLateJoin;
	MUID m_bossUid;
	std::string m_szElapsedTime;
	void SetBoss(MUID x) { m_bossUid = x; }
public:
	ZRuleQuestChallenge(ZMatch* pMatch);
	virtual ~ZRuleQuestChallenge();

	ZActorActionManager* GetActorActionMgr() { return m_pActorActionMgr; }

	std::vector<int> m_OurCQRewards;

	static bool LoadScenarioMap(const char* szScenarioName);

	virtual bool Init();
	virtual void OnUpdate(float fDelta);

	virtual float FilterDelayedCommand(MCommand* pCommand); 
	bool OnSectorBonus(MCommand* pCommand);
	virtual bool OnCommand(MCommand* pCommand);

	virtual int GetRoundMax();
	virtual int	GetCurrSector();
	void SpawnActor(const char* szActorDefName, MUID& uid, int nCustomSpawnTypeIndex, int nSpawnIndex, bool bMyControl, bool bIsBoss);
	void SpawnActor(const char* szActorDefName, MUID& uid, int nCustomSpawnTypeIndex, int nSpawnIndex, bool bMyControl, bool bIsBoss, MShortVector vec);
	void SpawnActorSummon(const char* szActorName, MUID& uid, int nNum, int nIndex, bool bMyControl,MShortVector vec, MShortVector vec2);
	void MoveToNextSector(int nSectorIndex, bool bLateJoin);
	virtual int GetNPC();
	// Custom CQ Func's
    int GetCQLevel() { return  m_pScenario->GetLevelLimit(); };
	int GetCQGoodTime() { return m_pScenario->GetGoodTime(); }
	int GetRewardXP() { return m_nRewardXP; }
	int GetRewardBP() { return m_nRewardBP; }
	int GetItemID() { return m_nItemID; }
	DWORD GetCurrTime();
	const char* GetCurrTimeString() { return m_szElapsedTime.c_str(); }
	MUID GetBoss() { return m_bossUid; }
	bool OnObtainZItem(MCommand* pCommand);
	bool GetLateJoin() { return m_bLateJoin; }
};



#endif