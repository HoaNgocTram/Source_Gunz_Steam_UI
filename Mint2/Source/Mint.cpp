#include "stdafx.h"
#include "Mint.h"
#include "MWidget.h"
#include "TChar.h"
#include "MFileDialog.h"
#include "MPopupMenu.h"
#include "MListBox.h"
#include "MMultiColListBox.h"
#include "MComboBox.h"
#include "MGroup.h"
#include "MSlider.h"
#include "MHyperText.h"
#include "MTextArea.h"
#include "MResource.h"
#include "MDragObject.h"
#include "MPicture.h"
#include "MAnimation.h"
#include "MHotKey.h"
//#include "MActionKey.h"
#include "MBmButton.h"
#include "MTabCtrl.h"
#include "MPanel.h"

class MMainFrame : public MWidget{
protected:
protected:
	virtual void OnDraw(MDrawContext* pDC){
		// Draw Nothing
	}
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		return false;
	}
public:
	MMainFrame(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL)
		: MWidget(szName, pParent, pListener){
	}
	virtual ~MMainFrame(void){
	}
};


Mint* Mint::m_pInstance = NULL;

// Hàm phụ trợ lấy CodePage dựa theo bộ gõ đang active
UINT GetCurrentIMECodePage()
{
	HKL hkl = GetKeyboardLayout(0); // Lấy bộ gõ hiện tại của người chơi
	WORD langId = LOWORD(hkl);     // Trích xuất ID ngôn ngữ (Language ID)

	switch (PRIMARYLANGID(langId))
	{
	case LANG_CHINESE:
		return 936;   // Bảng mã GBK / Tiếng Trung giản thể
	case LANG_KOREAN:
		return 949;   // Bảng mã Unified Hangul / Tiếng Hàn
	case LANG_JAPANESE:
		return 932;   // Bảng mã Shift-JIS / Tiếng Nhật
	case LANG_THAI:
		return 874;   // Bảng mã Tiếng Thái
	case LANG_VIETNAMESE:
		return 65001; // UTF-8 (Cho các bộ gõ tiếng Việt hiện đại)
	default:
		return CP_ACP; // Trả về mặc định hệ thống cho các nước dùng hệ chữ Latinh
	}
}

void Mint::DrawCandidateList(MDrawContext* pDC, MPOINT& p)
{
	if (GetCandidateCount() > 0) {
		MCOLOR c = pDC->GetColor();

		MRECT r(p.x, p.y, GetCandidateListWidth(), GetCandidateListHeight());

		pDC->SetColor(MCOLOR(0xFF050505));
		pDC->FillRectangle(r);

		pDC->SetColor(MCOLOR(0xFF505050));
		pDC->Rectangle(r.x + 1, r.y + 1, r.w, r.h);

		pDC->SetColor(MCOLOR(0xFFB0B0B0));
		pDC->Rectangle(r);

		MFont* pFont = pDC->GetFont();
		pDC->SetFont(MFontManager::Get("Default"));

		int nStart = GetCandidatePageStart();

		wchar_t tempW[MIMECOMPOSITIONSTRING_LENGTH + 4];
		char tempA[MIMECOMPOSITIONSTRING_LENGTH * 2];

		// 🚀 TỰ ĐỘNG LẤY CODEPAGE CỦA BỘ GÕ HIỆN TẠI
		UINT nTargetCodePage = GetCurrentIMECodePage();

		for (int i = nStart; i < (int)min(GetCandidateCount(), nStart + GetCandidatePageSize()); i++)
		{
			const char* szCandidate = GetCandidate(i);
			if (i == GetCandidateSelection())
				pDC->SetColor(MCOLOR(DEFCOLOR_NORMAL));
			else
				pDC->SetColor(MCOLOR(0xFF909090));

			int nIndexInPage = i - nStart;

			// ✅ SỬA ĐỔI: Dùng CodePage tự động nhận diện theo quốc gia bàn phím
			wchar_t szCandidateW[MIMECOMPOSITIONSTRING_LENGTH];
			MultiByteToWideChar(nTargetCodePage, 0, szCandidate, -1, szCandidateW, MIMECOMPOSITIONSTRING_LENGTH);

			swprintf(tempW, L"%d: %s", nIndexInPage + 1, szCandidateW);

			// Ép ngược lại thành UTF-8 để bộ Engine vẽ Text đa ngôn ngữ của GunZ render lên màn hình
			WideCharToMultiByte(CP_UTF8, 0, tempW, -1, tempA, sizeof(tempA), NULL, NULL);

			pDC->Text(p.x + 4, p.y + nIndexInPage * pDC->GetFont()->GetHeight() + 4, tempA);
		}

		// Định dạng số trang (X/Y)
		sprintf(tempA, "(%d/%d)", GetCandidateSelection() + 1, GetCandidateCount());
		pDC->SetColor(MCOLOR(DEFCOLOR_NORMAL));
		pDC->Text(p.x + 4, p.y + GetCandidatePageSize() * pDC->GetFont()->GetHeight() + 4, tempA);

		pDC->SetColor(c);
		pDC->SetFont(MFontManager::Get(pFont->m_szName));
	}
}

Mint::Mint()
{
	_ASSERT(m_pInstance==NULL);	// Singleton!!!
	m_pInstance = this;
	m_pMainFrame = NULL;
	m_pDC = NULL;
	m_hImc = NULL;

//	m_nDragObjectID = -1;
	m_szDragObjectString[0] = 0;
	m_szDragObjectItemString[0] = 0;
	m_pDragObjectBitmap = NULL;
	m_bVisibleDragObject = false;
	m_pDropableObject = NULL;
	m_pDragSourceObject = NULL;

	m_nWorkspaceWidth = 640;
	m_nWorkspaceHeight = 480;

	m_fnGlobalEventCallBack = NULL;

//	memset(m_ActionKeyPressedTable, 0, sizeof(bool)*ACTIONKEYMAP_IDCOUNT);

	m_pCandidateList = NULL;
	m_nCandidateListSize = 0;

	m_nCompositionAttributeSize = 0;
	memset(m_nCompositionAttributes, 0, sizeof(BYTE)*(MIMECOMPOSITIONSTRING_LENGTH));
	//memset(m_dwCompositionClauses, 0, sizeof(DWORD)*(MIMECOMPOSITIONSTRING_LENGTH));

	m_nCompositionCaretPosition = 0;

	m_bEnableIME = false;
}

