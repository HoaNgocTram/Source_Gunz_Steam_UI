#ifndef __RBSPOBJECT_H
#define __RBSPOBJECT_H

#include <stdio.h>
#include <list>

#include "RTypes.h"
//#include "RPath.h"
#include "RLightList.h"
#include "RMeshMgr.h"
#include "RAnimationMgr.h"
#include "RMaterialList.h"
#include "ROcclusionList.h"
#include "RDummyList.h"
#include "RSolidBsp.h"
#include "RNavigationMesh.h"

class MZFile;
class MZFileSystem;
class MXmlElement;

_NAMESPACE_REALSPACE2_BEGIN

struct RMATERIAL;
class RMaterialList;
class RDummyList;
class RBaseTexture;
class RSBspNode;


struct RDEBUGINFO {
	int nCall, nPolygon;
	int nMapObjectFrustumCulled;
	int nMapObjectOcclusionCulled;
	RSolidBspNode* pLastColNode;
};

struct BSPVERTEX {

	float x, y, z;		// world position
//	float nx,ny,nz;		// normal				// Áö±ÝÀº ÀÇ¹Ì¾ø´Ù
	float tu1, tv1;		// texture coordinates
	float tu2, tv2;

	rvector* Coord() { return (rvector*)&x; }
	//	rvector *Normal() { return (rvector*)&nx; }
};

//#define BSP_FVF	(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2)
#define BSP_FVF	(D3DFVF_XYZ | D3DFVF_TEX2)

#define LIGHT_BSP_FVF	(D3DFVF_XYZ | D3DFVF_TEX2 | D3DFVF_DIFFUSE)

struct RPOLYGONINFO {
	rplane	plane;
	int		nMaterial;
	int		nConvexPolygon;
	int		nLightmapTexture;
	int		nPolygonID;
	DWORD	dwFlags;

	BSPVERTEX* pVertices;
	int		nVertices;
	int		nIndicesPos;
};

struct RCONVEXPOLYGONINFO {
	rplane	plane;
	rvector* pVertices;
	rvector* pNormals;
	int	nVertices;
	int	nMaterial;
	float fArea;
	DWORD	dwFlags;
};

struct ROBJECTINFO {
	string		name;
	int			nMeshID;
	RVisualMesh* pVisualMesh;
	RLIGHT* pLight;
	float		fDist;
	float       fRadius = 0.f;
	float       fHeight = 0.f;
	bool		isCollidable = 0;
};

struct RBSPPICKINFO {
	RSBspNode* pNode;
	int nIndex;
	rvector PickPos;
	RPOLYGONINFO* pInfo;
};

class RMapObjectList {
public:
	RMapObjectList() { };
	virtual ~RMapObjectList();

	//Custom: Lightmaps Time Code
	std::vector<ROBJECTINFO*> m_MapObjectList;
	std::vector<ROBJECTINFO*>::iterator Delete(std::vector<ROBJECTINFO*>::iterator mapObjItr);

};

class RDrawInfo {
public:
	RDrawInfo() {
		nVertice = 0;
		pVertices = NULL;
		nIndicesOffset = 0;
		nTriangleCount = 0;
		pPlanes = NULL;
		pUAxis = NULL;
		pVAxis = NULL;

	}

	~RDrawInfo() {
		SAFE_DELETE(pVertices);
		SAFE_DELETE(pPlanes);
		SAFE_DELETE(pUAxis);
		SAFE_DELETE(pVAxis);
	}

	int				nVertice;		// ¹öÅØ½º ¼ö
	BSPVERTEX* pVertices;		// ¹öÅØ½º
	int				nIndicesOffset;	// index°¡ ½ÃÀÛÇÏ´Â°÷ÀÇ ¿É¼Â
	int				nTriangleCount;	// »ï°¢Çü °¹¼ö
	rplane* pPlanes;		// Æò¸éÀÇ ¹æÁ¤½Ä(»ï°¢Çü°³¼ö¸¸Å­)
	rvector* pUAxis;		// uv °è»ê¿¡ ÇÊ¿äÇÑ ±âÁØÃà
	rvector* pVAxis;		// uv °è»ê¿¡ ÇÊ¿äÇÑ ±âÁØÃà
};

