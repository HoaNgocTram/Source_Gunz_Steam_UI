#include "stdafx.h"
#ifdef _GLOBALANNOUNCE
#include "ZGlobalAnnounce.h"
#include "EmojiManager.h"
#include "AnimatedGifTexture.h"
#include "ZConfiguration.h"
#include "ZApplication.h"
#include "MZFileSystem.h"
#include "RealSpace2.h"

// ============================================================
//  Helpers
// ============================================================

static BYTE LerpByte(BYTE a, BYTE b, float t)
{
	return (BYTE)((float)a + ((float)b - (float)a) * t);
}

static DWORD LerpColor(DWORD c1, DWORD c2, float t)
{
	if (t <= 0.0f) return c1;
	if (t >= 1.0f) return c2;
	BYTE a = LerpByte((c1 >> 24) & 0xFF, (c2 >> 24) & 0xFF, t);
	BYTE r = LerpByte((c1 >> 16) & 0xFF, (c2 >> 16) & 0xFF, t);
	BYTE g = LerpByte((c1 >> 8) & 0xFF, (c2 >> 8) & 0xFF, t);
	BYTE b = LerpByte((c1) & 0xFF, (c2) & 0xFF, t);
	return ((DWORD)a << 24) | ((DWORD)r << 16) | ((DWORD)g << 8) | (DWORD)b;
}

// ============================================================
//  Constructor / Destructor
// ============================================================

ZGlobalAnnounce::ZGlobalAnnounce()
{
	m_bActive = false;
	m_dwStartTime = 0;
	m_dwLastUpdateTime = 0;
	m_fScrollOffset = 0;
	m_nTotalDisplayTime = 0;
	m_nSingleScrollTime = 0;
	m_bNeedScroll = false;
	m_nScrollDist = 0;
	m_szGlobalMsg[0] = 0;
	m_nMsgEmojiSet = 0;
	m_pIconTexture = NULL;
	m_bIconLoaded = false;
	LoadConfig();
}

ZGlobalAnnounce::~ZGlobalAnnounce()
{
	if (m_pIconTexture)
	{
		m_pIconTexture->Destroy();
		delete m_pIconTexture;
		m_pIconTexture = NULL;
	}
	m_Queue.clear();
}

// ============================================================
//  Queue management
// ============================================================

void ZGlobalAnnounce::SetGlobalMessage(const char* pszMessage)
{
	if (!pszMessage || strlen(pszMessage) == 0)
		return;

	if (!m_bActive)
	{
		// Không có message đang hiện → hiện ngay
		ActivateMessage(pszMessage);
		ZGetSoundEngine()->PlaySound("wall_notify");
	}
	else
	{
		// Đang hiện message → thêm vào queue
		if ((int)m_Queue.size() < ANNOUNCE_MAX_QUEUE)
		{
			m_Queue.push_back(std::string(pszMessage));
		}
	}
}

void ZGlobalAnnounce::ActivateMessage(const char* szMsg)
{
	strncpy(m_szGlobalMsg, szMsg, sizeof(m_szGlobalMsg) - 1);
	m_szGlobalMsg[sizeof(m_szGlobalMsg) - 1] = 0;

	// Strip emoji prefix once, store set for both CalcTextWidth and DrawTextWithEmoji
	m_nMsgEmojiSet = ZGetEmojiManager().GetActiveSet();
	{ int nParsedSet = EmojiManager::ScanAndStripPrefix(m_szGlobalMsg, (int)strlen(m_szGlobalMsg)); if (nParsedSet >= 0) m_nMsgEmojiSet = nParsedSet; }

	m_bActive = true;
	m_dwStartTime = timeGetTime();
	m_dwLastUpdateTime = m_dwStartTime;
	m_fScrollOffset = 0;

	// Load icon lần đầu
	if (!m_bIconLoaded)
		LoadIcon();

	// Tính render width
	MFont* pFont = MFontManager::Get(m_Config.szFontName);
	if (!pFont) pFont = MFontManager::Get("FONTa10_O2Wht");

	int nScreenW = MGetWorkspaceWidth();
	int nScreenH = MGetWorkspaceHeight();
	float fScale = (float)nScreenH / 600.0f;
	int nPadScaled = (int)(m_Config.nBoxPadding * fScale);
	int nBoxInnerW = (int)(m_Config.fBoxWidth * (float)nScreenW) - (nPadScaled * 2);
	int nTextW = pFont ? CalcRenderWidth(pFont) : 200;

	m_bNeedScroll = (nTextW > nBoxInnerW);

	if (!m_bNeedScroll)
	{
		m_nTotalDisplayTime = m_Config.nDisplayTime;
		m_nScrollDist = 0;
		m_nSingleScrollTime = 0;
	}
	else
	{
		m_nScrollDist = nTextW - nBoxInnerW;
		int nFade = m_Config.nFadeTime;
		int nPause = m_Config.nScrollPause;
		int nScrollTime = (int)((float)m_nScrollDist / m_Config.fScrollSpeed * 1000.0f);
		m_nSingleScrollTime = nPause + nScrollTime + nPause;
		m_nTotalDisplayTime = nFade + (m_Config.nRepeatCount * m_nSingleScrollTime) + nFade;
	}
}

