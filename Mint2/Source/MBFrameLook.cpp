#include "stdafx.h"
#include "Mint.h"
#include "MBFrameLook.h"
#include "MBitmapDrawer.h"


void MBFrameLook::OnDraw(MFrame* pFrame, MDrawContext* pDC)
{
	MRECT r = pFrame->GetInitialClientRect();

	// Tooltip frame (Custom Look 3)
	if (GetCustomLook() == 3) {
		if (pFrame->GetVisible() == false)
			return;

		MRECT rr = r;
		rr.x -= 10;
		rr.y -= 10;
		rr.w += 20;
		rr.h += 20;

		MRECT cliprect = MRECT(0, 0, MGetWorkspaceWidth(), MGetWorkspaceHeight());
		DrawBitmapFrame2(pDC, rr, cliprect, m_pFrameBitmaps);
		return;
	}

	MCOLOR color = MCOLOR(0xFF, 0xFF, 0xFF, pFrame->GetOpacity());

	if (GetCustomLook() == 1)
		DrawBitmapFrameCustom1(pDC, r, m_pFrameBitmaps, m_bStretch);
	else if (GetCustomLook() == 2)
		DrawBitmapFrameCustom2(pDC, r, m_pFrameBitmaps, m_BGColor, m_bStretch);
	else if (GetCustomLook() == 3)
		DrawBitmapFrameCustom3(pDC, r, m_pFrameBitmaps, m_BGColor, m_bStretch);
	else if (GetCustomLook() == 4)
	{
		pDC->SetColor(128, 128, 128);
		pDC->Rectangle(r);
	}
	else
		DrawBitmapFrame9(pDC, r, m_pFrameBitmaps, m_bStretch, GetScale());


	if (pFrame->m_bTitleBar == true) {
		// Vẽ Close Button
		if (pFrame->GetCloseButton()->m_bVisible)
		{
			MBitmap* pBitmap;
			if (pFrame->GetCloseButton()->IsButtonDown() == false)
				pBitmap = m_pCloseButtonBitmaps[0];
			else
			{
				pBitmap = m_pCloseButtonBitmaps[1];
				if (pBitmap == NULL) pBitmap = m_pCloseButtonBitmaps[0];
			}

			if (pBitmap != NULL)
			{
				int x = pFrame->GetCloseButton()->m_Rect.x;
				int y = pFrame->GetCloseButton()->m_Rect.y;
				pDC->SetBitmap(pBitmap);
				pDC->Draw(x, y);
			}
		}

		// Vẽ Minimize Button
		if (pFrame->GetMinimizeButton()->m_bVisible)
		{
			MBitmap* pBitmap;
			if (pFrame->GetMinimizeButton()->IsButtonDown() == false)
				pBitmap = m_pMinimizeButtonBitmaps[0];
			else
			{
				pBitmap = m_pMinimizeButtonBitmaps[1];
				if (pBitmap == NULL) pBitmap = m_pMinimizeButtonBitmaps[0];
			}

			if (pBitmap != NULL)
			{
				int x = pFrame->GetMinimizeButton()->m_Rect.x;
				int y = pFrame->GetMinimizeButton()->m_Rect.y;
				pDC->SetBitmap(pBitmap);
				pDC->Draw(x, y);
			}
		}


		MFont* pOldFont = pDC->GetFont();

		// [SỬA TẠI ĐÂY]: Đổi font cho Tiêu đề (Lấy font lớn hơn từ FontManager)
		MFont* pTitleFont = MFontManager::Get("FONTa10b"); // Hoặc MFontManager::Get("DEFAULT") / MFontManager::Get("BIG")
		if (pTitleFont != NULL)
		{
			pDC->SetFont(pTitleFont);
		}
		else if (m_pFont != NULL)
		{
			pDC->SetFont(m_pFont);
		}

		// Lấy pointer font hiện tại để tính chiều rộng chữ
		MFont* pCurFont = pDC->GetFont();

		// 1. Tính Y căn giữa chiều cao Title Bar
		int y = int(m_TitlePosition.y * GetScale());
		if (m_pFrameBitmaps[7])
		{
			int au = (int)(GetScale() * m_pFrameBitmaps[7]->GetHeight());
			y = (au - m_pFont->GetHeight()) / 2;
		}

		y += CONVERT600(15);

		// 2. Tính X căn giữa khung
		int x = int(r.x + m_TitlePosition.x * GetScale()); // Mặc định nếu không lấy được độ rộng chữ
		if (m_pFont != NULL && pFrame->m_szName[0] != '\0')
		{
			int nTextWidth = m_pFont->GetWidth(pFrame->m_szName);
			x = r.x + (r.w - nTextWidth) / 2; // Ép căn giữa 100%
		}

		// Shadow text cũ nếu có
		if (GetCustomLook() == 0)
		{
			pDC->SetColor(MCOLOR(0x0));
			pDC->Text(x + 1, r.y + y + 1, pFrame->m_szName);
		}

		// Vẽ Text Title ở chính giữa
		pDC->SetColor(m_FontColor);
		pDC->Text(x, r.y + y, pFrame->m_szName);
	}
}

MBFrameLook::MBFrameLook(void)	: MScalableLook()
{
	m_szDefaultTitle[0]=0;
	m_TitlePosition = MPOINT(14, 10);
	m_FontColor = MCOLOR(255,255,255,255);
	m_bStretch = true;

	for(int i=0; i<FRAME_BITMAP_COUNT; i++){
		m_pFrameBitmaps[i] = NULL;
	}
	for (int i=0; i<FRAME_BUTTON_BITMAP_COUNT; i++)
	{
		m_pCloseButtonBitmaps[i] = NULL;
		m_pMinimizeButtonBitmaps[i] = NULL;
	}
	m_iCustomLook = 0;
}

#define RECT_DEFAULT_SPACE 12
MRECT MBFrameLook::GetClientRect(MFrame* pFrame, MRECT& r)
{
	if(GetCustomLook() == 1)
	{
 		int al = FRAME_OUTLINE_WIDTH+FRAME_WIDTH+FRAME_INLINE_WIDTH+RECT_DEFAULT_SPACE;
		int au = m_pFrameBitmaps[7]->GetHeight() + FRAME_OUTLINE_WIDTH + FRAME_INLINE_WIDTH+RECT_DEFAULT_SPACE;
		return MRECT(r.x+al, r.y+au, r.w-(al*2), r.h-(al+au));
	}
	if( GetCustomLook() == 2 )
	{
#define CUSTOM2_FRAME_OFFSET_WIDTH 2
#define CUSTOM2_FRAME_OFFSET_HEIGHT 10
		return MRECT(r.x + CUSTOM2_FRAME_OFFSET_WIDTH, r.y + CUSTOM2_FRAME_OFFSET_HEIGHT , r.w - 2 * CUSTOM2_FRAME_OFFSET_WIDTH, r.h - 2 * CUSTOM2_FRAME_OFFSET_HEIGHT);
	}
	else
	{
		if(IsNull(m_pFrameBitmaps, FRAME_BITMAP_COUNT)==true) return r;

		float fScale = GetScale();
		int al = (int)(fScale * m_pFrameBitmaps[3]->GetWidth());
		int au = (int)(fScale * m_pFrameBitmaps[7]->GetHeight());
		int ar = (int)(fScale * m_pFrameBitmaps[5]->GetWidth());
		int ab = (int)(fScale * m_pFrameBitmaps[1]->GetHeight());
		return MRECT(r.x+al, r.y+au, r.w-(al+ar), r.h-(au+ab));
	}
}
