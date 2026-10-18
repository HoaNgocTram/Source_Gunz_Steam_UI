#ifndef _ZGLOBALANNOUNCE_H
#define _ZGLOBALANNOUNCE_H

#ifdef _GLOBALANNOUNCE
#include <string>
#include <deque>

class MDrawContext;
class MFont;
class AnimatedGifTexture;
struct EmojiEntry;

#define ANNOUNCE_MAX_COLORS		8
#define ANNOUNCE_MAX_QUEUE		50

struct ZAnnounceConfig {
	char	szFontName[64];
	DWORD	dwTextColor;
	DWORD	dwBgColor;
	DWORD	dwBorderColor;
	DWORD	dwShadowColor;
	float	fPosY;
	float	fBoxWidth;
	int		nBoxHeight;
	int		nDisplayTime;
	int		nFadeTime;
	float	fScrollSpeed;
	bool	bShadow;
	int		nShadowOffset;
	int		nBoxPadding;
	int		nScrollPause;
	int		nRepeatCount;
	bool	bColorCycle;
	int		nColorCycleSpeed;
	DWORD	dwCycleColors[ANNOUNCE_MAX_COLORS];
	int		nCycleColorCount;
	char	szIconFile[128];		// GIF icon file path
	int		nIconSize;				// Icon display size (pixels, trước scale)

	ZAnnounceConfig() {
		strcpy(szFontName, "FONTa10_O2Wht");
		dwTextColor = 0xFFFFFF00;
		dwBgColor = 0xC0000000;
		dwBorderColor = 0xFF888888;
		dwShadowColor = 0xAA000000;
		fPosY = 0.04f;
		fBoxWidth = 0.50f;
		nBoxHeight = 30;
		nDisplayTime = 8000;
		nFadeTime = 500;
		fScrollSpeed = 100.0f;
		bShadow = true;
		nShadowOffset = 1;
		nBoxPadding = 10;
		nScrollPause = 1000;
		nRepeatCount = 1;
		bColorCycle = false;
		nColorCycleSpeed = 300;
		nCycleColorCount = 0;
		strcpy(szIconFile, "interface/default/announce_icon.gif");
		nIconSize = 24;
	}
};

class ZGlobalAnnounce {
protected:
	// Current message
	char			m_szGlobalMsg[512];
	int				m_nMsgEmojiSet;  // emoji set detected from prefix
	DWORD			m_dwStartTime;
	bool			m_bActive;
	float			m_fScrollOffset;
	DWORD			m_dwLastUpdateTime;
	int				m_nTotalDisplayTime;
	int				m_nSingleScrollTime;
	bool			m_bNeedScroll;
	int				m_nScrollDist;

	// Queue
	std::deque<std::string>	m_Queue;

	// Icon
	AnimatedGifTexture* m_pIconTexture;
	bool					m_bIconLoaded;

	ZAnnounceConfig	m_Config;

	float CalcAlpha(DWORD dwNow);
	DWORD ApplyAlpha(DWORD dwColor, float fAlpha);
	DWORD GetCyclingColor(DWORD dwNow, float fAlpha);
	int CalcRenderWidth(MFont* pFont);
	void DrawTextWithEmoji(MDrawContext* pDC, int nStartX, int nTextY, DWORD dwTextColor, DWORD dwShadowColor);
	void DrawEmojiStatic(MDrawContext* pDC, const EmojiEntry* pEntry, int x, int y, int w, int h, int nSetId = 0);
	void DrawEmojiAnimated(const EmojiEntry* pEntry, int x, int y, int w, int h, int nSetId = 0);
	void DrawIcon(int x, int y, int w, int h, float fAlpha);
	void DrawQueueCount(MDrawContext* pDC, int x, int y, int nCount, float fAlpha);
	void ActivateMessage(const char* szMsg);	// Set message trực tiếp (không qua queue)
	void AdvanceQueue();						// Chuyển sang message tiếp theo trong queue

public:
	ZGlobalAnnounce();
	~ZGlobalAnnounce();

	const char* GetGlobalMessage() { return m_szGlobalMsg; }
	bool IsActive() { return m_bActive; }
	int GetQueueCount() { return (int)m_Queue.size(); }

	void SetGlobalMessage(const char* pszMessage);	// Thêm vào queue hoặc hiện ngay
	void Clear();									// Kết thúc message hiện tại, chuyển tiếp
	void ClearAll();								// Xóa tất cả (hiện tại + queue)
	void DrawAnnounce(MDrawContext* pDC);
	bool LoadConfig(const char* szFileName = "interface/default/announce.xml");
	bool LoadIcon();
	ZAnnounceConfig& GetConfig() { return m_Config; }
};

#endif
#endif