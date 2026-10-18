#include "stdafx.h"
// RBspObject.cpp: 99.2.9 by dubble

#include <crtdbg.h>
#include <map>

#include "rapidxml.hpp"
#include "MZFileSystem.h"
#include "RBspObject.h"
#include "RMaterialList.h"
#include "RealSpace2.h"
#include "RBaseTexture.h"
#include "MDebug.h"
#include "RVersions.h"
#include "RMaterialList.h"
#include "RVisualMeshMgr.h"
#include "FileInfo.h"
#include "ROcclusionList.h"
#include "MProfiler.h"
#include "RShaderMgr.h"
#include "RLenzFlare.h"
#ifdef _WIN64
#include "DxErr.h"
#else
#include "DxErr.h"
#endif
#include "RNavigationNode.h"
#define _MEMFIX 1

//#include "LightmapGenerator.h"

using namespace std;

_NAMESPACE_REALSPACE2_BEGIN


#define TOLERENCE 0.001f
#define SIGN(x) ( (x)<-TOLERENCE ? -1 : (x)>TOLERENCE ? 1 : 0 )

#define MAX_LIGHTMAP_SIZE		1024
#define MAX_LEVEL_COUNT			10			// 2^MAX_LEVEL_COUNT=MAX_LIGHTMAP_SIZE ¿©¾ßÇÑ´Ù..

//#define GENERATE_TEMP_FILES   // ÆÄÀÏ·Î ¶óÀÌÆ®¸ÊÀ» ÀúÀåÇÑ´Ù.

#define DEFAULT_BUFFER_SIZE	1000

// Å×½ºÆ®¿ë
LPDIRECT3DTEXTURE9 g_pShademap = NULL;
// µð¹ö±×¿ë.. 
int nsplitcount = 0, nleafcount = 0;
int g_nPoly, g_nCall;
int g_nPickCheckPolygon, g_nRealPickCheckPolygon;
int g_nFrameNumber = 0;

// ·ÎµùµîÀ» À§ÇØ ÇÊ¿äÇÑ ÀÓ½Ã º¯¼öµé..
int				g_nCreatingPosition;
BSPVERTEX* g_pLPVertices;
WORD* g_pLPIndices;
RSBspNode* g_pLPNode;
RPOLYGONINFO* g_pLPInfo;

rvector* g_pLPColVertices;
RSolidBspNode* g_pLPColNode;


void DrawBoundingBox(rboundingbox* bb, DWORD color)
{
	int i, j;

	int ind[8][3] = { {0,0,0},{1,0,0},{1,1,0},{0,1,0}, {0,0,1},{1,0,1},{1,1,1},{0,1,1} };
	int lines[12][2] = { {0,1},{1,5},{5,4},{4,0},{5,6},{1,2},{0,3},{4,7},{7,6},{6,2},{2,3},{3,7} };

	for (i = 0; i < 12; i++)
	{

		rvector a, b;
		for (j = 0; j < 3; j++)
		{
			a[j] = ind[lines[i][0]][j] ? bb->vmax[j] : bb->vmin[j];
			b[j] = ind[lines[i][1]][j] ? bb->vmax[j] : bb->vmin[j];
		}

		RDrawLine(a, b, color);
	}
}

static bool m_bisDrawLightMap = true;

/////////////////////////////////////////////////////////////
//	RSBspNode

RSBspNode::RSBspNode()
{
	m_pPositive = m_pNegative = NULL;
	nPolygon = 0;
	pInfo = NULL;
	ppInfoSorted = NULL;

	nFrameCount = -1;
	pDrawInfo = NULL;
}

RSBspNode::~RSBspNode()
{
	SAFE_DELETE_ARRAY(ppInfoSorted);
	SAFE_DELETE_ARRAY(pDrawInfo);
}

void RSBspNode::DrawBoundingBox(DWORD color)
{
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	RealSpace2::DrawBoundingBox(&bbTree, color);

	if (m_pNegative) m_pNegative->DrawBoundingBox(color);
	if (m_pPositive) m_pPositive->DrawBoundingBox(color);
}

void RSBspNode::DrawWireFrame(int nPolygon, DWORD color)
{
	RPOLYGONINFO* info = &pInfo[nPolygon];

	for (int i = 0; i < info->nVertices; i++)
	{
		RDrawLine(*info->pVertices[i].Coord(), *info->pVertices[(i + 1) % info->nVertices].Coord(), color);
	}
}

RSBspNode* RSBspNode::GetLeafNode(rvector& pos)
{
	if (nPolygon) return this;
	if (plane.a * pos.x + plane.b * pos.y + plane.c * pos.z + plane.d > 0)
		return m_pPositive->GetLeafNode(pos);
	else
		return m_pNegative->GetLeafNode(pos);
}

/////////////////////////////////////////////////////////////
//	RBspLightmapManager

RBspLightmapManager::RBspLightmapManager()
{
	m_nSize = MAX_LIGHTMAP_SIZE;
	m_pData = new DWORD[MAX_LIGHTMAP_SIZE * MAX_LIGHTMAP_SIZE];
	m_pFreeList = new RFREEBLOCKLIST[MAX_LEVEL_COUNT + 1];

	POINT p = { 0,0 };
	m_pFreeList[MAX_LEVEL_COUNT].push_back(p);
}

RBspLightmapManager::~RBspLightmapManager()
{
	Destroy();
}

void RBspLightmapManager::Destroy()
{
	SAFE_DELETE(m_pData);
	if (m_pFreeList) {
		delete[]m_pFreeList;
		m_pFreeList = NULL;
	}
}

float RBspLightmapManager::CalcUnused()
{
	float fUnused = 0.f;

	for (int i = 0; i <= MAX_LEVEL_COUNT; i++) {
		float fThisLevelSize = pow(0.25, (MAX_LEVEL_COUNT - i));
		fUnused += (float)m_pFreeList[i].size() * fThisLevelSize;
	}

	return fUnused;
}

bool RBspLightmapManager::GetFreeRect(int nLevel, POINT* pt)
{
	if (nLevel > MAX_LEVEL_COUNT) return false;

	if (!m_pFreeList[nLevel].size())		// ÇØ´çÇÏ´Â Å©±âÀÇ ºóºí·°ÀÌ ÇÏ³ªµµ ¾øÀ¸¸é 
	{
		POINT point;
		if (!GetFreeRect(nLevel + 1, &point))	// À­ºí·° ÇÏ³ª¸¦ ¹Þ¾Æ¿Â´Ù. ¸¸¾à ¾øÀ¸¸é ³²´Â°ø°£ÀÌ ¾ø´Â °ÍÀÌ¹Ç·Î ¹Ù·Î ³¡³½´Ù.
			return false;

		int nSize = 1 << nLevel;

		POINT newpoint;						// ¹Þ¾Æ¿Â À­ºí·°À» 4µîºÐÇØ¼­ ÇÑÁ¶°¢À» ¾²°í, ³ª¸ÓÁö ¼¼Á¶°¢À» ºóºí·° ¸®½ºÆ®¿¡ ³Ö¾îµÐ´Ù.

		newpoint.x = point.x + nSize; newpoint.y = point.y;
		m_pFreeList[nLevel].push_back(newpoint);

		newpoint.x = point.x; newpoint.y = point.y + nSize;
		m_pFreeList[nLevel].push_back(newpoint);

		newpoint.x = point.x + nSize; newpoint.y = point.y + nSize;
		m_pFreeList[nLevel].push_back(newpoint);

		*pt = point;

	}
	else
	{
		*pt = *m_pFreeList[nLevel].begin();
		m_pFreeList[nLevel].erase(m_pFreeList[nLevel].begin());
	}

	return true;
}

bool RBspLightmapManager::Add(DWORD* data, int nSize, POINT* retpoint)
{
	int nLevel = 0, nTemp = 1;
	while (nSize > nTemp)
	{
		nTemp = nTemp << 1;
		nLevel++;
	}
	_ASSERT(nSize == nTemp);

	POINT pt;
	if (!GetFreeRect(nLevel, &pt))		// ºó °ø°£ÀÌ ¾øÀ¸¸é ½ÇÆÐ
		return false;

	for (int y = 0; y < nSize; y++)
	{
		for (int x = 0; x < nSize; x++)
		{
			m_pData[(y + pt.y) * GetSize() + (x + pt.x)] = data[y * nSize + x];
		}
	}
	*retpoint = pt;
	return true;
}

void RBspLightmapManager::Save(const char* filename)
{
	RSaveAsBmp(GetSize(), GetSize(), m_pData, filename);
}

RMapObjectList::~RMapObjectList()
{

	for (auto mapObj : m_MapObjectList)
	{
		delete mapObj->pVisualMesh;
		delete mapObj;
		mapObj = nullptr;
	}

	m_MapObjectList.clear();
}

std::vector<ROBJECTINFO*>::iterator RMapObjectList::Delete(std::vector<ROBJECTINFO*>::iterator mapObjItr)
{
	if (mapObjItr != m_MapObjectList.end())
	{
		auto mapObj = *(mapObjItr);
		auto nextItr = m_MapObjectList.erase(mapObjItr);
		if (mapObj)
		{
			delete mapObj->pVisualMesh;
			delete mapObj;
			mapObj = nullptr;
		}
		return nextItr;
	}
	return mapObjItr;
}

////////////////////////////
// RBspObject

RBspObject::RBspObject()
{
	m_pBspRoot = NULL;
	m_pOcRoot = NULL;
	//m_ppLightmapTextures = NULL;
	m_pMaterials = NULL;
	g_pLPVertices = NULL;
	m_pVertexBuffer = NULL;
	m_pIndexBuffer = NULL;
	m_pConvexVertices = NULL;
	m_pConvexPolygons = NULL;
	m_pBspVertices = NULL;
	m_pBspIndices = NULL;
	m_pOcVertices = NULL;
	m_pOcIndices = NULL;
	m_pBspInfo = NULL;
	m_pOcInfo = NULL;

	m_bWireframe = false;
	m_bShowLightmap = false;
	m_AmbientLight = rvector(0, 0, 0);

	m_MeshList.SetMtrlAutoLoad(true);
	m_MeshList.SetMapObject(true);

	m_nMaterial = 0;
	m_nLightmap = 0;

	m_pColVertices = NULL;
	m_pColRoot = NULL;

	m_bNotOcclusion = false;


	m_nPolygon = 0;
	m_nNodeCount = 0;
	m_nBspPolygon = 0;
	m_nBspNodeCount = 0;

	m_nConvexPolygon = 0;
	m_nConvexVertices = 0;
	m_pConvexNormals = NULL;

	m_DebugInfo.pLastColNode = NULL;

	m_pDynLightVertexBuffer = NULL;

	m_OpenMode = ROpenFlag::ROF_RUNTIME;
	m_nBspIndices = 0;
	m_nBspVertices = 0;
	m_nIndices = 0;
	m_nVertices = 0;

	m_lightMapIndex = 0;
	m_isMultiLightMap = false;

}

void RBspObject::ClearLightmaps()
{

	for (size_t i = 0; i < m_LightmapList.size(); ++i)
	{
		delete m_LightmapList[i];
		m_LightmapList[i] = nullptr;
	}
	m_LightmapList.clear();

	for (size_t i = 0; i < m_ppLightmapTextures.size(); ++i)
	{
		for (size_t j = 0; j < m_ppLightmapTextures[i].size(); ++j)
		{
			m_ppLightmapTextures[i][j]->Release();
			m_ppLightmapTextures[i][j] = nullptr;
		}
	}
	m_ppLightmapTextures.clear();


	if (m_nLightmap)
	{
		for (int i = 0; i < m_nPolygon; i++)
			m_pOcInfo[i].nLightmapTexture = 0;
	}

	m_nLightmap = 0;
}

void RBspObject::LightMapOnOff(bool bDraw)
{
	if (m_bisDrawLightMap == bDraw)
		return;

	m_bisDrawLightMap = bDraw;

	if (bDraw)
	{	// ´Ù½Ã ÀÐ´Â´Ù
		if (m_lightMapNames.size() > 0)
		{
			for (int i = 0; i < m_lightMapNames.size(); ++i)
			{
				OpenLightmap(m_lightMapNames[i].c_str());
			}
		}
		else
		{
			OpenLightmap();
		}
	}
	else {	// Áö¿î´Ù~
		ClearLightmaps();
	}

	CreateRenderInfo();
}

void RBspObject::SetDrawLightMap(bool b) {
	m_bisDrawLightMap = b;
}

RBspObject::~RBspObject()
{
	ClearLightmaps();

	OnInvalidate();

	SAFE_RELEASE(m_pDynLightVertexBuffer);

	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);

	SAFE_DELETE_ARRAY(m_pConvexNormals);
	SAFE_DELETE_ARRAY(m_pConvexVertices);
	SAFE_DELETE_ARRAY(m_pConvexPolygons);

	SAFE_DELETE_ARRAY(m_pColVertices);
	if (m_pColRoot)
	{
		delete[]m_pColRoot;
		m_pColRoot = NULL;
	}

	SAFE_DELETE_ARRAY(m_pBspInfo);
	SAFE_DELETE_ARRAY(m_pBspVertices);
	SAFE_DELETE(m_pBspIndices);
	if (m_pBspRoot)
	{
		delete[]m_pBspRoot;
		m_pBspRoot = NULL;
	}

	SAFE_DELETE_ARRAY(m_pOcInfo);
	SAFE_DELETE_ARRAY(m_pOcVertices);
	SAFE_DELETE_ARRAY(m_pOcIndices);
	if (m_pOcRoot)
	{
		delete[]m_pOcRoot;
		m_pOcRoot = NULL;
	}

	if (m_nMaterial)
	{
		for (int i = 0; i < m_nMaterial; i++)
		{
			RDestroyBaseTexture(m_pMaterials[i].texture);
			m_pMaterials[i].texture = NULL;
		}
		if (m_pMaterials)
		{
			delete[]m_pMaterials;
			m_pMaterials = NULL;
		}
	}

	for (RLightList::iterator iter = m_StaticSunLigthtList.begin(); iter != m_StaticSunLigthtList.end(); )
	{
		SAFE_DELETE(*iter);
		iter = m_StaticSunLigthtList.erase(iter);
	}
	m_StaticSunLigthtList.clear();

	for (RLightList::iterator iter = m_StaticMapLightList.begin(); iter != m_StaticMapLightList.end();)
	{
		SAFE_DELETE(*iter);
		iter = m_StaticMapLightList.erase(iter);

	}
	m_StaticMapLightList.clear();

	for (RLightList::iterator iter = m_StaticObjectLightList.begin(); iter != m_StaticObjectLightList.end();)
	{
		SAFE_DELETE(*iter);
		iter = m_StaticObjectLightList.erase(iter);

	}
	m_StaticObjectLightList.clear();


	for (list<AmbSndInfo*>::iterator iter = m_AmbSndInfoList.begin(); iter != m_AmbSndInfoList.end(); )
	{
		AmbSndInfo* p = *iter;
		SAFE_DELETE(p);
		iter = m_AmbSndInfoList.erase(iter);
	}
	m_AmbSndInfoList.clear();

	//TODO: determine if this is necesary?
	//Custom: Clear objs from memory
	for (auto mapObj : m_ObjectList.m_MapObjectList)
	{
		delete mapObj->pVisualMesh;
		delete mapObj;
		mapObj = nullptr;
	}
	m_ObjectList.m_MapObjectList.clear();

	for (auto mapObj : m_SkyList.m_MapObjectList)
	{
		delete mapObj->pVisualMesh;
		delete mapObj;
		mapObj = nullptr;
	}
	m_SkyList.m_MapObjectList.clear();


	//	SAFE_DELETE_ARRAY(m_pSeqIndices);
}

void RBspObject::DrawNormal(int nIndex, float fSize)
{
	int j;
	RCONVEXPOLYGONINFO* pInfo = &m_pConvexPolygons[nIndex];

	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	for (j = 0; j < pInfo->nVertices; j++)
	{
		// draw edge
		RDrawLine(pInfo->pVertices[j], pInfo->pVertices[(j + 1) % pInfo->nVertices], 0xff808080);

		// draw normal
		RDrawLine(pInfo->pVertices[j], pInfo->pVertices[j] + fSize * pInfo->pNormals[j], 0xff00ff);
	}

	/*
	for(j=0;j<pInfo->nSourcePolygons*3;j++)
	{
		RDrawLine(*pInfo->pSourceVertices[j].Coord(),*pInfo->pSourceVertices[j].Coord()+fSize**pInfo->pSourceVertices[j].Normal(),0xff00ff);
	}
	*/
}

//void RBspObject::SetDiffuseMap(int nMaterial)
//{
//	//Custom: Fixed WhiteMaps
//	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();
//
//	RBaseTexture* pTex = m_pMaterials[nMaterial].texture;
//	if (pTex && GetWhiteMap() == 1)
//	{
//		DWORD dwDiffuse = 0xFF7A7A7A;
//		RGetDevice()->SetTexture(0, pTex->GetTexture());
//		RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
//	}
//	else
//	{
//		DWORD dwDiffuse = VECTOR2RGB24(m_pMaterials[nMaterial].Diffuse);
//		RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, dwDiffuse);
//		RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
//	}
//}

void RBspObject::SetDiffuseMap(int nMaterial)
{
	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();

	RBaseTexture* pTex = m_pMaterials[nMaterial].texture;
	if (GetWhiteMap() == 1)
	{
		
		DWORD dwDiffuse = 0xFF7A7A7A;
		pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, dwDiffuse);
		pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	}
	else
	{
		if (pTex)
		{
			pd3dDevice->SetTexture(0, pTex->GetTexture());
			pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		}
	}
}

bool RBspObject::Draw(RSBspNode* pNode, int nMaterial)
{

	if (pNode->nPolygon)
	{
		if (pNode->nFrameCount != g_nFrameNumber) return true;

		int nCount = pNode->pDrawInfo[nMaterial].nTriangleCount;
		if (nCount)
		{
			g_nCall++;
			g_nPoly += nCount;
			HRESULT hr = RGetDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
				pNode->m_nBaseVertexIndex, 0, pNode->m_nVertices,
				pNode->pDrawInfo[nMaterial].nIndicesOffset, nCount);
			return true;
		}
	}
	else
	{
		bool bOk = true;
		if (pNode->m_pNegative) {
			if (!Draw(pNode->m_pNegative, nMaterial))
				bOk = false;
		}
		if (pNode->m_pPositive) {
			if (!Draw(pNode->m_pPositive, nMaterial))
				bOk = false;
		}
		return bOk;
	}
	return true;
}

