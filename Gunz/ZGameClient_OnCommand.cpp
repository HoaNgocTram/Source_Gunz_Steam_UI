#include "stdafx.h"
#include "ZGameClient.h"



/// ±ÍÂú¾Æ¼­ ±×³É ±Ü¾î¿ÔÀ½... -   _-);;
#include <winsock2.h>
#include "MErrorTable.h"
#include "ZConfiguration.h"
#include "ZEventManager.h"
#include "MSharedCommandTable.h"
#include "ZConsole.h"
#include "MCommandLogFrame.h"
#include "ZIDLResource.h"
#include "MBlobArray.h"
#include "ZInterface.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "MMatchGlobal.h"
#include "MMatchChannel.h"
#include "MMatchStage.h"
#include "ZCommandTable.h"
#include "ZPost.h"
#include "ZPostLocal.h"
#include "MMatchNotify.h"
#include "ZMatch.h"
#include "MComboBox.h"
#include "MTextArea.h"
#include "ZCharacterViewList.h"
#include "ZCharacterView.h"
#include "MDebug.h"
#include "ZScreenEffectManager.h"
#include "ZNetmarble.h"
#include "ZRoomListBox.h"
#include "ZPlayerListBox.h"
#include "ZChat.h"
#include "ZWorldItem.h"
#include "ZChannelRule.h"
#include "ZNetRepository.h"
#include "ZMyInfo.h"
#include "MToolTip.h"
#include "ZColorTable.h"
#include "ZClan.h"
#include "ZSecurity.h"
#include "ZItemDesc.h"
#include "ZCharacterSelectView.h"
#include "ZChannelListItem.h"
#include "ZCombatInterface.h"
#include "ZLocale.h"
#include "ZMap.h"
#include "UPnP.h"
#include "MMD5.h"
#include "ZNPCInfoFromServer.h"
#include "ZInterfaceListener.h"
#include "ZShopEquipInterface.h"

#ifdef _SRVRPNG
int pPeerLatency(int nPing) {
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
#endif
void OnQuestNPCList( void* pBlobNPCList, MMATCH_GAMETYPE eGameType )
{
	ZBaseQuest* pBaseQuest = NULL;
	if (ZGetGameTypeManager()->IsQuestOnly(eGameType))
		pBaseQuest = ZGetQuestExactly();
	else if (ZGetGameTypeManager()->IsSurvivalOnly(eGameType))
		pBaseQuest = ZGetSurvivalExactly();
	else
	{
		ASSERT(0);
		return;
	}

	const int					nNPCCount		= MGetBlobArrayCount( pBlobNPCList );
	MTD_NPCINFO*				pQuestNPCInfo	= NULL;
	ZNPCInfoFromServerManager&	NPCMgr			= pBaseQuest->GetNPCInfoFromServerMgr();

	NPCMgr.Clear();

	for( int i = 0; i < nNPCCount; ++i )
	{
		pQuestNPCInfo = reinterpret_cast< MTD_NPCINFO* >( MGetBlobArrayElement(pBlobNPCList, i) );
		if( NULL == pQuestNPCInfo )
		{
			_ASSERT( 0 );
			NPCMgr.Clear();
			return;
		}
		
		NPCMgr.CreateNPCInfo( pQuestNPCInfo );
	}
}


void TimeReward_ShowCharEffect(MUID uidChar)
{
	if (ZGetCharacterManager())
	{
		ZCharacter* pChar = (ZCharacter*)ZGetCharacterManager()->Find(uidChar);
		if (pChar && pChar->IsVisible())
			ZGetEffectManager()->AddTimeRewardEffect(pChar->GetPosition(), pChar);	// ÀÌÆåÆ® Ãâ·Â
	}
}

void TimeReward_ChatOutput_RewardGet(const char* szRewardName, const char* szCharName, const char* szItemName)
{
	char szOutput[512];
	ZTransMsg(szOutput,MSG_BONUS_REWARD_GET,3,szRewardName,szCharName,szItemName);
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szOutput);
}

void TimeReward_ChatOutput_NoChance(const char* szRewardName)
{
	char szOutput[512];
	ZTransMsg(szOutput,MSG_BONUS_REWARD_NOCHANCE,1,szRewardName);
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szOutput);
}

void TimeReward_ChatOutput_RemainChance(int nRemainReward)
{
	char szOutput[512];
	char szRemain[64];
	sprintf(szRemain, "%d", nRemainReward);
	ZTransMsg(szOutput,MSG_BONUS_REWARD_REMAIN,1,szRemain);
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szOutput);
}

void TimeReward_ChatOutput_ResetChance(const char* szRewardResetDesc)
{
	if (0 != strlen(szRewardResetDesc))
	{
		// 'xxx ½ÃÁ¡¿¡ ±âÈ¸°¡ ´Ù½Ã ÃæÀüµÈ´Ù'¸¦ Ç¥½Ã
		//ZTransMsg(szOutput,MSG_BONUS_REWARD_RESET,1,szRewardResetDesc);
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szRewardResetDesc);
	}
}

bool ZGameClient::OnCommand(MCommand* pCommand)
{
	bool ret;
	ret = MMatchClient::OnCommand(pCommand);

#ifdef _LOG_ENABLE_CLIENT_COMMAND_
	char buf[256];
	sprintf(buf,"[ID:%d]: %s\n", pCommand->GetID(), pCommand->GetDescription());
	OutputDebugString(buf);
#endif

	switch(pCommand->GetID())
	{


#ifdef _CW_VOTE 
    //Custom: Interruptor Vote Map
	case MC_MATCH_LADDERWARS_RANDOM_MAPS:
	{
		int nRandomIndex[3];
		pCommand->GetParameter(&nRandomIndex[0], 0, MPT_INT);
		pCommand->GetParameter(&nRandomIndex[1], 1, MPT_INT);
		pCommand->GetParameter(&nRandomIndex[2], 2, MPT_INT);
		ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
		ZIDLResource* m_IDLResource = ZGetGameInterface()->GetIDLResource();
		char Name[128];
		for (int i = 0; i < 3; i++)
		{
			sprintf(Name, "PlayerVoteMapName%d", i);
			MLabel* pLabel = (MLabel*)pResource->FindWidget(Name);
			if (pLabel)
			{
				pLabel->SetText(MGetMapDescMgr()->GetMapName(nRandomIndex[i]));

				m_pSelectorImg0 = new MBitmapR2;
				char szFileName0[256];
				sprintf(szFileName0, "interface/Vote/%s", MGetMapDescMgr()->GetMapImageName(MGetMapDescMgr()->GetMapName(nRandomIndex[0])));
				((MBitmapR2*)m_pSelectorImg0)->Create("SelectorImg.png", RGetDevice(), szFileName0);
				if (m_pSelectorImg0)
				{
					MPicture* pPicture = (MPicture*)m_IDLResource->FindWidget("MapSelectorImg0");
					if (pPicture)
						pPicture->SetBitmap(m_pSelectorImg0->GetSourceBitmap());
				}

				m_pSelectorImg1 = new MBitmapR2;
				char szFileName1[256];
				sprintf(szFileName1, "interface/Vote/%s", MGetMapDescMgr()->GetMapImageName(MGetMapDescMgr()->GetMapName(nRandomIndex[1])));
				((MBitmapR2*)m_pSelectorImg1)->Create("SelectorImg.png", RGetDevice(), szFileName1);
				if (m_pSelectorImg1)
				{
					MPicture* pPicture = (MPicture*)m_IDLResource->FindWidget("MapSelectorImg1");
					if (pPicture)
						pPicture->SetBitmap(m_pSelectorImg1->GetSourceBitmap());
				}

				m_pSelectorImg2 = new MBitmapR2;
				char szFileName2[256];
				sprintf(szFileName2, "interface/Vote/%s", MGetMapDescMgr()->GetMapImageName(MGetMapDescMgr()->GetMapName(nRandomIndex[2])));
				((MBitmapR2*)m_pSelectorImg2)->Create("SelectorImg.png", RGetDevice(), szFileName2);
				if (m_pSelectorImg2)
				{
					MPicture* pPicture = (MPicture*)m_IDLResource->FindWidget("MapSelectorImg2");
					if (pPicture)
						pPicture->SetBitmap(m_pSelectorImg2->GetSourceBitmap());
				}
			}
		}
		ZGetGameInterface()->OnArrangedTeamGameUI(true, true);
		ZGetGameClient()->bMatching = true;
	}
	break;

	case MC_MATCH_LADDERWARS_VOTE_UPDATE:
	{
		int nRandomIndex[3];
		pCommand->GetParameter(&nRandomIndex[0], 0, MPT_INT);
		pCommand->GetParameter(&nRandomIndex[1], 1, MPT_INT);
		pCommand->GetParameter(&nRandomIndex[2], 2, MPT_INT);
		ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
		char Name[100], Text[40];
		for (int i = 0; i < 3; i++)
		{
			sprintf(Name, "PlayerVoteMap%d", i);
			sprintf(Text, "Votes: %d", nRandomIndex[i]);
			MLabel* pLabel = (MLabel*)pResource->FindWidget(Name);
			if (pLabel)
				pLabel->SetText(Text);

		}
	}
	break;
#endif // _CW_VOTE

	case MC_MATCH_LADDERWARS_INVITED:
	{
		ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
		ZGetGameClient()->LastVoteID = -1;
		MWidget* pWidget = pResource->FindWidget("LadderWarsGameDialog");
		if (pWidget != NULL)
			pWidget->Show(false);
		ZGetGameInterface()->OnArrangedTeamGameUI(true);
	}
	break;

		case MC_NET_ONDISCONNECT:
			{

			}
			break;
		case MC_NET_ONERROR:
			{

			}
			break;
		case ZC_CHANGESKIN:
			{
				char szSkinName[256];
				pCommand->GetParameter(szSkinName, 0, MPT_STR, sizeof(szSkinName) );

				// probably some weird stack issue
				if (strlen(szSkinName) <= 0 || szSkinName[0] <= 0)
				{
					mlog("skin error..\n");
					break;
				}

				if(ZApplication::GetGameInterface()->ChangeInterfaceSkin(szSkinName))
				{
					MClient::OutputMessage(MZMOM_LOCALREPLY, "Change Skin To %s", szSkinName);
				}
				else
				{
					MClient::OutputMessage(MZMOM_LOCALREPLY, "Change Skin Failed");
				}
			}
			break;
		case MC_ADMIN_TERMINAL:
			{
				#ifndef _PUBLISH
					char szText[65535]; szText[0] = 0;
					MUID uidChar;

					pCommand->GetParameter(&uidChar, 0, MPT_UID);
					pCommand->GetParameter(szText, 1, MPT_STR, sizeof(szText) );
					OutputToConsole(szText);
				#endif
			}
			break;
		case MC_NET_CHECKPING:
			{
				MUID uid;
				if (pCommand->GetParameter(&uid, 0, MPT_UID)==false) break;
				MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_PING), uid, m_This);
				pNew->AddParameter(new MCommandParameterUInt(timeGetTime()));
				Post(pNew);
				return true;
			}
		case MC_NET_PING:
			{
				unsigned int nTimeStamp;
				if (pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT)==false) break;
				MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_PONG), pCommand->m_Sender, m_This);
				pNew->AddParameter(new MCommandParameterUInt(nTimeStamp));
				Post(pNew);
				return true;
			}
		case MC_NET_PONG:
			{
				int nTimeStamp;
				pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);
				//MClient::OutputMessage(MZMOM_LOCALREPLY, "Ping from (%u:%u) = %d", pCommand->GetSenderUID().High, pCommand->GetSenderUID().Low, timeGetTime()-nTimeStamp);
			}
			break;
		case MC_UDP_PONG:
			{
				unsigned int nIp, nTimeStamp;
				pCommand->GetParameter(&nIp, 0, MPT_UINT);
				pCommand->GetParameter(&nTimeStamp, 1, MPT_UINT);

				ZApplication::GetGameInterface()->SetAgentPing(nIp, nTimeStamp);
			}
			break;
	#ifndef _M_X64
		case MC_HSHIELD_PING:
			{
				unsigned int nTimeStamp;

				if(pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT) == false) break;
				
				MCommandParameter* pParam = pCommand->GetParameter(1);
				if(pParam->GetType() != MPT_BLOB) 	break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);

				unsigned char* pReqMsg = (unsigned char*)MGetBlobArrayElement(pBlob, 0);
				
				DWORD dwRet = _AhnHS_MakeAckMsg(pReqMsg, ZGetMyInfo()->GetSystemInfo()->pbyAckMsg);

				if(dwRet != ERROR_SUCCESS)
					mlog("Making Ack Msg Failed. (Error code = %x)\n", dwRet);

				MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_HSHIELD_PONG), pCommand->m_Sender, m_This);
				pNew->AddParameter(new MCommandParameterUInt(nTimeStamp));
				void* pBlob2 = MMakeBlobArray(sizeof(unsigned char), SIZEOF_ACKMSG);
				unsigned char* pCmdBlock = (unsigned char*)MGetBlobArrayElement(pBlob2, 0);
				CopyMemory(pCmdBlock, ZGetMyInfo()->GetSystemInfo()->pbyAckMsg, SIZEOF_ACKMSG);

				pNew->AddParameter(new MCmdParamBlob(pBlob2, MGetBlobArraySize(pBlob2)));
