#include "stdafx.h"
#include "MButton.h"
#include "MColorTable.h"
#include "MTooltip.h"
#include "MMsgBox.h"
#include "Mint.h"

#define MBUTTON_DEFAULT_ALIGNMENT_MODE	(MAM_HCENTER|MAM_VCENTER)

/*
MButtonLook MButton::m_DefaultLook;
MButtonLook* MButton::m_pLook = &MButton::m_DefaultLook;
*/

IMPLEMENT_LOOK(MButton, MButtonLook)

/*
void MButton::OnDraw(MDrawContext* pDC)
{
	m_pLook->OnDraw(this, pDC);
}
*/

void MButton::OnMouseIn(void)
{
	if (m_bScaleOnHover == true && m_bIsScaled == false)
	{
		m_rcOriginal = GetRect(); // Lưu lại vị trí & kích thước chuẩn
		int nExpand = 10; // Số pixel muốn phóng to ra các hướng

		// Bơm to nút bấm ra
		SetBounds(m_rcOriginal.x - nExpand,
			m_rcOriginal.y - nExpand,
			m_rcOriginal.w + (nExpand * 2),
			m_rcOriginal.h + (nExpand * 2));

		m_bIsScaled = true;
	}
}

void MButton::OnMouseOut(void)
{
	if (m_bScaleOnHover == true && m_bIsScaled == true)
	{
		// Trả nút bấm về đúng kích thước và vị trí ban đầu
		SetBounds(m_rcOriginal);
		m_bIsScaled = false;
	}
}

void MButton::OnButtonDown(void)
{
}

void MButton::OnButtonUp(void)
{
}

void MButton::OnButtonClick(void)
{
	if(GetListener()) GetListener()->OnCommand(this, MBTN_CLK_MSG);
}

bool MButton::OnEvent(MEvent* pEvent, MListener* pListener)
{
	MRECT r = GetInitialClientRect();

	m_ClickPos = pEvent->Pos;

	switch(pEvent->nMessage){
	case MWM_LBUTTONDOWN:
		if(r.InPoint(pEvent->Pos)==true){
			m_bLButtonDown = true;

			m_LDragStartClickPos = pEvent->Pos;
			m_LDragVariationX = 0;
			m_LDragVariationY = 0;

			OnButtonDown();
			if(pListener!=NULL) pListener->OnCommand(this, MBTN_DN_MSG);
			SetCapture();
			return true;
		}
		break;

	case MWM_LBUTTONUP:
		if(m_bLButtonDown==true){
			m_bLButtonDown = false;

			m_LDragVariationX = 0;
			m_LDragVariationY = 0;

			OnButtonUp();
			ReleaseCapture();
			if(r.InPoint(pEvent->Pos)==true){
				if(GetType()==MBT_PUSH || GetType()==MBT_PUSH2 ) 
				{
					if(m_pButtonGroup)
						SetCheck(true);
					else
						SetCheck(!m_bChecked);
				}

				if(m_pMsgBox!=NULL) m_pMsgBox->Show(true, true);
				else if(pListener!=NULL) {
					pListener->OnCommand(this, MBTN_UP_MSG);
					OnButtonClick();
				}
				return true;
			}
		}
		break;

	case MWM_LBUTTONDBLCLK:
		if(r.InPoint(pEvent->Pos)==true){
			if(m_pMsgBox!=NULL) m_pMsgBox->Show(true, true);
			else OnButtonClick();
			return true;
		}
		break;

	case MWM_RBUTTONDBLCLK:
		if(r.InPoint(pEvent->Pos)==true){
			if(m_pMsgBox!=NULL) m_pMsgBox->Show(true, true);
			else if(pListener!=NULL) pListener->OnCommand(this, MBTN_RCLK_MSG);
			return true;
		}
		break;

	case MWM_RBUTTONDOWN:
		if(r.InPoint(pEvent->Pos)==true){
			m_bRButtonDown = true;
			if(pListener!=NULL) pListener->OnCommand(this, MBTN_RDN_MSG);
			SetCapture();
			return true;
		}
		break;

	case MWM_RBUTTONUP:
		if(m_bRButtonDown==true){
			m_bRButtonDown = false;
			ReleaseCapture();
			if(r.InPoint(pEvent->Pos)==true && pListener!=NULL){
				if(GetType()==MBT_PUSH && GetType() == MBT_PUSH2 ) m_bChecked = !m_bChecked;
				pListener->OnCommand(this, MBTN_RUP_MSG);
				pListener->OnCommand(this, MBTN_RCLK_MSG);
			}
			return true;
		}
		break;
	case MWM_MOUSEMOVE:
		if(r.InPoint(pEvent->Pos)==true){
			if(m_bMouseOver==false){
				m_bMouseOver = true;
				OnMouseIn();
				if(pListener!=NULL) pListener->OnCommand(this, MBTN_IN_MSG);
			}

			if(m_bLButtonDown) {//drag
				m_LDragVariationX = m_LDragStartClickPos.x - pEvent->Pos.x;
				m_LDragVariationY = m_LDragStartClickPos.y - pEvent->Pos.y;
			}
		}
		else{
			if(m_bMouseOver==true){
				m_bMouseOver = false;
				OnMouseOut();
				if(pListener!=NULL) pListener->OnCommand(this, MBTN_OUT_MSG);
			}
		}
		return false;
	case MWM_KEYDOWN:
		if(m_bEnableEnter==true && IsFocus()==true){
			if(pEvent->nKey==VK_RETURN){
				if(m_pMsgBox!=NULL) m_pMsgBox->Show(true, true);
				else OnButtonClick();
				return true;
			}
		}
		break;
	}
	return false;
}