bool RBspObject::DrawTNT(RSBspNode* pNode, int nMaterial)
{
	if (pNode->nFrameCount != g_nFrameNumber) return true;

	if (pNode->nPolygon)
	{
		int nCount = pNode->pDrawInfo[nMaterial].nTriangleCount;
		int nIndCount = nCount * 3;
		if (nCount)
		{
			g_nPoly += nCount;

			g_nCall++;
			if (nCount)
			{
				int index = pNode->pDrawInfo[nMaterial].nIndicesOffset;
				HRESULT hr = RGetDevice()->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, pNode->pDrawInfo[nMaterial].nVertice, nCount,
					m_pOcIndices + index, D3DFMT_INDEX16, pNode->pDrawInfo[nMaterial].pVertices, sizeof(BSPVERTEX));
				_ASSERT(hr == D3D_OK);
			}
		}
	}
	else
	{
		bool bOk = true;
		if (pNode->m_pNegative) {
			if (!DrawTNT(pNode->m_pNegative, nMaterial))
				bOk = false;
		}
		if (pNode->m_pPositive) {
			if (!DrawTNT(pNode->m_pPositive, nMaterial))
				bOk = false;
		}
		return bOk;
	}
	return true;
}

int g_nChosenNodeCount;

bool RBspObject::Draw()
{

	DrawSky();


	g_nFrameNumber++;
	g_nPoly = 0;
	g_nCall = 0;

	if (RIsHardwareTNL() &&
		(!m_pVertexBuffer || !m_pIndexBuffer)) return false;

	LPDIRECT3DDEVICE9 pLPDIRECT3DDEVICE9 = RGetDevice();

	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	if (RAdvancedGraphics::nMultiSampling > D3DMULTISAMPLE_NONE)
	{
		pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	}

	pLPDIRECT3DDEVICE9->SetFVF(BSP_FVF);

	RGetDevice()->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(BSPVERTEX));
	RGetDevice()->SetIndices(m_pIndexBuffer);
	/*std::vector<LPDIRECT3DTEXTURE9> lightMapTexture;
	if (m_bisDrawLightMap)
	{
		lightMapTexture = m_ppLightmapTextures.at(m_lightMapIndex);
	}*/
	std::vector<LPDIRECT3DTEXTURE9> lightMapTexture; //fix lightmap
	if (m_bisDrawLightMap && !m_ppLightmapTextures.empty())
	{
		if (m_lightMapIndex >= 0 && m_lightMapIndex < (int)m_ppLightmapTextures.size())
			lightMapTexture = m_ppLightmapTextures[m_lightMapIndex];
		else
			lightMapTexture = m_ppLightmapTextures[0]; // fallback
	}

	if (m_bWireframe)
	{
		pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	}
	else
	{
		if (m_bShowLightmap)
		{
			// lightmap ÀÇ ÇÊÅÍ¸µÀ» ²ü´Ï´Ù : µð¹ö±×¿ë
			pLPDIRECT3DDEVICE9->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			pLPDIRECT3DDEVICE9->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

			pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
			pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
			pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);

		}
		else
		{
			bool bTrilinear = RIsTrilinear();

			pLPDIRECT3DDEVICE9->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
			pLPDIRECT3DDEVICE9->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
			pLPDIRECT3DDEVICE9->SetSamplerState(0, D3DSAMP_MIPFILTER, bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);
			pLPDIRECT3DDEVICE9->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 16);
			pLPDIRECT3DDEVICE9->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
			pLPDIRECT3DDEVICE9->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
			pLPDIRECT3DDEVICE9->SetSamplerState(1, D3DSAMP_MIPFILTER, bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);

			pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

			if (!m_nLightmap)
			{
				pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
				pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			}
			else
			{
				pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
				pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
				pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			}
		}
	}

	pLPDIRECT3DDEVICE9->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	pLPDIRECT3DDEVICE9->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	pLPDIRECT3DDEVICE9->SetTexture(0, NULL);
	pLPDIRECT3DDEVICE9->SetTexture(1, NULL);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);


	// world matrix °¡ identity °¡ ¾Æ´Ñ°æ¿ì°¡ ÀÖÀ¸¹Ç·Î camera ¹× viewfrustum À» ¿Å°Ü¾ßÇÑ´Ù
	rmatrix mat;
	RGetDevice()->GetTransform(D3DTS_WORLD, &mat);

	m_OcclusionList.UpdateCamera(mat, RCameraPosition);

	// ¿ì¸®ÀÇ ¸ñÀûÀº viewfrustum Æò¸éÀ» bspÀÇ ·ÎÄÃ·Î º¯È¯ÇØ¾ß ÇÏ¹Ç·Î, inv(mat) ·Î º¯È¯ÇØÁÖ¾î¾ß ÇÑ´Ù
	// ±×·±µ¥ D3DXPlaneTransform ÀÇ »ç¿ë¹ýÀÌ º¯È¯Çà·ÄÀÇ inverse transpose ¸ÅÆ®¸¯½º¸¦ ³Ñ°ÜÁà¾ßÇÏ¹Ç·Î
	// tr(inv(inv(mat))) °¡ µÇ¹Ç·Î °á±¹ tr(mat) °¡ µÈ´Ù

	rmatrix trMat;
	D3DXMatrixTranspose(&trMat, &mat);

	int i;

	for (i = 0; i < 6; i++)
	{
		D3DXPlaneTransform(m_localViewFrustum + i, RGetViewFrustum() + i, &trMat);
	}

	g_nChosenNodeCount = 0;
	_BP("Choose Nodes..");
	ChooseNodes(m_pOcRoot);
	_EP("Choose Nodes..");

	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	RSetWBuffer(true);
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, true);

	int nCount = m_nMaterial * max(1, m_nLightmap);

	for (i = 0; i < nCount; i++)
	{
		int nMaterial = i % m_nMaterial;
		int nLightmap = i / m_nMaterial;
		if ((m_pMaterials[nMaterial].dwFlags & (RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY)) == 0)
		{
			if ((m_pMaterials[nMaterial].dwFlags & RM_FLAG_TWOSIDED) == 0)
				RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			else
				RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

			if (lightMapTexture.size() > 0)
				RGetDevice()->SetTexture(1, lightMapTexture[nLightmap]);

			SetDiffuseMap(nMaterial);
			if (RIsHardwareTNL())
				Draw(m_pOcRoot, i);
			else
				DrawTNT(m_pOcRoot, i);
		}
	}

	// opacity map À» ¾²´Â°Íµé

	for (int i = 0; i < nCount; i++)
	{
		int nMaterial = i % m_nMaterial;
		int nLightmap = i / m_nMaterial;
		if ((m_pMaterials[nMaterial].dwFlags & RM_FLAG_USEOPACITY) == RM_FLAG_USEOPACITY)
		{
			if ((m_pMaterials[nMaterial].dwFlags & RM_FLAG_TWOSIDED) == 0)
				RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			else
				RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

			if (lightMapTexture.size() > 0)
				RGetDevice()->SetTexture(1, lightMapTexture[nLightmap]);

			if ((m_pMaterials[nMaterial].dwFlags & RM_FLAG_USEALPHATEST) != 0) {
				RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, true);
				RGetDevice()->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x80808080);
				RGetDevice()->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
				RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, true);

				RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
				RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			}
			else {
				RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, false);
				RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, false);
				RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
				RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

				RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

				RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
				RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			}


			SetDiffuseMap(nMaterial);
			if (RIsHardwareTNL())
				Draw(m_pOcRoot, i);
			else
				DrawTNT(m_pOcRoot, i);
		}
	}

	// additive ÀÎ°ÍµéÀº ³ªÁß¿¡ Âï´Â´Ù.

	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, false);
	for (int i = 0; i < nCount; i++)
	{
		int nMaterial = i % m_nMaterial;
		int nLightmap = i / m_nMaterial;
		if ((m_pMaterials[nMaterial].dwFlags & RM_FLAG_ADDITIVE) == RM_FLAG_ADDITIVE)
		{
			if ((m_pMaterials[nMaterial].dwFlags & RM_FLAG_TWOSIDED) == 0)
				RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			else
				RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

			if (lightMapTexture.size() > 0)
				RGetDevice()->SetTexture(1, lightMapTexture[nLightmap]);

			SetDiffuseMap(nMaterial);
			if (RIsHardwareTNL())
				Draw(m_pOcRoot, i);
			else
				DrawTNT(m_pOcRoot, i);
		}
	}

	RGetDevice()->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
	RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	RGetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	RGetDevice()->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);

	// state µéÀ» º¹±¸ÇØ³õ°í..
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, true);

	pLPDIRECT3DDEVICE9->SetTexture(0, NULL);
	pLPDIRECT3DDEVICE9->SetTexture(1, NULL);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	pLPDIRECT3DDEVICE9->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);

	return true;
}

void RBspObject::SetObjectLight(rvector vPos)
{
	float fIntensityFirst = FLT_MIN;

	float fdistance = 0.f;
	float fIntensity = 0.f;

	RLIGHT* first = NULL;
	RLIGHT* plight = NULL;

	RLightList* pllist = GetObjectLightList();

	D3DLIGHT9 light;

	ZeroMemory(&light, sizeof(D3DLIGHT9));

	light.Type = D3DLIGHT_POINT;

	light.Attenuation0 = 0.f;
	light.Attenuation1 = 0.0010f;
	light.Attenuation2 = 0.f;

	for (RLightList::iterator i = pllist->begin(); i != pllist->end(); i++) {

		plight = *i;

		fdistance = Magnitude(plight->Position - vPos);
		fIntensity = (fdistance - plight->fAttnStart) / (plight->fAttnEnd - plight->fAttnStart);

		fIntensity = min(max(1.0f - fIntensity, 0), 1);
		fIntensity *= plight->fIntensity;

		fIntensity = min(max(fIntensity, 0), 1);

		if (fIntensityFirst < fIntensity) {
			fIntensityFirst = fIntensity;
			first = plight;
		}
	}

	if (first) {

		light.Position = first->Position;


		light.Ambient.r = min(first->Color.x * first->fIntensity * 0.25, 1.f);
		light.Ambient.g = min(first->Color.y * first->fIntensity * 0.25, 1.f);
		light.Ambient.b = min(first->Color.z * first->fIntensity * 0.25, 1.f);

		light.Diffuse.r = min(first->Color.x * first->fIntensity * 0.25, 1.f);
		light.Diffuse.g = min(first->Color.y * first->fIntensity * 0.25, 1.f);
		light.Diffuse.b = min(first->Color.z * first->fIntensity * 0.25, 1.f);

		light.Specular.r = 1.f;
		light.Specular.g = 1.f;
		light.Specular.b = 1.f;

		light.Range = first->fAttnEnd * 1.0f;

		RGetDevice()->SetLight(0, &light);
		RGetDevice()->LightEnable(0, TRUE);

		RShaderMgr::getShaderMgr()->setLight(0, &light);
	}
}

void RBspObject::SetCharactorLight(rvector pos)
{

}

bool e_mapobject_sort_float(ROBJECTINFO* _a, ROBJECTINFO* _b) {
	if (_a->fDist > _b->fDist)
		return true;
	return false;
}

//void RBspObject::DrawSky()
//{
//	if (m_SkyList.m_MapObjectList.size() == 0)
//		return;
//	ROBJECTINFO* const skyobj = m_SkyList.m_MapObjectList.at(m_lightMapIndex);
//	rmatrix identity;
//	D3DXMatrixIdentity(&identity);
//	skyobj->pVisualMesh->SetWorldMatrix(identity);
//	skyobj->pVisualMesh->Frame();
//	skyobj->pVisualMesh->Render();
//}
void RBspObject::DrawSky() //fix lightmap
{
	if (m_SkyList.m_MapObjectList.empty())
		return;

	ROBJECTINFO* skyobj = nullptr;

	// Kiểm tra giới hạn trước khi truy cập
	if (m_lightMapIndex >= 0 && m_lightMapIndex < (int)m_SkyList.m_MapObjectList.size())
		skyobj = m_SkyList.m_MapObjectList[m_lightMapIndex];
	else
		skyobj = m_SkyList.m_MapObjectList[0]; // fallback an toàn

	if (!skyobj || !skyobj->pVisualMesh)
		return;

	rmatrix identity;
	D3DXMatrixIdentity(&identity);

	skyobj->pVisualMesh->SetWorldMatrix(identity);
	skyobj->pVisualMesh->Frame();
	skyobj->pVisualMesh->Render();
}


void RBspObject::DrawFlags()
{
	rvector camera_pos = RealSpace2::RCameraPosition;
	rvector t_vec;
	rvector t_pos;

	for (const auto& itor : m_FlagList.m_MapObjectList)
	{

		ROBJECTINFO* pCurr = itor;

		if (pCurr == NULL) continue;

		t_pos.x = pCurr->pVisualMesh->GetWorldMat()._41;
		t_pos.y = pCurr->pVisualMesh->GetWorldMat()._42;
		t_pos.z = pCurr->pVisualMesh->GetWorldMat()._43;

		t_vec = camera_pos - t_pos;
		pCurr->fDist = Magnitude(t_vec);
	}

	std::sort(m_FlagList.m_MapObjectList.begin(), m_FlagList.m_MapObjectList.end(), e_mapobject_sort_float);

	for (const auto& iter : m_FlagList.m_MapObjectList)
	{

		if (iter != 0)	iter->pVisualMesh->Render();
	}
}

void RBspObject::DrawObjects()
{
	m_DebugInfo.nMapObjectFrustumCulled = 0;
	m_DebugInfo.nMapObjectOcclusionCulled = 0;


	RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, TRUE);

	//	m_MeshList.Frame();

	D3DXMATRIX world;
	RGetDevice()->GetTransform(D3DTS_WORLD, &world);

	rvector v_add = rvector(world._41, world._42, world._43);

	//	mlog("begin \n");

	rvector camera_pos = RealSpace2::RCameraPosition;
	rvector t_vec;

	for (auto const& worldObj : m_ObjectList.m_MapObjectList)
	{

		ROBJECTINFO* pInfo = worldObj;
		if (!pInfo->pVisualMesh) continue;

		if (pInfo) {
			t_vec = rvector(pInfo->pVisualMesh->GetWorldMat()._41,
				pInfo->pVisualMesh->GetWorldMat()._42,
				pInfo->pVisualMesh->GetWorldMat()._43);
			t_vec = camera_pos - t_vec;
			pInfo->fDist = Magnitude(t_vec);
		}
	}

	std::sort(m_ObjectList.m_MapObjectList.begin(), m_ObjectList.m_MapObjectList.end(), e_mapobject_sort_float);

	for (auto const& worldObj : m_ObjectList.m_MapObjectList)
	{
		ROBJECTINFO* pInfo = worldObj;
		if (!pInfo->pVisualMesh) continue;

		// occlusion test
		rboundingbox bb;
		bb.vmax = pInfo->pVisualMesh->GetBoundMax();
		bb.vmin = pInfo->pVisualMesh->GetBoundMin();

		if (!m_bNotOcclusion) {
			if (!IsVisible(bb)) {
				m_DebugInfo.nMapObjectOcclusionCulled++;
				continue;
			}
		}
		else {
			pInfo->pVisualMesh->SetCheckViewFrustum(false);
		}

		bool bLight = true;

		if (pInfo->pVisualMesh && pInfo->pVisualMesh->GetMesh())
			bLight = !pInfo->pVisualMesh->GetMesh()->m_LitVertexModel;

		if (pInfo->pLight && bLight)
		{
			D3DLIGHT9 light;
			ZeroMemory(&light, sizeof(D3DLIGHT9));

			light.Type = D3DLIGHT_POINT;

			light.Attenuation0 = 0.f;
			light.Attenuation1 = 0.0001f;
			light.Attenuation2 = 0.f;

			light.Position = pInfo->pLight->Position + v_add;

			rvector lightmapcolor(1, 1, 1);

			light.Diffuse.r = pInfo->pLight->Color.x * pInfo->pLight->fIntensity;
			light.Diffuse.g = pInfo->pLight->Color.y * pInfo->pLight->fIntensity;
			light.Diffuse.b = pInfo->pLight->Color.z * pInfo->pLight->fIntensity;

			light.Range = pInfo->pLight->fAttnEnd;

			RGetDevice()->SetLight(0, &light);
			RGetDevice()->LightEnable(0, TRUE);
			RGetDevice()->LightEnable(1, FALSE);
		}

		pInfo->pVisualMesh->SetWorldMatrix(world);
		pInfo->pVisualMesh->Frame();
		pInfo->pVisualMesh->Render(&m_OcclusionList);

		if (!pInfo->pVisualMesh->IsRender()) m_DebugInfo.nMapObjectFrustumCulled++;
	}
}

void RBspObject::DrawBoundingBox()
{
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_pOcRoot->DrawBoundingBox(0xffffff);
}

void RBspObject::DrawOcclusions()
{
	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
	RGetDevice()->SetRenderState(D3DRS_ZENABLE, false);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	for (ROcclusionList::iterator i = m_OcclusionList.begin(); i != m_OcclusionList.end(); i++)
	{
		ROcclusion* poc = *i;

		for (int j = 0; j < poc->nCount; j++)
		{
			RDrawLine(poc->pVertices[j], poc->pVertices[(j + 1) % poc->nCount], 0xffff00ff);
		}
	}

	RSetWBuffer(true);
}

#ifdef _DEBUGBSP
void RBspObject::DrawColNodePolygon(rvector& pos)
{
	m_pColRoot->DrawPos(pos);
}

void RBspObject::DrawCollision_Polygon()
{
	LPDIRECT3DDEVICE9 pLPDIRECT3DDEVICE9 = RGetDevice();

	pLPDIRECT3DDEVICE9->SetFVF(D3DFVF_XYZ);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_TEXTUREFACTOR, 0x406fa867);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	m_pColRoot->DrawPolygon();

	RSetWBuffer(true);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_ZWRITEENABLE, true);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ffffff);
	m_pColRoot->DrawPolygonWireframe();

	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ff00ff);
	m_pColRoot->DrawPolygonNormal();

	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

}

void RBspObject::DrawCollision_Solid()
{
	LPDIRECT3DDEVICE9 pLPDIRECT3DDEVICE9 = RGetDevice();

	pLPDIRECT3DDEVICE9->SetFVF(D3DFVF_XYZ);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40808080);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	m_pColRoot->DrawSolidPolygon();

	RSetWBuffer(true);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_ZWRITEENABLE, true);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ffffff);
	m_pColRoot->DrawSolidPolygonWireframe();

	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ff00ff);
	m_pColRoot->DrawSolidPolygonNormal();

	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

}

void RBspObject::DrawNavi_Polygon()
{
	m_NavigationMesh.Render();
}

void RBspObject::DrawNavi_Links()
{
	m_NavigationMesh.RenderLinks();
}

#else
void RBspObject::DrawColNodePolygon(rvector& pos) {}
void RBspObject::DrawCollision_Polygon() {}
void RBspObject::DrawCollision_Solid() {}
void RBspObject::DrawNavi_Polygon() {}
void RBspObject::DrawNavi_Links() {}

#endif

bool isInViewFrustumWithFarZ(rboundingbox* bb, rplane* plane)
{
	if (isInPlane(bb, plane) && isInPlane(bb, plane + 1) &&
		isInPlane(bb, plane + 2) && isInPlane(bb, plane + 3) &&
		isInPlane(bb, plane + 5)) return true;
	return false;
}

