#include "stdafx.h"

#include "ZGameClient.h"
#include ".\zroomlistbox.h"
#include "MButton.h"
#include "MBitmap.h"
#include "RTypes.h"
#include "MDrawContext.h"
#include "MDebug.h"
#include "MScrollBar.h"
#include "ZApplication.h"
#include "ZPost.h"

#define		ROOM_WIDTH					8
#define		ROOM_HEIGHT					7
const char* EMPTY_ROOM		=		"Empty";

template < class F, class S>
class string_key_equal
{
private:
	F string_key;
public:
	string_key_equal( const F& str ) : string_key(str) {}
	bool operator() (pair<const F, S> elem)
	{		return ( stricmp(elem.first.c_str(), string_key.c_str())== 0);		}
};

ZRoomListBox::ZRoomListBox(const char* szName, MWidget* pParent, MListener* pListener)
: MWidget(szName, pParent, pListener)
{
	m_nPrevStageCount = m_nNextStageCount = -1;
	//m_pMapInfo			= new sMapInfo[NUM_DISPLAY_ROOM];
	m_iNumRoom		= 0;
	m_RoomWidth		= 0;
	m_RoomHeight	= 0;
	m_pRoomFrame	= 0;
	m_Selection		= -1;
	m_currPage		=	0;
	m_iGapWidth		= ROOM_WIDTH;
	m_iGapHeight	= ROOM_HEIGHT;
	m_iGapCenter	= 10;
	m_uidSelectedPrivateStageUID	= MUID(0,0);
//	m_bHideFull		= false;
	m_ptMouse		= MPOINT(-1, -1);	// for new UI
}

ZRoomListBox::~ZRoomListBox(void)
{
	//SAFE_DELETE_ARRAY(m_pMapInfo);
	for( map<string, MBitmap*>::iterator	iter = m_pMapImage.begin(); iter != m_pMapImage.end(); )
	{
		m_pMapImage.erase(iter++);
	}
}

