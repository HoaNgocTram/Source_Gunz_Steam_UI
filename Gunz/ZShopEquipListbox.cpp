#include "stdafx.h"
#include "ZShopEquipListbox.h"
#include "ZShopEquipInterface.h"
#include "ZItemSlotView.h"
#include "ZShop.h"
#include "ZItemIconBitmap.h"


ZShopEquipListItem::ZShopEquipListItem(ZShopEquipItem* pItemData)
: m_pItemData(pItemData)
{
	_ASSERT(m_pItemData);
	m_pItemData->GetName(m_szName);
	m_pItemData->GetLevelResText(m_szLevel);
	m_pItemData->GetPriceText(m_szPrice);
}

ZShopEquipListItem::~ZShopEquipListItem()
{
	delete m_pItemData;
}

//void ZShopEquipListItem::OnDraw( MRECT& r, MDrawContext* pDC, bool bSelected, bool bMouseOver )
//{
//	// ¹è°æ»ö
//	if (bSelected)
//		pDC->SetColor(220, 220, 220);
//	else if (bMouseOver)
//		pDC->SetColor(60, 60, 60);
//	else
//		pDC->SetColor(30, 30, 30);//19 19 19
//
//	pDC->FillRectangle(r);
//
//	MRECT rcIcon;
//	GetIconRect(rcIcon, r);
//	rcIcon.x += r.x;
//	rcIcon.y += r.y;
//
//	// ¾ÆÀÌÄÜ ¹è°æ»ö
//	pDC->SetColor(15, 15, 15);
//	pDC->FillRectangle(rcIcon);
//
//	pDC->SetBitmap(m_pItemData->GetIcon());
//	pDC->Draw(rcIcon);
//
//	MRECT rc;	// ºñÆ®¸ÊÀ» Á¦¿ÜÇÑ ¿µ¿ª
//	rc.x = rcIcon.x + rcIcon.w + 2;
//	rc.w = r.w - rcIcon.w - 6;
//	rc.y = r.y + 2;
//	rc.h = r.h - 4;
//
//	// ÅØ½ºÆ® »ö
//	if (!bSelected)
//		pDC->SetColor(200, 200, 200);
//	else
//		pDC->SetColor(20, 20, 20); //19 19 19
//
//	// ¾ÆÀÌÅÛ¸í, °¡°Ý
//	pDC->TextMultiLine2(rc, GetString(), CONVERT600(2), true, MAM_LEFT | MAM_TOP);
//	pDC->Text(rc, m_szPrice, MAM_RIGHT | MAM_BOTTOM);
//
//	// ·¹º§ (·¹º§ÀÌ ³·Àº °æ¿ì ºÓÀº Ç¥½Ã)
//	// ½æ³×ÀÏ ¾ÆÀÌÄÜ À§¿¡ Âï±â ¶§¹®¿¡ ±×¸²ÀÚ¸¦ ±×·Á¼­ ½Ã¾È¼ºÀ» ³ôÀÎ´Ù
//
//	pDC->SetColor(20, 20, 20);
//	pDC->Text(MRECT(rcIcon.x+1, rcIcon.y, rcIcon.w, rcIcon.h), m_szLevel, MAM_LEFT | MAM_BOTTOM);	// 1ÇÈ¼¿¾¿ ºø°ÜÂïÀ½
//	pDC->Text(MRECT(rcIcon.x-1, rcIcon.y, rcIcon.w, rcIcon.h), m_szLevel, MAM_LEFT | MAM_BOTTOM);
//	pDC->Text(MRECT(rcIcon.x, rcIcon.y, rcIcon.w, rcIcon.h+1), m_szLevel, MAM_LEFT | MAM_BOTTOM);
//	pDC->Text(MRECT(rcIcon.x, rcIcon.y, rcIcon.w, rcIcon.h-1), m_szLevel, MAM_LEFT | MAM_BOTTOM);
//
//	if (ZGetMyInfo()->GetLevel() < m_pItemData->GetLevelRes())
//		pDC->SetColor(200, 10, 10);
//	else
//		pDC->SetColor(200, 200, 200);
//
//	pDC->Text(rcIcon, m_szLevel, MAM_LEFT | MAM_BOTTOM);
//}
void ZShopEquipListItem::OnDraw(MRECT& r, MDrawContext* pDC, bool bSelected, bool bMouseOver)
{
	if (!m_pItemData) return;

	// 1. TÌM VÀ VẼ HÌNH NỀN PNG THEO CURRENCY (BOUNTY / CASH / MEDAL / STAFF / EVENT)
	MBitmap* pSlotBG = NULL;
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

	ZShopEquipItem_Match* pMatchItem = dynamic_cast<ZShopEquipItem_Match*>(m_pItemData);

	// Nếu không dùng dynamic_cast được (hoặc source tắt RTTI), dùng pointer check:
	if (pMatchItem)
	{
		MMatchItemDesc* pDesc = pMatchItem->GetDesc();
		if (pDesc)
		{
			// Gọi itemClass() an toàn
			switch (pDesc->itemClass())
			{
			case ITEM_CASH:   pSlotBG = MBitmapManager::Get("slot_medal.png"); break;
			case ITEM_MEDAL:  pSlotBG = MBitmapManager::Get("slot_cash.png"); break;
			case ITEM_STAFF:  pSlotBG = MBitmapManager::Get("slot_staff.png"); break;
			case ITEM_EVENT:  pSlotBG = MBitmapManager::Get("slot_event.png"); break;
			case ITEM_BOUNTY:
			default:          pSlotBG = MBitmapManager::Get("slot_bounty.png"); break;
			}
		}
	}

	// 2. NẾU LÀ ITEM QUEST HOẶC KHÔNG PHẢI MATCH ITEM -> MẶC ĐỊNH LẤY NỀN BOUNTY
	if (!pSlotBG)
	{
		pSlotBG = MBitmapManager::Get("slot_bounty.png");
	}

	// Draw hình nền PNG phủ kín ô Item (Bounds r)
	if (pSlotBG)
	{
		pDC->SetBitmap(pSlotBG);
		pDC->Draw(r);
	}
	else
	{
		// Dự phòng vẽ màu xám đen nếu thiếu file PNG
		pDC->SetColor(MCOLOR(20, 24, 30));
		pDC->FillRectangle(r);
	}

	// 2. TẠO HIỆU ỨNG CHỌN (SELECT) HOẶC RÊ CHUỘT (HOVER)
	if (bSelected)
	{
		// Lớp phủ sáng nhẹ nền
		pDC->SetColor(MCOLOR(255, 255, 255, 30));
		pDC->FillRectangle(r);

		// Màu Cyan nổi bật (R=0, G=229, B=255)
		pDC->SetColor(MCOLOR(0, 229, 255));

		// Viền ngoài
		pDC->Rectangle(r);

		// Viền trong (Lùi vào 1px mỗi cạnh)
		MRECT rcInnerBorder(r.x + 1, r.y + 1, r.w - 2, r.h - 2);
		pDC->Rectangle(rcInnerBorder);
	}
	else if (bMouseOver)
	{
		// Rê chuột thì chỉ sáng nhẹ nền
		pDC->SetColor(MCOLOR(255, 255, 255, 25));
		pDC->FillRectangle(r);
	}

	// 3. VẼ ICON VẬT PHẨM (CẮT UV STEAM ATLAS CHO CẢ MATCH & QUEST ITEM)
	MRECT rcIcon;
	int nIconAreaHeight = r.h - 20;
	int nIconSize = min(r.w - 8, nIconAreaHeight);

	rcIcon.x = r.x + (r.w - nIconSize) / 2;
	rcIcon.y = r.y + (nIconAreaHeight - nIconSize) / 2 + 2;
	rcIcon.w = nIconSize;
	rcIcon.h = nIconSize;

	bool bDrawn = false;

	// Lấy Item ID trực tiếp qua m_pItemData (Lấy đúng m_nID hoặc m_nItemID)
	int nItemID = m_pItemData ? m_pItemData->GetItemID() : 0;

	if (nItemID > 0)
	{
		ZItemIconInfo iconInfo;
		if (ZGetGameInterface()->GetItemIconMgr()->GetIconInfo(nItemID, iconInfo))
		{
			MBitmap* pAtlas = ZGetGameInterface()->GetItemIconMgr()->GetAtlasBitmap(iconInfo.strSource.c_str());
			if (pAtlas)
			{
				MDrawContextR2* pDCR2 = (MDrawContextR2*)pDC;

				// CẮT UV TỰ ĐỘNG TỪ ATLAS PNG
				pDCR2->Draw(
					pAtlas,
					rcIcon.x, rcIcon.y, rcIcon.w, rcIcon.h,
					iconInfo.nX, iconInfo.nY, iconInfo.nWidth, iconInfo.nHeight
				);
				bDrawn = true;
			}
		}
	}

	// Dự phòng: Nếu item không có trong Atlas XML thì mới vẽ .tga lẻ cũ
	if (!bDrawn && m_pItemData && m_pItemData->GetIcon())
	{
		pDC->SetBitmap(m_pItemData->GetIcon());
		pDC->Draw(rcIcon);
	}

	MFont* pCurrentFont = pDC->GetFont();

	// Tìm font chữ nhỏ hơn (ví dụ font_default hoặc default_a8)
	MFont* pSmallFont = MFontManager::Get("FONTa7"); // Hoặc "default_a9", "font_default"
	if (!pSmallFont) pSmallFont = MFontManager::Get("FONTa7");

	if (pSmallFont) pDC->SetFont(pSmallFont);

	// 4. VẼ TÊN ITEM (SÁT MEP TRÁI BÊN DƯỚI ICON)
	MRECT rcName;
	rcName.x = r.x + 6;                    // Cách lề trái 6px
	rcName.y = r.y + r.h - 32;             // Nằm dòng trên của phần đáy
	rcName.w = r.w - 12;
	rcName.h = 14;

	pDC->SetColor(bSelected ? MCOLOR(255, 255, 255) : MCOLOR(200, 205, 210));
	pDC->Text(rcName, GetString(), MAM_LEFT | MAM_VCENTER); // MAM_LEFT: Căn lề trái

	// 5. VẼ GIÁ TIỀN (NẰM Ở DÒNG DƯỚI CÙNG - CĂN GIỮA HOẶC PHẢI)
	// m_szPrice đã chứa chuỗi giá tiền do GunZ tự format (ví dụ "880" hoặc "20,000")
	if (m_szPrice[0] != '\0')
	{
		MRECT rcPrice;
		rcPrice.x = r.x + 6;
		rcPrice.y = r.y + r.h - 16;        // Dòng sát đáy
		rcPrice.w = r.w - 12;
		rcPrice.h = 14;

		// Màu vàng cho giá tiền
		pDC->SetColor(MCOLOR(255, 200, 50));
		pDC->Text(rcPrice, m_szPrice, MAM_HCENTER | MAM_VCENTER); // MAM_HCENTER: Căn giữa ngang giống Steam
	}
}

