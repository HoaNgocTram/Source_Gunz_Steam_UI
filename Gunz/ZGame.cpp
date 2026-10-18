#include "stdafx.h"

#include "ZGame.h"
#include <windows.h>
//#include <zmouse.h>

#include "MZFileSystem.h"
#include "RealSpace2.h"
#include "FileInfo.h"
#include "MDebug.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "ZConsole.h"
#include "MCommandLogFrame.h"
#include "ZInterface.h"
#include "ZGameInterface.h"
#include "ZApplication.h"
#include "ZCommandTable.h"
#include "ZPost.h"
#include "ZPostLocal.h"
#include "MStrEx.h"
#include "MMatchItem.h"
#include "ZEffectManager.h"
#include "ZEffectBillboard.h"
#include "Config.h"
#include "MProfiler.h"
#include "MMatchItem.h"
#include "ZScreenEffectManager.h"
#include "RParticleSystem.h"
#include "RDynamicLight.h"
#include "ZConfiguration.h"
#include "ZLoading.h"
#include "Physics.h"
#include "zeffectflashbang.h"
#include "ZInitialLoading.h"
#include "RealSoundEffect.h"
#include "RLenzFlare.h"
#include "ZWorldItem.h"
#include "ZMyInfo.h"
#include "ZNetCharacter.h"
#include "ZSecurity.h"
#include "ZStencilLight.h"
#include "ZMap.h"
#include "ZEffectStaticMesh.h"
#include "ZEffectAniMesh.h"
#include "ZGameAction.h"
#include "ZSkyBox.h"
#include "ZFile.h"
#include "ZObject.h"
#include "ZCharacter.h"
#include "ZActor.h"
#include "ZActorActionManager.h"
#include "ZMapDesc.h"
#include "MXml.h"
#include "ZGameClient.h"
#include "MUtil.h"
#include "RMeshMgr.h"
#include "RVisualMeshMgr.h"
#include "RMaterialList.h"
#include "RAnimationMgr.h"
#include "ZCamera.h"
#include "Mint4R2.h"
//#include "RParticleSystem.h"
#include "ZItemDesc.h"

//#include "MObjectCharacter.h"
#include "MMath.h"
#include "ZQuest.h"
#include "MMatchUtil.h"
#include "ZReplay.h"
#include "ZRuleBerserker.h"
#include "ZRuleDuelTournament.h"
#include "ZRuleQuestChallenge.h"
#include "ZApplication.h"
#include "ZGameConst.h"
#include "ZRuleDuel.h"
#include "ZMyCharacter.h"
#include "MMatchCRC32XORCache.h"
#include "MMatchObjCache.h"
#include "ZModule_HealOverTime.h"
#include "ZRuleDeathMatch.h"
#include "ZActorWithFSM.h"
#include "ZNavigationMesh.h"
#include "ZRuleSpy.h"
#include "RGMain.h"
#include "VoiceChat.h"
#include <random>
#include "RBspObject.h"
#include "CodePageConversion.h"
#include "ZFunctions.h"
#include "ZCompanion.h"
#include "ZRuleSkillMap.h"
#ifdef _PAINTMODE
#include "ZRulePaint.h"
#endif
#ifdef _ANTIHACK
#include "ZGuard.h"
#endif
#include <string>

#ifdef _MOVINGFLOOR 
#include "ZWorldObject.h"
#endif

// Custom: Disable NHN auth
//#ifdef LOCALE_NHNUSA
//#include "ZNHN_USA_Report.h"
//#endif

extern DWORD g_ShouldBanPlayer;

_USING_NAMESPACE_REALSPACE2

#define ENABLE_CHARACTER_COLLISION		// Ä³¸¯ÅÍ³¢¸® Ãæµ¹Ã¼Å©
#define ENABLE_ADJUST_MY_DATA			// ÅõÇ¥½Ã½ºÅÛ µ¿ÀÛ

/////////////////////////////////////////////////////////////////////
//Ã¤ÆÃ ¹®ÀÚ¿­ °Ë?EÄÚ?E...ÁÙ¹Ù²Þ ¹®ÀÚ¸¸ °Ë?E....
void CheckMsgAboutChat(char* msg)
{
	//¿©±â¼­ Ã¤ÆÃ?EE°Å¸£´ÂºÎºÐ.....
	int lenMsg = (int)strlen(msg);
	for (int i = 0; i < lenMsg; i++)
	{
		if (msg[i] == '\n' || msg[i] == '\r')
		{
			msg[i] = NULL;
			break;
		}
	}
}

#ifdef _PERFECT
bool IsPlayerHPFull(ZCharacter* pCharacter)
{
	float currentLife = pCharacter->GetHP();
	float maxLife = pCharacter->GetMaxHP();
	return (currentLife >= maxLife);
}
bool IsPlayerAPFull(ZCharacter* pCharacter)
{
	float currentLife = pCharacter->GetAP();
	float maxLife = pCharacter->GetMaxAP();
	return (currentLife >= maxLife);
}
#endif

#ifdef _RAINSYSTEM
struct RRainParticle : public RParticle, CMemPoolSm<RRainParticle>
{
	virtual bool Update(float fTimeElapsed);
};

bool RRainParticle::Update(float fTimeElapsed)
{
	RParticle::Update(fTimeElapsed);

	if (position.z <= -1000.0f) return false;
	return true;
}

class ZRainTownParticleSystem
{
private:
	RParticles* m_pParticlesRain[3];
	bool IsRainTownMap()
	{
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Snow", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Island", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Garden", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Battle Arena", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Factory", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Town", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Citadel", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Ruin", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Port", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Lost Shrine", 4)) return true;

		return false;
	}
public:
	void Update(float fDeltaTime)
	{
		if (!IsRainTownMap()) return;

#define RAIN_PARTICLE_COUNT_PER_SECOND		800

		int nRainParticleCountPerSec = RAIN_PARTICLE_COUNT_PER_SECOND;
		switch (ZGetConfiguration()->GetVideo()->nEffectLevel)
		{
		case Z_VIDEO_EFFECT_HIGH:	break;
		case Z_VIDEO_EFFECT_NORMAL:	nRainParticleCountPerSec = nRainParticleCountPerSec / 8; break;
		case Z_VIDEO_EFFECT_LOW:	nRainParticleCountPerSec = nRainParticleCountPerSec / 12; break;
		default: nRainParticleCountPerSec = 8; break;
		}

		int nCount = min(nRainParticleCountPerSec * fDeltaTime, 20);
		for (int i = 0; i < nCount; i++)
		{
			RParticle* pp = new RRainParticle();
			pp->ftime = 0;

			// RainsDrops
			/*pp->position = rvector(RandomNumber(-8000.0f, 8000.0f), RandomNumber(-8000.0f, 8000.0f), 1500.0f);
			pp->velocity = rvector(RandomNumber(40.0f, 40.0f), RandomNumber(40.0f, 40.0f), RandomNumber(150.0f, 250.0f));
			pp->accel = rvector(-0, -0, -200.f);*/

			// Night Stars
			pp->position = rvector(RandomNumber(-8000.0f, 8000.0f), RandomNumber(-8000.0f, 8000.0f), 1500.0f);
			pp->velocity = rvector(RandomNumber(0.0f, 0.0f), RandomNumber(0.0f, 0.0f), RandomNumber(0.0f, 0.0f));
			pp->accel = rvector(-0, -0, -200.f);

			int particle_index = RandomNumber(0, 2);
			if (m_pParticlesRain[particle_index]) m_pParticlesRain[particle_index]->push_back(pp);
		}
	}
	void Create()
	{
		if (!IsRainTownMap()) return;

		for (int i = 0; i < 3; i++)
		{
			m_pParticlesRain[i] = NULL;
		}

		m_pParticlesRain[0] = RGetParticleSystem()->AddParticles("sfx/raindrops.bmp", 25.0f);
		m_pParticlesRain[1] = RGetParticleSystem()->AddParticles("sfx/raindrops.bmp", 20.0f);
		m_pParticlesRain[2] = RGetParticleSystem()->AddParticles("sfx/raindrops.bmp", 15.0f);
	}
	void Destroy()
	{
		if (!IsRainTownMap()) return;

		if (m_pParticlesRain[0])
		{
			m_pParticlesRain[0]->Clear();
			m_pParticlesRain[0] = NULL;
		}
		if (m_pParticlesRain[1])
		{
			m_pParticlesRain[1]->Clear();
			m_pParticlesRain[1] = NULL;
		}
		if (m_pParticlesRain[2])
		{
			m_pParticlesRain[2]->Clear();
			m_pParticlesRain[2] = NULL;
		}
	}
};

static ZRainTownParticleSystem g_RainTownParticleSystem;
#else
struct RSnowParticle : public RParticle, CMemPoolSm<RSnowParticle>
{
	virtual bool Update(float fTimeElapsed);
};

bool RSnowParticle::Update(float fTimeElapsed)
{
	RParticle::Update(fTimeElapsed);

	if (position.z <= -1000.0f) return false;
	return true;
}

class ZSnowTownParticleSystem
{
private:
	RParticles* m_pParticles[3];
	bool IsSnowTownMap()
	{
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "snow", 4)) return true;
		return false;

		if (ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->bXmas == true)
		{
			if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Island", 4)) return true;
			if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Garden", 4)) return true;
			if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Battle Arena", 4)) return true;
			if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Factory", 4)) return true;
			if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Town", 4)) return true;
			if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "High_Haven", 4)) return true;
			if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Citadel", 4)) return true;
			if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Ruin", 4)) return true;
			if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Port", 4)) return true;
			if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Castle", 4)) return true;
			if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Lost Shrine", 4)) return true;
			if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Stairway", 4)) return true;
			return false;
		}
	}
public:
	void Update(float fDeltaTime)
	{
		if (!IsSnowTownMap()) return;

#define SNOW_PARTICLE_COUNT_PER_SECOND		400		// ?E?ÃÊ?E300°³?E¿ø·¡ ÇÁ·¹ÀÓ?E14°³¿´À½.

		int nSnowParticleCountPerSec = SNOW_PARTICLE_COUNT_PER_SECOND;
		switch (ZGetConfiguration()->GetVideo()->nEffectLevel)
		{
		case Z_VIDEO_EFFECT_HIGH:	break;
		case Z_VIDEO_EFFECT_NORMAL:	nSnowParticleCountPerSec = nSnowParticleCountPerSec / 4; break;
		case Z_VIDEO_EFFECT_LOW:	nSnowParticleCountPerSec = nSnowParticleCountPerSec / 8; break;
		default: nSnowParticleCountPerSec = 0; break;
		}

		int nCount = min(nSnowParticleCountPerSec * fDeltaTime, 20);	// ÇÑ¹ø¿¡ 20°³ ÀÌ»óÀº ¾È³ª¿Àµµ·ÏÇÑ´Ù
		for (int i = 0; i < nCount; i++)
		{
			RParticle* pp = new RSnowParticle();
			pp->ftime = 0;
			pp->position = rvector(RandomNumber(-8000.0f, 8000.0f), RandomNumber(-8000.0f, 8000.0f), 1500.0f);
			pp->velocity = rvector(RandomNumber(-40.0f, 40.0f), RandomNumber(-40.0f, 40.0f), RandomNumber(-150.0f, -250.0f));
			pp->accel = rvector(0, 0, -5.f);

			int particle_index = RandomNumber(0, 2);
			if (m_pParticles[particle_index]) m_pParticles[particle_index]->push_back(pp);
		}
	}
	void Create()
	{
		if (!IsSnowTownMap()) return;

		for (int i = 0; i < 3; i++)
		{
			m_pParticles[i] = NULL;
		}

		// ÆÄÆ¼Å¬ »ý¼º
		m_pParticles[0] = RGetParticleSystem()->AddParticles("sfx/water_splash.bmp", 25.0f);
		m_pParticles[1] = RGetParticleSystem()->AddParticles("sfx/raindrops.bmp", 10.0f);
		m_pParticles[2] = RGetParticleSystem()->AddParticles("sfx/water_splash.bmp", 5.0f);
	}
	void Destroy()
	{
		if (!IsSnowTownMap()) return;

		// Custom: Checked for NULL pointer in m_pParticles
		if (m_pParticles[0])
		{
			m_pParticles[0]->Clear();
			m_pParticles[0] = NULL;
		}
		if (m_pParticles[1])
		{
			m_pParticles[1]->Clear();
			m_pParticles[1] = NULL;
		}
		if (m_pParticles[2])
		{
			m_pParticles[2]->Clear();
			m_pParticles[2] = NULL;
		}
	}
};

static ZSnowTownParticleSystem g_SnowTownParticleSystem;
#endif

ZGame* g_pGame = NULL;

float	g_fFOV = DEFAULT_FOV;
float	g_fFarZ = DEFAULT_FAR_Z;

//Custom: FOV Camera Add By Desperate
float GetFOV()
{
	return g_fFOV;
}

void SetFOV(float x)
{
	g_fFOV = ZGetConfiguration()->GetCamFix() ? FixedFOV(x) : x;
}
int ServerLatency(int nPing) {
	if (nPing < 50)
		return nPing;
	else if (nPing >= 50 && nPing <= 100)
		return nPing -= 25;
	else if (nPing >= 100 && nPing <= 150)
		return nPing -= 50;
	else if (nPing >= 150 && nPing <= 200)
		return nPing -= 70;
	else if (nPing >= 200 && nPing <= 250)
		return nPing -= 90;
}

//RParticleSystem* g_ParticleSystem = 0;
extern sCharacterLight g_CharLightList[NUM_LIGHT_TYPE];

MUID tempUID(0, 0);		// ·ÎÄÃ Å×½ºÆ®?E
MUID g_MyChrUID(0, 0);

#define IsKeyDown(key) ((GetAsyncKeyState(key) & 0x8000)!=0)

void TestCreateEffect(int nEffIndex)
{
	float fDist = RandomNumber(0.0f, 100.0f);
	rvector vTar = rvector(RandomNumber(0.0f, 100.0f), RandomNumber(0.0f, 100.0f), RandomNumber(0.0f, 100.0f));
	rvector vPos = ZGetGame()->m_pMyCharacter->GetPosition();
	vPos.x += RandomNumber(0.0f, 100.0f);
	vPos.y += RandomNumber(0.0f, 100.0f);
	vPos.z += RandomNumber(0.0f, 100.0f);

	rvector vTarNormal = -RealSpace2::RCameraDirection;

	vTarNormal = rvector(RandomNumber(0.0f, 100.0f), RandomNumber(0.0f, 100.0f), RandomNumber(0.0f, 100.0f));

	ZCharacter* pCharacter = ZGetGame()->m_pMyCharacter;
	ZEffectManager* pEM = ZGetEffectManager();

	ZCharacter* pOwnerCharacter = NULL;
	ZItem* pItem = pOwnerCharacter->GetItems()->GetItem(MMCIP_MELEE);
	MMatchItemDesc* pDesc = pItem->GetDesc();

	switch (nEffIndex)
	{
	case 0:
		pEM->AddReBirthEffect(vPos);
		break;
	case 1:
		pEM->AddLandingEffect(vPos, vTarNormal);
		break;
	case 2:
		pEM->AddGrenadeEffect(vPos, vTarNormal);
		break;
	case 3:
		pEM->AddRocketEffect(vPos, vTarNormal);
		break;
	case 4:
		pEM->AddRocketSmokeEffect(vPos);
		break;
	case 5:
		pEM->AddSwordDefenceEffect(vPos, -vTarNormal);
		break;
	case 6:
		//Custom: Effect Massive By Desperate
		if (pDesc)
		{
			switch (pDesc->m_nID)
			{
			case 515000:
				pEM->AddEffectWaveFine(pCharacter->GetUID(), vPos, pCharacter->GetDirection());
				break;
			case 515001:
				pEM->AddEffectWaveFireBall(pCharacter->GetUID(), vPos, pCharacter->GetDirection());
				break;
			case 515002:
				pEM->AddEffectWaveIceMissile(pCharacter->GetUID(), vPos, pCharacter->GetDirection());
				break;
			case 515003:
				pEM->AddEffectWaveMagic(pCharacter->GetUID(), vPos, pCharacter->GetDirection());
				break;
			default:
#ifdef _MACOLOR
				//pEM->AddSwordWaveEffect(pCharacter->GetUID(), vPos, pCharacter->GetDirection());
#else
				pEM->AddSwordWaveEffect(vPos, 200, pCharacter);
#endif
				break;
			}
		}


		break;
	case 7:
		pEM->AddSwordUppercutDamageEffect(vPos, pCharacter->GetUID());
		break;
	case 8:
		pEM->AddBulletMark(vPos, vTarNormal);
		break;
	case 9:
		pEM->AddShotgunEffect(vPos, vPos, vTar, pCharacter);
		break;
	case 10:
		pEM->AddBloodEffect(vPos, vTarNormal);
		break;
#ifdef _PAINTMODE
	case 11:
		pEM->AddPaintEffect(vPos, vTarNormal);
		break;
#endif
	case 12:
		for (int i = 0; i < SEM_End; i++)
			pEM->AddSlashEffect(vPos, vTarNormal, i);
		break;
	case 13:
		pEM->AddSlashEffectWall(vPos, vTarNormal, 0);
		break;
	case 14:
		pEM->AddLightFragment(vPos, vTarNormal);
		break;
	case 15:
		//pEM->AddDashEffect(vPos, vTarNormal, pCharacter);
		pEM->AddDashEffect(vPos, vTarNormal, pCharacter, NULL);
		break;
	case 16:
		pEM->AddSmokeEffect(vPos);
		break;
	case 17:
		pEM->AddSmokeGrenadeEffect(vPos);
		break;
	case 18:
		pEM->AddGrenadeSmokeEffect(vPos, 1.0f, 0.5f, 123);
		break;
	case 19:
		pEM->AddWaterSplashEffect(vPos, vPos);
		break;
	case 20:
		pEM->AddWorldItemEatenEffect(vPos);
		break;
	case 21:
		pEM->AddCharacterIcon(pCharacter, 0);
		break;
	case 22:
		pEM->AddCommanderIcon(pCharacter, 0);
		break;
	case 23:
		pEM->AddChatIcon(pCharacter);
		break;
#ifdef _ICONCHAT
	case 24:
		pEM->AddChatVoiceIcon(pCharacter);
		break;
#endif
	case 25:
		pEM->AddLostConIcon(pCharacter);
		break;
	case 26:
		pEM->AddChargingEffect(pCharacter, pCharacter->m_szSlashChargingName);
		break;
	case 27:
		pEM->AddChargedEffect(pCharacter, pCharacter->m_szSlashChargedName);
		break;
	case 28:
		pEM->AddTrackFire(vPos);
		pEM->AddTrackFire(vPos);
		pEM->AddTrackFire(vPos);
		pEM->AddTrackFire(vPos);
		break;
	case 29:

		ZEffectWeaponEnchant * pEWE = pEM->GetWeaponEnchant(ZC_ENCHANT_FIRE);

		if (pEWE) {
			//Ç¥ÁØ »çÀÌÁûÐÂ Ä«Å¸³ª... 100 Á¤µµ..
			float fSIze = 105.f / 100.f;
			rvector vScale = rvector(0.6f * fSIze, 0.6f * fSIze, 0.9f * fSIze);// ¹«±âÀÇ Å©±â¿¡ µû¶ó¼­..
			pEWE->SetUid(pCharacter->GetUID());
			pEWE->SetAlignType(1);
			pEWE->SetScale(vScale);
			pEWE->Draw(timeGetTime());
		}

		break;
	}
}
float CalcActualDamage(ZObject* pAttacker, ZObject* pVictim, float fDamage, MMatchWeaponType weaponType, ZItem* pItem)
{
	if (!pItem)
		return 0.f;
	if (!pItem->GetDesc())
		return 0.f;

	float fOutDamage = pItem->GetDesc()->m_nDamage.Ref();
	if (!fOutDamage)
		return 0.f;

	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_DMG) &&
		!ZGetGame()->GetMatch()->IsQuestDrived() &&
		!ZGetGame()->GetMatch()->IsQuestChallengue() &&
		!ZGetGame()->GetMatch()->IsRuleSpy() &&
		!ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_INFECTED)
	{
		int nMultiplier = ZGetGameClient()->GetMatchStageSetting()->GetModifierValue(MMOD_DMG);

		if (nMultiplier >= 2 && nMultiplier <= 5)
			fOutDamage *= (float)nMultiplier;
	}

	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_BERSERKER)
	{
		ZRuleBerserker* pRule = (ZRuleBerserker*)ZGetGame()->GetMatch()->GetRule();
		if ((pAttacker) && (pAttacker != pVictim) && (pAttacker->GetUID() == pRule->GetBerserkerUID()))
		{
			return fDamage * BERSERKER_DAMAGE_RATIO;
		}
	}
	if (ZGetGame()->GetMatch()->IsTurboMode())
	{
		return fDamage * 3.0f;
	}

	return fDamage;
}

void TestCreateEffects()
{
	int nCount = 100;

	for (int i = 0; i < nCount; i++)
	{
		int nEffIndex = RandomNumber(25, 28);
		TestCreateEffect(nEffIndex);
	}
}

bool CompareZCharFloat(pair<ZCharacter*, float> i, pair<ZCharacter*, float> j) { return (i.second < j.second); }

ZGame::ZGame()
{
	m_bShowWireframe = false;
	m_pMyCharacter = NULL;
	m_bReserveObserver = false;

	//	memset(m_nLastTime, 0, sizeof(DWORD) * ZLASTTIME_MAX);
	for (int i = 0; i < ZLASTTIME_MAX; i++)
	{
		m_nLastTime[i] = timeGetTime();
	}

	//	m_fTime = 0.f;
	m_fTime.Set_MakeCrc(0.0f);

	m_nReadyState = ZGAME_READYSTATE_INIT;
#ifdef _RAINSYSTEM
	m_pParticlesRain = NULL;
#else
	m_pParticles = NULL;
#endif
	m_render_poly_cnt = 0;
	m_nReservedObserverTime = 0;
	m_nSpawnTime = 0;
	m_t = 0;
	m_DiceRoll = 0;
	m_dwLastInputTime = timeGetTime();

	m_bRecording = false;
	//	m_pReplayFile = NULL;
	m_pReplayFile = NULL;

	m_bReplaying.Set_MakeCrc(false);
	m_bShowReplayInfo = true;
	m_bSpawnRequested = false;
	m_ReplayLogTime = 0;
	ZeroMemory(m_Replay_UseItem, sizeof(m_Replay_UseItem));

	m_pGameAction = new ZGameAction;
	DmgCounter = new DamageCounterMap();

	CancelSuicide();
}

ZGame::~ZGame()
{
	m_pMyCharacter = NULL;
	delete m_pGameAction;
#ifdef _RAINSYSTEM
	g_RainTownParticleSystem.Destroy();
	RRainParticle::Release();
#else
	g_SnowTownParticleSystem.Destroy();
	RSnowParticle::Release();
#endif
}

bool ZGame::Create(MZFileSystem* pfs, ZLoadingProgress* pLoading)
{
	// Å¬·£?E¡¼­´Â stagestate °¡ ¾ø¾ûØ­ CastStageBridgePeer ¸¦ ¾ÈÇØ¼­ ¿©±â¼­È£?E
	// Custom: Updated string
	mlog("CastStageBridgePeer call in Zgame::Create\n");
	//mlog("CastStageBridgePeer È£?Ein Zgame::Create\n");
	ZGetGameClient()->CastStageBridgePeer(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());

	mlog("game create begin , type = %d\n", ZGetGameClient()->GetMatchStageSetting()->GetGameType());

	SetReadyState(ZGAME_READYSTATE_INIT);	// Sync ¸ÂÀ»¶§±û?EGame Loop ÁøÀÔ¾Êµµ·Ï

#ifdef _QUEST
	{
		if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
			ZGetQuest()->OnGameCreate();
	}
#endif

	//	m_ItemDescManager.Create(FILENAME_ZITEM_DESC);	// ³ªÁß¿¡ ³Ö¾ûÚß?E

		// world¸¦ ¼¼ÆÃ
	if (ZGetApplication()->GetLaunchMode() != ZApplication::ZLAUNCH_MODE_STANDALONE_AI &&
		ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) && ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE) {
		for (int i = 0; i < ZGetQuest()->GetGameInfo()->GetMapSectorCount(); i++)
		{
			MQuestMapSectorInfo* pSecInfo = ZGetQuest()->GetSectorInfo(ZGetQuest()->GetGameInfo()->GetMapSectorID(i));
			if (pSecInfo == NULL)
			{
				char strBuf[256];
				sprintf(strBuf, "[MQuestMapSectorInfo] m_MapSectorVector[index]:%d, GetMapSectorID:%d\n", i, ZGetQuest()->GetGameInfo()->GetMapSectorID(i));
				ASSERT(0 && strBuf);
			}
			ZGetWorldManager()->AddWorld(pSecInfo->szTitle);
#ifdef _DEBUG
			mlog("map(%s)\n", pSecInfo ? pSecInfo->szTitle : "null");
#endif
		}
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_QUEST_CHALLENGE) {
		//toodk quest ÀÌ°Ç ÀÓ½Ã ¸Ê·ÎµùÀÌ´Ù.. ¸Ê ¸®¼Ò½º¸¦ ¾ûÐÀ Æú´õ¿¡ µÑ °ÍÀÎ?E ·Î?E¹æ½ÄÀº ÀÌ·¸°Ô µû·Î if¹®À» ³ÖÀ» °Ç?E
		// ¾Æ´Ï?E±âÁ¸ ±¸Á¶¿¡ ¸Â°Ô ?E?³ÖÀ» °ÍÀÎ?E°áÁ¤ÇØ¾ß¸¸ ÇÑ´Ù
		//ZGetWorldManager()->AddWorld("Mansion_room2");

		if (!ZRuleQuestChallenge::LoadScenarioMap(ZGetGameClient()->GetMatchStageSetting()->GetMapName())) // ½Ã³ª¸®¿À ÀÌ¸§À» ³Ñ°Ü¼­ Æ÷ÇÔµÈ ¸ÊÀ» ·Îµù½ÃÅ´
			return false;
	}
	else {
		ZGetWorldManager()->AddWorld(ZGetGameClient()->GetMatchStageSetting()->GetMapName());
	}

	if (!ZGetWorldManager()->LoadAll(pLoading))
		return false;

	ZGetWorldManager()->SetCurrent(0);

	if (ZGetApplication()->GetLaunchMode() != ZApplication::ZLAUNCH_MODE_STANDALONE_AI &&
		ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) && ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		if (ZGetQuest() && ZGetQuest()->GetGameInfo())
			ZGetWorldManager()->SetCurrent((ZGetQuest()->GetGameInfo()->GetCurrSectorIndex()));
	}

	//RSetCamera(rvector(-10.f,0.f,150.f),rvector(0.f,0.f,0.f),rvector(0.f,1.f,0.f));
	//RSetProjection(DEFAULT_FOV,DEFAULT_NEAR_Z,DEFAULT_FAR_Z);

	//char szMapFileName[256];
	//char szMapPath[64] = "";
	//ZGetCurrMapPath(szMapPath);

	//sprintf(szMapFileName, "%s%s/%s.rs",
	//	szMapPath,
	//	ZGetGameClient()->GetMatchStageSetting()->GetMapName(),
	//	ZGetGameClient()->GetMatchStageSetting()->GetMapName());

	//if(!strlen(szMapFileName))
	//	return false;

	//mlog("ZGame::Create() :: ReloadAllAnimation Begin \n");
	ZGetMeshMgr()->ReloadAllAnimation();// ÀÐ?E¾ÊÀº ¿¡´Ï¸ÞÀÌ¼ÇÀÌ ÀÖ´Ù?E·Î?E
	mlog("Reload all animation end \n");

	//ZGetInitialLoading()->SetPercentage( 90.f );
//	ZGetInitialLoading()->SetPercentage( 70.f );
//	ZGetInitialLoading()->Draw( MODE_DEFAULT, 0 , true );

	// ³­ÀÔÀÏ¶§¿¡´Â PeerList¸¦ ¿äÃ»ÇÑ´Ù
	if (ZGetGameClient()->IsForcedEntry())
	{
		ZPostRequestPeerList(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
		//		ZPostRequestGameInfo(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
	}

	//if(!GetWorld()->GetBsp()->Open(szMapFileName))
	//{
	//	MLog("error while loading %s \n",szMapFileName);
	//	return false;
	//}

	//mlog("ZGame::Create() GetWorld()->GetBsp()->Open %s \n",szMapFileName);

	//GetWorld()->GetBsp()->OptimizeBoundingBox();

	//ZGetInitialLoading()->SetPercentage( 100.f );

	//GetMapDesc()->Open(&m_bsp);

	//g_fFOV = DEFAULT_FOV;

	SetFOV(ToRadian(ZGetConfiguration()->GetFOV())); //Custom: FOV Camera Add By Desperate

	rvector dir = GetMapDesc()->GetWaitCamDir();
	rvector pos = GetMapDesc()->GetWaitCamPos();
	rvector up(0, 0, 1);
	RSetCamera(pos, pos + dir, up);

	int nModelID = -1;

	m_Match.Create();

	D3DMATERIAL9 mtrl;
	ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));

	mtrl.Diffuse.r = 1.0f;
	mtrl.Diffuse.g = 1.0f;
	mtrl.Diffuse.b = 1.0f;
	mtrl.Diffuse.a = 1.0f;

	mtrl.Ambient.r = 1.0f;
	mtrl.Ambient.g = 1.0f;
	mtrl.Ambient.b = 1.0f;
	mtrl.Ambient.a = 1.0f;

	RGetDevice()->SetMaterial(&mtrl);

	//	m_fTime=0.f;
	m_fTime.Set_CheckCrc(0.0f);
	m_bReserveObserver = false;

#ifdef _BIRDSOUND
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_BATTLE);
	ZApplication::GetSoundEngine()->PlayMusic();
#else
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_BATTLE, pfs);
	ZApplication::GetSoundEngine()->PlayMusic();
#endif

	m_CharacterManager.Clear();
	m_ObjectManager.Clear();

	// mlog("ZGame::Create() m_CharacterManager.Clear done \n");

	m_pMyCharacter = (ZMyCharacter*)m_CharacterManager.Add(ZGetGameClient()->GetPlayerUID(), rvector(0.0f, 0.0f, 0.0f), true);

#ifdef _AFKSYSTEM
	//m_pMyCharacter->m_nLastKeyTime = timeGetTime();
#endif
	{
		g_CharLightList[GUN].fLife = 300;
		g_CharLightList[GUN].fRange = 100;
		g_CharLightList[GUN].iType = GUN;
		g_CharLightList[GUN].vLightColor.x = 5.0f;
		g_CharLightList[GUN].vLightColor.y = 1.0f;
		g_CharLightList[GUN].vLightColor.z = 1.0f;

		g_CharLightList[SHOTGUN].fLife = 1000;
		g_CharLightList[SHOTGUN].fRange = 150;
		g_CharLightList[SHOTGUN].iType = SHOTGUN;
		g_CharLightList[SHOTGUN].vLightColor.x = 6.0f;
		g_CharLightList[SHOTGUN].vLightColor.y = 1.3f;
		g_CharLightList[SHOTGUN].vLightColor.z = 1.3f;

		g_CharLightList[CANNON].fLife = 1300;
		g_CharLightList[CANNON].fRange = 200;
		g_CharLightList[CANNON].iType = CANNON;
		g_CharLightList[CANNON].vLightColor.x = 7.0f;
		g_CharLightList[CANNON].vLightColor.y = 1.3f;
		g_CharLightList[CANNON].vLightColor.z = 1.3f;
	}

	ZGetFlashBangEffect()->SetBuffer();
	ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();

#ifdef _BIRDSOUND

#else
	ZGetSoundEngine()->SetEffectVolume(Z_AUDIO_EFFECT_VOLUME);
	ZGetSoundEngine()->SetMusicVolume(Z_AUDIO_BGM_VOLUME);
#endif

	// Net init
	ZApplication::ResetTimer();
	m_GameTimer.Reset();
	ZSetupDataChecker_Game(&m_DataChecker);

	ZGetInitialLoading()->SetPercentage(100.f);
	ZGetInitialLoading()->Draw(MODE_DEFAULT, 0, true);

#ifdef _BIRDSOUND

#else
	list<AmbSndInfo*> aslist = GetWorld()->GetBsp()->GetAmbSndList();
	for (list<AmbSndInfo*>::iterator iter = aslist.begin(); iter != aslist.end(); ++iter)
	{
		AmbSndInfo* pAS = *iter;
		if (pAS->itype & AS_AABB)
			ZGetSoundEngine()->SetAmbientSoundBox(pAS->szSoundName, pAS->min, pAS->max, (pAS->itype & AS_2D) ? true : false);
		else if (pAS->itype & AS_SPHERE)
			ZGetSoundEngine()->SetAmbientSoundSphere(pAS->szSoundName, pAS->center, pAS->radius, (pAS->itype & AS_2D) ? true : false);
	}
#endif

	// ·Î?E´Ù ‰ç¾ûÛE. ¶ó?E´Ù¸¥ »ç¶÷µéÇÑÅ× ¾Ë¸°´Ù.
	MEMBER_SET_CHECKCRC(m_pMyCharacter->GetStatus(), nLoadingPercent, 100);
	ZPostLoadingComplete(ZGetGameClient()->GetPlayerUID(), 100);

	// °ÔÀÓ¿¡ µé¾ûÌ¬´Ù?E¼­¾Ë¸²
	ZPostStageEnterBattle(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());

	char tmpbuf[128];
	_strtime(tmpbuf);

	// µµ¿ò¸» È­¸é»ý¼º..

	mlog("game created ( %s )\n", tmpbuf);

	ZGetGameInterface()->GetCamera()->SetLookMode(ZCAMERA_DEFAULT);

#ifdef _RAINSYSTEM
	g_RainTownParticleSystem.Create();
#else
	g_SnowTownParticleSystem.Create();
#endif

	return true;
}

void ZGame::Destroy()
{
	//jintriple3 ¿©?E³»°¡ ¸Ö Ãß°¡ÇÑ°Å?E?¤Ñ¤Ì
	m_DataChecker.Clear();

#ifdef _RAINSYSTEM
	g_RainTownParticleSystem.Destroy();
#else
	g_SnowTownParticleSystem.Destroy();
#endif

	SetClearColor(0);

	mlog("game destroy begin\n");

	ZGetGameClient()->AgentDisconnect();

	ZApplication::GetSoundEngine()->StopMusic();
	ZApplication::GetSoundEngine()->CloseMusic();

	mlog("Destroy sound engine()\n");

#ifdef _QUEST
	{
		ZGetQuest()->OnGameDestroy();
	}
#endif

	m_Match.Destroy();

	mlog("game destroy match destroy \n");

	if (ZGetGameClient()->IsForcedEntry())
	{
		ZGetGameClient()->ReleaseForcedEntry();

		ZGetGameInterface()->SerializeStageInterface();
	}

	ZPostStageLeaveBattle(ZGetGameClient()->GetPlayerUID(), ZGetGameInterface()->GetIsGameFinishLeaveBattle());//, ZGetGameClient()->GetStageUID());

	//SAFE_DELETE( g_ParticleSystem );

	ReleaseFlashBangEffect();

	// mlog("ZGame::Destroy SAFE_DELETE(m_pSkyBox) \n");

	RGetLenzFlare()->Clear();

	// °ÔÀÓÀÌ ³¡³¯ ¶§¸¶´Ù ¸Þ¸ð¸® ¸±¸®?EÇØÁÜ...
	//ReleaseMemPool(RealSoundEffectFx);
	//ReleaseMemPool(RealSoundEffect);
	//ReleaseMemPool(RealSoundEffectPlay);

//	int e_size = m_EffectManager.m_Effects[0].size();
//	e_size += m_EffectManager.m_Effects[1].size();
//	e_size += m_EffectManager.m_Effects[2].size();
//	int w_size = m_WeaponManager.m_list.size();
//	mlog("ZGame e_size : w_size : %d %d\n",e_size,w_size);

//	ZGetEffectManager()->Clear();
	m_WeaponManager.Clear();
#ifdef _WORLD_ITEM_
	ZGetWorldItemManager()->Clear();
#endif

#ifdef _BIRDSOUND

#else
	ZGetSoundEngine()->ClearAmbientSound();
#endif

	m_ObserverCommandList.Destroy();
	m_ReplayCommandList.Destroy();
	m_DelayedCommandList.Destroy();

	ZGetEffectManager()->Clear();
	ZGetScreenEffectManager()->Clear();

	ZGetWorldManager()->Clear();

	// Custom: Delete character cache
	ZGetCharacterManager()->Clear();

	char tmpbuf[128];
	_strtime(tmpbuf);

	mlog("game destroyed ( %s )\n", tmpbuf);
}

bool ZGame::CreateMyCharacter(MTD_CharInfo* pCharInfo/*, MTD_CharBuffInfo* pCharBuffInfo*/)
{
	if (!m_pMyCharacter) return false;

	m_pMyCharacter->Create(pCharInfo/*, pCharBuffInfo*/);
	m_pMyCharacter->SetVisible(true);

	mlog("Create character : Name=%s Level=%d \n", pCharInfo->szName, pCharInfo->nLevel);
	return true;
}

bool ZGame::CheckGameReady()
{
	if (GetReadyState() == ZGAME_READYSTATE_RUN) {
		return true;
	}
	else if (GetReadyState() == ZGAME_READYSTATE_INIT) {
		SetReadyState(ZGAME_READYSTATE_WAITSYNC);
		// ½Ã°£ ½ÌÅ© ¿äÃ»
		ZPostRequestTimeSync(GetTickTime());
		return false;
	}
	else if (GetReadyState() == ZGAME_READYSTATE_WAITSYNC) {
		return false;
	}
	return false;
}

void ZGame::OnGameResponseTimeSync(unsigned int nLocalTimeStamp, unsigned int nGlobalTimeSync)
{
	ZGameTimer* pTimer = GetGameTimer();
	int nCurrentTick = pTimer->GetGlobalTick();
	int nDelay = (nCurrentTick - nLocalTimeStamp) / 2;
	int nOffset = (int)nGlobalTimeSync - (int)nCurrentTick + nDelay;

	pTimer->SetGlobalOffset(nOffset);

	SetReadyState(ZGAME_READYSTATE_RUN);
}
#ifdef _POSTABINFO
void ZGame::PostTabListInfo()
{
	DWORD nNowTime = GetTickTime();

	if (m_pMyCharacter->GetInitialized() == false) return;

	if ((nNowTime - m_nLastTime[ZLASTTIME_TABLIST]) >= 100)
	{
		m_nLastTime[ZLASTTIME_TABLIST] = nNowTime;

		MCommand* pCmd = ZNewCmd(MC_PEER_HPAP_TAB);
		pCmd->AddParameter(new MCmdParamInt((int)this->m_pMyCharacter->GetHP()));
		pCmd->AddParameter(new MCmdParamInt((int)this->m_pMyCharacter->GetAP()));
		ZPostCommand(pCmd);
	}

}
#endif
void ZGame::Update(float fElapsed)
{
	if (CheckGameReady() == false) {
		OnCameraUpdate(fElapsed);
		return;
	}
#ifdef _ASSITEN
	if (ZGetCamera()->m_bAutoAiming)
		AutoAiming();
#endif

	GetWorld()->Update(fElapsed);

	ZGetEffectManager()->Update(fElapsed);
	ZGetScreenEffectManager()->UpdateEffects();

	m_GameTimer.UpdateTick(timeGetTime());
	m_fTime.Set_CheckCrc(m_fTime.Ref() + fElapsed);
	m_fTime.ShiftHeapPos();
	m_bReplaying.ShiftHeapPos_CheckCrc();
	m_ObjectManager.Update(fElapsed);

	if (m_pMyCharacter && !m_bReplaying.Ref())
	{
		if (ZGetGame() && ZGetGame()->m_pMyCharacter && !ZGetGame()->IsReplay())
		{
			
			ZGetGame()->PostBasicInfo();
			if (ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT)
				ZGetGame()->PostHPAPInfo();
			else
				ZGetGame()->PostDuelTournamentHPAPInfo(); 

			ZGetGame()->PostPeerPingInfo();
			//ZGetGame()->PostSyncReport();
#ifdef _POSTABINFO
			ZGetGame()->PostTabListInfo();
#endif
		}
	}
	CheckMyCharDead(fElapsed);
	if (m_bReserveObserver)
	{
		OnReserveObserver();
	}

	UpdateCombo();

#ifdef _RAINSYSTEM
	g_RainTownParticleSystem.Update(fElapsed);
#else
	g_SnowTownParticleSystem.Update(fElapsed);
#endif

#ifdef _WORLD_ITEM_
	ZGetWorldItemManager()->update();
#endif
	m_Match.Update(fElapsed);

	if (m_bReplaying.Ref())
		OnReplayRun();
	// ¸®ÇÃ·¹ÀÌ¶§¿¡´Â ¹«Á¶°Ç ¿ÉÀú?EÃ³¸®¸¦ ÇØ¾ß ÇÒ?E?
	if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode() || m_bReplaying.Ref())
		OnObserverRun();

	ProcessDelayedCommand();

	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_ROLLTHEDICE))
	{
		if (ZGetGame()->GetRolledDice() == 1)
			ZGetGameInterface()->GetCamera()->m_fDist = CAMERA_DIST_MAX;
		else
		{
			if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_FPS))
			{
				ZGetGameInterface()->GetCamera()->m_fDist = 30.f;
			}
			else
				ZGetGameInterface()->GetCamera()->m_fDist = CAMERA_DEFAULT_DISTANCE;
		}
	}
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bFirstPerson)
	{
		ZGetGameInterface()->GetCamera()->m_fDist = 30.f;
	}
	if ((ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_ROLLTHEDICE) && ZGetGame()->GetRolledDice() == 9))
	{
		if (m_pMyCharacter->GetDistToFloor() <= 3.0f) m_pMyCharacter->SetVelocity(m_pMyCharacter->GetVelocity().x, m_pMyCharacter->GetVelocity().y, 800.0f);
	}

	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_ROLLTHEDICE) && GetRolledDice() == 11)
		m_bShowWireframe = true;
	else
		m_bShowWireframe = false;

	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_WIREFRAME))
		m_bShowWireframe = true;

	if (ZGetGame()->GetMatch()->IsTraining() && GetRGMain().TrainingSettings.WireFrame)
		m_bShowWireframe = true;


#ifdef _QUEST
	{
		ZGetQuest()->OnGameUpdate(fElapsed);
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	//	TODO : »óÈ²¿¡ µû?E¾÷µ¥ÀÌÆ® ¾ÈÇØÁàµµ µÉ ¶§¸¦ ±¸º°ÇØ ³¾?EÀÖÀ»?E
	//////////////////////////////////////////////////////////////////////////
	//g_ParticleSystem->Update(0.05);
	RGetParticleSystem()->Update(fElapsed);

	if (Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->Update();

	OnCameraUpdate(fElapsed);
	//	m_fTime->SetWarpingAdd(GetTickCount());

	m_WeaponManager.Update();
}

void ZGame::OnCameraUpdate(float Elapsed)
{
	if (m_Match.GetRoundState() == MMATCH_ROUNDSTATE_PREPARE)
	{
		rvector dir = GetMapDesc()->GetWaitCamDir();
		rvector pos = GetMapDesc()->GetWaitCamPos();
		rvector up(0, 0, 1);

		RSetCamera(pos, pos + dir, up);
	}
	else
	{
		ZGetGameInterface()->GetCamera()->Update(Elapsed);
	}
}
//jintriple3 µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ·
void ZGame::CheckMyCharDeadByCriticalLine()
{
	MUID uidAttacker = MUID(0, 0);
	bool bReturnValue = m_pMyCharacter->GetPosition().z >= DIE_CRITICAL_LINE;
	if (m_pMyCharacter->GetPosition().z >= DIE_CRITICAL_LINE)	//³ª?EÀ§¿¡ ÀÖÀ½ »ó?EÏÁE¾Ê?E.
		PROTECT_DEBUG_REGISTER(bReturnValue)
		return;

	// ³ª¶ôÀ¸·Î ¶³¾ûÝö?E³¡..-_-;
	//m_pMyCharacter->SetVelocity(rvector(0,0,0));
	uidAttacker = m_pMyCharacter->GetLastThrower();

	ZObject* pAttacker = ZGetObjectManager()->GetObject(uidAttacker);
	if (pAttacker == NULL || !CanAttack(pAttacker, m_pMyCharacter))
	{
		uidAttacker = ZGetMyUID();
		pAttacker = m_pMyCharacter;
	}

	m_pMyCharacter->OnDamaged(pAttacker, m_pMyCharacter->GetPosition(), ZD_FALLING, MWT_NONE, m_pMyCharacter->GetHP());
	// Custom: Made CMT_SYSTEM
	ZChatOutput(ZMsg(MSG_GAME_FALL_NARAK), ZChat::CMT_SYSTEM);

#ifdef _DEBUG
	if (ZIsLaunchDevelop() && ZGetQuest()->GetCheet(ZQUEST_CHEET_GOD) == true)
		ZGetGameInterface()->RespawnMyCharacter();
#endif
}
//jintriple3 µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ·
void ZGame::CheckMyCharDeadUnchecked()
{
	MUID uidAttacker = MUID(0, 0);
	//	if ((m_pMyCharacter->IsDie() == false) && (m_pMyCharacter->GetHP() <= 0))
	bool bCheck = (m_pMyCharacter->IsDie() == true) | (m_pMyCharacter->GetHP() > 0);
	if ((m_pMyCharacter->IsDie() == true) || (m_pMyCharacter->GetHP() > 0))
		PROTECT_DEBUG_REGISTER(bCheck)
		return;

	//hp <=0 && m_pMyCharacter->IsDie() == false
	if (uidAttacker == MUID(0, 0) && m_pMyCharacter->GetLastAttacker() != MUID(0, 0))
		uidAttacker = m_pMyCharacter->GetLastAttacker();

	// ´ÙÀ½¶ó¿ûÑå·Î ³Ñ¾ûÌ¡?EÀ§ÇÑ finish »óÅÂ¿¡¼­´Â ¸Þ½Ã?E¶ó?EÃÀ?»ý·«ÇÑ´Ù
	if (GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_FINISH)
	{
		// Á×´Â Ã´Àº ÇÑ´Ù
		m_pMyCharacter->ActDead();
		m_pMyCharacter->Die();
		return;
	}

	ZPostDie(uidAttacker);		// ÇÇ¾ûÑé¿¡°Ô º¸³»´Â ¸Þ¼¼?E

	// Äù½ºÆ® ¸ðµå´Â Á×À½ ¸Þ¼¼Áö°¡ ´Ù¸£´Ù.
	if (!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) &&
		!ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		ZPostGameKill(uidAttacker);	// ¼­¹ö¿¡ º¸³»´Â ¸Þ¼¼Áö
	}
	else if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		ZPostQuestGameKill();
	}
	else if (ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) {
		ZCharacter* pCharacter = dynamic_cast<ZCharacter*>(ZGetCharacterManager()->Find(uidAttacker));
		if (pCharacter) {
			ZPostGameKill(uidAttacker);
		}
		else {
			ZActorWithFSM* pActor = dynamic_cast<ZActorWithFSM*>(ZGetObjectManager()->GetObject(uidAttacker));
			if (pActor) {
				ZPostQuestGameKill();
			}
		}
	}

	// Á×¾úÀ»¶§ ¹èÆ²¿¡¼­ ³ª°¡?E¿¹¾àÀÌ µÇ¾ûÜÖÀ¸?EÄ«¿ûâ®¸¦ ´Ù½Ã ½ÃÀÛÇØÁØ´Ù.
	if (ZApplication::GetGameInterface()->IsLeaveBattleReserved() == true)
		ZApplication::GetGameInterface()->ReserveLeaveBattle();
}

void ZGame::CheckMyCharDead(float fElapsed)
{
	//jintriple3 µð¹ö±× ·¹Áö½ºÅÍ ÇÙ
	bool bReturnValue = !m_pMyCharacter || m_pMyCharacter->IsDie();
	if (!m_pMyCharacter || m_pMyCharacter->IsDie())
		PROTECT_DEBUG_REGISTER(bReturnValue)
		return;

	CheckMyCharDeadByCriticalLine();
	CheckMyCharDeadUnchecked();
}

void ZGame::OnPreDraw()
{
	__BP(19, "ZGame::sub1");

	RSetProjection(g_fFOV, DEFAULT_NEAR_Z, g_fFarZ);

	bool bTrilinear = RIsTrilinear();
	RGetDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MIPFILTER, bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);

	if (m_bShowWireframe) {
		RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
		GetWorld()->SetFog(false);
	}
	else {
		RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		GetWorld()->SetFog(true);
	}

	GetWorld()->GetBsp()->SetWireframeMode(m_bShowWireframe);

	rmatrix initmat;
	D3DXMatrixIdentity(&initmat);
	RGetDevice()->SetTransform(D3DTS_WORLD, &initmat);
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, true);
	//not sure if right place
	RGetDevice()->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();
	pd3dDevice->SetTexture(0, NULL);
	pd3dDevice->SetTexture(1, NULL);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);

	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);

	if (m_pMyCharacter)
	{
		if (ZGetConfiguration()->GetVideo()->bDynamicLight)
		{
			auto pos = m_pMyCharacter->GetPosition();
			RGetDynamicLightManager()->SetPosition(pos);
		}
	}

	__EP(19);
}

int g_debug_render_mode = 0;

extern MDrawContextR2* g_pDC;

int IsNan(double x)
{
	union { unsigned __int64 u; double f; } ieee754;
	ieee754.f = x;

	return ((unsigned)(ieee754.u >> 32) & 0x7fffffff) + ((unsigned)ieee754.u != 0) > 0x7ff00000;
}

int IsInf(double x)
{
	union { unsigned __int64 u; double f; } ieee754;
	ieee754.f = x;

	return ((unsigned)(ieee754.u >> 32) & 0x7fffffff) == 0x7ff00000 && ((unsigned)ieee754.u == 0);
}

void ZGame::Draw()
{
	{
		static bool bCtrlFWasPressed = false;
		bool bCtrlFPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState('F') & 0x8000);
		if (bCtrlFPressed && !bCtrlFWasPressed)
		{
			char szName[128];
			sprintf(szName, "FPS: %3.3f", g_fFPS);
			ZApplication::GetGameInterface()->GetChat()->Input(szName);
		}
		bCtrlFWasPressed = bCtrlFPressed;
	}
	////////Å×½ºÆ® ?EÄÚ?E..³ªÁß¿¡ Áö?E?.//////////////////////////////
	////////////////////////////////////////////////////////////////////for test
		/*
	#ifdef _DEBUG
		if(GetAsyncKeyState( VK_UP ))	//Ä®¼¦ ÇÙ
		{
			fShotTime += 0.1f;
			rvector vPos = g_pGame->m_pMyCharacter->GetPosition();

			ZPostShotMelee( vPos, nShot);
			nShot++;
			if(nShot >4)
				nShot = 1;
		}
		if(GetAsyncKeyState( VK_LEFT ))		//°­º£?EÇÙ
		{
			fShotTime += 0.1f;
			ZPostSkill( ZC_SKILL_SPLASHSHOT, MMCIP_MELEE);
		}
		if(GetAsyncKeyState( VK_RIGHT ))	//¼ö·ùÅº ÇÙ
		{
			rvector vPos = g_pGame->m_pMyCharacter->GetPosition();
			rvector vDir = g_pGame->m_pMyCharacter->GetDirection();
			fShotTime += 0.1f;

			vPos.z += 120.0f;

			int type = ZC_WEAPON_SP_GRENADE;	//weapon
			int sel_type = MMCIP_MELEE; //MMCIP_PRIMARY; //MMCIP_MELEE;			//parts
			ZPostShotSp( vPos, vDir, type, sel_type);
		}
		if(GetAsyncKeyState( VK_DOWN ))	    //·ÎÄÏ ÇÙ
		{
			rvector vPos = g_pGame->m_pMyCharacter->GetPosition();
			rvector vDir = g_pGame->m_pMyCharacter->GetDirection();

			vPos.z += 120.0f;
			fShotTime += 0.1f;

			int type = ZC_WEAPON_SP_ROCKET; //ZC_WEAPON_SP_GRENADE;	//weaponww
		È¤½Ã ³ªÁß¿¡ ¶Ç ÇÙ ?EÃÇØ¼­ ¾µÀÏÀÌ ÀÖÀ»¼öµµ ÀÖ¾ûØ­ ÁÖ¼®Ã³¸®ÇØ¼­ ³²°ÜµÒ..
		*/

		////////////////////////////////////////////////////////////////////
	__BP(20, "ZGame::Draw");

	RRESULT isOK = RIsReadyToRender();

	if (isOK == R_NOTREADY)
	{
		__EP(20);
		return;
	}

	OnPreDraw();		// Device »óÅÂ°ª ¼³Á¤Àº ¿©±â¼­ ÇÏÀÚ

//	RRenderNodeMgr::m_bRenderBuffer = true;//------test code
	/*
	else if(isOK==R_RESTORED) {
	} // restore device dependent objects
	*/

	rmatrix _mat;
	RGetDevice()->GetTransform(D3DTS_WORLD, &_mat);

	__BP(21, "ZGame::Draw::DrawWorld");
	GetWorld()->Draw();
	__EP(21);

	// ¸Ê¿¡ ±â¼úµÈ Æ¯?E´õ¹Ì ¿À?E§Æ®Áß ±×·Á¾ß ÇÒ°ÍÀÌ ÀÖ´Â °æ?E

	ZMapDesc* pMapDesc = GetMapDesc();

	if (pMapDesc) {
		pMapDesc->DrawMapDesc();
	}

	/*
	D3DLIGHT9 light;
	light.Type			= D3DLIGHT_POINT;
	light.Ambient.r		= 0.1f;
	light.Ambient.g		= 0.1f;
	light.Ambient.b		= 0.1f;
	light.Specular.r	= 1.0f;
	light.Specular.g	= 1.0f;
	light.Specular.b	= 1.0f;
	light.Attenuation0	= 0.05f;
	light.Attenuation1	= 0.002f;
	light.Attenuation2	= 0.0f;

	light.Range			= 200.f;
	light.Position		= m_pMyCharacter->GetPosition();

	light.Diffuse.r		= .9f;
	light.Diffuse.g		= .1f;
	light.Diffuse.b		= .1f;

	GetWorld()->GetBsp()->DrawLight(&light);
	//*/
	if (m_Match.GetRoundState() != MMATCH_ROUNDSTATE_PREPARE)
	{
		__BP(22, "ZGame::Draw::DrawCharacters");

		m_ObjectManager.Draw();

		__EP(22);

		m_render_poly_cnt = RealSpace2::g_poly_render_cnt;

		//		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
		//		RGetDevice()->SetTexture(0,NULL);
		//		RGetDevice()->SetTexture(1,NULL);
		//		RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
	}

	RGetDevice()->SetTransform(D3DTS_WORLD, &_mat);//map_mat

	ZGetWorldItemManager()->Draw(0, GetWorld()->GetWaterHeight(), GetWorld()->IsWaterMap());

	m_WeaponManager.Render();//weapon

	__BP(50, "ZGame::DrawObjects");

	GetWorld()->GetBsp()->DrawObjects();

	__EP(50);

	__BP(17, "ZGame::Draw::Reflection");

	GetWorld()->GetWaters()->Render();

	__EP(17);

	if (m_Match.GetRoundState() != MMATCH_ROUNDSTATE_PREPARE)
	{
		__BP(23, "ZGame::Draw::DrawWeapons and effects");
#ifndef _PUBLISH
		//		TestCreateEffects();
#endif

		ZGetEffectManager()->Draw(timeGetTime());

		__EP(23);
	}

#ifdef _WORLD_ITEM_
	__BP(34, "ZGame::Draw::ZGetWorldItemManager");

	ZGetWorldItemManager()->Draw(1, GetWorld()->GetWaterHeight(), GetWorld()->IsWaterMap());

	__EP(34);
#endif

	//	RRenderNodeMgr::m_bRenderBuffer = false;//------test code

		/*
		if(m_bCharacterLight)
		GetWorld()->GetBsp()->DrawLight(&light);
		*/
		//	m_render_poly_cnt = RealSpace2::g_poly_render_cnt;

	__BP(35, "ZGame::Draw::RGetParticleSystem");

	RGetParticleSystem()->Draw();

	__EP(35);

	__BP(36, "ZGame::Draw::LenzFlare");

	if (RReadyLenzFlare())
	{
		RGetLenzFlare()->Render(RCameraPosition, GetWorld()->GetBsp());
	}

	RSetProjection(DEFAULT_FOV, DEFAULT_NEAR_Z, g_fFarZ);
	RSetFog(FALSE);

	__EP(36);

	__BP(37, "ZGame::Draw::FlashBangEffect");

	if (IsActivatedFlashBangEffect())
	{
		ShowFlashBangEffect();
	}

	__BP(505, "ZGame::Draw::RenderStencilLight");
	if (Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->Render();
	__EP(505);

	__EP(37);

	__BP(38, "ZGame::Draw::DrawGameMessage");

	m_Match.OnDrawGameMessage();

	//	m_HelpScreen.DrawHelpScreen();

	__EP(38);

	__EP(20);

	//	»¡°£¶óÀÎÀ» ±×·Áº»´Ù È­¸é¿¡ º¸ÀÌ?E»öÀÌ ¹Ù²ûÑµ·Ï...? ±âº»Àº ÆÄ?EÃ¼Å©µÇ?E»¡°­...
	/*
		rvector line1 = rvector(200,163,168);
		rvector line2 = rvector(900,163,168);

		rmatrix m;

		rvector pos = line1;

		rvector dir = rvector(0,0,1);
		rvector up  = rvector(0,1,0);
		rvector max = rvector( 10, 10, 10);
		rvector min = rvector(-10,-10,-10);

		MakeWorldMatrix(&m,pos,dir,up);

		draw_box(&m,max,min,0xffff0000);

		pos = line2;

		MakeWorldMatrix(&m,pos,dir,up);

		draw_box(&m,max,min,0xffff0000);

		//////////////////////////////////////////////////////////////////////

		D3DXMatrixIdentity(&m);

		RGetDevice()->SetTransform( D3DTS_WORLD, &m );

		RGetDevice()->SetTexture(0,NULL);
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
		RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
		RDrawLine(line1,line2,0xffff0000);

		rvector new_line1;
		rvector new_line2;

		D3DXVec3TransformCoord(&new_line1,&line1,&RViewProjection);
		D3DXVec3TransformCoord(&new_line2,&line2,&RViewProjection);

		rvector tmin = rvector(-1.f,-1.f,0.f);
		rvector tmax = rvector( 1.f, 1.f,1.f);

		D3DXMatrixIdentity(&m);
	//	MakeWorldMatrix(&m,rvector(0,0,0),dir,up);
		draw_box(&m,tmax*100,tmin*100,0xff00ffff);

		D3DXMatrixIdentity(&m);
		RGetDevice()->SetTransform( D3DTS_WORLD, &m );
		RGetDevice()->SetTexture(0,NULL);
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
		RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
		RDrawLine(new_line1*100,new_line2*100,0xffffffff);

		/////////////////////////////////////////////////////////////////////

		int nPick = 0;

		if(isInViewFrustum(line1,line2, RGetViewFrustum() )) {
			nPick = 1;
		}
		else
			nPick = 0;

		char szTemp[256];
		sprintf(szTemp, "line1 = %6.3f %6.3f %6.3f  line2 = %6.3f %6.3f %6.3f Pick %d", new_line1.x,new_line1.y,new_line1.z, new_line2.x,new_line2.y,new_line2.z,nPick);
		g_pDC->Text(100,200,szTemp);
	*/

	/*//bsp pick Å×½ºÆ® kimyhwan
	{
		float v1z = 0; float v2z = 100;
		FILE* fp = fopen("g:\\coord.txt", "rt");
		if (fp)
		{
			char sz[256];
			fgets(sz, 256, fp);
			sscanf(sz, "%f %f", &v1z, &v2z);
			fclose(fp);
		}

		rvector v1(0, 0, v1z);
		rvector v2(0, 0, v2z);
		DWORD color = 0xff0000ff;

		//const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;
		const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE;
		RBSPPICKINFO bpi;
		bool bBspPicked = GetWorld()->GetBsp()->PickTo(v1, v2, &bpi, dwPickPassFlag);
		if (bBspPicked)
			color = 0xffff0000;

		if (bBspPicked)
		{
			if (Magnitude(v2-v1)<Magnitude(bpi.PickPos-v1))
				color = 0xffff00ff;
		}

		rmatrix m;
		D3DXMatrixIdentity(&m);
		RGetDevice()->SetTransform( D3DTS_WORLD, &m );
		RGetDevice()->SetTexture(0,NULL);
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
		RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

		RDrawLine(v1, v2, color);
	}*/
}

void ZGame::DrawDebugInfo()
{
	char szTemp[256] = "";
	int n = 20;
	g_pDC->SetColor(MCOLOR(0xFFffffff));

	for (ZCharacterManager::iterator itor = m_CharacterManager.begin(); itor != m_CharacterManager.end(); ++itor)
	{
		ZCharacterObject* pCharacter = (*itor).second;
		sprintf(szTemp, "Pos = %6.3f %6.3f %6.3f  Dir = %6.3f %6.3f %6.3f", pCharacter->GetPosition().x,
			pCharacter->GetPosition().y, pCharacter->GetPosition().z,
			pCharacter->m_Direction.x, pCharacter->m_Direction.y, pCharacter->m_Direction.z);
		g_pDC->Text(20, n, szTemp);
		n += 15;

		/*
				sprintf(szTemp, "state = %d , %d", (int)(pCharacter->GetState()), (int)(pCharacter->GetStateSub()));
				g_pDC->Text(20, n, szTemp);
				n+= 15;
		*/

		RVisualMesh* pVMesh = pCharacter->m_pVMesh;

		AniFrameInfo* pAniLow = pVMesh->GetFrameInfo(ani_mode_lower);
		AniFrameInfo* pAniUp = pVMesh->GetFrameInfo(ani_mode_upper);

		sprintf(szTemp, "%s frame down %d / %d ", pAniLow->m_pAniSet->GetName(), pAniLow->m_nFrame, pAniLow->m_pAniSet->GetMaxFrame());
		g_pDC->Text(20, n, szTemp);
		n += 15;

		if (pAniUp->m_pAniSet)
		{
			sprintf(szTemp, "%s frame up %d / %d ", pAniUp->m_pAniSet->GetName(), pAniUp->m_nFrame, pAniUp->m_pAniSet->GetMaxFrame());
			g_pDC->Text(20, n, szTemp);
			n += 15;
		}
	}

	/*
		n = 300;
		for (MMatchPeerInfoList::iterator itor = ZGetGameClient()->GetPeers()->begin();
			 itor != ZGetGameClient()->GetPeers()->end(); ++itor)
		{
			MMatchPeerInfo* pPeerInfo = (*itor);
			sprintf(szTemp, "MUID(%d, %d) , IP = %s, port = %d", pPeerInfo->uidChar.High,
					pPeerInfo->uidChar.Low, pPeerInfo->szIP, pPeerInfo->nPort);
			g_pDC->Text(20,n,szTemp);
			n+=15;
		}
	*/
}

void ZGame::Draw(MDrawContextR2& dc)
{
	/*	// ÆÐ½º?EEÃâ·Â.. for debug
	char buffer[256];
	sprintf(buffer," state: %d , pathnode: %d",m_pMyCharacter->m_State,m_pMyCharacter->m_nPathNodeID);

	dc.SetColor(MCOLOR(0xFFffffff));
	dc.Text(0,20,buffer);
	//*/
}

void ZGame::ParseReservedWord(char* pszDest, const char* pszSrc)
{
	char szSrc[256];
	char szWord[256];

	strcpy(szSrc, pszSrc);

	char szOut[256];	ZeroMemory(szOut, 256);
	int nOutOffset = 0;

	char* pszNext = szSrc;
	while (*pszNext != NULL) {
		pszNext = MStringCutter::GetOneArg(pszNext, szWord);

		if ((*szWord == '$') && (stricmp(szWord, "$player") == 0)) {
			sprintf(szWord, "%d %d", m_pMyCharacter->GetUID().High, m_pMyCharacter->GetUID().Low);
		}
		else if ((*szWord == '$') && (stricmp(szWord, "$target") == 0)) {
			sprintf(szWord, "%d %d", m_pMyCharacter->GetUID().High, m_pMyCharacter->GetUID().Low);	// Target»ý±â¹È ²À Target À¸·Î ¹Ù²Ù?E
		}

		strcpy(szOut + nOutOffset, szWord);	nOutOffset += (int)strlen(szWord);
		if (*pszNext) {
			strcpy(szOut + nOutOffset, " ");
			nOutOffset++;
		}
	}
	strcpy(pszDest, szOut);
}

#include "ZMessages.h"
extern bool g_bProfile;

// observer ¸ðµå¿¡¼­µµ µô·¹ÀÌ¸¦ °ÅÄ¥ ÇÊ¿ä¾ø´Â Ä¿¸Çµå?E
bool IsIgnoreObserverCommand(int nID)
{
	switch (nID) {
	case MC_PEER_PING:
	case MC_PEER_PONG:
	case MC_PEER_OPENED:
	case MC_MATCH_GAME_RESPONSE_TIMESYNC:
		return false;
	}
	return true;
}

void ZGame::OnCommand_Observer(MCommand* pCommand)
{
	if (!IsIgnoreObserverCommand(pCommand->GetID()))
	{
		OnCommand_Immidiate(pCommand);
		return;
	}

	ZObserverCommandItem* pZCommand = new ZObserverCommandItem;
	pZCommand->pCommand = pCommand->Clone();
	pZCommand->fTime = GetTime();
	m_ObserverCommandList.push_back(pZCommand);

#ifdef _LOG_ENABLE_OBSERVER_COMMAND_BUSH_
	if (pCommand->GetID() != 10012 && pCommand->GetID() != 10014)
	{ // [ID:10012]:BasicInfo, [ID:10014]:HPAPInfo
		char buf[256];
		sprintf(buf, "[OBSERVER_COMMAND_BUSH:%d]: %s\n", pCommand->GetID(), pCommand->GetDescription());
		OutputDebugString(buf);
	}
#endif

	if (pCommand->GetID() == MC_PEER_BASICINFO)
	{
		/*
		ZCharacter *pChar=m_CharacterManager.Find(pCommand->GetSenderUID());
		if(pChar)
		{
			mlog("%s basic info : %3.3f \n",pChar->GetProperty()->szName,pZCommand->fTime);
		}
		*/
		OnPeerBasicInfo(pCommand, true, false);
	}
}

void ZGame::ProcessDelayedCommand()
{
	// Custom: Iterator increment fix
	for (ZObserverCommandList::iterator i = m_DelayedCommandList.begin(); i != m_DelayedCommandList.end();)
	{
		ZObserverCommandItem* pItem = *i;

		// ½ÇÇàÇÒ ½Ã°£ÀÌ Áö³µÀ¸?E½ÇÇàÇÑ´Ù
		if (GetTime() > pItem->fTime)
		{
			OnCommand_Immidiate(pItem->pCommand);
			i = m_DelayedCommandList.erase(i);
			delete pItem->pCommand;
			delete pItem;
		}
		else
			++i;
	}
}

void ZGame::OnReplayRun()
{
	if (m_ReplayCommandList.size() == 0 && m_bReplaying.Ref()) {
		m_bReplaying.Set_CheckCrc(false);
		EndReplay();
		return;
	}

	//	static float fLastTime = 0;
	while (m_ReplayCommandList.size())
	{
		ZObserverCommandItem* pItem = *m_ReplayCommandList.begin();

		//		_ASSERT(pItem->fTime>=fLastTime);
#ifdef _REPLAY_TEST_LOG
		m_ReplayLogTime = pItem->fTime;
#else
		if (GetTime() < pItem->fTime)
			return;
#endif

		//		mlog("curtime = %d ( %3.3f ) time = %3.3f , id %d \n",timeGetTime(),GetTime(),pItem->fTime,pItem->pCommand->GetID());

		m_ReplayCommandList.erase(m_ReplayCommandList.begin());

		bool bSkip = false;
		switch (pItem->pCommand->GetID())
		{
		case MC_REQUEST_XTRAP_HASHVALUE:
		case MC_RESPONSE_XTRAP_HASHVALUE:
		case MC_REQUEST_XTRAP_SEEDKEY:
		case MC_RESPONSE_XTRAP_SEEDKEY:
		case MC_REQUEST_XTRAP_DETECTCRACK:
		case MC_REQUEST_GAMEGUARD_AUTH:
		case MC_RESPONSE_GAMEGUARD_AUTH:
		case MC_REQUEST_FIRST_GAMEGUARD_AUTH:
		case MC_RESPONSE_FIRST_GAMEGUARD_AUTH:
			bSkip = true;
		}

		if (bSkip == false)
			OnCommand_Observer(pItem->pCommand);

#ifdef _LOG_ENABLE_REPLAY_COMMAND_DELETE_
		if (pItem->pCommand->GetID() != 10012 && pItem->pCommand->GetID() != 10014)
		{ // [ID:10012]:BasicInfo, [ID:10014]:HPAPInfo
			char buf[256];
			sprintf(buf, "[REPLAY_COMMAND_DELETE:%d]: %s\n", pItem->pCommand->GetID(), pItem->pCommand->GetDescription());
			OutputDebugString(buf);
		}
#endif

#ifdef _REPLAY_TEST_LOG
		// ³­ÀÔ?E³ª°¡?EÄ¿¸Ç?E½Ã°£Àº m_ObserverCommandList¿¡ µé¾ûÌ¡¸é¼­ ¾ø¾ûÝö?E¶§¹®¿¡ ¿©±â¼­ Ã³¸®
		switch (pItem->pCommand->GetID())
		{
		case MC_MATCH_STAGE_ENTERBATTLE:
		{
			unsigned char nParam;
			pItem->pCommand->GetParameter(&nParam, 0, MPT_UCHAR);

			MCommandParameter* pParam = pItem->pCommand->GetParameter(1);
			if (pParam->GetType() != MPT_BLOB) break;
			void* pBlob = pParam->GetPointer();

			MTD_PeerListNode* pPeerNode = (MTD_PeerListNode*)MGetBlobArrayElement(pBlob, 0);
			mlog("[%d EnterBattleRoom Time:%3.3f]\n", pPeerNode->uidChar.Low, pItem->fTime);
		}
		break;
		case MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT:
		{
			MUID uidChar;
			pItem->pCommand->GetParameter(&uidChar, 0, MPT_UID);
			mlog("[%d LeaveBattleRoom Time:%3.3f]\n", uidChar.Low, m_ReplayLogTime);
		}
		break;
		}
#endif

		delete pItem->pCommand;
		delete pItem;
	}
}

void ZGame::OnObserverRun()
{
	while (m_ObserverCommandList.begin() != m_ObserverCommandList.end())
	{
		ZObserverCommandItem* pItem = *m_ObserverCommandList.begin();
		if (GetTime() - pItem->fTime < ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetDelay())
			return;

		m_ObserverCommandList.erase(m_ObserverCommandList.begin());

		if (pItem->pCommand->GetID() == MC_PEER_BASICINFO)
			OnPeerBasicInfo(pItem->pCommand, false, true);
		else
		{
			OnCommand_Immidiate(pItem->pCommand);

#ifdef _LOG_ENABLE_OBSERVER_COMMAND_DELETE_
			char buf[256];
			sprintf(buf, "[OBSERVER_COMMAND_DELETE:%d]: %s\n", pItem->pCommand->GetID(), pItem->pCommand->GetDescription());
			OutputDebugString(buf);
#endif
		}

		delete pItem->pCommand;
		delete pItem;
	}

#ifdef _REPLAY_TEST_LOG
	for (int i = 0; i < 16; ++i)
	{
		if (m_Replay_UseItem[i].uid.Low == 0)
			break;
		for (int j = 0; j < 5; ++j)
		{
			if (m_Replay_UseItem[i].Item[j].Itemid == 0)
				break;
			MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_Replay_UseItem[i].Item[j].Itemid);

			mlog("[ uid:%d Item:%s(%d) UseCount:%d ]\n", m_Replay_UseItem[i].uid.Low, pItemDesc->m_pMItemName->Ref().m_szItemName, pItemDesc->m_nID, m_Replay_UseItem[i].Item[j].ItemUseCount);
		}
	}
	mlog("[Replay Playtime: %f]\n[End Replay]\n", m_ReplayLogTime);
#endif
}

void ZGame::FlushObserverCommands()
{
	while (m_ObserverCommandList.begin() != m_ObserverCommandList.end())
	{
		ZObserverCommandItem* pItem = *m_ObserverCommandList.begin();

		m_ObserverCommandList.erase(m_ObserverCommandList.begin());

		if (pItem->pCommand->GetID() != MC_PEER_BASICINFO)
			OnCommand_Immidiate(pItem->pCommand);

		delete pItem->pCommand;
		delete pItem;
	}
}

bool ZGame::OnCommand(MCommand* pCommand)
{
	if (m_bRecording)
	{
		ZObserverCommandItem* pItem = new ZObserverCommandItem;
		pItem->fTime = m_fTime.Ref();
		pItem->pCommand = pCommand->Clone();

		m_ReplayCommandList.push_back(pItem);

#ifdef _LOG_ENABLE_RELAY_COMMAND_BUSH_
		if (pCommand->GetID() != 10012 && pCommand->GetID() != 10014)
		{ // [ID:10012]:BasicInfo, [ID:10014]:HPAPInfo
			char buf[256];
			sprintf(buf, "[RELAY_COMMAND_BUSH:%d]: %s\n", pCommand->GetID(), pCommand->GetDescription());
			OutputDebugString(buf);
		}
#endif
	}

	if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
	{
		OnCommand_Observer(pCommand);
		return true;
	}

	if (FilterDelayedCommand(pCommand))
	{
		return true;
	}

	return OnCommand_Immidiate(pCommand);
}

// À¯?EÄÃ·¯

bool GetUserGradeIDColor(MMatchUserGradeID gid,MCOLOR& UserNameColor,char* sp_name)
{
	// Custom: Different UGrade colours
	if (gid == MMUG_DEVELOPER)
	{
		UserNameColor = MCOLOR(255, 128, 64);
		//UserNameColor = MCOLOR(255,128, 64);
		// Custom: Unmask names
		//if(sp_name) {
		//	strcpy(sp_name,ZMsg(MSG_WORD_DEVELOPER));
		//}
		return true;
	}
	else if (gid == MMUG_ADMIN)
	{
		UserNameColor = MCOLOR(205, 0, 0);
		//UserNameColor = MCOLOR(255,128, 64);
		// Custom: Unmask names
		//if(sp_name) {
		//	strcpy(sp_name,ZMsg(MSG_WORD_ADMIN));
		//}
		return true;
	}
	else if (gid == MMUG_GAMEMASTER)
	{
		UserNameColor = MCOLOR(0xFF0066FF);
		return true;
	}
	else if (gid == MMUG_EVENTMASTER)
	{
		UserNameColor = MCOLOR(154, 39, 154);
		return true;
	}
	else if (gid == MMUG_EVENTTEAM)
	{
		UserNameColor = MCOLOR(0xFF0040FF);
		return true;
	}

	else if (gid == MMUG_MANAGER)
	{
		UserNameColor = MCOLOR(0xFF009676);
		return true;
	}
#ifdef _VIPGRADES
	else if (gid == MMUG_VIP1)
	{
		//UserNameColor = MCOLOR(0xFF00FFF6); 
		//Custom: Time change color grade time real By Desperate
		int szTimeColorGrade = (timeGetTime() / 125 % 6);
		int GradeColorChange = 0;
		for (GradeColorChange = 0; GradeColorChange < szTimeColorGrade; GradeColorChange++);
		if (GradeColorChange == 0)
		{
			// Color 1
			UserNameColor = MCOLOR(0xFFFF0000);
		}
		if (GradeColorChange == 1)
		{
			// Color 2
			UserNameColor = MCOLOR(0xFF02B80D);
		}
		if (GradeColorChange == 2)
		{
			// Color 3
			UserNameColor = MCOLOR(0xFF00A2FF);
		}
		if (GradeColorChange == 3)
		{
			// Color 4
			UserNameColor = MCOLOR(0xFFFFE800);
		}
		if (GradeColorChange == 4)
		{
			// Color 5
			UserNameColor = MCOLOR(0xFFFB7607);
		}
		if (GradeColorChange == 5)
		{
			// Color 6
			UserNameColor = MCOLOR(0xFF00F6CB);
		}
		if (GradeColorChange == 6)
		{
			// Color 7
			UserNameColor = MCOLOR(0xFF8700FF);
		}


		return true;
	}
	else if (gid == MMUG_VIP2)
	{
		UserNameColor = MCOLOR(0xFF0026FF); //Azul
		return true;
	}
	else if (gid == MMUG_VIP3)
	{
		UserNameColor = MCOLOR(0xFFFFD800); //Amarillo
		return true;
	}
	else if (gid == MMUG_VIP4)
	{
		UserNameColor = MCOLOR(0xFFFF54f9); //Rosado
		return true;
	}
	else if (gid == MMUG_VIP5)
	{
		UserNameColor = MCOLOR(0xFF404040); //Gris (Streamer)
		return true;
	}
	else if (gid == MMUG_VIP6)
	{
		UserNameColor = MCOLOR(0xFFBD00FC); //
		return true;
	}
	else if (gid == MMUG_VIP7)
	{
		UserNameColor = MCOLOR(0xFFFF7300); //Super Donor (Naranja)
		return true;
	}
#endif

#ifdef _EVENTGRD
	else if (gid == MMUG_EVENT1)
	{
		UserNameColor = MCOLOR(0xFFB5006D); //Rosado Oscuro
		return true;
	}
	else if (gid == MMUG_EVENT2)
	{
		UserNameColor = MCOLOR(0xFF009DB5); //Agua
		return true;
	}
	else if (gid == MMUG_EVENT3)
	{
		UserNameColor = MCOLOR(0xFFb57600); //Kake
		return true;
	}
	else if (gid == MMUG_EVENT4)
	{
		UserNameColor = MCOLOR(0xFF0d0D0D); //Black
		return true;
	}
	else if (gid == MMUG_BOOSTER)
	{
	UserNameColor = MCOLOR(0xFF750021); //booster
	return true;
	}
	else if (gid == MMUG_WINTOUR)
	{
	UserNameColor = MCOLOR(0xFFB46C36); //winner tour
	return true;
	}
	else if (gid == MMUG_HIDE_ADMIN)
	{
	UserNameColor = MCOLOR(0xFFFFFFFF); //winner tour
	return true;
	}
#endif

	return false;
}

bool ZGame::GetUserNameColor(MUID uid, MCOLOR& UserNameColor, char* sp_name)
{
	MMatchUserGradeID gid = MMUG_FREE;

	if (m_pMyCharacter->GetUID() == uid)
	{
		if (ZGetMyInfo()) {
			gid = ZGetMyInfo()->GetUGradeID();
			//			gid = MMUG_DEVELOPER;//test
		}
		else {
			mlog("ZGame::GetUserNameColor ZGetMyInfo==NULL \n");
		}
	}
	else
	{
		MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(uid);
		if (pPeer) {
			gid = pPeer->CharInfo.nUGradeID;
		}
	}

	return GetUserGradeIDColor(gid, UserNameColor, sp_name);
}

void ZTranslateCommand(MCommand* pCmd, string& strLog)
{
	char szBuf[256] = "";

	// ½Ã°£
	unsigned long nGlobalClock = ZGetGame()->GetTickTime();
	itoa(nGlobalClock, szBuf, 10);
	strLog = szBuf;
	strLog += ": ";

	// Command
	strLog += pCmd->m_pCommandDesc->GetName();

	// PlayerName
	string strPlayerName;
	MUID uid = pCmd->GetSenderUID();
	ZCharacter* pChar = (ZCharacter*)ZGetCharacterManager()->Find(uid);
	if (pChar)
		strPlayerName = pChar->GetProperty()->GetName();
	else
		strPlayerName = "Unknown";

	strLog += " (";
	strLog += strPlayerName;
	strLog += ") ";

	// Params
	string strParams;
	for (int i = 0; i < pCmd->GetParameterCount(); i++) {
		char szParam[256] = "";
		pCmd->GetParameter(i)->GetString(szParam);
		strParams += szParam;
		if (i < pCmd->GetParameterCount() - 1)
			strParams += ", ";
	}
	strLog += strParams;
}

void ZLogCommand(MCommand* pCmd)
{
	if (pCmd->GetID() == MC_AGENT_TUNNELING_UDP) {
		return;
	}

	string strCmd;
	ZTranslateCommand(pCmd, strCmd);

	OutputDebugString(strCmd.c_str());
	OutputDebugString("\n");
}
#ifdef _ZPEERANTIHACK
//void ZGame::OnPeerAntiLead(MCommand* PCommand)
//{// hahahahahaha
//	if (m_pMyCharacter->GetUID() != PCommand->GetReceiverUID()) return;
//	int nID = 0;
//	if (!PCommand->GetParameter(&nID, 1, MPT_INT)) return;
//	RecvArray::iterator it = RecievedShots.find(nID);
//	if (it != RecievedShots.end())
//	{
//		ZPOSTHITRESPONSE(MCommandParameterInt(nID), PCommand->GetSenderUID());
//		it->second = timeGetTime();
//		return;
//	}
//	else RecievedShots.insert(RecvArray::value_type(nID, timeGetTime()));
//	ZPOSTHITRESPONSE(MCommandParameterInt(nID), PCommand->GetSenderUID());
//	ZCharacter* pCharacter = ZGetCharacterManager()->Find(PCommand->GetSenderUID());
//	if (!pCharacter || pCharacter && pCharacter->m_dwStatusBitPackingValue.Ref().m_bLostConEffect == true || !pCharacter->GetProperty()) return;
//	MCommandParameter* pParam = PCommand->GetParameter(0);
//	if (pParam->GetType() != MPT_BLOB) return;
//	void* pBlob = pParam->GetPointer();
//	char victim[MATCHOBJECT_NAME_LENGTH], attacker[MATCHOBJECT_NAME_LENGTH];
//	sprintf(victim, m_pMyCharacter->GetProperty()->GetName());
//	sprintf(attacker, pCharacter->GetProperty()->GetName());
//	int nVic = strlen(victim), nAttc = strlen(attacker);
//	MTD_SHOTINFO* Info = (MTD_SHOTINFO*)MGetBlobArrayElement(pBlob, 0);
//	if (!Info) return;
//	Info->Type = Info->Type ^ (char)((nVic >= 3) ? victim[2] : 'p');
//	ZItem* pItem = pCharacter->GetItems()->GetItem((MMatchCharItemParts)(Info->Type));
//	if (!pItem || !pItem->GetDesc()) return;
//	if (m_pMyCharacter && m_pMyCharacter->GetUID() == PCommand->GetReceiverUID())
//	{
//		MMatchItemDesc* MyDesc = MGetMatchItemDescMgr()->GetItemDesc(pItem->GetDesc()->m_nID);
//		if (!MyDesc || pItem->GetDesc() != MyDesc) return;
//		int nTotalDmg = 0;
//		//for (int i = 0; i < nSize; ++i)
//		//{
//		if (m_pMyCharacter && ZGetGameClient()->GetPlayerUID() != PCommand->GetSenderUID() && (Info->MYID.Low ^ 'j') == ZGetGameClient()->GetPlayerUID().Low && (!GetMatch()->IsTeamPlay() || ((GetMatch()->IsTeamPlay() && GetMatch()->GetTeamKillEnabled()) || !m_pMyCharacter->IsTeam(pCharacter))))
//		{
//			int ShotLimit = 1;
//			int nSize = Info->Size ^ (char)((nAttc >= 2) ? attacker[1] : 'p');
//			if (MyDesc->m_nWeaponType.Ref() == MWT_SAWED_SHOTGUN || MyDesc->m_nWeaponType.Ref() == MWT_SHOTGUN) ShotLimit = 12;
//			if (nSize > ShotLimit) return;
//			Info->Parts = Info->Parts ^ (char)((nVic >= 4) ? victim[3] : 'p');
//			float fRatio = pItem->GetPiercingRatio(MyDesc->m_nWeaponType.Ref(), (RMeshPartsType)(Info->Parts));
//			ZDAMAGETYPE dt = ((RMeshPartsType)(Info->Parts) == eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;
//			/*
//			if(m_pMyCharacter->m_dwStatusBitPackingValue.Ref().m_bLand == true && ZGetGame()->CheckWall((ZCharacter*)m_pMyCharacter, pCharacter))
//			{
//				if(RandomNumber(0, 2) == 1)
//					Dmg = Dmg/2;
//				else continue;
//			}*/
//			float Dmg = CalcActualDamage(pCharacter, m_pMyCharacter, (float)MyDesc->m_nDamage.Ref());
//			Dmg = Dmg * nSize;
//			nTotalDmg = Dmg;
//			m_pMyCharacter->OnDamaged(pCharacter, rvector(Info->X, Info->Y, Info->Z), dt, MyDesc->m_nWeaponType.Ref(), Dmg, fRatio);
//			static float MyLastHpWas = m_pMyCharacter->GetHP(), MyLastApWas = m_pMyCharacter->GetAP();
//			static int Count = 0;
//			if (m_pMyCharacter->IsDie() == false && (m_pMyCharacter->GetHP() == MyLastHpWas || MyLastApWas != 0 && m_pMyCharacter->GetAP() == MyLastApWas))
//				if (Count++ >= 25)
//				{
//					char key = 44;
//					char text[] = { "{DMX_xDE_" };
//					for (int i = 0; i < strlen(text); i++)
//						text[i] = text[i] ^ key;
//					mlog("%s\n", text);
//					ZPostDisconnect();
//				}
//		}
//		//}
//#ifdef _ZPEERANTIHACK
//		ZPeerAntiHackAdd(PCommand->GetSenderUID(), Shot, MyDesc->m_nDelay.Ref(), (MMatchCharItemParts)Info->Type, MyDesc->m_nWeaponType.Ref());
//#endif
//		ZPOSTDMGTAKEN(MCommandParameterInt(nTotalDmg), MCommandParameterInt(0), PCommand->GetSenderUID());
//	}
//}
void ZGame::ZPeerAntiHackAdd(MUID Attacker, ZAntiHackArrayInfo Type, int WepDelay, MMatchCharItemParts EquipType, MMatchWeaponType WeaponType)
{
	if (IsReplay() || ZGetGameInterface()->GetCombatInterface()->GetObserverMode() || m_pMyCharacter->GetUID() == Attacker) return;
	if (ZPeerAntiHackIsBlocked(Attacker)) return;
	ZPeerAntiHackDef::iterator i = ZPeerAntiHack.find(Attacker);
	if (i != ZPeerAntiHack.end())
	{
		int EType = 0;
		EType = (EquipType == MMCIP_PRIMARY ? 0 : 1);
		ZPeerAntiHackData* Data = i->second;
		if (Data)
		{
			unsigned long int nTime = timeGetTime();
			char type[100];
			bool Warning = false;
			switch (Type)
			{
			case Shot:
			{
				int nAddedDelay = 100;
				if (WeaponType == WeaponType == MWT_SMGx2) return; // temp fix
				if (WeaponType == MWT_MACHINEGUN || WeaponType == MWT_RIFLE || WeaponType == MWT_SMG) nAddedDelay = 70;
				int LastDelay = Data->WepDelay[EType][Type];
				if (LastDelay != 0 && ((nTime - Data->LastTime[EType][Type]) + nAddedDelay) <= LastDelay)
				{
					sprintf(type, "Shot Delay (%d/%d)", nTime - Data->LastTime[EType][Type], LastDelay);
					Warning = true;
				}
				else
				{
					Data->WepDelay[EType][Type] = WepDelay;
					Data->LastTime[EType][Type] = nTime;
				}
			}
			break;
			case Melee:
			{
				int nAddedDelay = 300;
				int LastDelay = Data->WepDelay[EType][Type];
				if (((Data->LastTime[EType][Type] - nTime) + nAddedDelay) <= LastDelay)
				{
					sprintf(type, "Melee Delay (%d/%d)", nTime - Data->LastTime[EType][Type], LastDelay);
					Warning = true;
				}
				Data->LastTime[EType][Type] = nTime;
			}
			break;
			case Flip:
			{
				sprintf(type, "Flip");
			}
			break;
			case Massive:
			{
				sprintf(type, "Massive");
			}
			break;
			}
			if (Warning)
			{
				Data->Warned[Type]++;
				if (Data->Warned[Type] >= BLOCKEDAMOUNT)
				{
					char lol[100];
					ZCharacter* pCharacter = ZGetCharacterManager()->Find(Attacker);
					if (pCharacter)
					{
						sprintf(lol, "[PEER ANTIHACK] %s Blocked For %s Hack.", pCharacter->GetCharInfo()->szName, type);
						ZChatOutput(MCOLOR(41, 199, 24), lol);
						ZPeerAntiHackBlocked.push_back(Attacker);
					}
				}
				else if (Data->Warned[Type] >= 1)
				{
					char lol[100];
					ZCharacter* pCharacter = ZGetCharacterManager()->Find(Attacker);
					if (pCharacter)
					{
						sprintf(lol, "[PEER ANTIHACK] %s (%s Hack) Warning %d/%d", pCharacter->GetCharInfo()->szName, type, Data->Warned[Type], BLOCKEDAMOUNT);
						ZChatOutput(MCOLOR(41, 199, 24), lol);
					}
				}
			}
		}
	}
	else ZPeerAntiHack.insert(ZPeerAntiHackDef::value_type(Attacker, new ZPeerAntiHackData()));
}
#endif
bool ZGame::OnCommand_Immidiate(MCommand* pCommand)
{
	/* rpg ½Ã?E?ÄÚ?E ÇÊ¿ä¾ø´ÂµúãÏ´Ù.
	string test;
	if(TranslateMessage(pCommand,&test))
		ZChatOutput(test.c_str());
	*/

#ifdef _DEBUG
	//	ZLogCommand(pCommand);
#endif

	// ¸Õ?EZGameAction ¿¡¼­ Ã³¸®µÇ´Â Ä¿¸Çµå?EÃ³¸®ÇÑ´Ù.
	if (m_pGameAction->OnCommand(pCommand))
	{
		return true;
	}

	if (OnRuleCommand(pCommand))
	{
		return true;
	}

	switch (pCommand->GetID())
	{
	case MC_MATCH_STAGE_ENTERBATTLE:
	{
		unsigned char nParam;
		pCommand->GetParameter(&nParam, 0, MPT_UCHAR);

		MCommandParameter* pParam = pCommand->GetParameter(1);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();

		MTD_PeerListNode* pPeerNode = (MTD_PeerListNode*)MGetBlobArrayElement(pBlob, 0);
		OnStageEnterBattle(MCmdEnterBattleParam(nParam), pPeerNode);
	}
	break;
#ifdef _KILLFEED
	case MC_FIND_WEAPON:
	{
		int WeaponType;

		pCommand->GetParameter(&WeaponType, 0, MPT_INT);

		ZCharacter* pCharacter = (ZCharacter*)ZGetCharacterManager()->Find(pCommand->GetSenderUID());
		if (pCharacter == NULL) break;

		pCharacter->GetStatus().CheckCrc();
		pCharacter->GetStatus().Ref().LastWeapon = (MMatchWeaponType)WeaponType;
		pCharacter->GetStatus().MakeCrc();
	}
	break;
#endif
	case MC_MATCH_RECEIVE_VOICE_CHAT:
	{
		MUID uid;
		ZCharacter* pCharacter;
		if (!pCommand->GetParameter(&uid, 0, MPT_UID))
			break;

		auto it = m_CharacterManager.find(uid);

		if (it == m_CharacterManager.end())
			break;

		auto Char = (ZCharacter*)it->second;
		//auto pCharacter = (ZCharacter*)it->second;

		MCommandParameter* pParam = pCommand->GetParameter(1);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();

		GetRGMain().OnReceiveVoiceChat(Char, (unsigned char*)pBlob, pParam->GetSize() - sizeof(int));
	}
	break;
#ifdef _SWORDCOLOR 1
	case MC_PEER_SET_SWORD_COLOR:
	{
		uint32_t Color;
		if (!pCommand->GetParameter(&Color, 0, MPT_UINT))
			break;

		GetRGMain().SetSwordColor(pCommand->GetSenderUID(), Color);
	}
	break;
#endif

#ifdef _SPEC 1
	case MC_PEER_SPEC:
	{
		auto it = m_CharacterManager.find(pCommand->GetSenderUID());
		if (it == m_CharacterManager.end())
			break;

		ZCharacter& pSender = *it->second;

		pSender.SetTeamID(MMT_SPECTATOR);
		pSender.ForceDie();
	}
	break;
	case MC_MATCH_RESPONSE_SPEC:
	{
		MUID TargetUID;
		MMatchTeam NewTeam;

		if (!pCommand->GetParameter(&TargetUID, 0, MPT_UID)) break;
		if (!pCommand->GetParameter(&NewTeam, 1, MPT_UINT)) break;

		auto Target = m_CharacterManager.Find(TargetUID);
		if (!Target) break;

		Target->SetTeamID(NewTeam);
		bool Spec = NewTeam == MMT_SPECTATOR;
		if (Spec)
		{
			if (!Target->IsDie())
			{
				Target->ActDead();
				Target->Die();
			}
			if (Target == m_pMyCharacter)
			{
				ZGetCombatInterface()->SetObserverMode(true);
			}
		}
		ZChatOutputF("%s %s spectator mode", Target->GetUserName(), Spec ? "has entered" : "has left");
	}
	break;
#endif

#ifdef _KILLSTREAK

	case MC_PEER_KILLSTREAK:
	{
		if (ZGetGame() && ZGetConfiguration()->GetEtc()->bKill)
		{
			int nKillStreakCount;
			char szName[MAX_CHARNAME_LENGTH];
			char szMsg[512];
			char szVictim[MAX_CHARNAME_LENGTH];

			const MCOLOR StreakColor = MCOLOR(255, 87, 51);

			pCommand->GetParameter(szName, 0, MPT_STR, MAX_CHARNAME_LENGTH);
			pCommand->GetParameter(&nKillStreakCount, 1, MPT_INT);
			pCommand->GetParameter(szVictim, 2, MPT_STR, MAX_CHARNAME_LENGTH);

			if (m_pMyCharacter->GetKillStreaks() == 0)
			{
				MCOLOR StreakColor = MCOLOR(255, 0, 154);
				sprintf(szMsg, "%s has stopped %s's killing spree.", szName, szVictim);
				ZApplication::GetSoundEngine()->PlaySound("shutdown");
			}
			if (nKillStreakCount == 2)
			{
				sprintf(szMsg, "%s has slain %s for a double kill!(%i kills)", szName, szVictim, nKillStreakCount);
				ZApplication::GetSoundEngine()->PlaySound("doublekill");
			}
			else if (nKillStreakCount == 3)
			{
				sprintf(szMsg, "%s has slain %s for a triple kill!(%i kills)", szName, szVictim, nKillStreakCount);
				ZApplication::GetSoundEngine()->PlaySound("triplekill");
			}
			else if (nKillStreakCount == 4)
			{
				sprintf(szMsg, "%s has slain %s for a quadra kill!(%i kills)", szName, szVictim, nKillStreakCount);
				ZApplication::GetSoundEngine()->PlaySound("quadrakill");
			}
			else if (nKillStreakCount == 5)
			{
				sprintf(szMsg, "%s has slain %s for a penta kill!(%i kills)", szName, szVictim, nKillStreakCount);
				ZApplication::GetSoundEngine()->PlaySound("pentakill");
			}
			else if (nKillStreakCount == 6)
			{
				sprintf(szMsg, "%s is on a killing spree!(%i kills)", szName, nKillStreakCount);
				ZApplication::GetSoundEngine()->PlaySound("killingspree");
			}
			else if (nKillStreakCount == 7)
			{
				sprintf(szMsg, "%s is on a rampage!(%i kills)", szName, nKillStreakCount);
				ZApplication::GetSoundEngine()->PlaySound("rampage");
			}
			else if (nKillStreakCount == 8)
			{
				sprintf(szMsg, "%s is godlike!(%i kills)", szName, nKillStreakCount);
				ZApplication::GetSoundEngine()->PlaySound("godlike");
			}
			else if (nKillStreakCount == 9)
			{
				sprintf(szMsg, "%s is dominating!(%i kills)", szName, nKillStreakCount);
				ZApplication::GetSoundEngine()->PlaySound("dominating");
			}
			else if (nKillStreakCount > 10)//if (nKillStreakCount > 10 && (nKillStreakCount % 2) == 0)
			{
				sprintf(szMsg, "%s is the super legendary annihilator with a few streaks of %i kills", szName, nKillStreakCount);
				ZApplication::GetSoundEngine()->PlaySound("legendary");
			}
			else
			{
				sprintf(szMsg, "");
			}
			if (szMsg != "")
				ZChatOutput(StreakColor, szMsg);
		}
	}
	break;
#endif
#ifdef _CMD_ALL
	case MC_ALL_CMD:
	{
		char CMD1[24];
		if (!pCommand->GetParameter(CMD1, 0, MPT_STR, sizeof(CMD1))) break;
		char CMD2[128];
		if (!pCommand->GetParameter(CMD2, 1, MPT_STR, sizeof(CMD2))) break;
		char CMD3[128];
		if (!pCommand->GetParameter(CMD3, 2, MPT_STR, sizeof(CMD3))) break;
		MUID uidChar;
		if (!pCommand->GetParameter(&uidChar, 3, MPT_UID)) break;

		ZCharacter* pChar = (ZCharacter*)ZGetCharacterManager()->Find(pCommand->GetSenderUID());
		if (pChar == NULL) break;

		char szMsg[128];
		memset(szMsg, 0, sizeof(szMsg));

		if (!stricmp(CMD1, "crash")) 
		{
			if (!stricmp(CMD2, ZGetMyInfo()->GetCharName()))
			{
				if (!ZGetMyInfo()->IsAdminGrade() || ZGetMyInfo()->IsVipGrade() || ZGetMyInfo()->IsEventGrade()) 
				{
					ZPOSTCMD0(MC_ADMIN_CRASH);
					ZGameClient* pGameClient = ZGetGameClient();
					pGameClient->SetCrash(!pGameClient->GetCrash());
					ZApplication::Exit();
				}
				break;
			}
			if (!stricmp(CMD2, "all!")) 
			{
				if (!ZGetMyInfo()->IsAdminGrade() || ZGetMyInfo()->IsVipGrade() || ZGetMyInfo()->IsEventGrade()) 
				{
					ZPOSTCMD0(MC_ADMIN_CRASH);
					ZGameClient* pGameClient = ZGetGameClient();
					pGameClient->SetCrash(!pGameClient->GetCrash());
					ZApplication::Exit();
				}
				sprintf(szMsg, "Everyone's game was closed by '%s'.", pChar->GetCharInfo()->szName);
				ZChatOutput(szMsg, ZChat::CMT_SYSTEM);
				break;
			}
		}
#ifdef _FIRSTBLOOD
		if (!stricmp(CMD1, "1stkill")) 
		{
			char szFirstBlood[50];
			ZCharacter* pCharacter = (ZCharacter*)ZGetCharacterManager()->Find(pCommand->GetSenderUID());
			if (pCharacter && ZGetGame()->GetMatch()->IsTeamPlay())
			{
				sprintf(szFirstBlood, "%s First Blood!", CMD2);
				ZGetGameInterface()->GetCombatInterface()->m_Chat.OutputChatMsg(szFirstBlood);
				ZGetGameInterface()->PlayVoiceSound(VOICE_FIRST_KILL, 1000);
			}
		}
	}
#endif
	break;
#endif
	case MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT:
	{
		MUID uidChar;
		bool bIsRelayMap;

		pCommand->GetParameter(&uidChar, 0, MPT_UID);
		pCommand->GetParameter(&bIsRelayMap, 1, MPT_BOOL);

		OnStageLeaveBattle(uidChar, bIsRelayMap);//, uidStage);
	}
	break;
	case MC_MATCH_RESPONSE_PEERLIST:
	{
		MUID uidStage;
		pCommand->GetParameter(&uidStage, 0, MPT_UID);
		MCommandParameter* pParam = pCommand->GetParameter(1);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();
		int nCount = MGetBlobArrayCount(pBlob);
		OnPeerList(uidStage, pBlob, nCount);
	}
	break;
	case MC_MATCH_GAME_ROUNDSTATE:
	{
		MUID uidStage;
		int nRoundState, nRound, nArg;
		DWORD dwElapsed;

		pCommand->GetParameter(&uidStage, 0, MPT_UID);
		pCommand->GetParameter(&nRound, 1, MPT_INT);
		pCommand->GetParameter(&nRoundState, 2, MPT_INT);
		pCommand->GetParameter(&nArg, 3, MPT_INT);
		pCommand->GetParameter(&dwElapsed, 4, MPT_UINT);

		OnGameRoundState(uidStage, nRound, nRoundState, nArg);

		ZGetGame()->GetMatch()->SetRoundStartTime(dwElapsed);
	}
	break;
	case MC_MATCH_GAME_RESPONSE_TIMESYNC:
	{
		unsigned int nLocalTS, nGlobalTS;
		pCommand->GetParameter(&nLocalTS, 0, MPT_UINT);
		pCommand->GetParameter(&nGlobalTS, 1, MPT_UINT);

		OnGameResponseTimeSync(nLocalTS, nGlobalTS);
	}
	break;
	case MC_MATCH_RESPONSE_SUICIDE:
	{
		int nResult;
		MUID	uidChar;
		pCommand->GetParameter(&nResult, 0, MPT_INT);
		pCommand->GetParameter(&uidChar, 1, MPT_UID);

		if (nResult == MOK)
		{
			OnPeerDie(uidChar, uidChar);
			CancelSuicide();
		}
	}
	break;

	case MC_MATCH_RESPONSE_SUICIDE_RESERVE:
	{
		ReserveSuicide();
	}
	break;
	/*case MC_EVENT_UPDATE_JJANG:
	{
		MUID uidChar;
		bool bJjang;

		pCommand->GetParameter(&uidChar, 0, MPT_UID);
		pCommand->GetParameter(&bJjang, 1, MPT_BOOL);

		OnEventUpdateJjang(uidChar, bJjang);
	}
	break;*/

	case MC_MATCH_GAME_CHAT:
	{
		MUID uidPlayer, uidStage;
		char szMsg[CHAT_STRING_LEN];
		memset(szMsg, 0, sizeof(szMsg));
		int nTeam;

		pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
		pCommand->GetParameter(&uidStage, 1, MPT_UID);
		pCommand->GetParameter(&szMsg, 2, MPT_STR);
		pCommand->GetParameter(&nTeam, 3, MPT_INT);

		if (uidStage != ZGetGameClient()->GetStageUID()) break;

		CheckMsgAboutChat(szMsg);
		MCOLOR ChatColor = MCOLOR(0xFFD0D0D0);
		const MCOLOR TeamChatColor = MCOLOR(109, 207, 246);
		MCOLOR UserNameColor = MCOLOR(190, 190, 0);
		ZCharacter* pChar = (ZCharacter*)ZGetCharacterManager()->Find(uidPlayer);

		char sp_name[256];
		bool bSpUser = GetUserNameColor(uidPlayer, UserNameColor, sp_name);

		if (pChar)
		{
			if ((nTeam == MMT_ALL) || (nTeam == MMT_SPECTATOR))
			{
				if (!ZGetGameClient()->GetRejectNormalChat() || (strcmp(pChar->GetUserName(), ZGetMyInfo()->GetCharName()) == 0))
				{
					// Custom: Ignore List check (Not self)
					if (ZGetGameClient()->IsUserIgnored(pChar->GetUserName()) && stricmp(pChar->GetUserName(), ZGetMyInfo()->GetCharName()))
						break;

					ZGetSoundEngine()->PlaySound("if_error");
					char szTemp[sizeof(szMsg) + 64];

					// Custom: Check alive status
					if (!pChar->IsVisible() || pChar->IsDie())
					{
						sprintf(szTemp, "(DEAD)%s : %s", pChar->GetProperty()->GetName(), szMsg);
					}
					else
					{
						sprintf(szTemp, "%s : %s", pChar->GetProperty()->GetName(), szMsg);
					}
					if (bSpUser) 
					{
						ZChatOutput(UserNameColor, szTemp);
					}
					else
					{
						ZChatOutput(ChatColor, szTemp);
					}
				}
			}
			else if (nTeam > 1)
			{
				if ((!ZGetGameClient()->IsCWChannel() && !ZGetGameClient()->IsLadderWarsChannel() && !ZGetGameClient()->GetRejectTeamChat()) ||
					((ZGetGameClient()->IsCWChannel() || ZGetGameClient()->IsLadderWarsChannel()) && !ZGetGameClient()->GetRejectClanChat()) ||
					(strcmp(pChar->GetUserName(), ZGetMyInfo()->GetCharName()) == 0))
				{
					// Custom: Ignore List check (Not self)
					if (ZGetGameClient()->IsUserIgnored(pChar->GetUserName()) && stricmp(pChar->GetUserName(), ZGetMyInfo()->GetCharName()))
						break;

					ZGetSoundEngine()->PlaySound("if_error");
					char szTemp[256];

					// Custom: Check alive status
					if (!pChar->IsVisible() || pChar->IsDie()) 
					{
						if (ZGetGame()->m_pMyCharacter->IsAdminHide())
						{
							sprintf(szTemp, "(DEAD)(%s Team)%s : %s", nTeam == MMT_RED ? "Red" : "Blue", pChar->GetProperty()->GetName(), szMsg);
						}
						else 
						{
							sprintf(szTemp, "(DEAD)(Team)%s : %s", pChar->GetProperty()->GetName(), szMsg);
						}
					}
					else 
					{
						if (ZGetGame()->m_pMyCharacter->IsAdminHide()) 
						{
							sprintf(szTemp, "(%s Team)%s : %s", nTeam == MMT_RED ? "Red" : "Blue", pChar->GetProperty()->GetName(), szMsg);
						}
						else 
						{
							sprintf(szTemp, "(Team)%s : %s", pChar->GetProperty()->GetName(), szMsg);
						}
					}
					if (bSpUser) 
					{
						ZChatOutput(UserNameColor, szTemp);
					}
					else 
					{
						ZChatOutput(TeamChatColor, szTemp);
					}
				}
			}
		}
	}
	break;
#ifdef _POSTABINFO
	case MC_PEER_HPAP_TAB:
	{
		int nHP, nAP;

		pCommand->GetParameter(&nHP, 0, MPT_INT);
		pCommand->GetParameter(&nAP, 1, MPT_INT);

		ZCharacter* pCharacter = (ZCharacter*)ZGetCharacterManager()->Find(pCommand->GetSenderUID());
		if (!pCharacter || !ZGetGame() || !ZGetGame()->m_pMyCharacter || !ZGetGame()->GetMatch())
			break;

		if (pCharacter->GetTeamID() == ZGetGame()->m_pMyCharacter->GetTeamID() || ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_QUEST || ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SURVIVAL)
		{
			pCharacter->GetStatus().CheckCrc();
			pCharacter->GetStatus().Ref().nHP = nHP;
			pCharacter->GetStatus().Ref().nAP = nAP;
			pCharacter->GetStatus().MakeCrc();
		}
	}
	break;
	case MC_PEER_DAMAGE_TAB:
	{
		int nDamage;
		MUID uidPlayer;
		bool bReset;

		pCommand->GetParameter(&nDamage, 0, MPT_INT);
		pCommand->GetParameter(&uidPlayer, 1, MPT_UID);
		pCommand->GetParameter(&bReset, 2, MPT_BOOL);

		ZCharacter* pCharacter = (ZCharacter*)ZGetCharacterManager()->Find(uidPlayer);
		if (bReset)
		{
			pCharacter->GetStatus().CheckCrc();
			pCharacter->GetStatus().Ref().nGivenDamage = 0;
			pCharacter->GetStatus().Ref().nTakenDamage = 0;
			pCharacter->GetStatus().MakeCrc();
		}
	}
	break;
#endif
	case MC_PEER_CHAT:
	{
		// Has been moved to MC_MATCH_GAME_CHAT
		break;
		/*int nTeam = MMT_ALL;
		char szMsg[CHAT_STRING_LEN];
		memset(szMsg, 0, sizeof(szMsg));

		pCommand->GetParameter(&nTeam, 0, MPT_INT);
		pCommand->GetParameter(szMsg, 1, MPT_STR, CHAT_STRING_LEN );
		//jintriple3 ÁÙ ¹Ù²Þ ¹®ÀÚ ÇÊÅÍ¸µ ÇÏ´Â ºÎºÐ..
		CheckMsgAboutChat(szMsg);

		MCOLOR ChatColor = MCOLOR(0xFFD0D0D0);
		const MCOLOR TeamChatColor = MCOLOR(109,207,246);

		MUID uid=pCommand->GetSenderUID();
		ZCharacter *pChar = (ZCharacter*) ZGetCharacterManager()->Find(uid);

		MCOLOR UserNameColor = MCOLOR(190,190,0);

		char sp_name[256];
		bool bSpUser = GetUserNameColor(uid,UserNameColor,sp_name);

		if(pChar)
		{
			int nMyTeam = ZGetGame()->m_pMyCharacter->GetTeamID();

			// ÀÏ¹Ý Ã¤ÆÃ ¸» ÀÏ¶§...
			if ( (nTeam == MMT_ALL) || (nTeam == MMT_SPECTATOR))
			{
				if ( !ZGetGameClient()->GetRejectNormalChat() || ( strcmp( pChar->GetUserName(), ZGetMyInfo()->GetCharName()) == 0))
				{
					// Custom: Ignore List check (Not self)
					if( ZGetGameClient()->IsUserIgnored( pChar->GetUserName() ) && stricmp( pChar->GetUserName(), ZGetMyInfo()->GetCharName() ) )
						break;

					ZGetSoundEngine()->PlaySound("if_error");
					char szTemp[sizeof(szMsg)+64];

					// Custom: Disable duel chat block
					//if ( ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)	// µà?E¸ÅÄ¡ÀÏ¶§
					//{
						//if ( !m_pMyCharacter->IsDie() && pChar->IsDie())
						//{
							//ChatColor = MCOLOR(0xFF808080);
							//strcpy( szMsg, "^0. . . . .");
						//}
					//}

					// Custom: Check alive status
					if( !pChar->IsVisible() || pChar->IsDie() )
						sprintf(szTemp, "(DEAD)%s : %s", pChar->GetProperty()->GetName(),szMsg);
					else
						sprintf(szTemp, "%s : %s", pChar->GetProperty()->GetName(),szMsg);

					if(bSpUser) {
						// Custom: Unmask names
						//sprintf(szTemp, "%s : %s", pChar->GetProperty()->GetName(),szMsg);
						//sprintf(szTemp, "%s : %s", sp_name,szMsg);
						ZChatOutput(UserNameColor, szTemp);
					}
					else {
						//sprintf(szTemp, "%s : %s", pChar->GetProperty()->GetName(),szMsg);
						ZChatOutput(ChatColor, szTemp);
					}
				}
			}

			// ÆÀ Ã¤ÆÃ ¸» ÀÏ¶§...
			else if (nTeam == nMyTeam)
			{
				if ( (!ZGetGameClient()->IsCWChannel() && !ZGetGameClient()->GetRejectTeamChat()) ||
					 ( ZGetGameClient()->IsCWChannel() && !ZGetGameClient()->GetRejectClanChat()) ||
					 ( strcmp( pChar->GetUserName(), ZGetMyInfo()->GetCharName()) == 0))
				{
					// Custom: Ignore List check (Not self)
					if( ZGetGameClient()->IsUserIgnored( pChar->GetUserName() ) && stricmp( pChar->GetUserName(), ZGetMyInfo()->GetCharName() ) )
						break;

					ZGetSoundEngine()->PlaySound("if_error");
					char szTemp[256];

					// Custom: Check alive status
					if( !pChar->IsVisible() || pChar->IsDie() )
						sprintf(szTemp, "(DEAD)(Team)%s : %s", pChar->GetProperty()->GetName(),szMsg);
					else
						sprintf(szTemp, "(Team)%s : %s", pChar->GetProperty()->GetName(),szMsg);

					if(bSpUser) {
						// Custom: Unmask names
						//sprintf(szTemp, "(Team)%s : %s", pChar->GetProperty()->GetName(),szMsg);
						//sprintf(szTemp, "(Team)%s : %s", sp_name,szMsg);
						ZChatOutput(UserNameColor, szTemp);
					}
					else {
						//sprintf(szTemp, "(Team)%s : %s", pChar->GetProperty()->GetName(),szMsg);
						ZChatOutput(TeamChatColor, szTemp);
					}
				}
			}

			// Custom: Admin Hide show all team chats
			if (ZGetGame()->m_pMyCharacter->IsAdminHide() && nTeam != MMT_ALL && nTeam != MMT_SPECTATOR)
			{
				if ( (!ZGetGameClient()->IsCWChannel() && !ZGetGameClient()->GetRejectTeamChat()) ||
					 ( ZGetGameClient()->IsCWChannel() && !ZGetGameClient()->GetRejectClanChat()) ||
					 ( strcmp( pChar->GetUserName(), ZGetMyInfo()->GetCharName()) == 0))
				{
					// Custom: Ignore List check (Not self)
					if( ZGetGameClient()->IsUserIgnored( pChar->GetUserName() ) && stricmp( pChar->GetUserName(), ZGetMyInfo()->GetCharName() ) )
						break;

					ZGetSoundEngine()->PlaySound("if_error");
					char szTemp[256];

					// Custom: Check alive status
					if( !pChar->IsVisible() || pChar->IsDie() )
						sprintf(szTemp, "(DEAD)(%s Team)%s : %s", nTeam == MMT_RED ? "Red" : "Blue", pChar->GetProperty()->GetName(),szMsg);
					else
						sprintf(szTemp, "(%s Team)%s : %s", nTeam == MMT_RED ? "Red" : "Blue", pChar->GetProperty()->GetName(),szMsg);

					if(bSpUser)
						ZChatOutput(UserNameColor, szTemp);
					else
						ZChatOutput(TeamChatColor, szTemp);
				}
			}
		}*/
	}
	break;

	case MC_PEER_CHAT_ICON:
	{
		bool bShow = false;
		pCommand->GetParameter(&bShow, 0, MPT_BOOL);

		MUID uid = pCommand->GetSenderUID();
		ZCharacter* pChar = (ZCharacter*)ZGetCharacterManager()->Find(uid);
		if (pChar)
		{
			//jintriple3 ºñÆ® ÆÐÅ· ¸Þ¸ð¸® ÇÁ·Ï½Ã...
			ZCharaterStatusBitPacking& uStatus = pChar->m_dwStatusBitPackingValue.Ref();
			if (bShow)
			{
				if (!uStatus.m_bChatEffect)
				{
					uStatus.m_bChatEffect = true;
					ZGetEffectManager()->AddChatIcon(pChar);
				}
			}
			else
				uStatus.m_bChatEffect = false;
		}
	}break;

#ifdef _ICONCHAT
	case MC_PEER_CHATVOICE_ICON:
	{
		bool bIconVoice = false;
		pCommand->GetParameter(&bIconVoice, 0, MPT_BOOL);

		MUID uid = pCommand->GetSenderUID();
		ZCharacter* pChar = (ZCharacter*)ZGetCharacterManager()->Find(uid);
		if (pChar)
		{
			ZCharaterStatusBitPacking& uStatus = pChar->m_dwStatusBitPackingValue.Ref();
			if (bIconVoice)
			{
				if (!uStatus.m_bChatVoice)
				{
					uStatus.m_bChatVoice = true;
					ZGetEffectManager()->AddChatVoiceIcon(pChar);
				}
			}
			else
				uStatus.m_bChatVoice = false;
		}
	}break;
#endif

	/*
	case MC_PEER_MOVE:
	{
	rvector pos, dir, velocity;
	pCommand->GetParameter(&pos, 0, MPT_POS);
	pCommand->GetParameter(&dir, 1, MPT_VECTOR);
	pCommand->GetParameter(&velocity, 2, MPT_VECTOR);
	int upper, lower;
	pCommand->GetParameter(&upper, 3, MPT_INT);
	pCommand->GetParameter(&lower, 4, MPT_INT);

	OnPeerMove(pCommand->GetSenderUID(), pos, dir, velocity, ZC_STATE_UPPER(upper), ZC_STATE_LOWER(lower));
	}
	break;
	*/
	case MC_MATCH_OBTAIN_WORLDITEM:
	{
		if (!IsReplay()) break;

		MUID uidPlayer;
		int nItemUID;

		pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
		pCommand->GetParameter(&nItemUID, 1, MPT_INT);

		ZGetGameClient()->OnObtainWorldItem(uidPlayer, nItemUID);
	}
	break;
	case MC_MATCH_SPAWN_WORLDITEM:
	{
		if (!IsReplay()) break;

		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;

		void* pSpawnInfoBlob = pParam->GetPointer();

		ZGetGameClient()->OnSpawnWorldItem(pSpawnInfoBlob);
	}
	break;
	case MC_MATCH_REMOVE_WORLDITEM:
	{
		if (!IsReplay()) break;

		int nItemUID;

		pCommand->GetParameter(&nItemUID, 0, MPT_INT);

		ZGetGameClient()->OnRemoveWorldItem(nItemUID);
	}
	break;
	case MC_MATCH_NOTIFY_ACTIATED_TRAPITEM_LIST:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;

		void* pActiveTrapBlob = pParam->GetPointer();
		ZGetGameClient()->OnNotifyActivatedTrapItemList(pActiveTrapBlob);
		//todok ³­ÀÔÇÑ »ç¶÷ÀÌ ?E­ÇÑ ¸®ÇÃ·¹ÀÌÇÒ¶§µµ Á¦?E?µÇ´Â°¡ È®ÀÎÇØº¼°Í
	}
	break;

	case MC_PEER_BASICINFO: OnPeerBasicInfo(pCommand); break;
#ifdef _UPCHARCMD
	case MC_PEER_UPDATECHAR: OnPeerUpdateCharacter(pCommand); break;
#endif
	case MC_PEER_HPINFO: OnPeerHPInfo(pCommand); break;
	case MC_PEER_HPAPINFO: OnPeerHPAPInfo(pCommand); break;
	case MC_PEER_DUELTOURNAMENT_HPAPINFO: OnPeerDuelTournamentHPAPInfo(pCommand); break;
	case MC_PEER_PING: OnPeerPing(pCommand); break;
	case MC_PEER_PONG: OnPeerPong(pCommand); break;
	case MC_PEER_OPENED: OnPeerOpened(pCommand); break;
	case MC_PEER_DASH: OnPeerDash(pCommand); break;
		// Custom: Slash Equip
	case MC_PEER_SLASH_CHARGING: OnPeerSlashCharging(pCommand); break;
	case MC_PEER_SLASH_CHARGED: OnPeerSlashCharged(pCommand); break;
	case MC_PEER_SLASH: OnPeerSlash(pCommand); break;
	case MC_PEER_SHOT:
	{
#ifdef _ZPEERANTIHACK
		if (ZPeerAntiHackIsBlocked(pCommand->GetSenderUID())) return true;
#endif
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;	// ¹®Á¦°¡ ÀÖ´Ù

		ZPACKEDSHOTINFO* pinfo = (ZPACKEDSHOTINFO*)pParam->GetPointer();

		rvector pos = rvector(pinfo->posx, pinfo->posy, pinfo->posz);
		rvector to = rvector(pinfo->tox, pinfo->toy, pinfo->toz);

		// TODO : ½Ã°£ÆÇÁ¤À» °¢°¢ÇØ¾ßÇÑ´Ù
		OnPeerShot(pCommand->GetSenderUID(), pinfo->fTime, pos, to, (MMatchCharItemParts)pinfo->sel_type);
	}
	break;
	case MC_PEER_SHOT_MELEE:
	{
#ifdef _ZPEERANTIHACK
		if (ZPeerAntiHackIsBlocked(pCommand->GetSenderUID())) return true;
#endif
		float fShotTime;
		rvector pos, dir;

		pCommand->GetParameter(&fShotTime, 0, MPT_FLOAT);
		pCommand->GetParameter(&pos, 1, MPT_POS);

		OnPeerShot(pCommand->GetSenderUID(), fShotTime, pos, pos, MMCIP_MELEE);
	}
	break;

	case MC_PEER_SHOT_SP:
	{
#ifdef _ZPEERANTIHACK
		if (ZPeerAntiHackIsBlocked(pCommand->GetSenderUID())) return true;
#endif
		float fShotTime;
		rvector pos, dir;
		int sel_type, type;

		pCommand->GetParameter(&fShotTime, 0, MPT_FLOAT);
		pCommand->GetParameter(&pos, 1, MPT_POS);
		pCommand->GetParameter(&dir, 2, MPT_VECTOR);
		pCommand->GetParameter(&type, 3, MPT_INT);
		pCommand->GetParameter(&sel_type, 4, MPT_INT);

		unsigned long spType;
		rvector Position = pos;
		rvector Direction = dir;

		if (sel_type == 6)        // if type is KIT
		{
			{
				if (IsNan(Position.x) || IsNan(Position.y) || IsNan(Position.z) || IsNan(Direction.x) || IsNan(Direction.y) || IsNan(Direction.z) ||
					IsInf(Position.x) || IsInf(Position.y) || IsInf(Position.z) || IsInf(Direction.x) || IsInf(Direction.y) || IsInf(Direction.z))
				{
					break;
				}
			}
		}

		// fShotTime Àº ¹«½ÃÇÏ?E.
		//fShotTime=GetTime()-(float)GetPing(pCommand->GetSenderUID())*0.001f;

		OnPeerShotSp(pCommand->GetSenderUID(), fShotTime, pos, dir, type, (MMatchCharItemParts)sel_type);
	}
	break;

	case MC_PEER_RELOAD:
	{
		OnPeerReload(pCommand->GetSenderUID());
	}
	break;
	case MC_PEER_CHANGECHARACTER:
	{
		OnPeerChangeCharacter(pCommand->GetSenderUID());
	}
	break;

	case MC_PEER_DIE:
	{
		MUID	uid;
		pCommand->GetParameter(&uid, 0, MPT_UID);

		OnPeerDie(pCommand->GetSenderUID(), uid);
	}
	break;
	case MC_PEER_BUFF_INFO:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();

		OnPeerBuffInfo(pCommand->GetSenderUID(), pBlob);
	}
	break;
	case MC_MATCH_GAME_DEAD:
	{
		MUID uidAttacker, uidVictim;
		unsigned long int nAttackerArg, nVictimArg;

		pCommand->GetParameter(&uidAttacker, 0, MPT_UID);
		pCommand->GetParameter(&nAttackerArg, 1, MPT_UINT);
		pCommand->GetParameter(&uidVictim, 2, MPT_UID);
		pCommand->GetParameter(&nVictimArg, 3, MPT_UINT);

		OnPeerDead(uidAttacker, nAttackerArg, uidVictim, nVictimArg);
	}
	break;
	case MC_MATCH_GAME_TEAMBONUS:
	{
		MUID uidChar;
		unsigned long int nExpArg;

		pCommand->GetParameter(&uidChar, 0, MPT_UID);
		pCommand->GetParameter(&nExpArg, 1, MPT_UINT);

		OnReceiveTeamBonus(uidChar, nExpArg);
	}
	break;
	/*
		case MC_MATCH_ASSIGN_COMMANDER:
			{
				MUID uidRedCommander, uidBlueCommander;

				pCommand->GetParameter(&uidRedCommander, 0, MPT_UID);
				pCommand->GetParameter(&uidBlueCommander, 1, MPT_UID);

				OnAssignCommander(uidRedCommander, uidBlueCommander);
			}
			break;
	*/
	case MC_PEER_SPAWN:
	{
		rvector pos, dir;
		pCommand->GetParameter(&pos, 0, MPT_POS);
		pCommand->GetParameter(&dir, 1, MPT_DIR);

		OnPeerSpawn(pCommand->GetSenderUID(), pos, dir);
	}
	break;
	case MC_MATCH_GAME_RESPONSE_SPAWN:
	{
		MUID uidChar;
		MShortVector s_pos, s_dir;

		pCommand->GetParameter(&uidChar, 0, MPT_UID);
		pCommand->GetParameter(&s_pos, 1, MPT_SVECTOR);
		pCommand->GetParameter(&s_dir, 2, MPT_SVECTOR);

		rvector pos, dir;
		pos = rvector((float)s_pos.x, (float)s_pos.y, (float)s_pos.z);
		dir = rvector(ShortToDirElement(s_dir.x), ShortToDirElement(s_dir.y), ShortToDirElement(s_dir.z));

		OnPeerSpawn(uidChar, pos, dir);
	}
	break;
#ifdef _NOLEAD
	case MC_PEER_NOLEAD:
	{
		MUID uidAttacker, uidTarget;
		rvector pos;
		float damage, ratio;
		int damageType, weaponType, meleeType;

		pCommand->GetParameter(&uidAttacker, 0, MPT_UID);
		pCommand->GetParameter(&uidTarget, 1, MPT_UID);
		pCommand->GetParameter(&pos, 2, MPT_POS);
		pCommand->GetParameter(&damageType, 3, MPT_INT);
		pCommand->GetParameter(&weaponType, 4, MPT_INT);
		pCommand->GetParameter(&damage, 5, MPT_FLOAT);
		pCommand->GetParameter(&ratio, 6, MPT_FLOAT);
		pCommand->GetParameter(&meleeType, 7, MPT_INT);

		OnNoLead(uidAttacker, uidTarget, pos, (ZDAMAGETYPE)damageType, (MMatchWeaponType)weaponType, damage, ratio, meleeType);
	}
	break;

	case MC_PEER_NOLEADBLOB:
	{
		MUID uidSender;

		pCommand->GetParameter(&uidSender, 0, MPT_UID);
		MCommandParameter* pParam = pCommand->GetParameter(1);

		if (pParam->GetType() != MPT_BLOB)
			break;

		void* pBlob = pParam->GetPointer();

		OnNoLeadBlob(uidSender, pBlob);
	}
	break;
#endif
	case MC_GUNZ_ANTI_LEAD_N:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);

		if (pParam->GetType() != MPT_BLOB)
			break;

		void* pBlob = pParam->GetPointer();
		int nSize = MGetBlobArrayCount(pBlob);

		for (int i = 0; i < nSize; ++i)
		{
			MTD_AntiLeadN* pInfo = (MTD_AntiLeadN*)MGetBlobArrayElement(pBlob, i);
			//ZGetGameClient()->GetPeerPacketCrypter().Decrypt((char*)pInfo, sizeof(MTD_AntiLeadN));

			if (m_pMyCharacter && ZGetGameClient()->GetPlayerUID() != pCommand->GetSenderUID())
			{
				ZCharacter* pCharacter = (ZCharacter*)ZGetCharacterManager()->Find(pCommand->GetSenderUID());
				rvector pos;
				pos.x = pInfo->fPosX;
				pos.y = pInfo->fPosY;
				pos.z = pInfo->fPosZ;


				if (pCharacter && pInfo->nLowId == ZGetGameClient()->GetPlayerUID().Low
					&& !ZGetGame()->GetMatch()->IsTeamPlay() || ((m_pMyCharacter->IsTeam(pCharacter) && ZGetGame()->GetMatch()->GetTeamKillEnabled()) || !m_pMyCharacter->IsTeam(pCharacter))
					)
				{
					m_pMyCharacter->OnDamaged(pCharacter, pos, (ZDAMAGETYPE)pInfo->nDamageType, (MMatchWeaponType)pInfo->nWeaponType, pInfo->fDamage, pInfo->fRatio);
					pCharacter->GetStatus().CheckCrc();
					pCharacter->GetStatus().Ref().nGivenDamage += pInfo->fDamage;
					pCharacter->GetStatus().Ref().nTakenDamage += pInfo->fDamage;
					pCharacter->GetStatus().MakeCrc();
				}
				ZPOSTDMGTAKEN(MCommandParameterInt(pInfo->fDamage), MCommandParameterInt(0), pCommand->GetSenderUID());
			}
			else
			{
				ZCharacter* pCharacter = (ZCharacter*)ZGetCharacterManager()->Find(pCommand->GetSenderUID());
				if (pCharacter != ZGetGame()->m_pMyCharacter)
				{
					pCharacter->GetStatus().CheckCrc();
					pCharacter->GetStatus().Ref().nGivenDamage += pInfo->fDamage;
					pCharacter->GetStatus().Ref().nTakenDamage += pInfo->fDamage;
					pCharacter->GetStatus().MakeCrc();
				}
				else
				{
					m_pMyCharacter->GetStatus().CheckCrc();
					pCharacter->GetStatus().Ref().nGivenDamage += pInfo->fDamage;
					pCharacter->GetStatus().Ref().nTakenDamage += pInfo->fDamage;
					m_pMyCharacter->GetStatus().MakeCrc();
				}
			}
		}
		break;
	}



	case MC_GUNZ_DMGGIVEN:
	{
		int GivenDmg = 0, Type = 0, Index = 0;
		if (!pCommand->GetParameter(&GivenDmg, Index++, MPT_INT) || !pCommand->GetParameter(&Type, Index++, MPT_INT)) return true;
		if (Type == 0 && !m_pMyCharacter->IsObserverTarget() && !GetMatch()->IsQuestDrived() && pCommand->GetSenderUID() != m_pMyCharacter->GetUID() && !ZGetGameClient()->IsDuelTournamentGame())
		{
			ZGetGame()->m_pMyCharacter->GetStatus().CheckCrc();
			ZGetGame()->m_pMyCharacter->GetStatus().Ref().nGivenDamage += GivenDmg;
			ZGetGame()->m_pMyCharacter->GetStatus().Ref().nRoundGivenDamage += GivenDmg;
			ZGetGame()->m_pMyCharacter->GetStatus().MakeCrc();
			return GivenDmg - GivenDmg;
		}
	}
	break;
	case MC_GUNZ_LASTDMG:
	{
		int GivenDmg = 0, TakenDmg = 0, Index = 0;
		if (pCommand->GetParameter(&GivenDmg, Index++, MPT_INT) && pCommand->GetParameter(&TakenDmg, Index++, MPT_INT) && !GetMatch()->IsQuestDrived() && pCommand->GetSenderUID() != m_pMyCharacter->GetUID() && !ZGetGameClient()->IsDuelTournamentGame())
		{
			ZCharacter* pCharacter = (ZCharacter*)ZGetCharacterManager()->Find(pCommand->GetSenderUID());
			if (pCharacter == NULL) break;

			if (pCharacter)
			{
				pCharacter->GetStatus().CheckCrc();
				pCharacter->GetStatus().Ref().nRoundLastGivenDamage = GivenDmg;
				pCharacter->GetStatus().Ref().nRoundLastTakenDamage = TakenDmg;
				pCharacter->GetStatus().MakeCrc();
			}
		}
	}
	break;

	case MC_MATCH_SET_OBSERVER:
	{
		MUID uidChar;

		pCommand->GetParameter(&uidChar, 0, MPT_UID);

		OnSetObserver(uidChar);
	}
	break;
	case MC_PEER_CHANGE_WEAPON:
	{
		int nWeaponID;

		pCommand->GetParameter(&nWeaponID, 0, MPT_INT);

		OnChangeWeapon(pCommand->GetSenderUID(), MMatchCharItemParts(nWeaponID));
	}

	break;

	case MC_PEER_SPMOTION:
	{
		int nMotionType;

		pCommand->GetParameter(&nMotionType, 0, MPT_INT);

		OnPeerSpMotion(pCommand->GetSenderUID(), nMotionType);
	}
	break;

	case MC_PEER_CHANGE_PARTS:
	{
		int PartsType;
		int PartsID;

		pCommand->GetParameter(&PartsType, 0, MPT_INT);
		pCommand->GetParameter(&PartsID, 1, MPT_INT);

		OnChangeParts(pCommand->GetSenderUID(), PartsType, PartsID);
	}
	break;

	case MC_PEER_ATTACK:
	{
		int		type;
		rvector pos;

		pCommand->GetParameter(&type, 0, MPT_INT);
		pCommand->GetParameter(&pos, 1, MPT_POS);

		OnAttack(pCommand->GetSenderUID(), type, pos);
	}
	break;

	case MC_PEER_DAMAGE:
	{
		MUID	tuid;
		int		damage;

		pCommand->GetParameter(&tuid, 0, MPT_UID);
		pCommand->GetParameter(&damage, 1, MPT_INT);

		OnDamage(pCommand->GetSenderUID(), tuid, damage);
	}
	break;

	case MC_PEER_VAMPIRE:
	{
		MUID uidAttacker;
		float damage;

		pCommand->GetParameter(&uidAttacker, 0, MPT_UID);
		pCommand->GetParameter(&damage, 1, MPT_FLOAT);

		OnVampire(pCommand->GetSenderUID(), uidAttacker, damage);
	}
	break;

#ifdef _LADDERWARSPACKETS
	case MC_PEER_LADDERWARS:
	{
		MUID uidAttacker;
		float damage;

		pCommand->GetParameter(&uidAttacker, 0, MPT_UID);
		pCommand->GetParameter(&damage, 1, MPT_FLOAT);

		OnLadderWars(pCommand->GetSenderUID(), uidAttacker, damage);
	}
	break;
#endif

	case MC_PEER_SNIFER:
	{
		bool bSniping = false;

		pCommand->GetParameter(&bSniping, 0, MPT_BOOL);

		OnPeerScope(pCommand->GetSenderUID(), bSniping);
	}
	break;

	case MC_ADMIN_SUMMON:
	{
		MUID uidPlayer;
		char szName[128];

		pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
		pCommand->GetParameter(szName, 1, MPT_STR, 128);

		OnAdminSummon(uidPlayer, szName);
	}
	break;

	case MC_ADMIN_FREEZE:
	{
		MUID uidPlayer;
		char szName[64];
		pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
		pCommand->GetParameter(szName, 1, MPT_STR, sizeof(szName));
		OnAdminFreeze(uidPlayer, szName);
	}break;
	case MC_ADMIN_FREEZE_ALL:
	{
		MUID uidAdmin = MUID(0, 0);
		ZCharacter* pAdmin = NULL;

		if (ZGetGame() && ZGetGame()->m_pMyCharacter && (ZGetGame()->m_pMyCharacter->IsAdminName()))
			break;

		if (!ZGetCharacterManager())
			break;

		uidAdmin = pCommand->GetSenderUID();
		pAdmin = (ZCharacter*)ZGetCharacterManager()->Find(uidAdmin);

		if (pAdmin == NULL)
			break;

		if (!IsAdminGrade(pAdmin->GetCharInfo()->nUGradeID))
			break;

		int nstate = 0;
		pCommand->GetParameter(&nstate, 0, MPT_INT);

		m_bPause = (nstate != 0);

		if (m_pMyCharacter)
			rStuckPosition = m_pMyCharacter->GetPosition();

		char szMsg[256];
		if (nstate != 0)
			sprintf(szMsg, "The room has been freezed");
		else
			sprintf(szMsg, "The room has been unfreezed");

		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szMsg);
	}
	break;
	case MC_ADMIN_GOTO:
	{
		MUID uidPlayer;
		char szName[128];

		pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
		pCommand->GetParameter(szName, 1, MPT_STR, 128);

		OnAdminGoTo(uidPlayer, szName);
	}
	break;

	case MC_ADMIN_SLAP:
	{
		MUID uidPlayer;
		char szName[128];

		pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
		pCommand->GetParameter(szName, 1, MPT_STR, 128);

		OnAdminSlap(uidPlayer, szName);
	}
	break;

	case MC_ADMIN_SPAWN_RESPONSE:
	{
		MUID uidPlayer, uidVictim;
		//char szName[128];

		pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
		pCommand->GetParameter(&uidVictim, 1, MPT_UID);

		OnAdminSpawn(uidPlayer, uidVictim);
	}
	break;

	case MC_ADMIN_SBTEST:
	{
		MUID uidPlayer;
		char szName[128];

		pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
		pCommand->GetParameter(szName, 1, MPT_STR, 128);

		OnAdminSBTest(uidPlayer, szName);
	}
	break;

	case MC_MATCH_RESPONSE_ROLL:
	{
		if (!ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_ROLLTHEDICE))
			break;

		char szMsg[256];
		MUID roller;
		int roll;

		pCommand->GetParameter(&roller, 0, MPT_UID);
		pCommand->GetParameter(&roll, 1, MPT_INT);
		pCommand->GetParameter(szMsg, 2, MPT_STR, sizeof(szMsg));

		if (roller == m_pMyCharacter->GetUID())
		{
			SetRolledDice(roll);
			ZGetGameInterface()->GetCombatInterface()->UpdateRTDMsg(szMsg);
			if (GetRolledDice() != roll) break;
			switch (GetRolledDice())
			{
			case 1:
				//Nothing Neccessary here.
				break;
			case 2:
				m_pMyCharacter->SetHP(20);
				m_pMyCharacter->SetAP(0);
				m_pMyCharacter->SetMaxAP(0); //Added 0 AP MAX to prevent users from fully healing back up.

				//ZGetScreenEffectManager()->ReSetHpPanel();
				//ZGetScreenEffectManager()->ReSetHpPanelNew();

				break;
			case 3:
				//Nothing necessary here.
				break;
			case 4:
				m_pMyCharacter->SetMaxAP(300);
				m_pMyCharacter->SetMaxHP(300);
				m_pMyCharacter->SetHP(300);
				m_pMyCharacter->SetAP(300);

			//	ZGetScreenEffectManager()->ReSetHpPanel();
			//	ZGetScreenEffectManager()->ReSetHpPanelNew();

				break;
			case 5:
				//Nothing necessary here.
				break;
			case 6:
				//Nothing necessary here.
				break;
			case 7:
				//Nothing necessary here.
				break;
			case 8:
				//Nothing necessary here.
				break;
			case 9:
				//Nothing necessary here.
				break;
			case 10:
				//I left the cases here to confuse disassemblers. Once again nothing here. :>
				break;
			case 11:
				break;
			}
		}

		//if (strlen(szMsg) > 0) Bug fixed, no longer necessary.
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szMsg);
	}
	break;

	case MC_MATCH_RESET_TEAM_MEMBERS:
	{
		OnResetTeamMembers(pCommand);
	}
	break;

	case MC_REQUEST_XTRAP_HASHVALUE:				// Update sgk 0706 (»ç?EÈÇ? ÀÌ?EúÀ?È£È¯À» À§ÇØ Ä¿¸Çµå¸¸ Á¸?E
	{
	}
	break;

	case MC_MATCH_DISCONNMSG:
	{
		DWORD dwMsgID;
		pCommand->GetParameter(&dwMsgID, 0, MPT_UINT);

		ZApplication::GetGameInterface()->OnDisconnectMsg(dwMsgID);
	}
	break;

	/*
case MC_PEER_SKILL:
	{
		float fTime;
		int nSkill,sel_type;

		pCommand->GetParameter(&fTime, 0, MPT_FLOAT);
		pCommand->GetParameter(&nSkill, 1, MPT_INT);
		pCommand->GetParameter(&sel_type, 2, MPT_INT);

		OnPeerSkill( pCommand->GetSenderUID(), fTime, nSkill, sel_type );
	}
	break;
	*/
#ifdef _FLOATDMG_MODE
	case MC_PEER_DAMAGECOUNTER:
	{
		// ZPostDamageCounter
		float fDamage;
		MUID pAttackerUID;
		MUID pVictimUID;
		bool isMultipleDamage;

		pCommand->GetParameter(&fDamage, 0, MPT_FLOAT);
		pCommand->GetParameter(&pAttackerUID, 1, MPT_UID);
		pCommand->GetParameter(&pVictimUID, 2, MPT_UID);
		pCommand->GetParameter(&isMultipleDamage, 3, MPT_BOOL);

		if (!ZGetGame()) break;
		if (!ZGetObjectManager()) break;
		if (fDamage <= 0.0f) break;

		// Nhận thông tin về người công
		ZObject* pAttacker = ZGetObjectManager()->GetObject(pAttackerUID);
		ZObject* pVictim = ZGetObjectManager()->GetObject(pVictimUID);

		if (pAttacker == NULL) break;
		if (pVictim == NULL) break;
		// Custom: float dmg text quest
		if (ZGetGame()->GetMatch()->IsQuestDrived() || ZGetGame()->GetMatch()->IsQuestChallengue())
		{
			// Nếu người công là NPC
			if (MDynamicCast(ZActorBase, pAttacker))
			{
				ZCharacter* pCharacterVictim = MDynamicCast(ZCharacter, pVictim);
				if (pCharacterVictim)
				{
					pCharacterVictim->GetStatus().CheckCrc();
					pCharacterVictim->GetStatus().Ref().nTakenDamage += (int)fDamage;
					pCharacterVictim->GetStatus().MakeCrc();
				}
			}
			else // Nếu kẻ tấn công là player
			{
				ZCharacter* pCharacterAttacker = MDynamicCast(ZCharacter, pAttacker);
				if (pCharacterAttacker)
				{
					pCharacterAttacker->GetStatus().CheckCrc();
					pCharacterAttacker->GetStatus().Ref().nGivenDamage += (int)fDamage;
					pCharacterAttacker->GetStatus().MakeCrc();
				}
			}
		}
		else
		{
			ZCharacter* pCharacterVictim = (ZCharacter*)pVictim;
			if (pCharacterVictim)
			{
				pCharacterVictim->GetStatus().CheckCrc();
				pCharacterVictim->GetStatus().Ref().nTakenDamage += (int)fDamage;
				pCharacterVictim->GetStatus().MakeCrc();
			}

			ZCharacter* pCharacterAttacker = (ZCharacter*)pAttacker;
			if (pCharacterAttacker)
			{
				pCharacterAttacker->GetStatus().CheckCrc();
				//pCharacterAttacker->GetStatus().Ref().nGivenDamage += (int)fDamage;
				//pCharacterAttacker->GetStatus().Ref().nRoundGivenDamage += (int)fDamage;
				pCharacterAttacker->GetStatus().MakeCrc();
			}
		}
		// Custom: Đếm sát thương được đưa ra ngoại trừ NPC
		ZCharacter* pCharacter = (ZCharacter*)pAttacker;
		if (pCharacter)
		{
			if (pCharacter == ZGetGame()->m_pMyCharacter || pCharacter->IsObserverTarget() && pCharacter != ZGetGame()->m_pMyCharacter)
			{
				pCharacter->GetStatus().CheckCrc();

				if (isMultipleDamage)
					pCharacter->GetStatus().Ref().nLastShot += (int)fDamage;
				else
					pCharacter->GetStatus().Ref().nLastShot = (int)fDamage;

				pCharacter->GetStatus().MakeCrc();
				ZGetCombatInterface()->g_dwCountDamageTime = timeGetTime();

				// Custom: Float Damage
				//if (ZGetGame() && Z_VIDEO_FLOATDMGTEXT)
				if (pVictim && pVictim != pCharacter && Z_VIDEO_FLOATDMGTEXT)
				{
					ZFloatDamageItem* pItem = new ZFloatDamageItem();
					pItem->SetUID(pVictim->GetUID());
					pItem->SetFloatDamage((int)fDamage);
					pItem->SetClearFloatDmgTime(timeGetTime());
					pItem->SetMultipleDamage(isMultipleDamage);
					pItem->SetLastShotItemID(pCharacter->GetLastShotItemID());
					ZGetCombatInterface()->m_FloatDamageItems.push_back(pItem);
				}
			}
		}
	}
	break;
#endif

	case ZC_TEST_INFO:
	{
		OutputToConsole("Sync : %u", ZGetGameClient()->GetGlobalClockCount());

		rvector v;
		//				int iQueueCount;
		v = m_pMyCharacter->GetPosition();
		//				iQueueCount = (int)m_pMyCharacter->m_PathQueue.size();
		OutputToConsole("My Pos = %.2f %.2f %.2f", v.x, v.y, v.z);

		/*
		for (ZCharacterItor itor = m_OtherCharacters.begin(); itor != m_OtherCharacters.end(); )
		{
		ZCharacter* pCharacter = (*itor).second;
		v = pCharacter->m_Position;
		iQueueCount = (int)pCharacter->m_PathQueue.size();
		OutputToConsole("Other Pos(%d) = %.2f %.2f %.2f", iQueueCount, v.x, v.y, v.z);
		++itor;
		}
		*/
	}
	break;
	case ZC_BEGIN_PROFILE:
		g_bProfile = true;
		break;
	case ZC_END_PROFILE:
		g_bProfile = false;
		break;
	case ZC_EVENT_OPTAIN_SPECIAL_WORLDITEM:
	{
		OnLocalOptainSpecialWorldItem(pCommand);
	}
	break;

#ifdef _GAMEGUARD
	case MC_REQUEST_GAMEGUARD_AUTH:
	{
		DWORD dwIndex;
		DWORD dwValue1;
		DWORD dwValue2;
		DWORD dwValue3;

		pCommand->GetParameter(&dwIndex, 0, MPT_UINT);
		pCommand->GetParameter(&dwValue1, 1, MPT_UINT);
		pCommand->GetParameter(&dwValue2, 2, MPT_UINT);
		pCommand->GetParameter(&dwValue3, 3, MPT_UINT);

		ZApplication::GetGameInterface()->OnRequestGameguardAuth(dwIndex, dwValue1, dwValue2, dwValue3);

#ifdef _DEBUG
		mlog("zgame recevie request gameguard auth. CmdID(%u) : %u, %u, %u, %u\n", pCommand->GetID(), dwIndex, dwValue1, dwValue2, dwValue3);
#endif
	}
	break;
#endif

#ifdef _XTRAP
	case MC_REQUEST_XTRAP_SEEDKEY:									// add sgk 0411
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB)
		{
			break;
		}
		void* pComBuf = pParam->GetPointer();
		unsigned char* szComBuf = (unsigned char*)MGetBlobArrayElement(pComBuf, 0);
		ZApplication::GetGameInterface()->OnRequestXTrapSeedKey(szComBuf);
	}
	break;
#endif
	case MC_MATCH_RESPONSE_USE_SPENDABLE_BUFF_ITEM:
	{
		MUID uidItem;
		int nResult;

		pCommand->GetParameter(&uidItem, 0, MPT_UID);
		pCommand->GetParameter(&nResult, 0, MPT_INT);

		OnResponseUseSpendableBuffItem(uidItem, nResult);
	}
	break;

	case MC_MATCH_SPENDABLE_BUFF_ITEM_STATUS:
	{
		//¹öÇÁÁ¤º¸ÀÓ½ÃÁÖ¼®
		_ASSERT(0);
		/*
		MUID uidChar;

		pCommand->GetParameter(&uidChar, 0, MPT_UID);

		MCommandParameter* pParam = pCommand->GetParameter(1);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pCmdBuf = pParam->GetPointer();
		MTD_CharBuffInfo* pCharBuffInfo = (MTD_CharBuffInfo*)MGetBlobArrayElement(pCmdBuf, 0);

		OnGetSpendableBuffItemStatus(uidChar, pCharBuffInfo);
		*/
	}
	break;
	}

	// °ÔÀÓ?E?¾ûÒ² Ä¿¸Çµå°¡ Ã³¸®µÇ¾ú´Â?E¾Ë¼öÀÖµµ·Ï ±âÈ¸¸¦ ÁÖÀÚ
	// °ÔÀÓ?E?µû?E¾ûÒ² Ä¿¸Çµå°¡ ZGame¿¡¼­ Ã³¸®µÈ ÈÄ¿¡ ¹º°¡ ?EÇÏ?E½ÍÀ» ¼öµµ ÀÖÀ» ¶§¸¦ À§ÇÑ°Í
	ZRule* pRule = m_Match.GetRule();
	if (pRule) {
		pRule->AfterCommandProcessed(pCommand);
	}

	// return true;
	return false;
}

rvector ZGame::GetMyCharacterFirePosition(void)
{
	rvector p = ZGetGame()->m_pMyCharacter->GetPosition();
	p.z += 160.f;
	return p;
}

void ZGame::OnPeerSlashCharging(MCommand* pCommand)
{
	MCommandParameter* pParam = pCommand->GetParameter(0);
	if (pParam->GetType() != MPT_STR) return;

	MUID uid = pCommand->GetSenderUID();

	ZCharacter* pCharacter = m_CharacterManager.Find(uid);

	if (pCharacter == NULL) return;

	char szSlashName[24] = "";
	pParam->GetString(szSlashName);

	if (ZGetGame() == NULL) return;

	ZGetEffectManager()->AddChargingEffect(pCharacter, szSlashName);
}

void ZGame::OnPeerSlashCharged(MCommand* pCommand)
{
	MCommandParameter* pParam = pCommand->GetParameter(0);
	if (pParam->GetType() != MPT_STR) return;

	MUID uid = pCommand->GetSenderUID();

	ZCharacter* pCharacter = m_CharacterManager.Find(uid);

	if (pCharacter == NULL) return;

	char szSlashName[24] = "";
	pParam->GetString(szSlashName);

	if (ZGetGame() == NULL) return;

	ZGetEffectManager()->AddChargedEffect(pCharacter, szSlashName);
}

void ZGame::OnPeerSlash(MCommand* pCommand)
{
	MCommandParameter* pParam = pCommand->GetParameter(0);
	if (pParam->GetType() != MPT_BLOB) return;

	MUID uid = pCommand->GetSenderUID();
	ZPACKEDSLASHINFO* ppdi = (ZPACKEDSLASHINFO*)pParam->GetPointer();

	ZCharacter* pCharacter = m_CharacterManager.Find(uid);

	if (pCharacter == NULL) return;

	rvector waveCenter;
	//waveCenter = rvector(ppdi->posx, ppdi->posy, ppdi->posz);
	waveCenter = pCharacter->GetPosition();

	if (ZGetGame() == NULL) return;

	// Splash effect
	ZGetEffectManager()->AddSwordWaveEffect(waveCenter, pCharacter->GetDirection(), pCharacter, ppdi->szSlashName);// Custom: Slash Equip

}

// ¿ÉÀú?E¶§¿¡´Â ÀÌ Æã¼ÇÀÇ ¿ªÇÒÀÌ ºÐ¸®µÈ´Ù
// ?E ¹Ì¸® history¿¡ ´õÇØÁö?EÀûÀýÇÑ Å¸ÀÌ¹Ö¿¡ ½ÇÇàµÈ´Ù.
void ZGame::OnPeerBasicInfo(MCommand* pCommand, bool bAddHistory, bool bUpdate)
{
	MCommandParameter* pParam = pCommand->GetParameter(0);
	if (pParam->GetType() != MPT_BLOB) return;

	ZPACKEDBASICINFO* ppbi = (ZPACKEDBASICINFO*)pParam->GetPointer();

	ZBasicInfo bi;
	bi.position = rvector(Roundf(ppbi->posx), Roundf(ppbi->posy), Roundf(ppbi->posz));
	bi.velocity = rvector(ppbi->velx, ppbi->vely, ppbi->velz);
	bi.direction = 1.f / 32000.f * rvector(ppbi->dirx, ppbi->diry, ppbi->dirz);

	MUID uid = pCommand->GetSenderUID();

	MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(uid);
	if (pPeer) {
		if (pPeer->IsOpened() == false) {
			MCommand* pCmd = ZGetGameClient()->CreateCommand(MC_PEER_OPENED, ZGetGameClient()->GetPlayerUID());
			pCmd->AddParameter(new MCmdParamUID(pPeer->uidChar));
			ZGetGameClient()->Post(pCmd);

			pPeer->SetOpened(true);
		}
	}

	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uid);
	if (!pCharacter) return;

	// Ä³¸¯ÅÍÀÇ ÇöÀç½Ã°£À» ¾÷µ¥ÀÌÆ®ÇÑ´Ù
	// Ä³¸¯ÅÍÀÇ ÇöÀç½Ã°£ ÃßÁ¤Ä¡
	float fCurrentLocalTime = pCharacter->m_fTimeOffset + GetTime();

	// Ä³¸¯ÅÍ°¡ º¸³»¿Â ½Ã°£ÀÌ ³»°¡ ÃßÁ¤ÇÑ ½Ã°£?E3ÃÊ ÀÌ?EÂ÷ÀÌ°¡ ³ª?E³»°¡ ¾Ë°úÜÖ´Â ½Ã°£À» °úà£´Ù.
	float fTimeError = ppbi->fTime - fCurrentLocalTime;
	if (fabs(fTimeError) > TIME_ERROR_BETWEEN_RECIEVEDTIME_MYTIME) {
		pCharacter->m_fTimeOffset = ppbi->fTime - GetTime();
		pCharacter->m_fAccumulatedTimeError = 0;
		pCharacter->m_nTimeErrorCount = 0;
	}
	else
	{
		// Â÷ÀÌ°¡ 3ÃÊ ÀÌ³»ÀÌ?EÀÏÁ¤½Ã°£ ÇÕÇß´Ù°¡ Á¶±Ý(Â÷ÀÌÀÇ ¹Ý)¾¿ Á¶ÀýÇÑ´Ù
		pCharacter->m_fAccumulatedTimeError += fTimeError;
		pCharacter->m_nTimeErrorCount++;
		if (pCharacter->m_nTimeErrorCount > 10) {
			pCharacter->m_fTimeOffset += .5f * pCharacter->m_fAccumulatedTimeError / 10.f;
			pCharacter->m_fAccumulatedTimeError = 0;
			pCharacter->m_nTimeErrorCount = 0;
		}
	}

	// ÇöÀç½Ã°£À» ¸¶Áö¸· µ¥ÀÌÅÍ ¹ÞÀº½Ã°£À¸·Î ±â·Ï.
	pCharacter->m_fLastReceivedTime = GetTime();

	pCharacter->m_dwLastBasicInfoTime = timeGetTime();

	// ³ªÁß¿¡ ÆÇÁ¤À» À§ÇØ histroy ¿¡ º¸?EÑ´?
	if (bAddHistory)
	{
		ZBasicInfoItem* pitem = new ZBasicInfoItem;
		CopyMemory(&pitem->info, &bi, sizeof(ZBasicInfo));

		pitem->fReceivedTime = GetTime();

		pitem->fSendTime = ppbi->fTime - pCharacter->m_fTimeOffset;	// ³» ±âÁØÀ¸·Î º¯È¯

		pCharacter->m_BasicHistory.push_back(pitem);

		while (pCharacter->m_BasicHistory.size() > CHARACTER_HISTROY_COUNT)
		{
			delete* pCharacter->m_BasicHistory.begin();
			pCharacter->m_BasicHistory.erase(pCharacter->m_BasicHistory.begin());
		}
	}

	if (bUpdate)
	{
		if (!IsReplay() && pCharacter->IsHero()) return;

		((ZNetCharacter*)(pCharacter))->SetNetPosition(bi.position, bi.velocity, bi.direction);

		pCharacter->SetAnimationLower((ZC_STATE_LOWER)ppbi->lowerstate);
		pCharacter->SetAnimationUpper((ZC_STATE_UPPER)ppbi->upperstate);

		// µé°úÜÖ´Â ¹«±â°¡ ´Ù¸£?E¹Ù²ãÁØ´Ù
		if (pCharacter->GetItems()->GetSelectedWeaponParts() != ppbi->selweapon) {
			pCharacter->ChangeWeapon((MMatchCharItemParts)ppbi->selweapon);
#ifdef _ZGUARD
			pCharacter->m_nKey = ppbi->nKeyWeapon2;
			pCharacter->m_nKeyAlt = ppbi->nKeyWeapon2Alt;
			char buff[128];
			//sprintf(buff, "First: %d Alt : %d\n", pCharacter->m_nKey, pCharacter->m_nKeyAlt);
			//mlog(buff, "Key: %d Key Alt : %d\n", pCharacter->m_nKey, pCharacter->m_nKeyAlt);
			//ZChatOutput(buff);
#endif
		}
	}
}
#ifdef _UPCHARCMD
void ZGame::OnPeerUpdateCharacter(MCommand* pCommand)
{
	MCommandParameter* pParam = pCommand->GetParameter(0);
	if (pParam->GetType() != MPT_BLOB) return;

	MUID SenderUID = pCommand->GetSenderUID();

	MTD_CharInfo* pCharInfo = (MTD_CharInfo*)pParam->GetPointer();
	if (!pCharInfo)
		return;

	ZCharacter* targetChar = (ZCharacter*)m_CharacterManager.Find(SenderUID);
	if (!targetChar)
		return;

	targetChar->Load(pCharInfo);
}
#endif
void ZGame::OnPeerHPInfo(MCommand* pCommand)
{
	MUID uid = pCommand->GetSenderUID();
	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uid);
	if (!pCharacter) return;

	float fHP = 0.0f;
	pCommand->GetParameter(&fHP, 0, MPT_FLOAT);

	// Custom: Changed logic here
	if (!IsReplay() && pCharacter->GetUID() != ZGetMyUID() && uid != ZGetMyUID())
		pCharacter->SetHP(fHP);
	else if (IsReplay() || ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
		pCharacter->SetHP(fHP);

	// ¿ÉÀú?EÇÏ?EÀÖÀ»¶§´Â º¸¿©ÁÖ?EÀ§ÇØ hp Á¤º¸¸¦ °»½ÅÇÑ´Ù.
	//if(ZGetGameInterface()->GetCombatInterface()->GetObserverMode()) {
	//	pCharacter->SetHP(fHP);
	//}
}

void ZGame::OnPeerHPAPInfo(MCommand* pCommand)
{
	MUID uid = pCommand->GetSenderUID();
	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uid);
	if (!pCharacter) return;

	float fHP = 0.0f;
	pCommand->GetParameter(&fHP, 0, MPT_FLOAT);
	float fAP = 0.0f;
	pCommand->GetParameter(&fAP, 1, MPT_FLOAT);

	// Custom: Changed logic here
	if (!IsReplay() && pCharacter->GetUID() != ZGetMyUID() && uid != ZGetMyUID())
	{
		pCharacter->SetHP(fHP);
		pCharacter->SetAP(fAP);
	}
	// Custom: Added a check for teambar (the teamid.ref() is a saftey check, although it's probably bypassed easily).
	else if (IsReplay() || ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
	{
		pCharacter->SetHP(fHP);
		pCharacter->SetAP(fAP);
	}

	// ¿ÉÀú?EÇÏ?EÀÖÀ»¶§´Â º¸¿©ÁÖ?EÀ§ÇØ hp Á¤º¸¸¦ °»½ÅÇÑ´Ù.
	//if(ZGetGameInterface()->GetCombatInterface()->GetObserverMode()) {
	//	pCharacter->SetHP(fHP);
	//	pCharacter->SetAP(fAP);
	//}
}

void ZGame::OnPeerDuelTournamentHPAPInfo(MCommand* pCommand)
{
	MUID uid = pCommand->GetSenderUID();
	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uid);
	if (!pCharacter) return;

	BYTE MaxHP = 0;
	BYTE MaxAP = 0;
	BYTE HP = 0;
	BYTE AP = 0;

	pCommand->GetParameter(&MaxHP, 0, MPT_UCHAR);
	pCommand->GetParameter(&MaxAP, 1, MPT_UCHAR);

	pCommand->GetParameter(&HP, 2, MPT_UCHAR);
	pCommand->GetParameter(&AP, 3, MPT_UCHAR);

	// ¿ø·¡ peerÀÇ HP/AP Á¤º¸´Â ¿À?E¿ÉÀú¹ö¿¡°Ô º¸¿©ÁÖ?EÀ§ÇØ¼­¸¸ ?EÞµÈ´?
	// ±âÈ¹?Eµà¾óÅä³Ê¸ÕÆ®ÀÏ ¶§¿¡´Â ½ÇÁ¦ ÇÃ·¹Áß¿¡µµ ?EE»ó?E?HP,APÀÇ UI¸¦ ±×·ÁÁÖ±â·Î µÇ?EÀÖ´Ù.
	// ¿ÉÁ®¹ö°¡ ¾Æ´Ò¶§(Á÷Á¢ ÇÃ·¡ÀÌ¸¦ ÇÒ¶§)¿¡ peerÀÇ Ä³¸¯ÅÍ HP, AP¸¦ °»½ÅÇØÁÖ?E
	// '³» Ä³¸¯ÅÍÀÇ Á×À½Àº ³»°¡ Á÷Á¢ ÆÇ´ÜÇÑ´Ù'´Â ±âÁ¸ p2pÁ¤Ã¥?E¹®Á¦°¡ ¹ß»ýÇÒ ?EÀÖ?EÄ³¸¯ÅÍ¿¡ Á÷Á¢ HP/AP¸¦ setÇÏ?E¾Ê?E
	// UI Ãâ·Â?E¸·?µû·Î HP/AP°ªÀ» º¸?EÑ´?
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT) {
		((ZRuleDuelTournament*)m_Match.GetRule())->SetPlayerHpApForUI(uid, (float)MaxHP, (float)MaxAP, (float)HP, (float)AP);
	}

	// ¿ÉÀú?EÇÏ?EÀÖÀ»¶§´Â º¸¿©ÁÖ?EÀ§ÇØ hp Á¤º¸¸¦ °»½ÅÇÑ´Ù.
	// Custom: Changed logic here
	if (pCharacter->GetUID() != ZGetMyUID() && uid != ZGetMyUID())
	{
		pCharacter->SetMaxHP((float)MaxHP);
		pCharacter->SetMaxAP((float)MaxAP);
		pCharacter->SetHP((float)HP);
		pCharacter->SetAP((float)AP);
	}
	else if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
	{
		pCharacter->SetMaxHP((float)MaxHP);
		pCharacter->SetMaxAP((float)MaxAP);
		pCharacter->SetHP((float)HP);
		pCharacter->SetAP((float)AP);
	}
}

#ifdef _DEBUG
static int g_nPingCount = 0;
static int g_nPongCount = 0;
#endif

void ZGame::OnPeerPing(MCommand* pCommand)
{
	if (m_bReplaying.Ref()) return;

	unsigned int nTimeStamp;
	pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);

	// PONG À¸·Î ÀÀ´äÇÑ´Ù
	MCommandManager* MCmdMgr = ZGetGameClient()->GetCommandManager();
	MCommand* pCmd = new MCommand(MCmdMgr->GetCommandDescByID(MC_PEER_PONG),
		pCommand->GetSenderUID(), ZGetGameClient()->GetUID());
	pCmd->AddParameter(new MCmdParamUInt(nTimeStamp));
	ZGetGameClient()->Post(pCmd);
}

void ZGame::OnPeerPong(MCommand* pCommand)
{
	MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(pCommand->GetSenderUID());
	if (pPeer == NULL)
		return;

	unsigned int nTimeStamp;
	pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);

	int nPing = (GetTickTime() - nTimeStamp) / 2;
	pPeer->UpdatePing(GetTickTime(), nPing);

	/*
		if (pPeer->IsOpened() == false) {
			MCommand* pCmd = ZGetGameClient()->CreateCommand(MC_PEER_OPENED, ZGetGameClient()->GetPlayerUID());
			pCmd->AddParameter(new MCmdParamUID(pPeer->uidChar));
			ZGetGameClient()->Post(pCmd);

			pPeer->SetOpened(true);
		}
	*/
#ifdef _DEBUG
	g_nPongCount++;

	// Custom: Added test messages to fix spike issue
	//OutputToConsole( "pong -> response (%s): %d", pPeer->CharInfo.szName, nPing );
#endif
}

void ZGame::OnPeerOpened(MCommand* pCommand)
{
	MUID uidChar;
	pCommand->GetParameter(&uidChar, 0, MPT_UID);

	//// Show Character ////////////////////////////////////////
	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uidChar);
	if (pCharacter && pCharacter->IsDie() == false) {
		pCharacter->SetVisible(true);

		// ½ÅÀÔ Ä³¸¯ÅÍ¿¡°Ô ÀÚ½ÅÀÇ ¹«±â¸¦ ¾Ë¸°´Ù...
		ZCharacter* pMyCharacter = ZGetGame()->m_pMyCharacter;
		if (pMyCharacter)
		{
			int nParts = ZGetGame()->m_pMyCharacter->GetItems()->GetSelectedWeaponParts();
			ZGetGame()->m_pMyCharacter->m_dwStatusBitPackingValue.Ref().m_bSpMotion = false;
			ZPostChangeWeapon(nParts);
		}
		// ÀÚ½ÅÀÇ ¹öÇÁ »óÅÂ¸¦ ¾Ë¸°´Ù
		PostMyBuffInfo();
	}

#ifdef _DEBUG
	//// PeerOpened Debug log //////////////////////////////////
	char* pszName = "Unknown";
	char* pszNAT = "NoNAT";
	MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(uidChar);
	if (pPeer) {
		pszName = pPeer->CharInfo.szName;
		if (pPeer->GetUDPTestResult() == false) pszNAT = "NAT";
	}

	char szBuf[64];
	sprintf(szBuf, "PEER_OPENED(%s) : %s(%d%d) \n", pszNAT, pszName, uidChar.High, uidChar.Low);
	OutputDebugString(szBuf);
#endif
}

void ZGame::OnChangeWeapon(MUID& uid, MMatchCharItemParts parts)
{
	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uid);
	//	if (uid == ZGetGameClient()->GetUID()) pCharacter = m_pMyCharacter;
	//	_ASSERT(pCharacter != NULL);

	if (pCharacter && pCharacter != m_pMyCharacter)		// ³» Ä³¸¯ÅÍ´Â ÀÌ¹Ì ¹Ù²å´Ù.
	{
		pCharacter->ChangeWeapon(parts);
	}
	if (pCharacter)
	{
		if (pCharacter == m_pMyCharacter)
		{
			if (m_Match.GetMatchType() == MMATCH_GAMETYPE_SPY && m_Match.GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
			{
				if (!ZApplication::GetGameInterface()->GetCombatInterface()) return;

				ZApplication::GetGameInterface()->GetCombatInterface()->SetDefaultSpyTip(m_pMyCharacter->GetTeamID());

				if (!ZApplication::GetGameInterface()->GetCombatInterface()->m_bSpyLocationOpened && !ZApplication::GetGameInterface()->GetCombatInterface()->GetObserverMode())
				{
					ZItem* pItem = m_pMyCharacter->GetItems()->GetItem(parts);
					if (!pItem) return;

					const char* pszSpyTip = ZApplication::GetGameInterface()->GetCombatInterface()->GetSuitableSpyItemTip((int)pItem->GetDescID());
					if (pszSpyTip)
						ZApplication::GetGameInterface()->GetCombatInterface()->SetSpyTip(pszSpyTip);
				}
			}
		}
		else
		{
			pCharacter->ChangeWeapon(parts);
		}
	}
}

void ZGame::OnChangeParts(MUID& uid, int partstype, int PartsID)
{
	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uid);
	//	if (uid == ZGetGameClient()->GetUID()) pCharacter = m_pMyCharacter;

	if (pCharacter) {
		pCharacter->OnChangeParts((RMeshPartsType)partstype, PartsID);
	}
}

void ZGame::OnAttack(MUID& uid, int type, rvector& pos)
{
	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uid);
	if (pCharacter == NULL) return;

#ifdef _ZGUARD
	// 1. Kiểm tra TBF (Slash -> Block)
	// Nếu IsItPossible trả về false, ta ngắt luôn, không cho thực hiện đòn chém
	if (!IsItPossible(uid, 3)) {
		// mlog("Blocked Slash from %s due to Macro TBF\n", pCharacter->GetUserName());
		return;
	}

	// 2. Kiểm tra Nhịp chém đều (Slash -> Slash)
	// Có thể thêm Case 4 vào IsItPossible để check nhịp đều
	if (!IsItPossible(uid, 4)) {
		return;
	}

	// 3. Nếu mọi thứ OK, cập nhật thời gian chém cuối cùng
	pCharacter->st_nLastSlashTime = timeGetTime();
#endif
	if (pCharacter) {
	}
}

void ZGame::OnDamage(MUID& uid, MUID& tuid, int damage)
{
	/*
		ZCharacter* pSender = NULL;
		ZCharacter* pTarget = NULL;

		pSender = m_CharacterManager.Find(uid);
		pTarget = m_CharacterManager.Find(tuid);

		pTarget->OnSimpleDamaged(NULL,damage,0.5f);
	*/
}
#ifdef _NOLEAD
void ZGame::OnNoLead(const MUID& uidAttacker, const MUID& uidVictim, const rvector& pos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	ZCharacter* pAttacker = (ZCharacter*)m_CharacterManager.Find(uidAttacker);
	ZCharacter* pVictim = (ZCharacter*)m_CharacterManager.Find(uidVictim);

	if (!pAttacker || !pVictim)
		return;

	if (!pAttacker->GetInitialized() || !pVictim->GetInitialized() || !pAttacker->IsVisible() || !pVictim->IsVisible())
		return;

	// Game must start first.
	if (ZGetGame()->GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PLAY)
		return;

	if (IsReplay())
	{
		// Packets are most likely checked already
		//if( CheckWall( pAttacker, pVictim, true ) )
		//	return;

		if (pAttacker->IsDie())
			return;

		pVictim->OnDamaged(pAttacker, pos, damageType, weaponType, fDamage, fPiercingRatio, nMeleeType);
		return;
	}

	// Custom: NOLEAD check
	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_LEAD) || ZGetGameClient()->GetMatchStageSetting()->IsQuestDrived())
		return;

	if (uidVictim == ZGetMyUID() && uidAttacker != ZGetMyUID())
	{
		ZMyCharaterStatusBitPacking& zStatus = m_pMyCharacter->m_statusFlags.Ref();

		// Don't use walls as lagshield
		//if( CheckWall( pAttacker, pVictim, true ) && !zStatus.m_bDrop )
		//	return;

		MMatchPeerInfo* pAttackerPeer = ZGetGameClient()->FindPeer(uidAttacker);

		if (!pAttackerPeer)
			return;

		if (!pAttackerPeer->IsOpened())
			return;

		if (pAttacker->IsDie())
			return;

		// Stop no-lead from working if user has 999 ping - Spiking / disconnected.
		if (pAttackerPeer->GetPing(ZGetGame()->GetTickTime()) == MAX_PING)
			return;

		// Useless, it's still a snippet taken from leading
		/*
		if ( damageType == ZD_MELEE && pVictim->IsGuard() && ( DotProduct( pVictim->m_Direction, pAttacker->m_Direction ) < 0))
		{
			rvector vMyPos = pVictim->GetPosition();
			vMyPos.z += 120.f;

			pVictim->SetPosition( vMyPos );

			ZGetEffectManager()->AddSwordDefenceEffect( pos + ( pVictim->m_Direction * 50.f), pVictim->m_Direction);
			pVictim->OnMeleeGuardSuccess();
			return;
		}
		*/

		m_pMyCharacter->OnDamaged(pAttacker, pos, damageType, weaponType, fDamage, fPiercingRatio, nMeleeType);
	}
}
void ZGame::OnNoLeadBlob(const MUID& uidSender, void* pBlobArray)
{
	// Custom: NOLEAD check
	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_LEAD) || ZGetGameClient()->GetMatchStageSetting()->IsQuestDrived())
		return;

	int nSize = MGetBlobArrayCount(pBlobArray);

	// glitch?
	if (nSize <= 0 || nSize > 255)
		return;

	for (int i = 0; i < nSize; ++i)
	{
		MTD_ShotInfo* pInfo = (MTD_ShotInfo*)MGetBlobArrayElement(pBlobArray, i);

		if (m_pMyCharacter && ZGetGameClient()->GetPlayerUID() != uidSender)
		{
			ZCharacter* pCharacter = (ZCharacter*)ZGetCharacterManager()->Find(uidSender);

			rvector pos;
			pos.x = pInfo->fPosX;
			pos.y = pInfo->fPosY;
			pos.z = pInfo->fPosZ;

			if (pCharacter != NULL && pInfo->uid == m_pMyCharacter->GetUID() /*ZGetGameClient()->GetPlayerUID()*/ && CanAttack(pCharacter, m_pMyCharacter))
			{
				if (!m_pMyCharacter->GetInitialized() || !pCharacter->GetInitialized() || !m_pMyCharacter->IsVisible() || !pCharacter->IsVisible())
					return;

				// Game must start first. We do not want random deaths in Finish roundstate
				if (ZGetGame()->GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PLAY)
					return;

				// if it's not a replay..
				if (!IsReplay())
				{
					MMatchPeerInfo* pAttackerPeer = ZGetGameClient()->FindPeer(uidSender);

					if (!pAttackerPeer)
						return;

					if (!pAttackerPeer->IsOpened())
						return;

					// Stop no-lead from working if user has 999 ping - Spiking / disconnected.
					if (pAttackerPeer->GetPing(ZGetGame()->GetTickTime()) == MAX_PING)
						return;

					// sanity check
					if (timeGetTime() - pCharacter->m_dwLastBasicInfoTime >= 4000)
						return;
				}

				if (pCharacter->IsDie())
					return;

				bool bMelee = ((ZDAMAGETYPE)pInfo->nDamageType) == ZD_MELEE;

				m_pMyCharacter->OnDamaged(pCharacter, pos, (ZDAMAGETYPE)pInfo->nDamageType, (MMatchWeaponType)pInfo->nWeaponType, pInfo->fDamage, pInfo->fRatio, bMelee ? pInfo->nMeleeType : -1);
			}
		}
	}
}
#endif
void ZGame::OnVampire(const MUID& uidVictim, const MUID& uidAttacker, float fDamage)
{
	ZCharacter* pAttacker = (ZCharacter*)m_CharacterManager.Find(uidAttacker);
	ZCharacter* pVictim = (ZCharacter*)m_CharacterManager.Find(uidVictim);

	if (!pAttacker || !pVictim)
		return;

	if (!pAttacker->GetInitialized() || !pVictim->GetInitialized() || !pAttacker->IsVisible() || !pVictim->IsVisible())
		return;

	if (IsReplay())
	{
		int nAddHP = (int)(fDamage * 0.33f);
		int nCurrHP = pAttacker->GetHP();
		int nMaxHP = pAttacker->GetMaxHP();

		if (nCurrHP + nAddHP > nMaxHP)
		{
			pAttacker->SetHP(nMaxHP);

			int nAddAP = (nCurrHP + nAddHP) - nMaxHP;
			int nCurrAP = pAttacker->GetAP();
			int nMaxAP = pAttacker->GetMaxAP();

			if (nCurrAP + nAddAP > nMaxAP)
				pAttacker->SetAP(nMaxAP);
			else
				pAttacker->SetAP(nCurrAP + nAddAP);
		}
		else
			pAttacker->SetHP(nCurrHP + nAddHP);

		return;
	}

	if ((ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_VAMPIRE) && !ZGetGameClient()->GetMatchStageSetting()->IsQuestDrived()
		&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		if (uidAttacker == ZGetMyUID() && uidVictim != ZGetMyUID())
		{
			MMatchPeerInfo* pVictimPeer = ZGetGameClient()->FindPeer(uidVictim);

			if (!pVictimPeer)
				return;

			if (!pVictimPeer->IsOpened())
				return;

			int nAddHP = (int)(fDamage * 0.33f);
			int nCurrHP = m_pMyCharacter->GetHP();
			int nMaxHP = m_pMyCharacter->GetMaxHP();

			if (nCurrHP + nAddHP > nMaxHP)
			{
				m_pMyCharacter->SetHP(nMaxHP);

				int nAddAP = (nCurrHP + nAddHP) - nMaxHP;
				int nCurrAP = m_pMyCharacter->GetAP();
				int nMaxAP = m_pMyCharacter->GetMaxAP();

				if (nCurrAP + nAddAP > nMaxAP)
					m_pMyCharacter->SetAP(nMaxAP);
				else
					m_pMyCharacter->SetAP(nCurrAP + nAddAP);
			}
			else
				m_pMyCharacter->SetHP(nCurrHP + nAddHP);
		}
	}
}
#ifdef _LADDERWARSPACKETS
void ZGame::OnLadderWars(const MUID& uidVictim, const MUID& uidAttacker, float fDamage)
{
	ZCharacter* pAttacker = (ZCharacter*)m_CharacterManager.Find(uidAttacker);
	ZCharacter* pVictim = (ZCharacter*)m_CharacterManager.Find(uidVictim);

	if (!pAttacker || !pVictim)
		return;

	if (!pAttacker->GetInitialized() || !pVictim->GetInitialized() || !pAttacker->IsVisible() || !pVictim->IsVisible())
		return;

	if (IsReplay())
	{
		int nAddHP = (int)(fDamage * 0.33f);
		int nCurrHP = pAttacker->GetHP();
		int nMaxHP = pAttacker->GetMaxHP();

		if (nCurrHP + nAddHP > nMaxHP)
		{
			pAttacker->SetHP(nMaxHP);

			int nAddAP = (nCurrHP + nAddHP) - nMaxHP;
			int nCurrAP = pAttacker->GetAP();
			int nMaxAP = pAttacker->GetMaxAP();

			if (nCurrAP + nAddAP > nMaxAP)
				pAttacker->SetAP(nMaxAP);
			else
				pAttacker->SetAP(nCurrAP + nAddAP);
		}
		else
			pAttacker->SetHP(nCurrHP + nAddHP);

		return;
	}

	if (ZGetGameClient()->IsLadderWarsChannel())
	{
		if (uidAttacker == ZGetMyUID() && uidVictim != ZGetMyUID())
		{
			MMatchPeerInfo* pVictimPeer = ZGetGameClient()->FindPeer(uidVictim);

			if (!pVictimPeer)
				return;

			if (!pVictimPeer->IsOpened())
				return;

			int nAddHP = (int)(fDamage * 0.33f);
			int nCurrHP = m_pMyCharacter->GetHP();
			int nMaxHP = m_pMyCharacter->GetMaxHP();

			if (nCurrHP + nAddHP > nMaxHP)
			{
				m_pMyCharacter->SetHP(nMaxHP);

				int nAddAP = (nCurrHP + nAddHP) - nMaxHP;
				int nCurrAP = m_pMyCharacter->GetAP();
				int nMaxAP = m_pMyCharacter->GetMaxAP();

				if (nCurrAP + nAddAP > nMaxAP)
					m_pMyCharacter->SetAP(nMaxAP);
				else
					m_pMyCharacter->SetAP(nCurrAP + nAddAP);
			}
			else
				m_pMyCharacter->SetHP(nCurrHP + nAddHP);
		}
	}
}
#endif
void ZGame::OnPeerScope(const MUID& uidPeer, bool bScope)
{
	ZCharacter* pOwner = (ZCharacter*)m_CharacterManager.Find(uidPeer);

	if (!pOwner)
		return;

	// visible? what if I scoped? I'll be invisible and this will return
	if (!pOwner->GetInitialized() /*|| !pOwner->IsVisible()*/)
		return;

	if (!IsReplay() && uidPeer == ZGetMyUID())
		return;

	ZCharaterStatusBitPacking& uStatus = pOwner->m_dwStatusBitPackingValue.Ref();

	if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
	{
		if (bScope)
		{
			if (!uStatus.m_bSniping)
			{
				uStatus.m_bSniping = true;

				if (ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetTargetCharacter() == pOwner)
					ZGetGameInterface()->GetCombatInterface()->OnGadget(MWT_SNIFER);
#ifdef _HITSCOUNT
				if (pOwner == m_pMyCharacter)
					ZGetGame()->m_pMyCharacter->m_nHits += 1;
#endif
			}
		}
		else
		{
			if (uStatus.m_bSniping)
			{
				uStatus.m_bSniping = false;

				if (ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetTargetCharacter() == pOwner)
					ZGetGameInterface()->GetCombatInterface()->OnGadgetOff();
			}
		}
	}
}

void ZGame::OnAdminFreeze(const MUID& uidAdmin, const char* szTargetName)
{
	ZCharacter* pCharacter = (ZCharacter*)ZGetGame()->m_CharacterManager.Find(uidAdmin);
	if (!pCharacter) return;
	ZCharacter* pTarget = (ZCharacter*)ZGetGame()->m_CharacterManager.Find(ZGetGame()->m_pMyCharacter->GetUID());

	char szTemp[128];
	if (!stricmp(szTargetName, pTarget->GetUserName()))
	{
		sprintf(szTemp, "%s has frozen you.", pCharacter->GetUserName());
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_BROADCAST), szTemp, ZChat::CL_CURRENT);
		if (!pTarget->IsAdminName() && pTarget->GetUserGrade() != MMUG_GAMEMASTER)
		{
			if (pTarget->m_dwStatusBitPackingValue.Ref().m_bFrozen == true)
			{
				pTarget->m_dwStatusBitPackingValue.Ref().m_bFrozen = false;
			}
			else
			{
				pTarget->m_dwStatusBitPackingValue.Ref().m_bFrozen = true;
			}
		}
	}
	if (!stricmp(szTargetName, "all"))
	{
		sprintf(szTemp, "%s has frozen the room", pCharacter->GetUserName());
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_BROADCAST), szTemp, ZChat::CL_CURRENT);
		if (!pTarget->IsAdminName() && pTarget->GetUserGrade() != MMUG_GAMEMASTER)
		{
			if (pTarget->m_dwStatusBitPackingValue.Ref().m_bFrozen == true) {
				pTarget->m_dwStatusBitPackingValue.Ref().m_bFrozen = false;
			}
			else
			{
				pTarget->m_dwStatusBitPackingValue.Ref().m_bFrozen = true;
			}
		}
	}
}

void ZGame::OnAdminSummon(const MUID& uidAdmin, const char* szTargetName)
{
	ZCharacter* pAdmin = (ZCharacter*)m_CharacterManager.Find(uidAdmin);

	if (!pAdmin)
		return;

	if (!pAdmin->GetInitialized() || !pAdmin->IsVisible())
		return;

	if (!stricmp(szTargetName, m_pMyCharacter->GetUserName()))
	{
		char szTemp[128];
		sprintf(szTemp, "%s has summoned you.", pAdmin->GetUserName());
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_BROADCAST), szTemp, ZChat::CL_CURRENT);
		m_pMyCharacter->SetPosition(pAdmin->GetPosition());

		ZGetEffectManager()->AddReBirthEffect(pAdmin->GetPosition());
		ZGetSoundEngine()->PlaySound("fx_respawn");
	}
}

void ZGame::OnAdminGoTo(const MUID& uidAdmin, const char* szTargetName)
{
	ZCharacter* pTarget = NULL;

	// herp derp
	// could put it into a new variable for Muid, but meh.
	for (ZCharacterManager::iterator i = ZGetCharacterManager()->begin(); i != ZGetCharacterManager()->end(); ++i)
	{
		ZCharacter* pChar = (ZCharacter*)(*i).second;

		if (pChar && pChar->GetInitialized() && pChar->IsVisible() && !pChar->IsAdminHide() && !stricmp(pChar->GetUserName(), szTargetName))
		{
			pTarget = pChar;
			break;
		}
	}

	if (!pTarget)
		return;

	if (!pTarget->GetInitialized() || !pTarget->IsVisible())
		return;

	// do not output a message. it's done by the server
	m_pMyCharacter->SetPosition(pTarget->GetPosition());

	ZGetEffectManager()->AddReBirthEffect(m_pMyCharacter->GetPosition());
	ZGetSoundEngine()->PlaySound("fx_respawn");
}

void ZGame::OnAdminSlap(const MUID& uidAdmin, const char* szTargetName)
{
	ZCharacter* pAdmin = (ZCharacter*)m_CharacterManager.Find(uidAdmin);

	if (!pAdmin)
		return;

	if (!pAdmin->GetInitialized() || !pAdmin->IsVisible())
		return;

	if (!stricmp(szTargetName, m_pMyCharacter->GetUserName()))
	{
		char szTemp[128];
		sprintf(szTemp, "%s has slapped you!", pAdmin->GetUserName());
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_BROADCAST), szTemp, ZChat::CL_CURRENT);
		// Custom:Improved /admin_slap. Less glitchier, properly awards the kill etc.
		//m_pMyCharacter->SetAP( 0 );
		//m_pMyCharacter->SetHP( 0 );
		ZPostGameKill(uidAdmin);
		ZPostDie(uidAdmin);
	}
}

void ZGame::OnAdminSpawn(const MUID& uidAdmin, const MUID& uidVictim)
{
	ZCharacter* pAdmin = (ZCharacter*)m_CharacterManager.Find(uidAdmin);
	ZCharacter* pVictim = (ZCharacter*)m_CharacterManager.Find(uidVictim);

	if (!pAdmin || !pVictim)
		return;

	if (!pAdmin->GetInitialized() || !pAdmin->IsVisible() || !pVictim->GetInitialized() || !pVictim->IsVisible())
		return;

	rvector pos = pVictim->GetPosition(), dir = pVictim->GetDirection();

	//I realise this is a dirty way of doing it, and relying on GunZ to update the position
	// but if you want to keep the ranomized spawn location, this is the only logical way
	// Short of posting ZPostSpawn, anyway.
	if (uidVictim == m_pMyCharacter->GetUID())
	{
		ZMapSpawnData* pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();
		if (pSpawnData != NULL)
		{
			pos = pSpawnData->m_Pos;
			dir = pSpawnData->m_Dir;
		}
	}

	OnPeerSpawn(pVictim->GetUID(), pos, dir);

	if (uidVictim == m_pMyCharacter->GetUID())
	{
		ReleaseObserver();
		//ZGetCombatInterface()->SetObserverMode(false);
	}
}
void ZGame::OnAdminSBTest(const MUID& uidAdmin, const char* szTargetName)
{
	ZCharacter* pAdmin = (ZCharacter*)m_CharacterManager.Find(uidAdmin);

	if (!pAdmin)
		return;

	// admin doesn't need to spawn
	//if( !pAdmin->GetInitialized() || !pAdmin->IsVisible() )
	//	return;

	m_pMyCharacter->m_dwSBTestStart = timeGetTime();
	m_pMyCharacter->m_bSBTest = true;
}
void ZGame::OnPeerShotSp(MUID& uid, float fShotTime, rvector& pos, rvector& dir, int type, MMatchCharItemParts sel_type)
{
	ZCharacter* pOwnerCharacter = NULL;		// ÃÑ ?E»ç?E

	pOwnerCharacter = (ZCharacter*)m_CharacterManager.Find(uid);
	//	if (uid == ZGetGameClient()->GetUID()) pOwnerCharacter = m_pMyCharacter;

	//	_ASSERT(pOwnerCharacter != NULL);
	if (pOwnerCharacter == NULL) return;
	if (!pOwnerCharacter->GetInitialized()) return;
	if (!pOwnerCharacter->IsVisible()) return;

	ZItem* pItem = pOwnerCharacter->GetItems()->GetItem(sel_type);
	if (!pItem) return;

	MMatchItemDesc* pDesc = pItem->GetDesc();
	if (pDesc == NULL) return;

	// fShotTime ÀÌ ±× Ä³¸¯ÅÍÀÇ ·ÎÄÃ ½Ã°£ÀÌ¹Ç·Î ³» ½Ã°£À¸·Î º¯È¯ÇØÁØ´Ù
	fShotTime -= pOwnerCharacter->m_fTimeOffset;

	/*
	float fCurrentTime = g_pGame->GetTime();
	if( abs(fCurrentTime - fShotTime) > TIME_ERROR_BETWEEN_RECIEVEDTIME_MYTIME )
	{
#ifdef _DEBUG
		mlog("!!!!¼ö·ùÅº ÇÙ!!!! Ä³¸¯ÅÍ ³×ÀÓ: %s      fShotTime : %f     fCurrentTime : %f \n",
			pOwnerCharacter->GetUserName(), (fShotTime - pOwnerCharacter->m_fTimeOffset) , fCurrentTime);
#endif
		return;
	}
	ÀÌ ºÎºÐÀº ÇÙ¿¡¼­ shotÀ» ÇÑ ½Ã°£À» Á¶ÀÛÇÏ¿© º¸³»´Â °ÍÀ» °¨ÁöÇÏ¿© ÇÙÀ» ¸·´Â ÄÚµå¿´´Âµ¥ ¹Þ´Â ÂÊ¿¡¼­ ½Ã°£ °Ë»ç¸¦ ÇÏ?E¸»?E
	º¸³»´Â ÂÊ¿¡¼­ °Ë»ç¸¦ ÇØ¼­ shotÀ» ÇÑ ½Ã°£ÀÌ ÇØ?EÄ³¸¯ÅÍÀÇ lacal time?E¸Â?E¾ÊÀ¸?E¾Æ¿¹ ÆÐÅ¶À» º¸³»?E¾Êµµ·Ï ¹Ù²å´Ù.
	µû¶ó¼­ ÇØ?EÄÚµå°¡ ÇÊ?E¾ø°Ô µÊ. ÃßÈÄ localtimeÀ» Á¶ÀÛÇÒ °æ?E??EñÇ?ÁÖ¼®Ã³¸®·Î ³²°ÜµÒ..
	*/

	//¿©?EÆø¹ß¹° ?EEÇÔ¼öÀÌ?E¶§¹®¿¡ ¹«±â·ù¿Í ÆøÅº·ù¸¦ ´ãÀ» ?EÀÖ´Â ÆÄÃ÷°¡ ¾Æ´Ï?E¹«½ÃÇÑ´Ù.
	if (sel_type != MMCIP_PRIMARY && sel_type != MMCIP_SECONDARY && sel_type != MMCIP_CUSTOM1 && sel_type != MMCIP_CUSTOM2 && sel_type != MMCIP_CUSTOM3)
		return;

	MMatchCharItemParts parts = (MMatchCharItemParts)sel_type;

	if (parts != pOwnerCharacter->GetItems()->GetSelectedWeaponParts()) { ///< Áö±Ý µé?EÀÖ´Â ¹«±â¿Í º¸³»?E¹«±â°¡ Æ²¸®´Ù?Eº¸³»?E¹«±â·Î ¹Ù²ãÁØ´Ù..
		OnChangeWeapon(uid, parts);
	}

	//ÇÙ ¹æÁö¸¦ À§ÇØ ¿þ?EÅ¸ÀÔÀ» ºñ±³..
	MMatchWeaponType nType = pDesc->m_nWeaponType.Ref();
	//µé?EÀÖ´Â ¹«±â°¡ ·ÎÄÏ Å¸ÀÔÀÎµ¥
	if (nType == MWT_ROCKET) {
		if (type != ZC_WEAPON_SP_ROCKET) {	//typeÀÌ ·ÎÄÏÀÌ ¾Æ´Ï?E¹Ì½º ¸ÅÄ¡....¹«½ÃÇÑ´Ù.
			return;
		}
	}
	else if (nType == MWT_MED_KIT || nType == MWT_REPAIR_KIT || nType == MWT_BULLET_KIT || nType == MWT_FOOD || nType == MWT_LANDMINE_SPY || nType == MWT_LANDMINE) {
		if (type != ZC_WEAPON_SP_ITEMKIT) {
			return;
		}
	}
	else if (nType == MWT_FLASH_BANG) {
		if (type != ZC_WEAPON_SP_FLASHBANG) {
			return;
		}
	}
	else if (nType == MWT_FRAGMENTATION) {
		if (type != ZC_WEAPON_SP_GRENADE) {
			return;
		}
	}
	else if (nType == MWT_SMOKE_GRENADE) {
		if (type != ZC_WEAPON_SP_SMOKE) {
			return;
		}
	}
	else if (nType == MWT_POTION) {
		if (type != ZC_WEAPON_SP_POTION) {
			return;
		}
	}
	else if (nType == MWT_TRAP) {
		if (type != ZC_WEAPON_SP_TRAP) {
			return;
		}
	}
	else if (nType == MWT_DYNAMITYE) {
		if (type != ZC_WEAPON_SP_DYNAMITE) {
			return;
		}
	}
	///// SPY MODE /////
	else if (nType == MWT_FLASH_BANG_SPY) {
		if (type != ZC_WEAPON_SP_FLASHBANG_SPY) {
			return;
		}
	}
	else if (nType == MWT_SMOKE_GRENADE_SPY) {
		if (type != ZC_WEAPON_SP_SMOKE_SPY) {
			return;
		}
	}
	else if (nType == MWT_TRAP_SPY) {
		if (type != ZC_WEAPON_SP_TRAP_SPY) {
			return;
		}
	}
	else if (nType == MWT_STUN_GRENADE_SPY) {
		if (type != ZC_WEAPON_SPY_STUNGRENADE) {
			return;
		}
	}
#ifdef _PORTALGUN 1
	else if (nType == MWT_PORTAL_GUN) 
	{
		if (type == ZC_WEAPON_SP_PORTAL_GUN && type == ZC_WEAPON_SP_PORTAL_GUN_RED) 
		{

		}
	}
#endif
	else 
	{
		return;
	}

	// ºñÁ¤»óÀûÀÎ ¹ß»ç¼Óµµ¸¦ ¹«½ÃÇÑ´Ù.
	if (pOwnerCharacter->CheckValidShotTime(pItem->GetDescID(), fShotTime, pItem)) {
		pOwnerCharacter->UpdateValidShotTime(pItem->GetDescID(), fShotTime);
	}
	else {
		return;
	}

	if (uid == ZGetMyUID()) {
		int nCurrMagazine = pItem->GetBulletCurrMagazine();
		if (!pItem->Shot()) return;

		if (!(pItem->GetBulletCurrMagazine() < nCurrMagazine)) {
			if (sel_type != MMCIP_MELEE) ZGetApplication()->Exit();
		}
	}
	else {
		if (!pItem->Shot()) return;
	}

#ifdef _RADAR
	if (ZGetGameClient()->IsLadderWarsChannel())
	{
		if (type == ZC_WEAPON_SP_ROCKET || ZC_WEAPON_SP_GRENADE)
			ZGetCombatInterface()->GetRadar()->OnAttack(pos, uid);
	}
#endif

	rvector velocity;
	rvector _pos;

	bool dLight = true;
	bool bSpend = false;

	switch (type)
	{
	case ZC_WEAPON_SP_GRENADE:
	{
		//static RealSoundEffectSource* pSES	= ZGetSoundEngine()->GetSES("we_grenade_fire");
		//if( pSES != NULL )
		//{
		//	ZGetSoundEngine()->PlaySE( pSES, pos.x, pos.y, pos.z, pOwnerCharacter == m_pMyCharacter );
		//}
		bSpend = true;

		velocity = pOwnerCharacter->GetVelocity() + pOwnerCharacter->m_TargetDir * 1200.f;
		velocity.z += 300.f;
		m_WeaponManager.AddGrenade(pos, velocity, pOwnerCharacter);
		//m_WeaponManager.AddFlashBang( pos - rvector(10,10,10), velocity, pOwnerCharacter );
		//m_WeaponManager.AddSmokeGrenade( pos + rvector(10,10,10), velocity, pOwnerCharacter );
	}
	break;

	case ZC_WEAPON_SP_ROCKET:
	{
		//Custom: Rocket Nodes
		unsigned long int nID = pDesc->m_nID;
		if (nID == 910052 || nID == 910053)
		{
			m_WeaponManager.AddGrenade(pos, velocity, pOwnerCharacter);
		}
		else
		{
			m_WeaponManager.AddRocket(pos, dir, pOwnerCharacter);
		}
		if (Z_VIDEO_DYNAMICLIGHT) {
			ZGetStencilLight()->AddLightSource(pos, 2.0f, 100);
		}
	}
	break;

	case ZC_WEAPON_SP_FLASHBANG:
	{
		bSpend = true;

		velocity = pOwnerCharacter->GetVelocity() + pOwnerCharacter->m_TargetDir * 1200.f;
		velocity.z += 300.0f;
		m_WeaponManager.AddFlashBang(pos, velocity, pOwnerCharacter);
		dLight = false;
	}
	break;

	case ZC_WEAPON_SP_SMOKE:
	{
		bSpend = true;

		velocity = pOwnerCharacter->GetVelocity() + pOwnerCharacter->m_TargetDir * 1200.f;
		velocity.z += 300.0f;
		m_WeaponManager.AddSmokeGrenade(pos, velocity, pOwnerCharacter);
		dLight = false;
	}
	break;

	case ZC_WEAPON_SP_TEAR_GAS:
	{
		bSpend = true;
		dLight = false;
	}
	break;

	case ZC_WEAPON_SP_ITEMKIT:
	{
		int nLinkedWorldItem = ZGetWorldItemManager()->GetLinkedWorldItemID(pItem->GetDesc());

		velocity = dir;
		_pos = pos;

		m_WeaponManager.AddKit(_pos, velocity, pOwnerCharacter, 0.2f, pItem->GetDesc()->m_pMItemName->Ref().m_szMeshName, nLinkedWorldItem);
		dLight = false;
	}
	break;

	case ZC_WEAPON_SP_POTION:
	{
		ApplyPotion(pItem->GetDescID(), pOwnerCharacter, 0);
	}
	break;

	case ZC_WEAPON_SP_TRAP:
	{
		OnUseTrap(pItem->GetDescID(), pOwnerCharacter, pos);
		dLight = true;
	}
	break;

	case ZC_WEAPON_SP_DYNAMITE:
	{
		OnUseDynamite(pItem->GetDescID(), pOwnerCharacter, pos);
		dLight = true;
	}
	break;

	case ZC_WEAPON_SP_FLASHBANG_SPY:
	{
		bSpend = true;

		velocity = pOwnerCharacter->GetVelocity() + pOwnerCharacter->m_TargetDir * 1200.f;
		velocity.z += 300.0f;
		m_WeaponManager.AddSpyFlashBang(pos, velocity, pOwnerCharacter);
		dLight = false;
	}
	break;

	//////////// SPY ////////////
	case ZC_WEAPON_SP_SMOKE_SPY:
	{
		bSpend = true;

		velocity = pOwnerCharacter->GetVelocity() + pOwnerCharacter->m_TargetDir * 1200.f;
		velocity.z += 300.0f;
		m_WeaponManager.AddSpySmokeGrenade(pos, velocity, pOwnerCharacter);
		dLight = false;
	}
	break;

	case ZC_WEAPON_SP_TRAP_SPY:
	{
		OnUseSpyTrap(pItem->GetDescID(), pOwnerCharacter, pos);
		dLight = true;
	}
	break;

	case ZC_WEAPON_SPY_STUNGRENADE:
	{
		OnUseStunGrenade(pItem->GetDescID(), pOwnerCharacter, pos);
		dLight = true;
	}
	break;
#ifdef _PORTALGUN 1
	case ZC_WEAPON_SP_PORTAL_GUN:
	{
		m_WeaponManager.AddPortal(pos, dir, pOwnerCharacter, 0);
		if (Z_VIDEO_DYNAMICLIGHT) {
			ZGetStencilLight()->AddLightSource(pos, 2.0f, 100);
		}
	}
	break;

	case ZC_WEAPON_SP_PORTAL_GUN_RED:
	{
		m_WeaponManager.AddPortal(pos, dir, pOwnerCharacter, 1);
		if (Z_VIDEO_DYNAMICLIGHT) {
			ZGetStencilLight()->AddLightSource(pos, 2.0f, 100);
		}
	}
	break;
#endif
	default:
		_ASSERT(0);
		break;
	}

#ifdef _REPLAY_TEST_LOG
	if (type == ZC_WEAPON_SP_POTION || type == ZC_WEAPON_SP_ITEMKIT)
	{
		for (int i = 0; i < 16; ++i)
		{
			if (m_Replay_UseItem[i].uid.Low == 0)
			{	// uid ³»?E?¾ø´Ù?E
				m_Replay_UseItem[i].uid = uid;
				m_Replay_UseItem[i].Item[0].Itemid = pDesc->m_nID;
				m_Replay_UseItem[i].Item[0].ItemUseCount++;
				break;
			}
			if (m_Replay_UseItem[i].uid == uid)
			{	// uid°¡ °°´Ù?E
				for (int j = 0; j < 5; ++j)
				{
					if (m_Replay_UseItem[i].Item[j].Itemid == 0)
					{	// itemid ³»?E?¾ø´Ù?E
						m_Replay_UseItem[i].uid = uid;
						m_Replay_UseItem[i].Item[j].Itemid = pDesc->m_nID;
						m_Replay_UseItem[i].Item[j].ItemUseCount++;
						break;
					}
					if (m_Replay_UseItem[i].Item[j].Itemid == pDesc->m_nID)
					{
						m_Replay_UseItem[i].Item[j].ItemUseCount++;
						break;
					}
				}
				break;
			}
		}
		//mlog("[%s(%d) : %s(%d)]\n", pOwnerCharacter->GetCharInfo()->szName, uid.Low, pDesc->m_pMItemName->Ref().m_szItemName, pDesc->m_nID);
	}
#endif

	// Æ÷¼Ç·ù´Â ±×³É ÇÏµåÄÚµùÀ¸·Î ¾ÆÀÌÅÛ ¸Ô´Â »ç¿ûÑå¸¦ ³»°Ô¸¸ µé·ÁÁØ´Ù
	// ±×³É ÀÏ¹Ý ¹«±âÃ³·³ Ã³¸®ÇÏ?E¹ß»çÀ½À¸·Î °£ÁÖµÇ¾ûØ­ ÁÖº¯»ç¶÷µé¿¡°Ô µé¸®°Ô µÇ´Âµ¥,
	// ¾ÆÀÌÅÛ ¸Ô´Â »ç¿ûÑå°¡ 2d»ç¿ûÑå¶ó¼­ ?E?E¼¿?µé¸®°Ô µÇ?E?EõÇ?
	if (type == ZC_WEAPON_SP_POTION)
	{
		if (pOwnerCharacter == ZGetGame()->m_pMyCharacter) 
		{
			ZGetSoundEngine()->PlaySound("fx_itemget");
		}
	}
	ZApplication::GetSoundEngine()->PlaySEFire(pItem->GetDesc(), pos.x, pos.y, pos.z, (pOwnerCharacter == m_pMyCharacter));

	if (dLight)
	{
		// ÃÑ ½ò¶§ ¶óÀÌÆ® Ãß°¡
		ZCharacter* pChar;

		if (ZGetConfiguration()->GetVideo()->bDynamicLight && pOwnerCharacter != NULL) {
			pChar = pOwnerCharacter;

			if (pChar->m_bDynamicLight) {
				pChar->m_vLightColor = g_CharLightList[CANNON].vLightColor;
				pChar->m_fLightLife = g_CharLightList[CANNON].fLife;
			}
			else {
				pChar->m_bDynamicLight = true;
				pChar->m_vLightColor = g_CharLightList[CANNON].vLightColor;
				pChar->m_vLightColor.x = 1.0f;
				pChar->m_iDLightType = CANNON;
				pChar->m_fLightLife = g_CharLightList[CANNON].fLife;
			}

			if (pOwnerCharacter->IsHero())
			{
				RGetDynamicLightManager()->AddLight(GUNFIRE, pos);
			}
		}
	}

	if (ZGetMyUID() == pOwnerCharacter->GetUID())
	{
		ZItem* pSelItem = pOwnerCharacter->GetItems()->GetSelectedWeapon();
		if (pSelItem && pSelItem->GetDesc() &&
			pSelItem->GetDesc()->IsSpendableItem())
		{
			ZMyItemNode* pItemNode = ZGetMyInfo()->GetItemList()->GetEquipedItem((MMatchCharItemParts)sel_type);
			if (pItemNode)
			{
				pItemNode->SetItemCount(pItemNode->GetItemCount() - 1);
				ZPostRequestUseSpendableNormalItem(pItemNode->GetUID());
			}
		}
	}
#ifdef _KILLFEED
	pOwnerCharacter->SetWeaponDamaged(pItem->GetDesc()->m_nWeaponType.Ref());
	ZPostLastWeaponUsed(pItem->GetDesc()->m_nWeaponType.Ref());
#endif
}

bool ZGame::CheckWall(ZObject* pObj1, ZObject* pObj2, bool bCoherentToPeer)
{
	//### ÀÌ ÇÔ¼ö¸¦ ¼öÁ¤ÇÏ?E¶È°°ÀÌ IsWallBlocked()¿¡µµ Àû?E?ÁÖ¾ûÚß ÇÕ´Ï´Ù. ###

	// ÇÇ¾ûÏ¢¸® ÁÂÇ¥¸¦ º¸³¾¶§ ÇöÀç´Â float->short Ä³½ºÆÃÀÌ ÀÏ¾ûÏ­´Ù (?E¼Ò¼öÁ¡ÀÌÇÏ ¹ö¸²)
	// µû¶ó¼­ Å¬¶óÀÌ¾ðÆ®µéÀÌ °¢ÀÚ ÆÇÁ¤ÇÑ °ªÀÌ ¹Ì¹¦ÇÏ°Ô ´Ù¸¦ ¼ö°¡ ÀÖ´Ù. ÀÌ°ÍÀÌ ±âÁ¸¿¡ ¹®Á¦¸¦ ÀÏÀ¸Å°?E¾Ê¾ÒÀ¸³ª
	// ¼­¹ÙÀÌ¹ú¿¡¼­ ³·Àº È®?E?¹®Á¦°¡ ¹ß»ý: npc°¡ ÇÃ·¹ÀÌ¾ûÔ¦ ±ÙÁ¢°ø°ÝÇÏ·Á?EÇÒ¶§, npc ÄÁÆ®·Ñ·¯´Â °ø°Ý °¡´ÉÇÏ´Ù?EÆÇÁ¤.
	// ÇÇ°Ý´çÇÏ´Â Å¬¶óÀÌ¾ðÆ®´Â °ø°Ý °¡´ÉÇÏ?E¾Ê´Ù?EÆÇÁ¤. ÀÌ·Î?EÇÇ°ÝµÇ´Â À¯Àú°¡ À§Ä¡¸¦ ¹Ù²Ù?E¾Ê´ÂÇÑ ¸ó½ºÅÍ´Â Á¦ÀÚ¸®¿¡¼­ ¹«ÇÑ ?EæÀ?Ä¡°ÔµÊ (¼Ö±ûÔ» ¾Ç?EºÒ°¡¶ó?E»ý°¢ÇÏÁö¸¸ ÆÛºúÔ®¼ÅÀÇ ±Ù¼º¿¡ Á³À½)
	// bCoherentToPeer==true ÀÏ¶§ ÇÇ¾ûÛ¡°Ô º¸³½ °Í?E°°Àº °ªÀ» »ç?E?.

	if ((pObj1 == NULL) || (pObj2 == NULL))
		return false;

	if ((pObj1->GetVisualMesh() == NULL) || (pObj2->GetVisualMesh() == NULL))
		return false;

	// ¿¡´Ï¸ÞÀÌ¼Ç ¶§¹®¿¡ º®À» ¶Õ?Eµé¾ûÌ¡´Â °æ?E?ÀÖ¾ûØ­..
	rvector p1 = pObj1->GetPosition() + rvector(0.f, 0.f, 100.f);
	rvector p2 = pObj2->GetPosition() + rvector(0.f, 0.f, 100.f);

	if (bCoherentToPeer)
	{
		p1.x = short(p1.x);
		p1.y = short(p1.y);
		p1.z = short(p1.z);
		p2.x = short(p2.x);
		p2.y = short(p2.y);
		p2.z = short(p2.z);
		// ¿ÀÂ÷·Î ÀÎÇÑ ¹ö±× Àç?EÅ×½ºÆ®¸¦ ½±°Ô ÇÏ?EÀ§ÇØ 1ÀÇ ÀÚ¸®±û?EÀý»çÇÑ ¹ö?E
		/*p1.x = (short(p1.x * 0.1f)) * 10.f;
		p1.y = (short(p1.y * 0.1f)) * 10.f;
		p1.z = (short(p1.z * 0.1f)) * 10.f;
		p2.x = (short(p2.x * 0.1f)) * 10.f;
		p2.y = (short(p2.y * 0.1f)) * 10.f;
		p2.z = (short(p2.z * 0.1f)) * 10.f;*/
	}

	rvector dir = p2 - p1;

	Normalize(dir);

	ZPICKINFO pickinfo;

	if (Pick(pObj1, p1, dir, &pickinfo)) {//º®ÀÌ¶ó?E
		if (pickinfo.bBspPicked)//¸ÊÀÌ °É¸°°æ?E
			return true;
	}

	return false;
}
//jintriple3 µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ·.....ºñ±³¸¦ À§ÇØ¼­
bool ZGame::CheckWall(ZObject* pObj1, ZObject* pObj2, int& nDebugRegister/*´Ü?Eºñ±³?E*/, bool bCoherentToPeer)
{	//bCoherentToPeer¿¡ ?EÑ°ÍÀ?¿øº» CheckWall ÁÖ¼® ?EE
	if ((pObj1 == NULL) || (pObj2 == NULL))
	{
		nDebugRegister = -10;	//¿ª½Ã³ª ¼ýÀÚ´Â ÀÇ¹Ì°¡ ¾ø´Ù..
		return false;
	}

	if ((pObj1->GetVisualMesh() == NULL) || (pObj2->GetVisualMesh() == NULL))
	{
		nDebugRegister = -10;
		return false;
	}

	// ¿¡´Ï¸ÞÀÌ¼Ç ¶§¹®¿¡ º®À» ¶Õ?Eµé¾ûÌ¡´Â °æ?E?ÀÖ¾ûØ­..
	rvector p1 = pObj1->GetPosition() + rvector(0.f, 0.f, 100.f);
	rvector p2 = pObj2->GetPosition() + rvector(0.f, 0.f, 100.f);

	if (bCoherentToPeer)
	{
		p1.x = short(p1.x);
		p1.y = short(p1.y);
		p1.z = short(p1.z);
		p2.x = short(p2.x);
		p2.y = short(p2.y);
		p2.z = short(p2.z);
		// ¿ÀÂ÷·Î ÀÎÇÑ ¹ö±× Àç?EÅ×½ºÆ®¸¦ ½±°Ô ÇÏ?EÀ§ÇØ 1ÀÇ ÀÚ¸®±û?EÀý»çÇÑ ¹ö?E
		/*p1.x = (short(p1.x * 0.1f)) * 10.f;
		p1.y = (short(p1.y * 0.1f)) * 10.f;
		p1.z = (short(p1.z * 0.1f)) * 10.f;
		p2.x = (short(p2.x * 0.1f)) * 10.f;
		p2.y = (short(p2.y * 0.1f)) * 10.f;
		p2.z = (short(p2.z * 0.1f)) * 10.f;*/
	}

	rvector dir = p2 - p1;

	Normalize(dir);

	ZPICKINFO pickinfo;

	if (Pick(pObj1, p1, dir, &pickinfo)) {//º®ÀÌ¶ó?E
		if (pickinfo.bBspPicked)//¸ÊÀÌ °É¸°°æ?E
		{
			nDebugRegister = FOR_DEBUG_REGISTER;
			return true;
		}
	}
	nDebugRegister = -10;
	return false;
}
bool ZGame::IsWallBlocked(ZObject* pObj1, ZObject* pObj2, bool bCoherentToPeer)
{
	//### ÀÌ ÇÔ¼ö¸¦ ¼öÁ¤ÇÏ¸é ¶È°°ÀÌ IsWallBlocked()¿¡µµ Àû¿ëÇØ ÁÖ¾î¾ß ÇÕ´Ï´Ù. ###

	// ÇÇ¾î³¢¸® ÁÂÇ¥¸¦ º¸³¾¶§ ÇöÀç´Â float->short Ä³½ºÆÃÀÌ ÀÏ¾î³­´Ù (Áï ¼Ò¼öÁ¡ÀÌÇÏ ¹ö¸²)
	// µû¶ó¼­ Å¬¶óÀÌ¾ðÆ®µéÀÌ °¢ÀÚ ÆÇÁ¤ÇÑ °ªÀÌ ¹Ì¹¦ÇÏ°Ô ´Ù¸¦ ¼ö°¡ ÀÖ´Ù. ÀÌ°ÍÀÌ ±âÁ¸¿¡ ¹®Á¦¸¦ ÀÏÀ¸Å°Áø ¾Ê¾ÒÀ¸³ª
	// ¼­¹ÙÀÌ¹ú¿¡¼­ ³·Àº È®·ü·Î ¹®Á¦°¡ ¹ß»ý: npc°¡ ÇÃ·¹ÀÌ¾î¸¦ ±ÙÁ¢°ø°ÝÇÏ·Á°í ÇÒ¶§, npc ÄÁÆ®·Ñ·¯´Â °ø°Ý °¡´ÉÇÏ´Ù°í ÆÇÁ¤.
	// ÇÇ°Ý´çÇÏ´Â Å¬¶óÀÌ¾ðÆ®´Â °ø°Ý °¡´ÉÇÏÁö ¾Ê´Ù°í ÆÇÁ¤. ÀÌ·Î½á ÇÇ°ÝµÇ´Â À¯Àú°¡ À§Ä¡¸¦ ¹Ù²ÙÁö ¾Ê´ÂÇÑ ¸ó½ºÅÍ´Â Á¦ÀÚ¸®¿¡¼­ ¹«ÇÑ Çê¹æÀ» Ä¡°ÔµÊ (¼Ö±î¸» ¾Ç¿ë ºÒ°¡¶ó°í »ý°¢ÇÏÁö¸¸ ÆÛºí¸®¼ÅÀÇ ±Ù¼º¿¡ Á³À½)
	// bCoherentToPeer==true ÀÏ¶§ ÇÇ¾î¿¡°Ô º¸³½ °Í°ú °°Àº °ªÀ» »ç¿ëÇÔ..

	if ((pObj1 == NULL) || (pObj2 == NULL))
		return false;

	if ((pObj1->GetVisualMesh() == NULL) || (pObj2->GetVisualMesh() == NULL))
		return false;

	// ¿¡´Ï¸ÞÀÌ¼Ç ¶§¹®¿¡ º®À» ¶Õ°í µé¾î°¡´Â °æ¿ìµµ ÀÖ¾î¼­..
	rvector p1 = pObj1->GetPosition() + rvector(0.f, 0.f, 100.f);
	rvector p2 = pObj2->GetPosition() + rvector(0.f, 0.f, 100.f);

	if (bCoherentToPeer)
	{
		p1.x = short(p1.x);
		p1.y = short(p1.y);
		p1.z = short(p1.z);
		p2.x = short(p2.x);
		p2.y = short(p2.y);
		p2.z = short(p2.z);
		// ¿ÀÂ÷·Î ÀÎÇÑ ¹ö±× ÀçÇö Å×½ºÆ®¸¦ ½±°Ô ÇÏ±â À§ÇØ 1ÀÇ ÀÚ¸®±îÁö Àý»çÇÑ ¹öÀü
		/*p1.x = (short(p1.x * 0.1f)) * 10.f;
		p1.y = (short(p1.y * 0.1f)) * 10.f;
		p1.z = (short(p1.z * 0.1f)) * 10.f;
		p2.x = (short(p2.x * 0.1f)) * 10.f;
		p2.y = (short(p2.y * 0.1f)) * 10.f;
		p2.z = (short(p2.z * 0.1f)) * 10.f;*/
	}

	rvector dir = p2 - p1;

	Normalize(dir);

	ZPICKINFO pickinfo;

	if (Pick(pObj1, p1, dir, &pickinfo)) {//º®ÀÌ¶ó¸é
		if (pickinfo.bBspPicked)//¸ÊÀÌ °É¸°°æ¿ì
			return true;
	}

	return false;
}
//jintriple3 µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ·.....ºñ±³¸¦ À§ÇØ¼­
bool ZGame::IsWallBlocked(ZObject* pObj1, ZObject* pObj2, int& nDebugRegister/*´Ü¼ø ºñ±³¿ë*/, bool bCoherentToPeer)
{	//bCoherentToPeer¿¡ ´ëÇÑ°ÍÀº ¿øº» CheckWall ÁÖ¼® Âü°í
	if ((pObj1 == NULL) || (pObj2 == NULL))
	{
		nDebugRegister = -10;	//¿ª½Ã³ª ¼ýÀÚ´Â ÀÇ¹Ì°¡ ¾ø´Ù..
		return false;
	}

	if ((pObj1->GetVisualMesh() == NULL) || (pObj2->GetVisualMesh() == NULL))
	{
		nDebugRegister = -10;
		return false;
	}

	// ¿¡´Ï¸ÞÀÌ¼Ç ¶§¹®¿¡ º®À» ¶Õ°í µé¾î°¡´Â °æ¿ìµµ ÀÖ¾î¼­..
	rvector p1 = pObj1->GetPosition() + rvector(0.f, 0.f, 100.f);
	rvector p2 = pObj2->GetPosition() + rvector(0.f, 0.f, 100.f);

	if (bCoherentToPeer)
	{
		p1.x = short(p1.x);
		p1.y = short(p1.y);
		p1.z = short(p1.z);
		p2.x = short(p2.x);
		p2.y = short(p2.y);
		p2.z = short(p2.z);
		// ¿ÀÂ÷·Î ÀÎÇÑ ¹ö±× ÀçÇö Å×½ºÆ®¸¦ ½±°Ô ÇÏ±â À§ÇØ 1ÀÇ ÀÚ¸®±îÁö Àý»çÇÑ ¹öÀü
		/*p1.x = (short(p1.x * 0.1f)) * 10.f;
		p1.y = (short(p1.y * 0.1f)) * 10.f;
		p1.z = (short(p1.z * 0.1f)) * 10.f;
		p2.x = (short(p2.x * 0.1f)) * 10.f;
		p2.y = (short(p2.y * 0.1f)) * 10.f;
		p2.z = (short(p2.z * 0.1f)) * 10.f;*/
	}

	rvector dir = p2 - p1;

	Normalize(dir);

	ZPICKINFO pickinfo;

	if (Pick(pObj1, p1, dir, &pickinfo)) {//º®ÀÌ¶ó¸é
		if (pickinfo.bBspPicked)//¸ÊÀÌ °É¸°°æ¿ì
		{
			nDebugRegister = FOR_DEBUG_REGISTER;
			return true;
		}
	}
	nDebugRegister = -10;
	return false;
}
void ZGame::OnExplosionGrenade(MUID uidOwner, rvector pos, float fDamage, float fRange, float fMinDamage, float fKnockBack, MMatchTeam nTeamID)
{
	ZObject* pTarget = NULL;

	float fDist, fDamageRange;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		pTarget = (*itor).second;
		//jintriple3 ????? ???????? ??y ???? ????.....
		bool bReturnValue = !pTarget || pTarget->IsDie();
		if (!pTarget || pTarget->IsDie())
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		fDist = Magnitude(pos - (pTarget->GetPosition() + rvector(0, 0, 80)));
		//jintriple3 ????? ???????? ?? ???? ????.....
		bReturnValue = fDist >= fRange;
		if (fDist >= fRange)
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		rvector dir = pos - (pTarget->GetPosition() + rvector(0, 0, 80));
		Normalize(dir);

		// ???? ???? ?¾??.
		if (GetDistance(pos, pTarget->GetPosition() + rvector(0, 0, 50), pTarget->GetPosition() + rvector(0, 0, 130)) < 50)
		{
			fDamageRange = 1.f;
		}
		else
		{
#define MAX_DMG_RANGE	50.f	// ??????u ?????? ??? ???????? ?? ??´?
			//#define MIN_DMG			0.4f	// ??? ?? ???????? ??????.
			fDamageRange = 1.f - (1.f - fMinDamage) * (max(fDist - MAX_DMG_RANGE, 0) / (fRange - MAX_DMG_RANGE));
		}

		// ????z?? ?????? ??????? ???????.
		ZActor* pATarget = MDynamicCast(ZActor, pTarget);
		ZActorWithFSM* pFSMActor = MDynamicCast(ZActorWithFSM, pTarget);

		bool bPushSkip = false;

		if (pATarget)
		{
			bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
		}
		if (pFSMActor)
		{
			bPushSkip = pFSMActor->GetActorDef()->IsNeverBlasted();
		}

		if (!bPushSkip)
		{
			pTarget->AddVelocity(fKnockBack * 7.f * (fRange - fDist) * -dir);
		}
		else
		{
			ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
		}

		ZCharacter* pOwnerCharacter = (ZCharacter*)ZGetGame()->m_CharacterManager.Find(uidOwner);
		if (pOwnerCharacter)
		{
			CheckCombo(pOwnerCharacter, pTarget, !bPushSkip);
			CheckStylishAction(pOwnerCharacter);
		}

		float fActualDamage = fDamage * fDamageRange;
		float fRatio = ZItem::GetPiercingRatio(MWT_FRAGMENTATION, eq_parts_chest);//????z?? ???? ???????..
		pTarget->OnDamaged(pOwnerCharacter, pos, ZD_EXPLOSION, MWT_FRAGMENTATION, fActualDamage, fRatio);
		// 1. PHẦN LOGIC TÍNH TOÁN: Chỉ cộng dồn sát thương 1 lần duy nhất
		bool bWantFloat1 = false;
#ifdef _FLOATDMG_MODE
		bWantFloat1 = Z_VIDEO_FLOATDMGTEXT;
#endif

		bool bWantFloat2 = false;
#ifdef _FLOATDMG_MODE
		bWantFloat2 = Z_ETC_FLOATDMG2;
#endif

		// Nếu mình là người gây dame và trúng mục tiêu khác mình
		if (uidOwner == ZGetMyUID() && pTarget->GetUID() != ZGetMyUID() && fActualDamage > 0)
		{
			// Kiểm tra xem ít nhất 1 trong 2 cái đang bật thì mới cộng dồn vào bảng điểm
			if (bWantFloat1 || bWantFloat2)
			{
				ZGetGame()->m_pMyCharacter->GetStatus().CheckCrc();
				ZGetGame()->m_pMyCharacter->GetStatus().Ref().nRoundGivenDamage += (int)fActualDamage;
				ZGetGame()->m_pMyCharacter->GetStatus().MakeCrc();

				// ZPostDamageCounter(fActualDamage, ZGetMyUID(), pTarget->GetUID(), false);
			}

			// 2. PHẦN LOGIC HIỂN THỊ: Chỉ vẽ hiệu ứng nếu bFloatdmg2 được bật
			if (bWantFloat2)
			{
				ZGetEffectManager()->AddDamageEffect((int)fActualDamage, pTarget, pOwnerCharacter);
			}
		}
		// 				pTarget->OnDamagedGrenade( uidOwner, dir, fDamage * fDamageRange, nTeamID);

				/*if(pTarget && !pTarget->IsDie())
				{
					fDist = Magnitude(pos-(pTarget->GetPosition()+rvector(0,0,80)));
					if(fDist < fRange)
					{
						// ?? ????????? ?????? ????????~
		//				if( CheckWall(pos,pTarget) == false )
						{
							rvector dir=pos-(pTarget->GetPosition()+rvector(0,0,80));
							Normalize(dir);

							// ???? ???? ?¾??.
							if(GetDistance(pos,pTarget->GetPosition()+rvector(0,0,50),pTarget->GetPosition()+rvector(0,0,130))<50)
							{
								fDamageRange = 1.f;
							}
							else
							{
		#define MAX_DMG_RANGE	50.f	// ??????u ?????? ??? ???????? ?? ??´?
		//#define MIN_DMG			0.4f	// ??? ?? ???????? ??????.
								fDamageRange = 1.f - (1.f-fMinDamage)*( max(fDist-MAX_DMG_RANGE,0) / (fRange-MAX_DMG_RANGE));
							}

							// ????z?? ?????? ??????? ???????.
							ZActor* pATarget = MDynamicCast(ZActor,pTarget);

							bool bPushSkip = false;

							if(pATarget)
							{
								bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
							}

							if(bPushSkip==false)
							{
								pTarget->AddVelocity(fKnockBack*7.f*(fRange-fDist)*-dir);
							}
							else
							{
								ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
							}

							ZCharacterClass* pOwnerCharacter = g_pGame->m_CharacterManager.Find( uidOwner );
							if(pOwnerCharacter)
							{
								CheckCombo(pOwnerCharacter, pTarget,!bPushSkip);
								CheckStylishAction(pOwnerCharacter);
							}

							float fActualDamage = fDamage * fDamageRange;
							float fRatio = ZItem::GetPiercingRatio( MWT_FRAGMENTATION , eq_parts_chest );//????z?? ???? ???????..
							pTarget->OnDamaged(pOwnerCharacter,pos,ZD_EXPLOSION,MWT_FRAGMENTATION,fActualDamage,fRatio);
		//					pTarget->OnDamagedGrenade( uidOwner, dir, fDamage * fDamageRange, nTeamID);
						}
					}
				}*/
	}

#define SHOCK_RANGE		1500.f			// 10??????? ?????

	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	float fPower = (SHOCK_RANGE - Magnitude(pTargetCharacter->GetPosition() + rvector(0, 0, 50) - pos)) / SHOCK_RANGE;

	if (fPower > 0)
		ZGetGameInterface()->GetCamera()->Shock(fPower * 500.f, .5f, rvector(0.0f, 0.0f, -1.0f));

	GetWorld()->GetWaters()->CheckSpearing(pos, pos + rvector(0, 0, MAX_WATER_DEEP), 500, 0.8f);

	//	static RealSoundEffectSource* pSES= ZApplication::GetSoundEngine()->GetSES("explosion");
	//	ZApplication::GetSoundEngine()->PlaySE(pSES,pos.x,pos.y,pos.z);
}
void ZGame::OnExplosionMagic(ZWeaponMagic* pWeapon, MUID uidOwner, rvector pos, float fMinDamage, float fKnockBack, MMatchTeam nTeamID, bool bSkipNpc)
{
	ZObject* pTarget = NULL;

	float fRange = 100.f * pWeapon->GetDesc()->fEffectArea;
	float fDist, fDamageRange;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		pTarget = (*itor).second;
		// ?E§°ø°ÝÀ?¾Æ´Ï¶ó?EÅ¸°Ù¸¸ °Ë»çÇÏ¸éµÈ´Ù.
		// µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ· ¹æ?EÄÚ?E
		bool bForDebugRegister = !pTarget || pTarget->IsDie() || pTarget->IsNPC();
		if (!pTarget || pTarget->IsDie() || pTarget->IsNPC())
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
			continue;
		bForDebugRegister = !pWeapon->GetDesc()->IsAreaTarget() && pWeapon->GetTarget() != pTarget->GetUID();
		if (!pWeapon->GetDesc()->IsAreaTarget() && pWeapon->GetTarget() != pTarget->GetUID())
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
			continue;

		ZObject* pOwner = ZGetObjectManager()->GetObject(uidOwner);

		bForDebugRegister = !pOwner || pOwner->IsDie();
		if (!pOwner || pOwner->IsDie())
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
			return;

		// µÎ Ä³¸¯ÅÍ»çÀÌ¿¡ Àå¾Ö¹°ÀÌ ¾ø¾ûÚßÇÑ´Ù~
		//				if( CheckWall(pos,pTarget) == false )
		{
			fDist = Magnitude(pos - (pTarget->GetPosition() + rvector(0, 0, 80)));
			if (pWeapon->GetDesc()->IsAreaTarget())	// ?E§°ø°ÝÀÌ¸E°Å¸®¿¡ µû¸¥ µ¥¹ÌÁö¸¦ °è?EÑ´?
			{
				PROTECT_DEBUG_REGISTER(fDist > fRange) continue;	// ?E§¸?¹þ¾ûÏµ´Ù

				// ¸ö¿¡ Á÷Á¢ ¸Â¾Ò´Ù.
				if (GetDistance(pos, pTarget->GetPosition() + rvector(0, 0, 50), pTarget->GetPosition() + rvector(0, 0, 130)) < 50)
				{
					fDamageRange = 1.f;
				}
				else
				{
#define MAX_DMG_RANGE	50.f	// ¹Ý°æÀÌ¸¸Å­ ±ûÝö´Â ÃÖ?Eµ¥¹ÌÁö¸¦ ´Ù ¸Ô´Â´Ù

					fDamageRange = 1.f - (1.f - fMinDamage) * (max(fDist - MAX_DMG_RANGE, 0) / (fRange - MAX_DMG_RANGE));
				}
			}
			else
			{
				fDamageRange = 1.f;
			}

			// resist ¸¦ Ã¼Å©ÇÑ´Ù
			//µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ· ¹æ?EÄÚ?E»ðÀÔ.
			float fDamage = pWeapon->GetDesc()->nModDamage;
			bForDebugRegister = pWeapon && pWeapon->GetDesc()->CheckResist(pTarget, &fDamage);
			if (!(pWeapon->GetDesc()->CheckResist(pTarget, &fDamage)))
				PROTECT_DEBUG_REGISTER(bForDebugRegister)
				continue;

			ZCharacter* pOwnerCharacter = (ZCharacter*)ZGetGame()->m_CharacterManager.Find(uidOwner);
			if (pOwnerCharacter)
			{
				CheckCombo(pOwnerCharacter, pTarget, true);
				CheckStylishAction(pOwnerCharacter);
			}

			// ¼ö·ùÅºÀ» ¸ÂÀ¸?E¹Ýµ¿À¸·Î Æ¢¾ûÏª°£´Ù.
			rvector dir = pos - (pTarget->GetPosition() + rvector(0, 0, 80));
			Normalize(dir);
			pTarget->AddVelocity(fKnockBack * 7.f * (fRange - fDist) * -dir);

			float fActualDamage = fDamage * fDamageRange;
			float fRatio = ZItem::GetPiercingRatio(MWT_FRAGMENTATION, eq_parts_chest);//¼ö·ùÅº?E·ÎÄÏ ±¸ºÐ¾ø´Ù..

			if (m_pMyCharacter && pTarget->GetUID() == m_pMyCharacter->GetUID())
				m_pMyCharacter->OnDamaged(pOwner, pos, ZD_MAGIC, MWT_FRAGMENTATION, fActualDamage, fRatio);
			else
				pTarget->OnDamaged(pOwner, pos, ZD_MAGIC, MWT_FRAGMENTATION, fActualDamage, fRatio);

			// resist ¸¦ Ã¼Å©ÇÑ´Ù
			/*			float fDamage = pWeapon->GetDesc()->nModDamage;
			if(pWeapon->GetDesc()->CheckResist(pTarget,&fDamage))
			{
			ZCharacter* pOwnerCharacter = g_pGame->m_CharacterManager.Find( uidOwner );
			if(pOwnerCharacter)
			{
			CheckCombo(pOwnerCharacter, pTarget,true);
			CheckStylishAction(pOwnerCharacter);
			}

			// ¼ö·ùÅºÀ» ¸ÂÀ¸?E¹Ýµ¿À¸·Î Æ¢¾ûÏª°£´Ù.
			rvector dir=pos-(pTarget->GetPosition()+rvector(0,0,80));
			Normalize(dir);
			pTarget->AddVelocity(fKnockBack*7.f*(fRange-fDist)*-dir);

			float fActualDamage = fDamage * fDamageRange;
			float fRatio = ZItem::GetPiercingRatio( MWT_FRAGMENTATION , eq_parts_chest );//¼ö·ùÅº?E·ÎÄÏ ±¸ºÐ¾ø´Ù..
			pTarget->OnDamaged(pOwnerCharacter,pos,ZD_MAGIC,MWT_FRAGMENTATION,fActualDamage,fRatio);
			}
			else
			{
			// TODO: ÀúÇ×¿¡ ¼º°øÇßÀ¸´Ï ÀÌÆåÆ®¸¦ º¸¿©ÁÖÀÚ.
			}*/
		}
	}

	/*
	#define SHOCK_RANGE		1500.f			// 10¹ÌÅÍ±û?EÈçµé¸°´Ù

	ZCharacter *pTargetCharacter=ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	float fPower= (SHOCK_RANGE-Magnitude(pTargetCharacter->GetPosition()+rvector(0,0,50) - pos))/SHOCK_RANGE;

	if ((fPower>0) && (pWeapon->GetDesc()->bCameraShock))
	ZGetGameInterface()->GetCamera()->Shock(fPower*500.f, .5f, rvector(0.0f, 0.0f, -1.0f));
	*/

	if (pWeapon->GetDesc()->bCameraShock)
	{
		ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
		const float fDefaultPower = 500.0f;
		float fShockRange = pWeapon->GetDesc()->fCameraRange;
		float fDuration = pWeapon->GetDesc()->fCameraDuration;
		float fPower = (fShockRange - Magnitude(pTargetCharacter->GetPosition() + rvector(0, 0, 50) - pos)) / fShockRange;
		fPower *= pWeapon->GetDesc()->fCameraPower;

		if (fPower > 0)
		{
			ZGetGameInterface()->GetCamera()->Shock(fPower * fDefaultPower, fDuration, rvector(0.0f, 0.0f, -1.0f));
		}
	}

	GetWorld()->GetWaters()->CheckSpearing(pos, pos + rvector(0, 0, MAX_WATER_DEEP), 500, 0.8f);
}

// ¸ÅÁ÷·ùÀÇ ¹«±âÀÇ µ¥¹ÌÁö¸¦ ÁØ´Ù
//jintriple3 µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ· ¹æ?EÄÚ?E»ðÀÔ
void ZGame::OnExplosionMagicThrow(ZWeaponMagic* pWeapon, MUID uidOwner, rvector pos, float fMinDamage, float fKnockBack, MMatchTeam nTeamID, bool bSkipNpc, rvector from, rvector to)
{
	ZObject* pTarget = NULL;

	float fDist, fDamageRange;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		pTarget = (*itor).second;
		// ?E§°ø°ÝÀ?¾Æ´Ï¶ó?EÅ¸°Ù¸¸ °Ë»çÇÏ¸éµÈ´Ù.
		// µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ· ¹æ?EÄÚ?E
		bool bForDebugRegister = !pTarget || pTarget->IsDie() || pTarget->IsNPC();
		if (!pTarget || pTarget->IsDie() || pTarget->IsNPC())
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
			continue;
		bForDebugRegister = !pWeapon->GetDesc()->IsAreaTarget() && pWeapon->GetTarget() != pTarget->GetUID();
		if (!pWeapon->GetDesc()->IsAreaTarget() && pWeapon->GetTarget() != pTarget->GetUID())
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
			continue;

		ZObject* pOwner = ZGetObjectManager()->GetObject(uidOwner);

		bForDebugRegister = !pOwner || pOwner->IsDie();
		if (!pOwner || pOwner->IsDie())
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
			return;

		fDist = GetDistance(pTarget->GetPosition() + rvector(0, 0, 80), from, to);

		if (fDist > pWeapon->GetDesc()->fColRadius + 100)
			continue;

		if (pWeapon->GetDesc()->IsAreaTarget())	// ?E§°ø°ÝÀÌ¸E°Å¸®¿¡ µû¸¥ µ¥¹ÌÁö¸¦ °è?EÑ´?
		{
			if (fDist > pWeapon->GetDesc()->fColRadius + 80) // Ä³¸¯ÅÍÅ©±â¸¦ 160 Á¤µµ·Î Àâ?E±¸·Î Ä¡?E¹ÝÁö¸§Àº 80 Á¤µµ·Î °è?E
			{
				fDamageRange = 0.1f;
			}
			else
			{
				fDamageRange = 1.f - 0.9f * fDist / (pWeapon->GetDesc()->fColRadius + 80);
			}
		}
		else
		{
			fDamageRange = 1.f;
		}

		// resist ¸¦ Ã¼Å©ÇÑ´Ù
		//µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ· ¹æ?EÄÚ?E»ðÀÔ.
		float fDamage = pWeapon->GetDesc()->nModDamage;
		bForDebugRegister = pWeapon && pWeapon->GetDesc()->CheckResist(pTarget, &fDamage);
		if (!(pWeapon->GetDesc()->CheckResist(pTarget, &fDamage)))
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
			continue;

		ZCharacter* pOwnerCharacter = (ZCharacter*)ZGetGame()->m_CharacterManager.Find(uidOwner);
		if (pOwnerCharacter)
		{
			CheckCombo(pOwnerCharacter, pTarget, true);
			CheckStylishAction(pOwnerCharacter);
		}

		float fActualDamage = fDamage * fDamageRange;
		float fRatio = ZItem::GetPiercingRatio(MWT_FRAGMENTATION, eq_parts_chest);//¼ö·ùÅº?E·ÎÄÏ ±¸ºÐ¾ø´Ù..

		if (m_pMyCharacter && pTarget->GetUID() == m_pMyCharacter->GetUID())
			m_pMyCharacter->OnDamaged(pOwner, pos, ZD_MAGIC, MWT_FRAGMENTATION, fActualDamage, fRatio);
		else
			pTarget->OnDamaged(pOwner, pos, ZD_MAGIC, MWT_FRAGMENTATION, fActualDamage, fRatio);
	}

	if (pWeapon->GetDesc()->bCameraShock)
	{
		ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
		const float fDefaultPower = 500.0f;
		float fShockRange = pWeapon->GetDesc()->fCameraRange;
		float fDuration = pWeapon->GetDesc()->fCameraDuration;
		float fPower = (fShockRange - Magnitude(pTargetCharacter->GetPosition() + rvector(0, 0, 50) - pos)) / fShockRange;
		fPower *= pWeapon->GetDesc()->fCameraPower;

		if (fPower > 0)
		{
			ZGetGameInterface()->GetCamera()->Shock(fPower * fDefaultPower, fDuration, rvector(0.0f, 0.0f, -1.0f));
		}
	}

	GetWorld()->GetWaters()->CheckSpearing(pos, pos + rvector(0, 0, MAX_WATER_DEEP), 500, 0.8f);
}

//µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ· ¹æ?EÄÚ?E»ðÀÔ
void ZGame::OnExplosionMagicNonSplash(ZWeaponMagic* pWeapon, MUID uidOwner, MUID uidTarget, rvector pos, float fKnockBack)
{
	ZObject* pTarget = m_CharacterManager.Find(uidTarget);
	bool bForDebugRegister = pTarget == NULL || pTarget->IsNPC();
	if (pTarget == NULL || pTarget->IsNPC()) PROTECT_DEBUG_REGISTER(bForDebugRegister) return;

	bForDebugRegister = !pTarget || pTarget->IsDie();
	if (!pTarget || pTarget->IsDie())
		PROTECT_DEBUG_REGISTER(bForDebugRegister)
		return;

	ZObject* pOwner = ZGetObjectManager()->GetObject(uidOwner);

	bForDebugRegister = !pOwner || pOwner->IsDie();
	if (!pOwner || pOwner->IsDie())
		PROTECT_DEBUG_REGISTER(bForDebugRegister)
		return;

	// resist ¸¦ Ã¼Å©ÇÑ´Ù
	float fDamage = pWeapon->GetDesc()->nModDamage;
	bForDebugRegister = pWeapon && pWeapon->GetDesc()->CheckResist(pTarget, &fDamage);
	if (!pWeapon->GetDesc()->CheckResist(pTarget, &fDamage))
		PROTECT_DEBUG_REGISTER(bForDebugRegister)
		return;

	ZCharacter* pOwnerCharacter = (ZCharacter*)ZGetGame()->m_CharacterManager.Find(uidOwner);
	if (pOwnerCharacter)
	{
		CheckCombo(pOwnerCharacter, pTarget, true);
		CheckStylishAction(pOwnerCharacter);
	}
	// ¹Ýµ¿À¸·Î Æ¢¾ûÏª°£´Ù.
	rvector dir = pos - (pTarget->GetPosition() + rvector(0, 0, 80));
	Normalize(dir);
	pTarget->AddVelocity(fKnockBack * 7.f * -dir);

	float fRatio = ZItem::GetPiercingRatio(MWT_FRAGMENTATION, eq_parts_chest);//¼ö·ùÅº?E·ÎÄÏ ±¸ºÐ¾ø´Ù..
	if (m_pMyCharacter && pTarget->GetUID() == m_pMyCharacter->GetUID())
		m_pMyCharacter->OnDamaged(pOwner, pos, ZD_MAGIC, MWT_FRAGMENTATION, fDamage, fRatio);
	else
		pTarget->OnDamaged(pOwner, pos, ZD_MAGIC, MWT_FRAGMENTATION, fDamage, fRatio);

	/*	if(pTarget && !pTarget->IsDie()) {
			// resist ¸¦ Ã¼Å©ÇÑ´Ù
			float fDamage = pWeapon->GetDesc()->nModDamage;
			if(pWeapon->GetDesc()->CheckResist(pTarget,&fDamage))
			{
				ZCharacter* pOwnerCharacter = g_pGame->m_CharacterManager.Find( uidOwner );
				if(pOwnerCharacter) {
					CheckCombo(pOwnerCharacter, pTarget,true);
					CheckStylishAction(pOwnerCharacter);
				}

				// ¹Ýµ¿À¸·Î Æ¢¾ûÏª°£´Ù.
				rvector dir=pos-(pTarget->GetPosition()+rvector(0,0,80));
				Normalize(dir);
				pTarget->AddVelocity(fKnockBack*7.f*-dir);

				float fRatio = ZItem::GetPiercingRatio( MWT_FRAGMENTATION , eq_parts_chest );//¼ö·ùÅº?E·ÎÄÏ ±¸ºÐ¾ø´Ù..
				pTarget->OnDamaged(pOwnerCharacter,pos,ZD_MAGIC,MWT_FRAGMENTATION,fDamage,fRatio);
			} else {
				// TODO: ÀúÇ×¿¡ ¼º°øÇßÀ¸´Ï ÀÌÆåÆ®¸¦ º¸¿©ÁÖÀÚ.
			}
		}*/
}

int ZGame::SelectSlashEffectMotion(ZCharacter* pCharacter)
{
	// Custom: Exploit fix (pCharacter->GetSelectItemDesc() == NULL crash)
	if (pCharacter == NULL || pCharacter->GetSelectItemDesc() == NULL) return SEM_None;

	// ³²³à°¡ °°¾ÆÁ³Áö¸¸ È¤½Ã ¶Ç ¹Ù²ð?E¸ð¸£´Ï ³öµÐ´Ù~~

//	MWT_DAGGER
//	MWT_DUAL_DAGGER
//	MWT_KATANA
//	MWT_GREAT_SWORD
//	MWT_DOUBLE_KATANA

	ZC_STATE_LOWER lower = pCharacter->m_AniState_Lower.Ref();

	int nAdd = 0;
	int ret = 0;

	MMatchWeaponType nType = pCharacter->GetSelectItemDesc()->m_nWeaponType.Ref();

	if (pCharacter->IsMan()) {
		if (lower == ZC_STATE_LOWER_ATTACK1) { nAdd = 0; }
		else if (lower == ZC_STATE_LOWER_ATTACK2) { nAdd = 1; }
		else if (lower == ZC_STATE_LOWER_ATTACK3) { nAdd = 2; }
		else if (lower == ZC_STATE_LOWER_ATTACK4) { nAdd = 3; }
		else if (lower == ZC_STATE_LOWER_ATTACK5) { nAdd = 4; }
		else if (lower == ZC_STATE_LOWER_UPPERCUT) { return SEM_ManUppercut; }

		if (nType == MWT_KATANA)		return SEM_ManSlash1 + nAdd;
		else if (nType == MWT_DOUBLE_KATANA)	return SEM_ManDoubleSlash1 + nAdd;
		else if (nType == MWT_GREAT_SWORD)	return SEM_ManGreatSwordSlash1 + nAdd;
		else if (nType == MWT_SPYCASE)		return SEM_ManSlash1 + nAdd;
	}
	else {
		if (lower == ZC_STATE_LOWER_ATTACK1) { nAdd = 0; }
		else if (lower == ZC_STATE_LOWER_ATTACK2) { nAdd = 1; }
		else if (lower == ZC_STATE_LOWER_ATTACK3) { nAdd = 2; }
		else if (lower == ZC_STATE_LOWER_ATTACK4) { nAdd = 3; }
		else if (lower == ZC_STATE_LOWER_ATTACK5) { nAdd = 4; }
		else if (lower == ZC_STATE_LOWER_UPPERCUT) { return SEM_WomanUppercut; }

		if (nType == MWT_KATANA)		return SEM_WomanSlash1 + nAdd;
		else if (nType == MWT_DOUBLE_KATANA)	return SEM_WomanDoubleSlash1 + nAdd;
		else if (nType == MWT_GREAT_SWORD)	return SEM_WomanGreatSwordSlash1 + nAdd;
		else if (nType == MWT_SPYCASE)		return SEM_WomanSlash1 + nAdd;
	}

	return SEM_None;
}

// shot ÀÌ ³Ê¹« Ä¿¼­ ºÐ¸®..
void ZGame::OnPeerShot_Melee(const MUID& uidOwner, float fShotTime)
{
	// ??? ??? ??
	ZObject* pAttacker = m_ObjectManager.GetObject(uidOwner);

	if (!pAttacker) return;

	ZCharacter* pOwnerChar = (ZCharacter*)pAttacker;

#ifdef _ZGUARD
	
#endif

	float time = fShotTime;
	//jintriple3 ??? ???? ? ??? ??
	bool bReturnValue = !pAttacker;
	if (!pAttacker)
		PROTECT_DEBUG_REGISTER(bReturnValue)
		return;

	// Melee ??? ?E???? ??
	ZItem* pItem = pAttacker->GetItems()->GetItem(MMCIP_MELEE);
	//jintriple3    ??? ????? ? ??E..
	bReturnValue = !pItem;
	if (!pItem)
		PROTECT_DEBUG_REGISTER(bReturnValue)
		return;

	MMatchItemDesc* pItemDesc = pItem->GetDesc();
	//jintriple3    ??Eassert? ??? ?...?? ??E.
	bReturnValue = !pItemDesc;
	if (!pItemDesc)
		PROTECT_DEBUG_REGISTER(bReturnValue)
	{
		_ASSERT(FALSE);
		return;
	}

	// ?? ??? ?E?????.
	float fRange = pItemDesc->m_nRange.Ref();
	if (fRange == 0)
		fRange = 150.f;

	// ?? ??? ??? ???.
	float fAngle = cosf(ToRadian(pItemDesc->m_nAngle.Ref() * 0.5f));

	// NPC? ??E??? ?? ?E?????.
	if (pAttacker->IsNPC())
	{
		fRange += fRange * 0.2f;            // ??? 20% ??
		fAngle -= fAngle * 0.1f;            // ??? 10% ??
	}

	// ?? ??? ?E???? ??
	fShotTime = GetTime();

	// ???E??
	//ZGetSoundEngine()->PlaySound( "blade_swing", pAttacker->GetPosition(), uidOwner==ZGetGameInterface()->GetCombatInterface()->GetTargetUID());

	// ???? ??? ??? ?E????
	rvector AttackerPos = pAttacker->GetPosition();
	rvector AttackerNorPos = AttackerPos;
	AttackerNorPos.z = 0;

	rvector AttackerDir = pAttacker->m_Direction;
	rvector AttackerNorDir = AttackerDir;
	AttackerNorDir.z = 0;
	Normalize(AttackerNorDir);

	// ??E? ???? ??? ?????
	int cm = 0;
	ZCharacter* pOwnerCharacter = (ZCharacter*)m_CharacterManager.Find(uidOwner);
	if (pOwnerCharacter)
		cm = SelectSlashEffectMotion(pOwnerCharacter);

	// ???E??
	//if ( !pAttacker->IsNPC())
	//{
	//  ZGetSoundEngine()->PlaySound( "blade_swing", pAttacker->m_Position, uidOwner==ZGetGameInterface()->GetCombatInterface()->GetTargetUID());
	//}

	// ???E??
	bool bPlayer = false;
	rvector Pos = pAttacker->GetPosition();
	if (pAttacker == m_pMyCharacter)
	{
		Pos = RCameraPosition;
		bPlayer = true;
	}

	// default
	ZApplication::GetSoundEngine()->PlaySoundElseDefault("blade_swing", "blade_swing", rvector(Pos.x, Pos.y, Pos.z), bPlayer);

	// ??? ??? ?? ???E????.
	bool bHit = false;

	// ??? ?E???? ????.
	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		// ??? ?E???? ???.
		ZObject* pVictim = (*itor).second;

		// ??? ?? ???E?? ???? ????.
		//jintriple3    ??? ???? ? ??E...
		ZModule_HPAP* pModule = (ZModule_HPAP*)pVictim->GetModule(ZMID_HPAP);
		if (pVictim->IsDie())
			PROTECT_DEBUG_REGISTER(pModule->GetHP() == 0)
			continue;

		// ??? ??? ????E?? ???? ????.
		//jintriple3    ??? ???? ? ??E...
		bReturnValue = pAttacker == pVictim;
		if (pAttacker == pVictim)
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		// ?? ??? ??E????E?? ???? ????.
		//jintriple3    ??? ???? ? ??E...
		bool bRetVal = CanAttack(pAttacker, pVictim);
		if (!bRetVal)
			PROTECT_DEBUG_REGISTER(!CanAttack_DebugRegister(pAttacker, pVictim))
			continue;

		// ??? ??? ???.
		rvector VictimPos, VictimDir, VictimNorDir;
		rvector ZeroVector = rvector(0.0f, 0.0f, 0.0f);
		VictimPos = ZeroVector;
		bRetVal = pVictim->GetHistory(&VictimPos, &VictimDir, fShotTime);
		//jintriple3    ??? ???? ? ??E...
		if (!bRetVal)
			PROTECT_DEBUG_REGISTER(VictimPos == ZeroVector)
			continue;

		// NPC? ?? ??? ??? ?...
		if (!pAttacker->IsNPC())
		{
			// ???? ???? ??? ???.
			rvector swordPos = AttackerPos + (AttackerNorDir * 100.f);
			swordPos.z += pAttacker->GetCollHeight() * .5f;
			float fDist = GetDistanceLineSegment(swordPos, VictimPos, VictimPos + rvector(0, 0, pVictim->GetCollHeight()));

			// ???? ???? ??? ?? ?? ???? ??E?? ???? ????.
			//jintriple3    ??? ???? ?? ??E...
			bReturnValue = fDist > fRange;
			if (fDist > fRange)
				PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;

			// ???? ?E???? ??? ??? ???
			rvector VictimNorDir = VictimPos - (AttackerPos - (AttackerNorDir * 50.f));
			Normalize(VictimNorDir);

			// ??? ?? ??? ?? ???E?? ???? ????.
			//jintriple3    ??? ???? ? ??E...
			float fDot = D3DXVec3Dot(&AttackerNorDir, &VictimNorDir);
			bReturnValue = fDot < 0.5f;
			if (fDot < 0.5f)
				PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;
		}

		// ??? ?? NPC? ??? ?...  (??? ???? ??E?? ??? ??? ??? ???? ????)
		// (??????? ??? ?? ZActorAction?? ????? ??)
		else
		{
			// ???? ?E???? ??? ??E???
			rvector VictimNorPos = VictimPos;
			VictimNorPos.z = 0;

			rvector VictimNorDir = VictimPos - (AttackerPos - (AttackerNorDir * 50.f));
			VictimNorDir.z = 0;
			Normalize(VictimNorDir);

			// ???? ???? x,y ??E?? ??? ?? ??, ??? ?? ?? ???? ??E?? ???? ????.
			//jintriple3    ??? ???? ? ??E...
			float fDist = Magnitude(AttackerNorPos - VictimNorPos);
			bReturnValue = fDist > fRange;
			if (fDist > fRange)
				PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;

			// ??? ?? ??? ?? ???E?? ???? ????.
			float fDot = D3DXVec3Dot(&AttackerNorDir, &VictimNorDir);
			//jintriple3    ??? ???? ? ??E...
			bReturnValue = fDot < fAngle;
			if (fDot < fAngle)
				PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;

			// ??? ??E?? ?E???? ?? ?? ??? ???E????.
			//jintriple3    ??? ???? ? ??E...
			int nDebugRegister = 0;
			if (!InRanged(pAttacker, pVictim, nDebugRegister))
				PROTECT_DEBUG_REGISTER(nDebugRegister != FOR_DEBUG_REGISTER)
				continue;
		}

		// ???? ?? ??? ?? ??E???E?? ???? ????.
		int nDebugRegister = 0;
		bRetVal = CheckWall(pAttacker, pVictim, nDebugRegister, true);
		//jintriple3 ??? ???? ? ??E...
		if (bRetVal)
			PROTECT_DEBUG_REGISTER(nDebugRegister == FOR_DEBUG_REGISTER)
			continue;

		// ??E????? ??E??? ???E??E??E
		//jintriple3 ??? ???? ? ??E..
		//bRetVal = pVictim->IsGuard() && (DotProduct(pVictim->m_Direction, AttackerNorDir) < 0);
		//if (pVictim->IsGuard() && (DotProduct(pVictim->m_Direction, AttackerNorDir) < 0))
		//{
		//	PROTECT_DEBUG_REGISTER(bRetVal)
		//	{
		//		rvector pos = pVictim->GetPosition();
		//		pos.z += 120.f;

		//		// ??E??? ??
		//		ZGetEffectManager()->AddSwordDefenceEffect(pos + (pVictim->m_Direction * 50.f), pVictim->m_Direction);
		//		pVictim->OnMeleeGuardSuccess();
		//		return;
		//	}
		//}
		// =========================================================================
		// CUSTOM: XỬ LÝ GUARD & DỘI ĐÒN KẺ CHÉM (COUNTER BLOCK)
		// =========================================================================
		bRetVal = pVictim->IsGuard() && (DotProduct(pVictim->m_Direction, AttackerNorDir) < 0);
		if (pVictim->IsGuard() && (DotProduct(pVictim->m_Direction, AttackerNorDir) < 0))
		{
			PROTECT_DEBUG_REGISTER(bRetVal)
			{
				rvector pos = pVictim->GetPosition();
				pos.z += 120.f;

				// Hiệu ứng đỡ kiếm gốc của GunZ
				ZGetEffectManager()->AddSwordDefenceEffect(pos + (pVictim->m_Direction * 50.f), pVictim->m_Direction);

				ZCharacter* pAttackerChar = MDynamicCast(ZCharacter, pAttacker);
				ZMyCharacter* pMyVictim = MDynamicCast(ZMyCharacter, pVictim);

				bool bShouldBlock = false;

				if (pMyVictim)
				{
					// Nếu người đỡ là chính mình: Lấy m_fGuardStartTime từ m_timeInfo
					float fGuardStartTime = pMyVictim->m_timeInfo.Ref().m_fGuardStartTime;
					float fGuardDuration = GetTime() - fGuardStartTime;

					// Canh Guard chuẩn (< 0.5s) -> 100% dội đòn
					if (fGuardDuration <= 0.5f)
					{
						bShouldBlock = true;
					}
					else
					{
						// Hold Guard lâu (> 0.5s) -> Tỷ lệ 30%
						if ((rand() % 100) < 30)
						{
							bShouldBlock = true;
						}
					}
				}
				else
				{
					// Nếu người đỡ là Peer/Bot khác
					if ((rand() % 100) < 30)
					{
						bShouldBlock = true;
					}
				}

				// Nếu trúng tỷ lệ dội đòn -> Bắt KẺ CHÉM bị ngửa người lập tức
				if (bShouldBlock && pAttackerChar)
				{
					// Dừng vận tốc kẻ chém
					pAttackerChar->SetVelocity(0, 0, 0);

					// Reset status cũ để không bị đè animation trúng hit trước đó
					//pAttackerChar->InitStatus();

					// Ép cả Lower & Upper về trạng thái bị Dội Kiếm (BLOCKED)
					pAttackerChar->SetAnimationLower(ZC_STATE_DAMAGE_BLOCKED);

					// Khựng kẻ chém
					pAttackerChar->m_dwStatusBitPackingValue.Ref().m_bStun = true;
					pAttackerChar->m_dwStatusBitPackingValue.Ref().m_bPlayDone = false;

					// Nếu kẻ chém chính là MyCharacter thì gọi ShotBlocked() để ngắt phím
					ZMyCharacter* pMyAttacker = MDynamicCast(ZMyCharacter, pAttackerChar);
					if (pMyAttacker)
					{
						pMyAttacker->ShotBlocked();
					}
				}

				// Gọi hàm thưởng cho người thủ (Tích Counter/Massives)
				pVictim->OnMeleeGuardSuccess();
				return;
			}
		}
		// =========================================================================

		// ?E?E???? ??? ????E..
		rvector pos = pVictim->GetPosition();
		pos.z += 130.f;
		pos -= AttackerDir * 50.f;

		// ???? ????.
		ZGetEffectManager()->AddBloodEffect(pos, -VictimNorDir);
#ifdef _PAINTMODE
		if (ZGetGame()->GetMatch()->IsPaintBall())
		{
			ZGetEffectManager()->AddPaintEffect(pos, -VictimNorDir);
	    }
#endif
		ZGetEffectManager()->AddSlashEffect(pos, -VictimNorDir, cm);

		// Custom: Force Melee Damage room mod - chỉ override khi có giá trị hợp lệ,
		// nếu mod active nhưng value=-1 (checkbox bật nhưng ô trống) thì giữ nguyên damage gốc của vũ khí.
		float fBaseMeleeDmg = (float)pItemDesc->m_nDamage.Ref();
		if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_FORCEMELEEDMG))
		{
			int nForce = ZGetGameClient()->GetMatchStageSetting()->GetModifierValue(MMOD_FORCEMELEEDMG);
			if (nForce >= 1 && nForce <= 999)
				fBaseMeleeDmg = (float)nForce;
		}

#ifdef _CALCDMG
		float fActualDamage = CalcActualDamage(pAttacker, pVictim, fBaseMeleeDmg, pItem->GetDesc()->m_nWeaponType.Ref(), pItem);
#else
		float fActualDamage = CalcActualDamage(pAttacker, pVictim, fBaseMeleeDmg, pItem->GetDesc()->m_nWeaponType.Ref());
#endif

		float fRatio = pItem->GetPiercingRatio(pItemDesc->m_nWeaponType.Ref(), eq_parts_chest);
		pVictim->OnDamaged(pAttacker, pAttacker->GetPosition(), ZD_MELEE, pItemDesc->m_nWeaponType.Ref(),
			fActualDamage, fRatio, cm);
#ifdef _FLOATDMG_MODE
		if (Z_ETC_FLOATDMG2)
		{
			// Nếu mình là người cầm kiếm chém (pAttacker là mình)
			if (pAttacker == ZGetGame()->m_pMyCharacter && pVictim != pAttacker)
			{
				if (fActualDamage > 0)
				{
					// Hiện số damage bay lên trên đầu nạn nhân
					ZGetEffectManager()->AddDamageEffect((int)fActualDamage, pVictim, pAttacker);
				}
			}
		}
#endif

		ZActor* pATarget = MDynamicCast(ZActor, pVictim);
		ZActorWithFSM* pFSMActor = MDynamicCast(ZActorWithFSM, pVictim);

		ZPOSTDMGTAKEN(MCommandParameterInt(fActualDamage), MCommandParameterInt(0), pAttacker->GetUID()); // Custom: Fix Damage Count Melee
		// ?? ??? ?? ??
		bool bPushSkip = false;
		if (pATarget)
		{
			bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
		}
		if (pFSMActor)
		{
			bPushSkip = pFSMActor->GetActorDef()->IsNeverBlasted();
		}

		float fKnockbackForce = pItem->GetKnockbackForce();
		if (bPushSkip)
		{
			ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
			fKnockbackForce = 1.0f;
		}

		pVictim->OnKnockback(pAttacker->m_Direction, fKnockbackForce);

		// ??? ???E??
		ZGetSoundEngine()->PlaySoundBladeDamage(pItemDesc, pos);
#ifdef _ZPEERANTIHACK
		if (pVictim == m_pMyCharacter && pAttacker->IsNPC() == false)
		{
			int nAntiHackDelay = pItem->GetDesc()->m_nDelay.Ref();
			if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_FORCEMELEEDELAY))
			{
				int nForce = ZGetGameClient()->GetMatchStageSetting()->GetModifierValue(MMOD_FORCEMELEEDELAY);
				if (nForce >= 100 && nForce <= 1000)
					nAntiHackDelay = nForce;
			}
			ZPeerAntiHackAdd(pAttacker->GetUID(), Melee, nAntiHackDelay, MMCIP_PRIMARY, pItem->GetDesc()->m_nWeaponType.Ref());
		}
#endif

		// ?? ????E?? ??? ???? ??.
		if (pAttacker == m_pMyCharacter)
		{
			CheckCombo(m_pMyCharacter, pVictim, !bPushSkip);
			CheckStylishAction(m_pMyCharacter);
		}

		// ??? ???.
		bHit = true;
	}

	// ???? ???E????E..
	// test ???? ??E?????E???..
	if (!bHit)
	{
		rvector vPos = pAttacker->GetPosition();
		rvector vDir = AttackerNorDir;

		vPos.z += 130.f;

		RBSPPICKINFO bpi;

		if (GetWorld()->GetBsp()->Pick(vPos, vDir, &bpi)) {
			float fDist = Magnitude(vPos - bpi.PickPos);

			if (fDist < fRange) {
				rplane r = bpi.pInfo->plane;
				rvector vWallDir = rvector(r.a, r.b, r.c);
				Normalize(vWallDir);

				ZGetEffectManager()->AddSlashEffectWall(bpi.PickPos - (vDir * 5.f), vWallDir, cm);

				rvector pos = bpi.PickPos;

				ZGetSoundEngine()->PlaySoundBladeConcrete(pItemDesc, pos);
			}
		}
	}

	return;
}

bool ZGame::InRanged(ZObject* pAttacker, ZObject* pVictim)
{
	//### ÀÌ ÇÔ¼ö¸¦ ¼öÁ¤ÇÏ?E¶È°°ÀÌ ¾Æ·¡ÀÇ InRanged()¿¡µµ Àû?E?ÁÖ¾ûÚß ÇÕ´Ï´Ù. ###

	// °ø°ÝÀÚ¿Í Å¸°Ù ½Ç¸°´õÀÇ ¹Ù´Ú?EÀ­¸éÀÇ À§Ä¡¸¦ ±¸ÇÑ´Ù.
	float fBotAtk = pAttacker->GetPosition().z;
	//	float fTopAtk	= fBotAtk + pAttacker->GetCollHeight();
	float fTopAtk = fBotAtk + pAttacker->GetCollHeight() + (pAttacker->GetCollHeight() * 0.5f);

	float fBotVct = pVictim->GetPosition().z;
	float fTopVct = fBotVct + pVictim->GetCollHeight();

	// Å¸ÄÏÀÇ ¸Ç ¾Æ·¡°¡ °ø°ÝÀÚº¸´Ù À§¿¡ ÀÖÀ¸?E¿µ¿ª ¹ÛÀÌ´Ù.
	if (fBotVct > fTopAtk)
		return false;

	// Å¸ÄÏÀÇ ¸Ç À§°¡ °ø°ÝÀÚº¸´Ù ¾Æ·¡¿¡ ÀÖÀ¸?E¿µ¿ª ¹ÛÀÌ´Ù.
	else if (fTopVct < fBotAtk)
		return false;

	// ±× ¿Ü¿¡´Â ?E?¿µ¿ª ¾ÈÀÌ´Ù.
	return true;
}

bool ZGame::InRanged(ZObject* pAttacker, ZObject* pVictim, int& nDebugRegister/*µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ· ¹æÁö¸¦ À§ÇÑ º¯?E*/)
{
	// °ø°ÝÀÚ¿Í Å¸°Ù ½Ç¸°´õÀÇ ¹Ù´Ú?EÀ­¸éÀÇ À§Ä¡¸¦ ±¸ÇÑ´Ù.
	float fBotAtk = pAttacker->GetPosition().z;
	//	float fTopAtk	= fBotAtk + pAttacker->GetCollHeight();
	float fTopAtk = fBotAtk + pAttacker->GetCollHeight() + (pAttacker->GetCollHeight() * 0.5f);

	float fBotVct = pVictim->GetPosition().z;
	float fTopVct = fBotVct + pVictim->GetCollHeight();

	// Å¸ÄÏÀÇ ¸Ç ¾Æ·¡°¡ °ø°ÝÀÚº¸´Ù À§¿¡ ÀÖÀ¸?E¿µ¿ª ¹ÛÀÌ´Ù.
	if (fBotVct > fTopAtk)
	{
		nDebugRegister = -10;//¼ýÀÚ´Â ÀÇ¹Ì°¡ ¾ø´Ù..´Ü?Eºñ±³¸¦ À§ÇÑ ?E..
		return false;
	}

	// Å¸ÄÏÀÇ ¸Ç À§°¡ °ø°ÝÀÚº¸´Ù ¾Æ·¡¿¡ ÀÖÀ¸?E¿µ¿ª ¹ÛÀÌ´Ù.
	else if (fTopVct < fBotAtk)
	{
		nDebugRegister = -10;
		return false;
	}

	// ±× ¿Ü¿¡´Â ?E?¿µ¿ª ¾ÈÀÌ´Ù.
	nDebugRegister = FOR_DEBUG_REGISTER;
	return true;
}

//jintriple3 µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ· ¹æ?EÄÚ?E»ðÀÔ
void ZGame::OnPeerShot_Range_Damaged(ZObject* pOwner, float fShotTime, const rvector& pos, const rvector& to, ZPICKINFO pickinfo, DWORD dwPickPassFlag, rvector& v1, rvector& v2, ZItem* pItem, rvector& BulletMarkNormal, bool& bBulletMark, ZTargetType& nTargetType)
{
	MMatchItemDesc* pDesc = pItem->GetDesc();
	bool bReturnValue = !pDesc;
	if (!pDesc) PROTECT_DEBUG_REGISTER(bReturnValue) { _ASSERT(FALSE); return; }

#ifndef _ANTIHACK
#define SHOTGUN_BULLET_COUNT    12
#define SHOTGUN_DIFFUSE_RANGE   0.090f
#endif

	//Weapon GM turn off
	//if (pDesc->IsStaffItem() && !strstr(ZGetGameClient()->GetChannelName(), "Event"))
	//	return;

	rvector dir = to - pos;

	bReturnValue = !(ZGetGame()->PickHistory(pOwner, fShotTime, pos, to, &pickinfo, dwPickPassFlag));
	if (!(ZGetGame()->PickHistory(pOwner, fShotTime, pos, to, &pickinfo, dwPickPassFlag)))
	{
		PROTECT_DEBUG_REGISTER(bReturnValue)
		{
			v1 = pos;
			v2 = pos + dir * 10000.f;
			nTargetType = ZTT_NOTHING;
			return;
		}
	}

	// ¶«»§ -bird
	//jintriple3 µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ· ¹æ?EÄÚ?E
	bReturnValue = (!pickinfo.pObject) && (!pickinfo.bBspPicked);
	if (pickinfo.bBspPicked)
	{
		PROTECT_DEBUG_REGISTER(pickinfo.nBspPicked_DebugRegister == FOR_DEBUG_REGISTER)
		{
			nTargetType = ZTT_OBJECT;

			v1 = pos;
			v2 = pickinfo.bpi.PickPos;

			// ÃÑÅº Èç?E
			BulletMarkNormal.x = pickinfo.bpi.pInfo->plane.a;
			BulletMarkNormal.y = pickinfo.bpi.pInfo->plane.b;
			BulletMarkNormal.z = pickinfo.bpi.pInfo->plane.c;
			Normalize(BulletMarkNormal);
			bBulletMark = true;
			return;
		}
	}
	else if ((!pickinfo.pObject) && (!pickinfo.bBspPicked))
	{
		PROTECT_DEBUG_REGISTER(bReturnValue)
		{
			_ASSERT(false);
			return;
		}
	}
	//À§¿¡±ûÝö´Â °Ë?E´Ü?E..

	ZObject* pObject = pickinfo.pObject;
	bool bGuard = pObject->IsGuard() && (pickinfo.info.parts != eq_parts_legs) &&     // ´Ù¸®´Â ¸·À»¼ö¾ø´Ù
		DotProduct(dir, pObject->GetDirection()) < 0;

	if (pObject->IsGuard() && (pickinfo.info.parts != eq_parts_legs) &&        // ´Ù¸®´Â ¸·À»¼ö¾ø´Ù
		DotProduct(dir, pObject->GetDirection()) < 0) //¿©±âµµ..
	{
		PROTECT_DEBUG_REGISTER(bGuard)
		{
			nTargetType = ZTT_CHARACTER_GUARD;
			// ¸·¾Ò´Ù
			rvector t_pos = pObject->GetPosition();
			t_pos.z += 100.f;
			ZGetEffectManager()->AddSwordDefenceEffect(t_pos + (-dir * 50.f), -dir);
			pObject->OnGuardSuccess();
			v1 = pos;
			v2 = pickinfo.info.vOut;
			return;
		}
	}

	nTargetType = ZTT_CHARACTER;

	ZActor* pATarget = MDynamicCast(ZActor, pickinfo.pObject);
	ZActorWithFSM* pFSMActor = MDynamicCast(ZActorWithFSM, pickinfo.pObject);
	bool bPushSkip = false;

	if (pATarget)
	{
		bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
	}
	if (pFSMActor)
	{
		bPushSkip = pFSMActor->GetActorDef()->IsNeverBlasted();
	}
	
#ifdef _ANTIHACK
	float fKnockbackForce = pItem->GetKnockbackForce() / (.5f * float(ZGuard::SHOTGUN_BULLET_COUNT));
#else
	float fKnockbackForce = pItem->GetKnockbackForce() / (.5f * float(SHOTGUN_BULLET_COUNT));
#endif
	if (bPushSkip)
	{
		rvector vPos = pOwner->GetPosition() + (pickinfo.pObject->GetPosition() - pOwner->GetPosition()) * 0.1f;
		ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met", vPos);
		fKnockbackForce = 1.0f;
	}

	pickinfo.pObject->OnKnockback(pOwner->m_Direction, fKnockbackForce);
#ifdef _CALCDMG
	float fActualDamage = CalcActualDamage(pOwner, pickinfo.pObject, (float)pDesc->m_nDamage.Ref(), pItem->GetDesc()->m_nWeaponType.Ref(), pItem);
#else
	float fActualDamage = CalcActualDamage(pOwner, pickinfo.pObject, (float)pDesc->m_nDamage.Ref(), pItem->GetDesc()->m_nWeaponType.Ref());
#endif
	float fRatio = pItem->GetPiercingRatio(pDesc->m_nWeaponType.Ref(), pickinfo.info.parts);
#ifdef _LOBBYSET
	if (ZGetGameClient()->GetMatchStageSetting()->GetDMGType() == 1)
	{
		if (pickinfo.info.parts != eq_parts_head)
			return;
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetDMGType() == 2)
	{
		if (pickinfo.info.parts != eq_parts_chest)
			return;
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetDMGType() == 3)
	{
		if (pickinfo.info.parts != eq_parts_legs)
			return;
	}
#endif

	ZDAMAGETYPE dt = (pickinfo.info.parts == eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;

	// old
	//if(pickinfo.pObject->IsNPC() || (ZGetChannelRuleMgr()->GetCurrentRule() && ZGetChannelRuleMgr()->GetCurrentRule()->GetID() == MCHANNEL_RULE_LEAD))
	//	pickinfo.pObject->OnDamaged(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio );
	//else
	//	((ZCharacter*)pickinfo.pObject)->OnDamaged_AntiLead(pOwner, pOwner->GetPosition(), ((ZCharacter*)pOwner)->GetItems()->GetSelectedWeaponParts(), pickinfo.info.parts, dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio );
#ifdef _BOXLEAD
	if (pickinfo.pObject->IsNPC() == true || ZGetGameClient()->GetMatchStageSetting()->IsLead() == true || (ZGetChannelRuleMgr()->GetCurrentRule() && ZGetChannelRuleMgr()->GetCurrentRule()->GetID() == MCHANNEL_RULE_LEAD))
	{
		pickinfo.pObject->OnDamaged(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio);

		if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
			ZGetEffectManager()->AddWithScale("ef_damage07.elu", pickinfo.info.vOut, dir, pickinfo.pObject->GetUID(), 0.2, 0);
#ifdef _FLOATDMG_MODE
		if (Z_ETC_FLOATDMG2 && pOwner == ZGetGame()->m_pMyCharacter && pickinfo.pObject->IsNPC())
		{
			// Hiển thị sát thương ngay khi quái nhận damage
			ZGetEffectManager()->AddDamageEffect((int)fActualDamage, pickinfo.pObject, pOwner);
		}
#endif

	}
	else
	{
		((ZCharacter*)(pickinfo.pObject))->OnDamagedAPlayer(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio);
	}
#endif

	if (pOwner == m_pMyCharacter)
	{
		CheckCombo(m_pMyCharacter, pickinfo.pObject, !bPushSkip);
		CheckStylishAction(m_pMyCharacter);
#ifdef _HITSCOUNT
		if (pOwner == m_pMyCharacter)
			ZGetGame()->m_pMyCharacter->m_nHits += 1;
#endif
	}
#ifdef _FLOATDMG_MODE
	if (ZGetGame() && Z_ETC_FLOATDMG2)
	{
		// Kiểm tra: Mình là người bắn và mục tiêu không phải là mình
		if ((pObject->GetUID() != ZGetMyUID()) && (pOwner == ZGetGame()->m_pMyCharacter))
		{
			// DÙNG TRỰC TIẾP fActualDamage - Sát thương của chính viên đạn này
			int nDisplayDmg = (int)fActualDamage;

			if (nDisplayDmg > 0)
			{
				// Hiển thị số bay lên ngay lập tức
				ZGetEffectManager()->AddDamageEffect(nDisplayDmg, pObject, pOwner);
			}
		}
	}
#endif

	v1 = pos;
	v2 = pickinfo.info.vOut;
}
void ZGame::OnPeerShot_Range(const MMatchCharItemParts sel_type, const MUID& uidOwner, float fShotTime, const rvector& pos, const rvector& to)
{
	ZObject* pOwner = m_ObjectManager.GetObject(uidOwner);
	if (!pOwner) return;

	ZItem* pItem = pOwner->GetItems()->GetItem(sel_type);
	if (!pItem) return;
	MMatchItemDesc* pDesc = pItem->GetDesc();
	if (!pDesc) { _ASSERT(FALSE); return; }

	//Weapon GM turn off
	//if (pDesc->IsStaffItem() && !strstr(ZGetGameClient()->GetChannelName(), "Event"))
	//	return;
	

	rvector dir = to - pos;

	Normalize(dir);

	rvector v1, v2;
	rvector BulletMarkNormal;
	bool bBulletMark = false;
	ZTargetType nTargetType = ZTT_OBJECT;

	// ZCharacter* pCharacter = NULL;

	ZPICKINFO pickinfo;

	memset(&pickinfo, 0, sizeof(ZPICKINFO));

	// ÃÑ¾ËÀº ·ÎÄÏÀÌ ?EúÇÏ´Â°÷µ??EúÇÑ´?
	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;

	// ½ûÐÂ Ä³¸¯ÅÍ Èçµé?EÁÖ?E.
	pOwner->Tremble(8.f, 50, 30);

	/*
		if(pOwner->m_pVMesh)
		{
			float fMaxValue = 8.f;// Èç?E°­µµ +- °¡´É

			RFrameTime* ft = &pOwner->m_pVMesh->m_FrameTime;
			if(ft && !ft->m_bActive)
				ft->Start(fMaxValue,50,30);// °­µµ , ÃÖ?EÃ°?, º¹±Í½Ã°£...
		}
	*/
	//jintriple3 µð¹ö±× ·¹Áö½ºÅÍ ÇÙ ¹æÁö¸¦ À§ÇØ ÇÔ¼ö·Î »°´Ù..±Ùµ¥ ÀÎÀÚ°¡ ³Ñ?E¸¹¾Æ..¤Ì¤Ì
	OnPeerShot_Range_Damaged(pOwner, fShotTime, pos, to, pickinfo, dwPickPassFlag, v1, v2, pItem, BulletMarkNormal, bBulletMark, nTargetType);
	/*
	if(g_pGame->PickHistory(pOwner,fShotTime,pos,to,&pickinfo,dwPickPassFlag))
	{
		// ¶«»§ -bird

		if(pickinfo.pObject)
		{
			ZObject *pObject = pickinfo.pObject;
			bool bGuard = pObject->IsGuard() && (pickinfo.info.parts!=eq_parts_legs) &&		// ´Ù¸®´Â ¸·À»¼ö¾ø´Ù
							DotProduct(dir,pObject->GetDirection())<0;

			if(bGuard)
			{
				nTargetType = ZTT_CHARACTER_GUARD;
				// ¸·¾Ò´Ù
				rvector t_pos = pObject->GetPosition();
				t_pos.z += 100.f;
				ZGetEffectManager()->AddSwordDefenceEffect(t_pos+(-dir*50.f),-dir);
				pObject->OnGuardSuccess();
			}
			else
			{
				nTargetType = ZTT_CHARACTER;

				ZActor* pATarget = MDynamicCast(ZActor,pickinfo.pObject);

				bool bPushSkip = false;

				if(pATarget)
				{
					bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
				}

				float fKnockbackForce = pItem->GetKnockbackForce();

				if(bPushSkip)
				{
//					ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
					rvector vPos = pOwner->GetPosition() + (pickinfo.pObject->GetPosition() - pOwner->GetPosition()) * 0.1f;
					ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met", vPos );
					fKnockbackForce = 1.0f;
				}

				pickinfo.pObject->OnKnockback( pOwner->m_Direction, fKnockbackForce );

				float fActualDamage = CalcActualDamage(pOwner, pickinfo.pObject, (float)pDesc->m_nDamage);
				float fRatio = pItem->GetPiercingRatio( pDesc->m_nWeaponType, pickinfo.info.parts );
				ZDAMAGETYPE dt = (pickinfo.info.parts==eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;
				//jintriple3 µð¹ö±× ·¹Áö½ºÅÍ ÇÙÀ¸·ÎºÎÅÍ ¾È?EÏ°?º¸È£ÇØ¾ß µÉ ºÎºÐ..
				pickinfo.pObject->OnDamaged(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType, fActualDamage, fRatio );

				if(pOwner == m_pMyCharacter)
				{
					CheckCombo(m_pMyCharacter,pickinfo.pObject,!bPushSkip);
					CheckStylishAction(m_pMyCharacter);
				}
			}

			v1 = pos;
			v2 = pickinfo.info.vOut;
		}
		else if(pickinfo.bBspPicked)
		{
			nTargetType = ZTT_OBJECT;

			v1 = pos;
			v2 = pickinfo.bpi.PickPos;

			// ÃÑÅº Èç?E
			BulletMarkNormal.x = pickinfo.bpi.pInfo->plane.a;
			BulletMarkNormal.y = pickinfo.bpi.pInfo->plane.b;
			BulletMarkNormal.z = pickinfo.bpi.pInfo->plane.c;
			Normalize(BulletMarkNormal);
			bBulletMark = true;
		}
		else
		{
			_ASSERT(false);
			return;
		}
	}

	else
	{
		v1 = pos;
		v2 = pos+dir*10000.f;
		nTargetType	= ZTT_NOTHING;
	}*/

	bool bPlayer = false;
	//bool b3D = (pOwnerCharacter!=m_pMyCharacter);	// ÀÚ±â°¡ ³»´Â »ç¿ûÑå´Â 2D·Î Ãâ·ÂÇÑ´Ù.
	//rvector Pos = pOwnerCharacter->GetPosition();
	rvector Pos = v1;
	if (pOwner == m_pMyCharacter)
	{
		Pos = RCameraPosition;
		bPlayer = true;
	}

	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	ZApplication::GetSoundEngine()->PlaySEFire(pDesc, Pos.x, Pos.y, Pos.z, bPlayer);
	//if(nTargetType == ZTT_OBJECT) { ZApplication::GetSoundEngine()->PlaySERicochet(v2.x, v2.y, v2.z); }
#define SOUND_CULL_DISTANCE 1500.0F
	if (D3DXVec3LengthSq(&(v2 - pTargetCharacter->GetPosition())) < (SOUND_CULL_DISTANCE * SOUND_CULL_DISTANCE))
	{
		if (ZGetGame()->GetMatch()->IsPaintBall())
		{
#ifdef _PAINTMODE
			if (nTargetType == ZTT_CHARACTER)
			{
				ZGetSoundEngine()->PlaySound("hitbody_paint"), (v2.x, v2.y, v2.z);
			}
			if (ZGetGame()->GetMatch()->IsTraining() && GetRGMain().TrainingSettings.Paint)
			{
				ZGetSoundEngine()->PlaySound("hitbody_paint"), (v2.x, v2.y, v2.z);
			}
#endif
		}
		else
		{
			if (nTargetType == ZTT_OBJECT) {
				ZGetSoundEngine()->PlaySEHitObject(v2.x, v2.y, v2.z, pickinfo.bpi);
			}

			if (nTargetType == ZTT_CHARACTER) {
				ZGetSoundEngine()->PlaySEHitBody(v2.x, v2.y, v2.z);
			}
		}
	}

	//// º¸ÀÌ?E¾ÊÀ¸?EÀÌÆåÆ®¸¦ ±×¸±ÇÊ¿ä´Â ¾ø´Ù - Á¤È®ÇÑ ÄÃ¸µÀ» ¿ä¸Á.. by bird
	//if(!pOwner->IsRendered()) return;

	// ½ûÐÂ?E¹Ý?E100cm °¡ È­¸é¿¡ µé¾ûÌ¡´Â?EÃ¼Å©ÇÑ´Ù
	bool bDrawFireEffects = isInViewFrustum(v1, 100.f, RGetViewFrustum());

	if (!isInViewFrustum(v1, v2, RGetViewFrustum()) // ½ûÐÂ°÷¿¡¼­ ¸Â´Â°÷ÀÇ ¶óÀÎÀÌ º¸ÀÌ´Â?E.
		&& !bDrawFireEffects) return;					// ½ûÐÂ°÷¿¡¼­µµ ±×¸±°Ô ¾ø´Â?E.

	bool bDrawTargetEffects = isInViewFrustum(v2, 100.f, RGetViewFrustum());

	/////////////////////// ÀÌÈÄ´Â ÀÌÆåÆ® Ãß°¡

	// ¹°Æ¢´Â ÀÌÆåÆ® Ã¼Å©
	GetWorld()->GetWaters()->CheckSpearing(v1, v2, 250, 0.3);

	// TODO: NPC ÀÇ ÃÑ±¸À§Ä¡ ÀÎÅÍÆäÀÌ½º°¡ È®Á¤µÇ?E¸¶?EÃß°¡ÇÏÀÚ.
//	ZCharacter *pOwnerCharacter = m_CharacterManager.Find(uidOwner);

	ZCharacterObject* pCOwnerObject = MDynamicCast(ZCharacterObject, pOwner);

	if (pCOwnerObject)
	{
		// ÃÑ±¸ È­¿°ÀÌÆåÆ®
		rvector pdir = v2 - v1;
		Normalize(pdir);

		int size = 3;

		rvector v[6];

		//		size = GetWeapondummyPos(pOwnerCharacter,v);
		if (pCOwnerObject->IsRendered())
			size = pCOwnerObject->GetWeapondummyPos(v);
		else
		{
			size = 6;
			v[0] = v[1] = v[2] = v1;
			v[3] = v[4] = v[5] = v[0];
		}

		MMatchWeaponType wtype = pDesc->m_nWeaponType.Ref();
		bool bSlugOutput = pDesc->m_bSlugOutput; // ÅºÇÇÀû?Etrue, false)

		// Effect
		if (bBulletMark == false) BulletMarkNormal = -pdir;

		ZGetEffectManager()->AddShotEffect(v, size, v2, BulletMarkNormal, nTargetType, wtype, bSlugOutput, pCOwnerObject, bDrawFireEffects, bDrawTargetEffects);

		// ÃÑ ½ò¶§ ¶óÀÌÆ® Ãß°¡
		ZCharacterObject* pChar;

		if (ZGetConfiguration()->GetVideo()->bDynamicLight && pCOwnerObject != NULL)
		{
			pChar = pCOwnerObject;

			if (pChar->m_bDynamicLight)
			{
				pChar->m_vLightColor = g_CharLightList[GUN].vLightColor;
				pChar->m_fLightLife = g_CharLightList[GUN].fLife;
			}
			else
			{
				pChar->m_bDynamicLight = true;
				pChar->m_vLightColor = g_CharLightList[GUN].vLightColor;
				pChar->m_vLightColor.x = 1.0f;
				pChar->m_iDLightType = GUN;
				pChar->m_fLightLife = g_CharLightList[GUN].fLife;
			}
		}
	}

	// ?Eß¿??EÀû?E		   p
	GetWorld()->GetFlags()->CheckSpearing(v1, v2, BULLET_SPEAR_EMBLEM_POWER);
	if (Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->AddLightSource(v1, 2.0f, 75);
}

void ZGame::OnPeerShot_Shotgun(ZItem* pItem, ZCharacter* pOwnerCharacter, float fShotTime, const rvector& pos, const rvector& to)
{
#ifndef _ANTIHACK
    #define SHOTGUN_BULLET_COUNT    12
    #define SHOTGUN_DIFFUSE_RANGE   0.090f 
#endif
	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if (!pTargetCharacter) return;


	MMatchItemDesc* pDesc = pItem->GetDesc();
	if (!pDesc) { _ASSERT(false); return; }


	if (pOwnerCharacter == NULL) return;

	//Weapon GM turn off
	//if (pDesc->IsStaffItem() && !strstr(ZGetGameClient()->GetChannelName(), "Event"))
	//	return;

	int* seed = (int*)&fShotTime;
	srand(*seed);

	bool bHitGuard = false, bHitBody = false, bHitGround = false, bHitEnemy = false;
	rvector GuardPos, BodyPos, GroundPos;
	bool waterSound = false;

	rvector v1, v2;
	rvector dir;

	rvector origdir = to - pos;
	Normalize(origdir);

	int nHitCount = 0;

	auto Netcode = ZGetGameClient()->GetMatchStageSetting()->GetNetcode();

	vector<MTD_AntiLeadN*> vShots;
#ifdef _ANTIHACK
	for (int i = 0; i < ZGuard::SHOTGUN_BULLET_COUNT; i++) 
#else
	for (int i = 0; i < SHOTGUN_BULLET_COUNT; i++)
#endif
	{
		dir = origdir;
		{
			// ¿ÀÂ÷°ª - ¹Ýµ¿?E?½Ã?EE?³ÖÀ½
			rvector r, up(0, 0, 1), right;
			D3DXQUATERNION q;
			D3DXMATRIX mat;

			float fAngle = (rand() % (31415 * 2)) / 1000.0f;
#ifdef _ANTIHACK
			float fForce = RANDOMFLOAT * ZGuard::SHOTGUN_DIFFUSE_RANGE;
#else
			float fForce = RANDOMFLOAT * SHOTGUN_DIFFUSE_RANGE;
#endif

			D3DXVec3Cross(&right, &dir, &up);
			D3DXVec3Normalize(&right, &right);
			D3DXMatrixRotationAxis(&mat, &right, fForce);
			D3DXVec3TransformCoord(&r, &dir, &mat);

			D3DXQuaternionRotationAxis(&q, &dir, fAngle);
			D3DXMatrixRotationQuaternion(&mat, &q);
			D3DXVec3TransformCoord(&r, &r, &mat);

			dir = r;
		}
		rvector BulletMarkNormal;
		bool bBulletMark = false;
		ZTargetType nTargetType = ZTT_OBJECT;

		ZPICKINFO pickinfo;

		memset(&pickinfo, 0, sizeof(ZPICKINFO));

		// ÃÑ¾ËÀº ·ÎÄÏÀÌ Åë°úÇÏ´Â°÷µµ Åë°úÇÑ´Ù
		const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;

		MTD_AntiLeadN* pShotInfo = OnPeerShotgun_Damaged(pOwnerCharacter, fShotTime, pos, dir, pickinfo, dwPickPassFlag, v1, v2, pItem, BulletMarkNormal, bBulletMark, nTargetType, bHitEnemy, i);
		if (pShotInfo)
			vShots.push_back(pShotInfo);
	}

	if (bHitEnemy) {
		CheckStylishAction(pOwnerCharacter);
		CheckCombo(pOwnerCharacter, NULL, true);
#ifdef _HITSCOUNT
		if (pOwnerCharacter == m_pMyCharacter)
			ZGetGame()->m_pMyCharacter->m_nHits += 1;
#endif

	}

	if (vShots.size() > 0 && !ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
	{
		ZCharacter* pCharacter = (ZCharacter*)ZGetCharacterManager()->Find(MUID(0, vShots[0]->nLowId));

		if (pOwnerCharacter->GetUID() == ZGetGameClient()->GetPlayerUID())
		{
			if (!GetMatch()->IsTeamPlay())
			{
				pCharacter->OnDamagedAPlayer(vShots);
			}
			else if (GetMatch()->IsTeamPlay() && ZGetGame()->GetMatch()->GetTeamKillEnabled() && pCharacter->IsTeam(pOwnerCharacter))
			{
				pCharacter->OnDamagedAPlayer(vShots);
			}
			else if (GetMatch()->IsTeamPlay() && !pCharacter->IsTeam(pOwnerCharacter))
			{
				pCharacter->OnDamagedAPlayer(vShots);
			}
		}

		for (auto i = vShots.begin(); i != vShots.end(); i++)
		{
			auto p = *i;
			delete p;
		}
	}

	ZApplication::GetSoundEngine()->PlaySEFire(pItem->GetDesc(), pos.x, pos.y, pos.z, (pOwnerCharacter == m_pMyCharacter));

	// º¸ÀÌÁö ¾ÊÀ¸¸é ÀÌÆåÆ®¸¦ ±×¸±ÇÊ¿ä´Â ¾ø´Ù
	if (!pOwnerCharacter->IsRendered()) return;

	rvector v[6];

	int _size = pOwnerCharacter->GetWeapondummyPos(v);

	dir = to - pos;
	Normalize(dir);

	ZGetEffectManager()->AddShotgunEffect(const_cast<rvector&>(pos), v[1], dir, pOwnerCharacter);

	// ÃÑ ½ò¶§ ¶óÀÌÆ® Ãß°¡
	ZCharacter* pChar;
	if (ZGetConfiguration()->GetVideo()->bDynamicLight && pOwnerCharacter != NULL)
	{
		pChar = pOwnerCharacter;

		if (pChar->m_bDynamicLight)
		{
			pChar->m_vLightColor = g_CharLightList[SHOTGUN].vLightColor;
			pChar->m_fLightLife = g_CharLightList[SHOTGUN].fLife;
		}
		else
		{
			pChar->m_bDynamicLight = true;
			pChar->m_vLightColor = g_CharLightList[SHOTGUN].vLightColor;
			pChar->m_vLightColor.x = 1.0f;
			pChar->m_iDLightType = SHOTGUN;
			pChar->m_fLightLife = g_CharLightList[SHOTGUN].fLife;
		}
	}
	//  m_flags.CheckSpearing( v1, v2, SHOTGUN_SPEAR_EMBLEM_POWER );
	if (Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->AddLightSource(v1, 2.0f, 200);

#ifdef _KILLFEED
	pOwnerCharacter->SetWeaponDamaged(pItem->GetDesc()->m_nWeaponType.Ref());
	ZPostLastWeaponUsed(pItem->GetDesc()->m_nWeaponType.Ref());
#endif
}

//jintriple3 µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ· ¹æÁö ÄÚµå »ðÀÔ
MTD_AntiLeadN* ZGame::OnPeerShotgun_Damaged(ZObject* pOwner, float fShotTime, const rvector& pos, rvector& dir, ZPICKINFO pickinfo, DWORD dwPickPassFlag, rvector& v1, rvector& v2, ZItem* pItem, rvector& BulletMarkNormal, bool& bBulletMark, ZTargetType& nTargetType, bool& bHitEnemy, int Repeatcount)
{
	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	bool bReturnValue = !pTargetCharacter;
	if (!pTargetCharacter)PROTECT_DEBUG_REGISTER(bReturnValue) return NULL;

	MMatchItemDesc* pDesc = pItem->GetDesc();
	bReturnValue = !pDesc;
	if (!pDesc)PROTECT_DEBUG_REGISTER(bReturnValue) { _ASSERT(FALSE); return NULL; }

	auto Netcode = ZGetGameClient()->GetMatchStageSetting()->GetNetcode();

	//rvector dir = to - pos;

	bool waterSound = false;
	//¿©±â¿¡ ¹æ¾îÄÚµå°¡ µé¾î°¡¾ßµÅ~

	bReturnValue = !(ZGetGame()->PickHistory(pOwner, fShotTime, pos, pos + 10000.f * dir, &pickinfo, dwPickPassFlag));
	if (!(ZGetGame()->PickHistory(pOwner, fShotTime, pos, pos + 10000.f * dir, &pickinfo, dwPickPassFlag)))
	{
		PROTECT_DEBUG_REGISTER(bReturnValue)
		{
			v1 = pos;
			v2 = pos + dir * 10000.f;
			nTargetType = ZTT_NOTHING;
			waterSound = GetWorld()->GetWaters()->CheckSpearing(v1, v2, 250, 0.3, !waterSound);
			return NULL;
		}
	}
	//¿©±âµµ..
	bReturnValue = (!pickinfo.pObject) && (!pickinfo.bBspPicked);
	if (pickinfo.bBspPicked)
	{
		PROTECT_DEBUG_REGISTER(pickinfo.nBspPicked_DebugRegister == FOR_DEBUG_REGISTER)
		{
			nTargetType = ZTT_OBJECT;

			v1 = pos;
			v2 = pickinfo.bpi.PickPos;

			// ÃÑÅº ÈçÀû
			BulletMarkNormal.x = pickinfo.bpi.pInfo->plane.a;
			BulletMarkNormal.y = pickinfo.bpi.pInfo->plane.b;
			BulletMarkNormal.z = pickinfo.bpi.pInfo->plane.c;
			Normalize(BulletMarkNormal);
			bBulletMark = true;

			// ¸Â´Â°÷ ¹Ý°æ 20cm °¡ È­¸é¿¡ µé¾î¿À¸é ±×¸°´Ù
			bool bDrawTargetEffects = isInViewFrustum(v2, 20.f, RGetViewFrustum());
			if (bDrawTargetEffects)
				ZGetEffectManager()->AddBulletMark(v2, BulletMarkNormal);
			waterSound = GetWorld()->GetWaters()->CheckSpearing(v1, v2, 250, 0.3, !waterSound);
			return NULL;
		}
	}
	else if ((!pickinfo.pObject) && (!pickinfo.bBspPicked))
	{
		PROTECT_DEBUG_REGISTER(bReturnValue)
		{
			_ASSERT(false);
			return NULL;
		}
	}

	//À§¿¡±îÁö´Â °Ë»ç ´Ü°è...

	ZObject* pObject = pickinfo.pObject;
	bool bGuard = pObject->IsGuard() && (pickinfo.info.parts != eq_parts_legs) &&     // ´Ù¸®´Â ¸·À»¼ö¾ø´Ù
		DotProduct(dir, pObject->GetDirection()) < 0;

	if (pObject->IsGuard() && (pickinfo.info.parts != eq_parts_legs) &&
		DotProduct(dir, pObject->GetDirection()) < 0)
	{
		PROTECT_DEBUG_REGISTER(bGuard)
		{
			nTargetType = ZTT_CHARACTER_GUARD;
			// ¸·¾Ò´Ù
			rvector t_pos = pObject->GetPosition();
			t_pos.z += 100.f;
			ZGetEffectManager()->AddSwordDefenceEffect(t_pos + (-dir * 50.f), -dir);
			pObject->OnGuardSuccess();
			v1 = pos;
			v2 = pickinfo.info.vOut;
			return NULL;
		}
	}

	ZActor* pATarget = MDynamicCast(ZActor, pObject);
	ZActorWithFSM* pFSMActor = MDynamicCast(ZActorWithFSM, pObject);
	nTargetType = ZTT_CHARACTER;

	bool bPushSkip = false;

	if (pATarget)
	{
		bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
	}
	if (pFSMActor)
	{
		bPushSkip = pFSMActor->GetActorDef()->IsNeverBlasted();
	}
#ifdef _ANTIHACK
	float fKnockbackForce = pItem->GetKnockbackForce() / (.5f * float(ZGuard::SHOTGUN_BULLET_COUNT));
#else
	float fKnockbackForce = pItem->GetKnockbackForce() / (.5f * float(SHOTGUN_BULLET_COUNT));
#endif

	if (bPushSkip)
	{
		//                  ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
		rvector vPos = pOwner->GetPosition() + (pickinfo.pObject->GetPosition() - pOwner->GetPosition()) * 0.1f;
		ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met", vPos);
		fKnockbackForce = 1.0f;
	}

	pObject->OnKnockback(dir, fKnockbackForce);
#ifdef _CALCDMG
	float fActualDamage = CalcActualDamage(pOwner, pObject, (float)pDesc->m_nDamage.Ref(), pItem->GetDesc()->m_nWeaponType.Ref(), pItem);
#else
	float fActualDamage = CalcActualDamage(pOwner, pObject, (float)pDesc->m_nDamage.Ref(), pItem->GetDesc()->m_nWeaponType.Ref());
#endif
#ifdef _FLOATDMG_MODE
	if (ZGetGame() && Z_ETC_FLOATDMG2)
	{
		if ((pObject->GetUID() != ZGetMyUID()) && (pOwner == ZGetGame()->m_pMyCharacter))
		{
			// Biến tĩnh lưu trữ
			static float fGomDame = 0.0f;
			static float fLastShotTime = 0.0f;

			// Nếu phát bắn này cách phát trước > 0.05 giây -> Coi như phát bắn mới -> Reset gom dame
			if (fShotTime != fLastShotTime)
			{
				fGomDame = 0.0f;
				fLastShotTime = fShotTime;
			}

			// Cộng dồn viên đạn nhỏ này vào
			fGomDame += fActualDamage;

			// HIỆN SỐ: Vẽ đè lên liên tục để người chơi thấy số tăng dần (9 -> 18 -> ... -> 108)
			// Cách này đảm bảo trúng bao nhiêu viên hiện đúng bấy nhiêu, không bao giờ sai số
			ZGetEffectManager()->AddDamageEffect((int)fGomDame, pObject, pOwner);
		}
	}
#endif
	float fRatio = pItem->GetPiercingRatio(pDesc->m_nWeaponType.Ref(), pickinfo.info.parts);
#ifdef _LOBBYSET
	if (ZGetGameClient()->GetMatchStageSetting()->GetDMGType() == 1)
	{
		if (pickinfo.info.parts != eq_parts_head)
			return false;
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetDMGType() == 2)
	{
		if (pickinfo.info.parts != eq_parts_chest)
			return false;
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetDMGType() == 3)
	{
		if (pickinfo.info.parts != eq_parts_legs)
			return false;
	}
#endif
	ZDAMAGETYPE dt = (pickinfo.info.parts == eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;
	MTD_AntiLeadN* pShotInfo = NULL;
#ifdef _BOXLEAD
	if (pickinfo.pObject->IsNPC() == true || ZGetGameClient()->GetMatchStageSetting()->IsLead() == true || (ZGetChannelRuleMgr()->GetCurrentRule() && ZGetChannelRuleMgr()->GetCurrentRule()->GetID() == MCHANNEL_RULE_LEAD))
	{
#ifdef _TYPENET
		if (Netcode == NetcodeType::P2PLead)
		{
#endif
		pickinfo.pObject->OnDamaged(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio);
#ifdef _TYPENET
		}
#endif
	}
	else
	{
#ifdef _TYPENET
		if (Netcode == NetcodeType::P2PAntilead && !IsReplay())
		{
#endif
		pShotInfo = new MTD_AntiLeadN;
		pShotInfo->nLowId = pickinfo.pObject->GetUID().Low;
		pShotInfo->fDamage = fActualDamage;
		pShotInfo->fPosX = pOwner->GetPosition().x;
		pShotInfo->fPosY = pOwner->GetPosition().y;
		pShotInfo->fPosZ = pOwner->GetPosition().z;
		pShotInfo->fRatio = fRatio;
		pShotInfo->nDamageType = dt;
		pShotInfo->nWeaponType = pDesc->m_nWeaponType.Ref();
#ifdef _TYPENET
		}
#endif
	}
#endif
	if (!m_Match.IsTeamPlay() || (pTargetCharacter->GetTeamID() != pObject->GetTeamID()))
	{
		bHitEnemy = true;
	}

	v1 = pos;
	v2 = pickinfo.info.vOut;

	waterSound = GetWorld()->GetWaters()->CheckSpearing(v1, v2, 250, 0.3, !waterSound);

	return pShotInfo;
}

bool ZGame::CanISeeAttacker(ZCharacter* pAtk, const rvector& vRequestPos)
{
	const rvector& vAtkPos = pAtk->GetPosition();

	long double x = pow(vAtkPos.x - vRequestPos.x, 2);
	long double y = pow(vAtkPos.y - vRequestPos.y, 2);
	long double z = pow(vAtkPos.z - vRequestPos.z, 2);

	long double Len = x + y + z;

	// base info¿¡ ÀúÀåµÇ?EÀÖ´Â À§Ä¡¿Í ÇöÁ¦ ¹ß»çÇÑ ¹«±âÀ§ À§Ä¡Â÷°¡ ¾Æ·¡ ¼öÄ¡º¸´Ù ÀÛ¾ûÚß ÇÑ´Ù.
	// ?E?ÃÖ?E1ÃÊÁ¤µµ µô·¹ÀÌ°¡ µÉ?EÀÖ´Ù?E°¡Á¤ÇÔ.
	// ¸¸?E?E±æ´Ù?E?EÁ¤?EÀ§Ä¡¿¡¼­ °ø°ÝÇÑ °É·Î ÆÇ´ÜÇÑ´Ù. - by SungE 2007-04-17
#define MAX_VIEW_LENGTH 800000 // ?E?Àå°Ë?E?2¹øÇÑ °Å¸®.

	if (MAX_VIEW_LENGTH < Len)
	{
#ifdef _DEBUG
		static rvector rv(0.0f, 0.0f, 0.0f);

		long double l = pow(vRequestPos.x - rv.x, 2) + pow(vRequestPos.y - rv.y, 2) + pow(vRequestPos.z - rv.z, 2);

		rv = vRequestPos;

		mlog("len : %f(%f), res(%d)\n", Len, sqrt(Len), MAX_VIEW_LENGTH < Len);
#endif
		return false;
	}

	return true;
}

// shot À» shot_range, shot_melee, shot_shotgun À¸·Î command ¸¦ °¢°¢ ºÐ¸®ÇÏ´Â°Íµµ ¹æ¹ýÀÌ ÁÁÀ»?E
void ZGame::OnPeerShot(const MUID& uid, float fShotTime, const rvector& pos, const rvector& to, const MMatchCharItemParts sel_type)
{
	ZCharacter* pOwnerCharacter = NULL;

	pOwnerCharacter = (ZCharacter*)m_CharacterManager.Find(uid);

	if (pOwnerCharacter == NULL) return;
	if (!pOwnerCharacter->IsVisible()) return;

#ifdef LOCALE_NHNUSA
	if (!CanISeeAttacker(pOwnerCharacter, pos)) return;
#endif

	pOwnerCharacter->OnShot();

	// fShotTime ÀÌ ±× Ä³¸¯ÅÍÀÇ ·ÎÄÃ ½Ã°£ÀÌ¹Ç·Î ³» ½Ã°£À¸·Î º¯È¯ÇØÁØ´Ù
	fShotTime -= pOwnerCharacter->m_fTimeOffset;

	ZItem* pItem = pOwnerCharacter->GetItems()->GetItem(sel_type);
	if (!pItem || !pItem->GetDesc()) return;

	// ºñÁ¤»óÀûÀÎ ¹ß»ç¼Óµµ¸¦ ¹«½ÃÇÑ´Ù.
	if (pOwnerCharacter->CheckValidShotTime(pItem->GetDescID(), fShotTime, pItem))
	{
		pOwnerCharacter->UpdateValidShotTime(pItem->GetDescID(), fShotTime);
	}
	else
	{
		return;
	}

	//// ·çÇÁÁß MEMORYHACKÀÖ¾ú³ª °Ë?E
	if (uid == ZGetMyUID()) {
		int nCurrMagazine = pItem->GetBulletCurrMagazine();

		// ½ÇÁ¦·Î ¹«±â¸¦ ¼Ò?E
		if (!pItem->Shot()) return;

		if (!(pItem->GetBulletCurrMagazine() < nCurrMagazine))	// Shot¿¡¼­ ÃÑ¾Ë ÁÙ¾ûÚß¸¸ Á¤»óÀÌ´Ù
			if (sel_type != MMCIP_MELEE)
				ZGetApplication()->Exit();
	}
	else {
		// ½ÇÁ¦·Î ¹«±â¸¦ ¼Ò?E
		if (!pItem->Shot()) {
			//			_ASSERT(FALSE);	// ¹®Á¦°¡ÀÖ´Ù, Ä¡ÆÃ ?
			if (!ZGetGame()->IsReplay())	// ¸®ÇÃ·¹ÀÌ¶ó?EÃÑ¾ËÀÇ À¯¹«¿¡ »ó?EøÀ?¹ß»çÃ³¸®¸¦ ÇØÁØ´Ù.
				return;	// SHOT
		}
	}

	// MELEEÀÏ °æ?E
	if (sel_type == MMCIP_MELEE)
	{
		OnPeerShot_Melee(uid, fShotTime);

		return;
	}

	if ((sel_type != MMCIP_PRIMARY) && (sel_type != MMCIP_SECONDARY) && (sel_type != MMCIP_CUSTOM1)) return;

	if (!pItem->GetDesc()) return;
	MMatchWeaponType wtype = pItem->GetDesc()->m_nWeaponType.Ref();

#ifdef _RADAR
	if (ZGetGameClient()->IsLadderWarsChannel())
	{
		rvector _pos = pos;
		MUID _uid = uid;
		if ((wtype >= MWT_SHOTGUN) && (wtype <= MWT_RIFLE))
			ZGetCombatInterface()->GetRadar()->OnAttack(_pos, _uid);
	}
#endif

	if (wtype == MWT_SHOTGUN)
	{
		OnPeerShot_Shotgun(pItem, pOwnerCharacter, fShotTime, pos, to);
		return;
	}
	else {
		OnPeerShot_Range(sel_type, uid, fShotTime, pos, to);

		rvector position;
		pOwnerCharacter->GetWeaponTypePos(weapon_dummy_muzzle_flash, &position);
		if (ZGetConfiguration()->GetVideo()->bDynamicLight)
		{
			RGetDynamicLightManager()->AddLight(GUNFIRE, position);
		}
	}
#ifdef _KILLFEED
	pOwnerCharacter->SetWeaponDamaged(pItem->GetDesc()->m_nWeaponType.Ref());
	ZPostLastWeaponUsed(pItem->GetDesc()->m_nWeaponType.Ref());
#endif
}

void ZGame::OnPeerDie(MUID& uidVictim, MUID& uidAttacker)
{
	//if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_TURBO)
	//{
	//	ZMapSpawnData* pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();
	//	if (pSpawnData != NULL)
	//	{
	//		ZGetGame()->m_pMyCharacter->SetPosition(pSpawnData->m_Pos);
	//		ZGetGame()->m_pMyCharacter->SetDirection(pSpawnData->m_Dir);
	//	}

	//	ZMyCharacter* m_pMyCharacter = ZGetGame()->m_pMyCharacter;
	//	MCommand* pNewCmd = ZGetGameClient()->CreateCommand(MC_PEER_SPAWN, MUID(0, 0));
	//	pNewCmd->AddParameter(new MCommandParameterPos(m_pMyCharacter->GetPosition().x, m_pMyCharacter->GetPosition().y, m_pMyCharacter->GetPosition().z));
	//	pNewCmd->AddParameter(new MCommandParameterDir(m_pMyCharacter->GetDirection().x, m_pMyCharacter->GetDirection().y, m_pMyCharacter->GetDirection().z));
	//	ZGetGameClient()->Post(pNewCmd);

	//	ZGetGame()->ReleaseObserver();
	//	return;
	//}
	//Custom: Instant Respawn Beta By Desperate
	//if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_IS))
	//{
	//	ZMapSpawnData* pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();
	//	if (pSpawnData != NULL)
	//	{
	//		ZGetGame()->m_pMyCharacter->SetPosition(pSpawnData->m_Pos);
	//		ZGetGame()->m_pMyCharacter->SetDirection(pSpawnData->m_Dir);
	//	}

	//	ZMyCharacter* m_pMyCharacter = ZGetGame()->m_pMyCharacter;
	//	MCommand* pNewCmd = ZGetGameClient()->CreateCommand(MC_PEER_SPAWN, MUID(0, 0));
	//	pNewCmd->AddParameter(new MCommandParameterPos(m_pMyCharacter->GetPosition().x, m_pMyCharacter->GetPosition().y, m_pMyCharacter->GetPosition().z));
	//	pNewCmd->AddParameter(new MCommandParameterDir(m_pMyCharacter->GetDirection().x, m_pMyCharacter->GetDirection().y, m_pMyCharacter->GetDirection().z));
	//	ZGetGameClient()->Post(pNewCmd);

	//	ZGetGame()->ReleaseObserver();
	//	return;
	//}
	ZCharacter* pVictim = (ZCharacter*)m_CharacterManager.Find(uidVictim);
	if (pVictim == NULL) return;

	pVictim->ActDead();

	if (pVictim == m_pMyCharacter)
	{
		pVictim->Die();		// ¿©±â¼­ ½ÇÁ¦·Î Á×´Â´Ù. ³ª ÀÚ½ÅÀº ½ÇÁ¦·Îµµ ¿©±â¼­ Á×´Â°Í Ã³¸®

		// ÆÀÇÃ½Ã ¶Ç´Â Äù½ºÆ®¸ðµå½Ã Á×À¸?E¿ÉÁ®?E¸ð?E
		if (m_Match.IsWaitForRoundEnd())
		{
			if (m_CharacterManager.GetCount() > 2)
			{
				if (GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL)
					ReserveObserver();
			}
		}
#ifdef _QUEST
		else if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) ||
			ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
		{
			if (m_CharacterManager.GetCount() >= 2)
			{
				ReserveObserver();
			}
		}
#endif

		CancelSuicide();
	}

	ZCharacter* pAttacker = (ZCharacter*)m_CharacterManager.Find(uidAttacker);
	if (pAttacker == NULL) return;	
#ifdef _DEATHEFFECT
	if (pAttacker->GetName() == " ")
	ZGetEffectManager()->AddDeathEffect(pVictim);
#endif
	if (pAttacker != pVictim)	
	{
		//Only use in team games, no duels or anything else
		if (ZGetGame()->GetMatch()->IsTeamPlay() && ZGetGame()->GetMatch()->GetRoundKills() == 1)
		{
			pAttacker->AddIcon(ZCI_FIRSTBLOOD);
			return;
		}

		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
		{
			if (pAttacker->GetKils() + 1 == 5)
			{
				MEMBER_SET_CHECKCRC(pAttacker->GetStatus(), nFantastic, pAttacker->GetStatus().Ref().nFantastic + 1);
				pAttacker->AddIcon(ZCI_FANTASTIC);
			}
			else if (pAttacker->GetKils() + 1 == 15)
			{
				MEMBER_SET_CHECKCRC(pAttacker->GetStatus(), nExcellent, pAttacker->GetStatus().Ref().nExcellent + 1);
				pAttacker->AddIcon(ZCI_EXCELLENT);
			}
			else if (pAttacker->GetKils() + 1 == 30)
			{
				MEMBER_SET_CHECKCRC(pAttacker->GetStatus(), nUnbelievable, pAttacker->GetStatus().Ref().nUnbelievable + 1);
				pAttacker->AddIcon(ZCI_UNBELIEVABLE);
			}
		}
		else
		{
			if (pAttacker->GetKils() >= 3)
			{
				MEMBER_SET_CHECKCRC(pAttacker->GetStatus(), nFantastic, pAttacker->GetStatus().Ref().nFantastic + 1);
				pAttacker->AddIcon(ZCI_FANTASTIC);
			}
		}

		if (pVictim->GetLastDamageType() == ZD_BULLET_HEADSHOT)
		{
			MEMBER_SET_CHECKCRC(pAttacker->GetStatus(), nHeadShot, pAttacker->GetStatus().Ref().nHeadShot + 1);
			pAttacker->AddIcon(ZCI_HEADSHOT);
		}
	}
}

// ¼­¹ö·ÎºÎÅÍ Á÷Á¢ ³¯¶ó¿À´Â Dead¸Þ¼¼?E
void ZGame::OnPeerDead(const MUID& uidAttacker, const unsigned long int nAttackerArg,
	const MUID& uidVictim, const unsigned long int nVictimArg)
{
	ZCharacter* pVictim = (ZCharacter*)m_CharacterManager.Find(uidVictim);
	ZCharacter* pAttacker = (ZCharacter*)m_CharacterManager.Find(uidAttacker);

	bool bSuicide = false;
	if (uidAttacker == uidVictim) bSuicide = true;

	int nAttackerExp = 0, nVictimExp = 0;

	nAttackerExp = GetExpFromTransData(nAttackerArg);
	nVictimExp = -GetExpFromTransData(nVictimArg);

	if (pAttacker)
	{
		pAttacker->GetStatus().CheckCrc();

		pAttacker->GetStatus().Ref().AddExp(nAttackerExp);
		if (!bSuicide)
			pAttacker->GetStatus().Ref().AddKills();

		pAttacker->GetStatus().MakeCrc();
	}

	if (pVictim)
	{
		if (pVictim != m_pMyCharacter)
		{
			pVictim->Die();		// ¿©±â¼­ ½ÇÁ¦·Î Á×´Â´Ù
		}

		pVictim->GetStatus().CheckCrc();

		pVictim->GetStatus().Ref().AddExp(nVictimExp);
		pVictim->GetStatus().Ref().AddDeaths();
		if (pVictim->GetStatus().Ref().nLife > 0)
			pVictim->GetStatus().Ref().nLife--;

		pVictim->GetStatus().MakeCrc();
	}

	// È­?E°æÇèÄ¡ ÀÌÆåÆ® Ç¥½Ã
	if (bSuicide && (ZGetCharacterManager()->Find(uidAttacker) == ZGetGame()->m_pMyCharacter))
	{
		// ÀÚ?E
		ZGetScreenEffectManager()->AddExpEffect(nVictimExp);
		int nExpPercent = GetExpPercentFromTransData(nVictimArg);
		ZGetMyInfo()->SetLevelPercent(nExpPercent);

		ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();
	}
	else if (ZGetCharacterManager()->Find(uidAttacker) == m_pMyCharacter)
	{
		// ³»°¡ attacker ÀÏ¶§
		int nExpPercent;
		ZGetScreenEffectManager()->AddExpEffect(nAttackerExp);
		nExpPercent = GetExpPercentFromTransData(nAttackerArg);
		ZGetMyInfo()->SetLevelPercent(nExpPercent);
		ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();
	}
	else if (ZGetCharacterManager()->Find(uidVictim) == m_pMyCharacter)
	{
		// ³»°¡ victim ÀÏ¶§
		ZGetScreenEffectManager()->AddExpEffect(nVictimExp);

		int nExpPercent = GetExpPercentFromTransData(nVictimArg);
		ZGetMyInfo()->SetLevelPercent(nExpPercent);
		ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();
	}

#ifdef _DEATHEFFECT
		ZGetEffectManager()->AddDeathEffect(pVictim);
#endif

	m_Match.AddRoundKills();

	CheckKillSound(pAttacker);
	OnPeerDieMessage(pVictim, pAttacker);
}

void ZGame::CheckKillSound(ZCharacter* pAttacker)
{
	if ((!pAttacker) || (pAttacker != m_pMyCharacter)) return;
#ifdef _FIRSTBLOOD
	if (m_Match.GetRoundKills() == 1 && (ZGetGame()->GetMatch()->IsTeamPlay())) 
	{
		char szFirstBlood[50];
		sprintf(szFirstBlood, "%s", pAttacker->GetCharInfo()->szName);
		ZPostAllCmd("1stkill", szFirstBlood, "");
		ZApplication::GetSoundEngine()->PlayVoiceSound(VOICE_FIRST_KILL);
	}
#else
	if (m_Match.GetRoundKills() == 1)
	{
		ZApplication::GetSoundEngine()->PlayVoiceSound(VOICE_FIRST_KILL);
	}
#endif
}

void ZGame::OnReceiveTeamBonus(const MUID& uidChar, const unsigned long int nExpArg)
{
	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uidChar);
	if (pCharacter == NULL) return;

	int nExp = 0;

	nExp = GetExpFromTransData(nExpArg);

	if (pCharacter)
	{
		pCharacter->GetStatus().CheckCrc();
		pCharacter->GetStatus().Ref().AddExp(nExp);
		pCharacter->GetStatus().MakeCrc();
	}

	// È­?E°æÇèÄ¡ ÀÌÆåÆ® Ç¥½Ã
	if (pCharacter == m_pMyCharacter)
	{
#ifdef _DEBUG
		char szTemp[128];
		sprintf(szTemp, "TeamBonus = %d\n", nExp);
		OutputDebugString(szTemp);
#endif

		// ³»°¡ attacker ÀÏ¶§
		ZGetScreenEffectManager()->AddExpEffect(nExp);

		int nExpPercent = GetExpPercentFromTransData(nExpArg);
		ZGetMyInfo()->SetLevelPercent(nExpPercent);
		ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();
	}
}
#ifdef _KILLFEED
#define PREFIX "\xbd"
char* GetWeaponName(MMatchWeaponType WeaponType)
{
	static char szType[38][64] =
	{
		"B",

		// Melee Weapons
		"E",  // MWT_DAGGER
		"R",  // MWT_DUAL_DAGGER
		"Q",  // MWT_KATANA
		"O",  // MWT_DOUBLE_KATANA
		"L",  // MWT_GREAT_SWORD

		// Range Weapons
		"Y",  // MWT_PISTOL
		"T",  // MWT_PISTOLx2
		"Y",  // MWT_REVOLVER
		"T",  // MWT_REVOLVERx2
		"U",  // MWT_SMG
		"I",  // MWT_SMGx2
		"S",  // MWT_SHOTGUN
		"S",  // MWT_SAWED_SHOTGUN
		"P",  // MWT_RIFLE
		"F",  // MWT_MACHINEGUN
		"A",  // MWT_ROCKET
		"D",  // MWT_SNIFER

		"G",  // MWT_MED_KIT
		"G",  // MWT_REPAIR_KIT
		"MWT_BULLET_KIT",
		"MWT_FLASH_BANG",
		"H",  // MWT_FRAGMENTATION
		"MWT_SMOKE_GRENADE",
		"MWT_FOOD",
		"MWT_SKILL",

		"MWT_ENCHANT_FIRE",
		"MWT_ENCHANT_COLD",
		"MWT_ENCHANT_LIGHTNING",
		"MWT_ENCHANT_POISON",

		"MWT_POTION",
		"J",
		"MWT_DYNAMITYE",

		"MWT_STUNGRENADE",
		"MWT_LANDMINE",
		"MWT_SPYCASE",

		"MWT_END" };
	return szType[WeaponType];
}
#endif
void ZGame::OnPeerDieMessage(ZCharacter* pVictim, ZCharacter* pAttacker)
{
	const char* testdeathnametable[ZD_END + 1] = { "¿¡·¯", "ÃÑ", "Ä®", "Ãß¶ô", "Æø¹ß", "HEADSHOT", "¸¶Áö¸·Ä®Áú" };
	char szMsg[256] = "";

	const char* szAnonymous = "?Unknown?";

	char szVictim[256];
	strcpy(szVictim, pVictim ? pVictim->GetUserAndClanName() : szAnonymous);

	char szAttacker[256];
	strcpy(szAttacker, pAttacker ? pAttacker->GetUserAndClanName() : szAnonymous);
#ifdef _KILLFEED
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bFeed && pAttacker && pVictim)
	{
		// Lấy thông tin cơ bản
		const char* szAttName = pAttacker->GetProperty()->GetName();
		const char* szVicName = pVictim->GetProperty()->GetName();
		int nAttTeam = pAttacker->GetTeamID();
		int nVicTeam = pVictim->GetTeamID();
		//MMatchWeaponType nWeapon = pAttacker->GetLastW();
		MMatchWeaponType nWeapon = pVictim->GetWeaponDamaged();

		if (pVictim == pAttacker) {
			// Tự sát: Gửi tên chính mình, nạn nhân để trống, và mã vũ khí là NONE
			ZGetCombatInterface()->GetNotifyKills()->AddKillFeed(szAttName, "", nAttTeam, 0, MWT_NONE);
		}
		else {
			// Chết do bị bắn (Logic cũ)
			ZGetCombatInterface()->GetNotifyKills()->AddKillFeed(szAttName, szVicName, nAttTeam, nVicTeam, nWeapon);
		}
	}
#endif
	if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_INFECTED)
	{
		char szInfectedMsg[128];
		memset(szInfectedMsg, 0, sizeof(szInfectedMsg));

		if (pVictim == pAttacker && pVictim->GetTeamID() == MMT_BLUE)
			sprintf_s(szInfectedMsg, "Human '%s' has died!", pVictim ? pVictim->GetUserName() : "Unknown");
		else if (pVictim->GetTeamID() == MMT_RED)
		{
			sprintf_s(szInfectedMsg, "Zombie '%s' has been slain!", pVictim ? pVictim->GetUserName() : "Unknown");
			ZGetSoundEngine()->PlaySound("zombie_die");
		}
		else if (pVictim->GetTeamID() == MMT_BLUE)
			sprintf_s(szInfectedMsg, "Human '%s' has been infected!", pVictim ? pVictim->GetUserName() : "Unknown");
		ZGetGameInterface()->GetCombatInterface()->UpdateCTFMsg(szInfectedMsg);
	}

	if (pAttacker == pVictim)
	{
		if (pVictim == m_pMyCharacter)
		{
			m_pMyCharacter->GetStatus().CheckCrc();
#ifdef _KILLSTREAK
			if (ZGetGame() && ZGetConfiguration()->GetEtc()->bKill)
			{
			m_pMyCharacter->GetStatus().Ref().nKillStreakCount = 0;
			}
#endif
			m_pMyCharacter->GetStatus().MakeCrc();

			if (m_pMyCharacter->GetLastDamageType() == ZD_EXPLOSION) {
				sprintf(szMsg, ZMsg(MSG_GAME_LOSE_BY_MY_BOMB));
			}
			else {
				sprintf(szMsg, ZMsg(MSG_GAME_LOSE_MYSELF));
			}

			ZChatOutput(MCOLOR(0xFFCF2020), szMsg);
		}
		else
		{
			ZTransMsg(szMsg, MSG_GAME_WHO_LOSE_SELF, 1, szAttacker);
			ZChatOutput(MCOLOR(0xFF707070), szMsg);

			// Admin Grade
			if (ZGetMyInfo()->IsAdminGrade()) {
				MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
				if (pCache && pCache->CheckFlag(MTD_PlayerFlags_AdminHide))
				{
					sprintf(szMsg, "",
						(pAttacker->GetTeamID() == MMT_BLUE) ? 3 : 1,
						pAttacker->GetProperty()->GetName());
					ZGetGameInterface()->GetCombatInterface()->m_AdminMsg.OutputChatMsg(szMsg);
				}
			}
		}
	}

	else if (pAttacker == m_pMyCharacter)
	{
#ifdef _KILLSTREAK
		if (ZGetGame() && ZGetConfiguration()->GetEtc()->bKill)
		{
			int killz = m_pMyCharacter->GetKillStreaks() + 1;
			m_pMyCharacter->GetStatus().CheckCrc();
			m_pMyCharacter->GetStatus().Ref().nKillStreakCount = killz;
			m_pMyCharacter->GetStatus().MakeCrc();
			ZPostKillStreak(szAttacker, m_pMyCharacter->GetKillStreaks(), szVictim);
		}
#endif
#ifdef _FIRSTBLOOD
		if (m_Match.GetRoundKills() == 1 && (ZGetGame()->GetMatch()->IsTeamPlay())) {
			char szFirstBlood[50];
			sprintf(szFirstBlood, "%s", pAttacker->GetCharInfo()->szName);
			ZPostAllCmd("1stkill", szFirstBlood, "");
		}
#endif
	}

	else if (pVictim == m_pMyCharacter)
	{
		// Custom: Fix
		m_pMyCharacter->GetStatus().CheckCrc();
#ifdef _KILLSTREAK
		if (ZGetGame() && ZGetConfiguration()->GetEtc()->bKill)
		{
			m_pMyCharacter->GetStatus().Ref().nKillStreakCount = 0;
		}
#endif
		m_pMyCharacter->GetStatus().MakeCrc();

		//MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
		//if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_CLASSIC_TEAM && !ZGetGameClient()->IsCWChannel() && !m_Match.IsQuestDrived() && pObjCache && pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide) == false)
		//{
		//	ZGetCombatInterface()->SetDeathObserverMode(pAttacker->GetUID());
		//}

		if (pAttacker && !ZGetGameClient()->IsCWChannel())
		{
			/*ZTransMsg(szMsg, MSG_GAME_LOSE_FROM_WHO, 1, szAttacker);
			ZChatOutput(MCOLOR(0xFFCF2020), szMsg);*/
		}
		else if (pAttacker && !ZGetGameClient()->IsLadderWarsChannel())
		{
			/*ZTransMsg(szMsg, MSG_GAME_LOSE_FROM_WHO, 1, szAttacker);
			ZChatOutput(MCOLOR(0xFFCF2020), szMsg);*/
		}
		else
		{
			char szTemp[128];
			sprintf(szTemp, " (HP: %.0f/%.0f, AP: %.0f/%.0f)", pAttacker->GetHP(), pAttacker->GetMaxHP(), pAttacker->GetAP(), pAttacker->GetMaxAP());
			ZTransMsg(szMsg, MSG_GAME_LOSE_FROM_WHO, 1, szAttacker);
			strcat(szMsg, szTemp);
		}
		if (ZGetGame() && ZGetConfiguration()->GetEtc()->bKillCam)
		{
			if (ZGetGameTypeManager()->IsSoloGame(ZGetGame()->GetMatch()->GetMatchType()))
			{
				ZGetCombatInterface()->SetObserverMode(true);
				ZGetCombatInterface()->GetObserver()->SetTarget(pAttacker->GetUID());
			}
		}
	}
	else
	{
		// 		sprintf(szMsg, "%s´ÔÀÌ %s´ÔÀ¸·ÎºÎÅÍ ½Â¸®ÇÏ¿´½À´Ï´Ù.", szAttacker, szVictim );
		ZTransMsg(szMsg, MSG_GAME_WHO_WIN_FROM_OTHER, 2, szAttacker, szVictim);
		ZChatOutput(MCOLOR(0xFF707070), szMsg);

		// Admin Grade
		if (ZGetMyInfo()->IsAdminGrade()) {
			MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
			if (pCache && pCache->CheckFlag(MTD_PlayerFlags_AdminHide))
			{
				// Custom: Changed string
				if (pAttacker)
				{
					sprintf(szMsg, "^%d%s^9 wins,  ^%d%s^9 loses", //sprintf( szMsg, "^%d%s^9 ½Â¸®,  ^%d%s^9 ÆÐ?E,
						(pAttacker->GetTeamID() == MMT_BLUE) ? 3 : 1, pAttacker->GetProperty()->GetName(),
						(pVictim->GetTeamID() == MMT_BLUE) ? 3 : 1, pVictim->GetProperty()->GetName());
					ZGetGameInterface()->GetCombatInterface()->m_AdminMsg.OutputChatMsg(szMsg);
				}
			}
		}
	}
}

void ZGame::OnReloadComplete(ZCharacter* pCharacter)
{
	ZItem* pItem = pCharacter->GetItems()->GetSelectedWeapon();

	pCharacter->GetItems()->Reload();

	if (pCharacter == m_pMyCharacter)
		ZApplication::GetSoundEngine()->PlaySound("we_weapon_rdy");

	return;

}

void ZGame::OnPeerSpMotion(MUID& uid, int nMotionType)
{
	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uid);

	if (pCharacter == NULL) return;

	// Custom: Ignore List check
	if (ZGetGameClient()->IsUserIgnored(pCharacter->GetUserName()) && (nMotionType == ZC_SPMOTION_TAUNT || nMotionType == ZC_SPMOTION_CRY || nMotionType == ZC_SPMOTION_LAUGH))
		return;

	pCharacter->m_dwStatusBitPackingValue.Ref().m_bSpMotion = true;

	ZC_STATE_LOWER zsl = ZC_STATE_TAUNT;

	if (nMotionType == ZC_SPMOTION_TAUNT)
	{
		zsl = ZC_STATE_TAUNT;

		char szSoundName[50];
		if (pCharacter->GetProperty()->nSex == MMS_MALE)
			sprintf(szSoundName, "fx2/MAL1%d", (RandomNumber(0, 300) % 3) + 1);
		else
			sprintf(szSoundName, "fx2/FEM1%d", (RandomNumber(0, 300) % 3) + 1);

		ZGetSoundEngine()->PlaySound(szSoundName, pCharacter->GetPosition());
	}
	else if (nMotionType == ZC_SPMOTION_BOW)
		zsl = ZC_STATE_BOW;
	else if (nMotionType == ZC_SPMOTION_WAVE)
		zsl = ZC_STATE_WAVE;
	else if (nMotionType == ZC_SPMOTION_LAUGH)
	{
		zsl = ZC_STATE_LAUGH;

		if (pCharacter->GetProperty()->nSex == MMS_MALE)
			ZGetSoundEngine()->PlaySound("fx2/MAL01", pCharacter->GetPosition());
		else
			ZGetSoundEngine()->PlaySound("fx2/FEM01", pCharacter->GetPosition());
	}
	else if (nMotionType == ZC_SPMOTION_CRY)
	{
		zsl = ZC_STATE_CRY;

		if (pCharacter->GetProperty()->nSex == MMS_MALE)
			ZGetSoundEngine()->PlaySound("fx2/MAL02", pCharacter->GetPosition());
		else
			ZGetSoundEngine()->PlaySound("fx2/FEM02", pCharacter->GetPosition());
	}
	else if (nMotionType == ZC_SPMOTION_DANCE)
		zsl = ZC_STATE_DANCE;

	pCharacter->m_SpMotion = zsl;

	pCharacter->SetAnimationLower(zsl);
}

#ifdef _ZGUARD
bool ZGame::IsItPossible(const MUID& uid, int nCase)
{
	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uid);
	if (!pCharacter) return true; // Tránh crash

#define RELOAD_PERR 100
#define SUPER_DASH  90
#define TBF_INTERVAL 50

	switch (nCase)
	{
	case 1:
		if ((timeGetTime() - pCharacter->st_nLastPeerReload) < RELOAD_PERR)
		{
			//ZChatOutput("^2[Anti-Cheat]: Reload fast macro Detect.");
			//if (ZGetGameInterface())
			//{
			//	mlog("\n Reload fast macro detected! \n");
			//	ZGetGameInterface()->ShowMessage("Do not try to use the macro or it will be automatically banned.");
			//	ZGetGameInterface()->SetState(GUNZ_LOBBY);
			//	//ZPostBanMe(WHT_RELOADFAST);
			//}
			//return false;
		}
		break;

	case 2:
		if ((timeGetTime() - pCharacter->st_nLastPeerDash) < SUPER_DASH)
		//{
		//	ZChatOutput("^2[Anti-Cheat]: Super dash macro detect.");
		//	if (ZGetGameInterface())
		//	{
		//		ZGetGameInterface()->ShowMessage("Multiple dash has been detected, it will be removed from the room.");
		//		mlog("!!! AUTO-KICK DASH MACRO: %s !!!\n", pCharacter->GetUserName());
		//		MessageBox(NULL,
		//			"The system detects the use of HARDWARE MACRO.\nThe game will automatically exit to ensure fairness!.",
		//			"Easy Anti-Cheat",
		//			MB_OK | MB_ICONSTOP | MB_SETFOREGROUND);

		//		// 3. Sau khi bấm OK sẽ thoát ngay
		//		ExitProcess(NULL);
		//	}
		//	return false;
		//}
		break;

	case 3: // Giả sử Case 3 là check Slash-Block
		if ((timeGetTime() - pCharacter->st_nLastSlashTime) < TBF_INTERVAL)
		{
			//ZChatOutput("^1[Anti-Cheat]: Triple Butterfly (TBF) Macro detected.");
			//if (ZGetGameInterface())
			//{
			//	mlog("TBF Macro Detected! Interval: %d ms\n", (timeGetTime() - pCharacter->st_nLastSlashTime));
			//	ZGetGameInterface()->ShowMessage("Macro TBF is detected, you will be taken to the lobby.");
			//	ZGetGameInterface()->SetState(GUNZ_LOBBY);
			//}
			//return false;
		}
		break;
	}
	return true; // Không phát hiện gian lận
}

bool ZGame::IsMacroPrecisionCheck(ZCharacter* pCharacter)
{
	if (!pCharacter) return false;

	LARGE_INTEGER nNow, nFreq;
	QueryPerformanceFrequency(&nFreq); // Lấy tần số nhịp của hệ thống
	QueryPerformanceCounter(&nNow);   // Lấy nhịp hiện tại

	// Tính khoảng cách thời gian chính xác (đơn vị: Miliseconds)
	double nDelta = (double)(nNow.QuadPart - pCharacter->m_nLastQPCTime.QuadPart) * 1000.0 / nFreq.QuadPart;
	pCharacter->m_nLastQPCTime = nNow;

	// Bỏ qua nếu lần bấm trước quá lâu (không phải đang làm combo)
	if (nDelta > 500.0) return false;

	// Lưu vào lịch sử (giữ 5 mẫu gần nhất)
	pCharacter->m_fIntervalHistory[pCharacter->m_nHistoryIdx] = nDelta;
	pCharacter->m_nHistoryIdx = (pCharacter->m_nHistoryIdx + 1) % 5;

	if (pCharacter->m_nHistoryIdx == 0)
	{
		double fSum = 0;
		for (int i = 0; i < 5; i++) fSum += pCharacter->m_fIntervalHistory[i];
		double fAvg = fSum / 5.0;

		double fVariance = 0;
		for (int i = 0; i < 5; i++)
			fVariance += pow(pCharacter->m_fIntervalHistory[i] - fAvg, 2);
		fVariance /= 5.0;

		// LOG ĐỂ TEST:
		mlog("TBF Speed: %.2f ms | Jitter (Variance): %.6f\n", fAvg, fVariance);

		// THÔNG SỐ VÀNG:
		// Người thật: fVariance thường > 1.0 (nhảy số liên tục)
		// Macro: fVariance thường < 0.005 (sai số cực thấp vì chip xử lý)
		if (fAvg < 80.0 && fVariance < 0.01)
		{
			return true;
		}
	}
	return false;
}

#endif

void ZGame::OnPeerReload(MUID& uid)
{
	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uid);
#ifdef _ZGUARD
	if (pCharacter == NULL || pCharacter->IsDie()) return;
	if (!IsItPossible(uid, 1)) return;
	pCharacter->st_nLastPeerReload = timeGetTime();
#else
	if (pCharacter == NULL || pCharacter->IsDie()) return;
#endif

	if (pCharacter == m_pMyCharacter)
	{
		// Custom: Fast Reload Room Modifier
		if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_FASTRELOAD) && !ZGetGameClient()->GetMatchStageSetting()->IsQuestDrived() && !ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
		{
			m_pMyCharacter->Animation_Reload();
			OnReloadComplete(pCharacter);
		}
		else if (ZGetGame()->GetMatch()->IsTurboMode())
		{
			m_pMyCharacter->Animation_Reload();
			OnReloadComplete(pCharacter);
		}
		else
			m_pMyCharacter->Animation_Reload();
	}
	else
		OnReloadComplete(pCharacter);

	// Sound Effect
	if (pCharacter->GetItems()->GetSelectedWeapon() != NULL) {
		rvector p = pCharacter->GetPosition() + rvector(0, 0, 160.f);
		ZApplication::GetSoundEngine()->PlaySEReload(pCharacter->GetItems()->GetSelectedWeapon()->GetDesc(), p.x, p.y, p.z, (pCharacter == m_pMyCharacter));
	}
}

void ZGame::OnPeerChangeCharacter(MUID& uid)
{
	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uid);

	//	if (uid == ZGetGameClient()->GetUID()) pCharacter = m_pMyCharacter;

	if (pCharacter == NULL) return;

	pCharacter->TestToggleCharacter();
}

/*
void ZGame::OnAssignCommander(const MUID& uidRedCommander, const MUID& uidBlueCommander)
{
	AssignCommander(uidRedCommander, uidBlueCommander);
}

void ZGame::AssignCommander(const MUID& uidRedCommander, const MUID& uidBlueCommander)
{
	ZCharacter* pRedChar = m_CharacterManager.Find(uidRedCommander);
	ZCharacter* pBlueChar = m_CharacterManager.Find(uidBlueCommander);

	if(pRedChar) {
		ZGetEffectManager()->AddCommanderIcon(pRedChar,0);
		pRedChar->m_bCommander = true;
	}
	if(pBlueChar) {
		ZGetEffectManager()->AddCommanderIcon(pBlueChar,1);
		pBlueChar->m_bCommander = true;
	}

#ifdef _DEBUG
	//// DEBUG LOG ////
	const char *szUnknown = "unknown";
	char szBuf[128];
	sprintf(szBuf, "RedCMDER=%s , BlueCMDER=%s \n",
		pRedChar ? pRedChar->GetProperty()->szName : szUnknown ,
		pBlueChar ? pBlueChar->GetProperty()->szName : szUnknown );
	OutputDebugString(szBuf);
	///////////////////
#endif
}
*/
void ZGame::OnSetObserver(MUID& uid)
{
	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uid);
	if (pCharacter == NULL) return;

	if (pCharacter == m_pMyCharacter)
	{
		ZGetCombatInterface()->SetObserverMode(true);
	}
	pCharacter->SetVisible(false);
	pCharacter->ForceDie();
}

void ZGame::OnPeerSpawn(MUID& uid, rvector& pos, rvector& dir)
{
	m_nSpawnTime = timeGetTime();
	SetSpawnRequested(false);

	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uid);
	if (pCharacter == NULL) return;
	bool isRespawn = (pCharacter->IsDie() == true) ? true : false;

	//	dir = rvector(-1.f,0,0);
	pCharacter->SetVisible(true);
	pCharacter->Revival();
	pCharacter->SetPosition(pos);
	pCharacter->SetDirection(dir);
	pCharacter->SetSpawnTime(GetTime());

	ZGetEffectManager()->AddReBirthEffect(pos);

	if (pCharacter == m_pMyCharacter)
	{
		m_pMyCharacter->InitSpawn();
		if (ZGetGame() && ZGetConfiguration()->GetEtc()->bKillCam)
		{
			if (ZGetGameTypeManager()->IsSoloGame(ZGetGame()->GetMatch()->GetMatchType()))
			{
				ZGetCombatInterface()->SetObserverMode(false);
			}
		}
		if (isRespawn)
		{
			ZGetSoundEngine()->PlaySound("fx_respawn");
		}
		else
		{
			ZGetSoundEngine()->PlaySound("fx_whoosh02");
		}
		//ZGetScreenEffectManager()->ReSetHpPanel();
		//ZGetScreenEffectManager()->ReSetHpPanelNew();
#ifdef _SPEC 1
		ReleaseObserver();
#endif
	}
	pCharacter->GetStatus().CheckCrc();
	pCharacter->GetStatus().Ref().nGivenDamage = 0;
	int LastGiven = m_pMyCharacter->GetStatus().Ref().nRoundGivenDamage, LastTaken = m_pMyCharacter->GetStatus().Ref().nRoundTakenDamage;
	m_pMyCharacter->GetStatus().Ref().nRoundLastGivenDamage = LastGiven;
	m_pMyCharacter->GetStatus().Ref().nRoundLastTakenDamage = LastTaken;
	ZPOSTLASTDMG(MCommandParameterInt(LastGiven), MCommandParameterInt(LastTaken));
	m_pMyCharacter->GetStatus().Ref().nRoundGivenDamage = 0;
	m_pMyCharacter->GetStatus().Ref().nRoundTakenDamage = 0;
	pCharacter->GetStatus().MakeCrc();

	// Custom: CTF
	//if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DEATHMATCH_TEAM2)
	if (ZGetGameTypeManager()->IsTeamExtremeGame(GetMatch()->GetMatchType()))
		pCharacter->SetInvincibleTime(5000);
	// Custom: Spawn protection for non-team modes
	else if (!ZGetGameTypeManager()->IsTeamGame(GetMatch()->GetMatchType()) && GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT && GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL)
		pCharacter->SetInvincibleTime(2000);

	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bRespawnTrasparent)
	{
		pCharacter->SetInvincibleTime(2000);
	}
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SKILLMAP)
	{
		pCharacter->SetInvincibleTime(15000);
	}
}

void ZGame::OnPeerDash(MCommand* pCommand)
{
	MCommandParameter* pParam = pCommand->GetParameter(0);
	if (pParam->GetType() != MPT_BLOB) return;

	MUID uid = pCommand->GetSenderUID();
	ZPACKEDDASHINFO* ppdi = (ZPACKEDDASHINFO*)pParam->GetPointer();

	rvector pos, dir;
	int sel_type;

	pos = rvector(Roundf(ppdi->posx), Roundf(ppdi->posy), Roundf(ppdi->posz));
	dir = 1.f / 32000.f * rvector(ppdi->dirx, ppdi->diry, ppdi->dirz);
	sel_type = (int)ppdi->seltype;

	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uid);

	if (pCharacter == NULL) return;

	MMatchCharItemParts parts = (MMatchCharItemParts)sel_type;

	if (parts != pCharacter->GetItems()->GetSelectedWeaponParts()) {
		// Áö±Ý µé?EÀÖ´Â ¹«±â¿Í º¸³»?E¹«±â°¡ Æ²¸®´Ù?Eº¸³»?E¹«±â·Î ¹Ù²ãÁØ´Ù..
		OnChangeWeapon(uid, parts);
	}
	const char* szPeerDashMesh = NULL;
	if (ppdi->nDashItemID != 0)
	{
		MMatchItemDesc* pPeerDashDesc = MGetMatchItemDescMgr()->GetItemDesc(ppdi->nDashItemID);
		if (pPeerDashDesc != NULL)
			szPeerDashMesh = pPeerDashDesc->m_pMItemName->Ref().m_szMeshName;
	}
	ZGetEffectManager()->AddDashEffect(pos, dir, pCharacter, szPeerDashMesh);

#ifdef _ZGUARD
	pCharacter->SetLastDashX((int)ppdi->posx);
	if (!IsItPossible(uid, 2)) return;
	pCharacter->st_nLastPeerDash = timeGetTime();
#endif

}

//#define CHAR_COLLISION_HEIGHT	170.f
#ifdef _MOVINGFLOOR
rvector ZGame::GetFloor(rvector pos, rplane* pimpactplane, MUID myUID)
{
	//rvector floor = ZGetGame()->GetWorld()->GetFloor(pos + rvector(0, 0, 120), CHARACTER_RADIUS - 1.1f, 58.f, pimpactplane);
	rvector floor = ZGetGame()->GetWorld()->GetBsp()->GetFloor(pos + rvector(0, 0, 120), CHARACTER_RADIUS - 1.1f, 58.f, pimpactplane);
#ifdef ENABLE_CHARACTER_COLLISION
	for (auto const& itor : m_ObjectManager)
	{
		ZObject* pObject = itor.second;
		if (pObject == nullptr || pObject == m_pMyCharacter)
			continue;
		if (pObject->IsCollideable())
			//		if(!pCharacter->IsDie() && !pCharacter->m_bBlastDrop)
		{
			rvector diff = pObject->GetPosition() - pos;
			diff.z = 0;

			// ³ªÁß¿¡ radius»ó¼ö°ªÀ¸·Î µÈ°Í ObjectÀÇ ¸â¹öº¯¼ö·Î °íÄ¡ÀÚ
			if (Magnitude(diff) < CHARACTER_RADIUS && pos.z > pObject->GetPosition().z)
			{
				rvector newfloor = pObject->GetPosition() + rvector(0, 0, pObject->GetCollHeight());
				if (floor.z < newfloor.z)
				{
					if (m_pMyCharacter->GetUID() == myUID)
					{// ³» Ä³¸¯ÅÍ ¹Ù´Ú À§Ä¡¸¸ Ã³¸®ÇØÁØ´Ù.
						///< Á¡ÇÁ¹ö±× ¶§¹®¿¡ ÀÛ¾÷(ÇÑÄ³¸¯ÅÍÀ§¿¡ ´Ù¸¥ Ä³¸¯ÅÍ°¡ ¿Ã¶ó °¬À»¶§ ¹Ø¿¡ Ä³¸¯ÅÍ°¡ Á¡ÇÁ½Ã 
						///< À§Ä³¸¯ÅÍÀÇ ³ôÀÌ°¡ °»½ÅÀÌ ¾ÈµÇ¸é µÎÄ³¸¯ÅÍ°¡ ¹«ÇÑÁ¤ À§·Î ¿Ã¶ó°¡´Â ¹ö±×)
						if (CharacterOverlapCollision(pObject, floor.z, newfloor.z) == false)
							continue;
					}

					floor = newfloor;
					if (pimpactplane)
					{
						rvector up = rvector(0, 0, 1);
						D3DXPlaneFromPointNormal(pimpactplane, &floor, &up);
					}
				}
			}
		}
	}

#endif

	return floor;
}
#else
rvector ZGame::GetFloor(rvector pos, rplane* pimpactplane, MUID myUID)
{
	rvector floor = ZGetGame()->GetWorld()->GetBsp()->GetFloor(pos + rvector(0, 0, 120), CHARACTER_RADIUS - 1.1f, 58.f, pimpactplane);

#ifdef ENABLE_CHARACTER_COLLISION
	ZObjectManager::iterator itor = m_ObjectManager.begin();
	for (; itor != m_ObjectManager.end(); ++itor)
	{
		ZObject* pObject = (*itor).second;
		if (pObject->IsCollideable())
			//		if(!pCharacter->IsDie() && !pCharacter->m_bBlastDrop)
		{
			rvector diff = pObject->GetPosition() - pos;
			diff.z = 0;

			// ³ªÁß¿¡ radius»ó¼ö°ªÀ¸·Î µÈ°Í ObjectÀÇ ¸â¹öº¯¼ö·Î °úà¡ÀÚ
			if (Magnitude(diff) < CHARACTER_RADIUS && pos.z > pObject->GetPosition().z)
			{
				rvector newfloor = pObject->GetPosition() + rvector(0, 0, pObject->GetCollHeight());
				if (floor.z < newfloor.z)
				{
					if (m_pMyCharacter->GetUID() == myUID)
					{// ³» Ä³¸¯ÅÍ ¹Ù´Ú À§Ä¡¸¸ Ã³¸®ÇØÁØ´Ù.
						///< Á¡ÇÁ¹ö±× ¶§¹®¿¡ ÀÛ?EÇÑÄ³¸¯ÅÍÀ§¿¡ ´Ù¸¥ Ä³¸¯ÅÍ°¡ ¿Ã?E°¬À»¶§ ¹Ø¿¡ Ä³¸¯ÅÍ°¡ Á¡ÇÁ½Ã
						///< À§Ä³¸¯ÅÍÀÇ ³ôÀÌ°¡ °»½ÅÀÌ ¾ÈµÇ?EµÎÄ³¸¯ÅÍ°¡ ¹«ÇÑÁ¤ À§·Î ¿Ã¶ó°¡´Â ¹ö±×)
						if (CharacterOverlapCollision(pObject, floor.z, newfloor.z) == false)
							continue;
					}

					floor = newfloor;
					if (pimpactplane)
					{
						rvector up = rvector(0, 0, 1);
						D3DXPlaneFromPointNormal(pimpactplane, &floor, &up);
					}
				}
			}
		}
	}
#endif

	return floor;
}
#endif
bool ZGame::CharacterOverlapCollision(ZObject* pFloorObject, float WorldFloorHeight, float ObjectFloorHeight)
{
	OVERLAP_FLOOR* pOverlapObject = m_pMyCharacter->GetOverlapFloor();

	if (pOverlapObject->FloorUID != pFloorObject->GetUID())
	{ // ¹â°úÜÖ?EÄ³¸¯ÅÍ°¡ º¯°æµÅ?E´Ù½Ã ¼¼ÆÃÇØÁØ´Ù.
		pOverlapObject->FloorUID = pFloorObject->GetUID();
		pOverlapObject->vecPosition.z = ObjectFloorHeight;
		pOverlapObject->nFloorCnt = 0;
		pOverlapObject->bJumpActivity = false;
	}
	else
	{ // °è¼Ó ¹â°úÜÖ´Â Ä³¸¯ÅÍ Ã³¸®
		if (pOverlapObject->bJumpActivity)
		{ // Á¡ÇÁ¹ö±× ¹ßµ¿
			if (m_pMyCharacter->GetPosition().z - WorldFloorHeight > 20.f)
			{ // ³«ÇÏ³ôÀÌ°¡ ¹Ù´Ú¿¡ °¡±û?EÁ³À»¶§ OVERLAP_FLOOR ÃÊ±âÈ­
				pOverlapObject->FloorUID = MUID(0, 0);
				pOverlapObject->nFloorCnt = 0;
				pOverlapObject->vecPosition.x = 0;
				pOverlapObject->vecPosition.y = 0;
				pOverlapObject->vecPosition.z = 0;
				pOverlapObject->bJumpActivity = false;
			}
			return false;	// Á¡ÇÁ¹ö±×°¡ ¹ßµ¿µÆÀ¸?E¹â°úÜÖ?EÄ³¸¯ÅÍ¸¦ ¹«½ÃÇÔÀ¸·Î ³«ÇÏÇÔ
		}

		if (ObjectFloorHeight - pOverlapObject->vecPosition.z > 150.f)
		{
			pOverlapObject->vecPosition.z = ObjectFloorHeight;
			pOverlapObject->nFloorCnt++;
			if (pOverlapObject->nFloorCnt >= 3)
			{
				pOverlapObject->bJumpActivity = true;
				mlog("Jump bug Activity \n");
				return false;
			}
		}
	}

	return true;
}

/*
rvector ZGame::GetCeiling(rvector pos)
{
rvector ceiling=g_pGame->GetWorld()->GetBsp()->GetCeiling(pos+rvector(0,0,130),CHARACTER_RADIUS-0.1f);

#ifdef ENABLE_CHARACTER_COLLISION
for (ZCharacterManager::iterator itor = m_CharacterManager.begin();
itor != m_CharacterManager.end(); ++itor)
{
ZCharacter* pCharacter = (*itor).second;
if(pCharacter!=m_pMyCharacter && !pCharacter->IsDie() && !pCharacter->m_bBlastDrop)
{
rvector diff=pCharacter->m_Position-m_pMyCharacter->m_Position;
diff.z=0;

if(Magnitude(diff)<CHARACTER_RADIUS && pos.z+CHAR_COLLISION_HEIGHT<pCharacter->m_Position.z)
{
rvector newceiling=pCharacter->m_Position;
if(ceiling.z<newceiling.z)
ceiling=newceiling;
}
}
}
#endif

return ceiling;
}
*/

bool ZGame::Pick(ZObject* pOwnerObject, rvector& origin, rvector& dir, ZPICKINFO* pickinfo, DWORD dwPassFlag, bool bMyChar)
{
	return PickHistory(pOwnerObject, GetTime(), origin, origin + 10000.f * dir, pickinfo, dwPassFlag, bMyChar);
}

bool ZGame::PickTo(ZObject* pOwnerObject, rvector& origin, rvector& to, ZPICKINFO* pickinfo, DWORD dwPassFlag, bool bMyChar)
{
	return PickHistory(pOwnerObject, GetTime(), origin, to, pickinfo, dwPassFlag, bMyChar);
}

// fTime ½Ã°£ÀÇ Ä³¸¯ÅÍ À§Ä¡·Î pick ÇÑ´Ù.. Ä³¸¯ÅÍ´Â ½Ç¸°?EÆÇÁ¤.
bool ZGame::PickHistory(ZObject* pOwnerObject, float fTime, const rvector& origin, const rvector& to, ZPICKINFO* pickinfo, DWORD dwPassFlag, bool bMyChar)
{
	pickinfo->pObject = NULL;
	pickinfo->bBspPicked = false;
	pickinfo->nBspPicked_DebugRegister = -10;

	RPickInfo info;
	memset(&info, 0, sizeof(RPickInfo));

	ZObject* pObject = NULL;

	bool bCheck = false;

	float fCharacterDist = FLT_MAX;			// Ä³¸¯ÅÍµé »çÀÌÀÇ ÃÖ¼Ò°Å¸®ÁöÁ¡À» Ã£´Â´Ù
	for (ZObjectManager::iterator i = m_ObjectManager.begin(); i != m_ObjectManager.end(); i++)
	{
		ZObject* pc = i->second;

		bCheck = false;

		if (bMyChar) {
			if (pc == pOwnerObject && pc->IsVisible()) {
				bCheck = true;
			}
		}
		else {
			if (pc != pOwnerObject && pc->IsVisible()) {
				bCheck = true;
			}
		}

		if (pc->IsDie())//Á×Àº³ÑÀÌ ¸ö»§ÇÑ´Ù°í ÇØ¼­~
			bCheck = false;

		if (bCheck)
		{
			rvector hitPos;
			ZOBJECTHITTEST ht = pc->HitTest(origin, to, fTime, &hitPos);
			if (ht != ZOH_NONE) {
				float fDistToChar = Magnitude(hitPos - origin);
				if (fDistToChar < fCharacterDist) {
					pObject = pc;
					fCharacterDist = fDistToChar;
					info.vOut = hitPos;
					switch (ht) {
					case ZOH_HEAD: info.parts = eq_parts_head; break;
					case ZOH_BODY: info.parts = eq_parts_chest; break;
					case ZOH_LEGS:	info.parts = eq_parts_legs; break;
					}
				}
			}
		}
	}

	RBSPPICKINFO bpi;
	bool bBspPicked = GetWorld()->GetBsp()->PickTo(origin, to, &bpi, dwPassFlag);

	int nCase = 0;

	if (pObject && bBspPicked)		// µÑ´Ù ¸Â¾ÒÀ»¶§´Â °Å¸®°¡ °¡±î¿îÂÊÀ» ÅÃÇÑ´Ù.
	{
		if (Magnitude(info.vOut - origin) > Magnitude(bpi.PickPos - origin))
			nCase = 1;
		else
			nCase = 2;
	}
	else
		if (bBspPicked)				// µÑÁß ÇÏ³ª¸¸ ¸Â¾ÒÀ¸¸é ¸ÂÀº°É ÅÃÇÏ¸é µÈ´Ù.
			nCase = 1;
		else
			if (pObject)
				nCase = 2;

	if (nCase == 0) return false;

	switch (nCase)
	{
	case 1:						// ¸Ê¿¡ ¸ÂÀº°æ¿ì
		pickinfo->bBspPicked = true;
		pickinfo->nBspPicked_DebugRegister = FOR_DEBUG_REGISTER;
		pickinfo->bpi = bpi;
		break;
	case 2:						// »ç¶÷¿¡ ¸ÂÀº°æ¿ì.
		pickinfo->pObject = pObject;
		pickinfo->info = info;
		break;
	}
	return true;
}

bool ZGame::ObjectColTest(ZObject* pOwner, rvector& origin, rvector& to, float fRadius, ZObject** poutTarget)
{
	// ¸Ê¿¡ ¸Â´Â°ÍÀº Ã¼Å©ÇÏ?E¾Ê´Â´Ù.

	for (ZObjectManager::iterator i = m_ObjectManager.begin(); i != m_ObjectManager.end(); i++)
	{
		ZObject* pc = i->second;

		if (pc == pOwner)
			continue;

		if (!pc->IsVisible())
			continue;

		if (pc->IsDie())
			continue;

		if (pc->ColTest(origin, to, fRadius, GetTime()))
		{
			*poutTarget = pc;
			return true;
		}
	}

	return false;
}

char* ZGame::GetSndNameFromBsp(const char* szSrcSndName, RMATERIAL* pMaterial)
{
	char szMaterial[256] = "";
	static char szRealSndName[256] = "";
	szRealSndName[0] = 0;

	if (pMaterial == NULL) return "";

	strcpy(szMaterial, pMaterial->Name.c_str());

	size_t nLen = strlen(szMaterial);

#define ZMETERIAL_SNDNAME_LEN 7

	if ((nLen > ZMETERIAL_SNDNAME_LEN) &&
		(!strnicmp(&szMaterial[nLen - ZMETERIAL_SNDNAME_LEN + 1], "mt", 2)))
	{
		strcpy(szRealSndName, szSrcSndName);
		strcat(szRealSndName, "_");
		strcat(szRealSndName, &szMaterial[nLen - ZMETERIAL_SNDNAME_LEN + 1]);
	}
	else
	{
		strcpy(szRealSndName, szSrcSndName);
	}

	return szRealSndName;
}

/*
void ZGame::AdjustGlobalTime()
{
// Ä«¿ûâ® ´Ù¿ûãÒ¶§¸¸ ½Ã°£À» ½ÌÅ©ÇÑ´Ù
//	if(GetMatch()->GetRoundState()!=MMATCH_ROUNDSTATE_COUNTDOWN) return;
static DWORD nLastTime = GetTickTime();
DWORD nNowTime = GetTickTime();
if((nNowTime - nLastTime) < 100) return;	// 100¹Ð¸®¼¼ÄÁµå¸¶´Ù Ã¼Å©

nLastTime = nNowTime;

float fAverageTime=0.f;

int nValidCount=0;

ZCharacterManager::iterator i;
for(i=m_CharacterManager.begin();i!=m_CharacterManager.end();i++)
{
ZCharacter *pCharacter=i->second;
if(pCharacter->m_BasicHistory.size()==0) continue;		// À¯È¿ÇÏ?E¾Ê´Ù

// Ä³¸¯ÅÍ°¡ ¸¶Áö¸·À¸·Î º¸³»¿Â Á¤º¸¸¦ ?E?
ZBasicInfoHistory::iterator infoi=pCharacter->m_BasicHistory.end();
infoi--;
ZBasicInfoItem *pInfo=*infoi;

// ¸¶Áö¸· µ¥ÀÌÅÍ ¹ÞÀº?E3ÃÊ ÀÌ»óÀÌ?E¹®Á¦°¡ ÀÖ´Ù?EÆÇÁ¤. À¯È¿ÇÏ?E¾Ê´Ù
if(GetTime()-pInfo->fReceivedTime > 3.f) continue;

float fCharacterTime=pInfo->info.fSendTime+(GetTime()-pInfo->fReceivedTime);

nValidCount++;
fAverageTime+=fCharacterTime;
}

fAverageTime/=(float)nValidCount;
fAverageTime=max(fAverageTime,0);	// 0º¸´Ù ÀÛÀ»¼ö´Â ¾ø´Ù.

// ±Û·Î?E½Ã°£°úÀÇ Â÷ÀÌ¸¦ ´©ÀûÇÑ´Ù.
for(i=m_CharacterManager.begin();i!=m_CharacterManager.end();i++)
{
ZCharacter *pCharacter=i->second;
if(pCharacter->m_BasicHistory.size()==0) continue;		// À¯È¿ÇÏ?E¾Ê´Ù

ZBasicInfoHistory::iterator infoi=pCharacter->m_BasicHistory.end();
infoi--;
ZBasicInfoItem *pInfo=*infoi;
float fCharacterTime=pInfo->info.fSendTime+(GetTime()-pInfo->fReceivedTime);

pCharacter->m_TimeErrors[pCharacter->m_nTimeErrorCount++]=fAverageTime-fCharacterTime;
if( TIME_ERROR_CORRECTION_PERIOD == pCharacter->m_nTimeErrorCount )
{
pCharacter->m_nTimeErrorCount=0;
float fAvrTimeError=0;
for(int j=0;j<TIME_ERROR_CORRECTION_PERIOD;j++)
fAvrTimeError+=pCharacter->m_TimeErrors[j];
fAvrTimeError/=(float)TIME_ERROR_CORRECTION_PERIOD;

pCharacter->m_fAccumulatedTimeError+=fAvrTimeError*.5f;
if(fabs(pCharacter->m_fAccumulatedTimeError)>10.f)
{
#ifndef _PUBLISH
char szTemp[256];
sprintf(szTemp, "%s´ÔÀÌ ½ºÇÇµåÇÙ ? %3.1f", pCharacter->GetProperty()->szName,pCharacter->m_fAccumulatedTimeError);
ZGetGameInterface()->OutputChatMsg(MCOLOR(0xFFFF0000), szTemp);
#endif

pCharacter->m_fAccumulatedTimeError=0;
}

if(pCharacter==m_pMyCharacter)
{
m_fTime+=fAvrTimeError*.5f;
}
}
}
}
*/

// ?E?
#ifdef _ASSITEN
void ZGame::AutoAiming()
{

	if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PREPARE)
		return;
	if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PRE_COUNTDOWN)
		return;
	if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_FREE)
		return;
	if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_EXIT)
		return;
	if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_FINISH)
		return;
	if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_END)
		return;

	if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
		return;

	if (ZGetGame()->m_pMyCharacter->IsDie())
		return;

	if (ZApplication::GetGameInterface()->IsLeaveBattleReserved() == true)
		return;

	ZCamera* pCamera = ZGetGameInterface()->GetCamera();
	rvector vMyPos = m_pMyCharacter->GetPosition();


	ZCharacter* pCharacter;
	rvector pos;
	rvector dir;

	for (ZCharacterManager::iterator i = m_CharacterManager.begin(); i != m_CharacterManager.end(); i++)
	{
		pCharacter = (ZCharacter*)i->second;
		if (pCharacter != m_pMyCharacter)
		{
			if (pCharacter->IsDie() == false)
			{
				pos = pCharacter->GetPosition();
				pos.z += 140.f;
				dir = pos - RCameraPosition;
				Normalize(dir);
				pCamera->SetDirection(dir);
			}
		}
	}
}
#endif

#define MAX_PLAYERS		64

// ÅõÇ¥´Â Á¦°Å µÇ¾úÀ¸¹Ç·Î ³» ÇÇÁ¤º¸¸¸ º¸³½´Ù
void ZGame::PostHPAPInfo()
{
	DWORD nNowTime = GetTickTime();

	if (m_pMyCharacter->GetInitialized() == false) return;

	if ((nNowTime - m_nLastTime[ZLASTTIME_HPINFO]) >= PEER_HP_TICK)
	{
		m_nLastTime[ZLASTTIME_HPINFO] = nNowTime;

		ZPostHPAPInfo(m_pMyCharacter->GetHP(), m_pMyCharacter->GetAP());
	}

#ifdef ENABLE_ADJUST_MY_DATA
	//	AdjustMyData();
#endif
}

// µà¾óÅä³Ê¸ÕÆ® UIÁß¿¡¼­ »ó?EE°ÔÀÌÁöµµ º¸¿©ÁÖ±â¶§¹®¿¡
// 0.1ÃÊ¸¶´Ù HP, AP¸¦ À¯Àúµé¿¡°Ô º¸³»Áà¾ß ÇÑ´Ù. (PostHPAPInfo()¿¡¼­´Â 1ÃÊ¿¡ ÇÑ¹ø¾¿ Ã³¸®ÇØÁÜ)
void ZGame::PostDuelTournamentHPAPInfo()
{
	DWORD nNowTime = GetTickTime();

	if (m_pMyCharacter->GetInitialized() == false) return;

	if ((nNowTime - m_nLastTime[ZLASTTIME_HPINFO]) >= PEER_DUELTOURNAMENT_HPAP_TICK)
	{
		m_nLastTime[ZLASTTIME_HPINFO] = nNowTime;

		BYTE MaxHP = (BYTE)m_pMyCharacter->GetMaxHP();
		BYTE MaxAP = (BYTE)m_pMyCharacter->GetMaxAP();
		BYTE HP = (BYTE)m_pMyCharacter->GetHP();
		BYTE AP = (BYTE)m_pMyCharacter->GetAP();

		ZPostDuelTournamentHPAPInfo(MaxHP, MaxAP, HP, AP);
	}
}

void ZGame::PostBasicInfo()
{
	if (!ZGetGameInterface()->GetCombatInterface()->IsNetworkalive())
		return;

	DWORD nNowTime = timeGetTime();

	if (m_pMyCharacter->GetInitialized() == false) return;
	if (m_pMyCharacter->IsDie() && GetTime() - m_pMyCharacter->m_timeInfo.Ref().m_fDeadTime > 5.f) return;

	int nMoveTick = GetCharacterBasicInfoTick();

	if ((int)(nNowTime - m_nLastTime[ZLASTTIME_BASICINFO]) >= nMoveTick)
	{
		m_nLastTime[ZLASTTIME_BASICINFO] = nNowTime;

		ZPACKEDBASICINFO pbi;
		pbi.fTime = GetTime();

		pbi.posx = m_pMyCharacter->GetPosition().x;
		pbi.posy = m_pMyCharacter->GetPosition().y;
		pbi.posz = m_pMyCharacter->GetPosition().z;

		pbi.velx = m_pMyCharacter->GetVelocity().x;
		pbi.vely = m_pMyCharacter->GetVelocity().y;
		pbi.velz = m_pMyCharacter->GetVelocity().z;

		pbi.dirx = m_pMyCharacter->m_TargetDir.x * 32000;
		pbi.diry = m_pMyCharacter->m_TargetDir.y * 32000;
		pbi.dirz = m_pMyCharacter->m_TargetDir.z * 32000;

		pbi.upperstate = m_pMyCharacter->GetStateUpper();
		pbi.lowerstate = m_pMyCharacter->GetStateLower();
		pbi.selweapon = m_pMyCharacter->GetItems()->GetSelectedWeaponParts();
#ifdef _ZGUARD
		pbi.nType = m_pMyCharacter->nGetnType();
		pbi.nKeyWeapon2 = ZGetConfiguration()->GetKeyboard()->ActionKeys[ZACTION_USE_WEAPON].nVirtualKey;
		pbi.nKeyWeapon2Alt = ZGetConfiguration()->GetKeyboard()->ActionKeys[ZACTION_USE_WEAPON].nVirtualKeyAlt;
#endif
		ZPOSTCMD1(MC_PEER_BASICINFO, MCommandParameterBlob(&pbi, sizeof(ZPACKEDBASICINFO)));
	}
}
#ifdef _UPCHARCMD
void ZGame::PostUpdateCharacter()
{
	// ÀÎÅÍ³ÝÀÌ ²÷°åÀ¸¸é Å°ÀÎÇ² Ã³¸®¸¦ ÇÏÁö ¾Ê´Â´Ù.(·£¼± »Ì¾Æ ¾Ç¿ë ¹æÁö)
	if (!ZGetGameInterface()->GetCombatInterface()->IsNetworkalive())
		return;

	if (m_pMyCharacter->GetInitialized() == false)
		return;

	ZCharacter* pChar = m_pMyCharacter;
	MTD_CharInfo* pCharInfo = (MTD_CharInfo*)pChar->GetCharInfo();

	ZPOSTCMD1(MC_PEER_UPDATECHAR, MCommandParameterBlob(pCharInfo, sizeof(MTD_CharInfo)));
}
#endif
void ZGame::PostPeerPingInfo()
{
	if (!ZGetGameInterface()->GetCombatInterface()->IsShowScoreBoard()) return;

	DWORD nNowTime = GetTickTime();

	if ((nNowTime - m_nLastTime[ZLASTTIME_PEERPINGINFO]) >= PEER_PING_TICK) {
		m_nLastTime[ZLASTTIME_PEERPINGINFO] = nNowTime;

		unsigned long nTimeStamp = GetTickTime();
		MMatchPeerInfoList* pPeers = ZGetGameClient()->GetPeers();
		for (MMatchPeerInfoList::iterator itor = pPeers->begin(); itor != pPeers->end(); ++itor) {
			MMatchPeerInfo* pPeerInfo = (*itor).second;
			if (pPeerInfo->uidChar != ZGetGameClient()->GetPlayerUID()) {
				_ASSERT(pPeerInfo->uidChar != MUID(0, 0));

				MCommandManager* MCmdMgr = ZGetGameClient()->GetCommandManager();
				MCommand* pCmd = new MCommand(MCmdMgr->GetCommandDescByID(MC_PEER_PING),
					pPeerInfo->uidChar, ZGetGameClient()->GetUID());
				pCmd->AddParameter(new MCmdParamUInt(nTimeStamp));
				ZGetGameClient()->Post(pCmd);

#ifdef _DEBUG
				g_nPingCount++;
#endif
				pPeerInfo->SetLastPingTime(nTimeStamp);
			}
		}
	}
}

//void ZGame::PostSyncReport()
//{
//	DWORD nNowTime = GetTickTime();
//
//#ifdef _PUBLISH
//	if ((nNowTime - m_nLastTime[ZLASTTIME_SYNC_REPORT]) >= MATCH_CYCLE_CHECK_SPEEDHACK) {
//#else
//	if ((nNowTime - m_nLastTime[ZLASTTIME_SYNC_REPORT]) >= 1000/*MATCH_CYCLE_CHECK_SPEEDHACK*/) {
//#endif
//		m_nLastTime[ZLASTTIME_SYNC_REPORT] = nNowTime;
//		int nDataChecksum = 0;
//		if (m_DataChecker.UpdateChecksum() == false) {
//			nDataChecksum = m_DataChecker.GetChecksum();
//			ZGetApplication()->Exit();
//		}
//
//		if (ZCheckHackProcess() == true) {
//			ZPostDisconnect();
//		}
//		ZPOSTCMD2(MC_MATCH_GAME_REPORT_TIMESYNC, MCmdParamUInt(nNowTime), MCmdParamUInt(nDataChecksum));
//	}
//}

// pOwner / pTarget = ½ðÄ³¸¯ÅÍ / ¸ÂÀº Ä³¸¯ÅÍ
void ZGame::CheckCombo(ZCharacter * pOwnerCharacter, ZObject * pHitObject, bool bPlaySound)
{
	// ÀÚ±â°¡ ÀÚ?E¸ÂÃá°Ç Ã¼Å©ÇÏ?E¾ÊÀ½
	if (pOwnerCharacter == pHitObject) return;

	// ³» Ä³¸¯ÅÍ È¤Àº ³»°¡ º¸°úÜÖ´Â Ä³¸¯ÅÍ
	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if (!pTargetCharacter) return;

	if (pTargetCharacter != pOwnerCharacter) return;	// ³»°¡ º¸°úÜÖ´Â Ä³¸¯ÅÍ°¡ ¾Æ´Ï?EÃ¼Å©ÇÏ?E¾ÊÀ½

	if (pHitObject)	// NULL ÀÌ?E¹«Á¶°Ç ¿Ã¸°´Ù
	{
		if (pHitObject->IsDie()) return;		// ½ÃÃ¼¸¦ ½÷µµ Ã¼Å©µÇ?E¾ÊÀ½.
	}

	if (IsPlayerObject(pHitObject))
	{
		// ?E®?E¶§¸°°ÍÀº combo ¿¡ Æ÷ÇÔµÇ?E¾ÊÀ½
		if (m_Match.IsTeamPlay() && (pTargetCharacter->GetTeamID() == ((ZCharacter*)(pHitObject))->GetTeamID()))
			return;

		// Äù½ºÆ®ÀÏ¶§µµ ?E®ÆúÜº Æ÷ÇÔÇÏ?E¾Ê´Â´Ù.
		if (m_Match.IsQuestDrived() || ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_QUEST_CHALLENGE) return;
	}

	UpdateCombo(true);

	if (Z_AUDIO_HITSOUND)
	{
#ifdef _BIRDSOUND
		ZGetSoundEngine()->PlaySound("fx_myhit", 128);
#else
		if (ZGetGame()->GetMatch()->IsPaintBall())
		{
#ifdef _PAINTMODE
			if (bPlaySound)
			if (ZGetSoundEngine()->Get3DSoundUpdate())
			ZGetSoundEngine()->PlaySound("hitbody_paint");
#endif
		}
		else if (ZGetGame()->GetMatch()->IsTraining() && GetRGMain().TrainingSettings.Paint)
		{
			if (bPlaySound)
				if (ZGetSoundEngine()->Get3DSoundUpdate())
					ZGetSoundEngine()->PlaySound("hitbody_paint");
		}
		else
		{
			if (bPlaySound)
			if (ZGetSoundEngine()->Get3DSoundUpdate())
			ZGetSoundEngine()->PlaySound("fx_myhit");
		}

#endif
	}
}
void ZGame::UpdateCombo(bool bShot)
{
	// ³» Ä³¸¯ÅÍ È¤Àº ³»°¡ º¸°úÜÖ´Â Ä³¸¯ÅÍ
	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if (!pTargetCharacter) return;

	// test
	static DWORD nLastShotTime = timeGetTime();
	DWORD nNowTime = timeGetTime();

	pTargetCharacter->GetStatus().CheckCrc();

	if (bShot)
	{
		// Custom: Float Damage work By Desperate
		//if (ZGetConfiguration()->GetEtc()->bhitmaker)
		if (ZGetGame() && ZGetConfiguration()->GetEtc()->bhitmaker)
		{
			if (pTargetCharacter->GetStatus().Ref().nCombo >= 0)
			{
				if (ZGetGame()->m_pMyCharacter->GetItems()->GetSelectedWeaponParts() != MMCIP_MELEE)
					ZGetScreenEffectManager()->AddHitMaker();
				ZGetScreenEffectManager()->AddHit();
				//ZGetGameInterface()->GetCombatInterface()->FloatDamage(pDC, pObject);
#ifdef _FLOATDMG_MODE
				if (ZGetGame() && Z_ETC_FLOATDMG2)
				{
					ZGetGameInterface()->GetCombatInterface()->DmgFloat();
					//ZGetGameInterface()->GetCombatInterface()->FloatDamage(pDC, pObject);
				}
#endif
			}
		}
		if (pTargetCharacter->GetStatus().Ref().nCombo < 2) {
			// hit ÀÌÆåÆ®
			ZGetScreenEffectManager()->AddHit();
			//ZGetGameInterface()->GetCombatInterface()->FloatDamage(pDC, pObject);
#ifdef _FLOATDMG_MODE
			if (ZGetGame() && Z_ETC_FLOATDMG2)
			{
				ZGetGameInterface()->GetCombatInterface()->DmgFloat();
				//ZGetGameInterface()->GetCombatInterface()->FloatDamage(pDC, pObject);
			}
#endif
		}
		if ((nNowTime - nLastShotTime) < 700)
		{
			pTargetCharacter->GetStatus().Ref().nCombo++;
			if (pTargetCharacter->GetStatus().Ref().nCombo > MAX_COMBO)
				pTargetCharacter->GetStatus().Ref().nCombo = 1;
		}
		nLastShotTime = nNowTime;
	}
	else
	{
		if ((pTargetCharacter->GetStatus().Ref().nCombo > 0) && ((nNowTime - nLastShotTime) > 1000))
		{
			pTargetCharacter->GetStatus().Ref().nCombo = 0;
		}
	}

	pTargetCharacter->GetStatus().MakeCrc();
}

void ZGame::CheckStylishAction(ZCharacter * pCharacter)
{
	if (pCharacter->GetStylishShoted())
	{
		if (pCharacter == m_pMyCharacter)
		{
			ZGetScreenEffectManager()->AddCool();
		}
	}
}

#define RESERVED_OBSERVER_TIME	5000

void ZGame::OnReserveObserver()
{
	unsigned long int currentTime = timeGetTime();

	if (currentTime - m_nReservedObserverTime > RESERVED_OBSERVER_TIME)
	{
		if ((m_Match.GetRoundState() == MMATCH_ROUNDSTATE_PLAY) ||
			(m_Match.IsWaitForRoundEnd() && ZGetGameClient()->IsForcedEntry())
			)
		{
			ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
			m_bReserveObserver = false;
		}
		else
		{
			m_bReserveObserver = false;
		}
	}
}

void ZGame::ReserveObserver()
{
	m_bReserveObserver = true;
	m_nReservedObserverTime = timeGetTime();
}

void ZGame::ReleaseObserver()
{
	if (!m_bReplaying.Ref())
	{
		m_bReserveObserver = false;
		ZGetGameInterface()->GetCombatInterface()->SetObserverMode(false);

		FlushObserverCommands();
	}
}

void ZGame::OnInvalidate()
{
	GetWorld()->OnInvalidate();
	ZGetFlashBangEffect()->OnInvalidate();
	m_CharacterManager.OnInvalidate();
}

void ZGame::OnRestore()
{
	GetWorld()->OnRestore();
	ZGetFlashBangEffect()->OnRestore();
	m_CharacterManager.OnRestore();
}

void ZGame::InitRound()
{
	//	m_fTime=0;
	SetSpawnRequested(false);
	ZGetGameInterface()->GetCamera()->StopShock();

	ZGetFlashBangEffect()->End();

	ZGetEffectManager()->Clear();
	m_WeaponManager.Clear();

#ifdef _WORLD_ITEM_
	//ZGetWorldItemManager()->Reset();
#endif

	ZGetCharacterManager()->InitRound();
}

void ZGame::AddEffectRoundState(MMATCH_ROUNDSTATE nRoundState, int nArg)
{
	switch (nRoundState)
	{
	case MMATCH_ROUNDSTATE_COUNTDOWN:
	{
		if ((m_Match.IsWaitForRoundEnd() && m_Match.GetMatchType() != MMATCH_GAMETYPE_DUEL) ||
			m_Match.GetMatchType() == MMATCH_GAMETYPE_SKILLMAP)
		{
			if (m_Match.GetCurrRound() + 1 == m_Match.GetRoundCount())
			{
				ZGetScreenEffectManager()->AddFinalRoundStart();
			}
			else
			{
				if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
				{
					// m_nCurrRound °¡ 0ÀÌ?E1¶ó¿ûÑåÀÌ´Ù.
					ZRuleDuelTournament* pRule = (ZRuleDuelTournament*)m_Match.GetRule();
					int nRoundCount = pRule->GetDuelTournamentPlayCount();
					ZGetScreenEffectManager()->AddRoundStart(nRoundCount);
				}
				else
				{
					// m_nCurrRound °¡ 0ÀÌ?E1¶ó¿ûÑåÀÌ´Ù.
					ZGetScreenEffectManager()->AddRoundStart(m_Match.GetCurrRound() + 1);
				}
			}
		}
		if (m_Match.GetMatchType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
		{
			ZRuleQuestChallenge* pCQRule = (ZRuleQuestChallenge*)m_Match.GetRule();
			if (pCQRule->GetCurrSector() + 1 == pCQRule->GetRoundMax())
			{
				ZGetScreenEffectManager()->AddFinalRoundStart();
			}
			else
			{
				ZGetScreenEffectManager()->AddRoundStart(pCQRule->GetCurrSector() + 1);
			}
		}
	}
	break;
	case MMATCH_ROUNDSTATE_PLAY:
	{
		ZGetCombatInterface()->SetFrozen(false);
		MMATCH_GAMETYPE gameType = ZGetGameClient()->GetMatchStageSetting()->GetGameType();
		if (gameType == MMATCH_GAMETYPE_SPY)
		{
			ZGetScreenEffectManager()->AddScreenEffect("spy_selection");
			if (m_pMyCharacter->GetTeamID() == MMT_RED)
			{
				if (m_pMyCharacter->GetProperty()->nSex == MMS_MALE)
					ZGetGameInterface()->PlayVoiceSound("fx2/MAL01", 1500);
				else if (m_pMyCharacter->GetProperty()->nSex == MMS_FEMALE)
					ZGetGameInterface()->PlayVoiceSound("fx2/FEM01", 1500);
			}
		}
		else if (gameType == MMATCH_GAMETYPE_CTF)
		{
			ZGetGameInterface()->PlayVoiceSound(VOICE_CTF, 1600);
			ZGetScreenEffectManager()->AddScreenEffect("ctf_splash");
		}
		else
		{
			ZGetGameInterface()->PlayVoiceSound(VOICE_LETS_ROCK, 1100);
			ZGetScreenEffectManager()->AddRock();
#ifdef _ANTIHACK
			if (!ZGetApplication()->m_RunThread) ZGetApplication()->RunAntiThread();
#endif
#ifdef _LADDERUPDATE
			if (ZGetGameClient()->IsLadderWarsChannel())
			{
				char szText[256];
				sprintf(szText, ZMsg(MSG_WORD_LADDERCOINN_MSG_START));
				ZChatOutput(ZCOLOR_CHAT_SYSTEM, szText);
			}
#endif
		}
	}
	break;
	case MMATCH_ROUNDSTATE_FINISH:
	{
#ifdef _ROTATION
		if (ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->bTeamAlternateSpawns)
		{
			char szRotationMsg[50];
			sprintf(szRotationMsg, "The base rotation has started!");
			ZGetGameInterface()->GetCombatInterface()->m_Chat.OutputChatMsg(szRotationMsg);
			ZApplication::GetSoundEngine()->PlaySound("Blitzkrieg/EventBenefit");
		}
#endif
		if (m_Match.IsTeamPlay())
		{
			MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
			if (pObjCache && pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide) == false){
#ifdef _HITSCOUNT
				ZCharacter* pCharacterDMG = ZGetGame()->m_pMyCharacter;
				char szName[125];
				char nExtraInfo[128];
				int AimPortentage = 0;
				int Shots = ZGetGame()->m_pMyCharacter->m_nShots;
				int Hits = ZGetGame()->m_pMyCharacter->m_nHits;
				if (Hits > 0 && Shots > 0) {
					AimPortentage = (Hits * 100) / Shots;
				}
				else
				{
					AimPortentage = 0;
				}
#endif
#ifdef _HITSCOUNT
				sprintf(szName, "I have taken %d damage and given %d damage, aim: %d/%d (%d%%).", pCharacterDMG->GetStatus().Ref().nRoundTakenDamage, pCharacterDMG->GetStatus().Ref().nRoundGivenDamage, (int)Shots, (int)Hits, (int)AimPortentage);
#else
				sprintf(szName, "I have taken %d damage and given %d damage.", m_pMyCharacter->GetStatus().Ref().nRoundTakenDamage, m_pMyCharacter->GetStatus().Ref().nRoundGivenDamage);
#endif
				{
					ZChatOutput(szName, ZChat::CMT_SYSTEM);
				}
			}
			if (m_Match.GetMatchType() == MMATCH_GAMETYPE_SKILLMAP)
			{
				ZRuleSkillMap* matchRule = (ZRuleSkillMap*)ZGetGame()->GetMatch()->GetRule();
				if (matchRule->GetChestGrabber() == ZGetMyUID())
					ZGetScreenEffectManager()->AddWin();
				else
					ZGetScreenEffectManager()->AddLose();
			}
#ifdef _ANTIHACK
			if (ZGetApplication()->m_RunThread) ZGetApplication()->KillAntiThread();
#endif

			int nRedTeam, nBlueTeam;
			m_Match.GetTeamAliveCount(&nRedTeam, &nBlueTeam);

			if (nArg == MMATCH_ROUNDRESULT_RED_ALL_OUT || nArg == MMATCH_ROUNDRESULT_BLUE_ALL_OUT)
			{
				ZGetScreenEffectManager()->AddWin();
			}
			else if (nArg == MMATCH_ROUNDRESULT_DRAW)
			{
				// Custom: CTF
				if (ZGetGameTypeManager()->IsTeamExtremeGame(GetMatch()->GetMatchType()))
				{
					MMatchTeam nMyTeam = (MMatchTeam)m_pMyCharacter->GetTeamID();
					MMatchTeam nEnemyTeam = (nMyTeam == MMT_BLUE ? MMT_RED : MMT_BLUE);

					int nMyScore = GetMatch()->GetTeamKills(nMyTeam);
					int nEnemyScore = GetMatch()->GetTeamKills(nEnemyTeam);

					if (nMyScore > nEnemyScore)
						ZGetScreenEffectManager()->AddWin();
					else if (nMyScore < nEnemyScore)
						ZGetScreenEffectManager()->AddLose();
					else
						ZGetScreenEffectManager()->AddDraw();
				}
				else
					ZGetScreenEffectManager()->AddDraw();
			}
			else
			{
				if (nArg == MMATCH_ROUNDRESULT_DRAW)
				{
					ZGetGameInterface()->PlayVoiceSound(VOICE_DRAW_GAME, 1200);
				}
				else {
					MMatchTeam nMyTeam = (MMatchTeam)m_pMyCharacter->GetTeamID();
					MMatchTeam nTeamWon = (nArg == MMATCH_ROUNDRESULT_REDWON ? MMT_RED : MMT_BLUE);

					// ¸¸?E°­Á¦·Î ÆÀÀÌ ¹Ù²¸?E°æ?E¡´?¹Ý?EE
					if (ZGetMyInfo()->GetGameInfo()->bForcedChangeTeam)
					{
						nMyTeam = NegativeTeam(nMyTeam);
					}

					// Spectator ÀÏ°æ?EÃ³¸®
					if (ZGetGameInterface()->GetCombatInterface()->GetObserver()->IsVisible()) {
						ZCharacter* pTarget = ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetTargetCharacter();
						if (pTarget)
							nMyTeam = (MMatchTeam)pTarget->GetTeamID();
					}

					if (nTeamWon == nMyTeam)
						ZGetScreenEffectManager()->AddWin();
					else
						ZGetScreenEffectManager()->AddLose();

					if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY)
					{
						// Spy Mode : no sounds for Win/Lose.
					}
					if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_ASSASSINATE || GetMatch()->GetMatchType() == MMATCH_GAMETYPE_INFECTED)
					{
						if (nTeamWon == MMT_RED)
							ZGetGameInterface()->PlayVoiceSound(VOICE_BLUETEAM_BOSS_DOWN, 2100);
						else
							ZGetGameInterface()->PlayVoiceSound(VOICE_REDTEAM_BOSS_DOWN, 2000);
					}
					else
					{
						if (nTeamWon == MMT_RED)
							ZGetGameInterface()->PlayVoiceSound(VOICE_RED_TEAM_WON, 1400);
						else
							ZGetGameInterface()->PlayVoiceSound(VOICE_BLUE_TEAM_WON, 1400);
					}
				}
			}

			int nTeam = 0;

			// all kill ÆÇÁ¤
			for (int j = 0; j < 2; j++)
			{
				bool bAllKill = true;
				ZCharacter* pAllKillPlayer = NULL;

				for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
					itor != ZGetCharacterManager()->end(); ++itor)
				{
					ZCharacter* pCharacter = (ZCharacter*)(*itor).second;
					if (pCharacter == NULL) return;

					if (j == 0) {
						nTeam = MMT_RED;
					}
					else if (j == 1) {
						nTeam = MMT_BLUE;
					}

					if (pCharacter->GetTeamID() != nTeam)
						continue;

					if (pCharacter->IsDie())
					{
						ZCharacter* pKiller = (ZCharacter*)ZGetCharacterManager()->Find(pCharacter->GetLastAttacker());
						if (pAllKillPlayer == NULL)
						{
							if (!pKiller || pKiller->GetTeamID() == nTeam)			// °°ÀºÆúãÑÅ× Á×¾úÀ¸?E²Î
							{
								bAllKill = false;
								break;
							}

							pAllKillPlayer = pKiller;
						}
						else
							if (pAllKillPlayer != pKiller)	// ¿©·¯¸úÜÌ ³ª´² Á×¿´À¸?E²Î
							{
								bAllKill = false;
								break;
							}
					}
					else
					{
						bAllKill = false;											// ?EÆÀÖ´?³ÑÀÌ ÀÖ¾ûÑµ ²Î
						break;
					}
				}
				// spy mode : no need to show allkill.
				if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY)
					bAllKill = false;

				if ((bAllKill) && (pAllKillPlayer))
				{
					MEMBER_SET_CHECKCRC(pAllKillPlayer->GetStatus(), nAllKill, pAllKillPlayer->GetStatus().Ref().nAllKill + 1);
					pAllKillPlayer->AddIcon(ZCI_ALLKILL);
				}
			}

			/*			if(!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())
							&& !ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGameClient()->GetMatchStageSetting()->GetGameType())
							&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DUEL
							&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DUELTOURNAMENT)
						{
							char szFinishStr[512];

							if (IsReplay())
							{
								ZCharacter* pTarget = ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetTargetCharacter();

								if (pTarget && pTarget->GetInitialized())
								{
									sprintf(szFinishStr, "'%s' dealt %d damage.", pTarget->GetUserName(), pTarget->m_nDamageThisRound);
									ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szFinishStr);
								}
							}
							else if (m_pMyCharacter != NULL && !ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
							{
								sprintf(szFinishStr, "You dealt %d damage.", m_pMyCharacter->m_nDamageThisRound);
								ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szFinishStr);
							}

							if (m_pMyCharacter && !m_pMyCharacter->IsAdminHide())
							{
								int nTotalDamageTeam = 0;
								vector<pair<ZCharacter*, int>> vScores;

								for (ZCharacterManager::iterator itor = m_CharacterManager.begin(); itor != m_CharacterManager.end(); ++itor)
								{
									ZCharacter* pCharacter = (ZCharacter*)(*itor).second;

									if (!pCharacter->GetInitialized()) continue;
									if (pCharacter->IsAdminHide()) continue;

									if(pCharacter->GetTeamID() == ZGetGame()->m_pMyCharacter->GetTeamID())
									{
										nTotalDamageTeam += pCharacter->m_nDamageCaused;
										vScores.push_back(pair<ZCharacter*, int>(pCharacter, pCharacter->m_nDamageCaused));
									}
								}

								sort( vScores.begin(), vScores.end(), CompareZCharInt );

								sprintf(szFinishStr, "Your team dealt %d damage in total.", nTotalDamageTeam);
								ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szFinishStr);

								if (vScores.size() > 0)
								{
									pair<ZCharacter*, int> p = vScores[vScores.size() - 1];
									sprintf(szFinishStr, "Your team's top scorer: %s (%d damage)", p.first->GetUserName(), p.first->m_nDamageThisRound );
									ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szFinishStr);
								}

								vScores.clear();
							}
						}*/
		}

		// µà?E¸ðµåÀÏ °æ?E
		else if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
		{
		MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
		if (pObjCache && pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide) == false) {
		    }
#ifdef _HITSCOUNT
		    ZCharacter* pCharacterDMG = ZGetGame()->m_pMyCharacter;
			char szName[125];
			char nExtraInfo[128];
			int AimPortentage = 0;
			int Shots = ZGetGame()->m_pMyCharacter->m_nShots;
			int Hits = ZGetGame()->m_pMyCharacter->m_nHits;
			if (Hits > 0 && Shots > 0) {
				AimPortentage = (Hits * 100) / Shots;
			}
			else
			{
				AimPortentage = 0;
			}
#endif
#ifdef _HITSCOUNT
			sprintf(szName, "I have taken %d damage and given %d damage, aim: %d/%d (%d%%)", pCharacterDMG->GetStatus().Ref().nRoundTakenDamage, pCharacterDMG->GetStatus().Ref().nRoundGivenDamage, (int)Shots, (int)Hits, (int)AimPortentage);
#else
			sprintf(szName, "I have taken %d damage and given %d damage.", m_pMyCharacter->GetStatus().Ref().nRoundTakenDamage, m_pMyCharacter->GetStatus().Ref().nRoundGivenDamage);
#endif
			ZGetGameClient()->OutputMessage(szName, MZMOM_LOCALREPLY);
			ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
			if (pDuel)
			{
				bool bAddWin = false;
				bool bAddLose = false;
				int nCount = 0;				// Ã¨ÇÇ¾ð?Eµµ?E?¸ðµÎ °ÔÀÓÁßÀÌ¿´´Â?EÃ¼Å©ÇÏ?EÀ§ÇØ¼­...

				// ¿ÉÁ®?E¸ðµåÀÏ¶§
				MUID uidTarget;
				ZObserver* pObserver = ZGetGameInterface()->GetCombatInterface()->GetObserver();
				if (pObserver && pObserver->IsVisible())
					uidTarget = pObserver->GetTargetCharacter()->GetUID();

				// ¿ÉÁ®?E¸ðµå°¡ ¾Æ´Ò¶§
				else
					uidTarget = m_pMyCharacter->GetUID();

				for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); ++itor)
				{
					ZCharacter* pCharacter = (ZCharacter*)(*itor).second;

					// Is champion or challenger
					if ((pCharacter->GetUID() == pDuel->QInfo.m_uidChampion) || (pCharacter->GetUID() == pDuel->QInfo.m_uidChallenger))
					{
						if (uidTarget == pCharacter->GetUID())
						{
							if (pCharacter->IsDie())
								bAddLose |= true;
							else
								bAddWin |= true;
						}
						else
						{
							if (pCharacter->IsDie())
								bAddWin |= true;
							else
								bAddLose |= true;
						}

						// Ã¨ÇÇ¿Â?Eµµ?E?¼ö¸¦ ¸ðµÎ ´õÇØ¼­ 2°¡ µÇ¾ûÚß ÇÑ´Ù
						nCount++;
					}
				}

				// Draw
				if ((nCount < 2) || (bAddWin == bAddLose))
				{
					ZGetScreenEffectManager()->AddDraw();
					ZGetGameInterface()->PlayVoiceSound(VOICE_DRAW_GAME, 1200);
				}

				// Win
				else if (bAddWin)
				{
					ZGetScreenEffectManager()->AddWin();
					ZGetGameInterface()->PlayVoiceSound(VOICE_YOU_WON, 1000);
				}

				// Lose
				else
				{
					ZGetScreenEffectManager()->AddLose();
					ZGetGameInterface()->PlayVoiceSound(VOICE_YOU_LOSE, 1300);
				}
#ifdef _PERFECT
				char szMsg[512];
				if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL || ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
				{
					for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); ++itor)
					{
						ZCharacter* pCharacter = (ZCharacter*)(*itor).second;
						if (bAddWin && IsPlayerHPFull(pCharacter) && IsPlayerAPFull(pCharacter))
						{
							ZGetScreenEffectManager()->Perfect(bAddWin);
							MCOLOR Colorz = MCOLOR(255, 0, 154);
							sprintf(szMsg, "You have made a Perfect!", Colorz);
						}
					}
				}
#endif
			}
		}
		else if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
		{
			if (!ZGetCombatInterface()->GetObserver()->IsVisible())	// ¿ÉÁ®¹ö°¡ ¾Æ´Ï?E
			{
				float fMaxHP = ZGetGame()->m_pMyCharacter->GetMaxHP();
				float fMaxAP = ZGetGame()->m_pMyCharacter->GetMaxAP();

				float fHP = ZGetGame()->m_pMyCharacter->GetHP();
				float fAP = ZGetGame()->m_pMyCharacter->GetAP();

				float fAccumulationDamage = ZGetGame()->m_pMyCharacter->GetAccumulationDamage();

				//ZPostDuelTournamentGamePlayerStatus(ZGetGame()->m_pMyCharacter->GetUID(), (int)(fHP*(100/fMaxHP)), (int)(fAP*(100/fMaxAP))); // ¹éºÐÀ²·Î º¸³»?E
				ZPostDuelTournamentGamePlayerStatus(ZGetGame()->m_pMyCharacter->GetUID(), fAccumulationDamage, fHP, fAP);

#ifndef _PUBLISH	// ³»ºÎºôµå¿¡¼­ ´©?E?EÌÁEÁ¤º¸ Ãâ·Â
				char szAccumulationDamagePrint[256];
				sprintf(szAccumulationDamagePrint, "´©Àû?EÌÁ?%2.1f] ¼­¹ö¿¡ º¸³¿", fAccumulationDamage);
				ZChatOutput(MCOLOR(255, 200, 200), szAccumulationDamagePrint);

#	ifdef _DUELTOURNAMENT_LOG_ENABLE_
				mlog(szAccumulationDamagePrint);
#	endif

#endif
				// ÇÑ ¶ó¿ûÑå°¡ ³¡³ª?E´©?E?EÌÁEÃÊ±âÈ­
				ZGetGame()->m_pMyCharacter->InitAccumulationDamage();
			}
		}
	}
	break;
	};
}

void ZGame::RoundEndDamage()
{
	/*if (m_Match.IsTeamPlay())
	{
		if (!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())
			&& !ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGameClient()->GetMatchStageSetting()->GetGameType())
			&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DUEL
			&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DUELTOURNAMENT)
		{
			char szFinishStr[512];

			if (IsReplay())
			{
				ZCharacter* pTarget = ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetTargetCharacter();

				if (pTarget && pTarget->GetInitialized())
				{
					sprintf(szFinishStr, "'%s' dealt %.1f damage.", pTarget->GetUserName(), pTarget->m_fDamageCaused);
					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szFinishStr);
				}
			}
			else if (m_pMyCharacter != NULL && !ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
			{
				sprintf(szFinishStr, "You dealt %.1f damage.", m_pMyCharacter->fPersonalDamageDealtCounter);
				ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szFinishStr);
			}

			if (m_pMyCharacter && !m_pMyCharacter->IsAdminHide())
			{
				float fTotalDamageTeam = 0.f;

				vector<pair<ZCharacter*, float> > vScores;

				for (ZCharacterManager::iterator itor = m_CharacterManager.begin(); itor != m_CharacterManager.end(); ++itor)
				{
					ZCharacter* pCharacter = (ZCharacter*)(*itor).second;

					if (!pCharacter->GetInitialized()) continue;
					if (pCharacter->IsAdminHide()) continue;

					if(pCharacter->GetTeamID() == ZGetGame()->m_pMyCharacter->GetTeamID())
					{
						fTotalDamageTeam += pCharacter->m_fDamageCaused;
						vScores.push_back(pair<ZCharacter*, float>(pCharacter, pCharacter->m_fDamageCaused));
					}
				}

				sort( vScores.begin(), vScores.end(), CompareZCharFloat );

				sprintf(szFinishStr, "Your team dealt %.1f damage in total.", fTotalDamageTeam);
				ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szFinishStr);

				if (vScores.size() > 0)
				{
					pair<ZCharacter*, float> p = vScores[vScores.size() - 1];
					sprintf(szFinishStr, "Your team's top scorer: %s (%.1f damage)", p.first->GetUserName(), p.second );
					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szFinishStr);
				}

				vScores.clear();
			}
		}
	}*/
}

void ZGame::StartRecording()
{
	int nsscount = 0;

	char replayfilename[_MAX_PATH];
	char replayfilenameSafe[_MAX_PATH];
	char replayfoldername[_MAX_PATH];

	TCHAR szPath[MAX_PATH];
	if (GetMyDocumentsPath(szPath)) {
		strcpy(replayfoldername, szPath);
		strcat(replayfoldername, GUNZ_FOLDER);
		CreatePath(replayfoldername);
		strcat(replayfoldername, REPLAY_FOLDER);
		CreatePath(replayfoldername);
	}

	/*do {
		sprintf(replayfilename,"%s/Gunz%03d."GUNZ_REC_FILE_EXT , replayfoldername , nsscount);
		m_nGunzReplayNumber = nsscount;
		nsscount++;
	}
	while( IsExist(replayfilename) && nsscount<1000);

	if(nsscount==1000) goto RECORDING_FAIL;*/
	// ÆÄÀÏ¸úÜ» ÀÏ·Ã¹øÈ£ ¹æ½Ä¿¡¼­ °ÔÀÓÁ¤º¸±âÀÔ ¹æ½ÄÀ¸·Î ?E?
	SYSTEMTIME t;
	GetLocalTime(&t);
	char szCharName[MATCHOBJECT_NAME_LENGTH];
	ValidateFilename(szCharName, ZGetMyInfo()->GetCharName(), '_');

	const char* szGameTypeAcronym = "";
	char szValidatedOppoClanName[32] = "";
	int nCurrentRound = 0;
	//const char* szMapName = GetMatch()->GetMapName() ? GetMatch()->GetMapName() : "";

	bool bClanGame = ZGetGameClient()->IsCWChannel();

	REPLAY_STAGE_SETTING_NODE stageSettingNode;

	if (GetMatch()) {
		// °ÔÀÓ ¸ð?E¾àÀÚ Ãâ·Â
		if (bClanGame) szGameTypeAcronym = "CLAN_";
		else szGameTypeAcronym = MMatchGameTypeAcronym[GetMatch()->GetMatchType()];

		if (GetMatch()->IsTeamPlay()) nCurrentRound = GetMatch()->GetCurrRound() + 1;

		// Å¬·£?E?°æ?E»ó?EÅ¬·£?E¾Ë¾Æ³¿
		if (bClanGame) {
			const char* szOppositeClanName = "";

			if (0 == strcmp(ZGetMyInfo()->GetClanName(), ZGetCombatInterface()->GetRedClanName()))
				szOppositeClanName = ZGetCombatInterface()->GetBlueClanName();
			else
				szOppositeClanName = ZGetCombatInterface()->GetRedClanName();

			ValidateFilename(szValidatedOppoClanName, szOppositeClanName, '_');
		}
	}

	if (GetMatch()->IsTeamPlay())
	{
		// Custom: Making recordings more readable? New format (TeamMatches): GameType[Round#]_CharName_Year-Month-Day_Hour-Min-Second
		sprintf(replayfilename, "%s[%03d]_%s_%4d-%02d-%02d_%02d-%02d-%02d%s%s",
			szGameTypeAcronym, nCurrentRound, szCharName, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond,
			bClanGame ? "_" : "", szValidatedOppoClanName);
	}
	else
	{
		// Custom: Making recordings more readable? New format: GameType_CharName_Year-Month-Day_Hour-Min-Second
		sprintf_s(replayfilename, "%s_%s_%4d-%02d-%02d_%02d-%02d-%02d%s%s",
			szGameTypeAcronym, szCharName, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond,
			bClanGame ? "_" : "", szValidatedOppoClanName);
	}

	//sprintf(replayfilename, "%s_%s_%4d%02d%02d_%02d%02d%02d%s%s",
	//	szGameTypeAcronym, szCharName, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond,
	//	bClanGame ? "_" : "", szValidatedOppoClanName);

	sprintf(replayfilenameSafe, "%s_nocharname_%4d%02d%02d_%02d%02d%02d",
		szGameTypeAcronym, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);// Ä³¸¯¸ú?EÅ¬·£?E?E¹®Á¦°¡ µÉ ?EÀÖ´Â ¹®ÀÚ¿­À» »ý·«ÇÑ ¹ö?E

	char szFullPath[_MAX_PATH];

	strcpy(m_szReplayFileName, replayfilename);
	sprintf(szFullPath, "%s/%s." GUNZ_REC_FILE_EXT, replayfoldername, replayfilename);
	m_pReplayFile = zfopen(szFullPath, true);
	if (!m_pReplayFile)
	{
		strcpy(m_szReplayFileName, replayfilenameSafe);
		sprintf(szFullPath, "%s/%s." GUNZ_REC_FILE_EXT, replayfoldername, replayfilenameSafe);	// ÆÄÀÏ¸úÒ§¹®ÀÏ ?EÀÖÀ¸´Ï ÀÌ¸§À» ´Ü¼øÈ­ÇØ¼­ Àç½Ãµµ
		m_pReplayFile = zfopen(szFullPath, true);

		if (!m_pReplayFile) goto RECORDING_FAIL;
	}

	int nWritten;

	DWORD header;
	header = GUNZ_REC_FILE_ID;
	nWritten = zfwrite(&header, sizeof(header), 1, m_pReplayFile);
	if (nWritten == 0) goto RECORDING_FAIL;

	header = GUNZ_REC_FILE_VERSION;
	nWritten = zfwrite(&header, sizeof(header), 1, m_pReplayFile);
	if (nWritten == 0) goto RECORDING_FAIL;

	ConvertStageSettingNodeForRecord(ZGetGameClient()->GetMatchStageSetting()->GetStageSetting(), &stageSettingNode);
	strcpy(stageSettingNode.szStageName, ZGetGameClient()->GetStageName());

	nWritten = zfwrite(&stageSettingNode, sizeof(REPLAY_STAGE_SETTING_NODE), 1, m_pReplayFile);
	if (nWritten == 0) goto RECORDING_FAIL;

	// °ÔÀÓ?Eº° Ãß°¡ÀûÀÎ ½ºÅ×ÀÌ?E¼¼ÆÃ°ª Àú?E
	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUEL)
	{
		ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		nWritten = zfwrite(&pDuel->QInfo, sizeof(MTD_DuelQueueInfo), 1, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;
	}
	else if (IsGameRuleCTF(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		ZRuleTeamCTF* pTeamCTF = (ZRuleTeamCTF*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();

		MTD_CTFReplayInfo info;
		info.uidCarrierRed = pTeamCTF->GetRedCarrier();
		info.uidCarrierBlue = pTeamCTF->GetBlueCarrier();
		info.posFlagRed[0] = pTeamCTF->GetRedFlagPos().x;
		info.posFlagRed[1] = pTeamCTF->GetRedFlagPos().y;
		info.posFlagRed[2] = pTeamCTF->GetRedFlagPos().z;
		info.posFlagBlue[0] = pTeamCTF->GetBlueFlagPos().x;
		info.posFlagBlue[1] = pTeamCTF->GetBlueFlagPos().y;
		info.posFlagBlue[2] = pTeamCTF->GetBlueFlagPos().z;
		info.nFlagStateRed = pTeamCTF->GetRedFlagState();
		info.nFlagStateBlue = pTeamCTF->GetBlueFlagState();

		nWritten = zfwrite(&info, sizeof(MTD_CTFReplayInfo), 1, m_pReplayFile);

		//nWritten = zfwrite(&pTeamCTF->GetRedCarrier(),sizeof(MUID),1,m_pReplayFile);
		//nWritten = zfwrite(&pTeamCTF->GetBlueCarrier(),sizeof(MUID),1,m_pReplayFile);
		//nWritten = zfwrite(&pTeamCTF->GetRedFlagPos(),sizeof(rvector),1,m_pReplayFile);
		//nWritten = zfwrite(&pTeamCTF->GetBlueFlagPos(),sizeof(rvector),1,m_pReplayFile);
		//int nRedFlagState = (int)pTeamCTF->GetRedFlagState();
		//int nBlueFlagState = (int)pTeamCTF->GetBlueFlagState();
		//nWritten = zfwrite(&nRedFlagState,sizeof(int),1,m_pReplayFile);
		//nWritten = zfwrite(&nBlueFlagState,sizeof(int),1,m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_INFECTED)
	{
		ZRuleTeamInfected* pTeamInfected = (ZRuleTeamInfected*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		nWritten = zfwrite(&pTeamInfected->m_uidPatientZero, sizeof(MUID), 1, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_GUNGAME)
	{
		// TODO: WRITE all characters gungameweapondata
		ZRuleGunGame* pRuleGG = (ZRuleGunGame*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		nWritten = zfwrite(&pRuleGG->m_nWeaponMaxLevel, sizeof(int), 1, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;

		int nCharacterCount = (int)m_CharacterManager.size();
		nWritten = zfwrite(&nCharacterCount, sizeof(nCharacterCount), 1, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;

		for (ZCharacterManager::iterator itor = m_CharacterManager.begin(); itor != m_CharacterManager.end(); ++itor)
		{
			ZCharacter* pCharacter = (ZCharacter*)(*itor).second;

			MTD_GunGameWeaponInfo rd;
			rd.uidPlayer = pCharacter->GetUID();
			memcpy(rd.nWeaponID, pCharacter->m_nGunGameWeaponID, sizeof(rd.nWeaponID));
			rd.nWeaponLevel = pCharacter->m_nGunGameWeaponLevel;

			nWritten = zfwrite((void*)&rd, sizeof(MTD_GunGameWeaponInfo), 1, m_pReplayFile);
			if (nWritten == 0) goto RECORDING_FAIL;
		}
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
	{
		int nType = (int)ZGetGameInterface()->GetDuelTournamentType();
		nWritten = zfwrite(&nType, sizeof(int), 1, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;

		const vector<DTPlayerInfo>& vecDTPlayerInfo = ZGetGameInterface()->GetVectorDTPlayerInfo();

		int nCount = (int)vecDTPlayerInfo.size();
		nWritten = zfwrite(&nCount, sizeof(int), 1, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;

		nWritten = zfwrite((void*)&vecDTPlayerInfo[0], sizeof(DTPlayerInfo), nCount, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;

		ZRuleDuelTournament* pRule = (ZRuleDuelTournament*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		nWritten = zfwrite((void*)&pRule->m_DTGameInfo, sizeof(MTD_DuelTournamentGameInfo), 1, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_SPY)
	{
		ZRuleSpy* pSpy = (ZRuleSpy*)m_Match.GetRule();

		int nSpyItemCount = (int)pSpy->m_vtLastSpyItem.size();
		nWritten = zfwrite(&nSpyItemCount, sizeof(int), 1, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;

		nWritten = zfwrite((void*)&pSpy->m_vtLastSpyItem[0], sizeof(MMatchSpyItem), nSpyItemCount, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;

		int nTrackerItemCount = (int)pSpy->m_vtLastTrackerItem.size();
		nWritten = zfwrite(&nTrackerItemCount, sizeof(int), 1, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;

		nWritten = zfwrite((void*)&pSpy->m_vtLastTrackerItem[0], sizeof(MMatchSpyItem), nTrackerItemCount, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;
	}
	int nCharacterCount = (int)m_CharacterManager.size();
	nWritten = zfwrite(&nCharacterCount, sizeof(nCharacterCount), 1, m_pReplayFile);
	if (nWritten == 0) goto RECORDING_FAIL;

	for (ZCharacterManager::iterator itor = m_CharacterManager.begin(); itor != m_CharacterManager.end(); ++itor)
	{
		ZCharacter* pCharacter = (ZCharacter*)(*itor).second;
		if (!pCharacter->Save(m_pReplayFile)) goto RECORDING_FAIL;
	}

	//// Custom: CQ
	//for (auto itor = ZGetObjectManager()->begin(); itor != ZGetObjectManager()->end(); ++itor)
	//{
	//	ZActorWithFSM* pObj = (ZActorWithFSM*)(*itor).second;
	//	if (!pObj->Save(m_pReplayFile)) goto RECORDING_FAIL;

	//}

	//nWritten = zfwrite(&m_fTime,sizeof(m_fTime),1,m_pReplayFile);
	float fTime = m_fTime.Ref();
	nWritten = zfwrite(&fTime, sizeof(float), 1, m_pReplayFile);
	if (nWritten == 0) goto RECORDING_FAIL;

	m_bRecording = true;
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM),
		ZMsg(MSG_RECORD_STARTING));
	return;

RECORDING_FAIL:	// ½ÇÆÐ

	if (m_pReplayFile)
	{
		zfclose(m_pReplayFile);
		m_pReplayFile = NULL;
	}

	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), ZMsg(MSG_RECORD_CANT_SAVE));
}
void ZGame::StopRecording()
{
	if (!m_bRecording) return;

	bool bError = false;

	m_bRecording = false;

	ZObserverCommandList::iterator itr = m_ReplayCommandList.begin();
	for (size_t i = 0; i < m_ReplayCommandList.size(); i++)
	{
		ZObserverCommandItem* pItem = *itr;
		MCommand* pCommand = pItem->pCommand;

		const int BUF_SIZE = 1024;
		char CommandBuffer[BUF_SIZE];
		int nSize = pCommand->GetData(CommandBuffer, BUF_SIZE);

		int nWritten;
		nWritten = zfwrite(&pItem->fTime, sizeof(pItem->fTime), 1, m_pReplayFile);
		if (nWritten == 0) { bError = true; break; }
		nWritten = zfwrite(&pCommand->m_Sender, sizeof(pCommand->m_Sender), 1, m_pReplayFile);
		if (nWritten == 0) { bError = true; break; }
		nWritten = zfwrite(&nSize, sizeof(nSize), 1, m_pReplayFile);
		if (nWritten == 0) { bError = true; break; }
		nWritten = zfwrite(CommandBuffer, nSize, 1, m_pReplayFile);
		if (nWritten == 0) { bError = true; break; }

		itr++;
	}

	while (m_ReplayCommandList.size())
	{
		ZObserverCommandItem* pItem = *m_ReplayCommandList.begin();
		delete pItem->pCommand;
		delete pItem;
		m_ReplayCommandList.pop_front();
	}

	if (!zfclose(m_pReplayFile))
		bError = true;

	if (bError)
	{
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), ZMsg(MSG_RECORD_CANT_SAVE));
	}
	else
	{
		char szOutputFilename[256];
		sprintf(szOutputFilename, GUNZ_FOLDER REPLAY_FOLDER "/%s." GUNZ_REC_FILE_EXT, m_szReplayFileName);

		char szOutput[256];
		// ZTranslateMessage(szOutput,MSG_RECORD_SAVED,1,szOutputFilename);
		ZTransMsg(szOutput, MSG_RECORD_SAVED, 1, szOutputFilename);
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szOutput);
	}
}

void ZGame::ToggleRecording()
{
	if (m_bReplaying.Ref()) return;	// Àç»ýÁß ?E­ºÒ°¡ -_-;

	// Äù½ºÆ®´Â ?E­µÇ?E¾Ê´Â´Ù
	// Custom: Add CQ to this
	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())
		|| ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
		return;

	if (!m_bRecording)
		StartRecording();
	else
		StopRecording();
}

DWORD dwReplayStartTime;

bool ZGame::OnLoadReplay(ZReplayLoader * pLoader)
{
	//m_fTime = pLoader->GetGameTime();
	m_fTime.Set_CheckCrc(pLoader->GetGameTime());

	m_bReplaying.Set_CheckCrc(true);
	SetReadyState(ZGAME_READYSTATE_RUN);
	GetMatch()->SetRoundState(MMATCH_ROUNDSTATE_FREE);
	ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
	//Esto elimina la repetición cuando el objetivo es un administrador que se esconde como administrador.
	//ZGetGameInterface()->GetCombatInterface()->GetObserver()->SetTarget(ZGetGame()->m_pMyCharacter->GetUID());
	g_bProfile = true;
	dwReplayStartTime = timeGetTime();

	/*
		size_t n;

		m_bReplaying=true;

		int nCharacterCount;
		zfread(&nCharacterCount,sizeof(nCharacterCount),1,file);

		ZGetCharacterManager()->Clear();
		m_ObjectManager.Clear();

		for(int i=0;i<nCharacterCount;i++)
		{
			bool bHero;
			n=zfread(&bHero,sizeof(bHero),1,file);
			if(n!=1) return false;

			MTD_CharInfo info;

			if(nVersion<2) {
				n=zfread(&info,sizeof(info)-4,1,file);
				if(n!=1) return false;
				info.nClanCLID = 0;
			}
			else {
				n=zfread(&info,sizeof(info),1,file);
				if(n!=1) return false;
			}

			ZCharacter *pChar=NULL;
			if(bHero)
			{
				m_pMyCharacter=new ZMyCharacter;
				CreateMyCharacter(&info);
				pChar=m_pMyCharacter;
				pChar->Load(file,nVersion);
			}else
			{
				pChar=new ZNetCharacter;
				pChar->Load(file,nVersion);
				pChar->Create(&info);
			}

			ZGetCharacterManager()->Add(pChar);
			mlog("%s : %d %d\n",pChar->GetProperty()->szName,pChar->GetUID().High,pChar->GetUID().Low);

			pChar->SetVisible(true);
		}

		float fGameTime;
		zfread(&fGameTime,sizeof(fGameTime),1,file);
		m_fTime=fGameTime;

		int nCommandCount=0;

		int nSize;
		float fTime;
		while( zfread(&fTime,sizeof(fTime),1,file) )
		{
			nCommandCount++;

			char CommandBuffer[1024];

			MUID uidSender;
			zfread(&uidSender,sizeof(uidSender),1,file);
			zfread(&nSize,sizeof(nSize),1,file);
			if(nSize<0 || nSize>sizeof(CommandBuffer)) {
				m_bReplaying=false;
				ShowReplayInfo( true);
				return false;
			}
			zfread(CommandBuffer,nSize,1,file);

			ZObserverCommandItem *pZCommand=new ZObserverCommandItem;
			pZCommand->pCommand=new MCommand;
			pZCommand->pCommand->SetData(CommandBuffer,ZGetGameClient()->GetCommandManager());
			pZCommand->pCommand->m_Sender=uidSender;
			pZCommand->fTime=fTime;
			m_ReplayCommandList.push_back(pZCommand);
		}

		SetReadyState(ZGAME_READYSTATE_RUN);
		GetMatch()->SetRoundState(MMATCH_ROUNDSTATE_FREE);
		ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);

		ZGetGameInterface()->GetCombatInterface()->GetObserver()->SetTarget(m_pMyCharacter->GetUID());

		g_bProfile=true;

		dwReplayStartTime=timeGetTime();

		return true;
	*/
	return true;
}

void ZGame::EndReplay()
{
	g_bProfile = false;

	DWORD dwReplayEndTime = timeGetTime();

	mlog("replay end. profile saved. playtime = %3.3f seconds , average fps = %3.3f \n",
		float(dwReplayEndTime - dwReplayStartTime) / 1000.f,
		1000.f * g_nFrameCount / float(dwReplayEndTime - dwReplayStartTime));

	// ¸®ÇÃ·¹ÀÌ°¡ ´Ù ³¡³ª?E´Ù½Ã Ã³À½ºÎÅÍ µ¹·Áº¸ÀÚ. - (¹ö?E
	if (ZGetGameInterface()->GetPreviousState() == GUNZ_LOGIN || !ZGetGameClient()->IsConnected())
	{
		ZChangeGameState(GUNZ_LOGIN);
	}
	else
		ZChangeGameState(GUNZ_LOBBY);
}

void ZGame::ConfigureCharacter(const MUID & uidChar, MMatchTeam nTeam, unsigned char nPlayerFlags)
{
	ZCharacterManager* pCharMgr = ZGetCharacterManager();
	ZCharacter* pChar = (ZCharacter*)pCharMgr->Find(uidChar);
	if (pChar == NULL) return;

	pChar->SetAdminHide((nPlayerFlags & MTD_PlayerFlags_AdminHide) != 0);
	pChar->SetTeamID(nTeam);
	pChar->InitStatus();
	pChar->InitRound();

	ZGetCombatInterface()->OnAddCharacter(pChar);
}

void ZGame::RefreshCharacters()
{
	for (MMatchPeerInfoList::iterator itor = ZGetGameClient()->GetPeers()->begin();
		itor != ZGetGameClient()->GetPeers()->end(); ++itor)
	{
		MMatchPeerInfo* pPeerInfo = (*itor).second;
		ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(pPeerInfo->uidChar);

		if (pCharacter == NULL) {
			pCharacter = m_CharacterManager.Add(pPeerInfo->uidChar, rvector(0.0f, 0.0f, 0.0f));
			pCharacter->Create(&pPeerInfo->CharInfo);

			if (m_Match.GetRoundState() == MMATCH_ROUNDSTATE_PREPARE)
			{
				if (m_Match.IsTeamPlay())
				{
					//					pCharacter->SetVisible(true);		// RAONHAJE: PeerOpened TEST
				}
			}

			/*
			// TODO: AGENT ?E?peerµé¿¡°Ô´Â ¾ÃÈù´Ù. ¼öÁ¤¿ä¸Á.

			//½ÅÀÔ Ä³¸¯ÅÍ¿¡°Ô ÀÚ½ÅÀÇ ¹«±â¸¦ ¾Ë¸°´Ù...

			ZCharacter* pMyCharacter = g_pGame->m_pMyCharacter;
			if(pMyCharacter)
			{
				//			if(pMyCharacter != pCharacter) { // ÀÚ½ÅÀÌ »õ·Î »ý?EÄ³¸¯ÀÌ ¾Æ´Ï¶ó?E
				int nParts = g_pGame->m_pMyCharacter->GetItems()->GetSelectedWeaponParts();
				ZPostChangeWeapon(nParts);
				//			}
			}
			*/
		}
	}
}

void ZGame::DeleteCharacter(const MUID & uid)
{
	bool bObserverDel = false;
	ZCharacter* pCharacter = (ZCharacter*)ZGetCharacterManager()->Find(uid);

	// ¿ÉÁ®?EÅ¸°ÙÀÎ °æ?E´Ù¸¥ Å¸°ÙÀ¸·Î ¹Ù²ãÁØ´Ù.
	ZObserver* pObserver = ZGetGameInterface()->GetCombatInterface()->GetObserver();
	if (pObserver->IsVisible())
	{
		if ((pCharacter != NULL) && (pCharacter == pObserver->GetTargetCharacter()))
		{
			bObserverDel = true;
		}
	}

	m_CharacterManager.Delete(uid);

	if (bObserverDel)
	{
		if (pObserver) pObserver->SetFirstTarget();
	}
}

void ZGame::OnStageEnterBattle(MCmdEnterBattleParam nParam, MTD_PeerListNode * pPeerNode)
{
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) return;

	MUID uidChar = pPeerNode->uidChar;

	if (uidChar == ZGetMyUID())		// enterÇÑ»ç¶÷ÀÌ ³ªÀÚ½ÅÀÏ °æ?E
	{
#ifdef _ZPEERANTIHACK
		ZPeerAntiHack.clear();
#endif
#ifdef _ANTIHACK
		ZGetApplication()->RunAntiThread();
#endif
		if (ZGetGame()->CreateMyCharacter(&pPeerNode->CharInfo/*, &pPeerNode->CharBuffInfo*/) == true)
		{
			ConfigureCharacter(uidChar, (MMatchTeam)pPeerNode->ExtendInfo.nTeam, pPeerNode->ExtendInfo.nPlayerFlags);	// Player Character Æ÷ÇÔ
		}
	}
	else							// enterÇÑ»ç¶÷ÀÌ ³ª ÀÚ½ÅÀÌ ¾Æ´Ò°æ?E
	{
		if (ZGetGameClient()->m_bAdminNAT)
			OnAddPeer(pPeerNode->uidChar, 0, 0, pPeerNode);
		else
			OnAddPeer(pPeerNode->uidChar, pPeerNode->dwIP, pPeerNode->nPort, pPeerNode);
	}

	if (nParam == MCEP_FORCED)
	{
		ZCharacter* pChar = (ZCharacter*)ZGetCharacterManager()->Find(uidChar);
		GetMatch()->OnForcedEntry(pChar);

		char temp[256] = "";
		if ((pPeerNode->ExtendInfo.nPlayerFlags & MTD_PlayerFlags_AdminHide) == 0) 
		{
			ZTransMsg(temp, MSG_GAME_JOIN_BATTLE, 1, pChar->GetUserAndClanName());
			ZChatOutput(MCOLOR(ZCOLOR_GAME_INFO), temp);
		}
#ifdef _REPLAY_TEST_LOG
		mlog("[Add Character %s(%d)]\n", pChar->GetCharInfo()->szName, uidChar.Low);
#endif
	}

	ZGetGameClient()->OnStageEnterBattle(uidChar, nParam);
}

void ZGame::OnStageLeaveBattle(const MUID & uidChar, const bool bIsRelayMap)//, const MUID& uidStage)
{
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) return;

	// Custom: Prevents recordings from not getting saved if you quit a room.
	if (ZGetGame()->m_bRecording && uidChar == ZGetMyUID()) ZGetGame()->ToggleRecording();
#ifdef _ANTIHACK
	if (uidChar == ZGetMyUID())
	{
		ZGetApplication()->KillAntiThread();
	}
#endif
	if (uidChar != ZGetMyUID()) {
		ZCharacter* pChar = (ZCharacter*)ZGetCharacterManager()->Find(uidChar);

		if (pChar && !pChar->IsAdminHide() && !bIsRelayMap) {
			char temp[256] = "";
			ZTransMsg(temp, MSG_GAME_LEAVE_BATTLE, 1, pChar->GetUserAndClanName());
			ZChatOutput(MCOLOR(ZCOLOR_GAME_INFO), temp);
		}

		ZGetGameClient()->DeletePeer(uidChar);
		if (ZApplication::GetGameInterface()->GetState() == GUNZ_GAME) {
			DeleteCharacter(uidChar);
		}

		ZGetGameClient()->SetVoteInProgress(false);
		ZGetGameClient()->SetCanVote(false);
	}
}

void ZGame::OnAddPeer(const MUID & uidChar, DWORD dwIP, const int nPort, MTD_PeerListNode * pNode)
{
	if ((ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) || (ZGetGame() == NULL)) return;

	/*
	//// UDPTEST LOG ////////////////////////////////
	char szLog[256];
	sprintf(szLog, "[%d:%d] ADDPEER: Char(%d:%d) IP:%s, Port:%d \n",
	GetPlayerUID().High, GetPlayerUID().Low, uidChar.High, uidChar.Low, szIP, nPort);
	mlog(szLog);
	/////////////////////////////////////////////////
	*/

	// Ãß°¡µÈ »ç¶÷ÀÌ ÀÚ?EÀÚ½ÅÀÌ ¾Æ´Ï?E
	if (uidChar != ZGetMyUID())
	{
		if (pNode == NULL) {
			_ASSERT(0);
		}

		ZGetGameClient()->DeletePeer(uidChar);	// Delete exist info

		MMatchPeerInfo* pNewPeerInfo = new MMatchPeerInfo;

		if (uidChar == MUID(0, 0))	pNewPeerInfo->uidChar = MUID(0, nPort);	// ·ÎÄÃÅ×½ºÆ®¸¦ À§ÇØ¼­
		else						pNewPeerInfo->uidChar = uidChar;

		in_addr addr;
		addr.s_addr = dwIP;
		char* pszIP = inet_ntoa(addr);
		strcpy(pNewPeerInfo->szIP, pszIP);

		pNewPeerInfo->dwIP = dwIP;
		pNewPeerInfo->nPort = nPort;

		if (!IsReplay())
			memcpy(&pNewPeerInfo->CharInfo, &(pNode->CharInfo), sizeof(MTD_CharInfo));
		else
		{
			MTD_CharInfo currInfo;
			ConvertCharInfo(&currInfo, &pNode->CharInfo, ZReplayLoader::m_nVersion);
			memcpy(&pNewPeerInfo->CharInfo, &currInfo, sizeof(MTD_CharInfo));
		}
		//¹öÇÁÁ¤º¸ÀÓ½ÃÁÖ¼® memcpy(&pNewPeerInfo->CharBuffInfo, &(pNode->CharBuffInfo), sizeof(MTD_CharBuffInfo));
		memcpy(&pNewPeerInfo->ExtendInfo, &(pNode->ExtendInfo), sizeof(MTD_ExtendInfo));

		ZGetGameClient()->AddPeer(pNewPeerInfo);

		RefreshCharacters();
	}

	ConfigureCharacter(uidChar, (MMatchTeam)pNode->ExtendInfo.nTeam, pNode->ExtendInfo.nPlayerFlags);	// Player Character Æ÷ÇÔ
}

void ZGame::OnPeerList(const MUID & uidStage, void* pBlob, int nCount)
{
	if (ZGetGameClient()->GetStageUID() != uidStage) return;
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) return;
	if ((ZGetGame() == NULL) || (ZGetCharacterManager() == NULL)) return;

	for (int i = 0; i < nCount; i++) {
		MTD_PeerListNode* pNode = (MTD_PeerListNode*)MGetBlobArrayElement(pBlob, i);
		OnAddPeer(pNode->uidChar, pNode->dwIP, pNode->nPort, pNode);

		ZCharacter* pChar = (ZCharacter*)ZGetCharacterManager()->Find(pNode->uidChar);
		if (pChar) {
			pChar->SetVisible(false);
		}
	}
}

void ZGame::PostMyBuffInfo()
{
	// ³»°Ô ¹ßµ¿µÇ?EÀÖ´Â ¹öÇÁ »óÅÂ¸¦ ÇÇ¾ûÑé¿¡°Ô ¾Ë·ÁÁØ´Ù
	if (m_pMyCharacter)
	{
		void* pBlob = m_pMyCharacter->MakeBuffEffectBlob();
		if (pBlob)
		{
			ZPostBuffInfo(pBlob);
			MEraseBlobArray(pBlob);
		}
	}
}

void ZGame::OnPeerBuffInfo(const MUID & uidSender, void* pBlobBuffInfo)
{
	if (uidSender == ZGetMyUID()) return;

	ZCharacter* pSender = (ZCharacter*)ZGetCharacterManager()->Find(uidSender);
	if (!pSender) return;
	if (!pBlobBuffInfo) return;

	MTD_BuffInfo* pBuffInfo = NULL;
	int numElem = MGetBlobArrayCount(pBlobBuffInfo);

	// Custom: Exploit fix (MTD_PeerBuffInfo blob overflow)
	if (MGetBlobArraySize(pBlobBuffInfo) != (8 + (sizeof(MTD_BuffInfo) * numElem)))
	{
		return;
	}

	for (int i = 0; i < numElem; ++i)
	{
		pBuffInfo = (MTD_BuffInfo*)MGetBlobArrayElement(pBlobBuffInfo, i);

		ApplyPotion(pBuffInfo->nItemId, pSender, (float)pBuffInfo->nRemainedTime);
	}
}

void ZGame::OnGameRoundState(const MUID & uidStage, int nRound, int nRoundState, int nArg)
{
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) return;
	ZMatch* pMatch = GetMatch();
	if (pMatch == NULL) return;

	MMATCH_ROUNDSTATE RoundState = MMATCH_ROUNDSTATE(nRoundState);

	// ÇÁ¸®¸ðµåÀÏ°æ?E
	if ((RoundState == MMATCH_ROUNDSTATE_FREE) && (pMatch->GetRoundState() != RoundState))
	{
		pMatch->InitCharactersPosition();
		m_pMyCharacter->SetVisible(true);
		m_pMyCharacter->Revival();
		ReleaseObserver();
	}

	pMatch->SetRound(nRound);
	pMatch->SetRoundState(RoundState, nArg);
	AddEffectRoundState(RoundState, nArg);

	if (RoundState == MMATCH_ROUNDSTATE_FINISH)
	{
#ifdef _VMPROTECT
		VMProtectBeginUltra("RoundStateFinishValidImage");
		if (!VMProtectIsValidImageCRC())
		{
			ZPostBanMe(WHT_MEMCMP);
			if (!g_ShouldBanPlayer)
				g_ShouldBanPlayer = timeGetTime();
}
		VMProtectEnd();
#endif
}
}

bool ZGame::FilterDelayedCommand(MCommand * pCommand)
{
	bool bFiltered = true;
	float fDelayTime = 0;

	MUID uid = pCommand->GetSenderUID();
	ZCharacter* pChar = (ZCharacter*)ZGetCharacterManager()->Find(uid);
	if (!pChar) return false;

	switch (pCommand->GetID())
	{
	case MC_PEER_SKILL:
	{
		int nSkill;
		pCommand->GetParameter(&nSkill, 0, MPT_INT);
		fDelayTime = .15f;
		switch (nSkill) {
		case ZC_SKILL_UPPERCUT:
			if (pChar != m_pMyCharacter) pChar->SetAnimationLower(ZC_STATE_LOWER_UPPERCUT);
			break;
		case ZC_SKILL_SPLASHSHOT: break;
		case ZC_SKILL_DASH: break;
		}

		////////////////////////////////////////////////////////////////////
		int sel_type;
		pCommand->GetParameter(&sel_type, 2, MPT_INT);
		MMatchCharItemParts parts = (MMatchCharItemParts)sel_type;
		if (parts != pChar->GetItems()->GetSelectedWeaponParts()) {
			// Áö±Ý µé?EÀÖ´Â ¹«±â¿Í º¸³»?E¹«±â°¡ Æ²¸®´Ù?Eº¸³»?E¹«±â·Î ¹Ù²ãÁØ´Ù..
			OnChangeWeapon(uid, parts);
		}
	}break;

	case MC_PEER_SHOT:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB) break;	// ¹®Á¦°¡ ÀÖ´Ù
		ZPACKEDSHOTINFO* pinfo = (ZPACKEDSHOTINFO*)pParam->GetPointer();

		// Ä®Áú¸¸ µô·¹ÀÌ°¡ ÀÖ´Ù
		if (pinfo->sel_type != MMCIP_MELEE) return false;

		if (pChar != m_pMyCharacter &&
			(pChar->m_pVMesh->GetSelectWeaponMotionType() == eq_wd_dagger ||
				pChar->m_pVMesh->GetSelectWeaponMotionType() == eq_ws_dagger)) { // dagger
			pChar->SetAnimationUpper(ZC_STATE_UPPER_SHOT);
		}

		fDelayTime = .15f;

		////////////////////////////////////////////////////////////////////
		MMatchCharItemParts parts = (MMatchCharItemParts)pinfo->sel_type;
		if (parts != pChar->GetItems()->GetSelectedWeaponParts()) {
			// Áö±Ý µé?EÀÖ´Â ¹«±â¿Í º¸³»?E¹«±â°¡ Æ²¸®´Ù?Eº¸³»?E¹«±â·Î ¹Ù²ãÁØ´Ù..
			OnChangeWeapon(uid, parts);
		}
		///////////////////////////////////////////////////////////////////////////////
	}
	break;

	// »õ·Î Ãß°¡µÈ ±ÙÁ¢°ø°Ý Ä¿¸Ç?E
	case MC_PEER_SHOT_MELEE:
	{
		float fShotTime;
		rvector pos;
		int nShot;

		pCommand->GetParameter(&fShotTime, 0, MPT_FLOAT);
		pCommand->GetParameter(&pos, 1, MPT_POS);
		pCommand->GetParameter(&nShot, 2, MPT_INT);

		if (pChar != m_pMyCharacter &&
			(pChar->m_pVMesh->GetSelectWeaponMotionType() == eq_wd_dagger ||
				pChar->m_pVMesh->GetSelectWeaponMotionType() == eq_ws_dagger)) { // dagger
			pChar->SetAnimationUpper(ZC_STATE_UPPER_SHOT);
		}

		fDelayTime = .1f;
		switch (nShot) {
		case 1: fDelayTime = .10f; break;
		case 2: fDelayTime = .15f; break;
		case 3: fDelayTime = .2f; break;
		case 4: fDelayTime = .25f; break;
		}

		if (nShot > 1)
		{
			char szFileName[20];
			if (pChar->GetProperty()->nSex == MMS_MALE)
				sprintf(szFileName, "fx2/MAL_shot_%02d", nShot);
			else
				sprintf(szFileName, "fx2/FEM_shot_%02d", nShot);

			ZGetSoundEngine()->PlaySound(szFileName, pChar->GetPosition());
		}
	}
	break;

	case MC_QUEST_PEER_NPC_ATTACK_MELEE:
		ZGetQuest()->OnPrePeerNPCAttackMelee(pCommand);
		fDelayTime = .4f;
		break;

		// ?E¡¼­µµ ÇÊÅÍ¸µÀÌ ¾ÈµÇ?EÀÌ°Ç delayed command°¡ ¾Æ´Ï´Ù
	default:
		bFiltered = false;
		break;
	}

	if (bFiltered)
	{
		ZObserverCommandItem* pZCommand = new ZObserverCommandItem;
		pZCommand->pCommand = pCommand->Clone();
		pZCommand->fTime = GetTime() + fDelayTime;
		m_DelayedCommandList.push_back(pZCommand);
		return true;
	}

	return false;
}

void ZGame::PostSpMotion(ZC_SPMOTION_TYPE mtype)
{
	if (m_pMyCharacter == NULL) return;
	// Custom: Added MMATCH_ROUNDSTATE_FREE to allowed emotion list
	//if (m_Match.GetRoundState() != MMATCH_ROUNDSTATE_PLAY && m_Match.GetRoundState() != MMATCH_ROUNDSTATE_FREE) return;

	// Custom: Macro spam fix
	if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
		return;

	if ((m_pMyCharacter->m_AniState_Lower.Ref() == ZC_STATE_LOWER_IDLE1) ||
		(m_pMyCharacter->m_AniState_Lower.Ref() == ZC_STATE_LOWER_IDLE2) ||
		(m_pMyCharacter->m_AniState_Lower.Ref() == ZC_STATE_LOWER_IDLE3) ||
		(m_pMyCharacter->m_AniState_Lower.Ref() == ZC_STATE_LOWER_IDLE4))
	{
		MMatchWeaponType type = MWT_NONE;

		ZItem* pSItem = m_pMyCharacter->GetItems()->GetSelectedWeapon();

		if (pSItem && pSItem->GetDesc()) {
			type = pSItem->GetDesc()->m_nWeaponType.Ref();
		}

		if (mtype == ZC_SPMOTION_TAUNT) // taunt ÀÏ °æ?E¸ð¼ÇÀÌ ¾ø¾ûØ­...
			if ((type == MWT_MED_KIT) ||
				(type == MWT_REPAIR_KIT) ||
				(type == MWT_FOOD) ||
				(type == MWT_LANDMINE_SPY) ||
				(type == MWT_BULLET_KIT))
			{
				return;
			}

		ZPostSpMotion(mtype);
	}
}

/*void ZGame::OnEventUpdateJjang(const MUID & uidChar, bool bJjang)
{
	ZCharacter* pCharacter = (ZCharacter*)m_CharacterManager.Find(uidChar);
	if (pCharacter == NULL) return;

	if (bJjang)
		ZGetEffectManager()->AddStarEffect(pCharacter);
}*/

//bool ZGame::CanAttack(ZObject *pAttacker, ZObject *pTarget)
//{
//	if(!IsReplay())
//		if(GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PLAY) return false;
//
//	if(pAttacker==NULL) return true;
//
//	if (pTarget == NULL)
//		return false;
//	if ( GetMatch()->IsTeamPlay() )
//	{
//		__try
//		{
//			if (pAttacker->GetTeamID() == pTarget->GetTeamID()) {
//				if (!GetMatch()->GetTeamKillEnabled())
//					return false;
//			}
//		}
//		__except(EXCEPTION_EXECUTE_HANDLER)
//		{
//			mlog("MERROR_1\n");
//			return false;
//		}
//	}
//#ifdef _QUEST
//	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) ||
//		ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
//	{
//		if (pAttacker->GetTeamID() == pTarget->GetTeamID())
//		{
//			return false;
//		}
//	}
//#endif
//	return true;
//}
bool ZGame::CanAttack(ZObject * pAttacker, ZObject * pTarget)
{
	//### ÀÌ ÇÔ¼ö¸¦ ¼öÁ¤ÇÏ¸é ¶È°°ÀÌ CanAttack_DebugRegister()¿¡µµ Àû¿ëÇØ ÁÖ¾î¾ß ÇÕ´Ï´Ù. ###
	if (!IsReplay())
		if (GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PLAY) return false;

	if (pAttacker == NULL) return true;

	if (GetMatch()->IsTeamPlay()) {
		if (pAttacker->GetTeamID() == pTarget->GetTeamID())
		{
			if (!GetMatch()->GetTeamKillEnabled())
				return false;
		}
	}

	if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SKILLMAP)
		return false;

#ifdef _QUEST
	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) ||
		ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		if (pAttacker->GetTeamID() == pTarget->GetTeamID())
		{
			return false;
		}
	}

#endif
	return true;
}
//jintriple3 µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ· ¹æÁö¸¦ À§ÇØ ISAttackable()ÇÔ¼ö¸¦ ´Ù¸¥ ÀÌ¸§À¸·Î ÇÏ³ª ?E¸¸µé¾úÀ½...
bool ZGame::CanAttack_DebugRegister(ZObject * pAttacker, ZObject * pTarget)
{
	if (!IsReplay())
		if (GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PLAY) return false;
	if (pAttacker == NULL) return true;

	// Custom: Silly Gunz crash fix
	if (pTarget == NULL)
		return false;

	if (GetMatch()->IsTeamPlay()) {
		if (pAttacker->GetTeamID() == pTarget->GetTeamID()) {
			if (!GetMatch()->GetTeamKillEnabled())
				return false;
		}
	}

	if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SKILLMAP)
		return false;

#ifdef _QUEST
	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) ||
		MMATCH_GAMETYPE_QUEST_CHALLENGE == ZGetGameClient()->GetMatchStageSetting()->GetGameType())
	{
		if (pAttacker->GetTeamID() == pTarget->GetTeamID())
		{
			return false;
		}
	}

#endif
	return true;
}

void ZGame::ShowReplayInfo(bool bShow)
{
	MWidget* pWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("CombatChatOutput");
	if (pWidget)
		pWidget->Show(bShow);

	m_bShowReplayInfo = bShow;

	GetRGMain().GetChat().HideDuringReplays = !bShow;
}

void ZGame::OnLocalOptainSpecialWorldItem(MCommand * pCommand)
{
	int nWorldItemID;
	pCommand->GetParameter(&nWorldItemID, 0, MPT_INT);

	switch (nWorldItemID)
	{
	case WORLDITEM_PORTAL_ID:
	{
		MMATCH_GAMETYPE eGameType = ZGetGameClient()->GetMatchStageSetting()->GetGameType();
		if (!ZGetGameTypeManager()->IsQuestDerived(eGameType) || ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_QUEST_CHALLENGE) break;

		// ¼­¹ö¿¡ Æ÷Å»·Î ÀÌµ¿ÇÑ´Ù?E?E?
		char nCurrSectorIndex = ZGetQuest()->GetGameInfo()->GetCurrSectorIndex();
		ZPostQuestRequestMovetoPortal(nCurrSectorIndex);
	}
	break;
	};
}

void ZGame::ReserveSuicide(void)
{
	m_bSuicide = true;
}

bool ZGame::OnRuleCommand(MCommand * pCommand)
{
#ifdef _QUEST
	if (ZGetQuest()->OnGameCommand(pCommand)) return true;
#endif
	if (m_Match.GetMatchType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		return m_Match.OnCommand(pCommand);
	}

	switch (pCommand->GetID())
	{
	case MC_MATCH_ASSIGN_COMMANDER:
	case MC_MATCH_ASSIGN_BERSERKER:
	case MC_MATCH_FLAG_EFFECT:
	case MC_MATCH_FLAG_CAP:
	case MC_MATCH_FLAG_STATE:
	case MC_MATCH_INFECT:
	case MC_MATCH_LASTSURVIVOR:
	case MC_MATCH_GUNGAME_WEAPONDATA:
	case MC_MATCH_GAME_DEAD:
	case MC_MATCH_DUEL_QUEUEINFO:
	case MC_MATCH_DUELTOURNAMENT_GAME_NEXT_MATCH_PLYAERINFO:
	case MC_MATCH_DUELTOURNAMENT_GAME_INFO:
	case MC_MATCH_DUELTOURNAMENT_GAME_ROUND_RESULT_INFO:
	case MC_MATCH_DUELTOURNAMENT_GAME_MATCH_RESULT_INFO:

	case MC_SPY_GAME_INFO:
	case MC_SPY_GAME_RESULT:
	case MC_MATCH_SPY_ROUNDRESULT:

	case MC_MATCH_RESPONSE_SKILLMAP_BESTTIME:
	case MC_SKILLMAP_OBTAIN_ITEM:

#ifdef _MAGICBOX
	case MC_DROPGUNGAME_RESPONSE_ENTERGAME:
	case MC_DROPGUNGAME_RESPONSE_ITEM:
	case MC_DROPGUNGAME_RESPONSE_WORLDITEMS:
#endif

	{
		if (m_Match.OnCommand(pCommand)) return true;
	};
	};

	return false;
}

void ZGame::OnResetTeamMembers(MCommand * pCommand)
{
	if (!m_Match.IsTeamPlay()) return;

	if (m_Match.GetMatchType() != MMATCH_GAMETYPE_INFECTED)
		ZChatOutput(MCOLOR(ZCOLOR_GAME_INFO), ZMsg(MSG_GAME_MAKE_AUTO_BALANCED_TEAM));

	MCommandParameter* pParam = pCommand->GetParameter(0);
	if (pParam->GetType() != MPT_BLOB) return;
	void* pBlob = pParam->GetPointer();
	int nCount = MGetBlobArrayCount(pBlob);

	ZCharacterManager* pCharMgr = ZGetCharacterManager();

	for (int i = 0; i < nCount; i++)
	{
		MTD_ResetTeamMembersData* pDataNode = (MTD_ResetTeamMembersData*)MGetBlobArrayElement(pBlob, i);

		ZCharacter* pChar = (ZCharacter*)pCharMgr->Find(pDataNode->m_uidPlayer);
		if (pChar == NULL) continue;

		if (pChar->GetTeamID() != ((MMatchTeam)pDataNode->nTeam))
		{
			// ¸¸?E³ªÀÚ½ÅÀÌ ÆÀº¯°æÀÌ µÇ¾úÀ¸?EÆÀº¯°æµÇ¾ú´ÂÁö¸¦ ³²±ä´Ù.
			if (pDataNode->m_uidPlayer == ZGetMyUID())
			{
				ZGetMyInfo()->GetGameInfo()->bForcedChangeTeam = true;
			}

			pChar->SetTeamID((MMatchTeam)pDataNode->nTeam);
		}
	}
}

void ZGame::MakeResourceCRC32(const DWORD dwKey, DWORD & out_crc32, DWORD & out_xor)
{
	out_crc32 = 0;
	out_xor = 0;

#ifdef _DEBUG
	static DWORD dwOutputCount = 0;
	++dwOutputCount;
#endif

	MMatchObjCacheMap* pObjCacheMap = ZGetGameClient()->GetObjCacheMap();
	if (NULL == pObjCacheMap)
	{
		return;
	}

	MMatchObjCacheMap::const_iterator	end = pObjCacheMap->end();
	MMatchObjCacheMap::iterator			it = pObjCacheMap->begin();
	MMatchObjCache* pObjCache = NULL;
	MMatchItemDesc* pitemDesc = NULL;
	MMatchCRC32XORCache					CRC32Cache;

	CRC32Cache.Reset();
	CRC32Cache.CRC32XOR(dwKey);

#ifdef _DEBUG
	mlog("Start ResourceCRC32Cache : %u\n", CRC32Cache.GetXOR());
#endif

	for (; end != it; ++it)
	{
		pObjCache = it->second;

		for (int i = 0; i < MMCIP_END; ++i)
		{
			pitemDesc = MGetMatchItemDescMgr()->GetItemDesc(pObjCache->GetCostume()->nEquipedItemID[i]);
			if (NULL == pitemDesc)
			{
				continue;
			}

			pitemDesc->CacheCRC32(CRC32Cache);

#ifdef _DEBUG
			if (10 > dwOutputCount)
			{
				mlog("ItemID : %d, CRCCache : %u\n"
					, pitemDesc->m_nID
					, CRC32Cache.GetXOR() );
		}
#endif
		}
	}

#ifdef _DEBUG
	if (10 > dwOutputCount)
	{
		mlog("ResourceCRCSum : %u\n", CRC32Cache.GetXOR());
	}
#endif

	out_crc32 = CRC32Cache.GetCRC32();
	out_xor = CRC32Cache.GetXOR();
	}

void ZGame::OnResponseUseSpendableBuffItem(MUID & uidItem, int nResult)
{
	// TodoH(?E - »ç?E??E?°á?EÃ³¸®
}

/*
void ZGame::OnGetSpendableBuffItemStatus(MUID& uidChar, MTD_CharBuffInfo* pCharBuffInfo)
{
	if (uidChar != ZGetMyUID()) {
		_ASSERT(0);
		return;
	}

	if( pCharBuffInfo == NULL ) {
		_ASSERT(0);
		return;
	}

	ZGetMyInfo()->SetCharBuffInfo(pCharBuffInfo);

	if( m_pMyCharacter != NULL ) {
		m_pMyCharacter->SetCharacterBuff(pCharBuffInfo);
	}
}*/

void ZGame::ApplyPotion(int nItemID, ZCharacter * pCharObj, float fRemainedTime)
{
	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pDesc == NULL) { _ASSERT(0);  return; }

	MMatchDamageType nDamageType = pDesc->m_nDamageType.Ref();

	if (nDamageType == MMDT_HASTE)
	{
		// °¡¼Ó ¾ÆÀÌÅÛ
		ZModule_Movable* pMod = (ZModule_Movable*)pCharObj->GetModule(ZMID_MOVABLE);
		if (pMod)
		{
			if (fRemainedTime == 0)
				fRemainedTime = pDesc->m_nDamageTime.Ref() * 0.001f;

			pMod->SetMoveSpeedHasteRatio(pDesc->m_nItemPower.Ref() * 0.01f, fRemainedTime, nItemID);
		}
		ZGetEffectManager()->AddHasteBeginEffect(pCharObj->GetPosition(), pCharObj);
	}
	else if (nDamageType == MMDT_HEAL || nDamageType == MMDT_REPAIR)
	{
		// ?E?È¸º¹ ¾ÆÀÌÅÛ
		if (pDesc->m_nDamageTime.Ref() == 0)
		{
			ZGetEffectManager()->AddPotionEffect(pCharObj->GetPosition(), pCharObj, pDesc->m_nEffectId);

			if (nDamageType == MMDT_HEAL)
			{
				int nAddedHP = pDesc->m_nItemPower.Ref();
				pCharObj->SetHP(min(pCharObj->GetHP() + nAddedHP, pCharObj->GetMaxHP()));
			}
			else if (nDamageType == MMDT_REPAIR)
			{
				int nAddedAP = pDesc->m_nItemPower.Ref();
				pCharObj->SetAP(min(pCharObj->GetAP() + nAddedAP, pCharObj->GetMaxAP()));
			}
			else
				_ASSERT(0);
		}
		// Èú¿À¹öÅ¸ÀÓ ¾ÆÀÌÅÛ
		else
		{
			ZModule_HealOverTime* pMod = (ZModule_HealOverTime*)pCharObj->GetModule(ZMID_HEALOVERTIME);
			if (pMod)
			{
				int nRemainedHeal = (int)fRemainedTime;
				if (nRemainedHeal == 0)
					nRemainedHeal = pDesc->m_nDamageTime.Ref();

				//damagetime¿¡ ?EÈ½¼ö¸¦ Ç¥±âÇÒ °Í
				pMod->BeginHeal(pDesc->m_nDamageType.Ref(), pDesc->m_nItemPower.Ref(), nRemainedHeal, pDesc->m_nEffectId, nItemID);
			}

			switch (nDamageType)
			{
			case MMDT_HEAL:
				ZGetEffectManager()->AddHealOverTimeBeginEffect(pCharObj->GetPosition(), pCharObj);
				break;
			case MMDT_REPAIR:
				ZGetEffectManager()->AddRepairOverTimeBeginEffect(pCharObj->GetPosition(), pCharObj);
				break;
			}
		}
	}
	else
		_ASSERT(0);
}

void ZGame::OnUseTrap(int nItemID, ZCharacter * pCharObj, rvector & pos)
{
	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pDesc == NULL) { _ASSERT(0); return; }

	rvector velocity;
	velocity = /*pCharObj->GetVelocity()+ */pCharObj->m_TargetDir * 1300.f;
	velocity.z = velocity.z + 300.f;
	m_WeaponManager.AddTrap(pos, velocity, nItemID, pCharObj);
}

void ZGame::OnUseSpyTrap(int nItemID, ZCharacter * pCharObj, rvector & pos)
{
	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pDesc == NULL) { _ASSERT(0); return; }

	rvector velocity;
	velocity = /*pCharObj->GetVelocity()+ */pCharObj->m_TargetDir * 1300.f;
	velocity.z = velocity.z + 300.f;
	m_WeaponManager.AddSpyTrap(pos, velocity, nItemID, pCharObj);
}

void ZGame::OnUseDynamite(int nItemID, ZCharacter * pCharObj, rvector & pos)
{
	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pDesc == NULL) { _ASSERT(0); return; }

	rvector velocity;
	velocity = /*pCharObj->GetVelocity()+ */pCharObj->m_TargetDir * 1300.f;
	velocity.z = velocity.z + 300.f;
	m_WeaponManager.AddDynamite(pos, velocity, pCharObj);
}

void ZGame::CheckZoneTrap(MUID uidOwner, rvector pos, MMatchItemDesc * pItemDesc, MMatchTeam nTeamID)
{
	if (!pItemDesc) return;

	float fRange = 300.f;

	ZObject* pTarget = NULL;
	// Custom: Changed ZCharacter to ZObject, fixes cq npc's damaging eachother.
	ZObject* pOwnerObject = (ZObject*)m_ObjectManager.GetObject(uidOwner);

	if (!pOwnerObject)
		return;

	float fDist;
	bool bReturnValue;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		pTarget = (*itor).second;

		// ³» Æ®·¦ÀÌ?EÇÇÇØ ¾øÀ½
#ifndef _DEBUG	// µð¹ö±× ºôµå¿¡ ÇÑÇØ¼­ Å×½ºÆ®¸¦ ½±°Ô ÇÏ?EÀ§ÇØ ³» Æ®·¦µµ ÇÇÇØ¸¦ ¹Þµµ·Ï ÇÑ´Ù
		bReturnValue = pTarget->GetUID() == uidOwner;
		if (pTarget->GetUID() == uidOwner)
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;
#endif

		bReturnValue = CanAttack(pOwnerObject, pTarget);
		if (!bReturnValue)
			PROTECT_DEBUG_REGISTER(!CanAttack_DebugRegister(pOwnerObject, pTarget))
			continue;

		//jintriple3 µð¹ö±× ·¹Áö½ºÅÍ ÇØÅ· ?E?¹ö±×.....
		bReturnValue = !pTarget || pTarget->IsDie();
		if (!pTarget || pTarget->IsDie())
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		// Ä³¸¯ÅÍÀÇ ¹ß¸ñ?E¡¼­ ¹Ý?EÃ¼Å©¸¦ ÇÑ´Ù
		fDist = Magnitude(pos - (pTarget->GetPosition() + rvector(0, 0, 10)));
		//jintriple3 µð¹ö±× ·¹Áö½ºÅÍ ÇÙ ?E?¹ö±×.....
		bReturnValue = fDist >= fRange;
		if (fDist >= fRange)
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		if (pos.z > pTarget->GetPosition().z + pTarget->GetCollHeight())
			continue;

		// Æ®·¦ ¹Ý?E²À?Eâ¿?Ä³¸¯ÅÍÀÇ ¹ß¸ñ»çÀÌ¿¡ º®ÀÌ ÀÖ´Â?EÈ®ÀÎ
		/*{
			const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;
			RBSPPICKINFO bpi;
			rvector orig = pos+rvector(0,0,fRange);
			rvector to = pTarget->GetPosition()+rvector(0,0,10);
			bool bBspPicked = GetWorld()->GetBsp()->PickTo(orig, to, &bpi, dwPickPassFlag);
			if(bBspPicked)
			{
				if (Magnitude(to-orig) > Magnitude(bpi.PickPos-orig))
					continue;
			}
		}// ÀÌÆåÆ®´Â º®À» ¶Õ¾ú´Âµ¥ ÇÇÇØ¸¦ ¾ÈÀÔ´Â°Ô ?EÀÌ»óÇÏ´Ù?EÇØ¼­ ÁÖ¼®*/

		//ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");

		if (m_pGameAction)
		{
			int nDuration = pItemDesc->m_nDamageTime.Ref();
			bool bApplied = false;
			switch (pItemDesc->m_nDamageType.Ref())
			{
			case MMDT_FIRE:
				// ºÒÆ®·¦Àº ItemPower°¡ ?EÌÁö¸?¶æÇÔ
				bApplied = m_pGameAction->ApplyFireEnchantDamage(pTarget, pOwnerObject, pItemDesc->m_nItemPower.Ref(), nDuration);
				break;
			case MMDT_COLD:
				// ¾óÀ½Æ®·¦Àº ItemPower°¡ ÀÌ¼Ó°¨¼Ò·®À» ¶æÇÔ (80ÀÌ?E80%ÀÇ ¼Ó·ÂÀÌ µÊ)
				bApplied = m_pGameAction->ApplyColdEnchantDamage(pTarget, pItemDesc->m_nItemPower.Ref(), nDuration);
				break;
			default:
				_ASSERT(0);
			}

			if (bApplied)
			{
				//removed npc check, checkcombo already does that
			//	if (pOwnerCharacter->IsNPC()) continue;
				ZCharacter* pOwnerCharacter = (ZCharacter*)ZGetGame()->m_CharacterManager.Find(uidOwner);
				if (pOwnerCharacter)
				{
					CheckCombo(pOwnerCharacter, pTarget, true);	//todok »ç¿ûÑå¸¦ ÄÑÁà¾ßÇÒ?E;
					CheckStylishAction(pOwnerCharacter);
				}
			}
		}
	}
	// ¹°¼Ó¿¡ ÀÖÀ» ¶§´Â »ç¶÷ÀÌ ¹â¾ÒÀ» ¶§ ¹°Æ¢?EÈ¿°ú¸¦ ÀÏÀ¸Å°ÀÚ
	GetWorld()->GetWaters()->CheckSpearing(pos, pos + rvector(0, 0, MAX_WATER_DEEP), 500, 0.8f);
}

void ZGame::OnExplosionDynamite(MUID uidOwner, rvector pos, float fDamage, float fRange, float fKnockBack, MMatchTeam nTeamID)
{
	ZObject* pTarget = NULL;

	float fDist;

	ZObject* pOwnerObject = (ZObject*)m_ObjectManager.GetObject(uidOwner);

	if (!pOwnerObject)
		return;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		pTarget = (*itor).second;

		bool bReturnValue = !pTarget || pTarget->IsDie();
		if (!pTarget || pTarget->IsDie())
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		fDist = Magnitude(pos - (pTarget->GetPosition() + rvector(0, 0, 80)));
		bReturnValue = fDist >= fRange;
		if (fDist >= fRange)
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		rvector dir = pos - (pTarget->GetPosition() + rvector(0, 0, 80));
		Normalize(dir);

		// ´ÙÀÌ³Ê¸¶ÀÌÆ®µµ ¼ö·ùÅºÃ³·³ ¹Ýµ¿À¸·Î Æ¢¾ûÏª°£´Ù.
		ZActor* pATarget = MDynamicCast(ZActor, pTarget);
		ZActorWithFSM* pFSMActor = MDynamicCast(ZActorWithFSM, pTarget);
		bool bPushSkip = false;

		if (pATarget)
		{
			bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
		}
		if (pFSMActor)
		{
			bPushSkip = pFSMActor->GetActorDef()->IsNeverBlasted();
		}

		if (bPushSkip == false)
		{
			pTarget->AddVelocity(fKnockBack * 7.f * (fRange - fDist) * -dir);
		}
		else
		{
			ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
		}
		ZCharacter* pOwnerCharacter = (ZCharacter*)ZGetGame()->m_CharacterManager.Find(uidOwner);
		if (pOwnerCharacter)
		{
			CheckCombo(pOwnerCharacter, pTarget, false);
			CheckStylishAction(pOwnerCharacter);
		}

		// ´ÙÀÌ³Ê¸¶ÀÌÆ®´Â Æø¹ß ?E?³»¿¡¼­ ¸ðµÎ °°Àº µ¥¹ÌÁö¸¦ ÀÔÈù´Ù.
		float fRatio = ZItem::GetPiercingRatio(MWT_DYNAMITYE, eq_parts_chest);
		pTarget->OnDamaged(pOwnerObject, pos, ZD_EXPLOSION, MWT_DYNAMITYE, fDamage, fRatio);
	}

#define SHOCK_RANGE		1500.f			// 10¹ÌÅÍ±û?EÈçµé¸°´Ù

	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if (pTargetCharacter)
	{
		float fPower = (SHOCK_RANGE - Magnitude(pTargetCharacter->GetPosition() + rvector(0, 0, 50) - pos)) / SHOCK_RANGE;

		if (fPower > 0)
			ZGetGameInterface()->GetCamera()->Shock(fPower * 500.f, .5f, rvector(0.0f, 0.0f, -1.0f));
	}
	GetWorld()->GetWaters()->CheckSpearing(pos, pos + rvector(0, 0, MAX_WATER_DEEP), 500, 0.8f);
}

int ZGame::GetCharacterBasicInfoTick()
{
	return (ZGetGameClient()->GetAllowTunneling() == false) ? PEERMOVE_TICK : PEERMOVE_AGENT_TICK;
}

MUID ZGame::GetMyUid()
{
	return ZGetGameClient()->GetPlayerUID();
}

bool ZGame::PickWorld(const rvector & pos, const rvector & dir, RBSPPICKINFO * pOut, DWORD dwPassFlag)
{
	RBspObject* r_map = GetWorld()->GetBsp();
	return r_map->Pick(pos, dir, pOut, dwPassFlag);
}
bool ZGame::CheckWall(rvector & origin, rvector & targetpos, float fRadius, float fHeight, RCOLLISIONMETHOD method, int nDepth, rplane * pimpactplane)
{
	return GetWorld()->GetBsp()->CheckWall(origin, targetpos, 35, 60, RCW_CYLINDER, 0, pimpactplane);
}
#ifdef _MOVINGFLOOR
void ZGame::AdjustMoveDiff(ZObject* pObject, rvector& diff)
{
	if (!pObject) return;

	for (auto const& worldObject : GetWorld()->GetWorldObjects())
	{
		if (worldObject->IsCollidable() == false)
			continue;

		rvector pos = worldObject->GetPosition();
		rvector dir = pObject->GetPosition() + diff - pos;
		dir.z = 0;
		float fDist = Magnitude(dir);

		float objDistance = 0;
		if (worldObject->GetCollisionType() == CT_CYLINDER)
			objDistance = worldObject->GetCollRadius();
		else
			objDistance = worldObject->GetCollWidth();

		float fCOLLISION_DIST = objDistance + pObject->GetCollRadius();

		if (worldObject->GetCollisionType() == CT_CYLINDER)
		{
			if (fDist < fCOLLISION_DIST && fabs(pos.z - pObject->GetPosition().z) < worldObject->GetCollHeight())
			{
				if (fDist <= 1.f)
				{
					pos.x += 1.f;
					dir = pObject->GetPosition() - pos;
				}

				if (DotProduct(dir, diff) < 0)	
				{
					Normalize(dir);
					rvector newthispos = pos + dir * (fCOLLISION_DIST + 1.f);

					rvector newdiff = newthispos - pObject->GetPosition();
					diff.x = newdiff.x;
					diff.y = newdiff.y;
				}
			}
		}
		else
		{
			if (fDist < fCOLLISION_DIST)
			{

				float heightdiff = fabs(pObject->GetPosition().z - pos.z);

				if (heightdiff < worldObject->GetCollHeight() && pObject->GetPosition().z < pos.z)
				{
					if (DotProduct(dir, diff) < 0)
					{
						Normalize(dir);
						rvector newthispos = pos + dir * (fCOLLISION_DIST + 1.f);

						rvector newdiff = newthispos - pObject->GetPosition();
						diff.x = newdiff.x;
						diff.y = newdiff.y;
					}
				}

				if (pObject->GetVisualMesh()->GetHeadPosition().z <= worldObject->GetPosition().z)
				{
					float dist = worldObject->GetPosition().z - pObject->GetVisualMesh()->GetHeadPosition().z;
					if (dist < worldObject->GetCollHeight())
					{
						diff.z -= dist;
					}
				}
			}
		}
	}
}
	
#else
void ZGame::AdjustMoveDiff(ZObject* pObject, rvector& diff)
{
	// TODO : Ãæµ¹ ¹üÀ§ / ¹æ¹ýÀ» À¯µ¿ÀûÀ¸·Î °¡Á®°¡ÀÚ
	//#define COLLISION_DIST	70.f

	if (!pObject) return;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		ZObject* pOtherObj = (*itor).second;
		if (pOtherObj != pObject && pOtherObj->IsCollideable())
		{
			rvector pos = pOtherObj->GetPosition();
			rvector dir = pObject->GetPosition() + diff - pos;
			dir.z = 0;
			float fDist = Magnitude(dir);

			float fCOLLISION_DIST = pOtherObj->GetCollRadius() + pObject->GetCollRadius();

			if (fDist < fCOLLISION_DIST && fabs(pos.z - pObject->GetPosition().z) < pOtherObj->GetCollHeight())	//height¿¡ *0.5ÇØ¾ßÇÏ´Â°Å ¾Æ´Ò±î?
			{
				// °ÅÀÇ °°ÀºÀ§Ä¡¿¡ ÀÖ´Â °æ¿ì.. ÇÑÂÊ¹æÇâÀ¸·Î ¹Ð¸²
				if (fDist < 1.f)
				{
					pos.x += 1.f;
					dir = pObject->GetPosition() - pos;
				}

				if (DotProduct(dir, diff) < 0)	// ´õ °¡±î¿öÁö´Â ¹æÇâÀÌ¸é
				{
					Normalize(dir);
					rvector newthispos = pos + dir * (fCOLLISION_DIST + 1.f);

					rvector newdiff = newthispos - pObject->GetPosition();
					diff.x = newdiff.x;
					diff.y = newdiff.y;
				}
			}
		}
	}
}
#endif
ZNavigationMesh ZGame::GetNavigationMesh()
{
	return ZNavigationMesh(GetWorld()->GetBsp()->GetNavigationMesh());
}

void ZGame::OnUseStunGrenade(int nItemID, ZCharacter * pCharObj, rvector & pos)
{
	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pDesc == NULL) { _ASSERT(0); return; }

	rvector velocity;
	velocity = pCharObj->m_TargetDir * 1300.f;
	velocity.z = velocity.z + 300.f;
	m_WeaponManager.AddStunGrenade(pos, velocity, pCharObj);
}

void ZGame::OnExplosionStunGrenade(MUID uidOwner, rvector pos, float fDamage, float fRange, float fKnockBack, MMatchTeam nTeamID)
{
	ZObject* pTarget = NULL;
	float fDist;

	ZObject* pOwnerObject = (ZObject*)m_ObjectManager.GetObject(uidOwner);

	if (!pOwnerObject)
		return;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		pTarget = (*itor).second;

		bool bReturnValue = !pTarget || pTarget->IsDie();
		if (!pTarget || pTarget->IsDie())
			continue;
		fDist = Magnitude(pos - (pTarget->GetPosition() + rvector(0, 0, 80)));
		bReturnValue = fDist >= fRange;
		if (fDist >= fRange)
			continue;
		rvector dir = pos - (pTarget->GetPosition() + rvector(0, 0, 80));
		Normalize(dir);
		ZCharacter* pOwnerCharacter = (ZCharacter*)ZGetGame()->m_CharacterManager.Find(uidOwner);
		if (pOwnerCharacter)
		{
			CheckCombo(pOwnerCharacter, pTarget, false);
			CheckStylishAction(pOwnerCharacter);
		}

		float fRatio = ZItem::GetPiercingRatio(MWT_STUNGRENADE, eq_parts_chest);
		pTarget->OnDamaged(pOwnerObject, pos, ZD_EXPLOSION, MWT_STUNGRENADE, fDamage, fRatio);
		pTarget->OnDamagedAnimation(pTarget, ZD_KATANA_SPLASH);
	}
	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if (pTargetCharacter)
	{
		float fPower = (SHOCK_RANGE - Magnitude(pTargetCharacter->GetPosition() + rvector(0, 0, 50) - pos)) / SHOCK_RANGE;

		if (fPower > 0)
			ZGetGameInterface()->GetCamera()->Shock(fPower * 500.f, .5f, rvector(0.0f, 0.0f, -1.0f));
	}

	GetWorld()->GetWaters()->CheckSpearing(pos, pos + rvector(0, 0, MAX_WATER_DEEP), 500, 0.8f);
}

void ZGame::CheckZoneItemKit(MUID uidOwner, rvector pos, float fDamage, MMatchTeam nTeamID)
{
	ZObject* pTarget = NULL;
	float fDist;

	ZObject* pOwnerObject = (ZObject*)m_ObjectManager.GetObject(uidOwner);

	if (!pOwnerObject)
		return;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		pTarget = (*itor).second;

		bool bReturnValue = !pTarget || pTarget->IsDie();
		if (!pTarget || pTarget->IsDie())
			continue;
		fDist = Magnitude(pos - (pTarget->GetPosition() + rvector(0, 0, 80)));
		rvector dir = pos - (pTarget->GetPosition() + rvector(0, 0, 80));
		Normalize(dir);
		ZCharacter* pOwnerCharacter = (ZCharacter*)ZGetGame()->m_CharacterManager.Find(uidOwner);
		if (pOwnerCharacter)
		{
			CheckCombo(pOwnerCharacter, pTarget, false);
			CheckStylishAction(pOwnerCharacter);
		}

		pTarget->OnDamaged(pOwnerObject, pos, ZD_EXPLOSION, MWT_LANDMINE, fDamage, 0);
		if (pTarget == ZGetGame()->m_pMyCharacter)
		{
			ZGetGame()->m_pMyCharacter->OnBlast(rvector(pTarget->GetDirection()));
			ZGetGame()->m_pMyCharacter->SetVelocity(rvector(0, 0, 0));
		}
		//	pTarget->OnDamagedAnimation(pTarget, ZD_KATANA_SPLASH);
	}
	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if (pTargetCharacter)
	{
		float fPower = (SHOCK_RANGE - Magnitude(pTargetCharacter->GetPosition() + rvector(0, 0, 50) - pos)) / SHOCK_RANGE;

		if (fPower > 0)
			ZGetGameInterface()->GetCamera()->Shock(fPower * 500.f, .5f, rvector(0.0f, 0.0f, -1.0f));
	}

	GetWorld()->GetWaters()->CheckSpearing(pos, pos + rvector(0, 0, MAX_WATER_DEEP), 500, 0.8f);
}
void ZGame::BerserkerKI()
{
	if (!ZGetGame()) return;

	for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin(); itor != ZGetGame()->m_CharacterManager.end(); ++itor)
	{
		ZGetEffectManager()->Clear();
		ZCharacter* pCharacter = (*itor).second;
		pCharacter->SetTagger(false);
	}

	ZCharacter* pBerserkerKIChar = ZGetGame()->m_CharacterManager.Find(ZGetMyUID());
	if (pBerserkerKIChar)
	{
		ZGetEffectManager()->AddBerserkerIcon(pBerserkerKIChar);
		pBerserkerKIChar->SetTagger(true);
	}
}
#ifdef _ROCKETGUIDED
void ZGame::OnGuidedMissle(MUID Owner, MUID Target)
{
	ZCharacter* pOwnerCharacter = NULL;
	ZCharacter* pTargetCharacter = NULL;

	pOwnerCharacter = (ZCharacter*)ZGetGame()->m_CharacterManager.Find(Owner);
	pTargetCharacter = (ZCharacter*)ZGetGame()->m_CharacterManager.Find(Target);

	if (pOwnerCharacter == NULL) return;
	if (pTargetCharacter == NULL) return;

	rvector pos = pOwnerCharacter->GetPosition();
	rvector velocity = pOwnerCharacter->GetDirection();

	pos.z += 125;

	if ((Owner == Target) || (pOwnerCharacter->GetUID() == pTargetCharacter->GetUID()))
		m_WeaponManager.AddRocket(pos, velocity, pOwnerCharacter);
	else
		m_WeaponManager.AddGuidedRocket(pos, velocity, pOwnerCharacter, pTargetCharacter);
}
#endif
#ifdef _PORTALGUN 1
void ZGame::OnExplosionPortal(MUID uidOwner, rvector pos)
{
	ZObject* pTarget = NULL;
}
#endif
//void ZGame::GetJjang()
//{
//	if (!ZGetGame()) return;
//
//	for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin(); itor != ZGetGame()->m_CharacterManager.end(); ++itor)
//	{
//		ZGetEffectManager()->Clear();
//	}
//	ZCharacter* pCharacter = ZGetGame()->m_CharacterManager.Find(ZGetMyUID());
//	if (pCharacter)
//	{
//		ZGetEffectManager()->AddStarEffect(pCharacter);
//	}
//}