//void ZRoomListBox::OnDraw(MDrawContext* pDC)
//{
//	MBitmap* pBitmap;
//	MRECT rect;
//	map<string, MBitmap*>::iterator iter;
//	map<MMATCH_GAMETYPE, MBitmap*>::iterator iterIcon;
//	int pressed_reposition = 0;
//
//	int index = 0;
//
//	for (int i = 0; i < NUM_DISPLAY_ROOM; ++i)
//	{
//		bool bRoomFull = false;
//
//		if (m_Selection == i) pressed_reposition = 1;
//		else                  pressed_reposition = 0;
//
//		const char* mapName = MGetMapDescMgr()->GetBannerName(m_pMapInfo[i].map_name);
//		if (m_pMapInfo[i].IsEmpty)
//		{
//			continue;
//		}
//		rect = GetInitialClientRect();
//
//		// =========================================================================
//		// [SỬA LẠI TẠI ĐÂY]: Tính vị trí Grid 3 cột x 3 hàng
//		// =========================================================================
//		int nCol = index % 3; // 0, 1, 2 (3 Cột)
//		int nRow = index / 3; // 0, 1, 2 (3 Hàng)
//
//		int width = (int)(m_iGapWidth + (m_RoomWidth + m_iGapWidth) * nCol);
//		int height = (int)(m_iGapHeight + (m_RoomHeight + m_iGapHeight) * nRow);
//		// =========================================================================
//
//		if (m_pMapInfo[i].nPeople >= m_pMapInfo[i].nMaxPeople)
//		{
//			bRoomFull = true;
//		}
//
//		iter = find_if(m_pMapImage.begin(), m_pMapImage.end(), string_key_equal<string, MBitmap*>(mapName));
//		if (iter != m_pMapImage.end())
//		{
//			pBitmap = iter->second;
//			if (pBitmap != 0)
//			{
//				if ((m_pMapInfo[i].roomState == GMAE_CLOSED) || bRoomFull)
//					pDC->SetBitmapColor(125, 125, 125, 255);
//				else
//					pDC->SetBitmapColor(255, 255, 255, 255);
//
//				pDC->SetBitmap(pBitmap);
//				pDC->Draw(width + pressed_reposition, height + pressed_reposition, m_RoomWidth, m_RoomHeight);
//			}
//
//			if (m_pRoomFrame != 0)
//			{
//				pDC->SetBitmap(m_pRoomFrame);
//				pDC->Draw(width + pressed_reposition, height + pressed_reposition, m_RoomWidth * 0.75, m_RoomHeight, 0, 0, 512, 32);
//			}
//		}
//
//		char szBuf[128];
//		MRECT r;
//
//		r.x = width + m_RoomWidth * 0.01f + pressed_reposition;
//		r.y = height + m_RoomHeight * 0.1f + pressed_reposition;
//		r.w = m_RoomWidth * 0.1;
//		r.h = m_RoomHeight * 0.5;
//		sprintf(szBuf, "%03d", m_pMapInfo[i].RoomNumber);
//
//		pDC->SetFont(MFontManager::Get("FONTc8b"));
//		pDC->SetColor(0, 0, 0);
//		pDC->Text(MRECT(r.x + 1, r.y + 1, r.w, r.h), szBuf);
//		if (m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull)
//			pDC->SetColor(115, 146, 173);
//		else
//			pDC->SetColor(181, 247, 66);
//		pDC->Text(r, szBuf);
//
//		r.x = width + m_RoomWidth * 0.01f + pressed_reposition;
//		r.y = height + m_RoomHeight * 0.5f + pressed_reposition;
//		r.w = m_RoomWidth * 0.1f;
//		r.h = m_RoomHeight * 0.5f;
//		sprintf(szBuf, "%d/%d", m_pMapInfo[i].nPeople, m_pMapInfo[i].nMaxPeople);
//
//		pDC->SetColor(0, 0, 0);
//		pDC->Text(MRECT(r.x + 1, r.y + 1, r.w, r.h), szBuf);
//		if (m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull)
//			pDC->SetColor(115, 146, 173);
//		else
//			pDC->SetColor(181, 247, 66);
//		pDC->Text(r, szBuf);
//
//		pDC->SetFont(MFontManager::Get("FONTc8b"));
//		r.x = width + m_RoomWidth * 0.12 + pressed_reposition;
//		r.y = height  + pressed_reposition;
//		r.w = m_RoomWidth * 0.75;
//		r.h = m_RoomHeight;
//
//		MFont* pFont = pDC->GetFont();
//		char szBufTemp[SMI_MAPNAME_LENGTH];
//		int strLength = 0;
//		int RoomWidth = pFont->GetWidth(m_pMapInfo[i].room_name);
//		if (RoomWidth > m_RoomWidth * 0.7)
//		{
//			while (strLength < 29)
//			{
//				if (m_pMapInfo[i].map_name[strLength] == '0') 
//					strcpy(szBufTemp, m_pMapInfo[i].room_name);
//				if (((unsigned char)m_pMapInfo[i].room_name[strLength]) > 127)
//					strLength += 2;
//				else
//					++strLength;
//			}
//			strncpy(szBufTemp, m_pMapInfo[i].room_name, strLength * sizeof(char));
//			szBufTemp[strLength] = '\0';
//			strcat(szBufTemp, "...");
//
//			pDC->SetColor(0, 0, 0);
//			pDC->Text(MRECT(r.x + 1, r.y + 1, r.w, r.h), szBufTemp, MAM_LEFT);
//			if (m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull)
//				pDC->SetColor(115, 146, 173);
//			else
//				pDC->SetColor(255, 255, 255);
//			pDC->Text(r, szBufTemp, MAM_LEFT);
//		}
//		else
//		{
//			pDC->SetColor(0, 0, 0);
//			pDC->Text(MRECT(r.x + 1, r.y + 1, r.w, r.h), m_pMapInfo[i].room_name, MAM_LEFT);
//			if (m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull)
//				pDC->SetColor(115, 146, 173);
//			else
//				pDC->SetColor(255, 255, 255);
//			pDC->Text(r, m_pMapInfo[i].room_name, MAM_LEFT);
//		}
//
//		r.x = width + m_RoomWidth * 0.75f + pressed_reposition;
//		r.y = height + pressed_reposition;
//		r.w = m_RoomWidth * 0.2f;
//		r.h = m_RoomHeight;
//
//		if (m_pMapInfo[i].bLimitLevel)
//		{
//			char szBufTemp[64];
//			sprintf(szBufTemp, "%d~%d", max(m_pMapInfo[i].nMasterLevel - m_pMapInfo[i].nLimitLevel, 1), m_pMapInfo[i].nMasterLevel + m_pMapInfo[i].nLimitLevel);
//
//			pDC->SetColor(0, 0, 0);
//			pDC->Text(MRECT(r.x + 1, r.y + 1, r.w, r.h), szBufTemp);
//			if (m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull)
//				pDC->SetColor(100, 100, 100);
//			else
//				pDC->SetColor(181, 247, 66);
//			pDC->Text(r, szBufTemp);
//		}
//
//		if (m_pMapInfo[i].bPrivate)
//		{
//
//
//#define FN_ROOMLIST_PRIVATE_KEY		"in_key.png"
//			pBitmap = MBitmapManager::Get(FN_ROOMLIST_PRIVATE_KEY);
//			if (pBitmap != NULL)
//			{
//				float x, y;
//				x = width + m_RoomWidth * 0.9 + pressed_reposition;
//				y = height + pressed_reposition + ((m_RoomHeight - pBitmap->GetHeight()) / 2);
//				pDC->SetBitmap(pBitmap);
//				pDC->Draw(x, y, pBitmap->GetWidth(), pBitmap->GetHeight());
//			}
//		}
//		else
//		{
//			iterIcon = m_pIconImage.find(m_pMapInfo[i].nGame_Type);
//			if (iterIcon != m_pIconImage.end())
//			{
//				pBitmap = iterIcon->second;
//				if (pBitmap != 0)
//				{
//					r.x = width + m_RoomWidth * 0.9 + pressed_reposition;
//					r.y = height + m_RoomHeight / 4.7 + pressed_reposition;
//
//					if (m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull)
//					{
//						pDC->SetBitmapColor(100, 100, 100, 100);
//					}
//					else
//					{
//						pDC->SetBitmapColor(255, 255, 255, 255);
//					}
//
//					pDC->SetBitmap(pBitmap);
//					pDC->Draw(r.x, height + pressed_reposition, m_RoomHeight, m_RoomHeight);
//				}
//			}
//		}
//
//		if (m_pMapInfo[i].roomState == GAME_PLAYING)
//		{
//#define FN_ROOMLIST_PLAYICON		"icon_play.tga"
//			pBitmap = MBitmapManager::Get(FN_ROOMLIST_PLAYICON);
//			if (pBitmap != NULL)
//			{
//				float x, y;
//				x = width + (m_RoomWidth * 0.955) + pressed_reposition;
//				y = height + (m_RoomHeight * 0.54) + pressed_reposition;
//				pDC->SetBitmap(pBitmap);
//				pDC->Draw(x, y, (m_RoomHeight * 0.5), (m_RoomHeight * 0.5));
//			}
//		}
//
//		if (i == m_Selection)
//		{
//			if (m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull) pDC->SetColor(115, 146, 173);
//			else pDC->SetColor(181, 247, 66);
//		}
//		else pDC->SetColor(128, 128, 128, 255);
//		pDC->Rectangle(width + pressed_reposition, height + pressed_reposition, m_RoomWidth, m_RoomHeight);
//
//		++index;
//	}
//	pDC->SetOpacity(255);
//	pDC->SetBitmapColor(255, 255, 255, 255);
//}
void ZRoomListBox::OnDraw(MDrawContext* pDC)
{
	MBitmap* pBitmap;
	MRECT rect;
	map<string, MBitmap*>::iterator iter;
	map<MMATCH_GAMETYPE, MBitmap*>::iterator iterIcon;
	int pressed_reposition = 0;

	int index = 0;

	// for new UI: icon dang duoc tro vao, ve tooltip sau vong lap de no nam tren cung
	bool bHoverTip = false;
	MPOINT ptHoverTip(0, 0);
	char szHoverTip[MWIDGET_NAME_LENGTH];
	szHoverTip[0] = 0;

	for (int i = 0; i < NUM_DISPLAY_ROOM; ++i)
	{
		bool bRoomFull = false;

		if (m_Selection == i) pressed_reposition = 1;
		else                  pressed_reposition = 0;

		const char* mapName = MGetMapDescMgr()->GetBannerName(m_pMapInfo[i].map_name);
		if (m_pMapInfo[i].IsEmpty)
		{
			continue;
		}
		rect = GetInitialClientRect();

		int nCol = index % 3;
		int nRow = index / 3;

		int width = (int)(m_iGapWidth + (m_RoomWidth + m_iGapWidth) * nCol);
		int height = (int)(m_iGapHeight + (m_RoomHeight + m_iGapHeight) * nRow);

		if (m_pMapInfo[i].nPeople >= m_pMapInfo[i].nMaxPeople)
		{
			bRoomFull = true;
		}

		// 1. VẼ BANNER MAP (NỀN)
		iter = find_if(m_pMapImage.begin(), m_pMapImage.end(), string_key_equal<string, MBitmap*>(mapName));
		if (iter != m_pMapImage.end())
		{
			pBitmap = iter->second;
			if (pBitmap != 0)
			{
				if ((m_pMapInfo[i].roomState == GMAE_CLOSED) || bRoomFull)
					pDC->SetBitmapColor(125, 125, 125, 255);
				else
					pDC->SetBitmapColor(255, 255, 255, 255);

				pDC->SetBitmap(pBitmap);
				pDC->Draw(width + pressed_reposition, height + pressed_reposition, m_RoomWidth, m_RoomHeight);
			}

			if (m_pRoomFrame != 0)
			{
				pDC->SetBitmap(m_pRoomFrame);
				pDC->Draw(width + pressed_reposition, height + pressed_reposition, m_RoomWidth, m_RoomHeight);
			}
		}

		char szBuf[128];
		MRECT r;

		// 2. TÊN MAP (CĂN GIỮA CHÍNH GIỮA THẺ CARD)
		pDC->SetFont(MFontManager::Get("FONTa10b")); // Có thể đổi sang font béo/lớn hơn nếu thích
		r.x = width + pressed_reposition;
		r.y = height + (m_RoomHeight * 0.38f) + pressed_reposition;
		r.w = m_RoomWidth;
		r.h = (int)(m_RoomHeight * 0.3f);

		pDC->SetColor(0, 0, 0);
		pDC->Text(MRECT(r.x + 1, r.y + 1, r.w, r.h), m_pMapInfo[i].map_name, MAM_HCENTER); // Bóng đen
		pDC->SetColor(220, 220, 220);
		pDC->Text(r, m_pMapInfo[i].map_name, MAM_HCENTER);


		// 3. GAME MODE (TEXT CĂN GÓC TRÊN BÊN PHẢI)
		pDC->SetFont(MFontManager::Get("FONTc8b"));
		r.x = width + (m_RoomWidth * 0.3f) + pressed_reposition;
		r.y = height + (m_RoomHeight * 0.05f) + pressed_reposition;
		r.w = (int)(m_RoomWidth * 0.65f);
		r.h = (int)(m_RoomHeight * 0.2f);

		// Truyền chỉ số i của vòng lặp phòng vào đây
		const char* szGameType = GetGameTypeStr(i);

		pDC->SetColor(0, 0, 0);
		pDC->Text(MRECT(r.x + 1, r.y + 1, r.w, r.h), szGameType, MAM_RIGHT);
		pDC->SetColor(160, 160, 160);
		pDC->Text(r, szGameType, MAM_RIGHT);


		// 4. SỐ THỨ TỰ + TÊN PHÒNG ([101] Elimination - DƯỚI BÊN TRÁI)
		pDC->SetFont(MFontManager::Get("FONTc8b"));
		r.x = width + (m_RoomWidth * 0.05f) + pressed_reposition;
		r.y = height + (m_RoomHeight * 0.82f) + pressed_reposition;
		r.w = (int)(m_RoomWidth * 0.65f);
		r.h = (int)(m_RoomHeight * 0.15f);

		sprintf(szBuf, "[%03d] %s", m_pMapInfo[i].RoomNumber, m_pMapInfo[i].room_name);

		pDC->SetColor(0, 0, 0);
		pDC->Text(MRECT(r.x + 1, r.y + 1, r.w, r.h), szBuf, MAM_LEFT);
		if (m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull)
			pDC->SetColor(120, 120, 120);
		else
			pDC->SetColor(240, 240, 240);
		pDC->Text(r, szBuf, MAM_LEFT);


		// 5. SỐ NGƯỜI CHƠI ([16/16] - DƯỚI BÊN PHẢI)
		r.x = width + (m_RoomWidth * 0.65f) + pressed_reposition;
		r.y = height + (m_RoomHeight * 0.82f) + pressed_reposition;
		r.w = (int)(m_RoomWidth * 0.30f);
		r.h = (int)(m_RoomHeight * 0.15f);

		sprintf(szBuf, "[%d/%d]", m_pMapInfo[i].nPeople, m_pMapInfo[i].nMaxPeople);

		pDC->SetColor(0, 0, 0);
		pDC->Text(MRECT(r.x + 1, r.y + 1, r.w, r.h), szBuf, MAM_RIGHT);
		if (m_pMapInfo[i].roomState == GMAE_CLOSED || bRoomFull)
			pDC->SetColor(120, 120, 120);
		else
			pDC->SetColor(240, 240, 240);
		pDC->Text(r, szBuf, MAM_RIGHT);


		// 6. KHÓA PHÒNG (NẾU CÓ PASSWORD)
		if (m_pMapInfo[i].bPrivate)
		{
#define FN_ROOMLIST_PRIVATE_KEY "in_key.png"
			pBitmap = MBitmapManager::Get(FN_ROOMLIST_PRIVATE_KEY);
			if (pBitmap != NULL)
			{
				float x = width + (m_RoomWidth * 0.03f) + pressed_reposition;
				float y = height + (m_RoomHeight * 0.05f) + pressed_reposition;
				pDC->SetBitmap(pBitmap);
				pDC->Draw(x, y, 16, 16); // Fix kích thước icon 16x16 gọn gàng
			}
		}
		// 6.5. ICON PLAYING (HIỂN THỊ KHI PHÒNG ĐANG TRONG TRẬN đấu)
		if (m_pMapInfo[i].roomState == GAME_PLAYING)
		{
#define FN_ROOMLIST_PLAYICON "icon_play.tga"
			pBitmap = MBitmapManager::Get(FN_ROOMLIST_PLAYICON);
			if (pBitmap != NULL)
			{
				// Nằm ở góc trên bên trái (nếu có mật khẩu thì tự động lùi sang phải một chút)
				float x = width + (m_pMapInfo[i].bPrivate ? (m_RoomWidth * 0.12f) : (m_RoomWidth * 0.03f)) + pressed_reposition;
				float y = height + (m_RoomHeight * 0.05f) + pressed_reposition;

				pDC->SetBitmap(pBitmap);
				pDC->Draw(x, y, 20, 20); // Đặt cố định 20x20px tránh bị phóng to tràn thẻ
			}
		}

		// 6.7. NHAN MOD PHONG
		{
			vector<MMatchRoomModifier> vMods;
			for (int m = 0; m < m_pMapInfo[i].nRoomModCount && m < MMOD_END; ++m)
				vMods.push_back(m_pMapInfo[i].RoomMods[m]);

			static const char* szModIcon[RMG_COUNT] =
			{
				"RoomMod_Weapon.png",
				"RoomMod_Physic.png",
				"RoomMod_HPSet.png",
				"RoomMod_Damage.png",
				"RoomMod_Gear.png",
				"RoomMod_Etc.png",
			};

			int nIconSize = 30;	// chinh to - nho icon mod ngoai lobby
			int nStep = nIconSize + 3;
			int nX = (int)(width + (m_RoomWidth * 0.03f)) + pressed_reposition;
			int nY = (int)(height + (m_RoomHeight * 0.64f)) + pressed_reposition;

			int nShown = 0, nHidden = 0;

			for (int g = 0; g < RMG_COUNT; ++g)
			{
				vector<MMatchRoomModifier> vGroupMods;
				for (int m = 0; m < (int)vMods.size(); ++m)
				{
					if (GetRoomModGroup(vMods[m].mod) == (ROOM_MOD_GROUP)g)
						vGroupMods.push_back(vMods[m]);
				}
				if (vGroupMods.empty()) continue;

				if (nShown >= 3) { ++nHidden; continue; }

				MRECT rIcon(nX + nStep * nShown, nY, nIconSize, nIconSize);

				pBitmap = MBitmapManager::Get(szModIcon[g]);
				if (pBitmap != NULL)
				{
					pDC->SetBitmap(pBitmap);
					pDC->Draw(rIcon.x, rIcon.y, rIcon.w, rIcon.h);
				}

				if (rIcon.InPoint(m_ptMouse))
				{
					BuildRoomModText(vGroupMods, szHoverTip, sizeof(szHoverTip));
					ptHoverTip = MPOINT(rIcon.x, rIcon.y);
					bHoverTip = true;
				}

				++nShown;
			}

			if (nHidden > 0)
			{
				char szMore[32];
				sprintf(szMore, "+%d", nHidden);
				pDC->SetColor(240, 240, 240);
				pDC->Text(MRECT(nX + nStep * nShown, nY, 30, nIconSize), szMore, MAM_LEFT | MAM_VCENTER);
			}
		}

		// 7. KẺ VIỀN CHỌN ROOM
		if (i == m_Selection)
		{
			pDC->SetColor(181, 247, 66); // Viền màu xanh lá khi chọn
			pDC->Rectangle(width + pressed_reposition, height + pressed_reposition, m_RoomWidth, m_RoomHeight);
		}

		++index;
	}

	// for new UI: tooltip cua nhan mod, ve sau cung de nam tren moi the phong
	if (bHoverTip && szHoverTip[0] != 0)
	{
		MFont* pFont = pDC->GetFont();
		if (pFont != NULL)
		{
			char szBuf[MWIDGET_NAME_LENGTH];
			strncpy(szBuf, szHoverTip, sizeof(szBuf) - 1);
			szBuf[sizeof(szBuf) - 1] = 0;

			int nLineHeight = pFont->GetHeight();
			int nMaxWidth = 0;
			int nLineCount = 0;

			char* szLine = szBuf;
			while (szLine != NULL)
			{
				char* szNext = strchr(szLine, '\n');
				if (szNext != NULL) *szNext = 0;

				int nW = pFont->GetWidth(szLine);
				if (nW > nMaxWidth) nMaxWidth = nW;
				++nLineCount;

				szLine = (szNext != NULL) ? szNext + 1 : NULL;
			}

			int nBoxW = nMaxWidth + 12;
			int nBoxH = nLineHeight * nLineCount + 6;
			int nBoxX = ptHoverTip.x;
			int nBoxY = ptHoverTip.y - nBoxH - 2;
			if (nBoxY < 0) nBoxY = ptHoverTip.y + nLineHeight + 6;

			pDC->SetColor(MCOLOR(DEFCOLOR_MTOOLTIP_PLANE));
			pDC->FillRectangle(nBoxX, nBoxY, nBoxW, nBoxH);
			pDC->SetColor(MCOLOR(DEFCOLOR_MTOOLTIP_OUTLINE));
			pDC->Rectangle(nBoxX, nBoxY, nBoxW, nBoxH);
			pDC->SetColor(MCOLOR(DEFCOLOR_MTOOLTIP_TEXT));

			// szBuf da bi cat thanh nhieu chuoi con o vong do ben tren
			int nTipY = nBoxY + 3;
			szLine = szBuf;
			for (int n = 0; n < nLineCount; ++n)
			{
				pDC->Text(MRECT(nBoxX + 6, nTipY, nMaxWidth, nLineHeight), szLine, MAM_LEFT | MAM_VCENTER);
				nTipY += nLineHeight;
				szLine += strlen(szLine) + 1;
			}
		}
	}

	pDC->SetOpacity(255);
	pDC->SetBitmapColor(255, 255, 255, 255);
}

