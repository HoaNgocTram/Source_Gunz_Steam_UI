#include "stdafx.h"

#include "zapplication.h"
#include "MMatchObjCache.h"
#include ".\zplayerlistbox.h"
#include "MBitmap.h"
#include "MListBox.h"
#include "zpost.h"
#include "ZCharacterView.h"
#include "ZPlayerMenu.h"
#include "MToolTip.h"
#include "ZButton.h"
#include "ZMyInfo.h"


#define PLAYER_SLOT_GAP 1
#define STAGE_SLOT_ROW_COUNT	8	// for new UI
#define PLAYERLIST_ITEM_HEIGHT	23

#define COLORTEXT_SUPPORT // Custom: Added this to support colours in player list box

bool GetUserGradeIDColor(MMatchUserGradeID gid,MCOLOR& UserNameColor,char* sp_name);
bool GetUserInfoUID(MUID uid,MCOLOR& _color,char* sp_name,MMatchUserGradeID& gid);

void ZPlayerListBoxLook::OnItemDraw2(MDrawContext* pDC, MRECT& r, const char* szText, MCOLOR color, bool bSelected, bool bFocus, int nAdjustWidth)
{
	if(szText==NULL) return;

	pDC->SetColor(color);

	MRECT rtemp, rtemp2;
	rtemp2 = rtemp = pDC->GetClipRect();
	rtemp2.x	+= r.x;
	rtemp2.y	+= r.y;
	rtemp2.w	 = r.w;
	rtemp2.h	 = r.h;
	pDC->SetClipRect(rtemp2);
#ifdef COLORTEXT_SUPPORT
	// Custom: Centre playerlist colour
	pDC->TextMultiLine(MRECT(r.x, r.y+(r.h-pDC->GetFont()->GetHeight())/2, r.w, r.h), szText, 0, false);
	//pDC->TextMultiLine(r, szText,0,false);
#else
	pDC->Text(r.x, r.y+(r.h-pDC->GetFont()->GetHeight())/2, szText);
#endif
	pDC->SetClipRect(rtemp);
}


void ZPlayerListBoxLook::OnItemDraw2(MDrawContext* pDC, MRECT& r, MBitmap* pBitmap, bool bSelected, bool bFocus, int nAdjustWidth)
{
	if(pBitmap==NULL) return;

	MRECT rtemp, rtemp2;
	rtemp2 = rtemp = pDC->GetClipRect();
	rtemp2.w -= nAdjustWidth;
	pDC->SetClipRect(rtemp2);

	pDC->SetBitmap(pBitmap);
//	pDC->Draw(r.x, r.y);
	pDC->Draw(r, MRECT(0,0,pBitmap->GetWidth(),pBitmap->GetHeight()));

	pDC->SetClipRect(rtemp);
}

MRECT ZPlayerListBoxLook::GetClientRect(MListBox* pListBox, MRECT& r)
{
	return r;
}

float GetF(float _old,float _new)
{
	return _old/_new;
}

float GetF(float _new)
{
	return _new/800.f;
}