class RSBspNode
{
public:
	int				nPolygon;
	//	int				nPosition;		// vertex buffer ³»ÀÇ À§Ä¡.
	RPOLYGONINFO* pInfo;			// Æú¸®°ï Á¤º¸ÀÇ ¹è¿­ ½ÃÀÛÀ§Ä¡
	RPOLYGONINFO** ppInfoSorted;	// ¼ÒÆÃµÈ Æú¸®°ï Á¤º¸ÀÇ ¹è¿­
	RDrawInfo* pDrawInfo;		// materialº° ÇØ´ç Æú¸®°ïÀ» ±×¸®±âÀ§ÇÑ Á¤º¸

	int				nFrameCount;		// ¸¶Áö¸· ·»´õ¸µµÈ ÇÁ·¹ÀÓ..
	int				m_nBaseVertexIndex;	///< ÀÎµ¦½ºµéÀÇ base vertex index
	int				m_nVertices;		///< ¹öÅØ½º ¼ö

//	bool			bVisibletest;		// pvs Å×½ºÆ®¿ë . ÀÓ½Ã.
//	bool			bSolid;

	RSBspNode* m_pPositive, * m_pNegative;

	rplane plane;
	rboundingbox	bbTree;

	RSBspNode();
	virtual ~RSBspNode();

	RSBspNode* GetLeafNode(rvector& pos);
	void DrawWireFrame(int nFace, DWORD color);
	void DrawBoundingBox(DWORD color);
};

// ÀÚÀßÇÑ lightmap À» Å« ÅØ½ºÃÄ¿¡ ÇÑ¹ø¿¡ ¿Ã¸®´Â°É µµ¿ÍÁØ´Ù.

typedef list<POINT> RFREEBLOCKLIST;
struct RLIGHTMAPTEXTURE {
	int nSize;
	DWORD* data;
	bool bLoaded;
	POINT position;
	int	nLightmapIndex;
};

struct RBSPMATERIAL : public RMATERIAL {
	RBSPMATERIAL() { texture = NULL; }
	RBSPMATERIAL(RMATERIAL* mat)
	{
		Ambient = mat->Ambient;
		Diffuse = mat->Diffuse;
		DiffuseMap = mat->DiffuseMap;
		dwFlags = mat->dwFlags;
		Name = mat->Name;
		Power = mat->Power;
		Specular = mat->Specular;
	};
	RBaseTexture* texture;
};

class RBspLightmapManager {

public:
	RBspLightmapManager();
	virtual ~RBspLightmapManager();

	void Destroy();

	int GetSize() { return m_nSize; }
	DWORD* GetData() { return m_pData; }

	void SetSize(int nSize) { m_nSize = nSize; }
	void SetData(DWORD* pData) { Destroy(); m_pData = pData; }

	bool Add(DWORD* data, int nSize, POINT* retpoint);
	// 2^nLevel Å©±âÀÇ »ç¿ëµÇÁö¾ÊÀº RECT¸¦ »©³»ÁØ´Ù..
	bool GetFreeRect(int nLevel, POINT* pt);

	void Save(const char* filename);

	// ½ÇÇà°ú´Â °ü°è°¡ ¾ø°í. ´Ü¼øÈ÷ Âü°íÇÏ±â À§ÇÑ µ¥ÀÌÅÍÀÓ
	// ³²Àº ¾ç 0~1 À» °è»êÇØ¼­ m_fUnused·Î ³Ö´Â´Ù
	float CalcUnused();
	float m_fUnused;

protected:
	RFREEBLOCKLIST* m_pFreeList;
	DWORD* m_pData;
	int m_nSize;
};

struct FogInfo
{
	bool bFogEnable;
	DWORD dwFogColor;
	float fNear;
	float fFar;
	FogInfo() { bFogEnable = false; }
};

struct AmbSndInfo
{
	int itype;
	char szSoundName[64];
	rvector min;
	rvector center;
	rvector max;
	float radius;
};

#define AS_AABB		0x01
#define AS_SPHERE	0x02
#define AS_2D		0x10
#define AS_3D		0x20

// ¿¡µðÅÍ¿¡¼­ Generate Lightmap¿¡ ¾²ÀÏ Progress bar ³ªÅ¸³¾¶§ ¾²´Â ÄÝ¹éÆã¼Ç Å¸ÀÔ. Ãë¼ÒµÇ¾úÀ¸¸é ¸®ÅÏ = false
typedef bool (*RGENERATELIGHTMAPCALLBACK)(float fProgress);



class RBspObject
{
public:
	ROpenFlag m_OpenMode;

