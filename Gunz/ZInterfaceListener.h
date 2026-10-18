#ifndef ZINTERFACELISTENER_H
#define ZINTERFACELISTENER_H

class MListener;

// ¸®½º³Ê ¼±¾ð ¸ÅÅ©·Î
#define DECLARE_LISTENER(_FunctionName)	MListener* _FunctionName(void);

/// ¸®½º³Ê Á¤ÀÇ ¸ÅÅ©·Î, BEGIN_ ~ END_ »çÀÌ¿¡ ¿øÇÏ´Â ÄÚµå¸¦ ³Ö´Â´Ù.
#define BEGIN_IMPLEMENT_LISTENER(_FunctionName, _szMessageName)					\
	MListener* _FunctionName(void){										\
class ListenerClass : public MListener{									\
public:																	\
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){	\
	if(MWidget::IsMsg(szMessage, _szMessageName)==true){

#define END_IMPLEMENT_LISTENER()			\
	return true;			\
	}							\
	return false;				\
	}								\
};									\
	static ListenerClass	Listener;	\
	return &Listener;					\
	}

// Gunz¿¡ ¾²ÀÌ´Â Àü¿ª Listener Object ¾ò±â
MListener* ZGetChatInputListener(void);
MListener* ZGetLoginListener(void);
MListener* ZGetLogoutListener(void);
MListener* ZGetExitListener(void);
MListener* ZGetChannelChatInputListener(void);
MListener* ZGetStageChatInputListener(void);
MListener* ZGetGameStartListener(void);
MListener* ZGetMapChangeListener(void);
MListener* ZGetMapSelectListener(void);
MListener* ZGetParentCloseListener(void);
//MListener* ZGetCombatChatInputListener(void);
MListener* ZGetStageCreateFrameCallerListener(void);
MListener* ZGetSelectCharacterComboBoxListener(void);
MListener* ZGetStageRefreshServerPingCallerListener(void);
// for new UI
MListener* ZGetStageOptTabOptionListener(void);
MListener* ZGetStageOptOkListener(void);
MListener* ZGetStageOptTabModListener(void);
MListener* ZGetStageOptStepperListener(void);
MListener* ZGetStageOptModeListener(void);

// ·Î±×ÀÎ
DECLARE_LISTENER(ZGetLoginStateButtonListener)
DECLARE_LISTENER(ZGetGreeterStateButtonListener)
DECLARE_LISTENER(ZGetOptionFrameButtonListener)

DECLARE_LISTENER(ZGetRegisterListener)
// WebView2///////////////////////////////////
DECLARE_LISTENER(GenericOpenWebViewListener)
DECLARE_LISTENER(OpenGachaListener)
DECLARE_LISTENER(OpenBattlePassListener)
DECLARE_LISTENER(OpenMarketListener)
DECLARE_LISTENER(OpenLadderRewardListener)
DECLARE_LISTENER(CloseWebViewListener)
//////////////////////////////////////////////
// Custom: LadderWar listeners
DECLARE_LISTENER(ZGetArrangedLadderWarsListener)
DECLARE_LISTENER(ZGetLadderWarsDialogOkListener);
DECLARE_LISTENER(ZGetLadderWarsDialogCloseListener);
DECLARE_LISTENER(ZGetLadderWarsVote0)
DECLARE_LISTENER(ZGetLadderWarsVote1)
DECLARE_LISTENER(ZGetLadderWarsVote2)

// ·Îºñ
/*
DECLARE_LISTENER(ZGetChannelPlayerOptionGroupAll)
DECLARE_LISTENER(ZGetChannelPlayerOptionGroupFriend)
DECLARE_LISTENER(ZGetChannelPlayerOptionGroupClan)
*/
//DECLARE_LISTENER(ZGetLobbyStageListListener)
DECLARE_LISTENER(ZGetLobbyPrevRoomListButtonListener)
DECLARE_LISTENER(ZGetLobbyNextRoomListPrevButtonListener)

/*
DECLARE_LISTENER(ZGetLobbyNextRoomNo1ButtonListener)
DECLARE_LISTENER(ZGetLobbyNextRoomNo2ButtonListener)
DECLARE_LISTENER(ZGetLobbyNextRoomNo3ButtonListener)
DECLARE_LISTENER(ZGetLobbyNextRoomNo4ButtonListener)
DECLARE_LISTENER(ZGetLobbyNextRoomNo5ButtonListener)
DECLARE_LISTENER(ZGetLobbyNextRoomNo6ButtonListener)
*/
DECLARE_LISTENER(ZGetLobbyNextRoomNoButtonListener)