void ZPlayerListBoxLook::OnDraw(MListBox* pListBox, MDrawContext* pDC)
{
	((ZPlayerListBox*)pListBox)->UpdateList(((ZPlayerListBox*)pListBox)->GetPlayerListMode());

//	float fA = ((ZPlayerListBox*)pListBox)->OnReSize();
	int newW = RGetScreenWidth();
	float fA = GetF(newW);

	m_SelectedPlaneColor = MCOLOR(180,220,180);

	// for new UI danh sach phong cho dung hang cao hon va chia hai cot
	ZPlayerListBox::PLAYERLISTMODE pmStage = ((ZPlayerListBox*)pListBox)->GetPlayerListMode();
	bool bStageMode = (pmStage == ZPlayerListBox::PLAYERLISTMODE_STAGE
		|| pmStage == ZPlayerListBox::PLAYERLISTMODE_STAGE_FRIEND);

	int nItemHeight = (bStageMode ? 35 : 23) * fA;
	int nShowCount = 1;

	MRECT r = pListBox->GetClientRect();
	MPOINT pos = pListBox->GetPosition();

	int nHeaderHeight = 0;

	MRECT rr = pListBox->m_Rect;
	rr.x = 0;
	rr.y = 0;

	bool bShowClanCreateFrame = 
		((ZPlayerListBox*)pListBox)->GetMode()==ZPlayerListBox::PLAYERLISTMODE_CHANNEL_CLAN 
		&& !ZGetMyInfo()->IsClanJoined();

	// ¹ÙÅÁ±×·ÁÁÖ°í(µ¿È¯ÀÌ°¡ »èÁ¦...)
//	if(bShowClanCreateFrame) 
//		pDC->SetColor(32,32,32);
//	else
//		pDC->SetColor(60,60,60);
//	pDC->FillRectangle(rr.x,rr.y+1,rr.w,rr.h);

	pDC->SetColor(10,10,10);

	ZPlayerListBox::PLAYERLISTMODE pm = ((ZPlayerListBox*)pListBox)->GetPlayerListMode();

	int nMode = 0;// 0 : lobby 1 : stage

	if( pm == ZPlayerListBox::PLAYERLISTMODE_STAGE || pm == ZPlayerListBox::PLAYERLISTMODE_STAGE_FRIEND)
		nMode = 1;

	// Å×µÎ¸® ±×·ÁÁÖ°í

	/*
	MRECT cliprect = pDC->GetClipRect();//Àá½ÃÇ®¾îÁØ´Ù..

	pDC->SetClipRect(0,0,RGetScreenWidth(),RGetScreenHeight());

	pDC->SetColor(128,128,128);
	pDC->Rectangle( MRECT(rr.x,rr.y,rr.w,rr.h) );
	*/

	/*
	//¿À¸¥ÂÊ °ËÀº»öÀ¸·Î ±×·ÁÁÖ°í
	pDC->SetColor(0,0,0);
	pDC->FillRectangle(MRECT(rr.x+rr.w-23*fA,rr.y+1,23*fA,rr.h-2));

	// ¿À¸¥ÂÊ¿¡ ÀÌ¹ÌÁö ±×·ÁÁÖ°í

	if( ((ZPlayerListBox*)pListBox)->GetBitmap()) {
		MBitmap* pBitmap = ((ZPlayerListBox*)pListBox)->GetBitmap();

		pDC->SetBitmap(pBitmap);
		pDC->Draw(MRECT(rr.x+rr.w-pBitmap->GetWidth()*fA,rr.y+1,pBitmap->GetWidth()*fA,pBitmap->GetHeight()*fA), 
			MRECT(0,0,pBitmap->GetWidth(),pBitmap->GetHeight()));
	}
	*/

//	pDC->SetClipRect(cliprect);

/*	if(!bShowClanCreateFrame)
	{
		pDC->SetColor(0,0,0);
		for(int i=0; i<pListBox->GetShowItemCount(); i++) {
			MPOINT p;
			p.x = r.x;
			p.y = r.y+nItemHeight*i+1;
			pDC->HLine(1,p.y+nItemHeight-1,rr.w-1);
		}
	}
*/

// for new UI: phong cho dung font nho hon, lobby giu nguyen
	MFont* pPrevFont = pDC->GetFont();
	if (bStageMode)
	{
		MFont* pStageFont = MFontManager::Get("FONTa7");
		if (pStageFont) pDC->SetFont(pStageFont);
	}

	MBitmap* pBaseBmp = NULL;

	// for new UI hai cot cho phong cho, mot cot cho lobby
	ZPlayerListBox* pZLB = (ZPlayerListBox*)pListBox;
	pZLB->m_ItemDrawRects.assign(pListBox->GetCount(), MRECT(0, 0, 0, 0));

	int nColCount = bStageMode ? 2 : 1;
	int nColWidth = r.w / nColCount;

	// for new UI: phong cho co so hang co dinh, o cao theo khung chu khong theo ITEMHEIGHT
	int nRowsPerCol;
	if (bStageMode)
	{
		// o luon cao bang 1/8 khung du phong it nguoi, chi so hang ve la thay doi
		nItemHeight = (r.h - nHeaderHeight) / STAGE_SLOT_ROW_COUNT;

		nRowsPerCol = ZGetGameClient()->GetMatchStageSetting()->GetMaxPlayers() / 2;
		if (nRowsPerCol < 1) nRowsPerCol = 1;
		if (nRowsPerCol > STAGE_SLOT_ROW_COUNT) nRowsPerCol = STAGE_SLOT_ROW_COUNT;
	}
	else
	{
		nRowsPerCol = (r.h - nHeaderHeight) / nItemHeight;
	}
	if (nRowsPerCol < 1) nRowsPerCol = 1;
	if (nItemHeight < 1) nItemHeight = 1;

	int nRowLeft = 0, nRowRight = 0;		// bat dau tu 1, giu nguyen khoang chua dau danh sach

	// for new UI: le cua o phai co gian theo do phan giai, khong thi khe ho lech voi hai vach mau
	int nCellPad = (int)(2 * fA);
	if (nCellPad < 1) nCellPad = 1;

	// for new UI: luoi ve truoc item, ca hai cot va moi che do, ke ca o co nguoi
	if (bStageMode)
	{
		for (int c = 0; c < nColCount; c++)
		{
			for (int k = 0; k < nRowsPerCol; k++)
			{
				int nGx = r.x + c * nColWidth;
				int nGy = r.y + nHeaderHeight + nItemHeight * k;

				MCOLOR cPrevGrid = pDC->SetColor(MCOLOR(255, 255, 255, 8));
				pDC->FillRectangle(nGx + nCellPad, nGy + nCellPad, nColWidth - nCellPad * 2, nItemHeight - nCellPad * 2);
				pDC->SetColor(MCOLOR(255, 255, 255, 20));
				pDC->Rectangle(nGx + nCellPad, nGy + nCellPad, nColWidth - nCellPad * 2, nItemHeight - nCellPad * 2);
				pDC->SetColor(cPrevGrid);
			}
		}
	}

	for (int i = pListBox->GetStartItem(); i < pListBox->GetCount(); i++) {

		MListItem* pItemForCol = pListBox->Get(i);

		int nCol = 0, nRow = nShowCount;
		if (nColCount == 2)
		{
			int nTeam = ((ZStagePlayerListItem*)pItemForCol)->m_nTeam;
			if (nTeam == 1) nCol = 0;		// do  -> cot trai
			else if (nTeam == 2) nCol = 1;		// xanh -> cot phai
			else                 nCol = (nRowLeft + nRowRight) % 2;	// khong doi -> xen ke

			if (nCol == 0) nRow = nRowLeft++;
			else           nRow = nRowRight++;

			if (nRow >= nRowsPerCol) continue;	// cot day roi thi bo qua
		}

		MRECT rItem;
		rItem.x = r.x + nCol * nColWidth;
		rItem.y = r.y;
		rItem.w = nColWidth;
		rItem.h = r.h;

		MPOINT p;
		p.x = rItem.x;
		p.y = r.y + nHeaderHeight + nItemHeight * nRow;

		// for new UI : ghi lai khung thuc te cua dong nay
		if (i >= 0 && i < (int)pZLB->m_ItemDrawRects.size())
			pZLB->m_ItemDrawRects[i] = MRECT(p.x, p.y, rItem.w, nItemHeight);

		// for new UI: o co nguoi dam hon o trong mot chut, ve truoc chu va icon
		if (bStageMode)
		{
			MCOLOR cPrevRow = pDC->SetColor(MCOLOR(255, 255, 255, 18));
			pDC->FillRectangle(p.x + nCellPad, p.y + nCellPad, rItem.w - nCellPad * 2, nItemHeight - nCellPad * 2);
			pDC->SetColor(cPrevRow);
		}
//		p.x = 0;
//		p.y = 0+nHeaderHeight+nItemHeight*nShowCount;

//		mlog(" ------- p.y = %d %d \n",p.y,nItemHeight);

		MListItem* pItem = pListBox->Get(i);
		bool bSelected = pItem->m_bSelected;
//		bool bSelected = (pListBox->IsSelected()) ? (pListBox->GetSelIndex()==i) : false;
		bool bFocused = (pListBox->IsFocus());

		int nFieldStartX = 0;

		// for new UI: chua cho vuong mien va chu trang thai o mep phai
		int nContentW = rItem.w;
		if (bStageMode) nContentW = rItem.w - (int)(95 * fA);

//		if(bSelected && bFocused) {//¼±ÅÃµÈ¾ÆÀÌÅÛÇ¥½Ã
		if(bSelected) {
//			pDC->SetColor(109,207,246);
//			pDC->FillRectangle(MRECT(p.x,p.y+1,r.x+r.w,nItemHeight-1));
			pDC->SetColor(130,130,130);
			pDC->Rectangle(MRECT(p.x + 2, p.y + 5, rItem.x + rItem.w - 8, nItemHeight - 4));	// for new UI
		}

		for (int j = 0; j < max(pListBox->GetFieldCount(), 1); j++) {

			// for new UI: phong cho - bo o level
			if (bStageMode && j == 1) continue;

			// for new UI: o trang thai (chu phong / san sang / doi) chuyen sang mep phai
			if (bStageMode && j == 0)
			{
				ZStagePlayerListItem* pSItem = (ZStagePlayerListItem*)pItem;

				const char* szStatus = pSItem->m_szStatus;
				if (ZGetGameClient()->IsForcedEntry()
					&& !(pSItem->m_PlayerUID == ZGetMyUID()))
				{
					szStatus = "In Game";
				}

				// chu trang thai, can phai
				int nStatusW = (int)(60 * fA);
				MRECT rst(rItem.x + rItem.w - nStatusW - (int)(8 * fA), p.y + 5,
					nStatusW, nItemHeight);
				OnItemDraw2(pDC, rst, szStatus, MCOLOR(200, 200, 200), bSelected, bFocused, 0);

				// vuong mien cho chu phong, dat truoc chu trang thai
				if (pSItem->m_bMaster)
				{
					MBitmap* pCrown = MBitmapManager::Get("RoomManagerIcon.png");
					if (pCrown)
					{
						int nCw = (int)(12 * fA), nCh = (int)(10 * fA);
						MRECT rc(rst.x - nCw - (int)(4 * fA),
							p.y + (nItemHeight - nCh) / 2, nCw, nCh);
						OnItemDraw2(pDC, rc, pCrown, bSelected, bFocused, 0);
					}
				}
				continue;
			}

			// for new UI
			int nTabSize = nContentW;
			if(j<pListBox->GetFieldCount()) nTabSize = pListBox->GetField(j)->nTabSize;

			int nWidth = min(nTabSize, nContentW - nFieldStartX);
			if (pListBox->m_bAbsoulteTabSpacing == false) nWidth = nContentW * nTabSize / 100;

			// phong cho - be rong rieng cho tung o, chua cho vuong mien va trang thai
			if (bStageMode)
			{
				switch (j)
				{
				case 2:  nWidth = rItem.w * 7 / 100; break;	// icon hang
				case 3:  nWidth = rItem.w * 32 / 100; break;	// ten
				case 4:  nWidth = (pItem->GetBitmap(4) != NULL) ? rItem.w * 5 / 100 : 0; break;
				case 5:  nWidth = rItem.w * 24 / 100; break;	// ten clan
				}
			}

			int nAdjustWidth = 0;

			if(pListBox->GetScrollBar()->IsVisible()){
				nAdjustWidth = pListBox->GetScrollBar()->GetRect().w + pListBox->GetScrollBar()->GetRect().w/2;
			}

//			MRECT ir(p.x+nFieldStartX+4+5, p.y+4, nWidth-8, nItemHeight-8);
//			MRECT irt(p.x+nFieldStartX+2+5, p.y+2, nWidth, nItemHeight);
			MRECT ir(p.x+nFieldStartX+5, p.y+7, nWidth-7, nItemHeight-7);			// ´Ù½Ã ±×¸²(µ¿È¯)
			MRECT irt(p.x+nFieldStartX, p.y+5, nWidth, nItemHeight);

			// for new UI: phong cho ve icon vuong, cao co dinh, can giua theo chieu doc
			if (bStageMode)
			{
				int nIconSize = (int)(16 * fA);
				if (nIconSize > nItemHeight - 4) nIconSize = nItemHeight - 4;
				if (nIconSize > nWidth)          nIconSize = nWidth;

				// huy hieu clan sat phai o cua no, de luon dinh canh ten clan
				ir.x = p.x + nFieldStartX + (nWidth - nIconSize) / 2;
				ir.y = p.y + (nItemHeight - nIconSize) / 2;
				ir.w = nIconSize;
				ir.h = nIconSize;
			}

			const char* szText = pItem->GetString(j);
			MBitmap* pBitmap = pItem->GetBitmap(j);
			MCOLOR color = pItem->GetColor();


			if(pBitmap!=NULL)//ÀÌ¹ÌÁö ±×¸®°í
				OnItemDraw2(pDC, ir, pBitmap,  bSelected, bFocused, nAdjustWidth);

			if(szText!=NULL)// ÅØ½ºÆ® ±×¸®°í
			{
//				if(bSelected && bFocused)
//				if(bSelected)
//					color = MCOLOR(0,0,0);

				OnItemDraw2(pDC, irt, szText, color, bSelected, bFocused, nAdjustWidth);
			}

			nFieldStartX += nWidth;
			if (nFieldStartX >= nContentW) break;	// for new UI
		}

		// for new UI
		nShowCount++;

		if (nColCount == 2)
		{
			if (nRowLeft > nRowsPerCol && nRowRight > nRowsPerCol) break;
		}
		else if (nShowCount >= pListBox->GetShowItemCount()) break;
			}

			// o trong cua che do team - ve nut tron de bam doi doi
			pZLB->m_TeamZoneRed = MRECT(0, 0, 0, 0);
			pZLB->m_TeamZoneBlue = MRECT(0, 0, 0, 0);

			if (bStageMode && nColCount == 2)
			{
				const MSTAGE_SETTING_NODE* pSet = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting();
				if (ZGetGameTypeManager()->IsTeamGame(pSet->nGameType))
				{
					MBitmap* pArrowL = MBitmapManager::Get("SlotStepper_Left.png");
					MBitmap* pArrowR = MBitmapManager::Get("SlotStepper_Right.png");
					MBitmap* pArrowLOver = MBitmapManager::Get("SlotStepper_Left_Over.png");
					MBitmap* pArrowROver = MBitmapManager::Get("SlotStepper_Right_Over.png");

					// vi tri chuot doi ve toa do cua listbox de biet o nao dang duoc tro vao
					MPOINT ptCursor = pZLB->m_ptMouse;

					// lam mo nut tron
					unsigned char nPrevOpacity = pDC->SetOpacity(80);

					for (int c = 0; c < 2; c++)
					{
						int nUsed = (c == 0) ? nRowLeft : nRowRight;
						if (nUsed >= nRowsPerCol) continue;

						int nX = r.x + c * nColWidth;
						int nY = r.y + nHeaderHeight + nItemHeight * nUsed;

						MRECT rz(nX, nY, nColWidth, nItemHeight * (nRowsPerCol - nUsed));
						if (c == 0) pZLB->m_TeamZoneRed = rz;
						else        pZLB->m_TeamZoneBlue = rz;

						MBitmap* pArrow = (c == 0) ? pArrowL : pArrowR;
						if (pArrow == NULL) continue;

						for (int k = nUsed; k < nRowsPerCol; k++)
						{
							int nRy = r.y + nHeaderHeight + nItemHeight * k;

							// giu ti le vuong theo anh goc 33x33
							int nBw = (int)(12 * fA);
							if (nBw > nItemHeight - 6) nBw = nItemHeight - 6;
							if (nBw < 1) nBw = 1;

							bool bHover = (ptCursor.x >= nX && ptCursor.x < nX + nColWidth &&
								ptCursor.y >= nRy && ptCursor.y < nRy + nItemHeight);

							if (bHover)
							{
								MCOLOR cPrevHover = pDC->SetColor(MCOLOR(255, 255, 255, 26));
								pDC->FillRectangle(nX + nCellPad, nRy + nCellPad, nColWidth - nCellPad * 2, nItemHeight - nCellPad * 2);
								pDC->SetColor(cPrevHover);
							}

							MBitmap* pDraw = pArrow;
							if (bHover)
							{
								MBitmap* pOver = (c == 0) ? pArrowLOver : pArrowROver;
								if (pOver != NULL) pDraw = pOver;
							}

							MRECT ra(nX + nColWidth / 2 - nBw / 2,
								nRy + (nItemHeight - nBw) / 2,
								nBw, nBw);

							pDC->SetOpacity(bHover ? 255 : 80);
							OnItemDraw2(pDC, ra, pDraw, false, false, 0);
						}
					}

					pDC->SetOpacity(nPrevOpacity);
				}
			}

			if (bStageMode && pPrevFont) pDC->SetFont(pPrevFont);
}