void RBspObject::ChooseNodes(RSBspNode* bspNode)
{
	if (isInViewFrustumWithFarZ(&bspNode->bbTree, m_localViewFrustum))
	{
		if (bspNode->nPolygon)
		{
			if (!IsVisible(bspNode->bbTree)) return;
		}

		g_nChosenNodeCount++;
		bspNode->nFrameCount = g_nFrameNumber;
		if (bspNode->m_pNegative)
			ChooseNodes(bspNode->m_pNegative);
		if (bspNode->m_pPositive)
			ChooseNodes(bspNode->m_pPositive);
	}
}

int RBspObject::ChooseNodes(RSBspNode* bspNode, rvector& center, float fRadius)
{
	if (bspNode == NULL) return 0;

	if (IsInSphere(bspNode->bbTree, center, fRadius))
		bspNode->nFrameCount = g_nFrameNumber;

	if (!bspNode->nPolygon)
	{
		int nm_pNegative = ChooseNodes(bspNode->m_pNegative, center, fRadius);
		int nm_pPositive = ChooseNodes(bspNode->m_pPositive, center, fRadius);

		return 1 + nm_pNegative + nm_pPositive;
	}
	return 0;
}

bool RBspObject::ReadString(MZFile* pfile, char* buffer, int nBufferSize)
{
	int nCount = 0;
	do {
		pfile->Read(buffer, 1);
		nCount++;
		buffer++;
		if (nCount >= nBufferSize)
			return false;
	} while ((*(buffer - 1)) != 0);
	return true;
}

bool bPathOnlyLoad = false;

void DeleteVoidNodes(RSBspNode* pNode)
{
	if (pNode->m_pPositive)
		DeleteVoidNodes(pNode->m_pPositive);
	if (pNode->m_pNegative)
		DeleteVoidNodes(pNode->m_pNegative);

	if (pNode->m_pPositive && !pNode->m_pPositive->nPolygon && !pNode->m_pPositive->m_pPositive && !pNode->m_pPositive->m_pNegative)
	{
		SAFE_DELETE(pNode->m_pPositive);
	}
	if (pNode->m_pNegative && !pNode->m_pNegative->nPolygon && !pNode->m_pNegative->m_pPositive && !pNode->m_pNegative->m_pNegative)
	{
		SAFE_DELETE(pNode->m_pNegative);
	}
}

void RecalcBoundingBox(RSBspNode* pNode)
{
	if (pNode->nPolygon)
	{
		rboundingbox* bb = &pNode->bbTree;
		bb->vmin.x = bb->vmin.y = bb->vmin.z = FLT_MAX;
		bb->vmax.x = bb->vmax.y = bb->vmax.z = -FLT_MAX;
		for (int i = 0; i < pNode->nPolygon; i++)
		{
			RPOLYGONINFO* pInfo = &pNode->pInfo[i];

			for (int j = 0; j < pInfo->nVertices; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					bb->vmin[k] = min(bb->vmin[k], (*pInfo->pVertices[j].Coord())[k]);
					bb->vmax[k] = max(bb->vmax[k], (*pInfo->pVertices[j].Coord())[k]);
				}
			}
		}
	}
	else
	{
		if (pNode->m_pPositive)
		{
			RecalcBoundingBox(pNode->m_pPositive);
			memcpy(&pNode->bbTree, &pNode->m_pPositive->bbTree, sizeof(rboundingbox));
		}
		if (pNode->m_pNegative)
		{
			RecalcBoundingBox(pNode->m_pNegative);
			memcpy(&pNode->bbTree, &pNode->m_pNegative->bbTree, sizeof(rboundingbox));
		}
		if (pNode->m_pPositive) MergeBoundingBox(&pNode->bbTree, &pNode->m_pPositive->bbTree);
		if (pNode->m_pNegative) MergeBoundingBox(&pNode->bbTree, &pNode->m_pNegative->bbTree);
	}
}

bool RBspObject::Open(const char* filename, const char* descExtension, ROpenFlag nOpenFlag, RFPROGRESSCALLBACK pfnProgressCallback, void* CallbackParam)
{
	mlog("BspObject open : begin %s \n", filename);

	m_OpenMode = nOpenFlag;
	m_filename = filename;

	char xmlname[_MAX_PATH];
	sprintf(xmlname, "%s.%s", filename, descExtension);

	if (!OpenDescription(xmlname))
	{
		MLog("Error while loading %s\n", xmlname);
		return false;
	}
	if (pfnProgressCallback) pfnProgressCallback(CallbackParam, .3f);

	mlog("RBspObject::Open : OpenDescription\n");

	if (!OpenRs(filename))
	{
		MLog("Error while loading %s\n", filename);
		return false;
	}

	if (pfnProgressCallback) pfnProgressCallback(CallbackParam, .6f);
	mlog("RBspObject::Open : OpenRs \n");

	/*	// ºÀÀÎ
		char pathfilename[_MAX_PATH];
		sprintf(pathfilename,"%s.pat",filename);
		if((nOpenFlag==ROF_ALL || nOpenFlag==ROF_BSPANDPATH) && !OpenPathNode(pathfilename))
			MLog("Error while loading %s\n",pathfilename);
	*/


	char bspname[_MAX_PATH];
	sprintf(bspname, "%s.bsp", filename);
	if (!OpenBsp(bspname))
	{
		MLog("Error while loading %s\n", bspname);
		return false;
	}

	if (pfnProgressCallback) pfnProgressCallback(CallbackParam, .8f);
	mlog("RBspObject::Open : OpenBsp \n");

	char colfilename[_MAX_PATH];
	sprintf(colfilename, "%s.col", filename);
	if (!OpenCol(colfilename))
	{
		MLog("Error while loading %s\n", colfilename);
		return false;
	}

	// Äù½ºÆ®¸Ê¸¸ ³×ºñ°ÔÀÌ¼Ç ¸ÊÀ» ÀÐ´Â´Ù.
	char navfilename[_MAX_PATH];
	sprintf(navfilename, "%s.nav", filename);
	if (!OpenNav(navfilename))
	{
		//MLog("Error while loading %s\n",navfilename);
		//return false;
	}


	if (RIsHardwareTNL())
	{
		if (!CreateVertexBuffer())
			mlog("Error while Creating VB\n");
	}

	// ¶óÀÌÆ®¸ÊÀ» ÀÐ´Â´Ù
	if (m_bisDrawLightMap)
	{
		if (m_lightMapNames.size() > 0)
		/*{
			for (int i = 0; i < m_lightMapNames.size(); ++i)
			{
				OpenLightmap(m_lightMapNames[i].c_str());
			}
		}*/
			OpenLightmap(m_lightMapNames[m_lightMapIndex].c_str());
		else
			OpenLightmap();
	}

	CreateRenderInfo();

	mlog("RBspObject::Open : done\n");

	if (pfnProgressCallback) pfnProgressCallback(CallbackParam, 1.f);

	return true;
}

void RBspObject::CreateRenderInfo()
{
	Sort_Nodes(m_pOcRoot);

	if (RIsHardwareTNL())
		UpdateVertexBuffer();

	CreatePolygonTableAndIndexBuffer();

	if (RIsHardwareTNL())
	{
		if (!CreateIndexBuffer())
			mlog("Error while Creating IB\n");

		UpdateIndexBuffer();
	}
}

void RBspObject::OptimizeBoundingBox()
{
	DeleteVoidNodes(m_pOcRoot);
	RecalcBoundingBox(m_pOcRoot);
}

bool RBspObject::CreateIndexBuffer()
{
	SAFE_RELEASE(m_pIndexBuffer);
	HRESULT hr;
	if (!g_isDirect3D9ExEnabled) {
		hr = (LPDIRECT3DDEVICE9(RGetDevice())->CreateIndexBuffer(sizeof(WORD) * m_nIndices, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIndexBuffer, NULL));
	}
	else {
		hr = RGetDevice()->CreateIndexBuffer(sizeof(WORD) * m_nIndices, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIndexBuffer, NULL);
	}
	if (D3D_OK != hr) {
		mlog("CreateIndexBuffer failed\n");
		return false;
	}

	return true;
}

bool RBspObject::UpdateIndexBuffer()
{
	if (!m_pIndexBuffer) return false;

	WORD* pIndices = NULL;
	m_pIndexBuffer->Lock(0, 0, (VOID**)&pIndices, 0);
	memcpy(pIndices, m_pOcIndices, sizeof(WORD) * m_nIndices);
	m_pIndexBuffer->Unlock();
	return true;
}


void RBspObject::OnInvalidate()
{
	InvalidateDynamicLightVertexBuffer();
}

void RBspObject::OnRestore()
{
	if (m_bisDrawLightMap)
	{
		//OpenLightmap();
	}
	else
		Sort_Nodes(m_pOcRoot);
}

bool RBspObject::Open_MaterialList(MXmlElement* pElement)
{
	RMaterialList ml;
	ml.Open(pElement);
	///// material Ã³¸®ÇÑ´Ù.

	// 0¹ø µðÆúÆ® ¸ÅÅÍ¸®¾óÀ» ÇÏ³ª ¸¸µç´Ù..  NULL texture
	// ¶Ç ·»´õ¸µ ÇÏÁö ¾Ê´Â (dwFlags¿¡ HIDE °¡ ÄÑÁ®ÀÖÀ¸¸é) material Àº -1¹ø.
	m_nMaterial = (int)ml.size();
	m_nMaterial = m_nMaterial + 1;

	m_pMaterials = new RBSPMATERIAL[m_nMaterial];

	m_pMaterials[0].texture = NULL;	//new RBaseTexture;
	m_pMaterials[0].Diffuse = rvector(1, 1, 1);
	m_pMaterials[0].dwFlags = 0;

	std::string strBase = m_filename;
	string::size_type nPos = strBase.find_last_of("\\"), nothing = -1;
	if (nPos == nothing)
		nPos = strBase.find_last_of("/");

	if (nPos == nothing)
		strBase = "";
	else
		strBase = strBase.substr(0, nPos) + "/";



	RMaterialList::iterator itor;
	itor = ml.begin();

	for (int i = 1; i < m_nMaterial; i++)
	{
		RMATERIAL* mat = *itor;

		m_pMaterials[i].dwFlags = mat->dwFlags;
		m_pMaterials[i].Diffuse = mat->Diffuse;
		m_pMaterials[i].Specular = mat->Specular;
		m_pMaterials[i].Ambient = mat->Ambient;
		m_pMaterials[i].Name = mat->Name;
		m_pMaterials[i].DiffuseMap = mat->DiffuseMap;

		std::string DiffuseMapName = strBase + mat->DiffuseMap;
		char szMapName[256];
		GetRefineFilename(szMapName, DiffuseMapName.c_str());

		if (strlen(szMapName))
		{
			m_pMaterials[i].texture = RCreateBaseTexture(szMapName, RTextureType_Map, true);
		}

		itor++;
	}

	mlog("RBspObject::Open : Open_MaterialList\n");
	return true;
}


bool RBspObject::Open_LightList(MXmlElement* pElement)
{
	RLightList llist;
	llist.Open(pElement);

	for (RLightList::iterator i = llist.begin(); i != llist.end(); i++)
	{
		RLIGHT* plight = *i;
		if (strnicmp(plight->Name.c_str(), RTOK_MAX_OBJLIGHT, strlen(RTOK_MAX_OBJLIGHT)) == 0)
			m_StaticObjectLightList.push_back(plight);
		else {
			// sun light µµ map light ¿¡ Æ÷ÇÔ½ÃÅ²´Ù.
			m_StaticMapLightList.push_back(plight);

			if (strnicmp(plight->Name.c_str(), "sun_omni", 8) == 0)
			{
				RLIGHT* psunlight = new RLIGHT;
				psunlight->Name = plight->Name;
				psunlight->dwFlags = plight->dwFlags;
				psunlight->fAttnEnd = plight->fAttnEnd;
				psunlight->fAttnStart = plight->fAttnStart;
				psunlight->fIntensity = plight->fIntensity;
				psunlight->Position = plight->Position;
				psunlight->Color = plight->Color;
				m_StaticSunLigthtList.push_back(psunlight);
			}
		}
	}
	llist.erase(llist.begin(), llist.end());

	mlog("RBspObject::Open : Open_LightList\n");
	return true;
}

bool RBspObject::Open_OcclusionList(MXmlElement* pElement)
{
	m_OcclusionList.Open(pElement);

	for (ROcclusionList::iterator i = m_OcclusionList.begin(); i != m_OcclusionList.end(); i++)
	{
		ROcclusion* poc = *i;
		poc->CalcPlane();
		poc->pPlanes = new rplane[poc->nCount + 1];
	}

	return true;
}

bool RBspObject::Open_ObjectList(MXmlElement* pElement)
{
	int i;

	mlog("BspObject open object list : begin\n");

	MXmlElement	aObjectNode, aChild;
	int nCount = pElement->GetChildNodeCount();


	char szTagName[256], szContents[256];
	for (i = 0; i < nCount; i++)
	{
		aObjectNode = pElement->GetChildNode(i);
		aObjectNode.GetTagName(szTagName);

		if (_stricmp(szTagName, RTOK_OBJECT) == 0)
		{
			ROBJECTINFO* pInfo = new ROBJECTINFO;
			aObjectNode.GetAttribute(szContents, RTOK_NAME);
			pInfo->name = szContents;

			char fname[_MAX_PATH];
			GetPurePath(fname, m_descfilename.c_str());
			strcat(fname, szContents);

			m_MeshList.SetMtrlAutoLoad(true);
			m_MeshList.SetMapObject(true);

			pInfo->nMeshID = m_MeshList.Add(fname);
			RMesh* pmesh = m_MeshList.GetFast(pInfo->nMeshID);

			if (pmesh)
			{
				char* pName = pmesh->GetFileName();
				while (pName[0] != '\0')
				{
					if (pName[0] == 'o' && pName[1] == 'b') break;
					++pName;
				}

				if (strncmp(pName, "obj_water", 9) != 0 && strncmp(pName, "obj_flag", 8) != 0)
				{
					strcat(fname, ".ani");

					m_AniList.Add(fname, fname, i);
					RAnimation* pAni = m_AniList.GetAnimation(fname);

					pmesh->SetAnimation(pAni);

					pInfo->pVisualMesh = new RVisualMesh;
					pInfo->pVisualMesh->Create(pmesh);
					pInfo->pVisualMesh->SetAnimation(pAni);
				}
				else
				{
					//if (strncmp(pName, "obj_moving", 10) == 0)
					//{
					//	if (aObjectNode.GetAttribute(szContents, "collideable"))
					//	{
					//		if (stricmp(szContents, "true"))
					//		{
					//		}
					//	}
					//	if (aObjectNode.GetAttribute(szContents, "collradius"))
					//	{
					//		pInfo->fRadius = atof(szContents);
					//	}
					//	if (aObjectNode.GetAttribute(szContents, "collheight"))
					//	{
					//		pInfo->fHeight = atof(szContents);
					//	}


					//}
					//else
					pInfo->pVisualMesh = 0;
				}

				//custom: if object is sky related, store in the skylist to draw later
				std::string objName = pName;
				if (objName.find("sky") != std::string::npos)
				{
					pInfo->pVisualMesh->GetMesh()->mbSkyBox = true;
					m_SkyList.m_MapObjectList.push_back(pInfo);
					continue;
				}

				////custom: if object is a flag type, store here and draw later
				//if (pmesh->m_data[0]->m_point_color_num > 0)
				//{
				//	m_AniList.Add(fname, fname, i);
				//	RAnimation* pAni = m_AniList.GetAnimation(fname);


				//	pInfo->pVisualMesh = new RVisualMesh;
				//	pInfo->pVisualMesh->Create(pmesh);
				//	pInfo->pVisualMesh->SetAnimation(pAni);

				//	m_FlagList.push_back(pInfo);
				//	continue;
				//}

				m_ObjectList.m_MapObjectList.push_back(pInfo);
				pInfo->pLight = NULL;
			}
			else
				delete pInfo;
		}
	}

	///// objectlist ¸¦ Ã³¸®ÇÑ´Ù.

	mlog("RBspObject::Open_ObjectList : size %d \n", m_ObjectList.m_MapObjectList.size());


	for (auto const& it : m_ObjectList.m_MapObjectList)
	{

		ROBJECTINFO* pInfo = it;

		//		mlog("RBspObject::Open_ObjectList %d : %s \n",__cnt,pInfo->name.c_str());

				// ¹ÌÄ¡´Â ¿µÇâÀÌ Å« ¼ø¼­·Î ÇÏ³ªÀÇ ±¤¿øÀ» °í¸¥´Ù.
		float fIntensityFirst = FLT_MIN;

		if (pInfo == NULL) {

			mlog("RBspObject::Open_ObjectList : pInfo == NULL pVisualMesh->CalcBox ¿øÀÎ\n");
			continue;
		}
		else {
			if (pInfo->pVisualMesh == NULL) {
				mlog("RBspObject::Open_ObjectList : pInfo->pVisualMesh == NULL \n");
				continue;
			}
		}

		//		mlog("RBspObject::Open_ObjectList : pInfo->pVisualMesh->CalcBox... \n");
		pInfo->pVisualMesh->CalcBox();

		rvector center = (pInfo->pVisualMesh->GetBoundMax() + pInfo->pVisualMesh->GetBoundMin()) * .5f;

		//		RLightList *pllist=GetMapLightList();
		RLightList* pllist = GetObjectLightList();

		for (RLightList::iterator rlit = pllist->begin(); rlit != pllist->end(); rlit++) {

			RLIGHT* plight = *rlit;

			if (plight) {

				float fdistance = Magnitude(plight->Position - center);

				float fDist = plight->fAttnEnd - plight->fAttnStart;
				float fIntensity;
				if (fDist == 0) fIntensity = 0;
				else fIntensity = (fdistance - plight->fAttnStart) / (plight->fAttnEnd - plight->fAttnStart);

				fIntensity = min(max(1.0f - fIntensity, 0), 1);
				fIntensity *= plight->fIntensity;
				fIntensity = min(max(fIntensity, 0), 1);

				if (fIntensityFirst < fIntensity) {
					fIntensityFirst = fIntensity;
					pInfo->pLight = plight;
				}
			}
		}
	}

	mlog("RBspObject::Open_ObjectList : end\n");

	return true;
}

bool RBspObject::Make_LenzFalreList()
{
	for (RDummyList::iterator itor = m_DummyList.begin(); itor != m_DummyList.end(); ++itor)
	{
		RDummy* pDummy = *itor;

		if (stricmp(pDummy->szName.c_str(), "sun_dummy") == 0)
		{
			if (!RGetLenzFlare()->SetLight(pDummy->Position))
			{
				mlog("Fail to Set LenzFlare Position...\n");
			}

			return true;
		}
	}

	return true;
}