	RBspObject();
	virtual ~RBspObject();

	void ClearLightmaps();
	void DrawLights();


	// open À» ¼öÇàÇÏ¸é ±âº» È®ÀåÀÚ·Î ´ÙÀ½ÀÇ Open...Æã¼ÇµéÀ» ¼ø¼­´ë·Î ºÎ¸¥´Ù.
	bool Open(const char*, const char* descExtension, ROpenFlag nOpenFlag = ROF_RUNTIME, RFPROGRESSCALLBACK pfnProgressCallback = NULL, void* CallbackParam = NULL);
	bool IsVisible(rboundingbox& bb);		// occlusion ¿¡ ÀÇÇØ °¡·ÁÁ®ÀÖÀ¸¸é false ¸¦ ¸®ÅÏ.
	bool Draw();
	bool Pick(const rvector& pos, const rvector& dir, RBSPPICKINFO* pOut, DWORD dwPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY | RM_FLAG_HIDE);
	bool PickTo(const rvector& pos, const rvector& to, RBSPPICKINFO* pOut, DWORD dwPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY | RM_FLAG_HIDE);
	bool PickOcTree(rvector& pos, rvector& dir, RBSPPICKINFO* pOut, DWORD dwPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY | RM_FLAG_HIDE);
	// origin ¿¡¼­ targetpos ·Î ÀÌµ¿ÇÏ´Âµ¥ ¹Ì²ô·¯ÁüÀ» °¨¾ÈÇØ¼­ targetpos ¸¦ Á¶ÀýÇØ¼­ ¸®ÅÏÇØÁØ´Ù.
	bool CheckWall(rvector& origin, rvector& targetpos, float fRadius, float fHeight = 0.f, RCOLLISIONMETHOD method = RCW_CYLINDER, int nDepth = 0, rplane* pimpactplane = NULL);
	// solid ¿µ¿ª ¾È¿¡ ÀÖ´ÂÁö ?
	bool CheckSolid(rvector& pos, float fRadius, float fHeight = 0.f, RCOLLISIONMETHOD method = RCW_CYLINDER);
	bool GetShadowPosition(rvector& pos_, rvector& dir_, rvector* outNormal_, rvector* outPos_);

	bool OpenDescription(const char*);				// µð½ºÅ©¸³¼Ç ÆÄÀÏ		.xml ÆÄÀÏÀ» ¿¬´Ù.
	bool OpenRs(const char*);						// ½ÇÁ¦ ¿ùµå Á¤º¸ÆÄÀÏ	.rs ÆÄÀÏÀ» ¿¬´Ù. 
	bool OpenBsp(const char*);						// bsp Á¤º¸ ÆÄÀÏ		.bsp ÆÄÀÏÀ» ¿¬´Ù.

	//Custom: Lightmaps Time Code
	bool OpenLightmap(const char* lmName = nullptr);

//	bool OpenPathNode(const char *);				// °æ·Î Á¤º¸ÆÄÀÏ		.pat ÆÄÀÏÀ» ¿¬´Ù.
	bool OpenCol(const char*);						// collision Á¤º¸ ÆÄÀÏ	.col ÆÄÀÏÀ» ¿¬´Ù. 
	bool OpenNav(const char*);						// ³×ºñ°ÔÀÌ¼Ç¸Ê Á¤º¸ÆÄÀÏ .nav ÆÄÀÏÀ» ¿¬´Ù.
	bool DrawLight(RSBspNode* pNode, int nMaterial);
	bool GenerateLightmap(const char* filename, int nMaxLightmapSize, int nMinLightmapSize, int nSuperSample, float fToler, RGENERATELIGHTMAPCALLBACK pProgressFn = NULL);

	bool GetWireframeMode() { return m_bWireframe; }
	bool GetShowLightmapMode() { return m_bShowLightmap; }

	void OptimizeBoundingBox();	// °ÔÀÓÀÇ ·±Å¸ÀÓ¿¡¼­´Â ½ÇÁ¦¹Ù¿îµù¹Ú½º·Î Å¸ÀÌÆ®ÇÏ°Ô Àâ¾ÆÁØ´Ù.
	void DrawObjects();
	//Custom: Sky
	void DrawSky();
	void DrawFlags();
	void DrawLight(D3DLIGHT9* pLight);			// ±¤¿ø Ã³¸®¸¦.. ¸ÖÆ¼ ÆÐ½º·Î µ¡±×¸°´Ù.
	void SetWireframeMode(bool bWireframe) { m_bWireframe = bWireframe; }
	void OnInvalidate();
	void OnRestore();
	void LightMapOnOff(bool b);