bool ZRoomListBox::OnShow()
{
	return true;
}

bool ZRoomListBox::OnCommand( MWidget* pWidget, const char* szMassage )
{
	SetPage();
	return true;
}

bool ZRoomListBox::OnEvent(MEvent* pEvent, MListener* pListener)
{
	MRECT r = GetInitialClientRect();
	MPOINT ClickPos = pEvent->Pos;
	MRECT rect;

	// for new UI: ghi lai vi tri chuot cho phan nhan mod
	if (pEvent->nMessage == MWM_MOUSEMOVE)
	{
		if (r.InPoint(ClickPos)) m_ptMouse = ClickPos;
		else m_ptMouse = MPOINT(-1, -1);
	}

	switch (pEvent->nMessage)
	{
		// -----------------------------------------------------------------
		// 1. THÊM TÍNH NĂNG LĂN CHUỘT (Mouse Wheel) ĐỂ CHUYỂN TRANG ROOM
		// -----------------------------------------------------------------
	//case MWM_MOUSEWHEEL:
	//{
	//	if (r.InPoint(ClickPos))
	//	{
	//		if (pEvent->nDelta > 0) // Lăn lên -> Trang trước
	//		{
	//			ZPostRequestPrevStageList();
	//		}
	//		else if (pEvent->nDelta < 0) // Lăn xuống -> Trang sau
	//		{
	//			ZPostRequestNextStageList();
	//		}
	//		return true;
	//	}
	//	break;
	//}

	// -----------------------------------------------------------------
	// 2. SỬA TỌA ĐỘ CLICK CHUỘT TRÁI (LBUTTONDOWN)
	// -----------------------------------------------------------------
	case MWM_LBUTTONDOWN:
	{
		if (r.InPoint(ClickPos))
		{
			int index = 0;
			for (int i = 0; i < NUM_DISPLAY_ROOM; ++i)
			{
				if (m_pMapInfo[i].IsEmpty) continue;

				int nCol = index % 3;
				int nRow = index / 3;

				int width = (int)(m_iGapWidth + (m_RoomWidth + m_iGapWidth) * nCol);
				int height = (int)(m_iGapHeight + (m_RoomHeight + m_iGapHeight) * nRow);

				rect.x = width;
				rect.y = height;
				rect.w = m_RoomWidth;
				rect.h = m_RoomHeight;

				if (rect.InPoint(ClickPos))
				{
					m_Selection = i;
					SetPrivateStageUID(MUID(0, 0));
					return true;
				}
				index++;
			}
			m_Selection = -1;
			return true;
		}
		break;
	}

	// -----------------------------------------------------------------
	// 3. SỬA TỌA ĐỘ DOUBLE CLICK ĐỂ VÀO ROOM (LBUTTONDBLCLK)
	// -----------------------------------------------------------------
	case MWM_LBUTTONDBLCLK:
	{
		if (r.InPoint(ClickPos))
		{
			int index = 0;
			for (int i = 0; i < NUM_DISPLAY_ROOM; ++i)
			{
				if (m_pMapInfo[i].IsEmpty) continue;

				int nCol = index % 3;
				int nRow = index / 3;

				int width = (int)(m_iGapWidth + (m_RoomWidth + m_iGapWidth) * nCol);
				int height = (int)(m_iGapHeight + (m_RoomHeight + m_iGapHeight) * nRow);

				rect.x = width;
				rect.y = height;
				rect.w = m_RoomWidth;
				rect.h = m_RoomHeight;

				if (rect.InPoint(ClickPos))
				{
					m_Selection = i;
					SetPrivateStageUID(MUID(0, 0));

					RequestSelStageJoin();
#ifdef LOBBYSET
					const sMapInfo* pSelRoomInfo = GetSelMapInfo();
					if (pSelRoomInfo)
						ZPOSTCMD2(MC_MATCH_REQUEST_LOBBYSTAGE_INFO, MCmdParamUID(pSelRoomInfo->uidStage), MCmdParamUID(ZGetMyUID()));
#endif
					return true;
				}
				index++;
			}
			m_Selection = -1;
			return true;
		}
		break;
	}
	}
	return false;
}

