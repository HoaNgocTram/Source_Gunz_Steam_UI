#ifndef _ZCOMBATINTERFACE_H
#define _ZCOMBATINTERFACE_H

#include "ZInterface.h"
#include "MPicture.h"
#include "MEdit.h"
#include "MListBox.h"
#include "MLabel.h"
#include "MAnimation.h"
#include "ZObserver.h"
#include "ZCombatChat.h"
#include "ZCrossHair.h"
#include "ZMiniMap.h"
#include "ZVoteInterface.h"
#ifdef _GLOBALANNOUNCE
#include "ZGlobalAnnounce.h"
#endif

#ifdef _RADAR
#include "ZRadar.h"
#endif

_USING_NAMESPACE_REALSPACE2

class ZCharacter;
class ZScreenEffect;
class ZWeaponScreenEffect;
class ZMiniMap;
class ZCombatQuestScreen;

struct ZResultBoardItem {
	char szName[64];
	char szClan[CLAN_NAME_LENGTH];
	int nClanID;
	int nTeam;
	int nScore;
	int nKills;
	int nDeaths;
	int	nAllKill;
	int	nExcellent;
	int	nFantastic;
	int	nHeadShot;
	int	nUnbelievable;
	bool bMyChar;
	bool bGameRoomUser;
	int nWins;
	int	nLosses;

	ZResultBoardItem() { }
	ZResultBoardItem(const char *_szName, const char *_szClan, int _nTeam, int _nScore, int _nKills, int _nDeaths, int _nWins, int _nLosses,bool _bMyChar = false, bool _bGameRoomUser = false) {
		strcpy(szName,_szName);
		strcpy(szClan,_szClan);
		nTeam = _nTeam;
		nScore = _nScore;
		nKills = _nKills;
		nDeaths = _nDeaths;
		// ÇÊ¿äÇÏ¸EÀÌ°Íµéµµ ¸¸µéÀÚ
		nAllKill = 0;
		nExcellent = 0;
		nFantastic = 0;
		nHeadShot = 0;
		nUnbelievable = 0;
		nWins = _nWins;
		nLosses = _nLosses;
		bMyChar = _bMyChar;
		bGameRoomUser = _bGameRoomUser;
	}
};
struct GAUGEVERTEX {
	float x, y, z, w;
	DWORD color;
	float u, v;
};

#ifdef _FLOATDMG_MODE
struct ZFloatDamageItem {
public:
	int floatDmgPosY;
	int floatDmgOpacity;
	unsigned long floatDmgTime;
	unsigned long clearFloatDmgTime;

	int nFloatDamage;
	bool bMultipleDamage;
	int lastShotItemID;

protected:
	MUID targetUID;

public:
	ZFloatDamageItem() {
		floatDmgPosY = 0;
		floatDmgOpacity = 255;
		floatDmgTime = 0;
		clearFloatDmgTime = 0;

		nFloatDamage = 0;
		bMultipleDamage = false;
	}

	MUID& GetUID() { return targetUID; }
	void SetUID(MUID& uid) { targetUID = uid; }

	int GetFloatDamage() { return nFloatDamage; }
	void SetFloatDamage(int floatDMG) { nFloatDamage = floatDMG; }
	void SetClearFloatDmgTime(unsigned long nTime) { clearFloatDmgTime = nTime; }
	void SetMultipleDamage(bool bMulti) { bMultipleDamage = bMulti; }
	void SetLastShotItemID(int itemID) { lastShotItemID = itemID; }

	~ZFloatDamageItem() {}
};

class ZFloatDamageList : public list<ZFloatDamageItem*> {
public:
	void Destroy() {
		while (!empty())
		{
			delete* begin();
			erase(begin());
		}
	}
};

class ZFloatDamageMap : public std::map<MUID, ZFloatDamageItem*> {};
class ZFloatDamageWeaponMap : public std::map<int, ZFloatDamageMap> {};
#endif

class ZResultBoardList : public list<ZResultBoardItem*>
{
public:
	void Destroy() { 
		while(!empty())
		{
			delete *begin();
			erase(begin());
		}
	}
};