bool ZShopEquipListItem::GetDragItem( MBitmap** ppDragBitmap, char* szDragString, char* szDragItemString )
{
	if (!m_pItemData) return true;

	*ppDragBitmap = m_pItemData->GetIcon();
	strcpy(szDragString, m_szName);
	strcpy(szDragItemString, m_szName);
	return true;
}

int ZShopEquipListItem::GetSortHint()
{
	// ¸®½ºÆ®¹Ú½º°¡ ¾ÆÀÌÅÛÀ» Á¾·ùº°·Î Á¤·ÄÇÏµµ·Ï Á¾·ù¿¡ µû¶ó¼­ ¼ø¹øÀ» ¸®ÅÏÇÑ´Ù
	// (°¢ ¾ÆÀÌÅÛÀÇ ·¡ÆÛ Å¬·¡½º°¡ °¢ÀÚ sorthint¸¦ ¸®ÅÏÇÏµµ·Ï ÇÏÁö ¾ÊÀº ÀÌÀ¯´Â °ªµéÀÌ ºÐ»êµÇ¾î ÀÖÀ¸¸é ÄÚµå ÀÐ±â Èûµå´Ï±î..)
	if (ZSEIT_GAMBLE == m_pItemData->GetType())
		return 1000000;

	if (ZSEIT_MATCH == m_pItemData->GetType())
	{
		int hint = 2000000;
		int nResLv = m_pItemData->GetLevelRes();
		MMatchItemDesc* pItemDesc = ((ZShopEquipItem_Match*)m_pItemData)->GetDesc();
		if (!pItemDesc) { _ASSERT(0); return 0; }

		// Custom: Removed automatic sorting for shop based on levels
		if (pItemDesc->m_nType.Ref() == MMIT_MELEE)
			return hint + 100000 + pItemDesc->m_nWeaponType.Ref() * 1000;//+ nResLv;
		if (pItemDesc->m_nType.Ref() == MMIT_RANGE)
			return hint + 200000 + pItemDesc->m_nWeaponType.Ref() * 1000;//+ nResLv;
		if (pItemDesc->m_nType.Ref() == MMIT_EQUIPMENT)
			return hint + 300000 + pItemDesc->m_nSlot * 1000;//+ nResLv;

		hint = 4000000;
		if (pItemDesc->m_nType.Ref() == MMIT_CUSTOM)
		{
			hint = hint + 400000 + pItemDesc->m_nWeaponType.Ref() * 1000;//+ nResLv;
			if (pItemDesc->m_nWeaponType.Ref() == MWT_POTION)
			{
				hint = hint + pItemDesc->m_nEffectId * 100;
			}
			return hint;
		}
		if (pItemDesc->m_nType.Ref() == MMIT_COMMUNITY)
			return hint + 500000 + pItemDesc->m_nWeaponType.Ref() * 1000;//+ nResLv;
		if (pItemDesc->m_nType.Ref() == MMIT_LONGBUFF)
			return hint + 600000 + pItemDesc->m_nWeaponType.Ref() * 1000;//+ nResLv;

		return hint + 700000;//+ nResLv;
	}

	if (ZSEIT_SET == m_pItemData->GetType())
	{
		int nResLv = ((ZShopEquipItem_Set*)m_pItemData)->GetLevelRes();
		return 3000000;//+ nResLv;
	}

	if (ZSEIT_QUEST == m_pItemData->GetType())
		return 5000000;

	return 9999999;
}

