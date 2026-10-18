#include "stdafx.h"

#include "ZApplication.h"
#include "ZInterfaceListener.h"
#include "MWidget.h"
#include "MEdit.h"
#include "MComboBox.h"
#include "ZMapListBox.h"
#include "ZPost.h"
#include "MMatchStage.h"
#include "ZConfiguration.h"
#include "MSlider.h"
#include "MMatchStage.h"
#include "ZCharacterView.h"
#include "ZCharacterViewList.h"
#include "ZCharacterSelectView.h"
#include "ZShop.h"
#include "ZMyItemList.h"
#include "ZMyInfo.h"
#include "ZStageSetting.h"
#include "MChattingFilter.h"
#include "ZRoomListBox.h"
#include "ZPlayerListBox.h"
#include "MDebug.h"
#include "ZChat.h"
#include "ZMsgBox.h"
#include "ZActionKey.h"
#include "ZPlayerSelectListBox.h"
#include "ZChannelListItem.h"
#include "MTabCtrl.h"

#include "ZApplication.h"
#include "ZServerView.h"
#include "ZCharacterView.h"

#include "ZMonsterBookInterface.h"

#include "MMatchGlobal.h"

#include "MMatchSpyMode.h"

#include "ZShopEquipInterface.h"
#include "ZShopEquipListbox.h"
#include <random>
#include "ZShop.h"
#include "WebViewURLMap.h"
#include "WebviewTokenHelper.h"
#include "ZEmojiPicker.h"
#include "ZEventManager.h"
#include "WebViewOverlay.h"
// Custom: Disable NHN auth
//#ifdef LOCALE_NHNUSA
//#include "ZNHN_USA_Report.h"
//#endif


// Chat Input Listener
class MChatInputListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MEDIT_ENTER_VALUE)==true){
			const char* szCommand = pWidget->GetText();
			ZChatOutput(szCommand);

			char szChat[512];
			strcpy(szChat, pWidget->GetText());
			if (ZGetGameInterface()->GetChat()->Input(szChat))
			{
			}

			pWidget->SetText("");
			return true;
		}
		else if ((MWidget::IsMsg(szMessage, MEDIT_CHAR_MSG)==true) || (MWidget::IsMsg(szMessage, MEDIT_KEYDOWN_MSG)==true))
		{
			ZGetGameInterface()->GetChat()->FilterWhisperKey(pWidget);
		}
		else if (MWidget::IsMsg(szMessage, MEDIT_EMOJI_PICKER) == true)
		{
			ZGetEmojiPicker().SetAnchor(pWidget->GetScreenRect().x, pWidget->GetScreenRect().y, pWidget->GetScreenRect().w);
			ZGetEmojiPicker().SetInsertCallback([](const char* p) {
				MEdit* pEdit = (MEdit*)ZGetGameInterface()->GetIDLResource()->FindWidget("ChannelChattingInput");
				if (!pEdit) pEdit = (MEdit*)ZGetGameInterface()->GetIDLResource()->FindWidget("StageChattingInput");
				if (pEdit) {
					pEdit->AddText(p);
					pEdit->SetFocus();
				}
				});
			ZGetEmojiPicker().Toggle();
			return true;
		}

		return false;
	}
};
MChatInputListener	g_ChatInputListener;



class MHotBarButton : public MButton{
protected:
	char	m_szCommandString[256];
protected:
	virtual bool OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString){
		m_pIcon = pBitmap;
		AttachToolTip(szString);
		strcpy(m_szCommandString, szItemString);
		//SetText(szString);
		return true;
	}

public:
	MHotBarButton(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL)
	: MButton(szName, pParent, pListener){
		strcpy(m_szCommandString, "Command is not assigned");
	}
	virtual bool IsDropable(MWidget* pSender){
		return true;
	}
	const char* GetCommandString(void){
		return m_szCommandString;
	}
};


class MHotBarButtonListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){
			MHotBarButton* pButton = (MHotBarButton*)pWidget;
			const char* szCommandString = pButton->GetCommandString();

			char szParse[256];
			ZGetGame()->ParseReservedWord(szParse, szCommandString);

			char szErrMsg[256];
			ZChatOutput(MCOLOR(0xFFFFFFFF), szCommandString);
			if( ZGetGameClient()->Post(szErrMsg, 256, szParse)==false ){
				ZChatOutput(MCOLOR(0xFFFFC600), szErrMsg);
			}

			return true;
		}
		return false;
	}
};
MHotBarButtonListener	g_HotBarButtonListener;

class MLoginListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true)
		{
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

			ZServerView* pServerList = (ZServerView*)pResource->FindWidget( "SelectedServer");
			if ( !pServerList)
				return false;

			ServerInfo* pServer = pServerList->GetSelectedServer();
			if ( pServer)
			{
				if ( pServer->nType == 0 )
					return false;

				if( !pServer->bIsLive )
					return false;

				MWidget* pWidget = pResource->FindWidget( "LoginOK");
				if ( pWidget)
					pWidget->Enable( false);

				pWidget = pResource->FindWidget( "LoginFrame");
				if ( pWidget)
					pWidget->Show( false);

				pWidget = pResource->FindWidget( "Login_ConnectingMsg");
				if ( pWidget)
					pWidget->Show( true);


				ZGetGameInterface()->m_bLoginTimeout = true;
				// Custom: Changed timeout to 7 seconds
				ZGetGameInterface()->m_dwLoginTimeout = timeGetTime() + 7000;//+ (90 * 1000);

				// Custom: MatchServer UDP Port
				//ZGetGameClient()->SetServerPeerPort( pServer->nPeerPort );

				if ( pServer->nType == 7)	// Debug server Type1À» Type7·Î º¯°æ
				{
					MWidget* pAddr = pResource->FindWidget( "ServerAddress");
					MWidget* pPort = pResource->FindWidget( "ServerPort");

					ZPostConnect( pAddr->GetText(), atoi(pPort->GetText()));		// Debug server
				}
				else
					ZPostConnect( pServer->szAddress, pServer->nPort);				// Game server

				MLabel* pLabel = (MLabel*)pResource->FindWidget( "LoginError");
				if ( pLabel)
					pLabel->SetText("");
			}
		}
		return false;
	}
};
MLoginListener	g_LoginListener;

class MLogoutListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){

			mlog("MLogoutListener !\n");
			// Ã¼Å©ÈÄ ·Î±×¾Æ¿ô~
			/////////////////
			ZPostDisconnect();
			ZGetGameInterface()->SetState(GUNZ_LOGIN);
			return true;
		}
		return false;
	}
};
MLogoutListener	g_LogoutListener;

class MExitListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){

			mlog("MExitListener !\n");
			ZApplication::Exit();

			return true;
		}
		return false;
	}
};
MExitListener	g_ExitListener;

// for new UI
static void ZStageOptShowPage(bool bMod)
{
	ZIDLResource* pRes = ZGetGameInterface()->GetIDLResource();
	MWidget* pPageOption = pRes->FindWidget("StageOpt_PageOption");
	MWidget* pPageMod = pRes->FindWidget("StageOpt_PageMod");
	if (pPageOption) pPageOption->Show(!bMod);
	if (pPageMod)    pPageMod->Show(bMod);

	MWidget* pInd0 = pRes->FindWidget("StageOpt_TabInd0");
	MWidget* pInd1 = pRes->FindWidget("StageOpt_TabInd1");
	if (pInd0) pInd0->Show(!bMod);
	if (pInd1) pInd1->Show(bMod);
}

// for new UI: OK phan nhanh theo noi dang dung
class MStageOptOkListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MBTN_CLK_MSG) == true) {
			mlog("OPT OK: state=%d\n", (int)ZGetGameInterface()->GetState());
			if (ZGetGameInterface()->GetState() == GUNZ_STAGE)
			{
				// dang trong phong : luu cai dat
				ZStageSetting::ApplyStageSettingDialog();
				return true;
			}
			// o lobby : tao phong moi
			ZStageOptRememberCreateCombos();
			return ZGetStageCreateBtnListener()->OnCommand(pWidget, szMessage);
		}
		return false;
	}
};
MStageOptOkListener	g_StageOptOkListener;

// for new UI: doi Mode thi dung lai danh sach Players / Round / Time Limit
class MStageOptModeListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MCMBBOX_CHANGED) == true) {
			ZIDLResource* pRes = ZGetGameInterface()->GetIDLResource();
			MComboBox* pSrc = (MComboBox*)pRes->FindWidget("StageOpt_Mode");
			MComboBox* pDst = (MComboBox*)pRes->FindWidget("StageType");
			if (pSrc && pDst && pSrc->GetSelIndex() >= 0)
			{
				pDst->SetSelIndex(pSrc->GetSelIndex());
				pDst->SetText(pSrc->GetString(pSrc->GetSelIndex()));
			}
			ZStageSetting::InitStageSettingGameFromGameType();
			ZStageOptMirrorCombos();
			return true;
		}
		return false;
	}
};
MStageOptModeListener	g_StageOptModeListener;

// for new UI: mui ten cua nut < gia tri >
class MStageOptStepperListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MBTN_CLK_MSG) == true) {
			ZStageOptStepperOnClick(pWidget);
			return true;
		}
		return false;
	}
};
MStageOptStepperListener	g_StageOptStepperListener;

class MStageOptTabOptionListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MBTN_CLK_MSG) == true) {
			ZStageOptShowPage(false);
			return true;
		}
		return false;
	}
};
MStageOptTabOptionListener	g_StageOptTabOptionListener;

class MStageOptTabModListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MBTN_CLK_MSG) == true) {
			ZStageOptShowPage(true);
			return true;
		}
		return false;
	}
};
MStageOptTabModListener	g_StageOptTabModListener;

class MChannelChatInputListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MEDIT_ENTER_VALUE)==true){
			char szChat[512];
			if (strlen(pWidget->GetText()) < 255)
			{
				strcpy(szChat, pWidget->GetText());
				if (ZGetGameInterface()->GetChat()->Input(szChat))
				{
					pWidget->SetText("");
				}
			}
			return true;
		}
		else if ((MWidget::IsMsg(szMessage, MEDIT_CHAR_MSG)==true) || (MWidget::IsMsg(szMessage, MEDIT_KEYDOWN_MSG)==true))
		{
			ZGetGameInterface()->GetChat()->FilterWhisperKey(pWidget);
		}
		else if (MWidget::IsMsg(szMessage, MEDIT_EMOJI_PICKER) == true)
		{
			ZGetEmojiPicker().SetAnchor(pWidget->GetScreenRect().x, pWidget->GetScreenRect().y, pWidget->GetScreenRect().w);
			ZGetEmojiPicker().SetInsertCallback([](const char* p) {
				MEdit* pEdit = (MEdit*)ZGetGameInterface()->GetIDLResource()->FindWidget("ChannelChattingInput");
				if (pEdit) {
					pEdit->AddText(p);
					pEdit->SetFocus();
				}
				});
			ZGetEmojiPicker().Toggle();
			return true;
		}

		return false;
	}
};
MChannelChatInputListener	g_ChannelChatInputListener;

class MStageChatInputListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MEDIT_ENTER_VALUE)==true){
			char szChat[512];
			if (strlen(pWidget->GetText()) < 255)
			{
				strcpy(szChat, pWidget->GetText());
				if (ZGetGameInterface()->GetChat()->Input(szChat))
				{
					pWidget->SetText("");
				}
			}
			return true;
		}
		else if ((MWidget::IsMsg(szMessage, MEDIT_CHAR_MSG)==true) || (MWidget::IsMsg(szMessage, MEDIT_KEYDOWN_MSG)==true))
		{
			ZGetGameInterface()->GetChat()->FilterWhisperKey(pWidget);
		}
		else if (MWidget::IsMsg(szMessage, MEDIT_EMOJI_PICKER) == true)
		{
			ZGetEmojiPicker().SetAnchor(pWidget->GetScreenRect().x, pWidget->GetScreenRect().y, pWidget->GetScreenRect().w);
			ZGetEmojiPicker().SetInsertCallback([](const char* p) {
				MEdit* pEdit = (MEdit*)ZGetGameInterface()->GetIDLResource()->FindWidget("StageChattingInput");
				if (pEdit) {
					pEdit->AddText(p);
					pEdit->SetFocus();
				}
				});
			ZGetEmojiPicker().Toggle();
			return true;
		}

		return false;
	}
};
MStageChatInputListener	g_StageChatInputListener;


class MGameStartListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){

			const MSTAGE_SETTING_NODE* pStageSetting = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting();

			// Custom: Modified too many players popup client-side (to be used only for Quest)
			int nPlayerCnt = (int)ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.size();
			if(nPlayerCnt > pStageSetting->nMaxPlayers && nPlayerCnt!=0 && pStageSetting->nMaxPlayers!=0 && (ZGetGameTypeManager()->IsQuestDerived(pStageSetting->nGameType)))
			{
				// ÃÖ´ëÀÎ¿øº¸´Ù ¹æÀÎ¿øÀÌ ¸¹À»½Ã Ã³¸®
				char szText[128] = {0,};
				sprintf(szText, "%s\n", ZErrStr(MERR_PERSONNEL_TOO_MUCH));
				ZGetGameInterface()->ShowMessage(szText);
				return false;
			}
			//Custom: Challenge Quest message popup client-side max player 4
			//if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
			//{
			//	char szText[256];

			//	if (nPlayerCnt > pStageSetting->nMaxPlayers && nPlayerCnt != 0 && pStageSetting->nMaxPlayers != 0)
			//	{
			//		sprintf_s(szText,sizeof(szText), "%s\n", ZErrStr(MERR_PERSONNEL_TOO_MUCH));
			//		ZGetGameInterface()->ShowMessage(szText);
			//		return false;
			//	}
			//	else if (nPlayerCnt < pStageSetting->nMaxPlayers && nPlayerCnt != 0 && pStageSetting->nMaxPlayers != 0)
			//	{
			//		szMessage = "When you start without the recommended number of players, you will receive experience, but you will not receive items and your time will not be added to the website. do you wish to continue?";
			//		ZGetGameInterface()->ShowConfirmMessage(szMessage, this);
			//		if(MWidget::IsMsg (szMessage, MMSGBOX_YES) == true)
			//		{
			//			return true;
			//		}
			//		return false;
			//	}
			//}

			// Custom: Disabled too many players popup client-side
			//int nPlayerCnt = (int)ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.size();
			//if(nPlayerCnt > pStageSetting->nMaxPlayers && nPlayerCnt!=0 && pStageSetting->nMaxPlayers!=0)
			//{
			//	// ÃÖ´ëÀÎ¿øº¸´Ù ¹æÀÎ¿øÀÌ ¸¹À»½Ã Ã³¸®
			//	char szText[128] = {0,};
			//	sprintf(szText, "%s\n", ZErrStr(MERR_PERSONNEL_TOO_MUCH));
			//	ZGetGameInterface()->ShowMessage(szText);
			//	return false;
			//}

			// ¸Ê µî·ÏÀÌ ¾ÈµÇ¾î ÀÖÀ¸¸é °ÔÀÓ ½ÃÀÛÀº ÇØÁÖÁö ¾Ê´Â´Ù.
			if(!ZApplication::GetStageInterface()->GetIsRelayMapRegisterComplete())
			{
				ZGetGameInterface()->ShowMessage(MSG_GAME_RELAYMAP_CONFIRM_BUTTON_PUSH);
				return true;
			}

			if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_SPY)
			{
				char szText[36];
#ifdef _SPYLADDER
				if (nPlayerCnt < 2)
#else
				if (nPlayerCnt < 4)
#endif
				{
					//sprintf_s(szText, sizeof(szText), "Not enough players to start a game.");
#ifdef _SPYLADDER
					ZGetGameInterface()->ShowMessage("There are not enough players to start a game, it takes 2 players or more to start the game.");
#else
					ZGetGameInterface()->ShowMessage("There are not enough players to start a game, it takes 4 or 5 players or more to start the game.");
#endif
					//ZChatOutput(szText);
					return false;
				}
				else if (nPlayerCnt > 8)
				{
					//sprintf_s(szText, sizeof(szText), "Too many players to start a game");
					ZGetGameInterface()->ShowMessage("Too many players to start a game, there is only capacity for 12 players in the room.");
					//ZChatOutput(szText);
					return false;
				}
				vector<int> vecMapIDs;
				if (ZApplication::GetStageInterface()->GetPlayableSpyMapList(nPlayerCnt, vecMapIDs))
				{
					void* pMapListBlob = MMakeBlobArray(sizeof(int), vecMapIDs.size());

					for (int i = 0; i < (int)vecMapIDs.size(); i++)
						*(int*)MGetBlobArrayElement(pMapListBlob, i) = vecMapIDs[i];

					ZPostSpyStageStart(pMapListBlob);

					MEraseBlobArray(pMapListBlob);
					return true;
				}
				else
				{
					ZChatOutput("No playable spymaps");
					return false;
				}

			}

			// ¸ÊÀÌ °ñ¶óÁ³À»¶§ ½ÃÀÛ °¡´É.
			if (ZGetGameClient()->GetMatchStageSetting()->GetMapName()[0] != 0)
			{
				ZApplication::GetStageInterface()->ChangeStageEnableReady(true);

				ZPostStageStart(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
			}
			else
			{
				ZGetGameInterface()->ShowMessage("¼±ÅÃÇÏ½Å ¸ÊÀÌ ¾ø½À´Ï´Ù. ¸ÊÀ» ¼±ÅÃÇØ ÁÖ¼¼¿ä.");
			}

			return true;
		}
		return false;
	}
};
MGameStartListener	g_GameStartListener;

class MMapChangeListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){
			ZGetGameInterface()->ShowWidget("MapFrame", true, true);

			return true;
		}
		return false;
	}
};
MMapChangeListener	g_MapChangeListener;

class MMapSelectListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
			ZMapListBox* pWidget = (ZMapListBox*)pResource->FindWidget("MapList");
			char szMapName[_MAX_DIR];
			strcpy(szMapName, pWidget->GetSelItemString());
			if(szMapName!=NULL){
				ZApplication::GetStageInterface()->SetMapName(szMapName);
				ZPostStageMap(ZGetGameClient()->GetStageUID(), szMapName);

				// ÇöÀç Ã¢À» ´Ý´Â´Ù.
				if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
			}

			return true;
		}
		return false;
	}
};
MMapSelectListener	g_MapSelectListener;



class MParentCloseListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){
			if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
			return true;
		}
		return false;
	}
};
MParentCloseListener	g_ParentCloseListener;


class MStageCreateFrameCallerListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
			// for new UI
			ZStageSetting::InitStageSettingGameType();			// dung danh sach che do
			ZStageSetting::InitStageSettingGameFromGameType();	// dung Players / Round / Time Limit theo che do

			// moi lan mo bang tao phong thi tra ban do ve mac dinh cua che do
			{
				MComboBox* pMap = (MComboBox*)pResource->FindWidget("MapSelection");
				if (pMap && pMap->GetCount() > 0)
				{
					pMap->SetSelIndex(0);
					pMap->SetText(pMap->GetString(0));
				}
			}

			ZStageOptMirrorCombos();

			MLabel* pTitle = (MLabel*)pResource->FindWidget("StageOpt_Title");
			if (pTitle) pTitle->SetText("Create room");

			MEdit* pNameEdit = (MEdit*)pResource->FindWidget("StageName");
			if (pNameEdit) pNameEdit->Enable(true);

			MWidget* pFindWidget = pResource->FindWidget("StageOptionFrame");
			if (pFindWidget != NULL) pFindWidget->Show(true, true);

			MEdit* pPassEdit = (MEdit*)pResource->FindWidget("StagePassword");
			if (pPassEdit!=NULL)
			{
				pPassEdit->SetMaxLength(STAGEPASSWD_LENGTH);
				pPassEdit->SetText("");
			}

			// Load saved mod selections from config
			{
				auto& rm = *ZGetConfiguration()->GetRoomMods();
				auto SetCheck = [&](const char* name, bool val) {
					MButton* p = (MButton*)pResource->FindWidget(name);
					if (p) p->SetCheck(val);
					};
				auto SetEdit = [&](const char* name, const char* val) {
					MEdit* p = (MEdit*)pResource->FindWidget(name);
					if (p) p->SetText(val);
					};
				auto SetComboIndex = [&](const char* name, int idx) {
					MComboBox* p = (MComboBox*)pResource->FindWidget(name);
					if (p) p->SetSelIndex(idx);
					};

				SetCheck("StageOpt_ModForceHPAP", rm.bForceHPAP);
				SetEdit("StageOpt_ModForceHP", rm.szForceHP);
				SetEdit("StageOpt_ModForceAP", rm.szForceAP);
				SetCheck("StageOpt_ModForceMelee", rm.bForceMelee);
				SetEdit("StageOpt_ModForceMeleeDelay", rm.szForceMeleeDelay);
				SetEdit("StageOpt_ModForceMeleeDmg", rm.szForceMeleeDmg);
				SetComboIndex("StageOpt_ModWeaponRestriction", rm.nWeaponRestriction);
				SetCheck("StageOpt_ModFastReload", rm.bFastReload);
				SetCheck("StageOpt_ModInfiniteAmmo", rm.bInfiniteAmmo);
				SetCheck("StageOpt_ModExplosion", rm.bExplosion);
				SetCheck("StageOpt_ModFlip", rm.bFlip);
				SetCheck("StageOpt_ModMassives", rm.bMassives);
				SetCheck("StageOpt_ModNinja", rm.bNinja);
				SetCheck("StageOpt_ModFPS", rm.bFPS);
				SetCheck("StageOpt_ModRollTheDice", rm.bRollTheDice);
				SetCheck("StageOpt_ModGravity", rm.bGravity);
				SetEdit("StageOpt_ModGravityVal", rm.szGravityVal);
				SetCheck("StageOpt_ModSpeed", rm.bSpeed);
				SetEdit("StageOpt_ModSpeedVal", rm.szSpeedVal);
				SetCheck("StageOpt_ModJump", rm.bJump);
				SetEdit("StageOpt_ModJumpVal", rm.szJumpVal);
				SetCheck("StageOpt_ModDamage", rm.bDamage);
				SetEdit("StageOpt_ModDamageVal", rm.szDamageVal);
				SetCheck("StageOpt_ModNoClothes", rm.bNoClothes);
				SetCheck("StageOpt_ModNoItems", rm.bNoItems);
				SetCheck("StageOpt_ModWIWeapon", rm.bWIWeapon);
				SetCheck("StageOpt_ModNoDamage", rm.bNoDamage);
				SetCheck("StageOpt_ModWireframe", rm.bWireframe);
			}

			return true;
		}
		return false;
	}
};
MStageCreateFrameCallerListener	g_StageCreateFrameCallerListener;




class MSelectCharacterComboBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MCMBBOX_CHANGED)==true)
		{
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

			if (ZGetGameInterface()->GetCharacterSelectView() != NULL)
			{
				ZGetGameInterface()->GetCharacterSelectView()->SelectChar( ZCharacterSelectView::GetSelectedCharacter() );
			}

			return true;
		}

		return false;
	}
};
MSelectCharacterComboBoxListener	g_SelectCharacterComboBoxListener;
class MStageRefreshServerPingCallerListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MBTN_CLK_MSG) == true) {
			ZPostServerPing(timeGetTime(), 1);
			return true;
		}
		else
			return false;
	}
};
MStageRefreshServerPingCallerListener	g_StageRefreshServerPingCaller;
MListener* ZGetChatInputListener(void)
{
	return &g_ChatInputListener;
}

MListener* ZGetLoginListener(void)
{
	return &g_LoginListener;
}

MListener* ZGetLogoutListener(void)
{
	return &g_LogoutListener;
}

MListener* ZGetExitListener(void)
{
	return &g_ExitListener;
}

// for new UI
MListener* ZGetStageOptTabOptionListener(void)
{
	return &g_StageOptTabOptionListener;
}

MListener* ZGetStageOptOkListener(void)
{
	return &g_StageOptOkListener;
}

MListener* ZGetStageOptModeListener(void)
{
	return &g_StageOptModeListener;
}

MListener* ZGetStageOptTabModListener(void)
{
	return &g_StageOptTabModListener;
}

MListener* ZGetStageOptStepperListener(void)
{
	return &g_StageOptStepperListener;
}

MListener* ZGetChannelChatInputListener(void)
{
	return &g_ChannelChatInputListener;
}

MListener* ZGetStageChatInputListener(void)
{
	return &g_StageChatInputListener;
}

MListener* ZGetGameStartListener(void)
{
	return &g_GameStartListener;
}

MListener* ZGetMapChangeListener(void)
{
	return &g_MapChangeListener;
}

MListener* ZGetMapSelectListener(void)
{
	return &g_MapSelectListener;
}

MListener* ZGetParentCloseListener(void)
{
	return &g_ParentCloseListener;
}
MListener* ZGetStageRefreshServerPingCallerListener(void)
{
	return &g_StageRefreshServerPingCaller;
}

MListener* ZGetStageCreateFrameCallerListener(void)
{
	return &g_StageCreateFrameCallerListener;
}

MListener* ZGetSelectCharacterComboBoxListener(void)
{
	return &g_SelectCharacterComboBoxListener;
}

#ifdef _LOBBYSET
BEGIN_IMPLEMENT_LISTENER(ZGetExtraStageSettingCallerListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ExtraStageSettingFrame");
	if(pWidget!=NULL)
		pWidget->Show(true, true);

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetExtraStageSettingApplyBtnListener, MBTN_CLK_MSG)
	ZStageSetting::ApplyStageSettingDialog();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetExtraStageSettingCloseBtnListener, MBTN_CLK_MSG)

	MWidget* pStage = ZGetGameInterface()->GetIDLResource()->FindWidget("ExtraStageSettingFrame");
	if(ZGetGameInterface()->GetState() == GUNZ_STAGE)
		pStage->Show(false, false);
END_IMPLEMENT_LISTENER()
#endif
// WebView2///////////////////////////////////
//BEGIN_IMPLEMENT_LISTENER(GenericOpenWebViewListener, MBTN_CLK_MSG)
//{
//	// Lấy tên widget từ XML
//	const char* widgetName = pWidget->m_szIDLName;
//
//	auto it = g_WebViewURLMap.find(widgetName);
//	const char* url = "https://gunz.vn"; // fallback
//
//	if (it != g_WebViewURLMap.end())
//		url = it->second.c_str();
//
//	ZGetGameInterface()->ToggleWebView(url);
//}
//END_IMPLEMENT_LISTENER()

// Hàm hỗ trợ lấy vị trí màn hình của khung WEBVIEW_CONTAINER
bool GetWebContainerRect(int& outX, int& outY, int& outW, int& outH)
{
	MWidget* pContainer = ZGetGameInterface()->GetIDLResource()->FindWidget("WEBVIEW_CONTAINER");
	MWidget* pArea = ZGetGameInterface()->GetIDLResource()->FindWidget("WebViewArea");

	// Nếu chưa tạo WebViewArea trong XML thì lấy pContainer
	MWidget* pTarget = pArea ? pArea : pContainer;
	if (!pContainer || !pTarget) return false;

	// 1. Hiển thị khung XML lên
	pContainer->Show(true);

	// 2. Lấy vị trí và kích thước của Widget trên độ phân giải ảo (Virtual Resolution của XML)
	MRECT r = pTarget->GetInitialClientRect();
	MPOINT pt = MClientToScreen(pTarget, MPOINT(r.x, r.y));

	// 3. Lấy kích thước thực tế của Client Window Game (tọa độ Win32)
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);
	int realScreenWidth = clientRect.right - clientRect.left;
	int realScreenHeight = clientRect.bottom - clientRect.top;

	// 4. Lấy độ phân giải Render Virtual từ GunZ Engine
	int virtualScreenWidth = RGetScreenWidth();
	int virtualScreenHeight = RGetScreenHeight();

	if (virtualScreenWidth <= 0 || virtualScreenHeight <= 0) {
		virtualScreenWidth = 800;
		virtualScreenHeight = 600;
	}

	// 5. Tính tỉ lệ Scale giữa Virtual UI và Win32 Client Screen
	float scaleX = (float)realScreenWidth / (float)virtualScreenWidth;
	float scaleY = (float)realScreenHeight / (float)virtualScreenHeight;

	// 6. Ép tọa độ thực tế để nhúng WebView2 vừa khít vùng WebViewArea
	outX = (int)(pt.x * scaleX);
	outY = (int)(pt.y * scaleY);
	outW = (int)(r.w * scaleX);
	outH = (int)(r.h * scaleY);

	return true;
}
BEGIN_IMPLEMENT_LISTENER(OpenGachaListener, MBTN_CLK_MSG)
{
	int x, y, w, h;
	if (GetWebContainerRect(x, y, w, h)) {
		OpenGachaInWebview(x, y, w, h);
	}
}END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(OpenBattlePassListener, MBTN_CLK_MSG)
{
	int x, y, w, h;
	if (GetWebContainerRect(x, y, w, h)) {
		OpenBattlePassInWebview(x, y, w, h);
	}
}END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(OpenMarketListener, MBTN_CLK_MSG)
{
	int x, y, w, h;
	if (GetWebContainerRect(x, y, w, h)) {
		OpenMarketInWebview(x, y, w, h);
	}
}END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(OpenLadderRewardListener, MBTN_CLK_MSG)
{
	int x, y, w, h;
	if (GetWebContainerRect(x, y, w, h)) {
		OpenLadderRewardInWebview(x, y, w, h);
	}
}END_IMPLEMENT_LISTENER()

// Listener cho nút Đóng (WEBVIEW_CLOSE_BTN)
BEGIN_IMPLEMENT_LISTENER(CloseWebViewListener, MBTN_CLK_MSG)
{
	MWidget* pContainer = ZGetGameInterface()->GetIDLResource()->FindWidget("WEBVIEW_CONTAINER");
	if (pContainer) pContainer->Show(false);

	ZHideWebWidget(); // Ẩn WebView2 Win32
}END_IMPLEMENT_LISTENER()
//////////////////////////////////////////////
#ifdef _SCREENSHOT_FILTER_CHAT
BEGIN_IMPLEMENT_LISTENER(ZGetScreenShotCallerButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->ShowScreenShotDialog( true);
END_IMPLEMENT_LISTENER();
BEGIN_IMPLEMENT_LISTENER(ZGetViewScreenShotCallerButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->ViewShowScreenShotDialog( true);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetExitScreenShotPictureCallerButtonListener, MBTN_CLK_MSG)
	MFrame* pFrame = (MFrame*)ZGetGameInterface()->GetIDLResource()->FindWidget("ScreenShotPictureFrame");
	pFrame->Show(false, false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetExitScreenShotCallerButtonListener, MBTN_CLK_MSG)
	MFrame* pMain = (MFrame*)ZGetGameInterface()->GetIDLResource()->FindWidget( "ScreenShot");
	pMain->Show(false, false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetNextScreenShotCallerButtonListener, MBTN_CLK_MSG)
		MFrame* pFrame = (MFrame*)ZGetGameInterface()->GetIDLResource()->FindWidget("ScreenShotPictureFrame");
		MListBox* pListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "ScreenShot_List");
		MPicture* pPicture = (MPicture*)ZGetGameInterface()->GetIDLResource()->FindWidget("ScreenShotPicture");
		pFrame->Show(true, true);
		pListBox->SetSelIndex(pListBox->GetSelIndex() + 1);
		MBitmapR2* m_pScreenShotView = new MBitmapR2;
		TCHAR szPath[ MAX_PATH];
		bool bRead = true;
		if ( GetMyDocumentsPath( szPath))
		{
			strcat( szPath, GUNZ_FOLDER);
			strcat( szPath, "/Screenshots/");
			CreatePath( szPath );
		}
		strcat(szPath, pListBox->GetSelItemString());
		pFrame->SetText( pListBox->GetSelItemString());

		MZFile::SetReadMode( MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE );
		bRead = m_pScreenShotView->Create( pListBox->GetSelItemString(), RGetDevice(), szPath, false);
		MZFile::SetReadMode( MZIPREADFLAG_MRS2 );

		pPicture->SetBitmap( m_pScreenShotView->GetSourceBitmap());
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetPrevScreenShotCallerButtonListener, MBTN_CLK_MSG)
		MFrame* pFrame = (MFrame*)ZGetGameInterface()->GetIDLResource()->FindWidget("ScreenShotPictureFrame");
		MListBox* pListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "ScreenShot_List");
		MPicture* pPicture = (MPicture*)ZGetGameInterface()->GetIDLResource()->FindWidget("ScreenShotPicture");
		pFrame->Show(true, true);
		pListBox->SetSelIndex(pListBox->GetSelIndex() - 1);
		MBitmapR2* m_pScreenShotView = new MBitmapR2;
		TCHAR szPath[ MAX_PATH];
		bool bRead = true;
		if ( GetMyDocumentsPath( szPath))
		{
			strcat( szPath, GUNZ_FOLDER);
			strcat( szPath, "/Screenshots/");
			CreatePath( szPath );
		}
		strcat(szPath, pListBox->GetSelItemString());
		pFrame->SetText( pListBox->GetSelItemString());

		MZFile::SetReadMode( MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE );
		bRead = m_pScreenShotView->Create( pListBox->GetSelItemString(), RGetDevice(), szPath, false);
		MZFile::SetReadMode( MZIPREADFLAG_MRS2 );

		pPicture->SetBitmap( m_pScreenShotView->GetSourceBitmap());
END_IMPLEMENT_LISTENER();

