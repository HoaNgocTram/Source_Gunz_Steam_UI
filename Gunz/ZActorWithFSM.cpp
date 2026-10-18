#include "stdafx.h"
#include "ZActorWithFSM.h"
#include "MActorDef.h"
#include "ZFSMManager.h"
#include "ZFSM.h"
#include "ZFSMState.h"
#include "ZActorActionManager.h"
#include "ZActorAction.h"
//#include "ZNavigationMesh.h"
#include "ZScreenEffectManager.h"
#include "ZModule_HPAP.h"
#include "ZModule_Resistance.h"
#include "ZModule_FireDamage.h"
#include "ZModule_ColdDamage.h"
#include "ZModule_LightningDamage.h"
#include "ZModule_PoisonDamage.h"
#include "ZActorMesh.h"
#include "ZRuleQuestChallenge.h"

MImplementRTTI(ZActorWithFSM, ZActorBase);


ZActorWithFSM::ZActorWithFSM(ZGame* pGame, ZActorActionManager* pActionMgr)
: ZActorBase(pGame)
, m_pActionMgr(pActionMgr)
, m_pActorDef(NULL)
, m_pFsm(NULL)
, m_pCurrState(NULL)
, m_pCurrAction(NULL)
, m_pCurrActionStartTime(0)
, m_uidTarget(0,0)
, m_fGroggy(0)
, m_bPostedActionExecute(false)
, m_bPostedRangeShotInAction(false)
, m_bMoving(false)
, m_bSuffering(false)
, m_RealPositionBefore(0,0,0)
, m_bMyControl(false)
, m_nLastTimePostBasicInfo(0)
, m_nSpeedAccel(0.f)
, m_nTeamID(MMT_ALL)
{
	m_bIsNPC = true;
	SetVisible(true);
	m_bInitialized = true;
	m_nLastUpdate = ZTimeGetTime();

	m_pModule_HPAP				= new ZModule_HPAP;
	m_pModule_Resistance		= new ZModule_Resistance;
	m_pModule_FireDamage		= new ZModule_FireDamage;
	m_pModule_ColdDamage		= new ZModule_ColdDamage;
	m_pModule_PoisonDamage		= new ZModule_PoisonDamage;
	m_pModule_LightningDamage	= new ZModule_LightningDamage;

	AddModule(m_pModule_HPAP);
	AddModule(m_pModule_Resistance);
	AddModule(m_pModule_FireDamage);
	AddModule(m_pModule_ColdDamage);
	AddModule(m_pModule_PoisonDamage);
	AddModule(m_pModule_LightningDamage);

	// 이동 모듈은 직접 업데이트 할 것이므로 비활성 상태로 만들어야 ZModuleContainer::UpdateModules()에서 중복 update되지 않는다
	m_pModule_Movable->Active(false);

}

ZActorWithFSM::~ZActorWithFSM()
{
	EmptyHistory();
	DestroyShadow();

	RemoveModule(m_pModule_HPAP);
	RemoveModule(m_pModule_Resistance);
	RemoveModule(m_pModule_FireDamage);
	RemoveModule(m_pModule_ColdDamage);
	RemoveModule(m_pModule_PoisonDamage);
	RemoveModule(m_pModule_LightningDamage);

	delete m_pModule_HPAP;
	delete m_pModule_Resistance;
	delete m_pModule_FireDamage;
	delete m_pModule_ColdDamage;
	delete m_pModule_PoisonDamage;
	delete m_pModule_LightningDamage;
}

void ZActorWithFSM::InitMesh(char* szMeshName)
{
	_ASSERT(m_pVMesh == NULL);

//	AllocObjectVMesh();
//	ZActorMesh::InitNpcMesh(this, szMeshName);
	//m_pVMesh->InitNpcMesh(this, szMeshName);
}

void ZActorWithFSM::OnDraw()
{
	if (m_pVMesh == NULL) return;

	Draw_SetLight(GetPosition() );

	//todok 죽었을 때 사라지는 표현 필요
//	if( IsDieAnimationDone() )
//	{
//#define TRAN_AFTER		0.5f	// 이 시간 이후부터
//#define VANISH_TIME		1.f		// 이 시간동안 투명해진다
//
//		if(m_timeDieAnimationDone==-1) m_timeDieAnimationDone = m_pGame->GetTime();
//
//		//float fOpacity = max(0.f,min(1.0f,(	VANISH_TIME-(m_pGame->GetTime()-GetDeadTime() - TRAN_AFTER))/VANISH_TIME));
//		float fOpacity = max(0.f,min(1.0f,(	VANISH_TIME-(m_pGame->GetTime()-m_timeDieAnimationDone - TRAN_AFTER))/VANISH_TIME));
//
//		m_pVMesh->SetVisibility(fOpacity);
//	}
//	else {
//		if(!m_bHero) m_pVMesh->SetVisibility(1.f);
//		m_timeDieAnimationDone = -1;
//	}

	m_pVMesh->Render();

	//Cond_Pick() 테스트용 : 해당 방향으로 라인을 그린다
	/*D3DXMATRIX world;
	MakeWorldMatrix(&world, GetPosition(), GetDirection(), rvector(0,0,1));
	RGetDevice()->SetTransform(D3DTS_WORLD, &world);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	if (!Cond_Pick(90, 200))
		RDrawLine(rvector(-200, 5, 0), rvector(0, 5, 0), 0xffffff);
	if (!Cond_Pick(-90, 200))
		RDrawLine(rvector(200, 5, 0), rvector(0, 5, 0), 0xffffff);
	if (!Cond_Pick(0, 200))
		RDrawLine(rvector(0, 5, -200), rvector(0, 5, 0), 0xffffff);
	if (!Cond_Pick(180, 200))
		RDrawLine(rvector(0, 5, 200), rvector(0, 5, 0), 0xffffff);
		*/
}

void ZActorWithFSM::OnUpdate( float fDelta )
{
	//todok 죽으면 실제로 제거해야한다. 지금 죽어서 안보이는 놈도 업데이트가 계속 돌아가고 있음

	if(m_pVMesh && !IsDie()) 
	{
		m_pVMesh->SetVisibility(1.f);
	}

	if (IsMyControl())
	{

		if (ZTimeGetTime() - m_nLastUpdate >= 10)
		{
			m_nLastUpdate = ZTimeGetTime();
			UpdateFsm(fDelta);
		}
		PostBasicInfo();

		UpdateVelocity(fDelta);
		UpdatePosition(fDelta);
		UpdateHeight(fDelta);
		UpdateGroggy(fDelta);
	}

	ProcessMotion(fDelta);

	ProcessShotInAction(fDelta);
}

void ZActorWithFSM::SetMyControl(bool bMyControl)
{
	m_bMyControl = bMyControl;
}

bool ZActorWithFSM::IsMyControl()
{
	return m_bMyControl;
}

bool ZActorWithFSM::IsMovingAnimation()
{
	/*if (!m_pCurrState) return false;
	IActorAction* pAction = m_pCurrState->GetAction();
	if (!pAction) return false;
	return pAction->IsMovingAnimation();*/

	if (!m_pCurrAction) return false;
	return m_pCurrAction->IsMovingAnimation();
}

bool ZActorWithFSM::IsCollideable()
{
	if (!m_Collision.IsCollideable())
		return false;

	return !IsDie();
}

