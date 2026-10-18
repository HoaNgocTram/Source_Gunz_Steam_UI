#include "stdafx.h"
#include "ZCombatChat.h"
#include "ZGameInterface.h"
#include "ZInterfaceItem.h"
#include "ZApplication.h"
#include "ZInterfaceListener.h"
//#include "MListBox.h"
#include "ZIDLResource.h"
#include "ZPost.h"
#include "MChattingFilter.h"
#include "MTextArea.h"
#include "ZConfiguration.h"
#include "EmojiManager.h"
#include "ZEmojiPicker.h"
#include "RGMain.h"
#include "RealSpace2.h"
#include "ZInput.h"
#define MAX_CHAT_OUTPUT_LINE 7
/////////////////
// ZTabPlayerList

ZTabPlayerList::ZTabPlayerList(const char* szName, MWidget* pParent, MListener* pListener)
	: MListBox(szName, pParent, pListener)
{
	SetChatControl(NULL);
}

bool ZTabPlayerList::OnShow(void)
{
	RemoveAll();

	// Push Target List
	for (ZCharacterManager::iterator i = ZGetCharacterManager()->begin(); i != ZGetCharacterManager()->end(); i++)
	{
		ZCharacter* pChar = (ZCharacter*)i->second;
		if (pChar->IsAdminHide()) continue;
		Add(pChar->GetProperty()->GetName());
	}

	return true;
}

void ZTabPlayerList::OnHide(void)
{
	if (m_pEditChat)
		m_pEditChat->SetFocus();
}

bool ZTabPlayerList::OnEvent(MEvent* pEvent, MListener* pListener)
{
	if (pEvent->nMessage == MWM_KEYDOWN) {
		if (pEvent->nKey == VK_ESCAPE)
		{
			Show(false);
			return true;
		}
		else if ((pEvent->nKey == VK_TAB) || (pEvent->nKey == VK_RETURN) || (pEvent->nKey == VK_SPACE))
		{
			OnPickPlayer();
			Show(false);
			return true;
		}
	}
	return MListBox::OnEvent(pEvent, pListener);
}

//void ZTabPlayerList::OnPickPlayer()
//{
//	const char* pszString = GetSelItemString();
//	if (m_pEditChat && pszString) {
//		m_pEditChat->AddText(pszString);
//	}
//}
void ZTabPlayerList::OnPickPlayer()
{
	const char* pszString = GetSelItemString();
	if (!pszString) return;

	// 1. Kiểm tra xem hệ thống NewChat có đang được bật trong Config không
	if (ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		// Gửi vào NewChat
		GetRGMain().GetChat().InsertTextAtCaret(pszString);
	}
	else if (m_pEditChat)
	{
		// Gửi vào OldChat (MINT)
		m_pEditChat->AddText(pszString);
	}

	// 2. Tắt bảng chọn
	Show(false);

	// 3. QUAN TRỌNG: Trả lại focus cho khung chat tương ứng
	// Nếu không set focus lại, hệ thống sẽ bị "treo" phím Enter vì nó không biết 
	// sự kiện phím phải đổ về đâu.
	if (ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
	
	}
	else if (m_pEditChat)
	{
		m_pEditChat->SetFocus();
	}
}
/////////////////

class MCombatChatInputListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) {
		if (MWidget::IsMsg(szMessage, MEDIT_ENTER_VALUE) == true)
		{
			if (strlen(pWidget->GetText()) >= 256) return false;

			const char* szCommand = pWidget->GetText();
			if (szCommand[0] != '\0')
			{
				char szMsg[512];
				strcpy(szMsg, szCommand);
				strcpy(szMsg, pWidget->GetText());

				if (!ZGetConfiguration()->GetViewGameChat())
				{
					if (ZGetCombatInterface())
					{
						ZGetCombatInterface()->ShowChatOutput(true);
					}
				}
				ZApplication::GetGameInterface()->GetChat()->Input(szMsg);
			}

			pWidget->SetText("");
			if (ZGetCombatInterface())
			{
				ZGetCombatInterface()->EnableInputChat(false);
			}
			return true;
		}
		else if (MWidget::IsMsg(szMessage, MEDIT_ESC_VALUE) == true)
		{
			pWidget->SetText("");
			ZGetCombatInterface()->EnableInputChat(false);
		}
		else if ((MWidget::IsMsg(szMessage, MEDIT_CHAR_MSG) == true) || (MWidget::IsMsg(szMessage, MEDIT_KEYDOWN_MSG) == true))
		{
			ZApplication::GetGameInterface()->GetChat()->FilterWhisperKey(pWidget);
		}
		else if (MWidget::IsMsg(szMessage, MEDIT_EMOJI_PICKER) == true)
		{
			ZGetEmojiPicker().SetAnchor(pWidget->GetScreenRect().x, pWidget->GetScreenRect().y, pWidget->GetScreenRect().w);
			ZGetEmojiPicker().SetInsertCallback([](const char* p) {
				MEdit* pEdit = (MEdit*)ZGetGameInterface()->GetIDLResource()->FindWidget(ZIITEM_COMBAT_CHATINPUT);
				if (pEdit) {
					pEdit->AddText(p);
					pEdit->SetFocus();
				}
				});
			ZGetEmojiPicker().Toggle();
			return true;
		}


		return false;
	}
};
MCombatChatInputListener	g_CombatChatInputListener;

MListener* ZGetCombatChatInputListener(void)
{
	return &g_CombatChatInputListener;
}


ZCombatChat::ZCombatChat()
{
	m_bChatInputVisible = true;
	m_nLastChattingMsgTime = 0;
	m_pIDLResource = ZApplication::GetGameInterface()->GetIDLResource();

	m_pChattingOutput = NULL;
	m_pInputEdit = NULL;
	m_pTabPlayerList = NULL;
	m_bTeamChat = false;
	m_bShowOutput = true;
}

ZCombatChat::~ZCombatChat()
{

}

bool ZCombatChat::Create(const char* szOutputTxtarea, bool bUsePlayerList)
{
	MWidget* pWidget = m_pIDLResource->FindWidget(ZIITEM_COMBAT_CHATINPUT);
	if (pWidget != NULL)
	{
		pWidget->SetListener(ZGetCombatChatInputListener());
	}
	m_pChattingOutput = NULL;
	//	pWidget = m_pIDLResource->FindWidget(ZIITEM_COMBAT_CHATOUTPUT);
	pWidget = m_pIDLResource->FindWidget(szOutputTxtarea);
	if (pWidget != NULL)
	{
		m_pChattingOutput = ((MTextArea*)pWidget);
	}

	if (m_pChattingOutput != NULL)
	{
		m_pChattingOutput->Clear();

		// Set line height to max(fontHeight, emojiSize) so layout accounts for emojis
		if (Z_ETC_EMOTE && ZGetEmojiManager().IsLoaded())
		{
			MFont* pFont = m_pChattingOutput->GetFont();
			int nFontH = pFont ? pFont->GetHeight() : 12;
			int nEmojiH = ZGetEmojiManager().GetMaxSize(false); // max lobby size across all sets
			if (nEmojiH > nFontH)
				m_pChattingOutput->SetCustomLineHeight(nEmojiH);
		}
	}

	pWidget = m_pIDLResource->FindWidget(ZIITEM_COMBAT_CHATINPUT);
	if (pWidget != NULL)
	{
		m_pInputEdit = (MEdit*)pWidget;
		m_pInputEdit->Show(false);

		if (bUsePlayerList)
		{
			// TabPlayerList
			MWidget* pPivot = m_pInputEdit->GetParent();
			m_pTabPlayerList = new ZTabPlayerList("TabPlayerList", pPivot, ZGetCombatInterface());
			m_pTabPlayerList->Show(false);
			m_pTabPlayerList->SetBounds(m_pInputEdit->GetPosition().x, m_pInputEdit->GetPosition().y - 120 - 5, 150, 120);
			m_pTabPlayerList->SetChatControl(m_pInputEdit);
			m_pInputEdit->SetTabHandler(m_pTabPlayerList);
		}
	}

	MLabel* pLabelToTeam = (MLabel*)m_pIDLResource->FindWidget(ZIITEM_COMBAT_CHATMODE_TOTEAM);
	if (pLabelToTeam) pLabelToTeam->Show(false);
	MLabel* pLabelToAll = (MLabel*)m_pIDLResource->FindWidget(ZIITEM_COMBAT_CHATMODE_TOALL);
	if (pLabelToAll) pLabelToAll->Show(false);

	return true;
}