void ZRoomListBox::SetRoomName( int i, char* pRoomName, sMapInfo* info )
{
	strcpy( m_pMapInfo[i].map_name,  info->map_name );
	strcpy( m_pMapInfo[i].room_name,  info->room_name );
	m_pMapInfo[i].nPeople		= info->nPeople;
	m_pMapInfo[i].RoomNumber	= info->RoomNumber;
	m_pMapInfo[i].nMaxPeople	= info->nMaxPeople;
	m_pMapInfo[i].roomState		= info->roomState;
}

void ZRoomListBox::SetBannerImage( char* pBannerName, MBitmap* pBitmap )
{
	m_pMapImage.insert( map<string, MBitmap*>::value_type( string(pBannerName), pBitmap) );
}
void ZRoomListBox::SetIconImage( MMATCH_GAMETYPE type, MBitmap* pBitmap )
{
	m_pIconImage.insert( map<MMATCH_GAMETYPE, MBitmap*>::value_type( type, pBitmap ) );
}

void ZRoomListBox::SetRoom(const _RoomInfoArg* pRoomInfo)
{
	int nIndex = pRoomInfo->nIndex; 

	if ((pRoomInfo->nIndex < 0) || (pRoomInfo->nIndex >= NUM_DISPLAY_ROOM)) return;

	m_pMapInfo[nIndex].IsEmpty = false;
	m_pMapInfo[nIndex].uidStage = pRoomInfo->uidStage;
	
	strcpy(m_pMapInfo[nIndex].map_name, pRoomInfo->szMapName);
	strcpy(m_pMapInfo[nIndex].room_name, pRoomInfo->szRoomName);
	m_pMapInfo[nIndex].nMaxPeople = pRoomInfo->nMaxPlayers;
	m_pMapInfo[nIndex].nPeople = pRoomInfo->nCurrPlayers;
	m_pMapInfo[nIndex].RoomNumber = pRoomInfo->nRoomNumber;
	m_pMapInfo[nIndex].nGame_Type	= pRoomInfo->nGameType;
	m_pMapInfo[nIndex].bForcedEnter = pRoomInfo->bForcedEntry;
	m_pMapInfo[nIndex].bLimitLevel	=	pRoomInfo->bLimitLevel;
	m_pMapInfo[nIndex].nMasterLevel = pRoomInfo->nMasterLevel;
	m_pMapInfo[nIndex].nLimitLevel = pRoomInfo->nLimitLevel;
	m_pMapInfo[nIndex].bPrivate = pRoomInfo->bPrivate;
	// for new UI
	m_pMapInfo[nIndex].nRoomModCount = pRoomInfo->nRoomModCount;
	if (m_pMapInfo[nIndex].nRoomModCount < 0) m_pMapInfo[nIndex].nRoomModCount = 0;
	if (m_pMapInfo[nIndex].nRoomModCount > MMOD_END) m_pMapInfo[nIndex].nRoomModCount = MMOD_END;
	if (pRoomInfo->pRoomMods != NULL)
		memcpy(m_pMapInfo[nIndex].RoomMods, pRoomInfo->pRoomMods, sizeof(m_pMapInfo[nIndex].RoomMods));
#ifdef _SPEC 1
	m_pMapInfo[nIndex].bObserverEnabled = pRoomInfo->bObserverEnabled;
#endif

 	if( pRoomInfo->nStageState == STAGE_STATE_STANDBY )
	{
		m_pMapInfo[nIndex].roomState = GAME_WAITING; // ´EâÁß
	}
	//else if(	pRoomInfo->nStageState == STAGE_STATE_CLOSE && !pRoomInfo->bForcedEntry )
	//{
	//	m_pMapInfo[nIndex].roomState	= GMAE_CLOSED; // ³­ÀÔ ºÒ°¡
	//}
	else if( pRoomInfo->nStageState == STAGE_STATE_RUN && pRoomInfo->bForcedEntry)
	{
		//m_pMapInfo[nIndex].roomState	= GAME_WAITING;
		m_pMapInfo[nIndex].roomState	= GAME_PLAYING	; // ³­ÀÔ °¡´É
	}
	else
	{
		m_pMapInfo[nIndex].roomState	= GMAE_CLOSED; // ³­ÀÔ ºÒ°¡
	}

}

