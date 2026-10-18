#include "stdafx.h"
#include "ZLadderWarsRankingListBox.h"

ZLadderWarsRankingListBox::ZLadderWarsRankingListBox(const char* szName, MWidget* pParent, MListener* pListener) 
: MWidget(szName, pParent, pListener)
{
	m_pBmpRankingItemBg = NULL;

	m_nMyRankIndex = -1;
}

ZLadderWarsRankingListBox::~ZLadderWarsRankingListBox()
{

}

void ZLadderWarsRankingListBox::ClearAll()
{
}

void ZLadderWarsRankingListBox::LoadInterfaceImgs()
{
	if (m_pBmpRankingItemBg == NULL) {
		m_pBmpRankingItemBg = new MBitmapR2;
		((MBitmapR2*)m_pBmpRankingItemBg)->Create( "DuelTournamentRankingItemBg.png", RGetDevice(), "interface/loadable/DuelTournamentRankingItemBg.png");
	}
}

void ZLadderWarsRankingListBox::UnloadInterfaceImgs()
{
	SAFE_DELETE(m_pBmpRankingItemBg);
}

void ZLadderWarsRankingListBox::SetRankInfo( unsigned int nIndex, const ZPLAYERRANKINGITEM& rankingItem )
{
	if (nIndex >= NUM_DISPLAY_LADDERWARS_RANKING) { _ASSERT(0); return; }
	
	m_rankingList[nIndex] = rankingItem;
}

//Custom: LadderPoints Rank
void ZLadderWarsRankingListBox::OnDraw(MDrawContext* pDC)
{

	const int nWidth = this->GetRect().w;
	const int nHeight = this->GetRect().h;
	const int nY_firstItem = nHeight * 0.17f;
	const int nItemHeight = (int)(nHeight / 7.f);
	char szTemp[128];
	MRECT rc;

	for (int i = 0; i < NUM_DISPLAY_LADDERWARS_RANKING; ++i)
	{
		int y = (nY_firstItem + nItemHeight * i) - (nItemHeight * 0.04f);
		pDC->SetBitmap(m_pBmpRankingItemBg);
		pDC->Draw(0, y, nWidth, nItemHeight);
		ZPLAYERRANKINGITEM* pRankItem = &m_rankingList[i];
		y = nY_firstItem + nItemHeight * i;

		if (pRankItem->nRank == -1)
			strcpy(szTemp, "--");
		else
			pDC->SetColor(MCOLOR(0xFFFFFFFF));
		sprintf(szTemp, "%d", pRankItem->nRank);
		rc.Set((int)(0.01f * nWidth), y, (int)(fabs(0.01f - 0.11f) * nWidth), nItemHeight);
		pDC->Text(rc, szTemp, MAM_RIGHT | MAM_VCENTER);

		rc.Set((int)(0.30f * nWidth), y, (int)(fabs(0.30f - 0.52f) * nWidth), nItemHeight);
		pDC->SetColor(MCOLOR(0xFFFF6414));
		pDC->Text(rc, pRankItem->szCharName, MAM_HCENTER | MAM_VCENTER);

		pDC->SetColor(MCOLOR(0xFF00FF3A));
		sprintf(szTemp, "%d", pRankItem->nWins);
		rc.Set((int)(0.60f * nWidth), y, (int)(fabs(0.60f - 0.70f) * nWidth), nItemHeight);
		pDC->Text(rc, szTemp, MAM_RIGHT | MAM_VCENTER);

		// LOSS
		pDC->SetColor(MCOLOR(0xFFFF3A3A)); // đỏ nhẹ
		sprintf(szTemp, "%d", pRankItem->nLosses);
		rc.Set((int)(0.80f * nWidth), y, (int)(fabs(0.80f - 0.90f) * nWidth), nItemHeight);
		pDC->Text(rc, szTemp, MAM_RIGHT | MAM_VCENTER);
		/*rc.Set((int)(0.70f * nWidth), y, (int)(fabs(0.70f - 0.81f) * nWidth), nItemHeight);
		pDC->SetColor(MCOLOR(0xFFFFFFFF));
		pDC->Text(rc, "/", MAM_HCENTER | MAM_VCENTER);

		pDC->SetColor(MCOLOR(0xFFFFC900));
		sprintf(szTemp, "%d", pRankItem->nScore);
		rc.Set((int)(0.81f * nWidth), y, (int)(fabs(0.81f - 0.91f) * nWidth), nItemHeight);
		pDC->Text(rc, szTemp, MAM_LEFT | MAM_VCENTER);*/

		if (i == m_nMyRankIndex)
		{
			MBitmapR2 *pBitmap=(MBitmapR2*)MBitmapManager::Get("button_glow.png");
			if(pBitmap) 
			{
				DWORD defaultcolor = 0x00d2cb; // Custom: Change color button_glow By Desperate and JohnWalker
				DWORD opacity=(DWORD)pDC->GetOpacity();
				MRECT rt(0, y, nWidth, nItemHeight);
				MDrawEffect prevEffect = pDC->GetEffect();
				pDC->SetEffect(MDE_ADD);
				MCOLOR prevColor = pDC->GetBitmapColor();
				pDC->SetBitmapColor(MCOLOR(defaultcolor));
				unsigned char prevOpacity = pDC->GetOpacity();
				pDC->SetOpacity(opacity);
				pDC->SetBitmap(pBitmap);
				pDC->Draw(rt.x,rt.y,rt.w,rt.h,0,0,64,32);
				pDC->SetBitmapColor(prevColor);
				pDC->SetEffect(prevEffect);
				pDC->SetOpacity(prevOpacity);
			}
		}
	}
}