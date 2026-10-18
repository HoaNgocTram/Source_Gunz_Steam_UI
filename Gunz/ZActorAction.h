#pragma once


class ZActorActionMeleeShot;
class ZActorActionRangeShot;
class ZActorActionGrenadeShot;
class ZActorActionEffect;
class ZActorActionSound;
class ZActorActionSummon;
class IActorAction
{
public:
	virtual ~IActorAction() {}

	virtual const char* GetName() = 0;
	virtual const char* GetAnimationName() = 0;
	virtual bool IsMovingAnimation() = 0;
	virtual const ZActorActionMeleeShot* GetMeleeShot(int idx) = 0;
	virtual const ZActorActionRangeShot* GetRangeShot(int idx) = 0;
	virtual const ZActorActionGrenadeShot* GetGrenadeShot(int idx) = 0;
	virtual const ZActorActionEffect* GetEffect(int idx) = 0;
	virtual const ZActorActionSound* GetSound(int idx) = 0;
	virtual const ZActorActionSummon* GetSummon(int idx) = 0;
	virtual int GetNumMeleeShot() = 0;
	virtual int GetNumGrenadeShot() = 0;
	virtual int GetNumRangeShot() = 0;
	virtual int GetNumSound() = 0;
	virtual int GetNumEffect() = 0;
	virtual int GetNumSummon() = 0;
};

// 액션은 기존에 npc의 특수행동을 정의하기 위해 사용했던 ZSkill과 비슷하다.
// 그러나 액션은 꼭 공격스킬일 필요는 없으며, 어떤 애니메이션을 단순히 재생하는 용도로 사용될 수도 있다.
// 공격스킬일 때도 ZSkill과는 다르게 공격판정을 액션 내에 직접 갖고있지 않으며 공격판정을 가진 객체를 
// 월드에 생성하기 위한 정보만을 갖게 된다.


class ZActorAction : public IActorAction
{
	string m_strName;
	string m_strAnimation;
	bool m_bMovingAnimation;

	ZActorActionMeleeShot* m_pMeleeShot;

	typedef vector<ZActorActionMeleeShot*>  VecMeleeShot;
	typedef VecMeleeShot::iterator			ItorMeleeShot;
	VecMeleeShot m_vecMeleeShot;

	typedef vector<ZActorActionRangeShot*>	VecRangeShot;
	typedef VecRangeShot::iterator			ItorRangeShot;
	VecRangeShot m_vecRangeShot;

	typedef vector<ZActorActionGrenadeShot*> VecGrenadeShot;
	typedef VecGrenadeShot::iterator		 ItorGrenadeShot;
	VecGrenadeShot m_vecGrenadeShot;

	typedef vector<ZActorActionEffect*>	VecEffect;
	typedef VecEffect::iterator			ItorEffect;
	VecEffect m_vecEffect;

	typedef vector<ZActorActionSound*>	VecSound;
	typedef VecSound::iterator			ItorSound;
	VecSound m_vecSound;

	typedef vector<ZActorActionSummon*>	VecSummon;
	typedef VecSummon::iterator			ItorSummon;
	VecSummon m_vecSummon;

public:
	ZActorAction()
		: m_pMeleeShot(0)
		, m_bMovingAnimation(false) {}
	virtual ~ZActorAction();

	void SetName(const char* sz) { m_strName = sz; }
	virtual const char* GetName() { return m_strName.c_str(); }

	void SetAnimationName(const char* sz) { m_strAnimation = sz; }
	virtual const char* GetAnimationName() { return m_strAnimation.c_str(); }

	void SetMovingAnimation(bool b) { m_bMovingAnimation = b; }
	virtual bool IsMovingAnimation() { return m_bMovingAnimation; }

	void SetMeleeShot(ZActorActionMeleeShot* p) { m_vecMeleeShot.push_back(p); }
	virtual const ZActorActionMeleeShot* GetMeleeShot(int idx) {
		if (idx < 0 || GetNumMeleeShot() <= idx) { _ASSERT(0); return NULL; }
		return m_vecMeleeShot[idx];
	}

