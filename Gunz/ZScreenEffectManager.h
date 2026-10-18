#ifndef __ZSCREENEFFECTMANAGER_H
#define __ZSCREENEFFECTMANAGER_H

#include "ZEffectManager.h"
#include "RMeshUtil.h"


enum ZCOMBOLEVEL {
	ZCL_NONE = 0,
	ZCL_GOOD,
	ZCL_NICE,
	ZCL_GREAT,
	ZCL_WONDERFUL
};



class ZScreenEffect : public ZEffect{	// È­¸éÁÂÇ¥°è¿¡ »Ñ·ÁÁö´Â ÀÌÆåÆ® (UI °ü·Ã)
protected:
	RealSpace2::RVisualMesh m_VMesh;	
	rvector	m_Offset;

public:
	ZScreenEffect(RMesh* pMesh,rvector offset=rvector(0,0,0));
	virtual bool Draw(u64 nTime) override;
	virtual void Update();
	virtual bool IsDeleteTime();
	bool DrawCustom(u64 nTime, const rvector& offset, float fAngle = 0.0f);
	RealSpace2::RVisualMesh *GetVMesh() { return &m_VMesh; }
	void SetOffset(rvector& offset) { m_Offset = offset; }
};


class ZScreenEffectLetterBox : public ZScreenEffect{
public:
	ZScreenEffectLetterBox(RMesh *pMesh,rvector offset=rvector(0,0,0))
		:ZScreenEffect(pMesh,offset) {}
	virtual bool Draw(u64 nTime) override;
};


class ZComboEffect : public ZScreenEffect {
public:
	bool bDelete;
	float fDeleteTime;
	ZComboEffect(RMesh* pMesh,rvector offset=rvector(0,0,0));
	virtual bool Draw(u64 nTime) override;
	void SetFrame(int nFrame);
	void DeleteAfter(float fTime=0.f);
};

class ZBossGaugeEffect : public ZScreenEffect {
private:
	int			m_nVisualValue;
	bool		m_bShocked;
	float		m_fShockStartTime;
	float		m_fShockPower;
	float		m_fLastTime;
	rvector		m_ShockOffset;
	rvector		m_ShockVelocity;
public:
	ZBossGaugeEffect(RMesh* pMesh,rvector offset=rvector(0,0,0));
	void Shock(float fPower);
	virtual bool Draw(u64 nTime) override;
};

class ZKOEffect : public ZScreenEffect {
public:
	ZKOEffect(RMesh* pMesh,rvector offset=rvector(0,0,0));
	void SetFrame(int nFrame);
	void InitFrame();
	int GetFrame();
};

class ZTDMBlinkEffect : public ZScreenEffect
{
public:
	ZTDMBlinkEffect(RMesh* pMesh,rvector offset=rvector(0,0,0));
	void SetAnimationSpeed(int nKillsDiff);
};



class ZScreenEffectManager : public ZEffectList {
private:
	RMeshMgr*		m_pEffectMeshMgr;
	RMeshMgr*		m_pQuestEffectMeshMgr;
	list<ZEffectList::iterator>	m_eraseQueue;

	ZCOMBOLEVEL m_CurrentComboLevel;

	RMesh*	m_pHit;
	RMesh*	m_pComboBeginEffect;
	RMesh*	m_pComboEndEffect;
	RMesh*	m_pComboNumberEffect[10];
	RMesh*	m_pExpPlusEffect;
	RMesh*	m_pExpMinusEffect;
	RMesh*	m_pExpNumberEffect[10];

	RMesh*	m_pPraiseEffect[ZCI_END];

	/*
	RMesh*	m_pExcellentEffect;
	RMesh*	m_pAllkillEffect;
	RMesh*	m_pHeadshotEffect;
	RMesh*	m_pFantasticEffect;
	RMesh*	m_pUnbelievableEffect;
	*/

	RMesh*	m_pGoodEffect;
	RMesh*	m_pNiceEffect;
	RMesh*	m_pGreatEffect;
	RMesh*	m_pWonderfullEffect;

	RMesh*	m_pCoolEffect;

	RMesh*	m_pAlertEffect[4];