bool RBspObject::Open_DummyList(MXmlElement* pElement)
{
	m_DummyList.Open(pElement);
#ifndef _MAP_CACHING
	Make_LenzFalreList();
#endif
	return true;
}

bool RBspObject::Open_LightmapList(MXmlElement* pElement)
{
	MXmlElement	aMaterialNode, aChild;
	int nCount = pElement->GetChildNodeCount();
	bool result = true;

	char szTagName[256], szContents[256];
	for (int i = 0; i < nCount; i++)
	{
		aMaterialNode = pElement->GetChildNode(i);
		aMaterialNode.GetTagName(szTagName);

		if (_stricmp(szTagName, "LIGHTMAP") == 0)
		{
			aMaterialNode.GetAttribute(szContents, "name");
			m_lightMapNames.push_back(szContents);
		}
	}
	return result;
}

bool RBspObject::Set_Fog(MXmlElement* pElement)
{
	MXmlElement childElem;
	DWORD dwColor = 0;
	int color;
	char name[8];

	m_FogInfo.bFogEnable = true;
	pElement->GetAttribute(&m_FogInfo.fNear, "min");
	pElement->GetAttribute(&m_FogInfo.fFar, "max");
	int nChild = pElement->GetChildNodeCount();
	for (int i = 0; i < nChild; ++i)
	{
		childElem = pElement->GetChildNode(i);
		childElem.GetContents(&color);
		childElem.GetNodeName(name);
		if (name[0] == 'R')	dwColor |= (color << 16);
		else if (name[0] == 'G') dwColor |= (color << 8);
		else if (name[0] == 'B') dwColor |= (color);
	}
	m_FogInfo.dwFogColor = dwColor;
	return true;
}

bool RBspObject::Set_AmbSound(MXmlElement* pElement)
{
	MXmlElement childElem, contentElem;
	char szBuffer[128];
	AmbSndInfo* asinfo = NULL;

	int nChild = pElement->GetChildNodeCount();
	for (int i = 0; i < nChild; ++i)
	{
		childElem = pElement->GetChildNode(i);
		childElem.GetTagName(szBuffer);
		if (stricmp(szBuffer, "AMBIENTSOUND") == 0)
		{
			asinfo = new AmbSndInfo;
			asinfo->itype = 0;


			childElem.GetAttribute(szBuffer, "type");
			if (szBuffer[0] == 'a')
				asinfo->itype |= AS_2D;
			else if (szBuffer[0] == 'b')
				asinfo->itype |= AS_3D;

			if (szBuffer[1] == '0')
				asinfo->itype |= AS_AABB;
			else if (szBuffer[1] == '1')
				asinfo->itype |= AS_SPHERE;

			childElem.GetAttribute(asinfo->szSoundName, "filename");

			int nContents = childElem.GetChildNodeCount();
			for (int j = 0; j < nContents; ++j)
			{
				char* token = NULL;
				contentElem = childElem.GetChildNode(j);
				contentElem.GetNodeName(szBuffer);
				if (stricmp("MIN_POSITION", szBuffer) == 0)
				{
					contentElem.GetContents(szBuffer);
					token = strtok(szBuffer, " ");
					if (token != NULL) asinfo->min.x = atof(token);
					token = strtok(NULL, " ");
					if (token != NULL) asinfo->min.y = atof(token);
					token = strtok(NULL, " ");
					if (token != NULL) asinfo->min.z = atof(token);
				}
				else if (stricmp("MAX_POSITION", szBuffer) == 0)
				{
					contentElem.GetContents(szBuffer);
					token = strtok(szBuffer, " ");
					if (token != NULL) asinfo->max.x = atof(token);
					token = strtok(NULL, " ");
					if (token != NULL) asinfo->max.y = atof(token);
					token = strtok(NULL, " ");
					if (token != NULL) asinfo->max.z = atof(token);
				}
				else if (stricmp("RADIUS", szBuffer) == 0)
				{
					contentElem.GetContents(szBuffer);
					asinfo->radius = atof(szBuffer);
				}
				else if (stricmp("CENTER", szBuffer) == 0)
				{
					contentElem.GetContents(szBuffer);
					token = strtok(szBuffer, " ");
					if (token != NULL) asinfo->center.x = atof(token);
					token = strtok(NULL, " ");
					if (token != NULL) asinfo->center.y = atof(token);
					token = strtok(NULL, " ");
					if (token != NULL) asinfo->center.z = atof(token);
				}
			}
			m_AmbSndInfoList.push_back(asinfo);
		}
	}

	return true;
}

bool RBspObject::OpenDescription(const char* filename)
{
	MZFile mzf;
	if (!mzf.Open(filename, g_pFileSystem))
		return false;

	m_descfilename = filename;

	char* buffer;
	buffer = new char[mzf.GetLength() + 1];
	mzf.Read(buffer, mzf.GetLength());
	buffer[mzf.GetLength()] = 0;

	MXmlDocument aXml;
	aXml.Create();
	if (!aXml.LoadFromMemory(buffer))
	{
		delete buffer;
		return false;
	}

	int iCount, i;
	MXmlElement		aParent, aChild;
	aParent = aXml.GetDocumentElement();
	iCount = aParent.GetChildNodeCount();

	char szTagName[256];
	for (i = 0; i < iCount; i++)
	{
		aChild = aParent.GetChildNode(i);
		aChild.GetTagName(szTagName);
		if (stricmp(szTagName, RTOK_MATERIALLIST) == 0)
			Open_MaterialList(&aChild); else
			if (stricmp(szTagName, RTOK_LIGHTLIST) == 0)
				Open_LightList(&aChild); else
				if (stricmp(szTagName, RTOK_OBJECTLIST) == 0)
					Open_ObjectList(&aChild); else
					if (stricmp(szTagName, RTOK_OCCLUSIONLIST) == 0)
						Open_OcclusionList(&aChild);
		if (stricmp(szTagName, RTOK_DUMMYLIST) == 0)
		{
			Open_DummyList(&aChild);
		}
		if (stricmp(szTagName, RTOK_FOG) == 0)
			Set_Fog(&aChild);
		if (stricmp(szTagName, "AMBIENTSOUNDLIST") == 0)
			Set_AmbSound(&aChild);
		if (_stricmp(szTagName, "LIGHTMAPLIST") == 0)
		{
			Open_LightmapList(&aChild);
		}
	}

	delete buffer;
	mzf.Close();

	return true;
}

/*
bool RBspObject::OpenPathNode(const char *pathfilename)
{
	if(m_PathNodes.Open(pathfilename,m_nConvexPolygon,g_pFileSystem))
	{
		mlog("%d pathnodes ",m_PathNodes.size());
		GeneratePathNodeTable();
		return true;
	}
	return false;
}
*/

bool RBspObject::OpenRs(const char* filename)
{
	MZFile file;
	if (!file.Open(filename, g_pFileSystem))
		return false;

	mlog("RBspObject::OpenRs : file.Open \n");

	RHEADER header;
	file.Read(&header, sizeof(RHEADER));
	if (header.dwID != RS_ID || header.dwVersion != RS_VERSION)
	{
		mlog("%s : %d , %d version required.\n", filename, header.dwVersion, RS_VERSION);
		file.Close();
		return false;
	}

	mlog("RBspObject::OpenRs : file.Read(&header) \n");

	// read material indices
	int nMaterial;
	file.Read(&nMaterial, sizeof(int));

	mlog("RBspObject::OpenRs : file.Read(&nMaterial) \n");

	if (m_nMaterial - 1 != nMaterial)
		return false;

	for (int i = 1; i < m_nMaterial; i++)
	{
		char buf[256];
		if (!ReadString(&file, buf, sizeof(buf)))
			return false;
	}

	Open_ConvexPolygons(&file);

	file.Read(&m_nBspNodeCount, sizeof(int));
	file.Read(&m_nBspPolygon, sizeof(int));
	file.Read(&m_nBspVertices, sizeof(int));
	file.Read(&m_nBspIndices, sizeof(int));

	file.Read(&m_nNodeCount, sizeof(int));
	file.Read(&m_nPolygon, sizeof(int));
	file.Read(&m_nVertices, sizeof(int));
	file.Read(&m_nIndices, sizeof(int));

	m_pOcRoot = new RSBspNode[m_nNodeCount];
	m_pOcInfo = new RPOLYGONINFO[m_nPolygon];
	m_pOcVertices = new BSPVERTEX[m_nVertices];
	m_pOcIndices = new WORD[m_nIndices];

	g_pLPNode = m_pOcRoot;
	g_pLPInfo = m_pOcInfo;
	g_nCreatingPosition = 0;
	g_pLPVertices = m_pOcVertices;

	mlog("RBspObject::OpenRs : Open_Nodes begin \n");

	Open_Nodes(m_pOcRoot, &file);

	mlog("RBspObject::OpenRs : Open_Nodes end \n");
	return true;
}
#ifdef _MEMFIX
bool RBspObject::OpenBsp(const char* filename)
{
	MZFile file;
	if (!file.Open(filename, g_pFileSystem))
		return false;

	RHEADER header;
	if (!file.Read(&header, sizeof(RHEADER))) {
		file.Close();
		return false;
	}

	if (header.dwID != RBSP_ID || header.dwVersion != RBSP_VERSION)
	{
		file.Close();
		return false;
	}

	int nBspNodeCount, nBspPolygon, nBspVertices, nBspIndices;
	file.Read(&nBspNodeCount, sizeof(int));
	file.Read(&nBspPolygon, sizeof(int));
	file.Read(&nBspVertices, sizeof(int));
	file.Read(&nBspIndices, sizeof(int));

	// ✅ Kiểm tra giới hạn hợp lý
	if (nBspNodeCount <= 0 || nBspNodeCount > 500000 ||
		nBspPolygon <= 0 || nBspPolygon > 1000000 ||
		nBspVertices <= 0 || nBspVertices > 2000000 ||
		nBspIndices <= 0 || nBspIndices > 8000000)
	{
		mlog("Invalid BSP counts (nodes:%d poly:%d vert:%d idx:%d) in %s\n",
			nBspNodeCount, nBspPolygon, nBspVertices, nBspIndices, filename);
		file.Close();
		return false;
	}

	try {
		m_pBspRoot = new RSBspNode[nBspNodeCount];
		m_pBspInfo = new RPOLYGONINFO[nBspPolygon];
		m_pBspVertices = new BSPVERTEX[nBspVertices];
	}
	catch (std::bad_alloc&) {
		mlog("Memory allocation failed when loading BSP: %s\n", filename);
		file.Close();
		return false;
	}

	g_pLPNode = m_pBspRoot;
	g_pLPInfo = m_pBspInfo;
	g_nCreatingPosition = 0;
	g_pLPVertices = m_pBspVertices;

	Open_Nodes(m_pBspRoot, &file);
	_ASSERT(m_pBspRoot + nBspNodeCount > g_pLPNode);

	file.Close();
	return true;
}
#else
bool RBspObject::OpenBsp(const char* filename)
{
	MZFile file;
	if (!file.Open(filename, g_pFileSystem))
		return false;

	RHEADER header;
	file.Read(&header, sizeof(RHEADER));
	if (header.dwID != RBSP_ID || header.dwVersion != RBSP_VERSION)
	{
		file.Close();
		return false;
	}

	int nBspNodeCount, nBspPolygon, nBspVertices, nBspIndices;
	// read tree information
	file.Read(&nBspNodeCount, sizeof(int));
	file.Read(&nBspPolygon, sizeof(int));
	file.Read(&nBspVertices, sizeof(int));
	file.Read(&nBspIndices, sizeof(int));

	if (m_nBspNodeCount != nBspNodeCount || m_nBspPolygon != nBspPolygon ||
		m_nBspVertices != nBspVertices || m_nBspIndices != nBspIndices)
	{
		file.Close();
		return false;
	}

	m_pBspRoot = new RSBspNode[m_nBspNodeCount];
	m_pBspInfo = new RPOLYGONINFO[m_nBspPolygon];
	m_pBspVertices = new BSPVERTEX[m_nBspVertices];

	g_pLPNode = m_pBspRoot;
	g_pLPInfo = m_pBspInfo;
	g_nCreatingPosition = 0;
	g_pLPVertices = m_pBspVertices;

	Open_Nodes(m_pBspRoot, &file);
	_ASSERT(m_pBspRoot + m_nBspNodeCount > g_pLPNode);

	file.Close();
	return true;
}
#endif

bool RBspObject::Open_ColNodes(RSolidBspNode* pNode, MZFile* pfile)
{
	pfile->Read(&pNode->m_Plane, sizeof(rplane));

	pfile->Read(&pNode->m_bSolid, sizeof(bool));

	bool flag;
	pfile->Read(&flag, sizeof(bool));
	if (flag)
	{
		g_pLPColNode++;
		pNode->m_pPositive = g_pLPColNode;
		Open_ColNodes(pNode->m_pPositive, pfile);
	}
	pfile->Read(&flag, sizeof(bool));
	if (flag)
	{
		g_pLPColNode++;
		pNode->m_pNegative = g_pLPColNode;
		Open_ColNodes(pNode->m_pNegative, pfile);
	}

	int nPolygon;
	pfile->Read(&nPolygon, sizeof(int));

	pfile->Seek(nPolygon * 4 * sizeof(rvector), MZFile::current);

	return true;
}

bool RBspObject::OpenCol(const char* filename)
{
	MZFile file;
	if (!file.Open(filename, g_pFileSystem))
		return false;

	RHEADER header;
	file.Read(&header, sizeof(RHEADER));
	if (header.dwID != R_COL_ID || header.dwVersion != R_COL_VERSION)
	{
		file.Close();
		return false;
	}

	int nBspNodeCount, nBspPolygon;
	// read tree information
	file.Read(&nBspNodeCount, sizeof(int));
	file.Read(&nBspPolygon, sizeof(int));

	m_pColRoot = new RSolidBspNode[nBspNodeCount];
	m_pColVertices = new rvector[nBspPolygon * 3];

	g_pLPColNode = m_pColRoot;
	g_pLPColVertices = m_pColVertices;
	g_nCreatingPosition = 0;

	Open_ColNodes(m_pColRoot, &file);
	_ASSERT(m_pColRoot + nBspNodeCount > g_pLPColNode);

	file.Close();
#ifdef _DEBUGBSP
	m_pColRoot->ConstructBoundingBox();
#endif
	return true;

}


bool RBspObject::OpenNav(const char* filename)
{
	return m_NavigationMesh.Open(filename, g_pFileSystem);
}

bool SaveMemoryBmp(int x, int y, void* data, void** retmemory, int* nsize);

bool RBspObject::OpenLightmap(const char* lightmapName)
{
	//	if(m_ppLightmapTextures) return true;	// ÀÌ¹Ì ·ÎµåµÇ¾îÀÖ´Ù
	MZFile file;

	if (lightmapName == nullptr)
	{
		char lightmapinfofilename[_MAX_PATH];
		sprintf(lightmapinfofilename, "%s.lm", m_filename.c_str());

		if (!file.Open(lightmapinfofilename, g_pFileSystem)) return false;
	}
	else
	{
		string targetFile = m_filename.substr(0, m_filename.find_last_of("/") + 1);
		targetFile.append(lightmapName);
		if (!file.Open(targetFile.c_str(), g_pFileSystem)) return false;
	}

	RHEADER header;
	file.Read(&header, sizeof(RHEADER));
	if (header.dwID != R_LM_ID || header.dwVersion != R_LM_VERSION)
	{
		file.Close();
		return false;
	}

	mlog("BspObject load lightmap : file.Read(&header)\n");

	int nSourcePolygon, nNodeCount;

	if (!file.Read(&nSourcePolygon, sizeof(int)))
	{
		mlog("Error reading lightmap\n");
		file.Close();
		return false;
	}
	if (!file.Read(&nNodeCount, sizeof(int)))
	{
		mlog("Error reading lightmap\n");
		file.Close();
		return false;
	}

	//	mlog("RBspObject::OpenLightmap : nSourcePolygon = %d ,nNodeCount = %d\n",nSourcePolygon,nNodeCount);

		// °°Àº ¸Ê¿¡¼­ »ý¼ºÇÑ ¶óÀÌÆ®¸ÊÀÎÁö È®ÀÎÂ÷ ÀúÀåÇØµÐ°ÍÀ» È®ÀÎÇÑ´Ù.
	if (nSourcePolygon != m_nConvexPolygon || m_nNodeCount != nNodeCount)
	{
		mlog("Error reading lightmap\n");
		file.Close();
		return false;
	}

	if (!file.Read(&m_nLightmap, sizeof(int)))
	{
		mlog("Error reading lightmap\n");
		file.Close();
		return false;
	}
	if (m_nLightmap == 0)
	{
		file.Close();
		return false;
	}

	std::vector<LPDIRECT3DTEXTURE9> lightMapTextures;

	mlog("BspObject load lightmap nCount = %d\n", m_nLightmap);
	for (int i = 0; i < m_nLightmap; i++)
	{
		mlog("BspObject load lightmap %d\n", i);

		int nBmpSize;
		if (!file.Read(&nBmpSize, sizeof(int)))
		{
			mlog("Error reading lightmap\n");
			file.Close();
			return false;
		}

		void* bmpmemory = new BYTE[nBmpSize];
		if (!file.Read(bmpmemory, nBmpSize))
		{
			mlog("Error reading lightmap\n");
			delete[] bmpmemory;
			file.Close();
			return false;
		}

		LPDIRECT3DTEXTURE9 tex;

		HRESULT hr;


		hr = D3DXCreateTextureFromFileInMemoryEx(
			RGetDevice(), bmpmemory, nBmpSize,
			D3DX_DEFAULT, D3DX_DEFAULT,
			D3DX_DEFAULT,
			0, D3DFMT_UNKNOWN, g_isDirect3D9ExEnabled ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
			D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR,
			D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR,
			0, NULL, NULL, &tex);

		if (hr != D3D_OK)
		{
			return false;
		}
		lightMapTextures.push_back(tex);// emplace((int)m_ppLightmapTextures.size(), tex);

		//if (hr != D3D_OK) //mlog("lightmap texture »ý¼º ½ÇÆÐ %s \n", DXGetErrorString(hr));
		delete[] bmpmemory;
		//		delete memory;
	}

	//Custom: Safety check to prevent lightmaptextures from being assigned to if it's nullptr
	if (lightMapTextures.size() > 0)
	{
		m_ppLightmapTextures.push_back(lightMapTextures);
	}
	mlog("BspObject load lightmap : file.Read(&m_nLightmap) done\n");

	// ÀúÀåµÉ¶§ÀÇ Æú¸®°ï ¼ø¼­¸¦ ÀÐ´Â´Ù
	int* pOrder = new int[m_nPolygon];
	if (!file.Read(pOrder, sizeof(int) * m_nPolygon))
	{
		mlog("Error reading lightmap\n");
		delete[] pOrder;
		file.Close();
		m_nLightmap = 0;
		return false;
	}

	// ÀúÀåµÉ¶§ÀÇ Æú¸®°ï ¼ø¼­¸¦ Âü°íÇÏ¿© Æú¸®°ï´ç ¶óÀÌÆ®¸Ê ¹øÈ£¸¦ °»½ÅÇÑ´Ù
	for (int i = 0; i < m_nPolygon; i++)
	{
		if (!file.Read(&(m_pOcInfo + pOrder[i])->nLightmapTexture, sizeof(int)))
		{
			mlog("Error reading lightmap\n");
			delete[] pOrder;
			file.Close();
			m_nLightmap = 0;
			return false;
		}
	}
	delete[] pOrder;

	//	mlog("RBspObject::OpenLightmap : file.Read(&(m_pOcInfo) \n");

		// ¶óÀÌÆ®¸Ê uv ÁÂÇ¥µµ ÀÐ´Â´Ù.
	for (int i = 0; i < m_nVertices; i++)
	{
		if (!file.Read(&(m_pOcVertices + i)->tu2, sizeof(float) * 2))
		{
			mlog("Error reading lightmap\n");
			file.Close();
			m_nLightmap = 0;
			return false;
		}
	}

	//	mlog("RBspObject::OpenLightmap : file.Read(&(m_pOcVertices) \n");

	file.Close();

	mlog("BspObject load lightmap : end \n");

	return true;
}