	void SetObjectLight(rvector pos);
	void SetCharactorLight(rvector pos);
	void DrawBoundingBox();
	void DrawOcclusions();
	void DrawNormal(int nIndex, float fSize = 1.f);	// index : convex polygon index

	void DrawCollision_Polygon();
	void DrawCollision_Solid();

	void DrawSolid();					// ¸ðµç solid ³ëµåµéÀ» ±×¸°´Ù
	void DrawSolidNode();				// ¸¶Áö¸· Ãæµ¹ÇÑ ³ëµå¸¦ ±×¸°´Ù
	void DrawColNodePolygon(rvector& pos);

	void DrawNavi_Polygon();
	void DrawNavi_Links();
	void GetNormal(int nConvexPolygon, rvector& position, rvector* normal);
	void test_MakePortals();
	void SetMapObjectOcclusion(bool b) { m_bNotOcclusion = b; }
	void SetShowLightmapMode(bool bShowLightmap) { m_bShowLightmap = bShowLightmap; }

	//bool GeneratePathData(const char *filename,float fAngle, float fToler);
	//void GeneratePathNodeTable();

	static void SetDrawLightMap(bool b);
	static void DestroyShadeMap();

private:
	friend struct LightmapGenerator;
public:


	// ¿ùµå ÁÂÇ¥ pos ¿¡¼­ dir ¹æÇâÀÇ ¹ÝÁ÷¼±¿¡ ´ëÇØ pick !
	// PickTo() ¿ª½Ã ¹ÝÁ÷¼± ÇÇÅ·ÀÌ¹Ç·Î ÇÔ¼ö¸í¿¡ ¼ÓÁö¸»°í °á°ú°¡ true°¡ ³ª¿ÔÀ» ¶§ °Å¸® °Ë»ç¸¦ Ãß°¡·Î ÇØÁÖ¾î¾ß ¿øÇÏ´Â °á°ú¸¦ ¾òÀ» ¼ö ÀÖ´Ù


	// pathnode ´Â ºÀÀÎ..
	/*
	// È­¸é x,y ÁÂÇ¥¿¡ ÀÖ´Â pathnode ¸¦ ¸®ÅÏÇÑ´Ù..
	bool PickPathNode(int x,int y,RPathNode **pOut,rvector *ColPos);
	// ¿ùµå ÁÂÇ¥ from ¿¡¼­ to ·Î °¡´Â ¹ÝÁ÷¼±¿¡ ´ëÇØ pick pathnode !
	bool PickPathNode(rvector &from,rvector &to,RPathNode **pOut,rvector *ColPos);

	RPathList *GetPathList() { return &m_PathNodes; }
	RPathNode *GetPathNode(RSBspNode *pNode,int nIndex);
*/

// ÇØ´ç À§Ä¡ÀÇ lightmap À» ¾ò¾î³½´Ù.
	DWORD GetLightmap(rvector& Pos, RSBspNode* pNode, int nIndex);

	// ÇØ´ç Æú¸®°ïÀÇ MaterialÀ» ¾ò¾î³½´Ù.
	RBSPMATERIAL* GetMaterial(RSBspNode* pNode, int nIndex) { return GetMaterial(pNode->pInfo[nIndex].nMaterial); }
	RBSPMATERIAL* GetMaterial(int nIndex);

	// material À» ¾ò¾î³½´Ù.
	int	GetMaterialCount() { return m_nMaterial; }

	RMapObjectList* GetMapObjectList() { return &m_ObjectList; }
	//Custom: SkyBox
	RMapObjectList* GetSkyBoxList() { return &m_SkyList; }
	void SetMapObjectList(RMapObjectList* objectList) { m_ObjectList = *objectList; }
	RDummyList* GetDummyList() { return &m_DummyList; }
	RBaseTexture* GetBaseTexture(int n) { if (n >= 0 && n < m_nMaterial) return m_pMaterials[n].texture; return NULL; }

	RLightList* GetMapLightList() { return &m_StaticMapLightList; }
	RLightList* GetObjectLightList() { return &m_StaticObjectLightList; }
	RLightList* GetSunLightList() { return &m_StaticSunLigthtList; }

