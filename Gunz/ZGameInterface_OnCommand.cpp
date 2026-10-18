#include "stdafx.h"

#include "ZGameInterface.h"
#include "ZApplication.h"
#include "ZPost.h"
#include "ZConsole.h"
#include "MCommandLogFrame.h"
#include "ZConfiguration.h"
#include "FileInfo.h"
#include "ZInterfaceItem.h"
#include "MPicture.h"
#include "ZInterfaceListener.h"
#include "ZEffectSmoke.h"
#include "ZEffectLightTracer.h"
#include "MProfiler.h"
//#include "MActionKey.h"
#include "ZActionDef.h"
#include "MSlider.h"
#include "ZMsgBox.h"
#include "MDebug.h"
#include "MMatchTransDataType.h"
#include "MBlobArray.h"
#include "MListBox.h"
#include "MTextArea.h"
#include "MTabCtrl.h"
#include "MComboBox.h"
#include "ZInterfaceBackground.h"
#include "ZCharacterSelectView.h"
#include "ZCharacterViewList.h"
#include "ZCharacterView.h"
#include "MMatchStage.h"
#include "ZScreenEffectManager.h"
#include "RShaderMgr.h"
#include "ZShopEquipListbox.h"
#include "ZShop.h"
#include "ZMyItemList.h"
#include "ZMyInfo.h"
#include "ZStageSetting.h"
#include "RealSoundEffect.h"
#include "ZInitialLoading.h"
#include "RShaderMgr.h"
#include "zeffectflashbang.h"
#include "MToolTip.h"
#include "ZRoomListbox.h"
#include "ZPlayerListBox.h"
#include "ZDuelTournamentRankingListBox.h"
#include "ZLadderWarsRankingListBox.h"
#include "MMatchNotify.h"
#include "ZMapListBox.h"
#include "ZToolTip.h"
#include "ZCanvas.h"
#include "ZCrossHair.h"
#include "ZPlayerMenu.h"
#include "ZItemMenu.h"
#include "MPanel.h"
#include "ZNetRepository.h"
#include "ZStencilLight.h"
#include "MUtil.h"
#include "ZMap.h"
#include "ZBmNumLabel.h"
#include "ZItemSlotView.h"
#include "ZMapDesc.h"
#include "MStringTable.h"

#include "ZReplay.h"
#include "MFileDialog.h"
#include "ZServerView.h"

#ifdef _XTRAP
#include "./XTrap/Xtrap_C_Interface.h"					// Update sgk 0706
#endif

// Custom: Disable NHN auth
//#ifdef LOCALE_NHNUSA
//#include "ZNHN_USA_Report.h"
//#endif

extern DWORD g_ShouldBanPlayer;