bool RBspObject::Open_ConvexPolygons(MZFile* pfile)
{
	int nConvexVertices;//,nSourcePolygon;

	pfile->Read(&m_nConvexPolygon, sizeof(int));
	pfile->Read(&nConvexVertices, sizeof(int));

	// ÀÌ Á¤º¸µéÀº ¶óÀÌÆ®¸ÊÀ» »ý¼ºÇÒ¶§ ÇÊ¿äÇÏ¹Ç·Î ·±Å¸ÀÓ¿¡¼­´Â ÀÐÀ»ÇÊ¿ä°¡ ¾ø´Ù

	if (m_OpenMode == ROF_RUNTIME) {
		for (int i = 0; i < m_nConvexPolygon; i++)
		{
			pfile->Seek(sizeof(int) + sizeof(DWORD) + sizeof(rplane) + sizeof(float), MZFile::current);

			int nVertices;
			pfile->Read(&nVertices, sizeof(int));

			pfile->Seek(nVertices * 2 * sizeof(rvector), MZFile::current);
		}
		return true;
	}

	m_pConvexPolygons = new RCONVEXPOLYGONINFO[m_nConvexPolygon];
	m_pConvexVertices = new rvector[nConvexVertices];
	m_pConvexNormals = new  rvector[nConvexVertices];

	rvector* pLoadingVertex = m_pConvexVertices;
	rvector* pLoadingNormal = m_pConvexNormals;

	for (int i = 0; i < m_nConvexPolygon; i++)
	{
		pfile->Read(&m_pConvexPolygons[i].nMaterial, sizeof(int));
		// 0¹øÀº ·»´õ¸µ ¾ÈÇÔ.. 1Àº default material·Î.. (ÆÄÀÏ¿¡´Â -1·Î ÀúÀåµÇ¾îÀÖ´Ù)
		m_pConvexPolygons[i].nMaterial += 2;
		pfile->Read(&m_pConvexPolygons[i].dwFlags, sizeof(DWORD));
		pfile->Read(&m_pConvexPolygons[i].plane, sizeof(rplane));
		pfile->Read(&m_pConvexPolygons[i].fArea, sizeof(float));
		pfile->Read(&m_pConvexPolygons[i].nVertices, sizeof(int));

		m_pConvexPolygons[i].pVertices = pLoadingVertex;
		for (int j = 0; j < m_pConvexPolygons[i].nVertices; j++)
		{
			pfile->Read(pLoadingVertex, sizeof(rvector));
			pLoadingVertex++;
		}
		m_pConvexPolygons[i].pNormals = pLoadingNormal;
		for (int j = 0; j < m_pConvexPolygons[i].nVertices; j++)
		{
			pfile->Read(pLoadingNormal, sizeof(rvector));
			pLoadingNormal++;
		}
	}
	return true;
}

void RBspObject::CreatePolygonTableAndIndexBuffer()
{
	g_pLPIndices = m_pOcIndices;
	CreatePolygonTableAndIndexBuffer(m_pOcRoot);
}

/// °°Àº material ³¢¸® ¸ð¿©ÀÖ´Â°ÍÀ» Å×ÀÌºí·Î ¸¸µé°í ·»´õ¸µÀ» À§ÇÑ ÀÎµ¦½º ¹öÆÛ¸¦ ±¸ÃàÇÑ´Ù
void RBspObject::CreatePolygonTableAndIndexBuffer(RSBspNode* pNode)
{
	if (pNode->m_pPositive)
		CreatePolygonTableAndIndexBuffer(pNode->m_pPositive);

	if (pNode->m_pNegative)
		CreatePolygonTableAndIndexBuffer(pNode->m_pNegative);

	if (pNode->nPolygon)
	{
		for (int i = 0; i < pNode->nPolygon; i++)
		{
			WORD* pInd = g_pLPIndices;
			RPOLYGONINFO* pInfo = pNode->ppInfoSorted[i];

			pInfo->nIndicesPos = g_pLPIndices - m_pOcIndices;
			if (RIsHardwareTNL())
			{
				//				WORD base = pInfo->pVertices - m_pOcVertices;
				WORD base = pInfo->pVertices - (m_pOcVertices + pNode->m_nBaseVertexIndex);
				for (int j = 0; j < pInfo->nVertices - 2; j++)
				{
					*pInd++ = base + 0;
					*pInd++ = base + j + 1;
					*pInd++ = base + j + 2;
				}
			}
			g_pLPIndices += (pInfo->nVertices - 2) * 3;
		}

		int nCount = m_nMaterial * max(1, m_nLightmap);

		SAFE_DELETE_ARRAY(pNode->pDrawInfo);
		pNode->pDrawInfo = new RDrawInfo[nCount];

		int lastmatind = pNode->ppInfoSorted[0]->nIndicesPos;
		int lastmat = pNode->ppInfoSorted[0]->nMaterial + pNode->ppInfoSorted[0]->nLightmapTexture * m_nMaterial;
		int nTriangle = pNode->ppInfoSorted[0]->nVertices - 2;
		int lastj = 0;

		for (int j = 1; j < pNode->nPolygon + 1; j++)
		{
			int nMatIndex = (j == pNode->nPolygon) ? -999 :	// ¸¶Áö¸·ÀÌ¸é Á¤¸®ÇÏÀÚ
				pNode->ppInfoSorted[j]->nMaterial + pNode->ppInfoSorted[j]->nLightmapTexture * m_nMaterial;

			if (nMatIndex != lastmat)
			{
				RDrawInfo* pdi = &pNode->pDrawInfo[lastmat];

				_ASSERT(lastmat != -1 && lastmat >= 0 && lastmat < nCount);

				pdi->nIndicesOffset = lastmatind;
				pdi->nTriangleCount = nTriangle;
				pdi->pPlanes = new rplane[nTriangle];
				pdi->pUAxis = new rvector[nTriangle];
				pdi->pVAxis = new rvector[nTriangle];

				int nPlaneIndex = 0;
				for (int k = lastj; k < j; k++)
				{
					for (int l = 0; l < pNode->ppInfoSorted[k]->nVertices - 2; l++)
					{
						rplane* pPlane = &pNode->ppInfoSorted[k]->plane;
						pdi->pPlanes[nPlaneIndex] = *pPlane;
						rvector normal = rvector(pPlane->a, pPlane->b, pPlane->c);

						rvector up;
						if (fabs(DotProduct(normal, rvector(1, 0, 0))) < 0.01) {
							up = rvector(1, 0, 0);
						}
						else
							up = rvector(0, 1, 0);

						rvector au;
						CrossProduct(&au, up, normal);
						Normalize(au);
						rvector av;
						CrossProduct(&av, au, normal);
						Normalize(av);

						pdi->pUAxis[nPlaneIndex] = au;
						pdi->pVAxis[nPlaneIndex] = av;

						nPlaneIndex++;
					}
				}
				_ASSERT(nPlaneIndex == pdi->nTriangleCount);

				if (!RIsHardwareTNL())
				{
					//					/*
					for (int k = lastj; k < j; k++)
					{
						pdi->nVertice += pNode->ppInfoSorted[k]->nVertices;
					}

					pdi->pVertices = new BSPVERTEX[pdi->nVertice];

					WORD base = 0;
					for (int k = lastj; k < j; k++)
					{
						memcpy(pdi->pVertices + base, pNode->ppInfoSorted[k]->pVertices, sizeof(BSPVERTEX) * pNode->ppInfoSorted[k]->nVertices);

						WORD* pInd = m_pOcIndices + pNode->ppInfoSorted[k]->nIndicesPos;
						for (int l = 0; l < pNode->ppInfoSorted[k]->nVertices - 2; l++)
						{
							*pInd++ = base + 0;
							*pInd++ = base + l + 1;
							*pInd++ = base + l + 2;
						}
						base += pNode->ppInfoSorted[k]->nVertices;
					}
				}

				if (j == pNode->nPolygon) break;

				lastmat = nMatIndex;
				lastmatind = pNode->ppInfoSorted[j]->nIndicesPos;
				nTriangle = 0;
				lastj = j;
			}
			nTriangle += pNode->ppInfoSorted[j]->nVertices - 2;
		}
	}
}

void RBspObject::Sort_Nodes(RSBspNode* pNode)
{
	if (pNode->m_pPositive)
		Sort_Nodes(pNode->m_pPositive);

	if (pNode->m_pNegative)
		Sort_Nodes(pNode->m_pNegative);

	if (pNode->nPolygon)
	{
		// ¶óÀÌÆ®¸Ê&material º°·Î sort ¸¦ ÇÑ´Ù..

		for (int j = 0; j < pNode->nPolygon - 1; j++)
		{
			for (int k = j + 1; k < pNode->nPolygon; k++)
			{
				RPOLYGONINFO* pj = pNode->ppInfoSorted[j], * pk = pNode->ppInfoSorted[k];

				if (pj->nLightmapTexture * m_nMaterial + pj->nMaterial > pk->nLightmapTexture * m_nMaterial + pk->nMaterial)
				{
					pNode->ppInfoSorted[j] = pk;
					pNode->ppInfoSorted[k] = pj;
				}
			}
		}
	}
}

bool RBspObject::Open_Nodes(RSBspNode* pNode, MZFile* pfile)
{
	pfile->Read(&pNode->bbTree, sizeof(rboundingbox));
	pfile->Read(&pNode->plane, sizeof(rplane));

	bool flag;
	pfile->Read(&flag, sizeof(bool));
	if (flag)
	{
		g_pLPNode++;
		pNode->m_pPositive = g_pLPNode;
		Open_Nodes(pNode->m_pPositive, pfile);
	}
	pfile->Read(&flag, sizeof(bool));
	if (flag)
	{
		g_pLPNode++;
		pNode->m_pNegative = g_pLPNode;
		Open_Nodes(pNode->m_pNegative, pfile);
	}

	pfile->Read(&pNode->nPolygon, sizeof(int));

	// 16bit index ¸¦ ¾²±â ¶§¹®¿¡ ³ëµå´ç Æú¸®°ï ¼ö Á¦ÇÑÀ» °É¾ú½À´Ï´Ù.
	// ¸¸¾à ÀÌ°Ô ¹ß»ýÇÑ´Ù¸é ÀÍ½ºÆ÷Æ®¶§ ÀÌ Æú¸®°ï ¼ö¸¦ ³ÑÁö ¾Êµµ·Ï °­Á¦·Î ³ëµå¸¦ ÂÉ°³¾ß ÇÕ´Ï´Ù.
	if (pNode->nPolygon >= (65535 / 3)) {
		_ASSERT(FALSE);
		return false;
	}

	if (pNode->nPolygon)
	{
		pNode->ppInfoSorted = new RPOLYGONINFO * [pNode->nPolygon];
		pNode->pInfo = g_pLPInfo; g_pLPInfo += pNode->nPolygon;
		pNode->m_nBaseVertexIndex = g_pLPVertices - m_pOcVertices;

		RPOLYGONINFO* pInfo = pNode->pInfo;

		int i;
		for (i = 0; i < pNode->nPolygon; i++)
		{
			pNode->ppInfoSorted[i] = pInfo;
			int mat;

			rvector c1, c2, c3, nor;

			pfile->Read(&mat, sizeof(int));
			pfile->Read(&pInfo->nConvexPolygon, sizeof(int));
			pfile->Read(&pInfo->dwFlags, sizeof(DWORD));
			pfile->Read(&pInfo->nVertices, sizeof(int));


			BSPVERTEX* pVertex = pInfo->pVertices = g_pLPVertices;

			// normal À» ÀÐ¾î¼­ ¹ö¸°´Ù
			for (int j = 0; j < pInfo->nVertices; j++)
			{
				rvector normal;
				pfile->Read(&pVertex->x, sizeof(float) * 3);
				pfile->Read(&normal, sizeof(float) * 3);
				pfile->Read(&pVertex->tu1, sizeof(float) * 4);
				pVertex++;
			}
			// */

			// normal À» ÀÐ´Â´Ù			
			//pfile->Read(pVertex,pInfo->nVertices*sizeof(BSPVERTEX));


			g_pLPVertices += pInfo->nVertices;


			pfile->Read(&nor, sizeof(rvector));
			//			Normalize(nor);
			pInfo->plane.a = nor.x;
			pInfo->plane.b = nor.y;
			pInfo->plane.c = nor.z;
			pInfo->plane.d = -DotProduct(nor, *pInfo->pVertices[0].Coord());

			if ((pInfo->dwFlags & RM_FLAG_HIDE) != 0)
				pInfo->nMaterial = -1;
			else
			{
				int nMaterial = mat + 1;		// -1 À» 0À¸·Î ...

				// ¿ø·¡ ÀÌ·±°æ¿ì´Â ¾øÀ¸³ª ¸¹Àº ¿¡·¯¸®Æ÷Æ®·ÎÀÎÇØ Ãß°¡ -_-;
				if (nMaterial < 0 || nMaterial >= m_nMaterial) nMaterial = 0;

				pInfo->nMaterial = nMaterial;
				pInfo->dwFlags |= m_pMaterials[nMaterial].dwFlags;
			}
			_ASSERT(pInfo->nMaterial < m_nMaterial);
			pInfo->nPolygonID = g_nCreatingPosition;
			pInfo->nLightmapTexture = 0;

			pInfo++;
			g_nCreatingPosition++;
		}

		pNode->m_nVertices = (g_pLPVertices - m_pOcVertices) - pNode->m_nBaseVertexIndex;
	}

	return true;
}

bool RBspObject::CreateVertexBuffer()
{
	if (/*m_nPolygon*3 > 65530 || */m_nPolygon == 0) return false;

	g_nCreatingPosition = 0;

	SAFE_RELEASE(m_pVertexBuffer);

	HRESULT hr;

	if (g_isDirect3D9ExEnabled)
	{
		hr = RGetDevice()->CreateVertexBuffer(sizeof(BSPVERTEX) * m_nVertices, D3DUSAGE_WRITEONLY, BSP_FVF, D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL);
	}
	else
	{
		hr = RGetDevice()->CreateVertexBuffer(sizeof(BSPVERTEX) * m_nVertices, D3DUSAGE_WRITEONLY, BSP_FVF, D3DPOOL_MANAGED, &m_pVertexBuffer, NULL);
	}
	_ASSERT(hr == D3D_OK);
	if (hr != D3D_OK) return false;

	return true;
}

bool RBspObject::UpdateVertexBuffer()
{
	if (!m_pVertexBuffer) return false;

	LPBYTE pVer = NULL;
	HRESULT hr = m_pVertexBuffer->Lock(0, 0, (VOID**)&pVer, 0);
	_ASSERT(hr == D3D_OK);
	memcpy(pVer, m_pOcVertices, sizeof(BSPVERTEX) * m_nVertices);
	hr = m_pVertexBuffer->Unlock();

	return true;
}


void RBspObject::GetNormal(RCONVEXPOLYGONINFO* poly, rvector& position, rvector* normal, int au, int av)
{

	int nSelPolygon = -1, nSelEdge = -1;
	float fMinDist = FLT_MAX;

	if (poly->nVertices == 3)
		nSelPolygon = 0;
	else
	{
		// Á¡ÀÌ Æú¸®°ï¾È¿¡ µé¾î°¡ÀÖÁö ¾ÊÀ¸¸é, °¡Àå °¡±î¿î edge ¸¦ °®´Â Æú¸®°ïÀ» ±âÁØÀ¸·Î °è»êÇÑ´Ù.
		rvector pnormal(poly->plane.a, poly->plane.b, poly->plane.c);

		for (int i = 0; i < poly->nVertices - 2; i++)
		{
			float t;
			rvector* a = &poly->pVertices[0];
			rvector* b = &poly->pVertices[i + 1];
			rvector* c = &poly->pVertices[i + 2];


			if (IsIntersect(position + pnormal, -pnormal, *a, *b, *c, &t))
			{
				nSelPolygon = i;
				nSelEdge = -1;
				break;
			}
			else
			{
				float dist = GetDistance(position, *a, *b);
				if (dist < fMinDist) { fMinDist = dist; nSelPolygon = i; nSelEdge = 0; }
				dist = GetDistance(position, *b, *c);
				if (dist < fMinDist) { fMinDist = dist; nSelPolygon = i; nSelEdge = 1; }
				dist = GetDistance(position, *c, *a);
				if (dist < fMinDist) { fMinDist = dist; nSelPolygon = i; nSelEdge = 2; }
			}
		}
	}

	rvector* v0 = &poly->pVertices[0];
	rvector* v1 = &poly->pVertices[nSelPolygon + 1];
	rvector* v2 = &poly->pVertices[nSelPolygon + 2];

	rvector* n0 = &poly->pNormals[0];
	rvector* n1 = &poly->pNormals[nSelPolygon + 1];
	rvector* n2 = &poly->pNormals[nSelPolygon + 2];

	rvector pos;
	if (nSelEdge != -1)
	{
		// Æú¸®°ïÀ» ¹þ¾î³­ Á¡¿¡ ´ëÇØ¼­´Â °¡Àå °¡±î¿î edgeÀ§ÀÇ Á¡À» ¼±ÅÃÇÑ´Ù

		rvector* e0 = nSelEdge == 0 ? v0 : nSelEdge == 1 ? v1 : v2;
		rvector* e1 = nSelEdge == 0 ? v1 : nSelEdge == 1 ? v2 : v0;

		rvector dir = *e1 - *e0;
		Normalize(dir);

		pos = *e0 + DotProduct(dir, position - *e0) * dir;
	}
	else
		pos = position;

	rvector a, b, x, tem;

	a = *v1 - *v0;
	b = *v2 - *v1;
	x = pos - *v0;

	float f = b[au] * x[av] - b[av] * x[au];
	//	_ASSERT(!IS_ZERO(f));
	if (IS_ZERO(f))
	{
		*normal = *n0;
		return;
	}
	float t = (a[av] * x[au] - a[au] * x[av]) / f;

	tem = InterpolatedVector(*n1, *n2, t);


	rvector inter = a + t * b;

	int axisfors;
	if (fabs(inter.x) > fabs(inter.y) && fabs(inter.x) > fabs(inter.z)) axisfors = 0;
	else
		if (fabs(inter.y) > fabs(inter.z)) axisfors = 1;
		else axisfors = 2;

	float s = x[axisfors] / inter[axisfors];
	//*/
	*normal = InterpolatedVector(*n0, tem, s);
}