void ZCombatChat::Destroy()
{
	if (m_pTabPlayerList)
	{
		delete m_pTabPlayerList;
		m_pTabPlayerList = NULL;
	}
	if (m_pInputEdit)
	{
		m_pInputEdit->SetListener(NULL);
	}
	m_pChattingOutput = NULL;
}

void ZCombatChat::Update()
{
	UpdateChattingBox();
	if ((m_pInputEdit) && (m_bChatInputVisible))
	{
		if (!m_pInputEdit->IsFocus())
		{
			if (m_pTabPlayerList && !m_pTabPlayerList->IsFocus())
				EnableInput(false);
		}
	}
}

void ZCombatChat::OnDraw(MDrawContext* pDC)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		if (m_pInputEdit)
		{
			if (m_pInputEdit->IsVisible())
			{
				pDC->SetColor(0x00, 0x00, 0x00, 140);
				pDC->FillRectangle(m_pInputEdit->GetScreenRect());
			}
		}
	}
	else
	{
		if (m_pInputEdit)
		{
			if (m_pInputEdit->IsVisible())
			{
				pDC->SetColor(0xFF, 0xFF, 0xFF, 50);
				pDC->FillRectangle(m_pInputEdit->GetScreenRect());
			}
		}
	}
}

void ZCombatChat::UpdateChattingBox()
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
#define CHAT_DELAY_TIME	5000
		if (m_pChattingOutput == NULL) return;
		m_pChattingOutput->Clear();
	}
	else
	{
		if (m_pChattingOutput == NULL) return;

		if (m_pChattingOutput->GetLineCount() > 0)
		{
			unsigned long int nNowTime = timeGetTime();

#define CHAT_DELAY_TIME	5000
			if ((nNowTime - m_nLastChattingMsgTime) > CHAT_DELAY_TIME)
			{
				m_pChattingOutput->DeleteFirstLine();
				m_nLastChattingMsgTime = nNowTime;
			}
		}
	}
}


void ZCombatChat::EnableInput(bool bEnable, bool bToTeam)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		bEnable = false;
		bToTeam = false;
	}
	else
	{
		MLabel* pLabelToTeam = (MLabel*)m_pIDLResource->FindWidget(ZIITEM_COMBAT_CHATMODE_TOTEAM);
		MLabel* pLabelToAll = (MLabel*)m_pIDLResource->FindWidget(ZIITEM_COMBAT_CHATMODE_TOALL);
		if (bEnable == true) {
			SetTeamChat(bToTeam);
			if (bToTeam) {
				if (pLabelToTeam != NULL) pLabelToTeam->Show(true);
				if (pLabelToAll != NULL) pLabelToAll->Show(false);
			}
			else {
				if (pLabelToTeam != NULL) pLabelToTeam->Show(false);
				if (pLabelToAll != NULL) pLabelToAll->Show(true);
			}
		}
		else {
			if (pLabelToTeam != NULL) pLabelToTeam->Show(false);
			if (pLabelToAll != NULL) pLabelToAll->Show(false);
		}

		if (m_pInputEdit)
		{
			m_pInputEdit->Show(bEnable);
		}

		if (bEnable)
		{
			if (m_pInputEdit)
			{
				if (!m_pInputEdit->IsFocus()) m_pInputEdit->SetFocus();
			}
		}
		else
		{
			//		ZApplication::GetGameInterface()->SetFocus();
		}

		m_bChatInputVisible = bEnable;

		if (ZGetGame()->m_pMyCharacter == NULL) return;

		if (bEnable)
		{
			ZPostPeerChatIcon(true);
		}
		else
		{
			ZPostPeerChatIcon(false);
		}

		// Cursor handling for old chat
		/*if (Z_CHAT_ENABLE_CURSOR)
		{
			ZGetGameInterface()->SetCursorEnable(bEnable);
			if (bEnable)
			{
				POINT pt = { RGetScreenWidth() / 2, RGetScreenHeight() / 2 };
				ClientToScreen(g_hWnd, &pt);
				SetCursorPos(pt.x, pt.y);
			}
			else
			{
				POINT pt = { RGetScreenWidth() / 2, RGetScreenHeight() / 2 };
				ClientToScreen(g_hWnd, &pt);
				SetCursorPos(pt.x, pt.y);

				ZGetInput()->ResetRotation();
				ZGetInput()->OffActionKeys();
			}
		}*/
		// Cursor handling for old chat
		POINT pt = { RGetScreenWidth() / 2, RGetScreenHeight() / 2 };
		ClientToScreen(g_hWnd, &pt);
		SetCursorPos(pt.x, pt.y);

		ZGetInput()->OffActionKeys();
		ZGetGameInterface()->SetCursorEnable(bEnable);
	}
}