Mint::~Mint()
{
	m_fnGlobalEventCallBack = NULL;
	m_pInstance = NULL;
}

Mint* Mint::GetInstance(void)
{
	_ASSERT(m_pInstance!=NULL);
	return m_pInstance;
}

bool Mint::Initialize(int nWorkspaceWidth, int nWorkspaceHeight, MDrawContext* pDC, MFont* pDefaultFont)
{
	_ASSERT(m_pMainFrame==NULL);

	m_pDC = pDC;

//	MFontManager::Add(pDefaultFont);
	MFontManager::SetDefaultFont(pDefaultFont);

	m_pMainFrame = new MMainFrame("Mint");
	m_pMainFrame->SetBounds(0, 0, nWorkspaceWidth, nWorkspaceHeight);

	SetWorkspaceSize(nWorkspaceWidth, nWorkspaceHeight);

	return true;
}

void Mint::Finalize(void)
{
	if(m_pMainFrame!=NULL){
		delete m_pMainFrame;
		m_pMainFrame = NULL;
	}

	MFontManager::Destroy();

	m_pDC = NULL;
}

bool Mint::ProcessEvent(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if(!m_pMainFrame) return false;

	MEvent e;
	int nResult = e.TranslateEvent(hwnd, message, wparam, lparam);
	if(nResult&EVENT_MINT_TRANSLATED){
		// Drag & Drop
		if(m_pDragSourceObject!=NULL){
#define DRAm_VISIBLE_LENGTH	2	// µå·¡±× ¿ÀºêÁ§Æ®¸¦ º¸¿©ÁÖ±â ½ÃÀÛÇÏ´Â °£°Ý
			if(e.nMessage==MWM_MOUSEMOVE){
				MPOINT p = e.Pos;
				//MPOINT p = MEvent::GetMousePos();
				int px = m_GrabPoint.x - p.x;
				if ( px < 0)  px *= -1;
				int py = m_GrabPoint.y - p.y;
				if ( py < 0)  py *= -1;

				if( m_bVisibleDragObject==false &&
				    ((px > DRAm_VISIBLE_LENGTH) || (py > DRAm_VISIBLE_LENGTH)))
                    m_bVisibleDragObject = true;
				MWidget* pFind = FindWidget(p);
				if(pFind!=NULL && pFind->IsDropable(m_pDragSourceObject)==true)
					m_pDropableObject = pFind;
				else
					m_pDropableObject = NULL;
			}
			if(e.nMessage==MWM_LBUTTONUP){
				MPOINT p = e.Pos;
				MWidget* pFind = FindWidgetDropAble(p);
//				MWidget* pFind = FindWidget(p);
				if(pFind!=NULL && pFind->IsDropable(m_pDragSourceObject)==true)
					pFind->Drop(m_pDragSourceObject, m_pDragObjectBitmap, m_szDragObjectString, m_szDragObjectItemString);	// ÇØ´ç À§Á¬¿¡ µå·Ó
				m_pDragSourceObject = NULL;
				m_pMainFrame->ReleaseCapture();
				return true;
			}
		}

		// Àü¿ª ÀÌº¥Æ® Ã³¸®
		if (m_fnGlobalEventCallBack) {
			if (m_fnGlobalEventCallBack(&e) == true) return true;
		}

		// ÀÏ¹Ý ÀÌº¥Æ® Ã³¸®
		if(m_pMainFrame->Event(&e)==true) return true;
		// ¾øÀ¸¸é Accelerator Ã³¸®
		if(m_pMainFrame->EventAccelerator(&e)==true) return true;
		// Default Key(Enter, ESC) Ã³¸®
		if(m_pMainFrame->EventDefaultKey(&e)==true) return true;

	}
	if(nResult&EVENT_PROCESSED) return true;	// ¹«Á¶°Ç Ã³¸®µÊ

	return false;
}

void Mint::Run(void)
{
	if(!m_pMainFrame) return;

	m_pMainFrame->Run();
}

void Mint::Draw(void)
{
	if(!m_pMainFrame) return;

	MDrawContext* pDC = m_pDC;
	if(pDC==NULL) return;

	pDC->SetOpacity(0xFF);
	m_pMainFrame->Draw(pDC);

	MRECT sr = m_pMainFrame->GetScreenRect();
	pDC->SetOrigin(MPOINT(sr.x, sr.y));
	pDC->SetClipRect(sr);


	// Candidate List ±×¸®±â
	DrawCandidateList(pDC, m_CandidateListPos);


	// Drag & Drop
	/*
	MDragObject* pDragObject = MGetDragObject();
	if(pDragObject!=NULL){
		pDC->SetBitmap(pDragObject->GetBitmap());
		MPOINT p = MEvent::GetMousePos();
		pDC->Draw(p);
		pDC->Text(p, pDragObject->GetString());
	}
	*/
	//int nDragObjectID = GetDragObject();
	MWidget* pDragSourceObject = GetDragObject();
	if(pDragSourceObject!=NULL && m_bVisibleDragObject==true){
		// ÇöÀç À§Ä¡¿¡¼­ µå·ÓÇÒ ¼ö ÀÖ´Â À§Á¬À» Ç¥½Ã
		if(m_pDropableObject!=NULL){
			MRECT r = m_pDropableObject->GetScreenRect();
			pDC->SetColor(0, 0, 0, 32);
			pDC->FillRectangle(r);
		}

		// Ä¿¼­¿¡ ºÙ´Â µå·¡±× ¿ÀºêÁ§Æ® ±×¸®±â
		pDC->SetBitmap(m_pDragObjectBitmap);
		MPOINT p = MEvent::GetMousePos();
		p.x-=(m_pDragObjectBitmap!=NULL?m_pDragObjectBitmap->GetWidth()/2:0);
		p.y-=(m_pDragObjectBitmap!=NULL?m_pDragObjectBitmap->GetHeight()/2:0);
		if (m_pDragObjectBitmap)
		{
			pDC->SetOpacity(128);
			pDC->Draw(p.x, p.y, m_pDragObjectBitmap->GetWidth(), m_pDragObjectBitmap->GetHeight());
			pDC->SetOpacity(0xFF);
		}
		p.x+=((m_pDragObjectBitmap!=NULL?m_pDragObjectBitmap->GetWidth():0) + 2);
		p.y+=((m_pDragObjectBitmap!=NULL?m_pDragObjectBitmap->GetHeight()/2:0)-pDC->GetFont()->GetHeight()/2);
		pDC->SetColor(0, 0, 0, 128);
		pDC->Text(p, m_szDragObjectString);
		pDC->SetColor(255, 255, 255, 128);
		p.x--;
		p.y--;
		pDC->Text(p, m_szDragObjectString);
	}
	
}