	void AddRangeShot(ZActorActionRangeShot* p) { m_vecRangeShot.push_back(p); }
	virtual const ZActorActionRangeShot* GetRangeShot(int idx) {
		if (idx < 0 || GetNumRangeShot() <= idx) { _ASSERT(0); return NULL; }
		return m_vecRangeShot[idx];
	}

	void AddGrenadeShot(ZActorActionGrenadeShot* p) { m_vecGrenadeShot.push_back(p); }
	virtual const ZActorActionGrenadeShot* GetGrenadeShot(int idx) {
		if (idx < 0 || GetNumGrenadeShot() <= idx) { _ASSERT(0); return NULL; }
		return m_vecGrenadeShot[idx];
	}

	void AddEffect(ZActorActionEffect* p) { m_vecEffect.push_back(p); }
	virtual const ZActorActionEffect* GetEffect(int idx) {
		if (idx < 0 || GetNumEffect() <= idx) { _ASSERT(0); return NULL; }
		return m_vecEffect[idx];
	}

	void AddSound(ZActorActionSound* p) { m_vecSound.push_back(p); }
	virtual const ZActorActionSound* GetSound(int idx) {
		if (idx < 0 || GetNumSound() <= idx) { _ASSERT(0); return NULL; }
		return m_vecSound[idx];
	}

	void AddSummon(ZActorActionSummon* p) { m_vecSummon.push_back(p); }
	virtual const ZActorActionSummon* GetSummon(int idx) {
		if (idx < 0 || GetNumSummon() <= idx) { _ASSERT(0); return NULL; }
		return m_vecSummon[idx];
	}

	virtual int GetNumMeleeShot() { return (int)m_vecMeleeShot.size(); }
	virtual int GetNumRangeShot() { return (int)m_vecRangeShot.size(); }
	virtual int GetNumGrenadeShot() { return (int)m_vecGrenadeShot.size(); }
	virtual int GetNumEffect() { return (int)m_vecEffect.size(); }
	virtual int GetNumSound() { return (int)m_vecSound.size(); }
	virtual int GetNumSummon() { return (int)m_vecSummon.size(); }
};

// 밀리샷과 레인지샷의 공통 속성 
class ZActorActionShotCommon
{
protected:
	float m_fDelay;		// 액션이 시작한 뒤 이 딜레이만큼 기다렸다가 샷이 발생
	float m_fDamage;
	float m_fPierce;	// 관통률
	string m_strSound;	// 히트 사운드(지정되지 않은 경우 기본 사운드 발생)
	float m_fKnockbackForce;	// 히트시 넉백 힘
	bool m_bUpperCut;
	bool m_bThrust;
public:
	ZActorActionShotCommon()
		: m_fDelay(0)
		, m_fDamage(0)
		, m_fPierce(0.5f)
		, m_fKnockbackForce(0)
		, m_bUpperCut(false)
		, m_bThrust(false) {}
	virtual ~ZActorActionShotCommon() {}

	float GetDelay() const { return m_fDelay; }
	float GetDamage() const { return m_fDamage; }
	float GetPierce() const { return m_fPierce; }
	const char* GetSound() const { return m_strSound.c_str(); }
	float GetKnockbackForce() const { return m_fKnockbackForce; }
	bool GetUpperCut() const { return m_bUpperCut; }
	bool GetThrust() const { return m_bThrust; }

	void SetDelay(float f) { m_fDelay = f; }
	void SetDamage(float f) { m_fDamage = f; }
	void SetPierce(float f) { m_fPierce = f; }
	void SetSound(const char* sz) { m_strSound = sz; }
	void SetKnockbackForce(float f) { m_fKnockbackForce = f; }
	void SetUpperCut(bool b) { m_bUpperCut = b; }
	void SetThrust(bool b) { m_bThrust = b; }
};

class ZActorActionMeleeShot : public ZActorActionShotCommon
{
	float m_fRange;		// 판정 최대 거리
	float m_fAngle;		// 판정 반경 radian

public:
	ZActorActionMeleeShot()
		: m_fRange(0)
		, m_fAngle(0) {}

