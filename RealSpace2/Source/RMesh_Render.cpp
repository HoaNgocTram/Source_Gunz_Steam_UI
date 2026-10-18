#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <tchar.h>

#include "MXml.h"

#include "RealSpace2.h"
#include "RMesh.h"
#include "RMeshMgr.h"

#include "MDebug.h"

#include "RAnimationMgr.h"
#include "RVisualmeshMgr.h"

#include "MZFileSystem.h"
#include "fileinfo.h"

#include "RShaderMgr.h"
#include "vector"

#include "ROcclusionList.h"

using namespace std;

#ifndef _PUBLISH

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

bool g_bVertex_Soft = false;

/////////////////////////////////////////////////////////////////////////////

_NAMESPACE_REALSPACE2_BEGIN

int g_poly_render_cnt;


#define RENDER_NODE_MAX 1000

int			g_render_cnt=0;
RRenderNode	g_render_node[ RENDER_NODE_MAX ];

RRenderNodeList	g_RenderNodeList[eRRenderNode_End];
RRenderNodeList	g_RenderLNodeList[eRRenderNode_End];

bool g_rmesh_render_start_begin = false;

int	 g_vert_index_pos = 0;
int	 g_lvert_index_pos = 0;

////////////////////////////////////////////////////////////

int __cdecl _SortAlpha( const VOID* arg1, const VOID* arg2 )
{
	RMeshNode* p1 = *(RMeshNode**)arg1;
	RMeshNode* p2 = *(RMeshNode**)arg2;

	if (p1->m_AlphaSortValue < p2->m_AlphaSortValue)
		return +1;

	return -1;
}

int __cdecl _SortLastName( const VOID* arg1, const VOID* arg2 )
{
	RMeshNode* p1 = *(RMeshNode**)arg1;
	RMeshNode* p2 = *(RMeshNode**)arg2;

	if (p1->m_Name < p2->m_Name)
		return +1;

	return -1;
}

void RMesh::Render(D3DXMATRIX* world_mat,bool NoPartsChange)
{
	RenderSub(world_mat,NoPartsChange,false);
}

static bool RMeshOutlineContainsNode(const std::vector<RMeshNode*>& nodes, RMeshNode* pNode)
{
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i] == pNode)
			return true;
	}
	return false;
}