//				MEraseBlobArray(pBlob);
				MEraseBlobArray(pBlob2);
				Post(pNew);
				
				return true;
			}
			break;
	#endif
		case ZC_CON_CONNECT:
			{
				char szBuf[256];
				sprintf(szBuf, "Net.Connect %s:%d", ZGetConfiguration()->GetServerIP(), 
													ZGetConfiguration()->GetServerPort());
				ConsoleInputEvent(szBuf);
				SetServerAddr(ZGetConfiguration()->GetServerIP(), ZGetConfiguration()->GetServerPort());
			}
			break;
		case ZC_CON_DISCONNECT:
			{
				ConsoleInputEvent("Net.Disconnect");
			}
			break;
		case ZC_CON_CLEAR:
			{
				if (ZGetConsole()) ZGetConsole()->ClearMessage();
			}
			break;
		case ZC_CON_HIDE:
			{
				if (ZGetConsole()) ZGetConsole()->Show(false);
			}
			break;
		case ZC_CON_SIZE:
			{
				if (ZGetConsole())
				{
					int iConWidth, iConHeight;
					pCommand->GetParameter(&iConWidth, 0, MPT_INT);
					pCommand->GetParameter(&iConHeight, 1, MPT_INT);
					if ((iConWidth > 30) && (iConHeight > 30))
					{
						MPOINT point = ZGetConsole()->GetPosition();
						ZGetConsole()->SetBounds(point.x, point.y, iConWidth, iConHeight);
					}
				}
			}
			break;
		case MC_CLOCK_SYNCHRONIZE:
			{
				unsigned long int nGlobalClock;
				pCommand->GetParameter(&nGlobalClock, 0, MPT_UINT);


				unsigned long int nLocalClock = GetClockCount();

				if (nGlobalClock > nLocalClock) m_bIsBigGlobalClock = true;
				else m_bIsBigGlobalClock = false;
				m_nClockDistance = ZGetClockDistance(nGlobalClock, nLocalClock);
			}
			break;
#ifdef _DEBUG
		case ZC_TEST_SETCLIENT1:
			{
				char szBuf[256];
				sprintf(szBuf, "peer.setport 10000");
				ConsoleInputEvent(szBuf);
				sprintf(szBuf, "peer.addpeer 127.0.0.1 10001");
				ConsoleInputEvent(szBuf);

				MClient::OutputMessage(MZMOM_LOCALREPLY, "Done SetClient1");
			}
			break;
		case ZC_TEST_SETCLIENT2:
			{
				char szBuf[256];
				sprintf(szBuf, "peer.setport 10001");
				ConsoleInputEvent(szBuf);
				sprintf(szBuf, "peer.addpeer 127.0.0.1 10000");
				ConsoleInputEvent(szBuf);

				MClient::OutputMessage(MZMOM_LOCALREPLY, "Done SetClient2");
			}
			break;
		case ZC_TEST_SETCLIENTALL:
			{
				char szMyIP[256];
				pCommand->GetParameter(szMyIP, 0, MPT_STR, sizeof(szMyIP) );

				
				char szBuf[256];
				char szIPs[][256] = { "192.168.0.100", "192.168.0.111", "192.168.0.10", 
					                  "192.168.0.11", "192.168.0.16", "192.168.0.20",
				                      "192.168.0.25", "192.168.0.30", "192.168.0.32",
										"192.168.0.200", "192.168.0.15", "192.168.0.17"};
				sprintf(szBuf, "peer.setport 10000");
				ConsoleInputEvent(szBuf);

				for (int i = 0; i < 12; i++)
				{
					if (!strcmp(szMyIP, szIPs[i])) continue;
					sprintf(szBuf, "peer.addpeer %s 10000", szIPs[i]);
					ConsoleInputEvent(szBuf);
				}

				MClient::OutputMessage(MZMOM_LOCALREPLY, "Done SetClient All");
			}
			break;
#endif
#ifndef _PUBLISH
		case ZC_TEST_BIRD1:
			{
				OnBirdTest();
			}
			break;