#endif
BEGIN_IMPLEMENT_LISTENER(ZGetMapListListener, MLB_ITEM_DBLCLK)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	ZMapListBox* pMapList = (ZMapListBox*)pResource->FindWidget("MapList");
	const char* pszSelItemString = pMapList->GetSelItemString();
	if (pszSelItemString) {
		char szMapName[_MAX_DIR];
		sprintf(szMapName, pszSelItemString);
		ZApplication::GetStageInterface()->SetMapName(szMapName);
		ZPostStageMap(ZGetGameClient()->GetStageUID(), szMapName);
		// ÇöÀç Ã¢À» ´Ý´Â´Ù.
		if(pWidget->GetParent()!=NULL) pWidget->GetParent()->GetParent()->Show(false);
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageListFrameCallerListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

	/*
	MWidget* pFindWidget = pResource->FindWidget("StageListFrame");
	if(pFindWidget!=NULL) pFindWidget->Show(true, true);
	*/
	ZGetGameClient()->StartStageList();
END_IMPLEMENT_LISTENER()

// Custom: Volumen VoiceChat.
BEGIN_IMPLEMENT_LISTENER(ZGetVoiceVolumeSizeSliderListener, MLIST_VALUE_CHANGED)
MSlider* pWidget = (MSlider*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("VoiceVolumeSlider");
if (pWidget)
{
	Z_ETC_VOICE_VOLUME = (float)((MSlider*)pWidget)->GetValue() / (float)10000;
	//ZGetSoundEngine()->SetEffectVolume(Z_ETC_VOICE_VOLUME);
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEffectVolumeSizeSliderListener, MLIST_VALUE_CHANGED)
MSlider* pWidget = (MSlider*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("EffectVolumeSlider");
if (pWidget)
{
	Z_AUDIO_EFFECT_VOLUME = (float)((MSlider*)pWidget)->GetValue() / (float)10000;
	ZGetSoundEngine()->SetEffectVolume(Z_AUDIO_EFFECT_VOLUME);
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetBGMVolumeSizeSliderListener, MLIST_VALUE_CHANGED)
MSlider* pWidget = (MSlider*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("BGMVolumeSlider");
if (pWidget)
{
	Z_AUDIO_BGM_VOLUME = (float)((MSlider*)pWidget)->GetValue() / (float)10000;
	ZGetSoundEngine()->SetMusicVolume(Z_AUDIO_BGM_VOLUME);
}
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetStageCreateBtnListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pNameWidget = pResource->FindWidget("StageName");
if (pNameWidget == NULL) return true;
char szStageName[512], szStagePassword[128];
bool bPrivate = false;
strcpy(szStageName, pNameWidget->GetText());

vector<MMatchRoomModifier> vCreateMods;

// Build room modifier tags from checkboxes
{
	std::string modTags;

	auto CheckMod = [&](const char* widgetName, const char* tag)
		{
			MButton* pBtn = (MButton*)pResource->FindWidget(widgetName);
			if (pBtn && pBtn->GetCheck())
				modTags += tag;
		};

	auto CheckModVal = [&](const char* widgetName, const char* editName,
		const char* tagOpen, const char* tagClose)
		{
			MButton* pBtn = (MButton*)pResource->FindWidget(widgetName);
			if (pBtn && pBtn->GetCheck())
			{
				MEdit* pEdit = (MEdit*)pResource->FindWidget(editName);
				if (pEdit && strlen(pEdit->GetText()) > 0)
					modTags += std::string(tagOpen) + pEdit->GetText() + tagClose;
				else
					modTags += std::string(tagOpen) + tagClose;
			}
		};

	// Force HP/AP
	{
		MButton* pBtn = (MButton*)pResource->FindWidget("ModForceHPAP");
		if (pBtn && pBtn->GetCheck())
		{
			MEdit* pHP = (MEdit*)pResource->FindWidget("ModForceHP");
			MEdit* pAP = (MEdit*)pResource->FindWidget("ModForceAP");
			const char* hp = (pHP && strlen(pHP->GetText()) > 0) ? pHP->GetText() : "";
			const char* ap = (pAP && strlen(pAP->GetText()) > 0) ? pAP->GetText() : "";
			if (strlen(hp) > 0 || strlen(ap) > 0)
			{
				modTags += "[HP=";
				modTags += (strlen(hp) > 0) ? hp : "0";
				modTags += "] [AP=";
				modTags += (strlen(ap) > 0) ? ap : "0";
				modTags += "]";
			}
			else
			{
				modTags += "[hpap]";
			}
		}
	}

	// Force Melee: chỉ emit tag khi có giá trị, checkbox ON với ô trống = no-op
	{
		MButton* pBtn = (MButton*)pResource->FindWidget("ModForceMelee");
		if (pBtn && pBtn->GetCheck())
		{
			MEdit* pD = (MEdit*)pResource->FindWidget("ModForceMeleeDelay");
			MEdit* pG = (MEdit*)pResource->FindWidget("ModForceMeleeDmg");
			const char* d = (pD && strlen(pD->GetText()) > 0) ? pD->GetText() : "";
			const char* g = (pG && strlen(pG->GetText()) > 0) ? pG->GetText() : "";
			if (strlen(d) > 0) { modTags += "[meleedelay="; modTags += d; modTags += "]"; }
			if (strlen(g) > 0) { modTags += "[meleedmg=";   modTags += g; modTags += "]"; }
		}
	}

	// Weapon Restriction (dropdown -> single tag)
	int nWR = 0;
	{
		MComboBox* pCB = (MComboBox*)pResource->FindWidget("ModWeaponRestriction");
		if (pCB) nWR = pCB->GetSelIndex();
		switch (nWR)
		{
		case 1: modTags += "[gla]";  break;	// Melee Only
		case 2: modTags += "[sgo]";  break;	// Shotgun Only
		case 3: modTags += "[rvo]";  break;	// Pistol/Revolver Only
		case 4: modTags += "[snio]"; break;	// Sniper Only
		case 5: modTags += "[ng]";   break;	// No Guns
		default: break;
		}
	}

	// Simple checkbox mods
	CheckMod("ModFastReload", "[r]");
	CheckMod("ModInfiniteAmmo", "[ia]");
	CheckMod("ModExplosion", "[ex]");
	CheckMod("ModFlip", "[f]");
	CheckMod("ModMassives", "[m]");
	CheckMod("ModNinja", "[n]");
	CheckMod("ModFPS", "[fps]");
	CheckMod("ModRollTheDice", "[rtd]");
	CheckMod("ModNoClothes", "[nc]");
	CheckMod("ModNoItems", "[ni]");
	CheckMod("ModWIWeapon", "[wiw]");
	CheckMod("ModNoDamage", "[nd]");
	CheckMod("ModWireframe", "[wire]");

	// Value mods
	CheckModVal("ModGravity", "ModGravityVal", "[g", "]");
	CheckModVal("ModSpeed", "ModSpeedVal", "[s", "]");
	CheckModVal("ModJump", "ModJumpVal", "[j", "]");
	CheckModVal("ModDamage", "ModDamageVal", "[dmg", "]");

	// for new UI: doc lai chinh chuoi tag vua dung
	ZParseRoomModsFromName(modTags.c_str(), vCreateMods);

	// Save mod selections to config
	{
		auto& rm = *ZGetConfiguration()->GetRoomMods();
		auto GetCheck = [&](const char* name) -> bool {
			MButton* p = (MButton*)pResource->FindWidget(name);
			return p && p->GetCheck();
			};
		auto GetEditText = [&](const char* name, char* dst, int dstSize) {
			MEdit* p = (MEdit*)pResource->FindWidget(name);
			if (p && strlen(p->GetText()) > 0)
				strncpy(dst, p->GetText(), dstSize - 1);
			else
				dst[0] = 0;
			};

		rm.bForceHPAP = GetCheck("ModForceHPAP");
		GetEditText("ModForceHP", rm.szForceHP, sizeof(rm.szForceHP));
		GetEditText("ModForceAP", rm.szForceAP, sizeof(rm.szForceAP));
		rm.bForceMelee = GetCheck("ModForceMelee");
		GetEditText("ModForceMeleeDelay", rm.szForceMeleeDelay, sizeof(rm.szForceMeleeDelay));
		GetEditText("ModForceMeleeDmg", rm.szForceMeleeDmg, sizeof(rm.szForceMeleeDmg));
		rm.nWeaponRestriction = nWR;
		rm.bFastReload = GetCheck("ModFastReload");
		rm.bInfiniteAmmo = GetCheck("ModInfiniteAmmo");
		rm.bExplosion = GetCheck("ModExplosion");
		rm.bFlip = GetCheck("ModFlip");
		rm.bMassives = GetCheck("ModMassives");
		rm.bNinja = GetCheck("ModNinja");
		rm.bFPS = GetCheck("ModFPS");
		rm.bRollTheDice = GetCheck("ModRollTheDice");
		rm.bGravity = GetCheck("ModGravity");
		GetEditText("ModGravityVal", rm.szGravityVal, sizeof(rm.szGravityVal));
		rm.bSpeed = GetCheck("ModSpeed");
		GetEditText("ModSpeedVal", rm.szSpeedVal, sizeof(rm.szSpeedVal));
		rm.bJump = GetCheck("ModJump");
		GetEditText("ModJumpVal", rm.szJumpVal, sizeof(rm.szJumpVal));
		rm.bDamage = GetCheck("ModDamage");
		GetEditText("ModDamageVal", rm.szDamageVal, sizeof(rm.szDamageVal));
		rm.bNoClothes = GetCheck("ModNoClothes");
		rm.bNoItems = GetCheck("ModNoItems");
		rm.bWIWeapon = GetCheck("ModWIWeapon");
		rm.bNoDamage = GetCheck("ModNoDamage");
		rm.bWireframe = GetCheck("ModWireframe");

		ZGetConfiguration()->Save();
	}
}

if (strlen(szStageName) <= 0)
{
	ZGetGameInterface()->ShowMessage("Enter a stage name.");
	return true;
}

MEdit* pPassEdit = (MEdit*)pResource->FindWidget("StagePassword");
if (pPassEdit)
{
	if ((strlen(pPassEdit->GetText()) > 0) && (strlen(pPassEdit->GetText()) <= STAGEPASSWD_LENGTH))
		bPrivate = true;
	else
		bPrivate = false;

	if (bPrivate == true)
	{
		strcpy(szStagePassword, pPassEdit->GetText());
	}
	else
	{
		memset(szStagePassword, 0, sizeof(szStagePassword));
	}
}

if (pWidget->GetParent() != NULL) pWidget->GetParent()->Show(false);
ZApplication::GetStageInterface()->ChangeStageButtons(false, true, false);

MSTAGE_SETTING_NODE setting = {};
setting.uidStage = MUID(0, 0);
setting.nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
setting.nRoundMax = 50;
setting.nLimitTime = 30 * 60 * 1000;
setting.nMaxPlayers = 8;
setting.bForcedEntryEnabled = true;
setting.bAutoTeamBalancing = true;
setting.Netcode = NetcodeType::P2PAntilead;

ZApplication::GetStageInterface()->ChangeStageGameSetting(&setting);

if (!MGetChattingFilter()->IsValidStr(szStageName, 1))
{
	char szMsg[256];
	ZTransMsg(szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr().c_str());
	ZGetGameInterface()->ShowMessage(szMsg);
}
else
{
	ZGetGameInterface()->EnableLobbyInterface(false);
	ZPostStageCreate(ZGetGameClient()->GetPlayerUID(), szStageName, bPrivate, szStagePassword, vCreateMods);
}
END_IMPLEMENT_LISTENER()

// ºñ¹Ð¹æ µé¾î°¡±â È®ÀÎ¹öÆ°
BEGIN_IMPLEMENT_LISTENER(ZGetPrivateStageJoinBtnListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	ZRoomListBox* pRoomListBox = (ZRoomListBox*)pResource->FindWidget("Lobby_StageList");
	if (pRoomListBox)
	{
		pRoomListBox->RequestSelPrivateStageJoin();
	}

	MWidget* pPrivateStageJoinFrame = pResource->FindWidget("PrivateStageJoinFrame");
	if (pPrivateStageJoinFrame)
	{
		pPrivateStageJoinFrame->Show(false);
	}
END_IMPLEMENT_LISTENER()


// Channel
BEGIN_IMPLEMENT_LISTENER(ZGetChannelListFrameCallerListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pFindWidget = pResource->FindWidget("ChannelListFrame");
	if(pFindWidget!=NULL) pFindWidget->Show(true, true);

	MButton* pButton = (MButton*)pResource->FindWidget("MyClanChannel");
	if ( pButton)
		pButton->Enable( ZGetMyInfo()->IsClanJoined());

	pButton = (MButton*)pResource->FindWidget("ChannelList_DuelTournament");
	if ( pButton)
		pButton->Show( ZGetGameClient()->IsEnabledDuelTournament());

	MCHANNEL_TYPE nCurrentChannelType = ZGetGameClient()->GetChannelType();
	ZGetGameInterface()->InitChannelFrame(nCurrentChannelType);
	ZGetGameClient()->StartChannelList(nCurrentChannelType);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetChannelListJoinButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MListBox* pChannelList = (MListBox*)pResource->FindWidget("ChannelList");
	ZChannelListItem* pItem = (ZChannelListItem*)pChannelList->GetSelItem();
	if (pItem) {
		ZGetGameClient()->RequestChannelJoin(pItem->GetUID());
	}
	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
	ZGetGameClient()->StopChannelList();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetChannelListCloseButtonListener, MBTN_CLK_MSG)
	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
	ZGetGameClient()->StopChannelList();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetChannelListListener, MLB_ITEM_DBLCLK)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MListBox* pChannelList = (MListBox*)pResource->FindWidget("ChannelList");
	ZChannelListItem* pItem = (ZChannelListItem*)pChannelList->GetSelItem();
	if (pItem) {
		ZGetGameClient()->RequestChannelJoin(pItem->GetUID());
	}
	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
	ZGetGameClient()->StopChannelList();
END_IMPLEMENT_LISTENER()


// ½ºÅ×ÀÌÁö Á¶ÀÎ
BEGIN_IMPLEMENT_LISTENER(ZGetStageJoinListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	ZRoomListBox* pRoomListBox = (ZRoomListBox*)pResource->FindWidget("Lobby_StageList");
	if (pRoomListBox)
	{
		pRoomListBox->RequestSelStageJoin();

	}
END_IMPLEMENT_LISTENER()


// ±âÅ¸ ¿É¼Ç
BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingCallerListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

// for new UI: dung chung khung voi tao phong, chi doi tieu de
MLabel* pTitle = (MLabel*)pResource->FindWidget("StageOpt_Title");
if (pTitle) pTitle->SetText("Option");

MEdit* pNameEdit = (MEdit*)pResource->FindWidget("StageName");
if (pNameEdit) pNameEdit->Enable(false);		// trong phong chua cho doi ten

// nap lai theo cai dat that cua phong truoc khi hien
ZStageOptMirrorCombos();

MWidget* pWidget = pResource->FindWidget("StageOptionFrame");
if (pWidget != NULL)
pWidget->Show(true, true);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingStageTypeListener, MCMBBOX_CHANGED)
	{
//		ZStageSetting::AdjustLimitTimeStageSettingDialog();
		ZStageSetting::InitStageSettingGameFromGameType();
		ZStageSetting::PostDataToServer();
	}
END_IMPLEMENT_LISTENER()

#ifdef _BOXLEAD
BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingChangedSettingListener, MBTN_CLK_MSG)
ZStageSetting::PostDataToServer();
END_IMPLEMENT_LISTENER()
#endif

BEGIN_IMPLEMENT_LISTENER(ZGetStageTeamRedListener, MBTN_CLK_MSG)
	MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("StageTeamRed");
	if(pButton && !pButton->GetCheck() ) pButton->SetCheck(true);
	pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("StageTeamBlue");
	if(pButton) pButton->SetCheck( false );
	ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_RED);
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetStageTeamBlueListener, MBTN_CLK_MSG)
	MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("StageTeamBlue");
	if(pButton && !pButton->GetCheck() ) pButton->SetCheck(true);
	pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("StageTeamRed");
	if(pButton) pButton->SetCheck( false );
	ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_BLUE);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageReadyListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
#ifdef _AFKSYSTEM
	ZGetGameInterface()->m_dwAfkTimer = timeGetTime();
#endif
	bool bReady = false;
	MButton* pReadyBtn = (MButton*)pResource->FindWidget("StageReady");
	if(pReadyBtn) bReady=pReadyBtn->GetCheck();

	MMatchObjectStageState nStageState;
	if (bReady)
		nStageState = MOSS_READY;
	else
		nStageState = MOSS_NONREADY;

	ZPostStageState(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), nStageState);
	ZApplication::GetStageInterface()->ChangeStageEnableReady(bReady);
END_IMPLEMENT_LISTENER()

// °üÀü ¸ðµå ¹öÆ°
BEGIN_IMPLEMENT_LISTENER(ZGetStageObserverBtnListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MButton* pObserverBtn = (MButton*)pResource->FindWidget("StageObserverBtn");
	if ( pObserverBtn)
	{
		if ( pObserverBtn->GetCheck())
			ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_SPECTATOR);
		else
		{
			MButton* pBlueBtn = (MButton*)pResource->FindWidget("StageTeamBlue");

			if ( ZGetGameInterface()->m_bTeamPlay) // ÆÀÀü ÀÌ¸é..
			{
				if ( pBlueBtn->GetCheck())
					ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_BLUE);
				else
					ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_RED);
			}
			else	// °³ÀÎÀüÀÌ¸é...
			{
				ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_ALL);
			}
		}
	}
END_IMPLEMENT_LISTENER()

// ½ºÅÂÀÌÁö º¯°æ½Ã
BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingChangedComboboxListener, MCMBBOX_CHANGED)
	ZStageSetting::PostDataToServer();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingApplyBtnListener, MBTN_CLK_MSG)
	ZStageSetting::ApplyStageSettingDialog();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingCancelBtnListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
MSTAGE_SETTING_NODE* pStage = (MSTAGE_SETTING_NODE*)ZGetGameClient()->GetMatchStageSetting()->GetStageSetting();
if (pResource == NULL || pStage == NULL) return true;

MWidget* pWidget = pResource->FindWidget("StageOptionFrame");
if (pWidget)
{
	MEdit* pEdit = (MEdit*)pResource->FindWidget("PingLimitLow");
	if (pEdit)
	{
		char ping[50];
		sprintf(ping, "%d", 0);
		pStage->nPingLimitHigh = 0;
		pStage->nPingLimitLow = 0;
		pEdit->SetText(ping);
		pEdit = (MEdit*)pResource->FindWidget("PingLimitHigh");
		if (pEdit)
			pEdit->SetText(ping);
	}
	pWidget->Show(false);
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyListener, MBTN_CLK_MSG)
	ZPostStageLeave(ZGetGameClient()->GetPlayerUID());//, ZGetGameClient()->GetStageUID());
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetLoginStateButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->SetState(GUNZ_LOGIN);
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetGreeterStateButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->SetState(GUNZ_GREETER);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetBattleExitButtonListener, MBTN_CLK_MSG)
	// Å¬·£ÀüÀÌ ÁøÇàÁßÀÌ¸é Á¾·áÇÏÁö ¾Ê´Â´Ù
	if (!ZGetGameClient()->IsCWChannel() && !ZGetGameClient()->IsLadderWarsChannel())
	{
		if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
//		ZGetGameInterface()->SetCursorEnable(false);
		ZGetGameInterface()->ReserveLeaveBattle();
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageExitButtonListener, MBTN_CLK_MSG)
	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
//	ZGetGameInterface()->SetCursorEnable(false);
	ZGetGameInterface()->ReserveLeaveStage();
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetCombatMenuCloseButtonListener, MBTN_CLK_MSG)
	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