void ZShopEquipListItem::GetIconRect(MRECT& out, const MRECT& rcItem)
{
	int len = rcItem.h-4;
	//out.Set(2, 2, len, len);
	out.Set(2, 2, rcItem.w - 4, rcItem.h - 4);
}

bool ZShopEquipListItem::IsPtInRectToShowToolTip(MRECT& rcItem, MPOINT& pt)
{
	// pt°¡ ½æ³×ÀÏ ¾ÆÀÌÄÜ ¿µ¿ª¿¡ ÀÖ´ÂÁö ÆÇ´Ü
	MRECT rcIcon;
	GetIconRect(rcIcon, rcItem);
	// ¸®½ºÆ® ÁÂÇ¥°è·Î º¯È¯
	rcIcon.x += rcItem.x;
	rcIcon.y += rcItem.y;
	// pt-in-rect ÆÇÁ¤
	return (rcIcon.x < pt.x && pt.x < rcIcon.x+rcIcon.w &&
		rcIcon.y < pt.y && pt.y < rcIcon.y+rcIcon.h);
}



ZShopEquipListbox::ZShopEquipListbox(const char* szName, MWidget* pParent, MListener* pListener)
: MMultiColListBox(szName, pParent, pListener)
{
	m_idxItemLastTooltip = -1;
}

bool ZShopEquipListbox::OnEvent(MEvent* pEvent, MListener* pListener)
{
	if (pEvent->nMessage == MWM_MOUSEMOVE)
	{
		// Gọi kiểm tra Tooltip khi di chuột qua từng ô
		SetupItemDescTooltip();
	}

	// Bắt buộc return theo hàm gốc để không bị kẹt Click/Wheel
	return MMultiColListBox::OnEvent(pEvent, pListener);
}