#endif
		case MC_MATCH_NOTIFY:
		{
			unsigned int nMsgID = 0;
			if (pCommand->GetParameter(&nMsgID, 0, MPT_UINT) == false) break;

			if (ZGetGameClient()->GetServerUID() != pCommand->GetSenderUID()) return false;
			if (nMsgID == MATCHNOTIFY_STAGE_NOT_EXIST) ZGetGameClient()->IsRejoin = false;
			OnMatchNotify(nMsgID);
		}
		break;
		case MC_MATCH_BRIDGEPEER_ACK:
			{
				MUID uidChar;
				int nCode;
				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&nCode, 1, MPT_INT);
				OnBridgePeerACK(uidChar, nCode);
			}
			break;
		case MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY:			// ³­ÀÔ
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				if (nResult == MOK)
				{
					OnForcedEntryToGame();
				}
				else
				{
					ZApplication::GetGameInterface()->ShowMessage("³­ÀÔÇÒ ¼ö ¾ø½À´Ï´Ù.");
				}
			}
			break;
		case MC_MATCH_STAGE_JOIN:
			{
				MUID uidChar, uidStage;
				unsigned int nRoomNo=0;
				char szStageName[256]="";

				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(&nRoomNo, 2, MPT_UINT);
				pCommand->GetParameter(szStageName, 3, MPT_STR, sizeof(szStageName) );

				OnStageJoin(uidChar, uidStage, nRoomNo, szStageName);
			}
			break;
		case MC_MATCH_STAGE_LEAVE:
			{
				MUID uidChar, uidStage;

				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);

				OnStageLeave(uidChar, uidStage);
			}
			break;
		case MC_MATCH_STAGE_START:
			{
				MUID uidChar, uidStage;
				int nCountdown;

				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(&nCountdown, 2, MPT_INT);

				OnStageStart(uidChar, uidStage, nCountdown);
			}
			break;

		case MC_MATCH_STAGE_LAUNCH:
			{
				MUID uidStage;
				char szMapName[_MAX_DIR];

				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				pCommand->GetParameter(szMapName, 1, MPT_STR, sizeof(szMapName) );
				OnStageLaunch(uidStage, szMapName);
			}
			break;

		case MC_MATCH_STAGE_RELAY_LAUNCH :
			{
				MUID uidStage;
				bool bIsIgnore;
				char szMapName[_MAX_DIR];

				pCommand->GetParameter(&uidStage,	0, MPT_UID);
				pCommand->GetParameter(szMapName,	1, MPT_STR, sizeof(szMapName) );
				pCommand->GetParameter(&bIsIgnore,	2, MPT_BOOL);

				if( !bIsIgnore ) OnStageLaunch(uidStage, szMapName);
				else {
					m_bForcedEntry = true;
					ZApplication::GetGameInterface()->SerializeStageInterface();
				}				
			}
			break;
			// Trong hàm OnCommand của ZGameClient / ZShopEquipInterface
		case MC_MATCH_RESPONSE_BUY_PACKAGE:
		{
			int nResult = 0;
			pCommand->GetParameter(&nResult, 0, MPT_INT);

			if (ZGetGameInterface() && ZGetGameInterface()->GetShopEquipInterface())
			{
				ZGetGameInterface()->GetShopEquipInterface()->OnResponseBuyPackage(nResult);
			}
		}
		break;

		// Ví dụ hàm xử lý Command phản hồi từ Server
		case MC_MATCH_EVENT_ATTENDANCE_INFO_RESPONSE:
		{
			int nTotalDays = 0;
			bool bIsTodayClaimed = false;

			pCommand->GetParameter(&nTotalDays, 0, MPT_INT);
			pCommand->GetParameter(&bIsTodayClaimed, 1, MPT_BOOL);

			// 1. Cập nhật trạng thái từng ngày trong EventManager
			auto& vtAttendance = ZGetEventManager()->GetAttendanceData();
			for (size_t i = 0; i < vtAttendance.size(); ++i)
			{
				int nDay = vtAttendance[i].nDay;

				if (nDay <= nTotalDays)
				{
					// Những ngày trước đó -> Đã nhận quà
					vtAttendance[i].nState = ZATTEND_STATE_RECEIVED;
				}
				else if (nDay == nTotalDays + 1)
				{
					// Ngày tiếp theo:
					// Nếu hôm nay CHƯA nhận -> Đổi sang "Được nhận" (Sáng lên để bấm)
					// Nếu hôm nay ĐÃ nhận rồi -> Khóa lại (Dành cho ngày mai)
					vtAttendance[i].nState = bIsTodayClaimed ? ZATTEND_STATE_LOCKED : ZATTEND_STATE_RECEIVE;
				}
				else
				{
					// Các ngày tương lai -> Khóa hoàn toàn
					vtAttendance[i].nState = ZATTEND_STATE_LOCKED;
				}
			}

			// 2. REFRESH LẠI UI ĐIỂM DANH
			if (ZApplication::GetGameInterface())
			{
				ZApplication::GetGameInterface()->UpdateAttendanceUI();
			}
		}
		break;

		case MC_MATCH_EVENT_CLAIM_ATTENDANCE_RESPONSE:
		{
			int nDay = 0, nResult = 0, nRewardType = 0, nRewardAmount = 0;
			pCommand->GetParameter(&nDay, 0, MPT_INT);
			pCommand->GetParameter(&nResult, 1, MPT_INT);
			pCommand->GetParameter(&nRewardType, 2, MPT_INT);
			pCommand->GetParameter(&nRewardAmount, 3, MPT_INT);

			if (nResult == 1)
			{
				// 1. Cập nhật danh sách điểm danh trong ZEventManager
				auto& vtAttendance = ZGetEventManager()->GetAttendanceData();
				for (size_t i = 0; i < vtAttendance.size(); ++i)
				{
					if (vtAttendance[i].nDay == nDay)
					{
						vtAttendance[i].nState = ZATTEND_STATE_RECEIVED; // Đã nhận thành công

						// ❌ ĐÃ XÓA ĐOẠN CODE TỰ MỞ KHÓA i + 1 Ở ĐÂY
						// Các ngày tiếp theo vẫn giữ nguyên trạng thái ZATTEND_STATE_LOCKED!
						break;
					}
				}

				// 2. Cập nhật chỉ số hiển thị RAM của Client qua ZGetMyInfo()
				if (ZGetMyInfo())
				{
					if (nRewardType == ZEVENT_REWARD_BOUNTY) // 0: Bounty
					{
						int nCurrentBP = ZGetMyInfo()->GetBP();
						ZGetMyInfo()->SetBP(nCurrentBP + nRewardAmount);
					}
					else if (nRewardType == ZEVENT_REWARD_ZCOIN) // 1: Medal / LC
					{
						int nCurrentLC = ZGetMyInfo()->GetLC();
						ZGetMyInfo()->SetLC(nCurrentLC + nRewardAmount);
					}
					else if (nRewardType == ZEVENT_REWARD_ITEM) // 2: Item
					{
						// Refresh lại kho đồ nếu nhận Item
						// ZPOSTCMD0(MC_MATCH_ACCOUNT_ITEM_LIST);
					}
				}

				// 3. Refresh giao diện UI Daily Attendance
				if (ZGetGameInterface())
				{
					ZGetGameInterface()->UpdateAttendanceUI();
				}

				// 4. Thông báo cho người chơi
				ZGetGameInterface()->ShowMessage("Receive a successful attendance reward!");
			}
			else
			{
				ZGetGameInterface()->ShowMessage("You have already received this reward or an error occurred!");
			}
		}break;

		case MC_MATCH_STAGE_FINISH_GAME:
			{
				bool bIsRelayMapUnFinish;
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				pCommand->GetParameter(&bIsRelayMapUnFinish, 1, MPT_BOOL);
				
				OnStageFinishGame(uidStage, bIsRelayMapUnFinish);
#ifdef _LADDERUPDATE 
				if (ZGetGameClient()->IsLadderWarsChannel())
				{
					char szText[256];
					sprintf(szText, ZMsg(MSG_WORD_LADDERCOINN_FINISH_MATCH));
					sprintf(szText, ZMsg(MSG_WORD_LADDERCOINN_MSG_DIALOG));
					ZChatOutput(ZCOLOR_CHAT_SYSTEM, szText);
				}
#endif

			}
			break;

		case MC_MATCH_STAGE_MAP:
			{
				MUID uidStage;
				char szMapName[_MAX_DIR];

				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				pCommand->GetParameter(szMapName, 1, MPT_STR, sizeof(szMapName) );

				OnStageMap(uidStage, szMapName);
			}
			break;
        //Custom: Lightmaps Time Code
		case MC_MATCH_STAGE_MAPTOD:
		{
			MUID stageUID;
			int  lightmapIndex;

			pCommand->GetParameter(&stageUID, 0, MPT_UID);
			pCommand->GetParameter(&lightmapIndex, 1, MPT_INT);

			OnStageMapLighting(stageUID, lightmapIndex);
		}
		break;


		case MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE:
			{
				MUID uidStage;
				int nRelayMapType = 0;
				int nRelayMapRepeatCount = 0;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				pCommand->GetParameter(&nRelayMapType, 1, MPT_INT );
				pCommand->GetParameter(&nRelayMapRepeatCount, 2, MPT_INT );
				MCommandParameter* pParam = pCommand->GetParameter(3);
				if (pParam->GetType() != MPT_BLOB)	break;
				void* pRelayMapListBlob = pParam->GetPointer();
				if( NULL == pRelayMapListBlob )	break;

				OnStageRelayMapListUpdate(nRelayMapType, nRelayMapRepeatCount, pRelayMapListBlob);
			}
			break;

		case MC_MATCH_STAGE_RELAY_MAP_ELEMENT_UPDATE:
			{
				MUID uidStage;
				int nRelayMapType = 0;
				int nRelayMapRepeatCount = 0;

				pCommand->GetParameter(&uidStage, 0, MPT_UID );
				pCommand->GetParameter(&nRelayMapType, 1, MPT_INT );
				pCommand->GetParameter(&nRelayMapRepeatCount, 2, MPT_INT );

				OnStageRelayMapElementUpdate(nRelayMapType, nRelayMapRepeatCount);
			}
			break;

		case MC_MATCH_STAGE_TEAM:
			{
				MUID uidChar, uidStage;
				unsigned int nTeam;
				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(&nTeam, 2, MPT_UINT);
				OnStageTeam(uidChar, uidStage, nTeam);
			}
			break;

		case MC_MATCH_STAGE_PLAYER_STATE:
			{
				MUID uidChar, uidStage;
				int nObjectStageState;
				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(&nObjectStageState, 2, MPT_INT);
				OnStagePlayerState(uidChar, uidStage, MMatchObjectStageState(nObjectStageState));
			}
			break;
		case MC_MATCH_STAGE_MASTER:
			{
				MUID uidChar, uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				pCommand->GetParameter(&uidChar, 1, MPT_UID);

				OnStageMaster(uidStage, uidChar);
			}
			break;
		case MC_MATCH_STAGE_CHAT:
			{
				MUID uidStage, uidChar;
				static char szChat[512];
				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(szChat, 2, MPT_STR, sizeof(szChat) );
				//check Chatting Message..jintriple3 ÁÙ ¹Ù²Þ ¹®ÀÚ ÇÊÅÍ¸µ
				CheckMsgAboutChat(szChat);
				OnStageChat(uidChar, uidStage, szChat);
			}
			break;
		case MC_MATCH_STAGE_LIST:
			{
				char nPrevStageCount, nNextStageCount;
				pCommand->GetParameter(&nPrevStageCount, 0, MPT_CHAR);
				pCommand->GetParameter(&nNextStageCount, 1, MPT_CHAR);

				MCommandParameter* pParam = pCommand->GetParameter(2);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);

				OnStageList((int)nPrevStageCount, (int)nNextStageCount, pBlob, nCount);
			}
			break;
		case MC_MATCH_CHANNEL_RESPONSE_PLAYER_LIST:
			{
				unsigned char nTotalPlayerCount, nPage;

				pCommand->GetParameter(&nTotalPlayerCount,	0, MPT_UCHAR);
				pCommand->GetParameter(&nPage,				1, MPT_UCHAR);

				MCommandParameter* pParam = pCommand->GetParameter(2);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);

				OnChannelPlayerList((int)nTotalPlayerCount, (int)nPage, pBlob, nCount);

			}
			break;
		case MC_MATCH_CHANNEL_RESPONSE_ALL_PLAYER_LIST:
			{
				MUID uidChannel;

				pCommand->GetParameter(&uidChannel, 0, MPT_UID);

				MCommandParameter* pParam = pCommand->GetParameter(1);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);

				OnChannelAllPlayerList(uidChannel, pBlob, nCount);
			}
			break;
		case MC_MATCH_RESPONSE_FRIENDLIST:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);

				OnResponseFriendList(pBlob, nCount);
			}
			break;
		case MC_MATCH_RESPONSE_STAGESETTING:
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);

				MCommandParameter* pStageParam = pCommand->GetParameter(1);
				if(pStageParam->GetType()!=MPT_BLOB) break;
				void* pStageBlob = pStageParam->GetPointer();
				int nStageCount = MGetBlobArrayCount(pStageBlob);

				MCommandParameter* pCharParam = pCommand->GetParameter(2);
				if(pCharParam->GetType()!=MPT_BLOB) break;
				void* pCharBlob = pCharParam->GetPointer();
				int nCharCount = MGetBlobArrayCount(pCharBlob);

				int nStageState;
				pCommand->GetParameter(&nStageState, 3, MPT_INT);

				MUID uidMaster;
				pCommand->GetParameter(&uidMaster, 4, MPT_UID);

				OnResponseStageSetting(uidStage, pStageBlob, nStageCount, pCharBlob, nCharCount, STAGE_STATE(nStageState), uidMaster);

				ChangeQuestStage();

			}
			break;
		case MC_MATCH_RESPONSE_PEER_RELAY:
			{
				MUID uidPeer;
				if (pCommand->GetParameter(&uidPeer, 0, MPT_UID) == false) break;

				OnResponsePeerRelay(uidPeer);			
			}
			break;

		case MC_MATCH_LOADING_COMPLETE:
			{
				MUID uidChar;
				int nPercent;

				if (pCommand->GetParameter(&uidChar, 0, MPT_UID) == false) break;
				if (pCommand->GetParameter(&nPercent, 1, MPT_INT) == false) break;

				OnLoadingComplete(uidChar, nPercent);
			}
			break;
		case MC_MATCH_ANNOUNCE:
			{
				unsigned int nType;
				char szMsg[256];
				pCommand->GetParameter(&nType, 0, MPT_UINT);
				pCommand->GetParameter(szMsg, 1, MPT_STR, sizeof(szMsg) );
				OnAnnounce(nType, szMsg);
			}
			break;
		case MC_MATCH_CHANNEL_RESPONSE_JOIN:
			{
				MUID uidChannel;
				int nChannelType;
				char szChannelName[256];
				bool bEnableInterface;

				pCommand->GetParameter(&uidChannel,			0, MPT_UID);
				pCommand->GetParameter(&nChannelType,		1, MPT_INT);
				pCommand->GetParameter(szChannelName,		2, MPT_STR, sizeof(szChannelName) );
				pCommand->GetParameter(&bEnableInterface,	3, MPT_BOOL);

				const char* szChannelNameTranslated = ZGetStringResManager()->GetStringFromXml(szChannelName);

				OnChannelResponseJoin(uidChannel, (MCHANNEL_TYPE)nChannelType, szChannelNameTranslated, bEnableInterface);
			}
			break;
		case MC_MATCH_CHANNEL_CHAT:
			{
				MUID uidChannel, uidChar;
				char szChat[512];
				char szName[256];
				int nGrade;

				pCommand->GetParameter(&uidChannel, 0, MPT_UID);
				pCommand->GetParameter(szName, 1, MPT_STR, sizeof(szName) );
				pCommand->GetParameter(szChat, 2, MPT_STR, sizeof(szChat) );
				pCommand->GetParameter(&nGrade,3, MPT_INT);

				//jintriple3 ÁÙ ¹Ù²Þ ¹®ÀÚ ÇÊÅÍ¸µ
				//check chatting Message
				CheckMsgAboutChat(szChat);

				OnChannelChat(uidChannel, szName, szChat, nGrade);
			}
			break;
		case MC_MATCH_CHANNEL_LIST:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);
				OnChannelList(pBlob, nCount);
			}
			break;
		case MC_MATCH_CHANNEL_RESPONSE_RULE:
			{
				MUID uidChannel;
				pCommand->GetParameter(&uidChannel, 0, MPT_UID);
				char szRuleName[128];
				pCommand->GetParameter(szRuleName, 1, MPT_STR, sizeof(szRuleName) );

				OnChannelResponseRule(uidChannel, szRuleName);
			}
			break;
		case MC_MATCH_RESPONSE_RECOMMANDED_CHANNEL:
			{
				MUID uidChannel;
				pCommand->GetParameter(&uidChannel, 0, MPT_UID);

				OnResponseRecommandedChannel(uidChannel);
			}
			break;
		case MC_ADMIN_ANNOUNCE:
			{
				char szChat[512];
				unsigned long int nMsgType = 0;

				pCommand->GetParameter(szChat, 1, MPT_STR, sizeof(szChat) );
				pCommand->GetParameter(&nMsgType, 2, MPT_UINT);

				OnAdminAnnounce(ZGetStringResManager()->GetStringFromXml(szChat), ZAdminAnnounceType(nMsgType));
			}
			break;