void ZCombatChat::OutputChatMsg(const char* szMsg)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		char szText[512];
		auto&& Cfg = *ZGetConfiguration();

		if (Cfg.GetChat()->TimeStamp)
		{
			SYSTEMTIME t;
			GetLocalTime(&t);

			char szTime[64];
			sprintf(szTime, "%02d:%02d", t.wHour, t.wMinute);
			wsprintf(szText, "[%s] %s", szTime, szMsg);
		}
		else
		{
			wsprintf(szText, "%s", szMsg);
		}


		GetRGMain().GetChat().OutputChatMsg(szText);
	}
	else
	{
		if (m_pChattingOutput == NULL) return;

		if (m_pChattingOutput->GetLineCount() == 0)
			for (int i = 0; i < (MAX_CHAT_OUTPUT_LINE - 1); i++) m_pChattingOutput->AddText("");
		m_pChattingOutput->AddText(szMsg);

		ProcessChatMsg();
		return;
	}
}

void ZCombatChat::OutputChatMsg(MCOLOR color, const char* szMsg)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		char szText[512];
		auto&& Cfg = *ZGetConfiguration();

		if (Cfg.GetChat()->TimeStamp)
		{
			SYSTEMTIME t;
			GetLocalTime(&t);

			char szTime[64];
			sprintf(szTime, "%02d:%02d", t.wHour, t.wMinute);
			wsprintf(szText, "[%s] %s", szTime, szMsg);
		}
		else
		{
			wsprintf(szText, "%s", szMsg);
		}

		GetRGMain().GetChat().OutputChatMsg(szText, color.GetARGB());
	}
	else
	{
		if (m_pChattingOutput == NULL) return;

		if (m_pChattingOutput->GetLineCount() == 0)
			for (int i = 0; i < (MAX_CHAT_OUTPUT_LINE - 1); i++) m_pChattingOutput->AddText("");
		m_pChattingOutput->AddText(szMsg, color);

		ProcessChatMsg();
		return;
	}
}

void ZCombatChat::ProcessChatMsg()
{
	while ((m_pChattingOutput->GetLineCount() > MAX_CHAT_OUTPUT_LINE))
	{
		m_pChattingOutput->DeleteFirstLine();
	}


	if (m_pChattingOutput->GetLineCount() >= 1)
	{
		m_nLastChattingMsgTime = timeGetTime();
	}
}

bool ZCombatChat::IsChat() const
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		return GetRGMain().GetChat().IsInputEnabled();
	}
	else
	{
		return m_bChatInputVisible;
	}
}

bool ZCombatChat::IsTeamChat() const
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		return GetRGMain().GetChat().IsTeamChat();
	}
	else
	{
		return m_bTeamChat;
	}
}
bool ZCombatChat::IsShow() const
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		return GetRGMain().GetChat().IsInputEnabled();
	}
	else
	{
		if (m_pInputEdit == NULL) return false;
		return m_pInputEdit->IsVisible();
	}
}

void ZCombatChat::SetFont(MFont* pFont)
{
	m_pChattingOutput->SetFont(pFont);
}


void ZCombatChat::ShowOutput(bool bShow)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNewChatEnable)
	{
		GetRGMain().GetChat().HideAlways = !bShow;
	}
	else
	{
		if (m_pChattingOutput) m_pChattingOutput->Show(bShow);
		m_bShowOutput = bShow;
		return;
	}
}


