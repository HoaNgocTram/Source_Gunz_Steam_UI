#include "stdafx.h"
#include "MFrame.h"
#include "MColorTable.h"
#include "Mint.h"

//#define MTITLEBAR_HEIGHT		18	//800*600 ±âÁØ ³ôÀÌ
#define MTITLEBAR_HEIGHT		(int(18 * MGetWorkspaceHeight() / 600.f))	//ÇØ»óµµ¿¡ µû¶ó ÀûÁ¤ ¹èÀ² Àû¿ë

/*
class MTitleBar : public MWidget{
protected:
	virtual void OnDraw(MDrawContext* pDC){
		pDC->SetColor(MCOLOR(DEFCOLOR_MMENUBAR_PLANE));
		pDC->FillRectangle(m_Rect);
		pDC->SetColor(MCOLOR(DEFCOLOR_MMENUBAR_TEXT));
		pDC->Text(m_Rect, m_szName, MAM_LEFT);
	}
public:
	MTitleBar(char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL)
		: MWidget(szName, pParent, pListener){}
	//virtual ~MTitleBar(void){}
};

class MClient : public MWidget{
protected:
	virtual void OnDraw(MDrawContext* pDC){
	}
public:
	MClient(char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL)
		: MWidget(szName, pParent, pListener){}
};
*/

#define FRAME_W	300
#define FRAME_X	10
#define FRAME_H	300
#define FRAME_Y	10

void MFrameLook::OnDraw(MFrame* pFrame, MDrawContext* pDC)
{
	MRECT r = pFrame->GetInitialClientRect();
	pDC->SetColor(MCOLOR(DEFCOLOR_MFRAME_PLANE));
	pDC->FillRectangle(r);
	pDC->SetColor(128,128,128,255);
	pDC->Rectangle(r);

	/*
	// Frame Board
	MRECT r(0, 0, pFrame->m_Rect.w, pFrame->m_Rect.h);
	pDC->SetColor(MCOLOR(DEFCOLOR_MFRAME_PLANE));
	pDC->FillRectangle(r);
	pDC->SetColor(MCOLOR(DEFCOLOR_MFRAME_OUTLINE));
	pDC->Rectangle(r);
	*/

	// TitleBar
	if(pFrame->m_bTitleBar==true){
		MRECT TitleBarRect(r.x, r.y, r.w, MTITLEBAR_HEIGHT);
		pDC->SetColor(MCOLOR(DEFCOLOR_MMENUBAR_PLANE));
		pDC->FillRectangle(TitleBarRect);
		pDC->SetColor(MCOLOR(DEFCOLOR_MMENUBAR_TEXT));
		pDC->Text(TitleBarRect, pFrame->m_szName, MAM_LEFT);
	}
}

MRECT MFrameLook::GetClientRect(MFrame* pFrame, MRECT& r)
{
	int nTitleBarHeight = (pFrame->m_bTitleBar==true?MTITLEBAR_HEIGHT:0);
	return MRECT(r.x+1, r.y + nTitleBarHeight, r.w-2, r.h - nTitleBarHeight-2);
}

IMPLEMENT_LOOK(MFrame, MFrameLook)


