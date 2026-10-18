#include "stdafx.h"
#include "MResource.h"
#include "MWidget.h"
#include "MToolTip.h"
#include "Mint.h"
#include "MButton.h"
#include "MEdit.h"


inline bool MIsSameWidgetClass(MWidget* pWidget, const char* szClassName)
{
	if (strcmp(pWidget->GetClassName(), szClassName) == 0) return true;
	return false;
}

inline bool MIsSameWidgetClass(MWidget* pWidgetA, MWidget* pWidgetB)
{
	if (strcmp(pWidgetA->GetClassName(), pWidgetB->GetClassName()) == 0) return true;
	return false;
}

MWidget* MWidget::m_pCapturedWidget = NULL;
MWidget* MWidget::m_pFocusedWidget = NULL;

static int g_nWidgetCount=0;

void MWidget::MakeLocalEvent(MEvent* pLoalEvent, const MEvent* pEvent)
{
	memcpy(pLoalEvent, pEvent, sizeof(MEvent));

	MRECT sr = GetScreenRect();
	pLoalEvent->Pos.x -= sr.x;
	pLoalEvent->Pos.y -= sr.y;
}

bool MWidget::EventResize(MEvent* pEvent)
{
	MEvent LocalEvent;
	MakeLocalEvent(&LocalEvent, pEvent);

	// Widget Resize
#define RESIZER_SIZE	4
	if(m_bResizable==true){
		MPOINT sp = pEvent->Pos;
		MWidget* pParent = GetParent();
		if(pParent==NULL) return false;

		if(pEvent->nMessage==MWM_LBUTTONDOWN){
			m_nResizeSide = 0;
			if(LocalEvent.Pos.y>=-RESIZER_SIZE && LocalEvent.Pos.y<=RESIZER_SIZE){
				if(LocalEvent.Pos.x>=-RESIZER_SIZE && LocalEvent.Pos.x<=m_Rect.w+RESIZER_SIZE)
					m_nResizeSide |= 1;
			}
			if(LocalEvent.Pos.x>=m_Rect.w-RESIZER_SIZE && LocalEvent.Pos.x<=m_Rect.w+RESIZER_SIZE){
				if(LocalEvent.Pos.y>=-RESIZER_SIZE && LocalEvent.Pos.y<=m_Rect.h+RESIZER_SIZE)
					m_nResizeSide |= 2;
			}
			if(LocalEvent.Pos.y>=m_Rect.h-RESIZER_SIZE && LocalEvent.Pos.y<=m_Rect.h+RESIZER_SIZE){
				if(LocalEvent.Pos.x>=-RESIZER_SIZE && LocalEvent.Pos.x<=m_Rect.w+RESIZER_SIZE)
					m_nResizeSide |= 4;
			}
			if(LocalEvent.Pos.x>=-RESIZER_SIZE && LocalEvent.Pos.x<=RESIZER_SIZE){
				if(LocalEvent.Pos.y>=-RESIZER_SIZE && LocalEvent.Pos.y<=m_Rect.h+RESIZER_SIZE)
					m_nResizeSide |= 8;
			}
			if(m_nResizeSide!=0){
				SetCapture();
				//
				return true;
			}
		}
		if(m_nResizeSide!=0 && pEvent->nMessage==MWM_LBUTTONUP){
			ReleaseCapture();
			m_nResizeSide = 0;
			return true;
		}
		if(pEvent->nMessage==MWM_MOUSEMOVE){
			if(m_nResizeSide!=0){
				MPOINT pp = MScreenToClient(pParent, sp);
				MPOINT tp = MScreenToClient(this, sp);
				MRECT r = m_Rect;
				if(m_nResizeSide&1){
					r.h += (r.y - pp.y);
					r.y = pp.y;
				}
				if(m_nResizeSide&2){
					r.w = tp.x;
				}
				if(m_nResizeSide&4){
					r.h = tp.y;
				}
				if(m_nResizeSide&8){
					r.w += (r.x - pp.x);
					r.x = pp.x;
				}
				SetPosition(r.x, r.y);
// ÀÓ½Ã·Î À§Á¬ ÃÖ¼Ò Å©±â¸¦ ÁöÁ¤, ³ªÁß¿¡ ¸â¹ö·Î ºüÁ®ÁÙ ÇÊ¿ä°¡ ÀÖ´Ù.
				if(r.w<m_nMinWidth) r.w = m_nMinWidth;
				if(r.h<m_nMinHeight) r.h = m_nMinHeight;
				SetSize(r.w, r.h);
				return true;
			}
		}
	}

	return false;
}


void MWidget::InsertChild(MWidget* pWidget)
{
	m_Children.InsertBefore(pWidget);
	pWidget->m_pParent = this;
}

void MWidget::AddChild(MWidget* pWidget)
{
	m_Children.Add(pWidget);
	pWidget->m_pParent = this;
}

void MWidget::RemoveChild(MWidget* pWidget)
{
	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pCurWnd = m_Children.Get(i);
		if(pCurWnd==pWidget){
			pWidget->m_pParent = NULL;
			m_Children.Delete(i);
			return;
		}
	}
}

void MWidget::AddExclusive(MWidget* pWidget)
{
	m_Exclusive.Add(pWidget);
}

bool MWidget::RemoveExclusive(MWidget* pWidget)
{
	for(int i=0; i<m_Exclusive.GetCount(); i++){
		MWidget* pThis = m_Exclusive.Get(i);
		if(pThis==pWidget){
			m_Exclusive.Delete(i);
			return true;
		}
	}
	return false;
}

MWidget* MWidget::GetLatestExclusive(void)
{
	if(m_Exclusive.GetCount()>0) return m_Exclusive.Get(m_Exclusive.GetCount()-1);
	return NULL;
}

/*
void MWidget::UseAcceleractorAndCharacter(char* szText)
{
	if(szText==NULL){
		m_nAccelerator = -1;
		return;
	}
	char szAnd[2] = {GetAndChar(szText), 0};
	_strupr(szAnd);
	m_nAccelerator = szAnd[0];
	if(m_nAccelerator==0) m_nAccelerator = -1;
}
*/