#ifdef _ANTISPAM
		case MC_ADMIN_WALL:
		{
			char szChat[512];
			char szCharName[50];

			pCommand->GetParameter(szChat, 1, MPT_STR, sizeof(szChat));
			pCommand->GetParameter(szCharName, 2, MPT_STR, sizeof(szCharName));

			OnAdminWall(szChat, szCharName);
		}
		break;
#endif

		case MC_SERVER_ANNOUNCE:
		{
			char szReason[2048];
			pCommand->GetParameter(szReason, 1, MPT_STR, sizeof(szReason));
			OnPlayerNotify(szReason);
		}break;
		case MC_MATCH_GAME_LEVEL_UP:
			{
				MUID uidChar;
#ifdef _UPREWARDLEVEL
				int nBounty;
				int nCash;
				int nItemID;
#endif

				pCommand->GetParameter(&uidChar, 0, MPT_UID);
#ifdef _UPREWARDLEVEL
				pCommand->GetParameter(&nBounty, 2, MPT_INT);
				pCommand->GetParameter(&nCash, 3, MPT_INT);
				pCommand->GetParameter(&nItemID, 4, MPT_INT);
				OnGameLevelUp(uidChar, nBounty, nCash, nItemID);
#else
				OnGameLevelUp(uidChar);
#endif
			}
			break;
		case MC_STAFF_CHAT:
		{
			char szChat[340];
			char szName[120];
			int UGradeID;
			pCommand->GetParameter(szChat, 0, MPT_STR, sizeof(szChat));
			pCommand->GetParameter(szName, 1, MPT_STR, sizeof(szName));
			pCommand->GetParameter(&UGradeID, 2, MPT_INT);
			OnStaffHelp(szChat, szName, UGradeID);
		}
		break;
		case MC_MATCH_GAME_LEVEL_DOWN:
			{
				MUID uidChar;
				pCommand->GetParameter(&uidChar, 0, MPT_UID);

				OnGameLevelDown(uidChar);
			}
			break;
		case MC_MATCH_RESPONSE_GAME_INFO:
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);

				MCommandParameter* pParam = pCommand->GetParameter(1);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pGameInfoBlob = pParam->GetPointer();

				pParam = pCommand->GetParameter(2);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pRuleInfoBlob = pParam->GetPointer();

				pParam = pCommand->GetParameter(3);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pPlayerInfoBlob = pParam->GetPointer();

				OnResponseGameInfo(uidStage, pGameInfoBlob, pRuleInfoBlob, pPlayerInfoBlob, false);
			}
			break;
		// Custom: Larger Game Info Packet
		case MC_MATCH_RESPONSE_GAME_INFO_V2:
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);

				MCommandParameter* pParam = pCommand->GetParameter(1);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pGameInfoBlob = pParam->GetPointer();

				pParam = pCommand->GetParameter(2);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pRuleInfoBlob = pParam->GetPointer();

				pParam = pCommand->GetParameter(3);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pPlayerInfoBlob = pParam->GetPointer();

				OnResponseGameInfo(uidStage, pGameInfoBlob, pRuleInfoBlob, pPlayerInfoBlob, true);
			}
			break;
#ifdef _PORTALGUN
		case MC_RESPONSE_MOVETO_GAMEPORTAL:
		{
			MVector pos;

			pCommand->GetParameter(&pos, 0, MPT_POS);

			OnResponseMoveToGamePortal(pos.x, pos.y, pos.z);
		}
		break;
#endif
		case MC_MATCH_OBTAIN_WORLDITEM:
			{
				MUID uidPlayer;
				int nItemUID;

				pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&nItemUID, 1, MPT_INT);

				OnObtainWorldItem(uidPlayer, nItemUID);
			}
			break;
		case MC_MATCH_SPAWN_WORLDITEM:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if (pParam->GetType()!=MPT_BLOB) break;

				void* pSpawnInfoBlob = pParam->GetPointer();

				OnSpawnWorldItem(pSpawnInfoBlob);
			}
			break;
		case MC_MATCH_REMOVE_WORLDITEM:
			{
				int nItemUID;

				pCommand->GetParameter(&nItemUID, 0, MPT_INT);

				OnRemoveWorldItem(nItemUID);
			}
			break;
