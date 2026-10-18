#include "stdafx.h"
#include "MMsgBox.h"
#include "MStringTable.h"
#include "Mint.h"

int charinstr(char* sz, char c)
{
	int len = strlen(sz);
	for(int i=0; i<len; i++){
		if(sz[i]==c) return i;
	}
	return -1;
}

/*
void MMsgBox::OnDrawText(MDrawContext* pDC)
{
	//MFrameDrawer* pFD = GetFrameDrawer();

	pDC->SetColor(MCOLOR(255, 255, 255));

	char* pStr = m_szMessage;
	int nLen = charinstr(pStr, '\n');
	int nY = m_Rect.y+10;
	if(nLen<0){
		//pFD->Text(pDC, MPOINT(m_Rect.x+10, m_Rect.y+10), pStr, MFDTS_NORMAL, false, &m_Rect);
		pDC->Text(m_Rect.x+10, m_Rect.y+10, pStr);
	}
	else{
		do{
			char temp[MWIDGET_NAME_LENGTH];
			memcpy(temp, pStr, nLen);
			temp[nLen] = 0;
			//pFD->Text(pDC, MPOINT(m_Rect.x+10, nY), temp, MFDTS_NORMAL, false, &m_Rect);
			pDC->Text(m_Rect.x+10, nY, temp);
			pStr += (nLen+1);
			nLen = charinstr(pStr, '\n');
			nY += GetFont()->GetHeight();
		}while(nLen>0);
	}
}

void MMsgBox::OnDraw(MDrawContext* pDC)
{
	OnDrawText(pDC);
}
*/



#define MMSGBOX_W	CONVERT800(800)
#define MMSGBOX_X	(MGetWorkspaceWidth()/2-MMSGBOX_W/2)
#define MMSGBOX_H	CONVERT600(196)
#define MMSGBOX_Y	(MGetWorkspaceHeight()/2-MMSGBOX_H/2)

//#define MMSGBOX_OK_W	80
//#define MMSGBOX_OK_H	32
#define MMSGBOX_OK_W	CONVERT800(117)
#define MMSGBOX_OK_H	CONVERT600(36)

// ¿¹Àü µðÆÄÀÎ °ª
/*#define MMSGBOX_W	400
#define MMSGBOX_X	(MGetWorkspaceWidth()/2-MMSGBOX_W/2)
#define MMSGBOX_H	140
#define MMSGBOX_Y	(MGetWorkspaceHeight()/2-MMSGBOX_H/2)

#define MMSGBOX_OK_W	80
#define MMSGBOX_OK_H	32
*/
bool MMsgBox::OnShow(void)
{
  	AdjustSize();
    
	if(m_pOK!=NULL && m_pOK->IsVisible()==true) 
		m_pOK->SetFocus();
	else if(m_pCancel!=NULL) m_pCancel->SetFocus();

	return true;
}

void MMsgBox::OnSize(int w, int h)
{
	MRECT r = GetInitialClientRect();

	int btnY = r.y + r.h - MMSGBOX_OK_H - CONVERT600(10); //nút số càng nhỏ càng sát lề dưới khung
	int msgY = r.y + CONVERT600(65); //nội dung chữ, số càng lớn đẩy xuống càng sâu
	int msgH = btnY - msgY - CONVERT600(10); 

	// Thu hẹp chiều rộng khung m_pMessage từ bên ngoài và đẩy tọa độ X vào giữa
	int nMsgW = r.w - CONVERT800(80); // Thu hẹp khung lại
	int nMsgX = r.x + (r.w - nMsgW) / 2; // Tự động đưa khung vào chính giữa MsgBox

	if (m_pMessage)
	{
		m_pMessage->SetBounds(MRECT(nMsgX, msgY, nMsgW, msgH));
	}

	// Code căn giữa Nút OK / CANCEL (giữ nguyên)
	if (m_nType == MT_OKCANCEL || m_nType == MT_YESNO)
	{
		int nSpacing = CONVERT800(20);
		int nTotalWidth = (MMSGBOX_OK_W * 2) + nSpacing;
		int nStartX = r.x + (r.w - nTotalWidth) / 2;

		if (m_pOK != NULL)
			m_pOK->SetBounds(MRECT(nStartX, btnY, MMSGBOX_OK_W, MMSGBOX_OK_H));

		if (m_pCancel != NULL)
			m_pCancel->SetBounds(MRECT(nStartX + MMSGBOX_OK_W + nSpacing, btnY, MMSGBOX_OK_W, MMSGBOX_OK_H));
	}
	else
	{
		if (m_pOK != NULL)
		{
			int btnX = r.x + (r.w - MMSGBOX_OK_W) / 2;
			m_pOK->SetBounds(MRECT(btnX, btnY, MMSGBOX_OK_W, MMSGBOX_OK_H));
		}
	}
}

