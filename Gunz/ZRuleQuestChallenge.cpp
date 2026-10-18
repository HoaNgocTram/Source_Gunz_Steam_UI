#include "stdafx.h"
#include "ZRuleQuestChallenge.h"
#include "ZMatch.h"
#include "MActorDef.h"
#include "MNewQuestScenario.h"
#include "MMatchFormula.h"
#include "ZFSMManager.h"
#include "ZActorActionManager.h"
#include "ZActorAction.h"
#include "ZActorWithFSM.h"
#include "ZApplication.h"
#include "ZFSMManager.h"
#include "ZFSMParser.h"
#include "ZFSM.h"
#include "ZScreenEffectManager.h"


ZRuleQuestChallenge::ZRuleQuestChallenge(ZMatch* pMatch) 
 : ZRule(pMatch)
 , m_pFsmManager(new ZFSMManager)
 , m_pActorDefMgr(new MActorDefManager)
 , m_pActorActionMgr(new ZActorActionManager)
 , m_pScenario(NULL)
 , m_nCurrSector(0)
{
	m_nNPC = 0;
	m_nRewardXP = 0;
	m_nRewardBP = 0;
	m_bossUid = MUID(0,0);
	m_szElapsedTime = "";
	m_fElapsedTime = ZGetGame()->GetTime();
	m_bLateJoin = false;
	Init();
	m_OurCQRewards.clear();
}

ZRuleQuestChallenge::~ZRuleQuestChallenge()
{
	SAFE_DELETE(m_pFsmManager);
	SAFE_DELETE(m_pActorDefMgr);
	SAFE_DELETE(m_pActorActionMgr);
}

bool ZRuleQuestChallenge::LoadScenarioMap(const char* szScenarioName)
{
	MNewQuestScenarioManager* pScenarioMgr = ZApplication::GetStageInterface()->GetChallengeQuestScenario();
	if (!pScenarioMgr) { _ASSERT(0); return false; }
	MNewQuestScenario* pScenario = pScenarioMgr->GetScenario(szScenarioName);
	if (!pScenario) { _ASSERT(0); return false; }

	int numSector = pScenario->GetNumSector();
	for (int i=0; i<numSector; ++i)
	{
		MNewQuestSector* pSector = pScenario->GetSector(i);
		if (!pSector)  { _ASSERT(0); return false; }

		const char* szMapName = pSector->GetMapName();
		_ASSERT(0 != strlen(szMapName));
		ZGetWorldManager()->AddWorld(szMapName);
	}

	return true;
}

int ZRuleQuestChallenge::GetRoundMax()
{
	if (!m_pScenario) {
		_ASSERT(0);
		return 1;
	}

	return m_pScenario->GetNumSector();
}

int ZRuleQuestChallenge::GetCurrSector()
{
	if (!m_pScenario) {
		_ASSERT(0);
		return 1;
	}

	return m_nCurrSector;
}

int ZRuleQuestChallenge::GetNPC()
{
	if (!m_pScenario) {
		_ASSERT(0);
		return 1;
	}

	return m_nNPC;
}

bool ZRuleQuestChallenge::Init()
{
	//todok quest °ÔÀÓÀ» ÇÒ¶§¸¶´Ù »õ·Î ·ÎµùÇÏ´Â °ÍÀ» °³¼±ÇÏÀÚ.
	
	if (!m_pActorActionMgr->ReadXml(ZApplication::GetFileSystem(), "system/zactoraction.xml"))
	{
		mlog("Error while Load actoraction.\n");
		return false;
	}
	vector<ZFSM*> vecFSM;
	ZFSMParser fsmParser;
	if (!fsmParser.ReadXml(ZApplication::GetFileSystem(), "system/aifsm.xml",vecFSM, m_pActorActionMgr))
	{
		mlog("Error while Load aifsm.\n");
		return false;
	}
	m_pFsmManager->AddFsm(&vecFSM[0], (unsigned int)vecFSM.size());

	if (ZGetNpcMeshMgr()->LoadXmlList("model/npc2.xml") == -1)
	{
		mlog("Error while Load NPC Models.\n");
		return false;
	}

	if (!m_pActorDefMgr->ReadXml(ZApplication::GetFileSystem(), "system/npc2.xml"))
	{
		mlog("Error while Read npc2.\n");
		return false;
	}

	MNewQuestScenarioManager* pScenarioMgr = ZApplication::GetStageInterface()->GetChallengeQuestScenario();
	m_pScenario = pScenarioMgr->GetScenario(ZGetGameClient()->GetMatchStageSetting()->GetMapName());

	m_nCurrSector = 0;
	m_nNPC = 0;
	ZGetScreenEffectManager()->CreateQuestRes();
	return true;
}