MWidget::MWidget(const char* szName, MWidget* pParent, MListener* pListener)
{
	g_nWidgetCount++;
	m_nID = g_nWidgetCount;

	if(szName==NULL) m_szName[0] = NULL;
	else{
		if(strlen(szName)<MWIDGET_NAME_LENGTH){
			strcpy(m_szName, szName);
		}
		else{
			// MWIDGET_NAME_LENGTHº¸´Ù Å« ¹®ÀÚ¿­Àº Àß¶ó ¹ö¸².
			memcpy(m_szName, szName, MWIDGET_NAME_LENGTH-4);
			m_szName[MWIDGET_NAME_LENGTH-4] = '.';
			m_szName[MWIDGET_NAME_LENGTH-3] = '.';
			m_szName[MWIDGET_NAME_LENGTH-2] = '.';
			m_szName[MWIDGET_NAME_LENGTH-1] = 0;
		}
	}

	// Default Region
	m_Rect.x = 0;
	m_Rect.y = 0;
	m_Rect.w = 100;
	m_Rect.h = 100;

	m_pParent = pParent;
	if(m_pParent!=NULL) m_pParent->AddChild(this);

	m_pListener = pListener;

	m_pCursor = NULL;		// NULL Cursor is Default Cursor
	m_pFont = NULL;			// NULL Font is Default Font

	m_bVisible = true;
	m_bEnable = true;
	m_bFocusEnable = false;	// Default Focus Disabled

	m_pToolTip = NULL;

	//EnableAccelerator(true);
	//SetLabelAccelerator();
	SetAccelerator(0);

	//Show(true, false);

	m_bZOrderChangable = false;
	m_bResizable = false;	// µðÆúÆ®´Â ¸®»çÀÌÁî ºÒ°¡´É
	m_nResizeSide = 0;

	m_bClipByParent = true;

//	if (m_pParent!=NULL) m_nOpacity = m_pParent->GetOpacity();
//	else 
		m_nOpacity = 255;

//	m_bVisibleChildren = true;

	m_bEnableDesignerMode = true;
	m_nDMDragWidget = 0;
	m_bModifiedByDesigner = false;
	m_bAddedByDesigner = false;
	//m_nID = -1;

	m_BoundsAlignment = MAM_NOTALIGN;

	/*
	m_bAnchorLeft = true;
	m_bAnchorRight = false;
	m_bAnchorTop = true;
	m_bAnchorBottom = false;
	*/

	m_nMinWidth = 10;
	m_nMinHeight = 10;

	m_bisListBox = false;

	m_nDebugType = 0;
	m_bEventAcceleratorCall = false;

	m_IDLRect = MRECT(-1,-1,-1,-1);

}

MWidget::~MWidget(void)
{
	ReleaseExclusive();

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pWidget = m_Children.Get(i);
		pWidget->m_pParent = NULL;	// ºÎ¸ð°¡ ¸ÕÀú Á×À» °æ¿ì Children's Parent Link »èÁ¦
	}

	/*
	// ±âÁ¸ ÄÚµå°¡ ¸í½ÃÀûÀ¸·Î Child¸¦ °¢°¢ °¡Áö°í ÀÖÀ»¶§ ¹®Á¦°¡ ¹ß»ýÇÑ´Ù.
	// °¡Áö°í ÀÖ´Â Childrenµµ ¸ðµÎ »èÁ¦
	while(m_Children.GetCount()>0){
		MWidget* pWidget = m_Children.Get(0);
		delete pWidget;
		//RemoveChild(pWidget);
	}
	*/

	if(m_pParent!=NULL) m_pParent->RemoveChild(this);
	if(MWidget::m_pCapturedWidget==this) MWidget::m_pCapturedWidget = NULL;
	if(MWidget::m_pFocusedWidget==this) MWidget::m_pFocusedWidget = NULL;
	DetachToolTip();
}

void MWidget::OnRun(void)
{
}

void MWidget::OnDraw(MDrawContext* pDC)
{
	// Draw Nothing
	pDC->SetColor(MCOLOR(196, 196, 196));
	pDC->FillRectangle(GetInitialClientRect());
}

/*
void MWidget::OnDraw3D(MDrawContext3D* pDC)
{
}
*/

bool MWidget::OnEvent(MEvent* pEvent, MListener* pListener)
{
	return false;
}

void MWidget::Run(void)
{
	OnRun();

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pCurWnd = m_Children.Get(i);
		pCurWnd->Run();
	}
}
#include "MDebug.h"

void MWidget::Draw(MDrawContext* pDC)
{
	if(m_bVisible==false) return;

//	MBeginProfile(100+m_nID,m_szIDLName);

	// Opacity ¼³Á¤
	unsigned char nLastOpacity;
	nLastOpacity = pDC->GetOpacity();

	MRECT sr = GetScreenRect();
	pDC->SetOrigin(MPOINT(sr.x, sr.y));

	// ±×¸®±â Àü ÃÊ±âÈ­
	if(m_pFont!=NULL) pDC->SetFont(m_pFont);
	else pDC->SetFont(MFontManager::Get(NULL));	// ÆùÆ®°¡ NULLÀÎ °æ¿ì Default Font·Î ¼¼ÆÃÇÑ´Ù.

	pDC->SetOpacity((unsigned char)(nLastOpacity * (float)(m_nOpacity / 255.0f)));
	if(!IsEnable())
		pDC->SetOpacity((unsigned char)(pDC->GetOpacity()*0.70));		// Disable »óÅÂÀÏ¶§ ³Ê¹« ¾îµÎ¿ö¼­ Á» ¹à°Ô ÇßÀ½(µ¿È¯)

	bool bIntersect = true;
	MRECT rectScreen(0, 0, MGetWorkspaceWidth()-1, MGetWorkspaceHeight()-1);
	MRECT PrevClipRect;
	if(GetParent()!=NULL) {
		MRECT parentClipRect = MClientToScreen(GetParent(), GetParent()->GetClientRect());
		bIntersect = rectScreen.Intersect(&PrevClipRect,parentClipRect);
	}else
		PrevClipRect = rectScreen;

	//if(GetParent()!=NULL) PrevClipRect = GetParent()->GetScreenRect();
	MRECT CurrClipRect = GetScreenRect();
	MRECT IntersectClipRect;

	if(m_bClipByParent==true){
		// ºÎ¸ð À§Á¬°ú °øÅëµÇ´Â ¿µ¿ªÀÌ ÀÖÀ» °æ¿ì¿¡¸¸ ±×¸°´Ù.
		if(PrevClipRect.Intersect(&IntersectClipRect, CurrClipRect)==true){
			MRECT test = IntersectClipRect;
			if(IntersectClipRect.w>0 && IntersectClipRect.h>0) {
				pDC->SetClipRect(IntersectClipRect);
				// ±×¸®±â
				OnDraw(pDC);
			}
		}
	}
	else{
		pDC->SetClipRect(CurrClipRect);
		// ±×¸®±â
		OnDraw(pDC);
	}

	// Child Widget Drawing
//	if(m_bVisibleChildren==true){
	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pCurWnd = m_Children.Get(i);
		if(pCurWnd==GetLatestExclusive()) continue;
		if(pCurWnd != NULL ) pCurWnd->Draw(pDC);
	}
	if(GetLatestExclusive()!=NULL) GetLatestExclusive()->Draw(pDC);	// Exclusive WidgetÀ» ¸¶Áö¸·¿¡ ±×¸°´Ù.
