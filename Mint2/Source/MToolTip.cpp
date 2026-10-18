#include "stdafx.h"
#include "MToolTip.h"
#include "MColorTable.h"
#include "Mint.h"

#define TOOLTIP_WIDTH_GAP	20
#define TOOLTIP_HEIGHT_GAP	5

void MToolTip::OnDraw(MDrawContext* pDC)
{
	MRECT r = GetClientRect();
	/*
	MFont* pFont = MFontManager::Get(NULL);
	if(pFont==NULL) return;
	pDC->SetFont(pFont);
	*/
	pDC->SetColor(MCOLOR(DEFCOLOR_MTOOLTIP_PLANE));
	pDC->FillRectangle(r);
	pDC->SetColor(MCOLOR(DEFCOLOR_MTOOLTIP_OUTLINE));
	pDC->Rectangle(r);
	pDC->SetColor(MCOLOR(DEFCOLOR_MTOOLTIP_TEXT));
	char* szName;
	if (m_bUseParentName == true) szName = GetParent()->m_szName;
	else szName = m_szName;

	// for new UI: tach theo \n de ve nhieu dong
	MFont* pFont = GetFont();
	int nLineHeight = (pFont != NULL) ? pFont->GetHeight() : 0;

	if (nLineHeight <= 0 || strchr(szName, '\n') == NULL)
	{
		pDC->TextWithHighlight(r, szName, (MAM_HCENTER | MAM_VCENTER));
		return;
	}

	char szBuf[MWIDGET_NAME_LENGTH];
	strncpy(szBuf, szName, sizeof(szBuf) - 1);
	szBuf[sizeof(szBuf) - 1] = 0;

	int nY = r.y + TOOLTIP_HEIGHT_GAP / 2;
	char* szLine = szBuf;

	while (szLine != NULL && *szLine != 0)
	{
		char* szNext = strchr(szLine, '\n');
		if (szNext != NULL) *szNext = 0;

		MRECT rLine(r.x, nY, r.w, nLineHeight);
		pDC->TextWithHighlight(rLine, szLine, (MAM_HCENTER | MAM_VCENTER));

		nY += nLineHeight;
		szLine = (szNext != NULL) ? szNext + 1 : NULL;
	}
	// 라인수 만큼 늘려 주기..
	//pDC->TextWithHighlight(r.x+TOOLTIP_WIDTH_GAP/2+1, r.y+TOOLTIP_HEIGHT_GAP/2, szName);
}

MToolTip::MToolTip(const char* szName, MWidget* pParent)
: MWidget(szName, pParent, NULL)
{
	_ASSERT(pParent!=NULL);
	SetText(szName);

	m_bClipByParent = false;

	SetFocusEnable(false);

	Show(false);
}

MToolTip::~MToolTip(void)
{
}

void MToolTip::SetBounds(void)
{
	MFont* pFont = GetFont();

	char szName[MWIDGET_NAME_LENGTH];
	RemoveAnd(szName, m_bUseParentName==true?GetParent()->m_szName:m_szName);
	// for new UI: rong theo dong dai nhat, cao theo so dong
	int nWidth = 0;
	int nLineCount = 0;
	{
		char szBuf[MWIDGET_NAME_LENGTH];
		strncpy(szBuf, szName, sizeof(szBuf) - 1);
		szBuf[sizeof(szBuf) - 1] = 0;

		char* szLine = szBuf;
		while (szLine != NULL)
		{
			char* szNext = strchr(szLine, '\n');
			if (szNext != NULL) *szNext = 0;

			int nLineWidth = pFont->GetWidthWithoutAmpersand(szLine);
			if (nLineWidth > nWidth) nWidth = nLineWidth;
			++nLineCount;

			szLine = (szNext != NULL) ? szNext + 1 : NULL;
		}
	}
	if (nLineCount < 1) nLineCount = 1;

	int nHeight = pFont->GetHeight() * nLineCount;
	int x, y;
	MRECT pr = GetParent()->GetClientRect();
	MRECT spr = MClientToScreen(GetParent(), pr);
	if(spr.x+(nWidth+TOOLTIP_WIDTH_GAP/2)<=MGetWorkspaceWidth())
		x = pr.x+TOOLTIP_WIDTH_GAP/2+1;
	else{
		MPOINT p = MScreenToClient(GetParent(), MPOINT(MGetWorkspaceWidth()-(nWidth+TOOLTIP_WIDTH_GAP/2), 0));
		x = p.x;
	}
	y = pr.y-(nHeight+TOOLTIP_HEIGHT_GAP);
	MPOINT p = MClientToScreen(GetParent(), MPOINT(0, y));
	if(p.y<0){
		y = p.y+pr.h+(nHeight+TOOLTIP_HEIGHT_GAP);
		if(y>MGetWorkspaceHeight()) y = 0;
		p = MScreenToClient(GetParent(), MPOINT(0, y));
		y = p.y;
	}
	MWidget::SetBounds(MRECT(x-TOOLTIP_WIDTH_GAP/2, y, nWidth+TOOLTIP_WIDTH_GAP, nHeight+TOOLTIP_HEIGHT_GAP));
}

void MToolTip::SetText(const char* szText)
{
	if(szText==NULL) m_bUseParentName = true;
	else m_bUseParentName = false;
	if(szText!=NULL) MWidget::SetText(szText);
	SetBounds();
}

bool MToolTip::IsUseParentName(void)
{
	return m_bUseParentName;
}
