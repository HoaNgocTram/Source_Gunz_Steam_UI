#include "stdafx.h"
#include "ZShopEquipInterface.h"
#include "ZShopEquipListbox.h"
#include "ZConfiguration.h"
#include "ZShop.h"
#include "ZItemSlotView.h"

#include "ZApplication.h"
#include "ZGameInterface.h"
#include "ZCharacterView.h"
#include "ZShopEquipItem.h"

static ZIDLResource* GetIDLResource() {
	return ZGetGameInterface()->GetIDLResource();
}

// ¹öÆ°¿¡ ¸¶¿ì½º¿À¹öÇÒ¶§ ³ªÅ¸³ª´Â ÃÊ·ÏºûÀÌ »ç¶óÁöÁö ¾Ê¾Æ¼­ ÀÌ·¸°Ô ÇØµÐ°Í °°´Ù. ÀÌÄÚµå°¡ ¿©±âÀú±â ¸¹¾Æ¼­ ÇÔ¼ö·Î ¸¸µé¾î¹ö·ÈÀ½
void WidgetHideDisableShow(const char* szWidget)
{
	MWidget* pWidget = GetIDLResource()->FindWidget( szWidget);
	if ( pWidget)
	{
		pWidget->Show( false);
		pWidget->Enable( false);
		pWidget->Show( true);
	}
}
// ÀÌ·¸°Ô µÈ °÷µµ ÀÖ´Âµ¥ ÀÌ°Ç ¿Ö ¼û±â±â¸¸ ÇÏ´ÂÁö ¸ð¸£°Ú´Ù -_-;
void WidgetHideDisable(const char* szWidget)
{
	MWidget* pWidget = GetIDLResource()->FindWidget( szWidget);
	if ( pWidget)
	{
		pWidget->Show( false);
		pWidget->Enable( false);
		//pWidget->Show( true);
	}
}

void WidgetEnableShow(const char* szWidget, bool bEnable, bool bShow)
{
	MWidget* pWidget = GetIDLResource()->FindWidget( szWidget);
	if (pWidget)
	{
		pWidget->Enable(bEnable);
		pWidget->Show(bShow);
	}
}


//##################################################################################################

ZShopEquipInterface::ZShopEquipInterface()
{
	m_nShopTabNum = 0;
	m_nEquipTabNum = 0;

	m_idxArmorWeaponTab = 1;

	m_pItemDescriptionClient = 0;

	m_pItemCountDlg = new ZItemCountDlg;
	m_pSimpleConfirmDlg = new ZSimpleConfirmDlg;
	m_pSellCashItemConfirmDlg = new ZSellCashItemConfirmDlg;

	// Custom: Shop/Equip View Switch
	m_bViewThumbnails = true;

	// --- Khởi tạo thêm cho Shop kiểu Steam ---
	m_nCurrentTab = ZSHOP_TAB_NEW;
	m_nSelectedPackageIndex = 0;
	m_nCurrentCategory = ZSHOP_CAT_ALL;
	m_nSelectedItemID = -1;
};

ZShopEquipInterface::~ZShopEquipInterface()
{
	delete m_pItemCountDlg;
	delete m_pSimpleConfirmDlg;
	delete m_pSellCashItemConfirmDlg;
}

void ZShopEquipInterface::ConfirmBuySimple()
{

}

ZShopEquipListItem* ZShopEquipInterface::GetListCurSelItem(const char* szListWidget)
{
	ZShopEquipListbox* pListbox = (ZShopEquipListbox*)ZGetGameInterface()->GetIDLResource()->FindWidget( szListWidget);
	if (pListbox)
		return (ZShopEquipListItem*)pListbox->GetSelItem();
	return NULL;
}

void ZShopEquipInterface::OnSellButton(void)
{
	static unsigned long int st_LastRequestTime = 0;	///< ¸· ´©¸£´Â °Í ¹æÁö
	unsigned long int nNowTime = timeGetTime();
	if ((nNowTime - st_LastRequestTime) < 1000) return;

	st_LastRequestTime = nNowTime;

	ZShopEquipListItem* pListItem = (ZShopEquipListItem*)GetListCurSelItem("EquipmentList");
	if (!pListItem) {
		ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NO_SELITEM ); 
		return;
	}

	if (pListItem->GetItemData()->GetHandleSell())
		pListItem->GetItemData()->GetHandleSell()->Sell();
}

void ZShopEquipInterface::OnBuyButton(void)
{
	static unsigned long int st_LastRequestTime = 0;	///< ¸· ´©¸£´Â °Í ¹æÁö
	unsigned long int nNowTime = timeGetTime();
	if ((nNowTime - st_LastRequestTime) < 1000) return;

	st_LastRequestTime = nNowTime;

	ZShopEquipListItem* pListItem = (ZShopEquipListItem*)GetListCurSelItem("AllEquipmentList");
	if (!pListItem) {
		ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NO_SELITEM ); 
		return;
	}

	if (pListItem->GetItemData()->GetHandlePurchase())
		pListItem->GetItemData()->GetHandlePurchase()->Buy();
}

void ZShopEquipInterface::OnBuyCashButton(void)
{
	static unsigned long int st_LastRequestTime = 0;	///< ¸· ´©¸£´Â °Í ¹æÁö
	unsigned long int nNowTime = timeGetTime();
	if ((nNowTime - st_LastRequestTime) < 1000) return;

	st_LastRequestTime = nNowTime;

	ZShopEquipListItem* pListItem = (ZShopEquipListItem*)GetListCurSelItem("CashEquipmentList");
	if (!pListItem) {
		ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NO_SELITEM ); 
		return;
	}

	if (pListItem->GetItemData()->GetHandlePurchase())
		pListItem->GetItemData()->GetHandlePurchase()->Buy();
}
void ZShopEquipInterface::OnBuyMedalButton(void)
{
	static unsigned long int st_LastRequestTime = 0;
	unsigned long int nNowTime = timeGetTime();
	if ((nNowTime - st_LastRequestTime) < 1000) return; // Anti-spam click 1s

	st_LastRequestTime = nNowTime;

	ZShopEquipListItem* pListItem = (ZShopEquipListItem*)GetListCurSelItem("MedalEquipmentList");
	if (!pListItem) {
		ZApplication::GetGameInterface()->ShowErrorMessage(MERR_NO_SELITEM);
		return;
	}

	if (pListItem->GetItemData()->GetHandlePurchase())
		pListItem->GetItemData()->GetHandlePurchase()->Buy();
}
void ZShopEquipInterface::OnBuyEventButton(void)
{
	static unsigned long int st_LastRequestTime = 0;	
	unsigned long int nNowTime = timeGetTime();
	if ((nNowTime - st_LastRequestTime) < 1000) return;

	st_LastRequestTime = nNowTime;

	ZShopEquipListItem* pListItem = (ZShopEquipListItem*)GetListCurSelItem("EventEquipmentList");
	if (!pListItem) {
		ZApplication::GetGameInterface()->ShowErrorMessage(MERR_NO_SELITEM);
		return;
	}

	if (pListItem->GetItemData()->GetHandlePurchase())
		pListItem->GetItemData()->GetHandlePurchase()->Buy();
}

void ZShopEquipInterface::OnClickBuyPackage()
{
	if (m_nSelectedPackageIndex < 0 || m_nSelectedPackageIndex >= (int)m_vPackages.size())
		return;

	const ZPACKAGE_INFO& pack = m_vPackages[m_nSelectedPackageIndex];

	// 1. Check rương đồ đầy
	if (ZGetMyInfo()->GetItemList()->GetItemCount() >= 500)
	{
		if (ZGetGameInterface())
			ZGetGameInterface()->ShowMessage("Your inventory is full.!", NULL);
		return;
	}

	// 2. Xác định tên đơn vị tiền tệ dựa trên nCurrencyType
	const char* szCurrency = "Cash";
	if (pack.nCurrencyType == ITEM_MEDAL) szCurrency = "Medal";
	else if (pack.nCurrencyType == ITEM_EVENT) szCurrency = "Event Coin";

	// 3. Mở Pop-up Confirm
	char szMsg[256];
	sprintf(szMsg, "Are you sure you want to buy the '%s' package for %d %s?",
		pack.szName.c_str(),
		pack.nPrice,
		szCurrency);

	if (m_pSimpleConfirmDlg)
	{
		m_pSimpleConfirmDlg->Open(szMsg, this);
	}
}
void ZShopEquipInterface::OnDone(bool bOk)
{
	if (!bOk) return; // Bấm Cancel -> Bỏ qua

	// Bấm OK -> Gọi hàm gửi Command lên Server
	RequestBuyPackage(m_nSelectedPackageIndex);
}

// ¾ÆÀÌÅÛÀÇ ½½·Ô Á¤º¸¿¡ µû¶ó Âø¿ëÇÒ ÆÄÃ÷¸¦ °ñ¶óÁØ´Ù
// ¿¹¸¦ µé¾î custom1¿¡ ÀÌ¹Ì ÀåÂøµÈ °æ¿ì custom2°¡ ºñ¾îÀÖÀ¸¸é custom2¸¦ °ñ¶óÁÖ°í, µÑ´Ù ÀåÂøÁßÀÌ¸é custom1À» °ñ¶óÁÖ´Â ½Ä
MMatchCharItemParts ZShopEquipInterface::RecommendEquipParts(MMatchItemSlotType slot)
{
	MMatchCharItemParts parts = MMCIP_END;

	if (slot == MMIST_RANGE)
	{
		if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_PRIMARY) == 0)
		{
			parts = MMCIP_PRIMARY;
		}
		else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_SECONDARY) == 0)
		{
			parts = MMCIP_SECONDARY;
		}
	}
	else if (slot == MMIST_CUSTOM)
	{
		if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_CUSTOM1) == 0)
		{
			parts = MMCIP_CUSTOM1;
		}
		else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_CUSTOM2) == 0)
		{
			parts = MMCIP_CUSTOM2;
		}
		else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_CUSTOM3) == 0)
		{
			parts = MMCIP_CUSTOM3;
		}
	}
	else if (slot == MMIST_FINGER)
	{
		if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_FINGERL) == 0)
		{
			parts = MMCIP_FINGERL;
		}
		else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_FINGERR) == 0)
		{
			parts = MMCIP_FINGERR;
		}
	}
	else if (slot == MMIST_AVATAR)
	{
		if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_AVATAR) == 0)
		{
			parts = MMCIP_AVATAR;
		}
	}
	else if (slot == MMIST_COMMUNITY)
	{
		if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_COMMUNITY1) == 0)
		{
			parts = MMCIP_COMMUNITY1;
		}
		else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_COMMUNITY2) == 0)
		{
			parts = MMCIP_COMMUNITY2;
		}

	}
	else if (slot == MMIST_LONGBUFF)
	{
		if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_LONGBUFF1) == 0)
		{
			parts = MMCIP_LONGBUFF1;
		}
		else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_LONGBUFF2) == 0)
		{
			parts = MMCIP_LONGBUFF2;
		}
	}

	if (parts == MMCIP_END)
	{
		parts = GetSuitableItemParts(slot);
	}

	return parts;
}