void ZRoomListBox::SetEmptyRoom(int nIndex)
{
	m_pMapInfo[nIndex].IsEmpty = true;
	m_pMapInfo[nIndex].uidStage = MUID(0,0);
	m_pMapInfo[nIndex].bPrivate = false;
	m_pMapInfo[nIndex].nRoomModCount = 0;
}


void ZRoomListBox::Clear()
{
	m_iNumRoom = 0;
	for (int i = 0; i < NUM_DISPLAY_ROOM; i++)
	{
		SetEmptyRoom(i);
	}
}

void ZRoomListBox::SetPage( )
{
	// ¼­¹ö¿¡ ÇÊ¿äÇÑ ÆäÀÌÁö¸¦ ¿äÃ»ÇÑ´Ù
	//static int oldvalue = 0;
 //	int value = m_pScrollBar->GetValue();
 //	if( abs(oldvalue -value) == 1 )
	//{
	//	if( oldvalue > value )
	//		value -= 19;
	//	else
	//		value += 19;
	//	value = min( max( value, 0 ), 999 );
	//	m_pScrollBar->SetValue( value );
	//}
	//oldvalue = m_pScrollBar->GetValue();
	//int i =min(max((int)m_pScrollBar->GetValue()/100, 0),9);
	//if( i == m_currPage ) return;
	//
	//m_currPage = i;
	//mlog("%d ÆäÀÌÁE¿äÃ»\n",i);
}