bool MButton::OnShow(void)
{
	m_bMouseOver = false;
	m_bLButtonDown = false;
	if(GetToolTip()!=NULL) GetToolTip()->Show(false);
	return true;
}

void MButton::OnHide(void)
{
	if(GetToolTip()!=NULL) GetToolTip()->Show(false);
	if(m_pMsgBox) m_pMsgBox->Show(false);
}

MButton::MButton(const char* szName, MWidget* pParent, MListener* pListener)
: MWidget(szName, pParent, pListener), m_TextColor(0, 255, 0)//, m_TextOffset(0, 0)
{
	m_bMouseOver = false;
	m_bLButtonDown = false;
	m_bShowText = true;

	m_AlignmentMode = MBUTTON_DEFAULT_ALIGNMENT_MODE;
	m_Type = MBT_NORMAL;
	m_bChecked = false;

	m_bEnableEnter = true;
	m_bHighlight = true;

	m_pIcon = NULL;

	m_nKeyAssigned = MBKA_NONE;

	SetFocusEnable(true);

	m_pMsgBox = NULL;

	m_ClickPos = MPOINT(0,0);

	m_LDragStartClickPos = MPOINT(0,0);
	m_LDragVariationX = 0;
	m_LDragVariationY = 0;

	m_bComboDropped = false;
	m_bStretch = false;

	m_pButtonGroup = NULL;
	m_nIndexInGroup = 0;

	LOOK_IN_CONSTRUCTOR()

	m_bScaleOnHover = false;
	m_bIsScaled = false;

	// TRICK NHẬN DIỆN TỪ XML: 
	// Nếu tên ID trong XML có chứa chữ "SCALE_" ở đầu thì tự động bật hiệu ứng
	if (szName != NULL && strstr(szName, "SCALE_") != NULL) {
		m_bScaleOnHover = true;
	}
}

MButton::~MButton(void)
{
	if(m_pMsgBox!=NULL) delete m_pMsgBox;
}


/*
void MButton::SetTextOffset(MPOINT p)
{
	m_TextOffset = p;
}
*/
void MButton::SetTextColor(MCOLOR color)
{
	m_TextColor = color;
}

MCOLOR MButton::GetTextColor(void)
{
	return m_TextColor;
}

void MButton::ShowText(bool bShow)
{
	m_bShowText = bShow;
}

bool MButton::DefaultCommand(void)
{
	MListener* pListener = GetListener();
	if(m_pMsgBox!=NULL) m_pMsgBox->Show(true, true);
	else OnButtonClick(); 
	return true;
}

MAlignmentMode MButton::GetAlignment(void)
{
	return m_AlignmentMode;
}

MAlignmentMode MButton::SetAlignment(MAlignmentMode am)
{
	MAlignmentMode temp = m_AlignmentMode;
	m_AlignmentMode = am;
	return temp;
}

void MButton::SetType(MButtonType t)
{
	m_Type = t;
}

MButtonType MButton::GetType(void)
{
	return m_Type;
}

void MButton::SetCheck(bool bCheck)
{
	if (bCheck && m_pButtonGroup)
	{
		// FIX: Kiểm tra m_pPrevious khác NULL trước khi gọi hàm
		if (m_pButtonGroup->m_pPrevious != NULL && m_pButtonGroup->m_pPrevious != this)
		{
			m_pButtonGroup->m_pPrevious->SetCheck(false);
		}

		m_pButtonGroup->m_pPrevious = this;
	}

	m_bChecked = bCheck;
}

