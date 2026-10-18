#include "stdafx.h"
#include "ZActorAction.h"
#include "ZGame.h"
#include "ZRuleQuestChallenge.h"
ZActorAction::~ZActorAction()
{
	int numMelee = GetNumMeleeShot();
	for (int i = 0; i < numMelee; ++i)
		delete m_vecMeleeShot[i];

	int num = GetNumRangeShot();
	for (int i = 0; i < num; ++i)
		delete m_vecRangeShot[i];

	int num2 = GetNumGrenadeShot();
	for (int i = 0; i < num2; ++i)
		delete m_vecGrenadeShot[i];

	int num3 = GetNumEffect();
	for (int i = 0; i < num3; ++i)
		delete m_vecEffect[i];

	int num4 = GetNumSummon();
	for (int i = 0; i < num4; ++i)
		delete m_vecSummon[i];
}

void ZActorActionMeleeShot::ProcessShot(ZGame* pGame, const MUID& uidOwner, float fShotTime)const
{
	ZObjectManager* pObjectManager = ZGetObjectManager();
	if (!pObjectManager) return;

	ZObject* pAttacker = pObjectManager->GetObject(uidOwner);
	if (!pAttacker) return;

	if (m_fRange == 0) return;

	float fRadius = cosf(ToRadian(m_fAngle * 0.5f));

	rvector AttackerPos = pAttacker->GetPosition();
	rvector AttackerNorPos = AttackerPos;
	AttackerNorPos.z = 0;

	rvector AttackerDir = pAttacker->m_Direction;
	rvector AttackerNorDir = AttackerDir;
	AttackerNorDir.z = 0;
	Normalize(AttackerNorDir);

	rvector Pos = pAttacker->GetPosition();
	ZGetSoundEngine()->PlaySoundElseDefault("blade_swing", "blade_swing", rvector(Pos.x, Pos.y, Pos.z), false);

	for (ZObjectManager::iterator itor = pObjectManager->begin(); itor != pObjectManager->end(); ++itor)
	{
		ZObject* pVictim = (*itor).second;
		if (pVictim->IsNPC())
			continue;

		ZModule_HPAP* pModule = (ZModule_HPAP*)pVictim->GetModule(ZMID_HPAP);
		if (pVictim->IsDie())
			continue;

		if (pAttacker == pVictim)
			continue;

		if (pAttacker->GetTeamID() == pVictim->GetTeamID())
			continue;

		rvector VictimPos, VictimDir;
		if (!pVictim->GetHistory(&VictimPos, &VictimDir, fShotTime))
			continue;

		rvector VictimNorPos = VictimPos;
		VictimNorPos.z = 0;

		rvector VictimNorDir = VictimPos - (AttackerPos - (AttackerNorDir * 50.f));
		VictimNorDir.z = 0;
		Normalize(VictimNorDir);

		float fDist = Magnitude(AttackerNorPos - VictimNorPos);
		if (fDist > m_fRange)
			continue;

		float fDot = D3DXVec3Dot(&AttackerNorDir, &VictimNorDir);
		if (fDot < fRadius)
			continue;

		if (!pGame->InRanged(pAttacker, pVictim))
			continue;

		int nDebugRegister = 0;
		//change back later maybe?
		if (pGame->CheckWall(pAttacker, pVictim, nDebugRegister, true))
			continue;

		if (GetThrust())
		{
			bool bDamage = false;

			if (fDist < GetRange())
			{
				float add_time = 0.3f * (fDist / 600.f);
				float time = fShotTime + add_time;

				ZGetEffectManager()->AddSwordUppercutDamageEffect(VictimPos, pVictim->GetUID(), (DWORD)(add_time * 1000));

				if (pVictim == ZGetGame()->m_pMyCharacter)
				{
					ZGetGame()->m_pMyCharacter->ReserveDashAttacked(pAttacker->GetUID(), time, AttackerDir);
				}
				pVictim->OnBlastDagger(AttackerDir, AttackerPos);

				if (ZGetGame()->CanAttack(pAttacker, pVictim))
				{
					pVictim->OnDamagedSkill(pAttacker, AttackerPos, ZD_MELEE, MWT_DAGGER, m_fDamage, m_fPierce);
				}

				if (!m_strSound.empty())
					ZGetSoundEngine()->PlaySound((char*)m_strSound.c_str(), VictimPos);
			}
		}
		else if (GetUpperCut())
		{
			if (pVictim == ZGetGame()->m_pMyCharacter)
			{
				ZGetGame()->m_pMyCharacter->SetLastThrower(pAttacker->GetUID(), fShotTime + 1.0f);
				ZPostReaction(fShotTime, 2);
				ZGetGame()->m_pMyCharacter->AddVelocity(rvector(0, 0, 1700));
			}
			pVictim->OnBlast(AttackerDir);

			if (ZGetGame()->CanAttack(pAttacker, pVictim))
				pVictim->OnDamagedSkill(pAttacker, AttackerPos, ZD_MELEE, MWT_DAGGER, m_fDamage, m_fPierce);


			if (!m_strSound.empty())
				ZGetSoundEngine()->PlaySound((char*)m_strSound.c_str(), VictimPos);
		}
		else
		{
			if (pVictim->IsGuard() && (DotProduct(pVictim->m_Direction, AttackerNorDir) < 0))
			{
				rvector pos = pVictim->GetPosition();
				pos.z += 120.f;

				ZGetEffectManager()->AddSwordDefenceEffect(pos + (pVictim->m_Direction * 50.f), pVictim->m_Direction);
				pVictim->OnMeleeGuardSuccess();
				return;
			}

			rvector pos = pVictim->GetPosition();
			pos.z += 130.f;
			pos -= AttackerDir * 50.f;

			//ZGetEffectManager()->AddBloodEffect(pos, -VictimNorDir);
			ZGetEffectManager()->AddSlashEffect(pos, -VictimNorDir, 0);

			float fActualDamage = m_fDamage;
			float fPierceRatio = m_fPierce;
			pVictim->OnDamaged(pAttacker, pAttacker->GetPosition(), ZD_MELEE, MWT_DAGGER, fActualDamage, fPierceRatio, 0);

			if (!m_strSound.empty())
				ZGetSoundEngine()->PlaySound((char*)m_strSound.c_str(), pos);
		}
	}
	return;
}

