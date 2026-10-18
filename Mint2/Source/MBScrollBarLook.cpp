#include "stdafx.h"
#include "MBScrollBarLook.h"
#include "MBitmapDrawer.h"

void MBArrowLook::OnDrawUpArrow(MDrawContext* pDC, MRECT& r, bool bPressed)
{
	if(bPressed==false) pDC->SetBitmap(m_pArrowBitmaps[4]);
	else pDC->SetBitmap(m_pArrowBitmaps[6]);
	pDC->Draw(r.x, r.y, r.w, r.h);
}  

void MBArrowLook::OnDrawDownArrow(MDrawContext* pDC, MRECT& r, bool bPressed)
{
	if(bPressed==false) pDC->SetBitmap(m_pArrowBitmaps[5]);
	else pDC->SetBitmap(m_pArrowBitmaps[7]);
	pDC->Draw(r.x, r.y, r.w, r.h);
}

void MBArrowLook::OnDrawLeftArrow(MDrawContext* pDC, MRECT& r, bool bPressed)
{
	if(bPressed==false) pDC->SetBitmap(m_pArrowBitmaps[0]);
	else pDC->SetBitmap(m_pArrowBitmaps[2]);
	pDC->Draw(r.x, r.y, r.w, r.h);
}

void MBArrowLook::OnDrawRightArrow(MDrawContext* pDC, MRECT& r, bool bPressed)
{
	if(bPressed==false) pDC->SetBitmap(m_pArrowBitmaps[1]);
	else pDC->SetBitmap(m_pArrowBitmaps[3]);
	pDC->Draw(r.x, r.y, r.w, r.h);
}

MBArrowLook::MBArrowLook(void)
{
	for(int i=0; i<8; i++){
		m_pArrowBitmaps[i] = NULL;
	}
}

MSIZE MBArrowLook::GetDefaultSize(MArrow* pThumb)
{
	if(pThumb->m_nDirection==0){
		if(m_pArrowBitmaps[4]==NULL) return MArrowLook::GetDefaultSize(pThumb);
		return MSIZE(m_pArrowBitmaps[4]->GetWidth(), m_pArrowBitmaps[4]->GetHeight());
	}
	else{
		if(m_pArrowBitmaps[0]==NULL) return MArrowLook::GetDefaultSize(pThumb);
		return MSIZE(m_pArrowBitmaps[0]->GetWidth(), m_pArrowBitmaps[0]->GetHeight());
	}
}


#define IMAGEVGAP		3
void MBThumbLook::OnDraw(MThumb* pThumb, MDrawContext* pDC)
{
	if (!pThumb || !pDC) return;

	MRECT r = pThumb->GetInitialClientRect();

	// 1. Kiểm tra chiều của Scrollbar (0: Horizontal, 1: Vertical)
	MBitmap* pBmp = NULL;
	if (pThumb->m_nDirection == 0) // Horizontal
	{
		if (m_pHBitmaps && m_pHBitmaps[0])
			pBmp = m_pHBitmaps[0];
	}
	else // Vertical
	{
		if (m_pVBitmaps && m_pVBitmaps[0])
			pBmp = m_pVBitmaps[0];
		else if (m_pHBitmaps && m_pHBitmaps[0]) // Fallback nếu chưa load vbitmaps
			pBmp = m_pHBitmaps[0];
	}

	// 2. BẮT BỎ CỬA NẾU BITMAP BỊ NULL (Tránh crash 0xC0000005)
	if (!pBmp)
	{
		// Mẹo: Vẽ tạm khung màu nhẹ nếu chưa nạp được Skin/Texture
		pDC->SetColor(MCOLOR(100, 100, 100, 150));
		pDC->FillRectangle(r);
		return;
	}

	// 3. Nếu Bitmap hợp lệ thì mới tiến hành Draw
	pDC->SetBitmap(pBmp);

	int nBmpWidth = pBmp->GetWidth();
	int nBmpHeight = pBmp->GetHeight();

	if (nBmpWidth <= 0 || nBmpHeight <= 0) return;

	pDC->Draw(r.x, r.y, r.w, r.h / 2,
		0, IMAGEVGAP, nBmpWidth, IMAGEVGAP);
	pDC->Draw(r.x, r.y + r.h / 2, r.w, r.h / 2,
		0, nBmpWidth - IMAGEVGAP * 2, nBmpWidth, IMAGEVGAP);
	pDC->Draw(r.x, r.y, r.w, IMAGEVGAP,
		0, 0, nBmpWidth, IMAGEVGAP);
	pDC->Draw(r.x, r.y + r.h - IMAGEVGAP, r.w, IMAGEVGAP,
		0, nBmpHeight - IMAGEVGAP, nBmpWidth, IMAGEVGAP);
	pDC->Draw(r.x, r.y + r.h / 2 - (nBmpHeight - IMAGEVGAP * 4) / 2, r.w, nBmpHeight - IMAGEVGAP * 4,
		0, IMAGEVGAP * 2, nBmpWidth, nBmpHeight - IMAGEVGAP * 4);
}