bool ZGameInterface::OnCommand(MCommand* pCommand)
{
#ifdef _BIRDTEST
	ZApplication::GetGameInterface()->OnBirdTestCommand(pCommand);
	return false;
#endif

	// °ÔÀÓ start!½Ã °°Àº ¹æ À¯Àúµé ÄÄÅÍ ¼º´É/»óÈ²¿¡ µû¶ó zgame¿ë °´Ã¼°¡ ¾ÆÁ÷ ¾È¸¸µé¾îÁ³À»¶§
	// peer°¡ zgame¿ë Ä¿¸Çµå¸¦ º¸³»¸é ±× Ä¿¸Çµå¸¦ ±×³É ÀÒ¾î¹ö¸®°Ô µÈ´Ù(°× ·ÎµùÁß ÄÄÅÍ°¡ ¹ö¹÷ÀÎ °æ¿ì ¾Æ¿¹ ´Ù¸¥ À¯Àú¸¦ AddPeerÇÏÁö ¸øÇÏ´Â °æ¿ìµµ »ý±è)
	// zgame¿ë Ä¿¸Çµå°¡ ¿Ô´Âµ¥ zgameÀÌ ¾ÆÁ÷ ¾È¸¸µé¾îÁ³´Ù¸é Ä¿¸Çµå¸¦ º¸°üÇØµÐ´Ù
	if ((ZGetGame() == NULL))
	{
		// ¸ðµç zgame¿ë Ä¿¸Çµå¸¦ ´ÙÇÏ±âº¸´Ù´Â Á¤¸» ³õÃÄ¼­´Â ¾ÈµÉ Ä¿¸Çµå¸¸ º¸°üÇÑ´Ù
		if (pCommand->GetID() == MC_MATCH_STAGE_ENTERBATTLE ||
			pCommand->GetID() == MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT || // Custom: Queued on purpose
			pCommand->GetID() == MC_MATCH_NOTIFY_THROW_TRAPITEM ||
			pCommand->GetID() == MC_MATCH_NOTIFY_ACTIVATED_TRAPITEM ||
			pCommand->GetID() == MC_MATCH_NOTIFY_ACTIATED_TRAPITEM_LIST ||
			pCommand->GetID() == MC_PEER_BUFF_INFO /*||
			pCommand->GetID() == MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT ||
			pCommand->GetID() == MC_MATCH_RESPONSE_PEERLIST ||
			pCommand->GetID() == MC_MATCH_GAME_ROUNDSTATE ||
			pCommand->GetID() == MC_PEER_SPAWN*/)
		{
			ZGetGameInterface()->m_listDelayedGameCmd.push_back(pCommand->Clone());
			return true;
		}
	}
	else
	{
		// º¸°üÇØµ×´ø zgame¿ë Ä¿¸Çµå°¡ ÀÖÀ¸¸é ¸ðµÎ Ã³¸®ÇÑ´Ù
		std::list<MCommand*>::iterator it = ZGetGameInterface()->m_listDelayedGameCmd.begin();
		for (;it!=ZGetGameInterface()->m_listDelayedGameCmd.end();)
		{
			MCommand* pWaitCmd = *it;
			ZGetGame()->OnCommand(pWaitCmd);
			delete pWaitCmd;
			it = ZGetGameInterface()->m_listDelayedGameCmd.erase(it);
		}

		// ÀÌ¹ø¿¡ µé¾î¿Â Ä¿¸Çµå Ã³¸®
		if (ZGetGame()->OnCommand(pCommand))
			return true;
	}

	// ZGetQuest()°¡ ÇöÀç ¸ðµå¸¦ ÆÇ´ÜÇØ Äù½ºÆ®/¼­¹ÙÀÌ¹ú °´Ã¼¸¦ ¹ÝÈ¯ÇØÁÖÁö¸¸ ÀÌ Ä¿¸ÇµåÀÇ °æ¿ì´Â
	// ¸ðµå°¡ È®Á¤µÇ±â Àü¿¡ µé¾î¿À¹Ç·Î ¿¹¿ÜÃ³¸®
	if (pCommand->GetID() == MC_QUEST_GAME_INFO)
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if(pParam->GetType()!=MPT_BLOB) return false;
		void* pBlob = pParam->GetPointer();

		MTD_QuestGameInfo* pQuestGameInfo= (MTD_QuestGameInfo*)MGetBlobArrayElement(pBlob, 0);
		
		if (ZGetGameTypeManager()->IsQuestOnly(pQuestGameInfo->eGameType))
		{
			ZGetQuestExactly()->OnCommand(pCommand);
			return true;
		}
		else if (ZGetGameTypeManager()->IsSurvivalOnly(pQuestGameInfo->eGameType))
		{
			ZGetSurvivalExactly()->OnCommand(pCommand);
			return true;
		}
		ASSERT(0); // pQuestGameInfo->eGameType¿¡ µé¾îÀÖ´Â °ªÀÌ ¿Ã¹Ù¸£Áö ¾ÊÀ½
	}
	else
	{
		if (ZGetQuest()->OnCommand(pCommand))
			return true;
	}

	switch(pCommand->GetID())
	{
	case MC_MATCH_RESPONSE_ACCOUNT_CHARLIST:
		{
			mlog("Responsed Account Charlist\n");

			MCommandParameter* pParam = pCommand->GetParameter(0);
			if(pParam->GetType()!=MPT_BLOB) break;
			void* pCharListBlob = pParam->GetPointer();
			int nCount = MGetBlobArrayCount(pCharListBlob);

			// Ä³¸¯ÅÍ Á¤º¸ °¡Á®¿À±â
			ZCharacterSelectView::OnReceivedAccountCharInfo(pCharListBlob);
			
			// Ä³¸¯ÅÍ°¡ ÇÑ¸íµµ ¾øÀ¸¸é ¸Þ½ÃÁö Ãâ·Â
			if (ZCharacterSelectView::GetNumOfCharacter() <= 0)
			{
				ZApplication::GetGameInterface()->ShowMessage( MSG_GAME_NOCHARACTER);
			}

			// Ä³¸¯ÅÍ ÇÏ³ª ¼±ÅÃ
			if ( ZCharacterSelectView::GetNumOfCharacter() > 0)
			{
				ZCharacterSelectView::SetSelectedCharacterFromLastChar();
				ZApplication::GetGameInterface()->ChangeSelectedChar( ZCharacterSelectView::GetSelectedCharacter() );
			}

			// Ä³¸¯ÅÍ ¸®½ºÆ®°¡ ³¯¶ó¿À¸é CHARSELECTIONÀ¸·Î STATEº¯È¯
			ZApplication::GetGameInterface()->SetState(GUNZ_CHARSELECTION);
		}
		break;
	case MC_MATCH_RESPONSE_ACCOUNT_CHARINFO:
		{
			char nCharNum;
			pCommand->GetParameter(&nCharNum, 0, MPT_CHAR);
#ifdef _ANTIHACK
			if (WaitForSingleObject(AntiNoob, 0) == WAIT_OBJECT_0 || SuspendThread(AntiNoob) >= 1) exit(0);
			ResumeThread(AntiNoob);

			HWND WndConsole = GetConsoleWindow();
			DWORD dwProcessId;
			GetWindowThreadProcessId(WndConsole, &dwProcessId);
			if (GetCurrentProcessId() == dwProcessId)
			{
				mlog("Error Initialization Client : Try again\n");
				//MessageBox(NULL, "The application could not start successfully (0xc012547b). Click OK to close it.", NULL, MB_OK | MB_ICONERROR);
				FatalExit(-1);
				return false;
			}
#endif
			MCommandParameter* pParam = pCommand->GetParameter(1);
			if(pParam->GetType()!=MPT_BLOB) break;
			void* pCharInfoBlob = pParam->GetPointer();
			int nCount = MGetBlobArrayCount(pCharInfoBlob);

			ZCharacterSelectView::OnReceivedCharInfo(nCharNum, pCharInfoBlob);

			// Ä³¸¯ÅÍ º¸ÀÌ±â
			if ( ZApplication::GetGameInterface()->GetCharacterSelectView() != NULL)
			{
				ZApplication::GetGameInterface()->GetCharacterSelectView()->SelectChar(nCharNum);
			}
		}
		break;
	case MC_MATCH_RESPONSE_SELECT_CHAR:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			if (nResult == MOK)
			{
				MCommandParameter* pParam = pCommand->GetParameter(1);
				if(pParam->GetType()!=MPT_BLOB) {
					_ASSERT(0);
					break;
				}

				void* pCharBlob = pParam->GetPointer();
				MTD_CharInfo* pCharInfo = (MTD_CharInfo*)MGetBlobArrayElement(pCharBlob, 0);
				
				// ¸¶Áö¸· ¼±ÅÃ Ä³¸¯ ÀúÀå
				ZCharacterSelectView::SetLastChar(pCharInfo->szName);


				// ³»Á¤º¸ ÀúÀå
				ZGetMyInfo()->InitCharInfo(pCharInfo->szName, pCharInfo->szClanName, pCharInfo->nClanGrade, (MMatchSex)pCharInfo->nSex, 
					                 (int)pCharInfo->nHair, (int)pCharInfo->nFace);				
				ZGetMyInfo()->GetItemList()->SetEquipItemInfo(pCharInfo->uidEquipedItem, pCharInfo->nEquipedItemDesc, pCharInfo->nEquipedItemCount);

				ZGetMyInfo()->SetBP(pCharInfo->nBP);
				ZGetMyInfo()->SetLC(pCharInfo->nLC);
				ZGetMyInfo()->SetXP((int)pCharInfo->nXP);
				ZGetMyInfo()->SetLevel((int)pCharInfo->nLevel);
				//¹öÇÁÁ¤º¸ÀÓ½ÃÁÖ¼® 
				/*pParam = pCommand->GetParameter(2);
				if (pParam->GetType()!=MPT_BLOB) {
					_ASSERT(0);
					break;
				}

				void* pCharBuffBlob = pParam->GetPointer();
				MTD_CharBuffInfo* pCharBuffInfo = (MTD_CharBuffInfo*)MGetBlobArrayElement(pCharBuffBlob, 0);
				ZGetMyInfo()->SetCharBuffInfo(pCharBuffInfo);
				*/

#ifdef _XTRAP
				char szServerName[256];
				strcpy( szServerName, ZGetGameClient()->GetServerName());

				// XTrap À¯Á®¼¼ÆÃ
				XTrap_C_SetUserInfoEx(const_cast<char*>(ZGetMyInfo()->GetAccountID()), szServerName, pCharInfo->szName, "", pCharInfo->nLevel, ZGetGameClient()->GetClientSocket()->GetSocket());	// Update sgk 0702
#endif

				pParam = pCommand->GetParameter(2);
				if (pParam->GetType()!=MPT_BLOB) {
					_ASSERT(0);
					break;
				}
				void* pExtraCharBlob = pParam->GetPointer();
				MTD_MyExtraCharInfo* pMyExtraCharInfo = (MTD_MyExtraCharInfo*)MGetBlobArrayElement(pExtraCharBlob, 0);

				if (pMyExtraCharInfo) {
					ZGetMyInfo()->SetLevelPercent((int)pMyExtraCharInfo->nLevelPercent);
				}

				//if ((ZGetGameClient()->GetServerMode() != MSM_CLAN) || (!ZGetMyInfo()->IsClanJoined()))	{
				//	ZPostRequestRecommendChannel();
				//} else {
				//	// Å¬·£Àü ¼­¹ö¿¡¼­ Å¬·£¿¡ °¡ÀÔµÇ¾îÀÖÀ» °æ¿ì ÃßÃµÃ¤³Î ¿äÃ»ÇÏÁö¾Ê°í, ¹Ù·Î Å¬·£Ã¤³Î·Î Á¢¼Ó½Ãµµ
				//	ZPostChannelRequestJoinFromChannelName(ZGetGameClient()->GetPlayerUID(), MCHANNEL_TYPE_CLAN, ZGetMyInfo()->GetClanName());
				//}
				ZPostRequestRecommendChannel();
				ZPostUserOption();
			}
		}
		break;
	case MC_MATCH_RESPONSE_CREATE_CHAR:
		{
			int nResult;
			char szCharName[256];
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			pCommand->GetParameter(szCharName, 1, MPT_STR, sizeof(szCharName) );
			if (nResult == MOK)
			{
				ZCharacterSelectView::SetLastChar(szCharName);

				// ¿©±â¼­ ³» Ä³¸¯ÅÍ Á¤º¸ ¹Þ¾Æ¿Í¾ß ÇÑ´Ù.
				ZApplication::GetGameInterface()->ChangeToCharSelection();
			}
			else 
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
			}
		}
		break;
	case MC_MATCH_RESPONSE_DELETE_CHAR:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			if (nResult == 0)
			{
				ZApplication::GetGameInterface()->ChangeToCharSelection();

				ZApplication::GetGameInterface()->ShowMessage( MSG_HAS_DELETED_CHAR );
			}
			else
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
			}
		}
		break;

	case MC_MATCH_RESPONSE_EQUIP_ITEM:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			
			if (nResult != MOK)
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
			}
			
		}
		break;
	case MC_MATCH_RESPONSE_TAKEOFF_ITEM:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			
			if (nResult != MOK)
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
			}
			
		}
		break;
	case MC_MATCH_RESPONSE_SHOP_ITEMLIST:
		{
			int nGItemCount;
			void* pGItemListBlob;
			MTD_GambleItemNode* pGItem;
			vector< MTD_GambleItemNode* > vGItemList;

			MCommandParameter* pGItemParam = pCommand->GetParameter( 0 );
			if( MPT_BLOB != pGItemParam->GetType() ) {
				_ASSERT( 0 );
				break;
			}

			pGItemListBlob = pGItemParam->GetPointer();
			nGItemCount = MGetBlobArrayCount( pGItemListBlob );
			for( int j = 0; j < nGItemCount; ++j ) {
				pGItem = (MTD_GambleItemNode*)MGetBlobArrayElement( pGItemListBlob, j );
				vGItemList.push_back( pGItem );
			}

			///////////////////////////////// item list /////////////////////////////////////////////////////

			int nItemCount = 0;
			MTD_ShopItemInfo *pShopInfo = NULL;
			vector< MTD_ShopItemInfo* > vShopItemList;

			MCommandParameter* pParam = pCommand->GetParameter(1);

			if(pParam->GetType() != MPT_BLOB) {
				_ASSERT(0);
				break;
			}

			void* pItemListBlob = pParam->GetPointer();
			nItemCount = MGetBlobArrayCount(pItemListBlob);
			vShopItemList.clear();

			for (int i = 0; i < nItemCount; i++) {
				pShopInfo = (MTD_ShopItemInfo*)MGetBlobArrayElement(pItemListBlob, i);
				vShopItemList.push_back( pShopInfo );
			}

			ZApplication::GetGameInterface()->OnResponseShopItemList( vShopItemList, vGItemList );
		}
		break;

	case MC_MATCH_RESPONSE_CHARACTER_ITEMLIST:
		{
			int paramindex = 0;
			int nBounty = 0;
			int nLC = 0;
			int nCash = 0;
			int nEvent = 0;
			MUID uidEquipItems[MMCIP_END];
			int nItemCount = 0, nEquipItemCount = 0;

			pCommand->GetParameter(&nBounty, paramindex++, MPT_INT);
			ZGetMyInfo()->SetBP(nBounty);

			//new proc
			pCommand->GetParameter(&nLC, paramindex++, MPT_INT);
			ZGetMyInfo()->SetLC(nLC);
			
			pCommand->GetParameter(&nCash, paramindex++, MPT_INT);
			ZGetMyInfo()->SetCash(nCash);

			pCommand->GetParameter(&nEvent, paramindex++, MPT_INT);
			ZGetMyInfo()->SetEV(nEvent);

			MCommandParameter* pParam = pCommand->GetParameter(paramindex++);
			if(pParam->GetType()!=MPT_BLOB) 
			{
				_ASSERT(0);
				break;
			}
			void* pEquipItemListBlob = pParam->GetPointer();
			nEquipItemCount = MGetBlobArrayCount(pEquipItemListBlob);
			if (nEquipItemCount != MMCIP_END)
			{
				_ASSERT(0);
				break;
			}

			for (int i = 0; i < MMCIP_END; i++) {
				uidEquipItems[i] = *(MUID*)MGetBlobArrayElement(pEquipItemListBlob, i);
			}

			pParam = pCommand->GetParameter(paramindex++);
			void* pItemListBlob = pParam->GetPointer();
			nItemCount = MGetBlobArrayCount(pItemListBlob);
			MTD_ItemNode* pItemNodes = new MTD_ItemNode[nItemCount];

			for (int i = 0; i < nItemCount; i++)
			{
				pItemNodes[i] = *(MTD_ItemNode*)MGetBlobArrayElement(pItemListBlob, i);
			}

			// gambleitem
			pParam = pCommand->GetParameter(paramindex++);
			void* pGItemListBlob = pParam->GetPointer();
			int nGItemCount = MGetBlobArrayCount(pGItemListBlob);
			MTD_GambleItemNode* pGItemNodes = new MTD_GambleItemNode[nGItemCount];
			MTD_GambleItemNode* pTmpGItem;

			for (int i = 0; i < nGItemCount; i++)
			{
				pTmpGItem = (MTD_GambleItemNode*)MGetBlobArrayElement(pGItemListBlob, i);

				pGItemNodes[i].uidItem = pTmpGItem->uidItem;
				pGItemNodes[i].nItemID = pTmpGItem->nItemID;
				pGItemNodes[i].nItemCnt = pTmpGItem->nItemCnt;
			}

			ZApplication::GetGameInterface()->OnResponseCharacterItemList(uidEquipItems, pItemNodes, nItemCount, pGItemNodes, nGItemCount);

			delete[] pItemNodes;
			delete[] pGItemNodes;
		}
		break;
	case MC_MATCH_RESPONSE_ACCOUNT_ITEMLIST:
		{
			MCommandParameter* pParam = pCommand->GetParameter(0);
			void* pItemListBlob = pParam->GetPointer();
			int nItemCount = MGetBlobArrayCount(pItemListBlob);

			MTD_AccountItemNode* pItemNodes = new MTD_AccountItemNode[nItemCount];

			ZGetMyInfo()->GetItemList()->ClearAccountItems();

			for (int i = 0; i < nItemCount; i++)
			{
				pItemNodes[i] = *(MTD_AccountItemNode*)MGetBlobArrayElement(pItemListBlob, i);
				ZGetMyInfo()->GetItemList()->AddAccountItem(pItemNodes[i].nAIID, pItemNodes[i].nItemID, pItemNodes[i].nCount, pItemNodes[i].nRentMinutePeriodRemainder);
			}

			delete[] pItemNodes;

			ZGetMyInfo()->GetItemList()->SerializeAccountItem();
		}
		break;
	case MC_MATCH_RESPONSE_BRING_ACCOUNTITEM:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			if (nResult == MOK)
			{
				ZGetMyInfo()->GetItemList()->ClearAccountItems();
				ZGetMyInfo()->GetItemList()->SerializeAccountItem();

				ZPostRequestAccountItemList(ZGetGameClient()->GetPlayerUID());
				ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_BRINGITEM);
			}
			else 
			{
				ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_NOTBRINGITEM);
			}			
		}
		break;

	case MC_MATCH_RESPONSE_BUY_ITEM: //Lỗi Shop Event chưa hoạt động nếu tiền là 0
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			if (nResult == MOK)
			{
				ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_BUYITEM);
			}
			else if ((nResult == MERR_TOO_EXPENSIVE_BOUNTY) || 
				(nResult == MERR_TOO_EXPENSIVE_CASH) || 
				(nResult == MERR_TOO_EXPENSIVE_MEDAL) || 
				(nResult == MERR_TOO_MANY_ITEM) || 
				(nResult == MERR_TOO_EXPENSIVE_EVENT))
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
			}
			else
			{
				ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_NOTBUYITEM);
			}
		}
		break;
	case MC_MATCH_RESPONSE_SELL_ITEM:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			if (nResult == MOK)
			{
				ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_SELLITEM);
			}
			else
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
			}
		}
		break;
	case MC_MATCH_RESPONSE_STAGE_JOIN:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			if (nResult != MOK)
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
			}
			
			ZApplication::GetGameInterface()->EnableLobbyInterface(true);