/*
DECLARE_LISTENER(ZGetListenerLobbyPlayerListTabChannel);
DECLARE_LISTENER(ZGetListenerLobbyPlayerListTabFriend);
DECLARE_LISTENER(ZGetListenerLobbyPlayerListTabClan);
*/
DECLARE_LISTENER(ZGetPlayerListPrevListener);
DECLARE_LISTENER(ZGetPlayerListNextListener);
#ifdef _CW_VOTE
DECLARE_LISTENER(ZGetLadderWarsVote0);
DECLARE_LISTENER(ZGetLadderWarsVote1);
DECLARE_LISTENER(ZGetLadderWarsVote2);
#endif // _CW_VOTE

DECLARE_LISTENER(ZGetArrangedTeamGameListener);
DECLARE_LISTENER(ZGetArrangedTeamDialogOkListener);
DECLARE_LISTENER(ZGetArrangedTeamDialogCloseListener);
DECLARE_LISTENER(ZGetArrangedTeamGame_CancelListener);

DECLARE_LISTENER(ZGetLeaveClanOKListener);
DECLARE_LISTENER(ZGetLeaveClanCancelListener);

DECLARE_LISTENER(ZGetInviteWarListener);

// Channel frame
DECLARE_LISTENER(ZGetChannelListJoinButtonListener)
DECLARE_LISTENER(ZGetChannelListCloseButtonListener)
DECLARE_LISTENER(ZGetChannelListListener)
DECLARE_LISTENER(ZGetPrivateChannelEnterListener);
DECLARE_LISTENER(ZGetMyClanChannel);
DECLARE_LISTENER(ZGetChannelList);
DECLARE_LISTENER(ZGetClanWarChannel);
DECLARE_LISTENER(ZGetCustomChannel);
DECLARE_LISTENER(ZGetLadderChannel);
DECLARE_LISTENER(ZGetChatCallerListener);
DECLARE_LISTENER(ZGetChatCallerToggleListener);
DECLARE_LISTENER(ZGetPlayerListTabLobbyListener);
DECLARE_LISTENER(ZGetPlayerListTabFriendListener);
DECLARE_LISTENER(ZGetPlayerListTabClanListener);
DECLARE_LISTENER(ZGetPlayerListTabChatListener);

// Stage
//Custom: Lightmaps Time Code
DECLARE_LISTENER(ZGetMapTimeComboListener);

DECLARE_LISTENER(ZGetMapListListener)
DECLARE_LISTENER(ZGetStageListFrameCallerListener)
DECLARE_LISTENER(ZGetStageCreateBtnListener)
DECLARE_LISTENER(ZGetPrivateStageJoinBtnListener)
DECLARE_LISTENER(ZGetChannelListFrameCallerListener)
DECLARE_LISTENER(ZGetStageJoinListener)
DECLARE_LISTENER(ZGetStageSettingCallerListener)
DECLARE_LISTENER(ZGetStageSettingStageTypeListener)
DECLARE_LISTENER(ZGetStageSettingChangedComboboxListener)			// µ¿È¯ÀÌ°¡ ÀÛ¼º
DECLARE_LISTENER(ZGetStageTeamRedListener)
DECLARE_LISTENER(ZGetStageTeamBlueListener)
DECLARE_LISTENER(ZGetStageObserverBtnListener)
DECLARE_LISTENER(ZGetStageReadyListener)
DECLARE_LISTENER(ZGetStageSettingApplyBtnListener)
DECLARE_LISTENER(ZGetLobbyListener)
DECLARE_LISTENER(ZGetBattleExitButtonListener)
DECLARE_LISTENER(ZGetStageExitButtonListener)
DECLARE_LISTENER(ZGetCombatMenuCloseButtonListener)
DECLARE_LISTENER(ZGetPreviousStateButtonListener)
DECLARE_LISTENER(ZGetShopCallerButtonListener)
DECLARE_LISTENER(ZGetMedalShopCallerButtonListener)
DECLARE_LISTENER(ZGetEventShopCallerButtonListener)
DECLARE_LISTENER(ZGetShopCloseButtonListener)
DECLARE_LISTENER(ZGetQuickJoinButtonListener)
DECLARE_LISTENER(ZGetDuelTournamentGameButtonListener)
DECLARE_LISTENER(ZGetDuelTournamentGame2TestButtonListener)
DECLARE_LISTENER(ZGetDuelTournamentGame4TestButtonListener)
DECLARE_LISTENER(ZGetDuelTournamentWaitCancelButtonListener)
DECLARE_LISTENER(ZGetLobbyCharInfoCallerButtonListener)
DECLARE_LISTENER(ZGetStageMapListSelectionListener)
//DECLARE_LISTENER(ZGetStageMapListCallerListener)
DECLARE_LISTENER(ZStageSacrificeItem0)
DECLARE_LISTENER(ZStageSacrificeItem1)
DECLARE_LISTENER(ZStagePutSacrificeItem)
DECLARE_LISTENER(ZStageSacrificeItemBoxOpen)
DECLARE_LISTENER(ZStageSacrificeItemBoxClose)

