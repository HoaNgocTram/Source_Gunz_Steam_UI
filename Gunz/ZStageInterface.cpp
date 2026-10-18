/***********************************************************************
  ZStageInterface.cpp
  
  ¿E µµ : ½ºÅ×ÀÌÁEÀÎÅÍÆäÀÌ½º¸¦ °E®ÇÏ´Â Å¬·¡½º. ÄÚµE°E®»óÀÇ ÆúÔ®¸¦ À§ÇØ
           ºÐ¸®ÇßÀ½(»ç½Ç ¾ÆÁE¿ÏÀEE´Ù ºÐ¸® ¸øÇßÀ½. -_-;).
  ÀÛ¼ºÀÏ : 11, MAR, 2004
  ÀÛ¼ºÀÚ : ÀÓµ¿È¯
************************************************************************/


#include "stdafx.h"							// Include stdafx.h
#include "ZStageInterface.h"				// Include ZStageInterface.h
#include "ZStageSetting.h"					// Include ZStageSetting.h
#include "ZGameInterface.h"
#include "ZPlayerListBox.h"
#include "ZCombatMenu.h"
#include "ZShopEquipListbox.h"
#include "ZMyItemList.h"
#include "ZItemSlotView.h"
#include "ZMessages.h"
#include "ZLanguageConf.h"
#include "MNewQuestScenario.h"
#include "MMatchSpyMap.h"
#include "ZConfiguration.h"

/* ÇØ¾ßÇÒ °ÍµE..

 1. ZStageSetting °EÃ ·çÆ¾À» ¿©±â·Î ¿Å°Ü¿Í¾ß ÇÏ´ÂµE..  -_-;
 2. ¹öÆ° UIÂÊµµ ¿ª½Ã ¿©±â·Î ¿Å°Ü¿Í¾ß ÇÏ´Âµ¥ Á¹¶E²¿¿©ÀÖ´Ù...  -_-;
*/
/***********************************************************************
  ZStageInterface : public
  
  desc : »ý¼ºÀÚ
************************************************************************/
ZStageInterface::ZStageInterface( void)
{
	m_bPrevQuest = false;
	m_bDrawStartMovieOfQuest = false;
	m_pTopBgImg = NULL;
	m_pStageFrameImg = NULL;
	m_pItemListFrameImg = NULL;
	m_nListFramePos = 0;
	m_nStateSacrificeItemBox = 0;
	m_pRelayMapListFrameImg = NULL;
	m_nRelayMapListFramePos = 0;
	m_bRelayMapRegisterComplete = true;
	m_bEnableWidgetByRelayMap = true;
	m_pChallengeQuestScenario = new MNewQuestScenarioManager;

	m_pSpyBanMapListFrameImg = NULL;
	m_nSpyBanMapListFramePos = 0;

}


/***********************************************************************
  ~ZStageInterface : public
  
  desc : ¼Ò¸EÚ
************************************************************************/
ZStageInterface::~ZStageInterface( void)
{
	if ( m_pTopBgImg != NULL)
	{
		delete m_pTopBgImg;
		m_pTopBgImg = NULL;
	}

	if ( m_pStageFrameImg != NULL)
	{
		delete m_pStageFrameImg;
		m_pStageFrameImg = NULL;
	}

	delete m_pChallengeQuestScenario;
}


/***********************************************************************
  Create : public
  
  desc : ÃÊ±âÈ­
************************************************************************/
void ZStageInterface::OnCreate( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// ÃÊ±âÈ­ ÇØÁÖ°E
	m_bPrevQuest = false;
	m_bDrawStartMovieOfQuest = false;
	m_nStateSacrificeItemBox = 0;		// Hide
	m_nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
	m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();
	m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

	ReadSenarioNameXML();

	_ASSERT(m_pChallengeQuestScenario);
	m_pChallengeQuestScenario->ReadXml(ZApplication::GetFileSystem(), "system/scenario2.xml", ZGetQuest()->GetDropTable());

	MPicture* pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage0");
	if ( pPicture)
		pPicture->SetOpacity( 255);
	pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage1");
	if ( pPicture)
		pPicture->SetOpacity( 255);

	pPicture = (MPicture*)pResource->FindWidget( "Stage_MainBGTop");
	if ( pPicture)
		pPicture->SetBitmap( MBitmapManager::Get( "main_bg_t.png"));
	pPicture = (MPicture*)pResource->FindWidget( "Stage_FrameBG");
	if ( pPicture)
	{
		m_pStageFrameImg = new MBitmapR2;
		((MBitmapR2*)m_pStageFrameImg)->Create( "stage_frame.png", RGetDevice(), "interface/loadable/stage_frame.png");

		if ( m_pStageFrameImg != NULL)
			pPicture->SetBitmap( m_pStageFrameImg->GetSourceBitmap());
	}
	pPicture = (MPicture*)pResource->FindWidget( "Stage_ItemListBG");
	if ( pPicture)
	{
		m_pItemListFrameImg = new MBitmapR2;
		((MBitmapR2*)m_pItemListFrameImg)->Create( "itemlistframe.png", RGetDevice(), "interface/loadable/itemlistframe.png");

		if ( m_pItemListFrameImg != NULL)
			pPicture->SetBitmap( m_pItemListFrameImg->GetSourceBitmap());
	}
	MWidget* pWidget = (MWidget*)pResource->FindWidget( "Stage_ItemListView");
	if ( pWidget)
	{
		MRECT rect;
		rect = pWidget->GetRect();
		rect.x = -rect.w;
		m_nListFramePos = rect.x;
		pWidget->SetBounds( rect);
	}
	MLabel* pLabel = (MLabel*)pResource->FindWidget( "Stage_SenarioName");
	if ( pLabel)
		pLabel->SetText( "");
	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_SenarioNameImg");
	if ( pWidget)
		pWidget->Show( false);
	MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_SacrificeItemListbox");
	if ( pListBox)
		pListBox->RemoveAll();
	MTextArea* pDesc = (MTextArea*)pResource->FindWidget( "Stage_ItemDesc");
	if ( pDesc)
	{
		pDesc->SetTextColor( MCOLOR(0xFF808080));
		//pDesc->SetText( "¾ÆÀÌÅÛÀ» È­¸EÁß¾Ó¿¡ ÀÖ´Â µÎ°³ÀÇ Á¦´Ü¿¡ ²ø¾ûÏõÀ½À¸·Î½E°ÔÀÓ ·¹º§À» Á¶Á¤ÇÒ ¼EÀÖ½À´Ï´Ù.");
		char szText[256];
		sprintf(szText, ZMsg( MSG_QUESTITEM_USE_DESCRIPTION ));
		pDesc->SetText(szText);
	}

	{ // ¸±·¹ÀÌ¸Ê "RelayMap" ÃÊ±âÈ­
		pPicture = (MPicture*)pResource->FindWidget( "Stage_RelayMapListBG" );
		if(pPicture)
		{ // f¸±·¹ÀÌ¸Ê ¸®½ºÆ® ¹è°EÀÌ¹ÌÁE
			m_pRelayMapListFrameImg = new MBitmapR2;
			((MBitmapR2*)m_pRelayMapListFrameImg)->Create( "relaymaplistframe.png", RGetDevice(), "interface/loadable/relaymaplistframe.png");

			if(m_pRelayMapListFrameImg != NULL)
				pPicture->SetBitmap(m_pRelayMapListFrameImg->GetSourceBitmap());
		}
		MWidget* pWidget = (MWidget*)pResource->FindWidget( "Stage_RelayMapListView");
		if ( pWidget)
		{
			MRECT rect;
			rect = pWidget->GetRect();
			rect.x = -rect.w;
			m_nRelayMapListFramePos = rect.x;
			pWidget->SetBounds( rect);
		}

		MComboBox* pCombo = (MComboBox*)pResource->FindWidget("Stage_RelayMapType");			
		if ( pCombo)
			pCombo->CloseComboBoxList();
		pCombo = (MComboBox*)pResource->FindWidget("Stage_RelayMapRepeatCount");					
		if ( pCombo)
			pCombo->CloseComboBoxList();

		MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_RelayMapListbox");
		if ( pListBox)
			pListBox->RemoveAll();
		pListBox = (MListBox*)pResource->FindWidget( "Stage_MapListbox");
		if ( pListBox)
			pListBox->RemoveAll();

	}

	{ // SPY mode : ban map list.
		pPicture = (MPicture*)pResource->FindWidget("Stage_SpyBanMapListBG");
		if (pPicture)
		{ // f¸±·¹ÀÌ¸Ê ¸®½ºÆ® ¹è°EÀÌ¹ÌÁE
			m_pSpyBanMapListFrameImg = new MBitmapR2;
			((MBitmapR2*)m_pSpyBanMapListFrameImg)->Create("relaymaplistframe.png", RGetDevice(), "interface/loadable/relaymaplistframe.png");

			if (m_pSpyBanMapListFrameImg != NULL)
				pPicture->SetBitmap(m_pSpyBanMapListFrameImg->GetSourceBitmap());
		}
		MWidget* pWidget = (MWidget*)pResource->FindWidget("Stage_SpyBanMapListView");
		if (pWidget)
		{
			MRECT rect;
			rect = pWidget->GetRect();
			rect.x = -rect.w;
			m_nSpyBanMapListFramePos = rect.x;
			pWidget->SetBounds(rect);
		}

		MListBox* pListBox = (MListBox*)pResource->FindWidget("Stage_SpyBanMapList");
		if (pListBox)
			pListBox->RemoveAll();

		pLabel = (MLabel*)pResource->FindWidget("Stage_SpyRandomMapLabel");
		if (pLabel)
			pLabel->Show(false);

		MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("MapSelection");
		if (pComboBox)
			pComboBox->Show(true);
	}

	ZApplication::GetGameInterface()->ShowWidget("Stage_Flame0", false);
	ZApplication::GetGameInterface()->ShowWidget("Stage_Flame1", false);

	// °ÔÀÓ ¹æ½ÄÀÎ Ã¤·Î ¹æ ³ª°¬´Ù°¡ ´Ù¸¥ ¹æ µé¾î°¡¸é ¹ö±× »ý±â´Â°Å ¼öÁ¤ 
	MComboBox* pCombo = (MComboBox*)pResource->FindWidget("StageType");
	if (pCombo)
		pCombo->CloseComboBoxList();

	// ¸Ê¸®½ºÆ® ¿¬ Ã¤·Î ¹æ ³ª°¬´Ù°¡ ´Ù¸¥ ¹æ µé¾î°¡¸é ¹æÀåÀÌ ¾Æ´Ñµ¥µµ ¸Ê ¹Ù²ãÁö´Â ¹ö±× ¼öÁ¤
	pCombo = (MComboBox*)pResource->FindWidget("MapSelection");
	if (pCombo)
		pCombo->CloseComboBoxList();


	// Ã¤³Î ¸®½ºÆ® ¹Ú½º´Â ´Ý¾Æ¹ö¸²
	pWidget = (MWidget*)pResource->FindWidget("ChannelListFrame");
	if (pWidget)
		pWidget->Show(false);


	// È­¸é ¾÷µ¥ ÇÑ¹ø ÇØÁÖ»ï~
	UpdateSacrificeItem();
	SerializeSacrificeItemListBox();

	// QL ÃÊ±âÈ­
	OnResponseQL(0);


	ZApplication::GetGameInterface()->ShowWidget("Stage_Lights0", false);
	ZApplication::GetGameInterface()->ShowWidget("Stage_Lights1", false);
}


