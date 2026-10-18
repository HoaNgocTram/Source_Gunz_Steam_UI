#include "stdafx.h"
#include "ZEmojiPicker.h"
#include "EmojiManager.h"
#include "AnimatedGifTexture.h"
#include "MDrawContext.h"
#include "MFont.h"
#include "RealSpace2.h"
#include "MZFileSystem.h"
#include "ZApplication.h"
#include "MDebug.h"
#include "ZConfiguration.h"

// ============================================================
//  Singleton
// ============================================================

ZEmojiPicker& ZEmojiPicker::GetInstance()
{
	static ZEmojiPicker instance;
	return instance;
}

ZEmojiPicker::ZEmojiPicker() {}

ZEmojiPicker::~ZEmojiPicker()
{
	Destroy();
}

void ZEmojiPicker::Destroy()
{
	for (auto& pair : m_TexCache)
	{
		if (pair.second)
			pair.second->Release();
	}
	m_TexCache.clear();
}

// ============================================================
//  Show / Hide / Toggle
// ============================================================

void ZEmojiPicker::Toggle()
{
	if (m_bVisible)
		Hide();
	else
		Show();
}

void ZEmojiPicker::Show()
{
	int nActiveSet = ZGetEmojiManager().GetActiveSet();
	if (nActiveSet <= 0 || !ZGetEmojiManager().IsSetLoaded(nActiveSet))
		return;

	const EmojiSet* pSet = ZGetEmojiManager().GetEmojiSet(nActiveSet);
	if (!pSet || pSet->emojis.empty())
		return;

	m_nTotalEmojis = (int)pSet->emojis.size();
	m_nScrollRow = 0;
	m_nHoverIndex = -1;
	m_bVisible = true;

	RecalcLayout();
}

void ZEmojiPicker::Hide()
{
	m_bVisible = false;
	m_nHoverIndex = -1;
}

void ZEmojiPicker::SetAnchor(int nAnchorX, int nAnchorY, int nInputW)
{
	m_nAnchorX = nAnchorX;
	m_nAnchorY = nAnchorY;
	m_nInputW = nInputW;
}

// ============================================================
//  Layout
// ============================================================

void ZEmojiPicker::RecalcLayout()
{
	m_nTotalRows = (m_nTotalEmojis + m_nColumns - 1) / m_nColumns;
	int nVisRows = min(m_nTotalRows, m_nMaxVisibleRows);

	m_nPanelW = m_nColumns * m_nCellSize + 12;
	m_nPanelH = nVisRows * m_nCellSize + m_nHeaderH + 12;

	if (m_nPanelW > m_nInputW && m_nInputW > 100)
		m_nPanelW = m_nInputW;

	// Position: above-right of anchor point
	m_nPanelX = m_nAnchorX + m_nInputW - m_nPanelW; // align right edge with input
	m_nPanelY = m_nAnchorY - m_nPanelH - 4;

	// Clamp to screen
	if (m_nPanelX < 0) m_nPanelX = 0;
	if (m_nPanelY < 0) m_nPanelY = 0;
	if (m_nPanelX + m_nPanelW > RGetScreenWidth())
		m_nPanelX = RGetScreenWidth() - m_nPanelW;
	if (m_nPanelY + m_nPanelH > RGetScreenHeight())
		m_nPanelY = RGetScreenHeight() - m_nPanelH;
}

int ZEmojiPicker::HitTest(int nX, int nY) const
{
	// Grid area starts at (panelX + 6, panelY + headerH + 6)
	int gridX = m_nPanelX + 6;
	int gridY = m_nPanelY + m_nHeaderH + 6;
	int gridW = m_nColumns * m_nCellSize;
	int nVisRows = min(m_nTotalRows - m_nScrollRow, m_nMaxVisibleRows);
	int gridH = nVisRows * m_nCellSize;

	if (nX < gridX || nX >= gridX + gridW || nY < gridY || nY >= gridY + gridH)
		return -1;

	int col = (nX - gridX) / m_nCellSize;
	int row = (nY - gridY) / m_nCellSize;

	if (col >= m_nColumns) return -1;

	int idx = (m_nScrollRow + row) * m_nColumns + col;
	if (idx >= m_nTotalEmojis) return -1;

	return idx;
}

// ============================================================
//  Texture Loading
// ============================================================

std::string ZEmojiPicker::MakeTexKey(const EmojiEntry* pEntry, int nSetId) const
{
	char buf[128];
	sprintf(buf, "%d:%s", nSetId, pEntry->filename.c_str());
	return std::string(buf);
}