void ZRoomListBox::SetScroll(int nPrevStageCount, int nNextStageCount)
{
	m_nNextStageCount = nNextStageCount;
	m_nPrevStageCount = nPrevStageCount;
}

int ZRoomListBox::GetFirstStageCursor()
{
	for (int i = 0; i < NUM_DISPLAY_ROOM; i++)
	{
		if (m_pMapInfo[i].IsEmpty == false)
		{
			return m_pMapInfo[i].RoomNumber - 1;
			break;
		}
	}

	return 0;
}

int ZRoomListBox::GetLastStageCursor()
{
	for (int i = NUM_DISPLAY_ROOM-1; i >= 0; i--)
	{
		if (m_pMapInfo[i].IsEmpty == false)
		{
			return m_pMapInfo[i].RoomNumber - 1;
			break;
		}
	}

	return 0;
}

MUID ZRoomListBox::GetSelRoomUID()
{
	if ((m_Selection >= 0) && (m_Selection < NUM_DISPLAY_ROOM))
	{
		return m_pMapInfo[m_Selection].uidStage;
	}

	return MUID(0,0);
}

MUID ZRoomListBox::GetSelectedPrivateStageUID()
{
	return m_uidSelectedPrivateStageUID;
}

#include "ZMyInfo.h"
void ZRoomListBox::RequestSelStageJoin()
{
	MUID uidChar = ZGetGameClient()->GetPlayerUID();

	if (GetSelRoomUID() != MUID(0,0))
	{
		// Custom: Event Team
		if ( (ZGetMyInfo()->IsAdminGrade() == false) && ZGetMyInfo()->GetUGradeID() != MMUG_GAMEMASTER && (m_pMapInfo[m_Selection].bPrivate) )
		{
			char szStageName[SMI_ROOMNAME_LENGTH] = "";
			MUID uidStage = MUID(0,0);

			const sMapInfo* pSelRoomInfo = GetSelMapInfo();

			if (pSelRoomInfo != NULL)
			{
				strcpy(szStageName, pSelRoomInfo->room_name);
				uidStage = pSelRoomInfo->uidStage;
			}

			SetPrivateStageUID(uidStage);

			// ºñ¹Ð¹E
			ZApplication::GetGameInterface()->ShowPrivateStageJoinFrame(szStageName);
		}
		else
		{
			ZPostRequestStageJoin(uidChar, GetSelRoomUID());	// °ø°³¹E
			ZApplication::GetGameInterface()->EnableLobbyInterface(false);
		}
	}
}