/***********************************************************************
  OnDestroy : public
  
  desc : ¼­¹ö³ª È¤Àº ½Ã½ºÅÛÀÇ ¿äÃ»À¸·ÎºÎÅÍ ½ºÅ×ÀÌÁEÈ­¸éÀ» »õ·Î °»½ÅÇÏ´Â ÇÔ¼E
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnDestroy( void)
{
	ZApplication::GetGameInterface()->ShowWidget( "Stage", false);

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_MainBGTop");
	if ( pPicture)
		pPicture->SetBitmap( MBitmapManager::Get( "main_bg_t.png"));
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_FrameBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemListBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);

	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapListBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);

	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_SpyBanMapListBG");
	if (pPicture)
		pPicture->SetBitmap(NULL);

	if ( m_pTopBgImg != NULL)
	{
		delete m_pTopBgImg;
		m_pTopBgImg = NULL;
	}
	if ( m_pStageFrameImg != NULL)
	{
		delete m_pStageFrameImg;
		m_pStageFrameImg = NULL;
	}
	if ( m_pItemListFrameImg != NULL)
	{
		delete m_pItemListFrameImg;
		m_pItemListFrameImg = NULL;
	}
	if (m_pRelayMapListFrameImg != NULL)
	{
		delete m_pRelayMapListFrameImg;
		m_pRelayMapListFrameImg = NULL;
	}
	if (m_pSpyBanMapListFrameImg != NULL)
	{
		delete m_pSpyBanMapListFrameImg;
		m_pSpyBanMapListFrameImg = NULL;
	}
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

//	m_SenarioDesc.clear();
}

void ZStageInterface::OnStageCharListSettup()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	for ( MStageCharSettingList::iterator itor = ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.begin();
		itor != ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.end();  ++itor) 
	{
		MSTAGE_CHAR_SETTING_NODE* pCharNode = (*itor);

		ZPlayerListBox* pItem = (ZPlayerListBox*)pResource->FindWidget( "StagePlayerList_");
		if ( pItem)
		{
			bool bMaster = false;

			if ( ZGetGameClient()->GetMatchStageSetting()->GetMasterUID() == pCharNode->uidChar)
				bMaster = true;

			pItem->UpdatePlayer( pCharNode->uidChar,(MMatchObjectStageState)pCharNode->nState,bMaster,MMatchTeam(pCharNode->nTeam));
		}
	}
}

// for new UI: hien icon nhom mod cua phong, kem tooltip liet ke day du
static void ZUpdateStageModIcons()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (pResource == NULL) return;

	MPicture* pIcon0 = (MPicture*)pResource->FindWidget("StageModIcon0");
	if (pIcon0 == NULL) return;

	// khung goc lay tu icon dau, da duoc nhan ti le san nen khoi tu quy doi
	MRECT rBase = pIcon0->GetRect();
	int nStep = rBase.w + (rBase.w / 4);

	// for new UI: mod lay thang tu node cai dat phong
	const MSTAGE_SETTING_NODE* pSet = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting();

	vector<MMatchRoomModifier> vMods;
	for (int i = 0; i < pSet->nRoomModCount && i < MMOD_END; ++i)
		vMods.push_back(pSet->RoomMods[i]);

	int nShown = 0;

	for (int g = 0; g < RMG_COUNT; ++g)
	{
		char szWidget[64];
		sprintf(szWidget, "StageModIcon%d", g);

		MPicture* pIcon = (MPicture*)pResource->FindWidget(szWidget);
		if (pIcon == NULL) continue;

		// chi lay mod thuoc dung nhom cua icon nay
		vector<MMatchRoomModifier> vGroupMods;
		for (int i = 0; i < (int)vMods.size(); ++i)
		{
			if (GetRoomModGroup(vMods[i].mod) == (ROOM_MOD_GROUP)g)
				vGroupMods.push_back(vMods[i]);
		}

		if (vGroupMods.empty())
		{
			pIcon->Show(false);
			continue;
		}

		char szTip[MWIDGET_NAME_LENGTH];	// for new UI
		BuildRoomModText(vGroupMods, szTip, sizeof(szTip));

		pIcon->SetBounds(rBase.x + nStep * nShown, rBase.y, rBase.w, rBase.h);
		pIcon->AttachToolTip(szTip);
		pIcon->Show(true);

		++nShown;
	}
}

/***********************************************************************
  OnStageInterfaceSettup : public
  
  desc : ¼­¹ö³ª È¤Àº ½Ã½ºÅÛÀÇ ¿äÃ»À¸·ÎºÎÅÍ ½ºÅ×ÀÌÁEÈ­¸éÀ» »õ·Î °»½ÅÇÏ´Â ÇÔ¼E
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnStageInterfaceSettup( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// for new UI: khung nhan vat phong cho chi xoay, khong zoom bang con lan
	ZMeshView* pCharView = (ZMeshView*)pResource->FindWidget("Stage_Charviewer");
	if (pCharView)
		pCharView->SetEnableRotateZoom(true, false);

	ZStageSetting::InitStageSettingGameType();
/*
	// ¸Ê Á¾·ù¸¦ ¼³Á¤ÇÑ´Ù.
	MComboBox* pCB = (MComboBox*)pResource->FindWidget( "MapSelection");
	if ( pCB)
	{
		int nSelected = pCB->GetSelIndex();

		InitMaps( pCB);

		if ( nSelected >= pCB->GetCount())
			nSelected = pCB->GetCount() - 1;

		pCB->SetSelIndex( nSelected);
	}
*/

	// CharListViewÀÇ Add, Remove, Update´Â ZGameClient::OnObjectCache ¿¡¼­ °E®ÇÑ´Ù.
	MSTAGE_CHAR_SETTING_NODE* pMyCharNode = NULL;
	bool bMyReady = false;		// Ready »óÅÂÀÎÁE¾Æ´ÑÁE..
	for ( MStageCharSettingList::iterator itor = ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.begin();
		itor != ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.end();  ++itor) 
	{
		MSTAGE_CHAR_SETTING_NODE* pCharNode = (*itor);

		// ³ª ÀÚ½ÅÀÏ °æ¿E
		if ( pCharNode->uidChar == ZGetGameClient()->GetPlayerUID()) 
		{
			pMyCharNode = pCharNode;
			if (pMyCharNode->nState == MOSS_READY)
				bMyReady = true;
			else
				bMyReady = false;
		}

		ZPlayerListBox* pItem = (ZPlayerListBox*)pResource->FindWidget( "StagePlayerList_");
		if ( pItem)
		{
			bool bMaster = false;

			if ( ZGetGameClient()->GetMatchStageSetting()->GetMasterUID() == pCharNode->uidChar)
				bMaster = true;
			
			pItem->UpdatePlayer( pCharNode->uidChar,(MMatchObjectStageState)pCharNode->nState,bMaster,MMatchTeam(pCharNode->nTeam));
		}
	}

	// ½ºÅ×ÀÌÁöÀÇ ¹öÆ° »óÅÂ(°ÔÀÓ½ÃÀÛ, ³­ÀÔ, ÁØºñ¿Ï·E¸¦ ¼³Á¤ÇÑ´Ù.
	ChangeStageButtons( ZGetGameClient()->IsForcedEntry(), ZGetGameClient()->AmIStageMaster(), bMyReady);

	// ½ºÅ×ÀÌÁöÀÇ...
	ChangeStageGameSetting( ZGetGameClient()->GetMatchStageSetting()->GetStageSetting());
	
	// ³­ÀÔ ¸â¹öÀÏ °æ¿E¡...
	if ( !ZGetGameClient()->AmIStageMaster() && ( ZGetGameClient()->IsForcedEntry()))
	{
		if ( pMyCharNode != NULL)
			ChangeStageEnableReady( bMyReady);
	}

	// ¸¸¾E³­ÀÔÀ¸·Î µé¾ûÛÔ´Âµ¥ ´Ù¸¥ »ç¶E´Ù ³ª°¡¼­ ³»°¡ ¹æÀåÀÌ µÇ¾ú´Ù¸E³­ÀÔ¸ðµEÇØÁ¦
	if ( (ZGetGameClient()->AmIStageMaster() == true) && ( ZGetGameClient()->IsForcedEntry()))
	{
		if ( ZGetGameClient()->GetMatchStageSetting()->GetStageState() == STAGE_STATE_STANDBY)
		{
			ZGetGameClient()->ReleaseForcedEntry();

			// ÀÎÅÍÆäÀÌ½º°EÃ
			if(ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_SPY)
			ZApplication::GetGameInterface()->EnableWidget("StageSettingCaller", true); 
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", true);			
			ZApplication::GetGameInterface()->EnableWidget( "StageType", true);				
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", true);		
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", true);
#ifdef _BOXLEAD
			ZApplication::GetGameInterface()->EnableWidget("Lead", true);
#endif

		}
		else	
		{
			
			ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);	
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", false);			
			ZApplication::GetGameInterface()->EnableWidget( "StageType", false);			
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);		
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", false);	
#ifdef _BOXLEAD
			ZApplication::GetGameInterface()->EnableWidget("Lead", false);
#endif
		}
	}
	/// ¸±·¹ÀÌ¸ÊÀÌ¸é¼­ ´ÙÀ½¸ÊÀ¸·Î °è¼Ó ÁøÇàÁßÀÏ¶§ ½ÃÀÛ ¹öÆ°À» ¸ðµÎ OFF ½ÃÄÑÁØ´Ù.
	if(	ZGetGameClient()->AmIStageMaster() && 
		!m_bEnableWidgetByRelayMap && 
		ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->bIsRelayMap)
	{
		ZApplication::GetGameInterface()->EnableWidget( "GameStart", false);
		// ZApplication::GetGameInterface()->EnableWidget( "StageReady", false);	// ÁØºñ¿Ï·á¹öÆ°(±âÈ¹¿¡ µû¶Eº¯°æ°¡´É)

		// ÀÎÅÍÆäÀÌ½º°EÃ
		ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller",	false);		// ¹æ¼³Á¤ ¹öÆ°
		ZApplication::GetGameInterface()->EnableWidget( "MapSelection",			false);		// ¸Ê¼±ÅÃ ÄÞº¸¹Ú½º
		ZApplication::GetGameInterface()->EnableWidget( "StageType",			false);		// °ÔÀÓ¹æ½Ä ÄÞº¸¹Ú½º
		ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer",		false);		// ÃÖ´EÎ¿EÄÞº¸¹Ú½º
		ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount",		false);		// °æ±âÈ½¼EÄÞº¸¹Ú½º
#ifdef _BOXLEAD
		ZApplication::GetGameInterface()->EnableWidget("Lead", false);
#endif

		ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller",		false);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", false);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapType",		false);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapRepeatCount", false);
	}

	MPicture* pPicture = 0;
	MBitmap* pBitmap = 0;
	// È­¸E»ó´ÜÀÇ ¸Ê ÀÌ¹ÌÁE¼³Á¤ÇÏ±E
	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		char szMapName[256];
		char szAlias[256];

		// for new UI
		const char* szMap = ZGetGameClient()->GetMatchStageSetting()->GetMapName();
		if (0 == strcmp(MMATCH_MAPNAME_RELAYMAP, szMap))
			szMap = MMATCH_DEFAULT_STAGESETTING_MAPNAME;

		pPicture = (MPicture*)pResource->FindWidget("Stage_MainBGTop");
		if (pPicture)
		{
			sprintf(szAlias, "RoomBanner_%s.png", szMap);
			MBitmap* pBanner = MBitmapManager::Get(szAlias);

			if (pBanner == NULL)
			{
				// khong co trong atlas -> quay ve cach cu, nap file roi
				sprintf(szMapName, "interface/MapGB/%s", MGetMapDescMgr()->GetMapImageName(szMap));
				if (m_pTopBgImg != NULL) { delete m_pTopBgImg; m_pTopBgImg = NULL; }
				m_pTopBgImg = new MBitmapR2;
				((MBitmapR2*)m_pTopBgImg)->Create("TopBgImg.png", RGetDevice(), szMapName);
				if (m_pTopBgImg != NULL)
					pBanner = m_pTopBgImg->GetSourceBitmap();
			}

			pPicture->SetBitmap(pBanner);
		}

		// lop chu ten map de len tren
		MPicture* pMapTitle = (MPicture*)pResource->FindWidget("Stage_MainBGTitleTop");
		if (pMapTitle)
		{
			sprintf(szAlias, "RoomBanner_%s_Title.png", szMap);
			pMapTitle->SetBitmap(MBitmapManager::Get(szAlias));
		}
	}

	MLabel* pLabel = (MLabel*)pResource->FindWidget("StageNameLabel");
	if (pLabel != 0)
	{
		char szStr[256];
		// Custom: remove servername from titlebar
		sprintf(szStr, "%03d: %s", ZGetGameClient()->GetStageNumber(), ZGetGameClient()->GetStageName());
		pLabel->SetText(szStr);

		// for new UI: o khoa dat ngay sau ten phong, do be rong chu de biet cho dat
		MPicture* pLockIcon = (MPicture*)pResource->FindWidget("StagePasswordIcon");
		if (pLockIcon != NULL)
		{
			bool bPrivate = ZGetGameClient()->GetMatchStageSetting()->IsPrivate();

			pLockIcon->Show(bPrivate);
			if (bPrivate)
			{
				MFont* pFont = pLabel->GetFont();
				MRECT rLabel = pLabel->GetRect();
				int nTextW = (pFont != NULL) ? pFont->GetWidth(szStr) : 0;

				MRECT rLock = pLockIcon->GetRect();
				pLockIcon->SetBounds(rLabel.x + nTextW + 4, rLabel.y + 6, rLock.w, rLock.h);
			}
		}
	}

	// »óÇÏ´Ü ½ºÆ®¶óÀÌÇÁÀÇ »ö»E¹Ù²Ù±E
#define SDM_COLOR			MCOLOR(255,0,0)
#define TDM_COLOR			MCOLOR(0,255,0)
#define SGD_COLOR			MCOLOR(0,0,255)
#define TGD_COLOR			MCOLOR(255,255,0)
#define ASSASIN_COLOR		MCOLOR(255,0,255)
#define TRAINING_COLOR		MCOLOR(0,255,255)
#define QUEST_COLOR			MCOLOR(255,255,255)
#define SURVIVAL_COLOR		MCOLOR(255,255,255)

	MCOLOR color;
	switch ( ZGetGameClient()->GetMatchStageSetting()->GetGameType() )
	{	
		case MMATCH_GAMETYPE_ASSASSINATE:
			color = ASSASIN_COLOR;
			break;

		case MMATCH_GAMETYPE_DEATHMATCH_SOLO:
			color = SDM_COLOR;
			break;

		case MMATCH_GAMETYPE_DEATHMATCH_TEAM:
		case MMATCH_GAMETYPE_DEATHMATCH_TEAM2:
		case MMATCH_GAMETYPE_GHOST:			// Custom: Ghost Mode - Desperate
		case MMATCH_GAMETYPE_CTF:
		case MMATCH_GAMETYPE_INFECTED:
			color = TDM_COLOR;
			break;

		case MMATCH_GAMETYPE_GLADIATOR_SOLO:
		case MMATCH_GAMETYPE_GUNGAME:
#ifdef _PAINTMODE
		case MMATCH_GAMETYPE_PAINTBALL_SOLO:
#endif
			color = SGD_COLOR;
			break;

		case MMATCH_GAMETYPE_GLADIATOR_TEAM:
			color = TGD_COLOR;
			break;

		case MMATCH_GAMETYPE_TRAINING:
#ifdef _PAINTMODE
		case MMATCH_GAMETYPE_TEAM_TRAINING:
#endif
		case MMATCH_GAMETYPE_PAINTBALL_TEAM:
			color = TRAINING_COLOR;
			break;

#ifdef _QUEST
		case MMATCH_GAMETYPE_SURVIVAL:
			color = QUEST_COLOR;
			break;

		case MMATCH_GAMETYPE_QUEST:
			color = SURVIVAL_COLOR;
			break;
#endif
		case MMATCH_GAMETYPE_QUEST_CHALLENGE:
			color = QUEST_COLOR; //todok Äù½ºÆ®Ã§¸°ÁE»ö±E
			break;

		case MMATCH_GAMETYPE_BERSERKER:
			color = SDM_COLOR;
			break;

		case MMATCH_GAMETYPE_DUEL:
			color = SDM_COLOR;
			break;
		case MMATCH_GAMETYPE_DUELTOURNAMENT:
			color = SDM_COLOR;
			break;
		case MMATCH_GAMETYPE_TURBO:
			color = SDM_COLOR;
			break;
		case MMATCH_GAMETYPE_DROPMAGIC:
			color = SDM_COLOR;
			break;
		case MMATCH_GAMETYPE_SKILLMAP:
			color = SDM_COLOR;
			break;
		case MMATCH_GAMETYPE_SPY:
			color = TDM_COLOR;
			break;

		//Custom: Rule Custom Game
		case MMATCH_GAMETYPE_CLASSIC_SOLO:
			color = SDM_COLOR;
			break;
		case MMATCH_GAMETYPE_MODE_STAFF:
			color = SDM_COLOR;
			break;
		case MMATCH_GAMETYPE_VAMPIRE:
			color = SDM_COLOR;
			break;
		case MMATCH_GAMETYPE_SNIPERMODE:
			color = TRAINING_COLOR;
			break;
		case MMATCH_GAMETYPE_CLASSIC_TEAM:
			color = TRAINING_COLOR;
			break;


		default:
			_ASSERT(0);
			color = MCOLOR(255,255,255,255);
	}
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripBottom");
	if(pPicture != NULL && !pPicture->IsAnim())
	{		
        pPicture->SetBitmapColor( color );
		if(!(pPicture->GetBitmapColor().GetARGB() == pPicture->GetReservedBitmapColor().GetARGB()))
			pPicture->SetAnimation( 2, 700.0f);
	}
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripTop");
	if(pPicture != NULL && !pPicture->IsAnim())	
	{
		pPicture->SetBitmapColor( color );
		if(!(pPicture->GetBitmapColor().GetARGB() == pPicture->GetReservedBitmapColor().GetARGB()))
			pPicture->SetAnimation( 3, 700.0f);		
	}
	ZStageOptApplyPendingCombos();	// for new UI
}