//	}

	// Opacity Àç¼³Á¤
	pDC->SetOpacity(nLastOpacity);

//	MEndProfile(100+m_nID);
}

/*
void MWidget::Draw3D(MDrawContext3D* pDC)
{
	if(m_bVisible==true) OnDraw3D(pDC);
	else return;

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pCurWnd = m_Children.Get(i);
		pCurWnd->Draw3D(pDC);
	}
}
*/

void MWidget::DrawAfterWidgets( MDrawContext* pDC )
{
	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pCurWnd = m_Children.Get(i);
		if(pCurWnd != NULL ) pCurWnd->DrawAfterWidgets(pDC);
	}
}

void MWidget::Redraw(void)
{
	Mint::GetInstance()->Update();
}

bool MWidget::Event(MEvent* pEvent)
{
	if(m_bVisible==false) return false;	// º¸ÀÌ´Â À©µµ¿ì¸¸ ÀÌº¥Æ®¸¦ Ã³¸®ÇÒ ¼ö ÀÖ´Ù.

	// Look¿¡¼­ Á¤ÀÇÇÑ Client¿µ¿ªÀÌ ¾Æ´Ñ ½ÇÁ¦ Client ¿µ¿ª
	//MRECT r = GetClientRect();
	MRECT r = GetRect();
	r.x = r.y = 0;

	if(pEvent->nMessage==MWM_LBUTTONDOWN)
		int k=0;

	// ÁÂÅ¬¸¯½Ã ÀÌº¥Æ®°¡ Àü´ÞµÇ¾î¿Â °èÃþÀ» È®ÀÎÇÒ ¼ö ÀÖ´Â ·Î±×
	/*if(pEvent->nMessage==MWM_LBUTTONDOWN)
	{
		MWidget* p = this;
		while (0!=stricmp(p->m_szIDLName, "Shop")) // »óÀ§ ºÎ¸ð ÀÌ¸§..
		{
			mlog("[%s,%s] = ", this->GetClassName(), m_szIDLName);
			p = p->GetParent();
			if (!p) break;
		}
		mlog("\n");
	}*/

	MEvent LocalEvent;
	MakeLocalEvent(&LocalEvent, pEvent);

	if(m_nDebugType==2){
		if(pEvent->nMessage==MWM_LBUTTONDOWN )
			int k=0;
	}
	// Disable»óÅÂ¿¡¼­µµ ÅøÆÁÀº º¸ÀÌ°Ô ÇØÁØ´Ù.
	//if(pEvent->nMessage==MWM_MOUSEMOVE && m_Rect.InPoint(pEvent->Pos)==true && (MWidget::m_pCapturedWidget==NULL || MWidget::m_pCapturedWidget==this)){
	if(r.InPoint(LocalEvent.Pos)==true && (MWidget::m_pCapturedWidget==NULL || MWidget::m_pCapturedWidget==this) && IsVisible()==true){
		m_bMouseHover = true;	// logic for text marquee scroll when text is wider than the label and mouse is hovering over it
		if(m_pCursor!=NULL) MCursorSystem::Set(m_pCursor);
		else MCursorSystem::Set(MCURSOR_ARROW);		// ¾Æ¹«°Íµµ ÁöÁ¤µÇ¾î ÀÖÁö ¾ÊÀ¸¸é Default Cursor·Î...

		if(m_pToolTip!=NULL){
			// ÅøÆÁ º¸¿©ÁÖ±â À§ÇØ ZOrder¸¦ ¹Ù²Ù¸é ¾ÈµÇÁö¸¸, ÇØ°áÃ¥ÀÌ ¾øÀ¸¹Ç·Î ¿ì¼± ÀÌ·¸°Ô ÇØµÐ´Ù.
			SetZOrder(MZ_TOP);	// ÀÌ À©µµ¿ì¸¦ Â÷ÀÏµå À©µµ¿ì ¸Ç ¾Õ¿¡ º¸ÀÌ°Ô ÇØ¼­.. ÅøÆÁÀÌ °¡¸®Áö ¾Ê°Ô..
			m_pToolTip->Show(true);
		}
	}
	else{
		m_bMouseHover = false;	// logic for text marquee scroll when text is wider than the label and mouse is hovering over it
		if(m_pToolTip!=NULL){
			m_pToolTip->Show(false);
		}
	}

	if(m_bEnable==false) return false;

//	if(m_bVisibleChildren==true && GetLatestExclusive()!=NULL){
	if(GetLatestExclusive()!=NULL){
		if(GetLatestExclusive()->Event(pEvent)==true) return true;
		if(GetLatestExclusive()!=NULL) return false;	// ÀÌº¥Æ® ÇÚµé·¯¿¡ ÀÇÇØ Exclusive°¡ Ç®¸®´Â °æ¿ì°¡ ÀÖÀ¸¸é ´ÙÀ½À¸·Î ³Ñ¾î°£´Ù.
	}

	if(pEvent->nMessage==MWM_LBUTTONDOWN)
	{
		if ( r.InPoint(LocalEvent.Pos)==true)
		{
			SetFocus();
//			OnSetFocus();
 			if((m_bFocusEnable)&&(m_bZOrderChangable==true)) SetZOrder(MZ_TOP);	// ÇØ´ç À§Á¬À» ¸Ç À§·Î ¿Ã·ÁÁØ´Ù.
		}
		else	// ReleaseFocus
		{
//			ReleaseFocus();
//			OnReleaseFocus();
//			ReleaseCapture();
		}
	}

//	if(m_bVisibleChildren==true && GetLatestExclusive()==NULL){
	if(GetLatestExclusive()==NULL){
		for(int i=m_Children.GetCount()-1; i>=0; i--){	// ¸¶Áö¸·¿¡ Ãß°¡µÈ À©µµ¿ìºÎÅÍ
			MWidget* pCurWnd = m_Children.Get(i);
			if(pCurWnd->Event(pEvent)==true) {
				return true;
			}
		}
	}

	// À§Á¬ ¸®»çÀÌÁî
	// Å¬¶óÀÌ¾ðÆ® ¿µ¿ªÀ» ¹þ¾î³ª¼­ Á¦¾îÇØ¾ßÇÏ¹Ç·Î, Event()¿¡¼­ Ã³¸®ÇÑ´Ù.
	if(EventResize(pEvent)==true) return true;

	//if(pEvent->nMessage==MWM_RESIZE) OnReSize();
	
	// HotKey
	/*
	if(pEvent->nMessage==MWM_KEYDOWN){
	//if(OnHotKey(pEvent->nKey, pEvent->GetShiftState(), pEvent->GetCtrlState(), pEvent->GetAltState())==true) return true;
	if(OnHotKey(pEvent->nKey, pEvent->GetShiftState(), pEvent->bCtrl, pEvent->bAlt)==true) return true;
	}
	*/
	if(pEvent->nMessage==MWM_HOTKEY){
		//if(OnHotKey(pEvent->nKey, pEvent->GetShiftState(), pEvent->GetCtrlState(), pEvent->GetAltState())==true) return true;
		if(OnHotKey(pEvent->nKey)==true) return true;
	}

	// Focused Event
	if(pEvent->nMessage==MWM_CHAR || pEvent->nMessage==MWM_KEYDOWN || pEvent->nMessage==MWM_IMECOMPOSE ||
		pEvent->nMessage==MWM_ACTIONKEYDOWN || pEvent->nMessage==MWM_ACTIONKEYUP || pEvent->nMessage==MWM_ACTIONPRESSED || pEvent->nMessage==MWM_ACTIONRELEASED){
		if (IsFocus()) {	// KEY Event´Â Focus ¾ø´Â À§Á¬¿¡´Â ÀÇ¹Ì¾ø´Ù
			//¸ÕÀú ÀÚ½Ä¿¡°Ô·Î ÀüÇØÁ®¼­ Ã³¸®µÇ¸é ok Ã³¸®ÇÑ ÀÚ½ÄÀÌ ¾øÀ¸¸é ºÎ¸ð¿¡°Ô±îÁö °£´Ù
			if(OnEvent(&LocalEvent, GetListener())==true) 
				return true;

			// Tab Key
			if(pEvent->nMessage==MWM_KEYDOWN && pEvent->nKey==VK_TAB){
				if(OnTab(!pEvent->GetShiftState())==true) 
					return true;
			}

			/*
			// Focused Widget¿¡°Ô ¿ì¼±±ÇÀÌ ÀÖÀ¸¸ç...
			if(MWidget::m_pFocusedWidget==this){	// MWM_CHAR ¸Þ¼¼ÁöÀÎ °æ¿ì ÇöÀç Æ÷Ä¿½ºµÈ À©µµ¿ì¿¡°Ô¸¸ ¸Þ¼¼Áö°¡ º¸³»Áø´Ù.
				if(OnEvent(&LocalEvent, GetListener())==true) 
					return true;
			}
			*/
			// Accelerator´Â EventAcceleratorÂÊÀ¸·Î ºüÁü
		}
	}
	else{
		if((MWidget::m_pCapturedWidget==NULL || MWidget::m_pCapturedWidget==this))	// Capture°¡ °É¸± °æ¿ì CaptureµÈ À©µµ¿ì¿¡¸¸ ¸Þ¼¼Áö º¸³»±â
			if(OnEvent(&LocalEvent, GetListener())==true) 
				return true;
	}

	if(pEvent->nMessage==MWM_LBUTTONUP) ReleaseCapture();

	return false;
}

