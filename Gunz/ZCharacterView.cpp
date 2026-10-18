#include "stdafx.h"

#include "ZCharacterView.h"
#include "RealSpace2.h"
#include "MMatchObject.h"
#include "ZApplication.h"
#include "ZMyInfo.h"
#include "ZGameClient.h"

extern bool Enable_Cloth;
static rvector force = rvector( 15, 0, 0 );
static float maxForce = 10;


char* szItemNames[MMCIP_END] = 
{
		"¸Ó¸®",
		"°¡½¿",
		"¼Õ",
		"´Ù¸®",
		"¹ß",
		"¿ÞÂÊ¼Õ°¡¶ô",
		"¿À¸¥ÂÊ¼Õ°¡¶ô",
		"±ÙÁ¢¹«±â",
		"ÁÖ¹«±â",
		"º¸Á¶¹«±â",
		"¾ÆÀÌÅÛ1",
		"¾ÆÀÌÅÛ2",
		"¾ÆÀÌÅÛ3",
};

unsigned long int GetVisualWeaponID(unsigned long int nMeleeItemID, unsigned long int nPrimaryItemID,
									unsigned long int nSecondaryItemID, unsigned long int nCustom1ItemID,
									unsigned long int nCustom2ItemID, unsigned long int nCustom3ItemID)
{
	if (nPrimaryItemID != 0) return nPrimaryItemID;
	else if (nSecondaryItemID != 0) return nSecondaryItemID;
	else if (nMeleeItemID != 0) return nMeleeItemID;
	else if (nCustom1ItemID != 0) return nCustom1ItemID;
	else if (nCustom2ItemID != 0) return nCustom2ItemID;
	else if (nCustom3ItemID != 0) return nCustom3ItemID;

	return 0;
}


void ZCharacterView::RepositionItemSlots(void)
{
	MRECT r = GetClientRect();

#define ARMORLIST_TOP_SPACE	10
#define ARMORLIST_BOTTOM_SPACE	50
#define LEFTITEM_COUNT	5
	int nItemHeight = (r.h-ARMORLIST_TOP_SPACE-ARMORLIST_BOTTOM_SPACE)/LEFTITEM_COUNT;
	int nItemWidth = nItemHeight;

#define ITEM_SPACE	6
	for(int i=0; i<LEFTITEM_COUNT; i++){
		MRECT ir(r.x+r.w-nItemWidth-ARMORLIST_TOP_SPACE, r.y+10+nItemHeight*i, nItemWidth, nItemHeight);
		ir.w-=ITEM_SPACE;
		ir.h-=ITEM_SPACE;
		m_ItemSlots[i].m_Rect = ir;
	}

	for(int i=0; i<MMCIP_END-LEFTITEM_COUNT; i++){
		MRECT ir(r.x+nItemWidth*(i<3?i:i-3)+ARMORLIST_TOP_SPACE, r.y+r.h-nItemHeight*(i<3?2:1), nItemWidth, nItemHeight);
		ir.w-=ITEM_SPACE;
		ir.h-=ITEM_SPACE;
		m_ItemSlots[i+LEFTITEM_COUNT].m_Rect = ir;
	}
}

void ZCharacterView::OnSize(int w, int h)
{
	RepositionItemSlots();
}

