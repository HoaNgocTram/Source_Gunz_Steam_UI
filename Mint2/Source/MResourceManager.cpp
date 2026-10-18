#include "stdafx.h"
#include "MResourceManager.h"
#include <algorithm>

// MBitmapManager Implementation
//////////////////////////////
//CMLinkedList<MBitmap> MBitmapManager::m_Bitmaps;
//CMLinkedList<MAniBitmap> MBitmapManager::m_AniBitmaps;
std::map<std::string, MBitmap*> MBitmapManager::m_Bitmaps;
std::map<std::string, MAniBitmap*> MBitmapManager::m_AniBitmaps;

void MBitmapManager::Destroy(void)
{
	for (std::map<std::string, MBitmap*>::iterator it = m_Bitmaps.begin(); it != m_Bitmaps.end(); ++it) {
		if (it->second) {
			it->second->Destroy();
			delete it->second; // Nếu bitmap được tạo bằng new
		}
	}
	m_Bitmaps.clear();
}

void MBitmapManager::DestroyAniBitmap(void)
{
	for (std::map<std::string, MAniBitmap*>::iterator it = m_AniBitmaps.begin(); it != m_AniBitmaps.end(); ++it) {
		if (it->second) {
			it->second->Destroy();
			delete it->second; // Nếu bitmap được tạo bằng new
		}
	}
	m_AniBitmaps.clear();
}

void MBitmapManager::Add(MBitmap* pBitmap)
{
	if (pBitmap == NULL) return;
	std::string name = pBitmap->m_szName;
	std::transform(name.begin(), name.end(), name.begin(), ::tolower); // Ép về chữ thường
	m_Bitmaps[name] = pBitmap;
}

void MBitmapManager::Add(MAniBitmap* pAniBitmap)
{
	if (pAniBitmap == NULL) return;
	std::string name = pAniBitmap->m_szName;
	std::transform(name.begin(), name.end(), name.begin(), ::tolower); // Ép về chữ thường
	m_AniBitmaps[name] = pAniBitmap;
}

void MBitmapManager::Delete(const char* szName)
{
	if (szName == NULL || szName[0] == '\0') return;

	// FIX: Phải ép chữ thường trước khi tìm để xóa
	std::string name = szName;
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);

	// Xóa trong danh sách Bitmap thường
	std::map<std::string, MBitmap*>::iterator it = m_Bitmaps.find(name);
	if (it != m_Bitmaps.end()) {
		// Nếu đại ca muốn giải phóng bộ nhớ luôn thì dùng 2 dòng dưới:
		// if(it->second) { it->second->Destroy(); delete it->second; }
		m_Bitmaps.erase(it);
		return;
	}

	// Xóa trong AniBitmap
	std::map<std::string, MAniBitmap*>::iterator itAni = m_AniBitmaps.find(name);
	if (itAni != m_AniBitmaps.end()) {
		// if(itAni->second) { itAni->second->Destroy(); delete itAni->second; }
		m_AniBitmaps.erase(itAni);
	}
}

MBitmap* MBitmapManager::Get(const char* szBitmapName)
{
	if (szBitmapName == NULL || szBitmapName[0] == '\0') return NULL;

	std::string name = szBitmapName;
	std::transform(name.begin(), name.end(), name.begin(), ::tolower); // QUAN TRỌNG: Phải ép thường ở đây

	std::map<std::string, MBitmap*>::iterator it = m_Bitmaps.find(name);
	if (it != m_Bitmaps.end()) return it->second;

	return NULL;
}

MBitmap* MBitmapManager::Get(int i)
{
	if (i < 0 || i >= (int)m_Bitmaps.size()) return NULL;

	std::map<std::string, MBitmap*>::iterator it = m_Bitmaps.begin();
	std::advance(it, i); // Nhảy đến vị trí thứ i

	if (it != m_Bitmaps.end()) return it->second;
	return NULL;
}

int MBitmapManager::GetCount(void)
{
	return (int)m_Bitmaps.size();
}

MAniBitmap* MBitmapManager::GetAniBitmap(const char* szBitmapName)
{
	if (szBitmapName == NULL || szBitmapName[0] == '\0') {
		if (m_AniBitmaps.empty()) return NULL;
		return m_AniBitmaps.begin()->second;
	}

	// FIX: Phải ép chữ thường ở đây thì mới khớp với lúc Add
	std::string name = szBitmapName;
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);

	std::map<std::string, MAniBitmap*>::iterator it = m_AniBitmaps.find(name);
	if (it != m_AniBitmaps.end()) {
		return it->second;
	}

	return NULL;
}