bool MWidget::EventAccelerator(MEvent* pEvent)
{
	if(m_bVisible==false) return false;	// º¸ÀÌ´Â À©µµ¿ì¸¸ ÀÌº¥Æ®¸¦ Ã³¸®ÇÒ ¼ö ÀÖ´Ù.
	if(m_bEnable==false) return false;

	if(!(pEvent->nMessage==MWM_KEYDOWN || pEvent->nMessage==MWM_SYSKEYDOWN)) return false;

	// KEYDOWNÀÏ¶§ Edit¿¡ Focus°¡ ÀÖÀ» °æ¿ì Accelerator´Â ¹«½ÃµÈ´Ù. (SYSKEYDOWN¸¸ À¯È¿ÇÏ°Ô µÈ´Ù)
	if(pEvent->nMessage==MWM_KEYDOWN)
		if(MWidget::m_pFocusedWidget!=NULL)
			if(strcmp(MWidget::m_pFocusedWidget->GetClassName(), MINT_EDIT)==0) return false;

	if(GetLatestExclusive()!=NULL){
		if(GetLatestExclusive()->EventAccelerator(pEvent)==true) return true;
		if(GetLatestExclusive()!=NULL) return false;	// ÀÌº¥Æ® ÇÚµé·¯¿¡ ÀÇÇØ Exclusive°¡ Ç®¸®´Â °æ¿ì°¡ ÀÖÀ¸¸é ´ÙÀ½À¸·Î ³Ñ¾î°£´Ù.
	}

	if(GetLatestExclusive()==NULL){
		for(int i=m_Children.GetCount()-1; i>=0; i--){	// ¸¶Áö¸·¿¡ Ãß°¡µÈ À©µµ¿ìºÎÅÍ
			MWidget* pCurWnd = m_Children.Get(i);
			if(pCurWnd->EventAccelerator(pEvent)==true) return true;
		}
	}

	// Accelerator
	char szKey[2] = {(char)pEvent->nKey, 0};
//	_strupr(szKey);
	if(m_nAccelerator==szKey[0]) {

		m_bEventAcceleratorCall = true;

		if(DefaultCommand()==true) {
			m_bEventAcceleratorCall = false;
			return true;
		}
		m_bEventAcceleratorCall = false;
	}

	return false;
}

bool MWidget::EventDefaultKey(MEvent* pEvent)
{
	if(m_bVisible==false) return false;	// º¸ÀÌ´Â À©µµ¿ì¸¸ ÀÌº¥Æ®¸¦ Ã³¸®ÇÒ ¼ö ÀÖ´Ù.
	if(m_bEnable==false) return false;

	if(!(pEvent->nMessage==MWM_KEYDOWN))return false;

	if(GetLatestExclusive()!=NULL){
		if(GetLatestExclusive()->EventDefaultKey(pEvent)==true) return true;
		if(GetLatestExclusive()!=NULL) return false;	// ÀÌº¥Æ® ÇÚµé·¯¿¡ ÀÇÇØ Exclusive°¡ Ç®¸®´Â °æ¿ì°¡ ÀÖÀ¸¸é ´ÙÀ½À¸·Î ³Ñ¾î°£´Ù.
	}

	if(GetLatestExclusive()==NULL){
		for(int i=m_Children.GetCount()-1; i>=0; i--){	// ¸¶Áö¸·¿¡ Ãß°¡µÈ À©µµ¿ìºÎÅÍ
			MWidget* pCurWnd = m_Children.Get(i);
			if(pCurWnd->EventDefaultKey(pEvent)==true) return true;
		}
	}

	if(strcmp(GetClassName(), MINT_BUTTON)!=0) return false;

	MButton* pButton = (MButton*)this;
	if((pButton->m_nKeyAssigned==MBKA_ENTER && pEvent->nKey==VK_RETURN) ||
		(pButton->m_nKeyAssigned==MBKA_ESC && pEvent->nKey==VK_ESCAPE))
		if(DefaultCommand()==true) return true;

	return false;
}

