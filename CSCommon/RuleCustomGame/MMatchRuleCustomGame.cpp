#include "stdafx.h"
#include "MMatchRuleCustomGame.h"
#include "MMatchRuleDeathMatch.h"
#include "MMatchTransDataType.h"
#include "MBlobArray.h"
#include "MMatchFormula.h"

//Custom: Rule Custom Game
///////////////////////////////////////////////////////////////////////////////////////////////////
// MMatchRuleVampire ////////////////////////////////////////////////////////////////////////
MMatchRuleVampire::MMatchRuleVampire(MMatchStage* pStage) : MMatchRule(pStage)
{

}

void MMatchRuleVampire::OnBegin()
{

}
void MMatchRuleVampire::OnEnd()
{
}

bool MMatchRuleVampire::RoundCount()
{
	if (++m_nRoundCount < 1) return true;
	return false;
}

bool MMatchRuleVampire::CheckKillCount(MMatchObject* pOutObject)
{
	MMatchStage* pStage = GetStage();
	for (MUIDRefCache::iterator i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (pObj->GetEnterBattle() == false) continue;

		if (pObj->GetKillCount() >= (unsigned int)pStage->GetStageSetting()->GetRoundMax())
		{
			pOutObject = pObj;
			return true;
		}
	}
	return false;
}

bool MMatchRuleVampire::OnCheckRoundFinish()
{
	MMatchObject* pObject = NULL;

	if (CheckKillCount(pObject))
	{
		return true;
	}
	return false;
}

void MMatchRuleVampire::OnRoundTimeOut()
{
	SetRoundArg(MMATCH_ROUNDRESULT_DRAW);
}

//////////////////////////////////////////////////////////////////////////////////
// MMatchRuleModeStaff ///////////////////////////////////////////////////////////
MMatchRuleModeStaff::MMatchRuleModeStaff(MMatchStage* pStage) : MMatchRule(pStage)
{

}

void MMatchRuleModeStaff::OnBegin()
{

}
void MMatchRuleModeStaff::OnEnd()
{
}

bool MMatchRuleModeStaff::RoundCount()
{
	if (++m_nRoundCount < 1) return true;
	return false;
}

bool MMatchRuleModeStaff::CheckKillCount(MMatchObject* pOutObject)
{
	MMatchStage* pStage = GetStage();
	for (MUIDRefCache::iterator i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (pObj->GetEnterBattle() == false) continue;

		if (pObj->GetKillCount() >= (unsigned int)pStage->GetStageSetting()->GetRoundMax())
		{
			pOutObject = pObj;
			return true;
		}
	}
	return false;
}

bool MMatchRuleModeStaff::OnCheckRoundFinish()
{
	MMatchObject* pObject = NULL;

	if (CheckKillCount(pObject))
	{
		return true;
	}
	return false;
}

