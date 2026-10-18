#pragma once

#include "ZShopEquipItemConfirm.h"

class MShopSetItem;
class MShopPriceMap;

class IShopEquipItemHandle_Purchase;
class IShopEquipItemHandle_Sell;
class IShopEquipItemHandle_SendAccount;
class IShopEquipItemHandle_BringAccount;

enum ZShopEquipItemType
{
	ZSEIT_GAMBLE,
	ZSEIT_MATCH,
	ZSEIT_QUEST,
	ZSEIT_SET,
};

// ¿ÜºÎ¿¡¼­ ¸ðµç ¾ÆÀÌÅÛÀ» Á¾·ù(°·ºí,¸ÅÄ¡¾ÆÀÌÅÛ,Äù½ºÆ®¾ÆÀÌÅÛ µî)¿¡ °ü°è¾øÀÌ ´Ù·ê ¼ö ÀÖµµ·Ï ÇÏ±â À§ÇÑ °øÅë ÀÎÅÍÆäÀÌ½º
class ZShopEquipItem
{
protected:
	IShopEquipItemHandle_Purchase*		m_pHandlerPurchase;
	IShopEquipItemHandle_Sell*			m_pHandlerSell;
	IShopEquipItemHandle_SendAccount*	m_pHandlerSendAccount;
	IShopEquipItemHandle_BringAccount*	m_pHandlerBringAccount;

	MBitmap* m_pIconBitmap;

public:
	ZShopEquipItem() 
		: m_pHandlerPurchase(NULL)
		, m_pHandlerSell(NULL)
		, m_pHandlerSendAccount(NULL)
		, m_pHandlerBringAccount(NULL)
		, m_pIconBitmap(NULL) {}
	virtual ~ZShopEquipItem();
	virtual int GetItemID() const { return 0; }
	void SetHandlePurchase(IShopEquipItemHandle_Purchase* p)		{ m_pHandlerPurchase = p; }
	void SetHandleSell(IShopEquipItemHandle_Sell* p)				{ m_pHandlerSell = p; }
	void SetHandleSendAccount(IShopEquipItemHandle_SendAccount* p)	{ m_pHandlerSendAccount= p; }
	void SetHandleBringAccount(IShopEquipItemHandle_BringAccount* p){ m_pHandlerBringAccount= p; }

	IShopEquipItemHandle_Purchase* GetHandlePurchase()			{ return m_pHandlerPurchase; }
	IShopEquipItemHandle_Sell* GetHandleSell()					{ return m_pHandlerSell; }
	IShopEquipItemHandle_SendAccount* GetHandleSendAccount()	{ return m_pHandlerSendAccount; }
	IShopEquipItemHandle_BringAccount* GetHandleBringAccount()	{ return m_pHandlerBringAccount; }

	virtual ZShopEquipItemType GetType() = 0;

	virtual MMatchItemSlotType GetSlotType() = 0;
	virtual bool CanEquip() = 0;
	virtual bool CanSendAccount() = 0;
	virtual bool CanSell() = 0;
	virtual MBitmap* GetIcon() { return m_pIconBitmap; }
	virtual const char* GetName(char* szBuf) = 0;
	virtual const char* GetPriceText(char* szBuf);
	virtual const char* GetLevelResText(char* szBuf) = 0;

	virtual int GetPrice(bool& out_bCash);
	virtual int GetLevelRes() = 0;

	virtual void FillItemDesc(MTextArea* pTextArea) = 0;
	virtual void UpdateCharInfoText() = 0;
	virtual void UpdateCharacterView(ZCharacterView* pCharacterView) {}
};

// »óÁ¡,ÀåºñÃ¢À» À§ÇÑ °·ºí¾ÆÀÌÅÛ ·¡ÆÛ
class ZShopEquipItem_Gamble : public ZShopEquipItem
{
	const ZGambleItemDefine* m_pItemDesc;

public:
	ZShopEquipItem_Gamble(const ZGambleItemDefine* pDesc);
	ZShopEquipItemType GetType() { return ZSEIT_GAMBLE; }

	const ZGambleItemDefine* GetDesc() { return m_pItemDesc; }

	virtual MMatchItemSlotType GetSlotType() { return MMIST_NONE; }
	virtual bool CanEquip() { return true; } // °·ºí¾ÆÀÌÅÛÀÇ equipÀº use¿Í °°Àº ÀÇ¹Ì
	virtual bool CanSendAccount() { return m_pItemDesc->IsCash(); }
	virtual bool CanSell() {
#ifndef _SELL_CASHITEM
		return (GetDesc() && !GetDesc()->IsCash());
#else
		return GetDesc() ? true : false;
#endif
	}

	virtual const char* GetName(char* szBuf);
	virtual const char* GetLevelResText(char* szBuf);

	virtual int GetLevelRes();