//	ZGetGameInterface()->SetCursorEnable(false);
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetPreviousStateButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->SetState(GUNZ_PREVIOUS);
END_IMPLEMENT_LISTENER()



BEGIN_IMPLEMENT_LISTENER(ZGetShopCallerButtonListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
if (pResource)
{
	MWidget* pEventWidget = pResource->FindWidget("Event");
	if (pEventWidget) pEventWidget->Show(false);
}
int nState = ZGetGameInterface()->GetState();

// 1. Nếu bấm Shop từ màn Hideout / Select Char
if (nState == GUNZ_CHARSELECTION)
{
	if (ZCharacterSelectView::GetNumOfCharacter() > 0)
	{
		if (ZGetGameInterface()->GetCharacterSelectView() != NULL)
		{
			ZGetGameInterface()->OnCharSelect();
		}
	}
	else
	{
		ZGetGameInterface()->ShowMessage(MSG_GAME_NOCHARACTER);
		return true;
	}
}

ZGetShop()->m_ShopMode = ZSHOP_NORMAL;

// 2. BẮT BUỘC: Mở khung Dialog Shop lên trước!
ZGetGameInterface()->ShowShopDialog(true);

// 3. Sau khi Shop đã hiện, gọi OnClickShopMainTab(0) để nạp Tab New và dọn dẹp Widget
ZShopEquipInterface* pShopInterface = ZGetGameInterface()->GetShopEquipInterface();
if (pShopInterface)
{
	pShopInterface->OnClickShopMainTab(0); // 0 = ZSHOP_TAB_NEW
}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEventShopCallerButtonListener, MBTN_CLK_MSG)
int nState = ZGetGameInterface()->GetState();

// 1. Nếu bấm Shop từ màn Hideout (Chưa chọn nhân vật chính thức)
if (nState == GUNZ_CHARSELECTION)
{
	if (ZCharacterSelectView::GetNumOfCharacter() > 0)
	{
		if (ZGetGameInterface()->GetCharacterSelectView() != NULL)
		{
			// Gọi OnCharSelect ngầm để Server sync Data giới tính/đồ đạc của nhân vật
			ZGetGameInterface()->OnCharSelect();
		}
	}
	else
	{
		ZGetGameInterface()->ShowMessage(MSG_GAME_NOCHARACTER);
		return true;
	}
}
	ZGetShop()->m_ShopMode = ZSHOP_EVENT;
ZGetGameInterface()->ShowShopDialog();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetMedalShopCallerButtonListener, MBTN_CLK_MSG)
int nState = ZGetGameInterface()->GetState();

// 1. Nếu bấm Shop từ màn Hideout (Chưa chọn nhân vật chính thức)
if (nState == GUNZ_CHARSELECTION)
{
	if (ZCharacterSelectView::GetNumOfCharacter() > 0)
	{
		if (ZGetGameInterface()->GetCharacterSelectView() != NULL)
		{
			// Gọi OnCharSelect ngầm để Server sync Data giới tính/đồ đạc của nhân vật
			ZGetGameInterface()->OnCharSelect();
		}
	}
	else
	{
		ZGetGameInterface()->ShowMessage(MSG_GAME_NOCHARACTER);
		return true;
	}
}
	ZGetShop()->m_ShopMode = ZSHOP_LADDER;
	ZGetGameInterface()->ShowShopDialog();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShopCloseButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->ShowShopDialog(false);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentCallerButtonListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
if (pResource)
{
	MWidget* pEventWidget = pResource->FindWidget("Event");
	if (pEventWidget) pEventWidget->Show(false);
}
int nState = ZGetGameInterface()->GetState();

// 1. Nếu bấm Shop từ màn Hideout (Chưa chọn nhân vật chính thức)
if (nState == GUNZ_CHARSELECTION)
{
	if (ZCharacterSelectView::GetNumOfCharacter() > 0)
	{
		if (ZGetGameInterface()->GetCharacterSelectView() != NULL)
		{
			// Gọi OnCharSelect ngầm để Server sync Data giới tính/đồ đạc của nhân vật
			ZGetGameInterface()->OnCharSelect();
		}
	}
	else
	{
		ZGetGameInterface()->ShowMessage(MSG_GAME_NOCHARACTER);
		return true;
	}
}
	ZGetGameInterface()->ShowEquipmentDialog();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentCloseButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->ShowEquipmentDialog(false);

{	

	int nState = ZGetGameInterface()->GetState();

	if(nState==GUNZ_LOBBY)
	{
		ZCharacterViewList* pVLL = ZGetCharacterViewList(GUNZ_LOBBY);
		if(pVLL)
			pVLL->ChangeCharacterInfo();
	}
	else if(nState==GUNZ_STAGE)
	{
		ZCharacterViewList* pVLS = ZGetCharacterViewList(GUNZ_STAGE);
		if(pVLS)
			pVLS->ChangeCharacterInfo();
	}
}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCharSelectionCallerButtonListener, MBTN_CLK_MSG)

// 1. Luôn đóng các cửa sổ Shop/Inventory đè lên trước
ZGetGameInterface()->ShowShopDialog(false);
ZGetGameInterface()->ShowEventDialog(false);
ZGetGameInterface()->ShowEquipmentDialog(false);
ZGetGameInterface()->ShowWidget("Lobby", false);  // Xóa khung Lobby đè
ZGetGameInterface()->ShowWidget("Stage", false);

int nState = ZGetGameInterface()->GetState();

// 2. Phân luồng theo State
if (nState == GUNZ_CHARSELECTION)
{
	// Nếu đã ở màn Hideout, chỉ cần bật lại giao diện Chọn Nhân Vật và load danh sách
	ZGetGameInterface()->EnableCharSelectionInterface(true);
	ZGetGameInterface()->ChangeToCharSelection();
}
else
{
	// Nếu đang ở Lobby/Stage/Khác, thực hiện thoát hẳn về màn Chọn Nhân Vật
	ZGetGameInterface()->ChangeToCharSelection();
}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetQuickJoinButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->RequestQuickJoin();
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetLobbyCharInfoCallerButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->ShowErrorMessage( MERR_NOT_SUPPORT );
END_IMPLEMENT_LISTENER()

//------------------------------------------------------------------------------------------------------

BEGIN_IMPLEMENT_LISTENER(ZGetBuyCashButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->OnBuyCashButton();
END_IMPLEMENT_LISTENER()

//------------------------------------------------------------------------------------------------------

BEGIN_IMPLEMENT_LISTENER(ZGetBuyEventButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnBuyEventButton();
END_IMPLEMENT_LISTENER()

//------------------------------------------------------------------------------------------------------

BEGIN_IMPLEMENT_LISTENER(ZGetBuyMedalButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnBuyMedalButton();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetBuyButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->OnBuyButton();
END_IMPLEMENT_LISTENER()

//------------------------------------------------------------------------------------------------------

BEGIN_IMPLEMENT_LISTENER(ZGetSellButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->OnSellButton();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetBuyPackageButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnBuyPackageButton();
END_IMPLEMENT_LISTENER()

//------------------------------------------------------------------------------------------------


BEGIN_IMPLEMENT_LISTENER(ZGetCountableItemTradeDlgCloseListener, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->GetItemCountDlg()->Close();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCountableItemTradeDlgOkButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->GetItemCountDlg()->OnDlgDone();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCountableItemTradeDlgCountUpButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->GetItemCountDlg()->AddCount(1);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCountableItemTradeDlgCountDnButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->GetItemCountDlg()->AddCount(-1);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCountableItemTradeDlgCountChangeListener, MEDIT_TEXT_CHANGED)
	ZGetGameInterface()->GetShopEquipInterface()->GetItemCountDlg()->OnEditBoxChanged();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSellCashItemConfirmDlgOkButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->GetSellCashItemConfirmDlg()->OnOkButton();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSellCashItemConfirmDlgCancelListener, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->GetSellCashItemConfirmDlg()->Close();
END_IMPLEMENT_LISTENER()

//------------------------------------------------------------------------------------------------------


BEGIN_IMPLEMENT_LISTENER(ZGetEquipButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->Equip();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentSearchButtonListener, MBTN_CLK_MSG)
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSendAccountItemButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->OnSendAccountButton();	
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetBringAccountItemButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->OnBringAccountButton();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShopCachRechargeButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->ShowErrorMessage( MERR_NOT_SUPPORT );
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShopSearchCallerButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->ShowErrorMessage( MERR_NOT_SUPPORT );
END_IMPLEMENT_LISTENER()




// ¸Ê¼±ÅÃ

void PostMapname()
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MComboBox* pMapCombo = (MComboBox*)pResource->FindWidget("MapSelection");
	const char* pszSelItemString = pMapCombo->GetSelItemString();

	if (pszSelItemString) 
	{
		char szMapName[_MAX_DIR];
		sprintf(szMapName, pszSelItemString);
		mlog("POST MAP: [%s] stageUID=%u:%u\n", szMapName,
			ZGetGameClient()->GetStageUID().High, ZGetGameClient()->GetStageUID().Low);
/*
		// ¿ÏÀü ÇÏµåÄÚµù... -_-;
		if ( strcmp( szMapName, "Island") == 0)
			strcpy( szMapName, "island");
		else if ( strcmp( szMapName, "Port") == 0)
			strcpy( szMapName, "port");
		else if ( strcmp( szMapName, "Snow Town") == 0)
			strcpy( szMapName, "Snow_Town");
*/
		ZApplication::GetStageInterface()->SetMapName(szMapName);
		ZPostStageMap(ZGetGameClient()->GetStageUID(), szMapName);

		/// ¸ÊÀÌ º¯°æµÇ¸é ¸±·¹ÀÌ¸Ê ¸®½ºÆ® ¼¼ÆÃÁßÀ» ³¡³½´Ù.
		ZApplication::GetStageInterface()->SetIsRelayMapRegisterComplete(true);

		if(strcmp(MMATCH_MAPNAME_RELAYMAP, pszSelItemString) == 0)
		{
			// ¸±·¹ÀÌ¸ÊÀ» °í¸£¸é Ã³À½¿¡ ¸Ç¼Ç¸Ê ÇÏ³ª¸¦ °­Á¦·Î ³Ö¾îÁØ´Ù
			void* pRelayMapListBlob = MMakeBlobArray(sizeof(MTD_RelayMap), 1);	// ºí·° ¸¸µé±â, ¸Ê¸®½ºÆ® ¼¼ÆÃ
			MTD_RelayMap* pRelayMapList = (MTD_RelayMap*)MGetBlobArrayElement(pRelayMapListBlob, 0);
			pRelayMapList->nMapID = MGetMapDescMgr()->GetMapID(MMATCH_MAP_MANSION);

			ZPostStageRelayMapInfoUpdate(ZGetGameClient()->GetStageUID(), RELAY_MAP_TURN, RELAY_MAP_3REPEAT, pRelayMapListBlob);
			MEraseBlobArray(pRelayMapListBlob);
		}
	}
}

BEGIN_IMPLEMENT_LISTENER(ZGetMapComboListener, MCMBBOX_CHANGED)
	PostMapname();
END_IMPLEMENT_LISTENER()

//Custom: Lightmaps Time Code
BEGIN_IMPLEMENT_LISTENER(ZGetMapTimeComboListener, MCMBBOX_CHANGED)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MComboBox* pMapCombo = (MComboBox*)pResource->FindWidget("MapTime");

ZApplication::GetStageInterface()->SetMapTOD(pMapCombo->GetSelIndex());
ZPostStageMapLighting(ZGetGameClient()->GetStageUID(), pMapCombo->GetSelIndex());
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectMapPrevButtonListener, MBTN_CLK_MSG)

	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("MapSelection");

	if(pComboBox)
	{
		pComboBox->SetPrevSel();
		PostMapname();
	}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectMapNextButtonListener, MBTN_CLK_MSG)

	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("MapSelection");

	if(pComboBox)
	{
		pComboBox->SetNextSel();
		PostMapname();
	}

END_IMPLEMENT_LISTENER()


// ¼±ÅÃµÈ Ä³¸¯ÅÍ µ¹¸®¸ç º¸±â ( Ä«¸Þ¶ó µ¹¸®±â Ã³¸® )

BEGIN_IMPLEMENT_LISTENER(ZGetSelectCameraLeftButtonListener, MBTN_CLK_MSG)
 
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectCameraRightButtonListener, MBTN_CLK_MSG)

END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetCreateCharacterLeftButtonListener, MBTN_CLK_MSG)
	if(ZGetGameInterface()->GetCharacterSelectView())
		ZGetGameInterface()->GetCharacterSelectView()->CharacterLeft();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCreateCharacterRightButtonListener, MBTN_CLK_MSG)
	if(ZGetGameInterface()->GetCharacterSelectView())
		ZGetGameInterface()->GetCharacterSelectView()->CharacterRight();
END_IMPLEMENT_LISTENER()


// Ä³¸¯ÅÍ ¼±ÅÃ, »ý¼º °ü·Ã
static DWORD g_dwClockCharSelBtn = 0;
void CharacterSelect( int nNum)
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

	// ´õºí Å¬¸¯½Ã Ä³¸¯ÅÍ ¹Ù·Î ¼±ÅÃ
	if ( (ZCharacterSelectView::GetSelectedCharacter() == nNum) && ((timeGetTime() - g_dwClockCharSelBtn ) <= 300))
	{
		ZGetGameInterface()->OnCharSelect();

		return ;
	}

	g_dwClockCharSelBtn = timeGetTime();

	ZGetGameInterface()->ChangeSelectedChar( nNum);
}

BEGIN_IMPLEMENT_LISTENER( ZGetSelectCharacterButtonListener0, MBTN_CLK_MSG)
	CharacterSelect( 0);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetSelectCharacterButtonListener1, MBTN_CLK_MSG)
	CharacterSelect( 1);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetSelectCharacterButtonListener2, MBTN_CLK_MSG)
	CharacterSelect( 2);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetSelectCharacterButtonListener3, MBTN_CLK_MSG)
	CharacterSelect( 3);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectCharacterButtonListener, MBTN_CLK_MSG)

int nState = ZGetGameInterface()->GetState();

if (nState == GUNZ_CHARSELECTION)
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	if (pResource)
	{
		// 1. ẨN TỰ THỦ CÔNG CÁC WIDGET SHOP & EQUIPMENT 
		// (Tránh gọi ShowShopDialog(false) để engine không restore lại màn Hideout 1s)
		MWidget* pShopWidget = pResource->FindWidget("Shop");
		if (pShopWidget) pShopWidget->Show(false);

		MWidget* pEquipWidget = pResource->FindWidget("Equipment");
		if (pEquipWidget) pEquipWidget->Show(false);

		// 2. Dọn dẹp các Widget nền rác
		MWidget* pLobbyWidget = pResource->FindWidget("Lobby");
		if (pLobbyWidget) pLobbyWidget->Show(false);

		MWidget* pEventWidget = pResource->FindWidget("Event");
		if (pEventWidget) pEventWidget->Show(false);

		MWidget* pStageWidget = pResource->FindWidget("Stage");
		if (pStageWidget) pStageWidget->Show(false);
	}

	// 3. Tắt UI CharSelect để không bị giật về Hideout
	ZGetGameInterface()->EnableCharSelectionInterface(false);

	// 4. Bắn gói tin chọn nhân vật chuyển thẳng sang Lobby
	if (ZCharacterSelectView::GetNumOfCharacter() > 0)
	{
		MWidget* pLobbyWidget = pResource->FindWidget("CharSelection");
		if (pLobbyWidget) pLobbyWidget->Show(false);

		ZGetGameInterface()->ShowWidget("Load", true);

		if (ZGetGameInterface()->GetCharacterSelectView() != NULL)
		{
			ZGetGameInterface()->OnCharSelect();

			MWidget* pWidget = pResource ? pResource->FindWidget("CS_SelectCharDefKey") : NULL;
			if (pWidget) pWidget->Enable(false);
		}
	}
	else
	{
		ZGetGameInterface()->ShowMessage(MSG_GAME_NOCHARACTER);
	}
}
else if (nState == GUNZ_LOBBY)
{
	// Khi đã ở Lobby sẵn thì đóng Shop/Equip và hiện lại Lobby
	ZGetGameInterface()->ShowShopDialog(false);
	ZGetGameInterface()->ShowEquipmentDialog(false);
	ZGetGameInterface()->ShowEventDialog(false);
	ZGetGameInterface()->ShowWidget("Lobby", true);
}
else if (nState == GUNZ_STAGE)
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	bool bIsShopVisible = false;
	bool bIsEquipVisible = false;
	bool bIsEventVisible = false;

	if (pResource)
	{
		MWidget* pShop = pResource->FindWidget("Shop");
		MWidget* pEquip = pResource->FindWidget("Equipment");
		MWidget* pEvent = pResource->FindWidget("Event");

		if (pShop && pShop->IsVisible())   bIsShopVisible = true;
		if (pEquip && pEquip->IsVisible()) bIsEquipVisible = true;
		if (pEvent && pEvent->IsVisible()) bIsEventVisible = true;
	}

	// ĐIỀU KIỆN 1: Nếu đang bật Shop hoặc Inventory trong Room
	if (bIsShopVisible || bIsEquipVisible || bIsEventVisible)
	{
		// Đóng Shop & Inventory, quay lại màn Stage chính
		ZGetGameInterface()->ShowShopDialog(false);
		ZGetGameInterface()->ShowEquipmentDialog(false);
		ZGetGameInterface()->ShowEventDialog(false);
		ZGetGameInterface()->ShowWidget("Stage", true);
	}
	// ĐIỀU KIỆN 2: Đang đứng ở màn Stage chính (Không bật Shop/Inventory)
	else
	{
		// Bật Message Box hỏi người chơi có muốn thoát Room về Lobby không
		// Dùng mã Message Box thoát Stage có sẵn của GunZ (hoặc MSG_STAGE_LEAVE_CONFIRM)
		ZPostStageLeave(ZGetGameClient()->GetPlayerUID());
	}
}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShowCreateCharacterButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->SetState(GUNZ_CHARCREATION);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetDeleteCharacterButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

	char szName[256];
	sprintf( szName, "CharSel_Name%d", ZCharacterSelectView::GetSelectedCharacter() );
	MLabel* pLabel = (MLabel*)pResource->FindWidget( szName);
	
	if( ZCharacterSelectView::GetNumOfCharacter())
	{
		int ret = ZGetGameClient()->ValidateRequestDeleteChar();

		if (ret != ZOK)
		{
			ZGetGameInterface()->ShowMessage( ret );

			return true;
		}

		MLabel* pCharNameLabel = (MLabel*)pResource->FindWidget("CS_DeleteCharLabel");
		if (pCharNameLabel)
			pCharNameLabel->SetText( pLabel->GetText());

		MEdit* pYesEdit = (MEdit*)pResource->FindWidget("CS_DeleteCharNameEdit");
		if (pYesEdit)
			pYesEdit->SetText("");

		MWidget* pWidget = pResource->FindWidget("CS_ConfirmDeleteChar");
		if (pWidget)
		{
			pWidget->Show(true, true);
			pWidget->SetFocus();
		}
	}
