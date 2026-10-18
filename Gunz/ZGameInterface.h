#ifndef _ZGAMEINTERFACE_H
#define _ZGAMEINTERFACE_H

#include "ZPrerequisites.h"
#include "ZInterface.h"
#include "ZCamera.h"
#include "ZChat.h"
#include "ZQuest.h"
#include "ZSurvival.h"
#include "ZGameType.h"
#include "ZTips.h"
#include "ZScreenDebugger.h"
#include "ZCombatMenu.h"
#include "ZMyCharacter.h"
#include "ZBandiCapturer.h" // µ¿¿µ»ó Ä¸ÃÄ
#include "ZBitmapManager.h"
#include "ZGame.h"
#ifdef _GLOBALANNOUNCE
#include "ZGlobalAnnounce.h"
#endif
/*
#include "ZGame.h"
#include "ZCharacter.h"
#include "ZCombatInterface.h"
#include "ZObserver.h"
#include "ZLoading.h"
#include "ZGameInput.h"
#include "ZMyItemList.h"
#include "ZMonsterBookInterface.h"
#include "ZInitialLoading.h"
*/

// ¿©±â¿¡ #include ¸¦ ´Þ±âÀü¿¡ ²À ±×·¡¾ß¸¸ÇÏ´ÂÁö ´Ù½ÃÇÑ¹ø »ý°¢ÇØº¸¼¼¿ä +_+  - dubble


#define LOGINSTATE_FADEIN				0
#define LOGINSTATE_SHOWLOGINFRAME		1
#define LOGINSTATE_STANDBY				2
#define LOGINSTATE_LOGINCOMPLETE		3
#define LOGINSTATE_FADEOUT				4

#if defined(_DEBUG) || defined(_RELEASE)
	#define _DUELTOURNAMENT_LOG_ENABLE_		// µà¾óÅä³Ê¸ÕÆ® ·Î±×(Å×½ºÆ® ÀÛ¾÷¿ë)
#endif

#if defined(_DEBUG) || defined(_RELEASE)
//#define _LOG_ENABLE_CLIENT_COMMAND_			// Å¬¶óÀÌ¾ðÆ® Ä¿¸Çµå ·Î±×(Å×½ºÆ® ÀÛ¾÷¿ë)
//#define _LOG_ENABLE_OBSERVER_COMMAND_BUSH_		// ¿ÉÀú¹ö Ä¿¸Çµå Çª½¬ ·Î±×(Å×½ºÆ® ÀÛ¾÷¿ë)
//#define _LOG_ENABLE_RELAY_COMMAND_BUSH_			// ¸®ÇÃ·¹ÀÌ Ä¿¸Çµå Çª½¬ ·Î±×(Å×½ºÆ® ÀÛ¾÷¿ë)
//#define _LOG_ENABLE_OBSERVER_COMMAND_DELETE_	// ¿ÉÀú¹ö ½ÇÇàµÈ ·Î±×(Å×½ºÆ® ÀÛ¾÷¿ë)
//#define _LOG_ENABLE_REPLAY_COMMAND_DELETE_		// ¸®ÇÃ·¹ÀÌ ½ÇÇàµÈ ·Î±×(Å×½ºÆ® ÀÛ¾÷¿ë)
#define _LOG_ENABLE_ROUNDSTATE_					// ¶ó¿îµå »óÅÂ ·Î±×(Å×½ºÆ® ÀÛ¾÷¿ë)
#endif

#if defined(_DEBUG) || defined(_RELEASE)
#define _CHATOUTPUT_ENABLE_CHAR_DAMAGE_INFO_				// °¢ Ä³¸¯ÅÍ µ¥¹ÌÁö ·Î±×(Å×½ºÆ® ÀÛ¾÷¿ë)
#endif

class ZLocatorList;
class ZGameInput;
class ZMonsterBookInterface;
class ZShopEquipInterface;

enum ZChangeWeaponType;

class MUserDataListItem : public MDefaultListItem{
	int m_nUserData;
public:
	MUserDataListItem(const char* szText, int nUserData)
		: MDefaultListItem(szText){
			m_nUserData=nUserData;
		}

	int GetUserData() { return m_nUserData; }
};

class ZItemIconMgr;

class ZGameInterface : public ZInterface {

private:
	ZItemIconMgr* m_pItemIconMgr; // Thêm biến này

public:
	GunzState			m_nInitialState;
	bool				m_bTeenVersion;
	bool				m_bViewUI;
	bool				m_bTeamPlay;
	bool				m_bLoginTimeout;
	bool				m_bPendingUIReload = false;	// reloadUI
	DWORD				m_dwLoginTimeout;
#ifdef _AFKSYSTEM
	DWORD				m_dwAfkTimer;
#endif

//	int					m_nSelectedCharacter;

	MTextAreaLook		m_textAreaLookItemDesc;	// »óÁ¡¿¡¼­ ¾ÆÀÌÅÛ ¼³¸í¿ë TextAreaÀÇ Look