	virtual void FillItemDesc(MTextArea* pTextArea);
	virtual void UpdateCharInfoText();
};

// »óÁ¡,ÀåºñÃ¢À» À§ÇÑ ¸ÅÄ¡¾ÆÀÌÅÛ ·¡ÆÛ
class ZShopEquipItem_Match : public ZShopEquipItem
{
	MMatchItemDesc* m_pItemDesc;
public:
	virtual int GetItemID() const override
	{
		return m_pItemDesc ? m_pItemDesc->m_nID : 0;
	}
	ZShopEquipItem_Match(MMatchItemDesc* pDesc);
	ZShopEquipItemType GetType() { return ZSEIT_MATCH; }

	MMatchItemDesc* GetDesc() { return m_pItemDesc; }

	virtual MMatchItemSlotType GetSlotType();
	virtual bool CanEquip() { return true; }
	virtual bool CanSendAccount() { return m_pItemDesc->IsCashItem(); }
	virtual bool CanSell() { 
#ifndef _SELL_CASHITEM
		return (GetDesc() && !GetDesc()->IsCashItem() && !GetDesc()->IsMedalItem() && !GetDesc()->IsEventItem());
#else
		return GetDesc() ? true : false;
#endif
	}

	virtual const char* GetName(char* szBuf);
	virtual const char* GetLevelResText(char* szBuf);

	virtual int GetLevelRes();

	virtual void FillItemDesc(MTextArea* pTextArea);
	static void FillItemDesc(MMatchItemDesc* pItemDesc, MTextArea* pTextArea, ZMyItemNode* pRentalNode);
	virtual void UpdateCharInfoText();
	virtual void UpdateCharacterView(ZCharacterView* pCharacterView);
};

// »óÁ¡,ÀåºñÃ¢À» À§ÇÑ Äù½ºÆ®¾ÆÀÌÅÛ ·¡ÆÛ
class ZShopEquipItem_Quest : public ZShopEquipItem
{
	MQuestItemDesc* m_pItemDesc;
public:
	virtual int GetItemID() const override
	{
		// Lấy m_nItemID thông qua m_pItemDesc
		return m_pItemDesc ? m_pItemDesc->m_nItemID : 0;
	}
	ZShopEquipItem_Quest(MQuestItemDesc* pDesc);
	ZShopEquipItemType GetType() { return ZSEIT_QUEST; }

	MQuestItemDesc* GetDesc() { return m_pItemDesc; }

	virtual MMatchItemSlotType GetSlotType() { return MMIST_NONE; }
	virtual bool CanEquip() { return false; }
	virtual bool CanSendAccount() { return false; }
	virtual bool CanSell() { return GetDesc() ? true : false; }

	virtual const char* GetName(char* szBuf);
	virtual const char* GetLevelResText(char* szBuf);

	virtual int GetLevelRes();

	virtual void FillItemDesc(MTextArea* pTextArea);
	virtual void UpdateCharInfoText();
};

// »óÁ¡À» À§ÇÑ ¼¼Æ®¾ÆÀÌÅÛ ·¡ÆÛ
class ZShopEquipItem_Set : public ZShopEquipItem
{
	// ¼¼Æ®´Â ¾îÂ÷ÇÇ »óÁ¡¿¡¼­¸¸ Á¸ÀçÇÏ´Â °ÍÀÌ¶ó¼­ MShopSetItemÀ» °¡Áö°í ÀÖ°ÔÇÔ
	MShopSetItem* m_pShopSetItem;
	int m_nLevelRes;
	int m_nSexRes;
	MMatchItemDesc m_sumSetSpec;	// ¼¼Æ® ±¸¼ºÇ°µéÀÇ ´É·Â ÇÕ»êÄ¡
public:
	ZShopEquipItem_Set(MShopSetItem* pSetItem);
	ZShopEquipItemType GetType() { return ZSEIT_SET; }

	virtual MMatchItemSlotType GetSlotType() { return MMIST_NONE; }
	virtual bool CanEquip() { return false; }		// ¼¼Æ®´Â »óÁ¡¿¡¼­¸¸ Á¸ÀçÇÏ´Â °ÍÀÌ¹Ç·Î
	virtual bool CanSendAccount() { return false; }
	virtual bool CanSell() { return false; }

	virtual const char* GetName(char* szBuf);
	virtual const char* GetLevelResText(char* szBuf);

	virtual int GetLevelRes();

	virtual void FillItemDesc(MTextArea* pTextArea);
	virtual void UpdateCharInfoText();
	virtual void UpdateCharacterView(ZCharacterView* pCharacterView);
};