void ZStageInterface::SetStageRelayMapImage()
{
	MPicture* pPicture = 0;
	MBitmap* pBitmap = 0;
	char szMapName[256];
	pPicture = (MPicture*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Stage_MainBGTop");
	if(!pPicture) return;
	MListBox* pRelayMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapListbox");
	if(pRelayMapListBox == NULL) return;
	if( 0 < pRelayMapListBox->GetCount())
	{
		sprintf( szMapName, "interface/MapGB/%s", MGetMapDescMgr()->GetMapImageName( pRelayMapListBox->GetString(pRelayMapListBox->GetStartItem())));
		if ( m_pTopBgImg != NULL)
		{
			delete m_pTopBgImg;
			m_pTopBgImg = NULL;
		}

		m_pTopBgImg = new MBitmapR2;
		((MBitmapR2*)m_pTopBgImg)->Create( "TopBgImg.png", RGetDevice(), szMapName);

		if ( m_pTopBgImg != NULL)
			pPicture->SetBitmap( m_pTopBgImg->GetSourceBitmap());
	}
}

void ZStageInterface::SetEnableWidgetByRelayMap(bool b)
{
	if(!ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->bIsRelayMap)
	{	// ¸±·¹ÀÌ¸ÊÀÌ ¾Æ´Ï¸E¸±·¹ÀÌ¸Ê¿EÀ§Á¬ ºñÈ°¼ºÈ­ Ã³¸®¸¦ ÇÒ ÇÊ¿E¾ø´Ù.
		m_bEnableWidgetByRelayMap = true;
		return;
	}

	m_bEnableWidgetByRelayMap = b;
}

/***********************************************************************
  ChangeStageGameSetting : public
  
  desc : ÀÌ°Íµµ °ÔÀÓ °EÃ ÀÎÅÍÆäÀÌ½º¸¦ ¼öÁ¤ÇÏ´Â°Å °°Àºµ¥... ¿ÖÀÌ·¸°Ô ¸¹ÀÌ ³ª´²³õÀº°ÅÁE -_-;
         ÁÖ·Î È­¸éÀÇ ÀE¼ÀûÀÎ UI¸¦ ¼³Á¤ÇÑ´Ù.
  arg  : pSetting = ½ºÅ×ÀÌÁE¼³Á¤ Á¤º¸
  ret  : none
************************************************************************/
void ZStageInterface::ChangeStageGameSetting( const MSTAGE_SETTING_NODE* pSetting)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	m_nGameType = pSetting->nGameType;
	SetMapName(pSetting->szMapName);

	//Custom: Lightmaps Time Code
	SetMapTOD(pSetting->lightMapIndex);

#ifdef _LIGHTURNOFF
	//ZGetConfiguration()->GetVideo()->bLightMap = pSetting->bLightMapTurn;
	//RBspObject::SetDrawLightMap(ZGetConfiguration()->GetVideo()->bLightMap);
#endif

	ZApplication::GetGameInterface()->m_bTeamPlay = ZGetGameTypeManager()->IsTeamGame( pSetting->nGameType);


	// °EEÇã¿E¿©ºÎ È®ÀÎ
	MComboBox* pCombo = (MComboBox*)pResource->FindWidget( "StageObserver");
	MButton* pObserverBtn = (MButton*)pResource->FindWidget( "StageObserverBtn");
	MLabel* pObserverLabel = (MLabel*)pResource->FindWidget( "StageObserverLabel");
	if ( pCombo && pObserverBtn && pObserverLabel)
	{
		if ( pCombo->GetSelIndex() == 1)
		{
			pObserverBtn->SetCheck( false);
			pObserverBtn->Enable( false);
			pObserverLabel->Enable( false);
		}
		else
		{
			pObserverBtn->Enable( true);
			pObserverLabel->Enable( true);
		}
	}

	// Ã»ÆÀ, È«ÆÀ »óÅÂ ¼³Á¤
	ZApplication::GetGameInterface()->UpdateBlueRedTeam();

	// for new UI
	{
		const char* szGameTypeStr = ZGetGameTypeManager()->GetGameTypeStr(pSetting->nGameType);
		bool bTeam = ZGetGameTypeManager()->IsTeamGame(pSetting->nGameType);

		// dòng mô tả dưới tên phòng
		MLabel* pModeDesc = (MLabel*)pResource->FindWidget("StageMapNameLabel");
		if (pModeDesc)
			pModeDesc->SetText(szGameTypeStr ? szGameTypeStr : "");

		// nhãn lớn giữa khung danh sách
		char szUpper[32] = "";
		if (szGameTypeStr)
		{
			strncpy(szUpper, szGameTypeStr, sizeof(szUpper) - 1);
			for (int i = 0; szUpper[i]; i++)
				szUpper[i] = (char)toupper((unsigned char)szUpper[i]);
		}

		MLabel* pModeLabel = (MLabel*)pResource->FindWidget("Stage_PlayerList_Mode");
		MLabel* pRedLabel = (MLabel*)pResource->FindWidget("Stage_PlayerList_Red");
		MLabel* pBlueLabel = (MLabel*)pResource->FindWidget("Stage_PlayerList_Blue");

		if (pModeLabel)
		{
			pModeLabel->SetText(szUpper);
			pModeLabel->Show(!bTeam);
		}
		if (pRedLabel)  pRedLabel->Show(bTeam);
		if (pBlueLabel) pBlueLabel->Show(bTeam);

		// for new UI
		MWidget* pDeco = pResource->FindWidget("Stage_PlayerList_Deco");
		MWidget* pDecoRed = pResource->FindWidget("Stage_PlayerList_Red_Deco");
		MWidget* pDecoBlue = pResource->FindWidget("Stage_PlayerList_Blue_Deco");
		if (pDeco)     pDeco->Show(!bTeam);
		if (pDecoRed)  pDecoRed->Show(bTeam);
		if (pDecoBlue) pDecoBlue->Show(bTeam);

		// for new UI: nhan mod ve lai moi khi cai dat phong doi
		ZUpdateStageModIcons();
	}

	// °ÔÀÓ ¹æ½Ä¿¡ µû¶ó¼­ UI¸¦ º¯°æÇÑ´Ù
	MAnimation* pAniMapImg = (MAnimation*)pResource->FindWidget( "Stage_MapNameBG");
	bool bQuestUI = false;
	bool bChallengeQuestUI = false;
	bool bSpyUI = false;

	//Custom: Rule Custom Game
	if ( (pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_SOLO) ||			// µ¥¾²¸ÅÄ¡ °³ÀÎÀEÌ°Å³ª...
		 (pSetting->nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO) ||				// Ä®ÀE°³ÀÎÀEÌ°Å³ª...
		 (pSetting->nGameType == MMATCH_GAMETYPE_BERSERKER) ||					// ¹ö¼­Ä¿¸ðµåÀÌ°Å³ª...
		 (pSetting->nGameType == MMATCH_GAMETYPE_TURBO) ||
#ifdef _PAINTMODE
		 (pSetting->nGameType == MMATCH_GAMETYPE_PAINTBALL_SOLO) ||
#endif
		 (pSetting->nGameType == MMATCH_GAMETYPE_DROPMAGIC) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_SKILLMAP) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_TRAINING) ||					// Æ®·¹ÀÌ´×ÀÌ°Å³ª...
		 (pSetting->nGameType == MMATCH_GAMETYPE_VAMPIRE) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_CLASSIC_SOLO) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_MODE_STAFF) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_DUEL))							// µà¾ó¸ðµEÀÌ¸E..
	{
		// ¸Ê ÀÌ¸§ ¹è°EÀÌ¹ÌÁEº¯È¯
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame(0);

		// Äù½ºÆ® UI °¨ÃE
		bQuestUI = false;
		bChallengeQuestUI = false;
	}
	else if ( (pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||		// µ¥¾²¸ÅÄ¡ ÆÀÀEÌ°Å³ª...
		(pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM2) ||			// ¹«ÇÑµ¥½º¸ÅÄ¡ ÆÀÀEÌ°Å³ª...
		 (pSetting->nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||				// Ä®ÀEÆÀÀEÌ°Å³ª...
		 (pSetting->nGameType == MMATCH_GAMETYPE_ASSASSINATE) ||				// ¾Ï»EEÀÌ¸E..
		 (pSetting->nGameType == MMATCH_GAMETYPE_GHOST) ||						// Custom: Ghost Mode - Desperate
#ifdef _PAINTMODE
		 (pSetting->nGameType == MMATCH_GAMETYPE_PAINTBALL_TEAM) ||
#endif
		 (pSetting->nGameType == MMATCH_GAMETYPE_TEAM_TRAINING) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_CLASSIC_TEAM) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_SNIPERMODE) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_CTF))
	{
		// ¸Ê ÀÌ¸§ ¹è°EÀÌ¹ÌÁEº¯È¯
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame(1);

		// Äù½ºÆ® UI °¨ÃE
		bQuestUI = false;
		bChallengeQuestUI = false;
	}
	else if ( pSetting->nGameType == MMATCH_GAMETYPE_SURVIVAL)					// Ã§¸°ÁöÄù½ºÆ® ¸ðµE/todok µû·Î ÇØ¾ßÇÒ±E
	{
		// ¸Ê ÀÌ¸§ ¹è°EÀÌ¹ÌÁEº¯È¯
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame(0);

		// Äù½ºÆ® UI °¨ÃE
		bQuestUI = false;
		bChallengeQuestUI = false;
	}
	else if ( pSetting->nGameType == MMATCH_GAMETYPE_QUEST)						// Äù½ºÆ® ¸ðµåÀÌ¸E..
	{
		// ¸Ê ÀÌ¸§ ¹è°EÀÌ¹ÌÁEº¯È¯
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame(2);

		// Äù½ºÆ® UI º¸ÀÓ
		bQuestUI = true;
		bChallengeQuestUI = false;
	}

	else if (pSetting->nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE)					// ¼­¹ÙÀÌ¹E¸ðµåÀÌ¸E..
	{
		// ¸Ê ÀÌ¸§ ¹è°EÀÌ¹ÌÁEº¯È¯
		if (pAniMapImg)
			pAniMapImg->SetCurrentFrame(3);

		// Äù½ºÆ® UI °¨ÃE
		bChallengeQuestUI = true;
	}

	else if ( pSetting->nGameType == MMATCH_GAMETYPE_INFECTED)					// Custom: Infected game mode stage
	{
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame(4);

		bQuestUI = false;
		bChallengeQuestUI = false;
	}

	else if (pSetting->nGameType == MMATCH_GAMETYPE_GUNGAME)					
	{
		if (pAniMapImg)
			pAniMapImg->SetCurrentFrame(5);

		bQuestUI = false;
		bChallengeQuestUI = false;
	}

	else if (pSetting->nGameType == MMATCH_GAMETYPE_SPY)
	{
		if (pAniMapImg)
			pAniMapImg->SetCurrentFrame(6);	// Check your interface templates.xml for more details about this number.

		bQuestUI = false;
		bChallengeQuestUI = false;
		bSpyUI = true;
	}

	// ¸Ê¼±ÅÃ½Ã ¸±·¹ÀÌ¸ÊÀÌ¸E¸±·¹ÀÌ¸Ê ¸®½ºÆ® ¹Ú½º¸¦ ¿­¾ûÝØ´Ù.
	 if(pSetting->bIsRelayMap)
		OpenRelayMapBox();
	else
		HideRelayMapBox();

	 ///// SPY MODE /////
	 if (bSpyUI)
		 OpenSpyBanMapBox();
	 else
		 HideSpyBanMapBox();

	 MLabel* pSpyRandomMapLabel = (MLabel*)pResource->FindWidget("Stage_SpyRandomMapLabel");
	 if (pSpyRandomMapLabel)
		 pSpyRandomMapLabel->Show(bSpyUI);

pCombo = (MComboBox*)pResource->FindWidget("MapSelection");
if (pCombo)
pCombo->Show(!bSpyUI);
////////////////////
ZApplication::GetGameInterface()->ShowWidget("Stage_SacrificeItemImage0", bQuestUI);
ZApplication::GetGameInterface()->ShowWidget("Stage_SacrificeItemImage1", bQuestUI);
ZApplication::GetGameInterface()->ShowWidget("Stage_QuestLevel", bQuestUI);
ZApplication::GetGameInterface()->ShowWidget("Stage_QuestLevelBG", bQuestUI);

#ifdef _BOXLEAD
MButton* pButton = (MButton*)pResource->FindWidget("Lead");
if (pButton != 0) pButton->SetCheck(!pSetting->bIsLeading);
#endif

// Custom: CQ Add Time Limit
ZApplication::GetGameInterface()->ShowWidget("Stage_ChallengeTime", bChallengeQuestUI);
ZApplication::GetGameInterface()->ShowWidget("Stage_ChallengeTimeBG", bChallengeQuestUI);

ZApplication::GetGameInterface()->ShowWidget("Stage_SacrificeItemButton0", bQuestUI);
ZApplication::GetGameInterface()->ShowWidget("Stage_SacrificeItemButton1", bQuestUI);
ZApplication::GetGameInterface()->ShowWidget("Quest_ItemListView", bQuestUI);
ZApplication::GetGameInterface()->ShowWidget("Stage_PutSacrificeItem", bQuestUI);