	// Custom: Made m_listDelayedGameCmd public
	list<MCommand*>	m_listDelayedGameCmd;

protected:
	ZScreenEffectManager* m_pScreenEffectManager;
	ZEffectManager* m_pEffectManager;

	GunzState			m_nPreviousState;

	ZCombatInterface* m_pCombatInterface;
	ZShopEquipInterface* m_pShopEquipInterface;
	ZGameInput* m_pGameInput;
	ZLoading* m_pLoadingInterface;
	ZPlayerMenu* m_pPlayerMenu;

	static ZGameClient* m_spGameClient;
	ZGame* m_pGame;
	ZCamera				m_Camera;
	ZChat				m_Chat;
	ZQuest				m_Quest;					///< Äù½ºÆ® °ü·Ãµé	(ÀÌ°Ç ·ÎÁ÷ °´Ã¼ÀÎµ¥ ¿Ö ÀÎÅÍÆäÀÌ½º¿¡ µé¾îÀÖ´Â°Ç°¡..)
	ZSurvival			m_Survival;					// ¼­¹ÙÀÌ¹ú
	ZGameTypeManager	m_GameTypeManager;			///< °ÔÀÓÅ¸ÀÔ °ü·Ãµé
//	ZClan				m_Clan;
	ZMiniMap* m_pMiniMap;
	ZTips				m_Tips;

	ZBandiCapturer* m_Capture;

	ZScreenDebugger		m_ScreenDebugger;			///< È­¸é¿¡ ³ª¿À´Â µð¹ö±× È­¸é
	ZCombatMenu			m_CombatMenu;				///< °ÔÀÓÁß ¸Þ´º

	ZMyCharacter* m_pMyCharacter;

	ZMonsterBookInterface* m_pMonsterBookInterface;



	bool				m_bShowInterface;

	bool				m_bCursor;					///< Ä¿¼­¸¦ »ç¿ëÇÒ ¼ö ÀÖ´Â »óÅÂ
	LPDIRECT3DSURFACE9	m_pCursorSurface;

	DWORD				m_dwFrameMoveClock;

	ZIDLResource		m_IDLResource;

	GunzState			m_nState;			///< ÇöÀç »óÅÂ
	bool				m_bLogin;			///< Login µÇ¾ú´Â°¡?

	bool				m_bLoading;
	bool				m_bWaitingArrangedGame;

	MBitmap* m_pMapThumbnail;///< ¸Ê ½æ³×ÀÏ

	ZMsgBox* m_pMsgBox;
	ZMsgBox* m_pConfirmMsgBox;
	ZInterfaceBackground* m_pBackground;
	ZCharacterSelectView* m_pCharacterSelectView;

#ifdef _GLOBALANNOUNCE
	ZGlobalAnnounce*      m_GlobalAnnounceLobby;
#endif

	bool				m_bOnEndOfReplay;		// ¸®ÇÃ·¹ÀÌ º¸±¸³ª¸é ÇÃ·¹ÀÌ¾îÀÇ Level Percent°¡ ¹Ù²î±â ¶§¹®¿¡ ¸®ÇÃ·¹ÀÌ ½ÃÀÛ Àü¿¡
	int					m_nLevelPercentCache;	// m_bOnEndOfReplay¸¦ true·Î ¼ÂÇÑ ´ÙÀ½ m_nLevelPercentCache¿¡ ÇöÀç LevelPercent
												// °ªÀ» ÀúÀåÇØ ³õ±¸¼­ ³¡³ª¸é ´Ù½Ã º¹¿øÇÑ´Ù. Á» ¾ÈÁÁÀº ±¸Á¶... ¹æ¹ýÀÌ ¾øÀ½. -_-;

	unsigned long int	m_nDrawCount;

	bool			m_bReservedWeapon;
	ZChangeWeaponType m_ReservedWeapon;

	bool			m_bLeaveBattleReserved;
	bool			m_bLeaveStageReserved;
	DWORD			m_dwLeaveBattleTime;
	bool			specialCase;
	DWORD			timeSpecialCase;


	int				m_nLoginState;
	DWORD			m_dwLoginTimer;
	DWORD			m_dwRefreshTime;
	int				m_nLocServ;