#ifdef _BOXLEAD
DECLARE_LISTENER(ZGetStageSettingChangedSettingListener)
#endif

DECLARE_LISTENER(ZGetRelayMapTypeListener)
DECLARE_LISTENER(ZGetRelayMapTurnCountListener)
DECLARE_LISTENER(ZStageRelayMapBoxOpen)
DECLARE_LISTENER(ZStageRelayMapBoxClose)
DECLARE_LISTENER(ZGetRelayMapOKButtonListener)

DECLARE_LISTENER(ZStageSpyBanMapBoxOpen)
DECLARE_LISTENER(ZStageSpyBanMapBoxClose)

#ifdef _LOBBYSET
DECLARE_LISTENER(ZGetExtraStageSettingCallerListener)
DECLARE_LISTENER(ZGetExtraStageSettingCloseBtnListener)
DECLARE_LISTENER(ZGetExtraStageSettingApplyBtnListener)
#endif

DECLARE_LISTENER(ZGetStagePlayerListPrevListener);
DECLARE_LISTENER(ZGetStagePlayerListNextListener);

// ¿É¼ÇµéÀº ZOptionInterface ·Î ¿Å±è

// ¿É¼Ç
// ¿É¼ÇµéÀº ZOptionInterface ·Î ¿Å±è

DECLARE_LISTENER(ZGetEquipmentCallerButtonListener)
DECLARE_LISTENER(ZGetEquipmentCloseButtonListener)
//DECLARE_LISTENER(ZGetBackToCharSelectButtonListener)
DECLARE_LISTENER(ZGetCharSelectionCallerButtonListener)
DECLARE_LISTENER(ZGetBuyCashButtonListener)
DECLARE_LISTENER(ZGetBuyMedalButtonListener)
DECLARE_LISTENER(ZGetBuyEventButtonListener)
DECLARE_LISTENER(ZGetBuyButtonListener)
DECLARE_LISTENER(ZGetBuyPackageButtonListener)
DECLARE_LISTENER(ZGetBuyItemDetailFrameGiftListener)
DECLARE_LISTENER(ZGetBuyItemDetailFrameCancelListener)
DECLARE_LISTENER(ZGetBuySpendableItemConfirmOpenListener)
DECLARE_LISTENER(ZGetSellButtonListener)
DECLARE_LISTENER(ZGetSellSpendableItemConfirmOpenListener)

DECLARE_LISTENER(ZGetCountableItemTradeDlgCloseListener)
DECLARE_LISTENER(ZGetCountableItemTradeDlgOkButtonListener)
DECLARE_LISTENER(ZGetCountableItemTradeDlgCountUpButtonListener)
DECLARE_LISTENER(ZGetCountableItemTradeDlgCountDnButtonListener)
DECLARE_LISTENER(ZGetCountableItemTradeDlgCountChangeListener)
DECLARE_LISTENER(ZGetSellCashItemConfirmDlgOkButtonListener)
DECLARE_LISTENER(ZGetSellCashItemConfirmDlgCancelListener)