bool ZActorWithFSM::IsDie() 
{ 
	if (!m_pFsm) { _ASSERT(0); return true; }

	if (m_pFsm->GetState(FSM_BISTATE_DIE) == m_pCurrState || m_pFsm->GetState(FSM_BISTATE_WAIT_DELETE) == m_pCurrState)
	{
		m_Collision.SetCollideable(false);
		return true;
	}

	return false;
}

bool ZActorWithFSM::InitWithActorDef( MActorDef* pActorDef, ZFSMManager* pFsmMgr )
{
	m_pActorDef = pActorDef;

	// init mesh
	_ASSERT(m_pVMesh == NULL);
	//AllocObjectVMesh();
	ZActorMesh::InitNpcMesh(this, m_pActorDef->GetModelName());
//	m_pVMesh->InitNpcMesh(this, m_pActorDef->GetModelName());
	if (!pActorDef->GetNoShadow())
		CreateShadow();

	m_Collision.SetHeight(pActorDef->GetCollup120() ? m_pActorDef->GetCollisionHeight() + 120 : m_pActorDef->GetCollisionHeight());
	m_Collision.SetRadius(m_pActorDef->GetCollisionRadius());
	// init hp ap
	m_pModule_HPAP->SetMaxHP(pActorDef->GetMaxHp());
	m_pModule_HPAP->SetHP(pActorDef->GetMaxHp());

	m_pModule_HPAP->SetMaxAP(pActorDef->GetMaxAp());
	m_pModule_HPAP->SetAP(pActorDef->GetMaxAp());
	
	m_pModule_HPAP->SetRealDamage(true);

	// init fsm
	m_pFsm = pFsmMgr->GetFsm( pActorDef->GetFsmName());
	if (!m_pFsm)
	{
		_ASSERT(0);
		return false;
	}

	m_pCurrState = m_pFsm->GetState( m_pFsm->GetEntryStateName());
	if (!m_pCurrState)
	{
		_ASSERT(0);
		return false;
	}

	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
		m_nTeamID = MMT_BLUE;

	OnEnterState();

	return true;
}

bool ZActorWithFSM::ProcessMotion(float fDelta)
{
	// 메쉬 애니메이션 업데이트를 담당하는 함수!

	if (!m_pVMesh) return false;
	m_pVMesh->Frame();

	rvector vProxyPosition = GetPosition();
	rvector vProxyDirection = GetDirection();

	D3DXMATRIX world;

	rvector MeshPosition;	// 이동량만큼을 뺀 메쉬 렌더링 위치
	rvector vAnimationPositionDiff(0,0,0);

	// 움직임이 있는 애니메이션은 위치조절을 해줘야 한다.
	// 이전 프레임과 현재 프레임에서의 애니메이션 발 위치 변화량을 구해서 실제 액터의 위치를 추가 이동시켜주고
	// 메쉬는 그만큼 오프셋을 반대로 걸어서 렌더링해야 한다.
	if(IsMovingAnimation())
	{
		MakeWorldMatrix(&world,rvector(0,0,0),vProxyDirection,rvector(0,0,1));

		// 발의 위치를 로컬 좌표계로 얻어낸다
		rvector footPosition = m_pVMesh->GetFootPosition();
		rvector realPosition = footPosition * world;

		MeshPosition = vProxyPosition - realPosition ;

		vAnimationPositionDiff = footPosition - m_RealPositionBefore;	// 이전 프레임과 현재의 발 위치 변화량

		// 애니메이션의 움직임을 월드의 움직임으로 변환한다
		vAnimationPositionDiff = vAnimationPositionDiff * world;

		m_RealPositionBefore = footPosition;
	}
	else
		MeshPosition = vProxyPosition;

	MakeWorldMatrix(&world, MeshPosition, vProxyDirection, rvector(0,0,1));
	m_pVMesh->SetWorldMatrix(world);

	if(IsMyControl() && IsMovingAnimation())
	{
		// 애니메이션에 포함된 이동량만큼 액터 위치를 이동
		rvector origdiff=fDelta*GetVelocity();

		rvector diff = vAnimationPositionDiff;
		diff.z+=origdiff.z;

		if(GetDistToFloor()<0 && diff.z<0) diff.z=-GetDistToFloor();

		Move(diff);
	}

/*	//todok 라이트닝 피격처리
	if(IsActiveModule(ZMID_LIGHTNINGDAMAGE)==false) {// 라이트닝 에니메이션 루프..

		if (m_pVMesh->isOncePlayDone())
		{
			m_Animation.Input(ZA_ANIM_DONE);
		}
	}
*/
	return true;
}

void ZActorWithFSM::UpdateVelocity(float fDelta)
{
	if (IsAir() /*&& !IsMovingAnimation()*/)
		m_pModule_Movable->UpdateGravity(fDelta);

	if (!IsSuffering() && !IsAir())
	{
		if (IsMoving())
		{
			const float fAccel = 10000.f;
			AddVelocity(m_Direction * (fAccel * fDelta));

			float fSpeed = GetSpeedAccel() > 1.f ? GetSpeedAccel() : m_pActorDef->GetSpeed();

			rvector vel = GetVelocity();
			if (Magnitude(vel) > fSpeed)
			{
				Normalize(vel);
				vel *= fSpeed;
				SetVelocity(vel);
			}
		}
		else
		{
			SetVelocity(rvector(0,0,GetVelocity().z));
		}
	}
}

void ZActorWithFSM::UpdatePosition(float fDelta)
{
	m_pModule_Movable->Update(fDelta);


	// 착지 순간의 처리
	if(m_pModule_Movable->isLanding())
	{
		//SetFlag(AF_LAND, true);
		//m_Animation.Input(ZA_EVENT_REACH_GROUND);
		SetVelocity(0,0,0);

		if(GetPosition().z + 100.f < m_pModule_Movable->GetFallHeight())
		{
			float fSpeed=fabs(GetVelocity().z);

			// 점프 착지시 먼지..
			rvector vPos = GetPosition();
			rvector vDir = rvector(0.f,0.f,-1.f);
			vPos.z += 50.f;

			RBSPPICKINFO pInfo;
			if (m_pGame->PickWorld(vPos, vDir, &pInfo))
			{
				vPos = pInfo.PickPos;

				vDir.x = pInfo.pInfo->plane.a;
				vDir.y = pInfo.pInfo->plane.b;
				vDir.z = pInfo.pInfo->plane.c;

				AddLandingEffect(vPos, vDir);
			}
		}
	}
}

void ZActorWithFSM::UpdateHeight(float fDelta)
{
	// 발이 뭍혔거나 내려가야 하는데 못간경우
	rvector diff=fDelta*GetVelocity();

	bool bUp = (diff.z>0.01f);
	bool bDownward= (diff.z<0.01f);

	if(GetDistToFloor()<0 || (bDownward && m_pModule_Movable->GetLastMove().z>=0))
	{
		if(GetVelocity().z<1.f && GetDistToFloor()<1.f)
		{
			if(GetVelocity().z<0)
				SetVelocity(GetVelocity().x,GetVelocity().y,0);
		}
	}

	// 계단 같은곳에서 스르륵 올라오도록..
	if(GetDistToFloor()<0 && !IsDie())
	{
		float fAdjust=400.f*fDelta;	// 초당 이만큼
		rvector diff=rvector(0,0,min(-GetDistToFloor(),fAdjust));
		Move(diff);
	}
}