if (m_bPrevQuest != bQuestUI)
{
	ZApplication::GetGameInterface()->ShowWidget("Stage_Lights0", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget("Stage_Lights1", bQuestUI);

	m_SacrificeItem[SACRIFICEITEM_SLOT0].RemoveItem();
	m_SacrificeItem[SACRIFICEITEM_SLOT1].RemoveItem();

	UpdateSacrificeItem();

	if (bQuestUI)
	{
		//			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);
		ZPostRequestSacrificeSlotInfo(ZGetGameClient()->GetPlayerUID());
		ZPostRequestQL(ZGetGameClient()->GetPlayerUID());
		OpenSacrificeItemBox();
	}
	else
	{
		MLabel* pLabel = (MLabel*)pResource->FindWidget("Stage_SenarioName");
		if (pLabel)
			pLabel->SetText("");
		ZApplication::GetGameInterface()->ShowWidget("Stage_SenarioNameImg", false);
		//			if (ZGetGameClient()->AmIStageMaster())
		//				ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", true);

		HideSacrificeItemBox();
	}

	m_bPrevQuest = !m_bPrevQuest;
}
if (pSetting->nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE || pSetting->nGameType == MMATCH_GAMETYPE_SPY)
{
	ZApplication::GetGameInterface()->EnableWidget("StageSettingCaller", false);
	ZApplication::GetGameInterface()->EnableWidget("MapSelection", true);
	ZApplication::GetGameInterface()->EnableWidget("StageType", true);
	ZApplication::GetGameInterface()->EnableWidget("StageMaxPlayer", true);
	ZApplication::GetGameInterface()->EnableWidget("StageRoundCount", false);
#ifdef _BOXLEAD
	ZApplication::GetGameInterface()->EnableWidget("Lead", true);
#endif
}
if ((pSetting->nGameType == MMATCH_GAMETYPE_SURVIVAL) || 
	(pSetting->nGameType == MMATCH_GAMETYPE_QUEST) || 
	(pSetting->nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE) ||
	(pSetting->nGameType == MMATCH_GAMETYPE_SPY))
ZApplication::GetGameInterface()->EnableWidget("StageSettingCaller", false);
ZApplication::GetGameInterface()->ShowWidget("Stage_SenarioDescription", bChallengeQuestUI);
ZApplication::GetGameInterface()->ShowWidget("Stage_SenarioTitle", bChallengeQuestUI);
ZApplication::GetGameInterface()->ShowWidget("Stage_ChallengeTime", bChallengeQuestUI);
if (bChallengeQuestUI)
{
	char szText[256];
	MNewQuestScenarioManager* pNewScenarioMgr = ZApplication::GetStageInterface()->GetChallengeQuestScenario();
	if (pNewScenarioMgr)
	{
		MNewQuestScenario* pNewScenario = pNewScenarioMgr->GetScenario(pSetting->szMapName);
		if (pNewScenario)
		{
			MTextArea* pNewScenarioDesc = (MTextArea*)pResource->FindWidget("Stage_SenarioDescription");
			if (pNewScenarioDesc)
			{
				pNewScenarioDesc->SetText(pNewScenario->GetDescription());
			}

			MLabel* pNewScenarioTitle = (MLabel*)pResource->FindWidget("Stage_SenarioTitle");
			if (pNewScenarioTitle)
			{
				pNewScenarioTitle->SetText(pNewScenario->GetName());
			}
			// Custom: CQ Add Time Limit _SYSINTERNEW
			MLabel* pGoodTime = (MLabel*)pResource->FindWidget("Stage_ChallengeTime");
			if (pGoodTime)
			{
				sprintf(szText, "Gen. Record : 8:00", pNewScenario->GetGoodTime() / 60, pNewScenario->GetGoodTime() % 60);
				pGoodTime->SetText(szText);
			}
			MPicture* pPicture = 0;
			MBitmap* pBitmap = 0;
			char szMapName[256];
			pPicture = (MPicture*)pResource->FindWidget("Stage_MainBGTop");
			if (pPicture)
			{
				if ((pNewScenario->GetID() == 101) || (pNewScenario->GetID() == 201) || 
					(pNewScenario->GetID() == 301) || (pNewScenario->GetID() == 401))
				{
					sprintf(szMapName, "interface/MapGB/challengequest_guerilla.bmp");
				}
				else
				{
					sprintf(szMapName, "interface/MapGB/challengequest_research.bmp");
				}
				if (pNewScenario->GetID() == 501)
				{
					sprintf(szMapName, "interface/MapGB/blitzkrieg.bmp");
				}
				if (m_pTopBgImg != NULL)
				{
					delete m_pTopBgImg;
					m_pTopBgImg = NULL;
				}

				m_pTopBgImg = new MBitmapR2;
				((MBitmapR2*)m_pTopBgImg)->Create("TopBgImg.png", RGetDevice(), szMapName);

				if (m_pTopBgImg != NULL)
					pPicture->SetBitmap(m_pTopBgImg->GetSourceBitmap());
				}
			}
		}
	}

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageRoundCountLabel");
	if ( pWidget)
	{
		if ((pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_SOLO) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM2) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_TRAINING) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_BERSERKER) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_DUEL) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_GUNGAME))
			pWidget->SetText( ZMsg(MSG_WORD_KILL));

		if (pSetting->nGameType == MMATCH_GAMETYPE_SKILLMAP)
			pWidget->SetText(ZMsg(MSG_WORD_ROUND));

		else if(pSetting->nGameType == MMATCH_GAMETYPE_CTF)
			pWidget->SetText(ZMsg(MSG_WORD_CAPUTRES));
		else
			pWidget->SetText( ZMsg(MSG_WORD_ROUND));
	}


	// ÄÞÆ÷³ÍÆ® ¾÷µ¥ÀÌÆ®
	ZStageSetting::ShowStageSettingDialog( pSetting, false);


	// °ÔÀÓÁß ¸Þ´º ¼öÁ¤ - ¸ÓÇÏ´Â ºÎºÐÀÎÁE¾Ë¼E¾øÀ½...
	// °ÔÀÓÁß ¸Þ´º¿¡¼­ Äù½ºÆ®¹× ¼­¹ÙÀÌ¹úÀÏ¶§ ´Eâ¹æÀ¸·Î ³ª°¡±E¹öÆ° ºñÈ°¼ºÈ­ ½ÃÅ´
#ifdef _QUEST
	if (ZGetGameTypeManager()->IsQuestDerived(pSetting->nGameType) || pSetting->nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE || ZGetGameClient()->IsCWChannel())
	{
		ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, false);
	}
	else if (ZGetGameClient()->IsLadderWarsChannel())
	{
		ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, false);
	}
	else
	{
		ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, true);
	}
#endif
	ZStageOptApplyPendingCombos();	// for new UI
}


/***********************************************************************
  ChangeStageButtons : public
  
  desc : ½ºÅ×ÀÌÁE³»ÀÇ ¹öÆ°µE°ÔÀÓ½ÃÀÛ, ³­ÀÔ, ÁØºñ¿Ï·EÀÇ »óÅÂ¸¦ ¼³Á¤ÇÑ´Ù.
         °ÔÀÓ ¼³Á¤°E°EÃµÈ À§Á¬ÀÇ UI¸¦ ¼³Á¤ÇÑ´Ù.
  arg  : bForcedEntry = ³­ÀÔ ¿©ºÎ(true or false)
         bMaster = ¹æÀE¿©ºÎ(true or false)
		 bReady = ÁØºE¿Ï·E¿©ºÎ(true or false)
  ret  : none
************************************************************************/
void ZStageInterface::ChangeStageButtons( bool bForcedEntry, bool bMaster, bool bReady)
{
	if ( bForcedEntry)	// ³­ÀÔ ¸ðµE
	{
		ZApplication::GetGameInterface()->ShowWidget( "GameStart", false);
		ZApplication::GetGameInterface()->ShowWidget( "StageReady", false);

		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame", true);
		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame2", true);

		ChangeStageEnableReady( false);
	}
	else
	{
		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame", false);
		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame2", false);

		ZApplication::GetGameInterface()->ShowWidget( "GameStart", bMaster);
		ZApplication::GetGameInterface()->ShowWidget( "StageReady", !bMaster);

		if ( bMaster)
		{
			// º¸ÅE¹æÀåÀÌ¶ó¸E¹æ¼¼ÆÃÀ» ¹Ù²Ü ¼EÀÖ´Â ±ÇÇÑÀ» °¡Áø´Ù (UI enable).
			// ±×·¯³ª ±×·¡¼­´Â ¾ÈµÇ´Â °æ¿E¡ ÀÖ´Âµ¥ ±×°É Ã¼Å©ÇÑ´Ù.
			bool bMustDisableUI_despiteMaster = false;
			
			// Äù½ºÆ®¸ðµå¿¡¼­ ¹æÀåÀÌ °ÔÀÓ½ÃÀÛÀ» ´©¸£°EÈñ»ý¾ÆÀÌÅÛ ºÒÅ¸´Â ¾Ö´Ï¸ÞÀÌ¼Ç µµÁß ¹æÀåÀÌ ½º½º·Î¸¦ kickÇØ¼­
			// ´Ù¸¥ »ç¶÷ÀÌ ¹æÀE±ÇÇÑÀ» ¾òÀº °æ¿E ³»°¡ ¹æÀåÀÏÁö¶óµµ ´õÀÌ»E¹æ¼¼ÆÃÀ» ¹Ù²Ü ¼ö´Â ¾ø´Ù
			if ( m_bDrawStartMovieOfQuest)
				bMustDisableUI_despiteMaster = true;


			if (bMustDisableUI_despiteMaster)
				ChangeStageEnableReady( true);	// UI disable
			else
				ChangeStageEnableReady( false);	// UI enable
		}
		else
			ChangeStageEnableReady( bReady);
	}
}


/***********************************************************************
  ChangeStageEnableReady : public
  
  desc : ½ºÅ×ÀÌÁöÀÇ ÀÎÅÍÆäÀÌ½º(¹öÆ° enableµE¸¦ ÄÑ°Å³ª ²ô´Â ÇÔ¼E
  arg  : true(=interface enable) or false(=interface disable)
  ret  : none
************************************************************************/
void ZStageInterface::ChangeStageEnableReady( bool bReady)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	ZApplication::GetGameInterface()->EnableWidget( "GameStart", !bReady);

	ZApplication::GetGameInterface()->EnableWidget( "StageTeamBlue",  !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "StageTeamBlue2", !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "StageTeamRed",  !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "StageTeamRed2", !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "Lobby_StageExit", !bReady);

	if ( (m_nGameType == MMATCH_GAMETYPE_SURVIVAL) || (m_nGameType == MMATCH_GAMETYPE_QUEST))
	{
		ZApplication::GetGameInterface()->EnableWidget( "Stage_SacrificeItemListbox", !bReady);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_PutSacrificeItem",     !bReady);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_SacrificeItemButton0", !bReady);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_SacrificeItemButton1", !bReady);
		if ( ZGetGameClient()->AmIStageMaster())
		{
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageType", !bReady);
		}
//		ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);
		ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);
		ZApplication::GetGameInterface()->EnableWidget("Lead", false);

		//Custom: Lightmaps Time Code
		ZApplication::GetGameInterface()->EnableWidget("MapTime", false);
	}
	else
	{
		if ( ZGetGameClient()->AmIStageMaster())
		{
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageType", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", !bReady);
			if (m_nGameType != MMATCH_GAMETYPE_SPY)
				ZApplication::GetGameInterface()->EnableWidget("StageSettingCaller", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapType", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapRepeatCount", !bReady);
#ifdef _BOXLEAD
			ZApplication::GetGameInterface()->EnableWidget( "Lead", !bReady);
#endif
			//Custom: Lightmaps Time Code
			ZApplication::GetGameInterface()->EnableWidget("MapTime", !bReady);

#ifdef _LOBBYSET
			ZApplication::GetGameInterface()->EnableWidget("ExtraStageSettingCaller", !bReady);
			ZApplication::GetGameInterface()->EnableWidget("ExtraStageSettingApplyBtn", !bReady);
			ZApplication::GetGameInterface()->EnableWidget("StageSettingApplyBtn", !bReady);
			ZApplication::GetGameInterface()->EnableWidget("GravityValueComboBox", !bReady);
			ZApplication::GetGameInterface()->EnableWidget("JumpValueComboBox", !bReady);
			ZApplication::GetGameInterface()->EnableWidget("SpeedValueComboBox", !bReady);
			ZApplication::GetGameInterface()->EnableWidget("DMGTypeComboBox", !bReady);
			ZApplication::GetGameInterface()->EnableWidget("EspName", !bReady);
#endif

		}
		else
		{
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageType", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", false);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapType", false);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapRepeatCount", false);
#ifdef _BOXLEAD
			ZApplication::GetGameInterface()->EnableWidget("Lead", false);
#endif
			//Custom: Lightmaps Time Code
			ZApplication::GetGameInterface()->EnableWidget("MapTime", false);

#ifdef _LOBBYSET
			ZApplication::GetGameInterface()->EnableWidget("ExtraStageSettingCaller", !bReady);
			ZApplication::GetGameInterface()->EnableWidget("ExtraStageSettingApplyBtn", false);
			ZApplication::GetGameInterface()->EnableWidget("StageSettingApplyBtn", false);
			ZApplication::GetGameInterface()->EnableWidget("GravityValueComboBox", false);
			ZApplication::GetGameInterface()->EnableWidget("JumpValueComboBox", false);
			ZApplication::GetGameInterface()->EnableWidget("SpeedValueComboBox", false);
			ZApplication::GetGameInterface()->EnableWidget("DMGTypeComboBox", false);
			ZApplication::GetGameInterface()->EnableWidget("EspName", false);
#endif


		}
	}
    
	if ((m_nGameType == MMATCH_GAMETYPE_SURVIVAL) || (m_nGameType == MMATCH_GAMETYPE_QUEST) || (m_nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE))
	{
		ZApplication::GetGameInterface()->EnableWidget("StageMaxPlayer", false);
		//Custom: Lightmaps Time Code
		ZApplication::GetGameInterface()->EnableWidget("MapTime", false);
	}

	BEGIN_WIDGETLIST( "Stage_OptionFrame", pResource, MButton*, pButton);
	pButton->Enable( !bReady);
	END_WIDGETLIST();

	BEGIN_WIDGETLIST( "EquipmentCaller", pResource, MButton*, pButton);
	pButton->Enable( !bReady);
	END_WIDGETLIST();

	// Custom: Show shop in stage
	BEGIN_WIDGETLIST( "ShopCaller", pResource, MButton*, pButton);
	pButton->Enable( !bReady);
	END_WIDGETLIST();

	BEGIN_WIDGETLIST("MedalShopCaller", pResource, MButton*, pButton);
	pButton->Enable(!bReady);
	END_WIDGETLIST();

	BEGIN_WIDGETLIST("EventShopCaller", pResource, MButton*, pButton);
	pButton->Enable(!bReady);
	END_WIDGETLIST();

	ZApplication::GetGameInterface()->EnableWidget( "StagePlayerList_", !bReady);
}