void RMesh::RenderOutline(D3DXMATRIX* world_mat)
{
	if (!world_mat) return;
	if (!m_pVisualMesh || !m_pVisualMesh->IsOutlineEnabled()) return;
	if (!RIsSupportVS() || !RShaderMgr::mbUsingShader) return;
	if (!RGetShaderMgr()->hasShader(SHADER_OUTLINE)) return;
	if (m_pVisualMesh->IsRenderMatrix()) return;

	LPDIRECT3DDEVICE9 dev = RGetDevice();
	if (!dev) return;

	DWORD oldLighting = TRUE;
	DWORD oldAlphaBlend = FALSE;
	DWORD oldAlphaTest = FALSE;
	DWORD oldZWrite = TRUE;
	DWORD oldZFunc = D3DCMP_LESSEQUAL;
	DWORD oldCullMode = D3DCULL_CCW;
	DWORD oldSlopeScaleDepthBias = 0;
	DWORD oldDepthBias = 0;
	DWORD oldColorWrite = 0x0000000F;
	DWORD oldTextureFactor = 0;
	DWORD oldStencilEnable = FALSE;
	DWORD oldStencilFail = D3DSTENCILOP_KEEP;
	DWORD oldStencilZFail = D3DSTENCILOP_KEEP;
	DWORD oldStencilPass = D3DSTENCILOP_KEEP;
	DWORD oldStencilFunc = D3DCMP_ALWAYS;
	DWORD oldStencilRef = 0;
	DWORD oldStencilMask = 0xFFFFFFFF;
	DWORD oldStencilWriteMask = 0xFFFFFFFF;
	DWORD oldColorOp0 = D3DTOP_MODULATE;
	DWORD oldColorArg10 = D3DTA_TEXTURE;
	DWORD oldAlphaOp0 = D3DTOP_MODULATE;
	DWORD oldAlphaArg10 = D3DTA_TEXTURE;
	DWORD oldColorOp1 = D3DTOP_DISABLE;
	DWORD oldAlphaOp1 = D3DTOP_DISABLE;
	LPDIRECT3DVERTEXDECLARATION9 oldDecl = NULL;
	LPDIRECT3DVERTEXSHADER9 oldVS = NULL;
	LPDIRECT3DBASETEXTURE9 oldTex0 = NULL;
	LPDIRECT3DBASETEXTURE9 oldTex1 = NULL;

	dev->GetRenderState(D3DRS_LIGHTING, &oldLighting);
	dev->GetRenderState(D3DRS_ALPHABLENDENABLE, &oldAlphaBlend);
	dev->GetRenderState(D3DRS_ALPHATESTENABLE, &oldAlphaTest);
	dev->GetRenderState(D3DRS_ZWRITEENABLE, &oldZWrite);
	dev->GetRenderState(D3DRS_ZFUNC, &oldZFunc);
	dev->GetRenderState(D3DRS_CULLMODE, &oldCullMode);
	dev->GetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, &oldSlopeScaleDepthBias);
	dev->GetRenderState(D3DRS_DEPTHBIAS, &oldDepthBias);
	dev->GetRenderState(D3DRS_COLORWRITEENABLE, &oldColorWrite);
	dev->GetRenderState(D3DRS_TEXTUREFACTOR, &oldTextureFactor);
	dev->GetRenderState(D3DRS_STENCILENABLE, &oldStencilEnable);
	dev->GetRenderState(D3DRS_STENCILFAIL, &oldStencilFail);
	dev->GetRenderState(D3DRS_STENCILZFAIL, &oldStencilZFail);
	dev->GetRenderState(D3DRS_STENCILPASS, &oldStencilPass);
	dev->GetRenderState(D3DRS_STENCILFUNC, &oldStencilFunc);
	dev->GetRenderState(D3DRS_STENCILREF, &oldStencilRef);
	dev->GetRenderState(D3DRS_STENCILMASK, &oldStencilMask);
	dev->GetRenderState(D3DRS_STENCILWRITEMASK, &oldStencilWriteMask);
	dev->GetTextureStageState(0, D3DTSS_COLOROP, &oldColorOp0);
	dev->GetTextureStageState(0, D3DTSS_COLORARG1, &oldColorArg10);
	dev->GetTextureStageState(0, D3DTSS_ALPHAOP, &oldAlphaOp0);
	dev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &oldAlphaArg10);
	dev->GetTextureStageState(1, D3DTSS_COLOROP, &oldColorOp1);
	dev->GetTextureStageState(1, D3DTSS_ALPHAOP, &oldAlphaOp1);
	dev->GetVertexDeclaration(&oldDecl);
	dev->GetVertexShader(&oldVS);
	dev->GetTexture(0, &oldTex0);
	dev->GetTexture(1, &oldTex1);

	dev->SetTexture(0, NULL);
	dev->SetTexture(1, NULL);
	dev->SetRenderState(D3DRS_LIGHTING, FALSE);
	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	dev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	std::vector<RMeshNode*> StencilNodes;
	std::vector<RMeshNode*> OutlineNodes;
	StencilNodes.reserve(m_list.size());
	OutlineNodes.reserve(m_list.size());

	RMeshNodeHashList_Iter it_obj = m_list.begin();
	while (it_obj != m_list.end())
	{
		RMeshNode* pMeshNode = (*it_obj);
		++it_obj;
		if (!pMeshNode) continue;

		RMeshNode* pPartsMeshNode = pMeshNode;

		if (m_pVisualMesh)
		{
			if (pMeshNode->m_PartsType == eq_parts_face && m_pVisualMesh->SkipRenderFaceParts())
				continue;

			pPartsMeshNode = m_pVisualMesh->GetParts(pMeshNode->m_PartsType);
			m_pVisualMesh->UpdateWeaponDummyMatrix(pMeshNode);

			if (pMeshNode->m_isWeaponMesh)
			{
				m_pVisualMesh->UpdateWeaponPartInfo(pMeshNode);
				continue;
			}
			else
			{
				if (pPartsMeshNode == NULL)
					pPartsMeshNode = pMeshNode;
				else if (pPartsMeshNode->m_PartsType != pMeshNode->m_PartsType)
					pPartsMeshNode = pMeshNode;
			}
		}

		pPartsMeshNode->m_pBaseMesh = this;

		if (pMeshNode->m_isAddMeshNode)
		{
			if (pMeshNode->m_pParent)
				pPartsMeshNode->m_mat_result = pMeshNode->m_mat_add * pMeshNode->m_pParent->m_mat_result;
		}

		if (pPartsMeshNode->m_WeaponDummyType != weapon_dummy_etc)
			continue;

		if (pPartsMeshNode->m_isDummyMesh)
			continue;

		pPartsMeshNode->GetNodeVisValue();

		if (pPartsMeshNode->m_PartsType == eq_parts_chest && !m_is_map_object)
		{
			if (m_pVisualMesh && m_pVisualMesh->isChestClothMesh() && pPartsMeshNode->m_isClothMeshNode)
				continue;
		}

		if (pPartsMeshNode->m_face_num == 0)
			continue;

		if (CheckOcclusion(pPartsMeshNode) == false)
			continue;

		if (!RMeshOutlineContainsNode(StencilNodes, pPartsMeshNode))
			StencilNodes.push_back(pPartsMeshNode);

		if (pPartsMeshNode->m_PartsType != eq_parts_face)
		{
			if (!RMeshOutlineContainsNode(OutlineNodes, pPartsMeshNode))
				OutlineNodes.push_back(pPartsMeshNode);
		}
	}

	const bool bHasBodyOutline = !StencilNodes.empty() && !OutlineNodes.empty();
	const bool bHasClothOutline = (m_pVisualMesh && m_pVisualMesh->isChestClothMesh());
	DWORD dwOutlineColor = m_pVisualMesh ? m_pVisualMesh->GetOutlineColor() : 0xFFFF0000;
	float fMeshAlpha = GetMeshVis();
	if (fMeshAlpha < 0.0f) fMeshAlpha = 0.0f;
	if (fMeshAlpha > 1.0f) fMeshAlpha = 1.0f;
	DWORD dwAlpha = (DWORD)(((dwOutlineColor >> 24) & 0xff) * fMeshAlpha);
	dwOutlineColor = (dwOutlineColor & 0x00ffffff) | (dwAlpha << 24);

	if (bHasBodyOutline || bHasClothOutline)
	{
		dev->SetRenderState(D3DRS_STENCILENABLE, TRUE);
		dev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
		dev->SetRenderState(D3DRS_STENCILREF, 1);
		dev->SetRenderState(D3DRS_STENCILMASK, 0xFFFFFFFF);
		dev->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
		dev->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
		dev->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
		dev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
		dev->SetRenderState(D3DRS_COLORWRITEENABLE, 0);

		if (bHasBodyOutline)
		{
			for (size_t i = 0; i < StencilNodes.size(); ++i)
				StencilNodes[i]->RenderOutlineVS(this, world_mat, 0.0f);
		}

		if (bHasClothOutline)
		{
			dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
			dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
			dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			dev->SetRenderState(D3DRS_TEXTUREFACTOR, dwOutlineColor);
			m_pVisualMesh->RenderClothOutline(0.0f);
		}

		const float fOutlineSlopeScaleBias = 1.0f;
		const float fOutlineDepthBias = 0.00002f;
		dev->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);
		dev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL);
		dev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
		dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
		dev->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&fOutlineSlopeScaleBias);
		dev->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&fOutlineDepthBias);

		if (bHasBodyOutline)
		{
			for (size_t i = 0; i < OutlineNodes.size(); ++i)
				OutlineNodes[i]->RenderOutlineVS(this, world_mat);
		}

		if (bHasClothOutline)
		{
			dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
			dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
			dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			dev->SetRenderState(D3DRS_TEXTUREFACTOR, dwOutlineColor);
			m_pVisualMesh->RenderClothOutline(m_pVisualMesh->GetOutlineThickness());
		}

		dev->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, 0);
		dev->SetRenderState(D3DRS_DEPTHBIAS, 0);
		dev->SetRenderState(D3DRS_ZFUNC, oldZFunc);
		dev->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
		dev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
		dev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_ZERO);
		dev->SetRenderState(D3DRS_STENCILREF, 0);

		if (bHasBodyOutline)
		{
			for (size_t i = 0; i < StencilNodes.size(); ++i)
				StencilNodes[i]->RenderOutlineVS(this, world_mat, 0.0f);
		}

		if (bHasClothOutline)
		{
			dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
			dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
			dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			dev->SetRenderState(D3DRS_TEXTUREFACTOR, dwOutlineColor);
			m_pVisualMesh->RenderClothOutline(0.0f);
		}
	}

	dev->SetVertexShader(oldVS);
	dev->SetVertexDeclaration(oldDecl);
	dev->SetTexture(0, oldTex0);
	dev->SetTexture(1, oldTex1);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, oldColorOp0);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, oldColorArg10);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, oldAlphaOp0);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, oldAlphaArg10);
	dev->SetTextureStageState(1, D3DTSS_COLOROP, oldColorOp1);
	dev->SetTextureStageState(1, D3DTSS_ALPHAOP, oldAlphaOp1);
	dev->SetRenderState(D3DRS_TEXTUREFACTOR, oldTextureFactor);
	dev->SetRenderState(D3DRS_COLORWRITEENABLE, oldColorWrite);
	dev->SetRenderState(D3DRS_STENCILENABLE, oldStencilEnable);
	dev->SetRenderState(D3DRS_STENCILFAIL, oldStencilFail);
	dev->SetRenderState(D3DRS_STENCILZFAIL, oldStencilZFail);
	dev->SetRenderState(D3DRS_STENCILPASS, oldStencilPass);
	dev->SetRenderState(D3DRS_STENCILFUNC, oldStencilFunc);
	dev->SetRenderState(D3DRS_STENCILREF, oldStencilRef);
	dev->SetRenderState(D3DRS_STENCILMASK, oldStencilMask);
	dev->SetRenderState(D3DRS_STENCILWRITEMASK, oldStencilWriteMask);
	dev->SetRenderState(D3DRS_CULLMODE, oldCullMode);
	dev->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, oldSlopeScaleDepthBias);
	dev->SetRenderState(D3DRS_DEPTHBIAS, oldDepthBias);
	dev->SetRenderState(D3DRS_ZFUNC, oldZFunc);
	dev->SetRenderState(D3DRS_ZWRITEENABLE, oldZWrite);
	dev->SetRenderState(D3DRS_ALPHATESTENABLE, oldAlphaTest);
	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, oldAlphaBlend);
	dev->SetRenderState(D3DRS_LIGHTING, oldLighting);

	SAFE_RELEASE(oldVS);
	SAFE_RELEASE(oldDecl);
	SAFE_RELEASE(oldTex0);
	SAFE_RELEASE(oldTex1);
}