DWORD ZRuleQuestChallenge::GetCurrTime()
{
	return m_fElapsedTime;
}

void ZRuleQuestChallenge::OnUpdate( float fDelta )
{
	if (ZGetGame() == NULL) return;

	if(ZGetCombatInterface() && !ZGetCombatInterface()->IsShowResult())
	{
		m_fElapsedTime = ZGetGame()->GetTime();
		DWORD dwTime = m_fElapsedTime;
		char szText[128];
		sprintf(szText, "%s : %d:%02d", "Time Passed", dwTime / 60, dwTime % 60);
		m_szElapsedTime = szText;
	}
}

float ZRuleQuestChallenge::FilterDelayedCommand( MCommand* pCommand )
{
	switch (pCommand->GetID())
	{
	//case MC_NEWQUEST_PEER_NPC_ATTACK_MELEE:	//todok ÀÌÁ¦ »ç¿ëÇÏÁö ¾ÊÀ½
	//	{
	//		char szActionName[64];	//todokkkk ¾×¼Ç ÀÌ¸§À» º¸³»Áö ¸»°í Á¤¼ö°°Àº È¿À²ÀûÀÎ °ªÀ¸·Î ±¸º°ÇÒ¼ö ÀÖ°Ô ÇÏÀÚ
	//		pCommand->GetParameter(szActionName,	1, MPT_STR, sizeof(szActionName));

	//		IActorAction* pAction = m_pActorActionMgr->GetAction(szActionName);
	//		if (!pAction) { _ASSERT(0); break; }

	//		const ZActorActionMeleeShot* pMeleeShot = pAction->GetMeleeShot();
	//		return pMeleeShot->GetDelay();
	//	}
	}

	return 0;
}

bool ZRuleQuestChallenge::OnSectorBonus(MCommand* pCommand)
{
	MUID uidPlayer;
	unsigned long int nRewardXP = 0;
	unsigned long int nRewardBP = 0;
	unsigned long int nLevelPct = 0;
	pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
	pCommand->GetParameter(&nRewardXP, 1, MPT_UINT);
	pCommand->GetParameter(&nRewardBP, 2, MPT_UINT);
	pCommand->GetParameter(&nLevelPct, 3, MPT_UINT);

	if (ZGetCharacterManager()->Find(uidPlayer) == ZGetGame()->m_pMyCharacter)
	{
		char szText[256];
		ZGetMyInfo()->SetXP(ZGetMyInfo()->GetXP() + nRewardXP);
		ZGetMyInfo()->SetLevelPercent(nLevelPct);
		ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();
		ZGetMyInfo()->SetBP(ZGetMyInfo()->GetBP() + nRewardBP);
		m_nRewardXP += nRewardXP;
		m_nRewardBP += nRewardBP;
		ZGetScreenEffectManager()->AddExpEffect(nRewardXP);		// È¹µæ °æÇèÄ¡ Ç¥½Ã ¾ø¾Ö±â
		sprintf(szText, "You have won %d XP and %d BP for clearing the round", nRewardXP, nRewardBP);
		ZChatOutput(ZCOLOR_CHAT_SYSTEM, szText);
	}

	return true;
}