/***********************************************************************
  SetMapName : public
  
  desc : ¸Ê ¼±ÅÃ ÄÞº¸¹Ú½º¿¡ ÇöÀE¼±ÅÃµÈ ¸Ê ÀÌ¸§À» ¼¼ÆÃÇÑ´Ù.
  arg  : szMapName = ¸Ê ÀÌ¸§
  ret  : none
************************************************************************/
void ZStageInterface::SetMapName( const char* szMapName)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if ( szMapName == NULL)
		return;

	MComboBox* pMapCombo = (MComboBox*)pResource->FindWidget( "MapSelection");
	if ( pMapCombo)
	{
		// ÀÏ´Ü ÀÓ½Ã ÇÏµåÄÚµE¿E¡¿¨~ ¤Ð.¤Ð)
//		if ( m_nGameType == MMATCH_GAMETYPE_QUEST)
//			pMapCombo->SetText( "Mansion");
//		else
		pMapCombo->SetText(szMapName);
	}

	//Custom: Lightmaps Time Code
	MComboBox* comboBox = (MComboBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("MapTime");
	if (comboBox != nullptr)
	{
		comboBox->RemoveAll();
		std::vector<string> lightMaps = MGetMapDescMgr()->GetLightMaps(ZGetGameClient()->GetMatchStageSetting()->GetMapIndex());
		for (int i = 0; i < lightMaps.size(); ++i)
		{
			if (lightMaps[i].length() == 0)
				continue;
			comboBox->Add(lightMaps[i].c_str());
		}
		comboBox->SetSelIndex(0);
	}

}

//Custom: Lightmaps Time Code
void ZStageInterface::SetMapTOD(int const& index)
{
	MComboBox* comboBox = (MComboBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("MapTime");
	if (comboBox != nullptr)
	{
		if (index > comboBox->GetCount())
			return;
		comboBox->SetSelIndex(index);
	}
}



/*
	¿©±â¼­ºÎÅÍ »õ·Î Ãß°¡µÈ ³»¿E..

	¿Ø¸¸ÇÑ°Ç ¸®½º³Ê³ª ´Ù¸¥ °÷¿¡¼­ ÀÚµ¿À¸·Î È£ÃâµÇµµ·Ï ÇØ³ùÀ¸³ª ¾ÆÁEÅ×½ºÆ®ÀÎ °Eè·Î
	¿Ïº®ÇÑ°Ç ¾Æ´Ô...  -_-;
*/



/***********************************************************************
  OpenSacrificeItemBox : public
  
  desc : Èñ»ý ¾ÆÀÌÅÛ ¼±ÅÃ Ã¢ ¿­±E
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OpenSacrificeItemBox(void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (!pResource) return;

	// 1. Luôn hiển thị khung chứa ô cúng đồ Quest
	MWidget* pItemListView = pResource->FindWidget("Stage_ItemListView");
	if (pItemListView)
	{
		pItemListView->Show(true);
	}

	// 2. Ép trạng thái cố định là Opened (2)
	m_nStateSacrificeItemBox = 2;

	// 3. Đặt vị trí cố định từ XML
	GetSacrificeItemBoxPos();
}


/***********************************************************************
  CloseSacrificeItemBox : public

  desc : Èñ»ý ¾ÆÀÌÅÛ ¼±ÅÃ Ã¢ ´Ý±E
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::CloseSacrificeItemBox(void)
{
	//ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	//MButton* pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxClose");
	//if ( pButton)
	//	pButton->Show( false);
	//pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxOpen");
	//if ( pButton)
	//	pButton->Show( true);

	//MWidget* pWidget = pResource->FindWidget( "Stage_CharacterInfo");
	//if ( pWidget)
	//	pWidget->Enable( true);

	//m_nStateSacrificeItemBox = 1;
	//GetSacrificeItemBoxPos();
	OpenSacrificeItemBox();
}


/***********************************************************************
  HideSacrificeItemBox : public

  desc : Èñ»ý ¾ÆÀÌÅÛ ¼±ÅÃ Ã¢ °¨Ãß±E
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::HideSacrificeItemBox(void)
{
	//ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	//MButton* pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxClose");
	//if ( pButton)
	//	pButton->Show( false);
	//pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxOpen");
	//if ( pButton)
	//	pButton->Show( true);

	//MWidget* pWidget = pResource->FindWidget( "Stage_CharacterInfo");
	//if ( pWidget)
	//	pWidget->Enable( true);

	//m_nStateSacrificeItemBox = 0;
	//GetSacrificeItemBoxPos();
	OpenSacrificeItemBox();
}


/***********************************************************************
  HideSacrificeItemBox : public

  desc : Èñ»ý ¾ÆÀÌÅÛ ¼±ÅÃ Ã¢ À§Ä¡ ±¸ÇÏ±E
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::GetSacrificeItemBoxPos(void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (!pResource) return;

	MWidget* pWidget = pResource->FindWidget("Stage_ItemListView");
	if (pWidget)
	{
		// 1. Lấy tọa độ & kích thước gốc ở 800x600 trong XML (X=241, Y=336)
		MRECT rc = pWidget->GetIDLRect();

		// 2. Lấy chiều rộng & chiều cao màn hình hiện tại
		int nCurrWidth = Mint::GetInstance()->GetWorkspaceWidth();     // Hoặc RGetMapViewWidth()
		int nCurrHeight = Mint::GetInstance()->GetWorkspaceHeight();   // Hoặc RGetMapViewHeight()

		// 3. Tính tỷ lệ Scale so với độ phân giải chuẩn 800x600
		float fScaleX = (float)nCurrWidth / 800.0f;
		float fScaleY = (float)nCurrHeight / 600.0f;

		// 4. Áp dụng tỷ lệ để tính tọa độ mới
		int nScaledX = (int)(rc.x * fScaleX);
		int nScaledY = (int)(rc.y * fScaleY);

		// Nếu muốn Scale luôn cả kích thước Width/Height cho vừa màn hình to:
		// int nScaledW = (int)(rc.w * fScaleX);
		// int nScaledH = (int)(rc.h * fScaleY);
		// pWidget->SetBounds(nScaledX, nScaledY, nScaledW, nScaledH);

		pWidget->SetPosition(nScaledX, nScaledY);
		m_nListFramePos = nScaledX;
	}

	m_nStateSacrificeItemBox = 2;
}


/***********************************************************************
  OnSacrificeItem0 : public
  
  desc : Èñ»ý ¾ÆÀÌÅÛ0À» ´­·¶À» ¶§
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnSacrificeItem0( void)
{
}


/***********************************************************************
  OnSacrificeItem1 : public
  
  desc : Èñ»ý ¾ÆÀÌÅÛ1¸¦ ´­·¶À» ¶§
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnSacrificeItem1( void)
{
}


/***********************************************************************
  UpdateSacrificeItem : protected
  
  desc : º¯°æµÈ Èñ»ý ¾ÆÀÌÅÛ ÀÌ¹ÌÁE Á¤º¸µûÜ» ¾÷µ¥ÀÌÆ® ÇÔ.
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::UpdateSacrificeItem( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// ½ºÅ×ÀÌÁE¿µ¿ª¿¡ ÀÖ´Â Èñ»ý ¾ÆÀÌÅÛ ÀÌ¹ÌÁE¼öÁ¤
	for ( int i = SACRIFICEITEM_SLOT0;  i <= SACRIFICEITEM_SLOT1;  i++)
	{
		char szWidgetNameItem[ 128];
		sprintf( szWidgetNameItem, "Stage_SacrificeItemImage%d", i);
		MPicture* pPicture = (MPicture*)pResource->FindWidget( szWidgetNameItem);
		if ( pPicture)
		{
			if ( m_SacrificeItem[ i].IsExist())
			{
				pPicture->SetBitmap( m_SacrificeItem[ i].GetIconBitmap());
				char szMsg[ 128];
				MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache( m_SacrificeItem[ i].GetUID());
				if ( pObjCache)
					sprintf( szMsg, "%s (%s)", m_SacrificeItem[ i].GetName(), pObjCache->GetName());
				else
					strcpy( szMsg, m_SacrificeItem[ i].GetName());
				pPicture->AttachToolTip( szMsg);
			}
			else
			{
				pPicture->SetBitmap( NULL);
				pPicture->DetachToolTip();
			}
		}
	}
}


/***********************************************************************
  SerializeSacrificeItemListBox : public
  
  desc : Èñ»ý ¾ÆÀÌÅÛ ¸®½ºÆ® ¹Ú½º¿¡ ÀÚ·á¸¦ ¹Þ´Â´Ù.
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::SerializeSacrificeItemListBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_SacrificeItemListbox");
	if ( !pListBox)
		return;

	int nStartIndex  = pListBox->GetStartItem();
	int nSelectIndex = pListBox->GetSelIndex();
	pListBox->RemoveAll();

	// ¸®½ºÆ®¿¡ Ãß°¡
	for ( MQUESTITEMNODEMAP::iterator questitem_itor = ZGetMyInfo()->GetItemList()->GetQuestItemMap().begin();
		  questitem_itor != ZGetMyInfo()->GetItemList()->GetQuestItemMap().end();
		  questitem_itor++)
	{
		ZMyQuestItemNode* pItemNode = (*questitem_itor).second;
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( pItemNode->GetItemID());
		if ( pItemDesc)
		{
			int nCount = pItemNode->m_nCount;
			if ( m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist() &&
				 (m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetUID() == ZGetGameClient()->GetPlayerUID()) &&
				 (pItemDesc->m_nItemID == m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetItemID()))
				nCount--;
			if ( m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist() &&
				 (m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetUID() == ZGetGameClient()->GetPlayerUID()) &&
				 (pItemDesc->m_nItemID == m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetItemID()))
				nCount--;

			if ( pItemDesc->m_bSecrifice && (nCount > 0))		// Èñ»ý ¾ÆÀÌÅÛ¸¸ Ãß°¡
			{
				pListBox->Add( new SacrificeItemListBoxItem( pItemDesc->m_nItemID,
															 ZApplication::GetGameInterface()->GetQuestItemIcon( pItemDesc->m_nItemID, true),
															 pItemDesc->m_szQuestItemName,
															 nCount,
															 pItemDesc->m_szDesc));
			}
		}
	}

	MWidget* pWidget = pResource->FindWidget( "Stage_NoItemLabel");
	if ( pWidget)
	{
		if ( pListBox->GetCount() > 0)
			pWidget->Show( false);
		else
			pWidget->Show( true);
	}

	pListBox->SetStartItem( nStartIndex);
	pListBox->SetSelIndex( min( (pListBox->GetCount() - 1), nSelectIndex));
}


/***********************************************************************
  OnDropSacrificeItem : public
  
  desc : Èñ»ý ¾ÆÀÌÅÛ ³õ±E
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnDropSacrificeItem( int nSlotNum)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pReadyBtn = (MButton*)pResource->FindWidget("StageReady");
	if(pReadyBtn) 
		if(pReadyBtn->GetCheck()) 
		{	// ÁØºE¿Ï·á¹öÆ°ÀÌ ´­·ÁÀÖ´Ù¸E
			ZApplication::GetStageInterface()->ChangeStageEnableReady(pReadyBtn->GetCheck());	// ´Ù¸¥ UI¸¦ ²¨ÁØ´Ù.
			return;	// Èñ»ý¾ÆÀÌÅÛÀ» ½½·Ô¿¡ ¿Ã¸®ÁE¾Ê´Â´Ù.
		}

	MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_SacrificeItemListbox");
	if ( !pListBox || (pListBox->GetSelIndex() < 0))
		return;

	SacrificeItemListBoxItem* pItemDesc = (SacrificeItemListBoxItem*)pListBox->Get( pListBox->GetSelIndex());
	if ( pItemDesc)
	{
		MTextArea* pDesc = (MTextArea*)pResource->FindWidget( "Stage_ItemDesc");

		// ½½·ÔÀÌ ºñ¾ûÜÖÀ¸¸E¹«Á¶°Ç ¿Ã¸²
		if ( ! m_SacrificeItem[ nSlotNum].IsExist())
		{
			ZPostRequestDropSacrificeItem( ZGetGameClient()->GetPlayerUID(), nSlotNum, pItemDesc->GetItemID());
			char szText[256];
			sprintf(szText, ZMsg( MSG_QUESTITEM_USE_DESCRIPTION ));
			pDesc->SetText(szText);
			//if ( pDesc)
			//	pDesc->Clear();
		}

		// ½½·ÔÀÌ ºñ¾ûÜÖÁE¾ÊÀ¸¸E..
		else
		{
			if ( (m_SacrificeItem[ nSlotNum].GetUID()    != ZGetGameClient()->GetPlayerUID()) ||
				 (m_SacrificeItem[ nSlotNum].GetItemID() != pItemDesc->GetItemID()))
				ZPostRequestDropSacrificeItem( ZGetGameClient()->GetPlayerUID(), nSlotNum, pItemDesc->GetItemID());

			char szText[256];
			sprintf(szText, ZMsg( MSG_QUESTITEM_USE_DESCRIPTION ));
			pDesc->SetText(szText);
			//if ( pDesc)
			//	pDesc->Clear();
		}
	}
}


/***********************************************************************
  OnRemoveSacrificeItem : public
  
  desc : Èñ»ý ¾ÆÀÌÅÛ »©±E
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnRemoveSacrificeItem( int nSlotNum)
{
	if ( !m_SacrificeItem[ nSlotNum].IsExist())
		return;

	ZPostRequestCallbackSacrificeItem( ZGetGameClient()->GetPlayerUID(),
									   nSlotNum,
									   m_SacrificeItem[ nSlotNum].GetItemID());

	MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
	char szText[256];
	sprintf(szText, ZMsg( MSG_QUESTITEM_USE_DESCRIPTION ));
	pDesc->SetText(szText);
	//if ( pDesc)
	//	pDesc->Clear();
}


/***********************************************************************
  MSacrificeItemListBoxListener
  
  desc : Èñ»ý ¾ÆÀÌÅÛ ¸®½ºÆ® ¹Ú½º ¸®½º³Ê
************************************************************************/
class MSacrificeItemListBoxListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		// On select
		if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL) == true)
		{
			MListBox* pListBox = (MListBox*)pWidget;

			// ¾ÆÀÌÅÛ ¼³¸E¾÷µ¥ÀÌÆ®
			SacrificeItemListBoxItem* pItemDesc = (SacrificeItemListBoxItem*)pListBox->GetSelItem();
			MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
			if ( pItemDesc && pDesc)
			{
				char szCount[ 128];
				sprintf( szCount, "%s : %d", ZMsg( MSG_WORD_QUANTITY), pItemDesc->GetItemCount());
				pDesc->SetTextColor( MCOLOR( 0xFFD0D0D0));
				pDesc->SetText( szCount);
				pDesc->AddText( "\n");
				pDesc->AddText( pItemDesc->GetItemDesc(), 0xFF808080);
			}

			return true;
		}


		// On double click
		else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
		{
			// Put item
			if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ 0].IsExist())
				ZApplication::GetStageInterface()->OnDropSacrificeItem( 0);
			else if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ 1].IsExist())
				ZApplication::GetStageInterface()->OnDropSacrificeItem( 1);
		
			return true;
		}

		return false;
	}
};