void ZGlobalAnnounce::AdvanceQueue()
{
	if (!m_Queue.empty())
	{
		std::string szNext = m_Queue.front();
		m_Queue.pop_front();
		ActivateMessage(szNext.c_str());
		ZGetSoundEngine()->PlaySound("wall_notify");
	}
}

void ZGlobalAnnounce::Clear()
{
	m_szGlobalMsg[0] = 0;
	m_bActive = false;
	m_fScrollOffset = 0;
	m_nTotalDisplayTime = 0;
	m_nSingleScrollTime = 0;
	m_bNeedScroll = false;
	m_nScrollDist = 0;

	// Chuyển sang message tiếp theo
	AdvanceQueue();
}

void ZGlobalAnnounce::ClearAll()
{
	m_Queue.clear();
	m_szGlobalMsg[0] = 0;
	m_bActive = false;
	m_fScrollOffset = 0;
	m_nTotalDisplayTime = 0;
	m_nSingleScrollTime = 0;
	m_bNeedScroll = false;
	m_nScrollDist = 0;
}

// ============================================================
//  Icon loading
// ============================================================

bool ZGlobalAnnounce::LoadIcon()
{
	m_bIconLoaded = true;	// Đánh dấu đã thử, không thử lại

	if (strlen(m_Config.szIconFile) == 0)
		return false;

	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();
	if (!pd3dDevice)
		return false;

	MZFile mzFile;
	if (!mzFile.Open(m_Config.szIconFile, ZApplication::GetFileSystem()))
	{
		if (!mzFile.Open(m_Config.szIconFile, (MZFileSystem*)NULL))
		{
			mlog("ZGlobalAnnounce: Cannot open icon file %s\n", m_Config.szIconFile);
			return false;
		}
	}

	unsigned long fileSize = mzFile.GetLength();
	uint8_t* fileData = new uint8_t[fileSize];
	mzFile.Read(fileData, fileSize);

	m_pIconTexture = new AnimatedGifTexture();
	if (!m_pIconTexture->CreateFromMemory(fileData, (int)fileSize, pd3dDevice))
	{
		delete m_pIconTexture;
		m_pIconTexture = NULL;
		delete[] fileData;
		mlog("ZGlobalAnnounce: Failed to load icon GIF %s\n", m_Config.szIconFile);
		return false;
	}

	delete[] fileData;
	return true;
}

// ============================================================
//  Render width tính cả emoji
// ============================================================

int ZGlobalAnnounce::CalcRenderWidth(MFont* pFont)
{
	if (!pFont) return 0;
	const char* szText = m_szGlobalMsg;
	int nLen = (int)strlen(szText);
	int totalW = 0;
	int segStart = 0;
	char szSeg[512];
	bool bEmoji = Z_ETC_EMOTE && ZGetEmojiManager().IsLoaded();
	int nMsgEmojiSet = m_nMsgEmojiSet;
	if (nMsgEmojiSet > 0) bEmoji = ZGetEmojiManager().IsSetLoaded(nMsgEmojiSet);

	for (int i = 0; i < nLen; )
	{
		if (bEmoji)
		{
			EmojiMatchResult match = ZGetEmojiManager().FindEmoji(szText, i, nLen, nMsgEmojiSet);
			if (match.pEntry)
			{
				if (i > segStart)
				{
					int segLen = i - segStart;
					if (segLen >= (int)sizeof(szSeg)) segLen = sizeof(szSeg) - 1;
					strncpy(szSeg, szText + segStart, segLen);
					szSeg[segLen] = 0;
					totalW += pFont->GetWidth(szSeg);
				}
				int nEmojiW, nDummyH; ZGetEmojiManager().GetDrawSize(match.pEntry, false, nMsgEmojiSet, nEmojiW, nDummyH);
				totalW += nEmojiW;
				i += match.nPatternLen;
				segStart = i;
				continue;
			}
		}
		i++;
	}

	if (segStart < nLen)
	{
		int segLen = nLen - segStart;
		if (segLen >= (int)sizeof(szSeg)) segLen = sizeof(szSeg) - 1;
		strncpy(szSeg, szText + segStart, segLen);
		szSeg[segLen] = 0;
		totalW += pFont->GetWidth(szSeg);
	}

	return totalW;
}