MBThumbLook::MBThumbLook(void)
{
	for(int i=0; i<3; i++){
		m_pHBitmaps[i] = NULL;
		m_pHPressedBitmaps[i] = NULL;
		m_pVBitmaps[i] = NULL;
		m_pVPressedBitmaps[i] = NULL;
	}
}

// for new UI
MBRenewalThumbLook::MBRenewalThumbLook(void)
{
	m_pBitmap = NULL;
	m_nGap = 3;
	m_nThickness = 0;
}

void MBRenewalThumbLook::OnDraw(MThumb* pThumb, MDrawContext* pDC)
{
	if (m_pBitmap == NULL) return;

	MRECT r = pThumb->GetInitialClientRect();

	// thu be ngang, can giua thanh cuon
	if (m_nThickness > 0 && r.w > m_nThickness)
	{
		r.x += (r.w - m_nThickness) / 2;
		r.w = m_nThickness;
	}

	int g = m_nGap;
	int bw = m_pBitmap->GetWidth();
	int bh = m_pBitmap->GetHeight();
	if (bw <= 0 || bh <= g * 2) return;

	pDC->SetBitmap(m_pBitmap);

	// vach qua ngan thi ve nguyen anh, khoi cat
	if (r.h <= g * 2)
	{
		pDC->Draw(r.x, r.y, r.w, r.h, 0, 0, bw, bh);
		return;
	}

	pDC->Draw(r.x, r.y, r.w, g, 0, 0, bw, g);
	pDC->Draw(r.x, r.y + g, r.w, r.h - g * 2, 0, g, bw, bh - g * 2);
	pDC->Draw(r.x, r.y + r.h - g, r.w, g, 0, bh - g, bw, g);
}

MBScrollBarLook::MBScrollBarLook(void)
{
	for(int i=0; i<3; i++){
		m_pVFrameBitmaps[i] = NULL;
		m_pHFrameBitmaps[i] = NULL;
	}
}

void MBScrollBarLook::OnDraw(MScrollBar* pScrollBar, MDrawContext* pDC)
{
	MRECT r = pScrollBar->GetInitialClientRect();
	//if(pScrollBar->GetType()==MSBT_VERTICAL)
	//	DrawBitmapFrameV3(pDC, r, m_pVFrameBitmaps);
	//else 
	//	DrawBitmapFrameH3(pDC, r, m_pHFrameBitmaps);
	MRECT rtemp  = pDC->GetClipRect();
	pDC->SetClipRect(rtemp.x, rtemp.y, rtemp.w, rtemp.h );
	pDC->SetColor(MCOLOR(0xFF000000));
	pDC->FillRectangle( r.x, r.y, r.w, r.h );
	pDC->SetClipRect(rtemp);
}

MRECT MBScrollBarLook::GetClientRect(MScrollBar* pScrollBar, MRECT& r)
{
	return r;
	/*
	if(IsNull(m_pVFrameBitmaps, 3)==true) return r;
	if(IsNull(m_pHFrameBitmaps, 3)==true) return r;

	if(pScrollBar->GetType()==MSBT_VERTICAL){
		int lw = m_pVFrameBitmaps[0]->GetWidth();
		int rw = m_pVFrameBitmaps[2]->GetWidth();
		return MRECT(r.x, r.y+lw, r.w, r.h-(lw+rw));
	}
	else{
		int lw = m_pHFrameBitmaps[0]->GetWidth();
		int rw = m_pHFrameBitmaps[2]->GetWidth();
		return MRECT(r.x+lw, r.y, r.w-(lw+rw), r.h);
	}
	*/
}