bool ZRuleQuestChallenge::OnCommand( MCommand* pCommand )
{
	switch (pCommand->GetID())
	{
	//case MC_NEWQUEST_PEER_NPC_ATTACK_MELEE:	//todok ÀÌÁ¦ »ç¿ëÇÏÁö ¾ÊÀ½
	//	{
	//		MUID uidNpc;
	//		char szActionName[64];	//todokkkk ¾×¼Ç ÀÌ¸§À» º¸³»Áö ¸»°í Á¤¼ö°°Àº È¿À²ÀûÀÎ °ªÀ¸·Î ±¸º°ÇÒ¼ö ÀÖ°Ô ÇÏÀÚ
	//		pCommand->GetParameter(&uidNpc, 0, MPT_UID);
	//		pCommand->GetParameter(szActionName,	1, MPT_STR, sizeof(szActionName));

	//		ZActorBase* pActor = (ZActorBase*)ZGetObjectManager()->GetNPCObject(uidNpc);
	//		ZActorWithFSM* pActorWithFSM = MDynamicCast(ZActorWithFSM, pActor);
	//		if (pActorWithFSM)
	//		{
	//			pActorWithFSM->OnPeerMeleeShot(szActionName);
	//		}
	//		return true;
	//	}
	//	break;

	case MC_NEWQUEST_NPC_SPAWN:
		{
			MUID uidController;
			MUID uidNpc;
			char szActorDefName[128];
			unsigned char nCustomSpawnTypeIndex, nSpawnIndex;
			bool bIsBoss;
			pCommand->GetParameter(&uidController,	0, MPT_UID);
			pCommand->GetParameter(&uidNpc,			1, MPT_UID);
			pCommand->GetParameter(szActorDefName,	2, MPT_STR, sizeof(szActorDefName));
			pCommand->GetParameter(&nCustomSpawnTypeIndex,	3, MPT_UCHAR);
			pCommand->GetParameter(&nSpawnIndex,			4, MPT_UCHAR);
			pCommand->GetParameter(&bIsBoss, 				5, MPT_BOOL);
			SpawnActor(szActorDefName, uidNpc, nCustomSpawnTypeIndex, nSpawnIndex, ZGetMyUID()==uidController, bIsBoss);
			return true;
		}
		break;

	case MC_NEWQUEST_REQUEST_NPC_SPAWN:
	{
		MUID uidController;
		MUID uidNpc;
		char szActorDefName[128];
		int num, index;
		MShortVector dir,pos;
		//bool bIsBoss;
		pCommand->GetParameter(&uidController, 0, MPT_UID);
		pCommand->GetParameter(&uidNpc, 1, MPT_UID);
		pCommand->GetParameter(&num, 2, MPT_INT);
		pCommand->GetParameter(&index, 3, MPT_INT);
		pCommand->GetParameter(szActorDefName, 4, MPT_STR, sizeof(szActorDefName));
		//todo: implement this
		pCommand->GetParameter(&pos, 5, MPT_SVECTOR);
		pCommand->GetParameter(&dir, 6, MPT_SVECTOR);

		SpawnActorSummon(szActorDefName, uidNpc, num, 0, ZGetMyUID() == uidController, pos,dir);
		return true;
	}
	break;

	case MC_NEWQUEST_NPC_DEAD:
		{
			MUID uidKiller, uidNPC;

			pCommand->GetParameter(&uidKiller,	0, MPT_UID);
			pCommand->GetParameter(&uidNPC,		1, MPT_UID);

			ZActorWithFSM* pActor = (ZActorWithFSM*)ZGetObjectManager()->GetNPCObject(uidNPC);
			if (pActor)
			{
				ZGetObjectManager()->Delete(pActor);

				//todok ¾Æ·¡ ÁÖ¼® ±¸ÇöÇØÁà¾ß ÇÔ
				//m_GameInfo.IncreaseNPCKilled();

				ZCharacter* pCharacter = (ZCharacter*) ZGetCharacterManager()->Find(uidKiller);
				if (pCharacter)
				{
					pCharacter->GetStatus().Ref().nKills += 1;
					if(uidKiller == ZGetGame()->m_pMyCharacter->GetUID())
						ZGetScreenEffectManager()->AddKO();

					pCharacter->GetStatus().MakeCrc();
				}
				m_nNPC--;
			}
			return true;
		}
		break;

	case MC_NEWQUEST_MOVE_TO_NEXT_SECTOR:
		{
			int nSectorIndex = 0;
			bool bLateJoin = 0;
			pCommand->GetParameter(&nSectorIndex, 0, MPT_INT);
			pCommand->GetParameter(&bLateJoin, 1, MPT_BOOL);
			MoveToNextSector(nSectorIndex, bLateJoin);
			return true;
		}
		break;

	case MC_QUEST_SECTOR_BONUS:
	{
		OnSectorBonus(pCommand);
		return true;
	}
	break;

	case MC_NEWQUEST_OBTAIN_ZITEM:
	{
		OnObtainZItem(pCommand);
		return true;
	}
	break;

	case MC_NEWQUEST_PEER_NPC_BASICINFO:
		{
			MCommandParameter* pParam = pCommand->GetParameter(0);
			if(pParam->GetType()!=MPT_BLOB)
			{
				_ASSERT(0); break;
			}

			ZACTOR_WITHFSM_BASICINFO* pbi= (ZACTOR_WITHFSM_BASICINFO*)pParam->GetPointer();
			ZActorWithFSM* pActor = (ZActorWithFSM*)ZGetObjectManager()->GetNPCObject(pbi->uidNPC);
			if (pActor)
			{
				pActor->OnBasicInfo(pbi);
			}
			return true;
		}
		break;

	case MC_NEWQUEST_PEER_NPC_ACTION_EXECUTE:
		{
			MUID uidNpc;
			int nActionIndex;
			pCommand->GetParameter(&uidNpc, 0, MPT_UID);
			pCommand->GetParameter(&nActionIndex, 1, MPT_INT, sizeof(nActionIndex));

			ZActorBase* pActor = (ZActorBase*)ZGetObjectManager()->GetNPCObject(uidNpc);
			ZActorWithFSM* pActorWithFSM = MDynamicCast(ZActorWithFSM, pActor);
			if (pActorWithFSM)
			{
				pActorWithFSM->OnPeerActionExecute(nActionIndex);
			}
			return true;
		}
		break;

	case MC_NEWQUEST_NEW_CONTROL:
		{
			MUID uidChar, uidNPC;
			pCommand->GetParameter(&uidChar,	0, MPT_UID);
			pCommand->GetParameter(&uidNPC,		1, MPT_UID);

			ZActorBase* pActor = (ZActorBase*)ZGetObjectManager()->GetNPCObject(uidNPC);
			ZActorWithFSM* pActorWithFSM = MDynamicCast(ZActorWithFSM, pActor);
			if (pActorWithFSM)
			{
				bool bMyControl = (uidChar == ZGetGameClient()->GetPlayerUID());

				ZCharacter *pOwner = ZGetCharacterManager()->Find(uidChar);
				pActorWithFSM->SetMyControl(bMyControl);
				pActorWithFSM->SetOwner(pOwner->GetUserName());
			}
			return true;
		}
		break;

	case MC_MATCH_QUEST_PLAYER_DEAD:
		{
			MUID uidVictim;
			pCommand->GetParameter(&uidVictim, 0, MPT_UID);

			ZCharacter* pVictim = ZGetCharacterManager()->Find(uidVictim);

			if (pVictim)
			{
			if (pVictim != ZGetGame()->m_pMyCharacter)
				{
					pVictim->Die();	
				}

				ZCharacterStatus pVictimCS = pVictim->GetStatus().Ref();
				pVictimCS.AddDeaths();
				pVictim->GetStatus().MakeCrc();

				if (pVictimCS.nLife > 0) 
					pVictimCS.nLife--;
				pVictim->GetStatus().MakeCrc();
			}
			return true;
		}


		break;
	}

	return false;
}