void ZCharacterView::OnDraw(MDrawContext* pDC)
{
	DWORD dwCurTime = timeGetTime();
	if ( !m_bLButtonDown && m_bAutoRotate && m_bEnableRotate)
		RotateRight( (float)( dwCurTime - m_dwTime) / 20.0f);

	m_dwTime = dwCurTime;


	if( !Enable_Cloth )
	{
		m_pTVisualMesh.m_pVisualMesh->DestroyCloth();
	}
	else if( m_pTVisualMesh.m_pVisualMesh->isChestClothMesh() )
	{
		srand( timeGetTime());
		int rint = rand() % 10;
		force.x += rint - 7;
		force.x = min(max( force.x, 5 ), maxForce * 0.3 );
		srand( timeGetTime());
		rint = rand() % (int)(maxForce*0.3);
		force.y += rint - 4;	
		force.y = min(max( force.y, 0 ), maxForce );
 		force.z = -90;

		m_pTVisualMesh.m_pVisualMesh->SetClothForce( force );

//		if( m_pTVisualMesh.m_pVisualMesh->GetClothMeshNodeRender() )
//			m_pTVisualMesh.m_pVisualMesh->SetClothMeshNodeRender(false);
	}

	//bool bGame = ZGetGame() ? true:false;	//< ¾îÂ÷ÇÇ ÀÌ°Ç UI´Ï±î °ÔÀÓÁßÀÌ°Ç ¾Æ´Ï°Ç bGame=false·Î °è»ê
	bool bGame = false;
	m_pTVisualMesh.m_pVisualMesh->SetClothValue(bGame,0.f); // cloth

	ZMeshView::OnDraw(pDC);

	if( m_pTVisualMesh.m_pVisualMesh->isChestClothMesh() )
	{
//		m_pTVisualMesh.m_pVisualMesh->UpdateCloth();
//		m_pTVisualMesh.m_pVisualMesh->RenderCloth();
	}

	if (GetDrawInfo() == false) 
		return;

	MRECT r; 
}

bool ZCharacterView::IsDropable(MWidget* pSender)
{
	return false;
}

bool ZCharacterView::OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	return false;
}

ZCharacterView::ZCharacterView(const char* szName, MWidget* pParent, MListener* pListener)
: ZMeshView(szName, pParent, pListener)
{
	// ZOrderº¯°æµÇ´Â ¹®Á¦·Î ÅøÆÁÀº ¿ì¼± º¸¿©ÁÖÁö ¾Ê´Â´Ù.
//	AttachToolTip(szName);
//	m_pTVisualMesh.SetVisualMesh();

	SetDrawInfo(false);

	m_fDist = 270.0f;//235.0f;
	m_fMaxDist = 400.0f;
	m_fMinDist = 120.0f;

	m_Eye = rvector(0.0f, 92, -m_fDist);
	m_At = rvector(0, 92, 0);
	m_Up = rvector(0, 1, 0);

	memset(&m_Info, 0, sizeof(ZCharacterViewInfo));

	m_bVisibleEquipment = false;
	m_nVisualWeaponParts = MMCIP_PRIMARY;


	m_bAutoRotate = false;
	m_dwTime = timeGetTime();


	for(int i=0; i<MMCIP_END; i++)
	{
		m_ItemSlots[i].m_nItemID = 0;
	}

	RepositionItemSlots();

	//jintriple3 ¸Þ¸ð¸® ÇÁ·Ï½Ã..
	m_Info.m_pMnTeam = new MProtectValue<int>;
	m_Info.m_pMnTeam->Set_MakeCrc(0);

	m_fCRot = 0.4f;
}

ZCharacterView::~ZCharacterView(void)
{
	delete m_Info.m_pMnTeam;
}

void ZCharacterView::InitCharParts(ZCharacterView* pCharView, MMatchCharItemParts nVisualWeaponParts, bool bShop)
{
	if(pCharView) {

		unsigned long int itemids[MMCIP_END];
		for (int i = 0; i < MMCIP_END; i++) itemids[i] = pCharView->m_ItemSlots[i].m_nItemID;

		InitCharParts(pCharView->m_Info.nSex, pCharView->m_Info.nHair, pCharView->m_Info.nFace,
						itemids, nVisualWeaponParts, bShop);
	}
}