// ============================================================
//  Alpha & Color
// ============================================================

float ZGlobalAnnounce::CalcAlpha(DWORD dwNow)
{
	int dwElapsed = (int)(dwNow - m_dwStartTime);
	int nFade = m_Config.nFadeTime;
	if (dwElapsed >= m_nTotalDisplayTime) return 0.0f;
	if (dwElapsed < nFade) return (float)dwElapsed / (float)nFade;
	if (dwElapsed > (m_nTotalDisplayTime - nFade))
		return (float)(m_nTotalDisplayTime - dwElapsed) / (float)nFade;
	return 1.0f;
}

DWORD ZGlobalAnnounce::ApplyAlpha(DWORD dwColor, float fAlpha)
{
	int a = (int)((float)((dwColor >> 24) & 0xFF) * fAlpha);
	if (a > 255) a = 255;
	if (a < 0) a = 0;
	return ((DWORD)a << 24) | (dwColor & 0x00FFFFFF);
}

DWORD ZGlobalAnnounce::GetCyclingColor(DWORD dwNow, float fAlpha)
{
	if (!m_Config.bColorCycle || m_Config.nCycleColorCount < 2)
		return ApplyAlpha(m_Config.dwTextColor, fAlpha);

	int nCycleLen = m_Config.nColorCycleSpeed * m_Config.nCycleColorCount;
	int nTime = (int)(dwNow % (DWORD)nCycleLen);
	int nColorIdx = nTime / m_Config.nColorCycleSpeed;
	float fT = (float)(nTime % m_Config.nColorCycleSpeed) / (float)m_Config.nColorCycleSpeed;
	int nNext = (nColorIdx + 1) % m_Config.nCycleColorCount;
	DWORD dwColor = LerpColor(m_Config.dwCycleColors[nColorIdx], m_Config.dwCycleColors[nNext], fT);
	return ApplyAlpha(dwColor, fAlpha);
}

// ============================================================
//  Emoji rendering
// ============================================================

void ZGlobalAnnounce::DrawEmojiStatic(MDrawContext* pDC, const EmojiEntry* pEntry, int x, int y, int w, int h, int nSetId)
{
	MBitmap* pBmp = MBitmapManager::Get(pEntry->filename.c_str());
	if (pBmp)
	{
		pDC->SetBitmap(pBmp);
		pDC->Draw(x, y, w, h);
	}
}

void ZGlobalAnnounce::DrawEmojiAnimated(const EmojiEntry* pEntry, int x, int y, int w, int h, int nSetId)
{
	AnimatedGifTexture* pAnimTex = ZGetEmojiManager().GetAnimatedTexture(pEntry->filename.c_str(), nSetId);
	if (!pAnimTex || !pAnimTex->IsValid()) return;
	LPDIRECT3DTEXTURE9 pFrameTex = pAnimTex->GetCurrentFrameTexture();
	if (!pFrameTex) return;
	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();
	if (!pd3dDevice) return;

	float fx = (float)x, fy = (float)y, fw = (float)w, fh = (float)h;
	DWORD dwPrevAB, dwPrevSB, dwPrevDB, dwPrevCO, dwPrevCA, dwPrevAO, dwPrevAA;
	pd3dDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwPrevAB);
	pd3dDevice->GetRenderState(D3DRS_SRCBLEND, &dwPrevSB);
	pd3dDevice->GetRenderState(D3DRS_DESTBLEND, &dwPrevDB);
	pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwPrevCO);
	pd3dDevice->GetTextureStageState(0, D3DTSS_COLORARG1, &dwPrevCA);
	pd3dDevice->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwPrevAO);
	pd3dDevice->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwPrevAA);

	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	pd3dDevice->SetTexture(0, pFrameTex);
	pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

	struct GIFVERT { float x, y, z, rhw, u, v; };
	GIFVERT verts[4] = {
		{ fx,      fy,      0.f, 1.f, 0.f, 0.f },
		{ fx + fw, fy,      0.f, 1.f, 1.f, 0.f },
		{ fx + fw, fy + fh, 0.f, 1.f, 1.f, 1.f },
		{ fx,      fy + fh, 0.f, 1.f, 0.f, 1.f },
	};
	pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, verts, sizeof(GIFVERT));

	pd3dDevice->SetTexture(0, nullptr);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwPrevAB);
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, dwPrevSB);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, dwPrevDB);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwPrevCO);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, dwPrevCA);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, dwPrevAO);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwPrevAA);
}