DECLARE_LISTENER(ZGetEquipButtonListener)
DECLARE_LISTENER(ZGetEquipmentSearchButtonListener)
DECLARE_LISTENER(ZGetStageForcedEntryToGameListener)
DECLARE_LISTENER(ZGetAllEquipmentListCallerButtonListener)			// ±¸ÀÔ¹öÆ°
DECLARE_LISTENER(ZGetMyAllEquipmentListCallerButtonListener)		// ÆÇ¸Å¹öÆ°
DECLARE_LISTENER(ZGetCashEquipmentListCallerButtonListener)			// Ä³½¬¼¥ ¹öÆ°
DECLARE_LISTENER(ZGetMedalEquipmentListCallerButtonListener)		// ¾ÆÀÌÅÛ Àåºñ ¹öÆ°
DECLARE_LISTENER(ZGetEventEquipmentListCallerButtonListener)		// ¼öÀû Àåºñ ¹öÆ°
DECLARE_LISTENER(ZGetShopEquipmentCallerButtonListener)				// ˜Þ->Àåºñ ¹öÆ° ¼±ÅÃ
DECLARE_LISTENER(ZGetSendAccountItemButtonListener)					// º¸³»±â

DECLARE_LISTENER(ZGetBringAccountItemButtonListener)					// °¡Á®¿À±â

DECLARE_LISTENER(ZShopItemEquipmentTabOpen);						// ÄÉ¸¯ÅÍ ÀÏ¹Ý ÀåºñÅÇ ¹öÆ°
DECLARE_LISTENER(ZShopWeaponEquipmentTabOpen);						// ÄÉ¸¯ÅÍ Ä¸½¶ ÀåºñÅÇ ¹öÆ°
DECLARE_LISTENER(ZShopMiscEquipmentTabOpen);						// Custom: MIsc tab for dash/jjang/future items
DECLARE_LISTENER(ZShopListFrameClose)								// ListFrame ´Ý±â ¹öÆ°
DECLARE_LISTENER(ZShopListFrameOpen)								// ListFrame ¿­±â ¹öÆ°

//DECLARE_LISTENER(ZGetShopAskButtonListener);						// Á¶¸£±â
//DECLARE_LISTENER(ZGetShopGiftButtonListener);						// ¼±¹°ÇÏ±â
DECLARE_LISTENER(ZGetShopCachRechargeButtonListener);				// ÃæÀüÇÏ±â
DECLARE_LISTENER(ZGetShopSearchCallerButtonListener);				// °Ë»ö

// Àåºñ
DECLARE_LISTENER(ZGetEquipmentCharacterTabButtonListener);			// Ä³¸¯ÅÍ ÅÇ ¼±ÅÃ
DECLARE_LISTENER(ZGetEquipmentAccountTabButtonListener);			// Áß¾ÓÀºÇà ÅÇ ¼±ÅÃ
DECLARE_LISTENER(ZGetEquipmentShopCallerButtonListener);			// Àåºñ -> ˜Þ ¹öÆ° ¼±ÅÃ
DECLARE_LISTENER(ZGetEquipmentMedalShopCallerButtonListener);			// Àåºñ -> ˜Þ ¹öÆ° ¼±ÅÃ
DECLARE_LISTENER(ZGetEquipmentEventShopCallerButtonListener);
DECLARE_LISTENER(ZGetLevelConfirmListenter);						// Áß¾ÓÀºÇà ·¹º§¹«½Ã °¡Á®¿À±â È®ÀÎ
DECLARE_LISTENER(ZEquipItemEquipmentTabOpen);						// ÄÉ¸¯ÅÍ ÀÏ¹Ý ÀåºñÅÇ ¹öÆ°
DECLARE_LISTENER(ZEquipWeaponEquipmentTabOpen);					// ÄÉ¸¯ÅÍ Ä¸½¶ ÀåºñÅÇ ¹öÆ°
DECLARE_LISTENER(ZEquipMiscEquipmentTabOpen);						// Custom: Misc Equipment for dash/jjang/future items
DECLARE_LISTENER(ZEquipListFrameClose)								// ListFrame ´Ý±â ¹öÆ°
DECLARE_LISTENER(ZEquipListFrameOpen)								// ListFrame ¿­±â ¹öÆ°
DECLARE_LISTENER(ZEquipmetRotateBtn)								// Ä³¸¯ÅÍ ºä È¸Àü º¯°æ ¹öÆ°


// Ä³¸¯ÅÍ ¼±ÅÃ °ü·Ã
DECLARE_LISTENER(ZGetSelectCharacterButtonListener);
DECLARE_LISTENER(ZGetShowCreateCharacterButtonListener);
DECLARE_LISTENER(ZGetDeleteCharacterButtonListener);
DECLARE_LISTENER(ZGetConfirmDeleteCharacterButtonListener);
DECLARE_LISTENER(ZGetCloseConfirmDeleteCharButtonListener);