struct DuelTournamentPlayer
{
	char m_szCharName[MATCHOBJECT_NAME_LENGTH];
	MUID uidPlayer;
	int m_nTP;
	int nVictory;
	int nMatchLevel;
	int nNumber;

	float fMaxHP;
	float fMaxAP;
	float fHP;
	float fAP;
};

class ZCombatInterface : public ZInterface
{
private:
	float				m_fElapsed;
protected:
	ZWeaponScreenEffect*		m_pWeaponScreenEffect;
//	ZScoreBoard*		m_pScoreBoard;

	// °á°úÈ­¸é¿¡ ÇÊ¿äÇÑ°Í
	ZScreenEffect*		m_pResultPanel;
	ZScreenEffect*		m_pResultPanel_Team;
	ZResultBoardList	m_ResultItems;
	ZScreenEffect*		m_pResultLeft;
	ZScreenEffect*		m_pResultRight;
	//std::vector<ZScoreBoardItem> m_vScoreBoardItems;

#ifdef _RADAR
	ZRadar* m_pRadar;
#endif

#ifdef _GLOBALANNOUNCE
	ZGlobalAnnounce*			m_GlobalAnnounce;
#endif

	int					m_nClanIDRed;			///< Å¬·£ÀEÏ¶§
	int					m_nClanIDBlue;			///< µÎ Å¬·£ ID
	char				m_szRedClanName[32];	
	char				m_szBlueClanName[32];	///< µÎ Å¬·£ÀÇ ÀÌ¸§

	ZCombatQuestScreen*	m_pQuestScreen;

	ZBandiCapturer*		m_Capture;					///< µ¿¿µ»EÄ¸ÃÄ...by kammir 2008.10.02
	bool				m_bShowUI;
	bool				m_bIsFrozen;

	ZObserver			m_Observer;			///< ¿ÉÁ®¹E¸ðµE
	ZCrossHair			m_CrossHair;		///< Å©·Î½º ÇEE
	ZVoteInterface		m_VoteInterface;

	ZIDLResource*		m_pIDLResource;

	MLabel*				m_pTargetLabel;
	MBitmap*			m_ppIcons[ZCI_END];		/// ÄªÂE¾ÆÀÌÄÜµE
	MBitmapR2*			m_pResultBgImg;
	
	bool				m_bMenuVisible;
	
	bool				m_bPickTarget;
	char				m_szTargetName[256];		// crosshair target ÀÌ¸§
	
	MMatchItemDesc*		m_pLastItemDesc;

	int					m_nBulletSpare;
	int					m_nBulletCurrMagazine;
	int					m_nMagazine;

	int					m_nBulletImageIndex;
	int					m_nMagazineImageIndex;

	char				m_szItemName[256];
	
	bool				m_bReserveFinish;
	unsigned long int	m_nReserveFinishTime;

	bool				m_bDrawLeaveBattle;
	int					m_nDrawLeaveBattleSeconds;

	bool				m_bOnFinish;
	bool				m_bShowResult;
	bool				m_bIsShowUI;					// ¸ðµEUI °¨Ãß±E.. by kammir 20081020 (À¯ÀúÀÇ ¼±ÅÃ»çÇ×)
	bool				m_bSkipUIDrawByRule;			// °ÔÀÓ·EÌ ÇÊ¿äÇÏ´Ù¸EUI µå·Î¿E¦ ²E¼EÀÖµµ·Ï

	bool				m_bDrawScoreBoard;
	MBitmapR2*			m_pGameModeBitmap[MMATCH_GAMETYPE_MAX];
	int					m_nScoreBoardIndexStart;
	DWORD				m_dwLastScoreBoardScrollTime;
//	bool				m_bKickPlayerListVisible;		// °ÔÀÓÈ­¸é¿¡ ÇÃ·¹ÀÌ¾E¸®½ºÆ® º¸¿©ÁØ´Ù

	float				m_fOrgMusicVolume;

	bool				m_bNetworkAlive;
	DWORD				m_dLastTimeTick;
	DWORD				m_dAbuseHandicapTick;
#ifdef _BARNPC
	DWORD				m_dStarTime;
#endif