void ZRoomListBox::RequestSelPrivateStageJoin()
{
	MUID uidChar = ZGetGameClient()->GetPlayerUID();
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MUID uidStageUID = GetSelectedPrivateStageUID();

	if (uidStageUID == MUID(0,0)) uidStageUID = GetSelRoomUID();

	if (uidStageUID != MUID(0,0))
	{
		MEdit* pPassEdit = (MEdit*)pResource->FindWidget("PrivateStagePassword");
		if (pPassEdit!=NULL)
		{
			char szPassword[256];
			strcpy(szPassword, pPassEdit->GetText());

			ZPostRequestPrivateStageJoin(uidChar, uidStageUID, szPassword);
			ZApplication::GetGameInterface()->EnableLobbyInterface(false);
		}
	}
}

void ZRoomListBox::Resize( float w, float h )
{
	m_RoomWidth	*= w;
	m_RoomHeight	*= h;
	//jintriple3 ·E¸®½ºÆ® ÇØ»óµµ¿¡ µû¶EÀ§Ä¡°¡ »E¦ º¯°æµÇ´Â ¹ö±×...
	float fGapCenter = (float)m_iGapCenter;
	float fGapWidth=  (float)m_iGapWidth;
	float fGapHeight = (float)m_iGapHeight;
	fGapCenter *= w;
	fGapWidth *= w;
	fGapHeight *=h;

	m_iGapCenter = (int)fGapCenter;
	if(fGapCenter - m_iGapCenter > 0.5f)
		m_iGapCenter++;

	m_iGapWidth = (int)fGapWidth;
	if(fGapWidth - m_iGapWidth > 0.5f)
		m_iGapWidth++;

	m_iGapHeight = (int)fGapHeight;
	if(fGapHeight - m_iGapHeight > 0.5f)
		m_iGapHeight++;

//	m_iGapCenter	*= w;
//	m_iGapWidth	*= w;
//	m_iGapHeight	*= h;
}

