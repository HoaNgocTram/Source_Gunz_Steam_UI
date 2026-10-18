#include "stdafx.h"
#include "ZMessages.h"
#include "ZApplication.h"
#include "ZClanListBox.h"

ZClanListBox::ZClanListBox(const char* szName, MWidget* pParent, MListener* pListener) 
			: MWidget(szName, pParent, pListener)
{
	m_iNumRoom		= 0;
	m_RoomWidth		= 0;
	m_RoomHeight	= 0;

	m_pRoomFrame = MBitmapManager::Get("banner_clan.bmp");
	m_pRoomDodgeFrame = MBitmapManager::Get("banner_clan.bmp");
	TeamDeathMatch = MBitmapManager::Get("teamdeath-mode.tga");
	//Assassinate = MBitmapManager::Get("Assasin-mode.tga");
	Gladiator = MBitmapManager::Get("teamGladiator-mode.tga");
	AntiLead = MBitmapManager::Get("antilead.png");
	Lead = MBitmapManager::Get("lead.png");
	Base = MBitmapManager::Get("BaseRotationOff.png");
	BaseRotation = MBitmapManager::Get("BaseRotation.png");
	for (int i = 0; i < NUM_DISPLAY_CLAN; i++)
	{
		ZCLANINFO *pInfo = m_pClanInfo+i;
		pInfo->bEmpty = true;
		pInfo->nClanEmblemID = 0;
	}
}

ZClanListBox::~ZClanListBox()
{
	ClearAll();
}

void ZClanListBox::OnDraw(MDrawContext* pDC)
{
	const int nWidth = 376;	// 원래 위젯 크기
	float fRatio = (float)m_Rect.w / (float)nWidth;

	const int nGap = 7;
	int j = 0, b = 0;
	for (int i = 0; i < NUM_DISPLAY_CLAN; i++) {

		ZCLANINFO* pInfo = m_pClanInfo + i;
		if (!pInfo->bEmpty)
		{
			if (pInfo->bDodge)
			{
				if (m_pRoomDodgeFrame)
				{
					pDC->SetBitmap(m_pRoomDodgeFrame);
					int y = (int)(fRatio * (m_pRoomDodgeFrame->GetHeight() + nGap) * i);
					pDC->Draw(0, y, (int)(fRatio * m_pRoomDodgeFrame->GetWidth()), (int)(fRatio * m_pRoomDodgeFrame->GetHeight()));
					pDC->SetColor(MCOLOR(0xffffffff));
					pDC->Text((int)(fRatio * 40), (int)(y + fRatio * 10), "Anonymous Details");

					char szBuffer[256];
					sprintf(szBuffer, ZMsg(MSG_LOBBY_WAITING), pInfo->nPlayers);
					pDC->Text((int)(fRatio * 280), (int)(y + fRatio * 10), szBuffer);
				}
			}
			else
			{
				pDC->SetBitmap(m_pRoomFrame);
				int y = (int)(fRatio * (m_pRoomFrame->GetHeight() + nGap) * i);
				pDC->Draw(0, y, (int)(fRatio * m_pRoomFrame->GetWidth()), (int)(fRatio * m_pRoomFrame->GetHeight()));

				MBitmap* pBitmap = ZGetEmblemInterface()->GetClanEmblem(pInfo->nClanEmblemID);
				if (pBitmap) 
				{
					int nSize = (int)(.95f * fRatio * m_pRoomFrame->GetHeight());
					int nMargin = (int)(.05f * fRatio * m_pRoomFrame->GetHeight());
					pDC->SetBitmap(pBitmap);
					pDC->Draw(nMargin, y + nMargin, nSize, nSize);
				}
				//Custom: Icon GameType
				switch (pInfo->GameType)
				{
				case 0:
					pDC->SetBitmap(TeamDeathMatch);
					pDC->Draw((int)(fRatio * 200), (int)(y + fRatio * 10), 16, 16);
					break;
				case 1:
					pDC->SetBitmap(Gladiator);
					pDC->Draw((int)(fRatio * 200), (int)(y + fRatio * 10), 16, 16);
					break;
				}

				//Custom: Base Rotation Icon
				if (pInfo->bRobase)
				{
					pDC->SetBitmap(BaseRotation);
					pDC->Draw((int)(fRatio * 230), (int)(y + fRatio * 10), 16, 16);
				}
				else
				{
					pDC->SetBitmap(Base);
					pDC->Draw((int)(fRatio * 230), (int)(y + fRatio * 10), 16, 16);
				}

				//Custom: Icon Lead
				if (pInfo->bIsLead)
				{
					pDC->SetBitmap(Lead);
					pDC->Draw((int)(fRatio * 260), (int)(y + fRatio * 10), 16, 16);
				}
				else
				{
					pDC->SetBitmap(AntiLead);
					pDC->Draw((int)(fRatio * 260), (int)(y + fRatio * 10), 16, 16);
				}

				pDC->SetColor(MCOLOR(0xffffffff));
				pDC->Text((int)(fRatio * 40), (int)(y + fRatio * 10), pInfo->szClanName);

				char szBuffer[256];
				sprintf(szBuffer, ZMsg(MSG_LOBBY_WAITING), pInfo->nPlayers);
				pDC->Text((int)(fRatio * 280), (int)(y + fRatio * 10), szBuffer);
			}
		}
	}
}

void ZClanListBox::SetInfo(int nIndex, int nEmblemID, const char *szName, int nPlayers, bool bIsAntiLead, int GameType, bool bDodge, bool bRobase)
{
	if(nIndex<0 || nIndex>=NUM_DISPLAY_CLAN) return;

	ZCLANINFO *pInfo = m_pClanInfo+nIndex;

	ZGetEmblemInterface()->AddClanInfo(nEmblemID);
	ZGetEmblemInterface()->DeleteClanInfo(pInfo->nClanEmblemID);
	pInfo->nClanEmblemID = nEmblemID;
	strcpy(pInfo->szClanName , szName);
	pInfo->nPlayers = nPlayers;
	pInfo->bIsLead = bIsAntiLead;
	pInfo->bEmpty = false;
	pInfo->GameType = GameType;
	pInfo->bDodge = bDodge;
	pInfo->bRobase = bRobase;
}

void ZClanListBox::Clear(int nIndex)
{
	if(nIndex<0 || nIndex>=NUM_DISPLAY_CLAN) return;

	ZCLANINFO *pInfo = m_pClanInfo+nIndex;
	pInfo->bEmpty = true;
	ZGetEmblemInterface()->DeleteClanInfo(pInfo->nClanEmblemID);
	pInfo->nClanEmblemID = 0;
	pInfo->bIsLead = false;
	pInfo->bRobase = false;
}

void ZClanListBox::ClearAll()
{
	for (int i = 0; i < NUM_DISPLAY_CLAN; i++)
	{
		Clear(i);
	}
}