bool ZShopEquipInterface::Equip(void)
{
	static unsigned long int st_LastRequestTime = 0;	///< ¸· ´©¸£´Â °Í ¹æÁö
	unsigned long int nNowTime = timeGetTime();
	if ((nNowTime - st_LastRequestTime) < 1000) return false;

	st_LastRequestTime = nNowTime;

	ZShopEquipListItem* pListItem = GetListCurSelItem("EquipmentList");
	if (!pListItem) 
	{
		ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NO_SELITEM );
		return false;
	}

	// °·ºí ¾ÆÀÌÅÛ
	if (ZSEIT_GAMBLE == pListItem->GetItemData()->GetType())
	{
		ZShopEquipItem_Gamble* pWrapperGItem = (ZShopEquipItem_Gamble*)pListItem->GetItemData();
		const ZGambleItemDefine* pGItemDesc = pWrapperGItem->GetDesc();
		if (pGItemDesc)
		{
			// ÀÌ¹ø¿¡ Ä³½Ã¿ë °×ºí¾ÆÀÌÅÛ Ãß°¡·Î ¼öÁ¤
			// ±âÁ¸ ÀÏº» ¿äÃ»À¸·Î Äù½ºÆ®¿ë °×ºí¾ÆÀÌÅÛÀ» Ãß°¡Çß¾úÀ½ (1000100~1000199»çÀÌ)
			// Äù½ºÆ®¿ë °×ºí¾ÆÀÌÅÛÀº 1000100¿¡¼­ 1000200·Î Á¦ÇÑÇÑ´Ù
			// Custom: Quest/Clan Server
			if( /*ZGetGameClient()->GetServerMode() != MSM_TEST &&*/ 1000100 <= pGItemDesc->GetGambleItemID()  && pGItemDesc->GetGambleItemID() < 1000200)
			{
				ZApplication::GetGameInterface()->ShowErrorMessage( MERR_CANNOT_CAHNGE_THIS_ITEM );
			}
			else
			{
				if (!pWrapperGItem->GetHandleSell()) { _ASSERT(0); return false; }
				ZPostRequestGamble( pWrapperGItem->GetHandleSell()->GetItemUID() );
				ZPostRequestCharacterItemList( ZGetGameClient()->GetPlayerUID() );
			}
		}
	}

	// ÀÏ¹Ý ¾ÆÀÌÅÛ
	if (ZSEIT_MATCH != pListItem->GetItemData()->GetType())
	{
		return false;
	}

	ZShopEquipItem_Match* pWrappedZItem = (ZShopEquipItem_Match*)pListItem->GetItemData();
	if (!pWrappedZItem->GetHandleSell()) { _ASSERT(0); return false; }

	MUID uidItem = pWrappedZItem->GetHandleSell()->GetItemUID();
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(ZGetMyInfo()->GetItemList()->GetItemID(uidItem));
	if (!pItemDesc) return false;

	if (pItemDesc->m_nSlot == MMIST_NONE)
		return false;

	MMatchCharItemParts parts = RecommendEquipParts(pItemDesc->m_nSlot);
	if (parts == MMCIP_END)
		return false;

	WidgetHideDisableShow("Equip");
	WidgetHideDisableShow("SendAccountItemBtn");

	SetKindableItem( MMIST_NONE);

	return Equip(parts, uidItem);
}

bool ZShopEquipInterface::Equip(MMatchCharItemParts parts, MUID& uidItem)
{
	// Custom: Infected game mode avatar check
	if( ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_INFECTED )
	{
		if( parts == MMCIP_AVATAR )
			return false;
	}
	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_SNIPERMODE)
	{
		if (parts == MMCIP_AVATAR)
			return false;
	}
	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_VAMPIRE)
	{
		if (parts == MMCIP_AVATAR)
			return false;
	}
	if( ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_GUNGAME )
	{
		if( parts == MMCIP_CUSTOM1 || parts == MMCIP_CUSTOM2)
			return false;
	}

	// Custom: Equip mod check (cleaned + bug fixes)
	if (!ZGetGameClient()->GetMatchStageSetting()->IsUserExempt(ZGetMyInfo()->GetUGradeID())) // Staff Exception
	{
		auto* pSetting = ZGetGameClient()->GetMatchStageSetting();

		// Cache item desc once for the whole block (used by NI / WIWEAPON / *ONLY mods)
		ZMyItemNode* pItemNode = ZGetMyInfo()->GetItemList()->GetItem(uidItem);
		MMatchItemDesc* pItemDesc = pItemNode ? MGetMatchItemDescMgr()->GetItemDesc(pItemNode->GetItemID()) : NULL;

		// --- Slot-based restrictions ---

		// No Clothes: chặn HEAD..FEET + AVATAR
		if (pSetting->IsModifierUsed(MMOD_NC) &&
			((parts >= MMCIP_HEAD && parts <= MMCIP_FEET) || parts == MMCIP_AVATAR))
			return false;

		// No Guns: chặn PRIMARY + SECONDARY
		if (pSetting->IsModifierUsed(MMOD_NG) &&
			(parts == MMCIP_PRIMARY || parts == MMCIP_SECONDARY))
			return false;

		// --- CUSTOM1/2/3: phân loại theo loại item phụ (FIX Bug #1 + WIWEAPON activation) ---
		if (parts == MMCIP_CUSTOM1 || parts == MMCIP_CUSTOM2 || parts == MMCIP_CUSTOM3)
		{
			if (pItemDesc)
			{
				MMatchWeaponType wt = pItemDesc->m_nWeaponType.Ref();

				// Recovery items: medkit / repair / bullet kit / food / potion
				bool isRecovery =
					(wt == MWT_MED_KIT ||
						wt == MWT_REPAIR_KIT ||
						wt == MWT_BULLET_KIT ||
						wt == MWT_FOOD ||
						wt == MWT_POTION);

				// MMOD_WIWEAPON: cấm equip recovery items (pickup map vẫn cho)
				if (pSetting->IsModifierUsed(MMOD_WIWEAPON) && isRecovery)
					return false;

				// MMOD_NI: cấm offensive/utility items (mọi thứ KHÔNG phải recovery)
				if (pSetting->IsModifierUsed(MMOD_NI) && !isRecovery)
					return false;
			}
		}

		// --- Weapon-type restrictions for PRIMARY/SECONDARY/MELEE ---

		// Melee Only (FIX Bug #2): cấm súng (RANGE), cho phép melee/clothes/items
		if (pSetting->IsModifierUsed(MMOD_MELEEONLY))
		{
			if (!pItemDesc) return false;
			if (pItemDesc->m_nType.Ref() == MMIT_RANGE)
				return false;
		}

		// Sniper Only (gametype-based)
		if (pSetting->GetGameType() == MMATCH_GAMETYPE_SNIPERMODE)
		{
			if (!pItemDesc) return false;
			if (pItemDesc->m_nType.Ref() == MMIT_RANGE &&
				pItemDesc->m_nWeaponType.Ref() != MWT_SNIFER)
				return false;
		}

		// Shotgun Only: súng phải là shotgun, melee/clothes/items không bị ảnh hưởng
		if (pSetting->IsModifierUsed(MMOD_SHOTGUNONLY))
		{
			if (!pItemDesc) return false;
			if (pItemDesc->m_nType.Ref() == MMIT_RANGE &&
				pItemDesc->m_nWeaponType.Ref() != MWT_SHOTGUN &&
				pItemDesc->m_nWeaponType.Ref() != MWT_SAWED_SHOTGUN)
				return false;
		}

		// Pistol/Revolver Only ("Chỉ súng lục" - allow cả pistol lẫn revolver)
		if (pSetting->IsModifierUsed(MMOD_REVONLY))
		{
			if (!pItemDesc) return false;
			if (pItemDesc->m_nType.Ref() == MMIT_RANGE)
			{
				auto wt = pItemDesc->m_nWeaponType.Ref();
				if (wt != MWT_PISTOL && wt != MWT_PISTOLx2 &&
					wt != MWT_REVOLVER && wt != MWT_REVOLVERx2)
					return false;
			}
		}

		// Sniper Only (room mod, khác với gametype SNIPERMODE)
		if (pSetting->IsModifierUsed(MMOD_SNIPERONLY))
		{
			if (!pItemDesc) return false;
			if (pItemDesc->m_nType.Ref() == MMIT_RANGE &&
				pItemDesc->m_nWeaponType.Ref() != MWT_SNIFER)
				return false;
		}

		// MMOD_EXPLOSION: chặn equip toàn bộ vũ khí/item có tính nổ
		if (pSetting->IsModifierUsed(MMOD_EXPLOSION) && pItemDesc)
		{
			auto wt = pItemDesc->m_nWeaponType.Ref();
			bool isExplosive =
				(wt == MWT_ROCKET ||
					wt == MWT_ROCKETGUIDED ||
					wt == MWT_FRAGMENTATION ||
					wt == MWT_DYNAMITYE ||
					wt == MWT_FLASH_BANG ||
					wt == MWT_SMOKE_GRENADE ||
					wt == MWT_STUNGRENADE ||
					wt == MWT_LANDMINE ||
					wt == MWT_TRAP);
			if (isExplosive)
				return false;
		}
	}

	ZPostRequestEquipItem(ZGetGameClient()->GetPlayerUID(), uidItem, parts);
#ifndef UPDATE_STAGE_EQUIP_LOOK
	ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
#endif
	return true;
}

// Àåºñ ¾ÆÀÌÄÜ µå·¡±× ¾Ø µå·Ó »óÅÂÀÏ¶§ ÀåÂø °¡´ÉÇÑ ItemSlot ºä ¼³Á¤ÇÏ±â
void ZShopEquipInterface::SetKindableItem( MMatchItemSlotType nSlotType)
{
	ZItemSlotView* itemSlot;
	for (int i = 0; i < MMCIP_END; i++)
	{
		itemSlot = (ZItemSlotView*)GetIDLResource()->FindWidget( GetItemSlotName( "Equip", i));
		if ( itemSlot)
		{
			bool bKindable = IsKindableItem(itemSlot->GetParts(), nSlotType);
			itemSlot->SetKindable( bKindable);
		}
	}
}

bool ZShopEquipInterface::IsKindableItem(MMatchCharItemParts nParts, MMatchItemSlotType nSlotType)
{
	switch (nSlotType)
	{
	case MMIST_MELEE :
		if ( nParts == MMCIP_MELEE)
			return true;
		break;
	case MMIST_RANGE :
		if ( (nParts == MMCIP_PRIMARY) || (nParts == MMCIP_SECONDARY))
			return true;
		break;
	case MMIST_CUSTOM :
		if ( (nParts == MMCIP_CUSTOM1) || (nParts == MMCIP_CUSTOM2) || (nParts == MMCIP_CUSTOM3))
			return true;
		break;
	case MMIST_HEAD :
		if (nParts == MMCIP_HEAD)
			return true;
		break;
	case MMIST_CHEST :
		if (nParts == MMCIP_CHEST)
			return true;
		break;
	case MMIST_HANDS :
		if (nParts == MMCIP_HANDS)
			return true;
		break;
	case MMIST_LEGS :
		if (nParts == MMCIP_LEGS)
			return true;
		break;
	case MMIST_FEET :
		if (nParts == MMCIP_FEET)
			return true;
		break;
	case MMIST_FINGER :
		if ( (nParts == MMCIP_FINGERL) || (nParts == MMCIP_FINGERR))
			return true;
		break;
	case MMIST_AVATAR :
		if ( nParts == MMCIP_AVATAR)
			return true;
		break;
	case MMIST_COMMUNITY :
		if ( (nParts == MMCIP_COMMUNITY1) || (nParts == MMCIP_COMMUNITY2) )
			return true;
		break;
	case MMIST_LONGBUFF :
		if ( (nParts == MMCIP_LONGBUFF1) || (nParts == MMCIP_LONGBUFF2) )
			return true;
		break;
	}
	return false;
}

int ZShopEquipInterface::_CheckRestrictBringAccountItem()
{
	// Return -1 : Error
	//		  0 : No restriction
	//		  1 : Sex restricted
	//		  2 : Level restricted

	ZShopEquipListItem* pListItem = GetListCurSelItem("AccountItemList");
	if (pListItem == NULL) return -1;

	if (pListItem->GetItemData()->GetHandleBringAccount())
	{
		int nAIID = pListItem->GetItemData()->GetHandleBringAccount()->GetAIID();
		ZMyItemNode* pMyItemNode = ZGetMyInfo()->GetItemList()->GetAccountItemByAIID(nAIID);
		
		if (!pMyItemNode) return 0;

		unsigned long nItemID = pMyItemNode->GetItemID();
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
		if (!pItemDesc)
		{
			if( ZGetGambleItemDefineMgr().GetGambleItemDefine(nItemID) ) {
				return 0;	// °×ºí ¾ÆÀÌÅÛÀº Á¸Àç¸¸ È®ÀÎÇÏ¸é µÈ´Ù.
			} else {
				return -1;				
			}
		}

		if ((pItemDesc->m_nResSex.Ref() != -1) && (ZGetMyInfo()->GetSex() != pItemDesc->m_nResSex.Ref()))
			return 1;	// Sex restricted
		if (ZGetMyInfo()->GetLevel() < pItemDesc->m_nResLevel.Ref())
			return 2;	// Level restricted

	}

	return 0;	// No Restriction
}

bool ZShopEquipInterface::CheckRestrictBringAccountItem()
{
	int nResult = _CheckRestrictBringAccountItem();
	if (nResult == 0 || nResult == 2)	// Restriction Passed
		return true;

	if (nResult == -1)
		ZGetGameInterface()->ShowErrorMessage(  MERR_NO_SELITEM );

	else if (nResult == 1)	// Sex Restrict
		ZGetGameInterface()->ShowErrorMessage( MERR_BRING_ACCOUNTITEM_BECAUSEOF_SEX );

	else
		_ASSERT(FALSE);	// Unknown Restriction

	return false;
}

