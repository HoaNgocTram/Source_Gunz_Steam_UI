#pragma once

#include "ZShopEquipItemConfirm.h"
#include "ZInterface.h"
#include "MMatchItem.h"
#include <vector>
#include <string>

// Forward declaration
class ZShopEquipListItem;

void WidgetEnableShow(const char* szWidget, bool bEnable, bool bShow);

// Struct Package dùng chung
struct ZPACKAGE_INFO
{
	int nPackageID;
	std::string szName;
	std::string szDesc;
	int nPrice;
	int nCurrencyType;
	std::vector<int> vItemIDsMale;
	std::vector<int> vItemIDsFemale;

	ZPACKAGE_INFO() : nPackageID(0), nPrice(0), nCurrencyType(0) {}
};

// Gộp duy nhất 1 Class ZShopEquipInterface
class ZShopEquipInterface : public ISimpleConfirmDlgDoneHandler
{
public:
	// Enum định nghĩa Tab
	enum INVENTORY_MAIN_TAB {
		INV_TAB_WEAPON = 0,
		INV_TAB_ARMOR,
		INV_TAB_ITEM,
		INV_TAB_ACCOUNT
	};

	enum ZSHOP_TAB
	{
		ZSHOP_TAB_NEW = 0,
		ZSHOP_TAB_ITEM,
		ZSHOP_TAB_CREDITS,
		ZSHOP_TAB_MEDAL,
		ZSHOP_TAB_EVENT,
		ZSHOP_TAB_SELL,
		ZSHOP_TAB_CHARGE
	};

	enum ZSHOP_CATEGORY
	{
		ZSHOP_CAT_ALL = 0,
		ZSHOP_CAT_ARMOR,
		ZSHOP_CAT_MELEE,
		ZSHOP_CAT_RANGED,
		ZSHOP_CAT_TACTICAL,
		ZSHOP_CAT_OTHERS
	};

private:
	int					m_nShopTabNum;
	int					m_nEquipTabNum;

	int					m_nSellQuestItemCount;
	int					m_nSellSpendableItemCount;
	int					m_nBuySpendableItemCount;

	ZItemCountDlg* m_pItemCountDlg;
	ZSimpleConfirmDlg* m_pSimpleConfirmDlg;
	ZSellCashItemConfirmDlg* m_pSellCashItemConfirmDlg;

	int m_idxArmorWeaponTab;
	void* m_pItemDescriptionClient;

	// Lưu trạng thái Tab & Package
	int m_nSelectedMainTab;
	int m_nSelectedSubFilter;
	int m_nCurrentTab;
	int m_nSelectedPackageIndex;
	int m_nCurrentCategory;
	int m_nSelectedItemID;

	std::vector<ZPACKAGE_INFO> m_vPackages;

public:
	bool				m_bViewThumbnails;

	ZShopEquipInterface();
	~ZShopEquipInterface();

	// Callback khi Confirm Mua Package (ISimpleConfirmDlgDoneHandler)
	virtual void OnDone(bool bOk) override;

	// Filtering & Tabs
	bool IsItemMatchFilter(MMatchItemDesc* pDesc, int nMainTab, int nSubFilter);
	void OnClickMainTab(int nTab);
	void OnClickSubFilter(int nSubFilter);
	int GetSelectedMainTab() const { return m_nSelectedMainTab; }
	int GetSelectedSubFilter() const { return m_nSelectedSubFilter; }

	void ConfirmBuySimple();
	ZShopEquipListItem* GetListCurSelItem(const char* szListWidget);

	ZItemCountDlg* GetItemCountDlg() { return m_pItemCountDlg; }
	ZSimpleConfirmDlg* GetSimpleConfirmDlg() { return m_pSimpleConfirmDlg; }
	ZSellCashItemConfirmDlg* GetSellCashItemConfirmDlg() { return m_pSellCashItemConfirmDlg; }

	void OnSellButton(void);
	void OnBuyButton(void);
	void OnBuyCashButton(void);
	void OnBuyEventButton(void);
	void OnBuyMedalButton(void);

	MMatchCharItemParts RecommendEquipParts(MMatchItemSlotType slot);

	bool Equip(void);
	bool Equip(MMatchCharItemParts parts, MUID& uidItem);

	void SetKindableItem(MMatchItemSlotType nSlotType);
	bool IsKindableItem(MMatchCharItemParts nParts, MMatchItemSlotType nSlotType);

	int _CheckRestrictBringAccountItem();
	bool CheckRestrictBringAccountItem();

	void OnSendAccountButton();
	void OnBringAccountButton();

	void SelectShopTab(int nTabIndex);
	void SelectEquipmentTab(int nTabIndex);

	void Update();

	void SelectEquipmentFrameList(const char* szName, bool bOpen);
	int GetArmorWeaponTabIndexContainItemParts(MMatchCharItemParts parts);
	bool IsEquipmentFrameListOpened(const char* szName = "Shop");

	void SetArmorWeaponTabIndex(int idx) { if (idx < 0 || idx > 3) return; else m_idxArmorWeaponTab = idx; }
	int GetArmorWeaponTabIndex() { return m_idxArmorWeaponTab; }
	void SelectArmorWeaponTabWithSlotType(MMatchItemSlotType slotType);
	void OnArmorWeaponTabButtonClicked(int nTab);

	void ShowItemDescription(bool bShow, MTextArea* pTextArea, void* pCaller);
	void DrawCharInfoText();
	void DrawCharInfoText(char* szShopOrEquip, int nReqLevel, int nNewWT, int nNewMaxWT, int nNewHP, int nNewAP, int nReqBounty, int nReqCash);

	// Handling Package & Shop Custom
	void Init();
	void InitPackages();
	void RequestBuyPackage(int nIndex);
	void OnBuyPackageButton();

	void OnClickShopMainTab(int nTab);
	void OnSelectPackage(int nIndex);
	void OnSelectCategory(int nCategory);
	void OnSelectItem(int nItemID);
	void OnClickBuySelected();
	void OnClickBuyPackage();

	void RefreshItemList();
	void UpdateCharacterPreview(int nItemID);
	void OnResponseBuyPackage(int nResult);
};
