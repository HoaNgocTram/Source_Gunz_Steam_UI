#ifndef _ZWEAPONSCREENEFFECT_H
#define _ZWEAPONSCREENEFFECT_H

#include "ZPrerequisites.h"

class ZScreenEffect;

class ZWeaponScreenEffect
{
private:
	MMatchWeaponType		m_nWeaponType;
	MBitmap*				m_pSniferPanel;
	MBitmap*				m_pSniferWidescreenPanel; // Custom: Widescreen support scope
	//ZScreenEffect*			m_pSniferPanelEffect;


	void InitWeapon(MMatchWeaponType nWeaponType);
	void FinalWeapon(MMatchWeaponType nWeaponType);
	float CustomFOV = 0.0f;                  // Fix Custom FOV 14/9/2023 Desperate
public:
	ZWeaponScreenEffect();
	virtual ~ZWeaponScreenEffect();
public:
	void Draw(MDrawContext* pDC);
	void OnGadget(MMatchWeaponType nWeaponType);
	void OnGadgetOff();
	bool IsScopeEnabled();
	void Create();
	void Destroy();
};




#endif
