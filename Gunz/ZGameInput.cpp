#include "stdafx.h"

#include "ZGameInput.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "ZGame.h"
#include "ZConfiguration.h"
#include "ZActionDef.h"
#include "Mint.h"
#include "MEvent.h"
#include "MWidget.h"
#include "ZGameClient.h"
#include "ZCombatInterface.h"
#include "ZConsole.h"
//#include "MActionKey.h"
#include "ZPost.h"
#include "ZScreenEffectManager.h"
#include "ZMyInfo.h"
#include "ZMinimap.h"
#include "ZInput.h"
#include "ZBandiCapturer.h" // µ¿¿µ»EÄ¸ÃÄ
#include <random>
#include "RGMain.h"
#undef _DONOTUSE_DINPUT_MOUSE

ZGameInput* ZGameInput::m_pInstance = NULL;
static DWORD g_dwMacroTime = 0;
static DWORD g_dwSpectDelayTime = 0;

ZGameInput::ZGameInput()
{
	m_pInstance = this;
	m_bCTOff = false;

	// ÀÌ°ÍµéÀº ½ÇÇàµÇ´Â ³»³» m_SequenceActions¾È¿¡ ÂE¶µÇ¹Ç·Î static À¸·Î ¼±¾ðµÇ¾EÀÖ´Ù.
	static ZKEYSEQUENCEITEM action_ftumble[]= { {true,ZACTION_FORWARD}, {false,ZACTION_FORWARD} , {true,ZACTION_FORWARD} };	// ¾Õ ¾Õ
	static ZKEYSEQUENCEITEM action_btumble[]= { {true,ZACTION_BACK}, {false,ZACTION_BACK} , {true,ZACTION_BACK} };	// µÚ µÚ
	static ZKEYSEQUENCEITEM action_rtumble[]= { {true,ZACTION_RIGHT}, {false,ZACTION_RIGHT} , {true,ZACTION_RIGHT} };
	static ZKEYSEQUENCEITEM action_ltumble[]= { {true,ZACTION_LEFT}, {false,ZACTION_LEFT} , {true,ZACTION_LEFT} };	

#define ADDKEYSEQUENCE(time,x) m_SequenceActions.push_back(ZKEYSEQUENCEACTION(time,sizeof(x)/sizeof(ZKEYSEQUENCEITEM),x));

	const float DASH_SEQUENCE_TIME = 0.2f;
	ADDKEYSEQUENCE(DASH_SEQUENCE_TIME,action_ftumble);
	ADDKEYSEQUENCE(DASH_SEQUENCE_TIME,action_btumble);
	ADDKEYSEQUENCE(DASH_SEQUENCE_TIME,action_rtumble);
	ADDKEYSEQUENCE(DASH_SEQUENCE_TIME,action_ltumble);
}

