#include "stdafx.h"

#include "ZItemSlotView.h"
#include "ZItemIconBitmap.h"
#include "ZIDLResource.h"
#include "ZApplication.h"
#include "ZMyInfo.h"
#include "ZMyItemList.h"
#include "ZPost.h"
#include "MComboBox.h"
#include "ZShopEquipInterface.h"
#include "ZShopEquipItem.h"
#include "ZShopEquipListbox.h"
extern MMatchItemDesc* g_pCurrentHoverDesc;
extern MTextArea* g_pTargetTooltipWidget;
extern MPOINT GetSmartTooltipPosition(int nMouseX, int nMouseY, int nTooltipW, int nTooltipH);

ZItemSlotView::ZItemSlotView(const char* szName, MWidget* pParent, MListener* pListener)
: MButton(szName, pParent, pListener)
{
	m_nItemID = -1;
	m_nItemCount = 0;

	m_nParts = MMCIP_END;
	m_pBackBitmap = NULL;
	m_bSelectBox = false;
	m_bKindable = false;
	m_bDragAndDrop = false;
	m_bHorizonalInverse = false;
	m_szItemSlotPlace[0] = '\0';
}

ZItemSlotView::~ZItemSlotView(void)
{
}


void ZItemSlotView::SetDefaultText(MMatchCharItemParts nParts)
{
	switch (nParts)
	{
	case MMCIP_HEAD:			SetText("");			 break;
	case MMCIP_CHEST:			SetText("");			 break;
	case MMCIP_HANDS:			SetText("");			 break;
	case MMCIP_LEGS:			SetText("");			 break;
	case MMCIP_FEET:			SetText("");			 break;
	case MMCIP_FINGERL:			SetText("< Left Finger >");		 break;
	case MMCIP_FINGERR:			SetText("< Right Finger >");	 break;	
#ifdef _AVATAR_ENABLE
	case MMCIP_AVATAR:			SetText("< Avatar >");			 break;
#endif
	case MMCIP_MELEE:			SetText("< Melee >");			 break;
	case MMCIP_PRIMARY:			SetText("< Primary Weapon >");	 break;
	case MMCIP_SECONDARY:		SetText("< Secondary Weapon >"); break;
	case MMCIP_CUSTOM1:			SetText("< Item 1 >");			 break;
	case MMCIP_CUSTOM2:			SetText("< Item 2 >");			 break;
	//case MMCIP_CUSTOM3:			SetText("< Item 3 >");			 break;

	case MMCIP_DASH:			SetText("Dash");				 break;
	case MMCIP_JJANG:			SetText("Jjang");				 break;
	case MMCIP_STUN:			SetText("Stun");				 break;
	//case MMCIP_EMOTE:			SetText("Emote");				 break;

	/*
	case MMCIP_COMMUNITY1:		SetText("< Community 1 >");		 break;
	case MMCIP_COMMUNITY2:		SetText("< Community 2 >");		 break;
	case MMCIP_LONGBUFF1:		SetText("< LongBuff 1 >");		 break;
	case MMCIP_LONGBUFF2:		SetText("< LongBuff 2 >");		 break;*/

	default : SetText(""); break;
	}
}