bool MWidget::OnShow(void)
{
	return true;
}

void MWidget::OnHide(void)
{
}

void MWidget::OnSize(int w, int h)
{
	// ÇØ»óµµ º¯°æ½Ã ÅøÆÁ ¹Ù¿îµå¸¦ ´Ù½Ã °è»ê
	if (m_pToolTip)
		m_pToolTip->SetBounds();
}

/*
void MWidget::OnReSize()
{
}
*/

void MWidget::OnShow(bool bVisible)
{
	for(int i=m_Children.GetCount()-1; i>=0; i--){	// ¸¶Áö¸·¿¡ Ãß°¡µÈ À©µµ¿ìºÎÅÍ
		MWidget* pCurWnd = m_Children.Get(i);
		if(pCurWnd->m_bVisible==true) pCurWnd->OnShow(bVisible);
	}

	if(bVisible==true) OnShow();
	else OnHide();

	if(bVisible==false && MWidget::m_pFocusedWidget==this) ReleaseFocus();
}

bool MWidget::OnTab(bool bForward)
{
	MWidget* pParent = GetParent();
	if(pParent==NULL) return false;

	if(m_pParent->GetLatestExclusive()==this) return false;

	int nThisIndex = pParent->GetChildIndex(this);
	if(nThisIndex<0) return false;

	for(int i=0; i<pParent->GetChildCount(); i++){
		int nIndex = 0;
		if(bForward==true) nIndex = (nThisIndex+i+1) % pParent->GetChildCount();
		else nIndex = (nThisIndex+pParent->GetChildCount()-1-i) % pParent->GetChildCount();

		MWidget* pSibling = pParent->GetChild( nIndex );
		if(pSibling->IsFocusEnable()==true && pSibling!=this && pSibling->IsVisible()==true && pSibling->IsEnable()==true){
			pSibling->SetFocus();
			return true;
		}
	}

	return false;
}

bool MWidget::OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	return false;
}

void MWidget::ResizeChildrenByAnchor(int w, int h)
{
	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pChild = m_Children.Get(i);
		if(pChild->GetBoundsAlignment()!=0){
			pChild->SetBoundsAlignment(pChild->GetBoundsAlignment(), w, h);
			continue;	// Bounds Alignment¿¡ ¿ì¼±¼øÀ§°¡ ÀÖ´Ù.
		}

		MRECT r = pChild->m_Rect;
		if(pChild->m_Anchors.m_bLeft==true && pChild->m_Anchors.m_bRight==true){
			r.w += (w-m_Rect.w);
		}
		else if(pChild->m_Anchors.m_bRight==true){
			r.x += (w-m_Rect.w);
		}
		if(pChild->m_Anchors.m_bTop==true && pChild->m_Anchors.m_bBottom==true){
			r.h += (h-m_Rect.h);
		}
		else if(pChild->m_Anchors.m_bBottom==true){
			r.y += (h-m_Rect.h);
		}
		pChild->SetBounds(r);
	}
}

void MWidget::Show(bool bVisible, bool bModal)
{
	if(m_bVisible==bVisible){
		if(bModal==true){
			if(m_pParent!=NULL && m_pParent->GetLatestExclusive()==this)
				return;
		}
		else return;
	}

	m_bVisible = bVisible;

	if(bVisible==true && bModal==true) SetExclusive();
	else if(bVisible==false) {
		ReleaseExclusive();
		if(MWidget::m_pFocusedWidget==this) ReleaseFocus();
	}

	OnShow(bVisible);
}

void MWidget::Enable(bool bEnable)
{
	m_bEnable = bEnable;
}

bool MWidget::IsVisible(void)
{
	return m_bVisible;
}

bool MWidget::IsEnable(void)
{
	return m_bEnable;
}

void MWidget::SetResizable(bool bEnable)
{
	m_bResizable = bEnable;
}

bool MWidget::IsResizable(void)
{
	return m_bResizable;
}

void MWidget::SetListener(MListener* pListener)
{
	m_pListener = pListener;
}

MListener* MWidget::GetListener(void)
{
	return m_pListener;
}

int MWidget::GetID(void)
{
	return m_nID;
}

void MWidget::SetID(int nID)
{
	m_nID = nID;
}

void MWidget::SetText(const char* szText)
{
	if(szText==NULL) return;
	_ASSERT(strlen(szText)<MWIDGET_NAME_LENGTH);
	if (strlen(szText) >= MWIDGET_NAME_LENGTH) return;

	strcpy(m_szName, szText);
	if(m_pToolTip!=NULL) if(m_pToolTip->IsUseParentName()==true) m_pToolTip->SetBounds();

	SetLabelAccelerator();
	//if(m_nAcceleratorType==1) UseAcceleratorAndCharacter(szText);
}

const char* MWidget::GetText(void)
{
	return m_szName;
}

void MWidget::SetCapture(void)
{
	MWidget::m_pCapturedWidget = this;
}

void MWidget::ReleaseCapture(void)
{
	MWidget::m_pCapturedWidget = NULL;
}

void MWidget::SetFocusEnable(bool bEnable)
{
	m_bFocusEnable = bEnable;
}

bool MWidget::IsFocusEnable(void)
{
	return m_bFocusEnable;
}