END_IMPLEMENT_LISTENER()

// ÁøÂ¥ Ä³¸¯ÅÍ »èÁ¦
BEGIN_IMPLEMENT_LISTENER(ZGetConfirmDeleteCharacterButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

	char szName[256];
	sprintf( szName, "CharSel_Name%d", ZCharacterSelectView::GetSelectedCharacter() );
	MLabel* pLabel = (MLabel*)pResource->FindWidget( szName);

	if( ZCharacterSelectView::GetNumOfCharacter())
	{
		MEdit* pYesEdit = (MEdit*)pResource->FindWidget( "CS_DeleteCharNameEdit");
		if (pYesEdit)
		{
			if ( (!stricmp( pYesEdit->GetText(), ZMsg(MSG_MENUITEM_YES))) && (ZCharacterSelectView::GetSelectedCharacter() >= 0) )
			{
				// Å¬·£¿¡ °¡ÀÔµÇ¾î ÀÖÀ¸¸é Ä³¸¯ÅÍ¸¦ »èÁ¦ÇÒ ¼ö ¾ø´Ù.
				if ((ZCharacterSelectView::m_CharInfo[ZCharacterSelectView::GetSelectedCharacter()].m_bLoaded) && 
					( ZCharacterSelectView::m_CharInfo[ZCharacterSelectView::GetSelectedCharacter()].m_CharInfo.szClanName[0] == 0))
				{
					ZPostDeleteMyChar(ZGetGameClient()->GetPlayerUID(), ZCharacterSelectView::GetSelectedCharacter(), (char*)pLabel->GetText());

					//ZCharacterSelectView::SetSelectedCharacter(-1); 
				}
				else
					ZGetGameInterface()->ShowMessage( MSG_CLAN_PLEASE_LEAVE_FROM_CHAR_DELETE );
				}
			else
				ZGetGameInterface()->ShowMessage( MSG_CHARDELETE_ERROR );
		}

		MWidget* pWidget = pResource->FindWidget( "CS_ConfirmDeleteChar");
		if ( pWidget)
			pWidget->Show( false);
	}
END_IMPLEMENT_LISTENER()

// Ä³¸¯ »èÁ¦ È®ÀÎÃ¢ ´Ý±â
BEGIN_IMPLEMENT_LISTENER(ZGetCloseConfirmDeleteCharButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("CS_ConfirmDeleteChar");
	if (pWidget)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER()



BEGIN_IMPLEMENT_LISTENER(ZGetCreateCharacterButtonListener, MBTN_CLK_MSG)
	int nEmptySlotIndex=-1;
	for(int i=0;i<4;i++)
	{
		if(ZGetGameInterface()->GetCharacterSelectView()->IsEmpty(i))
		{
			nEmptySlotIndex=i;
			break;
		}
	}

	if(nEmptySlotIndex>=0)
	{
		ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
		MEdit* pEdit = (MEdit*)pResource->FindWidget("CC_CharName");
		MComboBox* pSexCB, *pHairCB, *pFaceCB, *pCostumeCB;
		pSexCB = (MComboBox*)pResource->FindWidget("CC_Sex");
		pHairCB = (MComboBox*)pResource->FindWidget("CC_Hair");
		pFaceCB = (MComboBox*)pResource->FindWidget("CC_Face");
		pCostumeCB = (MComboBox*)pResource->FindWidget("CC_Costume");


		// »ý¼º Á¶°ÇÀÎÁö È®ÀÎÇÑ´Ù.
		if ( (pSexCB == NULL) || (pHairCB == NULL) || (pFaceCB == NULL) || (pCostumeCB == NULL) && (pEdit == NULL))
			return true;


		// Ä³¸¯ÅÍ ÀÌ¸§ÀÇ ±æÀÌ¸¦ ±¸ÇÑ´Ù.
		int nNameLen = (int)strlen( pEdit->GetText());

		if ( nNameLen <= 0)						// ÀÌ¸§À» ÀÌ·ÂÇÏÁö ¾Ê¾Ò´Ù.
		{
			ZGetGameInterface()->ShowErrorMessage( MERR_PLZ_INPUT_CHARNAME);
			return true;
		}
		else if ( nNameLen < MIN_CHARNAME)		// ÀÌ¸§ÀÌ ³Ê¹« Âª´Ù.
		{
			ZGetGameInterface()->ShowErrorMessage( MERR_TOO_SHORT_NAME);
			return true;
		}
		else if ( nNameLen > MAX_CHARNAME)		// ÀÌ¸§ÀÌ Á¦ÇÑ ±ÛÀÚ¼ö¸¦ ³Ñ¾ú´Ù.
		{
			ZGetGameInterface()->ShowErrorMessage( MERR_TOO_LONG_NAME);
			return true;
		}


#ifndef _SIMNAMEKOR
		bool bIsAbuse = MGetChattingFilter()->IsValidStr(pEdit->GetText(), NULL, false);
		if (!bIsAbuse)
		{
			char szMsg[256];
			ZTransMsg(szMsg, MSG_WRONG_WORD_NAME, NULL, MGetChattingFilter()->GetLastFilteredStr().c_str());
			ZGetGameInterface()->ShowMessage(szMsg, NULL, MSG_WRONG_WORD_NAME);
			return true;
		}
#else
		bool bIsAbuse = MGetChattingFilter()->IsValidStr(pEdit->GetText(), 2, true);
		if (!bIsAbuse)
		{
			char szMsg[256];
			ZTransMsg(szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr().c_str());
			ZGetGameInterface()->ShowMessage(szMsg, NULL, MSG_WRONG_WORD_NAME);

			return true;
		}
#endif
		ZPostCreateMyChar(ZGetGameClient()->GetPlayerUID(), nEmptySlotIndex, (char*)pEdit->GetText(), pSexCB->GetSelIndex(),
			pHairCB->GetSelIndex(), pFaceCB->GetSelIndex(), pCostumeCB->GetSelIndex());

	}
END_IMPLEMENT_LISTENER()


void SetCharacterInfoGroup(int n)
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MButton* pBtn = (MButton*)pResource->FindWidget("ShowChar_infoGroup");
	if(pBtn)pBtn->SetCheck(n==0);
	pBtn = (MButton*)pResource->FindWidget("ShowEquip_InfoGroup");
	if(pBtn)pBtn->SetCheck(n==1);

	MWidget *pFrame=(MFrame*)pResource->FindWidget("Char_infoGroup");
	if(pFrame) pFrame->Show(n==0);
	pFrame=(MFrame*)pResource->FindWidget("Equip_InfoGroup");
	if(pFrame) pFrame->Show(n==1);
}

BEGIN_IMPLEMENT_LISTENER(ZGetShowCharInfoGroupListener, MBTN_CLK_MSG)
SetCharacterInfoGroup(0);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShowEquipInfoGroupListener, MBTN_CLK_MSG)
SetCharacterInfoGroup(1);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCancelCreateCharacterButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->SetState(GUNZ_CHARSELECTION);
END_IMPLEMENT_LISTENER()



BEGIN_IMPLEMENT_LISTENER(ZChangeCreateCharInfoListener, MCMBBOX_CHANGED)
	if (ZGetGameInterface()->GetCharacterSelectView() != NULL)
	{
		ZGetGameInterface()->GetCharacterSelectView()->OnChangedCharCostume();
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageForcedEntryToGameListener, MBTN_CLK_MSG)
	if(ZGetGameClient()->GetMatchStageSetting()->GetMapName()[0]!=0)
	{
		ZApplication::GetStageInterface()->ChangeStageEnableReady( true);

		ZPostRequestForcedEntry(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
	}
END_IMPLEMENT_LISTENER()

/*
void ShowPlayerListGroup(int i)
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ChannelPlayerList_all");
	if(pWidget!=NULL) ((MButton*)pWidget)->SetCheck(i==0?true:false);
	pWidget = pResource->FindWidget("ChannelPlayerList_friend");
	if(pWidget!=NULL) ((MButton*)pWidget)->SetCheck(i==1?true:false);
	pWidget = pResource->FindWidget("ChannelPlayerList_clan");
	if(pWidget!=NULL) ((MButton*)pWidget)->SetCheck(i==2?true:false);
}

BEGIN_IMPLEMENT_LISTENER(ZGetChannelPlayerOptionGroupAll, MBTN_CLK_MSG)
ShowPlayerListGroup(0);
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetChannelPlayerOptionGroupFriend, MBTN_CLK_MSG)
ShowPlayerListGroup(1);
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetChannelPlayerOptionGroupClan, MBTN_CLK_MSG)
ShowPlayerListGroup(2);
END_IMPLEMENT_LISTENER()
*/
void HideNewShopWidgets()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (!pResource) return;

	// 1. Ẩn Container khung cha của Tab New (nếu có)
	MWidget* pNewTab = pResource->FindWidget("Shop_New_Tab");
	if (pNewTab) pNewTab->Show(false);

	// 2. Ẩn CharacterView của Tab New
	MWidget* pNewCharView = pResource->FindWidget("Shop_NewTabCharacterView");
	if (pNewCharView) pNewCharView->Show(false);

	// 3. Ẩn và Reset các Preview Picture của Tab New
	MPicture* pNewPreview = (MPicture*)pResource->FindWidget("Shop_NewProfileItem_Preview");
	if (pNewPreview)
	{
		pNewPreview->Show(false);
		pNewPreview->SetBitmap(NULL);
	}

	MPicture* pNewPreviewBG = (MPicture*)pResource->FindWidget("Shop_NewProfileItemBG_Preview");
	if (pNewPreviewBG)
	{
		pNewPreviewBG->Show(false);
		pNewPreviewBG->SetBitmap(NULL);
	}
}

BEGIN_IMPLEMENT_LISTENER(ZGetAllEquipmentListCallerButtonListener, MBTN_CLK_MSG)
	
	ZGetGameInterface()->GetShopEquipInterface()->SelectShopTab(0);
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetMyAllEquipmentListCallerButtonListener, MBTN_CLK_MSG)
	
	ZGetGameInterface()->GetShopEquipInterface()->SelectShopTab(1);
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetCashEquipmentListCallerButtonListener, MBTN_CLK_MSG)
	
	ZGetGameInterface()->GetShopEquipInterface()->SelectShopTab(2);
END_IMPLEMENT_LISTENER()

// Listener cho Tab 3: Medal Shop
BEGIN_IMPLEMENT_LISTENER(ZGetMedalEquipmentListCallerButtonListener, MBTN_CLK_MSG)
	
	ZGetGameInterface()->GetShopEquipInterface()->SelectShopTab(3);
END_IMPLEMENT_LISTENER()

// Listener cho Tab 4: Event Shop
BEGIN_IMPLEMENT_LISTENER(ZGetEventEquipmentListCallerButtonListener, MBTN_CLK_MSG)
	
	ZGetGameInterface()->GetShopEquipInterface()->SelectShopTab(4);
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentCharacterTabButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentTab(0);
END_IMPLEMENT_LISTENER()

// Áß¾ÓÀºÇà
BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentAccountTabButtonListener, MBTN_CLK_MSG)
	// À¯Á®°¡ ¹öÆ° ¸· ´©¸£´Â °ÍÀ» ¹æÁöÇÏµµ·Ï ÇÑ´Ù.
//	static unsigned long int st_nLastTime = 0;
//	unsigned long int nNowTime = timeGetTime();
#define MIN_ACCOUNT_ITEM_REQUEST_TIME		2000

	ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentTab(1);

//	if ((nNowTime - st_nLastTime) > MIN_ACCOUNT_ITEM_REQUEST_TIME)
//	{
		ZPostRequestAccountItemList(ZGetGameClient()->GetPlayerUID());
//		st_nLastTime = nNowTime;
//	}
END_IMPLEMENT_LISTENER()

// Áß¾ÓÀºÇà¿¡¼­ ·¹º§ ¹«½ÃÇÏ°í °¡Á®¿À±â È®ÀÎ (todok ¼ö·® ÀÔ·Â ¶§¹®¿¡ ±¸ÇöÇÏ±â°¡ ºý¼¼Á³´Ù.. ÀÌ°Ô ²ÀÇÊ¿äÇÒ±î? ±×³É »èÁ¦ÇÏ°í ½Í´Ù)
class ZLevelConfirmListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MMSGBOX_YES)==true){
			//int nCount = ZGetGameInterface()->m_nBringSpendableItemCount;
			//ZGetGameInterface()->GetShopEquipInterface()->BringAccountItem(nCount);
		} 
		pWidget->Show(false);
		return false;
	}
} g_LevelConfirmListener;

MListener* ZGetLevelConfirmListenter()
{
	return &g_LevelConfirmListener;
}

/*
///////////////////////////////////////////////////////////////////////////
// ÇÃ·¹ÀÌ¾î/Ä£±¸ ¸®½ºÆ® ÀüÈ¯
// ´©¸£¸é Ã¤³Î -> Ä£±¸ -> Å¬·£ -> Ã¤³Î ·Î ¼øÈ¯ÇÑ´Ù

BEGIN_IMPLEMENT_LISTENER(ZGetListenerLobbyPlayerListTabChannel, MBTN_CLK_MSG)
{
	ZPlayerListBox* pList = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
	if (pList) {
		pList->SetMode(ZPlayerListBox::PLAYERLISTMODE_CHANNEL_FRIEND);
		ZGetGameInterface()->SetupPlayerListButton(1);
	}
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetListenerLobbyPlayerListTabFriend, MBTN_CLK_MSG)
{
	ZPlayerListBox* pList = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
	if (pList) {
		pList->SetMode(ZPlayerListBox::PLAYERLISTMODE_CHANNEL_CLAN);
		ZGetGameInterface()->SetupPlayerListButton(2);
	}    
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetListenerLobbyPlayerListTabClan, MBTN_CLK_MSG)
{
	ZPlayerListBox* pList = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
	if (pList) {
		pList->SetMode(ZPlayerListBox::PLAYERLISTMODE_CHANNEL);
		ZGetGameInterface()->SetupPlayerListButton(0);
	}    
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetListenerGamePlayerListTabGame, MBTN_CLK_MSG)
{
	ZPlayerListBox* pList = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("StagePlayerList_");
	MButton* pBtnGame = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("StagePlayerListTabGame");
	MButton* pBtnFriend = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("StagePlayerListTabFriend");
	if (pList && pBtnGame && pBtnFriend) {
//		pBtnGame->Show(false);
//		pBtnFriend->Show(true);
		pList->SetMode(ZPlayerListBox::PLAYERLISTMODE_STAGE_FRIEND);
	}    
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetListenerGamePlayerListTabFriend, MBTN_CLK_MSG)
{
	ZPlayerListBox* pList = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("StagePlayerList_");
	MButton* pBtnGame = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("StagePlayerListTabGame");
	MButton* pBtnFriend = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("StagePlayerListTabFriend");
	if (pList && pBtnGame && pBtnFriend) {
//		pBtnGame->Show(true);
//		pBtnFriend->Show(false);
		pList->SetMode(ZPlayerListBox::PLAYERLISTMODE_STAGE);
	}    
}
END_IMPLEMENT_LISTENER()
*/
///////////////////////////////////////////////////////////////////////////
// Æ¯Á¤°¹¼ö´ÜÀ§·Î ÇÃ·¹ÀÌ¾î ¸®½ºÆ® °»½Å
BEGIN_IMPLEMENT_LISTENER(ZGetPlayerListPrevListener, MBTN_CLK_MSG)
{
	ZPlayerListBox* pWidget = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
	if (pWidget->GetMode() == ZPlayerListBox::PLAYERLISTMODE_CHANNEL_FRIEND) {
		int iStart = pWidget->GetStartItem();
		if (iStart > 0) 
			pWidget->SetStartItem(iStart-1);
		return true;
	}

	int nPage = pWidget->m_nPage;

	nPage--;

	if(nPage < 0) {
		return false;	
	}

	ZPostRequestChannelPlayerList(ZGetGameClient()->GetPlayerUID(),ZGetGameClient()->GetChannelUID(),nPage);

}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetPlayerListNextListener, MBTN_CLK_MSG)
{
	ZPlayerListBox* pWidget = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
	if (pWidget->GetMode() == ZPlayerListBox::PLAYERLISTMODE_CHANNEL_FRIEND) {
		int iStart = pWidget->GetStartItem();
		pWidget->SetStartItem(iStart+1);
		return true;
	}

	int nMaxPage = 0;
	if(pWidget->m_nTotalPlayerCount)
		nMaxPage = pWidget->m_nTotalPlayerCount/8;

	int nPage = pWidget->m_nPage;

	nPage++;

	if(nPage > nMaxPage) {
		return false;
	}

	ZPostRequestChannelPlayerList(ZGetGameClient()->GetPlayerUID(),ZGetGameClient()->GetChannelUID(),nPage);
	
}