bool ZRuleQuestChallenge::OnObtainZItem(MCommand* pCommand)
{
	MUID nPickerUID;
	unsigned int niLevel;
	unsigned int nRarity;
	pCommand->GetParameter(&nPickerUID, 0, MPT_UID);
	pCommand->GetParameter(&m_nItemID, 1, MPT_UINT);
	pCommand->GetParameter(&niLevel, 2, MPT_UINT);
	pCommand->GetParameter(&nRarity, 3, MPT_UINT);
#ifdef _QUEST_ITEM
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_nItemID);
	if (pItemDesc)
	{
		//Custom: iLevel name colors.
		string rarity = "^9";
		string raritystr = " (Common)";

		std::string totalName = "^9";

		// Àåºñ ÀÌ¸§
		if (pItemDesc)
		{

			switch (nRarity)
			{
			case 0:
				rarity = "^9";
				raritystr = " (Common)";
				break;
			case 1:
				rarity = "^2";
				raritystr = " (Uncommon)";
				break;
			case 2:
				rarity = "^C";
				raritystr = " (Rare)";
				break;
			case 3:
				rarity = "^E";
				raritystr = " (Epic)";
				break;
			case 4:
				rarity = "^8";
				raritystr = " (Legendary)";
				break;
			case 5:
				rarity = "^1";
				raritystr = " (Mythical)";
				break;
			default:
				rarity = "^9";
				raritystr = " (Common)";
				break;
			}
			totalName.append(rarity + pItemDesc->m_pMItemName->Ref().m_szItemName + raritystr);
			totalName.append(" (Item Lv. ");
			ostringstream ss;
			ss << niLevel;
			totalName.append(ss.str());
			totalName.append(")");
		}
		else
		{
			totalName.append(pItemDesc->m_pMItemName->Ref().m_szItemName);
		}
		if (ZGetGame() && ZGetCharacterManager())
		{
			ZCharacter* pChar = ZGetCharacterManager()->Find(nPickerUID);
			if (pChar && pChar->GetCharInfo())
			{

				std::string szMsg = pChar->GetCharInfo()->szName;
				szMsg.append(" obtains ");
				szMsg.append(totalName);
				m_OurCQRewards.push_back(pItemDesc->m_nID);
				ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM_GAME), szMsg.c_str());
			}
		}
		totalName.clear();
	}