#ifdef _PLAYERINO
		case MC_MATCH_INFORMATION_RECEIVE:
		{
			MFrame* pFrame = (MFrame*)ZGetGameInterface()->GetIDLResource()->FindWidget("Information");
			pFrame->Show(true, true);
			MCommandParameter* pParam = pCommand->GetParameter(0);
			if (pParam->GetType() != MPT_BLOB) break;
			void* pBlob = pParam->GetPointer();
			int nCount = MGetBlobArrayCount(pBlob);

			for (int i = 0; i < nCount; i++)
			{
				MTD_Information* pNode = (MTD_Information*)MGetBlobArrayElement(pBlob, i);

				MLabel* NameLabel = (MLabel*)ZGetGameInterface()->GetIDLResource()->FindWidget("NameLabel");
				MLabel* ClanLabel = (MLabel*)ZGetGameInterface()->GetIDLResource()->FindWidget("ClanLabel");
				MLabel* LevelLabel = (MLabel*)ZGetGameInterface()->GetIDLResource()->FindWidget("LevelLabel");
				MLabel* BountyLabel = (MLabel*)ZGetGameInterface()->GetIDLResource()->FindWidget("BountyLabel");
				MLabel* LadderLabel = (MLabel*)ZGetGameInterface()->GetIDLResource()->FindWidget("LadderLabel");
				MLabel* EXPLabel = (MLabel*)ZGetGameInterface()->GetIDLResource()->FindWidget("EXPLabel");
				MLabel* KillLabel = (MLabel*)ZGetGameInterface()->GetIDLResource()->FindWidget("KillLabel");
				MLabel* DeathLabel = (MLabel*)ZGetGameInterface()->GetIDLResource()->FindWidget("DeathLabel");

				NameLabel->SetText(pNode->szName);

				ClanLabel->SetText(pNode->szClan);
				char szLevel[128], szBounty[128], szLC[128], szKill[128], szDeath[128], szEXP[128];

				sprintf(szLevel, "%d", pNode->nLevel);
				LevelLabel->SetText(szLevel);

				sprintf(szEXP, "%d", pNode->nEXP);
				EXPLabel->SetText(szEXP);

				sprintf(szBounty, "%d", pNode->nBounty);
				BountyLabel->SetText(szBounty);

				sprintf(szLC, "%d", pNode->nLC);
				LadderLabel->SetText(szLC);

				sprintf(szKill, "%d", pNode->nKill);
				KillLabel->SetText(szKill);

				sprintf(szDeath, "%d", pNode->nDeath);
				DeathLabel->SetText(szDeath);

				MPicture* pPicture = (MPicture*)ZGetGameInterface()->GetIDLResource()->FindWidget("ClanInfoEmblem");
				if (pPicture)
					pPicture->SetBitmap(ZGetEmblemInterface()->GetClanEmblem2(pNode->nClanEmblem));



			}
		}
		break;

		case MC_MATCH_INFORMATION_HPAP:
		{
			char szSenderName[128] = "";
			pCommand->GetParameter(szSenderName, 0, MPT_STR, sizeof(szSenderName));
			ZPOSTCMD3(MC_MATCH_INFORMATION_HPAP_GO, MCommandParameterString(szSenderName), MCommandParameterInt(ZGetMyInfo()->GetHP()), MCommandParameterInt(ZGetMyInfo()->GetAP()));
		}
		break;
		case MC_MATCH_INFORMATION_CLOTHES:
		{
			char szSenderName[128] = "";
			pCommand->GetParameter(szSenderName, 0, MPT_STR, sizeof(szSenderName));

			void* pInformationFrame = MMakeBlobArray(sizeof(MTD_InformationClothes), 1);
			MTD_InformationClothes* pInformation = (MTD_InformationClothes*)MGetBlobArrayElement(pInformationFrame, 0);

			for (int i = 0; i < MMCIP_END; i++)
			{
				pInformation->nEquipedItemID[i] = ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMatchCharItemParts(i));
			}
			pInformation->nFace = ZGetMyInfo()->GetFace();
			pInformation->nHair = ZGetMyInfo()->GetHair();
			pInformation->nSex = ZGetMyInfo()->GetSex();

			MEraseBlobArray(pInformationFrame);
			ZPOSTCMD2(MC_MATCH_INFORMATION_CLOTHES_GO, MCommandParameterString(szSenderName), MCommandParameterBlob(pInformationFrame, MGetBlobArraySize(pInformationFrame)));


		}
		break;
		case MC_MATCH_INFORMATION_HPAP_GO:
		{
			int nHP, nAP;
			char szSenderName[128] = "";
			pCommand->GetParameter(szSenderName, 0, MPT_STR, sizeof(szSenderName));
			pCommand->GetParameter(&nHP, 1, MPT_INT);
			pCommand->GetParameter(&nAP, 2, MPT_INT);

			MLabel* HPLabel = (MLabel*)ZGetGameInterface()->GetIDLResource()->FindWidget("HPLabel");
			MLabel* APLabel = (MLabel*)ZGetGameInterface()->GetIDLResource()->FindWidget("APLabel");

			char szHP[128], szAP[128];

			sprintf(szHP, "%d", nHP);
			HPLabel->SetText(szHP);

			sprintf(szAP, "%d", nAP);
			APLabel->SetText(szAP);

		}
		break;
		case MC_MATCH_INFORMATION_CLOTHES_GO:
		{
			char szSenderName[128] = "";
			pCommand->GetParameter(szSenderName, 0, MPT_STR, sizeof(szSenderName));


			MCommandParameter* pParam = pCommand->GetParameter(1);
			if (pParam->GetType() != MPT_BLOB) break;
			void* pBlob = pParam->GetPointer();
			int nCount = MGetBlobArrayCount(pBlob);


			MTD_InformationClothes* pNode = (MTD_InformationClothes*)MGetBlobArrayElement(pBlob, 0);
			ZCharacterView* pCharView = (ZCharacterView*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("InformationViewer");
			unsigned long int nEquipedItemIDTest[MMCIP_END];
			for (int i = 0; i < MMCIP_END; i++)
			{
				nEquipedItemIDTest[i] = pNode->nEquipedItemID[i];
			}
			pCharView->InitCharParts((MMatchSex)pNode->nSex, pNode->nHair, pNode->nFace, nEquipedItemIDTest);

		}
		break;
#endif
#ifdef _SPMOTIONLAST
		case MC_MATCH_SPMOTION_REPLY:
			{
				int nMotionType;
				MUID uidPlayer;
				pCommand->GetParameter(&nMotionType, 0, MPT_INT);
				pCommand->GetParameter(&uidPlayer, 1, MPT_UID);

				ZCharacter* pCharacter = ZGetGame()->m_CharacterManager.Find(uidPlayer);
				pCharacter->m_dwStatusBitPackingValue.Ref().m_bSpMotion = true;
				ZC_STATE_LOWER zsl = ZC_STATE_DANCE;
				switch (nMotionType)
				{
					case 0:
					{
						zsl = ZC_STATE_DANCE;
					}
					break;
				}
				
				pCharacter->m_SpMotion = zsl;
				pCharacter->SetAnimationLower(zsl);
			}
			break;
#endif
		case MC_MATCH_USER_WHISPER:
			{
				char szSenderName[128]="";
				char szTargetName[128]="";
				char szMessage[1024]="";
				
				pCommand->GetParameter(szSenderName, 0, MPT_STR, sizeof(szSenderName) );
				pCommand->GetParameter(szTargetName, 1, MPT_STR, sizeof(szTargetName) );
				pCommand->GetParameter(szMessage, 2, MPT_STR, sizeof(szMessage) );

				//jintriple3 ÁÙ ¹Ù²Þ ¹®ÀÚ ÇÊÅÍ¸µ
				//check chatting Message
				CheckMsgAboutChat(szMessage);

				OnUserWhisper(szSenderName, szTargetName, szMessage);
			}
			break;
		case MC_MATCH_CHATROOM_JOIN:
			{
				char szPlayerName[128]="";
				char szChatRoomName[128]="";

				pCommand->GetParameter(szPlayerName, 0, MPT_STR, sizeof(szPlayerName) );
				pCommand->GetParameter(szChatRoomName, 1, MPT_STR, sizeof(szChatRoomName) );

				OnChatRoomJoin(szPlayerName, szChatRoomName);
			}
			break;
		case MC_MATCH_CHATROOM_LEAVE:
			{
				char szPlayerName[128]="";
				char szChatRoomName[128]="";

				pCommand->GetParameter(szPlayerName, 0, MPT_STR, sizeof(szPlayerName) );
				pCommand->GetParameter(szChatRoomName, 1, MPT_STR, sizeof(szChatRoomName) );

				OnChatRoomLeave(szPlayerName, szChatRoomName);
			}
			break;
		case MC_MATCH_CHATROOM_SELECT_WRITE:
			{
				char szChatRoomName[128]="";
				pCommand->GetParameter(szChatRoomName, 0, MPT_STR, sizeof(szChatRoomName) );

				OnChatRoomSelectWrite(szChatRoomName);
			}
			break;
		case MC_MATCH_CHATROOM_INVITE:
			{
				char szSenderName[64]="";
				char szTargetName[64]="";
				char szRoomName[128]="";

				pCommand->GetParameter(szSenderName, 0, MPT_STR, sizeof(szSenderName) );
				pCommand->GetParameter(szTargetName, 1, MPT_STR, sizeof(szTargetName) );
				pCommand->GetParameter(szRoomName, 2, MPT_STR, sizeof(szRoomName) );

				OnChatRoomInvite(szSenderName, szRoomName);
			}
			break;
		case MC_MATCH_CHATROOM_CHAT:
			{
				char szChatRoomName[128]="";
				char szPlayerName[128]="";
				char szChat[128]="";

				pCommand->GetParameter(szChatRoomName, 0, MPT_STR, sizeof(szChatRoomName) );
				pCommand->GetParameter(szPlayerName, 1, MPT_STR, sizeof(szPlayerName) );
				pCommand->GetParameter(szChat, 2, MPT_STR, sizeof(szChat) );

				//jintriple3 ÁÙ ¹Ù²Þ ¹®ÀÚ ÇÊÅÍ¸µ
				//check chatting Message
				CheckMsgAboutChat(szChat);

				OnChatRoomChat(szChatRoomName, szPlayerName, szChat);
			}
			break;
		case ZC_REPORT_119:
			{
				OnLocalReport119();
			}
			break;
		case ZC_MESSAGE:
			{
				int nMessageID;
				pCommand->GetParameter(&nMessageID, 0, MPT_INT);
				ZGetGameInterface()->ShowMessage(nMessageID);
			}break;
		case MC_TEST_PEERTEST_PING:
			{
				MUID uidSender = pCommand->GetSenderUID();
				char szLog[128];
				sprintf(szLog, "PEERTEST_PING: from (%d%d)", uidSender.High, uidSender.Low);
				ZChatOutput(szLog, ZChat::CMT_SYSTEM);
			}
			break;
		case MC_TEST_PEERTEST_PONG:
			{
			}
			break;

		// Å¬·£°ü·Ã
		case MC_MATCH_CLAN_RESPONSE_CREATE_CLAN:
			{
				int nResult, nRequestID;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				pCommand->GetParameter(&nRequestID, 1, MPT_INT);

				OnResponseCreateClan(nResult, nRequestID);

			}
			break;
		case MC_MATCH_CLAN_RESPONSE_AGREED_CREATE_CLAN:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);

				OnResponseAgreedCreateClan(nResult);
			}
			break;
		case MC_MATCH_CLAN_ASK_SPONSOR_AGREEMENT:
			{
				int nRequestID;
				char szClanName[256];
				MUID uidMasterObject;
				char szMasterName[256];


				pCommand->GetParameter(&nRequestID,			 0, MPT_INT);
				pCommand->GetParameter(szClanName,			1, MPT_STR, sizeof(szClanName) );
				pCommand->GetParameter(&uidMasterObject,	2, MPT_UID);
				pCommand->GetParameter(szMasterName,		3, MPT_STR, sizeof(szMasterName) );

				OnClanAskSponsorAgreement(nRequestID, szClanName, uidMasterObject, szMasterName);
			}
			break;
		case MC_MATCH_CLAN_ANSWER_SPONSOR_AGREEMENT:
			{
				MUID uidClanMaster;
				int nRequestID;
				bool bAnswer;
				char szCharName[256];

				pCommand->GetParameter(&nRequestID,		0, MPT_INT);
				pCommand->GetParameter(&uidClanMaster,	1, MPT_UID);
				pCommand->GetParameter(szCharName,		2, MPT_STR, sizeof(szCharName) );
				pCommand->GetParameter(&bAnswer,		3, MPT_BOOL);

				OnClanAnswerSponsorAgreement(nRequestID, uidClanMaster, szCharName, bAnswer);
			}
			break;
		case MC_MATCH_CLAN_RESPONSE_CLOSE_CLAN:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);

				OnClanResponseCloseClan(nResult);
			}
			break;
		case MC_MATCH_CLAN_RESPONSE_JOIN_CLAN:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				OnClanResponseJoinClan(nResult);
			}
			break;
		case MC_MATCH_CLAN_ASK_JOIN_AGREEMENT:
			{
				char szClanName[256], szClanAdmin[256];
				MUID uidClanAdmin;

				pCommand->GetParameter(szClanName,		0, MPT_STR, sizeof(szClanName) );
				pCommand->GetParameter(&uidClanAdmin,	1, MPT_UID);
				pCommand->GetParameter(szClanAdmin,		2, MPT_STR, sizeof(szClanAdmin) );

				OnClanAskJoinAgreement(szClanName, uidClanAdmin, szClanAdmin);
			}
			break;
		case MC_MATCH_CLAN_ANSWER_JOIN_AGREEMENT:
			{
				MUID uidClanAdmin;
				bool bAnswer;
				char szJoiner[256];

				pCommand->GetParameter(&uidClanAdmin,	0, MPT_UID);
				pCommand->GetParameter(szJoiner,		1, MPT_STR, sizeof(szJoiner) );
				pCommand->GetParameter(&bAnswer,		2, MPT_BOOL);

				OnClanAnswerJoinAgreement(uidClanAdmin, szJoiner, bAnswer);
			}
			break;
		case MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				OnClanResponseAgreedJoinClan(nResult);
			}
			break;
		case MC_MATCH_CLAN_UPDATE_CHAR_CLANINFO:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();

				OnClanUpdateCharClanInfo(pBlob);
			}
			break;
		case MC_MATCH_CLAN_RESPONSE_LEAVE_CLAN:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				OnClanResponseLeaveClan(nResult);
			}
			break;
		case MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				OnClanResponseChangeGrade(nResult);
			}
			break;
		case MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				OnClanResponseExpelMember(nResult);
			}
			break;
		case MC_MATCH_CLAN_MSG:
			{
				char szSenderName[256];
				char szMsg[512];

				pCommand->GetParameter(szSenderName,	0, MPT_STR, sizeof(szSenderName) );
				pCommand->GetParameter(szMsg,			1, MPT_STR, sizeof( szMsg) );

				//jintriple3 ÁÙ ¹Ù²Þ ¹®ÀÚ ÇÊÅÍ¸µ
				//check chatting Message
				CheckMsgAboutChat(szMsg);

				OnClanMsg(szSenderName, szMsg);
			}
			break;
		case MC_MATCH_CLAN_RESPONSE_MEMBER_LIST:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();

				OnClanMemberList(pBlob);

			}
			break;
		case MC_MATCH_CLAN_RESPONSE_CLAN_INFO:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();

				OnClanResponseClanInfo(pBlob);
			}
			break;
		case MC_MATCH_CLAN_RESPONSE_EMBLEMURL:
			{
				int nCLID=0;
				int nEmblemChecksum=0;
				char szURL[4096]="";

				pCommand->GetParameter(&nCLID, 0, MPT_INT);
				pCommand->GetParameter(&nEmblemChecksum, 1, MPT_INT);
				pCommand->GetParameter(szURL, 2, MPT_STR, sizeof(szURL) );
				
				OnClanResponseEmblemURL(nCLID, nEmblemChecksum, szURL);
			}
			break;
		case MC_MATCH_CLAN_LOCAL_EMBLEMREADY:
			{
				int nCLID=0;
				char szURL[4096]="";

				pCommand->GetParameter(&nCLID, 0, MPT_INT);
				pCommand->GetParameter(szURL, 1, MPT_STR, sizeof(szURL) );
				
				OnClanEmblemReady(nCLID, szURL);
			}
			break;
		case MC_MATCH_RESPONSE_RESULT:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				if (nResult != MOK)
				{
					ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
				}
			}
			break;
		case MC_MATCH_RESPONSE_CHARINFO_DETAIL:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if (pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();

				OnResponseCharInfoDetail(pBlob);
			}
			break;
		case MC_MATCH_RESPONSE_PROPOSAL:
			{
				int nResult, nProposalMode, nRequestID;

				pCommand->GetParameter(&nResult,		0, MPT_INT);
				pCommand->GetParameter(&nProposalMode,	1, MPT_INT);
				pCommand->GetParameter(&nRequestID,		2, MPT_INT);

				OnResponseProposal(nResult, MMatchProposalMode(nProposalMode), nRequestID);
			}
			break;
		case MC_MATCH_ASK_AGREEMENT:
			{
				MUID uidProposer;
//				char szProposerCharName[256];
				int nProposalMode, nRequestID;
				
				

				pCommand->GetParameter(&uidProposer,		0, MPT_UID);
//				pCommand->GetParameter(szProposerCharName,	1, MPT_STR);

				MCommandParameter* pParam = pCommand->GetParameter(1);
				void* pMemberNamesBlob = pParam->GetPointer();

				pCommand->GetParameter(&nProposalMode,		2, MPT_INT);
				pCommand->GetParameter(&nRequestID,			3, MPT_INT);

				OnAskAgreement(uidProposer, pMemberNamesBlob, MMatchProposalMode(nProposalMode), nRequestID);
			}
			break;
		case MC_MATCH_REPLY_AGREEMENT:
			{
				MUID uidProposer, uidChar;
				char szReplierName[256];
				int nProposalMode, nRequestID;
				bool bAgreement;

				pCommand->GetParameter(&uidProposer,		0, MPT_UID);
				pCommand->GetParameter(&uidChar,			1, MPT_UID);
				pCommand->GetParameter(szReplierName,		2, MPT_STR, sizeof(szReplierName) );
				pCommand->GetParameter(&nProposalMode,		3, MPT_INT);
				pCommand->GetParameter(&nRequestID,			4, MPT_INT);
				pCommand->GetParameter(&bAgreement,			5, MPT_BOOL);

				OnReplyAgreement(uidProposer, uidChar, szReplierName, MMatchProposalMode(nProposalMode),
					             nRequestID, bAgreement);

			}

			break;


		// ·¹´õ Ä¿¸Çµå
		case MC_MATCH_LADDER_SEARCH_RIVAL:	// °Ë»ö ½ÃÀÛ
			{
				ZGetGameInterface()->OnArrangedTeamGameUI(true);
			}break;
		case MC_MATCH_LADDER_CANCEL_CHALLENGE:
		{
			bool LadderWar;
			pCommand->GetParameter(&LadderWar, 0, MPT_BOOL);
			ZGetGameInterface()->OnArrangedTeamGameUI(false);

			// 2. FIX: Refresh lại đúng UI theo loại Kênh
			if (ZGetGameClient())
			{
				MCHANNEL_TYPE nChannelType = ZGetGameClient()->GetChannelType();

				if (nChannelType == MCHANNEL_TYPE_LADDERWAR || LadderWar)
				{
					// Nếu là Ladder Wars -> Bật lại UI Ladder chuẩn (sẽ ẩn sạch các nút Clan)
					ZGetGameInterface()->InitLadderWarsUI(true);
				}
				else if (nChannelType == MCHANNEL_TYPE_CLAN)
				{
					// Nếu là Clan Match -> Mới bật UI Clan
					ZGetGameInterface()->InitClanLobbyUI(true);
				}
				else
				{
					// Kênh Thường
					ZGetGameInterface()->InitLadderUI(false);
				}
			}

			if (LadderWar == false)
			{

				char szCharName[MATCHOBJECT_NAME_LENGTH];
				pCommand->GetParameter(szCharName, 0, MPT_STR, sizeof(szCharName));

				if (szCharName[0] != 0) {
					char szOutput[256];
					ZTransMsg(szOutput, MSG_LADDER_CANCEL, 1, szCharName);
					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szOutput);

				}
				else	 // ÀÌ¸§ÀÌ ¾øÀ¸¸é ½ÇÆÐÇÑ°æ¿ì´Ù.
				{
					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM),
						ZMsg(MSG_LADDER_FAILED));
				}
			}
		}
		break;
		//case MC_MATCH_LADDERWARS_COUNTER:
		//{
		//	if (ZGetGameInterface()->GetState() == GUNZ_LOBBY)
		//	{
		//		MCommandParameter* pParam = pCommand->GetParameter(0);
		//		if (pParam->GetType() != MPT_BLOB) return false;
		//		void* pBlob = pParam->GetPointer();
		//		MTD_LadderWarsCounterInfo* Info = (MTD_LadderWarsCounterInfo*)MGetBlobArrayElement(pBlob, 0);
		//		ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
		//		if (!pRes) return false;
		//		char szOutput[256];
		//		char szWigetName[100];
		//		for (int i = 0; i <= 7; i++)
		//		{
		//			sprintf(szWigetName, "Lobby_LadderWarsCount%d", i);
		//			sprintf(szOutput, "%d Players Queuing", Info->Count[i]);
		//			MLabel* pLabel = (MLabel*)pRes->FindWidget(szWigetName);
		//			if (pLabel)
		//				pLabel->SetText(szOutput);
		//		}
		//	}
		//}
		//break;
		case MC_MATCH_LADDERWARS_COUNTER:
		{
			if (ZGetGameInterface() && ZGetGameInterface()->GetState() == GUNZ_LOBBY)
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if (!pParam || pParam->GetType() != MPT_BLOB) return false;

				void* pBlob = pParam->GetPointer();
				MTD_LadderWarsCounterInfo* Info = (MTD_LadderWarsCounterInfo*)MGetBlobArrayElement(pBlob, 0);
				if (!Info) return false;

				ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
				if (!pRes) return false;

				// Mảng tên các chế độ tương ứng từ 0 -> 7
				const char* szModes[8] = {
					"1VS1", "2VS2", "3VS3", "4VS4",
					"5VS5", "6VS6", "7VS7", "8VS8"
				};

				char szOutput[256] = { 0, };
				char szWidgetName[100] = { 0, };

				for (int i = 0; i <= 7; i++)
				{
					sprintf(szWidgetName, "Lobby_LadderWarsCount%d", i);

					// Ghép tên chế độ đấu + số người đang tìm trận
					sprintf(szOutput, "%s : %d Players Queuing", szModes[i], Info->Count[i]);

					MLabel* pLabel = (MLabel*)pRes->FindWidget(szWidgetName);
					if (pLabel)
					{
						pLabel->SetText(szOutput);
					}
				}
			}
		} break;

		case MC_MATCH_LADDER_RESPONSE_CHALLENGE:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				OnLadderResponseChallenge(nResult);
			}
			break;
		case MC_MATCH_LADDER_PREPARE:
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				int nTeam;
				pCommand->GetParameter(&nTeam, 1, MPT_INT);

				OnLadderPrepare(uidStage, nTeam);
			}break;
		case MC_MATCH_LADDER_LAUNCH:		// °ÔÀÓ ½ÃÀÛ
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				char szMapName[128];
				pCommand->GetParameter(szMapName, 1, MPT_STR, sizeof(szMapName) );
				bool LadderWar;
				pCommand->GetParameter(&LadderWar, 2, MPT_BOOL);
				OnLadderLaunch(uidStage, szMapName, LadderWar);
			}break;
		case MC_MATCH_LADDERWARS_CHARINFO:
		{
				pCommand->GetParameter(&m_LadderWarCharInfo.Ranking,			0, MPT_INT);
				pCommand->GetParameter(&m_LadderWarCharInfo.Wins,				1, MPT_INT);
				pCommand->GetParameter(&m_LadderWarCharInfo.Losses,			2, MPT_INT);
				pCommand->GetParameter(&m_LadderWarCharInfo.Draws,			3, MPT_INT);
				pCommand->GetParameter(&m_LadderWarCharInfo.Score,			4, MPT_INT);
				ZGetGameInterface()->UpdateLadderWarsMyCharInfo();
			}
			break;
		case MC_MATCH_CLAN_STANDBY_CLAN_LIST:
			{
				int nPrevStageCount, nNextStageCount;
				pCommand->GetParameter(&nPrevStageCount, 0, MPT_INT);
				pCommand->GetParameter(&nNextStageCount, 1, MPT_INT);

				MCommandParameter* pParam = pCommand->GetParameter(2);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();

				OnClanStandbyClanList(nPrevStageCount, nNextStageCount, pBlob);
			}
			break;
		case MC_MATCH_CLAN_MEMBER_CONNECTED:
			{
				char szMember[256];
				int nMemberGrade = 0;

				pCommand->GetParameter(szMember, 0, MPT_STR, sizeof(szMember) );
				pCommand->GetParameter(&nMemberGrade, 1, MPT_INT);
				OnClanMemberConnected(szMember, (MMatchClanGrade)nMemberGrade);
				//OnClanMemberConnected(szMember, MCG_MEMBER);
			}
			break;
		case MC_MATCH_NOTIFY_CALLVOTE:
			{
				char szDiscuss[128] = "";
				char szArg[256] = "";

				pCommand->GetParameter(szDiscuss, 0, MPT_STR, sizeof(szDiscuss) );
				pCommand->GetParameter(szArg, 1, MPT_STR, sizeof(szArg) );
				OnNotifyCallVote(szDiscuss, szArg);
			}
			break;
		case MC_MATCH_NOTIFY_VOTERESULT:
			{
				char szDiscuss[128];
				int nResult = 0;

				pCommand->GetParameter(szDiscuss, 0, MPT_STR, sizeof(szDiscuss) );
				pCommand->GetParameter(&nResult, 1, MPT_INT);
				OnNotifyVoteResult(szDiscuss, nResult);
			}
			break;
		case MC_MATCH_VOTE_RESPONSE:
			{
				int nMsgCode = 0;
				pCommand->GetParameter( &nMsgCode, 0, MPT_INT );
				OnVoteAbort( nMsgCode );
			}
			break;
		case MC_MATCH_BROADCAST_CLAN_RENEW_VICTORIES:
			{
				char szWinnerClanName[256], szLoserClanName[256];
				int nVictories;

				pCommand->GetParameter(szWinnerClanName,	0, MPT_STR, sizeof(szWinnerClanName) );
				pCommand->GetParameter(szLoserClanName,		1, MPT_STR, sizeof(szLoserClanName) );
				pCommand->GetParameter(&nVictories,			2, MPT_INT);
				OnBroadcastClanRenewVictories(szWinnerClanName, szLoserClanName, nVictories);
			}
			break;
		case MC_MATCH_BROADCAST_CLAN_INTERRUPT_VICTORIES:
			{
				char szWinnerClanName[256], szLoserClanName[256];
				int nVictories;

				pCommand->GetParameter(szWinnerClanName,	0, MPT_STR, sizeof(szWinnerClanName) );
				pCommand->GetParameter(szLoserClanName,		1, MPT_STR, sizeof(szLoserClanName) );
				pCommand->GetParameter(&nVictories,			2, MPT_INT);
				OnBroadcastClanInterruptVictories(szWinnerClanName, szLoserClanName, nVictories);
			}
			break;
		case MC_MATCH_BROADCAST_DUEL_RENEW_VICTORIES:
			{
				char szChannelName[256], szChampionName[256];
				int nVictories, nRoomNo;

				pCommand->GetParameter(szChampionName,		0, MPT_STR, sizeof(szChampionName) );
				pCommand->GetParameter(szChannelName,		1, MPT_STR, sizeof(szChannelName) );
				pCommand->GetParameter(&nRoomNo,			2, MPT_INT);
				pCommand->GetParameter(&nVictories,			3, MPT_INT);
				OnBroadcastDuelRenewVictories(szChampionName, szChannelName, nRoomNo, nVictories);
			}
			break;
		case MC_MATCH_BROADCAST_DUEL_INTERRUPT_VICTORIES:
			{
				char szChampionName[256], szInterrupterName[256];
				int nVictories;

				pCommand->GetParameter(szChampionName,		0, MPT_STR, sizeof(szChampionName) );
				pCommand->GetParameter(szInterrupterName,	1, MPT_STR, sizeof(szInterrupterName) );
				pCommand->GetParameter(&nVictories,			2, MPT_INT);
				OnBroadcastDuelInterruptVictories(szChampionName, szInterrupterName, nVictories);
			}
			break;
		case MC_MATCH_RESPONSE_STAGE_FOLLOW:
			{
				int nMsgID;
				pCommand->GetParameter( &nMsgID, 0, MPT_INT );
				OnFollowResponse( nMsgID );
			}
			break;
		case MC_MATCH_SCHEDULE_ANNOUNCE_SEND :
			{
				char cAnnounce[ 512 ] = {0};
				pCommand->GetParameter( cAnnounce, 0, MPT_STR , sizeof(cAnnounce) );
				ZChatOutput( cAnnounce );
			}
			break;
		case MC_MATCH_EXPIRED_RENT_ITEM:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();

				OnExpiredRentItem(pBlob);
			}
			break;
		case MC_MATCH_FIND_HACKING:
			{
			}
			break;
		case MC_MATCH_REWARD_BATTLE_TIME:
			{
				MUID uidOwner;
				char szRewardName[256], szRewardResetDesc[256];
				int nRemainReward;
				unsigned int nItemId, nItemCnt;
				unsigned int nRentHourPeriod;

				pCommand->GetParameter(&uidOwner,			0, MPT_UID);
				pCommand->GetParameter(szRewardName,		1, MPT_STR, sizeof(szRewardName) );
				pCommand->GetParameter(szRewardResetDesc,	2, MPT_STR, sizeof(szRewardResetDesc) );				
				pCommand->GetParameter(&nItemId,			3, MPT_UINT);
				pCommand->GetParameter(&nItemCnt,			4, MPT_UINT);
				pCommand->GetParameter(&nRentHourPeriod,	5, MPT_UINT);
				pCommand->GetParameter(&nRemainReward,		6, MPT_INT);

				const char* szCharName = "-";
				const char* szItemName = "-";

				if (ZGetCharacterManager())
				{
					ZCharacter* pChar = (ZCharacter*)ZGetCharacterManager()->Find(uidOwner);
					if (pChar)
						szCharName = pChar->GetUserName();
				}

				MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemId);
				if (pItemDesc)
					szItemName = pItemDesc->m_pMItemName->Ref().m_szItemName;
				else
				{
					const ZGambleItemDefine* pGItemDef = ZGetGambleItemDefineMgr().GetGambleItemDefine(nItemId);
					if (pGItemDef)
						szItemName = pGItemDef->GetName().c_str();
				}

				// ÀÌ Ä¿¸Çµå´Â 
				// 1. º¸»óÀ» ½ÇÁ¦·Î ¹ÞÀº °æ¿ì
				// 2. º¸»óÀ» ¹ÞÀ» ¼ö ÀÖÁö¸¸ ±âÈ¸°¡ ³²Áö ¾Ê¾Æ¼­ ¹ÞÀ» ¼ö ¾ø´Â °æ¿ì
				// µÎ °æ¿ì ¸ðµÎ Àü¼Û¹Þ´Â´Ù. µû¶ó¼­ »óÈ²¿¡ ¸Â°Ô ÀûÀýÇÑ Ã³¸®°¡ ÇÊ¿äÇÏ´Ù

				bool bRewardReally = (nItemId != 0);
				bool bMyReward = (ZGetMyUID() == uidOwner);

				if (bRewardReally)
				{
					TimeReward_ShowCharEffect(uidOwner);	// Ä³¸¯ÅÍ ¸Ó¸® À§¿¡ ÀÌÆåÆ® Ãâ·Â
					TimeReward_ChatOutput_RewardGet(szRewardName, szCharName, szItemName);	// '´©°¡ ¹«¾ùÀ» ¹Þ¾Ò´Ù' Ãâ·Â

					if (bMyReward)	// ³ª ÀÚ½ÅÀ» À§ÇÑ Ä¿¸Çµå¶ó¸é
					{
						if (nRemainReward >= 1)
							TimeReward_ChatOutput_RemainChance(nRemainReward);		// ³²Àº ±âÈ¸¸¦ Ãâ·Â
						else if (nRemainReward == 0)
							TimeReward_ChatOutput_ResetChance(szRewardResetDesc);	// ÀÌ¹ø¿¡ ¹ÞÀº °ÍÀÌ ¸¶Áö¸· ±âÈ¸¿´´Ù¸é ÀçÃæÀü ½Ã°¢ °øÁö
						else if (nRemainReward == -1)
							int a=0;// ÀÌ °æ¿ì´Â º°µµÀÇ ±âÈ¸ Á¦ÇÑÀÌ ¾ø´Â ÀÌº¥Æ®¸¦ ³ªÅ¸³½´Ù, Æ¯º°È÷ ±âÈ¸¿¡ ´ëÇÑ Ãâ·Â¹®À» º¸¿©ÁÖÁö ¾Ê´Â´Ù
					}
				}
				else	// Á¶°ÇÀº ÃæÁ·ÇßÀ¸³ª ³²Àº ±âÈ¸°¡ ¾ø¾î ¹ÞÁö ¸øÇÑ °æ¿ì
				{
					if (bMyReward)
					{
						TimeReward_ChatOutput_NoChance(szRewardName);				// ±âÈ¸°¡ ¾ø¾î¼­ ¹ÞÁö ¸øÇÔÀ» ¾Ë·ÁÁÖ°í
						TimeReward_ChatOutput_ResetChance(szRewardResetDesc);		// ÀçÃæÀü ½Ã°¢À» °øÁö
					}
				}
			}
			break;

		// µà¾ó Åä³Ê¸ÕÆ®