bool MFrame::OnEvent(MEvent* pEvent, MListener* pListener)
{
	if (IsFocusEnable() == false)
		return false;

	//if(m_bTitleBar==true) return false;	// Å¸ÀÌÆ²¹Ù°¡ ¾ø´Â ÇÁ·¹ÀÓÀº ÀÌº¥Æ®¸¦ ¹ÞÁö ¾Ê´Â´Ù.

	MRECT TitleBarRect(0, 0, m_Rect.w, MTITLEBAR_HEIGHT);
	MRECT WidgetRect(0, 0, m_Rect.w, m_Rect.h);
	MPOINT sp = MClientToScreen(this, pEvent->Pos);
	//MPOINT sp(m_Rect.x, m_Rect.y);
	switch(pEvent->nMessage){
	case MWM_LBUTTONDOWN:

		if(m_bTitleBar==true && TitleBarRect.InPoint(pEvent->Pos)==true) {

			if ( m_BtnClose.m_Rect.InPoint(pEvent->Pos)==true)
			{
				m_BtnClose.m_bLButtonDown = true;
			}
			else if (m_BtnMinimize.m_Rect.InPoint(pEvent->Pos)==true)
			{
				m_BtnMinimize.m_bLButtonDown = true;
			}
			else
			{
				SetCapture();
				m_bDragWidget = true;
				MPOINT wp = MClientToScreen(GetParent(), MPOINT(m_Rect.x, m_Rect.y));
				sp.x -= wp.x;
				sp.y -= wp.y;
				m_DragPoint = sp;
			}
			return true;
		}
		else if(WidgetRect.InPoint(pEvent->Pos)==true){
			return true;
		}
		break;
	case MWM_LBUTTONUP:
		if (m_bTitleBar==true && m_BtnClose.m_Rect.InPoint(pEvent->Pos)==true)
		{
			if (m_BtnClose.m_bLButtonDown==true) OnCloseButtonClick();
		}
		else if (m_bTitleBar==true && m_BtnMinimize.m_Rect.InPoint(pEvent->Pos)==true)
		{
			if (m_BtnMinimize.m_bLButtonDown==true) OnMinimizeButtonClick();
		}

		m_BtnClose.m_bLButtonDown = m_BtnMinimize.m_bLButtonDown = false;

		if(m_bDragWidget==true){
			ReleaseCapture();
			m_bDragWidget = false;
			return true;
		}
		break;
	case MWM_MOUSEMOVE:
		if(m_bDragWidget==true){
			sp.x -= m_DragPoint.x;
			sp.y -= m_DragPoint.y;
			if(sp.x<0) sp.x = 0;
			if(sp.y<0) sp.y = 0;
			if(sp.x+m_Rect.w>MGetWorkspaceWidth()-1) sp.x = MGetWorkspaceWidth()-m_Rect.w-1;
			if(sp.y+m_Rect.h>MGetWorkspaceHeight()-1) sp.y = MGetWorkspaceHeight()-m_Rect.h-1;
			MPOINT p = MScreenToClient(GetParent(), sp);
			if (m_bMovable == true) {
				SetPosition(p.x, p.y);
			}
			
			return true;
		}
		else if(m_bTitleBar==true)
		{
			if(m_BtnClose.m_Rect.InPoint(pEvent->Pos)==true)
			{
				if(m_BtnClose.m_bMouseOver==false) m_BtnClose.m_bMouseOver = true;
			}
			else
			{
				if(m_BtnClose.m_bMouseOver==true) m_BtnClose.m_bMouseOver = false;
			}
			if(m_BtnMinimize.m_Rect.InPoint(pEvent->Pos)==true)
			{
				if(m_BtnMinimize.m_bMouseOver==false) m_BtnMinimize.m_bMouseOver = true;
			}
			else
			{
				if(m_BtnMinimize.m_bMouseOver==true) m_BtnMinimize.m_bMouseOver = false;
			}

		}
		break;
	case MWM_LBUTTONDBLCLK:
		// m_BtnClose °¡ ÃÊ±âÈ­°¡ ¾ÈµÇ¾î¼­ debug ¿¡¼­´Â µ¿ÀÛÀ» ¾ÈÇÔ. ÀÏ´Ü ÁÖ¼®
		/*
		if (m_bTitleBar==true && m_BtnClose.m_Rect.InPoint(pEvent->Pos)==true)
		{
			OnCloseButtonClick();
		}
		else if (m_bTitleBar==true && m_BtnMinimize.m_Rect.InPoint(pEvent->Pos)==true)
		{
			OnMinimizeButtonClick();
		}

		// Shade
		else if(m_bTitleBar==true && TitleBarRect.InPoint(pEvent->Pos)==true){
			SetShade(!m_bShade);
			return true;
		}

		else if(WidgetRect.InPoint(pEvent->Pos)==true){
			return true;
		}
		return false;
		*/
		break;
	}
	return false;
}

//bool MFrame::OnShow(void)
//{
//	/*
//	// ¿¡µðÆ® ÄÁÆ®·Ñ¿¡ ¿ì¼±¼øÀ§¸¦ µÎ°í...
//	for(int i=0; i<GetChildCount(); i++){
//		MWidget* pWidget = GetChild(i);
//		if(strcmp(pWidget->GetClassName(), "Edit")==0){
//			pWidget->SetFocus();
//			return true;
//		}
//	}
//	*/
//	// ChildrenÀÇ Ã¹¹øÂ° Widget¿¡°Ô Æ÷Ä¿½º¸¦ ÁØ´Ù.
//	if(GetChildCount()>0) GetChild(0)->SetFocus();
//	return true;
//}
bool MFrame::OnShow(void)
{
	// Bổ sung: Xử lý ngắt vẽ tức thì khi Frame/Tab bị ẩn (Show(false))
	if (!IsVisible())
	{
		m_bFadingOut = false; // Tắt trạng thái mờ dần (nếu có)
		SetOpacity(0);        // Ép Opacity về 0 để MWidget::Draw bỏ qua không vẽ nữa
		return true;
	}

	// Khi mở Frame (Show(true)): Reset lại độ mờ tối đa
	SetOpacity(255);

	/*	// ¿¡µðÆ® ÄÁÆ®·Ñ¿¡ ¿ì¼±¼øÀ§¸¦ µÎ°í...	for(int i=0; i<GetChildCount(); i++){		MWidget* pWidget = GetChild(i);		if(strcmp(pWidget->GetClassName(), "Edit")==0){			pWidget->SetFocus();			return true;		}	}	*/
	// ChildrenÀÇ Ã¹¹øÂ° Widget¿¡°Ô Æ÷Ä¿½º¸¦ ÁØ´Ù.
	if (GetChildCount() > 0) GetChild(0)->SetFocus();

	return true;
}