char* ZShopEquipListbox::GetItemDescTooltipName()
{
	if (GetParent() && 0==strcmp("Shop", GetParent()->m_szIDLName))
		return "Shop_ItemDescription";

	if (GetParent() && 0==strcmp("Equipment", GetParent()->m_szIDLName))
		return "Equip_ItemDescription";

	_ASSERT(0);
	return "_noExistWidgetName_";
}

//void ZShopEquipListbox::SetupItemDescTooltip()
//{
//	const char* szTextAreaName = GetItemDescTooltipName();
//	MTextArea* pItemDescTextArea = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szTextAreaName);
//	if (pItemDescTextArea)
//	{
//		MPOINT ptInList = MScreenToClient(this, MEvent::LatestPos);
//		int idxItem = FindItem(ptInList);
//		if (idxItem!=-1)
//		{
//			ZShopEquipListItem* pItem = (ZShopEquipListItem*)Get(idxItem);
//			MRECT rcListBox = GetRect();
//			MRECT rcItem;
//			if (pItem && CalcItemRect(idxItem, rcItem))	// Ç×¸ñÀÌ Ç¥½ÃµÇ°í ÀÖ´Â ¿µ¿ªÀ» ¾Ë¾Æ³¿ (¸®½ºÆ® ÁÂÇ¥°è)
//			{
//				//if (pItem->IsPtInRectToShowToolTip(rcItem, ptInList)) // Ç×¸ñ¿¡°Ô ÀÌ ÁÂÇ¥°¡ ½æ³×ÀÏ ÀÌ¹ÌÁö ¿µ¿ªÀÎÁö ¹°¾îº½
//				if (rcItem.x <= ptInList.x && ptInList.x <= rcItem.x + rcItem.w &&
//					rcItem.y <= ptInList.y && ptInList.y <= rcItem.y + rcItem.h)
//				{
//					if (m_idxItemLastTooltip==idxItem) return;
//					m_idxItemLastTooltip = idxItem;
//
//					pItem->GetItemData()->FillItemDesc(pItemDescTextArea);
//
//					// ÅøÆÁÀÇ À§Ä¡
//					MRECT rcTextArea = pItemDescTextArea->GetRect();
//					MPOINT posDesc(rcItem.x, rcItem.y);
//					posDesc = MClientToScreen(this, posDesc);
//					posDesc.x -= pItemDescTextArea->GetClientWidth();			// ÀÏ´Ü ¾ÆÀÌÄÜÀÇ ¿ÞÂÊÀ¸·Î
//					if (posDesc.y+rcTextArea.h > rcListBox.y + rcListBox.h)		// ¸®½ºÆ® ¹Ú½º ÇÏ´ÜÀ» ³Ñ¾î°¡Áö ¾Ê°Ô ÇÏÀÚ
//						posDesc.y = rcListBox.y + rcListBox.h - rcTextArea.h;
//					if (posDesc.y < 0)											// ±×·¸´Ù°í È­¸é À§·Î ¶Õ°í °¡¸é ¾ÈµÈ´Ù
//						posDesc.y = 0;
//					pItemDescTextArea->SetPosition(posDesc);
//					pItemDescTextArea->SetZOrder(MZ_TOP);
//					ZGetGameInterface()->GetShopEquipInterface()->ShowItemDescription(true, pItemDescTextArea, this);
//					return;
//				}
//			}
//		}
//	}
//
//	m_idxItemLastTooltip = -1;
//	ZGetGameInterface()->GetShopEquipInterface()->ShowItemDescription(false, pItemDescTextArea, this);
//}
extern MMatchItemDesc* g_pCurrentHoverDesc;
extern MTextArea* g_pTargetTooltipWidget;

// Hàm tính toán vị trí Tooltip đi theo chuột thông minh
MPOINT GetSmartTooltipPosition(int nMouseX, int nMouseY, int nTooltipW, int nTooltipH)
{
	int nScreenW = MGetWorkspaceWidth();
	int nScreenH = MGetWorkspaceHeight();
	int nOffset = CONVERT800(15); // Khoảng cách né con trỏ chuột

	MPOINT pos;

	// 1. MẶC ĐỊNH: Đặt Tooltip nằm BÊN PHẢI con trỏ chuột
	pos.x = nMouseX + nOffset;
	pos.y = nMouseY;

	// 2. ĐỔI BÊN SANG TRÁI: Nếu tràn mép phải màn hình (Lỗi typo nTooltipH đã sửa thành nTooltipW)
	if (pos.x + nTooltipW > nScreenW)
	{
		pos.x = nMouseX - nTooltipW - nOffset;
	}

	// 3. GIỮ TRONG MÀN HÌNH (CLAMPING): Nếu đẩy sang trái vẫn bị tràn mép trái
	if (pos.x < 0)
	{
		pos.x = 0;
	}

	// 4. GIỮ KHÔNG BỊ TRÀN MÉP DƯỚI (Nảy ngược lên trên)
	if (pos.y + nTooltipH > nScreenH)
	{
		pos.y = nScreenH - nTooltipH;
	}

	// 5. GIỮ KHÔNG BỊ TRÀN MÉP TRÊN
	if (pos.y < 0)
	{
		pos.y = 0;
	}

	return pos;
}

