#include "stdafx.h"
#include "MBButtonLook.h"
#include "MColorTable.h"
#include "MBitmapDrawer.h"
#include "Mint.h"

MBButtonLook::MBButtonLook(void) : MScalableLook()
{
	int i;
	for (i = 0; i < 9; i++) {
		m_pUpBitmaps[i] = NULL;
		m_pDownBitmaps[i] = NULL;
		m_pOverBitmaps[i] = NULL;
	}
	for (i = 0; i < 4; i++) {
		m_pFocusBitmaps[i] = NULL;
	}

	m_pFont = NULL;
	m_FontColor = MCOLOR(DEFCOLOR_MBUTTON_TEXT);
	m_FontDownColor = MCOLOR(DEFCOLOR_LIGHT);
	m_FontDisableColor = MCOLOR(DEFCOLOR_DISABLE);
	m_FontHighlightColor = MCOLOR(DEFCOLOR_PEN_HIGHLIGHT);
	m_bStretch = true;
	m_bCenterBitmap = false;	// for new UI
	m_bCustomLook = false;

	m_FontDownOffset = MPOINT(0, 0);
}

void MBButtonLook::DrawText(MButton* pButton, MDrawContext* pDC, MRECT& r, const char* szText, MAlignmentMode a)
{
	if (pButton->m_pIcon != NULL) {
		pDC->SetBitmap(pButton->m_pIcon);
		pDC->Draw(r.x, r.y + (r.h - pButton->m_pIcon->GetHeight()) / 2);

		r.x += pButton->m_pIcon->GetWidth();
		r.w -= pButton->m_pIcon->GetWidth();
	}

	if (pButton->IsButtonDown())
	{
		r.x += (int)(m_FontDownOffset.x * GetScale());
		r.y += (int)(m_FontDownOffset.y * GetScale());
	}

	if (m_pFont != NULL) pDC->SetFont(m_pFont);
	if (pButton->IsEnable() == true) {
		if (pButton->IsButtonDown() == true) pDC->SetColor(m_FontDownColor);
		else if (pButton->IsMouseOver()) pDC->SetColor(m_FontDownColor);
		else if (pButton->GetType() == MBT_PUSH2 && pButton->GetCheck()) pDC->SetColor(m_FontDownColor);
		else pDC->SetColor(m_FontColor);
	}
	else pDC->SetColor(m_FontDisableColor);
	if (pButton->m_bHighlight == true) {
		MCOLOR PrevHCol = pDC->SetHighlightColor(m_FontHighlightColor);
		pDC->TextWithHighlight(r, szText, a);
		pDC->SetHighlightColor(PrevHCol);
	}
	else {
		pDC->Text(r, szText, a);
	}
}

void MBButtonLook::DrawFocus(MDrawContext* pDC, MRECT& r)
{
	if (m_pFocusBitmaps[0] == NULL) return;

	MRECT cr = pDC->GetClipRect();
	MRECT pcr = cr;
	cr.x -= 2;
	cr.y -= 2;
	cr.w += 4;
	cr.h += 4;
	pDC->SetClipRect(cr);

	pDC->SetBitmap(m_pFocusBitmaps[0]);
	pDC->Draw(r.x - 2, r.y - 2);
	pDC->SetBitmap(m_pFocusBitmaps[1]);
	pDC->Draw(r.x + r.w - m_pFocusBitmaps[1]->GetWidth() + 2, r.y - 2);
	pDC->SetBitmap(m_pFocusBitmaps[2]);
	pDC->Draw(r.x - 2, r.y + r.h - m_pFocusBitmaps[1]->GetHeight() + 2);
	pDC->SetBitmap(m_pFocusBitmaps[3]);
	pDC->Draw(r.x + r.w - m_pFocusBitmaps[1]->GetWidth() + 2, r.y + r.h - m_pFocusBitmaps[1]->GetHeight() + 2);

	pDC->SetClipRect(pcr);
}

// for new UI: ve anh dau tien khac NULL, dung co goc, can giua nut
static void MBDrawBitmapCenter(MDrawContext* pDC, MRECT& r, MBitmap** ppBitmaps, int nCount)
{
	MBitmap* pBmp = NULL;
	for (int i = 0; i < nCount; i++)
	{
		if (ppBitmaps[i] != NULL) { pBmp = ppBitmaps[i]; break; }
	}
	if (pBmp == NULL) return;

	int w = pBmp->GetWidth();
	int h = pBmp->GetHeight();
	if (w > r.w) w = r.w;
	if (h > r.h) h = r.h;

	pDC->SetBitmap(pBmp);
	pDC->Draw(r.x + (r.w - w) / 2, r.y + (r.h - h) / 2, w, h);
}