void ZItemSlotView::OnDraw(MDrawContext* pDC)
{
	if (m_nParts == MMCIP_END) return;

	unsigned int nItemID;
	if (m_nItemID != -1) {
		nItemID = m_nItemID;
	}
	else {
		nItemID = ZGetMyInfo()->GetItemList()->GetEquipedItemID(m_nParts);
	}

	MMatchItemDesc* pItemDesc = NULL;
	if (nItemID != 0)
		pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

	MRECT r = GetRect();
	// Lấy trọn vẹn kích thước Bounds từ XML (không ép vuông hay cắt bớt)
	MRECT rcSquare(0, 0, r.w, r.h);

	// 1. VẼ HÌNH NỀN KHUNG (FULL Ô BACKGROUND)
	MRECT rcFull = GetClientRect(); // Tọa độ nội bộ full kích thước (W=100, H=67)
	MBitmap* pSlotBG = NULL;

	if (pItemDesc)
	{
		switch (pItemDesc->itemClass())
		{
		case ITEM_CASH:   pSlotBG = MBitmapManager::Get("slot_medal.png"); break;
		case ITEM_MEDAL:  pSlotBG = MBitmapManager::Get("slot_cash.png"); break;
		case ITEM_STAFF:  pSlotBG = MBitmapManager::Get("slot_staff.png"); break;
		case ITEM_EVENT:  pSlotBG = MBitmapManager::Get("slot_event.png"); break;
		case ITEM_BOUNTY:
		default:          pSlotBG = MBitmapManager::Get("slot_bounty.png"); break;
		}
	}

	if (!pSlotBG) pSlotBG = MBitmapManager::Get("slot_bounty.png");

	if (pSlotBG)
	{
		pDC->SetBitmap(pSlotBG);
		pDC->Draw(rcFull); // Phủ kín toàn bộ ô 100x67
	}
	else
	{
		pDC->SetColor(MCOLOR(20, 24, 30));
		pDC->FillRectangle(rcFull);
	}


	// 2. VẼ ICON VẬT PHẨM NẰM TRÊN NỀN
	if ((nItemID == 0) || (pItemDesc == NULL))
	{
		if (m_pBackBitmap != NULL)
		{
			pDC->SetBitmap(m_pBackBitmap);
			pDC->Draw(rcFull);
		}
	}
	else
	{
		int nPadding = 6;
		int nIconSize = min(rcFull.w - (nPadding * 2), rcFull.h - (nPadding * 2));

		MRECT rcIcon;
		rcIcon.x = (rcFull.w - nIconSize) / 2;
		rcIcon.y = (rcFull.h - nIconSize) / 2;
		rcIcon.w = nIconSize;
		rcIcon.h = nIconSize;

		bool bDrawn = false;

		// 1. Kiểm tra Atlas Icon
		ZItemIconInfo iconInfo;
		if (ZGetGameInterface()->GetItemIconMgr()->GetIconInfo(pItemDesc->m_nID, iconInfo))
		{
			MBitmap* pAtlas = ZGetGameInterface()->GetItemIconMgr()->GetAtlasBitmap(iconInfo.strSource.c_str());
			if (pAtlas)
			{
				// ÉP KIỂU SANG MDrawContextR2 ĐỂ GỌI HÀM DRAW UV CỦA ENGINE R2
				MDrawContextR2* pDCR2 = (MDrawContextR2*)pDC;

				pDCR2->Draw(
					pAtlas,                                                 // Pointer ảnh Atlas
					rcIcon.x, rcIcon.y, rcIcon.w, rcIcon.h,                 // Vị trí & Kích thước vẽ trên UI
					iconInfo.nX, iconInfo.nY, iconInfo.nWidth, iconInfo.nHeight // Tọa độ cắt UV trên Atlas
				);
				bDrawn = true;
			}
		}

		// 2. Fallback nếu không có trong Atlas XML
		if (!bDrawn)
		{
			MBitmap* pIconBitmap = GetItemIconBitmap(pItemDesc);
			if (pIconBitmap)
			{
				pDC->SetBitmap(pIconBitmap);
				pDC->Draw(rcIcon);
			}
		}
	}

	// 3. HIỆU ỨNG VIỀN SELECT HOẶC RÊ CHUỘT (HOVER)
	if (m_bSelectBox)
	{
		pDC->SetColor(MCOLOR(255, 255, 255, 30));
		pDC->FillRectangle(rcSquare);

		pDC->SetColor(MCOLOR(0, 229, 255)); // Viền Cyan nổi bật
		pDC->Rectangle(rcSquare);

		MRECT rcInnerBorder(rcSquare.x + 1, rcSquare.y + 1, rcSquare.w - 2, rcSquare.h - 2);
		pDC->Rectangle(rcInnerBorder);
	}
	else if (IsMouseOver())
	{
		pDC->SetColor(MCOLOR(255, 255, 255, 25));
		pDC->FillRectangle(rcSquare);
	}
	else
	{
		pDC->SetColor(m_bKindable ? MCOLOR(255, 255, 255, 100) : MCOLOR(60, 60, 60, 255));
		pDC->Rectangle(rcSquare);
	}

	// 4. XỬ LÝ TOOLTIP BÁM THEO CHUỘT DÙNG HÀM SMART TOOLTIP
	if (IsMouseOver() && pItemDesc != NULL)
	{
		g_pCurrentHoverDesc = pItemDesc;

		const char* szItemDescription = GetItemDescriptionWidgetName();
		MTextArea* pItemDescTextArea = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szItemDescription);

		if (pItemDescTextArea)
		{
			g_pTargetTooltipWidget = pItemDescTextArea;

			MRECT rcTextArea = pItemDescTextArea->GetRect();
			MPOINT ptMouse = MEvent::LatestPos;

			// Gọi trực tiếp hàm tính vị trí thông minh
			MPOINT posDesc = GetSmartTooltipPosition(ptMouse.x, ptMouse.y, rcTextArea.w, rcTextArea.h);

			pItemDescTextArea->SetPosition(posDesc);
			pItemDescTextArea->SetZOrder(MZ_TOP);
			pItemDescTextArea->Show(true);
		}
	}
	else
	{
		if (IsMouseOver())
		{
			const char* szItemDescription = GetItemDescriptionWidgetName();
			MTextArea* pItemDescTextArea = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szItemDescription);
			if (pItemDescTextArea)
			{
				pItemDescTextArea->Show(false);
			}
		}
	}
}