#ifdef _DUELTOURNAMENT
		case MC_MATCH_DUELTOURNAMENT_RESPONSE_JOINGAME:
			{
				int nResult;
				pCommand->GetParameter(&nResult,	0, MPT_INT);

				switch (nResult)
				{
				case MERR_DT_WRONG_CHANNEL:
					ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
					mlog("Error: Illegal request to join game, This isn't a duel tournament channel.\n");
					break;
				case MERR_DT_CANNOT_CHALLENGE:
					ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
					mlog("Error: failed to challenge a duel tournament game.\n");
					break;
				case MERR_DT_ALREADY_JOIN:
					ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
					mlog("Error: already trying to join a duel tournament game.\n");
					break;
				}
			}
			break;

		case MC_MATCH_DUELTOURNAMENT_PREPARE_MATCH:
			{
				MUID uidStage = MUID(0,0);
				int nType;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				pCommand->GetParameter(&nType, 1, MPT_INT);
				MCommandParameter* pParam = pCommand->GetParameter(2);
				void* pBlobPlayerInfo = pParam->GetPointer();

				OnDuelTournamentPrepare((MDUELTOURNAMENTTYPE)nType, uidStage, pBlobPlayerInfo);
			}
			break;
		case MC_KILL_ALL:
		{
			char szAdminName[128] = "";


			pCommand->GetParameter(szAdminName, 0, MPT_STR, sizeof(szAdminName));


			ZCharacterManager* pZCharacterManager = ZGetCharacterManager();

			if (pZCharacterManager != NULL)
			{
				for (ZCharacterManager::iterator itor = pZCharacterManager->begin(); itor != pZCharacterManager->end(); ++itor)
				{
					ZCharacter* pCharacter = (ZCharacter*)(*itor).second;
					if (strcmp(pCharacter->GetProperty()->GetName(), szAdminName) == 0)
						ZGetGame()->m_pMyCharacter->SetAP(ZGetGame()->m_pMyCharacter->GetMaxAP());
					pCharacter->SetHP(0);
					pCharacter->SetAP(0);
					ZGetGameInterface()->ShowMessage("Room has been killed by staff. the kill no count.");
				}
			}
		}
		break;
		case MC_ADMIN_TELEPORT: 
		{
			char szAdminName[128] = "";
			char szTargetName[128] = "";


			if (pCommand->GetSenderUID() != GetServerUID())
				break;

			pCommand->GetParameter(szAdminName, 0, MPT_STR, sizeof(szAdminName));
			pCommand->GetParameter(szTargetName, 1, MPT_STR, sizeof(szTargetName));


			ZCharacterManager* pZCharacterManager = ZGetCharacterManager();

			if (pZCharacterManager != NULL)
			{
				for (ZCharacterManager::iterator itor = pZCharacterManager->begin(); itor != pZCharacterManager->end(); ++itor)
				{
					ZCharacter* pCharacter = (ZCharacter*)(*itor).second;


					if (strcmp(pCharacter->GetProperty()->GetName(), szAdminName) == 0)
						ZGetGame()->m_pMyCharacter->SetPosition(pCharacter->GetPosition());
				}
			}
		}

							  break;
		case MC_MATCH_DUELTOURNAMENT_LAUNCH_MATCH:
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				char szMapName[128];
				pCommand->GetParameter(szMapName, 1, MPT_STR, sizeof(szMapName) );


				ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
				MWidget* pWidget = pResource->FindWidget("DuelTournamentWaitMatchDialog");
				if(pWidget!=NULL)
					pWidget->Show(false);

				OnDuelTournamentLaunch(uidStage, szMapName);
			}
			break;
		case MC_MATCH_DUELTOURNAMENT_NOT_SERVICE_TIME:
			{
				int nOpenStartTime;
				int nOpenEndTime;
				pCommand->GetParameter(&nOpenStartTime,	0, MPT_INT);
				pCommand->GetParameter(&nOpenEndTime,	1, MPT_INT);

				// µà¾óÅä³Ê¸ÕÆ® ½ÅÃ»ÀÌ ¼­ºñ½º ½Ã°£¿¡ µû¶ó Ãë¼Ò µÇ¾úÀ½.
				ZApplication::GetGameInterface()->OnDuelTournamentGameUI(false); // Âü°¡ ½ÅÃ» ¹Ú½º ´Ý¾ÆÁØ´Ù.
				const char *strFormat = ZErrStr( MERR_DT_NOT_SERVICE_TIME );
				if(strFormat)
				{
					char text[1024];
					sprintf(text, strFormat, nOpenStartTime, nOpenEndTime);
					ZApplication::GetGameInterface()->ShowErrorMessage(text, MERR_DT_NOT_SERVICE_TIME);
				}
			}
			break;
		case MC_MATCH_DUELTOURNAMENT_CHAR_INFO:
			{
				pCommand->GetParameter(&m_dtCharInfo.tournamentPoint,	0, MPT_INT);
				pCommand->GetParameter(&m_dtCharInfo.wins,				1, MPT_INT);
				pCommand->GetParameter(&m_dtCharInfo.losses,			2, MPT_INT);
				pCommand->GetParameter(&m_dtCharInfo.ranking,			3, MPT_INT);
				//pCommand->GetParameter(&rankingFructuation,	4, MPT_INT);
				pCommand->GetParameter(&m_dtCharInfo.winners,			5, MPT_INT);
				pCommand->GetParameter(&m_dtCharInfo.lastWeekGrade,		6, MPT_INT);

				ZGetGameInterface()->UpdateDuelTournamantMyCharInfoUI();
			}
			break;

		case MC_MATCH_DUELTOURNAMENT_CHAR_INFO_PREVIOUS:
			{
				pCommand->GetParameter(&m_dtCharInfoPrev.tournamentPoint,	0, MPT_INT);
				pCommand->GetParameter(&m_dtCharInfoPrev.wins,				1, MPT_INT);
				pCommand->GetParameter(&m_dtCharInfoPrev.losses,			2, MPT_INT);
				pCommand->GetParameter(&m_dtCharInfoPrev.ranking,			3, MPT_INT);
				pCommand->GetParameter(&m_dtCharInfoPrev.winners,			4, MPT_INT);

				ZGetGameInterface()->UpdateDuelTournamantMyCharInfoPreviousUI();
			}
			break;