RMeshNode* RMesh::FindNode(RMeshPartsPosInfoType type)
{
	RMeshNode* pNode = NULL;

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	while (it_obj !=  m_list.end()) {

		pNode = (*it_obj);

		if(pNode->m_PartsPosInfoType==type)
			return pNode;

		it_obj++;
	}

	return NULL;
}

bool RMesh::CalcParts(RMeshNode* pPartsMeshNode,RMeshNode* pMeshNode,bool NoPartsChange)
{
	return false;
}

#define _SORT_ALPHA_NODE_TABLE	100
#define _SORT_LAST_NODE_TABLE	100

void RMesh::RenderSub(D3DXMATRIX* world_mat,bool NoPartsChange,bool bRenderBuffer)
{
	__BP(198, "RMesh::RenderSub");

	LPDIRECT3DDEVICE9 dev = RGetDevice();

	D3DXMATRIX	buffer,Inv;

	m_vBBMax = D3DXVECTOR3(-9999.f,-9999.f,-9999.f);
	m_vBBMin = D3DXVECTOR3( 9999.f, 9999.f, 9999.f);

 	m_vBBMaxNodeMatrix = m_vBBMax;
	m_vBBMinNodeMatrix = m_vBBMin;

	DWORD _color = 0;

	__BP(202, "RMesh::RenderSub::RenderFrame");

	RenderFrame();	// ¿¡´Ï Ã³¸®

	__EP(202);

	__BP(199, "RMesh::RenderSub::State b");

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	RMeshNode* pMeshNode = NULL;
	RMeshNode* pPartsMeshNode = NULL;

	// init render state

	// ¸ðµ¨´ÜÀ§? light setting ÀÌÀü¿¡ ·»´õ¸µÇÑ°Í°ú visualmesh °¡ ´Ù¸£´Ù¸é ¶óÀÌÆ® ¼ÂÆÃÀ» ´Ù½ÃÇØÁØ´Ù..

	if(m_pVisualMesh && !m_LitVertexModel) { 
		m_pVisualMesh->UpdateLight();
	}

	if(m_LitVertexModel)
		dev->SetRenderState( D3DRS_LIGHTING, FALSE );
	else 
		dev->SetRenderState( D3DRS_LIGHTING, TRUE );

	dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	dev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	// Custom: enable multisampling when the charmesh is being rendered
	if (RAdvancedGraphics::nMultiSampling > 0)
		dev->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	else
		dev->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
	__EP(199);

	static RMeshNode* pAlphaNode[_SORT_ALPHA_NODE_TABLE];	//ÇÑ ¿ÀºêÁ§Æ®¿¡ ¾ËÆÄ ³ëµå°¡ 100°³³ÑÀ¸¸é ¹«½Ã..]
	static RMeshNode* pLastNode[_SORT_LAST_NODE_TABLE];		//ÇÑ ¿ÀºêÁ§Æ®¿¡ Last ³ëµå°¡ 100°³³ÑÀ¸¸é ¹«½Ã..]

	int nAlphaNodeCnt = 0;
	int nLastNodeCnt = 0;

	__BP(500, "RMesh::RenderSub::Loop");

	if(m_pVisualMesh && m_pVisualMesh->IsCharacter())
		m_pVisualMesh->ClearPartInfo();

	while (it_obj !=  m_list.end()) {

		pMeshNode = (*it_obj);

		pMeshNode->CheckAlign(world_mat);

		CalcNodeMatrixBBox( pMeshNode );

		pPartsMeshNode = pMeshNode;

		if( GetToolMesh() ) {

			// Åø¿¡¼­ Æ¯Á¤ ½½·Ô ºÎÀ§¸¸ ·»´õ¸µÇÏ°íÀÚ ÇÒ¶§
			if( m_OnlyRenderPartsType != eq_parts_end &&
				m_OnlyRenderPartsType != pPartsMeshNode->m_PartsType) {
				it_obj++;
				continue;
			}

			if(m_pToolSelectNode && pMeshNode==m_pToolSelectNode)// ÅøÀÌ°í ¼±ÅÃµÈ ³ëµå¶ó¸é ¹Ú½º±×·ÁÁÖ±â...
				draw_box(&pMeshNode->m_mat_result,pMeshNode->m_max,pMeshNode->m_min,0xffff0000);
		}

		if(m_pVisualMesh) {	// visual mesh °¡ ÀÖ´Â°æ¿ì

			if (pMeshNode->m_PartsType == eq_parts_face && m_pVisualMesh->SkipRenderFaceParts())
			{
				it_obj++;
				continue; 
			}

			if(NoPartsChange)	pPartsMeshNode = NULL;
			else				pPartsMeshNode = m_pVisualMesh->GetParts(pMeshNode->m_PartsType); // ÀåºñµÈ ¸ðµ¨ Æ÷ÀÎÅÍ ¾ò¾î³»±â..

			m_pVisualMesh->UpdateWeaponDummyMatrix(pMeshNode);

			if( pMeshNode->m_isWeaponMesh ) { // Ä³¸¯ÅÍ¿¡ ´Þ¸° ¹«±â´õ¹Ìµé , Ä®´õ¹Ì,ÃÑ´õ¹Ìµî..

				m_pVisualMesh->UpdateWeaponPartInfo(pMeshNode);

				it_obj++;
				continue;
			} 
			else {								// ÀåºñÇ°µé °ú »À´ë³ëµå..

				if( pPartsMeshNode==NULL ) {
					pPartsMeshNode = pMeshNode;
				} 
				else if( pPartsMeshNode->m_PartsType != pMeshNode->m_PartsType)	// °°Àº ÀåÂøÀ§Ä¡°¡ ¾Æ´Ï¸é
				{
					pPartsMeshNode = pMeshNode;
				}
			}
		}

		pPartsMeshNode->m_pBaseMesh = this;// base model ÀÌ ÀÌ ÆÄÀÏÀÓÀ» µî·Ï,,,

		if(pMeshNode->m_isAddMeshNode) //pPartsMeshNode Á¶»çÇÏ¸é ¾ÈµÊ.. pMeshNode °¡ ´õ¹ÌÀÓ...
		{
			if(pMeshNode->m_pParent) {
				pPartsMeshNode->m_mat_result = pMeshNode->m_mat_add * pMeshNode->m_pParent->m_mat_result;
			}
		}

		if(pPartsMeshNode->m_WeaponDummyType != weapon_dummy_etc) {//¹«±â¿¡ ºÙÀº ´õ¹Ì¸é~
			it_obj++;
			continue;
		}

		if(pPartsMeshNode->m_isDummyMesh)	{ //Bip,Bone,Dummy Skip

			if( m_pVisualMesh ) {
				if( m_pVisualMesh->GetBipMatrixArray() ) {
					if(pMeshNode->m_PartsPosInfoType!=eq_parts_pos_info_etc) { 
						m_pVisualMesh->GetBipMatrixArray()[pMeshNode->m_PartsPosInfoType] = pMeshNode->m_mat_result;
					}
				}
			}
			it_obj++;
			continue;
		}		

		pPartsMeshNode->GetNodeVisValue(); // ÀÌ¹øÇÁ·¹ÀÓÀÇ vis_alpha °ªÀ» ¾ò¾î³½´Ù..³»ºÎ¿¡¼­ m_vis °ªÀ» °»½Å,,,

		// Cloth MeshNode Skip

		if( pPartsMeshNode->m_PartsType == eq_parts_chest && !NoPartsChange && !m_is_map_object) //low poly model  ÀÌ ¾Æ´Ï¿©¾ßÇÔ..
		{
			if( m_pVisualMesh && m_pVisualMesh->isChestClothMesh() && pPartsMeshNode->m_isClothMeshNode ) {

				__BP(405, "RMesh::Draw::Cloth::update&render");

				m_pVisualMesh->UpdateCloth();
				m_pVisualMesh->RenderCloth();
				
				__EP(405);

				++it_obj;
				continue;
			}
		}

//		if(pPartsMeshNode->CheckName("hair"))
//			pPartsMeshNode->m_isLastModel = true;

		if( pPartsMeshNode->m_isLastModel ) { // ÀÚ½ÅÀÇ ¸ðµ¨´ÜÀ§¿¡¼­ ³ªÁß¿¡ ±×·ÁÁ®¾ß ÇÏ´Â°Íµé..

			pLastNode[nLastNodeCnt] = pPartsMeshNode;
			nLastNodeCnt++;

			if(nLastNodeCnt > _SORT_LAST_NODE_TABLE) {
				nLastNodeCnt = _SORT_LAST_NODE_TABLE;
				mlog("%s ¿ÀºêÁ§Æ®ÀÇ Last ³ëµå°¡ 100°³°¡ ³Ñ´Â´Ù..\n",GetFileName());
				nLastNodeCnt--;
			}

			it_obj++;
			continue;
		}

//		pPartsMeshNode->m_isAlphaMtrl = false;

		if( pPartsMeshNode->isAlphaMtrlNode() ) {//¾ËÆÄ ¿ÀºêÁ§Æ®´Â µÚ¿¡ ±×¸°´Ù..

			if( RRenderNodeMgr::m_bRenderBuffer ) {

//				RenderNodeMgr_Add( pMeshNode->m_ModelWorldMatrix,m,pMeshNode,0);

				//	º¸°üµÇ¾î¾ß ÇÒ°ªµéÀº?
				//	visual mesh.. ÀÚ½ÅÀÇ °íÀ¯ÇÑ°ªµé..
				//	m_pParentMesh->SetVisualMesh( pVMesh );
				//	m_pParentMesh->RenderNode( pMNode ); //frame °ªµîÀº º¸°üµÇ´Â°¡?
				//	visualmesh ´Â ÀÚ½ÅÀÇ ³ëµå ¸ÅÆ®¸¯½º¸¦ ¸Å ÇÁ·¹ÀÓ °»½Å º¸°ü...
				//	Ä«¸Þ¶ó¿¡¼­ ÃæºÐÈ÷ ¸Õ Ä³¸¯ÅÍ¶ó¸é... 1/2 1/3 1/4 ºóµµ·Î ÇÁ·¹ÀÓ °»½Å... 
				//	ÀÌÀü ¹öÆÛ¸¦ ±×³É »ç¿ëÇÏ±â °¡´É..±×·Á ÁÖ±â¸¸ ÇÑ´Ù..
				//	mat_result º¸°üµÇ¾î ÀÖ¾î¾ß ÇÔ..
				//	light ¼³Á¤°ª º¸°üÇØ¾ßÇÔ.. ( visual mesh °¡ º¸°ü..
				//	align , part_node ¿¡ ´ëÇÑ ¹®Á¦.. vis_alpha °ª...
				
				//	calcVertexBuffer ¿¡¼­... pMesh->Scale ÂüÁ¶ÇÔ... ----- frame °ªÀ» º¸°üÇÏ°íÀÖ¾î¾ß ÇÔ..
				//	vertex_ani + soft_vs ´Â ¹öÅØ½º ¹öÆÛ°¡ º¯°æµÊ...
				//	baseMesh ÀÇ »óÇÏÃ¼ ÇÁ·¹ÀÓ °ªÀ» »ç¿ëÇÔ.... ÇÁ·¹ÀÓ ´Ù½Ã ¼³Á¤ÇØ¾ßÇÔ...»óÇÏÃ¼ ¿¡´Ï¸ÞÀÌ¼Ç ÀÚÃ¼°¡ ¹Ù²î¾î ÀÖÀ»¼öµµ ÀÖÀ½... 

				//  visualmesh->render ¿¡¼­ set ÇÏ´Â°Íµé ´Ù½ÃÇØ¾ß ÀÇ¹Ì°¡ ÀÖÀ½,,,
				//  
			}
			else {
			
				pAlphaNode[nAlphaNodeCnt] = pPartsMeshNode;
				nAlphaNodeCnt++;

				if(nAlphaNodeCnt > _SORT_ALPHA_NODE_TABLE-1) {
					nAlphaNodeCnt = _SORT_ALPHA_NODE_TABLE-1;
					mlog("%s ¿ÀºêÁ§Æ®ÀÇ ¾ËÆÄ ³ëµå°¡ 100°³°¡ ³Ñ´Â´Ù..\n",GetFileName());
					nAlphaNodeCnt--;
				}

			}

			it_obj++;
			continue;

		}

		RenderNode(pPartsMeshNode,world_mat);

		it_obj++;
	}

	RMeshNode* pATMNode = NULL;
	RMeshNode* pTNodeHead = NULL;	
	RMeshNode* pTLastModel = NULL;

//	qsort( pAlphaNode, nAlphaNodeCnt, 4 , _SortAlpha );

	///////////////////////////////////////////////////////////////////

	for(int n=0;n<nAlphaNodeCnt;n++) {

		pATMNode = pAlphaNode[n];

		if( pATMNode == NULL ) continue;

		if(pATMNode->m_PartsType == eq_parts_head ) {
			pTNodeHead = pATMNode;
			continue;
		}

		RenderNode(pATMNode,world_mat);
	}

	if(pTNodeHead) {
		RenderNode(pTNodeHead,world_mat);
	}

	///////////////////////////////////////////////////////////////////

	qsort( pLastNode, nLastNodeCnt, 4 , _SortLastName );

	for(int n=0;n<nLastNodeCnt;n++) {

		pATMNode = pLastNode[n];

		if( pATMNode == NULL ) continue;

		RenderNode(pATMNode,world_mat);
	}


	///////////////////////////////////////////////////////////////////


	static D3DXMATRIX _init_mat = GetIdentityMatrix();

	dev->SetTransform( D3DTS_WORLD, &_init_mat );
	// Custom: after the model has been rendered, disable antialiasing (so nothing else gets antialiased).
	if (RAdvancedGraphics::nMultiSampling > 0)
		dev->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);

	__EP(500);

	__EP(198);
}