	float GetRange() const { return m_fRange; }
	float GetAngle() const { return m_fAngle; }

	void SetRange(float f) { m_fRange = f; }
	void SetAngle(float f) { m_fAngle = f; }

	void ProcessShot(ZGame* pGame, const MUID& uidOwner, float fShotTime) const;
};

class ZActorActionRangeShot : public ZActorActionShotCommon
{
	string m_strMeshName;
	_RMeshPartsPosInfoType m_posPartsType;	// 탄환의 발사위치
	float m_fSpeed;
	float m_fCollRadius;	//todok 이것은 폭발형(스플래쉬) 일때만 사용한다. 나중에 정리 필요
	rvector m_vDirectionMod;	// 자신이 보는 방향 기준으로 발사 각도 보정량
	int m_nZAxis;
	int m_nYAxis;
	bool m_bDirTarget;
public:
	ZActorActionRangeShot()
		: m_fSpeed(100.f)
		, m_fCollRadius(100.f)
		, m_vDirectionMod(0, 0, 0)
		, m_nZAxis(0)
		, m_nYAxis(0)
		, m_bDirTarget(false)
	{}

	const char* GetMeshName() const { return m_strMeshName.c_str(); }
	_RMeshPartsPosInfoType GetPosPartsType() const { return m_posPartsType; }
	float GetSpeed() const { return m_fSpeed; }
	float GetCollRadius() const { return m_fCollRadius; }
	const rvector& GetDirectionMod() const { return m_vDirectionMod; }
	int GetZAxis() const { return m_nZAxis; }
	int GetYAxis() const { return m_nYAxis; }
	bool GetDirTarget() const { return m_bDirTarget; }

	void SetMeshName(const char* sz) { m_strMeshName = sz; }
	void SetPosPartsType(_RMeshPartsPosInfoType t) { m_posPartsType = t; }
	void SetSpeed(float f) { m_fSpeed = f; }
	void SetCollRadius(float f) { m_fCollRadius = f; }
	void SetDirectionMod(const rvector& v) { m_vDirectionMod = v; }
	void SetZAxis(int n) { m_nZAxis = n; }
	void SetYAxis(int n) { m_nYAxis = n; }
	void SetDirTarget(bool b) { m_bDirTarget = b; }

	void ProcessShot(const rvector& pos, const rvector& dir, ZObject* pOwner) const;
};

class ZActorActionGrenadeShot : public ZActorActionShotCommon
{
protected:
	float m_fDelay;
	float m_fDamage;
	float m_fPierce;
	int   m_nGrenadeType;
	int	  m_nItemID;
	int   m_nZAxis;
	int   m_nYAxis;
	int   m_nForce;
	_RMeshPartsPosInfoType m_posPartsType;
	rvector r_PosMod;
	rvector r_DirMod;
	string StrSound;
public:
	void SetGrenadeType(int i) { m_nGrenadeType = i; }
	void SetItemID(int i) { m_nItemID = i; };
	void SetYAxis(int i) { m_nYAxis = i; };
	void SetZAxis(int i) { m_nZAxis = i; };
	void SetForce(int i) { m_nForce = i; };
	void SetPosPartsInfo(_RMeshPartsPosInfoType info) { m_posPartsType = info; };
	void SetPosModPosition(const rvector& position) { r_PosMod = position; };
	void SetDirModDirection(const rvector& direction) { r_DirMod = direction; };
	int  GetGrenadeType() { return m_nGrenadeType; };
	int  GetItemID() const { return m_nItemID; };
	int  GetYAxis()  const { return m_nYAxis; }
	int  GetZAxis()  const { return m_nZAxis; }
	int  GetForce()  const { return m_nForce; }
	const rvector& GetPosModPosition() const { return r_PosMod; };
	const rvector& GetDirModDirection() const { return r_DirMod; };
	_RMeshPartsPosInfoType GetPosPartsType() const { return m_posPartsType; };
	void ProcessShot(const rvector& pos, const rvector& dir, ZObject* pOwner) const;
};
class ZActorActionEffect
{
protected:
	string m_strMeshName;
	_RMeshPartsPosInfoType m_posPartsType;	// 탄환의 발사위치
	rvector m_vDirectionMod;	// 자신이 보는 방향 기준으로 발사 각도 보정량
	rvector m_vPosMod;	// 자신이 보는 방향 기준으로 발사 각도 보정량
	rvector m_vScale;
	float m_fDelay;		// 액션이 시작한 뒤 이 딜레이만큼 기다렸다가 샷이 발생
	int m_nZAxis;

public:
	ZActorActionEffect()
		: m_fDelay(100.f)
		, m_vDirectionMod(0, 0, 0)
		, m_vPosMod(0, 0, 0)
		, m_vScale(1.f, 1.f, 1.f)
		, m_nZAxis(0)
	{}
	const char* GetMeshName() const { return m_strMeshName.c_str(); }
	float GetDelay() const { return m_fDelay; }
	const rvector& GetScale() const { return m_vScale; }
	_RMeshPartsPosInfoType GetPosPartsType() const { return m_posPartsType; }
	const rvector& GetDirectionMod() const { return m_vDirectionMod; }
	const rvector& GetPosMod() const { return m_vPosMod; }
	int GetZAxis() const { return m_nZAxis; }