void ZShopEquipInterface::OnSendAccountButton()
{
	static unsigned long int st_LastRequestTime = 0;	///< ¸· ´©¸£´Â °Í ¹æÁö
	unsigned long int nNowTime = timeGetTime();
	if ((nNowTime - st_LastRequestTime) < 1000) return;

	st_LastRequestTime = nNowTime;

	ZShopEquipListItem* pListItem = GetListCurSelItem("EquipmentList");
	if (!pListItem)
	{
		ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NO_SELITEM ); 
		return;
	}

	if (pListItem->GetItemData()->GetHandleSendAccount())
		pListItem->GetItemData()->GetHandleSendAccount()->Send();

	SetKindableItem(MMIST_NONE);

	WidgetHideDisableShow("SendAccountItemBtn");
	WidgetHideDisableShow("Equip");
}

void ZShopEquipInterface::OnBringAccountButton()
{
	static unsigned long int st_LastRequestTime = 0;	///< ¸· ´©¸£´Â °Í ¹æÁö
	unsigned long int nNowTime = timeGetTime();
	if ((nNowTime - st_LastRequestTime) < 1000) return;

	if (ZGetMyInfo()->GetItemList()->GetItemCount() >= 500)
	{
		ZGetGameInterface()->ShowConfirmMessage("You have too many items in your inventory, please sell some items.");
		return;
	}

	st_LastRequestTime = nNowTime;

	ZShopEquipListItem* pListItem = GetListCurSelItem("AccountItemList");
	if (!pListItem)
	{
		ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NO_SELITEM ); 
		return;
	}

	if (pListItem->GetItemData()->GetHandleBringAccount())
		pListItem->GetItemData()->GetHandleBringAccount()->Bring();

	SetKindableItem(MMIST_NONE);

	WidgetHideDisableShow("BringAccountItemBtn");
}

//void ZShopEquipInterface::SelectShopTab(int nTabIndex)
//{
//	if (nTabIndex == -1)
//		nTabIndex = m_nShopTabNum;
//
//	ZIDLResource* pResource = GetIDLResource();
//
//	// ÇÁ¸®¹Ì¾ö ¼¥ - ¼³Á¤µÇ´Â ±¹°¡´ë·Î ÇÏ³ª¾¿ Áö¿ö³ª°£´Ù
///*#ifndef _DEBUG
//#if defined(LOCALE_BRAZIL) || defined(LOCALE_INDIA) || defined(LOCALE_US) || defined(LOCALE_JAPAN) || defined(LOCALE_KOREA) || defined(LOCALE_NHNUSA)
//	{
//		if ( nTabIndex == 2)
//			return;
//	}
//#endif
//#endif*/
//
//	MWidget* pWidget = pResource->FindWidget("AllEquipmentList");
//	if (pWidget != NULL) pWidget->Show(nTabIndex==0 ? true : false);
//	pWidget = pResource->FindWidget("MyAllEquipmentList");
//	if (pWidget != NULL) pWidget->Show(nTabIndex==1 ? true : false);
//	pWidget = pResource->FindWidget("CashEquipmentList");
//	if (pWidget != NULL) pWidget->Show(nTabIndex==2 ? true : false);
//	pWidget = pResource->FindWidget("EventEquipmentList");
//	if (pWidget != NULL) pWidget->Show(nTabIndex == 3 ? true : false);
//
//	// Set filter
//	MComboBox* pComboBox = (MComboBox*)pResource->FindWidget( "Shop_AllEquipmentFilter");
//	if(pComboBox) {
//		int sel = pComboBox->GetSelIndex();
//
//		ZMyItemList* pil = ZGetMyInfo()->GetItemList();
//		if ( pil) {
//			pil->m_ListFilter = sel;
//			pil->Serialize();
//		}
//	}
//
//	// Custom: Sub-Filter
//	if (nTabIndex == 2)
//	{
//		ZMyItemList* pil = ZGetMyInfo()->GetItemList();
//		if (pil)
//		{
//			pil->m_ListSubFilter = 0;
//
//			// Custom: Sub-Filter
//			MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentSubFilter");
//			if ( pComboBox)
//			{
//				pComboBox->RemoveAll();
//
//				if( ZGetShop()->m_ListFilter == 0 )
//					pComboBox->Add("Select Filter");
//				else
//					pComboBox->Add("All");
//
//				pComboBox->SetSelIndex( 0 );
//				pil->m_ListSubFilter = 0;
//
//				// Filter: 0 -> Show All Equipment.
//
//				int nListFilter = ZGetShop()->m_ListFilter;
//
//				if( nListFilter == 8 ) // Melee weapons
//				{
//					pComboBox->Add("Daggers");
//					pComboBox->Add("Sword / Katana");
//					pComboBox->Add("Kodachi");
//					pComboBox->Add("Large Swords");
//				}
//				else if( nListFilter == 9 ) // Ranged weapons
//				{
//					pComboBox->Add("Shotguns");
//					pComboBox->Add("SMGs");
//					pComboBox->Add("Rifles / MGs");
//					pComboBox->Add("Pistols");
//					pComboBox->Add("Revolvers");
//					pComboBox->Add("Rocket Launchers");
//					pComboBox->Add("Snipers");
//				}
//				else if( nListFilter == 10 ) // Kits or Pills
//				{
//					pComboBox->Add("Kits / Food");
//					pComboBox->Add("Pills");
//					pComboBox->Add("Grenades");
//				}
//			}
//		}
//	}
//
//	// ¹öÆ° ¼³Á¤
//	MButton* pButton = (MButton*)pResource->FindWidget( "BuyConfirmCaller");
//	if ( pButton)
//	{
//		pButton->Show( false);
//		pButton->Enable( false);
//	}
//	pButton = (MButton*)pResource->FindWidget( "SellConfirmCaller");
//	if ( pButton)
//	{
//		pButton->Show( false);
//		pButton->Enable( false);
//	}
//	pButton = (MButton*)pResource->FindWidget( "BuyCashConfirmCaller");
//	if ( pButton)
//	{
//		pButton->Show( false);
//		pButton->Enable( false);
//	}
//
//	if ( nTabIndex == 0)
//	{
//		pButton = (MButton*)pResource->FindWidget( "BuyConfirmCaller");
//		if ( pButton) 
//			pButton->Show( true);
//	}
//	else if ( nTabIndex == 1)
//	{
//		pButton = (MButton*)pResource->FindWidget( "SellConfirmCaller");
//		if ( pButton)
//			pButton->Show( true);
//	}
//	else if ( nTabIndex == 2)
//	{
//		pButton = (MButton*)pResource->FindWidget( "BuyCashConfirmCaller");
//		if ( pButton)
//			pButton->Show( true);
//	}
//
//	pButton = (MButton*)pResource->FindWidget("AllEquipmentListCaller");
//	if (pButton != NULL)
//		pButton->Show(nTabIndex!=0 ? true : false);
//	pButton = (MButton*)pResource->FindWidget("MyAllEquipmentListCaller");
//	if (pButton != NULL)
//		pButton->Show(nTabIndex!=1 ? true : false);
//	pButton = (MButton*)pResource->FindWidget("CashEquipmentListCaller");
//	if (pButton != NULL)
//		pButton->Show(nTabIndex!=2 ? true : false);
//
//
//	// ±¸ÀÔ, ÆÇ¸Å ¶óº§
//	MPicture* pPicture;
//	MBitmap* pBitmap;
//	pPicture = (MPicture*)pResource->FindWidget("Shop_FrameTabLabel1");
//	if ( pPicture)
//		pPicture->Show(nTabIndex==0 ? true : false);
//	pPicture = (MPicture*)pResource->FindWidget("Shop_FrameTabLabel2");
//	if ( pPicture)
//		pPicture->Show(nTabIndex==1 ? true : false);
//	pPicture = (MPicture*)pResource->FindWidget("Shop_FrameTabLabel3");
//	if ( pPicture)
//		pPicture->Show(nTabIndex==2 ? true : false);
//
//
//	// ÇÁ·¹ÀÓ ÅÇ
//	pPicture = (MPicture*)pResource->FindWidget("Shop_TabLabel");
//	if ( pPicture)
//	{
//		if ( nTabIndex == 0)
//			pBitmap = MBitmapManager::Get( "framepaneltab1.tga");
//		else if ( nTabIndex == 1)
//			pBitmap = MBitmapManager::Get( "framepaneltab2.tga");
//		else if ( nTabIndex == 2)
//			pBitmap = MBitmapManager::Get( "framepaneltab3.tga");
//
//		if ( pBitmap)
//			pPicture->SetBitmap( pBitmap);
//	}
//
//
///// ÇÁ¸®¹Ì¾ö ¼¥ - ¼³Á¤µÇ´Â ±¹°¡´ë·Î ÇÏ³ª¾¿ Áö¿ö³ª°£´Ù
//#ifndef _DEBUG
//#if defined(LOCALE_BRAZIL) || defined(LOCALE_INDIA) || defined(LOCALE_US) || defined(LOCALE_JAPAN) || defined(LOCALE_KOREA) || defined(LOCALE_NHNUSA)
//	if (ZGetShop()->GetShopType() == ZSHOP_LADDER)
//	{
//		pWidget = pResource->FindWidget( "Shop_TabLabelBg");
//		if ( pWidget)  pWidget->Show( false);
//
//		pWidget = pResource->FindWidget( "CashEquipmentListCaller");
//		if ( pWidget)  pWidget->Show( false);
//
//		pWidget = pResource->FindWidget( "Shop_FrameTabLabel3");
//		if ( pWidget)  pWidget->Show( false);
//
//		pWidget = pResource->FindWidget("Shop_FrameTabLabel4");
//		if (pWidget)  pWidget->Show(false);
//
//	}
//	else if (ZGetShop()->GetShopType() == ZSHOP_EVENT)
//	{
//		pWidget = pResource->FindWidget("Shop_TabLabelBg");
//		if (pWidget)  pWidget->Show(false);
//
//		pWidget = pResource->FindWidget("CashEquipmentListCaller");
//		if (pWidget)  pWidget->Show(false);
//
//		pWidget = pResource->FindWidget("Shop_FrameTabLabel3");
//		if (pWidget)  pWidget->Show(false);
//
//		pWidget = pResource->FindWidget("Shop_FrameTabLabel4");
//		if (pWidget)  pWidget->Show(false);
//
//	}
//	else
//	{
//		pWidget = pResource->FindWidget("Shop_TabLabelBg");
//		if (pWidget)  pWidget->Show(true);
//
//		pWidget = pResource->FindWidget("CashEquipmentListCaller");
//		if (pWidget)  pWidget->Show(true);
//
//
//	}
//#endif
//#endif
//
//	m_nShopTabNum = nTabIndex;
//
//	DrawCharInfoText();
//}
//
//
//void ZShopEquipInterface::SelectEquipmentTab(int nTabIndex)
//{
//	if (nTabIndex == -1)
//		nTabIndex = m_nEquipTabNum;
//
//	ZIDLResource* pResource = GetIDLResource();
//
//	SetKindableItem( MMIST_NONE);
//
//	// Set filter
//	MComboBox* pComboBox = (MComboBox*)pResource->FindWidget( "Equip_AllEquipmentFilter");
//	if(pComboBox) {
//		int sel = pComboBox->GetSelIndex();
//
//		ZMyItemList* pil = ZGetMyInfo()->GetItemList();
//		if ( pil) {
//			pil->m_ListFilter = sel;
//			pil->Serialize();
//		}
//	}
//
//	// EQUIPMENTLISTBOX
//	MWidget* pWidget = pResource->FindWidget("EquipmentList");
//	if (pWidget != NULL) pWidget->Show(nTabIndex==0 ? true:false);
//	pWidget = pResource->FindWidget("AccountItemList");
//	if (pWidget != NULL) pWidget->Show(nTabIndex==0 ? false:true);
//
//	// ÅÇ ¹öÆ°
//	MButton* pButton = (MButton*)pResource->FindWidget( "Equip");
//	if ( pButton)
//	{
//		pButton->Show( false);
//		pButton->Enable( false);
//	}
//
//	pButton = (MButton*)pResource->FindWidget( "SendAccountItemBtn");
//	if ( pButton)
//	{
//		pButton->Show( false);
//		pButton->Enable( false);
//	}
//
//	pButton = (MButton*)pResource->FindWidget( "BringAccountItemBtn");
//	if ( pButton)
//	{
//		pButton->Show( false);
//		pButton->Enable( false);
//	}
//
//	pButton = (MButton*)pResource->FindWidget( "BringAccountSpendableItemConfirmOpen");
//	if ( pButton)
//	{
//		pButton->Show( false);
//		pButton->Enable( false);
//	}
//
//	if ( nTabIndex == 0)
//	{
//		pButton = (MButton*)pResource->FindWidget( "Equip");
//		if (pButton) pButton->Show(true);
//
//		pButton = (MButton*)pResource->FindWidget( "SendAccountItemBtn");
//		if (pButton) pButton->Show(true);
//	}
//	else if ( nTabIndex == 1)
//	{
//		pButton = (MButton*)pResource->FindWidget( "BringAccountItemBtn");
//		if ( pButton) pButton->Show( true);
//
//		pButton = (MButton*)pResource->FindWidget( "BringAccountSpendableItemConfirmOpen");
//		if ( pButton) pButton->Show( false);
//	}
//
//	pButton = (MButton*)pResource->FindWidget("Equipment_CharacterTab");
//	if (pButton)
//		pButton->Show( nTabIndex==0 ? false : true);
//	pButton = (MButton*)pResource->FindWidget("Equipment_AccountTab");
//	if (pButton)
//		pButton->Show( nTabIndex==1 ? false : true);
//
//
//	// ÅÇ ¶óº§
//	MLabel* pLabel;
//	pLabel = (MLabel*)pResource->FindWidget("Equipment_FrameTabLabel1");
//	if ( pLabel)
//		pLabel->Show( nTabIndex==0 ? true : false);
//	pLabel = (MLabel*)pResource->FindWidget("Equipment_FrameTabLabel2");
//	if ( pLabel)
//		pLabel->Show( nTabIndex==1 ? true : false);
//
//	// ÅÇ ¸®½ºÆ®
//	MPicture* pPicture;
//	pPicture = (MPicture*)pResource->FindWidget("Equip_ListLabel1");
//	if ( pPicture)
//		pPicture->Show( nTabIndex==0 ? true : false);
//	pPicture = (MPicture*)pResource->FindWidget("Equip_ListLabel2");
//	if ( pPicture)
//		pPicture->Show( nTabIndex==1 ? true : false);
//
//
//	// ÇÁ·¹ÀÓ ÅÇ
//	pPicture = (MPicture*)pResource->FindWidget("Equip_TabLabel");
//	MBitmap* pBitmap;
//	if ( pPicture)
//	{
//		if ( nTabIndex == 0)
//			pBitmap = MBitmapManager::Get( "framepaneltab1.tga");
//		else
//			pBitmap = MBitmapManager::Get( "framepaneltab2.tga");
//
//		if ( pBitmap)
//			pPicture->SetBitmap( pBitmap);
//	}
//
//	// Áß¾ÓÀºÇà
//	if (nTabIndex == 1)
//	{
//		ZGetMyInfo()->GetItemList()->ClearAccountItems();
//		ZGetMyInfo()->GetItemList()->SerializeAccountItem();
//	}
//
//	for (int i = 0; i < MMCIP_END; i++)
//	{
//		ZItemSlotView* pItemSlot = (ZItemSlotView*)GetIDLResource()->FindWidget( GetItemSlotName( "Equip", i));
//		if( pItemSlot ) pItemSlot->EnableDragAndDrop( nTabIndex==0 ? true : false);
//	}
//
//	m_nEquipTabNum = nTabIndex;
//
//	DrawCharInfoText();
//}
void ZShopEquipInterface::SelectShopTab(int nTabIndex)
{
	if (nTabIndex == -1) nTabIndex = m_nShopTabNum;

	ZIDLResource* pResource = GetIDLResource();
	if (!pResource) return;

	// 1. Show/Hide 5 ListBox tương ứng
	MWidget* pWidget = NULL;
	if (pWidget = pResource->FindWidget("AllEquipmentList"))   pWidget->Show(nTabIndex == 0);
	if (pWidget = pResource->FindWidget("MyAllEquipmentList"))  pWidget->Show(nTabIndex == 1);
	if (pWidget = pResource->FindWidget("CashEquipmentList"))  pWidget->Show(nTabIndex == 2);
	if (pWidget = pResource->FindWidget("MedalEquipmentList")) pWidget->Show(nTabIndex == 3);
	if (pWidget = pResource->FindWidget("EventEquipmentList")) pWidget->Show(nTabIndex == 4);

	SelectEquipmentFrameList("Shop", true);
	InitPackages();

	// 2. Show/Hide Nút Mua Confirm tương ứng từng loại tiền
	MButton* pBtnBuy = (MButton*)pResource->FindWidget("BuyConfirmCaller");
	MButton* pBtnSell = (MButton*)pResource->FindWidget("SellConfirmCaller");
	MButton* pBtnCash = (MButton*)pResource->FindWidget("BuyCashConfirmCaller");
	MButton* pBtnMedal = (MButton*)pResource->FindWidget("BuyMedalConfirmCaller");
	MButton* pBtnEvent = (MButton*)pResource->FindWidget("BuyEventConfirmCaller");

	if (pBtnBuy) { pBtnBuy->Show(nTabIndex == 0);   pBtnBuy->Enable(nTabIndex == 0); }
	if (pBtnSell) { pBtnSell->Show(nTabIndex == 1);  pBtnSell->Enable(nTabIndex == 1); }
	if (pBtnCash) { pBtnCash->Show(nTabIndex == 2);  pBtnCash->Enable(nTabIndex == 2); }
	if (pBtnMedal) { pBtnMedal->Show(nTabIndex == 3); pBtnMedal->Enable(nTabIndex == 3); }
	if (pBtnEvent) { pBtnEvent->Show(nTabIndex == 4); pBtnEvent->Enable(nTabIndex == 4); }

	//// 3. Show/Hide các Nút Chuyển Tab (ẩn nút của Tab đang đứng)
	//MButton* pCaller0 = (MButton*)pResource->FindWidget("AllEquipmentListCaller");
	//MButton* pCaller1 = (MButton*)pResource->FindWidget("MyAllEquipmentListCaller");
	//MButton* pCaller2 = (MButton*)pResource->FindWidget("CashEquipmentListCaller");
	//MButton* pCaller3 = (MButton*)pResource->FindWidget("MedalEquipmentListCaller");
	//MButton* pCaller4 = (MButton*)pResource->FindWidget("EventEquipmentListCaller");

	//if (pCaller0) pCaller0->Show(nTabIndex != 0);
	//if (pCaller1) pCaller1->Show(nTabIndex != 1);
	//if (pCaller2) pCaller2->Show(nTabIndex != 2);
	//if (pCaller3) pCaller3->Show(nTabIndex != 3);
	//if (pCaller4) pCaller4->Show(nTabIndex != 4);

	// 4. Label Header Tab (Shop_FrameTabLabel1 -> Shop_FrameTabLabel5)
	for (int i = 1; i <= 5; ++i)
	{
		char szLabelName[32];
		sprintf_s(szLabelName, sizeof(szLabelName), "Shop_FrameTabLabel%d", i);
		MPicture* pPicture = (MPicture*)pResource->FindWidget(szLabelName);
		if (pPicture)
			pPicture->Show(nTabIndex == (i - 1));
	}

	// 5. Tab Background Bitmap (framepaneltab1.tga -> framepaneltab5.tga)
	MPicture* pTabBgPic = (MPicture*)pResource->FindWidget("Shop_TabLabel");
	if (pTabBgPic)
	{
		char szTabBgBuf[32];
		sprintf_s(szTabBgBuf, sizeof(szTabBgBuf), "framepaneltab%d.tga", nTabIndex + 1);
		MBitmap* pBitmap = MBitmapManager::Get(szTabBgBuf);
		if (pBitmap)
			pTabBgPic->SetBitmap(pBitmap);
	}

	m_nShopTabNum = nTabIndex;
	DrawCharInfoText();
}