	MBitmapR2*			m_pInfectedOverlay;
	MBitmapR2*			m_pInfectedWidescreenOverlay;

	void SetItemImageIndex(int nIndex);
	void DrawMatchStatus(MDrawContext* pDC);
	void DrawGaugeTeam(float x, float y, float fWidth, float fHeight, DWORD color);
	void SetItemName(const char* szName);
	void UpdateCombo(ZCharacter* pCharacter);
	
	void OnFinish();

	void GameCheckPickCharacter();

	// È­¸é¿¡ ±×¸®´Â°Í°E°EÃµÈ Æã¼ÇµE
	void IconRelative(MDrawContext* pDC,float x,float y,int nIcon);
	void DrawChallengeQuest(MDrawContext* pDC);
	void DrawFriendName(MDrawContext* pDC);			// °°ÀºÆEÀÌ¸§
	void DrawEnemyName(MDrawContext* pDC);			// ÀEÀÌ¸§
	void DrawAllPlayerName(MDrawContext* pDC);		// ¸ðµEÆÀ ÀÌ¸§ Ç¥½Ã (Free Spectator)
	void DrawScoreSelect(MDrawContext* pDC);
	void DrawScoreBoard(MDrawContext* pDC);
	void DrawScoreBoardSolo(MDrawContext* pDC);
	void DrawScoreBoardTeam(MDrawContext* pDC);

	void DrawDuelTournamentScoreBoard(MDrawContext* pDC);						// µà¾EÅä³Ê¸ÕÆ® ´EøÇ¥ È­¸E(tabÅ°)
	void DrawPlayTime(MDrawContext* pDC, float xPos, float yPos);	// ÇÃ·¹ÀÌ ½Ã°£
	void DrawResultBoard(MDrawContext* pDC);		// °ÔÀÓ °á°úÈ­¸E
	void DrawSoloSpawnTimeMessage(MDrawContext* pDC);	// ½ò·Î ½ºÆùÀEÅ¸ÀÌ¸Ó ¸Þ½ÃÁE
//	void DrawSpawnEffect(MDrawContext* pDC);
	void DrawLeaveBattleTimeMessage(MDrawContext* pDC);	// °ÔÀÓ¿¡¼­ ³ª°¥¶§ ±â´Ù¸®´Â ½Ã°£Ç¥½Ã
//	void DrawVoteMessage(MDrawContext* pDC);		// ÅõÇ¥°¡ ÁøÇàÁßÀÏ¶§ ¸Þ½ÃÁE
//	void DrawKickPlayerList(MDrawContext* pDC);		// kick ÇÒ ÇÃ·¹ÀÌ¾E¼±ÅÃÇÏ´Â È­¸E
	void GetResultInfo( void);

	void DrawTDMScore(MDrawContext* pDC);

	void DrawHPAPNPC(MDrawContext* pDC);
	void DrawHPAPNPCQ(MDrawContext* pDC);

	void DrawNPCName(MDrawContext* pDC);

	void UpdateNetworkAlive(MDrawContext* pDC);

	void EventAlive(MDrawContext* pDC);
	void LatencyServer(MDrawContext* pDC);
	//void DrawHPAPBars(MDrawContext* pDC);
#ifdef _FLOATDMG_MODE
	void DrawFloatDamageText(ZObject* pObject, ZFloatDamageItem* pItem, MDrawContext* pDC);
	void DrawAllObjectFloatDamage(MDrawContext* pDC);
#endif

public:
	ZCombatChat			m_Chat;
	ZCombatChat			m_AdminMsg;
#ifdef _FLOATDMG_MODE
	DWORD				g_dwCountDamageTime;
	ZFloatDamageList    m_FloatDamageItems;
	ZFloatDamageWeaponMap    m_FloatDamageWeaponMap;
#endif

#ifdef _KILLFEED
	ZNotifyKills* GetNotifyKills() { return &m_NotifyKills; }
	ZNotifyKills		m_NotifyKills;
#endif
	DWORD				m_nReservedOutTime;				// Finish ÈÄ¿¡ ¹ÛÀ¸·Î ³ª°¡´Â ½Ã°£À» ¼³Á¤
#ifdef _FLOATDMG_MODE
	DWORD               m_lastDamageFloatingTime;
#endif