// MFontManager Implementation
//////////////////////////////
//CMLinkedList<MFont> MFontManager::m_Fonts;
map<string,MFont*>	MFontManager::m_Fonts;
MFont *MFontManager::m_pDefaultFont=NULL;

void MFontManager::SetDefaultFont(MFont *pFont)
{
	m_pDefaultFont=pFont;
}

void MFontManager::Destroy(void)
{
	/*
	for(int i=0; i<m_Fonts.GetCount(); i++){
		MFont* pFont = m_Fonts.Get(i);
		pFont->Destroy();
	}
	m_Fonts.DeleteAll();
	*/

	while(!m_Fonts.empty())
	{
		delete m_Fonts.begin()->second;
		m_Fonts.erase(m_Fonts.begin());
	}
}

void MFontManager::Add(MFont* pFont)
{
//	m_Fonts.Add(pFont);

	map<string,MFont*>::iterator i=m_Fonts.find(string(pFont->m_szName));
	if(i!=m_Fonts.end())
	{
		delete i->second;
		m_Fonts.erase(i);
	}

	m_Fonts.insert(map<string,MFont*>::value_type(string(pFont->m_szName),pFont));
}

MFont* MFontManager::Get(const char* szFontName)
{
	if(szFontName)
	{
		map<string,MFont*>::iterator i=m_Fonts.find(string(szFontName));
		if(i!=m_Fonts.end())
		{
			return i->second;
		}
	}

	if(m_pDefaultFont) return m_pDefaultFont;

	return NULL;

	/*
	if(szFontName==NULL)	// Default Font
		if(m_Fonts.GetCount()>0)
			return m_Fonts.Get(0);

	for(int i=0; i<m_Fonts.GetCount(); i++){
		MFont* pFont = m_Fonts.Get(i);
		if(strcmp(pFont->m_szName, szFontName)==0) return pFont;
	}

	return NULL;
	*/
}

/*
MFont* MFontManager::Get(int i)
{
	if(i<0 || i>=m_Fonts.GetCount()) return NULL;
	return m_Fonts.Get(i);
}
*/

int MFontManager::GetCount(void)
{
//	return m_Fonts.GetCount();
	return m_Fonts.size();
}

void MFontManager::Resize(float ratio, int nMinimumHeight)
{
	map<string,MFont*>::iterator i = m_Fonts.begin();
	while (i!=m_Fonts.end()) {
		if (i->second) {
			i->second->Resize(ratio, nMinimumHeight);
		}
		++i;
	}
}

MCursor* MCursorSystem::m_pCursor;
CMLinkedList<MCursor> MCursorSystem::m_Cursors;
bool MCursorSystem::m_bShow = true;

void MCursorSystem::Destroy(void)
{
	m_Cursors.DeleteAll();
	m_pCursor=NULL;
}

MCursor* MCursorSystem::Set(MCursor* pCursor)
{
	MCursor* temp = m_pCursor;
	m_pCursor = pCursor;
	return temp;
}

MCursor* MCursorSystem::Set(const char* szName)
{
	if(szName==NULL){
		//...
	}
	MCursor* pCursor = Get(szName);
	if(pCursor!=NULL) return Set(pCursor);
	return NULL;
}

MCursor* MCursorSystem::Get(void)
{
	return m_pCursor;
}

MCursor* MCursorSystem::Get(const char* szName)
{
	for(int i=0; i<m_Cursors.GetCount(); i++){
		MCursor* pCursor = m_Cursors.Get(i);
		if(strcmp(pCursor->m_szName, szName)==0) return pCursor;
	}
	return NULL;
}

void MCursorSystem::Add(MCursor* pCursor)
{
	m_Cursors.Add(pCursor);
}

void MCursorSystem::Draw(MDrawContext* pDC, int x, int y)
{
	if(m_bShow==false) return;
	if(m_pCursor!=NULL) m_pCursor->Draw(pDC, x, y);
}

void MCursorSystem::Show(bool bShow)
{
	m_bShow = bShow;
}

bool MCursorSystem::IsVisible(void)
{
	return m_bShow;
}