MSacrificeItemListBoxListener g_SacrificeItemListBoxListener;

MListener* ZGetSacrificeItemListBoxListener( void)
{
	return &g_SacrificeItemListBoxListener;
}


/***********************************************************************
  OnDropCallbackRemoveSacrificeItem
  
  desc : Èñ»ý ¾ÆÀÌÅÛ Á¦°Å
************************************************************************/
void OnDropCallbackRemoveSacrificeItem( void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	if ( (pSender == NULL) || (strcmp(pSender->GetClassName(), MINT_ITEMSLOTVIEW)))
		return;

	ZItemSlotView* pItemSlotView = (ZItemSlotView*)pSender;
	ZApplication::GetStageInterface()->OnRemoveSacrificeItem( (strcmp( pItemSlotView->m_szItemSlotPlace, "SACRIFICE0") == 0) ? 0 : 1);
}


// ¸±·¹ÀÌ¸Ê ÀÎÅÍÆäÀÌ½º
/***********************************************************************
OpenRelayMapBox : public

desc : ¸±·¹ÀÌ¸Ê ¼±ÅÃ Ã¢ ¿­±E
arg  : none
ret  : none
************************************************************************/

void ZStageInterface::OpenRelayMapBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxOpen");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxClose");
	if ( pButton)
		pButton->Show( true);

	SetRelayMapBoxPos(2);

	// ¸®ÇÃ·¹ÀÌ ¹Ú½º¿¡ ¸Ê¸®½ºÆ®¸¦ »ý¼º
	ZApplication::GetStageInterface()->RelayMapCreateMapList();
}

/***********************************************************************
CloseRelayMapBox : public

desc : ¸±·¹ÀÌ¸Ê ¼±ÅÃ Ã¢ ´Ý±E
arg  : none
ret  : none
************************************************************************/

void ZStageInterface::CloseRelayMapBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxClose");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxOpen");
	if ( pButton)
		pButton->Show( true);

	MWidget* pWidget = pResource->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

	SetRelayMapBoxPos(1);
}

/***********************************************************************
HideRelayMapBox : public

desc : ¸±·¹ÀÌ¸Ê ¼±ÅÃ Ã¢ °¨Ãß±E
arg  : none
ret  : none
************************************************************************/

void ZStageInterface::HideRelayMapBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxClose");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxOpen");
	if ( pButton)
		pButton->Show( true);

	MWidget* pWidget = pResource->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

	SetRelayMapBoxPos(0);
}

/***********************************************************************
GetRelayMapBoxPos : public

desc : ¸±·¹ÀÌ¸Ê ¼±ÅÃ Ã¢ À§Ä¡ ±¸ÇÏ±E
arg  : none
ret  : none
************************************************************************/

void ZStageInterface::SetRelayMapBoxPos( int nBoxPos)
{
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapListView");
	if ( pWidget)
	{
		MRECT rect;

		switch ( nBoxPos)
		{
		case 0 :		// Hide
			rect = pWidget->GetRect();
			m_nRelayMapListFramePos = -rect.w;
			break;

		case 1 :		// Close
			rect = pWidget->GetRect();
			m_nRelayMapListFramePos = -rect.w + ( rect.w * 0.14);
			break;

		case 2 :		// Open
			m_nRelayMapListFramePos = 0;
			break;
		}
	}
}

/***********************************************************************
PostRelayMapType : public

desc : ¸±·¹ÀÌ¸Ê Å¸ÀÔ ¾÷µ¥ÀÌÆ® ¹× ¼­¹ö¿¡ ÀEÛ
arg  : none
ret  : none
************************************************************************/
void ZStageInterface::PostRelayMapElementUpdate( void)
{
	MComboBox* pCBRelayMapType = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapType" );
	if ( !pCBRelayMapType) return;
	MComboBox* pCBRelayMapTurnCount = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapRepeatCount" );
	if ( !pCBRelayMapTurnCount) return;
	ZPostStageRelayMapElementUpdate(ZGetGameClient()->GetStageUID(), pCBRelayMapType->GetSelIndex(), pCBRelayMapTurnCount->GetSelIndex());
}

/***********************************************************************
PostRelayMapListUpdate : public

desc : ¸±·¹ÀÌ¸Ê ¸®½ºÆ® ¾÷µ¥ÀÌÆ®(¼­¹ö¿¡µµ ¸®½ºÆ®¸¦ º¸³½´Ù.)
arg  : none
ret  : none
************************************************************************/
void ZStageInterface::PostRelayMapInfoUpdate( void)
{
	MComboBox* pCBRelayMapType = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapType" );
	if ( !pCBRelayMapType) return;
	MComboBox* pCBRelayMapRepeatCount = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapRepeatCount" );
	if ( !pCBRelayMapRepeatCount) return;

	MListBox* pRelayMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapListbox");
	if(pRelayMapListBox == NULL) return;

	if(pRelayMapListBox->GetCount() <= 0)	
	{
		ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_RELAYMAP_ONE_OR_MORE_MAP_SELECT);
		return;
	}
	
	//¸±·¹ÀÌ¸Ê ¸®½ºÆ® ÀEÛ
	void* pMapArray = MMakeBlobArray(sizeof(MTD_RelayMap), pRelayMapListBox->GetCount());
	int nMakeBlobCnt = 0;
	for(int i=0; i<pRelayMapListBox->GetCount(); i++)
	{
		MTD_RelayMap* pRelayMapNode = (MTD_RelayMap*)MGetBlobArrayElement(pMapArray, nMakeBlobCnt);
		for (int j = 0; j < MMATCH_MAP_COUNT; j++)
		{
			if(0 == strcmp(pRelayMapListBox->GetString(i), (char*)MGetMapDescMgr()->GetMapName(j)))
			{
				pRelayMapNode->nMapID = j;
				break;
			}
		}
		++nMakeBlobCnt;
	}

	ZPostStageRelayMapInfoUpdate(ZGetGameClient()->GetStageUID(), pCBRelayMapType->GetSelIndex(), pCBRelayMapRepeatCount->GetSelIndex(), pMapArray);
	MEraseBlobArray(pMapArray);

	ZApplication::GetStageInterface()->SetIsRelayMapRegisterComplete(true);
	ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", false);
}

/***********************************************************************
PostRelayMapList : public

desc : ¸±·¹ÀÌ¸Ê ¼±ÅÃÀ¸·Î ¸Ê ÄÞº¸¹Ú½º ¸®½ºÆ®¸¦ ¸¸µé¾EÁØ´Ù.
arg  : none
ret  : none
************************************************************************/
void ZStageInterface::RelayMapCreateMapList()
{
	MComboBox* pCombo = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("MapSelection");
	if(pCombo == NULL) return;

	// ¸Ê ¸®½ºÆ® ¸¸µé¾EÁÖ±E
	MListBox* pMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_MapListbox");
	if(pMapListBox == NULL) return;

	pMapListBox->RemoveAll();	// ±âÁ¸ ¸±·¹ÀÌ¸Ê ¸®½ºÆ®¸¦ ¸ðµÎ Áö¿öÁØ´Ù.
	for( int i = 0 ; i < pCombo->GetCount(); ++i )
	{
		if(strcmp(MMATCH_MAPNAME_RELAYMAP, pCombo->GetString(i)) == 0)
			continue;
		RelayMapList* pRelayMapList = new RelayMapList( pCombo->GetString(i), MBitmapManager::Get( "Mark_Arrow.bmp"));
		pMapListBox->Add( pRelayMapList);
	}


	// ¹æÀåÀÌ ¸±·¹ÀÌ¸Ê ¼¼ÆÃÁßÀÌ¸E¸±·¹ÀÌ¸Ê ¸®½ºÆ®¸¦ ¾÷µ¥ÀÌÆ® ÇÏÁE¾Ê´Â´Ù.
	if(!ZApplication::GetStageInterface()->GetIsRelayMapRegisterComplete())
		return;

	// ¸±·¹ÀÌ¸Ê ¸®½ºÆ® ¸¸µé¾EÁÖ±E
	MListBox* pRelaMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapListbox");
	if(pRelaMapListBox == NULL) return;
	RelayMap arrayRelayMapList[MAX_RELAYMAP_LIST_COUNT];
	memcpy(arrayRelayMapList, ZGetGameClient()->GetMatchStageSetting()->GetRelayMapList(), sizeof(RelayMap)*MAX_RELAYMAP_LIST_COUNT);
	
	pRelaMapListBox->RemoveAll();	// ±âÁ¸ ¸±·¹ÀÌ¸Ê ¸®½ºÆ®¸¦ ¸ðµÎ Áö¿öÁØ´Ù.
	for( int i = 0 ; i < ZGetGameClient()->GetMatchStageSetting()->GetRelayMapListCount(); ++i )
	{// ¸±·¹ÀÌ¸Ê ¸®½ºÆ®¿¡ µ¥ÀÌÅÍ¸¦ Ãß°¡ÇØÁØ´Ù.
		int nMapID = arrayRelayMapList[i].nMapID;
		RelayMapList* pRelayMapList = new RelayMapList( MGetMapDescMgr()->GetMapName(MGetMapDescMgr()->GetMapID(nMapID)), MBitmapManager::Get( "Mark_X.bmp"));
		pRelaMapListBox->Add( pRelayMapList);
	}
}

/***********************************************************************
MMapListBoxListener

desc : ¸Ê ¸®½ºÆ® ¹Ú½º ¸®½º³Ê
************************************************************************/
class MMapListBoxListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		// ½ºÅ×ÀÌÁE¸¶½ºÅÍ°¡ ¾Æ´Ï¸E¸Ê ¸®½ºÆ®¸¦ ÄÁÆ®·Ñ ÇÒ¼ö¾ø´Ù.
		if(!ZGetGameClient()->AmIStageMaster())
			return false;
		// On select
		if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL) == true)
		{
			// ¸Ê¸®½ºÆ®¿¡¼­ ¼±ÅÃµÈ ¸ÊÁ¤º¸¸¦ ¸±·¹ÀÌ¸Ê ¸®½ºÆ®¿¡ Ãß°¡ÇÑ´Ù.
			MListBox* pMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_MapListbox");
			if(pMapListBox == NULL) return false;
			MListBox* pRelayMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapListbox");
			if(pRelayMapListBox == NULL) return false;
			if(MAX_RELAYMAP_LIST_COUNT <= pRelayMapListBox->GetCount()) 
			{
				ZApplication::GetGameInterface()->ShowMessage( MSG_GAME_RELAYMAP_TOO_MANY_LIST_COUNT );
				return false;
			}

			RelayMapList* pMapList = (RelayMapList*)pMapListBox->GetSelItem();
			char szMapName[ MAPNAME_LENGTH ];
			strcpy(szMapName, pMapList->GetString());
			RelayMapList* pRelayMapList = new RelayMapList( szMapName, MBitmapManager::Get( "Mark_X.bmp"));

			pRelayMapListBox->Add( pRelayMapList);
			pRelayMapListBox->ShowItem(pRelayMapListBox->GetCount());	// ½ºÅ©·Ñ À§Ä¡ ¼¼ÆÃ

			// ¸±·¹ÀÌ ¸Ê ¼öÁ¤ ½ÃÀÛ
			ZApplication::GetStageInterface()->SetIsRelayMapRegisterComplete(false);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", true);

			return true;
		}

		// On double click
		else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
		{
			return true;
		}

		return false;
	}
};
MMapListBoxListener g_MapListBoxListener;
MListener* ZGetMapListBoxListener( void)
{
	return &g_MapListBoxListener;
}

/***********************************************************************
MRelayMapListBoxListener

desc : ¸±·¹ÀÌ¸Ê ¸®½ºÆ® ¹Ú½º ¸®½º³Ê
************************************************************************/
class MRelayMapListBoxListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		// ½ºÅ×ÀÌÁE¸¶½ºÅÍ°¡ ¾Æ´Ï¸E¸±·¹ÀÌ¸Ê ¸®½ºÆ®¸¦ ÄÁÆ®·Ñ ÇÒ¼ö¾ø´Ù.
		if(!ZGetGameClient()->AmIStageMaster())
			return false;
		// On select
		if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL) == true)
		{
			MListBox* pRelayMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapListbox");
			if(pRelayMapListBox == NULL) return false;
			if(pRelayMapListBox->GetCount() > 1)
			{	// ¸ÊÀÌ ÃÖ¼ÒÇÑ ÇÑ°³¶óµµ ÀÖ¾ûÚß ÇÑ´Ù.
				pRelayMapListBox->Remove(pRelayMapListBox->GetSelIndex());

				// ¸±·¹ÀÌ¸Ê ¸®½ºÆ® ½ºÅ©·Ñ À§Ä¡ ¼¼ÆÃ
				if(pRelayMapListBox->GetCount() <= pRelayMapListBox->GetShowItemCount())
					pRelayMapListBox->ShowItem(0);
				else
					pRelayMapListBox->SetStartItem(pRelayMapListBox->GetCount());
				
				// ¸±·¹ÀÌ ¸Ê ¼öÁ¤ ½ÃÀÛ
				ZApplication::GetStageInterface()->SetIsRelayMapRegisterComplete(false);
				ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", true);

				return true;
			}
			else
			{
				ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_RELAYMAP_ONE_OR_MORE_MAP_SELECT);
				return false;
			}
		}

		// On double click
		else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
		{
			return true;
		}
		return false;
	}
};
MRelayMapListBoxListener g_RelayMapListBoxListener;
MListener* ZGetRelayMapListBoxListener( void)
{
	return &g_RelayMapListBoxListener;
}