// ¾ÆÀÌÅÛÀÇ ±¸ÀÔ µ¿ÀÛÀ» À§ÇÑ ÇÚµé·¯ ÀÎÅÍÆäÀÌ½º
class IShopEquipItemHandle_Purchase
{
public:
	virtual ~IShopEquipItemHandle_Purchase() {}
	virtual bool GetPrice(int& nItemType, int& out_nRentalHour, int& out_nPrice) = 0;
	virtual const char* GetPriceText(char* szBuf);

	void OpenSimpleConfirmDlg(ISimpleConfirmDlgDoneHandler* pDoneHandler);
	void OpenCountableConfirmDlg(const char* szItemName, MBitmap* pIconBmp, int nPrice, int nMax, IItemCountDlgDoneHandler* pDoneHandler);

	virtual void Buy() = 0;
};

class ZShopEquipItemHandle_PurchaseGamble : public IShopEquipItemHandle_Purchase, public ISimpleConfirmDlgDoneHandler
{
	ZShopEquipItem_Gamble* m_pItem;
public:
	ZShopEquipItemHandle_PurchaseGamble(ZShopEquipItem_Gamble* pItem);

	bool GetPrice( int& nItemType, int& out_nRentalHour, int& out_nPrice );
	virtual void Buy();

	virtual void OnDone(bool bOk);
};

class ZShopEquipItemHandle_PurchaseMatch : public IShopEquipItemHandle_Purchase, public ISimpleConfirmDlgDoneHandler, public IItemCountDlgDoneHandler
{
	ZShopEquipItem_Match* m_pItem;
public:
	ZShopEquipItemHandle_PurchaseMatch(ZShopEquipItem_Match* pItem);

	bool GetPrice( int& nItemType, int& out_nRentalHour, int& out_nPrice );
	virtual void Buy();

	virtual void OnDone(bool bOk);
	virtual void OnDone(int nCount);
};

class ZShopEquipItemHandle_PurchaseQuest : public IShopEquipItemHandle_Purchase, public IItemCountDlgDoneHandler
{
	ZShopEquipItem_Quest* m_pItem;
public:
	ZShopEquipItemHandle_PurchaseQuest(ZShopEquipItem_Quest* pItem);

	bool GetPrice( int& nItemType, int& out_nRentalHour, int& out_nPrice);
	virtual void Buy();

	virtual void OnDone(int nCount);
};

//class ZShopEquipItemHandle_PurchaseSet : public IShopEquipItemHandle_Purchase, public ISimpleConfirmDlgDoneHandler
//{
//	ZShopEquipItem_Set* m_pItem;
//public:
//	ZShopEquipItemHandle_PurchaseSet(ZShopEquipItem_Set* pItem);
//
//	virtual bool GetPrice(int& out_nPriceType, int& out_nRentalHour, int& out_nPrice) override;
//	virtual void Buy() override;
//	virtual void OnDone(bool bOk) override; // Callback Popup
//};


// ¾ÆÀÌÅÛÀÇ ÆÇ¸Å µ¿ÀÛÀ» À§ÇÑ ÇÚµé·¯ ÀÎÅÍÆäÀÌ½º
class IShopEquipItemHandle_Sell
{
	MUID m_uidItem;
public:
	IShopEquipItemHandle_Sell() : m_uidItem(0,0) {}
	virtual ~IShopEquipItemHandle_Sell() {}
	void SetItemUID(const MUID& uid) { m_uidItem = uid; }
	MUID GetItemUID() { return m_uidItem; }
	virtual bool GetPrice(int& out_nPrice) = 0;
	virtual const char* GetPriceText(char* szBuf);

	void OpenSimpleConfirmDlg(ISimpleConfirmDlgDoneHandler* pDoneHandler);

	void OpenCountableConfirmDlg(const char* szItemName, MBitmap* pIconBmp, int nPrice, int nMax, IItemCountDlgDoneHandler* pDoneHandler);

	void OpenSellCashItemConfirmDlg(const char* szItemName, MBitmap* pIcon, int price, int count, ISellCashItemConfirmDlgDoneHandler* pHandler);

	virtual void Sell() = 0;
};

class ZShopEquipItemHandle_SellGamble : public IShopEquipItemHandle_Sell, public IItemCountDlgDoneHandler
{
	ZShopEquipItem_Gamble* m_pItem;
public:
	ZShopEquipItemHandle_SellGamble(ZShopEquipItem_Gamble* pItem);
	virtual bool GetPrice(int& out_nPrice);
	virtual void Sell();

	virtual void OnDone(int nCount);
};

class ZShopEquipItemHandle_SellMatch : public IShopEquipItemHandle_Sell, public ISimpleConfirmDlgDoneHandler, public IItemCountDlgDoneHandler, public ISellCashItemConfirmDlgDoneHandler
{
	ZShopEquipItem_Match* m_pItem;
public:
	ZShopEquipItemHandle_SellMatch(ZShopEquipItem_Match* pItem);
	virtual bool GetPrice(int& out_nPrice);
	virtual void Sell();