LPDIRECT3DTEXTURE9 ZEmojiPicker::GetEmojiTexture(const EmojiEntry* pEntry, int nSetId)
{
	// For animated GIFs, return current frame from EmojiManager
	if (pEntry->bAnimated)
	{
		AnimatedGifTexture* pAnim = ZGetEmojiManager().GetAnimatedTexture(
			pEntry->filename.c_str(), nSetId);
		if (pAnim && pAnim->IsValid())
			return pAnim->GetCurrentFrameTexture();
		return nullptr;
	}

	// For static PNGs, check cache
	std::string key = MakeTexKey(pEntry, nSetId);
	auto it = m_TexCache.find(key);
	if (it != m_TexCache.end())
		return it->second;

	// Load texture from file
	const EmojiSet* pSet = ZGetEmojiManager().GetEmojiSet(nSetId);
	if (!pSet) return nullptr;

	std::string fullPath = pSet->basePath + pEntry->filename;

	// Try MZFile first (supports MRS archives)
	MZFile mzFile;
	bool bOpened = false;
	if (mzFile.Open(fullPath.c_str(), ZApplication::GetFileSystem()))
		bOpened = true;
	else if (mzFile.Open(fullPath.c_str(), (MZFileSystem*)NULL))
		bOpened = true;

	if (!bOpened)
	{
		m_TexCache[key] = nullptr;
		return nullptr;
	}

	unsigned long nSize = mzFile.GetLength();
	uint8_t* pData = new uint8_t[nSize];
	mzFile.Read(pData, nSize);

	LPDIRECT3DTEXTURE9 pTex = nullptr;
	bool bIsEx = false;
    IDirect3DDevice9Ex* pTestEx = nullptr;
    if (SUCCEEDED(RGetDevice()->QueryInterface(IID_PPV_ARGS(&pTestEx)))) {
        bIsEx = true;
        pTestEx->Release();
    }

    HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(
        RGetDevice(), pData, nSize,
        D3DX_DEFAULT, D3DX_DEFAULT, 1, 
        0,                                 // Usage
        D3DFMT_A8R8G8B8, 
        bIsEx ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED, // <--- Sửa Pool ở đây
        D3DX_DEFAULT, D3DX_DEFAULT,
		0xFF000000,                                // Color key: Nên để 0 nếu PNG đã có alpha
        nullptr, nullptr, &pTex);

    delete[] pData;

	if (FAILED(hr))
		pTex = nullptr;

	m_TexCache[key] = pTex;
	return pTex;
}

// ============================================================
//  D3D Helpers
// ============================================================

void ZEmojiPicker::DrawQuad(LPDIRECT3DDEVICE9 pd3d, float x, float y, float w, float h,
	D3DCOLOR color) const
{
	struct V { float x, y, z, rhw; D3DCOLOR c; };
	V verts[4] = {
		{ x,     y,     0, 1, color },
		{ x + w, y,     0, 1, color },
		{ x + w, y + h, 0, 1, color },
		{ x,     y + h, 0, 1, color },
	};

	pd3d->SetTexture(0, nullptr);
	pd3d->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	pd3d->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, verts, sizeof(V));
}

void ZEmojiPicker::DrawQuadTex(LPDIRECT3DDEVICE9 pd3d, LPDIRECT3DTEXTURE9 pTex,
	float x, float y, float w, float h) const
{
	struct V { float x, y, z, rhw; float u, v; };
	V verts[4] = {
		{ x,     y,     0, 1, 0, 0 },
		{ x + w, y,     0, 1, 1, 0 },
		{ x + w, y + h, 0, 1, 1, 1 },
		{ x,     y + h, 0, 1, 0, 1 },
	};

	pd3d->SetTexture(0, pTex);
	pd3d->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
	pd3d->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, verts, sizeof(V));
}

void ZEmojiPicker::DrawText2D(MDrawContext* pDC, int x, int y, const char* szText, D3DCOLOR color) const
{
	if (!pDC || !szText) return;
	pDC->SetColor(MCOLOR(color));
	pDC->Text(x, y, szText);
}

// ============================================================
//  Main Draw
// ============================================================