void ZCharacterView::InitCharParts(MMatchSex nSex, unsigned int nHair, unsigned int nFace, 
				   const unsigned long int* nEquipItemIDs, MMatchCharItemParts nVisualWeaponParts, bool bShop)
{
	RMesh* pMesh = NULL;
	if (nSex == MMS_MALE) {
		pMesh = ZGetMeshMgr()->Get("heroman1");
	}
	else {
		pMesh = ZGetMeshMgr()->Get("herowoman1");
	}

	if (m_pTVisualMesh.GetVMesh(false)) {
		m_pTVisualMesh.Create();
		m_pTVisualMesh.GetVMesh()->Create(pMesh);
	}

	// Á¤º¸ ÀúÀå
	m_Info.nSex = nSex;
	m_Info.nHair = nHair;
	m_Info.nFace = nFace;

	for (int i = 0; i < MMCIP_END; i++)
	{
		m_ItemSlots[i].m_nItemID = nEquipItemIDs[i];
	}

	unsigned long int nWeaponID = 0;

	if (nVisualWeaponParts == MMCIP_PRIMARY) {
		nWeaponID= GetVisualWeaponID(nEquipItemIDs[MMCIP_MELEE], nEquipItemIDs[MMCIP_PRIMARY]
		, nEquipItemIDs[MMCIP_SECONDARY], nEquipItemIDs[MMCIP_CUSTOM1], nEquipItemIDs[MMCIP_CUSTOM2], nEquipItemIDs[MMCIP_CUSTOM3]);
	}
	else {
		nWeaponID = nEquipItemIDs[nVisualWeaponParts];
	}

	if (m_pTVisualMesh.GetVMesh())
	{
		// Custom: Avatar Clothing logic.
		if(!bShop)
		{
			ZChangeCharParts( m_pTVisualMesh.GetVMesh() , nSex, nHair, nFace, nEquipItemIDs );
		}
		else
		{
			ZChangeCharPartsNoAvatar( m_pTVisualMesh.GetVMesh() , nSex, nHair, nFace, nEquipItemIDs );
		}

		ZChangeCharWeaponMesh( m_pTVisualMesh.GetVMesh(), nWeaponID);

		//µé°í ÀÖ´Â ¹«±â¿¡ µû¶ó¼­ ¿¬°á..
		m_pTVisualMesh.GetVMesh()->SetAnimation("login_idle");
		m_pTVisualMesh.GetVMesh()->SetCheckViewFrustum(false);
	}

	if( Enable_Cloth )
	{
		m_pTVisualMesh.m_pVisualMesh->ChangeChestCloth(0.9f,1);
	}
}


void ZCharacterView::SetParts(MMatchCharItemParts nParts, unsigned int nItemID)
{
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc == NULL) return;
	if (IsSuitableItemSlot(pItemDesc->m_nSlot, nParts) == false) return;

	// ¼ºº° Á¦ÇÑ Á¶°Ç¸¸ Ã¼Å©ÇÑ´Ù.
	if (pItemDesc->m_nResSex.Ref() != -1)
	{
		if (pItemDesc->m_nResSex.Ref() != int(ZGetMyInfo()->GetSex())) return;
	}


	m_ItemSlots[nParts].m_nItemID = nItemID;

	if (nParts == MMCIP_PRIMARY)
	{
		ZChangeCharWeaponMesh(m_pTVisualMesh.GetVMesh(), m_ItemSlots[MMCIP_PRIMARY].m_nItemID);
	}
	else
	{
		unsigned long int itemids[MMCIP_END];
		for (int i = 0; i < MMCIP_END; i++) itemids[i] = m_ItemSlots[i].m_nItemID;
		ZChangeCharParts(m_pTVisualMesh.GetVMesh(), m_Info.nSex, m_Info.nHair, m_Info.nFace, itemids);

	}

	if( Enable_Cloth )
		m_pTVisualMesh.m_pVisualMesh->ChangeChestCloth(0.9f,1);
}

