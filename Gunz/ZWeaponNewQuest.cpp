#include "StdAfx.h"
#include "ZConfiguration.h"
#include "ZStencilLight.h"
#include "ZWeaponNewQuest.h"
#include "ZActorAction.h"

MImplementRTTI(ZWeaponNpcBullet, ZMovingWeapon);

void ZWeaponNpcBullet::Create(RMesh* pMesh, const ZActorActionRangeShot* pRangeShot, const rvector &pos, const rvector &dir, float fScale, ZObject* pOwner) 
{
	if (!pRangeShot) { _ASSERT(0); return; }

	ZWeapon::Create(pMesh);

	m_pRangeShot = pRangeShot;

	m_Position = pos;
	m_Dir = dir;
	m_Up = rvector(0,0,1);
	m_fScale = fScale;
	m_Velocity = dir * m_pRangeShot->GetSpeed();

	m_pVMesh->SetAnimation("play");

	m_fStartTime = ZGetGame()->GetTime();
	m_fLastAddTime = ZGetGame()->GetTime();

	m_pOwner=pOwner;
	m_uidOwner=pOwner->GetUID();
	m_nTeamID=pOwner->GetTeamID();

	m_fDamage = pRangeShot->GetDamage();

	/*//tododk 동적광원 일단 지원안함
	if( Z_VIDEO_DYNAMICLIGHT && !pSkill->GetDesc()->bOffDLight) {
		_ASSERT( m_SLSid == 0);
		m_SLSid = ZGetStencilLight()->AddLightSource( m_Position, 2.0f );
	}
	*/
}

bool ZWeaponNpcBullet::Update(float fElapsedTime)
{
	rvector oldPos = m_Position;

	#define WEAPON_LIFE			10.f		// 10초 뒤에 터짐

	if(ZGetGame()->GetTime() - m_fStartTime > WEAPON_LIFE ) {
		// 수류탄은 터지는 순간에 이펙트 추가.. 삭제...
		Explosion(NULL, rvector(0,1,0));

		/*//tododk 동적광원 일단 지원안함
		if(Z_VIDEO_DYNAMICLIGHT && m_SLSid ) {
			ZGetStencilLight()->DeleteLightSource( m_SLSid );
			m_SLSid = 0;
		}
		*/

		return false;
	}

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	rvector diff=m_Velocity*fElapsedTime;
	rvector dir=diff;
	Normalize(dir);

	ZPICKINFO zpi;

	ZObject* pOwnerObject = m_pOwner;
	ZObject* pPickObject = NULL;

	//todok collRadius를 사용하는 곳이 없다.. 피킹이 아니라 구체 충돌을 테스트해야 한다
	RBSPPICKINFO pInfo;
	bool bPicked=ZGetGame()->Pick(pOwnerObject, m_Position,dir,&zpi,dwPickPassFlag);
	bool bWorldPick = ZGetGame()->PickWorld(m_Position, dir, &pInfo);
	if (bWorldPick)
	{
		rvector pickpos = pInfo.PickPos;
		rvector pickdir;
		float fDist = Magnitude(diff);

		pickdir.x = pInfo.pInfo->plane.a;
		pickdir.y = pInfo.pInfo->plane.b;
		pickdir.z = pInfo.pInfo->plane.c;
		if (fabsf(Magnitude(pickpos - m_Position)) < fDist)
		{
			ExplosionOnMap();
			ZGetEffectManager()->AddWithScale("ef_damage07.elu", pickpos, pickdir, rvector(0.2, 0.2, 0.2));
			ZGetEffectManager()->AddLightFragment(pickpos, pickdir);
		}

	}
	if(bPicked)
	{
		rvector pickpos;
		rvector pickdir;	//todok 아직 쓰는데가 없다 정말 안쓴다면 없애버리자
		float fDist=Magnitude(diff);

		if(zpi.bBspPicked) {
			pickpos=zpi.bpi.PickPos;
			pickdir.x=zpi.bpi.pInfo->plane.a;
			pickdir.y=zpi.bpi.pInfo->plane.b;			
			pickdir.z=zpi.bpi.pInfo->plane.c;
			Normalize(pickdir);
		}	
		else if(zpi.pObject) {
			pPickObject = zpi.pObject;
			pickpos=zpi.info.vOut;
		}

		if ( fabsf(Magnitude(pickpos-m_Position)) < fDist )
		{
			if( pPickObject && !pPickObject->IsNPC() )	// 캐릭터에 충돌
			{
				Explosion(pPickObject, dir);
				ZGetEffectManager()->AddWithScale("ef_damage07.elu", pickpos,pickdir, pPickObject->GetUID(),rvector(0.2,0.2,0.2));
			}
			else
			{
				ExplosionOnMap();
			}

			/*//tododk 동적광원 일단 지원안함
			if(Z_VIDEO_DYNAMICLIGHT && m_SLSid )
			{
			ZGetStencilLight()->DeleteLightSource( m_SLSid );
			m_SLSid = 0;
			ZGetStencilLight()->AddLightSource( pickpos, 3.0f, 1300 );
			}
			*/
			return false;
		}
	}

	m_Position+=diff;

	rmatrix mat;
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

	m_pVMesh->SetScale(rvector(m_fScale,m_fScale,m_fScale));
	m_pVMesh->SetWorldMatrix(mat);

	float this_time = ZGetGame()->GetTime();

	if( this_time > m_fLastAddTime + 0.02f ) {

		rvector add = rvector(RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f);
		rvector pos = m_Position + 20.f*add;

		ZGetWorld()->GetFlags()->CheckSpearing( oldPos, pos	, ROCKET_SPEAR_EMBLEM_POWER );
		m_fLastAddTime = this_time;
	}

	/*//tododk 동적광원 일단 지원안함
	if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->SetLightSourcePosition( m_SLSid, m_Position	);
*/
	return true;
}