void ZShopEquipInterface::SelectEquipmentTab(int nTabIndex)
{
	if (nTabIndex == -1)
		nTabIndex = m_nEquipTabNum;

	ZIDLResource* pResource = GetIDLResource();
	SetKindableItem(MMIST_NONE);

	// Custom UI Filter: Nạp lại danh sách đồ trong Hành trang theo Filter mới
	SelectEquipmentFrameList("Equip", true);

	// EQUIPMENTLISTBOX
	MWidget* pWidget = pResource->FindWidget("EquipmentList");
	if (pWidget != NULL) pWidget->Show(nTabIndex == 0 ? true : false);
	pWidget = pResource->FindWidget("AccountItemList");
	if (pWidget != NULL) pWidget->Show(nTabIndex == 0 ? false : true);

	// Tab Button
	MButton* pButton = (MButton*)pResource->FindWidget("Equip");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}

	pButton = (MButton*)pResource->FindWidget("SendAccountItemBtn");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}

	pButton = (MButton*)pResource->FindWidget("BringAccountItemBtn");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}

	pButton = (MButton*)pResource->FindWidget("BringAccountSpendableItemConfirmOpen");
	if (pButton)
	{
		pButton->Show(false);
		pButton->Enable(false);
	}

	if (nTabIndex == 0)
	{
		pButton = (MButton*)pResource->FindWidget("Equip");
		if (pButton) pButton->Show(true);

		pButton = (MButton*)pResource->FindWidget("SendAccountItemBtn");
		if (pButton) pButton->Show(true);
	}
	else if (nTabIndex == 1)
	{
		pButton = (MButton*)pResource->FindWidget("BringAccountItemBtn");
		if (pButton) pButton->Show(true);

		pButton = (MButton*)pResource->FindWidget("BringAccountSpendableItemConfirmOpen");
		if (pButton) pButton->Show(false);
	}

	pButton = (MButton*)pResource->FindWidget("Equipment_CharacterTab");
	if (pButton)
		pButton->Show(nTabIndex == 0 ? false : true);
	pButton = (MButton*)pResource->FindWidget("Equipment_AccountTab");
	if (pButton)
		pButton->Show(nTabIndex == 1 ? false : true);

	// Tab Label
	MLabel* pLabel;
	pLabel = (MLabel*)pResource->FindWidget("Equipment_FrameTabLabel1");
	if (pLabel)
		pLabel->Show(nTabIndex == 0 ? true : false);
	pLabel = (MLabel*)pResource->FindWidget("Equipment_FrameTabLabel2");
	if (pLabel)
		pLabel->Show(nTabIndex == 1 ? true : false);

	// Tab List Image
	MPicture* pPicture;
	pPicture = (MPicture*)pResource->FindWidget("Equip_ListLabel1");
	if (pPicture)
		pPicture->Show(nTabIndex == 0 ? true : false);
	pPicture = (MPicture*)pResource->FindWidget("Equip_ListLabel2");
	if (pPicture)
		pPicture->Show(nTabIndex == 1 ? true : false);

	// Tab Label Background
	pPicture = (MPicture*)pResource->FindWidget("Equip_TabLabel");
	MBitmap* pBitmap;
	if (pPicture)
	{
		if (nTabIndex == 0)
			pBitmap = MBitmapManager::Get("framepaneltab1.tga");
		else
			pBitmap = MBitmapManager::Get("framepaneltab2.tga");

		if (pBitmap)
			pPicture->SetBitmap(pBitmap);
	}

	// Đồ trong Rương tài khoản (Account Item)
	if (nTabIndex == 1)
	{
		ZGetMyInfo()->GetItemList()->ClearAccountItems();
		ZGetMyInfo()->GetItemList()->SerializeAccountItem();
	}

	for (int i = 0; i < MMCIP_END; i++)
	{
		ZItemSlotView* pItemSlot = (ZItemSlotView*)GetIDLResource()->FindWidget(GetItemSlotName("Equip", i));
		if (pItemSlot) pItemSlot->EnableDragAndDrop(nTabIndex == 0 ? true : false);
	}

	m_nEquipTabNum = nTabIndex;
	DrawCharInfoText();
}