void ZItemSlotView::SetParts(MMatchCharItemParts nParts)
{
	SetDefaultText(nParts);
	m_nParts = nParts;
}

bool ZItemSlotView::IsDropable(MWidget* pSender)
{
	if (pSender == NULL)
		return false;

	return true;
}


bool ZItemSlotView::IsEquipableItem(unsigned long int nItemID, int nPlayerLevel, MMatchSex nPlayerSex)
{
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc == NULL) return false;

	// ¼ºº° Á¦ÇÑ Á¶°Ç
	if (pItemDesc->m_nResSex.Ref() != -1)
	{
		if (pItemDesc->m_nResSex.Ref() != int(nPlayerSex)) return false;
	}

	// ·¹º§ Á¦ÇÑ Á¶°Ç
	if (pItemDesc->m_nResLevel.Ref() > nPlayerLevel) return false;


	return true;
}

bool ZItemSlotView::OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	if ( (pSender == NULL) || ( m_bDragAndDrop == false))
		return false;

	if (strcmp(pSender->GetClassName(), MINT_EQUIPMENTLISTBOX)==0) {

		// widget id ¼³Á¤
		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		ZShopEquipListbox* pWidget = (ZShopEquipListbox*)pResource->FindWidget("EquipmentList");
		if (pWidget==NULL) return false;

		ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pWidget->GetSelItem();
		if (!pListItem) return false;

		if (ZSEIT_MATCH != pListItem->GetItemData()->GetType()) return false;

		ZShopEquipItem_Match* pWrappedMatchItem = (ZShopEquipItem_Match*)pListItem->GetItemData();
		MMatchItemDesc* pItemDesc = pWrappedMatchItem->GetDesc();
		if (pItemDesc == NULL) return false;
		
		IShopEquipItemHandle_Sell* pHandleSell = pWrappedMatchItem->GetHandleSell();
		MUID uidItem = pHandleSell->GetItemUID();

		if (!IsSuitableItemSlot(pItemDesc->m_nSlot, m_nParts)) return false;
		if (!IsEquipableItem(pItemDesc->m_nID, 99, ZGetMyInfo()->GetSex())) return false;

		ZGetGameInterface()->GetShopEquipInterface()->Equip( m_nParts, uidItem );
	}
	
	else if (strcmp(pSender->GetClassName(), MINT_ITEMSLOTVIEW)==0) {	// Equip ½½·Ô¿¡¼­ Equip ½½·ÔÀ¸·Î
		ZItemSlotView* pWidget = (ZItemSlotView*)pSender;
		MUID uidItem = ZGetMyInfo()->GetItemList()->GetEquipedItemUID(pWidget->GetParts());
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(ZGetMyInfo()->GetItemList()->GetItemID(uidItem));
		if (pItemDesc == NULL) return false;
		if (!IsSuitableItemSlot(pItemDesc->m_nSlot, m_nParts)) return false;
		if (!IsEquipableItem(pItemDesc->m_nID, 99, ZGetMyInfo()->GetSex())) return false;

		ZPostRequestTakeoffItem(ZGetGameClient()->GetPlayerUID(), pWidget->GetParts());
		ZGetGameInterface()->GetShopEquipInterface()->Equip(m_nParts, uidItem);
	}

	if (strcmp( m_szItemSlotPlace, "SACRIFICE0") == 0)
		ZApplication::GetStageInterface()->OnDropSacrificeItem( 0);
	else if ( strcmp( m_szItemSlotPlace, "SACRIFICE1") == 0)
		ZApplication::GetStageInterface()->OnDropSacrificeItem( 1);

	return true;
}