void ZActorWithFSM::UpdateGroggy(float fDelta)
{
	// 그로기의 자연 회복을 적용
	if (!m_pActorDef) { _ASSERT(0); return; }
	float fRecoverSpeed = m_pActorDef->GetGroggyRecoverySpeed();
	m_fGroggy -= fRecoverSpeed * fDelta;
	if (m_fGroggy < 0)
		m_fGroggy = 0;

	//todok del
	//mlog("ㅂACTOR_GROGGY : %f\n", m_fGroggy);
}

ZFSMState* ZActorWithFSM::CheckGlobalTransition()
{
	/* todok 죽음 전이는 fsm 정의 xml에서 수동으로 하도록 한다.
	// todok 따라서 서버에 죽음 통보하는 것은 func으로 구현해야 한다.
	// 우선 죽음 검사부터.. todok 여기에 두지 말고 함수로 빼서 옮기자
	ZFSMState* pDieState = m_pFsm->GetState(FSM_BISTATE_DIE);
	if (m_pCurrState != pDieState)
	{
		if (Cond_GetHp() <= 0)
		{
			//todok 서버에 보내야 한다. 죽음 상태 진입시 바로 보내자 근데 이걸 enter function으로 구현해야 하나?
			//ZPostQuestRequestNPCDead(uidAttacker, GetUID(), GetPosition());
			return pDieState;
		}
	}*/
	return NULL;
}

void ZActorWithFSM::SetAnimationInAction(IActorAction* pAction)
{
	if (!pAction) return;

	const char* szAnimName = pAction->GetAnimationName();
	if (!m_pVMesh->SetAnimation(ani_mode_lower, (char*)szAnimName, false))
		_ASSERT(0);
}

void ZActorWithFSM::UpdateFsm(float fDelta)
{
	if (!IsMyControl()) return;
	if (!m_pFsm) { _ASSERT(0); return; }
	if (!m_pCurrState) { _ASSERT(0); return; }

	ValidateTarget();

	// 전이 검사
	ZFSMState* pNextState = NULL;

	pNextState = CheckGlobalTransition();
	if (!pNextState)
		pNextState = m_pCurrState->CheckTransition(this);

	if (pNextState)
	{
		OnExitState();
		m_pCurrState = pNextState;
		OnEnterState();
	}

	// 액션 실행
	IActorAction* pAction = m_pCurrState->GetAction();
	if (pAction)
	{
		SetAnimationInAction(pAction);

		//todok del
/*		const ZActorActionMeleeShot* pMeleeShot = pAction->GetMeleeShot();
		const ZActorActionRangeShot* pRangeShot = pAction->GetRangeShot();

		// 공격 커맨드는 state내에서 1번만 보낸다
		if (pMeleeShot && !m_bPostedActionExecute)
		{
			ZPostNewQuestPeerNpcAttackMelee(m_UID, pAction->GetName());
			m_bPostedActionExecute = true;
		}
		if (pRangeShot && !m_bPostedRangeShotInAction)
		{
			ZPostNewQuestPeerNpcAttackRange(m_UID, pAction->GetName());
			m_bPostedRangeShotInAction = true;
		}*/
	}

	// function 실행
	int numFunc = m_pCurrState->GetFunctionCallCount();
	for (int i=0; i<numFunc; ++i)
	{
		const ZFSMFunctionCall* pFunc = m_pCurrState->GetFunctionCall(i);
		ExecuteFunction(fDelta, pFunc->id, pFunc->nArg[0], pFunc->nArg[1], pFunc->nArg[2], pFunc->nArg[3]);
	}
}

void ZActorWithFSM::ProcessShotInAction(float fDelta)
{
	if (!m_pCurrAction) return;

	float fCurrTime = ZGetGame()->GetTime();

	int nNumMeleeShot = m_pCurrAction->GetNumMeleeShot();
	for (int i = 0; i < nNumMeleeShot; ++i)
	{
		const ZActorActionMeleeShot* pMeleeShot = m_pCurrAction->GetMeleeShot(i);
		if (fCurrTime >= m_pCurrActionStartTime + pMeleeShot->GetDelay() && !IsProcessedShot(pMeleeShot))
		{
			pMeleeShot->ProcessShot(m_pGame, m_UID, fCurrTime);
			SetProcessedShot(pMeleeShot);
		}
	}

	int numRangeShot = m_pCurrAction->GetNumRangeShot();
	for (int i = 0; i<numRangeShot; ++i)
	{
		const ZActorActionRangeShot* pRangeShot = m_pCurrAction->GetRangeShot(i);
		if (pRangeShot)
		{
			if (fCurrTime >= m_pCurrActionStartTime + pRangeShot->GetDelay() &&
				!IsProcessedShot(pRangeShot))
			{
				rvector vDir;
				rmatrix mat;
				ZObject* pChar = m_pGame->GetCharacterMgr()->Find(m_uidTarget);
				if (pChar == NULL)
					vDir = GetDirection() + pRangeShot->GetDirectionMod();
				else
					vDir = (pChar->GetPosition() - GetPosition()) + pRangeShot->GetDirectionMod();

				D3DXMatrixRotationY(&mat, ToRadian(pRangeShot->GetYAxis()));
				D3DXMatrixRotationZ(&mat, ToRadian(pRangeShot->GetZAxis()));
				if (pRangeShot->GetZAxis() != 0 || pRangeShot->GetYAxis() != 0)
				{
					vDir = vDir * mat;
				}
				else
					vDir = vDir;
				Normalize(vDir);

				rvector vPos;
				_RMeshPartsPosInfoType posPartsType = pRangeShot->GetPosPartsType();
				if (posPartsType == eq_parts_pos_info_end)
					vPos = GetPosition();
				else
					vPos = m_pVMesh->GetBipTypePosition(posPartsType);
				//	vPos *= rvector(0,pRangeShot->GetZAxis(), pRangeShot->GetYAxis());
				pRangeShot->ProcessShot(vPos, vDir, this);
				SetProcessedShot(pRangeShot);
			}
		}
	}

	int numGrenadeShot = m_pCurrAction->GetNumGrenadeShot();
	for (int i = 0; i<numGrenadeShot; ++i)
	{
		const ZActorActionGrenadeShot* pGrenadeShot = m_pCurrAction->GetGrenadeShot(i);
		if (pGrenadeShot)
		{

			if (fCurrTime >= m_pCurrActionStartTime + pGrenadeShot->GetDelay() && !IsProcessedShot(pGrenadeShot))
			{
				rvector vPos, vDir;
				rmatrix mat;
				vDir = GetDirection() + pGrenadeShot->GetDirModDirection();

				D3DXMatrixRotationY(&mat, pGrenadeShot->GetYAxis());
				if (pGrenadeShot->GetZAxis() != 0)
				{
					D3DXMatrixRotationZ(&mat, pGrenadeShot->GetZAxis());
				}
				vDir = vDir * mat;
				Normalize(vDir);

				_RMeshPartsPosInfoType posPartsType = pGrenadeShot->GetPosPartsType();
				if (posPartsType == eq_parts_pos_info_end)
					vPos = GetPosition() + pGrenadeShot->GetPosModPosition();
				else
					vPos = m_pVMesh->GetBipTypePosition(posPartsType) + pGrenadeShot->GetPosModPosition();


					pGrenadeShot->ProcessShot(vPos, vDir * pGrenadeShot->GetForce(), this);
					SetProcessedShot(pGrenadeShot);
			}
		}
	}

	int numEffect = m_pCurrAction->GetNumEffect();
	for (int i = 0; i<numEffect; ++i)
	{
		const ZActorActionEffect* pEffect = m_pCurrAction->GetEffect(i);
		if (pEffect && fCurrTime >= m_pCurrActionStartTime + pEffect->GetDelay() && !IsProcessedEffect(pEffect))
		{
			rvector vDir = GetDirection() + pEffect->GetDirectionMod();
			rmatrix mat;
			D3DXMatrixRotationZ(&mat, pEffect->GetZAxis());
			pEffect->GetZAxis() == 0 ? vDir = vDir : vDir = vDir * mat;
			Normalize(vDir);
			rvector vPos;
			_RMeshPartsPosInfoType posPartsType = pEffect->GetPosPartsType();
			if (posPartsType == eq_parts_pos_info_end)
				vPos = GetPosition() + rvector(vDir.x * pEffect->GetPosMod().y, vDir.y * pEffect->GetPosMod().y, pEffect->GetPosMod().z);
			else
				vPos = m_pVMesh->GetBipTypePosition(posPartsType) + rvector(vDir.x * pEffect->GetPosMod().y, vDir.y * pEffect->GetPosMod().y, pEffect->GetPosMod().z);


			pEffect->ProcessShot(pEffect->GetMeshName(), vPos, vDir, m_UID);
			SetProcessedEffect(pEffect);
		}
	}

	int numSounds = m_pCurrAction->GetNumSound();
	for (int i = 0; i<numSounds; ++i)
	{
		const ZActorActionSound* pSound = m_pCurrAction->GetSound(i);
		if (pSound && fCurrTime >= m_pCurrActionStartTime + pSound->GetDelay() && !IsProcessedSound(pSound))
		{
			pSound->ProcessSound(pSound->GetSoundPath(), GetPosition());
			SetProcessedSound(pSound);
		}
	}

	//Check Players in game, if less than the max playercount, spawn less npc's (not in use atm)
	if (ZGetCharacterManager()->size() < ZGetGameClient()->GetMatchStageSetting()->GetMaxPlayers())
	{
		m_bAdjustPlayerNum = true;
	}
	int nNumSummon = m_pCurrAction->GetNumSummon();
	for (int i = 0; i < nNumSummon; ++i)
	{
		const ZActorActionSummon* pSummon = m_pCurrAction->GetSummon(i);
		if (pSummon && fCurrTime >= m_pCurrActionStartTime + pSummon->GetDelay() && !IsProcessedSummon(pSummon))
		{
			rvector pos = GetPosition() + rvector(GetDirection().x * pSummon->GetRange(), GetDirection().y *
				pSummon->GetRange(), 0);
			rvector dir = GetDirection() + rvector(pSummon->GetAngle(),0,0);
			pos.z = 0;
			Normalize(dir);
			pSummon->ProcessSummon(m_uidTarget, m_UID,i,pSummon->GetSummonName(), pos,dir,pSummon->GetAdjustPlayerNum()); //change charger name later
			SetProcessedSummon(pSummon);
		}
	}
}