void MBButtonLook::OnDownDraw(MButton* pButton, MDrawContext* pDC)
{
	MRECT r = pButton->GetInitialClientRect();
	if (GetCustomLook())
	{
		DrawBitmapButtonCustom1(pDC, r, m_pDownBitmaps, true, m_bStretch);
		r = pButton->GetClientRect();
		r.x += 1;
		r.y += 1;
		r.w += 1;
		r.h += 1;
	}
	else
	{
		// for new UI
		if (m_bCenterBitmap) MBDrawBitmapCenter(pDC, r, m_pDownBitmaps, 9);
		else DrawBitmapFrame9(pDC, r, m_pDownBitmaps, m_bStretch, GetScale());
		r = pButton->GetClientRect();
	}
	if (pButton->IsFocus() == true) DrawFocus(pDC, r);

	DrawText(pButton, pDC, r, pButton->m_szName, pButton->GetAlignment());
}

void MBButtonLook::OnUpDraw(MButton* pButton, MDrawContext* pDC)
{
	if (pButton->GetType() == MBT_PUSH2 && pButton->GetCheck())
	{
		OnDownDraw(pButton, pDC);
		return;
	}

	MRECT r = pButton->GetInitialClientRect();
	// for new UI
	if (GetCustomLook())
		DrawBitmapButtonCustom1(pDC, r, m_pUpBitmaps, false, m_bStretch);
	else if (m_bCenterBitmap)
		MBDrawBitmapCenter(pDC, r, m_pUpBitmaps, 9);
	else
		DrawBitmapFrame9(pDC, r, m_pUpBitmaps, m_bStretch, GetScale());
	if (pButton->IsFocus() == true) DrawFocus(pDC, r);

	DrawText(pButton, pDC, pButton->GetClientRect(), pButton->m_szName, pButton->GetAlignment());
}

void MBButtonLook::OnOverDraw(MButton* pButton, MDrawContext* pDC)
{
	if (pButton->GetType() == MBT_PUSH2)
	{
		if (pButton->GetCheck()) OnDownDraw(pButton, pDC);
		else OnUpDraw(pButton, pDC);
		return;
	}

	MRECT r = pButton->GetInitialClientRect();
	// for new UI
	if (GetCustomLook())
		DrawBitmapButtonCustom1(pDC, r, m_pOverBitmaps, false, m_bStretch);
	else if (m_bCenterBitmap)
		MBDrawBitmapCenter(pDC, r, m_pOverBitmaps, 9);
	else
		DrawBitmapFrame9(pDC, r, m_pOverBitmaps, m_bStretch, GetScale());
	if (pButton->IsFocus() == true) DrawFocus(pDC, r);

	DrawText(pButton, pDC, pButton->GetClientRect(), pButton->m_szName, pButton->GetAlignment());
}

void MBButtonLook::OnDisableDraw(MButton* pButton, MDrawContext* pDC)
{
	OnUpDraw(pButton, pDC);
}

void MBButtonLook::OnDraw(MButton* pButton, MDrawContext* pDC)
{
	if (GetWireLook())
	{
		MRECT rect = pButton->GetInitialClientRect();

		// Background
		pDC->SetColor(MCOLOR(DEFCOLOR_WIDGET_BG));
		pDC->FillRectangle(rect);

		// Dropdown arrow button area
		MRECT rectButtonBmp;
		rectButtonBmp.x = rect.x + rect.w - rect.h + 1;
		rectButtonBmp.y = rect.y + 1;
		rectButtonBmp.w = rect.h - 2;
		rectButtonBmp.h = rect.h - 2;

		if (m_pDownBitmaps[1] == NULL) return;		// thieu anh thi bo qua, khoi sap

		// chon anh theo trang thai
		MBitmap* pArrow = m_pDownBitmaps[1];
		if (pButton->GetComboDropped())
		{
			if (m_pDownBitmaps[2]) pArrow = m_pDownBitmaps[2];
		}
		else if (pButton->IsFocus())
		{
			if (m_pDownBitmaps[0]) pArrow = m_pDownBitmaps[0];
		}

		if (m_bCenterBitmap)
		{
			// ve dung co goc, can giua o vuong ben phai
			int nBmpW = pArrow->GetWidth();
			int nBmpH = pArrow->GetHeight();
			pDC->SetBitmap(pArrow);
			pDC->Draw(rect.x + rect.w - rect.h / 2 - nBmpW / 2,
				rect.y + (rect.h - nBmpH) / 2,
				nBmpW, nBmpH);
		}
		else
		{
			float fScale = (float)(rect.h - 2) / (float)pArrow->GetHeight();
			HLineBitmap(pDC, rectButtonBmp.x, rectButtonBmp.y, rectButtonBmp.w, pArrow, false, fScale);
		}

		// Text
		MRECT rectText = rect;
		rectText.w -= rect.h;
		pDC->SetColor(MCOLOR(DEFCOLOR_MEDIT_TEXT));
		pDC->Text(rectText, pButton->m_szName, pButton->GetAlignment());

		// Border - state-dependent color
		if (pButton->GetComboDropped() || pButton->IsFocus())
			pDC->SetColor(MCOLOR(DEFCOLOR_ACCENT));
		else if (pButton->IsMouseOver())
			pDC->SetColor(MCOLOR(DEFCOLOR_BORDER_HOVER));
		else
			pDC->SetColor(MCOLOR(DEFCOLOR_BORDER));
		pDC->Rectangle(rect);
	}
	else
		MButtonLook::OnDraw(pButton, pDC);
}