/*
bool Mint::EventActionKey(unsigned long int nKey, bool bPressed)
{
	MEvent e;

	if(bPressed==true) e.nMessage = MWM_ACTIONKEYDOWN;
	else e.nMessage = MWM_ACTIONKEYUP;
	e.nKey = nKey;

	if (m_pMainFrame == NULL)
		return false;

	if (m_fnGlobalEventCallBack) m_fnGlobalEventCallBack(&e);
	m_pMainFrame->Event(&e);

	ACTIONKEYMAP::iterator i = m_ActionKeyMap.find(nKey);
	int nActionID = -1;
	if(i!=m_ActionKeyMap.end()) nActionID = (*i).second;
	if(bPressed==true){
		e.nMessage = MWM_ACTIONPRESSED;
		if(nActionID>=0 && nActionID<ACTIONKEYMAP_IDCOUNT) m_ActionKeyPressedTable[nActionID] = true;
	}
	else{
		e.nMessage = MWM_ACTIONRELEASED;
		if(nActionID>=0 && nActionID<ACTIONKEYMAP_IDCOUNT) m_ActionKeyPressedTable[nActionID] = false;
	}
	e.nKey = nActionID;

	if (m_fnGlobalEventCallBack) m_fnGlobalEventCallBack(&e);

	return m_pMainFrame->Event(&e);
}
*/

MWidget* Mint::GetMainFrame(void)
{
	return m_pMainFrame;
}

MDrawContext* Mint::GetDrawContext(void)
{
	return m_pDC;
}

#ifdef WIN32
HWND m_hWnd = NULL;
void Mint::SetHWND(HWND hWnd)
{
	m_hWnd = hWnd;

	// Ẩn cửa sổ trạng thái IME mặc định của Windows (để game tự vẽ giao diện riêng)
	HWND imehwnd = ImmGetDefaultIMEWnd(Mint::GetInstance()->GetHWND());
	if (imehwnd) {
		SendMessage(imehwnd, WM_IME_CONTROL, IMC_CLOSESTATUSWINDOW, 0);
	}

	// Tắt bộ gõ lúc vừa mở game (khi nào bật chat mới kích hoạt lại)
	Mint::EnableIME(false);
}
HWND Mint::GetHWND(void)
{
	return m_hWnd;
}
#endif

void Mint::EnableIME(bool bEnable)
{
	_ASSERT(GetHWND());

	HIMC hImc = ImmGetContext(GetHWND());
	if (hImc)
	{
		if (bEnable)
		{
			// 1. Kích hoạt trạng thái mở bộ gõ
			ImmSetOpenStatus(hImc, TRUE);

			// 2. Cấu hình chế độ chuyển đổi cho phép nhận ký tự mở rộng (Unicode / Telex)
			DWORD dwConvMode, dwSentMode;
			ImmGetConversionStatus(hImc, &dwConvMode, &dwSentMode);

			// Bật Native chuyển đổi và Chặn lọc phím điều hướng rác của Windows IME
			dwConvMode |= IME_CMODE_NATIVE;
			dwConvMode &= ~IME_CMODE_ALPHANUMERIC;

			// Thêm flag hỗ trợ nhận diện đầy đủ ký tự dấu toàn phần
			dwConvMode |= IME_CMODE_FULLSHAPE;

			ImmSetConversionStatus(hImc, dwConvMode, dwSentMode);
		}
		else
		{
			// Hủy toàn bộ chuỗi ký tự đang gõ dở để không bị kẹt chữ rác
			ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
			ImmSetOpenStatus(hImc, FALSE);
		}

		ImmReleaseContext(GetHWND(), hImc);
	}

	m_bEnableIME = bEnable;

	// 🛠️ BẮT BUỘC: Thông báo cho Windows cập nhật lại ngữ cảnh cửa sổ ngay lập tức
	// Nếu không có dòng này, bộ gõ Windows sẽ bị "treo luồng" không gửi chữ vào game
	PostMessage(GetHWND(), WM_IME_NOTIFY, IMN_SETCONVERSIONMODE, 0);
}

bool Mint::IsEnableIME(void)
{
	return m_bEnableIME;
}


int Mint::RegisterHotKey(unsigned long int nModifier, unsigned long int nVirtKey)
{
#ifdef WIN32
	_ASSERT(m_hWnd!=NULL);	// Should call SetHWND() before this function

	char szAtomName[64] = {0,};
	if(nModifier==MMODIFIER_ALT) strcat(szAtomName, "Alt");
	if(nModifier==MMODIFIER_CTRL) strcat(szAtomName, "Ctrl");
	if(nModifier==MMODIFIER_SHIFT) strcat(szAtomName, "Shift");
	char szKey[16] = {0, };
	sprintf(szKey, "%d", nVirtKey);
	strcat(szAtomName, szKey);

	int nID = GlobalAddAtom(szAtomName);
	if(nID==0) return 0;

	if(::RegisterHotKey(m_hWnd, nID, nModifier, nVirtKey)==TRUE)
		return nID;

	return 0;
#else
	// Not Implemented
	_ASSERT(FALSE);
	return 0;
#endif
}
void Mint::UnregisterHotKey(int nID)
{
#ifdef WIN32
	_ASSERT(m_hWnd!=NULL);	// Should call SetHWND() before this function
	::UnregisterHotKey(m_hWnd, nID);
	GlobalDeleteAtom(nID);
#else
	// Not Implemented
	_ASSERT(FALSE);
	return 0;
#endif
}