bool ZItemSlotView::OnEvent(MEvent* pEvent, MListener* pListener)
{
	bool bRet = MButton::OnEvent(pEvent, pListener);

	m_bSelectBox = false;

	// Check rect range
	MRECT r = GetClientRect();
	if ( r.InPoint( pEvent->Pos) == false)
		return bRet;


	if (pEvent->nMessage == MWM_LBUTTONDOWN)
	{
		m_bSelectBox = true;

		// ¾ÆÀÌÅÛ ½½·ÔÀÏ °æ¿ì
		unsigned long int nItemID = ZGetMyInfo()->GetItemList()->GetEquipedItemID(m_nParts);
		MMatchItemDesc* pItemDesc = NULL;
		if (nItemID != 0)
		{
			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

			// Custom: Item slot view bugfix crash
			if (!pItemDesc) return false;

			// ¾ÆÀÌÅÛ ¼³¸í ¾÷µ¥ÀÌÆ®
			ZMyItemNode* pItemNode = ZGetMyInfo()->GetItemList()->GetEquipedItem(m_nParts);
			MTextArea* pTextArea = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(GetItemDescriptionWidgetName());

			if (pTextArea)
			{
				g_pCurrentHoverDesc = pItemDesc;
				g_pTargetTooltipWidget = pTextArea;

				// 1. Ép kích thước Steam UI chuẩn
				int nWidth = CONVERT800(165);
				int nHeight = CONVERT800(300);
				pTextArea->SetSize(nWidth, nHeight);

				// 2. Tính vị trí bám chuột thông minh khi Click
				MPOINT ptMouse = MEvent::LatestPos;
				MPOINT posDesc = GetSmartTooltipPosition(ptMouse.x, ptMouse.y, nWidth, nHeight);

				pTextArea->SetPosition(posDesc);
				pTextArea->SetZOrder(MZ_TOP);
				pTextArea->Show(true);
			}

			ZShopEquipItem_Match::FillItemDesc(pItemDesc, pTextArea, pItemNode);

			// µå·¡±× & µå·Ó
			if (pItemDesc && m_bDragAndDrop)
			{
				MBitmap* pIconBitmap = GetItemIconBitmap(pItemDesc);
				Mint::GetInstance()->SetDragObject(this, pIconBitmap, pItemDesc->m_pMItemName->Ref().m_szItemName,
					pItemDesc->m_pMItemName->Ref().m_szItemName);
			}

			ZGetGameInterface()->GetShopEquipInterface()->SetKindableItem(pItemDesc->m_nSlot);
		}


		// Èñ»ý ¾ÆÀÌÅÛ ½½·ÔÀÏ °æ¿ì
		if ( (strcmp( m_szItemSlotPlace, "SACRIFICE0") == 0) || (strcmp( m_szItemSlotPlace, "SACRIFICE1") == 0))
		{
			int nSlotNum = (strcmp( m_szItemSlotPlace, "SACRIFICE0") == 0) ? 0 : 1;

			if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ nSlotNum].IsExist())
				return bRet;

			// µå·¡±× & µå·Ó
			if ( m_bDragAndDrop)
			{
				Mint::GetInstance()->SetDragObject( this,
													ZApplication::GetStageInterface()->m_SacrificeItem[ nSlotNum].GetIconBitmap(),
													ZApplication::GetStageInterface()->m_SacrificeItem[ nSlotNum].GetName(),
													ZApplication::GetStageInterface()->m_SacrificeItem[ nSlotNum].GetName());
			}
		}
	}

	else if ( pEvent->nMessage == MWM_LBUTTONUP)
	{
		ZGetGameInterface()->GetShopEquipInterface()->SetKindableItem( MMIST_NONE);
	}
	
	// ´õºíÅ¬¸¯½Ã ¾ÆÀÌÅÛ ÇØÁ¦
	else if ( pEvent->nMessage == MWM_LBUTTONDBLCLK)
	{
		unsigned long int nItemID = ZGetMyInfo()->GetItemList()->GetEquipedItemID(m_nParts);
		MMatchItemDesc* pItemDesc = NULL;
		if (nItemID != 0)
		{
			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

			if (pItemDesc && m_bDragAndDrop)
			{
				ZPostRequestTakeoffItem( ZGetGameClient()->GetPlayerUID(), m_nParts);
#ifndef UPDATE_STAGE_EQUIP_LOOK
				ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
#endif
			}
		}

		// Èñ»ý ¾ÆÀÌÅÛ ½½·ÔÀÏ °æ¿ì
		if ( (strcmp( m_szItemSlotPlace, "SACRIFICE0") == 0) || (strcmp( m_szItemSlotPlace, "SACRIFICE1") == 0))
		{
			ZApplication::GetStageInterface()->OnRemoveSacrificeItem( (strcmp( m_szItemSlotPlace, "SACRIFICE0") == 0) ? 0 : 1);
		}
	}

	return bRet;
}