/***********************************************************************
  StartMovieOfQuest : public
  
  desc : Äù½ºÆ® ¸ðµå·Î ½ÃÀÛÇÒ¶§ ¾ÆÀÌÅÛ ÇÕÃÄÁö´Â ¹«ºñ¸¦ ½ÃÀÛÇÔ
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::StartMovieOfQuest( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	m_dwClockOfStartMovie = timeGetTime();

	// È­¿° ¾Ö´Ï¸ÞÀÌ¼Ç ½ÃÀÛ
	MAnimation* pAnimation = (MAnimation*)pResource->FindWidget( "Stage_Flame0");
	if ( pAnimation && m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist())
	{
		pAnimation->SetCurrentFrame( 0);
		pAnimation->Show( true);
		pAnimation->SetRunAnimation( true);
	}
	pAnimation = (MAnimation*)pResource->FindWidget( "Stage_Flame1");
	if ( pAnimation && m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist())
	{
		pAnimation->SetCurrentFrame( 0);
		pAnimation->Show( true);
		pAnimation->SetRunAnimation( true);
	}

	m_bDrawStartMovieOfQuest = true;
}


/***********************************************************************
  OnDrawStartMovieOfQuest : public
  
  desc : Äù½ºÆ® ¸ðµå·Î ½ÃÀÛÇÒ¶§ ¾ÆÀÌÅÛ ÇÕÃÄÁö´Â ¹«ºñ¸¦ ±×¸²
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnDrawStartMovieOfQuest( void)
{
	if ( !m_bDrawStartMovieOfQuest)
		return ;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// °æ°E½Ã°£À» ±¸ÇÑ´Ù.
	DWORD dwClock = timeGetTime() - m_dwClockOfStartMovie;

	// Èñ»ý ¾ÆÀÌÅÛ ÆäÀÌµE¾Æ¿E
	int nOpacity = 255 - dwClock * 0.12f;
	if ( nOpacity < 0)
		nOpacity = 0;

	MPicture* pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage0");
	if ( pPicture && m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist())
		pPicture->SetOpacity( nOpacity);

	pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage1");
	if ( pPicture && m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist())
		pPicture->SetOpacity( nOpacity);

	// Á¾·E½Ã°£ÀÏ °æ¿E¡...
	if ( dwClock > 3200)
	{
		m_bDrawStartMovieOfQuest = false;

		ZMyQuestItemMap::iterator itMyQItem;

		// ¹ö±×ÇØ°E.. Äù½ºÆ® °ÔÀÓ½ÃÀÛÀ» ´©¸£´Â ¼ø°£ ÇØ´EÄù½ºÆ® ¾ÆÀÌÅÛÀÌ °¨¼ÒµÈ »óÅÂ(¿Å¹Ù¸¥)ÀÇ °¹¼ö¸¦ 
		// DB¿¡¼­ ¹Þ¾Æ¿À±â¶§¹®¿¡ Ä«¿ûâ® °¨¼Ò¸¦ ÇÏ¸EÀß¸øµÈ°ªÀÌ µÈ´Ù....20090318 by kammir
		// ¿©±â¼­ ½½·Ô¿¡ ÀÚ½ÅÀÇ ¾ÆÀÌÅÛÀÌ ¿Ã·ÁÁ® ÀÖÀ¸¸EÇØ´E¾ÆÀÌÅÛ Ä«¿ûâ® °¨¼Ò.
		//if( ZGetGameClient()->GetUID() == m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetUID() )
		//{
		//	itMyQItem = ZGetMyInfo()->GetItemList()->GetQuestItemMap().find( m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetItemID() );
		//	itMyQItem->second->Decrease();
		//}
		//if( ZGetGameClient()->GetUID() == m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetUID() )
		//{
		//	itMyQItem = ZGetMyInfo()->GetItemList()->GetQuestItemMap().find( m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetItemID() );
		//	itMyQItem->second->Decrease();
		//}
		
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

		ZApplication::GetGameInterface()->SetState( GUNZ_GAME);
	}
}


/***********************************************************************
  IsShowStartMovieOfQuest : public
  
  desc : Äù½ºÆ® ¸ðµå·Î ½ÃÀÛÇÒ¶§ ¾ÆÀÌÅÛ ÇÕÃÄÁö´Â ¹«ºñ¸¦ º¸¿©ÁÙÁE¿©ºÎ¸¦ °áÁ¤.
  arg  : none
  ret  : true(=Quest mode start movie) or false(=none)
************************************************************************/
bool ZStageInterface::IsShowStartMovieOfQuest( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if ( m_nGameType == MMATCH_GAMETYPE_QUEST)
	{
		if ( m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist() || m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist())
			return true;
	}

	return false;
}


/***********************************************************************
  OnResponseDropSacrificeItemOnSlot : public
  
  desc : Èñ»ý ¾ÆÀÌÅÛÀÌ ¿Ã¶ó°¬À»¶§
  arg  : none
  ret  : none
************************************************************************/
bool ZStageInterface::OnResponseDropSacrificeItemOnSlot( const int nResult, const MUID& uidRequester, const int nSlotIndex, const int nItemID )
{
#ifdef _QUEST_ITEM
	if( MOK == nResult)
	{
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID);
		MBitmap* pIconBitmap = ZApplication::GetGameInterface()->GetQuestItemIcon( nItemID, false);

		m_SacrificeItem[ nSlotIndex].SetSacrificeItemSlot( uidRequester, nItemID, pIconBitmap, pItemDesc->m_szQuestItemName, pItemDesc->m_nLevel);
		SerializeSacrificeItemListBox();

		UpdateSacrificeItem();
	}
	else if( ITEM_TYPE_NOT_SACRIFICE == nResult)
	{
		// Èñ»ý ¾ÆÀÌÅÛÀÌ ¾Æ´Ô.
		return false;
	}
	else if( NEED_MORE_QUEST_ITEM == nResult )
	{
		// ÇöÁ¦ °¡Áö°EÀÖ´Â ¼ö·®À» ÃÊ°úÇØ¼­ ¿Ã·Á ³õÀ¸·Á°EÇßÀ»°æ¿E
	}
	else if( MOK != nResult )
	{
		// ½ÇÆÐ...
		return false;
	}
	else
	{
		// Á¤ÀÇµÇÁE¾ÊÀº error...
		ASSERT( 0 );
	}

#endif

	return true;
}


/***********************************************************************
  OnResponseCallbackSacrificeItem : public
  
  desc : Èñ»ý ¾ÆÀÌÅÛÀÌ ³»·Á°¬À»¶§
  arg  : none
  ret  : none
************************************************************************/
bool ZStageInterface::OnResponseCallbackSacrificeItem( const int nResult, const MUID& uidRequester, const int nSlotIndex, const int nItemID )
{
#ifdef _QUEST_ITEM
	if( MOK == nResult )
	{
		m_SacrificeItem[ nSlotIndex].RemoveItem();
		SerializeSacrificeItemListBox();

		UpdateSacrificeItem();

		MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
		char szText[256];
		sprintf(szText, ZMsg( MSG_QUESTITEM_USE_DESCRIPTION ));
		pDesc->SetText(szText);
		//if ( pDesc)
		//	pDesc->Clear();
	}
	else if( ERR_SACRIFICE_ITEM_INFO == nResult )
	{
		// Å¬¶óÀÌ¾ðÆ®¿¡¼­ º¸³½ Á¤º¸°¡ Àß¸øµÈ Á¤º¸. µû·¯ ¿¡·¯Ã³¸®°¡ ÇÊ¿äÇÏ¸E¿©±â¼­ ÇØÁÖ¸EµÊ.
	}

#endif

	return true;
}

#ifdef _QUEST_ITEM
///
// Fist : Ãß±³¼º.
// Last : Ãß±³¼º.
// 
// ¼­¹ö·ÎºÎÅÍ QLÀÇ Á¤º¸¸¦ ¹ÞÀ½.
///
bool ZStageInterface::OnResponseQL( const int nQL )
{
	ZGetQuest()->GetGameInfo()->SetQuestLevel( nQL);

	// ½ºÅ×ÀÌÁE¿µ¿ª¿¡ ÀÖ´Â Äù½ºÆ® ·¹º§ Ç¥½Ã ¼öÁ¤
	MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_QuestLevel");
	if ( pLabel)
	{
		char szText[125];
		sprintf( szText, "%s %s : %d", ZMsg( MSG_WORD_QUEST), ZMsg( MSG_CHARINFO_LEVEL), nQL);
		pLabel->SetText( szText);
	}

	return true;
}

bool ZStageInterface::OnStageGameInfo( const int nQL, const int nMapsetID, const unsigned int nScenarioID )
{
	if (nScenarioID != 0)
	{
		ZGetQuest()->GetGameInfo()->SetQuestLevel( nQL );
	}
	else
	{
		// ½Ã³ª¸®¿À°¡ ¾øÀ¸¸E±×³É 0À¸·Î º¸ÀÌ°Ô ÇÑ´Ù.
		ZGetQuest()->GetGameInfo()->SetQuestLevel( 0 );
	}

	ZGetQuest()->GetGameInfo()->SetMapsetID( nMapsetID );
	ZGetQuest()->GetGameInfo()->SetSenarioID( nScenarioID );

	UpdateStageGameInfo(nQL, nMapsetID, nScenarioID);

	return true;
}

bool ZStageInterface::OnResponseSacrificeSlotInfo( const MUID& uidOwner1, const unsigned long int nItemID1, 
												   const MUID& uidOwner2, const unsigned long int nItemID2 )
{
	if ( (uidOwner1 != MUID(0,0)) && nItemID1)
	{
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID1);
		MBitmap* pIconBitmap = ZApplication::GetGameInterface()->GetQuestItemIcon( nItemID1, false);
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].SetSacrificeItemSlot( uidOwner1, nItemID1, pIconBitmap, pItemDesc->m_szQuestItemName, pItemDesc->m_nLevel);
	}
	else
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();

	if ( (uidOwner2 != MUID(0,0)) && nItemID2)
	{
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID2);
		MBitmap* pIconBitmap = ZApplication::GetGameInterface()->GetQuestItemIcon( nItemID2, false);
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].SetSacrificeItemSlot( uidOwner2, nItemID2, pIconBitmap, pItemDesc->m_szQuestItemName, pItemDesc->m_nLevel);
	}
	else
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

	UpdateSacrificeItem();

	MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
	//if ( pDesc)
	//	pDesc->Clear();

	return true;
}


bool ZStageInterface::OnQuestStartFailed( const int nState )
{
	MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageChattingOutput");
	if ( pTextArea)
	{
		char text[256];
		sprintf(text, "%s", ZMsg(MSG_GANE_NO_QUEST_SCENARIO));
		pTextArea->AddText( text);
	}

/*
	if( MSQITRES_INV == nState )
	{
		// ÇØ´EQL¿¡´EÑ Èñ»ý¾ÆÀÌÅÛ Á¤º¸ Å×ÀÌºúÜÌ ¾øÀ½. ÀÌ°æ¿EÂ ¸ÂÁE¾Ê´Â Èñ»ý ¾ÆÀÌÅÛÀÌ ¿Ã·ÁÁ® ÀÖÀ»°æ¿E
		MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageChattingOutput");
		if ( pTextArea)
			pTextArea->AddText( "^1ÇöÀE³õ¿©ÀÖ´Â ¾ÆÀÌÅÛÀº Á¶°Ç¿¡ ¸ÂÁE¾Ê¾Æ °ÔÀÓÀ» ½ÃÀÛÇÒ ¼E¾ø½À´Ï´Ù.");
	}
	else if( MSQITRES_DUP == nState )
	{
		// ¾çÂÊ ½½·Ô¿¡ °°Àº ¾ÆÀÌÅÛÀÌ ¿Ã·ÁÁ® ÀÖÀ½.
		MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageChattingOutput");
		if ( pTextArea)
			pTextArea->AddText( "^1°°Àº ¾ÆÀÌÅÛ 2°³°¡ ³õ¿©ÀÖÀ¸¹Ç·Î °ÔÀÓÀ» ½ÃÀÛÇÒ ¼E¾ø½À´Ï´Ù.");
	}
*/
	return true;
}


bool ZStageInterface::OnNotAllReady()
{
	return true;
}
#endif