/*
bool Mint::RegisterActionKey(int nActionID, unsigned long int nKey)
{
	if(nActionID<0 || nActionID>=ACTIONKEYMAP_IDCOUNT){
		_ASSERT(FALSE);	// 0 ~ ACTIONKEYMAP_IDCOUNT-1 »çÀÌ°ªÀÌ¿©¾ß ÇÑ´Ù.
		return false;
	}

	m_ActionKeyMap.insert(ACTIONKEYMAP::value_type(nKey, nActionID));
	return true;
}

bool Mint::UnregisterActionKey(int nActionID)
{
	if(nActionID<0 || nActionID>=ACTIONKEYMAP_IDCOUNT){
		_ASSERT(FALSE);	// 0 ~ ACTIONKEYMAP_IDCOUNT-1 »çÀÌ°ªÀÌ¿©¾ß ÇÑ´Ù.
		return false;
	}

	for(ACTIONKEYMAP::iterator i=m_ActionKeyMap.begin(); i!=m_ActionKeyMap.end(); i++){
		if((*i).second==nActionID){
			m_ActionKeyMap.erase(i);
			return true;
		}
	}
	return false;
}

const char* Mint::GetActionKeyName(unsigned long int nKey)
{
	_ASSERT(FALSE);
	// Not Implemented. User must implement

	static char* szNullName = "Unknown";
	return szNullName;
}

unsigned long int Mint::GetActionKey(int nActionID)
{
	for(ACTIONKEYMAP::iterator i=m_ActionKeyMap.begin(); i!=m_ActionKeyMap.end(); i++){
		if((*i).second==nActionID){
			return (*i).first;
		}
	}
	return -1;
}

bool Mint::IsActionKeyPressed(int nActionID)
{
	if(nActionID<0 || nActionID>=ACTIONKEYMAP_IDCOUNT){
		_ASSERT(FALSE);	// 0 ~ ACTIONKEYMAP_IDCOUNT-1 »çÀÌ°ªÀÌ¿©¾ß ÇÑ´Ù.
		return false;
	}
	return m_ActionKeyPressedTable[nActionID];
}
*/

void Mint::SetGlobalEvent(MGLOBALEVENTCALLBACK pGlobalEventCallback)
{
	m_fnGlobalEventCallBack = pGlobalEventCallback;
}

/*
MDragObject* m_pDragObject = NULL;

MDragObject* MSetDragObject(MDragObject* pDragObject)
{
	MDragObject* pPrevDragObject = m_pDragObject;
	m_pDragObject = pDragObject;
	return pPrevDragObject;
}
MDragObject* MGetDragObject(void)
{
	return m_pDragObject;
}
*/

MWidget* Mint::SetDragObject(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	MWidget* pPrev = m_pDragSourceObject;
	m_pDragSourceObject = pSender;
	if(szString==NULL) m_szDragObjectString[0] = 0;
	else strncpy(m_szDragObjectString, szString, 256);
	if(szItemString==NULL) m_szDragObjectItemString[0] = 0;
	else strncpy(m_szDragObjectItemString, szItemString, 256);
	m_pDragObjectBitmap = pBitmap;
	m_GrabPoint = MEvent::GetMousePos();
	m_bVisibleDragObject = false;
	return pPrev;
}
MWidget* Mint::GetDragObject(void)
{
	return m_pDragSourceObject;
}

/*
#define MINT_WIDGET	"GenericWidget"
#define MINT_LABEL			"Label"
#define MINT_BUTTON			"Button"
#define MINT_EDIT			"Edit"
#define MINT_LISTBOX		"ListBox"
#define MINT_COMBOBOX		"ComboBox"
#define MINT_FILEBOX		"FileBox"
#define MINT_FRAME			"Frame"
#define MINT_GROUP			"Group"
#define MINT_POPUPMENU			"Menu"
#define MINT_SCROLLBAR		"ScrollBar"
#define MINT_SLIDER			"Slider"
#define MINT_TEXTAREA		"TextArea"
#define MINT_MSGBOX		"MessageBox"
*/

/*
char* m_szWidgetClasses[] = {
	MINT_WIDGET,
	MINT_LABEL,
	MINT_BUTTON,
	MINT_EDIT,
	MINT_LISTBOX,
	MINT_COMBOBOX,
	MINT_FILEBOX,
	MINT_FRAME,
	MINT_GROUP,
	MINT_POPUPMENU,
	MINT_SCROLLBAR,
	MINT_SLIDER,
	MINT_TEXTAREA,
	MINT_MSGBOX,
};
*/

MWidget* Mint::NewWidget(const char* szClass, const char* szName, MWidget* pParent, MListener* pListener)
{
	if(strcmp(szClass, MINT_WIDGET)==0) return new MWidget(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_LABEL)==0) return new MLabel(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_BUTTON)==0) return new MButton(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_BMBUTTON)==0) return new MBmButton(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_EDIT)==0) return new MEdit(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_LISTBOX)==0) return new MListBox(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_MULTICOLLISTBOX)==0) return new MMultiColListBox(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_COMBOBOX)==0) return new MComboBox(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_FILEBOX)==0) return new MFileBox(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_FRAME)==0) return new MFrame(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_GROUP)==0) return new MGroup(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_POPUPMENU)==0) return new MPopupMenu(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_SCROLLBAR)==0) return new MScrollBar(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_SLIDER)==0) return new MSlider(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_TEXTAREA)==0) return new MTextArea(256, szName, pParent, pListener);
	else if(strcmp(szClass, MINT_TABCTRL)==0) return new MTabCtrl(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_MSGBOX)==0) return new MMsgBox(szName, pParent, pListener, MT_OK);
	else if(strcmp(szClass, MINT_PICTURE)==0) return new MPicture(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_ANIMATION)==0) return new MAnimation(szName, NULL, pParent);
	else if(strcmp(szClass, MINT_MENUITEM)==0) return new MMenuItem(szName);
	else if(strcmp(szClass, MINT_HOTKEY)==0) return new MHotKey(szName, pParent, pListener);
