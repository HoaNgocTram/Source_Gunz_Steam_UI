#pragma once

class ZActorActionRangeShot;
class ZActorActionGrenadeShot;

class ZWeaponNpcBullet : public ZMovingWeapon
{
public:
	MDeclareRTTI
private:
	float		m_fScale;
	ZObject*	m_pOwner;
	const ZActorActionRangeShot* m_pRangeShot;
public:
	ZWeaponNpcBullet() : ZMovingWeapon() { m_fScale = 1.f; }
	void Create(RMesh* pMesh, const ZActorActionRangeShot* pRangeShot, const rvector &pos, const rvector &dir, float fScale, ZObject* pOwner);
	bool Update(float fElapsedTime);
	void Render();
	void Explosion(ZObject* pVictim, const rvector& dir);
	void ExplosionOnMap();
	float	m_fStartTime;
	float	m_fLastAddTime;
};