	MBitmapR2* m_pRoomListFrame;							// °ÔÀÓ¹æ ¸®½ºÆ® ÇÁ·¹ÀÓ ÀÌ¹ÌÁö
	MBitmapR2* m_pDuelTournamentLobbyFrame;				// µà¾óÅä³Ê¸ÕÆ® Ã¤³ÎÀÏ¶§ÀÇ ÇÁ·¹ÀÓ ÀÌ¹ÌÁö
	MBitmapR2* m_pBottomFrame;								// ÇÏ´Ü Á¤º¸Ã¢ ÇÁ·¹ÀÓ ÀÌ¹ÌÁö
	MBitmapR2* m_pClanInfoBg;								// Å¬·£ Á¤º¸ ¹è°æ ÀÌ¹ÌÁö
	MBitmapR2* m_pDuelTournamentInfoBg;					// µà¾óÅä³Ê¸ÕÆ® ÀüÀû Á¤º¸ ¹è°æ ÀÌ¹ÌÁö
	MBitmapR2* m_pDuelTournamentRankingLabel;				// µà¾óÅä³Ê¸ÕÆ® ·©Å· ¸®½ºÆ® »ó´Ü ·¹ÀÌºí ÀÌ¹ÌÁö
	MBitmapR2* m_pLoginBG;									// ·Î±×ÀÎ ¹è°æ ÀÌ¹ÌÁö
	MBitmapR2* m_pLoginPanel;								// ·Î±×ÀÎ ÆÐ³Î ÀÌ¹ÌÁö

	ZBitmapManager<int> m_ItemThumbnailMgr;							// »óÁ¡/ÀåºñÃ¢/µà¾óÅä³Ê¸ÕÆ® °ÔÀÓÁß¿¡ º¸¿©Áú ¾ÆÀÌÅÛ ½æ³×ÀÏ ¸Å´ÏÀú

	//¡èÀûÀýÇÑ ½Ã±â¸¶´Ù ¸ðµÎ ¾ð·ÎµåÇØ¼­ ¸Þ¸ð¸®¸¦ ³¶ºñÇÏÁö ¾Êµµ·Ï ÇØ¾ß ÇÕ´Ï´Ù. (»óÁ¡ ³ª°¥¶§, ÀåºñÃ¢ ³ª°¥¶§, µà¾óÅä³Ê¸ÕÆ® °ÔÀÓ ³ª°¥¶§)
#ifdef _EXPLOITCLIENT
public: 
	ZLocatorList* m_pLocatorList;
#else
	ZLocatorList*	m_pLocatorList;
#endif
	ZLocatorList*	m_pTLocatorList;

	DWORD			m_dwTimeCount;								// °ÔÀÓ °æ°ú½Ã°£ Ä«¿îÆ®. Ã»¼Ò³â ÀÚÀ²±ÔÁ¦ Àû¿ë¾È ¾²ºÒ...
	DWORD			m_dwHourCount;								// °ÔÀÓ °æ°ú½Ã°£(hour) Ä«¿îÆ®. Ã»¼Ò³â ÀÚÀ²±ÔÁ¦ Àû¿ë¾È ¾²ºÒ...

	DWORD			m_dwVoiceTime;								// ÇöÀç Ãâ·ÂÁßÀÎ º¸ÀÌ½º »ç¿îµå ½Ã°£
	char			m_szCurrVoice[ 256];						// ÇöÀç Ãâ·ÂÇÏ´Â º¸ÀÌ½º ÆÄÀÏ ÀÌ¸§
	char			m_szNextVoice[ 256];						// ´ÙÀ½¿¡ Ãâ·ÂÇÒ º¸ÀÌ½º ÆÄÀÏ ÀÌ¸§
	DWORD			m_dwNextVoiceTime;							// ´ÙÀ½¿¡ Ãâ·ÂÇÒ º¸ÀÌ½º »ç¿îµå ½Ã°£

	int				m_nRetryCount;

	bool			m_bReservedQuit;
	DWORD			m_dwReservedQuitTimer;

	bool			m_bReserveResetApp;							// for changing language

	static bool		m_bSkipGlobalEvent;

	DWORD			m_MyPort;

	DWORD			m_dErrMaxPalyerDelayTime;
	DWORD			m_bErrMaxPalyer;

	bool			m_bGameFinishLeaveBattle;		// ¹èÆ²¿¡¼­ ³ª°¥¶§ ½ºÅ×ÀÌÁö Á¾·á ¿©ºÎ

	//list<MCommand*>	m_listDelayedGameCmd;

// _DUELTOURNAMENT
	vector<DTPlayerInfo> m_vecDTPlayerInfo;
	MDUELTOURNAMENTTYPE m_eDuelTournamentType;

public: //Custom: change to protected to public (Training Option)
	ZItemIconMgr* GetItemIconMgr() { return m_pItemIconMgr; }
	static bool		OnGlobalEvent(MEvent* pEvent);
	virtual bool	OnEvent(MEvent* pEvent, MListener* pListener);
	bool			OnDebugEvent(MEvent* pEvent, MListener* pListener);
	virtual bool	OnCommand(MWidget* pWidget, const char* szMessage);
	static bool		OnCommand(MCommand* pCommand);