END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetStagePlayerListPrevListener, MBTN_CLK_MSG)
{
	ZPlayerListBox* pWidget = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("StagePlayerList_");
	if (pWidget->GetMode() == ZPlayerListBox::PLAYERLISTMODE_STAGE_FRIEND) {
		int iStart = pWidget->GetStartItem();
		if (iStart > 0) 
			pWidget->SetStartItem(iStart-1);
		return true;
	}

	pWidget->SetStartItem( pWidget->GetStartItem()-1 );
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStagePlayerListNextListener, MBTN_CLK_MSG)
{
	ZPlayerListBox* pWidget = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("StagePlayerList_");
	if (pWidget->GetMode() == ZPlayerListBox::PLAYERLISTMODE_STAGE_FRIEND) {
		int iStart = pWidget->GetStartItem();
		pWidget->SetStartItem(iStart+1);
		return true;
	}

	pWidget->SetStartItem( pWidget->GetStartItem()+1 );
}
END_IMPLEMENT_LISTENER()



BEGIN_IMPLEMENT_LISTENER(ZGetRoomListListener, MLIST_VALUE_CHANGED)
	ZRoomListBox* pWidget = (ZRoomListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Lobby_StageList");
	pWidget->SetPage();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyPrevRoomListButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestPrevStageList();	
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomListPrevButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestNextStageList();
END_IMPLEMENT_LISTENER()

/*
BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomNo1ButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestStageList(1);
	ZGetGameInterface()->SetRoomNoLight(1);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomNo2ButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestStageList(2);
	ZGetGameInterface()->SetRoomNoLight(2);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomNo3ButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestStageList(3);
	ZGetGameInterface()->SetRoomNoLight(3);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomNo4ButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestStageList(4);
	ZGetGameInterface()->SetRoomNoLight(4);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomNo5ButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestStageList(5);
	ZGetGameInterface()->SetRoomNoLight(5);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomNo6ButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestStageList(6);
	ZGetGameInterface()->SetRoomNoLight(6);
END_IMPLEMENT_LISTENER()
*/
BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomNoButtonListener, MBTN_CLK_MSG)
	MButton *pButton = (MButton*)pWidget;
	int nIndexInGroup = pButton->GetIndexInGroup();
	_ASSERT(0<=nIndexInGroup && nIndexInGroup<6);
	nIndexInGroup++;	// 0~5 -> 1~6
	ZGetGameClient()->RequestStageList(nIndexInGroup);
	ZGetGameInterface()->SetRoomNoLight(nIndexInGroup);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetEquipmentShopCallerButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->ShowEquipmentDialog(false);
	ZGetGameInterface()->ShowShopDialog(true);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentEventShopCallerButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->ShowEquipmentDialog(false);
ZGetGameInterface()->ShowShopDialog(true);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentMedalShopCallerButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->ShowEquipmentDialog(false);
	ZGetGameInterface()->ShowShopDialog(true);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetShopEquipmentCallerButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->ShowShopDialog(false);
	ZGetGameInterface()->ShowEquipmentDialog(true);
END_IMPLEMENT_LISTENER()

class ZMapListListener : public MListener{									
	public:																	
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{	
		if(MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true)
		{
			MListBox* pList = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("MapList");
			if(pList != NULL)
			{
				MComboBox* pCombo = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("MapSelection");
				if(pCombo != NULL )
				{
					int si = pList->GetSelIndex();
					pCombo->SetSelIndex(si);
					PostMapname();
				}
			}

			return true;
		}
		if(MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			MListBox* pList = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("MapList");
			if(pList != NULL)
			{
				pList->Show(FALSE);
			}
			return true;
		}
		return false;
	}
} g_MapListListener;

MListener* ZGetStageMapListSelectionListener()
{
	return &g_MapListListener;
}

BEGIN_IMPLEMENT_LISTENER(ZGetStageMapListCallerListener, MBTN_CLK_MSG)
	MListBox* pList = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("MapList");
	pList->Show(TRUE);
END_IMPLEMENT_LISTENER();



void ZReport112FromListener()
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

	MWidget* pWidget = pResource->FindWidget( "112Confirm");
	if ( !pWidget)
		return;

	MComboBox* pCombo1 = (MComboBox*)pResource->FindWidget( "112_ConfirmID");
	MComboBox* pCombo2 = (MComboBox*)pResource->FindWidget( "112_ConfirmReason");
	
	if ( !pCombo1 || !pCombo2)
		return;

	if ( ( pCombo2->GetSelIndex() < 0) || ( pCombo2->GetSelIndex() < 1))
		return;


	__time64_t long_time;
	_time64( &long_time);
	const struct tm* pLocalTime = _localtime64( &long_time);

	char szBuff[ 256];
	sprintf( szBuff, "%s\n%s\n%03d:%s\n%04d-%02d-%02d %02d:%02d:%02d\n",	ZGetMyInfo()->GetCharName(), pCombo1->GetSelItemString(),
																			100+pCombo2->GetSelIndex(), pCombo2->GetSelItemString(),
																			pLocalTime->tm_year+1900, pLocalTime->tm_mon+1, pLocalTime->tm_mday,
																			pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec);

	ZGetGameInterface()->GetChat()->Report112( szBuff);


	pWidget->Show( false);
}


BEGIN_IMPLEMENT_LISTENER(ZGet112ConfirmEditListener, MEDIT_ENTER_VALUE)
	ZReport112FromListener();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGet112ConfirmOKButtonListener, MBTN_CLK_MSG)
	ZReport112FromListener();
END_IMPLEMENT_LISTENER();


BEGIN_IMPLEMENT_LISTENER(ZGet112ConfirmCancelButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->Show112Dialog(false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanSponsorAgreementConfirm_OKButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->AnswerSponsorAgreement(true);

	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanSponsorAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanSponsorAgreementConfirm_CancelButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->AnswerSponsorAgreement(false);

	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanSponsorAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanSponsorAgreementWait_CancelButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanSponsorAgreementWait");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanJoinerAgreementConfirm_OKButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->AnswerJoinerAgreement(true);

	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanJoinerAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanJoinerAgreementConfirm_CancelButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->AnswerJoinerAgreement(false);

	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanJoinerAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanJoinerAgreementWait_CancelButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanJoinerAgreementWait");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

// Å¬·£ »ý¼º Ã¢
BEGIN_IMPLEMENT_LISTENER(ZGetLobbyPlayerListTabClanCreateButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanCreateDialog");
	if(pWidget!=NULL)
	{
		pWidget->Show(true,true);

		// Ã¢´Ü¸â¹ö·Î ¼±ÅÃ°¡´ÉÇÑ »ç¶÷ ¸®½ºÆ®¸¦ ¿äÃ»ÇÑ´Ù
		unsigned long int nPlaceFilter = 0;
		SetBitSet(nPlaceFilter, MMP_LOBBY);

		ZPostRequestChannelAllPlayerList(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetChannelUID(),nPlaceFilter,
			MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_NONCLAN);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanCreateDialogOk, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanCreateDialog");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);

		// Custom: New IJJI clan changes.
		//ZPlayerSelectListBox *pPlayerList = (ZPlayerSelectListBox*)pResource->FindWidget("ClanSponsorSelect");
		//if(pPlayerList)
		//{
			char szSponsors[CLAN_SPONSORS_COUNT][MATCHOBJECT_NAME_LENGTH];
			char *ppSponsors[CLAN_SPONSORS_COUNT];
			int nCount = 0;

			for (int i = 0; i < 4; ++i)
			{
				if (nCount >= CLAN_SPONSORS_COUNT)
					break;

				strcpy( szSponsors[nCount], "----" ); // no sponsor
				ppSponsors[nCount] = szSponsors[nCount];

				nCount++;
			}

			//for(int i=0;i<pPlayerList->GetCount();i++)
			//{
			//	MListItem *pItem = pPlayerList->Get(i);
			//	if(pItem->m_bSelected) {
			//		if(nCount>=CLAN_SPONSORS_COUNT) break;
			//		strcpy(szSponsors[nCount],pItem->GetString());
			//		ppSponsors[nCount]=szSponsors[nCount];
			//		nCount ++;
			//	}
			//}
			/*
#ifdef _DEBUG
			nCount = CLAN_SPONSORS_COUNT;
#endif
			*/

			// Á¤È®ÇÑ ¼ö°¡ ¸Â¾ÒÀ¸¸é..
			//if ( nCount==CLAN_SPONSORS_COUNT)
			//{
				MEdit *pEditClanName= (MEdit*)pResource->FindWidget("ClanCreate_ClanName");
				if ( !pEditClanName)
					return true;


				int nNameLen = (int)strlen( pEditClanName->GetText());

				if ( nNameLen <= 0)						// ÀÌ¸§À» ÀÌ·ÂÇÏÁö ¾Ê¾Ò´Ù.
				{
					ZGetGameInterface()->ShowErrorMessage( MERR_PLZ_INPUT_CHARNAME);
					return true;
				}
				else if ( nNameLen < MIN_CLANNAME)		// ÀÌ¸§ÀÌ ³Ê¹« Âª´Ù.
				{
					ZGetGameInterface()->ShowErrorMessage( MERR_TOO_SHORT_NAME);
					return true;
				}
				else if ( nNameLen > MAX_CLANNAME)		// ÀÌ¸§ÀÌ Á¦ÇÑ ±ÛÀÚ¼ö¸¦ ³Ñ¾ú´Ù.
				{
					ZGetGameInterface()->ShowErrorMessage( MERR_TOO_LONG_NAME);
					return true;
				}


				// À¯È¿ÇÑÁö °Ë»çÇÑ´Ù.
				if( !MGetChattingFilter()->IsValidStr( pEditClanName->GetText(), 2, true) )
				{
					char szMsg[ 256 ];
					ZTransMsg( szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr().c_str());
					ZGetGameInterface()->ShowMessage( szMsg, NULL, MSG_WRONG_WORD_NAME );
				}
				else if(pEditClanName)
				{
					char szClanName[CLAN_NAME_LENGTH]={0,};
					strcpy(szClanName,pEditClanName->GetText());
					ZGetGameClient()->RequestCreateClan(szClanName, ppSponsors);
				}
			//}
			// ¼ýÀÚ°¡ ¸ðÀÚ¸£´Ù
			//else
			//{
			//	char szMsgBox[256];
			//	char szArg[20];
			//	_itoa(CLAN_SPONSORS_COUNT, szArg, 10);

			//	ZTransMsg(szMsgBox, MSG_CLAN_CREATE_NEED_SOME_SPONSOR, 1, szArg);
			//	ZGetGameInterface()->ShowMessage(szMsgBox, NULL, MSG_CLAN_CREATE_NEED_SOME_SPONSOR);
			//}
		//}
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanCreateDialogClose, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanCreateDialog");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

// ¾ð¾î º¯È¯ È®ÀÎ
class ZLanguageChangeConfirmListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		pWidget->Show(false);
		if(MWidget::IsMsg(szMessage, MMSGBOX_YES)==true){
			ZGetGameInterface()->ReserveResetApp(true);
		}
		return false;
	}
} g_LanguageChangeConfirmListener;

MListener* ZGetLanguageChangeConfirmListenter()
{
	return &g_LanguageChangeConfirmListener;
}

// Å¬·£ Æó¼â È®ÀÎ
class ZClanCloseConfirmListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MMSGBOX_YES)==true){

			char szClanName[256];
			strcpy(szClanName, ZGetMyInfo()->GetClanName());
			// ¼­¹ö¿¡ Æó¼â ¿äÃ»
			ZPostRequestCloseClan(ZGetGameClient()->GetPlayerUID(), szClanName);
		} else {
		}
		pWidget->Show(false);
		return false;
	}
} g_ClanCloseConfirmListener;

MListener* ZGetClanCloseConfirmListenter()
{
	return &g_ClanCloseConfirmListener;
}

// Å¬·£ Å»Åð È®ÀÎ
class ZClanLeaveConfirmListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MMSGBOX_YES)==true){
			// ¼­¹ö¿¡ Å»Åð ¿äÃ»
			ZPostRequestLeaveClan(ZGetMyUID());
		} else {
		}
		pWidget->Show(false);
		return false;
	}
} g_ClanLeaveConfirmListener;

MListener* ZGetClanLeaveConfirmListenter()
{
	return &g_ClanLeaveConfirmListener;
}

#ifdef _CW_VOTE
BEGIN_IMPLEMENT_LISTENER(ZGetArrangedLadderWarsListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("MapSelector");
if (pWidget != NULL && pWidget->IsVisible()) return true;
pWidget = pResource->FindWidget("LadderWarsGameDialog");
if (pWidget != NULL && pWidget->IsVisible()) return true;
pWidget = pResource->FindWidget("LobbyFindPlayer");
if (pWidget != NULL && pWidget->IsVisible()) return true;

pWidget = pResource->FindWidget("LadderWarsGameDialog");
if (pWidget != NULL)
{
	pWidget->Show(true, true);
	unsigned long int nPlaceFilter = 0;
	SetBitSet(nPlaceFilter, MMP_LOBBY);
	char Name[100];
	for (int i = 0; i < 3; i++)
	{
		sprintf(Name, "PlayerVoteMap%d", i);
		MLabel* pLabel = (MLabel*)pResource->FindWidget(Name);
		if (pLabel)
			pLabel->SetTextColor(MCOLOR(205, 205, 205));
	}
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetLadderWarsVote0, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
char Name[100];
for (int i = 0; i < 3; i++)
{
	sprintf(Name, "PlayerVoteMap%d", i);
	MLabel* pLabel = (MLabel*)pResource->FindWidget(Name);
	if (pLabel)
		pLabel->SetTextColor(MCOLOR(205, 205, 205));
}
ZGetGameClient()->LastVoteID = 0;
MLabel* pLabel = (MLabel*)pResource->FindWidget("PlayerVoteMap0");
if (pLabel)
        pLabel->SetTextColor(MCOLOR(205, 205, 205));
ZPostLadderWarsVote(0);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetLadderWarsVote1, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
char Name[100];
for (int i = 0; i < 3; i++)
{
	sprintf(Name, "PlayerVoteMap%d", i);
	MLabel* pLabel = (MLabel*)pResource->FindWidget(Name);
	if (pLabel)
		pLabel->SetTextColor(MCOLOR(205, 205, 205));
}
ZGetGameClient()->LastVoteID = 1;
MLabel* pLabel = (MLabel*)pResource->FindWidget("PlayerVoteMap1");
if (pLabel)
pLabel->SetTextColor(MCOLOR(102, 205, 0));
ZPostLadderWarsVote(1);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetLadderWarsVote2, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
char Name[100];
for (int i = 0; i < 3; i++)
{
	sprintf(Name, "PlayerVoteMap%d", i);
	MLabel* pLabel = (MLabel*)pResource->FindWidget(Name);
	if (pLabel)
		pLabel->SetTextColor(MCOLOR(205, 205, 205));
}
ZGetGameClient()->LastVoteID = 2;
MLabel* pLabel = (MLabel*)pResource->FindWidget("PlayerVoteMap2");
if (pLabel)
pLabel->SetTextColor(MCOLOR(205, 205, 205));
ZPostLadderWarsVote(2);
END_IMPLEMENT_LISTENER();
#endif // _CW_VOTE

BEGIN_IMPLEMENT_LISTENER(ZGetLadderRejoinConfirmListener, MBTN_CLK_MSG)
ZGetGameClient()->IsRejoin = true;
ZPostLadderRejoin();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetLadderWarsDialogOkListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	ZGetGameClient()->LastVoteID = -1;
	MWidget* pWidget = pResource->FindWidget("LadderWarsGameDialog");
	if(pWidget!=NULL)
		pWidget->Show(false);
	int Arg1, Arg2;
	MButton* pButton = (MButton*)pResource->FindWidget("LadderWarsLead");
	if ((pButton) && (pButton->GetCheck()))
	{
		Arg1 = 1;
	}		
	MComboBox* pCWidget = (MComboBox*)pResource->FindWidget("LadderWarsGameType");
	if(pCWidget)	
	{
		Arg2 = pCWidget->GetSelIndex();
	}
	ZPostJoinLadderWars(Arg1, Arg2);
	ZGetGameInterface()->OnArrangedTeamGameUI(true);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetLadderWarsDialogCloseListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("LadderWarsGameDialog");
	if(pWidget!=NULL)
		pWidget->Show(false);

	pWidget = pResource->FindWidget("LobbyFindClanTeam");
	if(pWidget!=NULL)
		pWidget->Show(false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetInviteWarListener, MBTN_CLK_MSG)
ZPostInviteClanWar();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedTeamGameListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("ArrangedTeamGameDialog");
if (pWidget != NULL)
{
	MComboBox* pCombo = (MComboBox*)pResource->FindWidget("CWGameType");
	//if (pCombo) pCombo->Remove(0);
	pWidget->Show(true, true);

	// ÆÀÇÃ·¹ÀÌ ¼±¼ö·Î ¼±ÅÃ°¡´ÉÇÑ »ç¶÷ ¸®½ºÆ®¸¦ ¿äÃ»ÇÑ´Ù
	unsigned long int nPlaceFilter = 0;
	SetBitSet(nPlaceFilter, MMP_LOBBY);

	ZPostRequestChannelAllPlayerList(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetChannelUID(), nPlaceFilter,
		MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_MYCLAN);
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedTeamDialogOkListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ArrangedTeamGameDialog");
	if(pWidget!=NULL)
		pWidget->Show(false);

//	pWidget = pResource->FindWidget("LobbyFindClanTeam");
//	if(pWidget!=NULL)
//		pWidget->Show(true);

	// °ÔÀÓ ÃÊ´ë ¸Þ½ÃÁö¸¦ ¹ß¼ÛÇÑ´Ù

	ZPlayerSelectListBox *pPlayerList = (ZPlayerSelectListBox*)pResource->FindWidget("ArrangedTeamSelect");
	if(pPlayerList)
	{
		const int nMaxInviteCount = max(MAX_LADDER_TEAM_MEMBER,MAX_CLANBATTLE_TEAM_MEMBER) - 1;

		char szNames[nMaxInviteCount][MATCHOBJECT_NAME_LENGTH];
		char *ppNames[nMaxInviteCount];
		int nCount = 0;
		for(int i=0;i<pPlayerList->GetCount();i++)
		{
			MListItem *pItem = pPlayerList->Get(i);
			if(pItem->m_bSelected) {
				if(nCount>=nMaxInviteCount) {
					nCount++;
					break;
				}
				strcpy(szNames[nCount],pItem->GetString());
				ppNames[nCount]=szNames[nCount];
				nCount ++;
			}
		}

		switch (ZGetGameClient()->GetServerMode())
		{
		case MSM_LADDER:
			{
				// ÀûÀýÇÑ ¼öÀÇ ÆÀÀÌ¸é
				if(0<nCount && nCount<=nMaxInviteCount) {
					ZGetGameClient()->RequestProposal(MPROPOSAL_LADDER_INVITE, ppNames, nCount);
				}else
				{
//					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), MGetErrorString(MSG_LADDER_INVALID_COUNT));
					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), 
						ZErrStr(MSG_LADDER_INVALID_COUNT) );
				}
			}
			break;
		case MSM_CLAN:
			{
				bool bRightMember = false;
				for (int i = 0; i < MLADDERTYPE_MAX; i++)
				{
					if ((g_nNeedLadderMemberCount[i]-1) == nCount)
					{
						bRightMember = true;
						break;
					}
				}

				// ÀûÀýÇÑ ¼öÀÇ ÆÀÀÌ¸é

				if((0<nCount) && (bRightMember))
				{
					ZGetGameClient()->RequestProposal(MPROPOSAL_CLAN_INVITE, ppNames, nCount);
				} 
#ifdef _LADDERGAME	// 1¸íÅ×½ºÆ®
				else if (nCount == 0)
				{
					// ÀÚ±â ÀÚ½Å¸¸ÀÏ¶§
					char szMember[1][MATCHOBJECT_NAME_LENGTH];
					char* ppMember[1];

					ppMember[0] = szMember[0];
					strcpy(szMember[0], ZGetMyInfo()->GetCharName());

					// Balance ¿É¼Ç
					int nBalancedMatching = 0, AntiLead = 0, GameType = 0, nDodge = 0, nRobase = 0;
					ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
					MButton* pButton = (MButton*)pResource->FindWidget("BalancedMatchingCheckBox");
					if ((pButton) && (pButton->GetCheck()))
					{
						nBalancedMatching = 1;
					}
					pButton = (MButton*)pResource->FindWidget("ClanAntiLead");
					if ((pButton) && (pButton->GetCheck()))
					{
						AntiLead = 1;
					}
					MComboBox* pWidget = (MComboBox*)pResource->FindWidget("CWGameType");
					if (pWidget)
					{
						GameType = pWidget->GetSelIndex();
					}
					MButton* pDodge = (MButton*)pResource->FindWidget("DodgeCheckBox");
					if ((pDodge) && (pDodge->GetCheck()))
					{
						nDodge = 1;
					}
					MButton* pBaseRotation = (MButton*)pResource->FindWidget("RotationCheckBox");
					if ((pBaseRotation) && (pBaseRotation->GetCheck()))
					{
						nRobase = 1;
					}
					ZPostLadderRequestChallenge(ppMember, 1, nBalancedMatching, AntiLead, GameType, nDodge, nRobase);
				}
#endif
				else
				{
					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), 
						ZMsg(MSG_LADDER_INVALID_COUNT) );
				}
			}
			break;
		}
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedTeamDialogCloseListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ArrangedTeamGameDialog");
	if(pWidget!=NULL)
		pWidget->Show(false);

	pWidget = pResource->FindWidget("LobbyFindClanTeam");
	if(pWidget!=NULL)
		pWidget->Show(false);