void ZShopEquipListbox::SetupItemDescTooltip()
{
	const char* szTextAreaName = GetItemDescTooltipName();
	MTextArea* pItemDescTextArea = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szTextAreaName);
	if (pItemDescTextArea)
	{
		MPOINT ptInList = MScreenToClient(this, MEvent::LatestPos);
		int idxItem = FindItem(ptInList);
		if (idxItem != -1)
		{
			ZShopEquipListItem* pItem = (ZShopEquipListItem*)Get(idxItem);
			MRECT rcItem;

			if (pItem && CalcItemRect(idxItem, rcItem))
			{
				if (ptInList.x >= rcItem.x && ptInList.x <= (rcItem.x + rcItem.w) &&
					ptInList.y >= rcItem.y && ptInList.y <= (rcItem.y + rcItem.h))
				{
					m_idxItemLastTooltip = idxItem;

					ZShopEquipItem_Match* pMatchItem = dynamic_cast<ZShopEquipItem_Match*>(pItem->GetItemData());
					if (pMatchItem)
					{
						g_pCurrentHoverDesc = pMatchItem->GetDesc();
						g_pTargetTooltipWidget = pItemDescTextArea;

						// 1. CHỈNH SIZE CHUẨN STEAM UI
						int nWidth = CONVERT800(165);
						int nHeight = CONVERT800(300);
						pItemDescTextArea->SetSize(nWidth, nHeight);

						// 2. Tính vị trí bám theo con trỏ chuột thực tế
						MPOINT ptMouse = MEvent::LatestPos;
						MPOINT posDesc = GetSmartTooltipPosition(ptMouse.x, ptMouse.y, nWidth, nHeight);

						pItemDescTextArea->SetPosition(posDesc);
						pItemDescTextArea->SetZOrder(MZ_TOP);

						ZGetGameInterface()->GetShopEquipInterface()->ShowItemDescription(true, pItemDescTextArea, this);
						return;
					}
					else
					{
						g_pCurrentHoverDesc = NULL;
						g_pTargetTooltipWidget = NULL;
					}
				}
			}
		}
	}

	// Không trỏ vào item nào thì reset biến về NULL
	g_pCurrentHoverDesc = NULL;
	g_pTargetTooltipWidget = NULL;
	m_idxItemLastTooltip = -1;
	ZGetGameInterface()->GetShopEquipInterface()->ShowItemDescription(false, pItemDescTextArea, this);
}


// ============ ZShopPurchaseItemListBoxListener ===============================================

void ShopPurchaseItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
}

void ShopSaleItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
}

void CharacterEquipmentItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	if (pSender == NULL) return;
	if (strcmp(pSender->GetClassName(), MINT_ITEMSLOTVIEW)) return;

	ZItemSlotView* pItemSlotView = (ZItemSlotView*)pSender;

	ZPostRequestTakeoffItem(ZGetGameClient()->GetPlayerUID(), pItemSlotView->GetParts());
}

// ============ ZShopPurchaseItemListBoxListener ===============================================

class ZShopPurchaseItemListBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MLB_ITEM_SEL) == true)
		{
			ZShopEquipListbox* pEquipmentListBox = (ZShopEquipListbox*)pWidget;
			ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pEquipmentListBox->GetSelItem();
			if (pListItem == NULL) return true;

			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformationShop");
			if (pCharacterView)
				pListItem->GetItemData()->UpdateCharacterView(pCharacterView);

			pListItem->GetItemData()->UpdateCharInfoText();

			ZGetGameInterface()->GetShopEquipInterface()->SelectArmorWeaponTabWithSlotType( pListItem->GetItemData()->GetSlotType());

			WidgetEnableShow("BuyConfirmCaller", true, true);

			return true;
		}
		else if(MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK) == true)
		{
			ZGetGameInterface()->GetShopEquipInterface()->OnBuyButton();
			return true;
		}

		return false;
	}
};
ZShopPurchaseItemListBoxListener g_ShopPurchaseItemListBoxListener;

MListener* ZGetShopPurchaseItemListBoxListener(void)
{
	return &g_ShopPurchaseItemListBoxListener;
}

class ZEquipMyItemListBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if ( MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true) {

			ZShopEquipListbox* pEquipmentListBox = (ZShopEquipListbox*)pWidget;
			ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pEquipmentListBox->GetSelItem();
			if (!pListItem) return false;

			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformation");
			if (pCharacterView)
				pListItem->GetItemData()->UpdateCharacterView(pCharacterView);

			pListItem->GetItemData()->UpdateCharInfoText();

			ZGetGameInterface()->GetShopEquipInterface()->SetKindableItem( pListItem->GetItemData()->GetSlotType());
			ZGetGameInterface()->GetShopEquipInterface()->SelectArmorWeaponTabWithSlotType( pListItem->GetItemData()->GetSlotType());

			WidgetEnableShow("Equip",				pListItem->GetItemData()->CanEquip(), true);
			WidgetEnableShow("SendAccountItemBtn",	pListItem->GetItemData()->CanSendAccount(), true);

			return true;
		}
		else if ( MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			ZGetGameInterface()->GetShopEquipInterface()->Equip();		
			return true;
		}

		return false;
	}
};
ZEquipMyItemListBoxListener g_EquipmentItemListBoxListener;

MListener* ZGetEquipmentMyItemListBoxListener(void)
{
	return &g_EquipmentItemListBoxListener;
}


class ZShopSellItemListBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true)
		{
			ZShopEquipListbox* pListbox = (ZShopEquipListbox*)pWidget;
			ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pListbox->GetSelItem();
			
			if (!pListItem) return false;

			ZGetGameInterface()->GetShopEquipInterface()->DrawCharInfoText();
			ZGetGameInterface()->GetShopEquipInterface()->SetKindableItem( pListItem->GetItemData()->GetSlotType());

			WidgetEnableShow("SellConfirmCaller", pListItem->GetItemData()->CanSell(), true);
			return true;
		}
		else if ( MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			ZGetGameInterface()->GetShopEquipInterface()->OnSellButton();
			return true;
		}

		return false;
	}
};

ZShopSellItemListBoxListener g_ShopSellItemListBoxListener;

MListener* ZGetShopSellItemListBoxListener(void)
{
	return &g_ShopSellItemListBoxListener;
}

class ZAccountItemListBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if ( MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true) {

			ZShopEquipListbox* pListbox = (ZShopEquipListbox*)pWidget;
			ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pListbox->GetSelItem();
			if (!pListItem) return false;

			unsigned long int nItemID = ZGetMyInfo()->GetItemList()->GetAccountItemID( pListbox->GetSelIndex());

			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformation");
			if (pCharacterView)
				pListItem->GetItemData()->UpdateCharacterView(pCharacterView);

			pListItem->GetItemData()->UpdateCharInfoText();
			ZGetGameInterface()->GetShopEquipInterface()->SelectArmorWeaponTabWithSlotType( pListItem->GetItemData()->GetSlotType());

			// Áß¾ÓÀºÇà¿¡¼­ ¹Ù¿îÆ¼ ¾ÆÀÌÅÛÀÌ µé¾îÀÖ´Ù¶óµµ Á¶°Ç¾øÀÌ ³»Àåºñ·Î ¿Å±æ¼öÀÖ´Ù.
			WidgetEnableShow("BringAccountItemBtn", true, true);

			return true;
		}
		else if ( MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			ZGetGameInterface()->GetShopEquipInterface()->OnBringAccountButton();
			return true;
		}

		return false;
	}
};

ZAccountItemListBoxListener g_AccountItemListBoxListener;

MListener* ZGetAccountItemListBoxListener(void)
{
	return &g_AccountItemListBoxListener;
}


class ZShopListFilterListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MCMBBOX_CHANGED)==true)
		{
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

			// Custom: Sub-Filter
			ZGetShop()->m_ListSubFilter = 0;
			ZGetShop()->m_strSearch = "";

			// Custom: hid code in scope
			{
				MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentFilter");
				if ( pComboBox)
				{
					int sel = pComboBox->GetSelIndex();

					// »ç±â»óÅÂ¶ó¸é
					ZGetShop()->m_ListFilter = sel;
					ZGetShop()->Serialize();

					// ÆÈ±â»óÅÂ¶ó¸é - ÆÈ±â´Â ´Ù º¸¿©ÁØ´Ù..
					ZMyItemList* pil = ZGetMyInfo()->GetItemList();
					pil->m_ListFilter = sel;
					pil->Serialize();
				}
			}

			// Custom: Sub-Filter
			{
				MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentSubFilter");
				if ( pComboBox)
				{
					pComboBox->RemoveAll();

					if( ZGetShop()->m_ListFilter == 0 )
						pComboBox->Add("Select Filter");
					else
						pComboBox->Add("All");

					pComboBox->SetSelIndex( 0 );
					ZGetShop()->m_ListSubFilter = 0;

					// Filter: 0 -> Show All Equipment.

					int nListFilter = ZGetShop()->m_ListFilter;

					if( nListFilter == 8 ) // Melee weapons
					{
						pComboBox->Add("Daggers");
						pComboBox->Add("Sword / Katana");
						pComboBox->Add("Kodachi");
						pComboBox->Add("Large Swords");
					}
					else if( nListFilter == 9 ) // Ranged weapons
					{
						pComboBox->Add("Shotguns");
						pComboBox->Add("SMGs");
						pComboBox->Add("Rifles / MGs");
						pComboBox->Add("Pistols");
						pComboBox->Add("Revolvers");
						pComboBox->Add("Rocket Launchers");
						pComboBox->Add("Snipers");
					}
					else if( nListFilter == 10 ) // Kits or Pills
					{
						pComboBox->Add("Kits / Food");
						pComboBox->Add("Pills");
						pComboBox->Add("Grenades");
					}
				}
			}

			// Custom: ShopSearch

			{
				MEdit* pEdit = (MEdit*)pResource->FindWidget("Shop_Search");
				if (pEdit)
				{
					pEdit->GetText();
				}
			}

			ZGetGameInterface()->GetShopEquipInterface()->SelectShopTab(-1);
		}
		return true;
	}
};

ZShopListFilterListener g_ShopListFilterListener;

MListener* ZGetShopListFilterListener()
{
	return &g_ShopListFilterListener;
}


// Custom: Sub-Filter
class ZShopListSubFilterListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MCMBBOX_CHANGED)==true)
		{
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

			MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentSubFilter");
			if ( pComboBox)
			{
				int sel = pComboBox->GetSelIndex();

				ZGetShop()->m_ListSubFilter = sel;
				ZGetShop()->Serialize();

				ZMyItemList* pil = ZGetMyInfo()->GetItemList();
				pil->m_ListSubFilter = sel;
				pil->Serialize();
			}

			ZGetGameInterface()->GetShopEquipInterface()->SelectShopTab(-1);
		}
		return true;
	}
};