void ZWeaponNpcBullet::Render()
{
	ZWeapon::Render();


	// test 코드
#ifndef _PUBLISH
	if ((ZApplication::GetGameInterface()->GetScreenDebugger()->IsVisible()) && (ZIsLaunchDevelop()))
	{
		if (m_pRangeShot)
		{
			RDrawSphere(m_Position, m_pRangeShot->GetCollRadius(), 10);
		}
	}
#endif

}

#define MIN_DAMAGE	0.2f


void ZWeaponNpcBullet::Explosion(ZObject* pVictim, const rvector& dir)
{
	rvector v = m_Position-rvector(0,0,100.f);
	rvector pos = m_Position;

	// 이펙트 사운드 추가
	if (0!=strcmp("", m_pRangeShot->GetSound()))
		ZGetSoundEngine()->PlaySound( (char*)m_pRangeShot->GetSound(), m_Position);

	if (pVictim)
	{
		float fKnockBack = m_pRangeShot->GetKnockbackForce();
		pVictim->OnKnockback(rvector(dir), fKnockBack);
		//pVictim->AddVelocity(fKnockBack*7.f*(1.f/*fRange-fDist*/)*-dir);

		float fDamage = m_pRangeShot->GetDamage();
		float fPierceRatio = m_pRangeShot->GetPierce();
		pVictim->OnDamaged(m_pOwner, pos, ZD_MAGIC, MWT_PISTOL, fDamage, fPierceRatio);
	}
}

void ZWeaponNpcBullet::ExplosionOnMap()
{
	rvector v = m_Position-rvector(0,0,100.f);
	rvector pos = m_Position;

	// 이펙트 사운드 추가
	if (0!=strcmp("", m_pRangeShot->GetSound()))
		ZGetSoundEngine()->PlaySound( (char*)m_pRangeShot->GetSound(), m_Position);
	ZGetWorld()->GetFlags()->SetExplosion( v, EXPLOSION_EMBLEM_POWER );
}