	virtual void OnDone(bool bOk);
	virtual void OnDone(int nCount);
	virtual void OnConfirmSellCashItem(bool bOk);
};

class ZShopEquipItemHandle_SellQuest : public IShopEquipItemHandle_Sell, public IItemCountDlgDoneHandler
{
	ZShopEquipItem_Quest* m_pItem;
public:
	ZShopEquipItemHandle_SellQuest(ZShopEquipItem_Quest* pItem);
	virtual bool GetPrice(int& out_nPrice);
	virtual void Sell();

	virtual void OnDone(int nCount);
};


// ¾ÆÀÌÅÛÀÇ ÀºÇà³Ö±â µ¿ÀÛÀ» À§ÇÑ ÇÚµé·¯ ÀÎÅÍÆäÀÌ½º
class IShopEquipItemHandle_SendAccount
{
	MUID m_uidItem;
public:
	IShopEquipItemHandle_SendAccount() :  m_uidItem(0,0) {}
	virtual ~IShopEquipItemHandle_SendAccount() {}
	void SetItemUID(const MUID& uid) { m_uidItem = uid; }
	MUID GetItemUID() { return m_uidItem; }

	void OpenCountableConfirmDlg(const char* szItemName, MBitmap* pIconBmp, int nPrice, int nMax, IItemCountDlgDoneHandler* pDoneHandler);

	virtual void Send() = 0;
};

class ZShopEquipItemHandle_SendAccountGamble : public IShopEquipItemHandle_SendAccount, public IItemCountDlgDoneHandler
{
	ZShopEquipItem_Gamble* m_pItem;
public:
	ZShopEquipItemHandle_SendAccountGamble(ZShopEquipItem_Gamble* pItem);

	virtual void Send();
	virtual void OnDone(int nCount);
};

class ZShopEquipItemHandle_SendAccountMatch : public IShopEquipItemHandle_SendAccount, public IItemCountDlgDoneHandler
{
	ZShopEquipItem_Match* m_pItem;
public:
	ZShopEquipItemHandle_SendAccountMatch(ZShopEquipItem_Match* pItem);

	virtual void Send();
	virtual void OnDone(int nCount);
};



// ¾ÆÀÌÅÛÀÇ ÀºÇà²¨³»¿À±â µ¿ÀÛÀ» À§ÇÑ ÇÚµé·¯ ÀÎÅÍÆäÀÌ½º
class IShopEquipItemHandle_BringAccount
{
	int m_nAIID;
	ZMyItemNode* m_pMyItemNode;
public:
	IShopEquipItemHandle_BringAccount() : m_nAIID(0), m_pMyItemNode(NULL) {}
	virtual ~IShopEquipItemHandle_BringAccount() {}
	void SetAIID(int aiid) { m_nAIID = aiid; }
	int GetAIID() { return m_nAIID; }

	void SetMyItemNode(ZMyItemNode* pMyItemNode) { m_pMyItemNode = pMyItemNode; }
	ZMyItemNode* GetMyItemNode() { return m_pMyItemNode; }

	void OpenSimpleConfirmDlg(ISimpleConfirmDlgDoneHandler* pDoneHandler);
	void OpenCountableConfirmDlg(const char* szItemName, MBitmap* pIconBmp, int nPrice, int nMax, IItemCountDlgDoneHandler* pDoneHandler);

	virtual void Bring() = 0;

	// ÀºÇà¿¡ ÀÖ´Â °Ç Ä³½¬¾ÆÀÌÅÛ, Ä³½¬¾ÆÀÌÅÛÀÇ °¡°ÝÀº Ç¥½ÃÇÏÁö ¾Ê´Â´Ù GetPriceText() ÇÊ¿ä¾øÀ½
};

class ZShopEquipItemHandle_BringAccountGamble : public IShopEquipItemHandle_BringAccount, public IItemCountDlgDoneHandler
{
	ZShopEquipItem_Gamble* m_pItem;
public:
	ZShopEquipItemHandle_BringAccountGamble(ZShopEquipItem_Gamble* pItem);
	
	virtual void Bring();

	virtual void OnDone(int nCount);
};

class ZShopEquipItemHandle_BringAccountMatch : public IShopEquipItemHandle_BringAccount, public ISimpleConfirmDlgDoneHandler, public IItemCountDlgDoneHandler
{
	ZShopEquipItem_Match* m_pItem;
	ZMyItemNode* m_pMyItemNode;
public:
	ZShopEquipItemHandle_BringAccountMatch(ZShopEquipItem_Match* pItem);
	ZMyItemNode* GetMyItemNode() { return m_pMyItemNode; }
	virtual void Bring();

	virtual void OnDone(bool bOk);
	virtual void OnDone( int nCount );
};



