#ifndef MMATCHRULECUSTOMGAME_H
#define MMATCHRULECUSTOMGAME_H

#include "MMatchRule.h"

//Custom: Rule Custom Game
///////////////////////////////////////////////////////////////////////////////////////////////
class MMatchRuleVampire : public MMatchRule {
protected:
	bool CheckKillCount(MMatchObject* pOutObject);
	virtual void OnBegin();
	virtual void OnEnd();
	virtual void OnRoundTimeOut();
	virtual bool OnCheckRoundFinish();
	virtual bool RoundCount();
public:
	MMatchRuleVampire(MMatchStage* pStage);
	virtual ~MMatchRuleVampire() { }
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_VAMPIRE; }
};

class MMatchRuleModeStaff : public MMatchRule {
protected:
	bool CheckKillCount(MMatchObject* pOutObject);
	virtual void OnBegin();
	virtual void OnEnd();
	virtual void OnRoundTimeOut();
	virtual bool OnCheckRoundFinish();
	virtual bool RoundCount();
public:
	MMatchRuleModeStaff(MMatchStage* pStage);
	virtual ~MMatchRuleModeStaff() { }
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_MODE_STAFF; }
};

//////MMatchRuleSoloClassic////////////////////////////////////////////////////////////////////
class MMatchRuleSoloClassic : public MMatchRule {
protected:
	bool CheckKillCount(MMatchObject* pOutObject);
	virtual void OnBegin();
	virtual void OnEnd();
	virtual void OnRoundTimeOut();
	virtual bool OnCheckRoundFinish();
	virtual bool RoundCount();
public:
	MMatchRuleSoloClassic(MMatchStage* pStage);
	virtual ~MMatchRuleSoloClassic() { }
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_CLASSIC_SOLO; }
};

/////MMatchRuleTeamClassic///////////////////////////////////////////////////////////////////////
class MMatchRuleTeamClassic : public MMatchRule {
protected:
	void GetTeamScore(int* pRedTeamScore, int* pBLueTeamScore);
	virtual void OnBegin();
	virtual void OnEnd();
	virtual bool OnRun();
	virtual void OnRoundBegin();
	virtual void OnRoundEnd();
	virtual bool OnCheckRoundFinish();
	virtual void OnRoundTimeOut();
	virtual bool RoundCount();
	virtual void OnGameKill(const MUID& uidAttacker, const MUID& uidVictim);

public:
	MMatchRuleTeamClassic(MMatchStage* pStage);
	virtual ~MMatchRuleTeamClassic() {}
	virtual void CalcTeamBonus(MMatchObject* pAttacker, MMatchObject* pVictim,
		int nSrcExp, int* poutAttackerExp, int* poutTeamExp);
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_CLASSIC_TEAM; }
};


#endif