	ZCombatInterface(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZCombatInterface();

	void OnInvalidate();
	void OnRestore();

	void UpdateStaffCommands(const char* szMsg);
	void UpdatePlayerCommands(const char* szMsg);

	void UpdateSonglistPanel();
	void DrawSonglistPanel(MDrawContext* pDC);

	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void OnDraw(MDrawContext* pDC);	// ±×¸®´Â ¼ø¼­¶§¹®¿¡ Á÷Á¢ ±×¸°´Ù
	virtual void OnDrawCustom(MDrawContext* pDC);
	virtual void DrawAfterWidgets(MDrawContext* pDC);	//MWidget¿¡¼­ ÀçÁ¤ÀÇÇÑ ÇÔ¼E UI À§Á¬À» ¸ðµÎ ±×¸° ÈÄ Á÷Á¢ DC¿¡ Ãß°¡·Î ±×¸®±EÀ§ÇÑ ÇÔ¼E
	void		 DrawPont(MDrawContext* pDC);
	void		 DrawMyNamePont(MDrawContext* pDC);
	void		 RoomIconLead(MDrawContext* pDC);
	void		 TrainingDraw(MDrawContext* pDC);
#ifdef _SPEC 1
	void		 SpectModeInfo(MDrawContext* pDC);
#endif
	void		 ShowExtra(MDrawContext* pDC);
	void		 ShowFPS(MDrawContext* pDC);
#ifdef _INPUTFPS
	void		 ShowInputRate(MDrawContext* pDC);
#endif
#ifdef _PAINTMODE
	void		 PaintDraw(MDrawContext* pDC);
#endif
	void		 Commands(MDrawContext* pDC);
	void		 CommandStaff(MDrawContext* pDC);
	void		 DrawMyWeaponPont(MDrawContext* pDC);
	void		 DrawMyWeaponPontNEW(MDrawContext* pDC);
#ifdef _BULLETBAR
	void		 DrawMyWeaponBar(MDrawContext* pDC);
	void		 DrawMyWeaponBarNEW(MDrawContext* pDC);
#endif
#ifdef _FLOATDMG_MODE
	void		 DmgFloat();
	void         DamageFloatingEffect(MDrawContext* pDC);
#endif
	void         DrawMyHPAPPont(MDrawContext* pDC);
	void		 DrawScore(MDrawContext* pDC);
	void		 DrawBuffStatus(MDrawContext* pDC);
	void		 DrawFinish();
	int DrawVictory( MDrawContext* pDC, int x, int y, int nWinCount, bool bGetWidth = false);

	void UpdateCTFMsg( const char* szMsg );
	void UpdateRTDMsg( const char* szMsg );

	virtual bool IsDone();

	void OnAddCharacter(ZCharacter *pChar);

	void Resize(int w, int h);

	void OutputChatMsg(const char* szMsg);
	void OutputChatMsg(MCOLOR color, const char* szMsg);
#ifdef _KILLFEED
	void OutputNotifyKills(const char* szAtt, const char* szVic, int nAttTeam, int nVicTeam, MMatchWeaponType nWeapon);
#endif

	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);

	static MFont *GetGameFont();
	static MFont* GetGameFontHPAP();
	MPOINT GetCrosshairPoint() { return MPOINT(MGetWorkspaceWidth()/2,MGetWorkspaceHeight()/2); }
	
	ZBandiCapturer*	GetBandiCapturer()			{ return m_Capture; }					///< µ¿¿µ»EÄ¸ÃÄ...by kammir 2008.10.02

	void ShowMenu(bool bVisible = true);
	void ShowInfo(bool bVisible = true);
	void EnableInputChat(bool bInput=true, bool bTeamChat=false);

	void SetDrawLeaveBattle(bool bShow, int nSeconds);