//////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_LOOK(ZPlayerListBox, ZPlayerListBoxLook)

ZPlayerListBox::ZPlayerListBox(const char* szName, MWidget* pParent, MListener* pListener)
: MListBox(szName, pParent, pListener)
{
	LOOK_IN_CONSTRUCTOR()
/*
	if(strcmp(szName,"StagePlayerList_")==0) {
		m_nMode = PLAYERLISTMODE_STAGE;
	}
	else 
		m_nMode = PLAYERLISTMODE_CHANNEL;
*/
	SetVisibleHeader(false);

	m_bAbsoulteTabSpacing = true;

//	m_MyUID = MUID(0,0);
//	m_uidChannel = MUID(0,0);
	m_nTotalPlayerCount = 0;
	m_nPage = 0;

	m_bVisible = true;
	m_bVisibleHeader = true;

//	AddField(" ", 23);//icon
//	AddField(" ",130);//name
//	AddField(" ", 23);//icon
//	AddField(" ", 32);//level

	SetItemHeight(PLAYERLIST_ITEM_HEIGHT);

	m_ptMouse = MPOINT(-1, -1);	// for new UI

//	m_pBitmap = NULL;
//	m_pBitmapIn = NULL;
	
	mSelectedPlayer = 0;
	mStartToDisplay = 0;
//	mPlayerOrder.reserve( sizeof(MUID)*100 );
//	m_pScrollBar = new MScrollBar( this, this );
//	ZApplication::GetGameInterface()->GetIDLResource()->InsertWidget("PlayerListScrollBar", m_pScrollBar );
	m_bAlwaysVisibleScrollbar = false;
	m_bHideScrollBar = true;
	m_pScrollBar->SetVisible(false);
	m_pScrollBar->Enable(false);

	m_pScrollBar->m_nDebugType = 3;
	m_nDebugType = 2;

	m_nOldW = RGetScreenWidth();

	SetListener(this);

	m_pButton = new ZBmButton(NULL,this,this);
	m_pButton->SetStretch(true);

	m_nMode = PLAYERLISTMODE_CHANNEL;
	InitUI(m_nMode);
}

ZPlayerListBox::~ZPlayerListBox(void)
{
	SAFE_DELETE(m_pButton);
}

void ZPlayerListBox::SetupButton(const char *szOn, const char *szOff)
{
	m_pButton->SetUpBitmap(MBitmapManager::Get(szOff));
	m_pButton->SetDownBitmap(MBitmapManager::Get(szOn));
	m_pButton->SetOverBitmap(MBitmapManager::Get(szOff));
}

//void ZPlayerListBox::InitUI(PLAYERLISTMODE nMode)
//{
//#ifndef _SYSINTERNEW
//	int newW = RGetScreenWidth();
//	float fA = GetF(newW);
//
//	m_nMode = nMode;
//	_ASSERT(nMode>=0 && nMode<PLAYERLISTMODE_END);
//
//	RemoveAllField();
//
//	const int nFields[PLAYERLISTMODE_END] = { 6,6,2,2,4,4 }; 
//	for(int i=0;i<nFields[nMode];i++) {
//		AddField("",10);
//	}
//	OnSize(0,0);
//
//	bool bShowClanCreateFrame = false;
//
//	switch(m_nMode) {
//		case PLAYERLISTMODE_CHANNEL:	
//			SetupButton("pltab_lobby_on.png", "pltab_lobby_off.png");
//			break;	
//		case PLAYERLISTMODE_STAGE: 	
//			SetupButton("pltab_game_on.png", "pltab_game_off.png");
//			break;
//		case PLAYERLISTMODE_CHANNEL_FRIEND:
//		case PLAYERLISTMODE_STAGE_FRIEND: 	
//			SetupButton("pltab_friends_on.png", "pltab_friends_off.png");
//			break;	
//		case PLAYERLISTMODE_CHANNEL_CLAN:
//		case PLAYERLISTMODE_STAGE_CLAN: 	
//		{
//			SetupButton("pltab_clan_on.png", "pltab_clan_off.png");
//			bShowClanCreateFrame = !ZGetMyInfo()->IsClanJoined();
//		}break;
//	}
//
//	MWidget *pFrame = ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyPlayerListClanCreateFrame");
//	MButton* pButtonUp = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerListPrev");
//	MButton* pButtonDn = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerListNext");
//	if( pFrame)
//	{
//		pFrame->Show(bShowClanCreateFrame);
//		pButtonUp->Show(!bShowClanCreateFrame);
//		pButtonDn->Show(!bShowClanCreateFrame);
//	}
//#else
//	int newW = RGetScreenWidth();
//	float fA = GetF(newW);
//
//	m_nMode = nMode;
//	_ASSERT(nMode >= 0 && nMode < PLAYERLISTMODE_END);
//
//	RemoveAllField();
//
//	const int nFields[PLAYERLISTMODE_END] = { 6,6,2,2,4,4 };
//	for (int i = 0; i < nFields[nMode]; i++) {
//		AddField("", 10);	
//	}
//	OnSize(0, 0);
//
//	bool bShowClanCreateFrame = false;
//
//	//switch (m_nMode) 
//	//{
//	//case PLAYERLISTMODE_CHANNEL:	SetupButton("pltab_lobby_on.tga", "pltab_lobby_off.tga"); break;
//
//	//case PLAYERLISTMODE_STAGE: 		SetupButton("pltab_game_on.tga", "pltab_game_off.tga"); break;
//
//	//case PLAYERLISTMODE_CHANNEL_FRIEND:
//	//case PLAYERLISTMODE_STAGE_FRIEND: 	SetupButton("pltab_friends_on.tga", "pltab_friends_off.tga"); break;
//
//	//case PLAYERLISTMODE_CHANNEL_CLAN:
//	//case PLAYERLISTMODE_STAGE_CLAN:
//	//{
//	//	SetupButton("pltab_clan_on.tga", "pltab_clan_off.tga");
//	//	bShowClanCreateFrame = !ZGetMyInfo()->IsClanJoined();
//	//}
//	//break;
//	switch (m_nMode)
//	{
//	case PLAYERLISTMODE_CHANNEL:	SetupButton("LobbyTab_Lobby.png", "LobbyTab_Lobby.png"); break;
//
//	case PLAYERLISTMODE_STAGE: 		SetupButton("pltab_game_on.tga", "pltab_game_off.tga"); break;
//
//	case PLAYERLISTMODE_CHANNEL_FRIEND:
//	case PLAYERLISTMODE_STAGE_FRIEND: 	SetupButton("LobbyTab_Friends.png", "LobbyTab_Friends.png"); break;
//
//	case PLAYERLISTMODE_CHANNEL_CLAN:
//	case PLAYERLISTMODE_STAGE_CLAN:
//	{
//		SetupButton("LobbyTab_Clan.png", "LobbyTab_Clan.png");
//		bShowClanCreateFrame = !ZGetMyInfo()->IsClanJoined();
//	}
//	break;
//	}
//
//	MWidget* pFrame = ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyPlayerListClanCreateFrame");
//	MButton* pButtonUp = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerListPrev");
//	MButton* pButtonDn = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerListNext");
//	if (pFrame)
//	{
//		pFrame->Show(bShowClanCreateFrame);
//		pButtonUp->Show(!bShowClanCreateFrame);
//		pButtonDn->Show(!bShowClanCreateFrame);
//	}
//#endif
//}
void ZPlayerListBox::InitUI(PLAYERLISTMODE nMode)
{
	int newW = RGetScreenWidth();
	float fA = GetF(newW);

	m_nMode = nMode;
	_ASSERT(nMode >= 0 && nMode < PLAYERLISTMODE_END);

	RemoveAllField();

	const int nFields[PLAYERLISTMODE_END] = { 6,6,2,2,4,4 };
	for (int i = 0; i < nFields[nMode]; i++) {
		AddField("", 10);
	}
	OnSize(0, 0);

	bool bShowClanCreateFrame = false;

	if (m_nMode == PLAYERLISTMODE_CHANNEL_CLAN || m_nMode == PLAYERLISTMODE_STAGE_CLAN)
	{
		bShowClanCreateFrame = !ZGetMyInfo()->IsClanJoined();
	}

	MWidget* pFrame = ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyPlayerListClanCreateFrame");
	MButton* pButtonUp = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerListPrev");
	MButton* pButtonDn = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerListNext");

	if (pFrame)
	{
		pFrame->Show(bShowClanCreateFrame);
		if (pButtonUp) pButtonUp->Show(!bShowClanCreateFrame);
		if (pButtonDn) pButtonDn->Show(!bShowClanCreateFrame);
	}
}

void ZPlayerListBox::RefreshUI()
{
	InitUI(GetMode());
}