// billboard(aline) type ÀÌ¶ó¸é ¸¶Áö¸· È¸Àü ¸ÅÆ®¸¯½º ¼öÁ¤ Á¡ °¡°ø..
 
void OutPutMatrixLog(RMesh* pMesh,RMeshNode* pNode,char* pos,char* name,rmatrix& m)
{
	return;

	if(!pMesh||!pNode) 
		return;

	RAnimation* pAniSet = pMesh->GetNodeAniSet(pNode);

	if(pAniSet==NULL) 
		return;

	if( pos[0] && name[0] ) {

		mlog("%s %s %f,%f,%f,%f %f,%f,%f,%f %f,%f,%f,%f %f,%f,%f,%f \n", pos , name ,
			m._11,m._12,m._13,m._14,
			m._21,m._22,m._23,m._24,
			m._31,m._32,m._33,m._34,
			m._41,m._42,m._43,m._44	);

	} else {

		mlog("%f,%f,%f,%f %f,%f,%f,%f %f,%f,%f,%f %f,%f,%f,%f \n", 
			m._11,m._12,m._13,m._14,
			m._21,m._22,m._23,m._24,
			m._31,m._32,m._33,m._34,
			m._41,m._42,m._43,m._44	);
	}
}

////////////////////////////////////////////////////////////////////////////////////