//	else if(strcmp(szClass, MINT_ACTIONKEY)==0) return new MActionKey(szName, pParent, pListener);
	else if(strcmp(szClass, MINT_PANEL)==0) return new MPanel(szName, pParent, pListener);

	else _ASSERT(false);

	return new MWidget(szName, pParent, pListener);
}

MWidget* Mint::FindWidgetDropAble(MPOINT& p)
{
	return m_pMainFrame->FindDropAble(p);
}

MWidget* Mint::FindWidget(MPOINT& p)
{
	return m_pMainFrame->Find(p);
}

MWidget* Mint::FindWidget(int x, int y)
{
	return m_pMainFrame->Find(x, y);
}


int Mint::GetWorkspaceWidth(void)
{
	return m_nWorkspaceWidth;
}

int Mint::GetWorkspaceHeight(void)
{
	return m_nWorkspaceHeight;
}

void Mint::SetWorkspaceSize(int w, int h)
{
	m_nWorkspaceWidth = w;
	m_nWorkspaceHeight = h;
}

const char* Mint::GetDefaultFontName(void) const
{
	static char* szFontName[] = {
		"Arial",		// 0: Thay Tahoma bằng Arial (Arial Win mới nuốt được dải Unicode đa năng)
		"Gulim",		// 1: Hàn Quốc
		"MS PGothic",	// 2: Nhật Bản
		"MingLiU",		// 3: Trung Quốc Phồn Thể
		"SimSun",		// 4: Đổi NSimSun thành SimSun (Font Trung chuẩn)
		"Arial",	    // 5: Tiếng Thái
		"Arial"		    // 6: Tiếng Ấn Độ
	};

	int nFont = 0;
	int nLang = GetPrimaryLanguageIdentifier();

	if (nLang == LANG_KOREAN) nFont = 1;
	else if (nLang == LANG_JAPANESE) nFont = 2;
	else if (nLang == LANG_CHINESE) {
		if (GetSubLanguageIdentifier() == SUBLANG_CHINESE_TRADITIONAL) nFont = 3;
		else nFont = 4;
	}
	// Nếu máy đang ở bộ gõ Việt Nam hoặc mặc định quốc tế, ép dùng Arial xịn để ôm dải Unicode rộng hơn Tahoma cổ lỗ sĩ
	else {
		nFont = 0;
	}

	return szFontName[nFont];
}

int Mint::GetPrimaryLanguageIdentifier(void) const
{
	/*
	Language Identifiers
	+-------------------------+-------------------------+
	|      SubLanguage ID     |   Primary Language ID   |
	+-------------------------+-------------------------+
	15                    10  9                         0   bit

	ms-help://MS.VSCC.2003/MS.MSDNQTR.2003FEB.1042/intl/nls_8xo3.htm
	ms-help://MS.VSCC.2003/MS.MSDNQTR.2003FEB.1042/intl/nls_238z.htm
	*/
	HKL hKeyboardLayout = GetKeyboardLayout(0);
	WORD nLanguageIdentifier = LOWORD(hKeyboardLayout);
	return PRIMARYLANGID(nLanguageIdentifier);
}

int Mint::GetSubLanguageIdentifier(void) const
{
	HKL hKeyboardLayout = GetKeyboardLayout(0);
	WORD nLanguageIdentifier = LOWORD(hKeyboardLayout);
	return SUBLANGID(nLanguageIdentifier);
}

const char* Mint::GetLanguageIndicatorString(void) const
{
	// Mở rộng mảng hiển thị nhãn ngôn ngữ hệ thống
	static char* szIndicator[] = {
		"EN", "KO", "JP", "CH", "VN", "PT", "TH", "IN"
	};

	int nLangID = GetPrimaryLanguageIdentifier();

	// Kiểm tra nếu mã ngôn ngữ là Tiếng Việt (0x2A)
	if (nLangID == 0x2A || GetKeyboardLayout(0) == (HKL)0x042A042A || GetKeyboardLayout(0) == (HKL)0x042A0409)
	{
		return szIndicator[4]; // VN
	}

	switch (nLangID) {
	case LANG_ENGLISH:
		return szIndicator[0];
	case LANG_KOREAN:
		return szIndicator[1];
	case LANG_JAPANESE:
		return szIndicator[2];
	case LANG_CHINESE:
		return szIndicator[3];
	case LANG_PORTUGUESE:
		return szIndicator[5];
	case LANG_THAI:
		return szIndicator[6]; // Trả về nhãn "TH" cho tiếng Thái
	case LANG_HINDI:
		return szIndicator[7]; // Trả về nhãn "IN" cho tiếng Ấn Độ
	default:
		return szIndicator[0];
	}
}

bool Mint::IsNativeIME(void) const
{
	HIMC hImc = ImmGetContext(GetInstance()->GetHWND());
	if(hImc==NULL) return false;

	bool bNative = false;

	if(GetPrimaryLanguageIdentifier()==LANG_JAPANESE){
		// ÀÏº»¾îÀÎ°æ¿ì ConversionMode°¡ ¾Æ´Ñ OpenSatus·Î ÀÔ·Â±â°¡ È°¼ºÈ­ µÇ¾ú´ÂÁö ¾Ë ¼ö ÀÖ´Ù.
		// ÀÏº»¾î IME ÀÌ»óÇÔ!
		bNative = (ImmGetOpenStatus(hImc)==TRUE);
	}
	else{
		// ¿µ¹®(IME_CMODE_ALPHANUMERIC) ¶Ç´Â ³×ÀÌÆ¼ºê(IME_CMODE_NATIVE) ¾ð¾îÀÎÁö¸¦ ÆÇº°
		DWORD dwConvMode, dwSentMode;
		ImmGetConversionStatus(hImc, &dwConvMode, &dwSentMode);

		bNative = (dwConvMode&IME_CMODE_NATIVE);
	}

	ImmReleaseContext(GetInstance()->GetHWND(), hImc);

	return bNative;
}