bool RBspObject::GenerateLightmap(const char* filename, int nMaxlightmapsize, int nMinLightmapSize, int nSuperSample, float fToler, RGENERATELIGHTMAPCALLBACK pProgressFn)
{
	bool bReturnValue = true;

	ClearLightmaps();

	int i, j, k, l;

	float fMaximumArea = 0;

	// ÃÖ´ë ¸éÀûÀ» °¡Áø Æú¸®°ïÀ» Ã£´Â´Ù.
	for (i = 0; i < m_nConvexPolygon; i++)
	{
		fMaximumArea = max(fMaximumArea, m_pConvexPolygons[i].fArea);
	}

	int nConstCount = 0;
	int nLight;
	RLIGHT** pplight;
	pplight = new RLIGHT * [m_StaticMapLightList.size()];
	rvector* lightmap = new rvector[nMaxlightmapsize * nMaxlightmapsize];
	DWORD* lightmapdata = new DWORD[nMaxlightmapsize * nMaxlightmapsize];
	bool* isshadow = new bool[(nMaxlightmapsize + 1) * (nMaxlightmapsize + 1)];
	int* pSourceLightmap = new int[m_nConvexPolygon];
	map<DWORD, int> ConstmapTable;

	vector<RLIGHTMAPTEXTURE*> sourcelightmaplist;

	RHEADER header(R_LM_ID, R_LM_VERSION);

	for (i = 0; i < m_nConvexPolygon; i++)
	{
		RCONVEXPOLYGONINFO* poly = m_pConvexPolygons + i;

		// progress bar °»½ÅÇÑ´Ù. cancel µÇ¾úÀ¸¸é ³ª°£´Ù.
		if (pProgressFn)
		{
			bool bContinue = pProgressFn((float)i / (float)m_nConvexPolygon);
			if (!bContinue)
			{
				bReturnValue = false;
				goto clearandexit;
			}
		}

		rboundingbox bbox;	// ¹Ù¿îµù¹Ú½º¸¦ Ã£´Â´Ù..

		bbox.vmin = bbox.vmax = poly->pVertices[0];
		for (j = 1; j < poly->nVertices; j++)
		{
			for (k = 0; k < 3; k++)
			{
				bbox.vmin[k] = min(bbox.vmin[k], poly->pVertices[j][k]);
				bbox.vmax[k] = max(bbox.vmax[k], poly->pVertices[j][k]);
			}
		}

		int lightmapsize; //¶óÀÌÆ®¸ÊÀÇ size

		/*
		if(m_pMaterials[m_pSourceMaterial[i]]->dwFlags!=0)		// add ¸ðµåÀÌ¸é ¹«Á¶°Ç white
		{
			lightmapsize=2;
			lightmapdata[0]=lightmapdata[1]=lightmapdata[2]=lightmapdata[3]=0xffffffff;
		}
		else

		*/
		{
			// »ý¼ºµÉ ¶óÀÌÆ®¸ÊÀÇ size ¸¦ °áÁ¤ÇÑ´Ù.
			lightmapsize = nMaxlightmapsize;

			float targetarea = fMaximumArea / 4.f;
			while (poly->fArea<targetarea && lightmapsize>nMinLightmapSize)
			{
				targetarea /= 4.f;
				lightmapsize /= 2;
			}

			rvector diff = float(lightmapsize) / float(lightmapsize - 1) * (bbox.vmax - bbox.vmin);

			// 1 texel ÀÇ ¿©À¯¸¦ ¸¸µç´Ù.
			for (k = 0; k < 3; k++)
			{
				bbox.vmin[k] -= .5f / float(lightmapsize) * diff[k];
				bbox.vmax[k] += .5f / float(lightmapsize) * diff[k];
			}

			rvector pnormal = rvector(poly->plane.a, poly->plane.b, poly->plane.c);

			RBSPMATERIAL* pMaterial = &m_pMaterials[m_pConvexPolygons[i].nMaterial];

			rvector ambient = pMaterial->Ambient;

			// ÀÌ Æú¸®°ï¿¡ ¿µÇâÀ» ÁÙ¸¸ÇÑ ±¤¿øµéÀ» °¡·Á³½´Ù..
			nLight = 0;


			for (RLightList::iterator light = m_StaticMapLightList.begin(); light != m_StaticMapLightList.end(); light++) {

				// À¯È¿°Å¸®°¡ ¹þ¾î³ª¸é ¿µÇâÀÌ ¾ø´Ù
				if (GetDistance((*light)->Position, poly->plane) > (*light)->fAttnEnd) continue;

				for (int iv = 0; iv < poly->nVertices; iv++)
				{
					// ÇÏ³ª¶óµµ ºûÀ» ¹Þ´Â normal ÀÌ ÀÖ´Ù¸é
					if (DotProduct((*light)->Position - poly->pVertices[iv], poly->pNormals[iv]) > 0) {
						pplight[nLight++] = *light;
						break;
					}

				}
			}


			int au, av, ax; // ÃàÀÇ index    °¢°¢ ÅØ½ºÃÄ¿¡¼­ÀÇ uÃà, vÃà, ±×¸®°í ³ª¸ÓÁö ÇÑÃà..

			if (fabs(poly->plane.a) > fabs(poly->plane.b) && fabs(poly->plane.a) > fabs(poly->plane.c))
				ax = 0;   // yz Æò¸éÀ¸·Î projection	u ´Â y Ãà¿¡ v ´Â z Ãà¿¡ ´ëÀÀ, ³ª¸ÓÁöÇÑÃà ax´Â x Ãà
			else if (fabs(poly->plane.b) > fabs(poly->plane.c))
				ax = 1;	// xz Æò¸éÀ¸·Î ...
			else
				ax = 2;	// xy Æò¸éÀ¸·Î ...

			au = (ax + 1) % 3;
			av = (ax + 2) % 3;

			for (j = 0; j < lightmapsize; j++)			// v 
			{
				for (k = 0; k < lightmapsize; k++)		// u
				{
					//					lightmap[j*lightmapsize+k]=rvector(0,0,0);
					lightmap[j * lightmapsize + k] = m_AmbientLight;
					//ambient;
				}
			}

			for (l = 0; l < nLight; l++)
			{
				RLIGHT* plight = pplight[l];

				// ±×¸²ÀÚ°¡ µÇ´Â °æ¿ìÀÎÁö ¹Ì¸® Å×ÀÌºí(isshadow)·Î °è»êÇØ³õ´Â´Ù..
				for (j = 0; j < lightmapsize + 1; j++)			// v 
				{
					for (k = 0; k < lightmapsize + 1; k++)		// u
					{
						isshadow[k * (lightmapsize + 1) + j] = false;
						if ((plight->dwFlags & RM_FLAG_CASTSHADOW) == 0 ||
							(poly->dwFlags & RM_FLAG_RECEIVESHADOW) == 0) continue;
						_ASSERT(plight->dwFlags == 16);

						rvector position;
						position[au] = bbox.vmin[au] + ((float)k / (float)lightmapsize) * diff[au];
						position[av] = bbox.vmin[av] + ((float)j / (float)lightmapsize) * diff[av];
						// Æò¸éÀÇ ¹æÁ¤½Ä¿¡ ÀÇÇØ ³ª¸ÓÁö ÇÑ ÁÂÇ¥°¡ °áÁ¤µÈ´Ù.
						position[ax] = (-poly->plane.d - pnormal[au] * position[au] - pnormal[av] * position[av]) / pnormal[ax];

						float fDistanceToPolygon = Magnitude(position - plight->Position);

						RBSPPICKINFO bpi;
						if (PickShadow(plight->Position, position, &bpi))
						{
							float fDistanceToPickPos = Magnitude(bpi.PickPos - plight->Position);

							if (fDistanceToPolygon > fDistanceToPickPos + fToler)
								isshadow[k * (lightmapsize + 1) + j] = true;
						}
					}
				}


				for (j = 0; j < lightmapsize; j++)			// v 
				{
					for (k = 0; k < lightmapsize; k++)		// u
					{
						rvector color = rvector(0, 0, 0);

						// ºû°ú ÇöÀç Æú¸®°ï »çÀÌ¿¡ ¹º°¡°¡ ÀÖ´Â°æ¿ì´Â ±×¸²ÀÚÁö´Â°æ¿ì.. 
						// ±×³É Ã³¸®ÇÏ¸é, ¶óÀÌÆ®¸Ê¿¡ °è´ÜÇö»óÀÌ »ý±â¹Ç·Î ÀÌ°æ¿ì¿£ supersample ÀÌ ÇÊ¿äÇÏ´Ù..
						// ³× ±ÍÅüÀÌ°¡ ±×¸²ÀÚ°¡ ¾Æ´Ñ°æ¿ì¿Í ±×¸²ÀÚÀÎ °æ¿ì°¡ ¼¯¿© ÀÖÀ¸¸é
						// ¹Ù·Î ÀÌ°÷ÀÌ supersample ÀÌ ÇÊ¿äÇÏ´Ù..

						int nShadowCount = 0;

						for (int m = 0; m < 4; m++)
						{
							if (isshadow[(k + m % 2) * (lightmapsize + 1) + j + m / 2])
								nShadowCount++;
						}


						if (nShadowCount < 4)
						{
							if (nShadowCount > 0)		// super sample
//							if(1)
							{
								int m, n;
								rvector tempcolor = rvector(0, 0, 0);

								//*

								for (m = 0; m < nSuperSample; m++)
								{
									for (n = 0; n < nSuperSample; n++)
									{
										rvector position;
										position[au] = bbox.vmin[au] + (((float)k + ((float)n + .5f) / (float)nSuperSample) / (float)lightmapsize) * diff[au];
										position[av] = bbox.vmin[av] + (((float)j + ((float)m + .5f) / (float)nSuperSample) / (float)lightmapsize) * diff[av];
										// Æò¸éÀÇ ¹æÁ¤½Ä¿¡ ÀÇÇØ ³ª¸ÓÁö ÇÑ ÁÂÇ¥°¡ °áÁ¤µÈ´Ù.
										position[ax] = (-poly->plane.d - pnormal[au] * position[au] - pnormal[av] * position[av]) / pnormal[ax];

										bool bShadow = false;

										float fDistanceToPolygon = Magnitude(position - plight->Position);

										RBSPPICKINFO bpi;
										if (PickShadow(plight->Position, position, &bpi))
										{
											float fDistanceToPickPos = Magnitude(bpi.PickPos - plight->Position);
											if (fDistanceToPolygon > fDistanceToPickPos + fToler)
												bShadow = true;
										}

										if (!bShadow)
										{
											rvector dpos = plight->Position - position;
											float fdistance = Magnitude(dpos);
											float fIntensity = (fdistance - plight->fAttnStart) / (plight->fAttnEnd - plight->fAttnStart);
											fIntensity = min(max(1.0f - fIntensity, 0), 1);
											Normalize(dpos);

											rvector normal;
											GetNormal(poly, position, &normal, au, av);

											float fDot;
											fDot = DotProduct(dpos, normal);
											fDot = max(0, fDot);

											tempcolor += fIntensity * plight->fIntensity * fDot * plight->Color;
										}
									}
								}
								tempcolor *= 1.f / (nSuperSample * nSuperSample);
								//*/
								color += tempcolor;
							}
							else					// ±×¸²ÀÚ°¡ ¾ø´Â°æ¿ì´Â ÅØ¼¿ÀÇ Áß°£ ÁÂÇ¥·Î ÇÑ¹ø¸¸ °è»êÇÑ´Ù..
							{
								rvector position;
								position[au] = bbox.vmin[au] + (((float)k + .5f) / (float)lightmapsize) * diff[au];
								position[av] = bbox.vmin[av] + (((float)j + .5f) / (float)lightmapsize) * diff[av];
								// Æò¸éÀÇ ¹æÁ¤½Ä¿¡ ÀÇÇØ ³ª¸ÓÁö ÇÑ ÁÂÇ¥°¡ °áÁ¤µÈ´Ù.
								position[ax] = (-poly->plane.d - pnormal[au] * position[au] - pnormal[av] * position[av]) / pnormal[ax];

								rvector dpos = plight->Position - position;
								float fdistance = Magnitude(dpos);
								float fIntensity = (fdistance - plight->fAttnStart) / (plight->fAttnEnd - plight->fAttnStart);
								fIntensity = min(max(1.0f - fIntensity, 0), 1);
								Normalize(dpos);

								rvector normal;
								GetNormal(poly, position, &normal, au, av);

								float fDot;
								fDot = DotProduct(dpos, normal);
								fDot = max(0, fDot);

								color += fIntensity * plight->fIntensity * fDot * plight->Color;
							}
						}

						lightmap[j * lightmapsize + k] += color;
					}
				}
			}

			// 1,1,1 À» ³Ñ¾î°¡¸é Ä¿ÆÃÇØÁØ´Ù..

			for (j = 0; j < lightmapsize * lightmapsize; j++)
			{
				rvector color = lightmap[j];

				color *= 0.25f;
				color.x = min(color.x, 1);
				color.y = min(color.y, 1);
				color.z = min(color.z, 1);
				lightmap[j] = color;
				lightmapdata[j] = ((DWORD)(color.x * 255)) << 16 | ((DWORD)(color.y * 255)) << 8 | ((DWORD)(color.z * 255));
			}

			/*
			// supersample ·Îµµ ÇØ°áµÇÁö ¾Ê´Â °÷ÀÌ ÀÖ´Ù.. ±×·¡¼­
			// ºí·¯¸¦ ½ê¿öÁØ´Ù.. 1ÇÈ¼¿ÀÇ ¿©À¯°¡ ÀÖÀ¸¹Ç·Î.. ±â³É

			//      1 2 1
			//      2 4 2
			//      1 2 1

			for(j=1;j<lightmapsize-1;j++)
			{
				for(k=1;k<lightmapsize-1;k++)
				{
					rvector color=lightmap[j*lightmapsize+k]*4;
					color+=(lightmap[(j-1)*lightmapsize+k]+lightmap[j*lightmapsize+k-1]+
							lightmap[(j+1)*lightmapsize+k]+lightmap[j*lightmapsize+k+1])*2;
					color+=	lightmap[(j-1)*lightmapsize+(k-1)]+lightmap[(j-1)*lightmapsize+k+1]+
							lightmap[(j+1)*lightmapsize+(k-1)]+lightmap[(j+1)*lightmapsize+k+1];
					color/=16.f;

					lightmapdata[j*lightmapsize+k]=((DWORD)(color.x*255))<<16 | ((DWORD)(color.y*255))<<8 | ((DWORD)(color.z*255));
				}
			}
			*/
		}

		bool bConstmap = true;
		for (j = 0; j < lightmapsize * lightmapsize; j++)
		{
			if (lightmapdata[j] != lightmapdata[0])
			{
				bConstmap = false;
				nConstCount++;
				break;
			}
		}

		bool bNeedInsert = true;
		if (bConstmap)
		{
			// »ó¼öÀÌ¸é Å©±â¸¦ 2x2·Î ¹Ù²Û´Ù.. ¿Ö? ´Þ¾Æ¼­ -_-;; ±×³É ¹Ù²ãµµ »ó°ü¾ø´Ù.
			lightmapsize = 2;

			map<DWORD, int>::iterator it = ConstmapTable.find(lightmapdata[0]);
			if (it != ConstmapTable.end())
			{
				pSourceLightmap[i] = (*it).second;
				bNeedInsert = false;
			}
		}

		if (bNeedInsert)
		{
			size_t nLightmap = sourcelightmaplist.size();

			pSourceLightmap[i] = (int)nLightmap;
			if (bConstmap)
				ConstmapTable.insert(map<DWORD, int>::value_type(lightmapdata[0], (int)nLightmap));

#ifdef GENERATE_TEMP_FILES   // ÆÄÀÏ·Î ¶óÀÌÆ®¸ÊÀ» ÀúÀåÇÑ´Ù.

			if (i < 100)	// 100°³¸¸ÇÏÀÚ -_-;
			{
				char lightmapfilename[256];
				sprintf(lightmapfilename, "%s%d.bmp", m_filename.c_str(), nLightmap);
				RSaveAsBmp(lightmapsize, lightmapsize, lightmapdata, lightmapfilename);
			}
#endif			//*/

			RLIGHTMAPTEXTURE* pnew = new RLIGHTMAPTEXTURE;
			pnew->bLoaded = false;
			pnew->nSize = lightmapsize;
			pnew->data = new DWORD[lightmapsize * lightmapsize];
			memcpy(pnew->data, lightmapdata, lightmapsize * lightmapsize * sizeof(DWORD));
			sourcelightmaplist.push_back(pnew);
		}
	}

	// ÀÛÀº Å©±âÀÇ ¶óÀÌÆ®¸ÊÀ» Ä¿´Ù¶õ ÅØ½ºÃÄ·Î ÇÕÄ¡¸é¼­ »õ·Î¿î uvÁÂÇ¥¸¦ °è»êÇÑ´Ù.
	CalcLightmapUV(m_pBspRoot, pSourceLightmap, &sourcelightmaplist);
	CalcLightmapUV(m_pOcRoot, pSourceLightmap, &sourcelightmaplist);


	// »ý¼ºµÈ ¶óÀÌÆ®¸ÊÀ» ÀúÀåÇÑ´Ù.

	FILE* file = fopen(filename, "wb+");
	if (!file) {
		bReturnValue = false;
		goto clearandexit;
	}

	fwrite(&header, sizeof(RHEADER), 1, file);

	// °°Àº ¸Ê¿¡¼­ »ý¼ºÇÑ ¶óÀÌÆ®¸ÊÀÎÁö È®ÀÎÂ÷ ÀúÀåÇØµÐ´Ù.
	fwrite(&m_nConvexPolygon, sizeof(int), 1, file);
	fwrite(&m_nNodeCount, sizeof(int), 1, file);

	// Àû´çÇÑ Å©±â¿¡ ÇÕÃÄÁ® ³Ö¾îÁø ¶óÀÌÆ®¸ÊÀ» ÀúÀåÇÑ´Ù.
	m_nLightmap = (int)m_LightmapList.size();
	fwrite(&m_nLightmap, sizeof(int), 1, file);
	for (size_t i = 0; i < m_LightmapList.size(); i++)
	{
		/*//	¶óÀÌÆ®¸Ê ÀúÀå
		char filename[256];
		sprintf(filename,"temp%d.bmp",i);
		m_sourcelightmaplist[i]->Save(filename);
		//*/

		//		/*
				// ÇÕÃÄÁø Å« ¶óÀÌÆ®¸Ê ÀúÀå
		char lightfilename[256];
		sprintf(lightfilename, "%s.light%i.bmp", filename, (int)i);
		RSaveAsBmp(m_LightmapList[i]->GetSize(), m_LightmapList[i]->GetSize(), m_LightmapList[i]->GetData(), lightfilename);
		//*/

		void* memory;
		int nSize;
		bool bSaved = SaveMemoryBmp(m_LightmapList[i]->GetSize(), m_LightmapList[i]->GetSize(), m_LightmapList[i]->GetData(), &memory, &nSize);
		_ASSERT(bSaved);
		fwrite(&nSize, sizeof(int), 1, file);
		fwrite(memory, nSize, 1, file);
		delete memory;

		/*
		RBspLightmapManager *plm=m_LightmapList[i];

		int nSize=plm->GetSize();
		fwrite(&nSize,sizeof(int),1,file);
		float fUnused=plm->CalcUnused();
		fwrite(&fUnused,sizeof(float),1,file);
		fwrite(plm->GetData(),plm->GetSize()*plm->GetSize(),sizeof(DWORD),file);
		*/
	}


	Sort_Nodes(m_pOcRoot);

	// Æú¸®°ïÀÇ ¼ø¼­¸¦ ÀúÀåÇÑ´Ù.
	for (int i = 0; i < m_nPolygon; i++)
		fwrite(&(m_pOcInfo + i)->nPolygonID, sizeof(int), 1, file);

	// ¶óÀÌÆ®¸Ê ¹øÈ£¸¦ ÀúÀåÇÑ´Ù.
	/*
	for(int i=0;i<m_nBspPolygon;i++)
		fwrite(&(m_pBspInfo+i)->nLightmapTexture,sizeof(int),1,file);
	*/
	for (int i = 0; i < m_nPolygon; i++)
		fwrite(&(m_pOcInfo + i)->nLightmapTexture, sizeof(int), 1, file);

	// uv ÁÂÇ¥µéµµ ÀúÀåÇÑ´Ù.

	/*
	for(int i=0;i<m_nBspPolygon*3;i++)
		fwrite(&(m_pBspVertices+i)->tu2,sizeof(float),2,file);
	*/
	for (int i = 0; i < m_nVertices; i++)
		fwrite(&(m_pOcVertices + i)->tu2, sizeof(float), 2, file);

	fclose(file);

clearandexit:

	delete[]pplight;
	delete[]lightmap;
	delete[]lightmapdata;
	delete[]isshadow;

	delete[]pSourceLightmap;
	while (sourcelightmaplist.size())
	{
		delete (*sourcelightmaplist.begin())->data;
		delete* sourcelightmaplist.begin();
		sourcelightmaplist.erase(sourcelightmaplist.begin());
	}

	return bReturnValue;
}