void ZStageInterface::UpdateStageGameInfo(const int nQL, const int nMapsetID, const int nScenarioID)
{
	if (!ZGetGameTypeManager()->IsQuestOnly(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) return;

	// ½ºÅ×ÀÌÁE¿µ¿ª¿¡ ÀÖ´Â Äù½ºÆ® ·¹º§ Ç¥½Ã ¼öÁ¤
	MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_QuestLevel");
	if ( pLabel)
	{
		char szText[125];
		sprintf( szText, "%s %s : %d", ZMsg( MSG_WORD_QUEST), ZMsg( MSG_CHARINFO_LEVEL), nQL);
		pLabel->SetText( szText);
	}


#define		MAPSET_NORMAL		MCOLOR(0xFFFFFFFF)
#define		MAPSET_SPECIAL		MCOLOR(0xFFFFFF40)			// Green
//#define		MAPSET_SPECIAL		MCOLOR(0xFFFF2020)		// Red

	// ¿©±â¼­ ½Ã³ª¸®¿À ÀÌ¸§À» º¸¿©ÁØ´Ù.
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_SenarioName");
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_SenarioNameImg");
	MPicture* pPictureL = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_Lights0");
	MPicture* pPictureR = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_Lights1");
	if ( pLabel)
	{
		if (nScenarioID == 0)
		{
			// ½Ã³ª¸®¿À°¡ ¾ø´Â °æ¿E
			pLabel->SetText( "");
			if ( pWidget)
				pWidget->Show( false);
			if ( pPictureL) {
				pPictureL->Show( false);
			}
			if ( pPictureR) {
				pPictureR->Show( false);
			}
		}
		else
		{
			pLabel->SetAlignment( MAM_HCENTER | MAM_VCENTER);

			LIST_SCENARIONAME::iterator itr =  m_SenarioDesc.find( nScenarioID);
			if ( itr != m_SenarioDesc.end())
			{
				pLabel->SetText( (*itr).second.m_szName);
				pLabel->SetTextColor(MCOLOR(0xFFFFFF00));
				if ( pWidget)
					pWidget->Show( true);

				if ( pPictureL) {
					pPictureL->Show( true);
					pPictureL->SetBitmapColor(MAPSET_SPECIAL);
				}
				if ( pPictureR) {
					pPictureR->Show( true);
					pPictureR->SetBitmapColor(MAPSET_SPECIAL);
				}
			}
			else
			{
				// Æ¯º°½Ã³ª¸®¿À°¡ ¾øÀ»°æ¿EÂ Á¤±Ô½Ã³ª¸®¿ÀÀÌ´Ù.
				pLabel->SetText("");
				pLabel->SetTextColor(MCOLOR(0xFFFFFFFF));
				if ( pWidget)
					pWidget->Show( false);

				if ( pPictureL) {
					pPictureL->Show( true);
					pPictureL->SetBitmapColor(MAPSET_NORMAL);
				}
				if ( pPictureR) {
					pPictureR->Show( true);
					pPictureR->SetBitmapColor(MAPSET_NORMAL);
				}

			}
		}
	}
}

/***********************************************************************
  SetSacrificeItemSlot : public
  
  desc : Èñ»ý ¾ÆÀÌÅÛ ½½·Ô¿¡ ¾ÆÀÌÅÛ Á¤º¸¸¦ ÀÔ·Â
  arg  : none
  ret  : none
************************************************************************/
void SacrificeItemSlotDesc::SetSacrificeItemSlot( const MUID& uidUserID, const unsigned long int nItemID, MBitmap* pBitmap, const char* szItemName, const int nQL)
{
	m_uidUserID = uidUserID;
	m_nItemID = nItemID;
	m_pIconBitmap = pBitmap;
	strcpy( m_szItemName, szItemName);
	m_nQL = nQL;
	m_bExist = true;
}


/***********************************************************************
  ReadSenarioNameXML : protected
  
  desc : Äù½ºÆ® Èñ»ý ¾ÆÀÌÅÛ XMLÀ» ÀÐ´Â´Ù
  arg  : none
  ret  : true(=success) or false(=fail)
************************************************************************/
bool ZStageInterface::ReadSenarioNameXML( void)
{
	if ( (int)m_SenarioDesc.size())
		return true;


	// XML ÆÄÀÏÀ» ¿¬´Ù
	MXmlDocument xmlQuestItemDesc;
	xmlQuestItemDesc.Create();

	char			*buffer;
	MZFile			mzFile;

	string strFileScenario("System/scenario.xml");
#ifndef _DEBUG
	strFileScenario += ""; // MEF NULL
#endif
	if( !mzFile.Open(strFileScenario.c_str(), ZApplication::GetFileSystem())) 
	{
		mlog("Error while read : %s", strFileScenario.c_str());
		xmlQuestItemDesc.Destroy();
		return false;
	} 

	buffer = new char[ mzFile.GetLength() + 1];
	buffer[ mzFile.GetLength()] = 0;

	mzFile.Read( buffer, mzFile.GetLength());

	if( !xmlQuestItemDesc.LoadFromMemory( buffer))
	{
		delete[] buffer;
		xmlQuestItemDesc.Destroy();
		return false;
	}
	delete[] buffer;
	mzFile.Close();

	int nStdScenarioCount = 1000;

	// µ¥ÀÌÅÍ¸¦ ÀÐ¾ûÛÂ´Ù
	MXmlElement rootElement = xmlQuestItemDesc.GetDocumentElement();
	for ( int i = 0;  i < rootElement.GetChildNodeCount();  i++)
	{
		MXmlElement chrElement = rootElement.GetChildNode( i);

		char szTagName[ 256];
		chrElement.GetTagName( szTagName);

		if ( szTagName[ 0] == '#')
			continue;


		char szAttrName[64];
		char szAttrValue[256];
		int nItemID = 0;
		MSenarioList SenarioMapList;

		bool bFind = false;


		// Á¤±Ô ½Ã³ª¸®¿À
		if ( !stricmp( szTagName, "STANDARD_SCENARIO"))			// ÅÂ±× ½ÃÀÛ
		{
			// Set Tag
			for ( int k = 0;  k < chrElement.GetAttributeCount();  k++)
			{
				chrElement.GetAttribute( k, szAttrName, szAttrValue);

				SenarioMapList.m_ScenarioType = ST_STANDARD;	// Type

				if ( !stricmp( szAttrName, "title"))			// Title
					strcpy( SenarioMapList.m_szName, szAttrValue);

				else if ( !stricmp( szAttrName, "mapset"))		// Map set
					strcpy( SenarioMapList.m_szMapSet, szAttrValue);
			}

			nItemID = nStdScenarioCount++;					// ID

			bFind = true;
		}

		// Æ¯¼E½Ã³ª¸®¿À
		else if ( !stricmp( szTagName, "SPECIAL_SCENARIO"))			// ÅÂ±× ½ÃÀÛ
		{
			// Set Tag
			for ( int k = 0;  k < chrElement.GetAttributeCount();  k++)
			{
				chrElement.GetAttribute( k, szAttrName, szAttrValue);

				SenarioMapList.m_ScenarioType = ST_SPECIAL;		// Type

				if ( !stricmp( szAttrName, "id"))				// ID
					nItemID = atoi( szAttrValue);

				else if ( !stricmp( szAttrName, "title"))		// Title
					strcpy( SenarioMapList.m_szName, szAttrValue);

				else if ( !stricmp( szAttrName, "mapset"))		// Map set
					strcpy( SenarioMapList.m_szMapSet, szAttrValue);
			}

			bFind = true;
		}


		if ( bFind)
			m_SenarioDesc.insert( LIST_SCENARIONAME::value_type( nItemID, SenarioMapList));
	}

	xmlQuestItemDesc.Destroy();


	return true;
}

bool ZStageInterface::OnStopVote()
{
	ZGetGameClient()->SetVoteInProgress( false );
	ZGetGameClient()->SetCanVote( false );

#ifdef _DEBUG
	string str = ZMsg(MSG_VOTE_VOTE_STOP);
#endif

	ZChatOutput(ZMsg(MSG_VOTE_VOTE_STOP), ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	return true;
}


void ZStageInterface::OnStartFail( const int nType, const MUID& uidParam )
{
	if( ALL_PLAYER_NOT_READY == nType )
	{
		// ¸ðµEÀ¯Àú°¡ ·¹µð¸¦ ÇÏÁE¾Ê¾ÒÀ½.
		ZGetGameInterface()->PlayVoiceSound( VOICE_PLAYER_NOT_READY, 1500);
	}
	else if( QUEST_START_FAILED_BY_SACRIFICE_SLOT == nType )
	{
		OnQuestStartFailed( uidParam.Low );

		ZGetGameInterface()->PlayVoiceSound( VOICE_QUEST_START_FAIL, 2800);
	}
	else if( INVALID_TACKET_USER == nType )
	{
		// ÀÚ±EÀÚ½ÅÀÌ¸E-_-;;
		//if( uidParam == ZGetMyUID() )
		//{
			char szMsg[ 128 ];

			sprintf( szMsg, MGetStringResManager()->GetErrorStr(MERR_CANNOT_START_NEED_TICKET), ZGetGameClient()->GetObjName(uidParam).c_str() );

			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			ZChatOutput(szMsg, ZChat::CMT_BROADCAST);
		//}

		// ÀÔÀå±Ç ´©°¡ ¾ø´Â°Å¾ß? -_-+ - by SungE
		//const MMatchPeerInfo* pPeer = ZGetGameClient()->GetPeers()->Find( uidParam );
		//if( 0 != pPeer )
		//{
		//	char szMsg[ 128 ];

		//	sprintf( szMsg, MGetStringResManager()->GetErrorStr(MERR_CANNOT_START_NEED_TICKET), pPeer->CharInfo.szName );

		//	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		//	ZChatOutput(szMsg, ZChat::CMT_BROADCAST);
		//}
	}
	else if( INVALID_MAP == nType )
	{
		char szMsg[ 128 ];

			sprintf( szMsg, "INVALID MAP!" );
				//MGetStringResManager()->GetErrorStr(MERR_CANNOT_START_NEED_TICKET), 
				//ZApplication::GetInstance()->GetGameClient()->GetObjName(uidParam).c_str() );

			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			ZChatOutput(szMsg, ZChat::CMT_BROADCAST);
	}

	// Stage UI Enable
	ChangeStageEnableReady( false);
}

////////////////////////////////// Spy Mode. //////////////////////////////////
class MSpyBanMapListListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		// ½ºÅ×ÀÌÁE¸¶½ºÅÍ°¡ ¾Æ´Ï¸E¸Ê ¸®½ºÆ®¸¦ ÄÁÆ®·Ñ ÇÒ¼ö¾ø´Ù.
		if (!ZGetGameClient()->AmIStageMaster())
			return false;
		// On select
		if (MWidget::IsMsg(szMessage, MLB_ITEM_SEL) == true)
		{
			MListBox* pListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_SpyBanMapList");
			if (!pListBox) return false;

			SpyBanMapList* pSpyBanMapList = (SpyBanMapList*)pListBox->GetSelItem();
			if (!MMatchSpyMap::IsExistingMap(pSpyBanMapList->GetMapID())) return false;

			ZPostActivateSpyMap(pSpyBanMapList->GetMapID(), ZGetGameClient()->GetMatchStageSetting()->IsIncludedSpyMap(pSpyBanMapList->GetMapID()));
			return true;
		}

		return false;
	}
};
MSpyBanMapListListener g_SpyBanMapListListener;
MListener* ZGetSpyBanMapListListener(void)
{
	return &g_SpyBanMapListListener;
}

///////////////////////////////////////////////////////

void ZStageInterface::OpenSpyBanMapBox()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget("Stage_SpyBanMapBoxOpen");
	if (pButton)
		pButton->Show(false);
	pButton = (MButton*)pResource->FindWidget("Stage_SpyBanMapBoxClose");
	if (pButton)
		pButton->Show(true);

	SetSpyBanMapBoxPos(2);

	CreateSpyBanMapList();
}

void ZStageInterface::CloseSpyBanMapBox()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget("Stage_SpyBanMapBoxClose");
	if (pButton)
		pButton->Show(false);
	pButton = (MButton*)pResource->FindWidget("Stage_SpyBanMapBoxOpen");
	if (pButton)
		pButton->Show(true);

	MWidget* pWidget = pResource->FindWidget("Stage_CharacterInfo");
	if (pWidget)
		pWidget->Enable(true);

	SetSpyBanMapBoxPos(1);
}

void ZStageInterface::HideSpyBanMapBox()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget("Stage_SpyBanMapBoxClose");
	if (pButton)
		pButton->Show(false);
	pButton = (MButton*)pResource->FindWidget("Stage_SpyBanMapBoxOpen");
	if (pButton)
		pButton->Show(true);

	MWidget* pWidget = pResource->FindWidget("Stage_CharacterInfo");
	if (pWidget)
		pWidget->Enable(true);

	SetSpyBanMapBoxPos(0);
}

void ZStageInterface::SetSpyBanMapBoxPos(int nBoxPos)
{
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_SpyBanMapListView");
	if (pWidget)
	{
		MRECT rect;

		switch (nBoxPos)
		{
		case 0:		// Hide
			rect = pWidget->GetRect();
			m_nSpyBanMapListFramePos = -rect.w;
			break;

		case 1:		// Close
			rect = pWidget->GetRect();
			m_nSpyBanMapListFramePos = -rect.w + (rect.w * 0.14);
			break;

		case 2:		// Open
			m_nSpyBanMapListFramePos = 0;
			break;
		}
	}
}

void ZStageInterface::CreateSpyBanMapList()
{
	// ¸Ê ¸®½ºÆ® ¸¸µé¾EÁÖ±E
	MListBox* pMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_SpyBanMapList");
	if (pMapListBox == NULL) return;

	pMapListBox->RemoveAll();	// ±âÁ¸ ¸±·¹ÀÌ¸Ê ¸®½ºÆ®¸¦ ¸ðµÎ Áö¿öÁØ´Ù.
	int nCurrPlayers = (int)ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.size();

	for (int i = MMATCH_SPY_MAP_MANSION; i < MMATCH_SPY_MAP_MAX; i++)
	{
		char szListName[256];
		sprintf(szListName, "(%d~%d%s) %s", g_SpyMapNode[i].nMinPlayers, g_SpyMapNode[i].nMaxPlayers, ZMsg(MSG_SPY_WORD_PLAYERS), g_SpyMapNode[i].szName);

		SpyBanMapList* pSpyBanMapList = new SpyBanMapList(szListName,
			MBitmapManager::Get(
				ZGetGameClient()->GetMatchStageSetting()->IsIncludedSpyMap(g_SpyMapNode[i].nID) ?
				"Mark_Arrow.bmp" : "Mark_X.bmp"),
			g_SpyMapNode[i].nID);

		if (nCurrPlayers < g_SpyMapNode[i].nMinPlayers || nCurrPlayers > g_SpyMapNode[i].nMaxPlayers)
		{
			pSpyBanMapList->SetColor(MCOLOR(0xFFFF0000));	// RED color.
		}

		pMapListBox->Add(pSpyBanMapList);
	}
}

bool ZStageInterface::GetPlayableSpyMapList(int players, vector<int>& out)
{
	int nCurrPlayers = (int)ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.size();

	for (int i = MMATCH_SPY_MAP_MANSION; i < MMATCH_SPY_MAP_MAX; i++)
	{
		if (ZGetGameClient()->GetMatchStageSetting()->IsIncludedSpyMap(g_SpyMapNode[i].nID))
		{
#if !defined(_ARTIC_DEBUG)
			if (nCurrPlayers >= g_SpyMapNode[i].nMinPlayers && nCurrPlayers <= g_SpyMapNode[i].nMaxPlayers)
#endif
			{
				out.push_back(g_SpyMapNode[i].nID);
			}
		}
	}

	return !out.empty();
}

void ZStageInterface::OnSpyStageActivateMap(int nMapID, bool bExclude)
{
	MMatchStageSetting* pSetting = ZGetGameClient()->GetMatchStageSetting();

	if (bExclude)
		pSetting->ExcludeSpyMap(nMapID);
	else
		pSetting->IncludeSpyMap(nMapID);

	CreateSpyBanMapList();	// Refresh.
}

void ZStageInterface::OnSpyStageBanMapList(void* pMapListBlob)
{
	MMatchStageSetting* pSetting = ZGetGameClient()->GetMatchStageSetting();
	pSetting->UnbanAllSpyMap();

	int nCount = MGetBlobArrayCount(pMapListBlob);

	for (int i = 0; i < nCount; i++)
		pSetting->ExcludeSpyMap(*(int*)MGetBlobArrayElement(pMapListBlob, i));

	CreateSpyBanMapList();
}
///////////////////////////////////////////////////////////////////////////////