#endif


	return true;
}


void ZRuleQuestChallenge::SpawnActor(const char* szActorDefName, MUID& uid, int nCustomSpawnTypeIndex, int nSpawnIndex, bool bMyControl, bool bIsBoss)
{
	rvector pos(0,0,0);
	rvector dir(1,0,0);

	ZMapSpawnManager* pMSM = ZGetGame()->GetMapDesc()->GetSpawnManager();
	if (pMSM)
	{
		ZMapSpawnData* pSpawnData = pMSM->GetCustomSpawnData(nCustomSpawnTypeIndex, nSpawnIndex);
		if (pSpawnData)
		{
			pos = pSpawnData->m_Pos;
			dir = pSpawnData->m_Dir;
		}
	}

	MActorDef* pActorDef = m_pActorDefMgr->GetDef(szActorDefName);
	if (!pActorDef) 
	{
		mlog("ERROR : cannot found actordef:\'%s\'\n", szActorDefName);

		_ASSERT(0); 
		return; 
	}

	// ¸¸¾à ¸®¼Ò½º ·ÎµùÀ» ¾ÈÇßÀ¸¸é ·Îµå - ÀÌ·²ÀÏÀº Å×½ºÆ®»©°ï ¾ø¾î¾ßÇÑ´Ù.
	RMesh* pNPCMesh = ZGetNpcMeshMgr()->Get((char*)pActorDef->GetModelName());
	if (pNPCMesh)
	{
		if (!pNPCMesh->m_isMeshLoaded)
		{
			ZGetNpcMeshMgr()->Load((char*)pActorDef->GetModelName());
			ZGetNpcMeshMgr()->ReloadAllAnimation();
		}
	}

	ZActorWithFSM* pNewActor = new ZActorWithFSM(ZGetGame(), m_pActorActionMgr);
	pNewActor->InitWithActorDef(pActorDef, m_pFsmManager);
	pNewActor->SetUID(uid);
	pNewActor->SetPosition(pos);
	pNewActor->SetDirection(dir);
	pNewActor->SetMyControl(bMyControl);
	rvector scale;
	scale.x = pNewActor->m_pVMesh->GetScale().x * (pActorDef ? pActorDef->GetScale() : 1.0f);
	scale.y = pNewActor->m_pVMesh->GetScale().y * (pActorDef ? pActorDef->GetScale() : 1.0f);
	scale.z = pNewActor->m_pVMesh->GetScale().z * (pActorDef ? pActorDef->GetScale() : 1.0f);
	pNewActor->m_pVMesh->SetScale(scale);

	ZGetObjectManager()->Add(pNewActor);
	ZGetEffectManager()->AddReBirthEffect(pNewActor->GetPosition());
	if (bIsBoss)
	{
		ZGetSoundEngine()->PlaySound("fx_respawn");
		ZGetEffectManager()->AddReBirthEffect(pNewActor->GetPosition());
		SetBoss(uid);
	}

	m_nNPC++;
}