#ifdef _KILLFEED
const char* GetWeaponIconPath(MMatchWeaponType WeaponType)
{
	switch (WeaponType) {
	case MWT_SPYCASE:			return "dagger.png";
	case MWT_DAGGER:			return "dagger.png";
	case MWT_DUAL_DAGGER:		return "dualdagger.png";
	case MWT_KATANA:			return "sword.png";
	case MWT_GREAT_SWORD:		return "sword.png";

	case MWT_PISTOL:			return "singlepistol.png";
	case MWT_PISTOLx2:			return "dualpistol.png";
	case MWT_REVOLVER:			return "singlepistol.png";
	case MWT_REVOLVERx2:		return "dualpistol.png";

	case MWT_SMG:				return "smg.png";
	case MWT_SMGx2:				return "smgx2.png";

	case MWT_SHOTGUN:			return "shotgun.png";
	case MWT_RIFLE:				return "rifle.png";
	case MWT_MACHINEGUN:		return "lmg.png";
	case MWT_ROCKET:			return "rocket.png";
	case MWT_SNIFER:			return "sniper.png";
	case MWT_ROCKETGUIDED:		return "rocket.png";
	
	case MWT_FRAGMENTATION:		return "grenade.png";
	case MWT_DYNAMITYE:			return "grenade.png";
	case MWT_LANDMINE:			return "grenade.png";
	case MWT_LANDMINE_SPY:		return "grenade.png";
	case MWT_TRAP:				return "grenade.png";
	case MWT_TRAP_SPY:			return "grenade.png";
	case MWT_STUNGRENADE:		return "grenade.png";
	
	case MWT_SKILL:				return "suicidio.png";
	case MWT_ENCHANT_FIRE:		return "suicidio.png";
	case MWT_ENCHANT_COLD:		return "suicidio.png";
	case MWT_ENCHANT_LIGHTNING:	return "suicidio.png";
	case MWT_ENCHANT_POISON:	return "suicidio.png";
	case MWT_POTION:			return "suicidio.png";
	case MWT_NONE:				return "suicidio.png";    // Tự sát
		// ... Thêm các loại súng khác vào đây
	default:					return "blank.png";
	}
}

ZNotifyKills::ZNotifyKills() {
	m_pFont = NULL;
	m_vKillFeeds.clear();
}

ZNotifyKills::~ZNotifyKills() {
}

bool ZNotifyKills::Create() {
	// Lấy sẵn font icon vũ khí để vẽ
	m_pFont = MFontManager::Get("FONTa9b");
	return true;
}

void ZNotifyKills::Destroy() {
	m_vKillFeeds.clear();
}

void ZNotifyKills::Update() {
	if (m_vKillFeeds.empty()) return;

	unsigned long nNow = timeGetTime();
	// Tự động xóa dòng sau 5000ms (5 giây)
	for (auto it = m_vKillFeeds.begin(); it != m_vKillFeeds.end(); ) {
		if (nNow - it->nTime > 5000) {
			it = m_vKillFeeds.erase(it);
		}
		else {
			++it;
		}
	}
}

void ZNotifyKills::AddKillFeed(const char* szAtt, const char* szVic, int nAttTeam, int nVicTeam, MMatchWeaponType nWeapon)
{
	// 1. Kiểm tra đầu vào (Tránh trường hợp tên bị NULL gây crash game)
	if (!szAtt || !szAtt[0]) return;

	ZKillFeedItem item;
	item.strAttacker = szAtt;
	item.strVictim = (szVic && szVic[0]) ? szVic : ""; // Nếu không có nạn nhân (tự sát) thì để trống
	item.nAttTeam = nAttTeam;
	item.nVicTeam = nVicTeam;
	item.nTime = timeGetTime();

	// 2. Lấy đường dẫn icon từ hàm helper đã viết
	item.strIconPath = GetWeaponIconPath(nWeapon);

	m_vKillFeeds.push_back(item);

	// Giới hạn số lượng dòng hiển thị trên màn hình (thường là 5-6 dòng)
	if (m_vKillFeeds.size() > 6) {
		m_vKillFeeds.erase(m_vKillFeeds.begin());
	}
}