class RIVec
{
public:

	RIVec() {
		m_size = 0;
	}

	void Add(int v) {

		/*m_Value[m_size] = v;
		m_size++;

		if(m_size>2)
			m_size=2;*/
		if (m_size < 3) {          // chỉ ghi khi còn chỗ
			m_Value[m_size++] = v;
		}
	}

	int m_size;
	int m_Value[3];
};

class CIndexBufferMake
{
public:
	CIndexBufferMake() {

		m_pFaceIndex = NULL;
		m_tAddPos = 0;
	}

	~CIndexBufferMake() {
		Clear();
	}

	void Create(int face_num,int point_num) {//¸éÀÎµ¦½º¸¸Å­¸Þ¸ð¸®ÇÒ´ç..
		
		if(face_num) {

			m_pFaceIndex = new WORD[face_num*3];
			memset(m_pFaceIndex,0,sizeof(WORD)*face_num*3);
		}

		m_tAddPos = point_num;
		
	}

	void Clear() {

		if(m_pFaceIndex)
			delete[] m_pFaceIndex;
	}

	void MakeLVertex(RMeshNode* pMNode,RFaceInfo* pFace,int i,DWORD color)
	{
		int in = 0;

		RFaceNormalInfo* pFNL = &pMNode->m_face_normal_list[i];

		for(int k=0;k<3;k++) {

			int in = GetLIndex( pFace->m_point_index[k] , &pFace->m_point_tex[k] );

//			SetLVertex(((RLVertex*)g_pVert)+in,g_point_list[pFace->m_point_index[k]], color, pFace->m_point_tex[ k ]);

			m_pFaceIndex[i*3+k] = in;
//			mlog("face (%d,%d) = %d \n",i,k,in);
		}
	}

	// n¹ø¤Š ¸é¿¡ ´ëÇØ¼­
	// Áßº¹µÇÁö ¾Ê°Ô 3°³ÀÇ Á¡À» ¸¸µé¾î¾ß ÇÑ´Ù..
	void MakeVertex(RMeshNode* pMNode,RFaceInfo* pFace,int i)
	{
		int in = 0;

		RFaceNormalInfo* pFNL = &pMNode->m_face_normal_list[i];

		for(int k=0;k<3;k++) {

			int in = GetIndex( pFace->m_point_index[k] , &pFace->m_point_tex[k] );

//			SetVertex(g_vert+in,g_point_list[pFace->m_point_index[k]], color, pFace->m_point_tex[ k ]);

//			if( !g_bVertexNormalOnOff ) // tool ¿ë
//				SetVertex( ((RVertex*)g_pVert) +in  ,g_point_list[pFace->m_point_index[k]],pFNL->m_normal,pFace->m_point_tex[k]);
//			else 
//				SetVertex( ((RVertex*)g_pVert) +in  ,g_point_list[pFace->m_point_index[k]],pFNL->m_pointnormal[k],pFace->m_point_tex[k]);
			
			m_pFaceIndex[i*3+k] = in;
//			mlog("face (%d,%d) = %d \n",i,k,in);
		}
	}