void ZActorWithFSM::ValidateTarget()
{
	// 없어지거나 죽은 경우 타겟을 해제한다
	ZCharacter* pChar = (ZCharacter*) m_pGame->GetCharacterMgr()->Find(m_uidTarget);
	if (!pChar || pChar->IsDie())
	{
		m_uidTarget.SetInvalid();
		return;
	}
}

void ZActorWithFSM::PostBasicInfo()
{
	DWORD nNowTime = timeGetTime();
	if (GetInitialized() == false) return;

	// 죽고나서 5초가 지나면 basicinfo를 보내지 않는다.
	if(IsDie() && m_pGame->GetTime() - GetDeadTime()>5.f) return;
	//int nMoveTick = (ZGetGameClient()->GetAllowTunneling() == false) ? PEERMOVE_TICK : PEERMOVE_AGENT_TICK;
	int nMoveTick = 25; //40 fps
	//nMoveTick = 1; //todok del

	if ((int)(nNowTime - m_nLastTimePostBasicInfo) >= nMoveTick)
	{
		m_nLastTimePostBasicInfo = nNowTime;

		ZACTOR_WITHFSM_BASICINFO pbi;
		pbi.uidNPC = GetUID();

		pbi.posx = GetPosition().x;
		pbi.posy = GetPosition().y;
		pbi.posz = GetPosition().z;

		pbi.velx = GetVelocity().x;
		pbi.vely = GetVelocity().y;
		pbi.velz = GetVelocity().z;

		pbi.dirx = GetDirection().x*32000.0f;
		pbi.diry = GetDirection().y*32000.0f;
		pbi.dirz = GetDirection().z*32000.0f;

		pbi.nActionIndex = -1;
		if (m_pCurrAction)
			pbi.nActionIndex = m_pActionMgr->GetIndexOfAction(m_pCurrAction);


		ZPOSTCMD1(MC_NEWQUEST_PEER_NPC_BASICINFO, MCommandParameterBlob(&pbi,sizeof(ZACTOR_WITHFSM_BASICINFO)));


		// history 에 보관
#define ACTOR_HISTROY_COUNT 100
		ZBasicInfoItem *pitem=new ZBasicInfoItem;
		pitem->info.position = GetPosition();
		pitem->info.direction = GetDirection();
		pitem->info.velocity = GetVelocity();

		pitem->fSendTime = pitem->fReceivedTime = m_pGame->GetTime();
		m_BasicHistory.push_back(pitem);

		while(m_BasicHistory.size()>ACTOR_HISTROY_COUNT)
		{
			delete *m_BasicHistory.begin();
			m_BasicHistory.pop_front();
		}
	}
}

void ZActorWithFSM::OnBasicInfo(ZACTOR_WITHFSM_BASICINFO* pbi)
{
	if (!IsMyControl())
	{
		SetPosition(rvector(pbi->posx,pbi->posy,pbi->posz));
		SetVelocity(rvector(pbi->velx,pbi->vely,pbi->velz));
		SetDirection(1.f/32000.f * rvector(pbi->dirx,pbi->diry,pbi->dirz));

		IActorAction* pAction = m_pActionMgr->GetActionByIndex(pbi->nActionIndex);
		m_pCurrAction = pAction;
		SetAnimationInAction(pAction);
	}
}

void ZActorWithFSM::OnExitState()
{
	// 탈출 func 실행
	int numExitFunc = m_pCurrState->GetExitFunctionCallCount();
	for (int i=0; i<numExitFunc; ++i)
	{
		const ZFSMFunctionCall* pFunc = m_pCurrState->GetExitFunctionCall(i);
		ExecuteFunction(0, pFunc->id, pFunc->nArg[0], pFunc->nArg[1], pFunc->nArg[2], pFunc->nArg[3]);
	}
	// 이번 상태에서 켜둔 이동중 플래그가 있을 수 있으니 끈다
	SetMoving(false);
}