//void ZShopEquipInterface::SelectEquipmentFrameList( const char* szName, bool bOpen)
//{
//	if (szName == NULL)
//	{
//		SelectEquipmentFrameList("Shop", bOpen);
//		SelectEquipmentFrameList("Equip", bOpen);
//		return;
//	}
//
//	char szTemp[256];
//
//	ZIDLResource* pResource = GetIDLResource();
//
//
//	// Frame open/close background image
//	MPicture* pPicture;
//	strcpy( szTemp, szName);
//	strcat( szTemp, "_ArmorBGListFrameOpen");
//	pPicture = (MPicture*)pResource->FindWidget( szTemp);
//	if(pPicture != NULL) {
//		if( bOpen && GetArmorWeaponTabIndex() == 0 )  { pPicture->Show(true); } 
//		else										{ pPicture->Show(false);}		
//	}
//
//	strcpy( szTemp, szName);
//	strcat( szTemp, "_ArmorBGListFrameClose");
//	pPicture = (MPicture*)pResource->FindWidget( szTemp);
//	if(pPicture != NULL) {
//		if( !bOpen && GetArmorWeaponTabIndex() == 0 ) { pPicture->Show(true); } 
//		else										{ pPicture->Show(false);}		
//	}
//
//
//	// Frame open/close background image
//	strcpy( szTemp, szName);
//	strcat( szTemp, "_WeaponBGListFrameOpen");
//	pPicture = (MPicture*)pResource->FindWidget( szTemp);
//	if(pPicture != NULL) {
//		if( bOpen && GetArmorWeaponTabIndex() == 1 )  { pPicture->Show(true); } 
//		else										{ pPicture->Show(false);}		
//	}
//
//	strcpy( szTemp, szName);
//	strcat( szTemp, "_WeaponBGListFrameClose");
//	pPicture = (MPicture*)pResource->FindWidget( szTemp);
//	if(pPicture != NULL) {
//		if( !bOpen && GetArmorWeaponTabIndex() == 1 )  { pPicture->Show(true); } 
//		else										 { pPicture->Show(false);}		
//	}
//
//		strcpy( szTemp, szName);
//	strcat( szTemp, "_MiscBGListFrameOpen");
//	pPicture = (MPicture*)pResource->FindWidget( szTemp);
//	if(pPicture != NULL) {
//		if( bOpen && GetArmorWeaponTabIndex() == 2 )  { pPicture->Show(true); } 
//		else										{ pPicture->Show(false);}		
//	}
//
//	strcpy( szTemp, szName);
//	strcat( szTemp, "_MiscBGListFrameClose");
//	pPicture = (MPicture*)pResource->FindWidget( szTemp);
//	if(pPicture != NULL) {
//		if( !bOpen && GetArmorWeaponTabIndex() == 2 )  { pPicture->Show(true); } 
//		else										 { pPicture->Show(false);}		
//	}
//
//
//	// Frame open/close image
//	MButton* pButton;
//	strcpy( szTemp, szName);
//	strcat( szTemp, "_EquipListFrameCloseButton");
//	pButton = (MButton*)pResource->FindWidget( szTemp);
//	if ( pButton != NULL) pButton->Show( bOpen);
//
//	strcpy( szTemp, szName);
//	strcat( szTemp, "_EquipListFrameOpenButton");
//	pButton = (MButton*)pResource->FindWidget( szTemp);
//	if ( pButton != NULL) pButton->Show( !bOpen);
//
//	// Resize item slot
//	char szWidgetName[256];
//	sprintf(szWidgetName, "%s_EquipmentSlot_Head", szName);
//	MWidget* itemSlot = (MWidget*)pResource->FindWidget(szWidgetName);
//	if (itemSlot) {
//		MRECT rect = itemSlot->GetRect();
//
//		int nWidth;
//		if (bOpen)
//		{
//			nWidth = 220.0f * (float)RGetScreenWidth() / 800.0f;
//		}
//		else
//		{
//			// === FIX: Không dùng min(rect.w, rect.h) nữa ===
//			// Lấy trực tiếp chiều rộng gốc đặt trong XML (hoặc scale theo độ phân giải)
//			nWidth = rect.w;
//		}
//
//		for (int i = 0; i < MMCIP_END; i++) {
//			itemSlot = (MWidget*)pResource->FindWidget(GetItemSlotName(szName, i));
//
//			if (itemSlot) {
//				if (GetArmorWeaponTabIndex() == GetArmorWeaponTabIndexContainItemParts((MMatchCharItemParts)i)) {
//					rect = itemSlot->GetRect();
//
//					// Cập nhật lại Bounds chuẩn theo width gốc trong XML
//					itemSlot->SetBounds(rect.x, rect.y, rect.w, rect.h); // Thay nWidth bằng rect.w
//					itemSlot->Show(true);
//				}
//				else {
//					itemSlot->Show(false);
//				}
//			}
//		}
//	}
//
//	// »óÁ¡°ú ÀåºñÃ¢ÀÇ ÅÇ ¹öÆ° ´­¸² »óÅÂ¸¦ µ¿±âÈ­ÇØÁÖÀÚ -_-;
//	MBmButton* pTabBtn;
//	if (GetArmorWeaponTabIndex() == 0)
//	{
//		pTabBtn = (MBmButton*)pResource->FindWidget("Shop_ArmorEquipmentTab");
//		if (pTabBtn) pTabBtn->SetCheck(true);
//		pTabBtn = (MBmButton*)pResource->FindWidget("Equip_ArmorEquipmentTab");
//		if (pTabBtn) pTabBtn->SetCheck(true);
//	}
//	else if (GetArmorWeaponTabIndex() == 1)
//	{
//		pTabBtn = (MBmButton*)pResource->FindWidget("Shop_WeaponEquipmentTab");
//		if (pTabBtn) pTabBtn->SetCheck(true);
//		pTabBtn = (MBmButton*)pResource->FindWidget("Equip_WeaponEquipmentTab");
//		if (pTabBtn) pTabBtn->SetCheck(true);
//	}
//	else if (GetArmorWeaponTabIndex() == 2)
//	{
//		pTabBtn = (MBmButton*)pResource->FindWidget("Shop_MiscEquipmentTab");
//		if (pTabBtn) pTabBtn->SetCheck(true);
//		pTabBtn = (MBmButton*)pResource->FindWidget("Equip_MiscEquipmentTab");
//		if (pTabBtn) pTabBtn->SetCheck(true);
//	}
//}
void ZShopEquipInterface::SelectEquipmentFrameList(const char* szName, bool bOpen)
{
	if (szName == NULL)
	{
		SelectEquipmentFrameList("Shop", bOpen);
		SelectEquipmentFrameList("Equip", bOpen);
		return;
	}

	ZIDLResource* pResource = GetIDLResource();

	// 1. HIỂN THỊ CÁC Ô ITEM THEO TAB ĐANG CHỌN (BỎ HÀM CO CO-GIÃN DÒNG)
	for (int i = 0; i < MMCIP_END; i++)
	{
		MWidget* itemSlot = (MWidget*)pResource->FindWidget(GetItemSlotName(szName, i));

		if (itemSlot)
		{
			// Kiểm tra nếu Slot thuộc Tab hiện tại thì HIỆN, khác Tab thì ẨN
			if (GetArmorWeaponTabIndex() == GetArmorWeaponTabIndexContainItemParts((MMatchCharItemParts)i))
			{
				itemSlot->Show(true);
			}
			else
			{
				itemSlot->Show(false);
			}
		}
	}

	// 2. ĐỒNG BỘ TRẠNG THÁI ACTIVE (CHECK) CHO 3 NÚT TAB (WEAPON, ARMOR, MISC)
	int nCurTab = GetArmorWeaponTabIndex();

	MBmButton* pTabWeapon = (MBmButton*)pResource->FindWidget(szName == string("Shop") ? "Shop_ArmorEquipmentTab" : "Equip_ArmorEquipmentTab");
	MBmButton* pTabArmor = (MBmButton*)pResource->FindWidget(szName == string("Shop") ? "Shop_WeaponEquipmentTab" : "Equip_WeaponEquipmentTab");
	MBmButton* pTabMisc = (MBmButton*)pResource->FindWidget(szName == string("Shop") ? "Shop_MiscEquipmentTab" : "Equip_MiscEquipmentTab");

	if (pTabWeapon) { pTabWeapon->Show(true); pTabWeapon->SetCheck(nCurTab == 0); }
	if (pTabArmor) { pTabArmor->Show(true);  pTabArmor->SetCheck(nCurTab == 1); }
	if (pTabMisc) { pTabMisc->Show(true);   pTabMisc->SetCheck(nCurTab == 2); }
}

// ÀÎÀÚ·Î ÁÖ¾îÁø ¾ÆÀÌÅÛÆÄÃ÷¸¦ ´ã´Â ¾ÆÀÌÅÛ½½·ÔÀÌ ÀÖ´Â ÅÇ ÀÎµ¦½º¸¦ ¾Ë·ÁÁØ´Ù
//int ZShopEquipInterface::GetArmorWeaponTabIndexContainItemParts(MMatchCharItemParts parts)
//{
//	// Custom: Misc tab (cosmetic items: dash / jjang / stun / emote)
//	if (parts == MMCIP_DASH || parts == MMCIP_JJANG
//		|| parts == MMCIP_STUN || parts == MMCIP_EMOTE)
//		return 2;
//
//	// ¹æ¾î±¸ÅÇ
//	if (parts < MMCIP_MELEE || parts == MMCIP_AVATAR)
//		return 0;
//	// ¹«±âÅÇ
//	if (parts >= MMCIP_MELEE && parts < MMCIP_LONGBUFF2 + 1 && parts != MMCIP_AVATAR)
//		return 1;
//
//	//	_ASSERT(0);
//	return 0;
//}
int ZShopEquipInterface::GetArmorWeaponTabIndexContainItemParts(MMatchCharItemParts parts)
{
	// 0. LOẠI BỎ HOÀN TOÀN CÁC SLOT KHÔNG DÙNG (Không thuộc Tab nào)
	if (parts == MMCIP_CUSTOM3 || parts == MMCIP_EMOTE ||
		parts == MMCIP_COMMUNITY1 || parts == MMCIP_COMMUNITY2 ||
		parts == MMCIP_LONGBUFF1 || parts == MMCIP_LONGBUFF2)
	{
		return -1;
	}

	// 1. TAB 2 (MISC / COSMETIC): Dash, Jjang, Stun, Avatar
	if (parts == MMCIP_DASH || parts == MMCIP_JJANG ||
		parts == MMCIP_STUN || parts == MMCIP_AVATAR)
	{
		return 2;
	}

	// 2. TAB 1 (ARMOR): 5 món Mũ, Áo, Tay, Quần, Chân
	if (parts == MMCIP_HEAD || parts == MMCIP_CHEST ||
		parts == MMCIP_HANDS || parts == MMCIP_LEGS || parts == MMCIP_FEET)
	{
		return 1;
	}

	// 3. TAB 0 (WEAPON): Chỉ chứa Vũ khí (Melee, Primary, Secondary, Custom 1-2) + 2 Nhẫn
	if (parts == MMCIP_MELEE || parts == MMCIP_PRIMARY || parts == MMCIP_SECONDARY ||
		parts == MMCIP_CUSTOM1 || parts == MMCIP_CUSTOM2 ||
		parts == MMCIP_FINGERL || parts == MMCIP_FINGERR)
	{
		return 0;
	}

	return -1;
}

bool ZShopEquipInterface::IsEquipmentFrameListOpened( const char* szName)
{
	ZIDLResource* pResource = GetIDLResource();

	char szTemp[256];
	MPicture* pPicture;
	strcpy( szTemp, szName);
	if (GetArmorWeaponTabIndex() == 0)
	{
		strcat( szTemp, "_ArmorBGListFrameOpen");
		pPicture = (MPicture*)pResource->FindWidget( szTemp);
		if (pPicture) 
			return pPicture->IsVisible();
	}
	else if (GetArmorWeaponTabIndex() == 1)
	{
		strcat( szTemp, "_WeaponBGListFrameOpen");
		pPicture = (MPicture*)pResource->FindWidget( szTemp);
		if (pPicture) 
			return pPicture->IsVisible();
	}
	else if (GetArmorWeaponTabIndex() == 2)
	{
		strcat(szTemp, "_MiscBGListFrameOpen");
		pPicture = (MPicture*)pResource->FindWidget(szTemp);
		if (pPicture)
			return pPicture->IsVisible();
	}
	return false;
}