void MWidget::SetFocus(void)
{
	if(m_bFocusEnable==false) return;
	/*
	if(m_pParent!=NULL){
		if(m_pParent->m_pExclusive!=NULL && m_pParent->m_pExclusive!=MWidget::m_pFocusedWidget) return;
	}
	*/
	// ÀÚ½ÄÁß¿¡ Exclusive À§Á¬ÀÌ ÀÖ´Â°æ¿ì,
	// ÇöÀç Exclusive À§Á¬°ú ±× Children¿¡°Ô¸¸ Focus°¡ °¥ ¼ö ÀÖµµ·Ï ÇÑ´Ù.
	MWidget* pExDes = FindExclusiveDescendant();
	if(pExDes!=NULL) return;
	/*
	if(MWidget::m_pFocusedWidget!=NULL){
		if(MWidget::m_pFocusedWidget->m_pParent!=NULL){
			if(MWidget::m_pFocusedWidget->m_pParent->m_pExclusive==MWidget::m_pFocusedWidget){
				return;
			}
		}
	}
	*/

	if(MWidget::m_pFocusedWidget==this) return;

	if(MWidget::m_pFocusedWidget!=NULL) MWidget::m_pFocusedWidget->OnReleaseFocus();

	MWidget::m_pFocusedWidget = this;
	OnSetFocus();
}

void MWidget::ReleaseFocus(void)
{
	if(MWidget::m_pFocusedWidget==this) OnReleaseFocus();
	MWidget::m_pFocusedWidget = NULL;
}

bool MWidget::IsFocus(void)
{
	if(MWidget::m_pFocusedWidget==this) return true;
	return false;
}

MWidget* MWidget::GetParent(void)
{
	return m_pParent;
}

int MWidget::GetChildCount(void)
{
	return m_Children.GetCount();
}

MWidget* MWidget::GetChild(int i)
{
	return m_Children.Get(i);
}

int MWidget::GetChildIndex(MWidget* pWidget)
{
	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pChild = m_Children.Get(i);
		if(pChild==pWidget) return i;
	}

	return -1;
}

void MWidget::SetExclusive(void)
{
	if(m_pParent!=NULL){
		m_pParent->AddExclusive(this);
		SetFocus();	// Exclusive·Î º¸¿©Áú¶§ Æ÷Ä¿½Ì.
	}
}

void MWidget::ReleaseExclusive(void)
{
	if(m_pParent!=NULL)
		m_pParent->RemoveExclusive(this);
}

MCursor* MWidget::SetCursor(MCursor* pCursor)
{
	MCursor* pTemp = m_pCursor;
	m_pCursor = pCursor;
	return pTemp;
}

MCursor* MWidget::GetCursor(void)
{
	return m_pCursor;
}

MFont* MWidget::SetFont(MFont* pFont)
{
	MFont* pTemp = m_pFont;
	m_pFont = pFont;
	return pTemp;
}

MFont* MWidget::GetFont(void)
{
	if(m_pFont==NULL) return MFontManager::Get(NULL);
	else return m_pFont;
}

void MWidget::SetSize(int w, int h)
{
	if(w<0) w = 1;
	if(h<0) h = 1;

	// TODO: 100,100 À¸·Î ÁöÁ¤ÇÏ¸é default size °¡ µÇ¾î onsize°¡ ºÒ¸®Áö ¾ÊÀ½.
	//if(w==m_Rect.w && h==m_Rect.h) return;
	////////// ÀÌ°Ç ¿Ø Æ®·¦Ä«µå ¹ßµ¿ÀÎ°¡¿ä

	ResizeChildrenByAnchor(w, h);

	m_Rect.w = w;
	m_Rect.h = h;

	OnSize(w, h);
}

void MWidget::SetSize(MSIZE& s)
{
	SetSize(s.w, s.h);
}


void MWidget::SetPosition(int x, int y)
{
	MPOINT p;

	//if(m_BoundsAlignment!=MAM_NOTALIGN) return;	// ¾ó¶óÀÎ ¸ðµå ¼³Á¤µÈ À§Á¬Àº À§Ä¡ Àç¼³Á¤À» Çã¿ëÇÏÁö ¾Ê´Â´Ù.
	//m_BoundsAlignment = MAM_NOTALIGN;	// À§Ä¡¸¦ ¹Ù²Ù¸é ¾ó¶óÀÎÀÌ ¾ø¾îÁø´Ù.

	m_Rect.x = x;
	m_Rect.y = y;

}

void MWidget::SetPosition(MPOINT& p)
{
	SetPosition(p.x, p.y);
}

void MWidget::SetBounds(MRECT& r)
{
	SetBounds(r.x, r.y, r.w, r.h);
}

void MWidget::SetBounds(int x, int y, int w, int h)
{
	SetPosition(x, y);
	//m_Rect.x = x;
	//m_Rect.y = y;

	if(w<0) w = 1;
	if(h<0) h = 1;

	SetSize(w, h);
	/*
	bool bResized = false;
	if(w!=m_Rect.w || h!=m_Rect.h){
		m_Rect.w = w;
		m_Rect.h = h;
		bResized = true;
	}
	*/

	//if(bResized==true) OnSize(w, h);
}

MPOINT MWidget::GetPosition(void)
{
	return MPOINT(m_Rect.x, m_Rect.y);
}

MRECT MWidget::GetRect(void)
{
	return m_Rect;
}

MRECT MWidget::GetIDLRect(void)
{
	return m_IDLRect;
}

void MWidget::GetBoundsAlignmentPosition(MPOINT* p, MAlignmentMode am, int w, int h)
{
	p->x = m_Rect.x;
	p->y = m_Rect.y;

	MSIZE s(w, h);
	if(w==-1){
		s.w = MGetWorkspaceWidth();
		if(GetParent()!=NULL) s.w = GetParent()->m_Rect.w;
	}
	if(h==-1){
		s.h = MGetWorkspaceHeight();
		if(GetParent()!=NULL) s.h = GetParent()->m_Rect.h;
	}

	if(m_BoundsAlignment&MAM_LEFT) p->x = 0;
	else if(m_BoundsAlignment&MAM_RIGHT) p->x = s.w-m_Rect.w;
	else if(m_BoundsAlignment&MAM_HCENTER) p->x = (s.w-m_Rect.w)/2;

	if(m_BoundsAlignment&MAM_LEFT) p->y = 0;
	else if(m_BoundsAlignment&MAM_BOTTOM) p->y = s.h-m_Rect.h;
	else if(m_BoundsAlignment&MAM_VCENTER) p->y = (s.h-m_Rect.h)/2;
}

void MWidget::SetBoundsAlignment(MAlignmentMode am, int w, int h)
{
	m_BoundsAlignment = am;

	MPOINT p;
	GetBoundsAlignmentPosition(&p, m_BoundsAlignment, w, h);

	// Æ÷Áö¼Ç Á¤·Ä¿¡ ÀÇÇØ Àç¼³Á¤
	m_Rect.x = p.x;
	m_Rect.y = p.y;
}

MAlignmentMode MWidget::GetBoundsAlignment(void)
{
	return m_BoundsAlignment;
}


