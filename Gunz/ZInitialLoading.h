#pragma once

class MBitmapR2;
#include "Mint4r2.h"
#include "MThread.h"
#include "RBaseTexture.h"

enum LOADING_STAGE {
	STAGE_INTRO1,       // Hiện ảnh Intro 1
	STAGE_FADE_TO_2,    // Mờ dần Intro 1
	STAGE_INTRO2,       // Hiện ảnh Intro 2
	STAGE_FADE_TO_BG,   // Mờ dần Intro 2
	STAGE_BACKGROUND    // Hiện Background game (như hiện tại)
};

enum LOADING_SCENE_MODE
{
	MODE_DEFAULT,
	MODE_FADEOUT,
	MODE_FADEIN,				
	MODE_BLENDING,
};

#define GRADE_TIMER		5000

class ZInitialLoading
{
protected:
	MBitmapR2*	mBitmaps;
	MBitmapR2*	mBitmap_Bar;
	MBitmapR2*	mBitmap_Grade;		// °ÔÀÓ¹° µî±Þ À§¿øÈ¸ ÀÌ¹ÌÁö
	DWORD		m_Grade_Time;
	bool		m_bGradeFadeOutStart;
	bool		m_bGrade;
	int			miNumScene;
	int			miCurrScene;
	bool		mbUseEnable;
	bool		mbText;
	bool		mbBitmap;
	bool		mbBitmapBar;
	float		mx, my, mw, mh, msx, msy, msw, msh;
	float		mtx, mty, mtw, mth;
	MFontR2*	mpDC;
	char*		mpStr;
	int			m_nTipNum;
	static ZInitialLoading	msInstance;
	float		mPercentage;

	bool		m_bTipsVisible;
	char		m_szTips[256];
	char		m_szNotice[ 256];
	string		m_strLoadingStr;

protected:
	void	DrawTips();
	void	SetRandomTipsText();
	void	DrawDuelTournamentCharacterList();
public:
	void	Draw( LOADING_SCENE_MODE mode_ = MODE_DEFAULT, int destIndex_ = -1, bool bflip_ = true, bool fromThread_ = false );
	void	InitIntros();
	void	DrawBar( float x_, float y_, float w_, float h_, float percent_ );
	void	DrawGrade(float blendFactor); // °ÔÀÓ¹° µî±Þ À§¿øÈ¸ ÀÌ¹ÌÁö
	void	DrawText();
	void	SetText( MFontR2* pDc, float x, float y, char* str );
	void	SetTipNum( int nTipNum)		{ m_nTipNum = nTipNum;	}
	bool	Initialize( int numScene_, float x_, float y_, float w_, float h_, 	float sx_, float sy_, float sw_, float sh_, bool bTipsVisible=false);
	bool	AddBitmap( int index_, const char* bitmapName_ );
	bool	AddBitmapBar( const char* bitmapName_ );
	bool	AddBitmapGrade( const char* bitmapName_ ); // °ÔÀÓ¹° µî±Þ À§¿øÈ¸ ÀÌ¹ÌÁö
	bool	IsUseEnable() const	{		return mbUseEnable;	}
	bool	isText() const	{		return mbText;				}
	void	SetPercentage( float p_ ) 	{		mPercentage = p_;	}
	void	SetLoadingStr(string strParam) { m_strLoadingStr = strParam; }
	void	SetDuelTournamentCharacterList(char szCharacterNameList[8][32]);

	void	Release();
	LOADING_STAGE m_eStage;        // Trạng thái hiện tại
	LOADING_STAGE GetStage() const { return m_eStage; }
	
	static ZInitialLoading*		GetInstance()
	{
		return &msInstance;
	}

public:
	ZInitialLoading();
    ~ZInitialLoading();

private:
	double m_dStageStartTime;      // Thời gian bắt đầu trạng thái (tính bằng ms QPC)
	float m_fCurrentAlpha;         // Alpha hiện tại để vẽ fade

	// Thêm các con trỏ Texture riêng cho Intro (để không lẫn với Background map)
	RBaseTexture* m_pIntro1Tex; // Đổi từ RTexture thành RBaseTexture
	RBaseTexture* m_pIntro2Tex;
};

ZInitialLoading*	ZGetInitialLoading();


class ZLoadingProgress {
	string	m_strName;			// ÀÌ¸§
	ZLoadingProgress *m_pParent;
	float m_fTotalProgressStart;	// ÇöÀç progress ½ÃÀÛÁöÁ¡ (ÀüÃ¼±âÁØ)
	float m_fProgressStart;			// progress ½ÃÀÛÁöÁ¡ (ºÎ¸ð±âÁØ)
	float m_fThisAmount;			// ÀüÃ¼Áß¿¡ ÇöÀç ½ºÅÜÀÌ Â÷ÁöÇÏ´Â ºñÀ²
	float m_fCurrentProgress;		// ÇöÀç ÁøÇàÀ²
	
#ifdef _DEBUG
	static float ZLoadingProgress::m_fLastProgress;	// µÇµ¹¾Æ°¡´Â°æ¿ì°¡ ÀÖ³ª Ã¼Å©
#endif

public:
	ZLoadingProgress(string strName,ZLoadingProgress *pParent=NULL,float fRatio=1.f);

	void Update(float fProgress);
	void Draw();
	void UpdateAndDraw(float fProgress) {
		Update(fProgress);
		Draw();
	}
};