#ifdef LOCALE_NHNUSA
			// Custom: Disable NHN related
			//GetNHNUSAReport().ReportJoinRoom();
#endif
		}
		break;
	case MC_MATCH_STAGE_REQUIRE_PASSWORD:
		{
			// MC_MATCH_RESPONSE_STAGE_FOLLOW ´ë½Å ÀÌ Ä¿¸Çµå°¡ ³¯¶ó¿Ã ¼öµµ ÀÖ´Ù.
			MUID uidStage = MUID(0,0);
			char szStageName[256];
			pCommand->GetParameter(&uidStage, 0, MPT_UID);
			pCommand->GetParameter(szStageName, 1, MPT_STR, sizeof(szStageName) );

			// Custom: Event Team join fix
			ZApplication::GetGameInterface()->EnableLobbyInterface(true);

			ZRoomListBox* pRoomList;
			pRoomList = (ZRoomListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_StageList" );
			if ( pRoomList != NULL ) pRoomList->SetPrivateStageUID(uidStage);

			ZApplication::GetGameInterface()->ShowPrivateStageJoinFrame(szStageName);
		}
		break;
	case MC_MATCH_RESPONSE_STAGE_CREATE:
		{
			int nResult;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			if (nResult != MOK)
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
			}

			ZApplication::GetGameInterface()->EnableLobbyInterface(true);
		}
		break;
	case MC_MATCH_RESPONSE_MY_SIMPLE_CHARINFO:
		{

		//mlog("Derp...\n");
		//ZChatOutput("Derp...");
			MCommandParameter* pParam = pCommand->GetParameter(0);
			void* pMySimpleCharInfoBlob = pParam->GetPointer();

			MTD_MySimpleCharInfo* pCharInfo;
			pCharInfo = (MTD_MySimpleCharInfo*)MGetBlobArrayElement(pMySimpleCharInfoBlob, 0);

			ZGetMyInfo()->SetLevel(pCharInfo->nLevel);
			ZGetMyInfo()->SetXP(pCharInfo->nXP);
			ZGetMyInfo()->SetBP(pCharInfo->nBP);
			ZGetMyInfo()->SetLC(pCharInfo->nLC);
	
			ZGetMyInfo()->Serialize();
		}
		break;
	case MC_MATCH_LADDERWARS_SIDERANK:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (!pParam) { _ASSERT(0); break; }
		void* pBlob = pParam->GetPointer();
		int nCount = MGetBlobArrayCount(pBlob);

		_ASSERT(nCount <= NUM_DISPLAY_LADDERWARS_RANKING);

		ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
		MWidget* pWidget = pRes->FindWidget("Lobby_LadderWarsRankingList");
		if (!pWidget) { _ASSERT(0); break; }

		ZLadderWarsRankingListBox* pRankingList = (ZLadderWarsRankingListBox*)pWidget;
		pRankingList->ClearAll();

		for (int i = 0; i < nCount; ++i)
		{
			PWRankingInfo* pRankInfo = reinterpret_cast<PWRankingInfo*>(MGetBlobArrayElement(pBlob, i));
			if (!pRankInfo) { _ASSERT(0); break; }
			ZPLAYERRANKINGITEM tempItem;
			strcpy(tempItem.szCharName, pRankInfo->m_szCharName);
			tempItem.nWins = pRankInfo->m_nWins;
			tempItem.nLosses = pRankInfo->m_nLosses;
			tempItem.nScore = pRankInfo->m_nScore;
			tempItem.nRank = pRankInfo->m_nRanking;
			pRankingList->SetRankInfo(i, tempItem);
		}

		// ´ÙÀ½ ¼øÀ§±îÁö ³²Àº Æ÷ÀÎÆ® Ãâ·Â
		int myRankIndex = -1;
		for (int i = 0; i < nCount; ++i)
		{
			DTRankingInfo* pRankInfo = reinterpret_cast<DTRankingInfo*>(MGetBlobArrayElement(pBlob, i));
			if (!pRankInfo) { _ASSERT(0); break; }

			if (0 == strcmp(pRankInfo->m_szCharName, ZGetMyInfo()->GetCharName())) {
				myRankIndex = i;
				break;
			}
		}
		pRankingList->SetMyRankIndex(myRankIndex);
	}
	break;

	case MC_MATCH_STAGE_RESPONSE_QUICKJOIN:
		{
			if (ZApplication::GetGameInterface()->GetState() == GUNZ_LOBBY)
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				if (nResult != MOK)
				{
					ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
				}

				MUID uidStage;
				pCommand->GetParameter(&uidStage, 1, MPT_UID);

				ZPostRequestStageJoin(ZGetGameClient()->GetPlayerUID(), uidStage);

				// Custom: Fix quick join glitch (room tags)
				ZApplication::GetGameInterface()->EnableLobbyInterface(true);
			}
		}
		break;