void ZRuleQuestChallenge::SpawnActor(const char* szActorDefName, MUID& uid, int nCustomSpawnTypeIndex, int nSpawnIndex, bool bMyControl, bool bIsBoss, MShortVector vec)
{
	rvector pos(0, 0, 0);
	ZMapSpawnManager* pMSM = ZGetGame()->GetMapDesc()->GetSpawnManager();
	if (pMSM)
	{
		ZMapSpawnData* pSpawnData = pMSM->GetCustomSpawnData(nCustomSpawnTypeIndex, nSpawnIndex);
		if (pSpawnData)
		{
			pos = pSpawnData->m_Pos;
		}
	}

	MActorDef* pActorDef = m_pActorDefMgr->GetDef(szActorDefName);
	if (!pActorDef)
	{
		mlog("ERROR : cannot found actordef:\'%s\'\n", szActorDefName);

		_ASSERT(0);
		return;
	}

	// ¸¸¾à ¸®¼Ò½º ·ÎµùÀ» ¾ÈÇßÀ¸¸é ·Îµå - ÀÌ·²ÀÏÀº Å×½ºÆ®»©°ï ¾ø¾î¾ßÇÑ´Ù.
	RMesh* pNPCMesh = ZGetNpcMeshMgr()->Get((char*)pActorDef->GetModelName());
	if (pNPCMesh)
	{
		if (!pNPCMesh->m_isMeshLoaded)
		{
			ZGetNpcMeshMgr()->Load((char*)pActorDef->GetModelName());
			ZGetNpcMeshMgr()->ReloadAllAnimation();
		}
	}

	ZActorWithFSM* pNewActor = new ZActorWithFSM(ZGetGame(), m_pActorActionMgr);
	pNewActor->InitWithActorDef(pActorDef, m_pFsmManager);
	pNewActor->SetUID(uid);
	pos = rvector(vec.x, vec.y, vec.z);
	pNewActor->SetPosition(pos);
	pNewActor->SetMyControl(bMyControl);

	rvector scale;
	scale.x = pNewActor->m_pVMesh->GetScale().x * (pActorDef ? pActorDef->GetScale() : 1.0f);
	scale.y = pNewActor->m_pVMesh->GetScale().y * (pActorDef ? pActorDef->GetScale() : 1.0f);
	scale.z = pNewActor->m_pVMesh->GetScale().z * (pActorDef ? pActorDef->GetScale() : 1.0f);
	pNewActor->m_pVMesh->SetScale(scale);


	ZGetObjectManager()->Add(pNewActor);
	ZGetEffectManager()->AddReBirthEffect(pNewActor->GetPosition());
	if (bIsBoss)
	{
		ZGetSoundEngine()->PlaySound("fx_respawn");
		ZGetEffectManager()->AddReBirthEffect(pNewActor->GetPosition());
		SetBoss(uid);
	}

	m_nNPC++;
}