// ============================================================
//  Icon & Queue counter drawing
// ============================================================

void ZGlobalAnnounce::DrawIcon(int x, int y, int w, int h, float fAlpha)
{
	if (!m_pIconTexture || !m_pIconTexture->IsValid())
		return;

	LPDIRECT3DTEXTURE9 pFrameTex = m_pIconTexture->GetCurrentFrameTexture();
	if (!pFrameTex) return;
	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();
	if (!pd3dDevice) return;

	float fx = (float)x, fy = (float)y, fw = (float)w, fh = (float)h;

	DWORD dwPrevAB, dwPrevSB, dwPrevDB, dwPrevCO, dwPrevCA, dwPrevAO, dwPrevAA;
	pd3dDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwPrevAB);
	pd3dDevice->GetRenderState(D3DRS_SRCBLEND, &dwPrevSB);
	pd3dDevice->GetRenderState(D3DRS_DESTBLEND, &dwPrevDB);
	pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwPrevCO);
	pd3dDevice->GetTextureStageState(0, D3DTSS_COLORARG1, &dwPrevCA);
	pd3dDevice->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwPrevAO);
	pd3dDevice->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwPrevAA);

	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	// Apply alpha via diffuse color
	DWORD dwAlpha = ((DWORD)(fAlpha * 255.0f)) & 0xFF;
	DWORD dwDiffuse = (dwAlpha << 24) | 0x00FFFFFF;

	pd3dDevice->SetTexture(0, pFrameTex);
	pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	struct ICONVERT { float x, y, z, rhw; DWORD color; float u, v; };
	ICONVERT verts[4] = {
		{ fx,      fy,      0.f, 1.f, dwDiffuse, 0.f, 0.f },
		{ fx + fw, fy,      0.f, 1.f, dwDiffuse, 1.f, 0.f },
		{ fx + fw, fy + fh, 0.f, 1.f, dwDiffuse, 1.f, 1.f },
		{ fx,      fy + fh, 0.f, 1.f, dwDiffuse, 0.f, 1.f },
	};
	pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, verts, sizeof(ICONVERT));

	pd3dDevice->SetTexture(0, nullptr);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwPrevAB);
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, dwPrevSB);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, dwPrevDB);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwPrevCO);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, dwPrevCA);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, dwPrevAO);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwPrevAA);
}

void ZGlobalAnnounce::DrawQueueCount(MDrawContext* pDC, int x, int y, int nCount, float fAlpha)
{
	if (nCount <= 0) return;

	char szCount[16];
	sprintf(szCount, "+%d", nCount);

	MFont* pFont = MFontManager::Get("FONTa10_O2Wht");
	if (!pFont) return;

	pDC->SetFont(pFont);
	pDC->SetColor(MCOLOR(ApplyAlpha(0xFFFFFFFF, fAlpha)));
	pDC->Text(x, y, szCount);
}

// ============================================================
//  DrawTextWithEmoji
// ============================================================