MRECT MWidget::GetScreenRect(void)
{
	if(m_pParent!=NULL){
		MRECT sr = m_pParent->GetScreenRect();
		MRECT r = m_Rect;
		r.Offset(sr.x, sr.y);
		return r;
	}

	return m_Rect;
}

void MWidget::AttachToolTip(const char* szToolTipString)
{
	DetachToolTip();
	m_pToolTip = new MToolTip(szToolTipString, this);
}

void MWidget::AttachToolTip(MToolTip* pToolTip)
{
	DetachToolTip();
	m_pToolTip = pToolTip;
}

void MWidget::DetachToolTip(void)
{
	if(m_pToolTip!=NULL) delete m_pToolTip;
	m_pToolTip = NULL;
}

MToolTip* MWidget::GetToolTip(void)
{
	return m_pToolTip;
}

void MWidget::SetAccelerator(int a)
{
	//m_nAcceleratorType = 0;	// use custom accelerator type
	if(a==0) m_nAccelerator = -1;
	else m_nAccelerator = a;
}

void MWidget::SetLabelAccelerator(void)
{
	SetAccelerator(GetLabelAccelerator());
}

char MWidget::GetLabelAccelerator(void)
{
	char szAnd[2] = {GetAndChar(m_szName), 0};
	_strupr(szAnd);
	if(szAnd[0]==0) return -1;
	return szAnd[0];
}

char MWidget::GetToolTipAccelerator(void)
{
	if(m_pToolTip!=NULL){
		char szAnd[2] = {GetAndChar(m_pToolTip->m_szName), 0};
		_strupr(szAnd);
		if(szAnd[0]==0) return -1;
		return szAnd[0];
	}
	else{
		return -1;
	}
}
/*
void MWidget::SetLabelAccelerator(void)
{
	m_nAcceleratorType = 1;	// use label's & character
	UseAcceleractorAndCharacter(m_szName);
}

void MWidget::SetToolTipAccelerator(void)
{
	m_nAcceleratorType = 2;	// use tooltip's & character
	if(m_pToolTip!=NULL)
		UseAcceleractorAndCharacter(m_pToolTip->m_szName);
	else
		UseAcceleractorAndCharacter(NULL);
}

void MWidget::EnableAccelerator(bool bEnabled)
{
	m_bAcceleratorEnabled = bEnabled;
}
*/

MRECT MWidget::GetClientRect(void)
{
	return MRECT(0, 0, m_Rect.w, m_Rect.h);
}

MRECT MWidget::GetInitialClientRect(void)
{
	return MWidget::GetClientRect();
}

void MWidget::SetZOrder(MZOrder z)
{
	if(m_pParent==NULL) return;

	MWidget* pParent = m_pParent;
	pParent->RemoveChild(this);

	switch(z){
	case MZ_TOP:
		pParent->AddChild(this);
		break;
	case MZ_BOTTOM:
		pParent->InsertChild(this);
		break;
	}
}

MWidget* MWidget::FindExclusiveDescendant(void)
{
	if(m_Exclusive.GetCount()>0) return m_Exclusive.Get(m_Exclusive.GetCount()-1);	// ¸¶Áö¸·¿¡ Ãß°¡µÈ°Í

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pChild = m_Children.Get(i);
		MWidget* pExDes = pChild->FindExclusiveDescendant();
		if(pExDes!=NULL) return pExDes;
	}

	return NULL;
}

bool MWidget::IsExclusive(MWidget* pWidget)
{
	for(int i=0; i<m_Exclusive.GetCount(); i++){
		MWidget* pThis = m_Exclusive.Get(i);
		if(pThis==pWidget) return true;
	}
	return false;
}


MWidget* MWidget::Find(MPOINT& p)
{
	if(IsVisible()==false) return NULL;

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pChild = m_Children.Get(m_Children.GetCount()-i-1);
		MWidget* pFind = pChild->Find(p);
		if(pFind!=NULL) return pFind;
	}

	if(GetScreenRect().InPoint(p)==true)
		return this;

	return NULL;
}

MWidget* MWidget::FindDropAble(MPOINT& p)
{
	if(IsVisible()==false) return NULL;

	for(int i=0; i<m_Children.GetCount(); i++){
		MWidget* pChild = m_Children.Get(m_Children.GetCount()-i-1);
		MWidget* pFind = pChild->FindDropAble(p);
		if(pFind!=NULL) return pFind;
	}

	if(GetScreenRect().InPoint(p)==true)
		if(m_bFocusEnable==true)
			return this;

	return NULL;
}


bool MWidget::Drop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	return OnDrop(pSender, pBitmap, szString, szItemString);
}

void MWidget::GetHierarchicalName(char* szName)
{
	if(m_pParent!=NULL){
		m_pParent->GetHierarchicalName(szName);
		strcat(szName, "/");
		strcat(szName, m_szName);
	}
	else{
		strcpy(szName, m_szName);
	}
}

MWidget* MWidget::FindWidgetByHierarchicalName(const char* szName)
{
	char szHierachicalName[2048];
	GetHierarchicalName(szHierachicalName);
	if(strcmp(szName, szHierachicalName)==0){
		return this;
	}

	for(int i=0; i<GetChildCount(); i++){
		MWidget* pFind = GetChild(i)->FindWidgetByHierarchicalName(szName);
		if(pFind!=NULL) return pFind;
	}

	return NULL;
}

bool MWidget::IsMsg(const char* szMsg1, const char* szMsg2)
{
	// ´ë¼Ò¹®ÀÚ¸¦ ¾È°¡¸°´Ù.
	char szTmp1[256];
	char szTmp2[256];
	strcpy(szTmp1, szMsg1);
	strcpy(szTmp2, szMsg2);
	_strupr(szTmp1);
	_strupr(szTmp2);
	if(strcmp(szTmp1, szTmp2)==0) return true;
	return false;
}

void* MWidget::Query(const char* szQuery)
{
	return NULL;
}

void MWidget::SetOpacity(unsigned char nOpacity)
{
	m_nOpacity = nOpacity;
	/*
	for(int i=0; i<m_Children.GetCount(); i++)
	{
		MWidget* pCurWnd = m_Children.Get(i);
		if(pCurWnd==GetLatestExclusive()) continue;
		pCurWnd->SetOpacity(nOpacity);
	}
	*/
}