// ============================================================
// Toggle Switch - replaces old checkbox
// Draws a horizontal toggle track with sliding knob
// ============================================================
#define TOGGLE_TRACK_W	22		// Track width in base 800x600
#define TOGGLE_TRACK_H	12		// Track height
#define TOGGLE_KNOB_SIZE 10		// Knob size (square, slightly smaller than track height)

void MBButtonLook::OnCheckBoxDraw(MButton* pButton, MDrawContext* pDC, bool bPushed)
{
	// Scale toggle dimensions to current resolution
	const float scaleX = MGetWorkspaceWidth() / 800.f;
	const float scaleY = MGetWorkspaceHeight() / 600.f;

	const int trackW = (int)(TOGGLE_TRACK_W * scaleX);
	const int trackH = (int)(TOGGLE_TRACK_H * scaleY);
	const int knobSize = (int)(TOGGLE_KNOB_SIZE * min(scaleX, scaleY));
	const int knobPad = (trackH - knobSize) / 2;	// Padding inside track

	MRECT r = pButton->GetInitialClientRect();

	// Position toggle at left side of widget, vertically centered
	int trackX = r.x + 2;
	int trackY = r.y + (r.h - trackH) / 2;

	// --- Draw Track ---
	if (bPushed)
	{
		// ON state - cyan/accent track
		pDC->SetColor(MCOLOR(DEFCOLOR_TOGGLE_ON));
		pDC->FillRectangle(trackX, trackY, trackW, trackH);
	}
	else
	{
		// OFF state - dark track
		pDC->SetColor(MCOLOR(DEFCOLOR_TOGGLE_OFF));
		pDC->FillRectangle(trackX, trackY, trackW, trackH);

		// OFF border
		if (pButton->IsMouseOver())
			pDC->SetColor(MCOLOR(DEFCOLOR_BORDER_HOVER));
		else
			pDC->SetColor(MCOLOR(DEFCOLOR_BORDER));
		pDC->Rectangle(trackX, trackY, trackW, trackH);
	}

	// --- Draw Knob ---
	int knobX;
	if (bPushed)
		knobX = trackX + trackW - knobSize - knobPad;	// Right side when ON
	else
		knobX = trackX + knobPad;						// Left side when OFF
	int knobY = trackY + knobPad;

	if (bPushed)
		pDC->SetColor(MCOLOR(0xFFFFFFFF));	// White knob when ON
	else
		pDC->SetColor(MCOLOR(0xFF888888));	// Gray knob when OFF
	pDC->FillRectangle(knobX, knobY, knobSize, knobSize);

	// --- Draw Label Text ---
	r = pButton->GetClientRect();
	r.x += trackW + (int)(6 * scaleX);	// Text starts after toggle + gap
	r.w -= trackW + (int)(6 * scaleX);

	// Label color
	if (pButton->IsEnable() == false)
		pDC->SetColor(MCOLOR(DEFCOLOR_DISABLE));
	else if (bPushed)
		pDC->SetColor(MCOLOR(0xFFDDDDDD));	// Brighter when ON
	else if (pButton->IsMouseOver())
		pDC->SetColor(MCOLOR(0xFFBBBBBB));	// Slightly bright on hover
	else
		pDC->SetColor(MCOLOR(0xFF999999));	// Default gray

	if (m_pFont != NULL) pDC->SetFont(m_pFont);
	pDC->Text(r, pButton->m_szName, MAM_LEFT | MAM_VCENTER);
}

MRECT MBButtonLook::GetClientRect(MButton* pButton, MRECT& r)
{
	float fScale = GetScale();

	int al = (int)(fScale * GETWIDTH(m_pUpBitmaps[3]));
	int au = (int)(fScale * GETHEIGHT(m_pUpBitmaps[7]));
	int ar = (int)(fScale * GETWIDTH(m_pUpBitmaps[5]));
	int ab = (int)(fScale * GETHEIGHT(m_pUpBitmaps[1]));
	return MRECT(r.x + al, r.y + au, r.w - (al + ar), r.h - (au + ab));
}