void ZGlobalAnnounce::DrawTextWithEmoji(MDrawContext* pDC, int nStartX, int nTextY,
	DWORD dwTextColor, DWORD dwShadowColor)
{
	MFont* pFont = pDC->GetFont();
	if (!pFont) return;

	const char* szText = m_szGlobalMsg;
	int nLen = (int)strlen(szText);
	int nCurX = nStartX;
	int nFontH = pFont->GetHeight();
	char szSeg[512];
	int segStart = 0;
	bool bEmoji = Z_ETC_EMOTE && ZGetEmojiManager().IsLoaded();
	bool bShadow = m_Config.bShadow;
	int nMsgEmojiSet = m_nMsgEmojiSet;
	if (nMsgEmojiSet > 0) bEmoji = ZGetEmojiManager().IsSetLoaded(nMsgEmojiSet);

	int nBoxH = (int)(m_Config.nBoxHeight * ((float)MGetWorkspaceHeight() / 600.0f));

	for (int i = 0; i < nLen; )
	{
		bool bFoundEmoji = false;
		if (bEmoji)
		{
			EmojiMatchResult match = ZGetEmojiManager().FindEmoji(szText, i, nLen, nMsgEmojiSet);
			if (match.pEntry)
			{
				bFoundEmoji = true;
				if (i > segStart)
				{
					int segLen = i - segStart;
					if (segLen >= (int)sizeof(szSeg)) segLen = sizeof(szSeg) - 1;
					strncpy(szSeg, szText + segStart, segLen);
					szSeg[segLen] = 0;
					if (bShadow)
					{
						pDC->SetColor(MCOLOR(dwShadowColor));
						pDC->Text(nCurX + m_Config.nShadowOffset, nTextY + m_Config.nShadowOffset, szSeg);
					}
					pDC->SetColor(MCOLOR(dwTextColor));
					pDC->Text(nCurX, nTextY, szSeg);
					nCurX += pFont->GetWidth(szSeg);
				}

				int nEmojiW, nEmojiH; ZGetEmojiManager().GetDrawSize(match.pEntry, false, nMsgEmojiSet, nEmojiW, nEmojiH);
				// Co emoji nếu to hơn hộp
				if (nEmojiH > nBoxH - 4)
				{
					float ratio = (float)(nBoxH - 4) / (float)nEmojiH;
					nEmojiH = nBoxH - 4;
					nEmojiW = (int)(nEmojiW * ratio);
				}
				int emojiY = nTextY + (nFontH - nEmojiH) / 2;

				if (match.pEntry->bAnimated)
					DrawEmojiAnimated(match.pEntry, nCurX, emojiY, nEmojiW, nEmojiH, nMsgEmojiSet);
				else
					DrawEmojiStatic(pDC, match.pEntry, nCurX, emojiY, nEmojiW, nEmojiH, nMsgEmojiSet);

				nCurX += nEmojiW;
				i += match.nPatternLen;
				segStart = i;
				continue;
			}
		}
		if (!bFoundEmoji) i++;
	}

	if (segStart < nLen)
	{
		int segLen = nLen - segStart;
		if (segLen >= (int)sizeof(szSeg)) segLen = sizeof(szSeg) - 1;
		strncpy(szSeg, szText + segStart, segLen);
		szSeg[segLen] = 0;
		if (bShadow)
		{
			pDC->SetColor(MCOLOR(dwShadowColor));
			pDC->Text(nCurX + m_Config.nShadowOffset, nTextY + m_Config.nShadowOffset, szSeg);
		}
		pDC->SetColor(MCOLOR(dwTextColor));
		pDC->Text(nCurX, nTextY, szSeg);
	}
}

// ============================================================
//  DrawAnnounce — main draw
// ============================================================