void ZActorWithFSM::OnEnterState()
{
	m_stateEnteredTimeStamper.SetEnteredTime(m_pCurrState, timeGetTime());

	// 진입 func 실행
	int numEnterFunc = m_pCurrState->GetEnterFunctionCallCount();
	for (int i=0; i<numEnterFunc; ++i)
	{
		const ZFSMFunctionCall* pFunc = m_pCurrState->GetEnterFunctionCall(i);
		ExecuteFunction(0, pFunc->id, pFunc->nArg[0], pFunc->nArg[1], pFunc->nArg[2], pFunc->nArg[3]);
	}

	m_bPostedActionExecute = false;
	m_bPostedRangeShotInAction = false;
	m_RealPositionBefore = rvector(0,0,0);

	// 액션 시작을 알려서 각 peer가 이 액션에 포함된 공격을 처리하도록 한다
	int nActionIndex = -1;
	IActorAction* pAction = m_pCurrState->GetAction();
	if (pAction)
		nActionIndex = m_pActionMgr->GetIndexOfAction(pAction);
	ZPostNewQuestPeerNpcActionExecute(m_UID, nActionIndex);
}

void ZActorWithFSM::RotateToDirection(rvector vTargetDir, float fFrameDelta)
{
	float fRotSpeed = 1.0f;
	if (m_pActorDef)
		fRotSpeed = m_pActorDef->GetRotSpeed();

	rmatrix mat;
	rvector vMyDir = GetDirection();
	float fAngle=GetAngleOfVectors(vTargetDir, vMyDir);
	float fRotAngle = fFrameDelta * fRotSpeed;

	if (fAngle > 0.0f) fRotAngle = -fRotAngle;
	if (fabs(fRotAngle) > fabs(fAngle)) 
	{
		fRotAngle = -fAngle;
	}
	D3DXMatrixRotationZ(&mat, fRotAngle);

	SetDirection(vMyDir * mat);
}



void ZActorWithFSM::AddLandingEffect( rvector vPos, rvector vDir )
{
	ZGetEffectManager()->AddLandingEffect(vPos,vDir);
}

ZOBJECTHITTEST ZActorWithFSM::HitTest( const rvector& origin, const rvector& to, float fTime,  rvector *pOutPos )
{
	//todok 보스의 경우 메쉬피킹하도록 해야 한다. def xml 에 메쉬피킹 여부를 지정할 수 있도록 하기
	return HitTest_Actor(m_pActorDef->GetMeshPicking(), origin, to, fTime, pOutPos);
}

void ZActorWithFSM::OnBlast(rvector &dir)
{
	//if (!IsMyControl()) return;
	//if (!m_pActorDef) { _ASSERT(0); return; }
	//if (m_pActorDef->IsNeverBlasted()) return;
	//
	//if (!m_pFsm) return;
	//ZFSMState* pStateBlastedSoar = m_pFsm->GetState(FSM_BISTATE_BLASTEDSOAR);
	//if (!pStateBlastedSoar) { _ASSERT(0); return; }

	//m_blastedDir = dir;

	//OnExitState();
	//m_pCurrState = pStateBlastedSoar;
	//OnEnterState();
}

void ZActorWithFSM::OnBlastDagger(rvector &dir,rvector& pos)
{
	//if (!IsMyControl()) return;
	//if (!m_pActorDef) { _ASSERT(0); return; }
	//if (m_pActorDef->IsNeverBlasted()) return;

	//if (!m_pFsm) return;
	//ZFSMState* pStateBlastedThrust = m_pFsm->GetState(FSM_BISTATE_BLASTEDTHRUST);
	//if (!pStateBlastedThrust) { _ASSERT(0); return; }

	//m_daggerBlastedDir = dir;
	//m_daggerBlastedPos = pos;

	//OnExitState();
	//m_pCurrState = pStateBlastedThrust;
	//OnEnterState();
}

//void ZActorWithFSM::OnPeerMeleeShot(const char* szActionName)
//{
//	//todok del
//	if (!m_pActionMgr) { _ASSERT(0); return; }
//	
//	IActorAction* pAction = m_pActionMgr->GetAction(szActionName);
//	if (!pAction) { _ASSERT(0); return; }
//
//	const ZActorActionMeleeShot* pMeleeShot = pAction->GetMeleeShot();
//	if (!pMeleeShot) { _ASSERT(0); return; }
//
//	pMeleeShot->ProcessShot(m_pGame, m_UID, ZGetGame()->GetTime());
//}

void ZActorWithFSM::OnPeerActionExecute(int nActionIndex)
{
	m_pCurrAction = NULL;

	if (!m_pActionMgr) { _ASSERT(0); return; }

	IActorAction* pAction = m_pActionMgr->GetActionByIndex(nActionIndex);
	if (!pAction) { return; }

	m_pCurrAction = pAction;
	m_pCurrActionStartTime = ZGetGame()->GetTime();
	m_setProcessedShotInAction.clear();
	m_setProcessedEffectInAction.clear();
	m_setProcessedSoundInAction.clear();
	m_setProcessedSummonInAction.clear(); //custom: added summoning
}

void ZActorWithFSM::OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	if (IsMyControl())
	{
		ZCharacterObject* pCObj = MDynamicCast(ZCharacterObject, pAttacker);
		if (!pCObj)
			return;
		if (pCObj->GetTeamID() == m_nTeamID)
			return;

		if (damageType == ZD_KATANA_SPLASH)
		{
				ZC_ENCHANT etype = pCObj->GetEnchantType();
				if (etype == ZC_ENCHANT_LIGHTNING)
					OnDamaged_LightningSplash();
		}
		m_fGroggy += fDamage;

		//	ZGetEffectManager()->AddWithScale("ef_damage07.elu", this->GetVisualMesh()->GetBipTypePosition(, this->GetDirection(), m_UID, rvector(0.2,0.2,0.2));
	}
	ZObject::OnDamaged(pAttacker, srcPos, damageType, weaponType, fDamage, fPiercingRatio, nMeleeType);
}

void ZActorWithFSM::OnDamaged_LightningSplash()
{
	if (!IsMyControl()) return;
	if (!m_pFsm) { _ASSERT(0); return; }
	
	ZFSMState* pStatetLightningStun = m_pFsm->GetState(FSM_BISTATE_LIGHTNINGSTUN);
	if (!pStatetLightningStun) { _ASSERT(0); return; }

	OnExitState();
	m_pCurrState = pStatetLightningStun;
	OnEnterState();

	SetVelocity(0,0,0);
};

//////////////////////////////////////////////////////////////////////////
//		IFSMConditionSource 구현
//////////////////////////////////////////////////////////////////////////

bool ZActorWithFSM::Cond_IsEndAction()
{
	if (m_pVMesh)
	{
		if (m_pVMesh->isOncePlayDone())
			return true;
	}
	return false;
}

bool ZActorWithFSM::Cond_IsLanding()
{
	return m_pModule_Movable->isLanding();
}


bool ZActorWithFSM::Cond_HasTarget()
{
	if (m_uidTarget.IsValid())
	{
		return true;
	}

	return false;
}

bool ZActorWithFSM::Cond_HasNoTarget()
{
	if (!Cond_HasTarget())
	{
		mlog("Actor has no target");
		return true;
	}
	return false;
}