void MMsgBox::MultiplySize( float byIDLWidth, float byIDLHeight, float byCurrWidth, float byCurrHeight )
{
	// OnSize¿¡¼­ È­¸é ÇØ»óµµ¿¡ ¸Â°Ô Àç¼³Á¤ÇÏ°í ÀÖÀ¸¹Ç·Î Æ¯º°È÷ ¹èÀ²À» Àû¿ëÇÒ ÇÊ¿ä°¡ ¾ø´Ù
	AdjustSize();
}

MMsgBox::MMsgBox(const char* szMessage, MWidget* pParent, MListener* pListener, MMsgBoxType nType)
	: MFrame(MGetString(MSID_MESSAGE), pParent, (pListener == NULL) ? pParent : pListener)
{
	m_pMessage = new MTextArea(255, szMessage, this, this);
	m_pMessage->SetEditable(false);

	m_pOK = NULL;
	m_pCancel = NULL;

	if (nType != MT_NOTDECIDED)
		SetType(nType);

	m_bResizable = false;

	AdjustSize();

	Show(false);
}

MMsgBox::~MMsgBox(void)
{
	delete m_pMessage;
	if(m_pOK!=NULL) delete m_pOK;
	if(m_pCancel!=NULL) delete m_pCancel;
}

void MMsgBox::SetType(MMsgBoxType nType)
{
	m_nType = nType;

	if(m_pOK) delete m_pOK;
	if(m_pCancel) delete m_pCancel;

	if(nType!=MT_CANCEL){
		//m_pOK = new MButton(MGetString(MSID_OK), this, this);
		m_pOK = (MButton*)Mint::GetInstance()->NewWidget(MINT_BUTTON, MGetString(MSID_OK), this, this );
		m_pOK->SetLabelAccelerator();
		m_pOK->m_nKeyAssigned = MBKA_ENTER;
	}
	if(nType==MT_YESNO) m_pOK->SetText(MGetString(MSID_YES));

	if(nType==MT_OKCANCEL || nType==MT_YESNO || nType==MT_CANCEL){
		//m_pCancel = new MButton(MGetString(MSID_CANCEL), this, this);
		m_pCancel = (MButton*)Mint::GetInstance()->NewWidget(MINT_BUTTON, MGetString(MSID_CANCEL), this, this );
		if(nType==MT_YESNO) m_pCancel->SetText(MGetString(MSID_NO));
		m_pCancel->SetAccelerator(27);
		m_pCancel->m_nKeyAssigned = MBKA_ESC;
	}
	MMsgBox::OnSize(MMSGBOX_W, MMSGBOX_H);
}

bool MMsgBox::OnCommand(MWidget* pWindow, const char* szMessage)
{
	MListener* pListener = GetListener();
	if(pListener==NULL) return false;

	if(pWindow==m_pOK && strcmp(szMessage, MBTN_CLK_MSG)==0){
		if(m_nType==MT_YESNO) pListener->OnCommand(this, MMSGBOX_YES);
		else pListener->OnCommand(this, MMSGBOX_OK);
		return true;
	}
	else if(pWindow==m_pCancel && strcmp(szMessage, MBTN_CLK_MSG)==0){
		if(m_nType==MT_YESNO) pListener->OnCommand(this, MMSGBOX_NO);
		else pListener->OnCommand(this, MMSGBOX_CANCEL);
		return true;
	}

	return false;
}

void MMsgBox::SetTitle(const char* szTitle)
{
	strcpy(m_szName, szTitle);
}

const char* MMsgBox::GetTitle(void)
{
	return m_szName;
}

void MMsgBox::SetMessage(const char* szMessage)
{
	m_pMessage->SetText(szMessage);
	
	AdjustSize();
}

const char* MMsgBox::GetMessage(void)
{
	static char buffer[1024];
	buffer[0]=0;
	if(m_pMessage->GetText(buffer,sizeof(buffer)))
		return buffer;
	return buffer;
}

void MMsgBox::SetText(const char* szText)
{
	SetMessage(szText);
}

const char* MMsgBox::GetText(void)
{
	return GetMessage();
}

void MMsgBox::AdjustSize()
{
	int nLineCount = MMGetLineCount(m_pMessage->GetFont(), GetMessage(), MMSGBOX_W);

	// Tự động tính lại vị trí X, Y và Chiều cao vừa vặn giữa màn hình
	int nHeight = MMSGBOX_H + (nLineCount * m_pMessage->GetFont()->GetHeight());
	int nX = (MGetWorkspaceWidth() - MMSGBOX_W) / 2;
	int nY = (MGetWorkspaceHeight() - nHeight) / 2;

	SetBounds(MRECT(nX, nY, MMSGBOX_W, nHeight));
}