void ZGlobalAnnounce::DrawAnnounce(MDrawContext* pDC)
{
	if (!m_bActive || strlen(m_szGlobalMsg) == 0)
		return;

	DWORD dwNow = timeGetTime();
	int dwElapsed = (int)(dwNow - m_dwStartTime);

	if (dwElapsed >= m_nTotalDisplayTime)
	{
		Clear();	// Clear sẽ gọi AdvanceQueue
		return;
	}

	float fAlpha = CalcAlpha(dwNow);
	if (fAlpha <= 0.01f) return;

	MFont* pFont = MFontManager::Get(m_Config.szFontName);
	if (!pFont) pFont = MFontManager::Get("FONTa10_O2Wht");
	if (!pFont) return;
	pDC->SetFont(pFont);

	int nScreenW = MGetWorkspaceWidth();
	int nScreenH = MGetWorkspaceHeight();
	float fScale = (float)nScreenH / 600.0f;

	int nBoxW = (int)(m_Config.fBoxWidth * (float)nScreenW);
	int nBoxH = (int)(m_Config.nBoxHeight * fScale);
	int nBoxX = (nScreenW - nBoxW) / 2;
	int nBoxY = (int)(m_Config.fPosY * (float)nScreenH);
	int nPad = (int)(m_Config.nBoxPadding * fScale);
	int nBoxInnerW = nBoxW - (nPad * 2);

	int nRenderW = CalcRenderWidth(pFont);
	int nTextHeight = pFont->GetHeight();

	// === Icon — bên trái hộp ===
	int nIconSize = (int)(m_Config.nIconSize * fScale);
	int nIconX = nBoxX - nIconSize - (int)(4 * fScale);
	int nIconY = nBoxY + (nBoxH - nIconSize) / 2;
	DrawIcon(nIconX, nIconY, nIconSize, nIconSize, fAlpha);

	// === Queue counter — phía trên icon ===
	int nQueueCount = (int)m_Queue.size();
	if (nQueueCount > 0)
	{
		int nCountY = nIconY - (int)(12 * fScale);
		DrawQueueCount(pDC, nIconX, nCountY, nQueueCount, fAlpha);
	}

	// === Vẽ nền + viền hộp ===
	pDC->SetFont(pFont);	// Restore font sau DrawQueueCount
	pDC->SetColor(MCOLOR(ApplyAlpha(m_Config.dwBgColor, fAlpha)));
	pDC->FillRectangle(nBoxX, nBoxY, nBoxW, nBoxH);
	pDC->SetColor(MCOLOR(ApplyAlpha(m_Config.dwBorderColor, fAlpha)));
	pDC->Rectangle(nBoxX, nBoxY, nBoxW, nBoxH);

	// === Vị trí text ===
	int nTextY = nBoxY + (nBoxH - nTextHeight) / 2;
	int nTextX;

	if (!m_bNeedScroll)
	{
		nTextX = nBoxX + nPad + (nBoxInnerW - nRenderW) / 2;
	}
	else
	{
		int nFade = m_Config.nFadeTime;
		int nPause = m_Config.nScrollPause;
		int nScrollTime = m_nSingleScrollTime - (nPause * 2);

		int nContentElapsed = dwElapsed - nFade;
		if (nContentElapsed < 0) nContentElapsed = 0;

		int nRepeatIdx = nContentElapsed / m_nSingleScrollTime;
		int nInRepeat = nContentElapsed % m_nSingleScrollTime;

		if (nRepeatIdx >= m_Config.nRepeatCount)
			m_fScrollOffset = (float)m_nScrollDist;
		else if (nInRepeat < nPause)
			m_fScrollOffset = 0;
		else if (nInRepeat < (nPause + nScrollTime))
		{
			float fProgress = (float)(nInRepeat - nPause) / (float)nScrollTime;
			if (fProgress > 1.0f) fProgress = 1.0f;
			m_fScrollOffset = fProgress * (float)m_nScrollDist;
		}
		else
			m_fScrollOffset = (float)m_nScrollDist;

		nTextX = nBoxX + nPad - (int)m_fScrollOffset;
	}

	// === D3D Scissor Rect ===
	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();
	DWORD dwPrevScissor = 0;
	RECT prevScissorRect = { 0, 0, 0, 0 };
	bool bScissorSet = false;

	if (pd3dDevice)
	{
		pd3dDevice->GetRenderState(D3DRS_SCISSORTESTENABLE, &dwPrevScissor);
		pd3dDevice->GetScissorRect(&prevScissorRect);
		RECT scissor = { nBoxX + nPad, nBoxY, nBoxX + nPad + nBoxInnerW, nBoxY + nBoxH };
		pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
		pd3dDevice->SetScissorRect(&scissor);
		bScissorSet = true;
	}

	DWORD dwTextColor = GetCyclingColor(dwNow, fAlpha);
	DWORD dwShadowColor = ApplyAlpha(m_Config.dwShadowColor, fAlpha);
	DrawTextWithEmoji(pDC, nTextX, nTextY, dwTextColor, dwShadowColor);

	if (bScissorSet && pd3dDevice)
	{
		pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, dwPrevScissor);
		pd3dDevice->SetScissorRect(&prevScissorRect);
	}
}

// ============================================================
//  LoadConfig
// ============================================================

