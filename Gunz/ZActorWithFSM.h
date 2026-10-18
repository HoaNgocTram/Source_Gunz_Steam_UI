#pragma once
#include "ZActorBase.h"
#include "ZFSMCondition.h"
#include "ZFSMStateEnteredTimeStamper.h"
#include "ZFSMFunctionServer.h"
#include "MActorDef.h"

class ZActorActionManager;
class MActorDef;
class ZFSMManager;
class ZFSM;
class ZFSMState;
class IActorAction;
class ZActorActionShotCommon;
class ZActorActionGrenadeShot;
class ZActorActionEffect;

// 일정 시간간격으로 NPC정보 계속 주고받는 데이터
struct ZACTOR_WITHFSM_BASICINFO {
	//float			fTime;	//todok del
	MUID			uidNPC;
	short			posx,posy,posz;
	short			velx,vely,velz;
	short			dirx,diry,dirz;
	int				nActionIndex;
};

//todok 아래 가상함수들 중 빈구현이나 임시값 반환하는 것들 처분해야 한다

class ZActorWithFSM : public ZActorBase, public ZFSMFunctionServer, public IFSMConditionSource
{
	MDeclareRTTI;

private:
	ZActorActionManager* m_pActionMgr;

	MActorDef* m_pActorDef;
	ZFSM* m_pFsm;
	ZFSMState* m_pCurrState;
	ZFSMStateEnteredTimeStamper m_stateEnteredTimeStamper;

	bool m_bPostedActionExecute;	// 액션이 시작되었음을 알렸는가//todok del
	bool m_bPostedRangeShotInAction;	// 현재 state의 액션이 range shot을 갖고 있다면 이것을 peer post했는가	//todok del

	IActorAction* m_pCurrAction;		// 현재 실행 중인 액션
	float m_pCurrActionStartTime;		// 현재 실행 중인 액션 시작 시각
	
	set<const ZActorActionShotCommon*> m_setProcessedShotInAction;	// 현재 실행 중인 액션 내에서 처리된 샷 목록
	bool IsProcessedShot(const ZActorActionShotCommon* p) { return m_setProcessedShotInAction.end() != m_setProcessedShotInAction.find(p); }
	void SetProcessedShot(const ZActorActionShotCommon* p) { m_setProcessedShotInAction.insert(p); }
	set<const ZActorActionEffect*> m_setProcessedEffectInAction;	// 현재 실행 중인 액션 내에서 처리된 샷 목록
	bool IsProcessedEffect(const ZActorActionEffect* p) { return m_setProcessedEffectInAction.end() != m_setProcessedEffectInAction.find(p); }
	void SetProcessedEffect(const ZActorActionEffect* p) { m_setProcessedEffectInAction.insert(p); }
	set<const ZActorActionSound*> m_setProcessedSoundInAction;	// 현재 실행 중인 액션 내에서 처리된 샷 목록
	bool IsProcessedSound(const ZActorActionSound* p) { return m_setProcessedSoundInAction.end() != m_setProcessedSoundInAction.find(p); }
	void SetProcessedSound(const ZActorActionSound* p) { m_setProcessedSoundInAction.insert(p); }
	set<const ZActorActionSummon*> m_setProcessedSummonInAction;
	bool IsProcessedSummon(const ZActorActionSummon* p) { return m_setProcessedSummonInAction.end() != m_setProcessedSummonInAction.find(p); }
	void SetProcessedSummon(const ZActorActionSummon* p) { m_setProcessedSummonInAction.insert(p);}

	float m_fGroggy;
	unsigned long m_nLastUpdate;
	MUID m_uidTarget;
	unsigned long int		m_nFlags;

	list<rvector> m_listWaypoint;

	rvector m_blastedDir;
	rvector m_daggerBlastedDir;
	rvector m_daggerBlastedPos;

	rvector m_RealPositionBefore;	// 이동이 포함된 애니메이션 처리용


	ZModule_HPAP			*m_pModule_HPAP;
	ZModule_Resistance		*m_pModule_Resistance;
	ZModule_FireDamage		*m_pModule_FireDamage;
	ZModule_ColdDamage		*m_pModule_ColdDamage;
	ZModule_PoisonDamage	*m_pModule_PoisonDamage;
	ZModule_LightningDamage	*m_pModule_LightningDamage;

	bool m_bMyControl;
	unsigned long int m_nLastTimePostBasicInfo;
	MMatchTeam	 m_nTeamID;
public:
	ZActorWithFSM(ZGame* pGame, ZActorActionManager* pActionMgr);
	~ZActorWithFSM();

	virtual MMatchTeam GetTeamID() { return m_nTeamID; }			// NPC는 무조건 BLUE팀

	virtual void OnDraw();
	virtual void OnUpdate(float fDelta);

	virtual void SetMyControl(bool bMyControl);
	virtual bool IsMyControl();
	virtual void SetTeamID(MMatchTeam nTeamID) { m_nTeamID = nTeamID; }

	bool IsMovingAnimation();
	virtual bool IsCollideable();
	virtual bool IsDie();

	virtual bool IsNeverPushed() { return false; }
	virtual float GetCollideRadius() { return 35.f; }
	virtual float ModifyAttackDamage(float fDamage) { return fDamage; }

	virtual int GetActualHP() { return m_pModule_HPAP->GetHP(); }
	virtual int GetActualAP() { return m_pModule_HPAP->GetAP(); }
	virtual int GetActualMaxHP() { return m_pModule_HPAP->GetMaxHP(); }
	virtual int GetActualMaxAP() { return m_pModule_HPAP->GetMaxAP(); }

	inline bool ZActorWithFSM::CheckFlag(unsigned int nFlag)
	{
		return ((m_nFlags & nFlag) != 0);
	}