	int GetLIndex(int in,rvector* puv) // ½ÇÀçÁ¡ÀÌ ¸¸µé¾îÁú ÀÎµ¦½º ³Ñ±â±â..
	{
		static float _uv[4];

		int _size = m_pPointTable[in].m_size;
//		int index;

		if(_size) { // ÀÌ¹Ì µî·ÏµÈ Á¡ÀÌ¶ó¸é
/*
			RLVertex* pVert=(RLVertex*)g_pVert;

			for(int i=0;i<_size;i++) {
				index = m_pPointTable[in].m_Value[i];
				GetLUV(_uv,&pVert[index],puv);
				if(isEqualUV(_uv)) {//uv°¡ °°À¸¸é °°ÀºÁ¡À» »ç¿ë..
					return index;
				}
			}

			m_pPointTable[in].Add( m_tAddPos );//¸¶Áö¸·Á¡À» »ç¿ë
			m_tAddPos++;
*/
			return m_tAddPos-1;//¸¶Áö¸·Á¡ÀÇ ÀÎµ¦½º..
		}

		m_pPointTable[in].Add( in ); // »õ·Î µî·Ï ÇØ¾ßÇÏ´Â °æ¿ì ±×³É »ç¿ë..

		return in;
	}

	int GetIndex(int in,rvector* puv) // ½ÇÀçÁ¡ÀÌ ¸¸µé¾îÁú ÀÎµ¦½º ³Ñ±â±â..
	{
		static float _uv[4];

		int _size = m_pPointTable[in].m_size;
//		int index;

		if(_size) { // ÀÌ¹Ì µî·ÏµÈ Á¡ÀÌ¶ó¸é
/*
			RVertex* pVert = (RVertex*)g_pVert;

			for(int i=0;i<_size;i++) {
				index = m_pPointTable[in].m_Value[i];
				GetUV(_uv,&pVert[index],puv);
				if(isEqualUV(_uv)) {//uv°¡ °°À¸¸é °°ÀºÁ¡À» »ç¿ë..
					return index;
				}
			}

			m_pPointTable[in].Add( m_tAddPos );//¸¶Áö¸·Á¡À» »ç¿ë
			m_tAddPos++;
*/
			return m_tAddPos-1;//¸¶Áö¸·Á¡ÀÇ ÀÎµ¦½º..
		}

		m_pPointTable[in].Add( in ); // »õ·Î µî·Ï ÇØ¾ßÇÏ´Â °æ¿ì ±×³É »ç¿ë..

		return in;
	}

	void GetLUV(float* uv,RLVertex* pVert,rvector* v) {
		uv[0] = pVert->tu;
		uv[1] = pVert->tv;
		uv[2] = v->x;
		uv[3] = v->y;
	}


	void GetUV(float* uv,RVertex* pVert,rvector* v) {
		uv[0] = pVert->tu;
		uv[1] = pVert->tv;
		uv[2] = v->x;
		uv[3] = v->y;
	}

	bool isEqualUV(float u1,float v1,float u2,float v2) {

		if(u1 == u2 )
			if(v1 == v2 )
				return true;
//		if(u1 - u2 < 0.00001f)
//			if(v1 - v2 < 0.00001f)
//				return true;
		return false;
	}

	bool isEqualUV(float* uv) {
		return isEqualUV(uv[0],uv[1],uv[2],uv[3]);
	}

	RIVec	m_pPointTable[2000];
//	vector<int>	m_pPointTable[2000];//¿ì¼± 2000°³ÀÇ Á¡±îÁö¸¸
	WORD*	m_pFaceIndex;
	
	int		m_tAddPos;

};


void BBoxSubCalc(D3DXVECTOR3* max,D3DXVECTOR3* min);

bool RMesh::CheckOcclusion(RMeshNode *pMeshNode)
{
	if( m_is_map_object && !mbSkyBox)
	{
		if(m_pVisualMesh)
		{
			if(m_pVisualMesh->GetTOCCL())
			{
				rboundingbox bb;

				rmatrix _mm;

				static rmatrix _mrot = RGetRotY(180) * RGetRotX(90);
	
				_mm = pMeshNode->m_mat_base * _mrot;

				bb.vmax = pMeshNode->m_max * _mm;
				bb.vmin = pMeshNode->m_min * _mm;
				
				BBoxSubCalc(&bb.vmax,&bb.vmin);

				if(m_pVisualMesh->IsCheckViewFrustum()) {//tool ÀÌ¶ó¸é false
					if(isInViewFrustumWithZ( &bb, RGetViewFrustum()) == false) {
						return false;
					}
				}

				return m_pVisualMesh->GetTOCCL()->IsVisible(bb);
			}
		}
	}
	return true;
}

void RMesh::RenderNode(RMeshNode *pMeshNode,D3DXMATRIX* world_mat)
{
	if(pMeshNode->m_face_num == 0) return;

	if(m_pVisualMesh)
		if(m_pVisualMesh->IsRenderMatrix())//¸ÅÆ®¸¯½º °»½Å¸¸ ¿øÇÏ´Â °æ¿ì¶ó¸é....
			return;

	if(CheckOcclusion(pMeshNode)==false) // ¸ðµ¨ÀÌ ¾Æ´Ñ ³ëµå ´ÜÀ§ ÄÃ¸µ..
		return;

	__BP(501,"RMesh::RenderNode");

	SetMtrlUvAni_ON();

	bool bDrawCharPhysique = false;

	if( RIsSupportVS() && RShaderMgr::mbUsingShader )
	{
		if((m_pAniSet[0]) && (RAniType_Bone == m_pAniSet[0]->GetAnimationType()) 
			&& mHardwareAccellated && pMeshNode->m_MatrixCount > 0 )
		{
			bDrawCharPhysique = true;
		}
	}

	if(bDrawCharPhysique) {

		pMeshNode->RenderNodeVS(this,world_mat);

	}
//	else if( RRenderNodeMgr::m_bRenderBuffer ) { // ¸ð¾Æ ±×¸®±â..¼³Á¤ÀÌ µÇ¾îÀÖ°í..¾ËÆÄ¶ó¸é?

//		pMeshNode->CalcVertexBuffer(world_mat);
//		RenderNodeMgr_Add( pMeshNode->m_ModelWorldMatrix,pMeshNode,0);
		
//	} 
	else { // Áï½Ã ±×·Á¾ß ÇÑ´Ù¸é - ½ºÅ©¸°ÀÌÆåÆ®µî - ºä³ª ´Ù¸¥°Í°ú ÀÛ¿ëÇÏ´Â °Í...
		
//		RGetDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

		pMeshNode->CalcVertexBuffer(world_mat);
		pMeshNode->Render();
	}

	SetMtrlUvAni_OFF();

	__EP(501);

}

////////////////////////////////////////////////////////////////////

// find face

