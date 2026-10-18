#include "stdafx.h"
#include "ZItemIconBitmap.h"
#include "MZFileSystem.h"
#include "MXml.h"
#include "ZApplication.h"
#include "MBitmap.h"
#include "RealSpace2.h"

ZItemIconMgr::~ZItemIconMgr()
{
	CleanUpAtlas();
}

void ZItemIconMgr::CleanUpAtlas()
{
	for (auto& pair : m_mapAtlasBitmaps)
	{
		if (pair.second != NULL)
		{
			delete pair.second;
		}
	}
	m_mapAtlasBitmaps.clear();
	m_IconMap.clear();
}

bool ZItemIconMgr::ReadXML(const char* szFileName)
{
	MZFile mzf;
	if (!mzf.Open(szFileName, ZGetFileSystem()))
	{
		mlog("ZItemIconMgr::ReadXML - KHONG TIM THAY FILE: %s\n", szFileName);
		return false;
	}

	int nLen = mzf.GetLength();
	if (nLen <= 0)
	{
		mzf.Close();
		return false;
	}

	char* pBuffer = new char[nLen + 1];
	memset(pBuffer, 0, nLen + 1);

	mzf.Read(pBuffer, nLen);
	pBuffer[nLen] = '\0';
	mzf.Close();

	// BẮT BUỘC KHỞI TẠO MXmlDocument
	MXmlDocument xmlDoc;
	xmlDoc.Create(); // <- Rất quan trọng với MXmlDocument của GunZ!

	if (!xmlDoc.LoadFromMemory(pBuffer))
	{
		mlog("ZItemIconMgr::ReadXML - LOI PARSE XML FILE: %s\n", szFileName);
		delete[] pBuffer;
		return false;
	}

	delete[] pBuffer;

	// Duyệt các node <ITEMICONS>
	MXmlElement rootElement = xmlDoc.GetDocumentElement();
	if (rootElement.IsEmpty()) return false;

	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		MXmlNode childNode = rootElement.GetChildNode(i);
		if (childNode.IsEmpty()) continue;

		char szTagName[256] = { 0 };
		childNode.GetNodeName(szTagName);

		if (_stricmp(szTagName, "ITEMICONS") == 0 || _stricmp(szTagName, "ITEMICON") == 0)
		{
			MXmlElement itemElem = childNode;

			char szIconID[64] = { 0 };
			char szSource[256] = { 0 };
			int nOffset = 0;
			int nWidth = 100, nHeight = 100;
			int nAtlasWidth = 1024, nAtlasHeight = 1024;

			itemElem.GetChildContents(szIconID, "ICONID");
			itemElem.GetChildContents(szSource, "SOURCE");
			itemElem.GetChildContents(&nOffset, "OFFSET");

			// Lấy BOUNDS
			MXmlNode boundsNode;
			if (itemElem.FindChildNode("BOUNDS", &boundsNode))
			{
				MXmlElement boundsElem = boundsNode;
				boundsElem.GetChildContents(&nWidth, "W");
				boundsElem.GetChildContents(&nHeight, "H");
			}

			// Lấy FILESIZE
			MXmlNode fileSizeNode;
			if (itemElem.FindChildNode("FILESIZE", &fileSizeNode))
			{
				MXmlElement fileSizeElem = fileSizeNode;
				fileSizeElem.GetChildContents(&nAtlasWidth, "W");
				fileSizeElem.GetChildContents(&nAtlasHeight, "H");
			}

			// Lấy ID số từ ICONID (VD: HB00000 -> 0)
			char* pNumStr = szIconID;
			while (*pNumStr && !isdigit((unsigned char)*pNumStr))
			{
				pNumStr++;
			}
			int nItemID = atoi(pNumStr);

			if (nWidth > 0 && nAtlasWidth > 0)
			{
				int nCols = nAtlasWidth / nWidth;

				ZItemIconInfo info;
				info.strSource = szSource;
				info.nX = (nCols > 0) ? ((nOffset % nCols) * nWidth) : 0;
				info.nY = (nCols > 0) ? ((nOffset / nCols) * nHeight) : 0;
				info.nWidth = nWidth;
				info.nHeight = nHeight;

				m_IconMap[nItemID] = info;
			}
		}
	}

	//mlog("ZItemIconMgr::ReadXML THANH CONG! Da load %d icons tu %s\n", (int)m_IconMap.size(), szFileName);
	return true;
}

bool ZItemIconMgr::GetIconInfo(int nItemID, ZItemIconInfo& outInfo)
{
	auto it = m_IconMap.find(nItemID);
	if (it != m_IconMap.end())
	{
		outInfo = it->second;
		return true;
	}
	return false;
}
MBitmap* ZItemIconMgr::GetAtlasBitmap(const char* szSourceFile)
{
	if (szSourceFile == NULL || strlen(szSourceFile) == 0) return NULL;

	auto it = m_mapAtlasBitmaps.find(szSourceFile);
	if (it != m_mapAtlasBitmaps.end())
	{
		return (MBitmap*)it->second;
	}

	char szFullPath[256] = { 0 };
	sprintf(szFullPath, "interface/default/IconItems/%s", szSourceFile);

	MBitmapR2* pNewBitmap = new MBitmapR2();

	// Truyền RGetDevice() vào vị trí pd3dDevice
	if (!pNewBitmap->Create("ItemIconAtlas", RGetDevice(), szFullPath))
	{
		// Thử tìm ở đường dẫn dự phòng
		sprintf(szFullPath, "interface/default/%s", szSourceFile);
		if (!pNewBitmap->Create("ItemIconAtlas", RGetDevice(), szFullPath))
		{
			mlog("ZItemIconMgr::GetAtlasBitmap - KHONG LOAD DUOC ANH ATLAS: %s\n", szSourceFile);
			delete pNewBitmap;
			return NULL;
		}
	}

	m_mapAtlasBitmaps[szSourceFile] = pNewBitmap;
	return (MBitmap*)pNewBitmap;
}

MBitmap* GetItemIconBitmap(MMatchItemDesc* pItemDesc)
{
	if (pItemDesc == NULL) return NULL;

	// Giả sử ZGetGameInterface()->GetItemIconMgr() là nơi quản lý Icon Manager của bạn
	if (ZGetGameInterface() && ZGetGameInterface()->GetItemIconMgr())
	{
		ZItemIconInfo info;
		if (ZGetGameInterface()->GetItemIconMgr()->GetIconInfo(pItemDesc->m_nID, info))
		{
			return ZGetGameInterface()->GetItemIconMgr()->GetAtlasBitmap(info.strSource.c_str());
		}
	}

	return NULL;
}