bool ZActorWithFSM::Cond_TargetHeightHigher(float fHeight)
{
	if (m_uidTarget.IsValid())
	{
		ZCharacter* pChar = (ZCharacter*) m_pGame->GetCharacterMgr()->Find(m_uidTarget);
		int nDiff =  pChar->GetPosition().z - GetPosition().z;
		if(nDiff > fHeight)
			return true;
	}
	return false;
}

bool ZActorWithFSM::CanSee( ZObject* pTarget )
{
	// 시야에 보이는지 확인

	if(pTarget && !ZGetGame()->CheckWall(pTarget,this,true))
	{
	rvector vTargetDir = pTarget->GetPosition() - GetPosition();
	rvector vBodyDir = GetDirection();
	vBodyDir.z = vTargetDir.z = 0.0f;

	float angle = fabs(GetAngleOfVectors(vTargetDir, vBodyDir));
	if (angle <= 25) return true;
	}

	return false;
}


bool ZActorWithFSM::Cond_CanSeeTarget()
{
	if (!m_pGame) { _ASSERT(0); return false; }
	if (m_uidTarget.IsInvalid()) return false;

	ZCharacterObject* pChar = m_pGame->GetCharacterMgr()->Find(m_uidTarget);
	if (!pChar) return false;

	return CanSee(pChar);
}

bool ZActorWithFSM::Cond_CannotSeeTarget()
{
	if (!m_pGame) { _ASSERT(0); return false; }
	if (m_uidTarget.IsInvalid()) return false;
	ZCharacterObject* pChar = m_pGame->GetCharacterMgr()->Find(m_uidTarget);
	if (!CanSee(pChar))
	{
		return true;
	}
	return false;
}

bool ZActorWithFSM::Cond_FailedBuildWayPoints()
{
	if (m_listWaypoint.size() == 0)
	{
		return true;
	}
	return false;
}


void ZActorWithFSM::Cond_GetTargetPos( rvector& out )
{
	out = rvector(0,0,0);

	if (!m_pGame) { _ASSERT(0); return; }
	if (m_uidTarget.IsInvalid()) return;

	ZCharacterObject* pChar = m_pGame->GetCharacterMgr()->Find(m_uidTarget);
	if (!pChar) return;

	out = pChar->GetPosition();
}

bool ZActorWithFSM::Cond_AngleTargetHeight(float fActorPosLow, float fActorPosHigh )
{
	rvector out = rvector(0,0,0);
	rvector out2 = rvector(0,0,0);

	if (!m_pGame) { _ASSERT(0); return false; }
	if (m_uidTarget.IsInvalid()) return false;

	ZCharacterObject* pChar = m_pGame->GetCharacterMgr()->Find(m_uidTarget);
	if (!pChar) return false;

	out = pChar->GetPosition() - GetPosition();
	out2 = GetPosition();

	float angle = fabs(GetAngleOfVectors(out, out2));
	if (angle <= fActorPosHigh && angle >= fActorPosLow) return true;
	return false;
}

bool ZActorWithFSM::Cond_CheckStateCooltime( ZFSMState* pState, DWORD dwCooltime )
{
	if (!m_pFsm) { _ASSERT(0); return false; }

	return m_stateEnteredTimeStamper.CheckCooltime(pState, dwCooltime);
}

DWORD ZActorWithFSM::Cond_GetStateEnteredTime()
{
	if (!m_pFsm) { _ASSERT(0); return false; }
	if (!m_pCurrState) { _ASSERT(0); return false; }

	return m_stateEnteredTimeStamper.GetEnteredTime(m_pCurrState);
}

bool ZActorWithFSM::IsActiveLightningDamageModule()
{
	return IsActiveModule(ZMID_LIGHTNINGDAMAGE);
}