const sMapInfo* ZRoomListBox::GetSelMapInfo()
{ 
	if ((m_Selection >= 0) && (m_Selection < NUM_DISPLAY_ROOM)) return &m_pMapInfo[m_Selection];
	return NULL;
}


void ZRoomListBox::SetPrivateStageUID(MUID& uidStage)
{
	m_uidSelectedPrivateStageUID = uidStage;
}

const char* ZRoomListBox::GetGameTypeStr(int nIndex)
{
	// Kiểm tra Boundary an toàn tránh crash
	if (nIndex < 0 || nIndex >= NUM_DISPLAY_ROOM) return "";
	if (m_pMapInfo[nIndex].IsEmpty) return "";

	// Custom: Rule Custom Game
	switch (m_pMapInfo[nIndex].nGame_Type)
	{
	case MMATCH_GAMETYPE_DEATHMATCH_SOLO:
		return "Deathmatch";
	case MMATCH_GAMETYPE_DEATHMATCH_TEAM:
		return "Team Deathmatch";
	case MMATCH_GAMETYPE_GLADIATOR_SOLO:
		return "Gladiator";
	case MMATCH_GAMETYPE_GLADIATOR_TEAM:
		return "Team Gladiator";
	case MMATCH_GAMETYPE_ASSASSINATE:
		return "Assassination";
	case MMATCH_GAMETYPE_TRAINING:
		return "Training";
	case MMATCH_GAMETYPE_SURVIVAL:
		return "Survival";
	case MMATCH_GAMETYPE_QUEST:
		return "Quest";
	case MMATCH_GAMETYPE_BERSERKER:
		return "Deathmatch + berserker";
	case MMATCH_GAMETYPE_DEATHMATCH_TEAM2:
		return "Team Deathmatch + Extreme";
	case MMATCH_GAMETYPE_DUEL:
		return "The duel match";
	case MMATCH_GAMETYPE_QUEST_CHALLENGE:
		return "Challenge Quest";
	case MMATCH_GAMETYPE_TEAM_TRAINING:
		return "Team training";
	case MMATCH_GAMETYPE_CTF:
		return "Capture The Flag";
	case MMATCH_GAMETYPE_INFECTED:
		return "Zombie Infected";
	case MMATCH_GAMETYPE_GUNGAME:
		return "GunGame";
	case MMATCH_GAMETYPE_SPY:
		return "Spy Mode";
	case MMATCH_GAMETYPE_TURBO:
		return "New People";
	case MMATCH_GAMETYPE_SKILLMAP:
		return "SkillMap";
	case MMATCH_GAMETYPE_DROPMAGIC:
		return "Magic Box";
	case MMATCH_GAMETYPE_PAINTBALL_SOLO:
		return "Paint Ball";
	case MMATCH_GAMETYPE_PAINTBALL_TEAM:
		return "Paint Ball Team";
	case MMATCH_GAMETYPE_CLASSIC_SOLO:
		return "Classic Solo";
	case MMATCH_GAMETYPE_CLASSIC_TEAM:
		return "Classic Team";
	case MMATCH_GAMETYPE_MODE_STAFF:
		return "Staff Mode";
	case MMATCH_GAMETYPE_SNIPERMODE:
		return "Sniper";
	case MMATCH_GAMETYPE_VAMPIRE:
		return "Vampire";
	case MMATCH_GAMETYPE_GHOST:
		return "Ghost";
	}
	return "";
}