ZShopListSubFilterListener g_ShopListSubFilterListener;

MListener* ZGetShopListSubFilterListener()
{
	return &g_ShopListSubFilterListener;
}

class ZShopSearchListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szText)
	{
		if (MWidget::IsMsg(szText, MEDIT_TEXT_CHANGED) == true)
		{
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
			MEdit* pEdit = (MEdit*)pResource->FindWidget("Shop_Search");
			if (pEdit)
			{
				string strText = pEdit->GetText();
				ZGetShop()->m_strSearch = strText.c_str();
				ZMyItemList* pItemList = ZGetMyInfo()->GetItemList();
				pItemList->m_strSearch = strText.c_str();
				ZGetShop()->Serialize();
			}
		}
		return true;
	}
}; ZShopSearchListener g_ShopSearchListener;

MListener* ZGetShopSearchListener()
{
	return &g_ShopSearchListener;
}

/////////////////////////////////////////////////////////////////

class MEquipListFilterListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MCMBBOX_CHANGED)==true) {

			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

			// Custom: Sub-Filter
			ZMyItemList* pil = ZGetMyInfo()->GetItemList();
			pil->m_ListSubFilter = 0;
			pil->m_strSearch = "";

			// Custom: hid code in scope
			{
				MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Equip_AllEquipmentFilter");
				if ( pComboBox)
				{
					int sel = pComboBox->GetSelIndex();

					//ZMyItemList* pil = ZGetMyInfo()->GetItemList();
					pil->m_ListFilter = sel;
					pil->Serialize();
				}
			}

			// Custom: Sub-Filter
			{
				MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Equip_AllEquipmentSubFilter");
				if ( pComboBox)
				{
					pComboBox->RemoveAll();

					if( pil->m_ListFilter == 0 )
						pComboBox->Add("Select Filter");
					else
						pComboBox->Add("All");

					pComboBox->SetSelIndex( 0 );
					pil->m_ListSubFilter = 0;

					// Filter: 0 -> Show All Equipment.

					int nListFilter = pil->m_ListFilter;

					if( nListFilter == 8 ) // Melee weapons
					{
						pComboBox->Add("Daggers");
						pComboBox->Add("Sword / Katana");
						pComboBox->Add("Kodachi");
						pComboBox->Add("Large Swords");
					}
					else if( nListFilter == 9 ) // Ranged weapons
					{
						pComboBox->Add("Shotguns");
						pComboBox->Add("SMGs");
						pComboBox->Add("Rifles / MGs");
						pComboBox->Add("Pistols");
						pComboBox->Add("Revolvers");
						pComboBox->Add("Rocket Launchers");
						pComboBox->Add("Snipers");
					}
					else if( nListFilter == 10 ) // Kits or Pills
					{
						pComboBox->Add("Kits / Food");
						pComboBox->Add("Pills");
						pComboBox->Add("Grenades");
					}
					else if (nListFilter == 15) // Cash items
					{
						pComboBox->Add("Daggers");
						pComboBox->Add("Sword / Katana");
						pComboBox->Add("Kodachi");
						pComboBox->Add("Large Swords");
						pComboBox->Add("Shotguns");
						pComboBox->Add("SMGs");
						pComboBox->Add("Rifles / MGs");
						pComboBox->Add("Pistols");
						pComboBox->Add("Revolvers");
						pComboBox->Add("Rocket Launchers");
						pComboBox->Add("Snipers");
						pComboBox->Add("Kits / Food");
					}
				}
			}

			{
				MEdit* pEdit = (MEdit*)pResource->FindWidget("Equip_Search");
				if (pEdit)
				{
					pEdit->GetText();
				}
			}

			ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentTab( -1);
		}
		return true;
	}
};

MEquipListFilterListener g_EquipListFilterListener;

MListener* ZGetEquipListFilterListener()
{
	return &g_EquipListFilterListener;
}


// Custom: Sub-Filter
class ZEquipListSubFilterListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MCMBBOX_CHANGED)==true)
		{
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

			MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Equip_AllEquipmentSubFilter");
			if ( pComboBox)
			{
				int sel = pComboBox->GetSelIndex();

				ZMyItemList* pil = ZGetMyInfo()->GetItemList();
				pil->m_ListSubFilter = sel;
				pil->Serialize();
			}

			ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentTab(-1);
		}
		return true;
	}
};

ZEquipListSubFilterListener g_EquipListSubFilterListener;

MListener* ZGetEquipListSubFilterListener()
{
	return &g_EquipListSubFilterListener;
}

class ZEquipSearchListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szText)
	{
		if (MWidget::IsMsg(szText, MEDIT_TEXT_CHANGED) == true)
		{
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
			MEdit* pEdit = (MEdit*)pResource->FindWidget("Equip_Search");
			if (pEdit)
			{
				string strText = pEdit->GetText();
				ZMyItemList* pItemList = ZGetMyInfo()->GetItemList();
				pItemList->m_strSearch = strText.c_str();
				pItemList->Serialize();
			}
		}
		return true;
	}
}; ZEquipSearchListener g_EquipSearchListener;

MListener* ZGetEquipSearchListener()
{
	return &g_EquipSearchListener;
}