DECLARE_LISTENER(ZGetSelectCharacterButtonListener0);
DECLARE_LISTENER(ZGetSelectCharacterButtonListener1);
DECLARE_LISTENER(ZGetSelectCharacterButtonListener2);
DECLARE_LISTENER(ZGetSelectCharacterButtonListener3);

DECLARE_LISTENER(ZGetShowCharInfoGroupListener);
DECLARE_LISTENER(ZGetShowEquipInfoGroupListener);

// ¸Ê¼±ÅÃ °ü·Ã
DECLARE_LISTENER(ZGetMapComboListener);
DECLARE_LISTENER(ZGetSelectMapPrevButtonListener);
DECLARE_LISTENER(ZGetSelectMapNextButtonListener);

// ¼±ÅÃµÈ Ä³¸¯ÅÍ µ¹¸®±â
DECLARE_LISTENER(ZGetSelectCameraLeftButtonListener);
DECLARE_LISTENER(ZGetSelectCameraRightButtonListener);

// Ä³¸¯ÅÍ »ý¼º È­¸é
DECLARE_LISTENER(ZGetCreateCharacterButtonListener);
DECLARE_LISTENER(ZGetCancelCreateCharacterButtonListener);
DECLARE_LISTENER(ZChangeCreateCharInfoListener);
DECLARE_LISTENER(ZGetCreateCharacterLeftButtonListener);
DECLARE_LISTENER(ZGetCreateCharacterRightButtonListener);

// ¸®ÇÃ·¹ÀÌ ºê¶ó¿ìÁ® °ü·Ã
DECLARE_LISTENER(ZReplayOk);
DECLARE_LISTENER(ZGetReplayCallerButtonListener);
// Custom: Delete Replay
DECLARE_LISTENER(ZGetReplayDeleteButtonListener);
DECLARE_LISTENER(ZGetReplayViewButtonListener);
DECLARE_LISTENER(ZGetReplayExitButtonListener);
DECLARE_LISTENER(ZGetReplayFileListBoxListener);


// °ÔÀÓ Á¾·á °ü·Ã
DECLARE_LISTENER(ZGetGameResultQuit);


// ¸ó½ºÅÍ µµ°¨
DECLARE_LISTENER(ZGetMonsterInterfacePrevPage);
DECLARE_LISTENER(ZGetMonsterInterfaceNextPage);
DECLARE_LISTENER(ZGetMonsterInterfaceQuit);


// ±âÅ¸
//DECLARE_LISTENER(ZGetListenerGamePlayerListTabGame);
//DECLARE_LISTENER(ZGetListenerGamePlayerListTabFriend);
DECLARE_LISTENER(ZGetRoomListListener);
DECLARE_LISTENER(ZGetMonsterBookCaller);


// 112 È®ÀÎÃ¢
DECLARE_LISTENER(ZGet112ConfirmEditListener);
DECLARE_LISTENER(ZGet112ConfirmOKButtonListener);
DECLARE_LISTENER(ZGet112ConfirmCancelButtonListener);

// Custom: Enhanced Kick System
DECLARE_LISTENER(ZGetKickPlayerOKButtonListener);
DECLARE_LISTENER(ZGetKickPlayerCancelButtonListener);

// Å¬·£ °ü·Ã
DECLARE_LISTENER(ZGetClanSponsorAgreementConfirm_OKButtonListener);
DECLARE_LISTENER(ZGetClanSponsorAgreementConfirm_CancelButtonListener);
DECLARE_LISTENER(ZGetClanSponsorAgreementWait_CancelButtonListener);
DECLARE_LISTENER(ZGetClanJoinerAgreementConfirm_OKButtonListener);
DECLARE_LISTENER(ZGetClanJoinerAgreementConfirm_CancelButtonListener);
DECLARE_LISTENER(ZGetClanJoinerAgreementWait_CancelButtonListener);

// Å¬·£ »ý¼º ´ÙÀÌ¾ó·Î±×
DECLARE_LISTENER(ZGetLobbyPlayerListTabClanCreateButtonListener);
DECLARE_LISTENER(ZGetClanCreateDialogOk);
DECLARE_LISTENER(ZGetClanCreateDialogClose);

// È®ÀÎ ´ÙÀÌ¾Ë·Î±×
DECLARE_LISTENER(ZGetClanCloseConfirmListenter);
DECLARE_LISTENER(ZGetClanLeaveConfirmListenter);