void ZCharacterView::ChangeVisualWeaponParts(MMatchCharItemParts nVisualWeaponParts)
{
	if (m_pTVisualMesh.GetVMesh() == NULL) return;

	unsigned long int itemids[MMCIP_END];
	for (int i = 0; i < MMCIP_END; i++) itemids[i] = m_ItemSlots[i].m_nItemID;

	unsigned long int nWeaponID;

	if (nVisualWeaponParts == MMCIP_PRIMARY)
	{
		nWeaponID= GetVisualWeaponID(itemids[MMCIP_MELEE],
			itemids[MMCIP_PRIMARY], itemids[MMCIP_SECONDARY],
			itemids[MMCIP_CUSTOM1], itemids[MMCIP_CUSTOM2], itemids[MMCIP_CUSTOM3]);
	}
	else
	{
		nWeaponID = itemids[nVisualWeaponParts];
	}

	ZChangeCharWeaponMesh(m_pTVisualMesh.GetVMesh(), nWeaponID);

	//µé°í ÀÖ´Â ¹«±â¿¡ µû¶ó¼­ ¿¬°á..

	m_pTVisualMesh.GetVMesh()->SetAnimation("login_idle");
	m_pTVisualMesh.GetVMesh()->SetCheckViewFrustum(false);

	m_nVisualWeaponParts = nVisualWeaponParts;
}

void ZCharacterView::SetSelectMyCharacter()
{
	ZMyInfo* pmi = ZGetMyInfo();
	ZMyItemList* pil = ZGetMyInfo()->GetItemList();

	unsigned long int nWeaponID = GetVisualWeaponID(pil->GetEquipedItemID(MMCIP_MELEE),
		pil->GetEquipedItemID(MMCIP_PRIMARY), pil->GetEquipedItemID(MMCIP_SECONDARY),
		pil->GetEquipedItemID(MMCIP_CUSTOM1), pil->GetEquipedItemID(MMCIP_CUSTOM2), pil->GetEquipedItemID(MMCIP_CUSTOM3));

	unsigned long int nItemids[MMCIP_END];

	if(pmi) 
	{
		for (int i = 0; i < MMCIP_END; i++)
		{
			nItemids[i] = pil->GetEquipedItemID(MMatchCharItemParts(i));
		}

		InitCharParts(pmi->GetSex(), pmi->GetHair(), pmi->GetFace(), nItemids);
	}
}

void ZCharacterView::OnInvalidate()
{
	if(m_pTVisualMesh.m_pVisualMesh) 
		m_pTVisualMesh.m_pVisualMesh->OnInvalidate();
}
void ZCharacterView::OnRestore()
{
	if(m_pTVisualMesh.m_pVisualMesh) 
		m_pTVisualMesh.m_pVisualMesh->OnRestore();
}

bool GetUserInfoUID(MUID uid,MCOLOR& _color,char* sp_name,MMatchUserGradeID& gid);

void ZCharacterView::SetCharacter( MUID uid  )
{
	if (m_Info.UID != uid)
	{
		m_Info.UID = uid;

		MMatchObjCacheMap* pObjCacheMap = ZGetGameClient()->GetObjCacheMap();
		for (MMatchObjCacheMap::iterator itor = pObjCacheMap->begin(); itor != pObjCacheMap->end(); ++itor)
		{
			MMatchObjCache* pObj = (*itor).second;
			if (pObj->GetUID() == m_Info.UID)
			{
				InitCharParts(pObj->GetCostume()->nSex, pObj->GetCostume()->nHair, pObj->GetCostume()->nFace,
					pObj->GetCostume()->nEquipedItemID);

				MCOLOR _color;
				char sp_name[255];
				MMatchUserGradeID gid;

				if (GetUserInfoUID(m_Info.UID, _color, sp_name, gid)) {//Æ¯º°ÇÑ À¯ÀúÀÎ °æ¿ì
					// Custom: Unmask names
					SetText(pObj->GetName());
					m_Info.nLevel = pObj->GetLevel();
					//SetText( sp_name );
					m_Info.nLevel = 0;//pObj->GetLevel();
				}
				else {
					SetText(pObj->GetName());
					m_Info.nLevel = pObj->GetLevel();
				}

				SetDrawInfo(true);
			}
		}
	}
}