#include "stdafx.h"

#include "ZGameClient.h"
#include "ZApplication.h"
#include "ZMsgBox.h"
#include "Mint.h"

// ZMsgBox Listener
class ZMsgBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		ZMsgBox* pMsgBox = (ZMsgBox*)pWidget;
		if (pMsgBox->GetCustomListener()) {
			bool bResult = pMsgBox->GetCustomListener()->OnCommand(pWidget, szMessage);
			pMsgBox->SetCustomListener(NULL);
			return bResult;
		}

		if(MWidget::IsMsg(szMessage, MMSGBOX_OK)==true){
			const char* pText = pWidget->GetText();

			// ÀÎ¿øÀÌ ²ËÂ÷ Á¾·á
//			if(!stricmp(pText, MGetErrorString(MERR_CLIENT_FULL_PLAYERS)))
			if(!stricmp(pText, ZErrStr(MERR_CLIENT_FULL_PLAYERS)))
			{
				if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_NETMARBLE)
				{

					ZApplication::Exit();
					return true;
				}
			}
		}

		pWidget->Show(false);

		return false;
	}
};
ZMsgBoxListener	g_MsgBoxListener;

MListener* ZGetMsgBoxListener(void)
{
	return &g_MsgBoxListener;
}

// ZConfirmMsgBox Listener : Default Listener ÀÏ»Ó. ¹°¾îº¼¶§¸¶´Ù CustomListener ÁöÁ¤ÇØ¼­ ¾µ°Í
class ZConfirmMsgBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		ZMsgBox* pMsgBox = (ZMsgBox*)pWidget;
		if (pMsgBox->GetCustomListener()) {
			bool bResult = pMsgBox->GetCustomListener()->OnCommand(pWidget, szMessage);
			pMsgBox->SetCustomListener(NULL);
			return bResult;
		}

		pWidget->Show(false);
		return false;
	}
};
ZConfirmMsgBoxListener	g_CofirmMsgBoxListener;

MListener* ZGetConfirmMsgBoxListener(void)
{
	return &g_CofirmMsgBoxListener;
}


#define TRANSIENT_TIME	200

void ZMsgBox::Show(bool bVisible, bool bModal)
{
	DWORD elapsed=(timeGetTime()-m_nShowTime);

	if(m_bNextVisible==m_bVisible && m_bVisible==bVisible && elapsed>TRANSIENT_TIME)
		return;

	// »óÅÂ°¡ º¯°æµÇ´Â Áß°£¿¡ ´Ù¸¥¹æÇâÀ¸·Î °¡¾ßÇÒ°æ¿ì
	if(m_bNextVisible!=bVisible){
		if(elapsed<TRANSIENT_TIME)
			m_nShowTime=timeGetTime()-(TRANSIENT_TIME-elapsed);
		else
			m_nShowTime=timeGetTime();
	}
	m_bNextVisible = bVisible;

	MMsgBox::Show(bVisible,bModal);
	m_bVisible = true;

	Enable(bVisible);
}

void ZMsgBox::OnDraw(MDrawContext* pDC)
{
	float fOpacity = 0;
	if(m_bNextVisible==false){	// Hide
		fOpacity = 1.0f-min(float(timeGetTime()-m_nShowTime)/(float)TRANSIENT_TIME, 1.0f);
		if(fOpacity==0.0f) {
			m_bVisible = false;
			m_bExclusive = false;
		}
		SetOpacity(unsigned char(fOpacity*0xFF));
	}
	else{	// Show
		fOpacity = min(float(timeGetTime()-m_nShowTime)/(float)TRANSIENT_TIME, 1.0f);
		SetOpacity(unsigned char(fOpacity*0xFF));
	}

	MMsgBox::OnDraw(pDC);
}

ZMsgBox::ZMsgBox(const char* szMessage, MWidget* pParent, MListener* pListener, MMsgBoxType nType)
	: MMsgBox(szMessage, pParent, pListener, nType)
{
	m_bCanShade = false;
	m_bNextVisible = false;
	m_nShowTime = timeGetTime() - TRANSIENT_TIME * 2;
	SetOpacity(0);
	m_pCustomListener = NULL;

	if (ZGetGameInterface() && ZGetGameInterface()->GetIDLResource())
	{
		// 1. Ép kiểu về MButtonLook* từ hàm FindButtonLook vừa tạo ở MIDLResource
		MButtonLook* pBtnLook = (MButtonLook*)ZGetGameInterface()->GetIDLResource()->FindButtonLook("SteamBlueButtonLock");

		// 2. Dùng ChangeCustomLook để gán Look riêng cho nút OK & Cancel
		if (pBtnLook)
		{
			if (m_pOK) m_pOK->ChangeCustomLook(pBtnLook);
			if (m_pCancel) m_pCancel->ChangeCustomLook(pBtnLook);
		}
	}
}

ZMsgBox::~ZMsgBox(void)
{
}