	bool ResizeWidget(const char* szName, int w, int h);
	bool ResizeWidgetRecursive( MWidget* pWidget, int w, int h);
	void SetListenerWidget(const char* szName, MListener* pListener);
	void ToggleWebViewWidget(const char* pszURL, int x, int y, int w, int h);
	void OpenGachaInWebview(int x, int y, int w, int h);
	void OpenBattlePassInWebview(int x, int y, int w, int h);
	void OpenMarketInWebview(int x, int y, int w, int h);
	void OpenLadderRewardInWebview(int x, int y, int w, int h);
	void UpdateCursorEnable();
	void UpdateDuelTournamentWaitMsgDots();

//	void LoadCustomBitmap();
	bool InitInterface(const char* szSkinName,ZLoadingProgress *pLoadingProgress = NULL);
	bool InitInterfaceListener();
	void FinalInterface();

	void LoadBitmaps(const char* szDir, const char* szSubDir, ZLoadingProgress *pLoadingProgress);

	void LeaveBattle();

	void OnGreeterCreate(void);
	void OnGreeterDestroy(void);

	void OnLoginCreate(void);
	void OnLoginDestroy(void);

	void OnDirectLoginCreate(void);
	void OnDirectLoginDestroy(void);

	void OnNetmarbleLoginCreate(void);
	void OnNetmarbleLoginDestroy(void);

	void OnGameOnLoginCreate(void);
	void OnGameOnLoginDestroy(void);
	void SelectBackground(int i);
	void OnLobbyCreate(void);
	void OnLobbyDestroy(void);

	void OnStageCreate(void);
	void OnStageDestroy(void);

	void OnCharSelectionCreate(void);
	void OnCharSelectionDestroy(void);

	void OnCharCreationCreate(void);
	void OnCharCreationDestroy(void);

	void OnShutdownState();

#ifdef _BIRDTEST
	void OnBirdTestCreate();
	void OnBirdTestDestroy();
	void OnBirdTestUpdate();
	void OnBirdTestDraw();
	void OnBirdTestCommand(MCommand* pCmd);
#endif

	void OnUpdateGameMessage(void);

	void HideAllWidgets();

	void OnResponseShopItemList( const vector< MTD_ShopItemInfo*> &vShopItemList  , const vector<MTD_GambleItemNode*>& vGItemList );
	void OnResponseCharacterItemList(MUID* puidEquipItem
		, MTD_ItemNode* pItemNodes
		, int nItemCount
		, MTD_GambleItemNode* pGItemNodes
		, int nGItemCount );

	void OnSendGambleItemList( void* pGItemArray, const DWORD dwCount );

	void OnDrawStateGame(MDrawContext* pDC);
	void OnDrawStateLogin(MDrawContext* pDC);
	void OnDrawStateLobbyNStage(MDrawContext* pDC);
	void OnDrawStateCharSelection(MDrawContext* pDC);

#ifdef _QUEST_ITEM
	void OnResponseCharacterItemList_QuestItem( MTD_QuestItemNode* pQuestItemNode, int nQuestItemCount );
	void OnResponseBuyQuestItem( const int nResult, const int nBP );
	void OnResponseSellQuestItem( const int nResult, const int nBP );
#endif

	void OnResponseServerStatusInfoList( const int nListCount, void* pBlob );
	void OnResponseBlockCountryCodeIP( const char* pszBlockCountryCode, const char* pszRoutingURL );