void MMatchRuleModeStaff::OnRoundTimeOut()
{
	SetRoundArg(MMATCH_ROUNDRESULT_DRAW);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MMatchRuleSoloClassic ////////////////////////////////////////////////////////////////////////
MMatchRuleSoloClassic::MMatchRuleSoloClassic(MMatchStage* pStage) : MMatchRule(pStage)
{

}

void MMatchRuleSoloClassic::OnBegin()
{

}
void MMatchRuleSoloClassic::OnEnd()
{
}

bool MMatchRuleSoloClassic::RoundCount()
{
	if (++m_nRoundCount < 1) return true;
	return false;
}

bool MMatchRuleSoloClassic::CheckKillCount(MMatchObject* pOutObject)
{
	MMatchStage* pStage = GetStage();
	for (MUIDRefCache::iterator i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (pObj->GetEnterBattle() == false) continue;

		if (pObj->GetKillCount() >= (unsigned int)pStage->GetStageSetting()->GetRoundMax())
		{
			pOutObject = pObj;
			return true;
		}
	}
	return false;
}

bool MMatchRuleSoloClassic::OnCheckRoundFinish()
{
	MMatchObject* pObject = NULL;

	if (CheckKillCount(pObject))
	{
		return true;
	}
	return false;
}

void MMatchRuleSoloClassic::OnRoundTimeOut()
{
	SetRoundArg(MMATCH_ROUNDRESULT_DRAW);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MMatchRuleTeamClassic ////////////////////////////////////////////////////////////////////////
MMatchRuleTeamClassic::MMatchRuleTeamClassic(MMatchStage* pStage) : MMatchRule(pStage)
{
}

void MMatchRuleTeamClassic::OnBegin()
{
	m_pStage->InitTeamKills();
}

void MMatchRuleTeamClassic::OnEnd()
{
}

bool MMatchRuleTeamClassic::OnRun()
{
	bool ret = MMatchRule::OnRun();


	return ret;
}

void MMatchRuleTeamClassic::OnRoundBegin()
{
	MMatchRule::OnRoundBegin();
}

void MMatchRuleTeamClassic::OnRoundEnd()
{
	if (m_pStage != NULL)
	{
		if (m_nRoundArg == MMATCH_ROUNDRESULT_REDWON)
		{
			m_pStage->OnRoundEnd_FromTeamGame(MMT_RED);
		}
		else if (m_nRoundArg == MMATCH_ROUNDRESULT_BLUEWON)
		{
			m_pStage->OnRoundEnd_FromTeamGame(MMT_BLUE);
		}
		else if (m_nRoundArg == MMATCH_ROUNDRESULT_DRAW){
		}
	}

	MMatchRule::OnRoundEnd();
}

void MMatchRuleTeamClassic::GetTeamScore(int* pRedTeamScore, int* pBlueTeamScore)
{
	(*pRedTeamScore) = 0;
	(*pBlueTeamScore) = 0;

	MMatchStage* pStage = GetStage();
	if (pStage == NULL) return;

	(*pRedTeamScore) = pStage->GetTeamKills(MMT_RED);
	(*pBlueTeamScore) = pStage->GetTeamKills(MMT_BLUE);

	return;
}

bool MMatchRuleTeamClassic::OnCheckRoundFinish()
{
	int nRedScore, nBlueScore;
	GetTeamScore(&nRedScore, &nBlueScore);

	MMatchStage* pStage = GetStage();

	if (nRedScore >= pStage->GetStageSetting()->GetRoundMax())
	{
		SetRoundArg(MMATCH_ROUNDRESULT_REDWON);
		return true;
	}
	else if (nBlueScore >= pStage->GetStageSetting()->GetRoundMax())
	{
		SetRoundArg(MMATCH_ROUNDRESULT_BLUEWON);
		return true;
	}

	return false;
}

void MMatchRuleTeamClassic::OnRoundTimeOut()
{
	if (!OnCheckRoundFinish())
		SetRoundArg(MMATCH_ROUNDRESULT_DRAW);
}

bool MMatchRuleTeamClassic::RoundCount()
{
	if (++m_nRoundCount < 1) return true;
	return false;
}

void MMatchRuleTeamClassic::CalcTeamBonus(MMatchObject* pAttacker, MMatchObject* pVictim,
	int nSrcExp, int* poutAttackerExp, int* poutTeamExp)
{
	if (m_pStage == NULL)
	{
		*poutAttackerExp = nSrcExp;
		*poutTeamExp = 0;
		return;
	}

	*poutTeamExp = (int)(nSrcExp * m_pStage->GetStageSetting()->GetCurrGameTypeInfo()->fTeamBonusExpRatio);
	*poutAttackerExp = (int)(nSrcExp * m_pStage->GetStageSetting()->GetCurrGameTypeInfo()->fTeamMyExpRatio);
}




void MMatchRuleTeamClassic::OnGameKill(const MUID& uidAttacker, const MUID& uidVictim)
{
	MMatchObject* pAttacker = MMatchServer::GetInstance()->GetObject(uidAttacker);
	MMatchObject* pVictim = MMatchServer::GetInstance()->GetObject(uidVictim);

	if (m_pStage != NULL)
	{
		m_pStage->AddTeamKills(pVictim->GetTeam() == MMT_BLUE ? MMT_RED : MMT_BLUE);		
	}
}