void ZActorActionRangeShot::ProcessShot(const rvector& pos, const rvector& dir, ZObject* pOwner) const
{
	//rvector vMissilePos, vMissileDir;
	//GetMissilePosDir( vMissileDir, vMissilePos, targetPos);
	//ZGetGame()->m_WeaponManager.AddNewQuestProjectile(this, pos, dir, pOwner);

	ZGetGame()->m_WeaponManager.AddNewQuestProjectile(this, pos, dir, pOwner);
}

//cq grenadeshot velocity fix
void ZActorActionGrenadeShot::ProcessShot(const rvector& pos, const rvector& dir, ZObject* pOwner) const
{
	//this function needs rewriting, currently sucks.
	switch (m_nGrenadeType)
	{
	case 0:
	{
		ZGetGame()->m_WeaponManager.AddDynamite(static_cast<rvector>(pos), static_cast<rvector>(dir), pOwner);
	}
	break;
	case 3:
	{
		ZGetGame()->m_WeaponManager.AddTrap(static_cast<rvector>(pos), static_cast<rvector>(dir), m_nItemID, pOwner);
	}
	break;
	default:
		break;
	}

	if (!m_strSound.empty())
		ZGetSoundEngine()->PlaySound((char*)m_strSound.c_str(), pos);
}

void ZActorActionEffect::ProcessShot(const char* szEffectName, const rvector& Pos, const rvector& Dir, const MUID& uidOwner) const
{
	if (GetMeshName() == "")
	{
		return;
	}
	ZGetEffectManager()->AddWithScale(szEffectName, Pos, Dir, uidOwner, GetScale());
}

void ZActorActionSound::ProcessSound(string strSoundPath, const rvector& Pos) const
{
	if (strSoundPath.c_str() == "")
	{
#ifdef _BETABUILD
		mlog("[Error - ZActorActionSound::ProcessShot] - Error no sound\n");
#endif
		return;
	}
	ZGetSoundEngine()->PlaySound((char*)strSoundPath.c_str(), Pos);
}

void ZActorActionSummon::ProcessSummon(MUID& uidOwner, MUID& uidTarget, int nNum, string strNpcName, const rvector& pos, const rvector& dir, bool bAdjustPlayerNum) const
{
	if (strNpcName.c_str() == "")
	{
		mlog("Error in ZActorAction, no summoning npc name could be found.\n");
		return;
	}
	ZPostNewQuestRequestNpcSpawn(uidOwner, uidTarget, nNum, 0, strNpcName.c_str(), pos, dir);
}