void RBspObject::CalcLightmapUV(RSBspNode* pNode, int* pSourceLightmap, vector<RLIGHTMAPTEXTURE*>* pSourceLightmaps)
{
	if (pNode->nPolygon)
	{
		int i, j, k;
		for (i = 0; i < pNode->nPolygon; i++)
		{
			int is = pNode->pInfo[i].nConvexPolygon;
			int nSI = pSourceLightmap[is];	// nSI = Á¶°¢Á¶°¢ »ý¼ºµÈ ¶óÀÌÆ®¸ÊÀÇ ¿ø·¡ ÀÎµ¦½º

			RCONVEXPOLYGONINFO* poly = m_pConvexPolygons + is;

			rboundingbox bbox;	// ¹Ù¿îµù¹Ú½º¸¦ Ã£´Â´Ù..

			bbox.vmin = bbox.vmax = poly->pVertices[0];
			for (j = 1; j < poly->nVertices; j++)
			{
				for (k = 0; k < 3; k++)
				{
					bbox.vmin[k] = min(bbox.vmin[k], poly->pVertices[j][k]);
					bbox.vmax[k] = max(bbox.vmax[k], poly->pVertices[j][k]);
				}
			}

			RLIGHTMAPTEXTURE* pDestLightmap = pSourceLightmaps->at(nSI);

			int lightmapsize = pDestLightmap->nSize;

			rvector diff = float(lightmapsize) / float(lightmapsize - 1) * (bbox.vmax - bbox.vmin);

			// 1 texel ÀÇ ¿©À¯¸¦ ¸¸µç´Ù.
			for (k = 0; k < 3; k++)
			{
				bbox.vmin[k] -= .5f / float(lightmapsize) * diff[k];
				bbox.vmax[k] += .5f / float(lightmapsize) * diff[k];
			}

			int au, av, ax; // ÃàÀÇ index    °¢°¢ ÅØ½ºÃÄ¿¡¼­ÀÇ uÃà, vÃà, ±×¸®°í ³ª¸ÓÁö ÇÑÃà..

			if (fabs(poly->plane.a) > fabs(poly->plane.b) && fabs(poly->plane.a) > fabs(poly->plane.c))
				ax = 0;   // yz Æò¸éÀ¸·Î projection	u ´Â y Ãà¿¡ v ´Â z Ãà¿¡ ´ëÀÀ, ³ª¸ÓÁöÇÑÃà ax´Â x Ãà
			else if (fabs(poly->plane.b) > fabs(poly->plane.c))
				ax = 1;	// xz Æò¸éÀ¸·Î ...
			else
				ax = 2;	// xy Æò¸éÀ¸·Î ...

			au = (ax + 1) % 3;
			av = (ax + 2) % 3;

			RPOLYGONINFO* pInfo = &pNode->pInfo[i];
			// u2,v2 lightmap ÀÇ uvÁÂÇ¥¸¦ °áÁ¤ÇÑ´Ù.
			for (j = 0; j < pInfo->nVertices; j++)
			{
				pInfo->pVertices[j].tu2 = ((*pInfo->pVertices[j].Coord())[au] - bbox.vmin[au]) / diff[au];
				pInfo->pVertices[j].tv2 = ((*pInfo->pVertices[j].Coord())[av] - bbox.vmin[av]) / diff[av];
			}

			RBspLightmapManager* pCurrentLightmap = m_LightmapList.size() ? m_LightmapList[m_LightmapList.size() - 1] : NULL;

			if (!pDestLightmap->bLoaded)
			{
				POINT pt;

				while (!pCurrentLightmap ||
					!pCurrentLightmap->Add(pDestLightmap->data, pDestLightmap->nSize, &pt))
				{
					pCurrentLightmap = new RBspLightmapManager;
					m_LightmapList.push_back(pCurrentLightmap);
				}
				pDestLightmap->bLoaded = true;
				pDestLightmap->position = pt;
				pDestLightmap->nLightmapIndex = (int)(m_LightmapList.size()) - 1;
			}

			pNode->pInfo[i].nLightmapTexture = pDestLightmap->nLightmapIndex;

			// ½ÇÁ¦ ºñµð¿À¸Þ¸ð¸®¿¡ »ý¼ºµÈ ÅØ½ºÃ³À§ÀÇ ÁÂÇ¥·Î º¯È¯ÇÑ´Ù.
			float fScaleFactor = (float)pDestLightmap->nSize / (float)pCurrentLightmap->GetSize();
			for (j = 0; j < pInfo->nVertices; j++)
			{
				pInfo->pVertices[j].tu2 =
					pInfo->pVertices[j].tu2 * fScaleFactor + (float)pDestLightmap->position.x / (float)pCurrentLightmap->GetSize();
				pInfo->pVertices[j].tv2 =
					pInfo->pVertices[j].tv2 * fScaleFactor + (float)pDestLightmap->position.y / (float)pCurrentLightmap->GetSize();
			}
		}
	}

	if (pNode->m_pPositive) CalcLightmapUV(pNode->m_pPositive, pSourceLightmap, pSourceLightmaps);
	if (pNode->m_pNegative) CalcLightmapUV(pNode->m_pNegative, pSourceLightmap, pSourceLightmaps);
}

void RBspObject::GetUV(rvector& Pos, RSBspNode* pNode, int nIndex, float* uv)
{

}

DWORD RBspObject::GetLightmap(rvector& Pos, RSBspNode* pNode, int nIndex)
{
	return 0xffffffff;
}

rvector RBspObject::GetDimension()
{
	if (!m_pOcRoot)
		return rvector(0, 0, 0);

	return m_pOcRoot->bbTree.vmax - m_pOcRoot->bbTree.vmin;
}

void RBspObject::DrawSolid()					// ¸ðµç solid ³ëµåµéÀ» ±×¸°´Ù
{
#ifdef _DEBUGBSP

	LPDIRECT3DDEVICE9 pLPDIRECT3DDEVICE9 = RGetDevice();

	pLPDIRECT3DDEVICE9->SetFVF(D3DFVF_XYZ);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_TEXTUREFACTOR, 0x406fa867);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	m_pColRoot->DrawSolidPolygon();

	RSetWBuffer(true);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_ZWRITEENABLE, true);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ffffff);
	m_pColRoot->DrawSolidPolygonWireframe();

	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ff00ff);
	m_pColRoot->DrawSolidPolygonNormal();

	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ff0000);
#endif
}

void RBspObject::DrawSolidNode()
{
	if (!m_DebugInfo.pLastColNode) return;
#ifdef _DEBUGBSP
	LPDIRECT3DDEVICE9 pLPDIRECT3DDEVICE9 = RGetDevice();

	pLPDIRECT3DDEVICE9->SetFVF(D3DFVF_XYZ);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40808080);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	pLPDIRECT3DDEVICE9->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	pLPDIRECT3DDEVICE9->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	m_DebugInfo.pLastColNode->DrawSolidPolygon();

	RSetWBuffer(true);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_ZWRITEENABLE, true);
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ffffff);
	m_DebugInfo.pLastColNode->DrawSolidPolygonWireframe();

	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ff00ff);
	m_DebugInfo.pLastColNode->DrawSolidPolygonNormal();

	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ff0000);

	/*
	{
		// Ãß°¡µÈ ¹öÅØ½º Æò¸éÀÇ normalÀ» ±×·Áº»´Ù.

		RSolidBspNode *pNode=m_pColRoot;
		for(int i=0;i<g_nPathDepth;i++)
		{
			rplane plane=pNode->plane;
			if(g_impactpath[i]==0)
				pNode=pNode->m_pNegative;
			else
				pNode=pNode->m_pPositive;

			// ¸ðµÎ °°Àº µü ÇÏ³ªÀÇ Á¡À» Áö³­´Ù¸é.. »ÏÁ·ÇÑ ¹öÅØ½º¿¡ Ãß°¡ÇÑ Æò¸éÀÌ´Ù
			bool bExist=false;
			rvector pos;
			for(int j=0;j<m_DebugInfo.pLastColNode->nPolygon*3;j++)
			{
				if(fabs(D3DXPlaneDotCoord(&plane,&m_DebugInfo.pLastColNode->pVertices[j]))<0.01f) {
					if(!bExist) {
						pos=m_DebugInfo.pLastColNode->pVertices[j];
						bExist=true;
					}
					else
					{
						if(!IS_EQ3(pos,m_DebugInfo.pLastColNode->pVertices[j])) {
							bExist=false;
							break;
						}
					}
				}
			}

			if(bExist)
			{
				rvector v[2];
				v[0]=pos;
				v[1]=pos+rvector(plane.a,plane.b,plane.c)*50.f;
				RGetDevice()->DrawPrimitiveUP(D3DPT_LINESTRIP,1,&v,sizeof(rvector));
			}
		}
	}
	*/
	pLPDIRECT3DDEVICE9->SetRenderState(D3DRS_TEXTUREFACTOR, 0x40ffffff);

#endif
}

bool RBspObject::CheckWall(rvector& origin, rvector& targetpos, float fRadius, float fHeight, RCOLLISIONMETHOD method, int nDepth, rplane* pimpactplane)
{
	return RSolidBspNode::CheckWall(m_pColRoot, origin, targetpos, fRadius, fHeight, method, nDepth, pimpactplane);
}

bool RBspObject::CheckSolid(rvector& pos, float fRadius, float fHeight, RCOLLISIONMETHOD method)
{
	RImpactPlanes impactPlanes;
	if (method == RCW_SPHERE)
		return m_pColRoot->GetColPlanes_Sphere(&impactPlanes, pos, pos, fRadius);
	else
		return m_pColRoot->GetColPlanes_Cylinder(&impactPlanes, pos, pos, fRadius, fHeight);
}

rvector RBspObject::GetFloor(rvector& origin, float fRadius, float fHeight, rplane* pimpactplane)
{
	rvector targetpos = origin + rvector(0, 0, -10000);

	RImpactPlanes impactPlanes;
	bool bIntersect = m_pColRoot->GetColPlanes_Cylinder(&impactPlanes, origin, targetpos, fRadius, fHeight);
	//	_ASSERT(bIntersect);
	if (!bIntersect)
		return targetpos;

	rvector floor = m_pColRoot->GetLastColPos();
	floor.z -= fHeight;
	if (pimpactplane)
		*pimpactplane = m_pColRoot->GetLastColPlane();

	return floor;
}

RBSPMATERIAL* RBspObject::GetMaterial(int nIndex)
{
	_ASSERT(nIndex < m_nMaterial);
	return &m_pMaterials[nIndex];
}

// world matrix °¡ °¨¾ÈµÇ¾îÀÖÁö ¾Ê´Ù. ¹«Á¶°Ç world==identity¶ó °¡Á¤
bool RBspObject::IsVisible(rboundingbox& bb)		// occlusion ¿¡ ÀÇÇØ °¡·ÁÁ®ÀÖÀ¸¸é false ¸¦ ¸®ÅÏ.
{
	return m_OcclusionList.IsVisible(bb);
}

bool RBspObject::GetShadowPosition(rvector& pos_, rvector& dir_, rvector* outNormal_, rvector* outPos_)
{
	RBSPPICKINFO pick_info;
	if (!Pick(pos_, dir_, &pick_info))
		return false;
	*outPos_ = pick_info.PickPos;
	*outNormal_ = rvector(pick_info.pInfo[pick_info.nIndex].plane.a,
		pick_info.pInfo[pick_info.nIndex].plane.b, pick_info.pInfo[pick_info.nIndex].plane.c);
	//*outNormal_= rvector(pick_info.pNode->plane.a,pick_info.pNode->plane.b, pick_info.pNode->plane.c);
	return true;
}
//*/

/*
// center¸¦ ¹Ø¸éÀÇ Áß½É poleÀ» ²ÀÁöÁ¡, radius¸¦ ¹Ø¸éÀÇ ¹ÝÁö¸§À¸·Î ÇÏ´Â ÄÜ¿¡ µé¾î¿À´Â °¡Àå °¡±î¿î Á¡À» ¸®ÅÏ. (Ä«¸Þ¶ó¿¡ »ç¿ë)
bool RBspObject::CheckWall_Corn(rvector *pOut,rvector &center,rvector &pole,float fRadius)
{
	float fMinLength=0.f;

	rvector dir=pole-center;
	float fLength=Magnitude(dir);
	Normalize(dir);

	impactcount=0;
	impactPlanes.erase(impactPlanes.begin(),impactPlanes.end());

	bool bIntersectThis=CheckWall_Sphere(m_pColRoot,center,pole,fRadius);

	for(RImpactPlanes::iterator i=impactPlanes.begin();i!=impactPlanes.end();i++)
	{
		rplane plane=i->second;

		if(D3DXPlaneDotNormal(&plane,&dir)<0.01){
			rplane shiftplane=plane;
			shiftplane.d+=fRadius;

			// ½ÇÁ¦ Ãæµ¹ À§Ä¡
			rvector impactpos;
			D3DXPlaneIntersectLine(&impactpos,&shiftplane,&center,&pole);
			impactpos-=fRadius*rvector(plane.a,plane.b,plane.c);

			// Ãæµ¹À§Ä¡¸¦ Áö³ª°í cornÀÇ Ãà¿¡ ¼öÁ÷ÀÎ Æò¸é
			rplane ortplane;
			D3DXPlaneFromPointNormal(&ortplane,&impactpos,&dir);

			// ±× Æò¸é°ú Ãà°úÀÇ ±³Á¡
			rvector ortcenter;
			D3DXPlaneIntersectLine(&ortcenter,&ortplane,&center,&pole);

			// ±× Æò¸é¿¡ ´ëÇÑ ´Ü¸éÀÇ ¿øÀÇ ¹Ý°æ
			float fortlength=max(DotProduct(pole-ortcenter,dir),0);
			float fortradius=fRadius*fortlength/fLength;

			float fDistToCorn=fortradius-Magnitude(impactpos-ortcenter);

			// 0º¸´Ù Å©¸é Ãæµ¹ÇÏÁö ¾ÊÀº°ÍÀÓ
			if(fDistToCorn<fMinLength)
			{
				fMinLength=fDistToCorn;
			}
		}
	}

	*pOut=pole+dir*fMinLength;

	return true;
}
*/

// pick À» À§ÇÑ Àü¿ª º¯¼ö..
bool			g_bPickFound;
rvector			g_PickOrigin;
rvector			g_PickTo;
rvector			g_PickDir;
RBSPPICKINFO* g_pPickOut;
//RPathNode		**g_pPickOutPathNode;
rvector* g_pColPos;
rplane			g_PickPlane;
float			g_fPickDist;
DWORD			g_dwPassFlag;
//*/

#ifndef _PUBLISH

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

bool RBspObject::Pick(const rvector& pos, const rvector& dir, RBSPPICKINFO* pOut, DWORD dwPassFlag)
{
	if (!m_pBspRoot) return false;

	__BP(195, "RBspObject::Pick");

	g_PickOrigin = pos;
	g_PickTo = pos + 10000.f * dir;
	g_PickDir = dir;
	Normalize(g_PickDir);

	g_bPickFound = false;
	g_pPickOut = pOut;
	D3DXPlaneFromPointNormal(&g_PickPlane, &g_PickOrigin, &g_PickDir);
	g_fPickDist = FLT_MAX;

	g_dwPassFlag = dwPassFlag;

	g_bPickFound = Pick(m_pBspRoot, pos, g_PickTo);

	__EP(195);

	if (g_bPickFound)
		return true;

	return false;
}

bool RBspObject::PickTo(const rvector& pos, const rvector& to, RBSPPICKINFO* pOut, DWORD dwPassFlag)
{
	if (!m_pBspRoot) return false;

	__BP(195, "RBspObject::Pick");

	g_PickOrigin = pos;
	g_PickTo = to;
	g_PickDir = to - pos;
	Normalize(g_PickDir);

	g_bPickFound = false;
	g_pPickOut = pOut;
	D3DXPlaneFromPointNormal(&g_PickPlane, &g_PickOrigin, &g_PickDir);
	g_fPickDist = FLT_MAX;

	g_dwPassFlag = dwPassFlag;

	g_bPickFound = Pick(m_pBspRoot, pos, g_PickTo);

	__EP(195);

	if (g_bPickFound)
		return true;

	return false;
}