void Mint::OpenCandidateList(void)
{
	// Candidate List ¾ò¾î³»±â
	HIMC hImc = ImmGetContext(GetInstance()->GetHWND());
	if(hImc==NULL) return;

	m_nCandidateListSize = ImmGetCandidateList(hImc, 0, NULL, 0);

	if(m_pCandidateList!=NULL) delete[] m_pCandidateList;
	m_pCandidateList = new unsigned char[m_nCandidateListSize];

	if(m_nCandidateListSize>0) { 
		ImmGetCandidateList(hImc, 0, (LPCANDIDATELIST)m_pCandidateList, m_nCandidateListSize);
	}

	ImmReleaseContext(GetInstance()->GetHWND(), hImc);
}

void Mint::CloseCandidateList(void)
{
	// Candidate List ´ÝÈû
	if(m_pCandidateList!=NULL) delete[] m_pCandidateList;
	m_pCandidateList = NULL;
	m_nCandidateListSize = 0;
}

const char* Mint::GetCandidate(int nIndex) const
{
	if(m_pCandidateList==NULL) return NULL;

	CANDIDATELIST* pCandidateList = (CANDIDATELIST*)m_pCandidateList;

	if(nIndex>=(int)pCandidateList->dwCount) return NULL;

	char* pCandidate = (char*)((BYTE*)pCandidateList+pCandidateList->dwOffset[nIndex]);
	return pCandidate;
}

int Mint::GetCandidateCount(void) const
{
	if(m_pCandidateList==NULL) return 0;

	CANDIDATELIST* pCandidateList = (CANDIDATELIST*)m_pCandidateList;

	return pCandidateList->dwCount;
}

int Mint::GetCandidateSelection(void) const
{
	if(m_pCandidateList==NULL) return 0;

	CANDIDATELIST* pCandidateList = (CANDIDATELIST*)m_pCandidateList;

	return pCandidateList->dwSelection;
}

int Mint::GetCandidatePageStart(void) const
{
	if(m_pCandidateList==NULL) return 0;

	// GetCandidatePageStart(); °¡ ÀÏº»¾î¿¡¼­ ¹ö±×°¡ ÀÖÀ¸¹Ç·Î, ¼öµ¿À¸·Î °è»ê
	int nStart = GetCandidatePageSize() * (GetCandidateSelection()/GetCandidatePageSize());

	return nStart;

	/*
	// ÀÏº»¾î¸¦ Á¦¿ÜÇÏ°í ÀÛµ¿µÇ´Â ¿ø·¡ ÄÚµå
	if(m_pCandidateList==NULL) return 0;

	CANDIDATELIST* pCandidateList = (CANDIDATELIST*)m_pCandidateList;

	return pCandidateList->dwPageStart;
	*/
}

int Mint::GetCandidatePageSize(void) const
{
	if(m_pCandidateList==NULL) return 0;

	CANDIDATELIST* pCandidateList = (CANDIDATELIST*)m_pCandidateList;

	return pCandidateList->dwPageSize;
}

void Mint::SetCandidateListPosition(MPOINT& p, int nWidgetHeight)
{
	MPOINT cp = p;

	// °¡·Î ¿µ¿ª Ã¼Å©
	if((cp.x+GetCandidateListWidth())>=MGetWorkspaceWidth()){
		cp.x = MGetWorkspaceWidth()-GetCandidateListWidth();
	}
	else{
//		cp.x -= 4;
	}
	// ¼¼·Î ¿µ¿ª Ã¼Å©
	if((cp.y+GetCandidateListHeight()+nWidgetHeight+8)>=MGetWorkspaceHeight()){
		cp.y -= GetCandidateListHeight() + 6;
	}
	else{
		cp.y += (nWidgetHeight+6);
	}

	m_CandidateListPos = cp;
}

int Mint::GetCandidateListWidth(void)
{
	int w = 60;
	if(GetCandidateCount()>0){
		const char* szCandidate = GetCandidate(0);
		w = max(w, MFontManager::Get( "Default")->GetWidth(szCandidate)+100);	// ´Ù¸¥ ¹®ÀÚ¿­ÀÇ ³Êºñ°¡ ´õ Å¬ ¼ö ÀÖÀ¸¹Ç·Î ¿©À¯°ªÀ» ÃæºÐÈ÷ ÁØ´Ù.
	}
	return w + 4;
}

int Mint::GetCandidateListHeight(void)
{
	return (MFontManager::Get( "Default")->GetHeight()*(GetCandidatePageSize()+1) + 6);
}

// ¸ÖÆ¼¶óÀÎ Áö¿øÀ» À§ÇØ ±ÛÀÚ ´ÜÀ§ Ãâ·Â
int Mint::DrawCompositionAttribute(MDrawContext* pDC, MPOINT& p, const char* szComposition, int i)
{
	if(i>=(int)strlen(szComposition)) return 0;
	//if(pMint->GetPrimaryLanguageIdentifier()!=LANG_JAPANESE) return;

	const BYTE* pCompAttr = GetCompositionAttributes();
	DWORD nCompAttrSize = GetCompositionAttributeSize();

	if(i>=(int)nCompAttrSize) return 0;	// Composition Attribute ¹üÀ§¸¦ ¹þ¾î³ª´Â °æ¿ì

	MFont* pFont = pDC->GetFont();
	int nFontHeight = pFont->GetHeight();
	MCOLOR c = pDC->GetColor();

	int nCharSize = 1;
	bool bTwoByteChar = IsHangul(szComposition[i]);
	if(bTwoByteChar) nCharSize = 2;
	int nWidth = pFont->GetWidth(&(szComposition[i]), nCharSize);

	if(pCompAttr[i]==ATTR_TARGET_CONVERTED)				// º¯È¯µÉ ³»¿ë
		pDC->SetColor(MCOLOR(255, 0, 0, 128));
	else if(pCompAttr[i]==ATTR_TARGET_NOTCONVERTED)		// º¯È¯µÇÁö ¾Ê´Â ³»¿ë
		pDC->SetColor(MCOLOR(0, 196, 0, 128));
	else												// º¯È¯ °¡´É Àý
		pDC->SetColor(MCOLOR(128, 128, 128, 128));

	pDC->FillRectangle(p.x, p.y, nWidth, nFontHeight);

	pDC->SetColor(c);

	return nWidth;
}