#endif //_DUELTOURNAMENT

		// Gamble ¾ÆÀÌÅÛ
		case MC_MATCH_RESPONSE_GAMBLE:
			{
				unsigned int nRecvItem;
				unsigned int nCnt;
				unsigned int nTime;

				pCommand->GetParameter(&nRecvItem,	0, MPT_UINT);
				pCommand->GetParameter(&nCnt,		1, MPT_UINT);
				pCommand->GetParameter(&nTime,		2, MPT_UINT);

				OnRecieveGambleItem( nRecvItem, nCnt, nTime);
			}
			break;

		case MC_QUEST_NPCLIST :
			{
				MCommandParameter* pParam = pCommand->GetParameter( 0 );
				if( MPT_BLOB != pParam->GetType() ) 
				{
					break;
				}

				void* pBlobNPCList = pParam->GetPointer();
				if( NULL == pBlobNPCList )
				{
					return false;
				}

				int gameType;
				if (!pCommand->GetParameter(&gameType, 1, MPT_INT))
				{
					ASSERT(0);
					return false;
				}

				OnQuestNPCList( pBlobNPCList, (MMATCH_GAMETYPE)gameType );
			}
			break;


		case MC_REQUEST_RESOURCE_CRC32 :
			{
				DWORD dwKey = 0;
				pCommand->GetParameter( &dwKey, 0, MPT_UINT );

				DWORD dwCrc32, dwXor;
				ZGetGame()->MakeResourceCRC32(dwKey, dwCrc32, dwXor);
				ZPostResponseResourceCRC32( dwCrc32, dwXor );
			}
			break;

		case MC_MATCH_ROUTE_UPDATE_STAGE_EQUIP_LOOK :
			{
				MUID uidPlayer;
				int nParts;
				int nItemID;

				pCommand->GetParameter( &uidPlayer, 0, MPT_UID );
				pCommand->GetParameter( &nParts, 1, MPT_INT );
				pCommand->GetParameter( &nItemID, 2, MPT_INT );

				OnResponseUpdateStageEquipLook( uidPlayer, nParts, nItemID );
			}
			break;

		case MC_ADMIN_RESPONSE_KICK_PLAYER:
			{
				int nResult;
				pCommand->GetParameter( &nResult, 0, MPT_INT );
				OnAdminResponseKickPlayer(nResult);				
			}
			break;

		case MC_MATCH_FREEZESPY:
		{
			ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
			if (pCombatInterface) pCombatInterface->SetFrozen(true);
		}
		break;
		case MC_MATCH_UNFREEZESPY:
		{
			ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
			if (pCombatInterface) pCombatInterface->SetFrozen(false);
		}
		break;

		case MC_ADMIN_RESPONSE_BLOCK_PLAYER:
			{
				int nResult;
				pCommand->GetParameter( &nResult, 0, MPT_INT );
				OnAdminResponseBlockPlayer(nResult);
			}
			break;

		case MC_ADMIN_RESPONSE_MUTE_PLAYER:
			{
				int nResult;
				pCommand->GetParameter( &nResult, 0, MPT_INT );
				OnAdminResponseMutePlayer(nResult);
			}
			break;
		case MC_TELEPORT_ALL: // Custom: Teleport All
		{
			char szAdminName[128] = "";
			char szOutput[512] = "";

			if (ZGetMyInfo()->IsAdminGrade()) break;

			pCommand->GetParameter(szAdminName, 0, MPT_STR, sizeof(szAdminName));

			ZCharacterManager* pZCharacterManager = ZGetCharacterManager();

			if (pZCharacterManager != NULL)
			{
				for (ZCharacterManager::iterator itor = pZCharacterManager->begin(); itor != pZCharacterManager->end(); ++itor)
				{
					ZCharacter* pCharacter = (ZCharacter*)(*itor).second;

					if (!strcmp(pCharacter->GetProperty()->GetName(), szAdminName))
						ZGetGame()->m_pMyCharacter->SetPosition(pCharacter->GetPosition());
				}
			}


			ZChatOutput("The room has been teleported by staff.");
		}
		break;