void ZEmojiPicker::Draw(MDrawContext* pDC)
{
	if (!m_bVisible) return;

	int nActiveSet = ZGetEmojiManager().GetActiveSet();
	const EmojiSet* pSet = ZGetEmojiManager().GetEmojiSet(nActiveSet);
	if (!pSet || pSet->emojis.empty())
	{
		Hide();
		return;
	}

	// Recalc in case screen changed
	RecalcLayout();

	LPDIRECT3DDEVICE9 pd3d = RGetDevice();
	if (!pd3d) return;

	// Save render states
	DWORD dwPrevAlpha, dwPrevSrc, dwPrevDst, dwPrevLighting;
	pd3d->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwPrevAlpha);
	pd3d->GetRenderState(D3DRS_SRCBLEND, &dwPrevSrc);
	pd3d->GetRenderState(D3DRS_DESTBLEND, &dwPrevDst);
	pd3d->GetRenderState(D3DRS_LIGHTING, &dwPrevLighting);

	pd3d->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pd3d->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pd3d->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pd3d->SetRenderState(D3DRS_LIGHTING, FALSE);

	// Texture states for proper rendering
	pd3d->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pd3d->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3d->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pd3d->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	// --- Background panel ---
	DrawQuad(pd3d, (float)m_nPanelX, (float)m_nPanelY,
		(float)m_nPanelW, (float)m_nPanelH,
		D3DCOLOR_ARGB(230, 25, 25, 30));

	// --- Border ---
	// Top
	DrawQuad(pd3d, (float)m_nPanelX, (float)m_nPanelY, (float)m_nPanelW, 1, D3DCOLOR_ARGB(200, 80, 80, 90));
	// Bottom
	DrawQuad(pd3d, (float)m_nPanelX, (float)(m_nPanelY + m_nPanelH - 1), (float)m_nPanelW, 1, D3DCOLOR_ARGB(200, 80, 80, 90));
	// Left
	DrawQuad(pd3d, (float)m_nPanelX, (float)m_nPanelY, 1, (float)m_nPanelH, D3DCOLOR_ARGB(200, 80, 80, 90));
	// Right
	DrawQuad(pd3d, (float)(m_nPanelX + m_nPanelW - 1), (float)m_nPanelY, 1, (float)m_nPanelH, D3DCOLOR_ARGB(200, 80, 80, 90));

	// --- Header ---
	char szHeader[64];
	const char* setNames[] = { "", "Default", "Noto", "Facebook", "Yahoo" };
	const char* setName = (nActiveSet >= 1 && nActiveSet <= 4) ? setNames[nActiveSet] : "Emoji";
	sprintf(szHeader, "Emoji: %s (%d)", setName, m_nTotalEmojis);

	// Header separator line
	DrawQuad(pd3d, (float)(m_nPanelX + 4), (float)(m_nPanelY + m_nHeaderH),
		(float)(m_nPanelW - 8), 1, D3DCOLOR_ARGB(150, 80, 80, 90));

	// Reset texture for text drawing
	pd3d->SetTexture(0, nullptr);
	DrawText2D(pDC, m_nPanelX + 8, m_nPanelY + 4, szHeader, D3DCOLOR_ARGB(220, 200, 200, 200));

	// --- Scroll indicator ---
	if (m_nTotalRows > m_nMaxVisibleRows)
	{
		char szScroll[32];
		sprintf(szScroll, "%d/%d", m_nScrollRow + 1, m_nTotalRows - m_nMaxVisibleRows + 1);
		int sw = pDC->GetFont() ? pDC->GetFont()->GetWidth(szScroll) : 40;
		pd3d->SetTexture(0, nullptr);
		DrawText2D(pDC, m_nPanelX + m_nPanelW - sw - 8, m_nPanelY + 4, szScroll,
			D3DCOLOR_ARGB(180, 150, 150, 150));
	}

	// --- Emoji Grid ---
	int gridX = m_nPanelX + 6;
	int gridY = m_nPanelY + m_nHeaderH + 6;
	int nVisRows = min(m_nTotalRows - m_nScrollRow, m_nMaxVisibleRows);

	for (int row = 0; row < nVisRows; row++)
	{
		for (int col = 0; col < m_nColumns; col++)
		{
			int idx = (m_nScrollRow + row) * m_nColumns + col;
			if (idx >= m_nTotalEmojis) break;

			const EmojiEntry& entry = pSet->emojis[idx];
			int cellX = gridX + col * m_nCellSize;
			int cellY = gridY + row * m_nCellSize;
			bool bHover = (idx == m_nHoverIndex);

			// Hover highlight
			if (bHover)
			{
				DrawQuad(pd3d, (float)cellX, (float)cellY,
					(float)m_nCellSize, (float)m_nCellSize,
					D3DCOLOR_ARGB(100, 80, 120, 200));
			}

			// Draw emoji image centered in cell
			int pad = (m_nCellSize - m_nEmojiSize) / 2;
			int imgX = cellX + pad;
			int imgY = cellY + pad;

			LPDIRECT3DTEXTURE9 pTex = GetEmojiTexture(&entry, nActiveSet);
			if (pTex)
			{
				// For animated GIFs, auto-calc aspect ratio
				int drawW = m_nEmojiSize;
				int drawH = m_nEmojiSize;

				if (entry.bAnimated)
				{
					AnimatedGifTexture* pAnim = ZGetEmojiManager().GetAnimatedTexture(
						entry.filename.c_str(), nActiveSet);
					if (pAnim && pAnim->GetWidth() > 0 && pAnim->GetHeight() > 0)
					{
						int srcW = pAnim->GetWidth();
						int srcH = pAnim->GetHeight();
						if (srcW > srcH)
						{
							drawH = m_nEmojiSize;
							drawW = (int)((float)m_nEmojiSize * srcW / srcH);
							if (drawW > m_nCellSize - 2) drawW = m_nCellSize - 2;
						}
						else if (srcH > srcW)
						{
							drawW = m_nEmojiSize;
							drawH = (int)((float)m_nEmojiSize * srcH / srcW);
						}
						imgX = cellX + (m_nCellSize - drawW) / 2;
						imgY = cellY + (m_nCellSize - drawH) / 2;
					}
				}

				DrawQuadTex(pd3d, pTex, (float)imgX, (float)imgY, (float)drawW, (float)drawH);
			}
			else
			{
				// Fallback: draw pattern text
				pd3d->SetTexture(0, nullptr);
				DrawText2D(pDC, cellX + 2, cellY + m_nCellSize / 2 - 6,
					entry.pattern.c_str(), D3DCOLOR_ARGB(200, 180, 180, 180));
			}
		}
	}

	// --- Tooltip on hover ---
	if (m_nHoverIndex >= 0 && m_nHoverIndex < m_nTotalEmojis)
	{
		const EmojiEntry& entry = pSet->emojis[m_nHoverIndex];
		char szTip[128];
		sprintf(szTip, " %s ", entry.pattern.c_str());

		MFont* pFont = pDC->GetFont();
		int tipW = pFont ? pFont->GetWidth(szTip) + 8 : 60;
		int tipH = pFont ? pFont->GetHeight() + 6 : 18;

		// Position tooltip below the panel
		int tipX = m_nPanelX;
		int tipY = m_nPanelY + m_nPanelH + 2;

		// Clamp
		if (tipX + tipW > RGetScreenWidth()) tipX = RGetScreenWidth() - tipW;

		DrawQuad(pd3d, (float)tipX, (float)tipY, (float)tipW, (float)tipH,
			D3DCOLOR_ARGB(220, 40, 40, 45));
		DrawQuad(pd3d, (float)tipX, (float)tipY, (float)tipW, 1,
			D3DCOLOR_ARGB(180, 100, 100, 110));

		pd3d->SetTexture(0, nullptr);
		DrawText2D(pDC, tipX + 4, tipY + 3, szTip, D3DCOLOR_ARGB(255, 255, 220, 100));
	}

	// Restore states
	pd3d->SetTexture(0, nullptr);
	pd3d->SetRenderState(D3DRS_ALPHABLENDENABLE, dwPrevAlpha);
	pd3d->SetRenderState(D3DRS_SRCBLEND, dwPrevSrc);
	pd3d->SetRenderState(D3DRS_DESTBLEND, dwPrevDst);
	pd3d->SetRenderState(D3DRS_LIGHTING, dwPrevLighting);
}