	virtual void InputBasicInfo(ZBasicInfo* pni, BYTE anistate) {}
	virtual void InputBossHpAp(float fHp, float fAp) {}

	virtual void OnPeerDie(MUID& uidKiller) {}
	virtual void OnDie() {}

	// 디버그용
	virtual void GetDebugInfoString(char* szBuf1, char* szBuf2, int index) {}

public:
	// IFSMConditionSource 구현
	virtual float Cond_GetHp() { return GetActualHP(); }
	virtual float Cond_GetGroggy() { return m_fGroggy; }
	virtual const rvector& Cond_GetVelocity() { return GetVelocity(); }
	virtual bool Cond_IsEndAction();
	virtual bool Cond_IsLanding();

	virtual bool Cond_HasTarget();
	virtual bool Cond_HasNoTarget();
	virtual bool Cond_TargetHeightHigher(float fHeight);
	virtual bool Cond_CanSeeTarget();
	virtual bool Cond_CannotSeeTarget();
	virtual void Cond_GetPos(rvector& out) { out = GetPosition(); }
	virtual void Cond_GetTargetPos(rvector& out);
	virtual bool Cond_AngleTargetHeight(float fActorPos1,float fActorPos2);
	virtual bool Cond_FailedBuildWayPoints();
	virtual bool Cond_SummonLess(int nArg1);
	virtual void Cond_GetDirection(rvector& out) { out = GetDirection(); }

	virtual int Cond_GetDice() { return GetDice(); }

	virtual bool Cond_CheckStateCooltime(ZFSMState* pState, DWORD dwCooltime);
	virtual DWORD Cond_GetStateEnteredTime();

	virtual bool IsActiveLightningDamageModule();

	virtual bool Cond_Pick(float fAngleDegree, float fDist);

public:
	// ZFSMFunctionServer 구현
	virtual void Func_FindTarget();
	virtual void Func_SetTargetLastAttacker();
	virtual void Func_RotateToTarget(float fFrameDelta);
	virtual void Func_FaceToTarget();

	virtual void Func_ReduceGroggy(float f);

	virtual void Func_BuildWaypointsToTarget();
	virtual void Func_RunWaypoints(float fFrameDelta);
	
	virtual void Func_Disappear(float fFrameDelta, DWORD nDelay, float fRatio);
	virtual void Func_PostDead();
	virtual void Func_OnEnterBlastedSoar();
	virtual void Func_OnEnterBlastedThrust();
	virtual void Func_OnUpdateBlastedThrust(float fFrameDelta);
	virtual void Func_ExitSuffering();
	virtual void Func_OnEnterDie();

	virtual void Func_FindTargetInHeight(int nHeight);
	virtual void Func_RunAlongTargetOrbital(float fFrameDelta,int nArg1);
	virtual void Func_SpeedAccel(int nArg1);
	virtual void Func_TurnOrbitalDir();
	virtual void Func_ClearWayPoints();
	virtual void Func_FindTargetInDist(int nDist);
	virtual void Func_RunWayPointsAlongRoute(float fFrameDelta);
	virtual void Func_FaceToLatestAttacker();
	bool         Save(ZFile* pFile);

public:
	bool InitWithActorDef(MActorDef* pActorDef, ZFSMManager* pFsmMgr);
	MActorDef* GetActorDef() { return m_pActorDef; };
	MUID GetTargetUID() { return m_uidTarget; }
	MUID GetLastAttacker() { return m_pModule_HPAP->GetLastAttacker(); }
	void SetLastAttacker(MUID uid) { m_pModule_HPAP->SetLastAttacker(uid); }

private:
	bool m_bSuffering;		// 움직일 수 없는 피격 동작 중인가
	bool m_bMoving;
	int  m_nSpeedAccel;
	bool m_bAdjustPlayerNum;
public:
	bool IsMoving() { return m_bMoving; }
	void SetMoving(bool b) { m_bMoving = b; }
	bool IsSuffering() { return m_bSuffering; }
	bool IsAir() 
	{
		// 공중에 떠있거나 떠오르는 중인지 체크
		bool bAir = (GetDistToFloor() > 0.1f || GetVelocity().z > 0.1f);
		return bAir;
	}
	int GetSpeedAccel() { return m_nSpeedAccel; }
	void SetSpeedAccel(int i) { m_nSpeedAccel = i; }

public:
	void InitMesh(char* szMeshName);
	bool ProcessMotion(float fDelta);
	void ProcessShotInAction(float fDelta);
	void UpdateVelocity(float fDelta);
	void UpdatePosition(float fDelta);
	void UpdateHeight(float fDelta);
	void UpdateGroggy(float fDelta);
	void SetAnimationInAction(IActorAction* pAction);
	void UpdateFsm(float fDelta);
	void OnExitState();
	void OnEnterState();
	bool CanSee(ZObject* pTarget);
	ZFSMState* CheckGlobalTransition();

	void RotateToDirection(rvector vTargetDir, float fFrameDelta);

	virtual void AddLandingEffect(rvector vPos, rvector vDir);

	void ValidateTarget();

	void PostBasicInfo();
	void OnBasicInfo(ZACTOR_WITHFSM_BASICINFO* pbi);

	// 동작/이벤트 상속
	//void OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType);

	virtual void OnBlast(rvector &dir);
	virtual void OnBlastDagger(rvector &dir,rvector& pos);

	virtual ZOBJECTHITTEST HitTest( const rvector& origin, const rvector& to, float fTime, rvector *pOutPos );
	//void OnPeerMeleeShot(const char* szActionName);	//todok del
	void OnPeerActionExecute(int nActionIndex);
	void OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType);
	void OnDamaged_LightningSplash();
};