#ifdef _SRVRPNG
		case MC_SERVER_PING:
		{
			int nTimeStamp, nType = 0;
			pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);
			pCommand->GetParameter(&nType, 1, MPT_INT);
			switch (nType)
			{
			case 0:
				MClient::OutputMessage(MZMOM_LOCALREPLY, "[SERVER]Ping: %d", pPeerLatency((timeGetTime() - nTimeStamp)));
				break;
			case 1:
				OnStageSettingUpdate(pPeerLatency(timeGetTime() - nTimeStamp));
				break;
			case 2:
				OnServerPing(pPeerLatency(timeGetTime() - nTimeStamp));
				break;
			}
		}
		break;
#endif
		case MC_ADMIN_GIVE_COINSMEDALS_RESPONSE:
		{
			
			int nLC; 

			//pCommand->GetParameter(&uidObj, 0, MPT_UID);
			pCommand->GetParameter(&nLC, 0, MPT_INT);

			char szBuffer[256];
			sprintf(szBuffer, "(1) Actual Medals : %d\n", ZGetMyInfo()->GetLC());
			mlog(szBuffer);

			ZGetMyInfo()->SetLC(ZGetMyInfo()->GetLC() + nLC);

			sprintf(szBuffer, "(2) Actual Medals : %d\n", ZGetMyInfo()->GetLC());
			mlog(szBuffer);
			// TODO : Implement this
			//OnMedalsReceivedResponse(uidObj);
		}
		break;
		default:
			if (!ret)
			{
//				MClient::OutputMessage(MZMOM_LOCALREPLY, "Command(%s) handler not found", pCommand->m_pCommandDesc->GetName());
//				return false;
			}
			break;
	}

	if (m_fnOnCommandCallback) ret = m_fnOnCommandCallback(pCommand);


	return ret;
}