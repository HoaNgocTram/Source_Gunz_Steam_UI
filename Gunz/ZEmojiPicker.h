#pragma once
#include <string>
#include <map>
#include <d3d9.h>
#include <d3dx9.h>

// ============================================================
//  ZEmojiPicker — Popup grid for selecting emojis
//  Trigger: ':' + Tab in chat input
//  Renders as D3D9 overlay above chat input
// ============================================================

struct EmojiEntry;
class MDrawContext;

class ZEmojiPicker
{
public:
	static ZEmojiPicker& GetInstance();

	// Show/Hide
	void Toggle();
	void Show();
	void Hide();
	bool IsVisible() const { return m_bVisible; }

	// Set insert callback (called with pattern when emoji picked)
	typedef void(*InsertFunc)(const char* szPattern);
	void SetInsertCallback(InsertFunc fn) { m_fnInsert = fn; }

	// Set position: picker appears ABOVE this Y coordinate
	void SetAnchor(int nAnchorX, int nAnchorY, int nInputW);

	// Rendering (call from draw pipeline)
	void Draw(MDrawContext* pDC);

	// Input — returns true if consumed
	bool OnMouseMove(int nX, int nY);
	bool OnLButtonDown(int nX, int nY);
	bool OnMouseWheel(int nDelta);
	bool OnKeyDown(int nKey);

	// Cleanup textures (call on device lost / shutdown)
	void Destroy();

private:
	ZEmojiPicker();
	~ZEmojiPicker();

	bool m_bVisible = false;
	InsertFunc m_fnInsert = nullptr;

	// Layout
	int m_nColumns = 8;
	int m_nCellSize = 34;     // cell draw size including padding
	int m_nEmojiSize = 28;    // emoji image size within cell
	int m_nMaxVisibleRows = 5;
	int m_nScrollRow = 0;
	int m_nHoverIndex = -1;
	int m_nTotalRows = 0;
	int m_nTotalEmojis = 0;

	// Panel position/size
	int m_nAnchorX = 0;
	int m_nAnchorY = 0;
	int m_nInputW = 400;
	int m_nPanelX = 0;
	int m_nPanelY = 0;
	int m_nPanelW = 0;
	int m_nPanelH = 0;
	int m_nHeaderH = 22;

	void RecalcLayout();
	int  HitTest(int nX, int nY) const;

	// Texture cache: "setId:filename" -> texture
	std::map<std::string, LPDIRECT3DTEXTURE9> m_TexCache;
	LPDIRECT3DTEXTURE9 GetEmojiTexture(const EmojiEntry* pEntry, int nSetId);
	std::string MakeTexKey(const EmojiEntry* pEntry, int nSetId) const;

	// D3D helpers
	void DrawQuad(LPDIRECT3DDEVICE9 pd3d, float x, float y, float w, float h,
		D3DCOLOR color) const;
	void DrawQuadTex(LPDIRECT3DDEVICE9 pd3d, LPDIRECT3DTEXTURE9 pTex,
		float x, float y, float w, float h) const;
	void DrawText2D(MDrawContext* pDC, int x, int y, const char* szText, D3DCOLOR color) const;
};

#define ZGetEmojiPicker() ZEmojiPicker::GetInstance()