bool find_intersects_triangle_sub(rvector& orig,rvector& dir, rvector& v0,rvector& v1, rvector& v2, float* t, float* u, float* v )
{
	rvector edge1 = v1 - v0;
	rvector edge2 = v2 - v0;

	rvector pvec;
	D3DXVec3Cross( &pvec, &dir, &edge2 );

	FLOAT det = D3DXVec3Dot( &edge1, &pvec );
	if( det < 0.0001f )
		return false;

	rvector tvec = orig - v0;

	*u = D3DXVec3Dot( &tvec, &pvec );

	if( *u < 0.0f || *u > det )
		return false;

	rvector qvec;
	D3DXVec3Cross( &qvec, &tvec, &edge1 );

	*v = D3DXVec3Dot( &dir, &qvec );

	if( *v < 0.0f || *u + *v > det )
		return false;

	*t = D3DXVec3Dot( &edge2, &qvec );

	FLOAT fInvDet = 1.0f / det;

	*t *= fInvDet;
	*u *= fInvDet;
	*v *= fInvDet;

	return true;
}

inline bool find_intersects_triangle_sub(rvector* vec, rvector* vPoint, float* t, float* u, float* v ) {
	return find_intersects_triangle_sub(vec[0],vec[1], vPoint[0],vPoint[1], vPoint[2],t,u,v );
}