bool ZGlobalAnnounce::LoadConfig(const char* szFileName)
{
	MXmlDocument xmlDoc;
	xmlDoc.Create();

	if (!xmlDoc.LoadFromFile(szFileName))
	{
		xmlDoc.Destroy();
		return false;
	}

	MXmlElement rootElement, childElement;
	char szTagName[256];

	rootElement = xmlDoc.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		childElement = rootElement.GetChildNode(i);
		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, "ANNOUNCE"))
		{
			char szBuf[512];

			if (childElement.GetAttribute(szBuf, "font", ""))
				if (strlen(szBuf) > 0)
					strncpy(m_Config.szFontName, szBuf, sizeof(m_Config.szFontName) - 1);

			if (childElement.GetAttribute(szBuf, "text_color", ""))
				if (strlen(szBuf) > 0) m_Config.dwTextColor = strtoul(szBuf, NULL, 16);
			if (childElement.GetAttribute(szBuf, "bg_color", ""))
				if (strlen(szBuf) > 0) m_Config.dwBgColor = strtoul(szBuf, NULL, 16);
			if (childElement.GetAttribute(szBuf, "border_color", ""))
				if (strlen(szBuf) > 0) m_Config.dwBorderColor = strtoul(szBuf, NULL, 16);
			if (childElement.GetAttribute(szBuf, "shadow_color", ""))
				if (strlen(szBuf) > 0) m_Config.dwShadowColor = strtoul(szBuf, NULL, 16);

			float fVal;
			childElement.GetAttribute(&fVal, "pos_y", m_Config.fPosY); m_Config.fPosY = fVal;
			childElement.GetAttribute(&fVal, "box_width", m_Config.fBoxWidth); m_Config.fBoxWidth = fVal;
			childElement.GetAttribute(&fVal, "scroll_speed", m_Config.fScrollSpeed); m_Config.fScrollSpeed = fVal;

			int nVal;
			childElement.GetAttribute(&nVal, "box_height", m_Config.nBoxHeight); m_Config.nBoxHeight = nVal;
			childElement.GetAttribute(&nVal, "display_time", m_Config.nDisplayTime); m_Config.nDisplayTime = nVal;
			childElement.GetAttribute(&nVal, "fade_time", m_Config.nFadeTime); m_Config.nFadeTime = nVal;
			childElement.GetAttribute(&nVal, "padding", m_Config.nBoxPadding); m_Config.nBoxPadding = nVal;
			childElement.GetAttribute(&nVal, "scroll_pause", m_Config.nScrollPause); m_Config.nScrollPause = nVal;
			childElement.GetAttribute(&nVal, "repeat", m_Config.nRepeatCount); m_Config.nRepeatCount = nVal;
			if (m_Config.nRepeatCount < 1) m_Config.nRepeatCount = 1;

			int nShadow = m_Config.bShadow ? 1 : 0;
			childElement.GetAttribute(&nShadow, "shadow", nShadow); m_Config.bShadow = (nShadow != 0);
			childElement.GetAttribute(&nVal, "shadow_offset", m_Config.nShadowOffset); m_Config.nShadowOffset = nVal;

			int nCycle = 0;
			childElement.GetAttribute(&nCycle, "color_cycle", 0); m_Config.bColorCycle = (nCycle != 0);
			childElement.GetAttribute(&nVal, "color_speed", m_Config.nColorCycleSpeed); m_Config.nColorCycleSpeed = nVal;
			if (m_Config.nColorCycleSpeed < 50) m_Config.nColorCycleSpeed = 50;

			m_Config.nCycleColorCount = 0;
			if (childElement.GetAttribute(szBuf, "colors", ""))
			{
				if (strlen(szBuf) > 0)
				{
					char* pToken = strtok(szBuf, ",");
					while (pToken && m_Config.nCycleColorCount < ANNOUNCE_MAX_COLORS)
					{
						while (*pToken == ' ') pToken++;
						m_Config.dwCycleColors[m_Config.nCycleColorCount++] = strtoul(pToken, NULL, 16);
						pToken = strtok(NULL, ",");
					}
				}
			}

			// Icon config
			if (childElement.GetAttribute(szBuf, "icon", ""))
				if (strlen(szBuf) > 0)
					strncpy(m_Config.szIconFile, szBuf, sizeof(m_Config.szIconFile) - 1);

			childElement.GetAttribute(&nVal, "icon_size", m_Config.nIconSize); m_Config.nIconSize = nVal;

			break;
		}
	}

	xmlDoc.Destroy();
	return true;
}

#endif // _GLOBALANNOUNCE