// µ¿ÀÇ¿äÃ» Ãë¼Ò ¹öÆ°
DECLARE_LISTENER(ZGetProposalAgreementWait_CancelButtonListener);
DECLARE_LISTENER(ZGetProposalAgreementConfirm_OKButtonListener);
DECLARE_LISTENER(ZGetProposalAgreementConfirm_CancelButtonListener);
// µ¿ÀÇ½Â¶ô

// ¾ð¾î ¼±ÅÃ ÈÄ Àç½ÃÀÛ È®ÀÎ ´ÙÀÌ¾ó·Î±×
DECLARE_LISTENER(ZGetLanguageChangeConfirmListenter);


DECLARE_LISTENER(ZGetLadderRejoinConfirmListener);

#ifdef _SCREENSHOT_FILTER_CHAT
DECLARE_LISTENER(ZGetScreenShotCallerButtonListener);
DECLARE_LISTENER(ZGetViewScreenShotCallerButtonListener);
DECLARE_LISTENER(ZGetExitScreenShotPictureCallerButtonListener);
DECLARE_LISTENER(ZGetExitScreenShotCallerButtonListener);
DECLARE_LISTENER(ZGetNextScreenShotCallerButtonListener);
DECLARE_LISTENER(ZGetPrevScreenShotCallerButtonListener);
#endif
#endif

// ==========================================
// 1. MAIN TABS LISTENER DECLARATIONS
// ==========================================
DECLARE_LISTENER(ZEquipWeaponTabOpen)
DECLARE_LISTENER(ZEquipArmorTabOpen)
DECLARE_LISTENER(ZEquipItemTabOpen)
DECLARE_LISTENER(ZEquipAccountTabOpen)

// ==========================================
// 2. WEAPON SUB-FILTERS LISTENER DECLARATIONS
// ==========================================
DECLARE_LISTENER(ZBtnWepAllClk)
DECLARE_LISTENER(ZBtnWepMeleeClk)
DECLARE_LISTENER(ZBtnWepPistolClk)
DECLARE_LISTENER(ZBtnWepRevolverClk)
DECLARE_LISTENER(ZBtnWepSMGClk)
DECLARE_LISTENER(ZBtnWepShotgunClk)
DECLARE_LISTENER(ZBtnWepRifleClk)
DECLARE_LISTENER(ZBtnWepMGRktClk)
DECLARE_LISTENER(ZBtnWepRocketClk)

// ==========================================
// 3. ARMOR SUB-FILTERS LISTENER DECLARATIONS
// ==========================================
DECLARE_LISTENER(ZBtnArmAllClk)
DECLARE_LISTENER(ZBtnArmHeadClk)
DECLARE_LISTENER(ZBtnArmChestClk)
DECLARE_LISTENER(ZBtnArmHandsClk)
DECLARE_LISTENER(ZBtnArmLegsClk)
DECLARE_LISTENER(ZBtnArmFeetClk)
DECLARE_LISTENER(ZBtnArmRingsClk)

// --- 4 MAIN TAB SHOP ---
DECLARE_LISTENER(ZBtnShopTabNewClk)
DECLARE_LISTENER(ZBtnShopTabBPClk)
DECLARE_LISTENER(ZBtnShopTabCashClk)
DECLARE_LISTENER(ZBtnShopTabMedalClk)
DECLARE_LISTENER(ZBtnShopTabSellClk)

// --- 3 BANNER PACK & NÚT MUA TRONG TAB NEW ---
DECLARE_LISTENER(ZBtnPackage01Clk)
DECLARE_LISTENER(ZBtnPackage02Clk)
DECLARE_LISTENER(ZBtnPackage03Clk)
DECLARE_LISTENER(ZBtnBuyPackageClk)

// for new UI
DECLARE_LISTENER(ZGetEventCallerButtonListener)
DECLARE_LISTENER(ZGetStageGameInfoToggleListener)
DECLARE_LISTENER(ZGetStageChattingToggleListener)
void PostMapname();
DECLARE_LISTENER(ZBtnDay1Clk)
DECLARE_LISTENER(ZBtnDay2Clk)
DECLARE_LISTENER(ZBtnDay3Clk)
DECLARE_LISTENER(ZBtnDay4Clk)
DECLARE_LISTENER(ZBtnDay5Clk)
DECLARE_LISTENER(ZBtnDay6Clk)
DECLARE_LISTENER(ZBtnDay7Clk)

DECLARE_LISTENER(ZResetPreviewCallerButtonListener)