// ÇØ»óµµ º¯°æ¿¡ µû¶ó¼­ À§Á¬ÀÇ Å©±â¸¦ ´Ü¼ø ¹èÀ²½ÃÅ³ ¶§ »ç¿ëÇÑ´Ù, ÆùÆ® Å©±â±îÁö °°ÀÌ Ä¿Áö´Â °æ¿ì¿¡ »ç¿ëÇÑ´Ù.
// ÆùÆ® Å©±â´Â ±×´ë·ÎÀÌ°í ¼ø¼öÇÏ°Ô ÇÁ·¹ÀÓÃ¢ Å©±â¸¸ º¯°æÇÒ ¶§´Â Resize, SetSize ÇÔ¼ö±ºÀ» »ç¿ëÇØ¾ß ÇÑ´Ù.
// ÀÌ ÇÔ¼ö´Â Resize, SetSize ÇÔ¼ö±º°ú´Â ´Ù¸£°Ô anchor¸¦ ¹«½ÃÇÑ´Ù. (Áï ÀÚ½ÄÀ§Á¬µé¿¡µµ ÇÔ²² ´Ü¼ø ¹èÀ² Àû¿ëÇÔ)
// byIDLWidth, byIDLHeight : IDL Á¤ÀÇ°¡ 800*600 ±âÁØÀÌ¹Ç·Î ÇöÀç ÇØ»óµµ°¡ 800*600¿¡ ´ëÇØ¼­ ¸î¹èÀ²ÀÎ°¡¸¦ ÁöÁ¤.
// byCurrWidth, byCurrHeight : IDL Á¤ÀÇ°¡ ¾ø´Â À§Á¬Àº ±¸ÇØ»óµµ->½ÅÇØ»óµµÀÇ ¹èÀ²·Î Å©±â¸¦ Á¶Á¤ÇÑ´Ù.
void MWidget::MultiplySize(float byIDLWidth, float byIDLHeight, float byCurrWidth, float byCurrHeight)
{
	// idl¿¡¼­ ÀÐ¾î¼­ ÀûÀýÇÑ ÃÊ±â°ªÀ» °¡Áö°í ÀÖ´Â À§Á¬ÀÌ¶ó¸é ±×°ªÀ¸·Î »õ Å©±â¸¦ ±¸ÇÑ´Ù
	if(GetIDLRect().w>0 && GetIDLRect().h>0)	
	{
		// idl ¿¡¼­´Â 800x600 ±âÁØÀ¸·Î ±â¼úµÇ¾îÀÖ´Ù
		m_Rect.x = int(m_IDLRect.x * byIDLWidth +0.5f);
		m_Rect.w = int(m_IDLRect.w * byIDLWidth +0.5f);
		m_Rect.y = int(m_IDLRect.y * byIDLHeight +0.5f);
		m_Rect.h = int(m_IDLRect.h * byIDLHeight +0.5f);
	}
	else 
	{
		// idl Á¤ÀÇ°¡ ¾ø´Â À§Á¬Àº ¿¹Àü ÇØ»óµµ -> »õ ÇØ»óµµ °£ÀÇ ¹èÀ²·Î Å©±â¸¦ Á¶Á¤ÇÑ´Ù
		m_Rect.x = int(m_Rect.x * byCurrWidth +0.5f);
		m_Rect.w = int(m_Rect.w * byCurrWidth +0.5f);
		m_Rect.y = int(m_Rect.y * byCurrHeight +0.5f);
		m_Rect.h = int(m_Rect.h * byCurrHeight +0.5f);
	}

	// x,y¼³Á¤½Ã ¾ó¶óÀÎ¸ÕÆ®°¡ Ç®¸®¹Ç·Î Àç¼³Á¤
	SetBoundsAlignment( GetBoundsAlignment(), -1, -1);

	// ÀÚ½Äµµ ¹èÀ² Àû¿ë
	int n = GetChildCount();
	for( int i = 0; i < n; ++i)
	{
		GetChild(i)->MultiplySize(byIDLWidth, byIDLHeight, byCurrWidth, byCurrHeight);
	}
}

unsigned char MWidget::GetOpacity()
{
	return m_nOpacity;
}

int GetAndPos(const char* szText)
{
	if(szText==NULL) return -1;
	int nLen = strlen(szText);
	for(int i=0; i<nLen; i++){
		if(szText[i]=='&') return i;
	}
	return -1;
}

char GetAndChar(const char* szText)
{
	int nLen = GetAndPos(szText);
	if(nLen==-1) return 0;
	return szText[nLen+1];
}

int RemoveAnd(char* szText)
{
	if(szText==NULL) return -1;
	int nPos = GetAndPos(szText);
	if(nPos==-1) return -1;
	int nLen = strlen(szText);
	for(int i=0; i<(nLen-nPos); i++){
		szText[nPos+i] = szText[nPos+i+1];
	}
	return nPos;
}

int RemoveAnd(char* szRemovedText, const char* szText)
{
	if(szText==NULL) return -1;
	strcpy(szRemovedText, szText);
	return RemoveAnd(szRemovedText);
}

int RemoveAnd(char* szRemovedFrontText, char* cUnderLineChar, char* szRemovedBackText, const char* szText)
{
	if(szText==NULL) return -1;
	int nPos = GetAndPos(szText);
	if(nPos==-1) return -1;
	memcpy(szRemovedFrontText, szText, nPos);
	szRemovedFrontText[nPos] = 0;
	*cUnderLineChar = szText[nPos+1];
	strcpy(szRemovedBackText, szText+nPos+2);
	return nPos;
}

MPOINT MClientToScreen(MWidget* pWidget, MPOINT& p)
{
	MRECT r = pWidget->GetScreenRect();
	return MPOINT(p.x+r.x, p.y+r.y);
}

MPOINT MScreenToClient(MWidget* pWidget, MPOINT& p)
{
	MRECT r = pWidget->GetScreenRect();
	return MPOINT(p.x-r.x, p.y-r.y);
}

MRECT MClientToScreen(MWidget* pWidget, MRECT& p)
{
	MRECT r = pWidget->GetScreenRect();
	return MRECT(p.x+r.x, p.y+r.y, p.w, p.h);
}

MRECT MScreenToClient(MWidget* pWidget, MRECT& p)
{
	MRECT r = pWidget->GetScreenRect();
	return MRECT(p.x-r.x, p.y-r.y, p.w, p.h);
}

MPOINT GetCursorPosition(void)
{
	POINT p;
	GetCursorPos(&p);
	MPOINT mp;
	mp.x = p.x;
	mp.y = p.y;
	return mp;
}

void MWidget::BringToTop()
{
	if (m_pParent)
	{
		m_pParent->RemoveChild(this);
		m_pParent->AddChild(this);
	}
}