void ZNotifyKills::OnDraw(MDrawContext* pDC)
{
	if (m_vKillFeeds.empty()) return;

	float nScreenWidth = (float)RGetScreenWidth();
	float fScale = nScreenWidth / 800.0f;

	int nY = (int)(25.0f * fScale);
	int nMarginRight = (int)(10.0f * fScale);
	int nLineHeight = (int)(25.0f * fScale);

	int nIconWidth = (int)(35.0f * fScale);
	int nIconHeight = (int)(12.0f * fScale);
	int nSpacing = (int)(5.0f * fScale);

	for (const auto& item : m_vKillFeeds)
	{
		MFont* pFont = MFontManager::Get("FONTa9b");
		if (!pFont) pFont = pDC->GetFont();
		pDC->SetFont(pFont);

		// --- PHẪU THUẬT BƯỚC 3: TÍNH TOÁN ĐỘ RỘNG LINH HOẠT ---
		int nAttWidth = pFont->GetWidth(item.strAttacker.c_str());

		// Kiểm tra xem có nạn nhân hay không (để tránh lệch lề khi tự tử)
		bool bHasVictim = (item.strVictim.length() > 0);
		int nVicWidth = bHasVictim ? pFont->GetWidth(item.strVictim.c_str()) : 0;

		// Tổng độ rộng = Tên sát thủ + Icon + (Khoảng cách & Tên nạn nhân nếu có)
		int nTotalWidth = nAttWidth + nSpacing + nIconWidth;
		if (bHasVictim) {
			nTotalWidth += nSpacing + nVicWidth;
		}

		int nDrawX = (int)nScreenWidth - nTotalWidth - nMarginRight;

		// --- PHẪU THUẬT BƯỚC 4 & 6: XỬ LÝ MÀU ---
		MCOLOR colAtt, colVic;
		if (ZGetGame()->GetMatch()->IsSoloPlay()) {
			colAtt = colVic = MCOLOR(252, 238, 187); // Màu vàng cát
		}
		else {
			colAtt = (item.nAttTeam == MMT_RED) ? MCOLOR(255, 50, 50) : MCOLOR(50, 50, 255);
			colVic = (item.nVicTeam == MMT_RED) ? MCOLOR(255, 50, 50) : MCOLOR(50, 50, 255);
		}

		// Vẽ Tên Sát Thủ
		if (bHasVictim) {
			// 1. TRƯỜNG HỢP CÓ NẠN NHÂN: Giữ nguyên thứ tự [A] [Súng] [B]
			// Vẽ Tên Sát Thủ
			pDC->SetColor(0, 0, 0, 200);
			pDC->Text(nDrawX + 1, nY + 1, item.strAttacker.c_str());
			pDC->SetColor(colAtt);
			pDC->Text(nDrawX, nY, item.strAttacker.c_str());
			nDrawX += nAttWidth + nSpacing;

			// Vẽ Icon Súng
			MBitmap* pIcon = MBitmapManager::Get(item.strIconPath.c_str());
			if (pIcon) {
				pDC->SetBitmap(pIcon);
				pDC->Draw(nDrawX, nY + (nIconHeight / 4) - (int)(2.0f * fScale), nIconWidth, nIconHeight);
			}
			nDrawX += nIconWidth + nSpacing;

			// Vẽ Tên Nạn Nhân
			pDC->SetColor(0, 0, 0, 200);
			pDC->Text(nDrawX + 1, nY + 1, item.strVictim.c_str());
			pDC->SetColor(colVic);
			pDC->Text(nDrawX, nY, item.strVictim.c_str());
		}
		else {
			// 2. TRƯỜNG HỢP TỰ TỬ: Đảo lại thành [Icon Đầu Lâu] [Tên]
			// Vẽ Icon Đầu Lâu trước
			MBitmap* pIcon = MBitmapManager::Get(item.strIconPath.c_str());
			if (pIcon) {
				pDC->SetBitmap(pIcon);
				pDC->Draw(nDrawX, nY + (nIconHeight / 4) - (int)(2.0f * fScale), nIconWidth, nIconHeight);
			}
			nDrawX += nIconWidth + nSpacing;

			// Vẽ Tên người tự sát sau
			pDC->SetColor(0, 0, 0, 200);
			pDC->Text(nDrawX + 1, nY + 1, item.strAttacker.c_str());
			pDC->SetColor(colAtt);
			pDC->Text(nDrawX, nY, item.strAttacker.c_str());
		}

		nY += nLineHeight;
	}
}

void ZNotifyKills::SetFont(MFont* pFont) {
	m_pFont = pFont;
}
#endif