// ============================================================
//  Input Handling
// ============================================================

bool ZEmojiPicker::OnMouseMove(int nX, int nY)
{
	if (!m_bVisible) return false;
	m_nHoverIndex = HitTest(nX, nY);
	return (nX >= m_nPanelX && nX < m_nPanelX + m_nPanelW &&
		nY >= m_nPanelY && nY < m_nPanelY + m_nPanelH);
}

bool ZEmojiPicker::OnLButtonDown(int nX, int nY)
{
	if (!m_bVisible) return false;

	int idx = HitTest(nX, nY);
	if (idx >= 0)
	{
		int nActiveSet = ZGetEmojiManager().GetActiveSet();
		const EmojiSet* pSet = ZGetEmojiManager().GetEmojiSet(nActiveSet);
		if (pSet && idx < (int)pSet->emojis.size())
		{
			const EmojiEntry& entry = pSet->emojis[idx];
			if (m_fnInsert)
				m_fnInsert(entry.pattern.c_str());
			Hide();
			return true;
		}
	}

	// Click outside panel = close
	if (nX < m_nPanelX || nX >= m_nPanelX + m_nPanelW ||
		nY < m_nPanelY || nY >= m_nPanelY + m_nPanelH)
	{
		Hide();
		return false; // don't consume, let the click go through
	}

	return true; // consumed by panel but no emoji picked
}

bool ZEmojiPicker::OnMouseWheel(int nDelta)
{
	if (!m_bVisible) return false;

	if (nDelta > 0)
	{
		if (m_nScrollRow > 0) m_nScrollRow--;
	}
	else if (nDelta < 0)
	{
		int maxScroll = max(0, m_nTotalRows - m_nMaxVisibleRows);
		if (m_nScrollRow < maxScroll) m_nScrollRow++;
	}
	return true;
}

bool ZEmojiPicker::OnKeyDown(int nKey)
{
	if (!m_bVisible) return false;

	if (nKey == VK_ESCAPE || nKey == VK_TAB)
	{
		Hide();
		return true;
	}

	return false;
}