void Mint::DrawCompositionAttributes(MDrawContext* pDC, MPOINT& p, const char* szComposition)
{
	if(strlen(szComposition)==0) return;

	int nWidth = 0;

	for(int i=0; szComposition[i]!=NULL; i++){
		bool bTwoByteChar = IsHangul(szComposition[i]);

		nWidth += DrawCompositionAttribute(pDC, MPOINT(p.x+nWidth, p.y), szComposition, i);

		if(bTwoByteChar) i++;
	}
}

void Mint::DrawIndicator(MDrawContext* pDC, MRECT& r)
{
	MFont* pFont = pDC->GetFont();
	int nFontHeight = pFont->GetHeight();

	// ÇØ´ç ¾ð¾î¿¡ µû¶ó Ç¥±â
	const char* szLanguageIndicator = GetLanguageIndicatorString();

	int nIdicatorWidth = pFont->GetWidth(szLanguageIndicator);
	MCOLOR c = pDC->GetColor();

	MRECT fr(r.x+r.w-nIdicatorWidth-4, r.y+2, nIdicatorWidth+3, r.h-4);

	if(IsNativeIME()==true){
		pDC->SetColor(MCOLOR(130,130,130));
		pDC->FillRectangle(fr);
		pDC->SetColor(MCOLOR(0, 0, 0));
		pDC->Text(r.x+r.w-nIdicatorWidth-2, r.y + (r.h-nFontHeight)/2, szLanguageIndicator);
	}
	else{
//		pDC->SetColor(MCOLOR(0, 0, 0));
//		pDC->FillRectangle(fr);
//		pDC->SetColor(MCOLOR(DEFCOLOR_NORMAL));
	}

//	pDC->Text(r.x+r.w-nIdicatorWidth-2, r.y + (r.h-nFontHeight)/2, szLanguageIndicator);
	pDC->SetColor(c);
}



// Sample Widgets


TCHAR* szStar[] = {
	_T("°èÀýÀÌ Áö³ª°¡´Â ÇÏ´Ã¿¡´Â"),
	_T("°¡À»·Î °¡µæ Â÷ ÀÖ½À´Ï´Ù."),
	_T("³ª´Â ¾Æ¹« °ÆÁ¤µµ ¾øÀÌ"),
	_T("°¡À» ¼ÓÀÇ º°µéÀ» ´Ù ÇìÀÏ µíÇÕ´Ï´Ù. "),
	_T("°¡½¿ ¼Ó¿¡ ÇÏ³ª µÑ »õ°ÜÁö´Â º°À»  "),
	_T("ÀÌÁ¦ ´Ù ¸øÇì´Â °ÍÀº"),
	_T("½¬ÀÌ ¾ÆÄ§ÀÌ ¿À´Â ±î´ßÀÌ¿À, "),
	_T("³»ÀÏ¹ãÀÌ ³²Àº ±î´ßÀÌ¿À,"),
	_T("¾ÆÁ÷ ³ªÀÇ Ã»ÃáÀÌ ´ÙÇÏÁö ¾ÊÀº ±î´ßÀÔ´Ï´Ù. "),
	_T("º° ÇÏ³ª¿¡ Ãß¾ï°ú"),
	_T("º° ÇÏ³ª¿¡ »ç¶û°ú"),
	_T("º° ÇÏ³ª¿¡  ¾µ¾µÇÔ°ú"),
	_T("º° ÇÏ³ª¿¡ µ¿°æ°ú"),
	_T("º° ÇÏ³ª¿¡ ½Ã¿Í"),
	_T("º° ÇÏ ³ª¿¡ ¾î¸Ó´Ï, ¾î¸Ó´Ï"),
	_T("¾î¸Ó´Ï, ³ª´Â º° ÇÏ³ª¿¡ ¾Æ¸§ ´Ù¿î ¸» ÇÑ ¸¶µð¾¿ ºÒ·¯ º¾´Ï´Ù. ¼ÒÇÐ±³ ¶§ Ã¥»óÀ» "),
	_T("°°ÀÌÇß´ø ¾ÆÀÌµéÀÇ ÀÌ¸§°ú, ÆÐ, °æ, ¿Á, ÀÌ·± ÀÌ±¹ ¼Ò³àµéÀÇ ÀÌ¸§°ú,"),
	_T("¹ú½á ¾Æ±â ¾î¸Ó´Ï µÈ °èÁý¾ÖµéÀÇ ÀÌ¸§°ú, °¡³­ÇÑ ÀÌ¿ô »ç¶÷µéÀÇ ÀÌ¸§°ú,"),
	_T("ºñµÑ±â, °­¾ÆÁö, Åä³¢, ³ë»õ,  ³ë·ç, ÇÁ¶û½Ã½º Àë, ¶óÀÌ³Ê ¸¶¸®¾Æ ¸±ÄÉ,"),
	_T("ÀÌ·± ½ÃÀÎÀÇ ÀÌ¸§À» ºÒ·¯ º¾´Ï´Ù."),
	_T("ÀÌ³×µéÀº ³Ê¹«³ª ¸Ö¸® ÀÖ½À´Ï´Ù."),
	_T("º°ÀÌ ¾Æ½º¶óÀÌ ¸Ö µíÀÌ,"),
	_T("¾î¸Ó´Ô,"),
	_T("±×¸®°í ´ç½ÅÀº ¸Ö¸® ºÏ°£µµ¿¡ °è½Ê´Ï´Ù."),
	_T("³ª´Â ¹«¾ùÀÎÁö ±×¸®¿ö"),
	_T("ÀÌ ¸¹Àº º°ºûÀÌ ³»¸° ¾ð´ö À§¿¡"),
	_T("³» ÀÌ¸§ÀÚ¸¦ ½á º¸°í,"),
	_T("ÈëÀ¸·Î µ¤¾î ¹ö¸®¾ú½À´Ï´Ù."),
	_T("µýÀº, ¹ãÀ» »õ¿ö ¿ì´Â ¹ú·¹´Â"),
	_T("ºÎ²ô·¯¿î ÀÌ¸§À» ½½ÆÛÇÏ´Â ±î´ßÀÔ´Ï´Ù."),
	_T("±×·¯³ª °Ü¿ïÀÌ Áö³ª°í ³ªÀÇ º°¿¡µµ º½ÀÌ ¿À¸é,"),
	_T("¹«¾ö À§¿¡ ÆÄ¶õ ÀÜµð°¡ ÇÇ¾î³ªµíÀÌ"),
	_T("³» ÀÌ¸§ÀÚ ¹¯Èù ¾ð´ö À§¿¡µµ,"),
	_T("ÀÚ¶ûÃ³·³ Ç®ÀÌ ¹«¼ºÇÒ °Å¿Ü´Ù. "),
};