	ZScreenEffect*	m_pHPPanel;
	ZScreenEffect*  m_pHPPanelNew;
	ZScreenEffect*	m_pHPPanelEffect;
	ZScreenEffect*  m_pHPPanelEffectNew; //Custom: Toggle change bar HP
	ZScreenEffect*	m_pScorePanel;
	ZScreenEffect*	m_pBuffPanel;

	ZScreenEffect* m_pScorePanelTeam;
	ZScreenEffect* m_pScorePanelSolo;

	ZScreenEffect*	m_pWeaponIcons[MWT_END];
	
	// Æ÷¼Ç·ù(¾ÚÇÃ,Ä¸½¶)¿Í Æ®·¦·ù´Â µû·Î °ü¸®ÇÑ´Ù
	typedef map<MMatchItemEffectId, ZScreenEffect*>		MapWeaponIconPotion;
	typedef MapWeaponIconPotion::iterator				ItorWeaponIconPotion;
	MapWeaponIconPotion m_mapWeaponIconPotion;
	
	typedef map<MMatchDamageType, ZScreenEffect*>		MapWeaponIconTrap;
	typedef MapWeaponIconTrap::iterator					ItorWeaponIconTrap;
	MapWeaponIconTrap m_mapWeaponIconTrap;

	ZScreenEffectLetterBox*	m_pSpectator;
	
#define COMBOEFFECTS_COUNT	5
	ZComboEffect	*m_pComboEffects[COMBOEFFECTS_COUNT];

	RBaseTexture	*m_pGaugeTexture;

	void DrawGauges(MDrawContext* pDC);

	float m_fGaugeHP,m_fGaugeAP,m_fGaugeEXP;
	float m_fCurGaugeHP,m_fCurGaugeAP;

	bool m_bGameStart;

	MMatchWeaponType	m_WeaponType;
	MMatchItemDesc*		m_SelectItemDesc;

	int	m_nHpReset;
	int m_nHpResetNew; //Custom: Toggle change bar HP

	// Custom: OW2 HUD state
	float	m_fLastDrawnHP;
	float	m_fLastDrawnAP;
	DWORD	m_dwDamageHPTime;
	DWORD	m_dwDamageAPTime;
	float	m_fDamageHPAmount;
	float	m_fDamageAPAmount;

	// Custom: HUD portrait (OW2-style hero head at level circle)
	class ZCharacterView* m_pPortraitView;
	bool					m_bPortraitReady;
	unsigned long int	m_nPortraitCachedItems[MMCIP_END];
	int						m_nPortraitCachedHair;
	int						m_nPortraitCachedFace;

	ZScreenEffect*	m_pReload;
	ZScreenEffect*	m_pEmpty;

	bool m_bShowReload;
	bool m_bShowEmpty;
	MBitmap* m_pReloadBitmap;
	MBitmap* pEmptyBitmap;
	float m_fReloadTime;

	// Äù½ºÆ® °ü·Ã
	ZBossGaugeEffect*	m_pBossHPPanel;
	ZScreenEffect*		m_pArrow;
	ZKOEffect*			m_pKONumberEffect[10];
	ZKOEffect*			m_pKO;
	int					m_nKO;

	ZScreenEffect*		m_pTDScoreBoard;
	ZTDMBlinkEffect*	m_pTDScoreBlink_R;
	ZTDMBlinkEffect*	m_pTDScoreBlink_B;
protected:
	//void DrawCombo();
	void PlaySoundScoreFlyby();
	void PlaySoundScoreGet();
	void DrawQuestEffects();
	void DrawDuelEffects();
	void DrawTDMEffects();
	void DrawCTFEffects();
	void DrawArrow(rvector& vTargetPos);
	void DrawKO();
public:
	ZScreenEffect* GetCurrWeaponImage();
	void DrawEffects();
	void DrawCombo(MDrawContext* pDC);
	ZScreenEffectManager();
	~ZScreenEffectManager();

	bool Create();
	void Destroy();
	bool CreateQuestRes();		// Äù½ºÆ®¿ë ¸®¼Ò½º ·Îµå
	void DestroyQuestRes();