bool ZActorWithFSM::Cond_Pick(float fAngleDegree, float fDist)
{
	rvector pos = GetPosition() + rvector(0,0,100);
	rvector dir = GetDirection();
	dir.z = 0;
	D3DXMATRIX mat;
	D3DXMatrixRotationZ(&mat, ToRadian(fAngleDegree));
	dir = dir*mat;
	Normalize(dir);

	ZPICKINFO pickinfo;
	memset(&pickinfo,0,sizeof(ZPICKINFO));

	const DWORD dwPickPassFlag=RM_FLAG_ADDITIVE | RM_FLAG_HIDE;

	if (m_pGame->Pick(this,pos, dir, &pickinfo, dwPickPassFlag)) 
	{
		rvector pickpos;

		if (pickinfo.bBspPicked)
		{
			pickpos = pickinfo.bpi.PickPos;
		}
		else if (pickinfo.pObject)
		{
			pickpos = pickinfo.pObject->GetPosition();
			pickpos.z += 100;
		}

		float fDistSq = fDist * fDist;
		if (MagnitudeSq(pos - pickpos) < fDistSq)
			return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
//		ZFSMFunctionServer 구현
//////////////////////////////////////////////////////////////////////////

void ZActorWithFSM::Func_FindTarget()
{
	if (!m_pGame) { _ASSERT(0); return; }

	MUID uidTarget	= MUID(0,0);
	float fDist		= FLT_MAX;

	ZCharacterManager* pCharMgr = m_pGame->GetCharacterMgr();
	ZCharacterManager::iterator it;
	ZCharacter* pChar;
	for (it=pCharMgr->begin(); it!=pCharMgr->end(); ++it)
	{
		pChar = (ZCharacter*) it->second;
		if ( pChar->IsDie())
			continue;

		// 거리를 구한다.
		float dist = MagnitudeSq( pChar->GetPosition() - this->GetPosition());

		// 더 가까운 놈이면 이놈을 타겟으로 정한다.
		if ( dist < fDist)
		{
			fDist = dist;
			uidTarget = pChar->GetUID();
		}
	}

	m_uidTarget = uidTarget;
}

void ZActorWithFSM::Func_SetTargetLastAttacker()
{
	m_uidTarget = m_pModule_HPAP->GetLastAttacker();
}

void ZActorWithFSM::Func_RotateToTarget(float fFrameDelta)
{
	if (!m_pGame) { _ASSERT(0); return; }

	if (m_uidTarget.IsInvalid()) return;

	ZCharacterObject* pChar = m_pGame->GetCharacterMgr()->Find(m_uidTarget);
	if (!pChar) return;

	// 이번 프레임에 회전할 만큼만 타겟을 향해 회전
	rvector targetDir(pChar->GetPosition() - GetPosition());
	Normalize(targetDir);

	RotateToDirection(targetDir, fFrameDelta);
}

void ZActorWithFSM::Func_FaceToTarget()
{
	if (!m_pGame) { _ASSERT(0); return; }

	if (m_uidTarget.IsInvalid()) return;

	ZCharacterObject* pChar = m_pGame->GetCharacterMgr()->Find(m_uidTarget);
	if (!pChar) return;

	// 이번 프레임에 회전할 만큼만 타겟을 향해 회전
	rvector targetDir(pChar->GetPosition() - GetPosition());
	targetDir.z = 0;
	Normalize(targetDir);

	SetDirection(targetDir);
}

void ZActorWithFSM::Func_ReduceGroggy(float f)
{
	m_fGroggy -= f;

	//todok del
	//mlog("ㄷACTOR_GROGGY : %f\n", m_fGroggy);
}

void ZActorWithFSM::Func_BuildWaypointsToTarget()
{
	if (!m_pGame) { _ASSERT(0); return; }

	if (m_uidTarget.IsInvalid()) return;

	ZCharacterObject* pTarget = m_pGame->GetCharacterMgr()->Find(m_uidTarget);
	if (!pTarget) return;

	// Make path
	ZNavigationMesh navMesh = m_pGame->GetNavigationMesh();
	if ( navMesh.IsNull())
		return;

	// Make navigation path
	rvector tarpos = pTarget->GetPosition();
	if ( !navMesh.BuildNavigationPath( GetPosition(), tarpos))
		return;

	m_listWaypoint.clear();
	for (list<rvector>::iterator it=navMesh.GetWaypointList().begin(); it!=navMesh.GetWaypointList().end(); ++it)
		m_listWaypoint.push_back(*it);

	//AdjustWayPointWithBound(m_WayPointList, navMesh);
}

void ZActorWithFSM::Func_RunAlongTargetOrbital(float fFrameDelta, int nArg1)
{
	if (!ZGetGame())
	{
		_ASSERT(0);
		return;
	}


	if (m_uidTarget.IsInvalid())
		return;

	ZCharacterObject* pTarget = ZGetGame()->GetCharacterMgr()->Find(m_uidTarget);
	if (!pTarget)
		return;

	//Old Method
	rvector vDistance = pTarget->GetPosition() - GetPosition();
	vDistance.x += vDistance.x < 0.f ? RandomNumber(nArg1 / 6, nArg1) : -RandomNumber(nArg1 / 6, nArg1);
	vDistance.y += vDistance.y < 0.f ? RandomNumber(nArg1 / 6, nArg1) : -RandomNumber(nArg1 / 6, nArg1);
	vDistance.z = 0;

	//New Method
	//rmatrix mat;
	//MakeWorldMatrix(&mat, pTarget->GetPosition(), pTarget->GetDirection(), rvector(0, 0, 1));

	ZNavigationMesh navMesh = ZGetGame()->GetNavigationMesh();
	if (navMesh.IsNull())
		return;

	if (navMesh.BuildNavigationPath(GetPosition(), GetPosition() + vDistance))
	{
		m_listWaypoint.clear();
		for (list<rvector>::iterator it = navMesh.GetWaypointList().begin(); it != navMesh.GetWaypointList().end(); ++it)
			m_listWaypoint.push_back(*it);



		Func_RunWaypoints(fFrameDelta);
	}
}

void ZActorWithFSM::Func_RunWaypoints(float fFrameDelta)
{
	if (m_listWaypoint.empty()) return;
	if (!m_pActorDef) return;


	rvector nextWaypoint = *m_listWaypoint.begin();
	rvector myPos = GetPosition();

	nextWaypoint.z = 0;
	myPos.z = 0;

	float diffSq = MagnitudeSq(nextWaypoint - myPos);
	if (diffSq <= 20.f*20.f)
	{
		m_listWaypoint.pop_front();
		Func_RunWaypoints(fFrameDelta);	// 웨이포인트에 도달했으면 재귀호출로 다음 웨이포인트에 대한 처리를 하게 한다
		return;
	}

	//rvector dir = nextWaypoint - myPos;
	//Normalize(dir);
	//SetDirection(dir);

	rvector vWaypointDir = nextWaypoint - myPos;
	Normalize(vWaypointDir);

	// 다음 웨이포인트를 향해 방향 전환
	RotateToDirection(vWaypointDir, fFrameDelta);

	// 웨이포인트 향해 이동
	//float fSpeed = m_pActorDef->GetSpeed();
	//rvector newPos = vWaypointDir * (fSpeed * fFrameDelta) + myPos;
	//SetPosition(newPos);
	SetMoving(true);
}

void ZActorWithFSM::Func_Disappear(float fFrameDelta, DWORD nDelay, float fRatio)
{
	if (!m_pVMesh) { _ASSERT(0); return; }
	if (!m_pCurrState) { _ASSERT(0); return; }

	DWORD elapsed = timeGetTime() - Cond_GetStateEnteredTime();

	// 딜레이만큼 기다렸다가 투명화 조작
	if (elapsed >= nDelay)
	{
		elapsed -= nDelay;

		float fOpacity = 1.f - fRatio * (elapsed*0.001f);

		if (fOpacity > 1.f) fOpacity = 1.f;
		if (fOpacity < 0) fOpacity = 0;

		m_pVMesh->SetVisibility(fOpacity);
	}
}

void ZActorWithFSM::Func_PostDead()
{
	if (IsMyControl())
		ZPostNewQuestRequestNPCDead(m_pModule_HPAP->GetLastAttacker(), GetUID(), GetPosition());
	else
		_ASSERT(0);
}

void ZActorWithFSM::Func_OnEnterBlastedSoar()
{
	rvector dir = GetDirection();
	dir.x = -m_blastedDir.x;
	dir.y = -m_blastedDir.y;
	SetDirection(dir);

	SetVelocity(m_blastedDir * 300.f + rvector(0,0, (float)RandomNumber( 1500, 1900)));
	//m_bBlasting = true;
	//SetFlag(AF_LAND, false); //todok land 플래그 만들어지면 처리하자
}

void ZActorWithFSM::Func_OnEnterBlastedThrust()
{
	rvector dir = GetDirection();
	dir.x = -m_daggerBlastedDir.x;
	dir.y = -m_daggerBlastedDir.y;
	SetDirection(dir);

	Normalize(m_daggerBlastedDir);
	AddVelocity(m_daggerBlastedDir * (RUN_SPEED * 4));

	m_bSuffering = true;

	/*rvector dir = GetDirection();
	dir.x = -m_daggerBlastedDir.x;
	dir.y = -m_daggerBlastedDir.y;
	SetDirection(dir);

	SetVelocity(m_daggerBlastedDir * 300.f + rvector(0,0,100.f));

	m_vAddBlastVel = GetPosition() - m_daggerBlastedPos;
	m_vAddBlastVel.z = 0.f;

	Normalize(m_vAddBlastVel);

	m_fAddBlastVelTime = m_pGame->GetTime();

	m_bSuffering = true;
	//SetFlag(AF_LAND, false); //todok land 플래그 만들어지면 처리하자*/
}

float accum_fBlastedThrustVelSubtract = 0;

void ZActorWithFSM::Func_OnUpdateBlastedThrust(float fFrameDelta)
{
	_ASSERT(IsSuffering());

	// 시간의 흐름에 따라 속도에서 x, y 성분을 감속시킨다
	rvector dir=rvector(GetVelocity().x,GetVelocity().y,0);
	float fSpeed = Magnitude(dir);
	Normalize(dir);

	const float fBlastThrustBreak = 4000.f;

	fSpeed -= fBlastThrustBreak * fFrameDelta;
	if (fSpeed < 0)
		fSpeed = 0;

	SetVelocity(dir.x*fSpeed, dir.y*fSpeed, GetVelocity().z);

	//// 시간의 흐름에 따라 속도에서 x, y 성분을 감속시킨다
	//rvector vel = GetVelocity();
	//vel.x *= 0.9;
	//vel.y *= 0.9;
	//SetVelocity(vel);

	/*float subtractRatio = 0.9f * fFrameDelta;
	accum_fBlastedThrustVelSubtract += subtractRatio;
	if (accum_fBlastedThrustVelSubtract > 0.1f)
	{
		vel.x -= vel.x * subtractRatio;
		vel.y -= vel.y * subtractRatio;

		SetVelocity(vel);

		accum_fBlastedThrustVelSubtract = 0;
	}*/
	
	//vel.x -= 500.f * fFrameDelta;
	//vel.y -= 500.f * fFrameDelta;

	//todok del
	if (IsSuffering())
	{
		mlog("* suffering vel setted :%f %f %f\n", GetVelocity().x, GetVelocity().y, GetVelocity().z);
	}

/*
#define BLAST_DAGGER_MAX_TIME 0.8f

	float fTime = max((1.f - (m_pGame->GetTime() - m_fAddBlastVelTime) / BLAST_DAGGER_MAX_TIME),0.0f);

	if( fTime < 0.4f )
		fTime = 0.f;

	float fPower = 400.f * fTime * fTime * fFrameDelta * 80.f;

	//todok 아래 주석이 필요할런지? 확인이 필요
//	if(fPower==0.f)
//		SetFlag(AF_BLAST_DAGGER,false);//힘이 다된것

	rvector vel = m_vAddBlastVel * fPower;

	SetVelocity(vel);*/
}

void ZActorWithFSM::Func_ExitSuffering()
{
	m_bSuffering = false;
}

void ZActorWithFSM::Func_OnEnterDie()
{
	SetDeadTime(m_pGame->GetTime());
}

void ZActorWithFSM::Func_FindTargetInHeight(int nHeight)
{
	if (!m_pGame) { _ASSERT(0); return; }

	MUID uidTarget	= MUID(0,0);
	float fDist		= FLT_MAX;

	ZCharacterManager* pCharMgr = m_pGame->GetCharacterMgr();
	ZCharacterManager::iterator it;
	ZCharacter* pChar;
	for (it=pCharMgr->begin(); it!=pCharMgr->end(); ++it)
	{
		pChar = (ZCharacter*) it->second;
		if ( pChar->IsDie())
			continue;

		// 거리를 구한다.
		float dist = MagnitudeSq( pChar->GetPosition() - this->GetPosition());

		// 더 가까운 놈이면 이놈을 타겟으로 정한다.
		int nDiff = pChar->GetPosition().z - this->GetPosition().z;

		if ( dist < fDist && (nDiff > -nHeight && nDiff < nHeight))
		{
			fDist = dist;
			uidTarget = pChar->GetUID();
		}
	}

	m_uidTarget = uidTarget;
}


void ZActorWithFSM::Func_SpeedAccel(int nArg1)
{
	/*
	rvector dir=rvector(GetVelocity().x,GetVelocity().y,0);
	//float fSpeed = Magnitude(dir);
	Normalize(dir);
	SetVelocity(dir.x+fFrameDelta, dir.y*fFrameDelta, GetVelocity().z);
	*/
	SetSpeedAccel(nArg1);
}

void ZActorWithFSM::Func_ClearWayPoints()
{
	if (m_listWaypoint.empty()) return;
	m_listWaypoint.clear();
}

void ZActorWithFSM::Func_TurnOrbitalDir()
{
	rvector newDir(GetDirection());
	newDir.z = 0;
	Normalize(newDir);
	newDir.x = (0.f - newDir.x);
	newDir.y = (0.f - newDir.y);
	if (newDir.x > 1.f || newDir.x < -1.f)
	{
		newDir.x = 0.f;
	}
	if (newDir.y > 1.f || newDir.y < -1.f)
	{
		newDir.y = 0.f;
	}
	//RotateToDirection(newDir)
	SetDirection(newDir);
}

bool ZActorWithFSM::Cond_SummonLess(int nArg1)
{
	ZRuleQuestChallenge* pCQRule = NULL;
	if (ZGetGame()->GetMatch()->GetRule())
		pCQRule = (ZRuleQuestChallenge*)ZGetGame()->GetMatch()->GetRule();
	//Arg1 is int number of npcs to spawn.
	if (m_bAdjustPlayerNum)
	{
		//
	}
	if(IsMyControl() && pCQRule->GetNPC() - 1 < nArg1) //ignore boss
	{
		return true;
	}
	return false;
}

void ZActorWithFSM::Func_FindTargetInDist(int nDist)
{
	if (!m_pGame) { _ASSERT(0); return; }

	MUID uidTarget = MUID(0, 0);
	float fDist = FLT_MAX;


	for (auto itor = ZGetObjectManager()->begin(); itor != ZGetObjectManager()->end(); ++itor)
	{
		ZCharacterObject* pObj = (ZCharacterObject*)(*itor).second;
		if (pObj->IsDie())
			continue;

		// 거리를 구한다.
		float dist = MagnitudeSq(pObj->GetPosition() - this->GetPosition());

		// 더 가까운 놈이면 이놈을 타겟으로 정한다.
		int nDiff = pObj->GetPosition().x - this->GetPosition().x;

		if (dist < fDist && (nDiff > -nDist && nDiff < nDist))
		{
			fDist = dist;
			uidTarget = pObj->GetUID();
		}
	}

	m_uidTarget = uidTarget;
}

void ZActorWithFSM::Func_RunWayPointsAlongRoute(float fFrameDelta)
{

	if (m_listWaypoint.empty()) return;
	if (!m_pActorDef) return;

	rvector nextWaypoint = *m_listWaypoint.begin();
	rvector myPos = GetPosition();

	nextWaypoint.z = 0;
	myPos.z = 0;

	float diffSq = MagnitudeSq(nextWaypoint - myPos);
	if (diffSq <= 20.f*20.f)
	{
		m_listWaypoint.pop_front();
		this->Func_RunWayPointsAlongRoute(fFrameDelta);	// 웨이포인트에 도달했으면 재귀호출로 다음 웨이포인트에 대한 처리를 하게 한다
		return;
	}

	//rvector dir = nextWaypoint - myPos;
	//Normalize(dir);
	//SetDirection(dir);

	rvector vWaypointDir = nextWaypoint - myPos;
	Normalize(vWaypointDir);

	// 다음 웨이포인트를 향해 방향 전환
	RotateToDirection(vWaypointDir, fFrameDelta);

	// 웨이포인트 향해 이동
	//float fSpeed = m_pActorDef->GetSpeed();
	//rvector newPos = vWaypointDir * (fSpeed * fFrameDelta) + myPos;
	//SetPosition(newPos);
	SetMoving(true);
}

void ZActorWithFSM::Func_FaceToLatestAttacker()
{
	if (!m_pGame) { _ASSERT(0); return; }

	if (m_uidTarget.IsInvalid()) return;

	ZCharacterObject* pObj = (ZCharacterObject*)ZGetObjectManager()->GetObjectA(GetLastAttacker());
	if (!pObj) return;

	// 이번 프레임에 회전할 만큼만 타겟을 향해 회전
	rvector targetDir(pObj->GetPosition() - GetPosition());
	targetDir.z = 0;
	Normalize(targetDir);

	SetDirection(targetDir);
}

bool ZActorWithFSM::Save(ZFile* pFile)
{
	// TODO: Making recordings work in challengequest
	return false;
}