bool RBspObject::PickOcTree(rvector& pos, rvector& dir, RBSPPICKINFO* pOut, DWORD dwPassFlag)
{
	if (!m_pOcRoot) return false;

	g_PickOrigin = pos;
	g_PickTo = pos + 10000.f * dir;
	g_PickDir = dir;
	Normalize(g_PickDir);

	g_bPickFound = false;
	g_pPickOut = pOut;
	D3DXPlaneFromPointNormal(&g_PickPlane, &g_PickOrigin, &g_PickDir);
	g_fPickDist = FLT_MAX;

	g_dwPassFlag = dwPassFlag;

	g_bPickFound = Pick(m_pOcRoot, pos, g_PickTo);

	if (g_bPickFound)
		return true;

	return false;
}

bool RBspObject::PickShadow(rvector& pos, rvector& to, RBSPPICKINFO* pOut)
{
	if (!m_pBspRoot) return false;

	g_PickOrigin = pos;
	g_PickTo = to;
	g_PickDir = to - pos;
	Normalize(g_PickDir);

	g_bPickFound = false;
	g_pPickOut = pOut;
	D3DXPlaneFromPointNormal(&g_PickPlane, &g_PickOrigin, &g_PickDir);
	g_fPickDist = FLT_MAX;

	g_bPickFound = PickShadow(m_pBspRoot, pos, to);

	if (g_bPickFound)
		return true;

	return false;
}

#define PICK_TOLERENCE 0.01f
#define PICK_SIGN(x) ( (x)<-PICK_TOLERENCE ? -1 : (x)>PICK_TOLERENCE ? 1 : 0 )

// side ÂÊ°ú v0-v1 ¼±ºÐÀÌ ±³Â÷ÇÏ´Â ºÎºÐÀÌ ÀÖÀ¸¸é true ¸¦ ¸®ÅÏÇÏ¸é¼­ ±³Â÷ºÎºÐÀ» w0-w1·Î ¸®ÅÏ
bool pick_checkplane(int side, rplane& plane, const rvector& v0, const rvector& v1, rvector* w0, rvector* w1)
{
	float dotv0 = D3DXPlaneDotCoord(&plane, &v0);
	float dotv1 = D3DXPlaneDotCoord(&plane, &v1);

	int signv0 = PICK_SIGN(dotv0), signv1 = PICK_SIGN(dotv1);

	if (signv0 != -side) {
		*w0 = v0;

		if (signv1 != -side)
			*w1 = v1;
		else
		{
			rvector intersect;
			if (D3DXPlaneIntersectLine(&intersect, &plane, &v0, &v1))
				*w1 = intersect;
			else
				*w1 = v1;
		}
		return true;
	}

	if (signv1 != -side) {
		*w1 = v1;

		if (signv0 != -side)
			*w0 = v0;
		else
		{
			rvector intersect;
			if (D3DXPlaneIntersectLine(&intersect, &plane, &v0, &v1))
				*w0 = intersect;
			else
				*w0 = v0;
		}
		return true;
	}

	return false;
}


bool RBspObject::Pick(RSBspNode* pNode, const rvector& v0, const rvector& v1)
{
	if (!pNode) return false;

	// leaf node ÀÌ¸é
	if (pNode->nPolygon) {
		bool bPicked = false;

		for (int i = 0; i < pNode->nPolygon; i++)
		{
			RPOLYGONINFO* pInfo = &pNode->pInfo[i];
			if ((pInfo->dwFlags & g_dwPassFlag) != 0) continue;
			if (D3DXPlaneDotCoord(&pInfo->plane, &g_PickOrigin) < 0) continue;

			for (int j = 0; j < pInfo->nVertices - 2; j++)
			{
				float fDist;
				if (IsIntersect(g_PickOrigin, g_PickDir,
					*pInfo->pVertices[0].Coord(),
					*pInfo->pVertices[j + 1].Coord(),
					*pInfo->pVertices[j + 2].Coord(), &fDist))
				{
					rvector pos;
					D3DXPlaneIntersectLine(&pos, &pNode->pInfo[i].plane, &g_PickOrigin, &g_PickTo);

					if (D3DXPlaneDotCoord(&g_PickPlane, &pos) >= 0)
					{
						float fDist = Magnitude(pos - g_PickOrigin);
						if (fDist < g_fPickDist)
						{
							bPicked = true;
							g_fPickDist = fDist;
							g_pPickOut->PickPos = pos;
							g_pPickOut->pNode = pNode;
							g_pPickOut->nIndex = i;
							g_pPickOut->pInfo = &pNode->pInfo[i];
						}
					}
				}
			}
		}

		return bPicked;
	}

	rvector w0, w1;
	bool bHit = false;
	if (D3DXPlaneDotNormal(&pNode->plane, &g_PickDir) > 0) {
		// ºÐÇÒÆò¸éÀÇ ¹ý¼±ÀÌ °°Àº¹æÇâÀÌ¸é m_pNegative -> postive ¼øÀ¸·Î °Ë»ç

		if (pick_checkplane(-1, pNode->plane, v0, v1, &w0, &w1))
		{
			bHit = Pick(pNode->m_pNegative, w0, w1);
			if (bHit) return true;
		}

		if (pick_checkplane(1, pNode->plane, v0, v1, &w0, &w1))
		{
			bHit |= Pick(pNode->m_pPositive, w0, w1);
		}

		return bHit;
	}
	else
	{
		if (pick_checkplane(1, pNode->plane, v0, v1, &w0, &w1))
		{
			bHit = Pick(pNode->m_pPositive, w0, w1);
			if (bHit) return true;
		}

		if (pick_checkplane(-1, pNode->plane, v0, v1, &w0, &w1))
		{
			bHit |= Pick(pNode->m_pNegative, w0, w1);
		}

		return bHit;
	}

	return false;
}

// lightmap »ý¼ºÇÒ¶§¸¸ ¾²ÀÌ´Â Æã¼Ç
// ÃßÈÄ¿¡ opacity ¸ÊÀ» ¾´³ÑÀº ÅØ½ºÃÄ¸¦ Ã£¾Æº¸¸é ÁÁ´Ù. Áö±ÝÀº pick ¿¡ ¿µÇâÀ» ¾È¹ÌÄ§.
bool RBspObject::PickShadow(RSBspNode* pNode, rvector& v0, rvector& v1)
{
	if (!pNode) return false;

	// leaf node ÀÌ¸é
	if (pNode->nPolygon) {
		bool bPicked = false;

		for (int i = 0; i < pNode->nPolygon; i++)
		{
			RPOLYGONINFO* pInfo = &pNode->pInfo[i];
			if ((pInfo->dwFlags & (RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY | RM_FLAG_HIDE)) != 0 ||
				(pInfo->dwFlags & RM_FLAG_CASTSHADOW) == 0 ||
				(D3DXPlaneDotCoord(&pInfo->plane, &g_PickOrigin) < 0)) continue;

			for (int j = 0; j < pInfo->nVertices - 2; j++)
			{
				float fDist;
				if (
					/*
					(pInfo->dwFlags & (RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY | RM_FLAG_HIDE)) == 0 &&
					(pInfo->dwFlags & RM_FLAG_CASTSHADOW) != 0 &&
					(D3DXPlaneDotCoord(&pInfo->plane,&g_PickOrigin)>=0) &&
					*/
					IsIntersect(g_PickOrigin, g_PickDir,
						*pInfo->pVertices[0].Coord(),
						*pInfo->pVertices[j + 1].Coord(),
						*pInfo->pVertices[j + 2].Coord(), &fDist))
				{
					rvector pos;
					D3DXPlaneIntersectLine(&pos, &pInfo->plane, &g_PickOrigin, &g_PickTo);

					if (D3DXPlaneDotCoord(&g_PickPlane, &pos) >= 0)
					{
						float fDist = Magnitude(pos - g_PickOrigin);
						if (fDist < g_fPickDist)
						{
							bPicked = true;
							g_fPickDist = fDist;
							g_pPickOut->PickPos = pos;
							g_pPickOut->pNode = pNode;
							g_pPickOut->nIndex = i;
							g_pPickOut->pInfo = pInfo;
						}
					}
				}
			}
		}

		return bPicked;
	}

	rvector w0, w1;
	bool bHit = false;
	if (D3DXPlaneDotNormal(&pNode->plane, &g_PickDir) > 0) {
		// ºÐÇÒÆò¸éÀÇ ¹ý¼±ÀÌ °°Àº¹æÇâÀÌ¸é m_pNegative -> postive ¼øÀ¸·Î °Ë»ç

		if (pick_checkplane(-1, pNode->plane, v0, v1, &w0, &w1))
		{
			bHit = PickShadow(pNode->m_pNegative, w0, w1);
			if (bHit) {
				return true;
				//	v1=g_pPickOut->PickPos;
				//	return true;
			}
		}

		if (pick_checkplane(1, pNode->plane, v0, v1, &w0, &w1))
		{
			bHit |= PickShadow(pNode->m_pPositive, w0, w1);
		}

		return bHit;
	}
	else
	{
		if (pick_checkplane(1, pNode->plane, v0, v1, &w0, &w1))
		{
			bHit = PickShadow(pNode->m_pPositive, w0, w1);
			if (bHit) return true;

			/*
			if(bHit) {
				return true;
				rvector dir=g_pPickOut->PickPos-v0;
				Normalize(dir);
				_ASSERT(DotProduct(dir,g_PickDir)>0.99);
				v1=g_pPickOut->PickPos;
			}
			*/
		}

		if (pick_checkplane(-1, pNode->plane, v0, v1, &w0, &w1))
		{
			bHit |= PickShadow(pNode->m_pNegative, w0, w1);
		}

		return bHit;
	}

	return false;
}

void RBspObject::GetNormal(int nConvexPolygon, rvector& position, rvector* normal)
{
	RCONVEXPOLYGONINFO* poly = m_pConvexPolygons + nConvexPolygon;
	int au, av, ax; // ÃàÀÇ index    °¢°¢ ÅØ½ºÃÄ¿¡¼­ÀÇ uÃà, vÃà, ±×¸®°í ³ª¸ÓÁö ÇÑÃà..

	if (fabs(poly->plane.a) > fabs(poly->plane.b) && fabs(poly->plane.a) > fabs(poly->plane.c))
		ax = 0;   // yz Æò¸éÀ¸·Î projection	u ´Â y Ãà¿¡ v ´Â z Ãà¿¡ ´ëÀÀ, ³ª¸ÓÁöÇÑÃà ax´Â x Ãà
	else if (fabs(poly->plane.b) > fabs(poly->plane.c))
		ax = 1;	// xz Æò¸éÀ¸·Î ...
	else
		ax = 2;	// xy Æò¸éÀ¸·Î ...

	au = (ax + 1) % 3;
	av = (ax + 2) % 3;

	GetNormal(poly, position, normal, au, av);
}


// µ¿Àû±¤¿øÀ» ±×¸®±âÀ§ÇÑ Æã¼Çµé.. ¾ÆÁ÷ Å×½ºÆ®Áß

RBaseTexture* RBspObject::m_pShadeMap;

bool RBspObject::CreateShadeMap(const char* szShadeMap)
{
	if (m_pShadeMap)
		DestroyShadeMap();
	m_pShadeMap = RCreateBaseTexture(szShadeMap, RTextureType_Etc, false);
	return true;
}

void RBspObject::DestroyShadeMap()
{
	RDestroyBaseTexture(m_pShadeMap);
	m_pShadeMap = NULL;
}

struct LIGHTBSPVERTEX {
	rvector coord;
	DWORD dwColor;
	float tu1, tv1;
	float tu2, tv2;
};

#define LIGHTVERTEXBUFFER_SIZE	1024

DWORD m_dwLightVBBase = 0;
LIGHTBSPVERTEX* m_pLightVertex;

bool RBspObject::CreateDynamicLightVertexBuffer()
{
	InvalidateDynamicLightVertexBuffer();
	HRESULT hr = RGetDevice()->CreateVertexBuffer(sizeof(LIGHTBSPVERTEX) * LIGHTVERTEXBUFFER_SIZE * 3,
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, LIGHT_BSP_FVF, D3DPOOL_DEFAULT, &m_pDynLightVertexBuffer, NULL);

	return true;
}

void RBspObject::InvalidateDynamicLightVertexBuffer()
{
	SAFE_RELEASE(m_pDynLightVertexBuffer);
}


bool RBspObject::FlushLightVB()
{
	m_pDynLightVertexBuffer->Unlock();
	if (m_dwLightVBBase == 0) return true;

	g_nCall++;
	HRESULT hr = RGetDevice()->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_dwLightVBBase);
	_ASSERT(hr == D3D_OK);
	return true;
}

bool RBspObject::LockLightVB()
{
	HRESULT hr;
	if (FAILED(hr = m_pDynLightVertexBuffer->Lock(0,
		LIGHTVERTEXBUFFER_SIZE * sizeof(LIGHTBSPVERTEX),
		(LPVOID*)&m_pLightVertex, D3DLOCK_DISCARD)))
	{
		return false;
	}
	m_dwLightVBBase = 0;

	return true;
}

D3DLIGHT9* g_pTargetLight;
DWORD		g_dwTargetLightColor;

bool RBspObject::DrawLight(RSBspNode* pNode, int nMaterial)
{
	if (pNode->nFrameCount != g_nFrameNumber) return true;

	if (pNode->nPolygon)
	{
		int nCount = pNode->pDrawInfo[nMaterial].nTriangleCount;

		if (nCount)
		{
			//g_nPoly += nCount;

			RDrawInfo* pdi = &pNode->pDrawInfo[nMaterial];
			int index = pdi->nIndicesOffset;

			for (int i = 0; i < nCount; ++i)
			{
				rplane* pPlane = &pdi->pPlanes[i];

				WORD indices[3] = { m_pOcIndices[index++],m_pOcIndices[index++],m_pOcIndices[index++] };

				float fPlaneDotCoord = pPlane->a * g_pTargetLight->Position.x + pPlane->b * g_pTargetLight->Position.y +
					pPlane->c * g_pTargetLight->Position.z + pPlane->d;

				if (fPlaneDotCoord > g_pTargetLight->Range) continue;

#define BACK_FACE_DISTANCE 200.f

				if (fPlaneDotCoord < -BACK_FACE_DISTANCE) continue;
				if (fPlaneDotCoord < 0) fPlaneDotCoord = -fPlaneDotCoord / BACK_FACE_DISTANCE * g_pTargetLight->Range;

				LIGHTBSPVERTEX* v = m_pLightVertex + m_dwLightVBBase * 3;

				for (int j = 0; j < 3; j++) {
					BSPVERTEX* pv = &m_pOcVertices[indices[j]] + pNode->m_nBaseVertexIndex;;

					if (pv)
					{
						v[j].coord = *pv->Coord();
						v[j].tu2 = pv->tu1;
						v[j].tv2 = pv->tv1;
					}
				}

				for (int j = 0; j < 3; j++)
				{
					rvector l = v[j].coord - g_pTargetLight->Position;
					l *= 1.f / g_pTargetLight->Range;

					v[j].tu1 = -DotProduct(pdi->pUAxis[i], l) * .5 + .5;
					v[j].tv1 = -DotProduct(pdi->pVAxis[i], l) * .5 + .5;

					float fIntensity = min(1.f, max(0, 1.f - fPlaneDotCoord / g_pTargetLight->Range));

					v[j].dwColor = DWORD(fIntensity * 255) << 24 | g_dwTargetLightColor;
				}

				m_dwLightVBBase++;

				if (m_dwLightVBBase == LIGHTVERTEXBUFFER_SIZE)
				{
					FlushLightVB();
					LockLightVB();
				}
			}
		}
	}
	else
	{
		bool bOk = true;
		if (pNode->m_pNegative) {
			if (!DrawLight(pNode->m_pNegative, nMaterial))
				bOk = false;
		}
		if (pNode->m_pPositive) {
			if (!DrawLight(pNode->m_pPositive, nMaterial))
				bOk = false;
		}
		return bOk;
	}
	return true;
}

void RBspObject::DrawLight(D3DLIGHT9* pLight)
{
	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();
	if (!m_pVertexBuffer)
		return;

	if (!m_pDynLightVertexBuffer)
		CreateDynamicLightVertexBuffer();

	pd3dDevice->SetTexture(0, m_pShadeMap->GetTexture());

	RGetDevice()->SetStreamSource(0, m_pDynLightVertexBuffer, 0, sizeof(LIGHTBSPVERTEX));

	g_pTargetLight = pLight;
	g_dwTargetLightColor = FLOAT2RGB24(
		min(1.f, max(0.f, g_pTargetLight->Diffuse.r)),
		min(1.f, max(0.f, g_pTargetLight->Diffuse.g)),
		min(1.f, max(0.f, g_pTargetLight->Diffuse.b)));

	//g_nPoly = 0;
	//g_nCall = 0;
	g_nFrameNumber++;

	int nChosen = ChooseNodes(m_pOcRoot, rvector(pLight->Position), pLight->Range);

	for (int i = 0; i < m_nMaterial; i++)
	{
		if ((m_pMaterials[i % m_nMaterial].dwFlags & RM_FLAG_TWOSIDED) == 0)
			RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		else
			RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		if ((m_pMaterials[i % m_nMaterial].dwFlags & RM_FLAG_ADDITIVE) == 0)
		{
			int nMaterial = i % m_nMaterial;
			RBaseTexture* pTex = m_pMaterials[nMaterial].texture;
			if (pTex)
			{
				pd3dDevice->SetTexture(1, pTex->GetTexture());
				pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			}
			else
			{
				DWORD dwDiffuse = VECTOR2RGB24(m_pMaterials[nMaterial].Diffuse);
				pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, dwDiffuse);
				pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TFACTOR);
			}

			LockLightVB();
			DrawLight(m_pOcRoot, i);
			FlushLightVB();
		}
	}

	pd3dDevice->SetStreamSource(0, NULL, 0, 0);
}

void RBspObject::OnUpdate(float fElapsed)
{
	return;
	for (auto const& itor : m_FlagList.m_MapObjectList)
	{
		ROBJECTINFO* flagObj = itor;
		D3DXVECTOR3 pos = flagObj->pVisualMesh->GetPosition();

		if (isInViewFrustum(flagObj->pVisualMesh->GetPosition(), RGetViewFrustum()) == false)
		{
			continue;
		}
    }
}
	

void RBspObject::DrawLights()
{
	for (RLightList::iterator itor = GetMapLightList()->begin(); itor != GetMapLightList()->end(); ++itor)
	{
		RLIGHT* mapLight = *itor;

		if (isInViewFrustum(mapLight->Position, RGetViewFrustum()))
		{
			RDrawSphere(mapLight->Position, 10, 10);
		}
	}
}

_NAMESPACE_REALSPACE2_END