	int GetCount() { return (int)size();}

	void Clear();

	//void Draw();
	void Draw(MDrawContext* pDC);
	void DrawReloadStatus(MDrawContext* pDC);
	void DrawScoreBoard();
	void DrawSpectator();
	void DrawMyHPAPHud(MDrawContext* pDC); //Custom: Toggle change bar HP

	// Custom: Portrait API (call RefreshPortrait() when equipment changes)
	void DrawPortrait(MDrawContext* pDC, float px, float py, float size);
	void RefreshPortrait() { m_bPortraitReady = false; }
	void DestroyPortrait();

	void DrawMyWeaponImage();
	void DrawMyBuffImage();

	void DrawScoreBoardTeam();
	void DrawScoreBoardSolo();

	void ResetSpectator();

	int  DrawResetGauges();

	void UpdateEffects();

	void SetGameStart(bool b) {
		m_bGameStart = b;
	}

	void ReSetHpPanel();
	void ReSetHpPanelNew(); //Custom: Toggle change bar HP

	void SetGauge_HP(float fHP); 
	void SetGauge_AP(float fAP);
	void SetGauge_EXP(float fEXP) 
	{	
		m_fGaugeEXP=fEXP; 
	}
	void SetGaugeExpFromMyInfo();

	void SetWeapon(MMatchWeaponType wtype,MMatchItemDesc* pDesc) {
		m_WeaponType=wtype; 
		m_SelectItemDesc = pDesc;
	}

	void Add(ZEffect *pEffect);

	// screen ·¹º§ ÀÎÅÍÆäÀÌ½ºÀÇ ÀÌÆåÆ®µé
	void AddScreenEffect(RMesh *pMesh,rvector offset=rvector(0,0,0)) { 
		if(pMesh) Add(new ZScreenEffect(pMesh,offset));	
	}

	void AddScreenEffect(char *szEffectName,rvector offset=rvector(0,0,0)) { 
		AddScreenEffect(m_pEffectMeshMgr->Get(szEffectName),offset);	
	}

	ZScreenEffect* CreateScreenEffect(char *szEffectName,rvector offset=rvector(0,0,0)) { 
		return new ZScreenEffect(m_pEffectMeshMgr->Get(szEffectName),offset); 
	}

	void SetCombo(int nCombo);
	void AddExpEffect(int nExp);

	void AddRoundStart(int nRound);

	void AddFinalRoundStart()	{	AddScreenEffect("finalround");		ZGetGameInterface()->PlayVoiceSound( VOICE_FINAL_ROUND, 1000);	}

	void AddRoundFinish()		{	AddScreenEffect("finishround"); }
	void AddRock();
	void AddCTFEffect(MMatchTeam nTeam, int nArg);

	void AddWin()				{	AddScreenEffect("win"); }
	void AddLose()				{	AddScreenEffect("lose"); }
	void AddDraw()				{	AddScreenEffect("draw"); }

	void AddHit();// { AddScreenEffect("hit"); }
	void AddHitMaker()          {   AddScreenEffect("hitmaker"); }

	
	void AddPraise(int nPraise);	// Score·Î ³¯¶ó°¡´Â ÀÌÆåÆ®
#ifdef _PERFECT
	void Perfect(int Perf);
#endif
	void AddGood();
	void AddNice();
	void AddGreat();
	void AddWonderful();
	void AddCool();

	void ShowReload(bool b) {
		m_bShowReload = b;
	}

	void ShowEmpty(bool b) {
		m_bShowEmpty = b;
	}

	void AddAlert(const rvector& vVictimPos, rvector& vVictimDir, rvector& vAttackerPos);
	void AddKO(int nKills=1);
	void SetKO(int nKills);

	void ShockBossGauge(float fPower);

	void UpdateDuelEffects();		// ¿¬½Â
#ifdef _FLOATDMG_MODE
	RMesh* m_pDmgPlusEffect;
	RMesh* m_pDmgMinusEffect;
	RMesh* m_pDmgNumberEffect[10];

	void AddDamegeEffect(int nDmg);
#endif
#ifdef _PERFECT
	RMesh* m_pPerfect;
#endif
};

#endif