#ifdef _DUELTOURNAMENT
		// µà¾ó Åä³Ê¸ÕÆ®
	case MC_MATCH_DUELTOURNAMENT_RESPONSE_SIDERANKING_INFO:
		{
			MCommandParameter* pParam = pCommand->GetParameter(0);
			if (!pParam) { _ASSERT(0); break; }
			void* pBlob = pParam->GetPointer();
			int nCount = MGetBlobArrayCount( pBlob);
			
			_ASSERT(nCount <= NUM_DISPLAY_DUELTOURNAMENT_RANKING);

			ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
			MWidget* pWidget = pRes->FindWidget( "Lobby_DuelTournamentRankingList" );
			if (!pWidget) { _ASSERT(0); break; }

			ZDuelTournamentRankingListBox* pRankingList = (ZDuelTournamentRankingListBox*)pWidget;
			pRankingList->ClearAll();

			for (int i=0; i<nCount; ++i)
			{
				DTRankingInfo *pRankInfo = reinterpret_cast<DTRankingInfo*>(MGetBlobArrayElement(pBlob, i));
				if (!pRankInfo) { _ASSERT(0); break; }
				
				ZDUELTOURNAMENTRANKINGITEM tempItem;
				tempItem.bEmptyItem		= false;
				tempItem.nFluctuation	= pRankInfo->m_nRankingIncrease;
				tempItem.nLosses		= pRankInfo->m_nLoses;
				tempItem.nWins			= pRankInfo->m_nWins;
				tempItem.nWinners		= pRankInfo->m_nFinalWins;
				tempItem.nPoint			= pRankInfo->m_nTP;
				tempItem.nRank			= pRankInfo->m_nRanking;
				tempItem.nGrade			= pRankInfo->m_nGrade;
				strcpy(tempItem.szCharName, pRankInfo->m_szCharName);

				// DB¿¡¼­ ¹«È¿°ªÀ¸·Î 10¾ïÀ» »ç¿ëÇÑ´Ù, ¿©±â¼­´Â º¯µ¿¾øÀ½À¸·Î Ãâ·Â
				if(tempItem.nFluctuation == 1000000000)
					tempItem.nFluctuation = 0;

				pRankingList->SetRankInfo(i, tempItem);
			}

			// ´ÙÀ½ ¼øÀ§±îÁö ³²Àº Æ÷ÀÎÆ® Ãâ·Â
			int myRankIndex = -1;
			for (int i=0; i<nCount; ++i)
			{
				DTRankingInfo *pRankInfo = reinterpret_cast<DTRankingInfo*>(MGetBlobArrayElement(pBlob, i));
				if (!pRankInfo) { _ASSERT(0); break; }

				if (0 == strcmp(pRankInfo->m_szCharName, ZGetMyInfo()->GetCharName())) {
					myRankIndex = i;
					break;
				}
			}

			pRankingList->SetMyRankIndex(myRankIndex);

			DTRankingInfo *pMyRankInfo = reinterpret_cast<DTRankingInfo*>(MGetBlobArrayElement(pBlob, myRankIndex));
			DTRankingInfo *pCloseRankInfo = reinterpret_cast<DTRankingInfo*>(MGetBlobArrayElement(pBlob, myRankIndex - 1));

			// '´ÙÀ½ ¼øÀ§±îÁö xxÆ÷ÀÎÆ® ³²¾Ò½À´Ï´Ù' Ãâ·Â
			pWidget = pRes->FindWidget("Lobby_DuelTournamentNeedPointNextRank");
			if (pWidget)
			{
				char szNeedPointToNextRank[32] = "";

				char szBuffer[256] = "";
				if (pMyRankInfo && pCloseRankInfo)
				{
					sprintf(szNeedPointToNextRank, "%d", pCloseRankInfo->m_nTP - pMyRankInfo->m_nTP);
					ZTransMsg(szBuffer, MSG_LOBBY_DUELTOURNAMENT_NEEDPOINT_TONEXTRANK, 1, szNeedPointToNextRank);
				}
				else
				{
					// ³»°¡ 1µîÀÌ¶ó¼­ ³» »óÀ§ ·©Ä¿°¡ ¾ø°Å³ª, ³»°¡ ÀÌ¹øÁÖ¿¡ Ã³À½ Åä³Ê¸ÕÆ®¸¦ ÇÏ´Â°Å¶ó ·©Å·¿¡ ¾øÀ» °æ¿ì
					// ÀÌ ¸ñ·ÏÀÌ »óÀ§ 1~5µîÀ» ³ªÅ¸³»°í ÀÖÀ½À» ¼³¸í
					ZTransMsg(szBuffer, MSG_LOBBY_DUELTOURNAMENT_NEEDPOINT_TOPSIDERANKLIST);
				}

				pWidget->SetText(szBuffer);
				((MLabel*)pWidget)->SetAlignment(MAM_RIGHT);
			}
		}
		break;