bool MButton::GetCheck(void)
{
	return m_bChecked;
}

bool MButton::IsButtonDown(void)
{
	if(m_bLButtonDown==true && m_bMouseOver==true) return true;
	return false;
}

bool MButton::IsMouseOver(void)
{
	return m_bMouseOver;
}

void MButton::SetConfirmMessageBox(const char* szMessage)
{
	if(szMessage==NULL || szMessage[0]==0){
		if(m_pMsgBox!=NULL) delete m_pMsgBox;
		m_pMsgBox = NULL;
		return;
	}

	
	//if(m_pMsgBox==NULL) m_pMsgBox = new MMsgBox(szMessage, GetParent(), MT_OKCANCEL, this);
	if(m_pMsgBox==NULL) {
		m_pMsgBox = (MMsgBox*)Mint::GetInstance()->NewWidget(MINT_MSGBOX, "", Mint::GetInstance()->GetMainFrame(), this );
		m_pMsgBox->SetType(MT_OKCANCEL);
//		m_pMsgBox->SetTitle(szMessage);		// title Àº lookÀÇ default title·Î ´ëÃ¼
	}
	
	m_pMsgBox->SetText(szMessage);
	m_strIDLConfirmText = szMessage;
}

void MButton::SetAlterableConfirmMessage(const char* szMessage)
{
	if (m_pMsgBox)
		m_pMsgBox->SetText(szMessage);
}
void MButton::RestoreIDLConfirmMessage()
{
	if (m_pMsgBox)
		m_pMsgBox->SetText(m_strIDLConfirmText.c_str());
}

bool MButton::OnCommand(MWidget* pWidget, const char* szMessage)
{
	if(pWidget==m_pMsgBox){
		m_pMsgBox->Show(false);
		MListener* pListener = GetListener();
		if(strcmp(szMessage, MMSGBOX_OK)==0) if(pListener!=NULL) pListener->OnCommand(this, MBTN_CLK_MSG);
		return true;
	}
	return false;
}

void MButton::SetButtonGroup(MButtonGroup *pGroup)
{
	m_pButtonGroup = pGroup;
	m_nIndexInGroup = pGroup->m_nCount;
	pGroup->m_nCount++;
	
	if(GetCheck()==true)
		pGroup->m_pPrevious = this;
}

void MButton::MultiplySize( float byIDLWidth, float byIDLHeight, float byCurrWidth, float byCurrHeight )
{
	MWidget::MultiplySize(byIDLWidth, byIDLHeight, byCurrWidth, byCurrHeight);

	if (m_pMsgBox)
		m_pMsgBox->MultiplySize(byIDLWidth, byIDLHeight, byCurrWidth, byCurrHeight);
}
//void MButtonLook::OnDrawText(MButton* pButton, MRECT& r, MDrawContext* pDC)
//{
//	if( !pButton->IsVisible() ) return;
//	if(pButton->m_pIcon!=NULL){
//		pDC->SetBitmap(pButton->m_pIcon);
//		pDC->Draw(r.x, r.y+(r.h-pButton->m_pIcon->GetHeight())/2);
//		r.x+=pButton->m_pIcon->GetWidth();
//		r.w-=pButton->m_pIcon->GetWidth();
//	}
//
//	pDC->SetColor(MCOLOR(DEFCOLOR_MBUTTON_TEXT));
//	if(pButton->m_bHighlight==true){
//		MCOLOR PrevHCol = pDC->SetHighlightColor(MCOLOR(DEFCOLOR_PEN_HIGHLIGHT));
//		pDC->TextWithHighlight(r, pButton->m_szName, pButton->GetAlignment());
//		pDC->SetHighlightColor(PrevHCol);
//	}
//	else{
//		pDC->Text(r, pButton->m_szName, pButton->GetAlignment());
//	}
//}
void MButtonLook::OnDrawText(MButton* pButton, MRECT& r, MDrawContext* pDC)
{
	if (!pButton->IsVisible()) return;
	if (pButton->m_pIcon != NULL) {
		pDC->SetBitmap(pButton->m_pIcon);
		pDC->Draw(r.x, r.y + (r.h - pButton->m_pIcon->GetHeight()) / 2);
		r.x += pButton->m_pIcon->GetWidth();
		r.w -= pButton->m_pIcon->GetWidth();
	}

	// Đổi màu text sáng chuẩn Modern UI (Xám trắng)
	pDC->SetColor(pButton->GetTextColor());
	if (pButton->m_bHighlight == true) {
		MCOLOR PrevHCol = pDC->SetHighlightColor(MCOLOR(0, 210, 255)); // Highlight Cyan
		pDC->TextWithHighlight(r, pButton->m_szName, pButton->GetAlignment());
		pDC->SetHighlightColor(PrevHCol);
	}
	else {
		pDC->Text(r, pButton->m_szName, pButton->GetAlignment());
	}
}

