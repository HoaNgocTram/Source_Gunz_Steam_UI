#include "stdafx.h"
#include "MMatchShop.h"
#include "MMatchConfig.h"
#include "MMatchGambleMachine.h"

MMatchShop::MMatchShop()
{

}
MMatchShop::~MMatchShop()
{

}
bool MMatchShop::Create(const char* szDescFileName)
{
	return ReadXml(szDescFileName);

	return true;
}


void MMatchShop::Destroy()
{
	Clear();
}


bool MMatchShop::ReadXml(const char* szFileName)
{
	MXmlDocument	xmlDocument;

	xmlDocument.Create();

	if (!xmlDocument.LoadFromFile(szFileName))
	{
		xmlDocument.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlDocument.GetDocumentElement();

	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, MTOK_SELL))
		{
			ParseSellItem(chrElement);
		}
	}

	xmlDocument.Destroy();

	return true;
}

bool yesOrNo(float probabilityOfYes) {
	return rand() % 100 < (probabilityOfYes * 100);
}
void MMatchShop::ParseSellItem(MXmlElement& element)
{
	ShopItemNode *pNewItemNode = new ShopItemNode;

	int nDescID = 0;
	int	nItemCount = 1;
	float fRatio = 0.f;

	if( nItemCount == 0 )	{ pNewItemNode->nItemCount = 1; } 
	else					{ pNewItemNode->nItemCount = nItemCount; }	

	element.GetAttribute(&nDescID, MTOK_SELL_ITEMID);
	element.GetAttribute(&pNewItemNode->nRentPeriodHour, MTOK_SELL_RENT_PERIOD_HOUR, 0);
	element.GetAttribute(&pNewItemNode->nMedalItem, MTOK_SELL_MEDAL_ITEM, 0);
	element.GetAttribute(&pNewItemNode->nEventItem, MTOK_SELL_EVENT_ITEM, 0);
	element.GetAttribute(&fRatio, MTOK_SELL_STOCK_RATE, 0.0);

	pNewItemNode->nItemID = nDescID;
	pNewItemNode->bIsRentItem = (pNewItemNode->nRentPeriodHour > 0);	
	pNewItemNode->bIsMedalItem = (pNewItemNode->nMedalItem > 0);
	pNewItemNode->bIsEventItem = (pNewItemNode->nEventItem > 0);
	pNewItemNode->fStockRate = fRatio;

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nDescID);
	if (pItemDesc != NULL)
	{
		if (pNewItemNode->bIsMedalItem)
		{
			if (yesOrNo(fRatio))
			{
				m_ItemNodeVector.push_back(pNewItemNode);
				m_ItemNodeMap.insert(map<unsigned int, ShopItemNode*>::value_type(pNewItemNode->nItemID, pNewItemNode));
			}
			else
			{

			}
		}
		else if (pNewItemNode->bIsEventItem)
		{
			if (yesOrNo(fRatio))
			{
				m_ItemNodeVector.push_back(pNewItemNode);
				m_ItemNodeMap.insert(map<unsigned int, ShopItemNode*>::value_type(pNewItemNode->nItemID, pNewItemNode));
			}
			else
			{

			}
		}
		else
		{
			m_ItemNodeVector.push_back(pNewItemNode);
			m_ItemNodeMap.insert(map<unsigned int, ShopItemNode*>::value_type(pNewItemNode->nItemID, pNewItemNode));
		}

	}
#ifdef _QUEST_ITEM
	else
	{
		if ( QuestTestServer() == false )
		{
			delete pNewItemNode;
			return;
		}

		//MatchItem에서 없을경우 QuestItem에서 다시 한번 검사를 함.
		MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc( nDescID );
		if( 0 == pQuestItemDesc )
		{
			delete pNewItemNode;
			return;
		}

		m_ItemNodeVector.push_back( pNewItemNode );
		m_ItemNodeMap.insert( map<unsigned int, ShopItemNode*>::value_type(pNewItemNode->nItemID, pNewItemNode) );
	}
#endif
}

void MMatchShop::Clear()
{
	int nVectorSize = (int)m_ItemNodeVector.size();
	for (int i = 0; i < nVectorSize; i++)
	{
		ShopItemNode* pNode = m_ItemNodeVector[i];
		delete pNode;
	}

	m_ItemNodeVector.clear();
	m_ItemNodeMap.clear();
}


MMatchShop* MMatchShop::GetInstance()
{
	static MMatchShop g_stMatchShop;
	return &g_stMatchShop;
}

bool MMatchShop::IsSellItem(const unsigned long int nItemID)
{
	map<unsigned int, ShopItemNode*>::iterator itor = m_ItemNodeMap.find(nItemID);
	if (itor != m_ItemNodeMap.end()) {
		return true;
	}

	return false;
}

ShopItemNode* MMatchShop::GetSellItemByIndex(int nListIndex)
{
	if ((nListIndex < 0) || (nListIndex >= GetCount())) return NULL;

	return m_ItemNodeVector[nListIndex];
}

ShopItemNode* MMatchShop::GetSellItemByItemID(int nItemID)
{
	map<unsigned int, ShopItemNode*>::iterator itor = m_ItemNodeMap.find(nItemID);
	if (itor == m_ItemNodeMap.end()) {
		return NULL;
	}

	return (ShopItemNode*)(itor->second);
}