void MFrame::FadeOutAndClose(float fDuration)
{
	m_bFadingOut = true;
	m_fFadeDuration = fDuration;
	m_fFadeStartTime = (float)timeGetTime() / 1000.0f; // Lấy thời gian hiện tại (giây)
}

void MFrame::OnRun(void)
{
	MWidget::OnRun();

	if (m_bFadingOut)
	{
		float fCurrentTime = (float)timeGetTime() / 1000.0f;
		float fElapsed = fCurrentTime - m_fFadeStartTime;

		if (fElapsed >= m_fFadeDuration)
		{
			// Đã fade xong -> Ẩn thực sự và reset
			m_bFadingOut = false;
			SetOpacity(255);
			Show(false);
		}
		else
		{
			// Tính toán độ mờ giảm dần từ 255 về 0
			float fProgress = fElapsed / m_fFadeDuration;
			unsigned char nAlpha = (unsigned char)(255.0f * (1.0f - fProgress));
			SetOpacity(nAlpha);
		}
	}
}

MFrame::MFrame(const char* szName, MWidget* pParent, MListener* pListener)
	: MWidget(szName, pParent, pListener)
{
	LOOK_IN_CONSTRUCTOR()

		SetBounds(MRECT(FRAME_X, FRAME_Y, FRAME_W, FRAME_H));
	m_OldRect = m_Rect;

	Show(false);

	// =========================================================================
	// Bật hiệu ứng Fade Out mượt mà khi đóng cửa sổ
	m_bFadingOut = false;
	m_fFadeStartTime = 0.0f;
	m_fFadeDuration = 0.15f;
	SetOpacity(255);
	// =========================================================================

	m_bDragWidget = false;
	m_bZOrderChangable = true;

	// Resizable
	m_bResizable = true;
	m_bMovable = true;

	m_bShade = false;
	m_bCanShade = true;

	m_bTitleBar = true;

	m_nMinWidth = 300;
	m_nMinHeight = 200;
}

MFrame::~MFrame(void)
{

}

void MFrame::OnSize(int w, int h)
{
	ResizeBtnsByAnchors(w, h);
	m_OldRect = m_Rect;
}

bool MFrame::OnCommand(MWidget* pWindow, const char* szMessage)
{

	return false;
}

void MFrame::SetShade(bool bShade)
{
	if (!m_bCanShade) return;

	if (m_bShade == bShade) return;

	if(m_bShade==false){
		m_BeforeShade = MSIZE(m_Rect.w, m_Rect.h);
		SetSize(m_Rect.w, MTITLEBAR_HEIGHT);
		m_bShade = true;
		m_bResizable = false;
	}
	else{
		SetSize(m_BeforeShade);
		m_bShade = false;
		m_bResizable = true;
	}
}

void ResizeByAnchors(MFrameBtn* pFrameBtn, MRECT parentRect, int w, int h)
{
	MRECT r = pFrameBtn->m_Rect;
	if(pFrameBtn->m_Anchors.m_bLeft==true && pFrameBtn->m_Anchors.m_bRight==true)
	{
		r.w += (w-parentRect.w);
	}
	else if(pFrameBtn->m_Anchors.m_bRight==true)
	{
		r.x += (w-parentRect.w);
	}
	if(pFrameBtn->m_Anchors.m_bTop==true && pFrameBtn->m_Anchors.m_bBottom==true)
	{
		r.h += (h-parentRect.h);
	}
	else if(pFrameBtn->m_Anchors.m_bBottom==true)
	{
		r.y += (h-parentRect.h);
	}
	pFrameBtn->m_Rect = r;
}

void MFrame::ResizeBtnsByAnchors(int w, int h)
{
	ResizeByAnchors(&m_BtnClose, m_OldRect, w, h);
	ResizeByAnchors(&m_BtnMinimize, m_OldRect, w, h);
}

void MFrame::OnCloseButtonClick()
{
	//Show(false);
	FadeOutAndClose(0.15f);
}
void MFrame::OnMinimizeButtonClick()
{
	SetShade(!m_bShade);
}