void MCreateSample(void)
{
	MWidget* pMainFrame = Mint::GetInstance()->GetMainFrame();
	MWidget* pNew = new MFileDialog("*.*", pMainFrame, pMainFrame);
	//pNew->SetBounds(10, 10, 300, 300);
	pNew->Show(true);

	class MFrameBitmap : public MFrame{
		int			m_nBitmap;
	protected:
		virtual void OnDraw(MDrawContext* pDC){
			MFrame::OnDraw(pDC);

			MRECT r = GetClientRect();

			if(MBitmapManager::GetCount()==0) return;
			m_nBitmap %= MBitmapManager::GetCount();
			pDC->SetBitmap(MBitmapManager::Get(m_nBitmap));
			m_nBitmap++;
			pDC->Draw(r.x, r.y);
			//pDC->Draw(r);
		}
	public:
		MFrameBitmap(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL)
			: MFrame(szName, pParent, pListener){
			m_nBitmap = 0;
		}
		virtual ~MFrameBitmap(void){
		}
	};

	MWidget* pNewFrame = new MFrameBitmap("Bitmap Test", pMainFrame, pMainFrame);
	pNewFrame->SetBounds(0, 20, 200, 200);
	pNewFrame->Show(true);

	class MFrameInfo : public MFrame{
		int m_nRenderCount;
		DWORD m_nPrevTime;
		int	m_nFPS;
	protected:
		virtual void OnDraw(MDrawContext* pDC){
			MFrame::OnDraw(pDC);

			MRECT r = GetClientRect();
			char temp[256];

			pDC->SetColor(MCOLOR(255,255,255));

			// FPS
			DWORD nCurrTime = timeGetTime();
			if(nCurrTime-m_nPrevTime>1000){
				m_nFPS = m_nRenderCount;
				m_nPrevTime = nCurrTime;
				m_nRenderCount = 0;
			}
			m_nRenderCount++;
			sprintf(temp, "FPS = %d", m_nFPS);
			pDC->Text(r.x, r.y, temp);

			// Cursor Pos
			/*
			POINT p;
			GetCursorPos(&p);
			ScreenToClient(m_hWnd, &p);
			*/
			MPOINT p = MEvent::GetMousePos();
			sprintf(temp, "Cursor Pos = %d, %d", p.x, p.y);
			pDC->Text(r.x, r.y+GetFont()->GetHeight(), temp);

		}
	public:
		MFrameInfo(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL)
			: MFrame(szName, pParent, pListener){
			m_nRenderCount = 0;
			m_nPrevTime = timeGetTime();
			m_nFPS = 0;
		}
		virtual ~MFrameInfo(void){
		}
	};

	MWidget* pNewCursorInfo = new MFrameInfo("Á¤º¸", pMainFrame, pMainFrame);
	pNewCursorInfo->SetBounds(400, 0, 200, 200);
	pNewCursorInfo->Show(true);

	class MFrameStar : public MFrame{
	protected:
		virtual void OnDraw(MDrawContext* pDC){
			MFrame::OnDraw(pDC);

			MRECT r = GetClientRect();
			int nRawSize = sizeof(szStar)/sizeof(char*);
			static int nRaw = 0;
			pDC->SetColor(MCOLOR(128,128,255));
			for(int i=0; i<20; i++){
				pDC->Text(r.x, r.y+i*(GetFont()->GetHeight()+2), szStar[(nRaw+i)%nRawSize]);
			}
			nRaw++;
			nRaw%=nRawSize;
		}
		virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
			/*
			if(pWidget->GetID()==MGetResourceID("ID_OK") && IsMsg(szMessage, MBTN_CLK_MSG)==true){
				Hide();
			}
			*/
			return false;
		}
	public:
		MFrameStar(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL)
			: MFrame(szName, pParent, pListener){
		}
		virtual ~MFrameStar(void){
		}
	};

	MWidget* pNewStar = new MFrameStar("º° Çì´Â ¹ã", pMainFrame, pMainFrame);
	pNewStar->SetBounds(30, 250, 500, 200);
	pNewStar->Show(true);

	MPopupMenu* pNewMenu = new MPopupMenu("SampleMenu", pMainFrame, pMainFrame, MPMT_HORIZONTAL);
	MMenuItem* pFile = pNewMenu->AddMenuItem("File");
	MMenuItem* pWidget = pNewMenu->AddMenuItem("Widget");
	MPopupMenu* pFileMenu = pFile->CreateSubMenu();
	MPopupMenu* pWidgetMenu = pWidget->CreateSubMenu();
	pFileMenu->AddMenuItem("Open");
	pFileMenu->AddMenuItem("Exit");
	pWidgetMenu->AddMenuItem("Info");
	pWidgetMenu->AddMenuItem("º°Çì´Â¹ã");

	pNewMenu->SetBounds(0, 0, MGetWorkspaceWidth(), 16);
	pNewMenu->Show(0, 0, true);
	pNewMenu->Show(true);
}

void MDestroySample(void)
{
}

/*
TODO: Edit ¼¿·º¼Ç Áö¿ø
TODO: Text Area IME Áö¿ø
*/