//void MButtonLook::OnDownDraw(MButton* pButton, MDrawContext* pDC)
//{
//	MRECT r = pButton->GetInitialClientRect();
//	pDC->SetColor(MCOLOR(DEFCOLOR_MBUTTON_DARKPLANE));
//	pDC->FillRectangle(r);
//
//	OnDrawText(pButton, pButton->GetClientRect(), pDC);
//}
void MButtonLook::OnDownDraw(MButton* pButton, MDrawContext* pDC)
{
	// Để XML (Template) tự vẽ Bitmap trạng thái DOWN
	OnDrawText(pButton, pButton->GetClientRect(), pDC);
}

//void MButtonLook::OnUpDraw(MButton* pButton, MDrawContext* pDC)
//{
//	MRECT r = pButton->GetInitialClientRect();
//	pDC->SetColor(MCOLOR(DEFCOLOR_MBUTTON_PLANE));
//	pDC->FillRectangle(r);
//
//	OnDrawText(pButton, pButton->GetClientRect(), pDC);
//}
void MButtonLook::OnUpDraw(MButton* pButton, MDrawContext* pDC)
{
	// Để XML (Template) tự vẽ Bitmap trạng thái UP
	OnDrawText(pButton, pButton->GetClientRect(), pDC);
}

//void MButtonLook::OnOverDraw(MButton* pButton, MDrawContext* pDC)
//{
//	if(pButton->GetType()==MBT_PUSH){
//		//if(pButton->GetCheck()==true) OnDownDraw(pButton, pDC);
//		//else OnUpDraw(pButton, pDC);
//		OnCheckBoxDraw(pButton, pDC, pButton->GetCheck());
//	}
//	else{
//		MRECT r = pButton->GetInitialClientRect();
//		pDC->SetColor(MCOLOR(DEFCOLOR_MBUTTON_PLANE));
//		pDC->FillRectangle(r);
//
//		OnDrawText(pButton, pButton->GetClientRect(), pDC);
//	}
//}
void MButtonLook::OnOverDraw(MButton* pButton, MDrawContext* pDC)
{
	if (pButton->GetType() == MBT_PUSH) {
		OnCheckBoxDraw(pButton, pDC, pButton->GetCheck());
	}
	else {
		// Bỏ hoàn toàn FillRectangle và Rectangle màu Cyan ở đây
		// Để XML (Template) tự vẽ Bitmap trạng thái OVER sáng background tĩnh
		OnDrawText(pButton, pButton->GetClientRect(), pDC);
	}
}

void MButtonLook::OnDisableDraw(MButton* pButton, MDrawContext* pDC)
{
	MRECT r = pButton->GetInitialClientRect();
	pDC->SetColor(MCOLOR(DEFCOLOR_MBUTTON_PLANE));
	pDC->FillRectangle(r);

	OnDrawText(pButton, pButton->GetClientRect(), pDC);
}

void MButtonLook::OnDraw(MButton* pButton, MDrawContext* pDC)
{
	if (pButton->GetType() == MBT_PUSH)
	{
		OnCheckBoxDraw(pButton, pDC, pButton->GetCheck());
	}
	else if (pButton->IsEnable() == false)
	{
		OnDisableDraw(pButton, pDC);
	}
	else if (pButton->IsButtonDown() == true)
	{
		OnDownDraw(pButton, pDC);
	}
	// =========================================================================
	// [THÊM NHÁNH NÀY]: Nếu nút đang được Check (SetCheck(true)) thì vẽ dạng Down (Sáng dính)
	// =========================================================================
	else if (pButton->GetCheck() == true)
	{
		OnDownDraw(pButton, pDC); // Vẽ hiệu ứng đè nút / highlight nền sáng
	}
	// =========================================================================
	else if (pButton->IsMouseOver() == true)
	{
		OnOverDraw(pButton, pDC);
	}
	else
	{
		OnUpDraw(pButton, pDC);
	}
}

MRECT MButtonLook::GetClientRect(MButton* pButton, MRECT& r)
{
	return r;
}

////////////////////////////////////
// MButtonGroup

MButtonGroup::MButtonGroup()
{
	m_nCount = 0;
	m_pPrevious = NULL;
}

MButtonGroup::~MButtonGroup()
{
}
