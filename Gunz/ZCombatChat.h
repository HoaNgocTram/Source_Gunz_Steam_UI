#ifndef _ZCOMBATCHAT_H
#define _ZCOMBATCHAT_H
#include "ZInterface.h"
#pragma once
#include <vector>
#include <string>

//class MListBox;
class MEdit;
class MTextArea;

/////////////////
// ZTabPlayerList
#define ZPLB_ITEM_PICKPLAYER	"picked"
class ZTabPlayerList : public MListBox {
protected:
	MEdit*	m_pEditChat;

public:
	ZTabPlayerList(const char* szName, MWidget* pParent=NULL, MListener* pListener=NULL);
	void SetChatControl(MEdit* pEdit)	{ m_pEditChat = pEdit; }

	virtual bool OnShow(void);
	virtual void OnHide(void);
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);

	void OnPickPlayer();
};

#ifdef _KILLFEED
struct ZKillFeedItem {
	unsigned long nTime; // Lưu thời điểm xuất hiện
	std::string strAttacker;
	std::string strVictim;
	std::string strIconPath; // Đường dẫn ảnh súng
	int nAttTeam;
	int nVicTeam;
};

class ZNotifyKills {
private:
	std::vector<ZKillFeedItem> m_vKillFeeds;
	MFont* m_pFont;

public:
	ZNotifyKills();
	virtual ~ZNotifyKills();

	bool Create(); // Không cần tham số XML nữa
	void Destroy();
	void Update();
	void OnDraw(MDrawContext* pDC);

	void AddKillFeed(const char* szAtt, const char* szVic, int nAttTeam, int nVicTeam, MMatchWeaponType nWeapon);
	void SetFont(MFont* pFont);
};
#endif

class ZCombatChat
{
private:
protected:
	ZIDLResource*		m_pIDLResource{};
	MEdit*				m_pInputEdit{};
	ZTabPlayerList*		m_pTabPlayerList{};

	bool				m_bChatInputVisible = true;
	u32					m_nLastChattingMsgTime{};
	bool				m_bTeamChat{};
	bool				m_bShowOutput = true;

	void SetTeamChat(bool bVal)	{ m_bTeamChat = bVal; }
	void UpdateChattingBox();
	void ProcessChatMsg();
public:
	MTextArea*			m_pChattingOutput;

	ZCombatChat();
	virtual ~ZCombatChat();
	bool Create( const char* szOutputTxtarea,bool bUsePlayerList);
	void Destroy();

	void Update();
	void EnableInput(bool bEnable, bool bToTeam=false);
	void OutputChatMsg(const char* szMsg);
	void OutputChatMsg(MCOLOR color, const char* szMsg);

	void OnDraw(MDrawContext* pDC);

	bool IsChat() const;
	bool IsTeamChat() const;
	bool IsShow() const;

	void SetFont( MFont* pFont);

	void ShowOutput(bool bShow);
};
#endif