bool RMesh::CalcIntersectsTriangle(rvector* vInVec, RPickInfo* pInfo, D3DXMATRIX* world_mat,bool fastmode)
{
	//////////////////////////////////////////////////

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	D3DXVECTOR3 _v;

	float best_t = 9999.f;

	RMeshNode* pFindMeshNode = NULL;
	RMeshNode* pPartsMeshNode = NULL;

	rvector		vFindVec[3];
	
	bool  bFind = false;

	rmatrix result_mat;

	while (it_obj !=  m_list.end())	{

		RMeshNode*	pMeshNode = (*it_obj);

		pPartsMeshNode = UpdateNodeAniMatrix(pMeshNode);

		// Ãæµ¹¿ë ³ëµå¸¸ »ç¿ë
		if( m_PickingType == pick_collision_mesh ) {
			if( !pPartsMeshNode->m_isCollisionMesh ) {
				it_obj++;
				continue;
			}
		}
		else if( m_PickingType == pick_real_mesh ) {
			if( pPartsMeshNode->m_isDummyMesh ) {//Bip,Bone,Dummy Skip
				it_obj++;
				continue;
			}
		}

		if(pPartsMeshNode->m_face_num==0) {
			it_obj++;
			continue;
		}

		static D3DXVECTOR3 pVecPick[10000];// ÀÛµ¿ÇÏ¸é ´Ù½Ã Ã³¸®..

//		pPartsMeshNode->CalcVertexBuffer(world_mat,true);//world mat °öÇÑ vertex

		pPartsMeshNode->CalcPickVertexBuffer(world_mat,pVecPick);

		float t,u,v;
		rvector vec[3];//¿ì¼± ÁöÀú ºÐÇØ º¸ÀÌ´Ï±î Ä«ÇÇ

		for (int i = 0; i < pPartsMeshNode->m_face_num  ; i ++) {

			vec[0] = pVecPick[ pPartsMeshNode->m_face_list[i].m_point_index[0] ];
			vec[1] = pVecPick[ pPartsMeshNode->m_face_list[i].m_point_index[1] ];
			vec[2] = pVecPick[ pPartsMeshNode->m_face_list[i].m_point_index[2] ];
			
			// pos , dir , vec
			if( find_intersects_triangle_sub( vInVec , vec, &t, &u, &v ) ) {

				if(t < best_t) {
					best_t = t;
					pFindMeshNode = pPartsMeshNode;
					memcpy(vFindVec,vec,sizeof(rvector)*3);
				}

				if(fastmode) {	// °¡Àå ¸ÕÀúÃ£Àº°Í
					if(pInfo) {
						pInfo->vOut	= vec[0] + u * ( vec[1] - vec[0] ) + v * ( vec[2] - vec[0] );
						pInfo->t = best_t;
					}
					return true;
				}

				bFind = true;
			}
		}
		it_obj++;
	}

	if( bFind && pFindMeshNode ) {

		if(pInfo) {

			D3DXPLANE pl;
			D3DXPlaneFromPoints(&pl,&vFindVec[0],&vFindVec[1],&vFindVec[2]);

			rvector p,at;

			at  = vInVec[0];
			at += vInVec[1] * 10000.f;//´ëÃæ ÃæºÐÈ÷ ¸Õ °Å¸®..

			D3DXPlaneIntersectLine(&p,&pl,&vInVec[0],&at);

			pInfo->vOut	 = p;
			pInfo->t	 = best_t;
			pInfo->parts = pFindMeshNode->m_PartsType;
		}
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

static RRenderNodeMgr g_render_node_mgr;

// ÀÚ±â ÀÚ·á·Î ±×¸®±â..
void RRenderNode::Render()
{
	if(m_pNode && m_pNode->m_pParentMesh) {
		m_pNode->Render(&m_matWorld);
	}
}

void RRenderNodeMgr::Clear() 
{
	for(int i=0;i<eRRenderNode_End;i++) {
		m_RenderNodeList[i].Clear();
	}

	m_nTotalCount = 0;
}

int RRenderNodeMgr::Add(rmatrix& m,int mode,RMeshNode* pMNode,int nMtrl)
{
	bool lit = false;

	if(pMNode==NULL)
		return m_nTotalCount;

	RRenderNode* pNode = new RRenderNode;

	pNode->Set(mode,m,pMNode,nMtrl,0,0,1.f);

	m_RenderNodeList[mode].push_back( pNode );

#ifdef _DEBUG
	int _size = m_RenderNodeList[mode].size();
	m_RenderNodeList[mode].m_data[_size-1] = pNode;
#endif

	m_nTotalCount++;

	return m_nTotalCount;
}

void RRenderNodeMgr::Render()
{
	// sort

	// normal	- name
	// diffuse	- name
	// alpha	- name
	// add		- dist

	// light on
	
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	for(int i=0;i<eRRenderNode_End;i++) {
		m_RenderNodeList[i].Render();
	}

	// light off

	Clear();
}

void RenderNodeMgr_Add(rmatrix& m,int mode,RMeshNode* pMNode,int nMtrl)
{
	if(RRenderNodeMgr::m_bRenderBuffer==false) 
		return;

	g_render_node_mgr.Add(m,mode,pMNode,nMtrl);
}

void RenderNodeMgr_Render()
{
	if(RRenderNodeMgr::m_bRenderBuffer==false) 
		return;

	g_render_node_mgr.Render();
}

// temp --------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////
// ¸ð¾ÆÂï±â¿ë..Å×½ºÆ® ÄÚµå

void RMeshRenderS(bool lit,int Rmode,rmatrix m,RMeshNode* pMNode,RMtrl* pMtrl,int begin,int size ,float vis_alpha);
bool RMeshRenderSBegin();
bool RMeshRenderSEnd();

RRenderNode* GetNewRenderNode() {
	if(g_render_cnt > RENDER_NODE_MAX-1) {
//		mlog("g_render_buffer ¸¦ ´Ã·Á¶ó\n");
		return NULL;
	}
	g_render_node[g_render_cnt].Clear();
	g_render_cnt++;
	return &g_render_node[g_render_cnt-1];
}

void ClearRenderNodeBuffer() {
	g_render_cnt = 0;
}

void RMeshRenderS(bool lit,int Rmode,rmatrix m,RMeshNode* pMNode,RMtrl* pMtrl,int begin,int size ,float vis_alpha)
{
	RRenderNode* pRNode = GetNewRenderNode();

	if(pRNode==NULL) return;

	pRNode->Set(Rmode,m,pMNode,pMtrl,begin,size,vis_alpha);

	if(lit) {
		g_RenderLNodeList[Rmode].push_back(pRNode);
	}
	else {
		g_RenderNodeList[Rmode].push_back(pRNode);
	}
}

bool RMeshRenderSBegin()
{
	if(g_rmesh_render_start_begin) {
		mlog("RMeshRenderSEnd() À» ¸ÕÀú ÇÑ ÈÄ »ç¿ë\n");
		return false;
	}

	g_rmesh_render_start_begin = true;

	ClearRenderNodeBuffer();

	g_vert_index_pos = 0;
	g_lvert_index_pos = 0;

	return true;
}

//void SetMtrl(RMtrl* pMtrl,float vis_alpha);

bool RMeshRenderSEnd()
{
/*
	if(!g_rmesh_render_start_begin) {
		mlog("RMeshRenderSBegin() À» ¸ÕÀú ÇÑ ÈÄ »ç¿ë\n");
		return false;
	}

	g_rmesh_render_start_begin = false;

	// buffer render..

	RRenderNodeList::iterator node;
	RRenderNode* pRNode = NULL;
	RMtrl* pBackupMtrl = NULL;

	// sort
//	for(int i=0;i<eRRenderNode_End;i++) {
//		g_RenderNodeList[i].sort();
//		g_RenderLNodeList[i].sort();
//	}

	LPDIRECT3DDEVICE9 dev = RGetDevice();

//	dev->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
	dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
//	dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	RSetWBuffer(true);
	dev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );

	for(int i=0;i<eRRenderNode_End;i++) {

		dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		dev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		dev->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

		if(i == eRRenderNode_Normal) {
			dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE);
			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
			dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
			dev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE);

		}
		else if(i == eRRenderNode_Alpha) {

			dev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
			dev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
			dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
			dev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE);

			dev->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
			dev->SetRenderState( D3DRS_ALPHAREF, 0x04 );
			dev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );//vis ani ignore
			dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );//light 
		}
		else if(i == eRRenderNode_Add) {

			dev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
			dev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
			dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);
			dev->SetRenderState( D3DRS_ZWRITEENABLE, FALSE);

			dev->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
			dev->SetRenderState( D3DRS_ALPHAREF,         0x04 );
			dev->SetRenderState( D3DRS_ALPHAFUNC,  D3DCMP_GREATEREQUAL );

			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );//light ignore
		//	dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
			dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

		}
		else if(i == eRRenderNode_Diffuse) {

//			SetMtrl(&pSMtrl->m_diffuse,GetMeshNodeVis(pMeshNode));

//			dev->SetTexture( 0, NULL);

//			if(GetMeshNodeVis(pMeshNode) != 1.f) {
//				dev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
//				dev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
//				dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
//			}
//			else {
//				dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
//			}

//			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
//			dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );

//			dev->SetFVF( RVertexType );
//			dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, t_face_cnt , (LPVOID) g_vert, sizeof(RVertex));

//			g_poly_render_cnt += t_face_cnt;//<-----

		//	SetMtrl(pSMtrl,GetMeshNodeVis(pMeshNode));

//			dev->SetRenderState( D3DRS_ALPHABLENDENABLE,   FALSE );
			dev->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
			dev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
			dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
			dev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE);

			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_BLENDTEXTUREALPHA );
//			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
			dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );//vis ignore
		}

		dev->SetRenderState( D3DRS_LIGHTING, TRUE );
		dev->SetFVF( RVertexType );

		for(node = g_RenderNodeList[i].begin(); node != g_RenderNodeList[i].end(); ) {
			pRNode = (*node);

			if(pBackupMtrl != pRNode->m_pMtrl) {//ÃÖ´ëÇÑ mtrl ÀÌ °°µµ·Ï Á¤·ÄÇÑ´Ù..
				pBackupMtrl = pRNode->m_pMtrl;

//				if(pBackupMtrl->m_bTwoSided) {
//					dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
//				}

				SetMtrl(pBackupMtrl,pRNode->m_vis_alpha);

				if(g_bTextureRenderOnOff && pBackupMtrl) // debug func
					dev->SetTexture( 0, pBackupMtrl->GetTexture());
				else
					dev->SetTexture( 0, NULL);
			}
			dev->SetTransform(D3DTS_WORLD, &pRNode->m_matWorld);
//			dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, pRNode->m_size, (LPVOID) (g_vert_s + pRNode->m_begin) , sizeof(RVertex));
			dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, pRNode->m_size, (LPVOID) &g_vert_s[pRNode->m_begin] , sizeof(RVertex));

			g_poly_render_cnt += pRNode->m_size;
			++node;
		}

		dev->SetRenderState( D3DRS_LIGHTING, FALSE );
		dev->SetFVF( RLVertexType );

		for(node = g_RenderLNodeList[i].begin(); node != g_RenderLNodeList[i].end(); ) {
			pRNode = (*node);

			if(pBackupMtrl != pRNode->m_pMtrl) {
				pBackupMtrl = pRNode->m_pMtrl;

//				if(pBackupMtrl->m_bTwoSided) {
//					dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
//				}

//				SetMtrl(pBackupMtrl,);

				if(g_bTextureRenderOnOff && pBackupMtrl) // debug func
					dev->SetTexture( 0, pBackupMtrl->GetTexture());
				else
					dev->SetTexture( 0, NULL);
			}

			dev->SetTransform(D3DTS_WORLD, &pRNode->m_matWorld);
			dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, pRNode->m_size , (LPVOID) &g_lvert_s[pRNode->m_begin] , sizeof(RLVertex));
			g_poly_render_cnt += pRNode->m_size;
			++node;
		}
	}

	for(int i=0;i<eRRenderNode_End;i++) {
		g_RenderNodeList[i].clear();
		g_RenderLNodeList[i].clear();
	}
*/
	return true;
}

_NAMESPACE_REALSPACE2_END