void ZItemSlotView::SetBackBitmap(MBitmap* pBitmap)
{
	m_pBackBitmap = pBitmap;
	if( !GetStretch() && m_pBackBitmap!=NULL)
	{
		SetSize(m_pBackBitmap->GetWidth(), m_pBackBitmap->GetHeight());
	}

}

void ZItemSlotView::SetIConBitmap(MBitmap* pBitmap)
{
}

void ZItemSlotView::EnableDragAndDrop( bool bEnable)
{
	m_bDragAndDrop = bEnable;
}

void ZItemSlotView::SetKindable( bool bKindable)
{
	m_bKindable = bKindable;
}

//void ZItemSlotView::OnMouseIn( void )
//{
//	MButton::OnMouseIn();
//
//	// »óÁ¡ ¹× Àåºñ ¾ÆÀÌÅÛ ½½·ÔÀÏ °æ¿ì ÅøÆÁ Ç¥½Ã
//	unsigned long int nItemID = ZGetMyInfo()->GetItemList()->GetEquipedItemID(m_nParts);
//	MMatchItemDesc* pItemDesc = NULL;
//	if (nItemID == 0) return;
//
//	const char* szItemDescription = GetItemDescriptionWidgetName();
//	MTextArea* pItemDescTextArea = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szItemDescription);
//	if (!pItemDescTextArea) return;
//
//	pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
//
//	// Custom: Item slot view bugfix crash
//	if (!pItemDesc) return;
//
//	// ¾ÆÀÌÅÛ ¼³¸í ¾÷µ¥ÀÌÆ®
//	ZMyItemNode* pItemNode = ZGetMyInfo()->GetItemList()->GetEquipedItem( m_nParts);
//	MTextArea* pTextArea = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget( szItemDescription);
//	ZShopEquipItem_Match::FillItemDesc(pItemDesc, pTextArea, pItemNode);
//
//	// ÅøÆÁÀÇ À§Ä¡
//	MRECT rcSlotView = GetRect();
//	MRECT rcTextArea = pItemDescTextArea->GetRect();
//	MPOINT posDesc(rcSlotView.x, rcSlotView.y);
//	posDesc = MClientToScreen(GetParent(), posDesc);
//	posDesc.x = rcSlotView.x + rcSlotView.w + CONVERT800(20);	// ÀÏ´Ü ½½·Ôºä ¿ìÃøÀ¸·Î
//	if (posDesc.y+rcTextArea.h > MGetWorkspaceHeight())			// È­¸é ÇÏ´Ü¿¡ Â©¸®Áö ¾Ê°Ô
//		posDesc.y = MGetWorkspaceHeight() - rcTextArea.h;
//	pItemDescTextArea->SetPosition(posDesc);
//	pItemDescTextArea->SetZOrder(MZ_TOP);
//	ZGetGameInterface()->GetShopEquipInterface()->ShowItemDescription(true, pItemDescTextArea, this);
//}
//
//void ZItemSlotView::OnMouseOut( void )
//{
//	MButton::OnMouseOut();
//
//	MTextArea* pItemDescTextArea = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(GetItemDescriptionWidgetName());
//	if (!pItemDescTextArea) return;
//
//	ZGetGameInterface()->GetShopEquipInterface()->ShowItemDescription(false, pItemDescTextArea, this);
//}
void ZItemSlotView::OnMouseIn(void)
{
	MButton::OnMouseIn();

	unsigned long int nItemID = (m_nItemID != -1) ? m_nItemID : ZGetMyInfo()->GetItemList()->GetEquipedItemID(m_nParts);
	if (nItemID == 0) return;

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (!pItemDesc) return;

	const char* szItemDescription = GetItemDescriptionWidgetName();
	MTextArea* pItemDescTextArea = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szItemDescription);
	if (!pItemDescTextArea) return;

	// Gán Pointer dữ liệu Steam Tooltip
	g_pCurrentHoverDesc = pItemDesc;
	g_pTargetTooltipWidget = pItemDescTextArea;

	// 1. KÍCH THƯỚC CHUẨN
	int nWidth = CONVERT800(165);
	int nHeight = CONVERT800(300);
	pItemDescTextArea->SetSize(nWidth, nHeight);

	// 2. Tính vị trí bám theo con trỏ chuột
	MPOINT ptMouse = MEvent::LatestPos;
	MPOINT posDesc = GetSmartTooltipPosition(ptMouse.x, ptMouse.y, nWidth, nHeight);

	pItemDescTextArea->SetPosition(posDesc);
	pItemDescTextArea->SetZOrder(MZ_TOP);

	// Hiển thị MTextArea
	pItemDescTextArea->Show(true);
}