	RSBspNode* GetOcRootNode() { return m_pOcRoot; }
	RSBspNode* GetRootNode() { return m_pBspRoot; }

	rvector GetDimension();

	int	GetVertexCount() { return m_nVertices; }
	int	GetPolygonCount() { return m_nPolygon; }
	int GetNodeCount() { return m_nNodeCount; }
	int	GetBspPolygonCount() { return m_nBspPolygon; }
	int GetBspNodeCount() { return m_nBspNodeCount; }
	int GetConvexPolygonCount() { return m_nConvexPolygon; }
	int GetLightmapCount() { return m_nLightmap; }
	float GetUnusedLightmapSize(int index) { return m_LightmapList[index]->m_fUnused; }



	// À§Ä¡¿¡¼­ ¹Ù´Ú¿¡ ´ê´Â Á¡À» ±¸ÇÑ´Ù.
	rvector GetFloor(rvector& origin, float fRadius, float fHeight, rplane* pimpactplane = NULL);



	// ¸Ê¿¡ µî·ÏµÈ ¿ÀºêÁ§Æ® ÀÌ¿ÜÀÇ ¿ÀºêÁ§Æ® Ã³¸® ( Ä³¸¯ÅÍ ¼±ÅÃÈ­¸éµî )



	RMeshMgr* GetMeshManager() {
		return &m_MeshList;
	}


	// debug ¸¦ À§ÇØ ±×·Áº¼¸¸ÇÑ°Íµé.
	//void DrawPathNode();


	RSolidBspNode* GetColRoot() { return m_pColRoot; }


	FogInfo GetFogInfo() { return m_FogInfo; }
	list<AmbSndInfo*> GetAmbSndList() { return m_AmbSndInfoList; }


	static bool CreateShadeMap(const char* szShadeMap);

	RDEBUGINFO* GetDebugInfo() { return &m_DebugInfo; }
	RNavigationMesh* GetNavigationMesh() { return &m_NavigationMesh; }

	string m_filename;
	string m_descfilename;
private:

	// ¿¡µðÅÍ¹× µð¹ö±×¸¦ À§ÇÑ ¸ðµå 
	bool m_bWireframe;
	bool m_bShowLightmap;
	// Á÷Á¢È£ÃâµÉÀÏÀº ¾ø´Ù..
	bool DrawTNT(RSBspNode* bspNode, int nMaterial);	// no hardware T&L
	bool Draw(RSBspNode* bspNode, int nMaterial);

	void SetDiffuseMap(int nMaterial);

	bool Pick(RSBspNode* pNode, const rvector& v0, const rvector& v1);
	//	bool PickCol(RSolidBspNode *pNode,rvector v0,rvector v1);
	//	bool PickPathNode(RSBspNode *pNode);
	bool PickShadow(rvector& pos, rvector& to, RBSPPICKINFO* pOut);
	bool PickShadow(RSBspNode* pNode, rvector& v0, rvector& v1);

	void ChooseNodes(RSBspNode* bspNode);
	int ChooseNodes(RSBspNode* bspNode, rvector& center, float fRadius);			// ¸®ÅÏ°ªÀº ¼±ÅÃµÈ ³ëµåÀÇ °³¼ö
//	void TraverseTreeAndRender(RSBspNode *bspNode);
//	void DrawNodeFaces(RSBspNode *bspNode);

	inline RSBspNode* GetLeafNode(rvector& pos) { return m_pBspRoot->GetLeafNode(pos); }

	//void GetFloor(rvector *ret,RSBspNode *pNode,rvector &origin,const rvector &diff);

// for loading
	bool ReadString(MZFile* pfile, char* buffer, int nBufferSize);
	bool Open_Nodes(RSBspNode* pNode, MZFile* pfile);
	bool Open_ColNodes(RSolidBspNode* pNode, MZFile* pfile);
	bool Open_MaterialList(MXmlElement* pElement);
	bool Open_LightList(MXmlElement* pElement);
	bool Open_ObjectList(MXmlElement* pElement);
	bool Open_DummyList(MXmlElement* pElement);

	//Custom: Lightmaps Time Code
	vector<string> m_lightMapNames;
	bool Open_LightmapList(MXmlElement* pElement);

	bool Open_ConvexPolygons(MZFile* pfile);
	bool Open_OcclusionList(MXmlElement* pElement);
public:
	bool Make_LenzFalreList();
	void OnUpdate(float fElapsed);