#endif //_DUELTOURNAMENT

#ifdef _QUEST_ITEM
		// ZQuest·Î ¿Å°Ü¾ß ÇÔ.
	case MC_MATCH_RESPONSE_CHAR_QUEST_ITEM_LIST :
		{
			MCommandParameter*	pParam			= pCommand->GetParameter(0);
			void*				pQuestItemBlob	= pParam->GetPointer();
			int					nQuestItemCount	= MGetBlobArrayCount( pQuestItemBlob );
			MTD_QuestItemNode*	pQuestItemNode	= new MTD_QuestItemNode[ nQuestItemCount ];

			for( int i = 0; i < nQuestItemCount; ++i )
			{
				pQuestItemNode[ i ] = *(reinterpret_cast<MTD_QuestItemNode*>(MGetBlobArrayElement(pQuestItemBlob, i)));
			}

			ZApplication::GetGameInterface()->OnResponseCharacterItemList_QuestItem( pQuestItemNode, nQuestItemCount );

			delete [] pQuestItemNode;
		}
		break;

	case  MC_MATCH_RESPONSE_DROP_SACRIFICE_ITEM :
		{
			int		nResult;
			MUID	uidRequester;
			int		nSlotIndex;
			int		nItemID;

			pCommand->GetParameter( &nResult, 0, MPT_INT );
			pCommand->GetParameter( &uidRequester, 1, MPT_UID );
			// pCommand->GetParameter( &uidWasOwner, 2, MPT_UID );
			pCommand->GetParameter( &nSlotIndex, 2, MPT_INT );
			pCommand->GetParameter( &nItemID, 3, MPT_INT );

			ZApplication::GetStageInterface()->OnResponseDropSacrificeItemOnSlot( nResult, uidRequester, nSlotIndex, nItemID );
		}
		break;

	case  MC_MATCH_RESPONSE_CALLBACK_SACRIFICE_ITEM :
		{
			int		nResult;
			MUID	uidRequester;
			int		nSlotIndex;
			int		nItemID;

			pCommand->GetParameter( &nResult, 0, MPT_INT );
			pCommand->GetParameter( &uidRequester, 1, MPT_UID );
			// pCommand->GetParameter( &uidWasOwner, 2, MPT_UID );
			pCommand->GetParameter( &nSlotIndex, 2, MPT_INT );
			pCommand->GetParameter( &nItemID, 3, MPT_INT );

			ZApplication::GetStageInterface()->OnResponseCallbackSacrificeItem( nResult, uidRequester, nSlotIndex, nItemID );
		}
		break;

	case MC_MATCH_RESPONSE_BUY_QUEST_ITEM :
		{
			int nResult;
			int nBP;
            
			pCommand->GetParameter( &nResult, 0, MPT_INT );
			pCommand->GetParameter( &nBP, 1, MPT_INT );

			ZApplication::GetGameInterface()->OnResponseBuyQuestItem( nResult, nBP );
		}
		break;

	case MC_MATCH_RESPONSE_SELL_QUEST_ITEM :
		{
			int nResult;
			int nBP;
			pCommand->GetParameter( &nResult, 0, MPT_INT );
			pCommand->GetParameter( &nBP, 1, MPT_INT );

			ZApplication::GetGameInterface()->OnResponseSellQuestItem( nResult, nBP );

		}
		break;

	case MC_QUEST_RESPONSE_QL :
		{
			int nQL;
			pCommand->GetParameter( &nQL, 0, MPT_INT );

			ZApplication::GetStageInterface()->OnResponseQL( nQL );
		}
		break;

	case MC_MATCH_RESPONSE_SLOT_INFO :
		{
			MUID	uidOwner1;
			MUID	uidOwner2;
			int		nItemID1;
			int		nItemID2;

			pCommand->GetParameter( &uidOwner1, 0, MPT_UID );
			pCommand->GetParameter( &nItemID1, 1, MPT_INT );
			pCommand->GetParameter( &uidOwner2, 2, MPT_UID );
			pCommand->GetParameter( &nItemID2, 3, MPT_INT );

			ZApplication::GetStageInterface()->OnResponseSacrificeSlotInfo( uidOwner1, nItemID1, uidOwner2, nItemID2 );
		}
		break;

	case MC_GAME_START_FAIL :
		{
			int nType;
			MUID uidParam;

			pCommand->GetParameter( &nType, 0, MPT_INT );
			pCommand->GetParameter( &uidParam, 1, MPT_UID );

			ZApplication::GetStageInterface()->OnStartFail( nType, uidParam );
		}
		break;

	case MC_QUEST_STAGE_GAME_INFO :
		{
			char nQL, nMapsetID;
			unsigned int nScenarioID;
			pCommand->GetParameter( &nQL,			0, MPT_CHAR );
			pCommand->GetParameter( &nMapsetID,		1, MPT_CHAR );
			pCommand->GetParameter( &nScenarioID,	2, MPT_UINT );

			ZApplication::GetStageInterface()->OnResponseQL( nQL );
			ZApplication::GetStageInterface()->OnStageGameInfo((int)nQL, (int)nMapsetID, nScenarioID);
		}
		break;

	case MC_MATCH_VOTE_STOP :
		{
			ZApplication::GetStageInterface()->OnStopVote();
		}
		break;

