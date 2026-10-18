#include "stdafx.h"
#include "MBLabelLook.h"
#include "MBitmapDrawer.h"
#include <mmsystem.h>

// logic for text marquee scroll when text is wider than the label and mouse is hovering over it
void MBLabelLook::OnDraw(MLabel* pLabel, MDrawContext* pDC)
{
	MRECT r = pLabel->GetInitialClientRect();
	if (pLabel->GetFont() != NULL) pDC->SetFont(pLabel->GetFont());
	else if (m_pFont != NULL) pDC->SetFont(m_pFont);
	pDC->SetColor(pLabel->GetTextColor());

	// --- Marquee scroll for overflow text ---
	MFont* pFont = pDC->GetFont();
	if (pFont)
	{
		int nTextWidth = pFont->GetWidth(pLabel->m_szName);
		if (nTextWidth > r.w && pLabel->m_bMouseHover)
		{
			int nOverflow = nTextWidth - r.w + 20;
			DWORD nTime = timeGetTime();
			int nCycleMs = nOverflow * 25;
			if (nCycleMs < 1500) nCycleMs = 1500;
			int nPhase = nTime % (nCycleMs * 2);
			int nOffset;
			if (nPhase < nCycleMs)
				nOffset = (int)((float)nPhase / nCycleMs * nOverflow);
			else
				nOffset = (int)((float)(nCycleMs * 2 - nPhase) / nCycleMs * nOverflow);

			MRECT sr = pLabel->GetScreenRect();
			MRECT prevClip = pDC->GetClipRect();
			pDC->SetClipRect(sr);
			r.x -= nOffset;
			r.w = nTextWidth + 20;
			pDC->Text(r, pLabel->m_szName, pLabel->GetAlignment());
			pDC->SetClipRect(prevClip);
			return;
		}
	}

	// --- Normal draw ---
	pDC->Text(r, pLabel->m_szName, pLabel->GetAlignment());
}

MBLabelLook::MBLabelLook(void)
{
	m_FontColor = MCOLOR(255, 255, 255);
}

MRECT MBLabelLook::GetClientRect(MLabel* pLabel, MRECT& r)
{
	return r;
}