ZGameInput::~ZGameInput()
{
	m_pInstance = NULL;
}
// Custom: Teleport Save and Load By Desperate
rvector PlayerPosHotKey;
bool ZGameInput::OnEvent(MEvent* pEvent)
{
	int sel = 0;

	if ((ZGetGameInterface()->GetState() != GUNZ_GAME)) return false;
	if ( ZGetGameInterface()->GetGame() == NULL ) return false;

	MWidget* pMenuWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("CombatMenuFrame");
	if ((pMenuWidget) && (pMenuWidget->IsVisible())) return false;
	MWidget* pChatWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("CombatChatInput");
	if ((pChatWidget) && (pChatWidget->IsVisible())) return false;
	MWidget* p112ConfirmWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("112Confirm");
	if (p112ConfirmWidget->IsVisible()) return false;
	// Custom: Enhanced Kick System
	MWidget* pKickPlayerWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("KickPlayer");
	if (pKickPlayerWidget && pKickPlayerWidget->IsVisible()) return false;

	if (GetRGMain().OnEvent(pEvent))
		return true;
		
#ifndef _PUBLISH
	if (m_pInstance) { 
		if (m_pInstance->OnDebugEvent(pEvent) == true) 
			return true;
	}
#endif

	ZMyCharacter* pMyCharacter = ZGetGameInterface()->GetGame()->m_pMyCharacter;
	if ((!pMyCharacter) || (!pMyCharacter->GetInitialized())) return false;

#ifdef _AFKSYSTEM
	//pMyCharacter->m_nLastKeyTime = timeGetTime();
#endif

	////////////////////////////////////////////////////////////////////////////
	switch(pEvent->nMessage){
	case MWM_HOTKEY:
		{
			int nKey = pEvent->nKey;
			ZHOTKEY *hk=ZGetConfiguration()->GetHotkey(nKey);
			//if(ProcessLowLevelCommand(hk->command.c_str())==false)
			
			char buffer[256];
			strcpy(buffer,hk->command.c_str());
			ZApplication::GetGameInterface()->GetChat()->Input(buffer);

//			ConsoleInputEvent(hk->command.c_str());
		}break;

	case MWM_LBUTTONDOWN:
		{
			ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();

			if ( ZGetCombatInterface()->IsShowResult())
			{
				if ( ((ZGetCombatInterface()->m_nReservedOutTime - timeGetTime()) / 1000) < 13)
				{
					if (ZGetGameClient()->IsLadderWarsChannel() || ZGetGameClient()->IsCWChannel() || ZGetGameClient()->IsDuelTournamentGame())
						ZChangeGameState(GUNZ_LOBBY);
					else
						ZChangeGameState(GUNZ_STAGE);

					return true;
				}
			}

			if (pCombatInterface->IsChat())
			{
				pCombatInterface->EnableInputChat(false);
			}

			if (pCombatInterface->GetObserver()->IsVisible())
			{
				pCombatInterface->GetObserver()->ChangeToNextTarget();
				return true;
			}

/*			if ((pMyCharacter) && (pMyCharacter->IsDie()))	//// ½Ç¼­ºñ½º¿¡¼­ ½ºÆù¾ÈµÇ´Â ¹ö±×À¯¹ß. ¿øÀÎºÒ¸E_PUBLISH´©¶E ¿µ±¸ºÀ¼E
			{
				// È¥ÀÚÅ×½ºÆ®ÇÒ¶§ µÇ»EÆ³ª±E
				if(g_pGame->m_CharacterManager.size()==1)
				{
#ifndef _PUBLISH
					ZGetGameInterface()->RespawnMyCharacter();
					return true;
#endif
				}
			}*/
			if (ZGetGameInterface()->IsCursorEnable())
				return false;
		}
		return true;
	case MWM_RBUTTONDOWN:
		{
			if (ZGetGameInterface()->GetCombatInterface()->IsChat())
			{
				ZGetGameInterface()->GetCombatInterface()->EnableInputChat(false);
			}

			ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
			if (pCombatInterface->GetObserver()->IsVisible())
			{
				pCombatInterface->GetObserver()->NextLookMode();
			}
		}
		return true;
	case MWM_MBUTTONDOWN:
		if (ZGetGameInterface()->GetCombatInterface()->IsChat())
		{
			ZGetGameInterface()->GetCombatInterface()->EnableInputChat(false);
		}
		return true;
	case MWM_ACTIONRELEASED:
		{
			switch(pEvent->nKey)
			{
				if (ZGetGame()->m_pMyCharacter)
#ifdef _ICONCHAT
					ZGetGame()->m_pMyCharacter->m_dwStatusBitPackingValue.Ref().m_bChatVoice == false;
#endif
			case ZACTION_FORWARD:
			case ZACTION_BACK:
			case ZACTION_LEFT:
			case ZACTION_RIGHT:
				if (m_pInstance) 
					m_pInstance->m_ActionKeyHistory.push_back(ZACTIONKEYITEM(ZGetGame()->GetTime(),false,pEvent->nKey));
				return true;

			case ZACTION_DEFENCE:
				{
					if(ZGetGame()->m_pMyCharacter)
						ZGetGame()->m_pMyCharacter->m_statusFlags.Ref().m_bGuardKey = false;
				}
				return true;
			}
		}break;
	case MWM_ACTIONPRESSED:
		if ( !ZGetGame()->IsReservedSuicide() && !ZGetGame()->m_pMyCharacter->m_dwStatusBitPackingValue.Ref().m_bFrozen)		// ÀÚ»"¿¹Á¤ÀÎ °æ¿"´"¬¸¦ ÇÒ¼ö¾ø°Ô ¸·´Â´Ù
		{
		switch(pEvent->nKey)
		{
#ifdef _ICONCHAT
		case ZACTION_VOICE_CHAT:
		{
			if (ZGetGame()->m_pMyCharacter)
				ZGetGame()->m_pMyCharacter->m_dwStatusBitPackingValue.Ref().m_bChatVoice == false;
		}
#endif
			case ZACTION_FORWARD:
			case ZACTION_BACK:
			case ZACTION_LEFT:
			case ZACTION_RIGHT:
			case ZACTION_JUMP:
				if (m_pInstance) 
					m_pInstance->m_ActionKeyHistory.push_back(ZACTIONKEYITEM(ZGetGame()->GetTime(),true,pEvent->nKey));
				return true;
			case ZACTION_MELEE_WEAPON:
				{
					if ( !ZGetGame()->IsReplay())
#ifdef _PAINTMODE
					if (!ZGetGame()->GetMatch()->IsPaintBall())
#endif
						ZGetGameInterface()->ChangeWeapon(ZCWT_MELEE);
				}
				return true;
			case ZACTION_PRIMARY_WEAPON:
				{
					if ( !ZGetGame()->IsReplay())
						ZGetGameInterface()->ChangeWeapon(ZCWT_PRIMARY);
				}
				return true;
			case ZACTION_SECONDARY_WEAPON:
				{
					if ( !ZGetGame()->IsReplay())
						ZGetGameInterface()->ChangeWeapon(ZCWT_SECONDARY);
				}
				return true;

			case ZACTION_ITEM1:
			case ZACTION_ITEM2:
			{
				int nIndex = pEvent->nKey - ZACTION_ITEM1 + ZCWT_CUSTOM1;
				if (!ZGetGame()->IsReplay()) 
				{
					ZGetGameInterface()->ChangeWeapon(ZChangeWeaponType(nIndex));
				}
			}
			return true;

			case ZACTION_ITEM3:
			{
				if (!ZGetGame()->IsReplay())
					ZGetGameInterface()->ChangeWeapon(ZCWT_CUSTOM3);
			}
			return true;

			/*case ZACTION_COMMUNITYITEM1:	mlog("Community Item1 Selected!\n"); return true;
			case ZACTION_COMMUNITYITEM2:	mlog("Community Item2 Selected!\n"); return true;*/
			case ZACTION_PREV_WEAPON:
				{
					if ( !ZGetGame()->IsReplay())
						ZGetGameInterface()->ChangeWeapon(ZCWT_PREV);
				}
				return true;
			case ZACTION_NEXT_WEAPON:
				{
					if ( !ZGetGame()->IsReplay())
						ZGetGameInterface()->ChangeWeapon(ZCWT_NEXT);
				}
				return true;
			case ZACTION_RELOAD:
				{
					if ( !ZGetGame()->IsReplay())
						ZGetGameInterface()->Reload();
				}
				return true;
			case ZACTION_DEFENCE:
				{
					if ( ZGetGame()->m_pMyCharacter && !ZGetGame()->IsReplay())
						ZGetGame()->m_pMyCharacter->m_statusFlags.Ref().m_bGuardKey = true;
				}
				return true;

			case ZACTION_TAUNT:		// Æ¿´ÙÅ°
			case ZACTION_BOW:
			case ZACTION_WAVE:
			case ZACTION_LAUGH:
			case ZACTION_CRY:
			case ZACTION_DANCE:
				{
					if ( ZGetGame()->IsReplay())
						break;
					if ( MEvent::GetShiftState())
						break;
					if(ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
						break;

					ZC_SPMOTION_TYPE mtype;
#ifdef _SPMOTIONLAST
					if (pEvent->nKey == ZACTION_TAUNT)
					{
						mtype = ZC_SPMOTION_TAUNT;
					}
					else if (pEvent->nKey == ZACTION_BOW)
					{
						mtype = ZC_SPMOTION_BOW;
					}
					else if (pEvent->nKey == ZACTION_WAVE)
					{
						mtype = ZC_SPMOTION_WAVE;
					}
					else if (pEvent->nKey == ZACTION_LAUGH)
					{
						mtype = ZC_SPMOTION_LAUGH;
					}
					else if (pEvent->nKey == ZACTION_CRY)
					{
						mtype = ZC_SPMOTION_CRY;
					}
					else if (pEvent->nKey == ZACTION_DANCE)
					{
						ZPOSTCMD0(CLOAK_CMD_ID(MC_MATCH_SPMOTION, 123));
						return true;
					}
					else
						return true;
#else
					if (pEvent->nKey == ZACTION_TAUNT) mtype = ZC_SPMOTION_TAUNT;
					else if (pEvent->nKey == ZACTION_BOW) mtype = ZC_SPMOTION_BOW;
					else if (pEvent->nKey == ZACTION_WAVE) mtype = ZC_SPMOTION_WAVE;
					else if (pEvent->nKey == ZACTION_LAUGH) mtype = ZC_SPMOTION_LAUGH;
					else if (pEvent->nKey == ZACTION_CRY) mtype = ZC_SPMOTION_CRY;
					else if (pEvent->nKey == ZACTION_DANCE) mtype = ZC_SPMOTION_DANCE;
					else
						return true;
#endif
					if (ZGetGame())
					{
						ZGetGame()->PostSpMotion(mtype);	// ZPostSpMotion(mtype);
					}
					
				}
				return true;

			case ZACTION_SAVE:
			{
				if (ZGetGame()->GetMatch()->IsTraining() && !ZGetGameClient()->GetMatchStageSetting()->IsTeamPlay()
					&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DEATHMATCH_SOLO)
				{
					PlayerPosHotKey = pMyCharacter->GetPosition();
				}
				return true;
			}

			case ZACTION_TELEPORT:
			{
				if (ZGetGame()->GetMatch()->IsTraining() && !ZGetGameClient()->GetMatchStageSetting()->IsTeamPlay()
					&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DEATHMATCH_SOLO)
				{
					pMyCharacter->SetPosition(PlayerPosHotKey);
				}
				return true;
			}

			case ZACTION_RECORD:
				{
					if ( ZGetGame() && !ZGetGame()->IsReplay())
						ZGetGame()->ToggleRecording();
				}
				return true;
			case ZACTION_SCREENSHOT:
			{

			}
			return true;
			//case ZACTION_MOVING_PICTURE:
			//	{	
			//		if (ZGetGameInterface()->GetBandiCapturer() != NULL)
			//			ZGetGameInterface()->GetBandiCapturer()->ToggleStart();
			//	}
			//	return true;
			case ZACTION_TOGGLE_CHAT:
				{
					if(ZGetCombatInterface()->IsShowUI())
					{ // UIÅä±ÛÀÌ ÄÑÁ® ÀÖÀ»¶§¸¸ Ã¤ÆÃÅä±ÛÀ» Ã³¸®ÇØÁØ´Ù.
						if (ZGetGame())
						{
							ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
							ZGetSoundEngine()->PlaySound("if_error");
							pCombatInterface->ShowChatOutput(!ZGetConfiguration()->GetViewGameChat());
						}
					}
				}
				return true;
			case ZACTION_USE_WEAPON:
			case ZACTION_USE_WEAPON2:
				{
					return true;
				}

			case ZACTION_SENSITIVITY_INC:
			case ZACTION_SENSITIVITY_DEC:
				{
					int nPrev = ZGetConfiguration()->GetMouseSensitivityInInt();
					float senstivity = Z_MOUSE_SENSITIVITY;

					if (pEvent->nKey == ZACTION_SENSITIVITY_INC)
						senstivity += 0.01f;
					else
						senstivity -= 0.01f;

					ZGetConfiguration()->SetMouseSensitivityInFloat(senstivity);

					int nNew = ZGetConfiguration()->GetMouseSensitivityInInt();
					
					ZGetConfiguration()->ReserveSave();
					ZChatOutputMouseSensitivityChanged(nPrev, nNew);
					return true;
			}
			}
		}break;


	case MWM_KEYDOWN:
		{
			ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
			ZGetGame()->m_dwLastInputTime = timeGetTime(); // Custom: Sets last keydown time.

			switch (pEvent->nKey)
			{
			// Custom: Posicion
			/*case VK_NUMPAD0:  
			{
				const auto& CharPos = ZGetGame()->m_pMyCharacter->GetPosition();
				mlog("Pos: %d %d %d", int(CharPos.x), int(CharPos.y), int(CharPos.z));
				const auto& CamDir = ZGetCamera()->GetCurrentDir();
				mlog("Cam dir: %.4f %.4f %.4f", CamDir.x, CamDir.y, CamDir.z);
			}
			break;*/
			// Custom: No Teleport
			//case VK_NUMPAD2: 
			//{
			//	//if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_TELEPORT) && !ZGetGameClient()->GetMatchStageSetting()->IsTeamPlay()
			//	//	&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DEATHMATCH_SOLO)
			//	//{
			//	//	PlayerPosHotKey = pMyCharacter->GetPosition();
			//	//}

			//	if (ZGetGame()->GetMatch()->IsTraining() && !ZGetGameClient()->GetMatchStageSetting()->IsTeamPlay()
			//		&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DEATHMATCH_SOLO)
			//	{
			//		PlayerPosHotKey = pMyCharacter->GetPosition();
			//	}

			//}
			//			   return true;
			//case VK_NUMPAD1: 
			//{
			//	//if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_TELEPORT) && !ZGetGameClient()->GetMatchStageSetting()->IsTeamPlay()
			//	//	&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DEATHMATCH_SOLO)
			//	//{
			//	//	pMyCharacter->SetPosition(PlayerPosHotKey);
			//	//}

			//	if (ZGetGame()->GetMatch()->IsTraining() && !ZGetGameClient()->GetMatchStageSetting()->IsTeamPlay()
			//		&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DEATHMATCH_SOLO)
			//	{
			//		pMyCharacter->SetPosition(PlayerPosHotKey);
			//	}
			//}
			//			   return true;
			case VK_F1:
			case VK_F2:
			case VK_F3:
			case VK_F4:
			case VK_F5:
			case VK_F6:
			case VK_F7:
			case VK_F8:

				if( pEvent->nKey == VK_F1 ) sel = 0;
				else if( pEvent->nKey == VK_F2 ) sel = 1;
				else if( pEvent->nKey == VK_F3 ) sel = 2;
				else if( pEvent->nKey == VK_F4 ) sel = 3;
				else if( pEvent->nKey == VK_F5 ) sel = 4;
				else if( pEvent->nKey == VK_F6 ) sel = 5;
				else if( pEvent->nKey == VK_F7 ) sel = 6;
				else if( pEvent->nKey == VK_F8 ) sel = 7;

				if(ZGetConfiguration()) {

					char* str = ZGetConfiguration()->GetMacro()->GetString( sel );

					if(str) {
						if(ZApplication::GetGameInterface())
							if(ZApplication::GetGameInterface()->GetChat())
							{
								// Custom: Macro spam fix
								if ( MEvent::GetShiftState())
									break;
#ifdef _MACROTIME
								if (timeGetTime() - g_dwMacroTime > 2000) // Custom: Macro Spam Delay here
#else
								if (timeGetTime() - g_dwMacroTime > 0)
#endif
								{
									ZApplication::GetGameInterface()->GetChat()->Input(str);
									g_dwMacroTime = timeGetTime();
								}
								else
								{
									break; //not sure if this was neccessary but put it in.
								}
							}
					}
				}
				return true;

			case 'G':
			{
				if (!MEvent::GetCtrlState()) break;
				char szName[128];
				if (ZGetGame()->GetMatch()->IsTeamPlay())
				{
#ifdef _HITSCOUNT
					if (ZGetGame()->m_pMyCharacter->IsDie()) break;
					int AimPortentage = 0;
					int Shots = ZGetGame()->m_pMyCharacter->m_nShots;
					int Hits = ZGetGame()->m_pMyCharacter->m_nHits;
					if (Hits > 0 && Shots > 0) {
						AimPortentage = (Hits * 100) / Shots;
					}
					else {
						AimPortentage = 0;
					}
#endif
#ifdef _HITSCOUNT
					sprintf(szName, "!(HP: %d / %d AP: %d / %d My targeting percentage: %d%%)", (int)ZGetGame()->m_pMyCharacter->GetHP(), (int)ZGetGame()->m_pMyCharacter->GetMaxHP(), (int)ZGetGame()->m_pMyCharacter->GetAP(), (int)ZGetGame()->m_pMyCharacter->GetMaxAP(), (int)AimPortentage);
#else
					sprintf(szName, "!(HP: %d / %d AP: %d / %d)", (int)ZGetGame()->m_pMyCharacter->GetHP(), (int)ZGetGame()->m_pMyCharacter->GetMaxHP(), (int)ZGetGame()->m_pMyCharacter->GetAP(), (int)ZGetGame()->m_pMyCharacter->GetMaxAP());
#endif
					ZApplication::GetGameInterface()->GetChat()->Input(szName);
					return true;
				}
				else
				{
#ifdef _HITSCOUNT
					if (ZGetGame()->m_pMyCharacter->IsDie()) break;
					int AimPortentage = 0;
					int Shots = ZGetGame()->m_pMyCharacter->m_nShots;
					int Hits = ZGetGame()->m_pMyCharacter->m_nHits;
					if (Hits > 0 && Shots > 0) {
						AimPortentage = (Hits * 100) / Shots;
					}
					else {
						AimPortentage = 0;
					}
#endif
#ifdef _HITSCOUNT
					sprintf(szName, "(HP: %d / %d AP: %d / %d My targeting percentage: %d%%)", (int)ZGetGame()->m_pMyCharacter->GetHP(), (int)ZGetGame()->m_pMyCharacter->GetMaxHP(), (int)ZGetGame()->m_pMyCharacter->GetAP(), (int)ZGetGame()->m_pMyCharacter->GetMaxAP(), (int)AimPortentage);
#else
					sprintf(szName, "(HP: %d / %d AP: %d / %d)", (int)ZGetGame()->m_pMyCharacter->GetHP(), (int)ZGetGame()->m_pMyCharacter->GetMaxHP(), (int)ZGetGame()->m_pMyCharacter->GetAP(), (int)ZGetGame()->m_pMyCharacter->GetMaxAP());
#endif
					ZApplication::GetGameInterface()->GetChat()->Input(szName);
					return true;
				}
			}

			case VK_RETURN:
			case VK_OEM_2:
				{
					if (!ShowCombatInputChat()) return false;
				}
				return true;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':

			case 'A':
			case 'B':
			case 'D':
			case 'E':
			case 'F':
			case 'Y':
			case 'N':
				if (pCombatInterface->GetObserver()->IsVisible())
					pCombatInterface->GetObserver()->OnKeyEvent(pEvent->bCtrl, pEvent->nKey);

				if (ZGetGameClient()->CanVote() || ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->GetShowTargetList() ) 
				{
					ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->VoteInput(pEvent->nKey);
				}
#ifdef _RADIO
				else if (ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->GetRadioCommandList())
				{
					ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->RadioCommandInput(pEvent->nKey);
				}
#endif
				break;
			case VK_ESCAPE:		// ¸Þ´º¸¦ ºÎ¸£°Å³ª kick player¸¦ ÃEÒÇÑ´Ù
				if (ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->GetShowTargetList()) {
					ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->CancelVote();
				} else {
					ZGetGameInterface()->ShowMenu(!ZGetGameInterface()->IsMenuVisible());
					ZGetGameInterface()->Show112Dialog(false);
					// Custom: Enhanced Kick System
					ZGetGameInterface()->ShowKickPlayerDialog(false);
				}

				return true;
#ifdef _PAINTMODE
			case VK_INSERT:
			{
				if (ZGetGame()->GetMatch()->IsPaintBall())
				{
					ZGetGameInterface()->GetCamera()->m_fDist = 30.0f;
				}
				if (ZGetGame()->GetMatch()->IsClassiMode())
				{
					ZGetGameInterface()->GetCamera()->m_fDist = 100.0f;
				}
			}
			break;
			case VK_DELETE:
			{
				if (ZGetGame()->GetMatch()->IsPaintBall())
				{
					ZGetGameInterface()->GetCamera()->m_fDist = CAMERA_DEFAULT_DISTANCE;
				}
				if (ZGetGame()->GetMatch()->IsClassiMode())
				{
					ZGetGameInterface()->GetCamera()->m_fDist = CAMERA_DEFAULT_DISTANCE;
				}
			}
			break;
#endif
#ifdef _SPEC 1
			case VK_HOME:
			{
				bool bClanGame = ZGetGameClient()->IsCWChannel();
				bool bLadderWars = ZGetGameClient()->IsLadderWarsChannel();
				bool bDuelTournament = ZGetGameClient()->IsDTChannel();

                if (!ZGetGame()->GetMatch()->IsTeamPlay() && 
					!ZGetGame()->GetMatch()->IsQuestDrived() && 
					!ZGetGame()->GetMatch()->IsRuleSpy() &&
					ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL &&
					ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT &&
					!ZGetGame()->GetMatch()->IsMapSkill() &&
					!ZGetGame()->GetMatch()->IsQuestChallengue() && 
					!bDuelTournament && !bLadderWars && !bClanGame)
			    {
					if (ZGetGame())
					{
						if (ZGetGameClient()->GetMatchStageSetting()->GetSpecMode() == true)
						{
							if (timeGetTime() - g_dwSpectDelayTime > 10000)
							{
								if (ZGetGameInterface()->GetState() != GUNZ_GAME)
									return true;

									bool IsSpec = ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_SPECTATOR;
									ZPostSpec(!IsSpec);
									g_dwSpectDelayTime = timeGetTime();
							}
						}
					}
			    } 
		    }
			break;
#endif

			case 'M' : 
				if (ZGetGame()->GetMatch()->IsClassiMode())
				{
					if (ZGetGameInterface()->GetCamera()->GetLookMode() == ZCAMERA_MINIMAP)
						ZGetGameInterface()->GetCamera()->SetLookMode(ZCAMERA_DEFAULT);
					else
						ZGetGameInterface()->GetCamera()->SetLookMode(ZCAMERA_MINIMAP);
				}
				if ( ZGetGame()->IsReplay() && pCombatInterface->GetObserver()->IsVisible())
				{
					if(ZGetGameInterface()->GetCamera()->GetLookMode()==ZCAMERA_FREELOOK)
						ZGetGameInterface()->GetCamera()->SetLookMode(ZCAMERA_MINIMAP);
					else
						ZGetGameInterface()->GetCamera()->SetLookMode(ZCAMERA_FREELOOK);
				}
				break;
			case 'T' :
				if(ZGetGame()->m_pMyCharacter->GetTeamID()==MMT_SPECTATOR &&
					ZGetGame()->GetMatch()->IsTeamPlay() && 
					pCombatInterface->GetObserver()->IsVisible()) {
						ZObserver *pObserver = pCombatInterface->GetObserver();
						pObserver->SetType(pObserver->GetType()==ZOM_BLUE ? ZOM_RED : ZOM_BLUE);
						pObserver->ChangeToNextTarget();

				}break;
			case 'H':
				if ( ZGetGame()->IsReplay() && pCombatInterface->GetObserver()->IsVisible())
				{
					if ( ZGetGame()->IsShowReplayInfo())
						ZGetGame()->ShowReplayInfo( false);
					else
						ZGetGame()->ShowReplayInfo( true);
				}
				break;
			case 'J':
				{
					#ifdef _CMD_PROFILE
						if ((pEvent->bCtrl) && (ZIsLaunchDevelop()))
						{
							#ifndef _PUBLISH
								ZGetGameClient()->m_CommandProfiler.Analysis();
							#endif
						}
					#endif
				}
				break;
#ifdef _DEBUG
			case 'K':
				{
					rvector pos = ZGetGame()->m_pMyCharacter->GetPosition();
					pos.x+=1;
					ZGetGame()->m_pMyCharacter->SetPosition(pos);
				}
				break;
			case 'L':
				{
					rvector pos = ZGetGame()->m_pMyCharacter->GetPosition();
					pos.x-=1;
					ZGetGame()->m_pMyCharacter->SetPosition(pos);
				}
				break;
			//case 'J':
			//	{
			//		ZGetGame()->m_pMyCharacter->GetPosition().z = ZGetGame()->m_pMyCharacter->GetPosition().z+1;
			//	}
			//	break;
			//case 'M':
			//	{
			//		ZGetGame()->m_pMyCharacter->GetPosition().z = ZGetGame()->m_pMyCharacter->GetPosition().z-1;
			//	}
			//	break;
			case 'U':
				{
					rvector pos = ZGetGame()->m_pMyCharacter->GetPosition();
					pos.x = -3809;
					pos.y = -1330;
					pos.z = 100;
					ZGetGame()->m_pMyCharacter->SetPosition(pos);
					//ZGetGame()->m_pMyCharacter->GetPosition().x = -3809;
					//ZGetGame()->m_pMyCharacter->GetPosition().y = -1337.5;
					//ZGetGame()->m_pMyCharacter->GetPosition().z = 461;
				}
				break;
#endif
			}
		}
		break;

	case MWM_CHAR:
		{
			ZMatch* pMatch = ZGetGame()->GetMatch();
			if (pMatch->IsTeamPlay()) {
				switch(pEvent->nKey) {
				case '\'':
				case '\"':
					{
						ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
						pCombatInterface->EnableInputChat(true, true);
					}
					return true;
				};
			}

			// for deutsch, spanish keyboard
			if (pEvent->nKey == '/') {
				if (!ShowCombatInputChat()) return false;
			}
		}
		break;

	case MWM_SYSKEYDOWN:
		{
			// alt+a ~ z(65~90)
			if(pEvent->nKey==90){	// Alt+'Z' // ¸ðµEUI °¨Ãß±E.. by kammir 20081020
				ZGetCombatInterface()->SetIsShowUI(!ZGetCombatInterface()->IsShowUI());
				if (ZGetGame())
				{
					ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
					ZGetSoundEngine()->PlaySound("if_error");
					pCombatInterface->ShowChatOutput(ZGetCombatInterface()->IsShowUI());
				}
			}
		}
		break;

	case MWM_MOUSEWHEEL:
		{
			if ( ZGetGame()->IsReplay())
				break;

			// Custom: Bugfix for [FPS]
			if ( ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed( MMOD_FPS ) )
				break;

			int nDelta = pEvent->nDelta;

			if ( (ZGetMyInfo()->IsAdminGrade() && ZGetCombatInterface()->GetObserver()->IsVisible()) ||
				(ZGetGameInterface()->GetScreenDebugger()->IsVisible()) || 
				(!ZGetGameInterface()->m_bViewUI))
			{
				ZCamera* pCamera = ZGetGameInterface()->GetCamera();
				pCamera->m_fDist+=-(float)nDelta;
				pCamera->m_fDist=max(CAMERA_DIST_MIN,pCamera->m_fDist);
				pCamera->m_fDist=min(CAMERA_DIST_MAX,pCamera->m_fDist);
				break;
			}

//			if (nDelta > 0)	ZGetGameInterface()->ChangeWeapon(ZCWT_PREV);
//			else if (nDelta < 0) ZGetGameInterface()->ChangeWeapon(ZCWT_NEXT);
		}break;

	case MWM_MOUSEMOVE:
		{
			if(ZGetGameInterface()->IsCursorEnable()==false)
			{
				return true;
			}
		}
		break;

	} // switch (message)


	return false;
}

bool ZGameInput::ShowCombatInputChat()
{
	if( ZGetCombatInterface()->IsShowUI() && !ZGetCombatInterface()->IsShowResult() )
	{ // UIÅä±ÛÀÌ ÄÑÁ® ÀÖÀ»¶§¸¸ Ã¤ÆÃÅä±ÛÀ» Ã³¸®ÇØÁØ´Ù.
		ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
		if ((pCombatInterface) && (!pCombatInterface->IsChat()) && !ZGetGame()->IsReplay())
		{
			MWidget* pWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("112Confirm");
			if (pWidget && pWidget->IsVisible()) return false;
			// Custom: Enhanced Kick System
			pWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("KickPlayer");
			if (pWidget && pWidget->IsVisible()) return false;
			pCombatInterface->EnableInputChat(true);
		}
	}
	return true;
}

#include "MTextArea.h"

void ZGameInput::Update(float fElapsed)
{

	const ZCharaterStatusBitPacking &uStatus = ZGetGame()->m_pMyCharacter->m_dwStatusBitPackingValue.Ref();
	ZMyCharaterStatusBitPacking & zStatus = ZGetGame()->m_pMyCharacter->m_statusFlags.Ref();

#ifdef _AFKSYSTEM
	if (ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_TRAINING && ZGetGame()->GetMatch()->GetMatchType() !=
		MMATCH_GAMETYPE_TEAM_TRAINING)
	{
		if (timeGetTime() - ZGetGame()->m_dwLastInputTime >= 300000 && timeGetTime() - ZGetGame()->m_dwLastInputTime <= 302000 && !ZGetGame()->IsReplay())
			ZGetGameInterface()->ShowMessage("You've been inactive for 5 minutes. Please use the keyboard or you will be kicked!");
		if (timeGetTime() - ZGetGame()->m_dwLastInputTime >= 600000 && timeGetTime() - ZGetGame()->m_dwLastInputTime <= 602000 && !ZGetGame()->IsReplay())
		{
			ZGetGameInterface()->ReserveLeaveStage();
			ZGetGameInterface()->ShowMessage("You were kicked for being inactive for 10 minutes.");
		}
	}
#endif
	if(RIsActive())
	{
		ZCamera* pCamera = ZGetGameInterface()->GetCamera();
		ZMyCharacter* pMyCharacter = ZGetGame()->m_pMyCharacter;
		if ((!pMyCharacter) || (!pMyCharacter->GetInitialized())) return;

		// Ä¿¼­°¡ ¾ø´Â »óÅÂ¿¡¼­¸¸ Ä«¸Þ¶ó¹× °ÔÀÓÀÔ·ÂÀ» ¹Þ´Â´Ù
		if(!ZGetGameInterface()->IsCursorEnable())
		{
			{
				float fRotateX = 0;
				float fRotateY = 0;

#ifdef _DONOTUSE_DINPUT_MOUSE
				// DINPUT À» »ç¿EÏÁE¾Ê´Â°æ¿E
				int iDeltaX, iDeltaY;

				POINT pt;
				GetCursorPos(&pt);
				ScreenToClient(g_hWnd,&pt);
				iDeltaX = pt.x-RGetScreenWidth()/2;
				iDeltaY = pt.y-RGetScreenHeight()/2;

				float fRotateStep = 0.0005f * Z_MOUSE_SENSITIVITY*10.0f;
				fRotateX = (iDeltaX * fRotateStep);
				fRotateY = (iDeltaY * fRotateStep);

#else
				// ¸¶¿Eº ÀÔ·Â dinput Ã³¸®

				ZGetInput()->GetRotation(&fRotateX,&fRotateY);
#endif

				bool bRotateEnable = !pMyCharacter->IsDirLocked() || ZGetConfiguration()->GetUnlockedDir();

				//bool bRotateEnable=false;
				// TODO : Ä®·Î º®¿¡ ²È¾ÒÀ»¶§ ÇÁ¸®Ä«¸Þ¶ó·Î ¹Ù²ÙÀÚ
				if( !zStatus.m_bSkill && !uStatus.m_bWallJump && !uStatus.m_bWallJump2 && !zStatus.m_bWallHang && 
					!uStatus.m_bTumble && !uStatus.m_bBlast && !uStatus.m_bBlastStand && !uStatus.m_bBlastDrop )
					bRotateEnable=true;
				if (pMyCharacter->IsDie()) bRotateEnable = true;

				if (RIsActive())
				{
					ZCamera* pCamera = ZGetGameInterface()->GetCamera();

					pCamera->m_fAngleX += fRotateY;
					pCamera->m_fAngleZ += fRotateX;

					if (pCamera->GetLookMode() == ZCAMERA_MINIMAP)
					{
						pCamera->m_fAngleX = max(PI_FLOAT / 2 + .1f, pCamera->m_fAngleX);
						pCamera->m_fAngleX = min(PI_FLOAT - 0.1f, pCamera->m_fAngleX);
					}
					else
					{
						if (bRotateEnable)
						{
							pCamera->m_fAngleZ = fmod(pCamera->m_fAngleZ, 2 * PI);
							pCamera->m_fAngleX = fmod(pCamera->m_fAngleX, 2 * PI);

							pCamera->m_fAngleX = max(CAMERA_ANGLEX_MIN, pCamera->m_fAngleX);
							pCamera->m_fAngleX = min(CAMERA_ANGLEX_MAX, pCamera->m_fAngleX);

							lastanglex = pCamera->m_fAngleX;
							lastanglez = pCamera->m_fAngleZ;
						}
						else
						{
							pCamera->m_fAngleX = max(CAMERA_ANGLEX_MIN, pCamera->m_fAngleX);
							pCamera->m_fAngleX = min(CAMERA_ANGLEX_MAX, pCamera->m_fAngleX);

							pCamera->m_fAngleX = max(lastanglex - PI_FLOAT / 4.f, pCamera->m_fAngleX);
							pCamera->m_fAngleX = min(lastanglex + PI_FLOAT / 4.f, pCamera->m_fAngleX);

							pCamera->m_fAngleZ = max(lastanglez - PI_FLOAT / 4.f, pCamera->m_fAngleZ);
							pCamera->m_fAngleZ = min(lastanglez + PI_FLOAT / 4.f, pCamera->m_fAngleZ);

				}
			}

					ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
					if (pCombatInterface && !pCombatInterface->IsChat() &&
						(pCamera->GetLookMode()==ZCAMERA_FREELOOK || pCamera->GetLookMode()==ZCAMERA_MINIMAP))
					{

						rvector right;
						rvector forward=RCameraDirection;
						CrossProduct(&right,rvector(0,0,1),forward);
						Normalize(right);
						const rvector up = rvector(0,0,1);

						rvector accel = rvector(0,0,0);

						if(ZIsActionKeyPressed(ZACTION_FORWARD)==true)	accel+=forward;
						if(ZIsActionKeyPressed(ZACTION_BACK)==true)		accel-=forward;
						if(ZIsActionKeyPressed(ZACTION_LEFT)==true)		accel-=right;
						if(ZIsActionKeyPressed(ZACTION_RIGHT)==true)	accel+=right;
						if(ZIsActionKeyPressed(ZACTION_JUMP)==true)		accel+=up;
						if(ZIsActionKeyPressed(ZACTION_USE_WEAPON)==true)			accel-=up;

						rvector cameraMove = 
							(pCamera->GetLookMode()==ZCAMERA_FREELOOK ? 1000.f : 10000.f )		// ¹Ì´Ï¸Ê¸ðµå´Â »¡¸® ¿òÁ÷ÀÓ
							* fElapsed*accel;

						rvector targetPos = pCamera->GetPosition()+cameraMove;

						// ÇÁ¸®·èÀº Ãæµ¹Ã¼Å©¸¦ ÇÑ´Ù
						if(pCamera->GetLookMode()==ZCAMERA_FREELOOK)
							ZGetGame()->GetWorld()->GetBsp()->CheckWall(pCamera->GetPosition(),targetPos,ZFREEOBSERVER_RADIUS,0.f,RCW_SPHERE);
						else
						// ¹Ì´Ï¸ÊÀº ¹E§³»¿¡ ÀÖ´ÂÁEÃ¼Å©ÇÑ´Ù
						{
							rboundingbox *pbb = &ZGetGame()->GetWorld()->GetBsp()->GetRootNode()->bbTree;
							targetPos.x = max(min(targetPos.x,pbb->maxx),pbb->minx);
							targetPos.y = max(min(targetPos.y,pbb->maxy),pbb->miny);

							ZMiniMap *pMinimap = ZGetGameInterface()->GetMiniMap();
							if(pMinimap)
								targetPos.z = max(min(targetPos.z,pMinimap->GetHeightMax()),pMinimap->GetHeightMin());
							else
								targetPos.z = max(min(targetPos.z,7000),2000);

							
						}

						pCamera->SetPosition(targetPos);

					}
					else if ( !ZGetGame()->IsReplay())
					{
						pMyCharacter->ProcessInput( fElapsed);
					}
				}
			}
			POINT pt={RGetScreenWidth()/2,RGetScreenHeight()/2};
			ClientToScreen(g_hWnd,&pt);
			SetCursorPos(pt.x,pt.y);

			// ´E¬ Å° ÀÔ·Â °Ë»E
			GameCheckSequenceKeyCommand();

		}else
			pMyCharacter->ReleaseButtonState();	// ¸Þ´º°¡ ³ª¿ÔÀ»¶§´Â ¹öÆ°ÀÌ ´­¸®ÁE¾ÊÀº»óÅÂ·Î µ¹·Á³õ´Â´Ù
	}
}


#define MAX_KEY_SEQUENCE_TIME	2.f

void ZGameInput::GameCheckSequenceKeyCommand()
{
	// Ã¶Áö³­ Å° ÀÔ·ÂÀº ÀÏ´Ü Á¦°ÅÇÑ´Ù.
	while(m_ActionKeyHistory.size()>0 && (ZGetGame()->GetTime()-(*m_ActionKeyHistory.begin()).fTime>MAX_KEY_SEQUENCE_TIME))
	{
		m_ActionKeyHistory.erase(m_ActionKeyHistory.begin());
	}

	// Custom: Iterator decrement fix
	if(m_ActionKeyHistory.size())
	{
		for(int ai=0;ai<(int)m_SequenceActions.size();ai++)
		{
			ZKEYSEQUENCEACTION action=m_SequenceActions.at(ai);

			list<ZACTIONKEYITEM>::reverse_iterator itr = m_ActionKeyHistory.rbegin();

			bool bAction = true;

			for( int i = action.nKeyCount - 1; i >= 0; --i )
			{
				// fix
				if( itr == m_ActionKeyHistory.rend() )
				{
					bAction = false;
					break;
				}

				ZACTIONKEYITEM itm=*itr;

				if(i==0)
				{
					if(ZGetGame()->GetTime()-itm.fTime>action.fTotalTime)
					{
						bAction=false;
						break;
					}
				}
				if(itm.nActionKey!=action.pKeys[i].nActionKey || itm.bPressed!=action.pKeys[i].bPressed)
				{
					bAction=false;
					break;
				}
				if(i!=0 && itr==m_ActionKeyHistory.rend()) 
				{
					bAction=false;
					break;
				}

				++itr;
			}

			if(bAction)
			{
				if( m_ActionKeyHistory.size() )
					m_ActionKeyHistory.clear();

				if(ai>=0 && ai<=3)
					ZGetGame()->m_pMyCharacter->OnTumble(ai);
			}

			/*
			list<ZACTIONKEYITEM>::iterator itr=m_ActionKeyHistory.end();
			itr--;
			bool bAction=true;
			for(int i=action.nKeyCount-1;i>=0;i--)
			{
				ZACTIONKEYITEM itm=*itr;
				if(i==0)
				{
					if(ZGetGame()->GetTime()-itm.fTime>action.fTotalTime)
					{
						bAction=false;
						break;
					}
				}
				if(itm.nActionKey!=action.pKeys[i].nActionKey || itm.bPressed!=action.pKeys[i].bPressed)
				{
					bAction=false;
					break;
				}
				if(i!=0 && itr==m_ActionKeyHistory.begin()) 
				{
					bAction=false;
					break;
				}
				itr--;
			}

			if(bAction)
			{
				while(m_ActionKeyHistory.size())
				{
					m_ActionKeyHistory.erase(m_ActionKeyHistory.begin());
				}

				if(ai>=0 && ai<=3)		// ´ýºúÔµ
					ZGetGame()->m_pMyCharacter->OnTumble(ai);
			}
			*/
		}
	}
}