	// locator°ü·Ã.
	void RequestServerStatusListInfo();

/*
	GUNZ_NA = 0,
	GUNZ_GAME = 1,
	GUNZ_LOGIN = 2,
	GUNZ_NETMARBLELOGIN = 3,
	GUNZ_LOBBY = 4,
	GUNZ_STAGE = 5,
	GUNZ_GREETER = 6,
	GUNZ_CHARSELECTION = 7,
	GUNZ_CHARCREATION = 8,
	GUNZ_PREVIOUS = 10,
	GUNZ_SHUTDOWN = 11,
	GUNZ_BIRDTEST
*/
public:
	ZGameInterface(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	~ZGameInterface();

	static bool m_sbRemainClientConnectionForResetApp;	// ¾ð¾î¸¦ ¹Ù²Ù¾î¼­ ¸®¼Ò½º¸¦ ´Ù½Ã ·ÎµùÇØ¾ß ÇÒ¶§ ÀÌ°É true·Î ÇØÁà¾ß ÇÔ

	bool OnCreate(ZLoadingProgress *pLoadingProgress);
	void OnDestroy();

	void OnInvalidate();
	void OnRestore();

	bool Update(float fElapsed);
	void OnDraw(MDrawContext *pDC);
	void DrawAfterWidgets(MDrawContext* pDC);

	void SetCursorEnable(bool bEnable);
	void OnResetCursor();
	bool IsCursorEnable() { return m_bCursor; }

	bool SetState(GunzState nState);
	GunzState GetState(void){ return m_nState; }
	GunzState GetPreviousState(void){ return m_nPreviousState; } // Custom: Get Previous State
	
	void UpdateBlueRedTeam(void);		// µ¿È¯ÀÌ°¡ Ãß°¡

	void ChangeToCharSelection(void);	///< Ä³¸¯ÅÍ ¼±ÅÃÀ¸·Î ÀÌµ¿

	bool ChangeInterfaceSkin(const char* szNewSkinName);

	/// ÇØ´çÇÏ´Â ÀÌ¸§À» ¾ÆÀÌÅÛÀ¸·Î °¡Áø À§Á¬ÀÇ Visible»óÅÂ¸¦ ¹Ù²Û´Ù.
	bool ShowWidget(const char* szName, bool bVisible, bool bModal=false);
	void SetTextWidget(const char* szName, const char* szText);
	void EnableWidget(const char* szName, bool bEnable);
	void ShowPersistentTopFrame(bool bShow);	// for new UI

	// ³ªÁß¿¡ Áö¿ï°Í..¿ì¼± ´çÀå Å×½ºÆ®¸¦ À§ÇØ
	void TestChangeParts(int mode);
	void TestChangePartsAll();
	void TestChangeWeapon(RVisualMesh* pVMesh = NULL);
	void TestToggleCharacter();

	void ChangeParts(int mode);
	void ChangeWeapon(ZChangeWeaponType nType);
	
	void Reload();

	void RespawnMyCharacter();	// È¥ÀÚÅ×½ºÆ®ÇÒ¶§ Å¬¸¯ÇÏ¸é µÇ»ì¾Æ³­´Ù.

	void ReserveLeaveStage();	// ½ºÅ×ÀÌÁö¿¡¼­ ³ª°¥¶§ ÀÏÁ¤½Ã°£ Èå¸¥µÚ ³ª°£´Ù
	void ReserveLeaveStagePreGame();
	void ReserveLeaveBattle();	// ´ë±â¹æÀ¸·Î ³ª°¥‹š ÀÏÁ¤½Ã°£ Èå¸¥µÚ ³ª°£´Ù
	void FinishGame(void);
	bool IsLeaveBattleReserved() { return m_bLeaveBattleReserved; }

	void ReserveResetApp(bool b)	{ m_bReserveResetApp = b; }
	bool IsReservedResetApp()		{ return m_bReserveResetApp; }

	void SaveScreenShot();

	void ShowMessage(const char* szText, MListener* pCustomListenter=NULL, int nMessageID=0);
	void ShowConfirmMessage(const char* szText, MListener* pCustomListenter=NULL);
	void ShowMessage(int nMessageID);
	void ShowErrorMessage(int nErrorID);
	void ShowErrorMessage(const char* szErrorMsg, int nErrorID);
	
	void ShowInterface(bool bShowInterface);
	bool IsShowInterface() { return m_bShowInterface; }

	void SetTeenVersion(bool bt) { m_bTeenVersion = bt; }
	bool GetTeenVersion() { return m_bTeenVersion; }

	void OnCharSelect(void);

	bool OnGameCreate(void);
	void OnGameDestroy(void);
	void OnGameUpdate(float fElapsed);


	// ·Îºñ UI ¼³Á¤
//	void SetupPlayerListButton(int index=-1);
//	void SetupPlayerListTab();
#ifdef _CW_VOTE
	void OnArrangedTeamGameUI(bool bFinding, bool isvote = false);
#else
	void OnArrangedTeamGameUI(bool bFinding);
#endif // _CW_VOTE
	void OnDuelTournamentGameUI(bool bWaiting);

#ifdef _SYSINTERNEW
	void NormalChannelType();
	void ClanChannelType();
	void DuelChannelType();
	void PlayerChannelType();
	void BackGroundButtom();
	void InitLadderWarsUI(bool bLadderWars);
#endif
	void InitLobbyUIByChannelType();
	void InitLadderUI(bool bLadderEnable);
	void InitClanLobbyUI(bool bClanBattleEnable);
	void InitDuelTournamentLobbyUI(bool bEnableDuelTournamentUI);
	void InitChannelFrame(MCHANNEL_TYPE nChannelType);

//	bool InitLocatorList( MZFileSystem* pFileSystem, const char* pszLocatorList );

	// ½ºÅ×ÀÌÁö UI ¼³Á¤
	void SetMapThumbnail(const char* szMapName);
	void ClearMapThumbnail();
	void SerializeStageInterface();

	void EnableLobbyInterface(bool bEnable);
	void EnableStageInterface(bool bEnable);
	void ShowPrivateStageJoinFrame(const char* szStageName);

	void SetRoomNoLight( int d );

	void ShowEquipmentDialog(bool bShow=true);
	void ShowShopDialog(bool bShow = true);
	void ShowEventDialog(bool bShow = true);
	void UpdateAttendanceUI();

	// Ä³¸¯ÅÍ ¼±ÅÃ
	void ChangeSelectedChar( int nNum);

	void ShowReplayDialog( bool bShow);
	void ViewReplay( void);
	// Custom: Delete Replay
	void DeleteReplay( void);


	void ShowMenu(bool bEnable);
	void Show112Dialog(bool bShow);
	// Custom: Enhanced Kick System
	void ShowKickPlayerDialog(bool bShow);
	bool IsMenuVisible();

	bool OpenMiniMap();
	bool IsMiniMapEnable();

	void RequestQuickJoin();
	// WebView2///////////////////////////////////
	//void ToggleWebView(const char* pszURL = NULL);
	//////////////////////////////////////////////
	void EnableCharSelectionInterface(bool bEnable);
#ifdef _SCREENSHOT_FILTER_CHAT
	char m_szScreenShots[100][84];
	char m_szScreenShotPath[MAX_PATH];
	int m_szScreenShotsSize;
	void LoadCustomImages();
	void ShowScreenShotDialog(bool bShow);
	void ViewShowScreenShotDialog(bool bShow);
#endif
public:

	bool IsReadyToPropose();

	// ¸®ÇÃ·¹ÀÌ
	void OnReplay();

#ifdef _KILLFEED
	void LoadWeapons();
	void LoadBlank();
#endif

	// XTrap
	void OnRequestXTrapSeedKey(unsigned char *pComBuf);			// add sgk 0402

#ifdef _ANTIHACK
	void ScreenZGuard();
#endif
	void OnDisconnectMsg( const DWORD dwMsgID );
	void ShowDisconnectMsg( DWORD errStrID, DWORD delayTime );

	void OnAnnounceDeleteClan( const string& strAnnounce );

	// Äù½ºÆ® ¾ÆÀÌÅÛ ¾ÆÀÌÄÜ ºñÆ®¸Ê ¾ò±â(¾²´Âµ§ ¸¹Àºµ¥ ¸¶¶¥È÷ µÑµ¥°¡ ¾ø¾î¼­... -_-;)
	MBitmap* GetQuestItemIcon( int nItemID, bool bSmallIcon);

	// ZActionKey ÀÔ·ÂÁß GlobalEvent ¹«·ÂÈ­
	static bool CheckSkipGlobalEvent() { return m_bSkipGlobalEvent; }
	void SetSkipGlobalEvent(bool bSkip) { m_bSkipGlobalEvent = bSkip; }

	// º¸ÀÌ½º »ç¿îµå Ãâ·Â
	void OnVoiceSound();
	void PlayVoiceSound( char* pszSoundName, DWORD time=0);

	void SetAgentPing(DWORD nIP, DWORD nTimeStamp);

	void OnRequestGameguardAuth( const DWORD dwIndex, const DWORD dwValue1, const DWORD dwValue2, const DWORD dwValue3 );

	void SetErrMaxPlayerDelayTime(DWORD dDelayTime) { m_dErrMaxPalyerDelayTime = dDelayTime; }
	DWORD GetErrMaxPlayerDelayTime() { return m_dErrMaxPalyerDelayTime; }
	void SetErrMaxPlayer(bool bErrMaxPalyer) { m_bErrMaxPalyer = bErrMaxPalyer; }
	bool IsErrMaxPlayer() { return m_bErrMaxPalyer == 0 ? false : true; }

	virtual void MultiplySize(float byIDLWidth, float byIDLHeight, float byCurrWidth, float byCurrHeight);
	void UpdateLadderWarsMyCharInfo();

// _DUELTOURNAMENT
	void SetDuelTournamentCharacterList(MDUELTOURNAMENTTYPE nType, const vector<DTPlayerInfo>& vecDTPlayerInfo);
	const vector<DTPlayerInfo>& GetVectorDTPlayerInfo()	{ return m_vecDTPlayerInfo; }
	void SetDuelTournamantType(MDUELTOURNAMENTTYPE eType)		{ m_eDuelTournamentType = eType; }
	MDUELTOURNAMENTTYPE GetDuelTournamentType()		{ return m_eDuelTournamentType; }

	void UpdateDuelTournamantMyCharInfoUI();
	void UpdateDuelTournamantMyCharInfoPreviousUI();

// ¸±·¹ÀÌ¸Ê
	bool GetIsGameFinishLeaveBattle()			{ return m_bGameFinishLeaveBattle; }

	// ¹Ù±ù¿¡¼­ ¾òÀ»¸¸ÇÑ ÀÎÅÍÆäÀÌ½ºµé (__forceinline ´Â dll-injection ÇÙ ¹æÇØ¸¦ À§ÇÑ °Í)
	__forceinline ZGameClient* GetGameClient(void)			{ return m_spGameClient; }
	__forceinline ZGame* GetGame(void)						{ return m_pGame; }
	__forceinline ZCombatInterface* GetCombatInterface(void) { return m_pCombatInterface; }
	__forceinline ZShopEquipInterface* GetShopEquipInterface(void) { return m_pShopEquipInterface; }
	__forceinline ZCamera* GetCamera()						{ return &m_Camera; }
	__forceinline ZCharacter*	GetMyCharacter()			{ return (ZCharacter*)m_pMyCharacter; }
	__forceinline ZBaseQuest* GetQuest();					// Äù½ºÆ®/¼­¹ÙÀÌ¹ú Áß ÇöÀç ¸ðµå¿¡ ¸Â´Â °É ¹ÝÈ¯
	__forceinline ZQuest* GetQuestExactly()					{ return &m_Quest; }		// Äù½ºÆ® °´Ã¼¸¦ ¹ÝÈ¯
	__forceinline ZSurvival* GetSurvivalExactly()			{ return &m_Survival; }		// ¼­¹ÙÀÌ¹ú °´Ã¼¸¦ ¹ÝÈ¯
	__forceinline ZChat* GetChat()							{ return &m_Chat; }
	__forceinline ZGameTypeManager* GetGameTypeManager()		{ return &m_GameTypeManager; }


	ZScreenEffectManager* GetScreenEffectManager() { return m_pScreenEffectManager; }
	ZEffectManager* GetEffectManager()			{ return m_pEffectManager; }
	void SetGameClient(ZGameClient* pGameClient){ m_spGameClient = pGameClient; }
	
	ZCharacterSelectView* GetCharacterSelectView() { return m_pCharacterSelectView; }
	ZIDLResource* GetIDLResource(void)			{ return &m_IDLResource; }
	ZPlayerMenu* GetPlayerMenu()				{ return m_pPlayerMenu; }
	ZMiniMap*	GetMiniMap()					{ return m_pMiniMap; }
	
	ZTips* GetTips()							{ return &m_Tips; }
	ZBandiCapturer*	GetBandiCapturer()			{ return m_Capture; }					///< µ¿¿µ»ó Ä¸ÃÄ...by kammir 2008.10.02
	ZScreenDebugger* GetScreenDebugger()		{ return &m_ScreenDebugger; }
	ZCombatMenu*	 GetCombatMenu()			{ return &m_CombatMenu; }
	ZMonsterBookInterface* GetMonsterBookInterface()	{ return m_pMonsterBookInterface; }

	ZBitmapManager<int>* GetItemThumbnailMgr()		{ return &m_ItemThumbnailMgr; }
};

__forceinline ZBaseQuest* ZGameInterface::GetQuest()
{
	if (m_pGame && m_pGame->GetMatch())
	{
		MMATCH_GAMETYPE gameType = m_pGame->GetMatch()->GetMatchType();

		if (m_GameTypeManager.IsQuestOnly( gameType ))
			return static_cast<ZBaseQuest*>(&m_Quest);
		else if (m_GameTypeManager.IsSurvivalOnly( gameType ))
			return static_cast<ZBaseQuest*>(&m_Survival);
	}
	
	//ASSERT(0);
	return static_cast<ZBaseQuest*>(&m_Quest);
}


#define BEGIN_WIDGETLIST(_ITEM, _IDLRESPTR, _CLASS, _INSTANCE)								\
{																							\
	MWidgetList WidgetList;																	\
	(_IDLRESPTR)->FindWidgets(WidgetList, _ITEM);											\
	for (MWidgetList::iterator itor = WidgetList.begin(); itor != WidgetList.end(); ++itor) \
{																							\
	if ((*itor) != NULL)																	\
{																							\
	_CLASS _INSTANCE = ((_CLASS)(*itor));

#define END_WIDGETLIST()		}}}


#define DEFAULT_INTERFACE_SKIN "Default"


#define WM_CHANGE_GAMESTATE		(WM_USER + 25)
void ZChangeGameState(GunzState state);		/// ¾²·¹µå¿¡ ¾ÈÀüÇÏ±â À§ÇØ¼­´Â ¸¸µê

inline ZIDLResource* ZGetIDLResource() {
	return ZGetGameInterface()->GetIDLResource();
}

inline MWidget* ZFindWidget(const char* Name) {
	return ZGetIDLResource()->FindWidget(Name);
}

template <typename T>
inline T* ZFindWidgetAs(const char* Name) {
	auto Widget = ZFindWidget(Name);
#ifdef _DEBUG
	assert(Widget == nullptr || dynamic_cast<T*>(Widget) != nullptr || !"Illegal typecast");
#endif
	return static_cast<T*>(Widget);
}

//void ZLoadBitmap(const char* szDir, const char* szFilter, bool bAddDirToAliasName = false);

inline void GetCountryFlagIconFileName(char* out_sz, int nCountry)
{
	switch (nCountry)
	{
	default:
		sprintf(out_sz, "None.png");
		break;
	case PANAMA:
		sprintf(out_sz, "Panama.png");
		break;
	case JAPON:
		sprintf(out_sz, "Japon.png");
		break;
	case BRASIL:
		sprintf(out_sz, "Brasil.png");
		break;
	case ARGENTINA:
		sprintf(out_sz, "Argentina.png");
		break;
	case BOLIVIA:
		sprintf(out_sz, "Bolivia.png");
		break;
	case KOREA:
		sprintf(out_sz, "Korea.png");
		break;
	case COLOMBIA:
		sprintf(out_sz, "Colombia.png");
		break;
	case COSTA_RICA:
		sprintf(out_sz, "CostaRica.png");
		break;
	case CUBA:
		sprintf(out_sz, "Cuba.png");
		break;
	case ECUADOR:
		sprintf(out_sz, "Ecuador.png");
		break;
	case MEXICO:
		sprintf(out_sz, "Mexico.png");
		break;
	case PARAGUAY:
		sprintf(out_sz, "Paraguay.png");
		break;
	case URUGUAY:
		sprintf(out_sz, "Uruguay.png");
		break;
	case VENEZUELA:
		sprintf(out_sz, "Venezuela.png");
		break;
	case SPAIN:
		sprintf(out_sz, "Spain.png");
		break;
	case CANADA:
		sprintf(out_sz, "Canada.png");
		break;
	case PERU:
		sprintf(out_sz, "Peru.png");
		break;
	case USA:
		sprintf(out_sz, "USA.png");
		break;
	case CHILE:
		sprintf(out_sz, "Chile.png");
		break;
	case INDIA:
		sprintf(out_sz, "India.png");
		break;
	case INDONESIA:
		sprintf(out_sz, "Indonesia.png");
		break;
	case KOREA2:
		sprintf(out_sz, "Korea2.png");
		break;
	case FILIPINA:
		sprintf(out_sz, "Filipinas.png");
		break;
	case SINGAPOR:
		sprintf(out_sz, "Singapor.png");
		break;
	case MALASIA:
		sprintf(out_sz, "Malasia.png");
		break;
	case DOMINICANA:
		sprintf(out_sz, "Dominicana.png");
		break;
	case ROMANIA: //Gay
		sprintf(out_sz, "Gey.png");
		break;
	case ALEMANIA: 
		sprintf(out_sz, "Germany.png");
		break;
	case VIETNAM:
		sprintf(out_sz, "vn.png");
		break;
	}
}

inline void GetGradeIDIcon(char* Image, int nGrade = 0)
{
	if (nGrade == 250) {
	sprintf(Image, "icon_adm.png", nGrade);
	}
	else if (nGrade == 251) {
		sprintf(Image, "icon_event.png", nGrade);
	}
	else if (nGrade == 252) {
		sprintf(Image, "icon_gm.png", nGrade);
	}
	else if (nGrade == 254) {
		sprintf(Image, "icon_dev.png", nGrade);
	}
	else if (nGrade == 255) {
		sprintf(Image, "icon_ceo.png", nGrade);
	}
	else if (nGrade == 256) {
		sprintf(Image, "icon_mod.png", nGrade);
	}
	else if (nGrade == 3 || nGrade == 4 || nGrade == 5 || nGrade == 6 || nGrade == 7 
		|| nGrade == 8 || nGrade == 9 || nGrade == 10 || nGrade == 11 || 
		nGrade == 12 || nGrade == 13 || nGrade == 14 ||
		nGrade == 15) 
	{
		sprintf(Image, "icon_vip.png", nGrade);
	}
	else {
		sprintf(Image, "icon_normal.png", nGrade);
	}
}

inline void GetDuelTournamentGradeIconFileName(char* out_sz, int grade)
{
	sprintf(out_sz, "dt_grade%d.png", grade);
}

//Custom: LadderPoints Rank
inline void GetTierIconFile(char* out_sz, int tier)
{
	if (tier >= 0 && tier <= 10)
	{
		sprintf(out_sz, "rank_1.png");
	}
	else if (tier >= 11 && tier <= 30)
	{
		sprintf(out_sz, "rank_2.png");
	}
	else if (tier >= 31 && tier <= 50)
	{
		sprintf(out_sz, "rank_3.png");
	}
	else if (tier >= 51 && tier <= 80)
	{
		sprintf(out_sz, "rank_4.png");
	}
	else if (tier >= 81 && tier <= 120)
	{
		sprintf(out_sz, "rank_5.png");
	}
	else if (tier >= 121 && tier <= 170)
	{
		sprintf(out_sz, "rank_5_1.png");
	}
	else if (tier >= 171 && tier <= 230)
	{
		sprintf(out_sz, "rank_5_2.png");
	}
	else if (tier >= 231)
	{
		sprintf(out_sz, "rank_6.png");
	}
	else
	{
		sprintf(out_sz, "rank_1.png");
	}

	//sprintf(out_sz, "rank_%d.png", nTier); 
}

char* GetItemSlotName( const char* szName, int nItem);
bool SetWidgetToolTipText(char* szWidget,const char* szToolTipText, MAlignmentMode mam=MAM_LEFT|MAM_TOP);

#endif