void ZPlayerListBox::SetMode(PLAYERLISTMODE nMode)
{
	ZPlayerListBox* pWidget = (ZPlayerListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
	if(pWidget) {
		pWidget->RemoveAll();
	}

	// mlog("ZPlayerListBox::SetMode %d \n" , nMode);

	InitUI(nMode);

	if(!ZGetGameClient()) return;
	if(nMode==PLAYERLISTMODE_CHANNEL) {
		if (ZGetGameClient()->IsConnected()) {
			//UpdateList(nMode);
			if(pWidget) {
				int nPage = pWidget->m_nPage;
				ZPostRequestChannelPlayerList(ZGetGameClient()->GetPlayerUID(),ZGetGameClient()->GetChannelUID(),nPage);
			}
		}
	}
	else if(nMode==PLAYERLISTMODE_STAGE) {
		if (ZGetGameClient()->IsConnected())
			ZPostRequestStagePlayerList(ZGetGameClient()->GetStageUID());
	}
	else if(nMode==PLAYERLISTMODE_CHANNEL_FRIEND || nMode==PLAYERLISTMODE_STAGE_FRIEND) {
		if (ZGetGameClient()->IsConnected())
			ZPostFriendList();
	}
	else if (nMode==PLAYERLISTMODE_CHANNEL_CLAN || nMode==PLAYERLISTMODE_STAGE_CLAN) {
		if (ZGetGameClient()->IsConnected())
			ZPostRequestClanMemberList(ZGetGameClient()->GetPlayerUID());
	}
}
/*
void ZPlayerListBox::SetBitmap( MBitmap* pBitmap)
{
	m_pBitmap	= pBitmap;
}
*/

void ZPlayerListBox::AddTestItems()
{
/*
	switch (m_nMode)
	{
	case PLAYERLISTMODE_CHANNEL:
		AddPlayer(MUID(0,0),PS_FIGHT,34,"¾ÆÀÌµð6±ÛÀÚ1","Å¬·£¸í¿©¼¸ÀÚ",24401,MMUG_FREE,1);
		AddPlayer(MUID(0,1),PS_WAIT ,10,"¾ÆÀÌµð6±ÛÀÚ2","Å¬·£¸í¿©¼¸ÀÚ",24401,MMUG_FREE,2);
		AddPlayer(MUID(0,2),PS_LOBBY,11,"¾ÆÀÌµð6±ÛÀÚ3","Å¬·£¸í¿©¼¸ÀÚ",24401,MMUG_FREE,3);
		AddPlayer(MUID(0,3),PS_LOBBY,99,"¾ÆÀÌµð6±ÛÀÚ4","Å¬·£¸í¿©¼¸ÀÚ",24401,MMUG_FREE,4);
		AddPlayer(MUID(0,4),PS_LOBBY, 3,"¾ÆÀÌµð6±ÛÀÚ5","Å¬·£¸í¿©¼¸ÀÚ",24401,MMUG_FREE,5);
		AddPlayer(MUID(0,5),PS_LOBBY, 1,"¾ÆÀÌµð6±ÛÀÚ6","Å¬·£¸í¿©¼¸ÀÚ",24401,MMUG_FREE,6);
		AddPlayer(MUID(0,6),PS_LOBBY,33,"¾ÆÀÌµð6±ÛÀÚ7","Å¬·£¸í¿©¼¸ÀÚ",24401,MMUG_FREE,7);
		AddPlayer(MUID(0,7),PS_LOBBY,45,"¾ÆÀÌµð6±ÛÀÚ8","Å¬·£¸í¿©¼¸ÀÚ",24401,MMUG_FREE,8);
		AddPlayer(MUID(0,8),PS_LOBBY,44,"¾ÆÀÌµð6±ÛÀÚ9","Å¬·£¸í¿©¼¸ÀÚ",24401,MMUG_FREE,9);
		break;
	}
//*/
}

void GetRectMul(MRECT* rect,MRECT* org_rect,float f)
{
	rect->x = org_rect->x * f;
	rect->y = org_rect->y * f;
	rect->w = org_rect->w * f;
	rect->h = org_rect->h * f;
}

void ZPlayerListBox::OnSize(int w,int h)
{
	if(m_Fields.GetCount()==0) return;

	int newW = RGetScreenWidth();
	float fA = GetF(newW);

	m_nItemHeight = PLAYERLIST_ITEM_HEIGHT * fA;

//	m_pButton->SetBounds(1,1,m_Rect.w*.8f,m_nItemHeight+1);
	//m_pButton->SetBounds(0, 0, m_Rect.w, (int)(28.0/*ÀÌ¹ÌÁö ³ôÀÌ°¡ 28ÀÌ¶ó¼­...*/*fA));		// ´Ù½Ã ±×¸²(µ¿È¯)
	if (m_pButton) {
		m_pButton->SetBounds(0, 0, 0, 0);
		m_pButton->Enable(false);
	}

	switch(m_nMode) {
	case PLAYERLISTMODE_CHANNEL:
		{
			m_Fields.Get(0)->nTabSize = 23*fA;	//icon
			m_Fields.Get(1)->nTabSize = 16*fA;	//level
			m_Fields.Get(2)->nTabSize = 23*fA;	//icon (duel tournament grade)
			m_Fields.Get(3)->nTabSize = 94*fA;	//name
			m_Fields.Get(4)->nTabSize = 23*fA;	//icon
			m_Fields.Get(5)->nTabSize = 94*fA;	//clan name
		}
		break;
	case PLAYERLISTMODE_STAGE:
		{
			m_Fields.Get(0)->nTabSize = 23*fA;	//icon
			m_Fields.Get(1)->nTabSize = 16*fA;	//level
			m_Fields.Get(2)->nTabSize = 23*fA;	//icon (duel tournament grade)
			m_Fields.Get(3)->nTabSize = 94*fA;	//name
			m_Fields.Get(4)->nTabSize = 23*fA;	//icon
			m_Fields.Get(5)->nTabSize = 94*fA;	//clan name
		}
		break;
	case PLAYERLISTMODE_STAGE_FRIEND:
	case PLAYERLISTMODE_CHANNEL_FRIEND:
		{
			m_Fields.Get(0)->nTabSize = 23*fA;	//icon
			m_Fields.Get(1)->nTabSize = 72*fA;	//name
		}
		break;
	case PLAYERLISTMODE_CHANNEL_CLAN:
	case PLAYERLISTMODE_STAGE_CLAN:
		{
			m_Fields.Get(0)->nTabSize = 23*fA;	//icon
			m_Fields.Get(1)->nTabSize = 85*fA;	//name
			m_Fields.Get(2)->nTabSize = 23*fA;	//icon
			m_Fields.Get(3)->nTabSize = 90*fA;	//clan grade
		}
		break;
	};
	RecalcList();
}

// mode PLAYERLISTMODE_CHANNEL
void ZPlayerListBox::AddPlayer(MUID& puid, ePlayerState state, int  nLevel, char* szName, char* szClanName, unsigned int nClanID, MMatchUserGradeID nGrade, int duelTournamentGrade, int nPWPoints)
{
	if ((int)strlen(szName) == 0)
		return;

	char szFileName[64] = "";
	char szLevel[64] = "";

	char* szRefName = NULL;

	MCOLOR _color;
	char sp_name[256];
	bool bSpUser = false;

	if (IsAdminGrade(nGrade) && nGrade != MMUG_HIDE_ADMIN)
		//sprintf(szLevel, "%2d", nLevel);
		sprintf(szLevel, "--");
	else
		sprintf(szLevel, "%2d", nLevel);
	szRefName = szName;

	//#ifdef _VIPGRADES
	//	if ((IsVipGrade(nGrade)))
	//		sprintf(szLevel, "%2d", nLevel);
	//#endif
	//	else 
	//	{
	//		sprintf(szLevel,"%2d",nLevel);
	//		szRefName = szName;
	//	}

	switch (state) {

	case PS_FIGHT: strcpy(szFileName, "player_status_player.tga");	break;
	case PS_WAIT: strcpy(szFileName, "player_status_game.tga");		break;
	case PS_LOBBY: strcpy(szFileName, "player_status_lobby.tga");	break;
	}

	char szIcon[64];

	switch (ZGetGameClient()->GetChannelType())
	{
	case MCHANNEL_TYPE_PRESET:
	case MCHANNEL_TYPE_USER:
	case MCHANNEL_TYPE_PRIVATE:
	case MCHANNEL_TYPE_CLAN:
	case MCHANNEL_TYPE_QUEST:
		GetTierIconFile(szIcon, nPWPoints); //GetGradeIDIcon(szIcon, nGrade);	//đổi icon trước tên
		break;
	case MCHANNEL_TYPE_DUELTOURNAMENT:
		GetDuelTournamentGradeIconFileName(szIcon, duelTournamentGrade);
		break;
	case MCHANNEL_TYPE_LADDERWAR:
		GetTierIconFile(szIcon, nPWPoints);
		break;
	default:
		mlog("ZPlayerListBox::AddPlayer() - Unknown channel type\n");
	}

	MBitmap* pBmpIcon = MBitmapManager::Get(szIcon);
	ZLobbyPlayerListItem* pItem = new ZLobbyPlayerListItem(puid, MBitmapManager::Get(szFileName), nClanID, szLevel, szRefName, szClanName, state, nGrade, pBmpIcon);

	if (GetUserInfoUID(puid, _color, szName, nGrade))
		pItem->SetColor(_color);
	MListBox::Add(pItem);

}

// mode PLAYERLISTMODE_STAGE
void ZPlayerListBox::AddPlayer(MUID& puid, MMatchObjectStageState state, int nLevel, char* szName, char* szClanName, unsigned int nClanID, bool isMaster, MMatchTeam nTeam, int duelTournamentGrade, int nPWPoints)
{
	if ( (int)strlen( szName) == 0)
		return;

	char szFileName[64] = "";
	char szFileNameState[64] = "";
	char szLevel[64] = "";

	char* szRefName = NULL;

	MCOLOR _color;
	char sp_name[256];
	bool bSpUser = false;
	MMatchUserGradeID nGrade = MMUG_FREE;


	if (GetUserInfoUID(puid, _color, sp_name, nGrade))
	{
		bSpUser = true;
	}
	MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(puid);
	if (IsAdminGrade(pObjCache->GetUGrade()) && pObjCache->GetUGrade() != MMUG_HIDE_ADMIN)
		sprintf(szLevel, "--");
		//sprintf(szLevel, "%2d", nLevel);
	else
		sprintf(szLevel, "%2d", nLevel);
	szRefName = szName;

//#ifdef _VIPGRADES
//	MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(puid);
//	if (IsVipGrade(pObjCache->GetUGrade()))
//		sprintf(szLevel, "%2d", nLevel);
//#endif
//	else 
//	{
//		sprintf(szLevel,"%2d",nLevel);
//		szRefName = szName;
//	}

	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_SPY)
		nTeam = MMT_ALL;

	MBitmap* pBitmap = NULL;
	if(isMaster) {
		switch (state) {
			case MOSS_NONREADY	:
				if(nTeam == MMT_RED)			strcpy(szFileName, "stg_status_master_red.tga");
				else if(nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_master_blue.tga");
				else if(nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_master_observer.tga");
				else 							strcpy(szFileName, "stg_status_master_normal.tga");
				break;
			case MOSS_READY		:
				if(nTeam == MMT_RED)			strcpy(szFileName, "stg_status_master_red_ready.tga");
				else if(nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_master_blue_ready.tga");
				else if(nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_master_observer.tga");
				else 							strcpy(szFileName, "stg_status_master_normal_ready.tga");
				break;
			case MOSS_EQUIPMENT	: 
				if(nTeam == MMT_RED)			strcpy(szFileName, "stg_status_master_red_equip.tga");
				else if(nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_master_blue_equip.tga");
				else if(nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_master_observer.tga");
				else 							strcpy(szFileName, "stg_status_master_normal_equip.tga");
				break;
			default :
				strcpy(szFileName, " ");
				break;
		}
	}
	else {
		switch (state) {
			case MOSS_NONREADY	:
				if(nTeam == MMT_RED)			strcpy(szFileName, "stg_status_member_red.tga");
				else if(nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_member_blue.tga");	
				else if(nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_member_observer.tga");	
				else							strcpy(szFileName, "stg_status_member_normal.tga");	
				break;
			case MOSS_READY		: 
				if(nTeam == MMT_RED)			strcpy(szFileName, "stg_status_member_red_ready.tga");
				else if(nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_member_blue_ready.tga");	
				else if(nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_member_observer.tga");	
				else							strcpy(szFileName, "stg_status_member_normal_ready.tga");	
				break;
			case MOSS_EQUIPMENT	: 
				if(nTeam == MMT_RED)			strcpy(szFileName, "stg_status_member_red_equip.tga");
				else if(nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_member_blue_equip.tga");	
				else if(nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_member_observer.tga");	
				else							strcpy(szFileName, "stg_status_member_normal_equip.tga");	
				break;
		}
	}

	pBitmap = MBitmapManager::Get(szFileName);

	char szLadderIconFileName[64];
	GetTierIconFile(szLadderIconFileName, nPWPoints);
	MBitmap* pBmpLadderIcon = MBitmapManager::Get(szLadderIconFileName);
	ZStagePlayerListItem* pItem = new ZStagePlayerListItem(puid, pBitmap, nClanID, szRefName, szClanName, szLevel, nGrade, pBmpLadderIcon);
	/*char szGradeIcon[64];	//đổi icon trước tên (room)
	GetGradeIDIcon(szGradeIcon, nGrade);
	MBitmap* pBmpGradeIcon = MBitmapManager::Get(szGradeIcon);
	ZStagePlayerListItem* pItem = new ZStagePlayerListItem(puid, pBitmap, nClanID, szRefName, szClanName, szLevel, nGrade, pBmpGradeIcon);*/

	// for new UI
	pItem->m_bMaster = isMaster;
	switch (state)
	{
	case MOSS_READY:     strcpy(pItem->m_szStatus, "Ready");   break;
	case MOSS_SHOP:      strcpy(pItem->m_szStatus, "Shop");    break;
	case MOSS_EQUIPMENT: strcpy(pItem->m_szStatus, "Equip");   break;
	default:             strcpy(pItem->m_szStatus, "Standby"); break;
	}

	if (bSpUser)
		pItem->SetColor(_color);
	MListBox::Add(pItem);

	if( ZGetMyUID() == puid )
	{
		bool bBlue, bRed;
		bBlue = bRed = false;
		if( nTeam == MMT_BLUE)	bBlue = true;
		if( nTeam == MMT_RED)	bRed = true;
		
		MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamBlue");
		pButton->SetCheck( bBlue );

		pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamRed");
		pButton->SetCheck( bRed );
	}
}

// mode PLAYERLISTMODE_CHANNEL_FRIEND, PLAYERLISTMODE_STAGE_FRIEND
void ZPlayerListBox::AddPlayer(ePlayerState state, char* szName, char* szLocation)
{
	if ( (int)strlen( szName) == 0)
		return;


	char szFileName[64] = "";
	switch (state) {
		case PS_FIGHT	: strcpy(szFileName, "player_status_player.tga");	break;
		case PS_WAIT	: strcpy(szFileName, "player_status_game.tga");		break;
		case PS_LOBBY	: strcpy(szFileName, "player_status_lobby.tga");	break;
		// Custom: Added custom player status image
		case PS_LOGOUT  : strcpy(szFileName, "player_status_offline.tga");   break;
	}

	MListBox::Add(new ZFriendPlayerListItem(MUID(0,0),MBitmapManager::Get(szFileName), szName,NULL,szLocation,state,MMUG_FREE));
}

// mode PLAYERLISTMODE_CHANNEL_CLAN
void ZPlayerListBox::AddPlayer(MUID& puid, ePlayerState state, char* szName, int nLevel ,MMatchClanGrade nGrade )
{
	if ( (int)strlen( szName) == 0)
		return;


	char szFileName[64] = "";
	char szGradeName[64];

	char* szRefName = NULL;

	MCOLOR _color = MCOLOR(240,64,64);
//	char sp_name[256];
	bool bSpUser = false;

	switch(nGrade) {
		case MCG_MASTER : 
			sprintf(szGradeName, ZMsg( MSG_WORD_CLAN_MASTER));
			bSpUser = true;
			break;
		case MCG_ADMIN	: 
			sprintf(szGradeName, ZMsg( MSG_WORD_CLAN_ADMIN));
			bSpUser = true;
			break;
		default : sprintf(szGradeName, ZMsg( MSG_WORD_CLAN_MEMBER));
			break;
	}
	szRefName = szName;

	switch (state) {

		case PS_FIGHT	: strcpy(szFileName, "player_status_player.tga");	break;
		case PS_WAIT	: strcpy(szFileName, "player_status_game.tga");		break;
		case PS_LOBBY	: strcpy(szFileName, "player_status_lobby.tga");	break;
		// Custom: Added custom player status image
		case PS_LOGOUT  : strcpy(szFileName, "player_status_offline.tga");   break;
	}

	ZClanPlayerListItem* pItem = new ZClanPlayerListItem(puid, MBitmapManager::Get(szFileName), szRefName, szGradeName, NULL, state, nGrade );

	if(bSpUser)
		pItem->SetColor(_color);

	MListBox::Add( pItem );
}
#ifdef _SPEC 1
// Gets the flag bitmap that is displayed in the gameroom.
static MBitmap* GetStagePlayerBitmap(MMatchObjectStageState State, bool IsMaster, MMatchTeam Team)
{
	{
		const auto* pStageSetting = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting();
		const auto IsTeamGameType = ZGetGameTypeManager()->IsTeamGame(pStageSetting->nGameType);

		// If they're not on the spectator team, and it's not a team gametype
		// they can only be MMT_ALL (since MMT_RED and MMT_BLUE are only for team gametypes).
		if (Team != MMT_SPECTATOR && IsTeamGameType == false) {
			Team = MMT_ALL;
		}
	}

	auto GetTeamString = [](auto Team)
	{
		switch (Team)
		{
		case MMT_RED:
			return "red";
		case MMT_BLUE:
			return "blue";
		case MMT_SPECTATOR:
			return "observer";
		case MMT_ALL:
		default:
			return "normal";
		};
	};

	// The return value includes the underscore if it's not empty.
	auto GetReadyString = [](auto StageState, auto Team)
	{
		// Spectators can't be ready.
		if (Team == MMT_SPECTATOR)
			return "";

		switch (StageState)
		{
		case MOSS_READY:
			return "_ready";
		case MOSS_EQUIPMENT:
		case MOSS_SHOP:
			return "_equip";
		case MOSS_NONREADY:
		default:
			return "";
		};
	};

	auto PlayerStatusString = IsMaster ? "master" : "member";
	auto TeamString = GetTeamString(Team);
	auto ReadyString = GetReadyString(State, Team);

	char BitmapFilename[512];
	// ReadyString includes the underscore if it's not empty.
	sprintf_safe(BitmapFilename, "stg_status_%s_%s%s.tga",
		PlayerStatusString, TeamString, ReadyString);

	return MBitmapManager::Get(BitmapFilename);
}
// Updates the checkmarks in the buttons that select your team.
static void UpdateTeamButtonChecks(MMatchTeam Team)
{
	auto SetButtonCheck = [](auto&& Name, bool Value) {
		auto* pButton = ZFindWidgetAs<MButton>(Name);
		if (pButton) {
			pButton->SetCheck(Value);
		}
	};

	SetButtonCheck("StageTeamBlue", Team == MMT_BLUE);
	SetButtonCheck("StageTeamRed", Team == MMT_RED);
}
#endif
void ZPlayerListBox::DelPlayer(MUID& puid)
{
	ZPlayerListItem* pItem = NULL;

	for(int i=0;i<GetCount();i++) {
		pItem = (ZPlayerListItem*)Get(i);
		if(pItem->m_PlayerUID==puid){
			Remove(i);
			return;
		}
	}
}

// ³ªÁß¿¡ ¹Ù²ÙÀÚ ¹Ù»Ú´Ù.. »õº® 5:16ºÐ..

ZPlayerListItem* ZPlayerListBox::GetUID(MUID uid)
{
	ZPlayerListItem* pItem = NULL;

	for(int i=0;i<GetCount();i++) {
		pItem = (ZPlayerListItem*)Get(i);
		if(pItem->m_PlayerUID==uid)
			return pItem;
	}
	return NULL;
}


const char* ZPlayerListBox::GetPlayerName( int nIndex)
{
	ZPlayerListItem* pItem = (ZPlayerListItem*)Get( nIndex);

	if ( !pItem)
		return NULL;

	return pItem->m_szName;
}


// À§Ä¡´Â Á» ÀÌ»óÇÏÁö¸¸~

static DWORD g_zplayer_list_update_time = 0;

#define ZPLAYERLIST_UPDATE_TIME 2000

void ZPlayerListBox::UpdateList(int mode)
{
	if (ZGetGameClient()->IsConnected() == false) return;

	DWORD this_time = timeGetTime();

	if( this_time < g_zplayer_list_update_time + ZPLAYERLIST_UPDATE_TIME)
		return;

	if(mode==PLAYERLISTMODE_CHANNEL) {
/*		ZPlayerListBox* pWidget = (ZPlayerListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
		int nPage = pWidget->m_nPage;
		if(pWidget)
		{
			ZPostRequestChannelPlayerList(pWidget->m_MyUID,pWidget->m_uidChannel,nPage);
		}*/
	}
	else if(mode==PLAYERLISTMODE_STAGE) {
		// Do Nothing !!
	}
	else if(mode==PLAYERLISTMODE_CHANNEL_CLAN) {

		// test·Î ¿©±â¼­ ¾÷µ¥ÀÌÆ®ÇÒ¶§ Å¬·£ ¸®½ºÆ®´Þ¶ó°íµµ °è~¼Ó ¿äÃ»ÇÑ´Ù. Å×½ºÆ®³¡³ª¸é »èÁ¦¿ä¸Á - by bird
//#ifndef _PUBLISH
//			ZPostRequestClanMemberList(ZGetGameClient()->GetPlayerUID());				
//#endif
	}

	g_zplayer_list_update_time = this_time;
}


void ZPlayerListBox::UpdatePlayer(MUID& puid, MMatchObjectStageState state, bool isMaster, MMatchTeam nTeam)
{
#ifdef _SPEC 1
	auto* pItem = static_cast<ZStagePlayerListItem*>(GetUID(puid));
#else
	ZStagePlayerListItem* pItem = (ZStagePlayerListItem*)GetUID(puid);
#endif
	if (pItem)
	{
#ifdef _SPEC 1
		auto Bitmap = GetStagePlayerBitmap(state, isMaster, nTeam);
		pItem->m_pBitmap = Bitmap;
		pItem->Team = nTeam;
#endif
		char szFileName[64] = "";
		char szFileNameState[64] = "";

		MBitmap* pBitmap = NULL;
		MBitmap* pBitmapState = NULL;

		// for new UI
		pItem->m_bMaster = isMaster;
		switch (state)
		{
		case MOSS_READY:     strcpy(pItem->m_szStatus, "Ready");   break;
		case MOSS_SHOP:      strcpy(pItem->m_szStatus, "Shop");    break;
		case MOSS_EQUIPMENT: strcpy(pItem->m_szStatus, "Equip");   break;
		default:             strcpy(pItem->m_szStatus, "Standby"); break;
		}

		const MSTAGE_SETTING_NODE* pStageSetting = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting();

		// Custom: Infected game mode room list
		if ((nTeam != MMT_SPECTATOR) && (ZGetGameTypeManager()->IsTeamGame(pStageSetting->nGameType) == false) || pStageSetting->nGameType == MMATCH_GAMETYPE_INFECTED || pStageSetting->nGameType == MMATCH_GAMETYPE_SPY)
		{
			nTeam = MMT_ALL;
		}

		if (isMaster) {
			switch (state) {
			case MOSS_NONREADY:
				if (nTeam == MMT_RED) { strcpy(szFileName, "stg_status_master_red.tga");				pItem->m_nTeam = 1; }
				else if (nTeam == MMT_BLUE) { strcpy(szFileName, "stg_status_master_blue.tga");				pItem->m_nTeam = 2; }
				else if (nTeam == MMT_SPECTATOR) { strcpy(szFileName, "stg_status_master_observer.tga");			pItem->m_nTeam = 3; }
				else { strcpy(szFileName, "stg_status_master_normal.tga");			pItem->m_nTeam = 0; }
				break;
			case MOSS_READY:
				if (nTeam == MMT_RED) { strcpy(szFileName, "stg_status_master_red_ready.tga");		pItem->m_nTeam = 1; }
				else if (nTeam == MMT_BLUE) { strcpy(szFileName, "stg_status_master_blue_ready.tga");		pItem->m_nTeam = 2; }
				else if (nTeam == MMT_SPECTATOR) { strcpy(szFileName, "stg_status_master_observer.tga");			pItem->m_nTeam = 3; }
				else { strcpy(szFileName, "stg_status_master_normal_ready.tga");		pItem->m_nTeam = 0; }
				break;
			case MOSS_EQUIPMENT:
				if (nTeam == MMT_RED) { strcpy(szFileName, "stg_status_master_red_equip.tga");		pItem->m_nTeam = 1; }
				else if (nTeam == MMT_BLUE) { strcpy(szFileName, "stg_status_master_blue_equip.tga");		pItem->m_nTeam = 2; }
				else if (nTeam == MMT_SPECTATOR) { strcpy(szFileName, "stg_status_master_observer.tga");			pItem->m_nTeam = 3; }
				else { strcpy(szFileName, "stg_status_master_normal_equip.tga");		pItem->m_nTeam = 0; }
				break;
			default:
				strcpy(szFileName, " ");
				break;
			}
		}
		else {
			switch (state) {
			case MOSS_NONREADY:
				if (nTeam == MMT_RED) { strcpy(szFileName, "stg_status_member_red.tga");				pItem->m_nTeam = 1; }
				else if (nTeam == MMT_BLUE) { strcpy(szFileName, "stg_status_member_blue.tga");				pItem->m_nTeam = 2; }
				else if (nTeam == MMT_SPECTATOR) { strcpy(szFileName, "stg_status_member_observer.tga");			pItem->m_nTeam = 3; }
				else { strcpy(szFileName, "stg_status_member_normal.tga");			pItem->m_nTeam = 0; }
				break;
			case MOSS_READY:
				if (nTeam == MMT_RED) { strcpy(szFileName, "stg_status_member_red_ready.tga");		pItem->m_nTeam = 1; }
				else if (nTeam == MMT_BLUE) { strcpy(szFileName, "stg_status_member_blue_ready.tga");		pItem->m_nTeam = 2; }
				else if (nTeam == MMT_SPECTATOR) { strcpy(szFileName, "stg_status_member_observer.tga");			pItem->m_nTeam = 3; }
				else { strcpy(szFileName, "stg_status_member_normal_ready.tga");		pItem->m_nTeam = 0; }
				break;
			case MOSS_EQUIPMENT:
				if (nTeam == MMT_RED) { strcpy(szFileName, "stg_status_member_red_equip.tga");		pItem->m_nTeam = 1; }
				else if (nTeam == MMT_BLUE) { strcpy(szFileName, "stg_status_member_blue_equip.tga");		pItem->m_nTeam = 2; }
				else if (nTeam == MMT_SPECTATOR) { strcpy(szFileName, "stg_status_member_observer.tga");			pItem->m_nTeam = 3; }
				else { strcpy(szFileName, "stg_status_member_normal_equip.tga");		pItem->m_nTeam = 0; }
				break;
			}
		}

		pBitmap = MBitmapManager::Get(szFileName);
		pItem->m_pBitmap = pBitmap;

	}

	ZCharacterView* pCharView = (ZCharacterView*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_Charviewer_");
	if (pCharView != 0 && puid == pCharView->m_Info.UID)
	{
		pCharView->m_Info.bMaster = isMaster;
		pCharView->m_Info.m_pMnTeam->Set_CheckCrc((int)nTeam);
		pCharView->m_Info.nStageState = state;
	}

	if ((nTeam != MMT_SPECTATOR) && (ZGetMyUID() == puid))
	{
		bool bBlue, bRed;
		bBlue = bRed = false;
		if (nTeam == MMT_BLUE)	bBlue = true;
		if (nTeam == MMT_RED)	bRed = true;

		MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamBlue");
		pButton->SetCheck(bBlue);

		pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamRed");
		pButton->SetCheck(bRed);
	}
#ifdef _SPEC 1
	if (nTeam != MMT_SPECTATOR && ZGetMyUID() == puid)
	{
		UpdateTeamButtonChecks(nTeam);
	}
#endif
}


void ZPlayerListBox::UpdatePlayer(MUID& puid, MMatchObjectStageState state, char* szName, int  nLevel, bool isMaster, MMatchTeam nTeam)
{
	return;

#ifdef _SPEC 1
	auto* pItem = static_cast<ZStagePlayerListItem*>(GetUID(puid));
#else
	ZStagePlayerListItem* pItem = (ZStagePlayerListItem*)GetUID(puid);
#endif
	if (pItem)
	{
#ifdef _SPEC 1
		auto Bitmap = GetStagePlayerBitmap(state, isMaster, nTeam);
		pItem->m_pBitmap = Bitmap;
		pItem->Team = nTeam;
#endif
		char szFileName[64] = "";
		char szFileNameState[64] = "";
		char szLevel[64];

		MBitmap* pBitmap = NULL;
		MBitmap* pBitmapState = NULL;

		if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_SPY)
			nTeam = MMT_ALL;

		if (isMaster) {
			switch (state) {
			case MOSS_NONREADY:
				if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_master_red.tga");
				else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_master_blue.tga");
				else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_master_observer.tga");
				else 							strcpy(szFileName, "stg_status_master_normal.tga");
				break;
			case MOSS_READY:
				if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_master_red_ready.tga");
				else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_master_blue_ready.tga");
				else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_master_observer.tga");
				else 							strcpy(szFileName, "stg_status_master_normal_ready.tga");
				break;
			case MOSS_EQUIPMENT:
				if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_master_red_equip.tga");
				else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_master_blue_equip.tga");
				else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_master_observer.tga");
				else 							strcpy(szFileName, "stg_status_master_normal_equip.tga");
				break;
			default:
				strcpy(szFileName, " ");
				break;
			}
		}
		else {
			switch (state) {
			case MOSS_NONREADY:
				if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_member_red.tga");
				else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_member_blue.tga");
				else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_member_observer.tga");
				else 							strcpy(szFileName, "stg_status_member_normal.tga");
				break;
			case MOSS_READY:
				if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_member_red_ready.tga");
				else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_member_blue_ready.tga");
				else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_member_observer.tga");
				else 							strcpy(szFileName, "stg_status_member_normal_ready.tga");
				break;
			case MOSS_EQUIPMENT:
				if (nTeam == MMT_RED)			strcpy(szFileName, "stg_status_member_red_equip.tga");
				else if (nTeam == MMT_BLUE)		strcpy(szFileName, "stg_status_member_blue_equip.tga");
				else if (nTeam == MMT_SPECTATOR)	strcpy(szFileName, "stg_status_member_observer.tga");
				else 							strcpy(szFileName, "stg_status_member_normal_equip.tga");
				break;
			}
		}

		pBitmap = MBitmapManager::Get(szFileName);
		pItem->m_pBitmap = pBitmap;

		sprintf(szLevel, "Lv %2d", nLevel);
		strcpy(pItem->m_szLevel, szLevel);
		strcpy(pItem->m_szName, szName);

	}

	ZCharacterView* pCharView = (ZCharacterView*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_Charviewer_");
	if (pCharView != 0 && puid == pCharView->m_Info.UID)
	{
		pCharView->m_Info.bMaster = isMaster;
		pCharView->m_Info.m_pMnTeam->Set_CheckCrc((int)nTeam);
		pCharView->m_Info.nStageState = state;
		pCharView->m_Info.nLevel = nLevel;
		pCharView->SetText(szName);
	}

	if (ZGetMyUID() == puid)
	{
		bool bBlue, bRed;
		bBlue = bRed = false;
		if (nTeam == MMT_BLUE)	bBlue = true;
		if (nTeam == MMT_RED)	bRed = true;

		MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamBlue");
		pButton->SetCheck(bBlue);

		pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamRed");
		pButton->SetCheck(bRed);
	}
#ifdef _SPEC 1
	if (nTeam != MMT_SPECTATOR && ZGetMyUID() == puid)
	{
		UpdateTeamButtonChecks(nTeam);
	}
#endif
}

bool ZPlayerListBox::OnCommand(MWidget* pWidget, const char* szMessage)
{
//	if(pWidget==m_pButton) {
//		if( strcmp(szMessage, MBTN_CLK_MSG)==0 ) {
//			switch(GetMode()) {
//				//Ã¤³Î¾È¿¡¼­ ·ÎÅ×ÀÌÆ®
//			case PLAYERLISTMODE_CHANNEL :
//				SetMode(PLAYERLISTMODE_CHANNEL_FRIEND);
//				break;
//			case PLAYERLISTMODE_CHANNEL_FRIEND :
//				SetMode(PLAYERLISTMODE_CHANNEL_CLAN);
//				break;
//			case PLAYERLISTMODE_CHANNEL_CLAN :
//				SetMode(PLAYERLISTMODE_CHANNEL);
//				break;
//				// ½ºÅ×ÀÌÁö ¾È¿¡¼­..
//			case PLAYERLISTMODE_STAGE :
////				SetMode(PLAYERLISTMODE_STAGE_FRIEND);
//				break;
//			case PLAYERLISTMODE_STAGE_FRIEND :
///*
//				if(!ZGetMyInfo()->IsClanJoined())	// Å¬·£ÀÌ ¾øÀ¸¸é ³Ñ¾î°£´Ù
//					SetMode(PLAYERLISTMODE_STAGE);
//				else
//					SetMode(PLAYERLISTMODE_STAGE_CLAN);
//*/
//				break;
//			case PLAYERLISTMODE_STAGE_CLAN :
////				SetMode(PLAYERLISTMODE_STAGE);
//				break;
//			default:
//				SetMode(PLAYERLISTMODE_CHANNEL);
//				break;
//			}
//			return true;
//		}
//	}else
	if( strcmp(szMessage, "selected")==0 ) {

		if(m_nSelItem != -1) {
			ZStagePlayerListItem* pItem = (ZStagePlayerListItem*)Get(m_nSelItem);
			if(pItem) {
				MUID uid = pItem->m_PlayerUID;

				// ÇÃ·¹ÀÌ¾î¸¦ Å¬¸¯ÇÏ¸é ÁÂÃøÀÇ Ä³¸¯ÅÍºä¿¡ ÇØ´ç Ä³¸¯ÅÍ ÀÌ¹ÌÁö°¡
				// ¶ß´Â°Å ¸·À½. ¾ÆÁ÷ Ä³¸¯ÅÍ Á¤º¸±îÁö º¯°æ ¾ÈµÇ´Â °ü°è·Î...  -_-;(µ¿È¯)
				ZCharacterView* pCharView = (ZCharacterView*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_Charviewer");
				if(pCharView!=0) pCharView->SetCharacter( uid );
			}
		}
//		ZStagePlayerListItem* pItem = (ZStagePlayerListItem*) pWidget;
		return true;
	}
	else if(strcmp(szMessage, "selected2")==0) {
		// Â÷ÈÄ ¸Þ´º·Î ¼öÁ¤~
		if((GetKeyState(VK_MENU)&0x8000)!=0) {
//			ZStagePlayerListItem* pItem = (ZStagePlayerListItem*) pWidget;
			if(m_nSelItem != -1) {
				ZStagePlayerListItem* pItem = (ZStagePlayerListItem*)Get(m_nSelItem);
				if(pItem) {
					char temp[1024];
					sprintf(temp,"/kick %s", pItem->m_szName);
					ZPostStageChat(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), temp);
				}
			}
		}
	}

	return true;
}
bool ZPlayerListBox::OnEvent(MEvent* pEvent, MListener* pListener)
{
	MRECT rtClient = GetClientRect();

	// for new UI: ghi lai vi tri chuot de OnDraw biet o nao dang duoc tro vao
	if (pEvent->nMessage == MWM_MOUSEMOVE)
	{
		if (rtClient.InPoint(pEvent->Pos)) m_ptMouse = pEvent->Pos;
		else m_ptMouse = MPOINT(-1, -1);
	}

	// for new UI: bam vao o trong cua mot cot de chuyen sang doi do
	if (pEvent->nMessage == MWM_LBUTTONDOWN &&
		(m_nMode == PLAYERLISTMODE_STAGE || m_nMode == PLAYERLISTMODE_STAGE_FRIEND))
	{
		if (m_TeamZoneRed.w > 0 && m_TeamZoneRed.InPoint(pEvent->Pos))
		{
			ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_RED);
			return true;
		}
		if (m_TeamZoneBlue.w > 0 && m_TeamZoneBlue.InPoint(pEvent->Pos))
		{
			ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_BLUE);
			return true;
		}
	}

	if (pEvent->nMessage == MWM_RBUTTONDOWN) {
		if(rtClient.InPoint(pEvent->Pos)==true) {
			int nSelItem = FindItem(pEvent->Pos);
			if (nSelItem != -1) {
				ZLobbyPlayerListItem* pItem = (ZLobbyPlayerListItem*)Get(nSelItem);

				bool bShow = true;

				MCOLOR _color;
				char sp_name[256];
				MMatchUserGradeID gid;

				if(pItem->GetUID() == ZGetMyUID() && 
					GetMode()!=PLAYERLISTMODE_CHANNEL_CLAN)	
					bShow = false;

				GetUserInfoUID(pItem->GetUID(),_color,sp_name,gid);

				// Custom: Show the player list box for all users (including admins)
				//if((gid==MMUG_DEVELOPER) || (gid==MMUG_ADMIN)) { // °³¹ßÀÚ³ª °ü¸®ÀÚ¶ó¸é..
				//	bShow = false;
				//}

				SetSelIndex(nSelItem);
				if (bShow) {

					ZPlayerMenu* pMenu = ZApplication::GetGameInterface()->GetPlayerMenu();
					pMenu->SetTargetName(pItem->GetString());
					pMenu->SetTargetUID(pItem->GetUID());

					switch(GetMode()) {
					case PLAYERLISTMODE_CHANNEL:
						pMenu->SetupMenu(ZPLAYERMENU_SET_LOBBY);
						break;
					case PLAYERLISTMODE_STAGE:
						pMenu->SetupMenu(ZPLAYERMENU_SET_STAGE);
						break;
					case PLAYERLISTMODE_CHANNEL_FRIEND:
						pMenu->SetupMenu(ZPLAYERMENU_SET_FRIEND);
						break;
					case PLAYERLISTMODE_STAGE_FRIEND:
						pMenu->SetupMenu(ZPLAYERMENU_SET_FRIEND);
						break;
					case PLAYERLISTMODE_CHANNEL_CLAN:
						if(pItem->GetUID() == ZGetMyUID())
							pMenu->SetupMenu(ZPLAYERMENU_SET_CLAN_ME);
						else
							pMenu->SetupMenu(ZPLAYERMENU_SET_CLAN);
						break;
					default:
						_ASSERT("Unknown PlayerMenu Setup");
					};

					// À§Ä¡¸¦ Á¶ÀýÇØÁØ´Ù
/*					MPOINT posItem;
					GetItemPos(&posItem, nSelItem);
					MPOINT posMenu;
					posMenu.x = posItem.x + GetRect().w/2;
					posMenu.y = posItem.y + GetItemHeight()/2;
					posMenu=MClientToScreen(this,posMenu);

					if(posMenu.y+pMenu->GetRect().h > MGetWorkspaceHeight()) {
						posMenu.y-= pMenu->GetRect().h ;
					}

					pMenu->Show(posMenu.x, posMenu.y, true);
*/
					MPOINT posItem;
					posItem = pEvent->Pos;
					MPOINT posMenu = MClientToScreen(this, posItem);
					
					if ( (posMenu.x + pMenu->GetClientRect().w) > (MGetWorkspaceWidth() - 5))
						posMenu.x = MGetWorkspaceWidth() - pMenu->GetClientRect().w - 5;

					if ( (posMenu.y + pMenu->GetClientRect().h) > (MGetWorkspaceHeight() - 5))
						posMenu.y = MGetWorkspaceHeight() - pMenu->GetClientRect().h - 5;

					pMenu->Show( posMenu.x, posMenu.y);
				}
				return true;
			}
		}
	}


	// ÇÃ·¹ÀÌ¾î Á¤º¸ Ãâ·Â
	else if ( pEvent->nMessage == MWM_LBUTTONDBLCLK)
	{
		if ( rtClient.InPoint( pEvent->Pos) == true)
		{
			int nSelItem = FindItem( pEvent->Pos);

			if ( nSelItem != -1)
			{
#ifdef _DEBUG
				ZLobbyPlayerListItem* pItem = (ZLobbyPlayerListItem*)Get( nSelItem);
				ZPostRequestCharInfoDetail( ZGetMyUID(), pItem->m_szName);
#endif
			}
		}
	}


/*	if (GetToolTip()) {
		if ( (GetMode() == PLAYERLISTMODE_CHANNEL_FRIEND) || (GetMode() == PLAYERLISTMODE_STAGE_FRIEND) ) {
			if(rtClient.InPoint(pEvent->Pos)==true) {
				int nSelItem = FindItem(pEvent->Pos);
				if (nSelItem != -1) {
					ZFriendPlayerListItem* pItem = (ZFriendPlayerListItem*)Get(nSelItem);

					MPOINT posItem;
					GetItemPos(&posItem, nSelItem);
					MPOINT posToolTip;
					posToolTip.x = GetRect().x + posItem.x + GetRect().w/2;
					posToolTip.y = GetRect().y + posItem.y + GetItemHeight()/2;

					GetToolTip()->SetPosition(posToolTip.x, posToolTip.y);
					GetToolTip()->SetText(pItem->GetLocation());
				} else {
					GetToolTip()->Show(false);
				}
			}
		} else {
			GetToolTip()->Show(false);
		}
	}
*/
	return MListBox::OnEvent(pEvent, pListener);;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/*
IMPLEMENT_LOOK(ZStagePlayerListBox, ZPlayerListBoxLook)

////////////////////////////////////////////////////////////////////////////////////////////////////

ZStagePlayerListBox::ZStagePlayerListBox(const char* szName, MWidget* pParent, MListener* pListener)
: MListBox(szName, pParent, pListener)
{
	LOOK_IN_CONSTRUCTOR()

	SetVisibleHeader(false);

	m_bAbsoulteTabSpacing = true;

	m_MyUID = MUID(0,0);
	m_uidChannel = MUID(0,0);
	m_nTotalPlayerCount = 0;
	m_nPage = 0;

	m_bVisible = true;

	AddField(" ", 23);//icon
	AddField(" ",130);//name
	AddField(" ", 23);//state icon
	AddField(" ", 32);//level

	SetItemHeight(23);

//	m_pBitmap = NULL;
	m_pBitmap = MBitmapManager::Get("playerlist_tab_game.png");
	
	mSelectedPlayer = 0;
	mStartToDisplay = 0;
	mPlayerOrder.reserve( sizeof(MUID)*100 );
//	m_pScrollBar = new MScrollBar( this, this );
//	ZApplication::GetGameInterface()->GetIDLResource()->InsertWidget("PlayerListScrollBar", m_pScrollBar );

//	AddPlayer(MUID(0,0), MOSS_READY		, "aaaaa", 10 ,true ,MMT_RED );
//	AddPlayer(MUID(0,1), MOSS_SHOP		, "bbbbb", 11 ,false,MMT_BLUE);
//	AddPlayer(MUID(0,2), MOSS_EQUIPMENT	, "ccccc", 12 ,false,MMT_RED );
//	AddPlayer(MUID(0,3), MOSS_NONREADY	, "ddddd", 13 ,false,MMT_BLUE);

	m_bAlwaysVisibleScrollbar = false;
	m_bHideScrollBar = true;
	m_pScrollBar->SetVisible(false);
	m_pScrollBar->Enable(false);

	m_pScrollBar->m_nDebugType = 3;
	m_nDebugType = 2;

	m_nOldW = RGetScreenWidth();
}

ZStagePlayerListBox::~ZStagePlayerListBox(void)
{
}

void ZStagePlayerListBox::SetBitmap( MBitmap* pBitmap)
{
	m_pBitmap	= pBitmap;
}

void ZStagePlayerListBox::Resize(float x,float y)
{
//	OnReSize();
}

float ZStagePlayerListBox::OnReSize()
{
	int newW = RGetScreenWidth();
	float fA = GetF(newW);

	m_nItemHeight = 23 * fA;

	MLISTFIELD* fi;

	for(int i=0;i<m_Fields.GetCount();i++) {
			
		fi = m_Fields.Get(i);

		if(i==0)		fi->nTabSize = 23 * fA;
		else if(i==1)	fi->nTabSize = 130 * fA;
		else if(i==2) 	fi->nTabSize = 23 * fA;
		else if(i==3) 	fi->nTabSize = 32 * fA;
	}

	return fA;
}

void ZStagePlayerListBox::DelPlayer(MUID& puid)
{
	ZStagePlayerListItem* pItem = NULL;

	for(int i=0;i<GetCount();i++) {
		pItem = (ZStagePlayerListItem*)Get(i);
		if(pItem->m_PlayerUID==puid){
			Remove(i);
			return;
		}
	}
}

// ³ªÁß¿¡ ¹Ù²ÙÀÚ ¹Ù»Ú´Ù.. »õº® 5:16ºÐ..

ZStagePlayerListItem* ZStagePlayerListBox::GetUID(MUID uid)
{
	ZStagePlayerListItem* pItem = NULL;

	for(int i=0;i<GetCount();i++) {
		pItem = (ZStagePlayerListItem*)Get(i);
		if(pItem->m_PlayerUID==uid)
			return pItem;
	}
	return NULL;
}

void ZStagePlayerListBox::UpdatePlayer(MUID& puid,eStagePlayerState state, char* szName, int  nLevel ,bool isMaster,int nTeam)
{
	ZStagePlayerListItem* pItem = GetUID(puid);
	if(pItem) {

		char szFileName[64] = "";
		char szFileNameState[64] = "";
		char szLevel[64];

		sprintf(szLevel,"Lv %2d",nLevel);

		MBitmap* pBitmap = NULL;
		MBitmap* pBitmapState = NULL;

		if(isMaster) {
				 if(nTeam == MMT_RED)	strcpy(szFileName, "stg_status_master_red.tga");	
			else if(nTeam == MMT_BLUE)	strcpy(szFileName, "stg_status_master_blue.tga");	
			else 						strcpy(szFileName, "stg_status_master_normal.tga");	
		}
		else {
			if(nTeam == MMT_RED)		strcpy(szFileName, "stg_status_member_red.tga");	
			else if(nTeam == MMT_BLUE)	strcpy(szFileName, "stg_status_member_blue.tga");	
			else						strcpy(szFileName, " ");	
		}

		switch (state) {

		case MOSS_NONREADY	: strcpy(szFileNameState, " ");						break;
		case MOSS_READY		: strcpy(szFileNameState, "char_stat_ready.tga");	break;
		case MOSS_SHOP		: strcpy(szFileNameState, "char_stat_shop.tga");	break;
		case MOSS_EQUIPMENT	: strcpy(szFileNameState, "char_stat_equip.tga");	break;
		}

		pBitmap = MBitmapManager::Get(szFileName);
		pBitmapState = MBitmapManager::Get(szFileNameState);

		pItem->m_pBitmap = pBitmap;
		pItem->m_pBitmapState = pBitmapState;
		
		sprintf(szLevel,"Lv %2d",nLevel);
		strcpy(pItem->m_szLevel,szLevel);
		strcpy(pItem->m_szName,szName);
	}
}

void ZStagePlayerListBox::AddPlayer(MMatchObjCache* pCache)
{
	if(!pCache) return;

	AddPlayer(pCache->GetUID(), MOSS_NONREADY,pCache->GetName(), pCache->GetLevel(),false,MMT_ALL);
}

void ZStagePlayerListBox::AddPlayer(MUID& puid, MMatchObjectStageState state, char* szName, int  nLevel ,bool isMaster,MMatchTeam nTeam)
{
	char szFileName[64] = "";
	char szFileNameState[64] = "";
	char szLevel[64];

	sprintf(szLevel,"Lv %2d",nLevel);

	MBitmap* pBitmap = NULL;
	MBitmap* pBitmapState = NULL;

	if(isMaster) {
			 if(nTeam == MMT_RED)	strcpy(szFileName, "stg_status_master_red.tga");	
		else if(nTeam == MMT_BLUE)	strcpy(szFileName, "stg_status_master_blue.tga");	
		else 						strcpy(szFileName, "stg_status_master_normal.tga");	
	}
	else {
		if(nTeam == MMT_RED)		strcpy(szFileName, "stg_status_member_red.tga");	
		else if(nTeam == MMT_BLUE)	strcpy(szFileName, "stg_status_member_blue.tga");	
		else						strcpy(szFileName, " ");	
	}

	switch (state) {

		case MOSS_NONREADY	: strcpy(szFileNameState, " ");						break;
		case MOSS_READY		: strcpy(szFileNameState, "char_stat_ready.tga");	break;
		case MOSS_SHOP		: strcpy(szFileNameState, "char_stat_shop.tga");	break;
		case MOSS_EQUIPMENT	: strcpy(szFileNameState, "char_stat_equip.tga");	break;
	}

	pBitmap = MBitmapManager::Get(szFileName);
	pBitmapState = MBitmapManager::Get(szFileNameState);

//	MListBox::Add(new ZLobbyPlayerListItem(puid, pBitmap, szName, szLevel));
	MListBox::Add(new ZStagePlayerListItem(puid, pBitmap, pBitmapState, szName, szLevel));
}

bool ZStagePlayerListBox::OnEvent(MEvent* pEvent, MListener* pListener)
{
	if(pEvent->nMessage==MWM_MOUSEMOVE)	{
	}
	else if(pEvent->nMessage==MWM_MBUTTONDOWN) {
		int k=0;
	}

	return MListBox::OnEvent(pEvent, pListener);
}
*/
/*
void ZStagePlayerListBox::AddPlayer( MMatchObjCache* pCache )
{
	MUID uid = pCache->GetUID();

	sPlayerInfo* pInfo = new sPlayerInfo;
	strcpy( pInfo->szName, pCache->GetName() );
	pInfo->Level	=	pCache->GetLevel();

	MListBox::Add(new ZLobbyPlayerListItem(uidItem, pIconBitmap, szName, szWeight, szSlot, szPrice));
}
*/

// for new UI: do click theo dung khung da ve, thay vi cong thuc mot cot cua MListBox
int ZPlayerListBox::FindItem(MPOINT& p)
{
	bool bStageMode = (m_nMode == PLAYERLISTMODE_STAGE || m_nMode == PLAYERLISTMODE_STAGE_FRIEND);
	if (bStageMode == false)
		return MListBox::FindItem(p);

	for (int i = 0; i < (int)m_ItemDrawRects.size(); i++)
	{
		if (m_ItemDrawRects[i].w <= 0) continue;
		if (m_ItemDrawRects[i].InPoint(p)) return i;
	}
	return -1;
}


MUID ZPlayerListBox::GetSelectedPlayerUID() 
{
	ZLobbyPlayerListItem* pItem = (ZLobbyPlayerListItem*)GetSelItem();
	if(!pItem) return MUID(0,0);
    
	return pItem->GetUID();
}

void ZPlayerListBox::SelectPlayer(MUID uid)
{
	for(int i=0;i<GetCount();i++)
	{
		ZLobbyPlayerListItem* pItem = (ZLobbyPlayerListItem*)Get(i);
		if(pItem->GetUID()==uid){
			SetSelIndex(i);
			return;
		}
	}
}

void ZPlayerListBox::MultiplySize( float byIDLWidth, float byIDLHeight, float byCurrWidth, float byCurrHeight )
{
	MListBox::MultiplySize(byIDLWidth, byIDLHeight, byCurrWidth, byCurrHeight);

	OnSize(GetRect().w, GetRect().h);	// onsize¾È¿¡¼­ Ä¿½ºÅÒÇÏ°Ô Ã³¸®ÇÏ´Â ºÎºÐµéÀ» ½ÇÇà½ÃÅ°ÀÚ
}