// ÁÖ¾îÁø ½½·ÔÅ¸ÀÔÀ» ´ã°í ÀÖ´Â ÅÇÀ» ¼±ÅÃÇØÁØ´Ù - ¸®½ºÆ®¿¡¼­ ¾î¶² ¾ÆÀÌÅÛÀ» ¼±ÅÃÇÑ °æ¿ì ±× ¾ÆÀÌÅÛ¿¡ ¸Â´Â ¹æ¾î±¸/¹«±âÅÇÀ» ÀÚµ¿º¯°æÇØÁÖ´Â ±â´ÉÀ» À§ÇØ
void ZShopEquipInterface::SelectArmorWeaponTabWithSlotType(MMatchItemSlotType nSlotType)
{
	ZItemSlotView* pItemSlot;
	for (int i = 0; i < MMCIP_END; i++)
	{
		pItemSlot = (ZItemSlotView*)GetIDLResource()->FindWidget( GetItemSlotName( "Equip", i));
		if ( pItemSlot)
		{
			if (IsKindableItem(pItemSlot->GetParts(), nSlotType))
			{
				int tabIndex = GetArmorWeaponTabIndexContainItemParts(pItemSlot->GetParts());
				
				//bool bOpened = ZGetGameInterface()->GetShopEquipInterface()->IsEquipmentFrameListOpened();
				bool bOpened = false;
				ZGetGameInterface()->GetShopEquipInterface()->SetArmorWeaponTabIndex(tabIndex);
				ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentFrameList(NULL, bOpened);

				return;
			}
		}
	}
}

void ZShopEquipInterface::OnArmorWeaponTabButtonClicked(int nTab)
{
	//bool bOpened = IsEquipmentFrameListOpened();
	bool bOpened = false;
	SetArmorWeaponTabIndex(nTab);
	SelectEquipmentFrameList(NULL, bOpened);
}

void ZShopEquipInterface::ShowItemDescription(bool bShow, MTextArea* pTextArea, void* pCaller)
{
	if (!pTextArea) return;

	if (bShow)
	{
		pTextArea->Show(true);
		m_pItemDescriptionClient = pCaller;
	}
	else if (!bShow && pCaller==m_pItemDescriptionClient)
	{
		// ÅøÆÁÀ» ²ô·Á°í ÇÒ¶§´Â ÀÚ½ÅÀÌ Ä×¾ú¾î¾ß ÇÑ´Ù. µµÁß¿¡ ´Ù¸¥ °÷¿¡¼­ ÅøÆÁÀ» Á¡À¯Çß´Ù¸é ²ø ±ÇÇÑÀÌ ¾øÀ½
		pTextArea->Show(false);
	}
}

void ZShopEquipInterface::DrawCharInfoText()
{
	// ÇöÀç ³» »óÅÂ·Î ÃÊ±âÈ­
	DrawCharInfoText(NULL, 0,
		ZGetMyInfo()->GetItemList()->GetEquipedTotalWeight(),
		ZGetMyInfo()->GetItemList()->GetMaxWeight(),
		ZGetMyInfo()->GetHP(),
		ZGetMyInfo()->GetAP(),
		0, 0);
}