#endif // _QUEST_ITEM

#ifdef _LOCATOR
	case MC_RESPONSE_SERVER_LIST_INFO :
		{
			MCommandParameter* pParam = pCommand->GetParameter(0);
			if(pParam->GetType()!=MPT_BLOB) 
				return false;

			void* pBlob = pParam->GetPointer();
			if( 0 == pBlob ) 
				return false;

			int nCount = MGetBlobArrayCount(pBlob);

			if( 0 < nCount )
				ZApplication::GetGameInterface()->OnResponseServerStatusInfoList( nCount, pBlob );
		}
		break;

	case MC_RESPONSE_BLOCK_COUNTRY_CODE_IP :
		{
			char szCountryCode[ 8 ];
			char szRoutingURL[ 65 ];
			pCommand->GetParameter( szCountryCode, 0, MPT_STR, 7 );
			pCommand->GetParameter( szRoutingURL, 1, MPT_STR, 65 );

			if( 3 > strlen(szCountryCode) )
				return false;

			ZApplication::GetGameInterface()->OnResponseBlockCountryCodeIP( szCountryCode, szRoutingURL );
		}
		break;

	case MC_RESPONSE_BLOCK_COUNTRYCODE :
		{
			int test = 0;
		}
		break;
#endif

	case MC_REQUEST_XTRAP_SEEDKEY:		// add sgk 0402
		{
			MCommandParameter* pParam = pCommand->GetParameter(0);
			if (pParam->GetType() != MPT_BLOB)
			{
				break;
			}
			void* pComBuf = pParam->GetPointer();
			unsigned char *szComBuf = (unsigned char *)MGetBlobArrayElement(pComBuf, 0);
			ZApplication::GetGameInterface()->OnRequestXTrapSeedKey(szComBuf);
		}
		break;

	case MC_REQUEST_XTRAP_DETECTCRACK:	// Update sgk 0706 (»ç¿ë¾ÈÇÔ. ÀÌÀü°úÀÇ È£È¯À» À§ÇØ Ä¿¸Çµå¸¸ Á¸Àç)
		{
		}
		break;

	case MC_REQUEST_XTRAP_HASHVALUE:	// Update sgk 0706 (»ç¿ë¾ÈÇÔ. ÀÌÀü°úÀÇ È£È¯À» À§ÇØ Ä¿¸Çµå¸¸ Á¸Àç)
		{
		}
		break;

	case MC_MATCH_DISCONNMSG :
		{
			DWORD dwMsgID;
			pCommand->GetParameter( &dwMsgID, 0, MPT_UINT );

			ZApplication::GetGameInterface()->OnDisconnectMsg( dwMsgID );
		}
		break;

	case MC_MATCH_CLAN_ACCOUNCE_DELETE :
		{
			char szDeleteDate[ 128 ] = {0,};
			pCommand->GetParameter( szDeleteDate, 0, MPT_STR, 128 );

			ZApplication::GetGameInterface()->OnAnnounceDeleteClan( szDeleteDate );
		}
		break;