void ZRuleQuestChallenge::SpawnActorSummon(const char* szActorName, MUID& uid, int nNum, int nIndex, bool bMyControl, MShortVector vec, MShortVector vec2)
{
	rvector pos = rvector(0,0,0);
	rvector dir = rvector(1, 0, 0);

	MActorDef* pActorDef = m_pActorDefMgr->GetDef(szActorName);
	if (!pActorDef)
	{
		mlog("ERROR : cannot found actordef:\'%s\'\n", szActorName);

		_ASSERT(0);
		return;
	}

	// ¸¸¾à ¸®¼Ò½º ·ÎµùÀ» ¾ÈÇßÀ¸¸é ·Îµå - ÀÌ·²ÀÏÀº Å×½ºÆ®»©°ï ¾ø¾î¾ßÇÑ´Ù.
	RMesh* pNPCMesh = ZGetNpcMeshMgr()->Get((char*)szActorName);
	if (pNPCMesh)
	{
		if (!pNPCMesh->m_isMeshLoaded)
		{
			ZGetNpcMeshMgr()->Load((char*)szActorName);
			ZGetNpcMeshMgr()->ReloadAllAnimation();
		}
	}

	ZActorWithFSM* pNewActor = new ZActorWithFSM(ZGetGame(), m_pActorActionMgr);
	pNewActor->InitWithActorDef(pActorDef, m_pFsmManager);
	pNewActor->SetUID(uid);
	pos = rvector(vec.x, vec.y, vec.z);
	dir = rvector(vec2.x, vec2.y, vec2.z);
	pNewActor->SetPosition(pos);
	pNewActor->SetDirection(dir);
	pNewActor->SetMyControl(bMyControl);

	rvector scale;
	scale.x = pNewActor->m_pVMesh->GetScale().x * (pActorDef ? pActorDef->GetScale() : 1.0f);
	scale.y = pNewActor->m_pVMesh->GetScale().y * (pActorDef ? pActorDef->GetScale() : 1.0f);
	scale.z = pNewActor->m_pVMesh->GetScale().z * (pActorDef ? pActorDef->GetScale() : 1.0f);
	pNewActor->m_pVMesh->SetScale(scale);


	ZGetObjectManager()->Add(pNewActor);
	ZGetEffectManager()->AddReBirthEffect(pNewActor->GetPosition());

	m_nNPC++;
}

void ZRuleQuestChallenge::MoveToNextSector(int nSectorIndex, bool bLateJoin)
{
	ZCharacter *pMyChar = ZGetGame()->m_pMyCharacter;
	//Custom: Fixes despawn when the game is ending
	if (m_nCurrSector + 1 >= GetRoundMax()) return;

	if (!bLateJoin && nSectorIndex == m_nCurrSector)
	{
		pMyChar->InitStatus();

		ZGetEffectManager()->Clear();
		ZGetGame()->m_WeaponManager.Clear();
	}

	if (!bLateJoin)
	{
		m_nCurrSector = nSectorIndex;
		ZGetWorldManager()->SetCurrent(m_nCurrSector);
		m_nNPC = 0;
	}
	else
	{
		m_nCurrSector = nSectorIndex; //Gotta display the correct sector with the UI
		ZGetWorldManager()->SetCurrent(nSectorIndex); //don't set m_nCurrSector, will send you to incorrect sector
		m_nNPC = GetNPC(); // Get current npc count 
		for (auto itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); ++itor)
		{
			ZCharacter* pCharacter = (*itor).second;
			pCharacter->GetKils();
			pCharacter->GetStatus().Ref().nDeaths;
		}
	}

	if (!bLateJoin)
	{
		int nPosIndex = ZGetCharacterManager()->GetCharacterIndex(pMyChar->GetUID(), false);
		if (nPosIndex < 0) nPosIndex = 0;
		ZMapSpawnData* pSpawnData = ZGetWorld()->GetDesc()->GetSpawnManager()->GetSoloData(nPosIndex);

		if (pSpawnData != NULL && pMyChar != NULL)
		{
			pMyChar->SetPosition(pSpawnData->m_Pos);
			pMyChar->SetDirection(pSpawnData->m_Dir);
		}
	}

	if (bLateJoin)
	{

	}

	else
	{
		for (ZCharacterManager::iterator i = ZGetCharacterManager()->begin(); i != ZGetCharacterManager()->end(); i++)
		{
			i->second->SetVisible(false);
		}
	}

	if (!bLateJoin)
	{
		int nKills = ZGetGame()->m_pMyCharacter->GetStatus().Ref().nKills;
		ZGetScreenEffectManager()->SetKO(nKills);
	}
	//Don't free a scenario that uses multiple of the same map
	//if ((m_pScenario->GetID() != 102) && (m_pScenario->GetID() != 202) &&
	//	(m_pScenario->GetID() != 302) && (m_pScenario->GetID() != 402))
	//{
	//	ZGetWorldManager()->GetWorld(m_nCurrSector - 1)->Destroy();//free memory
	//}
}