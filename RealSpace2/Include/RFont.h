#ifndef RFONT_H
#define RFONT_H

#pragma warning(disable: 4786)

#include "RTypes.h"
#include <d3dx9.h>
#include <map>
#include <list>
#include <string>
#include "Realspace2.h"
#include "StringView.h"
#include "MemPool.h"
#include "MUtil.h"

using namespace std;

_NAMESPACE_REALSPACE2_BEGIN

struct RCHARINFO : public CMemPoolSm<RCHARINFO> {		// ÇÑ±ÛÀÚÀÇ Á¤º¸¸¦ °®´Â´Ù
	int nWidth;
	int nFontTextureID;
	int nFontTextureIndex;
};

typedef map<WORD,RCHARINFO*> RCHARINFOMAP;

struct RFONTTEXTURECELLINFO;

typedef list<RFONTTEXTURECELLINFO*> RFONTTEXTURECELLINFOLIST;


// Ä¿´Ù¶õ ÅØ½ºÃÄÀÎ RFontTexture¾ÈÀÇ °¢ ¼¿ÀÇ Á¤º¸¸¦ ´ã°íÀÖ´Â ±¸Á¶Ã¼
struct RFONTTEXTURECELLINFO {
	int nID;
	int nIndex;
	RFONTTEXTURECELLINFOLIST::iterator itr;
};


class RFontTexture {		// ¿©·¯±ÛÀÚ¸¦ ÀúÀåÇÏ°í ÀÖ´Â Ä¿´Ù¶õ ÅØ½ºÃÄ ÇÑÀå

	// ÅØ½ºÃÄ¿¡ µé¾î°¥ ±ÛÀÚ¸¦ ±×¸®´Â ÀÓ½Ã dc & dibsection
	HDC		m_hDC;
	DWORD	*m_pBitmapBits;
	HBITMAP m_hbmBitmap;
//	HFONT	hPrevFont;
	HBITMAP m_hPrevBitmap;


	LPDIRECT3DTEXTURE9		m_pTexture;
	int m_nWidth;
	int	m_nHeight;
	int m_nX,m_nY;
	int m_nCell;			// ¼¿ÀÇ °³¼ö = nX * nY
	int m_LastUsedID;

	int m_nCellSize;

	RFONTTEXTURECELLINFO	*m_CellInfo;
	RFONTTEXTURECELLINFOLIST m_PriorityQueue;	// °¡Àå ÃÖ±Ù¿¡ ¾²ÀÎ °ÍÀÌ µÚÂÊÀ¸·Î Á¤·ÄÇÑ´Ù

	bool UploadTexture(RCHARINFO *pCharInfo,DWORD* pBitmapBits,int w,int h);

	bool InitCellInfo();
	void ReleaseCellInfo();

public:
	RFontTexture();
	~RFontTexture();

	bool Create();
	void Destroy();

	LPDIRECT3DTEXTURE9 GetTexture() { return m_pTexture; }

	int GetCharWidth(HFONT hFont, const TCHAR* szChar);
	int GetCharWidth(HFONT hFont, const wchar_t* szChar);
	bool MakeFontBitmap(HFONT hFont, RCHARINFO *pInfo, const TCHAR* szText, int nOutlineStyle, DWORD nColorArg1, DWORD nColorArg2);
	bool MakeFontBitmap(HFONT hFont, RCHARINFO *pInfo, const wchar_t* szText, int nOutlineStyle, u32 nColorArg1, u32 nColorArg2);

	bool IsNeedUpdate(int nIndex, int nID);		// °»½ÅµÇ¾î¾ß ÇÏ´ÂÁö °Ë»ç
	
	int GetWidth() { return m_nWidth; }
	int GetHeight() { return m_nHeight; }
	int GetCellCountX() { return m_nX; }
	int GetCellCountY() { return m_nY; }
	
	int GetCellSize() { return m_nCellSize; }
	void ChangeCellSize(int size);
};


class RFont {
	template <typename CharT>
	void DrawTextImpl(float x, float y, const BasicStringView<CharT>& Text, u32 dwColor = 0xFFFFFFFF, float fScale = 1.0f);

	template <typename CharT>
	int GetTextWidthImpl(const CharT* szText, int nSize = -1);

	HFONT	m_hFont;	// Font Handle
	int		m_nHeight;
	int		m_nOutlineStyle;
//	int		m_nSamplingMultiplier;
	bool	m_bAntiAlias;

	DWORD	m_ColorArg1;
	DWORD	m_ColorArg2;

	RCHARINFOMAP m_CharInfoMap;
	RFontTexture *m_pFontTexture;

	static	bool	m_bInFont;		// beginfont endfont »çÀÌ¿¡ ÀÖ´ÂÁö.

public:
	RFont(void);
	virtual ~RFont(void);

	bool Create(const TCHAR* szFontName, int nHeight, bool bBold=false, bool bItalic=false, int nOutlineStyle=0, int nCacheSize=-1, bool bAntiAlias=false, DWORD nColorArg1=0, DWORD nColorArg2=0);
	void Destroy(void);

	bool BeginFont();
	bool EndFont();

	//void DrawText(float x, float y, const TCHAR* szText, DWORD dwColor=0xFFFFFFFF, float fScale=1.0f);
	// Draws an extended ASCII string in the current codepage.
	void DrawText(float x, float y, const StringView& Text, u32 Color = 0xFFFFFFFF, float Scale = 1.0f);

	// Draws a UTF-16 string.
	// Doesn't support astral plane characters.
	void DrawText(float x, float y, const WStringView& Text, u32 Color = 0xFFFFFFFF, float Scale = 1.0f);

	// Draws an extended ASCII string in the current codepage.
	void DrawTextWSV(float x, float y, const StringView& Text, u32 Color = 0xFFFFFFFF, float Scale = 1.0f);

	// Draws a UTF-16 string.
	// Doesn't support astral plane characters.
	void DrawTextWSV(float x, float y, const WStringView& Text, u32 Color = 0xFFFFFFFF, float Scale = 1.0f);

	int GetHeight(void){ return m_nHeight; }
	int GetTextWidth(const TCHAR* szText, int nSize=-1);
	int GetTextWidth(const wchar_t* szText, int nSize = -1);
};

// debug
bool DumpFontTexture();
bool RFontCreate();
void RFontDestroy();

_NAMESPACE_REALSPACE2_END

bool CheckFont();
void SetUserDefineFont(char* FontName);

#endif