//void ZShopEquipInterface::DrawCharInfoText(char* szShopOrEquip, int nReqLevel, int nNewWT, int nNewMaxWT, int nNewHP, int nNewAP, int nReqBounty, int nReqCash)
//{
//	if (szShopOrEquip == NULL) 
//	{
//		DrawCharInfoText("Shop", nReqLevel, nNewWT, nNewMaxWT, nNewHP, nNewAP, nReqBounty, nReqCash);
//		DrawCharInfoText("Equip", nReqLevel, nNewWT, nNewMaxWT, nNewHP, nNewAP, nReqBounty, nReqCash);
//		return;
//	}
//
//	if (0!=strcmp(szShopOrEquip, "Shop") && 0!=strcmp(szShopOrEquip, "Equip")) return;
//
//	MTextArea* pTextArea[4];
//	char sz1[256], sz2[256], szTextArea[64];
//	const char* szRed	= "";
//	const char* szGreen	= "^2";
//	const char* szGray	= "^9";
//	const char* szColor	= NULL;
//
//	// prefix¿¡ µû¶ó¼­ "Shop_MyInfo1" "Shop_MyInfo2" "Shop_MyInfo3" "Equip_MyInfo1" "Equip_MyInfo2" "Equip_MyInfo3"ÀÇ ÀÌ¸§À» ±¸¼º
//	// (°¡·Î·Î ´Ã¾î³õÀº TextArea ¼¼°³¿¡ ³ª´²¼­ Ç¥½ÃÇÏ±â ¶§¹®¿¡..)
//	for (int i=0; i<4; ++i)
//	{
//		sprintf(szTextArea, "%s_MyInfo%d", szShopOrEquip, i+1);
//		pTextArea[i] = (MTextArea*)GetIDLResource()->FindWidget(szTextArea);
//		if (NULL == pTextArea[i])
//		{
//			_ASSERT(0);
//			return;	// ÇÏ³ª¶óµµ ¸øÃ£À¸¸é ¸®ÅÏÇØ¹ö¸®ÀÚ
//		}
//
//		pTextArea[i]->Clear();
//	}
//
//	// Ã¹¹øÂ° TextArea (·¹º§, ¹«°Ô)
//	szColor = szGray;
//	if (nReqLevel > ZGetMyInfo()->GetLevel())
//		szColor = szRed;
//	sprintf(sz1, "^9%s: %s%d ^9%s", ZMsg(MSG_CHARINFO_LEVEL), szColor, ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
//	pTextArea[0]->AddText(sz1);
//
//	int nCurrMaxWT = ZGetMyInfo()->GetItemList()->GetMaxWeight();
//	sprintf(sz1, "^9%s: ", ZMsg(MSG_CHARINFO_WEIGHT));
//	szColor = (nNewWT > nNewMaxWT) ? szRed : szGray;
//	sprintf(sz2, "%s%d", szColor, nNewWT);
//	strcat(sz1, sz2);
//	sprintf(sz2, "^9/%d", nCurrMaxWT);
//	strcat(sz1, sz2);
//	int nDiffMaxWT = nNewMaxWT - nCurrMaxWT;
//	if (nDiffMaxWT != 0)
//	{
//		szColor = (nDiffMaxWT > 0) ? szGreen : szRed;
//		sprintf(sz2, "%s%+d", szColor, nDiffMaxWT);
//		strcat(sz1, sz2);
//	}
//	pTextArea[0]->AddText(sz1);
//
//
//
//	// µÎ¹øÂ° TextArea (Ã¼·Â, ¹æ¾î)
//	sprintf(sz1, "^9%s: %d ", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
//	int nDiffHP = nNewHP - ZGetMyInfo()->GetHP();
//	if (nDiffHP != 0)
//	{
//		szColor = (nDiffHP > 0) ? szGreen : szRed;
//		sprintf(sz2, "%s%+d", szColor, nDiffHP);
//		strcat(sz1, sz2);
//	}
//	pTextArea[1]->AddText(sz1);
//
//	sprintf(sz1, "^9%s: %d ", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
//	int nDiffAP = nNewAP - ZGetMyInfo()->GetAP();
//	if (nDiffAP != 0)
//	{
//		szColor = (nDiffAP > 0) ? szGreen : szRed;
//		sprintf(sz2, "%s%+d", szColor, nDiffAP);
//		strcat(sz1, sz2);
//	}
//	pTextArea[1]->AddText(sz1);
//
//
//	// ¼¼¹øÂ° TextArea (¹Ù¿îÆ¼, Ä³½Ã)
//	sprintf(sz1, "^9%s: ", ZMsg(MSG_CHARINFO_BOUNTY));
//	szColor = (nReqBounty > ZGetMyInfo()->GetBP()) ? szRed : szGray;
//	sprintf(sz2, "%s%d", szColor, ZGetMyInfo()->GetBP());
//	strcat(sz1, sz2);
//	pTextArea[2]->AddText(sz1);
//
//	//sprintf(sz1, "^9%s: ", "Cash");
//	sprintf(sz1, "^9%s: ", ZMsg(MSG_CHARINFO_CASH));
//	szColor = (nReqBounty > ZGetMyInfo()->GetCash()) ? szRed : szGray;
//	sprintf(sz2, "%s%d", szColor, ZGetMyInfo()->GetCash());
//	strcat(sz1, sz2);
//	pTextArea[2]->AddText(sz1);
//
//	//sprintf(sz1, "^9%s: ", "Medal");
//	sprintf(sz1, "^9%s: ", ZMsg(MSG_CHARINFO_MEDAL));
//	szColor = (nReqBounty > ZGetMyInfo()->GetLC()) ? szRed : szGray;
//	sprintf(sz2, "%s%d", szColor, ZGetMyInfo()->GetLC());
//	strcat(sz1, sz2);
//	pTextArea[3]->AddText(sz1);
//
//	//sprintf(sz1, "^9%s: ", "Event");
//	sprintf(sz1, "^9%s: ", ZMsg(MSG_CHARINFO_EVENT));
//	szColor = (nReqBounty > ZGetMyInfo()->GetEV()) ? szRed : szGray;
//	sprintf(sz2, "%s%d", szColor, ZGetMyInfo()->GetEV());
//	strcat(sz1, sz2);
//	pTextArea[3]->AddText(sz1);
//
//}
void ZShopEquipInterface::DrawCharInfoText(char* szShopOrEquip, int nReqLevel, int nNewWT, int nNewMaxWT, int nNewHP, int nNewAP, int nReqBounty, int nReqCash)
{
	if (szShopOrEquip == NULL)
	{
		DrawCharInfoText("Shop", nReqLevel, nNewWT, nNewMaxWT, nNewHP, nNewAP, nReqBounty, nReqCash);
		DrawCharInfoText("Equip", nReqLevel, nNewWT, nNewMaxWT, nNewHP, nNewAP, nReqBounty, nReqCash);
		DrawCharInfoText("Event", nReqLevel, nNewWT, nNewMaxWT, nNewHP, nNewAP, nReqBounty, nReqCash);
		return;
	}

	if (0 != strcmp(szShopOrEquip, "Shop") && 0 != strcmp(szShopOrEquip, "Equip") && 0 != strcmp(szShopOrEquip, "Event")) return;

	const int MAX_INFO_WIDGETS = 10;
	MLabel* pLabel[MAX_INFO_WIDGETS];
	char sz1[256], sz2[256], szLabelName[64];

	// Bảng màu sắc cho Label
	MCOLOR colorWhite = MCOLOR(255, 255, 255);
	MCOLOR colorGray = MCOLOR(160, 160, 160);
	MCOLOR colorRed = MCOLOR(255, 60, 60);
	MCOLOR colorGreen = MCOLOR(0, 255, 128);

	// 1. Khởi tạo và Clear Label
	for (int i = 0; i < MAX_INFO_WIDGETS; ++i)
	{
		sprintf(szLabelName, "%s_MyInfo%d", szShopOrEquip, i + 1);
		pLabel[i] = (MLabel*)GetIDLResource()->FindWidget(szLabelName);
		if (NULL != pLabel[i])
		{
			pLabel[i]->SetText("");
			pLabel[i]->SetTextColor(colorGray); // Màu mặc định
		}
	}

	// 1. MyInfo1: NAME
	if (pLabel[0])
	{
		sprintf(sz1, "%s", ZGetMyInfo()->GetCharName());
		pLabel[0]->SetText(sz1);
		pLabel[0]->SetTextColor(colorWhite);
	}

	// 2. MyInfo2: LEVEL
	if (pLabel[1])
	{
		sprintf(sz1, "%s %d", ZMsg(MSG_CHARINFO_LEVELMARKER), ZGetMyInfo()->GetLevel());
		pLabel[1]->SetText(sz1);

		if (nReqLevel > ZGetMyInfo()->GetLevel())
			pLabel[1]->SetTextColor(colorRed);
		else
			pLabel[1]->SetTextColor(colorWhite);
	}

	// 3. MyInfo3: WEIGHT
	if (pLabel[2])
	{
		int nCurrMaxWT = ZGetMyInfo()->GetItemList()->GetMaxWeight();
		int nDiffMaxWT = nNewMaxWT - nCurrMaxWT;

		if (nDiffMaxWT != 0)
			sprintf(sz1, "%s: %d/%d (%+d)", ZMsg(MSG_CHARINFO_WEIGHT), nNewWT, nCurrMaxWT, nDiffMaxWT);
		else
			sprintf(sz1, "%s: %d/%d", ZMsg(MSG_CHARINFO_WEIGHT), nNewWT, nCurrMaxWT);

		pLabel[2]->SetText(sz1);

		if (nNewWT > nNewMaxWT)
			pLabel[2]->SetTextColor(colorRed);
		else
			pLabel[2]->SetTextColor(colorWhite);
	}

	// 4. MyInfo4: HP
	if (pLabel[3])
	{
		int nDiffHP = nNewHP - ZGetMyInfo()->GetHP();
		if (nDiffHP != 0)
			sprintf(sz1, "%s: %d (%+d)", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP(), nDiffHP);
		else
			sprintf(sz1, "%s: %d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());

		pLabel[3]->SetText(sz1);

		if (nDiffHP > 0)
			pLabel[3]->SetTextColor(colorGreen);
		else if (nDiffHP < 0)
			pLabel[3]->SetTextColor(colorRed);
		else
			pLabel[3]->SetTextColor(colorWhite);
	}

	// 5. MyInfo5: AP
	if (pLabel[4])
	{
		int nDiffAP = nNewAP - ZGetMyInfo()->GetAP();
		if (nDiffAP != 0)
			sprintf(sz1, "%s: %d (%+d)", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP(), nDiffAP);
		else
			sprintf(sz1, "%s: %d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());

		pLabel[4]->SetText(sz1);

		if (nDiffAP > 0)
			pLabel[4]->SetTextColor(colorGreen);
		else if (nDiffAP < 0)
			pLabel[4]->SetTextColor(colorRed);
		else
			pLabel[4]->SetTextColor(colorWhite);
	}

	// 6. MyInfo6: BOUNTY (BP)
	if (pLabel[5])
	{
		sprintf(sz1, "%d", ZGetMyInfo()->GetBP());
		pLabel[5]->SetText(sz1);
		pLabel[5]->SetTextColor(colorWhite);
	}

	// 7. MyInfo7: CASH
	if (pLabel[6])
	{
		sprintf(sz1, "%d", ZGetMyInfo()->GetCash());
		pLabel[6]->SetText(sz1);
		pLabel[6]->SetTextColor(colorWhite);
	}

	// 8. MyInfo8: MEDAL (LC)
	if (pLabel[7])
	{
		sprintf(sz1, "%d", ZGetMyInfo()->GetLC());
		pLabel[7]->SetText(sz1);
		pLabel[7]->SetTextColor(colorWhite);
	}

	// 9. MyInfo9: EVENT (EV)
	if (pLabel[8])
	{
		sprintf(sz1, "%d", ZGetMyInfo()->GetEV());
		pLabel[8]->SetText(sz1);
		pLabel[8]->SetTextColor(colorWhite);
	}

	// 10. MyInfo10: EXP (EXP)
	if (pLabel[9])
	{
		sprintf(sz1, "EXP %lu", ZGetMyInfo()->GetXP());
		pLabel[9]->SetText(sz1);
		pLabel[9]->SetTextColor(colorWhite);
	}
}

void ZShopEquipInterface::Update()
{
	if (m_pSellCashItemConfirmDlg)
		m_pSellCashItemConfirmDlg->Update();
}

bool ZShopEquipInterface::IsItemMatchFilter(MMatchItemDesc* pDesc, int nMainTab, int nSubFilter)
{
	if (!pDesc) return false;

	// Dùng .Ref() đúng theo khai báo trong MProtectValue.h
	MMatchItemType nType = pDesc->m_nType.Ref();
	MMatchWeaponType nWeaponType = pDesc->m_nWeaponType.Ref();
	MMatchItemSlotType nSlot = pDesc->m_nSlot; // m_nSlot trong struct MMatchItemDesc là enum thường, không dùng .Ref()

	switch (nMainTab)
	{
		// 1. TAB WEAPON (VŨ KHÍ)
	case INV_TAB_WEAPON:
	{
		// Chỉ lấy Item là Vũ khí Cận chiến (MELEE) hoặc Tầm xa (RANGE)
		if (nType != MMIT_MELEE && nType != MMIT_RANGE)
			return false;

		// SubFilter 0: ALL
		if (nSubFilter == 0) return true;

		// SubFilter 1: Cận chiến
		if (nSubFilter == 1 && nType == MMIT_MELEE) return true;

		// Lọc súng theo nWeaponType
		if (nSubFilter == 2 && (nWeaponType == MWT_PISTOL || nWeaponType == MWT_PISTOLx2)) return true; // Pistol
		if (nSubFilter == 3 && (nWeaponType == MWT_REVOLVER || nWeaponType == MWT_REVOLVERx2)) return true; // Revolver
		if (nSubFilter == 4 && (nWeaponType == MWT_SMG || nWeaponType == MWT_SMGx2)) return true; // SMG
		if (nSubFilter == 5 && (nWeaponType == MWT_SHOTGUN || nWeaponType == MWT_SAWED_SHOTGUN)) return true; // Shotgun
		if (nSubFilter == 6 && (nWeaponType == MWT_RIFLE || nWeaponType == MWT_SNIFER)) return true; // Rifle / Sniper
		if (nSubFilter == 7 && (nWeaponType == MWT_MACHINEGUN)) return true; // MG / Rocket
		if (nSubFilter == 8 && (nWeaponType == MWT_ROCKET)) return true; // Rocket

		return false;
	}

	// 2. TAB ARMOR (TRANG PHỤC / GIÁP)
	case INV_TAB_ARMOR:
	{
		if (nType != MMIT_EQUIPMENT) return false;

		if (nSubFilter == 0) return true;

		// Lưu ý: Lọc slot của MMatchItemSlotType (MMIST_...)
		if (nSubFilter == 1 && nSlot == MMIST_HEAD) return true;   // Mũ
		if (nSubFilter == 2 && nSlot == MMIST_CHEST) return true;  // Áo
		if (nSubFilter == 3 && nSlot == MMIST_HANDS) return true;  // Găng
		if (nSubFilter == 4 && nSlot == MMIST_LEGS) return true;   // Quần
		if (nSubFilter == 5 && nSlot == MMIST_FEET) return true;   // Giày
		if (nSubFilter == 6 && (nSlot == MMIST_FINGER || nSlot == MMIST_AVATAR)) return true; // Nhẫn / Phụ kiện

		return false;
	}

	// 3. TAB ITEM (TIÊU HAO / THẺ / AVATAR)
	case INV_TAB_ITEM:
	{
		unsigned long int nItemID = pDesc->m_nID;

		// 1. Nhận toàn bộ Quest Item & Sacrifices (Dải ID 200000 - 200044)
		if (nItemID >= 200000 && nItemID <= 200044)
			return true;

		// 2. Nhận các loại Item Tiêu hao / Lựu đạn / Bẫy / Thuốc hồi máu / Giáp
		if (nWeaponType == MWT_MED_KIT ||
			nWeaponType == MWT_REPAIR_KIT ||
			nWeaponType == MWT_BULLET_KIT ||
			nWeaponType == MWT_FRAGMENTATION ||
			nWeaponType == MWT_FLASH_BANG ||
			nWeaponType == MWT_SMOKE_GRENADE ||
			nWeaponType == MWT_FOOD ||
			nWeaponType == MWT_POTION ||
			nWeaponType == MWT_TRAP)
		{
			return true;
		}

		// 3. Nhận các loại Item có Slot thuộc dạng Custom, Avatar, Buff, Ring
		if (nType == MMIT_CUSTOM || nType == MMIT_TICKET || nType == MMIT_AVATAR || nSlot == MMIST_DASH || nSlot == MMIST_JJANG || nSlot == MMIST_STUN || nSlot == MMIST_EMOTE ||
			nSlot == MMIST_CUSTOM || nSlot == MMIST_AVATAR || nSlot == MMIST_COMMUNITY || nSlot == MMIST_LONGBUFF)
		{
			return true;
		}

		return false;
	}

	// 4. TAB ACCOUNT / STORAGE
	case INV_TAB_ACCOUNT:
	{
		return true;
	}
	}

	return false;
}

void ZShopEquipInterface::OnClickMainTab(int nTab)
{
	m_nSelectedMainTab = nTab;
	m_nSelectedSubFilter = 0; // Reset sub-filter

	ZIDLResource* pResource = GetIDLResource();

	// 1. LẤY CON TRỎ CÁC PICTURE HÌNH NỀN NÚT
	MWidget* pAccountBg = pResource->FindWidget("AccountTab_Bg");
	MWidget* pWeaponBg = pResource->FindWidget("WeaponTab_Bg");
	MWidget* pArmorBg = pResource->FindWidget("ArmorTab_Bg");
	MWidget* pItemBg = pResource->FindWidget("ItemTab_Bg");

	// 2. ẨN / HIỆN HÌNH NỀN NÚT THEO TAB ĐANG CHỌN
	if (pAccountBg) pAccountBg->Show(nTab == INV_TAB_ACCOUNT);
	if (pWeaponBg)  pWeaponBg->Show(nTab == INV_TAB_WEAPON);
	if (pArmorBg)   pArmorBg->Show(nTab == INV_TAB_ARMOR);
	if (pItemBg)    pItemBg->Show(nTab == INV_TAB_ITEM);

	// -----------------------------------------------------------
	// (Các đoạn code xử lý SubFilter, ListBox)
	// -----------------------------------------------------------

	// 3. Ẩn/Hiện nhóm Sub-Filter
	MWidget* pWepSubFrame = pResource->FindWidget("WeaponSubFilterGroup");
	MWidget* pArmSubFrame = pResource->FindWidget("ArmorSubFilterGroup");

	if (pWepSubFrame) pWepSubFrame->Show(nTab == INV_TAB_WEAPON);
	if (pArmSubFrame) pArmSubFrame->Show(nTab == INV_TAB_ARMOR);

	// 4. Chuyển đổi Widget List
	bool bIsAccountTab = (nTab == INV_TAB_ACCOUNT);

	MWidget* pEquipList = pResource->FindWidget("EquipmentList");
	MWidget* pAccountList = pResource->FindWidget("AccountItemList");

	if (pEquipList)   pEquipList->Show(!bIsAccountTab);
	if (pAccountList) pAccountList->Show(bIsAccountTab);

	// 5. Tải dữ liệu theo Tab
	if (bIsAccountTab)
	{
		SelectEquipmentTab(1);
		ZPostRequestAccountItemList(ZGetGameClient()->GetPlayerUID());
	}
	else
	{
		SelectEquipmentTab(0);
		ZGetMyInfo()->GetItemList()->Serialize();
	}

	SelectEquipmentFrameList("Equip", true);
}

void ZShopEquipInterface::OnClickSubFilter(int nSubFilter)
{
	m_nSelectedSubFilter = nSubFilter;

	// ÉP TẢI LẠI DANH SÁCH ITEM THEO SUB-FILTER MỚI
	if (ZGetMyInfo() && ZGetMyInfo()->GetItemList())
	{
		ZGetMyInfo()->GetItemList()->Serialize();
	}

	// Refresh lại hiển thị khung Slot trang bị
	SelectEquipmentFrameList("Equip", true);
	SelectEquipmentFrameList("Shop", true);
}

void ZShopEquipInterface::Init()
{
	InitPackages();
	OnClickShopMainTab(ZSHOP_TAB_NEW);
}

void ZShopEquipInterface::InitPackages()
{
	m_vPackages.clear();

	// Package 1: Imperial Guardian
	ZPACKAGE_INFO pack1;
	pack1.nPackageID = 1;
	pack1.szName = "Imperial Guardian Package";
	pack1.szDesc = "A package containing the complete Imperial Guardian uniform.";
	pack1.nPrice = 1890;
	pack1.nCurrencyType = ITEM_CASH; //ITEM_CASH
	// Item ID Nam
	pack1.vItemIDsMale = { 3000021, 3010020, 3020021, 3030016, 3040016 };
	// Item ID Nữ
	pack1.vItemIDsFemale = { 3110155, 3100097, 3120077, 3130025, 3140053 };

	m_vPackages.push_back(pack1);

	// Package 2: Travia Preacher
	ZPACKAGE_INFO pack2;
	pack2.nPackageID = 2;
	pack2.szName = "Travia Preacher Package";
	pack2.szDesc = "A package containing the complete Travia Preacher uniform.";
	pack2.nPrice = 1890;
	pack2.nCurrencyType = ITEM_CASH; //ITEM_MEDAL;
	// Item ID Nam
	pack2.vItemIDsMale = { 3000010, 3010011, 3020011, 3030011, 3040011 };
	// Item ID Nữ
	pack2.vItemIDsFemale = { 3110153, 3100090, 3120073, 3130019, 3140049 };
	m_vPackages.push_back(pack2);

	// Package 3: Axium Soul Reaper
	ZPACKAGE_INFO pack3;
	pack3.nPackageID = 3;
	pack3.szName = "Axium Soul Reaper Package";
	pack3.szDesc = "A package containing the complete Axium Soul Reaper uniform.";
	pack3.nPrice = 1890;
	pack3.nCurrencyType = ITEM_CASH; //ITEM_MEDAL;
	// Item ID Nam
	pack3.vItemIDsMale = { 3000011, 3010012, 3020013, 3030012, 3040015 };
	// Item ID Nữ
	pack3.vItemIDsFemale = { 3100091, 3110154, 3120074, 3130020, 3140050 };
	m_vPackages.push_back(pack3);
}

void ZShopEquipInterface::OnClickShopMainTab(int nTab)
{
	m_nCurrentTab = nTab;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (!pResource) return;

	bool bIsNewTab = (nTab == ZSHOP_TAB_NEW); // 0 (Tab New / Packages)

	// =========================================================================
	// 1. DỌN DẸP TRIỆT ĐỂ WIDGET CỦA TAB NEW (Dù ở đâu cũng phải Clear sạch)
	// =========================================================================
	MWidget* pFrameNew = pResource->FindWidget("Shop_New_Frame");
	if (pFrameNew) pFrameNew->Show(bIsNewTab);

	MWidget* pNewCharView = pResource->FindWidget("Shop_NewTabCharacterView");
	if (pNewCharView) pNewCharView->Show(bIsNewTab);

	// Reset Bitmap Preview của Tab New để tránh bị dính hình cũ
	MPicture* pNewPreview = (MPicture*)pResource->FindWidget("Shop_NewProfileItem_Preview");
	if (pNewPreview)
	{
		pNewPreview->Show(bIsNewTab);
		if (!bIsNewTab) pNewPreview->SetBitmap(NULL);
	}

	// =========================================================================
	// 2. SHOW / HIDE THANH SEARCH & FILTER
	// =========================================================================
	const char* szFilterWidgets[] = {
		"EquipmentInformationShop",
		"Shop_AllEquipmentFilter",
		"Shop_AllEquipmentSubFilter",
		"Shop_Search",
		"shop_label_search",
		"Shop_Search_Text",
		"Shop_FilterFrame"
	};

	for (const char* szWidgetName : szFilterWidgets)
	{
		MWidget* pW = pResource->FindWidget(szWidgetName);
		if (pW) pW->Show(!bIsNewTab);
	}

	// =========================================================================
	// 3. ĐIỀU HƯỚNG TAB CHUẨN (Ánh xạ đúng Index 0, 1, 2, 3, 4 cho SelectShopTab)
	// =========================================================================
	if (bIsNewTab)
	{
		// Ở Tab NEW -> Ẩn toàn bộ 5 ListBox & Nút Mua/Bán truyền thống
		const char* szOldLists[] = { "AllEquipmentList", "MyAllEquipmentList", "CashEquipmentList", "MedalEquipmentList", "EventEquipmentList" };
		for (int i = 0; i < 5; ++i)
		{
			MWidget* pList = pResource->FindWidget(szOldLists[i]);
			if (pList) pList->Show(false);
		}

		// Ẩn các nút Buy Confirm cũ
		WidgetEnableShow("BuyConfirmCaller", false, false);
		WidgetEnableShow("SellConfirmCaller", false, false);
		WidgetEnableShow("BuyCashConfirmCaller", false, false);
		WidgetEnableShow("BuyMedalConfirmCaller", false, false);
		WidgetEnableShow("BuyEventConfirmCaller", false, false);

		OnSelectPackage(0); // Load gói đồ đầu tiên của Tab New
	}
	else
	{
		// BẮT BUỘC TRUYỀN ĐÚNG INDEX LISTBOX (0: BP, 1: Sell, 2: Cash, 3: Medal, 4: Event)
		int nTargetSubTab = 0;

		switch (nTab)
		{
		case ZSHOP_TAB_ITEM:      nTargetSubTab = 0; break; // BP Shop
		case ZSHOP_TAB_SELL:      nTargetSubTab = 1; break; // Sell Shop
		case ZSHOP_TAB_CREDITS:   nTargetSubTab = 2; break; // Cash Shop
		case ZSHOP_TAB_MEDAL:     nTargetSubTab = 3; break; // Medal Shop
		case ZSHOP_TAB_EVENT:     nTargetSubTab = 4; break; // Event Shop
		default:                  nTargetSubTab = 0; break;
		}

		// Gọi hàm chuyển SubTab truyền thống (Hàm này sẽ tự Show ListBox + Serialize đúng Tab)
		SelectShopTab(nTargetSubTab);

		// TUYỆT ĐỐI KHÔNG GỌI ShowShopDialog(true) Ở ĐÂY NỮA!
		m_nCurrentCategory = ZSHOP_CAT_ALL;
		RefreshItemList();
	}
}

void ZShopEquipInterface::OnSelectPackage(int nIndex)
{
	//mlog("=== OnSelectPackage Debug Start: Index %d ===\n", nIndex);
	//mlog("Total Packages in memory: %d\n", (int)m_vPackages.size());

	// Nếu m_vPackages bị rỗng, tự động gọi InitPackages để nạp lại ngay lập tức
	if (m_vPackages.empty())
	{
		mlog("Packages vector is EMPTY! Auto re-initializing...\n");
		InitPackages();
	}

	if (nIndex < 0 || nIndex >= (int)m_vPackages.size())
	{
		mlog("[ERROR] Invalid Index %d (Total Packages: %d)\n", nIndex, (int)m_vPackages.size());
		return;
	}
	m_nSelectedPackageIndex = nIndex;

	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	if (!pResource) return;

	// Đổi lại về Widget Preview mới của bác (hoặc EquipmentInformationShop)
	ZCharacterView* pCharView = (ZCharacterView*)pResource->FindWidget("Shop_NewTabCharacterView");
	if (!pCharView) pCharView = (ZCharacterView*)pResource->FindWidget("EquipmentInformationShop");

	if (!pCharView)
	{
		mlog("Error: CharacterView Widget NOT FOUND!\n");
		return;
	}

	pCharView->Show(true);

	ZMyInfo* pmi = ZGetMyInfo();
	MMatchSex nSex = pmi->GetSex();
	int nHair = pmi->GetHair();
	int nFace = pmi->GetFace();

	const vector<int>& vItemIDs = (nSex == MMS_FEMALE) ? m_vPackages[nIndex].vItemIDsFemale : m_vPackages[nIndex].vItemIDsMale;
	mlog("Pack Items Count: %d | Sex: %d\n", vItemIDs.size(), (int)nSex);

	unsigned long int nEquipedItemID[MMCIP_END];
	memset(nEquipedItemID, 0, sizeof(nEquipedItemID));

	for (size_t i = 0; i < vItemIDs.size(); ++i)
	{
		int nItemID = vItemIDs[i];
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
		if (!pItemDesc)
		{
			mlog("[ERROR] Item ID %d INVALID in zitem.xml!\n", nItemID);
			continue;
		}

		MMatchCharItemParts nPart = MMCIP_END;
		switch (pItemDesc->m_nSlot)
		{
		case MMIST_HEAD:	nPart = MMCIP_HEAD; break;
		case MMIST_CHEST:	nPart = MMCIP_CHEST; break;
		case MMIST_HANDS:	nPart = MMCIP_HANDS; break;
		case MMIST_LEGS:	nPart = MMCIP_LEGS; break;
		case MMIST_FEET:	nPart = MMCIP_FEET; break;
		case MMIST_MELEE:	nPart = MMCIP_MELEE; break;
		case MMIST_RANGE:	nPart = MMCIP_PRIMARY; break;
		default:			nPart = MMCIP_END; break;
		}

		if (nPart != MMCIP_END)
		{
			nEquipedItemID[nPart] = nItemID;
			mlog("Mapped Part %d -> ItemID %d\n", (int)nPart, nItemID);
		}
	}

	pCharView->InitCharParts(nSex, nHair, nFace, nEquipedItemID);
}

void ZShopEquipInterface::OnSelectCategory(int nCategory)
{
	m_nCurrentCategory = nCategory;
	RefreshItemList();
}

void ZShopEquipInterface::RefreshItemList()
{}

void ZShopEquipInterface::OnSelectItem(int nItemID)
{
	m_nSelectedItemID = nItemID;
	UpdateCharacterPreview(nItemID);
}

void ZShopEquipInterface::UpdateCharacterPreview(int nItemID)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (!pResource) return;

	ZCharacterView* pCharView = (ZCharacterView*)pResource->FindWidget("Shop_NewTabCharacterView");
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

	if (pCharView && pItemDesc)
	{
		MMatchCharItemParts nPart = GetSuitableItemParts(pItemDesc->m_nSlot);
		MMatchItemType nType = pItemDesc->m_nType.Ref();

		if (nPart != MMCIP_END)
		{
			pCharView->SetParts(nPart, pItemDesc->m_nID);

			if (nType == MMIT_MELEE || nType == MMIT_RANGE || nType == MMIT_CUSTOM)
			{
				pCharView->ChangeVisualWeaponParts(nPart);
			}
		}
	}
}

// 1. Hàm gửi Command chính thức lên Server
void ZShopEquipInterface::RequestBuyPackage(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_vPackages.size()) return;

	ZPACKAGE_INFO& pack = m_vPackages[nIndex];

	// Bắn Command gửi Package ID lên Server
	ZPOSTCMD1(MC_MATCH_BUY_PACKAGE, MCmdParamInt(pack.nPackageID));
}