class MCashShopItemListBoxListener : public MListener
{
/*public:
	virtual bool OnCommand( MWidget* pWidget, const char* szMessage)
	{
		if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL)==true)
		{
			unsigned long int nItemID = 0;

			ZShopEquipListbox* pEquipmentListBox = (ZShopEquipListbox*)pWidget;
			ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pEquipmentListBox->GetSelItem();
			if ( pListItem)
			{
				//MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc( pListItem->GetItemID());
				ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget( "EquipmentInformationShop");
				
				if (pCharacterView)
					pListItem->GetItemData()->UpdateCharacterView(pCharacterView);

				pListItem->GetItemData()->UpdateCharInfoText();

				ZGetGameInterface()->GetShopEquipInterface()->SelectArmorWeaponTabWithSlotType(pListItem->GetItemData()->GetSlotType());

				MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "BuyCashConfirmCaller");
				if ( pButton)
					pButton->Enable( true);

				return true;
			}
		}

		else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
		{
			return true;
		}


		return false;
	}*/
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MLB_ITEM_SEL) == true)
		{
			ZShopEquipListbox* pEquipmentListBox = (ZShopEquipListbox*)pWidget;
			ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pEquipmentListBox->GetSelItem();
			if (pListItem == NULL) return true;

			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformationShop");
			if (pCharacterView)
				pListItem->GetItemData()->UpdateCharacterView(pCharacterView);

			pListItem->GetItemData()->UpdateCharInfoText();

			ZGetGameInterface()->GetShopEquipInterface()->SelectArmorWeaponTabWithSlotType(pListItem->GetItemData()->GetSlotType());

			WidgetEnableShow("BuyCashConfirmCaller", true, true);

			return true;
		}
		else if (MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK) == true)
		{
			ZGetGameInterface()->GetShopEquipInterface()->OnBuyCashButton();
			return true;
		}

		return false;
	}
};



MCashShopItemListBoxListener g_CashShopItemListBoxListener;

MListener* ZGetCashShopItemListBoxListener(void)
{
	return &g_CashShopItemListBoxListener;
}

class MMedalShopItemListBoxListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MLB_ITEM_SEL) == true)
		{
			ZShopEquipListbox* pEquipmentListBox = (ZShopEquipListbox*)pWidget;
			ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pEquipmentListBox->GetSelItem();
			if (pListItem == NULL) return true;

			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformationShop");
			if (pCharacterView)
				pListItem->GetItemData()->UpdateCharacterView(pCharacterView);

			pListItem->GetItemData()->UpdateCharInfoText();

			ZGetGameInterface()->GetShopEquipInterface()->SelectArmorWeaponTabWithSlotType(pListItem->GetItemData()->GetSlotType());

			// Bật nút Mua của Medal Shop
			WidgetEnableShow("BuyMedalConfirmCaller", true, true);

			return true;
		}
		else if (MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK) == true)
		{
			// Double click để mua đồ Medal
			ZGetGameInterface()->GetShopEquipInterface()->OnBuyMedalButton();
			return true;
		}

		return false;
	}
};

MMedalShopItemListBoxListener g_MedalShopItemListBoxListener;
MListener* ZGetMedalShopItemListBoxListener(void)
{
	return &g_MedalShopItemListBoxListener;
}

class MEventShopItemListBoxListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MLB_ITEM_SEL) == true)
		{
			ZShopEquipListbox* pEquipmentListBox = (ZShopEquipListbox*)pWidget;
			ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pEquipmentListBox->GetSelItem();
			if (pListItem == NULL) return true;

			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformationShop");
			if (pCharacterView)
				pListItem->GetItemData()->UpdateCharacterView(pCharacterView);

			pListItem->GetItemData()->UpdateCharInfoText();

			ZGetGameInterface()->GetShopEquipInterface()->SelectArmorWeaponTabWithSlotType(pListItem->GetItemData()->GetSlotType());

			// Bật nút Mua của Event Shop
			WidgetEnableShow("BuyEventConfirmCaller", true, true);

			return true;
		}
		else if (MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK) == true)
		{
			// Double click để mua đồ Event
			ZGetGameInterface()->GetShopEquipInterface()->OnBuyEventButton();
			return true;
		}

		return false;
	}
};

MEventShopItemListBoxListener g_EventShopItemListBoxListener;
MListener* ZGetEventShopItemListBoxListener(void)
{
	return &g_EventShopItemListBoxListener;
}

class MPackageShopItemListBoxListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		// 1. Khi CLICK CHỌN Package trong danh sách
		if (MWidget::IsMsg(szMessage, MLB_ITEM_SEL) == true)
		{
			MListBox* pListBox = (MListBox*)pWidget;
			int nSelIndex = pListBox->GetSelIndex();
			if (nSelIndex < 0) return true;

			// Lưu Index Package đang chọn & Preview nguyên Set đồ lên 3D
			ZGetGameInterface()->GetShopEquipInterface()->OnSelectPackage(nSelIndex);

			// Bật nút Buy Package Confirm (nếu bác xài popup xác nhận Mua)
			WidgetEnableShow("BuyPackageConfirmCaller", true, true);

			return true;
		}
		// 2. Khi DOUBLE-CLICK vào Package -> Thực hiện Mua luôn
		else if (MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK) == true)
		{
			ZGetGameInterface()->GetShopEquipInterface()->OnBuyPackageButton();
			return true;
		}

		return false;
	}
};

MPackageShopItemListBoxListener g_PackageShopItemListBoxListener;
MListener* ZGetPackageShopItemListBoxListener(void)
{
	return &g_PackageShopItemListBoxListener;
}