	//Custom: Lightmaps Time Code
	void SetLightMapIndex(int const& index) { m_lightMapIndex = index; }

protected:
	bool Set_Fog(MXmlElement* pElement);
	bool Set_AmbSound(MXmlElement* pElement);

	void CreateRenderInfo();
	void CreatePolygonTableAndIndexBuffer();
	void CreatePolygonTableAndIndexBuffer(RSBspNode* pNode);
	void Sort_Nodes(RSBspNode* pNode);

	bool CreateVertexBuffer();
	bool UpdateVertexBuffer();

	bool CreateIndexBuffer();
	bool UpdateIndexBuffer();

	bool CreateDynamicLightVertexBuffer();
	void InvalidateDynamicLightVertexBuffer();
	bool FlushLightVB();
	bool LockLightVB();
	LPDIRECT3DVERTEXBUFFER9 m_pDynLightVertexBuffer;

	static RBaseTexture* m_pShadeMap;

	// ½ÇÁ¦ Æ®¸®
	BSPVERTEX* m_pBspVertices, * m_pOcVertices;
	WORD* m_pBspIndices, * m_pOcIndices;
	RSBspNode* m_pBspRoot, * m_pOcRoot;
	RPOLYGONINFO* m_pBspInfo, * m_pOcInfo;
	int m_nPolygon, m_nNodeCount, m_nVertices, m_nIndices;
	int m_nBspPolygon, m_nBspNodeCount, m_nBspVertices, m_nBspIndices;
	LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;
	LPDIRECT3DINDEXBUFFER9 m_pIndexBuffer;

	// Materials & texture map
	int m_nMaterial;
	RBSPMATERIAL* m_pMaterials;

	rplane m_localViewFrustum[6];

	// occlusions
	ROcclusionList m_OcclusionList;

	/*
	int m_nOcclusion;
	ROcclusion	*m_pOcclusion;
	*/


	//Custom: Lightmaps Time Code

	std::vector<vector<LPDIRECT3DTEXTURE9>> m_ppLightmapTextures;
	vector<RBspLightmapManager*> m_LightmapList;

	int							m_nLightmap;
	int                         m_lightMapIndex;
	bool                        m_isMultiLightMap;

	void CalcLightmapUV(RSBspNode* pNode, int* pLightmapInfo, vector<RLIGHTMAPTEXTURE*>* pLightmaps);

	// interpolated normal
	void GetNormal(RCONVEXPOLYGONINFO* poly, rvector& position, rvector* normal, int au, int av);
	// interpolated uv
	void GetUV(rvector& Pos, RSBspNode* pNode, int nIndex, float* uv);

	// ¿øº» Æú¸®°ï º¸°ü
	int					m_nConvexPolygon, m_nConvexVertices;
	rvector* m_pConvexVertices;
	rvector* m_pConvexNormals;
	RCONVEXPOLYGONINFO* m_pConvexPolygons;

	// ±¤¿øÁ¤º¸µé..
	rvector		m_AmbientLight;
	RLightList	m_StaticMapLightList;
	RLightList	m_StaticObjectLightList;
	RLightList	m_StaticSunLigthtList;

	// pathfinding ¿¡ ÇÊ¿äÇÑ ÀÚ·á
	//	RPathList m_PathNodes;

	// ¸Ê¿¡ ±âº»ÀûÀ¸·Î ÀÖ´Â object µé..
	RMeshMgr			m_MeshList;
	RAnimationMgr		m_AniList;
	RMapObjectList		m_ObjectList;
	RMapObjectList		m_SkyList;
	RMapObjectList		m_FlagList;
	bool				m_bNotOcclusion;

	// Ãæµ¹Ã¼Å©¿ë ¸Ê
	RSolidBspNode* m_pColRoot;
	rvector* m_pColVertices;

	// ³×ºñ°ÔÀÌ¼Ç¿ë ¸Ê
	RNavigationMesh		m_NavigationMesh;


	// ´õ¹Ì ¸®½ºÆ®
	RDummyList	m_DummyList;

	// FogÁ¤º¸
	FogInfo m_FogInfo;

	// »ç¿îµå Á¤º¸
	list<AmbSndInfo*>	m_AmbSndInfoList;

	// µð¹ö±× Á¤º¸
	RDEBUGINFO			m_DebugInfo;
};

_NAMESPACE_REALSPACE2_END


#endif