// 2. Hàm khi bấm nút Mua trên UI (Bật Pop-up)
void ZShopEquipInterface::OnBuyPackageButton(void)
{
	if (m_nSelectedPackageIndex < 0 || m_nSelectedPackageIndex >= (int)m_vPackages.size()) return;

	ZPACKAGE_INFO& pack = m_vPackages[m_nSelectedPackageIndex];

	// 2. Check số dư tiền mặt
	bool bHasEnoughMoney = true;
	const char* szCurrency = "Cash";

	// Ép kiểm tra chính xác theo ITEM_CASH / ITEM_MEDAL / ITEM_EVENT
	if (pack.nCurrencyType == ITEM_CASH)
	{
		szCurrency = "Cash";
		if (ZGetMyInfo()->GetCash() < pack.nPrice)
		{
			bHasEnoughMoney = false;
		}
	}
	else if (pack.nCurrencyType == ITEM_MEDAL)
	{
		szCurrency = "Medal";
		if (ZGetMyInfo()->GetLC() < pack.nPrice)
		{
			bHasEnoughMoney = false;
		}
	}
	else if (pack.nCurrencyType == ITEM_EVENT)
	{
		szCurrency = "Event Coin";
		if (ZGetMyInfo()->GetEV() < pack.nPrice)
		{
			bHasEnoughMoney = false;
		}
	}

	// Thông báo ngay nếu thiếu tiền
	if (!bHasEnoughMoney)
	{
		char szErrMsg[128];
		sprintf(szErrMsg, "You do not have enough %s to purchase this package!", szCurrency);
		if (ZGetGameInterface())
			ZGetGameInterface()->ShowMessage(szErrMsg, NULL);
		return;
	}

	// Tên loại tiền
	if (pack.nCurrencyType == ITEM_CASH) szCurrency = "Cash";
	else if (pack.nCurrencyType == ITEM_MEDAL) szCurrency = "Medal";
	else if (pack.nCurrencyType == ITEM_EVENT) szCurrency = "Event Coin";

	// Ghép chuỗi thông báo
	char szMsg[256];
	sprintf(szMsg, "Are you sure you want to buy the '%s' package for %d %s?",
		pack.szName.c_str(),
		pack.nPrice,
		szCurrency);

	// Mở Pop-up Confirm
	if (m_pSimpleConfirmDlg)
	{
		m_pSimpleConfirmDlg->Open(szMsg, this);
	}
}

void ZShopEquipInterface::OnClickBuySelected()
{
	OnClickBuyPackage();
}

void ZShopEquipInterface::OnResponseBuyPackage(int nResult)
{
	if (!ZGetGameInterface()) return;

	if (nResult == MOK) // 0: Mua thành công
	{
		// Hiện Pop-up thông báo thành công
		ZGetGameInterface()->ShowMessage("Package purchased successfully! The items have been transferred to your Inventory.", NULL);

		// SỬA Ở ĐÂY: Truyền UID của nhân vật vào hàm
		ZPostRequestAccountItemList(ZGetMyUID());
	}
	else if (nResult == MERR_TOO_EXPENSIVE_CASH)
	{
		ZGetGameInterface()->ShowMessage("Transaction failed! You do not have enough funds to purchase this package.", NULL);
	}
	else
	{
		ZGetGameInterface()->ShowMessage("Transaction failed! An error occurred while purchasing the package.", NULL);
	}

	//ZGetGameInterface()->ShowMessage(nResult);
}