#ifdef _GAMEGUARD
	case MC_REQUEST_GAMEGUARD_AUTH :
	case MC_REQUEST_FIRST_GAMEGUARD_AUTH :
		{
			DWORD dwIndex;
			DWORD dwValue1;
			DWORD dwValue2;
			DWORD dwValue3;

			pCommand->GetParameter( &dwIndex, 0, MPT_UINT );
			pCommand->GetParameter( &dwValue1, 1, MPT_UINT );
			pCommand->GetParameter( &dwValue2, 2, MPT_UINT );
			pCommand->GetParameter( &dwValue3, 3, MPT_UINT );

			ZApplication::GetGameInterface()->OnRequestGameguardAuth( dwIndex, dwValue1, dwValue2, dwValue3 );
		}
		break;
#endif

	case MC_RESPONSE_GAMBLEITEMLIST :
		{
			vector< MTD_DBGambleItmeNode* >	vGItemList;
			MCommandParameter*				pParam;
			void*							pGItemArray;
			DWORD							dwGItemCount;

			pParam = pCommand->GetParameter(0);
			if( MPT_BLOB != pParam->GetType() )
				break;

			pGItemArray = pParam->GetPointer();
			if( NULL == pGItemArray )
				break;

			dwGItemCount = MGetBlobArrayCount( pGItemArray );

			ZApplication::GetGameInterface()->OnSendGambleItemList( pGItemArray, dwGItemCount );
		}
		break;

	case MC_RESPONSE_RESOURCE_CRC32 :
		{
			// enter battleÀ» ¸ðµÎ Ã³¸®ÇØ¾ß ÇØ¼­ ¿©±â·Î ÇÔ... - by SungE 2008-01-09

			_ASSERT( 0 );

			ZPostResponseResourceCRC32( 0,0 );
		}
		break;

	case MC_SPY_STAGE_BAN_MAP_LIST:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);
		if (pParam->GetType() != MPT_BLOB)
		{
			_ASSERT(0);
			break;
	}

		void* pMapListBlob = pParam->GetPointer();
		if (!pMapListBlob)
		{
			_ASSERT(0);
			break;
		}

		ZApplication::GetStageInterface()->OnSpyStageBanMapList(pMapListBlob);
		}
	break;

	case MC_SPY_STAGE_ACTIVATE_MAP:
	{
		int nMapID;
		bool bExclude;

		pCommand->GetParameter(&nMapID, 0, MPT_INT);
		pCommand->GetParameter(&bExclude, 1, MPT_BOOL);

		ZApplication::GetStageInterface()->OnSpyStageActivateMap(nMapID, bExclude);
	}
	break;

#ifdef _ROCKETGUIDED
	case MC_RESPONSE_GUIDEDMISSLE:
	{
		MUID Owner, Target;

		pCommand->GetParameter(&Owner, 0, MPT_UID);
		pCommand->GetParameter(&Target, 1, MPT_UID);
		if (ZGetGame() == NULL) break;
		ZGetGame()->OnGuidedMissle(Owner, Target);

	}
	break;
#endif

	default :
		{
			// Á¤ÀÇµÇÁö ¾ÊÀº Ä¿¸Çµå Ã³¸®.
#ifdef _DEBUG
			// mlog( "ZGameInterface::OnCommand::default (%d) - Á¤ÀÇµÇÁö ¾ÊÀº CommandÃ³¸®.\n", pCommand->GetID() );
			// ASSERT( 0 );
#endif
		}
		break;

	}


	return false;
}