END_IMPLEMENT_LISTENER();

void ZKickPlayerFromListener()
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("KickPlayer");

	if(!pWidget)
		return;

	MComboBox* pCombo = (MComboBox*)pResource->FindWidget( "KickPlayer_ConfirmID");
	
	if ( !pCombo )
		return;

	if ( pCombo->GetSelIndex() < 0 )
		return;

	const char* szItem = pCombo->Get(pCombo->GetSelIndex())->GetString();

	ZPOSTCMD2(MC_MATCH_CALLVOTE, MCmdParamStr("kick"), MCmdParamStr(szItem) );
	ZPOSTCMD0(MC_MATCH_VOTE_YES);
	ZGetGameClient()->SetCanVote(false);

	pWidget->Show( false);
}

// Custom: Enhanced Kick System
BEGIN_IMPLEMENT_LISTENER(ZGetKickPlayerOKButtonListener, MBTN_CLK_MSG)
	ZKickPlayerFromListener();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetKickPlayerCancelButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->ShowKickPlayerDialog(false);

END_IMPLEMENT_LISTENER();


BEGIN_IMPLEMENT_LISTENER(ZGetProposalAgreementWait_CancelButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ProposalAgreementWait");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetProposalAgreementConfirm_OKButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->ReplyAgreement(true);

	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ProposalAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetProposalAgreementConfirm_CancelButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->ReplyAgreement(false);

	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ProposalAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedTeamGame_CancelListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pWidget = pResource->FindWidget("LobbyFindClanTeam");
if (pWidget != NULL)
pWidget->Show(false);

bool bLadderWarsUI = (ZGetGameClient()->GetChannelType() == MCHANNEL_TYPE_LADDERWAR);
ZPostLadderCancel(bLadderWarsUI);

// REFRESH LẠI UI ĐÚNG THEO CHANNEL
if (bLadderWarsUI)
{
	ZGetGameInterface()->InitLadderWarsUI(true);
}
else if (ZGetGameClient()->GetChannelType() == MCHANNEL_TYPE_CLAN)
{
	ZGetGameInterface()->InitClanLobbyUI(true);
}
END_IMPLEMENT_LISTENER();



BEGIN_IMPLEMENT_LISTENER(ZGetPrivateChannelEnterListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	MEdit* pEdit = (MEdit*)pResource->FindWidget("PrivateChannelInput");
	if(pEdit!=NULL)
	{
		int nNameLen = (int)strlen( pEdit->GetText());

		if ( nNameLen <= 0)						// ÀÌ¸§À» ÀÌ·ÂÇÏÁö ¾Ê¾Ò´Ù.
		{
			ZGetGameInterface()->ShowErrorMessage( MERR_PLZ_INPUT_CHARNAME);
			return true;
		}
		else if ( nNameLen < MIN_CLANNAME)		// ÀÌ¸§ÀÌ ³Ê¹« Âª´Ù.
		{
			ZGetGameInterface()->ShowErrorMessage( MERR_TOO_SHORT_NAME);
			return true;
		}
		else if ( nNameLen > MAX_CLANNAME)		// ÀÌ¸§ÀÌ Á¦ÇÑ ±ÛÀÚ¼ö¸¦ ³Ñ¾ú´Ù.
		{
			ZGetGameInterface()->ShowErrorMessage( MERR_TOO_LONG_NAME);
			return true;
		}


		if( !MGetChattingFilter()->IsValidStr( pEdit->GetText(), 1))
		{
			char szMsg[ 256 ];
			ZTransMsg( szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr().c_str());
			ZGetGameInterface()->ShowMessage( szMsg, NULL, MSG_WRONG_WORD_NAME );
		}
		else
		{
			ZPostChannelRequestJoinFromChannelName(ZGetMyUID(),MCHANNEL_TYPE_USER,pEdit->GetText());

			MWidget* pFindWidget = pResource->FindWidget("ChannelListFrame");
			if(pFindWidget!=NULL) pFindWidget->Show(false);
		}
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetChannelList, MBTN_CLK_MSG)
	MButton *pButton = (MButton*)pWidget;
	int nIndexInGroup = pButton->GetIndexInGroup();
	_ASSERT(0<=nIndexInGroup && nIndexInGroup<4);

	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

	MCHANNEL_TYPE nChannelType = MCHANNEL_TYPE_PRESET;
	// 0 = ÀÏ¹Ý , 1 = »ç¼³ , 2 = Å¬·£ Ã¤³Î ¸®½ºÆ® , 3 = µà¾ó Åä³Ê¸ÕÆ® Ã¤³Î ¸®½ºÆ®¸¦ ¿äÃ»ÇÑ´Ù
	switch(nIndexInGroup) {
	case 0 : nChannelType = MCHANNEL_TYPE_PRESET; break;
	case 1 : nChannelType = MCHANNEL_TYPE_USER; break;
	case 2 : nChannelType = MCHANNEL_TYPE_CLAN; break;
	case 3 : nChannelType = MCHANNEL_TYPE_DUELTOURNAMENT; break;
	case 4 : nChannelType = MCHANNEL_TYPE_LADDERWAR; break;

	default : _ASSERT(FALSE);
	}

	ZGetGameInterface()->InitChannelFrame(nChannelType);
	ZGetGameClient()->StartChannelList(nChannelType);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetMyClanChannel, MBTN_CLK_MSG)
	if(ZGetMyInfo()->IsClanJoined())
	{
		ZPostChannelRequestJoinFromChannelName(ZGetMyUID(),MCHANNEL_TYPE_CLAN,ZGetMyInfo()->GetClanName());

#ifdef LOCALE_NHNUSA
		// Custom: Disable NHN related
		//GetNHNUSAReport().ReportJoinChannel();
#endif

		ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
		MWidget* pFindWidget = pResource->FindWidget("ChannelListFrame");
		if(pFindWidget!=NULL) pFindWidget->Show(false);
	}
	else {
		// Å¬·£¿¡ °¡ÀÔµÇ¾îÀÖÁö ¾Ê½À´Ï´Ù ¸Þ½ÃÁö ¾È³»
		ZGetGameInterface()->ShowMessage( MSG_LOBBY_NO_CLAN );
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanWarChannel, MBTN_CLK_MSG)
if (ZGetMyInfo()->IsClanJoined())
{
	ZPostChannelRequestJoinFromChannelName(ZGetMyUID(), MCHANNEL_TYPE_CLAN, ZGetMyInfo()->GetClanName());
}
else
{
	ZGetGameInterface()->ShowMessage(MSG_LOBBY_NO_CLAN); // Thông báo chưa có Clan
}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetCustomChannel, MBTN_CLK_MSG)
	ZPostRequestRecommendChannel();
	//ZPostChannelRequestJoinFromChannelName(ZGetMyUID(), MCHANNEL_TYPE_GENERAL, "Free Channel");
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetLadderChannel, MBTN_CLK_MSG)
	ZPostChannelRequestJoinFromChannelName(ZGetMyUID(), MCHANNEL_TYPE_LADDERWAR, "Ladder Channel");
	//ZApplication::GetSoundEngine()->StopMusic();
	//ZApplication::GetSoundEngine()->OpenMusic(BGMID_LADDER, ZApplication::GetFileSystem());
	//ZApplication::GetSoundEngine()->PlayMusic();
END_IMPLEMENT_LISTENER();

void SwitchLobbySidebarView(bool bShowChat)
{
	ZIDLResource* pRes = ZGetGameInterface()->GetIDLResource();
	MWidget* pListFrame = pRes->FindWidget("LobbyListFrame");
	MWidget* pChatFrame = pRes->FindWidget("LobbyChatFrame");

	if (pListFrame && pChatFrame)
	{
		pListFrame->Show(!bShowChat);
		pChatFrame->Show(bShowChat);

		if (bShowChat)
		{
			MWidget* pInput = pRes->FindWidget("ChannelChattingInput");
			if (pInput) pInput->SetFocus();
		}
	}
}