	void ShowChatOutput(bool bShow);
	void SetFrozen(bool b) { m_bIsFrozen = b; }
	bool IsFrozen() { return m_bIsFrozen; }
	bool IsChat() { return m_Chat.IsChat(); }
	bool IsTeamChat() { return m_Chat.IsTeamChat(); }
	bool IsMenuVisible() { return m_bMenuVisible; }

	void Update(float fElapsed);
	void SetPickTarget(long trash, bool bPick, ZCharacter* pCharacter = NULL);

//	void ShowScoreBoard(bool bVisible = true);
//	bool IsScoreBoardVisible() { return m_pScoreBoard->IsVisible(); }

	void Finish();
	bool IsFinish();

	ZCharacter* GetTargetCharacter();
	MUID		GetTargetUID();

	int GetPlayTime();

	void SetObserverMode(bool bEnable);
	bool GetObserverMode() { return m_Observer.IsVisible(); }
	ZObserver* GetObserver() { return &m_Observer; }
	ZCrossHair* GetCrossHair() { return &m_CrossHair; }

	ZVoteInterface* GetVoteInterface()	{ return &m_VoteInterface; }

#ifdef _RADAR
	ZRadar* GetRadar() { return m_pRadar; }
#endif

	void ShowCrossHair(bool bVisible) {	m_CrossHair.Show(bVisible); 	}
	void OnGadget(MMatchWeaponType nWeaponType);
	void OnGadgetOff();

	void SetSkipUIDraw(bool b) { m_bSkipUIDrawByRule = b; }
	bool IsSkupUIDraw() { return m_bSkipUIDrawByRule; }

	bool IsShowResult( void)  { return m_bShowResult; }
	bool IsShowUI( void)  { return m_bIsShowUI; }
	void SetIsShowUI(bool bIsShowUI)  { m_bIsShowUI = bIsShowUI; }
	bool IsShowScoreBoard()   { return m_bDrawScoreBoard; }
//	void SetKickPlayerListVisible(bool bShow = true) { m_bKickPlayerListVisible = bShow; }
//	bool IsKickPlayerListVisible() { return m_bKickPlayerListVisible; }

	bool IsNetworkalive()	{ return m_bNetworkAlive; }

	const char* GetRedClanName() const { return m_szRedClanName; }
	const char* GetBlueClanName() const { return m_szBlueClanName; }

	// Custom: Snipers
	ZWeaponScreenEffect* GetWeaponScreenEffect() { return m_pWeaponScreenEffect; }

#ifdef _GLOBALANNOUNCE
	ZGlobalAnnounce* GetGlobalAnnounce() { return m_GlobalAnnounce; }
#endif
	// Spy mode Interface.
public:
	void FloatDamage(MDrawContext* pDC, ZObject* pObject);
	void OnSpyCreate();
	void OnSpyDestroy();

	void OnSkillDestroy();
	void CreateSkillGameInterface();
	void CreateSkillWaitInterface();

	void DrawSpyName(MDrawContext* pDC);
	void CreateSpyGameInterface();
	void CreateSpyWaitInterface();

	void OnSpyUpdate(float fElapsed);
	void OnSpyDraw(MDrawContext* pDC);

	void SetSpyTip(const char* msg);
	void SetSpyEventMsg(const char* imgName);
	void SetSpyNotice(const char* imgName);

	void SetDefaultSpyTip(MMatchTeam team);
	const char* GetSuitableSpyItemTip(int itemid);

	bool m_bSpyLocationOpened;


protected:
	MBitmapR2* m_pSpyIcon;

protected:
	void SetSpyTimeLimitValue(int m, int s, int ms);	// if you want to set time limit, use SetSpyTimer() instead.
	void SetSpyTimeLimitValue(DWORD dwTime);

public:
	void SetSpyTimer(DWORD dwTimeLimit);
protected:
	DWORD m_dwSpyTimer;	// contains end of time limit.

public:
	void PlaySpyNotice(const char* imgName);
protected:
	bool m_bSpyNoticePlaying;
	DWORD m_dwSpyNoticePlayStartedTime;
};

void TextRelative(MDrawContext* pDC,float x,float y,const char *szText,bool bCenter=false);

#endif