void ZItemSlotView::OnMouseOut(void)
{
	MButton::OnMouseOut();

	const char* szItemDescription = GetItemDescriptionWidgetName();
	MTextArea* pItemDescTextArea = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szItemDescription);

	if (pItemDescTextArea)
	{
		pItemDescTextArea->Show(false); // Ẩn ngay khi di chuột ra ngoài
	}

	// Chỉ reset khi chính Widget này đang giữ Hover
	if (g_pTargetTooltipWidget == pItemDescTextArea)
	{
		g_pCurrentHoverDesc = NULL;
		g_pTargetTooltipWidget = NULL;
	}
}

//const char* ZItemSlotView::GetItemDescriptionWidgetName()
//{
//	static char szItemDescription[128];
//	sprintf( szItemDescription, "%s_ItemDescription", m_szItemSlotPlace);
//	return szItemDescription;
//}
const char* ZItemSlotView::GetItemDescriptionWidgetName()
{
	// Nếu ô này thuộc giao diện Shop
	if (strstr(m_szItemSlotPlace, "Shop") != NULL || strstr(m_szItemSlotPlace, "SHOP") != NULL)
	{
		return "Shop_ItemDescription";
	}

	// Tất cả các trường hợp trang bị nhân vật còn lại (Equip, MyEquipListFrame...)
	return "Equip_ItemDescription";
}