// 1. Nút mở/đóng Frame Tổng (ChatCaller)
BEGIN_IMPLEMENT_LISTENER(ZGetChatCallerToggleListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pMainFrame = pResource->FindWidget("LobbyChatAndPlayerFrame");
if (pMainFrame != NULL)
{
	pMainFrame->Show(!pMainFrame->IsVisible());
}
END_IMPLEMENT_LISTENER()

// 2. Tab LOBBY LIST -> Tắt Chat, mở List
BEGIN_IMPLEMENT_LISTENER(ZGetPlayerListTabLobbyListener, MBTN_CLK_MSG)
SwitchLobbySidebarView(false); // Ẩn Chat, Hiện List
ZPlayerListBox* pList = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
if (pList) pList->SetMode(ZPlayerListBox::PLAYERLISTMODE_CHANNEL);
END_IMPLEMENT_LISTENER()

// 3. Tab FRIEND -> Tắt Chat, mở List
BEGIN_IMPLEMENT_LISTENER(ZGetPlayerListTabFriendListener, MBTN_CLK_MSG)
SwitchLobbySidebarView(false); // Ẩn Chat, Hiện List
ZPlayerListBox* pList = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
if (pList) pList->SetMode(ZPlayerListBox::PLAYERLISTMODE_CHANNEL_FRIEND);
END_IMPLEMENT_LISTENER()

// 4. Tab CLAN -> Tắt Chat, mở List
BEGIN_IMPLEMENT_LISTENER(ZGetPlayerListTabClanListener, MBTN_CLK_MSG)
SwitchLobbySidebarView(false); // Ẩn Chat, Hiện List
ZPlayerListBox* pList = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
if (pList) pList->SetMode(ZPlayerListBox::PLAYERLISTMODE_CHANNEL_CLAN);
END_IMPLEMENT_LISTENER()

// 5. Tab LOBBY CHAT -> Mở Chat đè lên, tự Focus ô gõ
BEGIN_IMPLEMENT_LISTENER(ZGetPlayerListTabChatListener, MBTN_CLK_MSG)
SwitchLobbySidebarView(true); // Mở Chat, Ẩn List
END_IMPLEMENT_LISTENER()

// for new UI cờ lê bật tắt bảng thiết lập phòng
BEGIN_IMPLEMENT_LISTENER(ZGetStageGameInfoToggleListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pFrame = pResource->FindWidget("StageGameInfo");
if (pFrame != NULL)
{
	pFrame->Show(!pFrame->IsVisible());
}
END_IMPLEMENT_LISTENER()


// Shop list frame open/close
BEGIN_IMPLEMENT_LISTENER(ZShopItemEquipmentTabOpen, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->OnArmorWeaponTabButtonClicked(0);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZShopWeaponEquipmentTabOpen, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->OnArmorWeaponTabButtonClicked(1);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZShopMiscEquipmentTabOpen, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->OnArmorWeaponTabButtonClicked(2);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZShopListFrameClose, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentFrameList(NULL, false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZShopListFrameOpen, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentFrameList(NULL, true);
END_IMPLEMENT_LISTENER();


// Equipment list frame open/close
BEGIN_IMPLEMENT_LISTENER(ZEquipItemEquipmentTabOpen, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->OnArmorWeaponTabButtonClicked(0);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZEquipWeaponEquipmentTabOpen, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->OnArmorWeaponTabButtonClicked(1);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZEquipMiscEquipmentTabOpen, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->OnArmorWeaponTabButtonClicked(2);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZEquipListFrameClose, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentFrameList(NULL, false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZEquipListFrameOpen, MBTN_CLK_MSG)
	ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentFrameList(NULL, true);
END_IMPLEMENT_LISTENER();

// Equipment character view rotate
BEGIN_IMPLEMENT_LISTENER(ZEquipmetRotateBtn, MBTN_CLK_MSG)
	ZCharacterView* pCharView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget( "EquipmentInformation");
	if ( pCharView)
	{
		pCharView->EnableAutoRotate( !pCharView->IsAutoRotate());
		
		MBmButton* pButton = (MBmButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Equipment_CharacterRotate");
		if ( pButton)
		{
			if ( pCharView->IsAutoRotate())
			{
				pButton->SetUpBitmap(   MBitmapManager::Get( "btn_rotate.tga"));
				pButton->SetDownBitmap( MBitmapManager::Get( "btn_rotate.tga"));
				pButton->SetOverBitmap( MBitmapManager::Get( "btn_rotate.tga"));
			}
			else
			{
				pButton->SetUpBitmap(   MBitmapManager::Get( "btn_stop.tga"));
				pButton->SetDownBitmap( MBitmapManager::Get( "btn_stop.tga"));
				pButton->SetOverBitmap( MBitmapManager::Get( "btn_stop.tga"));
			}
		}
	}
END_IMPLEMENT_LISTENER();


// Replay
BEGIN_IMPLEMENT_LISTENER(ZReplayOk, MBTN_CLK_MSG)
	ZGetGameInterface()->OnReplay();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetReplayCallerButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->ShowReplayDialog( true);
END_IMPLEMENT_LISTENER();

// Custom: Delete Replay
BEGIN_IMPLEMENT_LISTENER(ZGetReplayDeleteButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->DeleteReplay();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetReplayViewButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->ViewReplay();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetReplayExitButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->ShowReplayDialog( false);
END_IMPLEMENT_LISTENER();

MListener* ZGetReplayFileListBoxListener( void)
{
	class ListenerClass : public MListener
	{
		public:
		virtual bool OnCommand( MWidget* pWidget, const char* szMessage)
		{
			// Item select
			if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL) == true)
			{
				MWidget* pFindWidget = ZGetGameInterface()->GetIDLResource()->FindWidget( "Replay_View");
				if ( pFindWidget != NULL)
					pFindWidget->Enable( true);

				// Custom: Delete Replay
				pFindWidget = ZGetGameInterface()->GetIDLResource()->FindWidget( "Replay_Delete");
				if ( pFindWidget != NULL)
					pFindWidget->Enable( true);

                return true;
			}
			// Item double click
			else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
			{
				ZGetGameInterface()->ViewReplay();

                return true;
			}

			return false;
		}
	};
	static ListenerClass	Listener;
	return &Listener;
}


BEGIN_IMPLEMENT_LISTENER(ZGetLeaveClanOKListener, MBTN_CLK_MSG)
	MWidget* pWidget = (MWidget*)ZGetGameInterface()->GetIDLResource()->FindWidget( "ConfirmLeaveClan");
	if ( pWidget)
		pWidget->Show( false);

	ZPostRequestLeaveClan(ZGetMyUID());

	ZPlayerListBox *pPlayerListBox = (ZPlayerListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "LobbyChannelPlayerList" );
	if(pPlayerListBox)
		pPlayerListBox->SetMode(ZPlayerListBox::PLAYERLISTMODE_CHANNEL_CLAN);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetLeaveClanCancelListener, MBTN_CLK_MSG)
	MWidget* pWidget = (MWidget*)ZGetGameInterface()->GetIDLResource()->FindWidget( "ConfirmLeaveClan");
	if ( pWidget)
		pWidget->Show( false);
END_IMPLEMENT_LISTENER();

int g_lastPressedDuelTournamentGameBtn = 8;

// µà¾ó Åä³Ê¸ÕÆ® Âü°¡ ¹öÆ°
BEGIN_IMPLEMENT_LISTENER(ZGetDuelTournamentGameButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->OnDuelTournamentGameUI(true);
	ZPostDuelTournamentRequestJoinGame( ZGetMyUID(), MDUELTOURNAMENTTYPE_QUATERFINAL);
	g_lastPressedDuelTournamentGameBtn = 8;
END_IMPLEMENT_LISTENER();

// µà¾ó Åä³Ê¸ÕÆ® Âü°¡ Å×½ºÆ®¿ë 2ÀÎ¿ë ¹öÆ°
BEGIN_IMPLEMENT_LISTENER(ZGetDuelTournamentGame2TestButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->OnDuelTournamentGameUI(true);
	ZPostDuelTournamentRequestJoinGame( ZGetMyUID(), MDUELTOURNAMENTTYPE_FINAL);
	g_lastPressedDuelTournamentGameBtn = 2;
END_IMPLEMENT_LISTENER();

// µà¾ó Åä³Ê¸ÕÆ® Âü°¡ Å×½ºÆ®¿ë 4ÀÎ¿ë ¹öÆ°
BEGIN_IMPLEMENT_LISTENER(ZGetDuelTournamentGame4TestButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->OnDuelTournamentGameUI(true);
	ZPostDuelTournamentRequestJoinGame( ZGetMyUID(), MDUELTOURNAMENTTYPE_SEMIFINAL);
	g_lastPressedDuelTournamentGameBtn = 4;
END_IMPLEMENT_LISTENER();

// µà¾ó Åä³Ê¸ÕÆ® Âü°¡ Ãë¼Ò ¹öÆ°
BEGIN_IMPLEMENT_LISTENER(ZGetDuelTournamentWaitCancelButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->OnDuelTournamentGameUI(false);

#ifndef _LADDERDUEL
	ZPostDuelTournamentRequestCancelGame( ZGetMyUID(), MDUELTOURNAMENTTYPE_QUATERFINAL);
#else
	switch (g_lastPressedDuelTournamentGameBtn) {
		case 2 : ZPostDuelTournamentRequestCancelGame( ZGetMyUID(), MDUELTOURNAMENTTYPE_FINAL); break;
		case 4 : ZPostDuelTournamentRequestCancelGame( ZGetMyUID(), MDUELTOURNAMENTTYPE_SEMIFINAL); break;
		default : ZPostDuelTournamentRequestCancelGame( ZGetMyUID(), MDUELTOURNAMENTTYPE_QUATERFINAL); break;
	}
#endif
		
END_IMPLEMENT_LISTENER();

// ½ºÅ×ÀÌÁö Èñ»ý ¾ÆÀÌÅÛ
BEGIN_IMPLEMENT_LISTENER( ZStageSacrificeItem0, MBTN_CLK_MSG)
	ZApplication::GetStageInterface()->OnSacrificeItem0();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZStageSacrificeItem1, MBTN_CLK_MSG)
	ZApplication::GetStageInterface()->OnSacrificeItem1();
END_IMPLEMENT_LISTENER();

// ½ºÅ×ÀÌÁö Èñ»ý ¾ÆÀÌÅÛ ¼±ÅÃ ¹Ú½º
BEGIN_IMPLEMENT_LISTENER( ZStagePutSacrificeItem, MBTN_CLK_MSG)
	if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ 0].IsExist())
		ZApplication::GetStageInterface()->OnDropSacrificeItem( 0);
	else if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ 1].IsExist())
		ZApplication::GetStageInterface()->OnDropSacrificeItem( 1);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZStageSacrificeItemBoxOpen, MBTN_CLK_MSG)
	ZApplication::GetStageInterface()->OpenSacrificeItemBox();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZStageSacrificeItemBoxClose, MBTN_CLK_MSG)
	ZApplication::GetStageInterface()->CloseSacrificeItemBox();
END_IMPLEMENT_LISTENER();


// ¸±·¹ÀÌ¸Ê
BEGIN_IMPLEMENT_LISTENER(ZGetRelayMapTypeListener, MCMBBOX_CHANGED)
	ZApplication::GetStageInterface()->PostRelayMapElementUpdate();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetRelayMapTurnCountListener, MCMBBOX_CHANGED)
	ZApplication::GetStageInterface()->PostRelayMapElementUpdate();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZStageRelayMapBoxOpen, MBTN_CLK_MSG)
	ZApplication::GetStageInterface()->OpenRelayMapBox();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZStageRelayMapBoxClose, MBTN_CLK_MSG)
	ZApplication::GetStageInterface()->CloseRelayMapBox();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZGetRelayMapOKButtonListener, MBTN_CLK_MSG)
	ZApplication::GetStageInterface()->PostRelayMapInfoUpdate();
END_IMPLEMENT_LISTENER();


// Spy Mode.
BEGIN_IMPLEMENT_LISTENER(ZStageSpyBanMapBoxOpen, MBTN_CLK_MSG)
ZApplication::GetStageInterface()->OpenSpyBanMapBox();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZStageSpyBanMapBoxClose, MBTN_CLK_MSG)
ZApplication::GetStageInterface()->CloseSpyBanMapBox();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetGameResultQuit, MBTN_CLK_MSG)
if (!ZGetGameClient()->IsLadderWarsChannel())
{
	if (ZGetGameClient()->IsCWChannel() || ZGetGameClient()->IsDuelTournamentGame())
		PostMessage(g_hWnd, WM_CHANGE_GAMESTATE, GUNZ_LOBBY, 0);
	else
		PostMessage(g_hWnd, WM_CHANGE_GAMESTATE, GUNZ_STAGE, 0);
}
END_IMPLEMENT_LISTENER();



// Å×½ºÆ® ¹öÆ°
BEGIN_IMPLEMENT_LISTENER( ZGetMonsterBookCaller, MBTN_CLK_MSG)
	ZGetGameInterface()->GetMonsterBookInterface()->OnCreate();
END_IMPLEMENT_LISTENER();


// ¸ó½ºÅÍ µµ°¨
BEGIN_IMPLEMENT_LISTENER( ZGetMonsterInterfacePrevPage, MBTN_CLK_MSG)
	ZGetGameInterface()->GetMonsterBookInterface()->OnPrevPage();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZGetMonsterInterfaceNextPage, MBTN_CLK_MSG)
	ZGetGameInterface()->GetMonsterBookInterface()->OnNextPage();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZGetMonsterInterfaceQuit, MBTN_CLK_MSG)
	ZGetGameInterface()->GetMonsterBookInterface()->OnDestroy();
END_IMPLEMENT_LISTENER();


// µî·Ï
BEGIN_IMPLEMENT_LISTENER(ZGetRegisterListener, MBTN_CLK_MSG)
//	ShellExecute( g_hWnd, "open", "IEXPLORE.EXE", "http://www.gunzonline.com/start.htm", NULL, SW_SHOW);
//	ShowWindow( g_hWnd, SW_SHOWMINIMIZED);
END_IMPLEMENT_LISTENER();

// ==========================================
// 1. MAIN TABS
// ==========================================
BEGIN_IMPLEMENT_LISTENER(ZEquipWeaponTabOpen, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickMainTab(0);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZEquipArmorTabOpen, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickMainTab(1);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZEquipItemTabOpen, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickMainTab(2);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZEquipAccountTabOpen, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickMainTab(3);
END_IMPLEMENT_LISTENER();


// ==========================================
// 2. SUB-FILTERS
// ==========================================
BEGIN_IMPLEMENT_LISTENER(ZBtnWepAllClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickSubFilter(0); // All
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnWepMeleeClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickSubFilter(1); // Melee
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnWepPistolClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickSubFilter(2); // Pistol
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnWepRevolverClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickSubFilter(3); // Revolver
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnWepSMGClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickSubFilter(4); // SMG
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnWepShotgunClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickSubFilter(5); // Shotgun
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnWepRifleClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickSubFilter(6); // Rifle
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnWepMGRktClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickSubFilter(7); // Heavy
END_IMPLEMENT_LISTENER();
BEGIN_IMPLEMENT_LISTENER(ZBtnWepRocketClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickSubFilter(8); // Rocket
END_IMPLEMENT_LISTENER();


// ==========================================
// 3. ARMOR SUB-FILTER LISTENER
// ==========================================
BEGIN_IMPLEMENT_LISTENER(ZBtnArmAllClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickSubFilter(0); // All
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnArmHeadClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickSubFilter(1); // Head
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnArmChestClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickSubFilter(2); // Chest
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnArmHandsClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickSubFilter(3); // Hands
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnArmLegsClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickSubFilter(4); // Legs
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnArmFeetClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickSubFilter(5); // Feet
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnArmRingsClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickSubFilter(6); // Rings
END_IMPLEMENT_LISTENER();

// --- LOGIC 4 MAIN TAB ---
// --- LOGIC MAIN TAB SHOP CHUẨN ENUM ---
BEGIN_IMPLEMENT_LISTENER(ZBtnShopTabNewClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickShopMainTab(ZShopEquipInterface::ZSHOP_TAB_NEW);   // Truyền 0
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnShopTabBPClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickShopMainTab(ZShopEquipInterface::ZSHOP_TAB_ITEM);    // Truyền 1
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnShopTabCashClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickShopMainTab(ZShopEquipInterface::ZSHOP_TAB_CREDITS);  // Truyền 2
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnShopTabMedalClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickShopMainTab(ZShopEquipInterface::ZSHOP_TAB_MEDAL); // Truyền 3
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZBtnShopTabSellClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickShopMainTab(ZShopEquipInterface::ZSHOP_TAB_SELL);  // Truyền 4
END_IMPLEMENT_LISTENER();

// Hàm bổ trợ nhỏ giúp ẩn toàn bộ BG Pack trước khi bật BG được chọn
void HideAllPackBG(ZIDLResource* pResource)
{
	if (!pResource) return;
	MWidget* pBg0 = pResource->FindWidget("Pack_Header_Label_0"); // Hoặc Pack_Header_Label_0
	MWidget* pBg1 = pResource->FindWidget("Pack_Header_Label_1");
	MWidget* pBg2 = pResource->FindWidget("Pack_Header_Label_2");

	if (pBg0) pBg0->Show(false);
	if (pBg1) pBg1->Show(false);
	if (pBg2) pBg2->Show(false);
}

// -------------------------------------------------------------
// LISTENER CÁC NÚT PACKAGE
// -------------------------------------------------------------

BEGIN_IMPLEMENT_LISTENER(ZBtnPackage01Clk, MBTN_CLK_MSG)
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	if (pResource)
	{
		// 1. Ép ẩn tất cả BG cũ
		HideAllPackBG(pResource);

		// 2. Hiện BG riêng của Pack 1
		MWidget* pBg = pResource->FindWidget("Pack_Header_Label_0");
		if (pBg) pBg->Show(true);
	}

	// 3. Load dữ liệu Pack 0
	ZGetGameInterface()->GetShopEquipInterface()->OnSelectPackage(0);
}
END_IMPLEMENT_LISTENER();


BEGIN_IMPLEMENT_LISTENER(ZBtnPackage02Clk, MBTN_CLK_MSG)
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	if (pResource)
	{
		HideAllPackBG(pResource);

		MWidget* pBg = pResource->FindWidget("Pack_Header_Label_1");
		if (pBg) pBg->Show(true);
	}

	ZGetGameInterface()->GetShopEquipInterface()->OnSelectPackage(1);
}
END_IMPLEMENT_LISTENER();


BEGIN_IMPLEMENT_LISTENER(ZBtnPackage03Clk, MBTN_CLK_MSG)
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	if (pResource)
	{
		HideAllPackBG(pResource);

		MWidget* pBg = pResource->FindWidget("Pack_Header_Label_2");
		if (pBg) pBg->Show(true);
	}

	ZGetGameInterface()->GetShopEquipInterface()->OnSelectPackage(2);
}
END_IMPLEMENT_LISTENER();


BEGIN_IMPLEMENT_LISTENER(ZBtnBuyPackageClk, MBTN_CLK_MSG)
ZGetGameInterface()->GetShopEquipInterface()->OnClickBuyPackage();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZResetPreviewCallerButtonListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
if (!pResource) return true;

ZCharacterView* pCharacterView = (ZCharacterView*)pResource->FindWidget("EquipmentInformationShop");
if (pCharacterView)
{
	// Khôi phục lại nguyên bản đồ đang mặc trên người
	pCharacterView->SetSelectMyCharacter();
}

return true; // Trả về true để báo sự kiện Click đã xử lý xong


END_IMPLEMENT_LISTENER()

// Custom: tab Event chưa làm
BEGIN_IMPLEMENT_LISTENER(ZGetEventCallerButtonListener, MBTN_CLK_MSG)
//ZGetGameInterface()->ShowMessage("Coming soon");
int nState = ZGetGameInterface()->GetState();
if (nState == GUNZ_CHARSELECTION)
{
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	if (pResource)
	{
		// 1. ẨN TỰ THỦ CÔNG CÁC WIDGET SHOP & EQUIPMENT 
		// (Tránh gọi ShowShopDialog(false) để engine không restore lại màn Hideout 1s)
		MWidget* pShopWidget = pResource->FindWidget("Shop");
		if (pShopWidget) pShopWidget->Show(false);

		MWidget* pEquipWidget = pResource->FindWidget("Equipment");
		if (pEquipWidget) pEquipWidget->Show(false);

		// 2. Dọn dẹp các Widget nền rác
		MWidget* pLobbyWidget = pResource->FindWidget("Lobby");
		if (pLobbyWidget) pLobbyWidget->Show(false);

		MWidget* pEventWidget = pResource->FindWidget("Event");
		if (pEventWidget) pEventWidget->Show(false);

		MWidget* pStageWidget = pResource->FindWidget("Stage");
		if (pStageWidget) pStageWidget->Show(false);
	}

	// 3. Tắt UI CharSelect để không bị giật về Hideout
	ZGetGameInterface()->EnableCharSelectionInterface(false);

	// 4. Bắn gói tin chọn nhân vật chuyển thẳng sang Lobby
	if (ZCharacterSelectView::GetNumOfCharacter() > 0)
	{
		MWidget* pHideOutWidget = pResource->FindWidget("CharSelection");
		if (pHideOutWidget) pHideOutWidget->Show(false);

		ZGetGameInterface()->ShowWidget("Load", true);

		if (ZGetGameInterface()->GetCharacterSelectView() != NULL)
		{
			ZGetGameInterface()->OnCharSelect();

			MWidget* pWidget = pResource ? pResource->FindWidget("CS_SelectCharDefKey") : NULL;
			if (pWidget) pWidget->Enable(false);
		}
	}
	else
	{
		ZGetGameInterface()->ShowMessage(MSG_GAME_NOCHARACTER);
	}
}
else {
	ZGetGameInterface()->ShowEventDialog(true);
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZBtnDay1Clk, MBTN_CLK_MSG)
{
	ZGetEventManager()->RequestClaimAttendance(1);
}END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZBtnDay2Clk, MBTN_CLK_MSG)
{
	ZGetEventManager()->RequestClaimAttendance(2);
}END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZBtnDay3Clk, MBTN_CLK_MSG)
{
	ZGetEventManager()->RequestClaimAttendance(3);
}END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZBtnDay4Clk, MBTN_CLK_MSG)
{
	ZGetEventManager()->RequestClaimAttendance(4);
}END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZBtnDay5Clk, MBTN_CLK_MSG)
{
	ZGetEventManager()->RequestClaimAttendance(5);
}END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZBtnDay6Clk, MBTN_CLK_MSG)
{
	ZGetEventManager()->RequestClaimAttendance(6);
}END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZBtnDay7Clk, MBTN_CLK_MSG)
{
	ZGetEventManager()->RequestClaimAttendance(7);
}END_IMPLEMENT_LISTENER()

// for new UI: nut bong bong bat tat khung chat trong phong cho
BEGIN_IMPLEMENT_LISTENER(ZGetStageChattingToggleListener, MBTN_CLK_MSG)
ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
MWidget* pFrame = pResource->FindWidget("StageChattingFrame");
if (pFrame != NULL)
{
	pFrame->Show(!pFrame->IsVisible());
}
END_IMPLEMENT_LISTENER()