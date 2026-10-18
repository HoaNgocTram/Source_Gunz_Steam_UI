#ifndef _ZCHAT_H
#define _ZCHAT_H

#include "ZPrerequisites.h"
#include <vector>
using namespace std;

#include "ZChatCmd.h"
#include "ZReportAbuse.h"
#include "ZColorTable.h"

class MWidget;
class Chat;

class ZChat
{
	friend Chat;
private:
	ZChatCmdManager		m_CmdManager;

	unsigned long int	m_nLastInputTime;
	int					m_nSameMsgCount;
	char				m_nLastInputMsg[512];
	unsigned long int	m_nLastAbuseTime;
	int					m_nAbuseCounter;
	char				m_szWhisperLastSender[64];		// ¸¶Áö¸·¿¡ ³ªÇÑÅ× ±Ó¸» º¸³Â´ø »ç¶÷

	ZReportAbuse		m_ReportAbuse;

	void LobbyChatOutput(const char* szChat,MCOLOR color = MCOLOR(ZCOLOR_CHAT_LOBBY_DEFALT) );
	void StageChatOutput(const char* szChat,MCOLOR color = MCOLOR(ZCOLOR_CHAT_STAGE_DEFAULT) );

	void InitCmds();
	bool CheckRepeatInput(string szMsg);
public:
	enum ZCHAT_MSG_TYPE
	{
		CMT_NORMAL = 0,
		CMT_SYSTEM = 1,
		CMT_BROADCAST = 2,
		CMT_STAFFHELP = 3,
		CMT_CURSOUR = 4,
#ifdef _STAFFCHAT
		CMT_STAFFCHAT = 5,
#endif
		CMT_END
	};
	enum ZCHAT_LOC
	{
		CL_CURRENT = 0,		// Áö±Ý º¸°í ÀÖ´Â Ã¤ÆÃÃ¢
		CL_LOBBY = 1,		// ·Îºñ Ã¤ÆÃÃ¢
		CL_STAGE = 2,		// ½ºÅ×ÀÌÁö Ã¤ÆÃÃ¢
		CL_GAME = 3,		// °ÔÀÓ¾È Ã¤ÆÃÃ¢
		CL_END
	};

	ZChat();
	virtual ~ZChat();

	bool Input(char* szMsg);
	void Output(const char* szMsg, ZCHAT_MSG_TYPE msgtype = CMT_NORMAL, ZCHAT_LOC loc=CL_CURRENT,MCOLOR _color=MCOLOR(0,0,0));
	void Output(MCOLOR color, const char* szMsg, ZCHAT_LOC loc=CL_CURRENT);

	void Clear(ZCHAT_LOC loc=CL_CURRENT);
	void Report112(const char* szReason);
	bool CheckChatFilter(const char* szMsg);	///< ¿åÇÊÅÍ¸µ °Ë»ç. ±Ó¸»µîÀÇ Ä¿¸Çµå¸í·É¾î¿¡¼­´Â µû·Î Ã³¸®¸¦ ÇØÁà¾ßÇÑ´Ù.
	void FilterWhisperKey(MWidget* pWidget);
	void SetWhisperLastSender(char* szSenderName) { strcpy(m_szWhisperLastSender, szSenderName); }

	ZChatCmdManager* GetCmdManager() { return &m_CmdManager; }
};

// ÆíÀÇ¸¦ À§ÇØ¼­ ¸¸µç ÇÔ¼ö ---
void ZChatOutputF(const char* szFormat, ...);
void ZChatOutput(const char* szMsg, ZChat::ZCHAT_MSG_TYPE msgtype=ZChat::CMT_NORMAL, ZChat::ZCHAT_LOC loc=ZChat::CL_CURRENT,MCOLOR _color=MCOLOR(ZCOLOR_CHAT_SYSTEM));
void ZChatOutput(MCOLOR color, const char* szMsg, ZChat::ZCHAT_LOC loc=ZChat::CL_CURRENT);
void ZChatOutputMouseSensitivityChanged(int old, int neo);
void ZChatOutputMouseSensitivityCurrent(int i);


#endif