	void SetScale(rvector v) { m_vScale = v; }
	void SetMeshName(const char* sz) { m_strMeshName = sz; }
	void SetPosPartsType(_RMeshPartsPosInfoType t) { m_posPartsType = t; }
	void SetDirectionMod(const rvector& v) { m_vDirectionMod = v; }
	void SetPosMod(const rvector& v) { m_vPosMod = v; }
	void SetDelay(float f) { m_fDelay = f; }
	void SetZAxis(int n) { m_nZAxis = n; }
	void ProcessShot(const char* szMeshName, const rvector& pos, const rvector& dir, const MUID& uidOwner) const;
};

class ZActorActionSound
{
protected:
	string m_strSoundPath;
	float m_fDelay;

public:
	ZActorActionSound()
		: m_fDelay(100.f)
	{}
	const char* GetSoundPath() const { return m_strSoundPath.c_str(); }
	float GetDelay() const { return m_fDelay; }

	void SetSoundPath(const char* sz) { m_strSoundPath = sz; }
	void SetDelay(float f) { m_fDelay = f; }

	void ProcessSound(string m_strSoundPath, const rvector& pos) const;
};

class ZActorActionSummon
{
protected:
	string m_strSummonName;
	float  m_fDelay;
	float  m_fRange;
	float  m_fAngle;
	int    m_nRoute;
	bool   m_bAdjustPlayerNum;
	const char* m_szDropTableID;
public:
	ZActorActionSummon()
		: m_fDelay(100.f)
		, m_fRange(400.f)
		, m_fAngle(-90.f)
		, m_nRoute(100)
		, m_bAdjustPlayerNum(false)
	{}
	const char* GetSummonName() const { return m_strSummonName.c_str(); }
	float GetDelay() const { return m_fDelay; }
	float GetRange() const { return m_fRange; }
	float GetAngle() const { return m_fAngle; }
	int GetRoute() const { return m_nRoute; }
	bool GetAdjustPlayerNum() const { return m_bAdjustPlayerNum; }
	const char* GetDropTableID() const { return m_szDropTableID; }

	void SetSummonName(const char* sz) { m_strSummonName = sz; }
	void SetDelay(float f) { m_fDelay = f; }
	void SetRange(float f) { m_fRange = f; }
	void SetAngle(float f) { m_fAngle = f; }
	void SetRoute(int n) { m_nRoute = n; }
	void SetAdjustPlayerNum(bool b) { m_bAdjustPlayerNum = b; }
	void SetDropTableID(const char* sz) { m_szDropTableID = sz; }

	void ProcessSummon(MUID& uidOwner, MUID& uidTarget, int nNum, string strNpcName, const rvector& pos, const rvector& dir, bool bAdjustPlayerNum) const;
};