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

#ifndef _PUBLISH

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN

//////////////////////////////////////////////////////////////////////////////////
// RMeshNodeInfo Class

#define RVERTEX_MAX		1024*100*32			// 34133 face

///////////////////////////////////////////////////////////////////////////////////

RMeshNodeInfo::RMeshNodeInfo()
{
	m_isAddMeshNode = false;
	m_isDummy		= false;
	m_isDummyMesh	= false;
	m_isWeaponMesh	= false;
	m_isAlphaMtrl	= false;
	m_isCollisionMesh = false;
	m_isLastModel	= false;
	m_isClothMeshNode	= false;

	m_vis_alpha = 1.f;
	m_bClothMeshNodeSkip  = true;
	
	m_CutPartsType	  = cut_parts_upper_body; //»óÃ¼¾Æ´Ï¸é ÇÏÃ¼..
	m_LookAtParts	  = lookat_parts_etc;
	m_WeaponDummyType = weapon_dummy_etc;

	m_PartsPosInfoType = eq_parts_pos_info_etc;
	m_PartsType = eq_parts_etc;

	m_nAlign = 0;

	m_AlphaSortValue = 0.f;
	m_bNpcWeaponMeshNode = false;
}

RMeshNodeInfo::~RMeshNodeInfo()
{

}

///////////////////////////////////////////////////////////////////////////////////

RBatch::RBatch()
{
	for(int i=0;i<MAX_PRIMITIVE;i++) {

		m_ib[i] = NULL;
	}

	m_bIsMakeVertexBuffer = false;

	m_vsb = NULL;
	m_vb = NULL;

}

RBatch::~RBatch()
{
	for(int i=0;i<MAX_PRIMITIVE;i++) {
		DEL(m_ib[i]);
	}

	m_bIsMakeVertexBuffer = false;

	DEL(m_vb);
	DEL(m_vsb);
}

bool RBatch::CreateVertexBuffer(char* pVert,DWORD fvf,int vertexsize,int vert_num,DWORD flag)
{
	if(m_vb==NULL) {
		m_vb = new RVertexBuffer;
		m_vb->Create(pVert,fvf,vertexsize,vert_num,flag);
	}
	return true;
}

bool RBatch::UpdateVertexBuffer(char* pVert)
{
	if(m_vb==NULL) return false;

	m_vb->UpdateData(pVert);

	return true;
}

bool RBatch::UpdateVertexBufferSoft(char* pVert)
{
	if(m_vb==NULL) return false;

	m_vb->UpdateDataSW(pVert);

	return true;
}

bool RBatch::CreateIndexBuffer(int index,WORD* pIndex,int _size)
{
	if(m_ib[index]==NULL) {
		m_ib[index] = new RIndexBuffer;
		m_ib[index]->Create(_size,pIndex);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////////////
// RMeshNode Class

RMeshNode::RMeshNode()
{
	m_pParentMesh = NULL;
	m_pBaseMesh = NULL;

	m_Next = NULL;
	m_ChildRoot = NULL;

	m_pAnimationNode = NULL;

	m_Parent[0] = NULL;
	m_nParentNodeID = -1;

	m_pParent = NULL;

	int i = 0;

	m_MatrixCount = 0;

	for(i=0;i<MAX_MATRIX;i++) {
		m_MatrixMap[i] = 0;
	}

	m_BoneBaseMatrix = NULL;
	m_nBoneBaseMatrixCnt = 0;

	m_bConnectPhysique = false;
}

RMeshNode::~RMeshNode()
{

	DEL2(m_BoneBaseMatrix);

	m_nBoneBaseMatrixCnt = 0;

}

///////////////////////////////////////////////////////////

bool RMeshNode::ConnectMtrl()
{
	if(m_face_num==0)//¸éÀÌ ¾ø´Ù¸é ¿¬°áÇÒ ÇÊ¿äµµ ¾ø´Ù..
	{
//		mlog( "%s MeshNode mtrl ¿¬°áÇÒ ÇÊ¿ä ¾øÀ½\n" , m_Name );
		return false;
	}

	RMtrlMgr* pMtrlList = NULL;

	if(m_pParentMesh) {
		pMtrlList = &m_pParentMesh->m_mtrl_list_ex;

		RMtrl* pMtrl = NULL;

		if(pMtrlList) {
			pMtrl = pMtrlList->Get_s(m_mtrl_id,-1);

			if(pMtrl) {

				int mtrl_cnt = 1;

				if( pMtrl->m_sub_mtrl_num ) {// submtrl
					mtrl_cnt = pMtrl->m_sub_mtrl_num;

					m_pMtrlTable = new RMtrl*[mtrl_cnt];

					for (int s = 0; s < mtrl_cnt ; s ++) {
						m_pMtrlTable[s] = pMtrlList->Get_s(m_mtrl_id,s);
					}
				}
				else 
				{
					m_pMtrlTable = new RMtrl*[1];
					m_pMtrlTable[0] = pMtrl;
				}

				m_nMtrlCnt = mtrl_cnt;
				if(m_nMtrlCnt > MAX_PRIMITIVE)
				{
					mlog( "%s multi-sub material 이 10개를 넘을 수 없습니다\n" , m_Name.c_str() );
					return false;
				}
				return true;
			}
		}
	}

	mlog( "%s MeshNode mtrl ¿¬°á½ÇÆÐ\n" , m_Name.c_str() );

	return false;
}

#define MAX_BONE			130
#define MAX_MATRIX_LIMIT	64
#define MAX_VERTEX			1024*64


#define SetBVert(vt,_pos,_normal,u,v,w1,w2)				\
	vt.p=_pos;											\
	vt.matIndex[0]=0;									\
	vt.matIndex[1]=0;									\
	vt.matIndex[2]=0;									\
	vt.normal = _normal;								\
	vt.tu = u;											\
	vt.tv = v;											\
	vt.weight1 = w1;									\
	vt.weight2 = w2;									\


bool RMeshNode::SetBVertData(RBlendVertex* pBVert,int i,int j,int pv_index,int* DifferenceMap,int& matrixIndex)
{
	int point_index = m_face_list[i].m_point_index[j];
	RPhysiqueInfo* pPhysique = &m_physique[point_index];

	int point_pos = 3*i+j;

	float w1,w2;

	if( point_index < 0 || point_index >= m_point_num ) {
		mlog("Index of Vertex(Pointer) is Out of Range.. Point Index : %d, Num Vertices : %d, Mesh Node : %s \n", point_index, m_point_num, m_Name );
		return false;
	}

	/*if( pPhysique->m_num > 3 || pPhysique->m_num <= 0 ) {
		mlog("%s mesh %s node %d face %d point -> physique 3 Greater than\n",m_pParentMesh->GetFileName() ,m_Name.c_str(),i,j);
		return false;
	}*/
	if (pPhysique->m_num > 3)
	{
		float sum = pPhysique->m_weight[0] + pPhysique->m_weight[1] + pPhysique->m_weight[2];
		for (int k = 0; k < 3; ++k)
			pPhysique->m_weight[k] /= sum;
		pPhysique->m_num = 3;
	}


	if( 1 == pPhysique->m_num ) {
		w1 = 1;
		w2 = 0;
	}
	else {
		w1 = pPhysique->m_weight[0];
		w2 = pPhysique->m_weight[1];
	}

	SetBVert(pBVert[point_pos],
		m_point_list[point_index],
		m_face_normal_list[i].m_pointnormal[j],
		m_face_list[i].m_point_tex[j].x,
		m_face_list[i].m_point_tex[j].y, 
		w1,w2);

	int index;

	for( int k = 0 ; k < pPhysique->m_num; ++k )
	{
		index = pPhysique->m_parent_id[k]; // ÇöÀç ÂüÁ¶ÇÏ°í ÀÖ´Â boneÀÇ index

		_ASSERT(0 <= index && index < MAX_BONE);

		if( DifferenceMap[index] == -1 )
		{
			DifferenceMap[index] = 3 * matrixIndex + ANIMATION_MATRIX_BASE;
			m_MatrixMap[matrixIndex++] = index;
		}

		pBVert[pv_index].matIndex[k] = DifferenceMap[index];
	}

	return true;
}

void RMeshNode::MakeVertexBuffer(int index,bool lvert,char* pBuf,int _vsize,DWORD flag)
{
	DWORD fvf = 0;
	int vertsize = 0;

	if(lvert) {
		fvf = RLVertexType;
		vertsize = sizeof(RLVertex);
	}
	else {
		fvf = RVertexType;
		vertsize = sizeof(RVertex);
	}

	CreateVertexBuffer(pBuf,fvf,vertsize,_vsize,flag);

	m_bIsMakeVertexBuffer = true;
}

bool RMeshNode::MakeVSVertexBuffer()
{
	_ASSERT( m_physique );
	_ASSERT( m_pParentMesh );
	_ASSERT( (m_face_num * 3 ) < MAX_VERTEX );
	_ASSERT( RIsSupportVS() );
	_ASSERT( m_physique_num == m_point_num );

	if( (m_face_num * 3 ) > MAX_VERTEX ) {
		mlog("Point Number is larger than defined max vertex .. \n");
		return false;
	}

	if( !RIsSupportVS() )	return false;

	int i;

	LPDIRECT3DDEVICE9 dev =	RGetDevice(); // Get Device Pointer

	int numMatrices = 0;				// ¾²ÀÌ´Â ¸ÅÆ®¸¯½ºÀÇ °¹¼ö
	int matrixIndex = 0;				// ½¦ÀÌ´õÀÇ »ó¼ö·¹Áö½ºÅÍÀÇ ÀÎµ¦½º

	int DifferenceMap[MAX_BONE];		// [ boneÀÇ index ] => [ matrix palletteÀÇ index ]

	memset( DifferenceMap , -1, sizeof(int)*MAX_BONE );

	static RBlendVertex pBVert[MAX_VERTEX];

	for( i = 0 ; i < m_face_num ; ++i )	{

		for( int j = 0 ; j < 3; ++j ) {

			if(!SetBVertData(pBVert,i,j, 3*i+j,DifferenceMap,matrixIndex)) 
				mlog("Loi tai SetBVertData cho model %s o mat %d!\n", m_Name.c_str(), i);
				return false;
		}
	}

	if(m_vsb==NULL) {
		m_vsb = new RVertexBuffer;
		m_vsb->Create((char*)pBVert,RBLENDVERTEXTYPE,sizeof(RBlendVertex),m_face_num * 3,USE_VERTEX_HW);
	}

	if( MAX_MATRIX_LIMIT < matrixIndex )
	{
		m_MatrixCount = -1;
		return false;
	}

	m_MatrixCount = matrixIndex;

	return true;
}

void RMeshNode::RenderNodeVS(RMesh* pMesh,D3DXMATRIX* pWorldMat_,ESHADER shader_ )
{
	int i;

	__BP(5009,"RMesh::RenderNodeVS");

	LPDIRECT3DDEVICE9 dev = RGetDevice();

	// Ã³À½ ·Î±×ÀÎ È­¸é¿¡¼­ Ä³¸¯ÅÍ ¾Èº¸ÀÌ´Â Çö»ó

	rmatrix matTemp;
	rmatrix world;
	rmatrix view;
	rmatrix proj;
	rmatrix transformation;
	rmatrix tworldmat;

	if(RGetShaderMgr()->mbUsingShader )
	{
		RGetShaderMgr()->mpMtrl->m_diffuse.a = min(m_vis_alpha,pMesh->m_fVis);
	}

	__BP(5010,"RMesh::RenderNodeVS_SetVertexShaderConstant");

	//Register Matrix
	dev->SetVertexShaderConstantF( CAMERA_POSITION, RCameraPosition, 1 );

	dev->GetTransform( D3DTS_WORLD, &world );
	dev->GetTransform( D3DTS_VIEW, &view );
	dev->GetTransform( D3DTS_PROJECTION, &proj );

	D3DXMatrixIdentity( &matTemp );

	dev->SetVertexShaderConstantF( 0, (float*)&matTemp, 3 );

	for( i = 0 ; i < m_MatrixCount; ++i )
	{
		matTemp =  m_mat_ref * pMesh->m_data[ m_MatrixMap[i]]->m_mat_ref_inv * pMesh->m_data[ m_MatrixMap[i]]->m_mat_result;

		D3DXMatrixTranspose( &matTemp, &matTemp );
		dev->SetVertexShaderConstantF( ANIMATION_MATRIX_BASE + (i)*3, (float*)&matTemp, 3);
	}

	if( pMesh->m_isScale ) {
		rmatrix _scale_mat;
		D3DXMatrixScaling(&_scale_mat,pMesh->m_vScale.x,pMesh->m_vScale.y,pMesh->m_vScale.z);
		tworldmat = _scale_mat**pWorldMat_;
	}
	else 
		tworldmat = *pWorldMat_;

	// set Transformation matrix
	D3DXMatrixTranspose( &matTemp, &tworldmat );
	dev->SetVertexShaderConstantF( WORLD_MATRIX, (float*)&matTemp, 4 );

	transformation = view * proj;
	D3DXMatrixTranspose( &matTemp, &transformation );
	dev->SetVertexShaderConstantF( VIEW_PROJECTION_MATRIX, (float*)&matTemp, 4 );

	RMtrl *pMtrl =  m_pMtrlTable[0];
	int num_mtrl =  m_nMtrlCnt;

	__EP(5010);

	__BP(5011,"RMesh::RenderNodeVS_SetCharacterMtrl_ON");

	if( shader_ == SHADER_SKIN_SPEC )
	{
		BOOL b;
		RGetDevice()->GetLightEnable(1, &b);
		if( b )
		{
			dev->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
			dev->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

			dev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MULTIPLYADD );
			dev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			dev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			dev->SetTextureStageState( 1, D3DTSS_COLORARG0, D3DTA_CURRENT );
		}
		else
		{
			dev->SetTexture( 1, NULL );
			dev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
		}
	}
	else
	{
		dev->SetTexture( 1, NULL );
		dev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	}

	__EP(5011);

	//material
	RMtrl* psMtrl;

	
	dev->SetVertexDeclaration( RGetShaderMgr()->getShaderDecl(0) );
	dev->SetVertexShader( RGetShaderMgr()->getShader(shader_) );

	m_vsb->SetVSVertexBuffer();

	for( i = 0 ; i <  m_nMtrlCnt  ; ++i )
	{
		__BP(5012,"RMesh::SetCharacterMtrl_ON");

		psMtrl =  GetMtrl(i);
		pMesh->SetCharacterMtrl_ON( psMtrl,this, min( m_vis_alpha,pMesh->m_fVis),  GetTColor());
			
		__EP(5012);

		__BP(5013,"RMesh::RenderNodeVS_Update");

		RGetShaderMgr()->Update();

		__EP(5013);

		__BP(5014,"RMesh::RenderNodeVS_DrawPrimitive");

		m_vsb->RenderIndexBuffer(m_ib[i]);

		__EP(5014);

		pMesh->SetCharacterMtrl_OFF( psMtrl, min( m_vis_alpha,pMesh->m_fVis) );
	}

	__EP(5009);

#ifdef USE_TOON_RENDER
	if (RMesh::m_bSilhouette && m_pParentMesh && (RMesh::m_fSilhouetteLength > 0.f))
	{
		static bool bLoggedOnce = false;
		if (!bLoggedOnce) {
			mlog("RenderNodeVS: outline block entered, ib count=%d, vsb=%p\n", m_nMtrlCnt, m_vsb);
			bLoggedOnce = true;
		}

		static bool bLogVB = false;
		if (!bLogVB) { mlog("Outline: m_vb=%p after CalcVertexBuffer\n", m_vb); bLogVB = true; }

		ToonRenderSilhouetteSettingOn();

		dev->SetVertexDeclaration(RGetShaderMgr()->getShaderDecl(0));
		dev->SetPixelShader(NULL);
		m_vsb->SetVSVertexBuffer();

		for (i = 0; i < m_nMtrlCnt; ++i)
		{
			if (m_ib[i] == NULL) continue;

			dev->SetVertexShader(RGetShaderMgr()->getShader(SHADER_OUTLINE));

			float fSil[4] = { RMesh::m_fSilhouetteLength, 0, 0, 0 };
			dev->SetVertexShaderConstantF(97, fSil, 1);

			m_vsb->RenderIndexBuffer(m_ib[i]);
		}

		ToonRenderSilhouetteSettingOff();
	}
#endif

	dev->SetVertexShader(NULL);
}

void RMeshNode::RenderOutlineVS(RMesh* pMesh, D3DXMATRIX* pWorldMat_)
{
	RenderOutlineVS(pMesh, pWorldMat_, -1.0f);
}

void RMeshNode::RenderOutlineVS(RMesh* pMesh, D3DXMATRIX* pWorldMat_, float fThicknessOverride)
{
	if (!pMesh || !pWorldMat_) return;
	if (!pMesh->GetVisualMesh()) return;
	if (m_face_num <= 0) return;

	const DWORD dwColor = pMesh->GetVisualMesh()->GetOutlineColor();
	float fAlpha = pMesh->GetMeshVis();
	if (fAlpha < 0.0f) fAlpha = 0.0f;
	if (fAlpha > 1.0f) fAlpha = 1.0f;
	DWORD dwAlpha = (DWORD)(((dwColor >> 24) & 0xff) * fAlpha);
	DWORD dwOutlineColor = (dwColor & 0x00ffffff) | (dwAlpha << 24);
	const float fThickness = (fThicknessOverride >= 0.0f) ? fThicknessOverride : pMesh->GetVisualMesh()->GetOutlineThickness();

	if (!m_vsb || m_MatrixCount <= 0 || !RGetShaderMgr()->hasShader(SHADER_OUTLINE))
	{
		CalcVertexBuffer(pWorldMat_);

		LPDIRECT3DDEVICE9 dev = RGetDevice();
		if (!dev) return;

		rmatrix oldWorld;
		DWORD oldFVF = 0;
		DWORD oldLighting = TRUE;
		DWORD oldCullMode = D3DCULL_NONE;
		DWORD oldSpecular = FALSE;
		DWORD oldFillMode = D3DFILL_SOLID;
		DWORD oldTextureFactor = 0;
		DWORD oldColorOp0 = D3DTOP_MODULATE;
		DWORD oldColorArg10 = D3DTA_TEXTURE;
		DWORD oldAlphaOp0 = D3DTOP_MODULATE;
		DWORD oldAlphaArg10 = D3DTA_TEXTURE;
		DWORD oldColorOp1 = D3DTOP_DISABLE;
		DWORD oldAlphaOp1 = D3DTOP_DISABLE;
		LPDIRECT3DVERTEXDECLARATION9 oldDecl = NULL;
		LPDIRECT3DVERTEXSHADER9 oldVS = NULL;

		dev->GetTransform(D3DTS_WORLD, &oldWorld);
		dev->GetFVF(&oldFVF);
		dev->GetRenderState(D3DRS_LIGHTING, &oldLighting);
		dev->GetRenderState(D3DRS_CULLMODE, &oldCullMode);
		dev->GetRenderState(D3DRS_SPECULARENABLE, &oldSpecular);
		dev->GetRenderState(D3DRS_FILLMODE, &oldFillMode);
		dev->GetRenderState(D3DRS_TEXTUREFACTOR, &oldTextureFactor);
		dev->GetTextureStageState(0, D3DTSS_COLOROP, &oldColorOp0);
		dev->GetTextureStageState(0, D3DTSS_COLORARG1, &oldColorArg10);
		dev->GetTextureStageState(0, D3DTSS_ALPHAOP, &oldAlphaOp0);
		dev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &oldAlphaArg10);
		dev->GetTextureStageState(1, D3DTSS_COLOROP, &oldColorOp1);
		dev->GetTextureStageState(1, D3DTSS_ALPHAOP, &oldAlphaOp1);
		dev->GetVertexDeclaration(&oldDecl);
		dev->GetVertexShader(&oldVS);

		dev->SetTransform(D3DTS_WORLD, &m_ModelWorldMatrix);
		dev->SetVertexShader(NULL);
		dev->SetVertexDeclaration(NULL);
		dev->SetFVF(RVertexType);
		dev->SetRenderState(D3DRS_LIGHTING, FALSE);
		dev->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
		dev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
		dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		dev->SetRenderState(D3DRS_TEXTUREFACTOR, dwOutlineColor);

		std::vector<RVertex> baseVertices(m_face_num * 3);
		for (int i = 0; i < m_face_num; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				const int nIndex = i * 3 + j;
				const int nPointIndex = m_face_list[i].m_point_index[j];
				baseVertices[nIndex].p = m_point_list[nPointIndex];
				baseVertices[nIndex].n = m_face_normal_list ? m_face_normal_list[i].m_pointnormal[j] : D3DXVECTOR3(0.0f, 1.0f, 0.0f);
				baseVertices[nIndex].tu = m_face_list[i].m_point_tex[j].x;
				baseVertices[nIndex].tv = m_face_list[i].m_point_tex[j].y;
			}
		}

		const float fAbsThickness = (fThickness >= 0.0f) ? fThickness : -fThickness;
		if (fAbsThickness <= 0.0001f)
		{
			dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, m_face_num, &baseVertices[0], sizeof(RVertex));
		}
		else
		{
			std::vector<RVertex> shellVertices(baseVertices);

			for (int i = 0; i < m_face_num * 3; ++i)
			{
				shellVertices[i].p += shellVertices[i].n * fAbsThickness;
			}
			dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, m_face_num, &shellVertices[0], sizeof(RVertex));

			shellVertices = baseVertices;
			for (int i = 0; i < m_face_num * 3; ++i)
			{
				shellVertices[i].p -= shellVertices[i].n * fAbsThickness;
			}
			dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, m_face_num, &shellVertices[0], sizeof(RVertex));
		}

		dev->SetTransform(D3DTS_WORLD, &oldWorld);
		dev->SetRenderState(D3DRS_LIGHTING, oldLighting);
		dev->SetRenderState(D3DRS_CULLMODE, oldCullMode);
		dev->SetRenderState(D3DRS_SPECULARENABLE, oldSpecular);
		dev->SetRenderState(D3DRS_FILLMODE, oldFillMode);
		dev->SetRenderState(D3DRS_TEXTUREFACTOR, oldTextureFactor);
		dev->SetTextureStageState(0, D3DTSS_COLOROP, oldColorOp0);
		dev->SetTextureStageState(0, D3DTSS_COLORARG1, oldColorArg10);
		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, oldAlphaOp0);
		dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, oldAlphaArg10);
		dev->SetTextureStageState(1, D3DTSS_COLOROP, oldColorOp1);
		dev->SetTextureStageState(1, D3DTSS_ALPHAOP, oldAlphaOp1);
		dev->SetFVF(oldFVF);
		dev->SetVertexDeclaration(oldDecl);
		dev->SetVertexShader(oldVS);

		SAFE_RELEASE(oldDecl);
		SAFE_RELEASE(oldVS);
		return;
	}

	LPDIRECT3DDEVICE9 dev = RGetDevice();
	rmatrix matTemp;
	rmatrix view;
	rmatrix proj;
	rmatrix transformation;
	rmatrix tworldmat;

	dev->GetTransform(D3DTS_VIEW, &view);
	dev->GetTransform(D3DTS_PROJECTION, &proj);

	D3DXMatrixIdentity(&matTemp);
	dev->SetVertexShaderConstantF(0, (float*)&matTemp, 3);

	for (int i = 0; i < m_MatrixCount; ++i)
	{
		matTemp = m_mat_ref * pMesh->m_data[m_MatrixMap[i]]->m_mat_ref_inv * pMesh->m_data[m_MatrixMap[i]]->m_mat_result;
		D3DXMatrixTranspose(&matTemp, &matTemp);
		dev->SetVertexShaderConstantF(ANIMATION_MATRIX_BASE + (i) * 3, (float*)&matTemp, 3);
	}

	if (pMesh->m_isScale)
	{
		rmatrix _scale_mat;
		D3DXMatrixScaling(&_scale_mat, pMesh->m_vScale.x, pMesh->m_vScale.y, pMesh->m_vScale.z);
		tworldmat = _scale_mat * *pWorldMat_;
	}
	else
	{
		tworldmat = *pWorldMat_;
	}

	D3DXMatrixTranspose(&matTemp, &tworldmat);
	dev->SetVertexShaderConstantF(WORLD_MATRIX, (float*)&matTemp, 4);

	transformation = view * proj;
	D3DXMatrixTranspose(&matTemp, &transformation);
	dev->SetVertexShaderConstantF(VIEW_PROJECTION_MATRIX, (float*)&matTemp, 4);

	float fOutlineColor[4] = {
		((dwColor >> 16) & 0xff) / 255.0f,
		((dwColor >> 8) & 0xff) / 255.0f,
		((dwColor) & 0xff) / 255.0f,
		((dwAlpha) & 0xff) / 255.0f
	};
	float fOutlineThickness[4] = { fThickness, 0.f, 0.f, 0.f };
	dev->SetVertexShaderConstantF(MATERIAL_DIFFUSE, fOutlineColor, 1);
	dev->SetVertexShaderConstantF(MATERIAL_POWER, fOutlineThickness, 1);

	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	auto pDecl = RGetShaderMgr()->getShaderDecl(0);
	auto pShader = RGetShaderMgr()->getShader(SHADER_OUTLINE);
	dev->SetVertexDeclaration(RGetShaderMgr()->getShaderDecl(0));
	dev->SetVertexShader(RGetShaderMgr()->getShader(SHADER_OUTLINE));
	if (pDecl != nullptr && pShader != nullptr)
	{
		dev->SetVertexDeclaration(pDecl);
		dev->SetVertexShader(pShader);
	}
	else
	{
		// Nếu chưa có shader thì tạm thời dùng Shader mặc định hoặc bỏ qua lần vẽ này
		// mlog("Waiting for Shader/Decl to load...\n"); 
		return;
	}
	dev->SetRenderState(D3DRS_CULLMODE, (fThickness <= 0.0001f) ? D3DCULL_CCW : D3DCULL_CW);

	m_vsb->SetVSVertexBuffer();
	for (int i = 0; i < m_nMtrlCnt; ++i)
	{
		m_vsb->RenderIndexBuffer(m_ib[i]);
	}
}



void RMeshNode::ConnectToNameID()
{
	int id = RGetMeshNodeStringTable()->Get( m_Name );
	
	if(id==-1) {//bip °¡ ¾Æ´Ñ ÀÏ¹Ý ¿ÀºêÁ§Æ®µé...
		mlog("µî·ÏºÒ°¡ ÆÄÃ÷ %s \n",m_Name);
	}

	m_NameID = id;
}

RBoneBaseMatrix* RMeshNode::GetBaseMatrix(int pid)
{
	for(int i=0;i<m_nBoneBaseMatrixCnt;i++) 
	{
		if( m_BoneBaseMatrix[i].id == pid )
			return &m_BoneBaseMatrix[i];
	}
	return NULL;
}

/*
#define SetVertex(v,p,n,uv)						\
		v->p = p;								\
		v->n = n;								\
		v->tu = uv->x;							\
		v->tv = uv->y;							\

#define SetLVertex(v,p,c,uv)					\
		v->p = p;								\
		v->color = c;							\
		v->tu = uv->x;							\
		v->tv = uv->y;							\
*/

inline void SetVertex(RVertex* v,D3DXVECTOR3& p,D3DXVECTOR3& n,D3DXVECTOR3& uv) {

	v->p = p;	
	v->n = n;
	v->tu = uv.x;
	v->tv = uv.y;
}

inline void SetLVertex(RLVertex* v,D3DXVECTOR3& p,DWORD c,D3DXVECTOR3& uv) {

	v->p = p;	
	v->color = c;
	v->tu = uv.x;
	v->tv = uv.y;
}

void RMeshNode::UpdateNodeBuffer()
{
	if( !m_vb ) 
		_ASSERT(0);

	bool lvert	= m_pBaseMesh->m_LitVertexModel;

	RFaceInfo* pFace = NULL;

	D3DXVECTOR3* pP = m_point_list;
	RFaceNormalInfo* pFNL = NULL;

	// soft buffer ¸¸ ¾÷µ¥ÀÌÆ®.. ¹öÅØ½º ¿¡´Ï¸ÞÀÌ¼ÇÀÌ³ª.. vertex_ani Áß¿¡ uv ¿¡´Ï°¡ ÀÖ´Â°¡? À§Ä¡°ª¸¸ ÀÖ´Ù...

	if(!m_vb->m_pVert)
		_ASSERT(0);

	RVertex*	 pV  = (RVertex*)m_vb->m_pVert;
	RLVertex*	 pLV = (RLVertex*)m_vb->m_pVert;

	int w,p0,p1,p2;

	for (int i = 0; i < m_face_num  ; i ++) {

		w = 3*i;

		pFace = &m_face_list[i];
		pFNL = &m_face_normal_list[i];

		p0 = pFace->m_point_index[0];
		p1 = pFace->m_point_index[1];
		p2 = pFace->m_point_index[2];

		////////////////////////////////////////////////////

		if(lvert) {

			(pLV+w  )->p = pP[p0];
			(pLV+w+1)->p = pP[p1];
			(pLV+w+2)->p = pP[p2];
		} 
		else if(!RMesh::m_bVertexNormalOnOff) {

			(pV+w  )->p = pP[p0];
			(pV+w+1)->p = pP[p1];
			(pV+w+2)->p = pP[p2];

		} else {

			(pV+w  )->p = pP[p0];
			(pV+w+1)->p = pP[p1];
			(pV+w+2)->p = pP[p2];
		}
	}
}

void RMeshNode::MakeNodeBuffer(DWORD flag)
{
	static char	_pVert[RVERTEX_MAX];// 34133 face
	static WORD _pIndex[RVERTEX_MAX*3];

	bool lvert	= m_pBaseMesh->m_LitVertexModel;

	RFaceInfo* pFace = NULL;

	D3DXVECTOR3* pP = m_point_list;
	RFaceNormalInfo* pFNL = NULL;

	RVertex*	 pV  = (RVertex*)_pVert;
	RLVertex*	 pLV = (RLVertex*)_pVert;

	DWORD color = 0xffffffff;

	int w,w2,p0,p1,p2;

	if(m_face_num*3*32 > RVERTEX_MAX - 20 )
		mlog("elu ´ÜÀÏ ¿ÀºêÁ§Æ® ¹öÅØ½º °¹¼ö°¡ %d °³°¡ ³Ñ½À´Ï´Ù\n", RVERTEX_MAX-20 );

	int sub_mtrl;
	int face_cnt=0;		

	for (int i = 0; i < m_face_num  ; i ++) {

		w = 3*i;

		pFace = &m_face_list[i];
		pFNL = &m_face_normal_list[i];

		p0 = pFace->m_point_index[0];
		p1 = pFace->m_point_index[1];
		p2 = pFace->m_point_index[2];

		if(lvert) {

			SetLVertex((pLV+w  ),pP[p0],color,pFace->m_point_tex[0]);
			SetLVertex((pLV+w+1),pP[p1],color,pFace->m_point_tex[1]);
			SetLVertex((pLV+w+2),pP[p2],color,pFace->m_point_tex[2]);

		} 
		else if(!RMesh::m_bVertexNormalOnOff) {

			SetVertex((pV+w  ),pP[p0],pFNL->m_normal,pFace->m_point_tex[0]);
			SetVertex((pV+w+1),pP[p1],pFNL->m_normal,pFace->m_point_tex[1]);
			SetVertex((pV+w+2),pP[p2],pFNL->m_normal,pFace->m_point_tex[2]);

		} else {

			SetVertex((pV+w  ),pP[p0],pFNL->m_pointnormal[0],pFace->m_point_tex[0]);
			SetVertex((pV+w+1),pP[p1],pFNL->m_pointnormal[1],pFace->m_point_tex[1]);
			SetVertex((pV+w+2),pP[p2],pFNL->m_pointnormal[2],pFace->m_point_tex[2]);
		}
	}

	MakeVertexBuffer(0,lvert,_pVert,m_face_num*3,flag);

	for (int index = 0; index < m_nMtrlCnt ; index ++) 
	{
		face_cnt = 0;

		for (int i = 0; i < m_face_num  ; i ++) {

			if(m_nMtrlCnt != 1) {
				sub_mtrl = m_face_list[i].m_mtrl_id;
				if(sub_mtrl >= m_nMtrlCnt) 
					sub_mtrl -= m_nMtrlCnt;
				if(sub_mtrl != index) 
					continue;
			}

			w  = 3*face_cnt;
			w2 = 3*i;

			_pIndex[w  ] = w2;
			_pIndex[w+1] = w2+1;
			_pIndex[w+2] = w2+2;

			face_cnt++;
		}

		if(face_cnt)
			CreateIndexBuffer(index,_pIndex,face_cnt*3);
	}
}

bool RMeshNode::isSoftRender()
{
	bool bSoft = !RIsHardwareTNL();

	bool bVertexAnimation = m_pBaseMesh->isVertexAnimation(this);

	if( m_pBaseMesh->m_isCharacterMesh || 
		m_pBaseMesh->m_isNPCMesh || 
		bVertexAnimation ||	m_physique_num )
		bSoft = true;

	return bSoft;	
}

////////////////////////////////////////////////////////////////////////////////////
void RMeshNode::ToonRenderSettingOnOld(RMtrl* pMtrl)
{
	if( m_pParentMesh && m_pParentMesh->m_pVisualMesh && m_pParentMesh->m_pVisualMesh->m_ToonTexture ) {
		
		bool toonLighting	= m_pParentMesh->m_pVisualMesh->m_bToonLighting;
		bool toonTexture	= m_pParentMesh->m_pVisualMesh->m_bToonTextureRender;

		/////////////////////////////////////////////////////////////
		// ÄÃ·¯¼±ÅÃ

		DWORD color = 0xffffffff;
		D3DXCOLOR  dx_color = D3DXCOLOR(1.f,1.f,1.f,1.f);

		int ColorMode = 0;

		// ÀçÁúÀÇ ÄÃ·¯¼³Á¤ÀÌ ¿ì¼±±Ç...

		if( pMtrl->GetTColor() != D3DCOLOR_COLORVALUE(0.0f,1.0f,0.0f,0.0f) ) {
			ColorMode = 1;
			color = pMtrl->GetTColor() | 0xff000000;	// ÆÄÃ÷º°·Î »ö ÁöÁ¤ÇÏ´Â°É »öÀ¸·Î µî·Ï.. ¾ËÆÄ´Â Ã¤¿öÁØ´Ù..
		}
		else if( m_dwTFactorColor != D3DCOLOR_COLORVALUE(0.0f,1.0f,0.0f,0.0f) ) {
			ColorMode = 2;
			color = m_dwTFactorColor | 0xff000000;		// ÆÄÃ÷º°·Î »ö ÁöÁ¤ÇÏ´Â°É »öÀ¸·Î µî·Ï.. ¾ËÆÄ´Â Ã¤¿öÁØ´Ù..
		}
		
		if(ColorMode) {

			BYTE a,r,g,b;

			a = 255;//(color>>24)&0xff;
			r = (color>>16)&0xff;
			g = (color>> 8)&0xff;
			b = (color    )&0xff;

			dx_color = D3DXCOLOR(r/255.f,g/255.f,b/255.f,a/255.f);
		}

		// Toon Type
		// 0 ÀÏ¹Ý.. ¶óÀÌÆ® ÄÑ°í µÑ´Ù ±×¸®°í..
		// 1 ¶óÀÌÆ® ²ô°í µÑ´Ù ±×¸®°í..
		// ÅØ½ºÃÄ ¹Ù¸¦Áö ²øÁöÀÇ ¿©ºÎ...

		LPDIRECT3DDEVICE9 dev = RGetDevice();

		if( toonLighting ) { // Å÷¿¡¼­ÀÇ ¶óÀÌÆ®´Â ¾²ÀÌÁö ¾ÊÀ»°Í °°À½..

			dev->SetRenderState( D3DRS_LIGHTING , TRUE );

			dev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

			// ±âº»»óÅÂ...

//			dev->SetSamplerState( 0, D3DSAMP_MAGFILTER , D3DTEXF_LINEAR);
//			dev->SetSamplerState( 0, D3DSAMP_MINFILTER , D3DTEXF_LINEAR);
//			dev->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );

			SetMtrl(&dx_color,1.f);
		}
		else {

			dev->SetRenderState( D3DRS_LIGHTING , FALSE );
			dev->SetRenderState( D3DRS_TEXTUREFACTOR, color);

			dev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );

			dev->SetSamplerState( 0, D3DSAMP_MAGFILTER , D3DTEXF_POINT);
			dev->SetSamplerState( 0, D3DSAMP_MINFILTER , D3DTEXF_POINT);
			dev->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
		}

		dev->SetRenderState( D3DRS_ALPHABLENDENABLE , FALSE );
		dev->SetRenderState( D3DRS_ZENABLE , TRUE );
		dev->SetRenderState( D3DRS_ZWRITEENABLE , TRUE );

		dev->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		dev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

		dev->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
		dev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );

		dev->SetTransform( D3DTS_TEXTURE0, &m_pParentMesh->m_pVisualMesh->m_ToonUVMat ); 
		dev->SetTexture( 0, m_pParentMesh->m_pVisualMesh->m_ToonTexture );

//		float fDepthBias = -0.001f;
//		dev->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&fDepthBias); 

		if(toonTexture) {

//			dev->SetRenderState( D3DRS_TEXTUREFACTOR, color);

//			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_BLENDTEXTUREALPHA );
//			dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
//			dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );
//			dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
//			dev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

    		dev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			dev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
//			dev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE );
			dev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE4X );

			dev->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			dev->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );

			dev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
			dev->SetTexture( 1, pMtrl->GetTexture() ); // 1d ÅØ½ºÃÄ
		}
		else {

			dev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
			dev->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
//			dev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
			dev->SetTexture( 1, NULL ); // 1d ÅØ½ºÃÄ
		}
	}
}

void RMeshNode::ToonRenderSettingOn(RMtrl* pMtrl)
{
	if( m_pParentMesh && m_pParentMesh->m_pVisualMesh && m_pParentMesh->m_pVisualMesh->m_ToonTexture ) {
		
		bool toonLighting	= m_pParentMesh->m_pVisualMesh->m_bToonLighting;
		bool toonTexture	= m_pParentMesh->m_pVisualMesh->m_bToonTextureRender;

		/////////////////////////////////////////////////////////////
		// ÄÃ·¯¼±ÅÃ

		DWORD color = 0xffffffff;
		D3DXCOLOR  dx_color = D3DXCOLOR(1.f,1.f,1.f,1.f);

		int ColorMode = 0;

		// ÀçÁúÀÇ ÄÃ·¯ ¼³Á¤ÀÌ ¿ì¼±±ÇÀ» °®´Â´Ù...
		// Åø¿¡¼­´Â ¸ðµ¨°ú ÀçÁú 2°÷¿¡¼­ ÄÃ·¯ ¼±ÅÃÀ» ÇÏ°Ô µÇ¾îÀÖ´Ù..  
		// ¾ÕÀ¸·Î´Â ÀçÁú ´ÜÀ§·Î ÄÃ·¯¼³Á¤À» ÇÒ°Í...

		if( pMtrl->GetTColor() != D3DCOLOR_COLORVALUE(0.0f,1.0f,0.0f,0.0f) ) {
			ColorMode = 1;
			color = pMtrl->GetTColor() | 0xff000000;	// ÆÄÃ÷º°·Î »ö ÁöÁ¤ÇÏ´Â°É »öÀ¸·Î µî·Ï.. ¾ËÆÄ´Â Ã¤¿öÁØ´Ù..
		}
		else if( m_dwTFactorColor != D3DCOLOR_COLORVALUE(0.0f,1.0f,0.0f,0.0f) ) {
			ColorMode = 2;
			color = m_dwTFactorColor | 0xff000000;		// ÆÄÃ÷º°·Î »ö ÁöÁ¤ÇÏ´Â°É »öÀ¸·Î µî·Ï.. ¾ËÆÄ´Â Ã¤¿öÁØ´Ù..
		}
		
		if(ColorMode) {

			BYTE a,r,g,b;

			a = 255;//(color>>24)&0xff;
			r = (color>>16)&0xff;
			g = (color>> 8)&0xff;
			b = (color    )&0xff;

			dx_color = D3DXCOLOR(r/255.f,g/255.f,b/255.f,a/255.f);
		}

		// Toon Type
		// 0 ÀÏ¹Ý.. ¶óÀÌÆ® ÄÑ°í µÑ´Ù ±×¸®°í..
		// 1 ¶óÀÌÆ® ²ô°í µÑ´Ù ±×¸®°í..
		// ÅØ½ºÃÄ ¹Ù¸¦Áö ²øÁöÀÇ ¿©ºÎ...

		LPDIRECT3DDEVICE9 dev = RGetDevice();

		if( toonLighting ) { // Å÷¿¡¼­ÀÇ ¶óÀÌÆ®´Â ¾²ÀÌÁö ¾ÊÀ»°Í °°À½..Åø Å×½ºÆ®¿ë...

			dev->SetRenderState( D3DRS_LIGHTING , TRUE );

			dev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

			// ±âº»»óÅÂ...

//			dev->SetSamplerState( 0, D3DSAMP_MAGFILTER , D3DTEXF_LINEAR);
//			dev->SetSamplerState( 0, D3DSAMP_MINFILTER , D3DTEXF_LINEAR);
//			dev->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );

			SetMtrl(&dx_color,1.f);
		}
		else {

			dev->SetRenderState( D3DRS_LIGHTING , FALSE );
			dev->SetRenderState( D3DRS_TEXTUREFACTOR, color);

			// texture ¿Í color ¸¦ ¾ËÆÄ Ã¤³Î ºñÀ²·Î ¼¯´Â´Ù...

			dev->SetTextureStageState( 0, D3DTSS_COLOROP, pMtrl->m_TextureBlendMode);
//			dev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );

			dev->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
		}

		dev->SetRenderState( D3DRS_ALPHABLENDENABLE , FALSE );
		dev->SetRenderState( D3DRS_ZENABLE , TRUE );
		dev->SetRenderState( D3DRS_ZWRITEENABLE , TRUE );

		// texture ÀÇ ÇÊÅÍ¸µ... ÀçÁú ¿É¼ÇÀ¸·Î »©¾ßÇÑ´Ù..

		dev->SetSamplerState( 0, D3DSAMP_MAGFILTER , pMtrl->m_FilterType );
		dev->SetSamplerState( 0, D3DSAMP_MINFILTER , pMtrl->m_FilterType );

		// ¾ËÆÄÃ¤³ÎÀÌ 0,1 ÀÌ¸é µÑ´Ù ¾È±×¸°´Ù.. ¸¶½ºÅ©·Î »ç¿ë...

		dev->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
		dev->SetRenderState( D3DRS_ALPHAREF, pMtrl->m_AlphaRefValue );
		dev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

		dev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );

		dev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
		dev->SetTexture( 0, pMtrl->GetTexture() ); // 1d ÅØ½ºÃÄ

//		float fDepthBias = -0.001f;
//		dev->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&fDepthBias); 

		// ±Û·Î¿ì È¿°ú Áö¿øÇØ¾ßÇÔ...

		if(toonTexture) {

			dev->SetTextureStageState( 1, D3DTSS_COLOROP,   pMtrl->m_ToonTextureBlendMode );
			dev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			dev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );

			dev->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			dev->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );

			dev->SetSamplerState( 1, D3DSAMP_MAGFILTER , pMtrl->m_ToonFilterType);
			dev->SetSamplerState( 1, D3DSAMP_MINFILTER , pMtrl->m_ToonFilterType);

			dev->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
			dev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );
			
			// tool ¿¡¼­ ¹Þ¾Æ¿À´Â°ª..
/*
			//Ä«¸Þ¶ó °ø°£ inverse ±¸ÇÏ±â

			rmatrix matInvView = RView;

			matInvView._41 = 0; 
			matInvView._42 = 0; 
			matInvView._43 = 0; // position ¹«½Ã 

			D3DXMatrixTranspose( &matInvView, &matInvView );	// scale ¾øÀ¸¹Ç·Î ±×³É transpose ÇØÁàµµ µÊ.

			rmatrix matLightDir;
			rvector lightvec;

			memset( &matLightDir, 0, sizeof(rmatrix) );

			D3DXVec3Normalize(&lightvec,&-g_light_pos);

			matLightDir._11 = -0.5 * lightvec.x;
			matLightDir._21 = -0.5 * lightvec.y;
			matLightDir._31 = -0.5 * lightvec.z;
			matLightDir._41 = 0.5f;								// -0.5 °öÇÏ°í 0.5 ´õÇÏ±â. 
			matLightDir._44 = 1.00f;

			D3DXMatrixMultiply( &matLightDir, &matInvView, &matLightDir );
	
			pVMesh->m_ToonUVMat		= matLightDir;
			pVMesh->m_ToonTexture	= g_toon_tex;	// µðÀÚÀÌ³Ê°¡ ¸¸µç Toon Texture...
*/
			dev->SetTransform( D3DTS_TEXTURE1, &m_pParentMesh->m_pVisualMesh->m_ToonUVMat ); 

			if(pMtrl->m_pToonTexture)	// °³º°ÀûÀ¸·Î ÁöÁ¤µÈ°ÍÀÌ ÀÖÀ¸¸é ±×°Í »ç¿ë..
				dev->SetTexture( 1, pMtrl->m_pToonTexture->GetTexture() );
			else						
				dev->SetTexture( 1, m_pParentMesh->m_pVisualMesh->m_ToonTexture );

		}
		else {

			dev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
			dev->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
//			dev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
			dev->SetTexture( 1, NULL ); // 1d ÅØ½ºÃÄ
		}
	}
}


void RMeshNode::ToonRenderSettingOff()
{
	if( m_pParentMesh && m_pParentMesh->m_pVisualMesh && m_pParentMesh->m_pVisualMesh->m_ToonTexture ) {

		LPDIRECT3DDEVICE9 dev = RGetDevice();

		bool toonLighting	= m_pParentMesh->m_pVisualMesh->m_bToonLighting;
		bool toonTexture	= m_pParentMesh->m_pVisualMesh->m_bToonTextureRender;


//		float fDepthBias = -1.0f;
//		dev->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&fDepthBias); 

		///////////////////////////////////////////////////////////////////////////////

		if(!toonLighting)
		{
			dev->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
			dev->SetSamplerState( 0, D3DSAMP_MAGFILTER , D3DTEXF_LINEAR);
			dev->SetSamplerState( 0, D3DSAMP_MINFILTER , D3DTEXF_LINEAR);
		}

		if(toonTexture) {
			dev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
			dev->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
			dev->SetTexture( 1, NULL ); // 1d ÅØ½ºÃÄ
		}

	}
}

void RMeshNode::ToonRenderSilhouetteSettingOn()
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();
	DWORD color = RMesh::m_dwSilhouetteColor | 0xff000000;

	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	dev->SetRenderState(D3DRS_LIGHTING, FALSE);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dev->SetRenderState(D3DRS_TEXTUREFACTOR, color);
	dev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
	dev->SetTexture(0, NULL);
	dev->SetTexture(1, NULL);
	dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
}

void RMeshNode::ToonRenderSilhouetteSettingOff()
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	dev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	dev->SetRenderState(D3DRS_LIGHTING, TRUE);
	dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dev->SetPixelShader(NULL);
	dev->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
}
uint32_t BlendColor = 0;

void RMeshNode::Render(D3DXMATRIX* pWorldMatrix)
{
	RMtrl* pMtrl = NULL;
	RMesh* pMesh = m_pBaseMesh;

	if (pMesh == NULL) return;

	bool bSoft = isSoftRender();

	{

		if (pWorldMatrix)	RGetDevice()->SetTransform(D3DTS_WORLD, pWorldMatrix); // ±×¸®±â°¡ º¸·ùµÈ °æ¿ì,,,
		else				RGetDevice()->SetTransform(D3DTS_WORLD, &m_ModelWorldMatrix);

	}

	if (bSoft == false)
		m_vb->SetVertexBuffer();

	for (int index = 0; index < m_nMtrlCnt; index++) {

		pMtrl = GetMtrl(index);

		if (pMtrl == NULL) return;
		if (m_ib[index] == NULL) continue;

		__BP(402, "RMeshNode::Render State b");

		pMesh->SetCharacterMtrl_ON(pMtrl, this, pMesh->GetMeshNodeVis(this), GetTColor());

		__EP(402);

#ifdef USE_TOON_RENDER

		/////////////////////////////////////////////////////////////////////////////////

		ToonRenderSettingOn(pMtrl);

		__BP(403, "RMeshNode::RenderIndex");

		if (m_pParentMesh && m_pParentMesh->m_pVisualMesh && m_pParentMesh->m_pVisualMesh->m_ToonTexture) {
			bSoft = true;
		}

		/////////////////////////////////////////////////////////////////////////////////		
#endif

		if (BlendColor)
		{
			RGetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
			RGetDevice()->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
			RGetDevice()->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CONSTANT);
			RGetDevice()->SetTextureStageState(1, D3DTSS_CONSTANT, BlendColor);

			RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);

			RGetDevice()->SetTexture(1, nullptr);
		}

		if (bSoft)	m_vb->RenderIndexSoft(m_ib[index]);
		else 		m_vb->RenderIndexBuffer(m_ib[index]);

		if (BlendColor)
		{
			RGetDevice()->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			RGetDevice()->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		}

		__EP(403);


#ifdef USE_TOON_RENDER

		if (RMesh::m_bSilhouette && m_pParentMesh && (RMesh::m_fSilhouetteLength > 0.f) && RShaderMgr::mbUsingShader)
		{
			ToonRenderSilhouetteSettingOn();

			if (m_vb && m_ib[index]) {
				rmatrix matWorld = m_ModelWorldMatrix;
				if (pWorldMatrix)
					matWorld = *pWorldMatrix;

				RGetDevice()->SetTransform(D3DTS_WORLD, &matWorld);

				// Draw the outline using a vertex shader
				RGetDevice()->SetVertexShader(RGetShaderMgr()->getShader(SHADER_OUTLINE));

				rmatrix matWorldViewProj = matWorld * RView * RProjection;
				D3DXMatrixTranspose(&matWorldViewProj, &matWorldViewProj);
				RGetDevice()->SetVertexShaderConstantF(0, (float*)&matWorldViewProj, 4);
				float fSilhouetteLength = RMesh::m_fSilhouetteLength;
				RGetDevice()->SetVertexShaderConstantF(4, (float*)&fSilhouetteLength, 1);

				m_vb->RenderIndexBuffer(m_ib[index]);

				RGetDevice()->SetVertexShader(NULL);
			}

			ToonRenderSilhouetteSettingOff();
		}

		ToonRenderSettingOff();

#endif

		__BP(411, "RMeshNode::Render State e");

		pMesh->SetCharacterMtrl_OFF(pMtrl, pMesh->GetMeshNodeVis(this));

		__EP(411);

	}
}

void RMeshNode::CheckAlignMapObject(rmatrix& hr_mat) // ¸Ê¿ÀºêÁ§Æ®¸¸..
{
	int align = m_nAlign;

	if( m_pBaseMesh->m_is_map_object==false )	return;
	if( align == 0 )							return;

	rvector cam_dir = RCameraDirection;
	rmatrix ret_mat, scale_mat;

	ret_mat = hr_mat;

	float sx = Magnitude(rvector(ret_mat._11,ret_mat._12,ret_mat._13));//right
	float sy = Magnitude(rvector(ret_mat._21,ret_mat._22,ret_mat._23));//up
	float sz = Magnitude(rvector(ret_mat._31,ret_mat._32,ret_mat._33));//dir

	D3DXMatrixScaling(&scale_mat,sx,sy,sz);

	if(align==1) {

		rmatrix mat;

		rvector right,up,vUp,vDir,vPos,vRight;

		vUp		= rvector(ret_mat._21,ret_mat._22,ret_mat._23);
		vDir	= cam_dir;
		vPos	= rvector(ret_mat._41,ret_mat._42,ret_mat._43);

		D3DXVec3Normalize(&vDir, &vDir);

		D3DXVec3Cross(&right, &vUp, &vDir);
		D3DXVec3Normalize(&right, &right);

		D3DXVec3Cross(&up, &vDir, &right);
		D3DXVec3Normalize(&up, &up);

		mat._14 = 0.f;
		mat._24 = 0.f;
		mat._34 = 0.f;
		mat._44 = 1.f;

		mat._11 = right.x;
		mat._12 = right.y;
		mat._13 = right.z;

		mat._21 = up.x;
		mat._22 = up.y;
		mat._23 = up.z;

		mat._31 = vDir.x;
		mat._32 = vDir.y;
		mat._33 = vDir.z;

		mat._41 = vPos.x;
		mat._42 = vPos.y;
		mat._43 = vPos.z;

		hr_mat = scale_mat * mat;
	}
	else if(align==2) {

		rmatrix mat;

		mat = ret_mat;//pMNode->m_mat_result;

		rvector right = rvector(mat._11,mat._12,mat._13);
		rvector dir	= rvector(mat._21,mat._22,mat._23);
		rvector up = rvector(mat._31,mat._32,mat._33);

		CrossProduct(&right,dir,cam_dir);
		Normalize(right);

		CrossProduct(&up,right,dir);
		Normalize(up);

		mat._11 = right.x;
		mat._12 = right.y;
		mat._13 = right.z;

		mat._21 = dir.x;
		mat._22 = dir.y;
		mat._23 = dir.z;

		mat._31 = up.x;
		mat._32 = up.y;
		mat._33 = up.z;

		hr_mat = scale_mat * mat ;
	}
}

void RMeshNode::CheckAlign(rmatrix* world_mat)
{
	if (m_nAlign == 0)	return;

	if(world_mat==NULL) return;

	if(m_pBaseMesh->m_is_map_object) return;//¸Ê ¿ÀºêÁ§Æ®´Â µû·Î..

	rvector cam_dir;//=RCameraDirection;

	// local ¿¡¼­ºÃÀ»¶§ÀÇ camera direction

	//	rmatrix inv;
	//	float fDet;
	//	D3DXMatrixInverse(&inv,&fDet,world_mat);
	//	D3DXVec3TransformNormal(&cam_dir,&RCameraDirection,&inv);

	// À§ ÁÖ¼®µÈ ºÎºÐÀÇ inverse matrix ´ë½Å transpose matrix ¸¦ °öÇØÁÖ¾ú´Ù.
	// ±×·¯³ª ¸¸¾à world_mat ¿¡ scale ¼ººÐÀÌ ÀÖ´Ù¸é, cam_dirÀÇ normalize °¡ ÇÊ¿äÇÏ´Ù.

	cam_dir.x=RCameraDirection.x*world_mat->_11+RCameraDirection.y*world_mat->_12+RCameraDirection.z*world_mat->_13;
	cam_dir.y=RCameraDirection.x*world_mat->_21+RCameraDirection.y*world_mat->_22+RCameraDirection.z*world_mat->_23;
	cam_dir.z=RCameraDirection.x*world_mat->_31+RCameraDirection.y*world_mat->_32+RCameraDirection.z*world_mat->_33;

	int align = m_nAlign;

	rmatrix ret_mat, scale_mat;

	ret_mat = m_mat_result;

	float sx = Magnitude(rvector(ret_mat._11,ret_mat._12,ret_mat._13));//right
	float sy = Magnitude(rvector(ret_mat._21,ret_mat._22,ret_mat._23));//up
	float sz = Magnitude(rvector(ret_mat._31,ret_mat._32,ret_mat._33));//dir

	D3DXMatrixScaling(&scale_mat,sx,sy,sz);

	if(align==1) {

		rmatrix mat;
//		rmatrix base_mat = m_mat_base;
//		rmatrix base_mat = ret_mat;//m_mat_base;

		rvector right,up,vUp,vDir,vPos,vRight;

		vUp		= rvector(ret_mat._21,ret_mat._22,ret_mat._23);
		vDir	= cam_dir;
		vPos	= rvector(ret_mat._41,ret_mat._42,ret_mat._43);

		D3DXVec3Normalize(&vDir, &vDir);

		D3DXVec3Cross(&right, &vUp, &vDir);
		D3DXVec3Normalize(&right, &right);

		D3DXVec3Cross(&up, &vDir, &right);
		D3DXVec3Normalize(&up, &up);

//		D3DXMatrixIdentity(&mat);

		mat._14 = 0.f;
		mat._24 = 0.f;
		mat._34 = 0.f;
		mat._44 = 1.f;

		mat._11 = right.x;
		mat._12 = right.y;
		mat._13 = right.z;

		mat._21 = up.x;
		mat._22 = up.y;
		mat._23 = up.z;

		mat._31 = vDir.x;
		mat._32 = vDir.y;
		mat._33 = vDir.z;

		mat._41 = vPos.x;
		mat._42 = vPos.y;
		mat._43 = vPos.z;

		m_mat_result = scale_mat * mat;
	}
	else if(align==2) {

		rmatrix mat;

		mat = ret_mat;//m_mat_result;

		rvector right = rvector(mat._11,mat._12,mat._13);
		rvector dir	= rvector(mat._21,mat._22,mat._23);
		rvector up = rvector(mat._31,mat._32,mat._33);

		Normalize(dir);
		Normalize(cam_dir);

		CrossProduct(&right,dir,cam_dir);
		Normalize(right);

		CrossProduct(&up,right,dir);
		Normalize(up);

		mat._11 = right.x;
		mat._12 = right.y;
		mat._13 = right.z;

		mat._21 = dir.x;
		mat._22 = dir.y;
		mat._23 = dir.z;

		mat._31 = up.x;
		mat._32 = up.y;
		mat._33 = up.z;

		m_mat_result = scale_mat * mat ;
	}
}

int RMeshNode::CalcVertexBuffer_VertexAni(int frame)
{
	RAnimationNode* pANode = m_pAnimationNode;

	if(pANode) {

		if( m_point_num == pANode->m_vertex_vcnt) {

			int nCnt = pANode->GetVecValue(frame,m_point_list);
		}
		else { // ¹öÅØ½º¼ö°¡ Æ²¸±°æ¿ì
			mlog("vertex ani ¿¡¼­ ¹öÅØ½º °¹¼ö°¡ Æ²¸²\n");// ÀÌ·± °æ¿ì°¡ ¾ø¾î¾ß ÇÔ..
		}
	}

	return 1;
}


void RMeshNode::CalcVertexBuffer_Physique(D3DXMATRIX* world_mat,int frame)
{
	int p_num,i,j,p_id;
	D3DXVECTOR3 _vec_all,_vec;
	float weight;
	D3DXMATRIX t_mat,local,basemat,inv;

	RMeshNode* pTMP = NULL;

	RMesh* pMesh = m_pBaseMesh;

	for(i=0;i<m_physique_num;i++) { // point_num

		_vec_all = rvector(0,0,0);

		p_num = m_physique[i].m_num;

		if(p_num > MAX_PHYSIQUE_KEY) p_num = MAX_PHYSIQUE_KEY;

		for(j=0;j<p_num;j++) {// 4

			p_id	= m_physique[i].m_parent_id[j];
			weight	= m_physique[i].m_weight[j];

			pTMP = pMesh->m_data[p_id];

			if( pTMP) 	t_mat = pTMP->m_mat_result;
			else 		mlog("RMesh::CalcVertexBuffer() %s node : %d physique :num %d :not found !!! \n",GetName(),i,j);

			_vec = m_physique[i].m_offset[j];

//			_vec = pMeshNode->m_point_list[i];
//			RBoneBaseMatrix* BoneMatrix = pMeshNode->GetBaseMatrix(p_id);
//			if(BoneMatrix==NULL)	t_mat =  pTMP->m_mat_result;
//			else 					t_mat =  BoneMatrix->mat * pTMP->m_mat_result;
//			t_mat =  pMeshNode->m_mat_ref * pTMP->m_mat_ref_inv * pTMP->m_mat_result;

			D3DXVec3TransformCoord(&_vec,&_vec,&t_mat);
			_vec_all += _vec * weight;
		}

		m_point_list[i] = _vec_all;
	}
}


void RMeshNode::CalcVertexBuffer_Tm(D3DXMATRIX* world_mat,int frame)
{

}

void RMeshNode::CalcVertexBuffer_Bbox(CalcVertexBufferBboxMode nBboxMode,rmatrix& mat)
{
	int nCnt = m_point_num;
	rvector v;

	RMesh* pMesh = m_pBaseMesh;

	if(	nBboxMode==CalcVertexBufferBboxMode_VertexAni || 
		nBboxMode==CalcVertexBufferBboxMode_Physique ) {

			for(int i=0;i<nCnt;i++) {
				pMesh->SubCalcBBox( &pMesh->m_vBBMax, &pMesh->m_vBBMin, &m_point_list[i]);
			}
		}
	else if(nBboxMode==CalcVertexBufferBboxMode_TM_MapObject||
		nBboxMode==CalcVertexBufferBboxMode_TM_Object) {

			for(int i=0;i<nCnt;i++) { 
				v = m_point_list[i] * mat;
				pMesh->SubCalcBBox( &pMesh->m_vBBMax, &pMesh->m_vBBMin, &v);
			}
		}
}

bool RMeshNode::CalcPickVertexBuffer(D3DXMATRIX* world_mat,D3DXVECTOR3* pVec)
{
	RMesh* pMesh = m_pBaseMesh;

	CalcVertexBufferBboxMode nBboxMode = CalcVertexBufferBboxMode_None;
	rmatrix BBoxMatrix;

	LPDIRECT3DDEVICE9  dev = RGetDevice();

	D3DXMATRIX result_mat = m_mat_result;
	D3DXMATRIX scale_mat = GetIdentityMatrix();

	if( pMesh->m_isScale ) {
		D3DXMatrixScaling(&scale_mat, pMesh->m_vScale.x, pMesh->m_vScale.y, pMesh->m_vScale.z);
		result_mat = result_mat * scale_mat;
	}

	RAnimation* pAniSet = pMesh->GetNodeAniSet(this);

	int frame = GetNodeAniSetFrame();

	static rmatrix map_rot_mat = RGetRotY(180) * RGetRotX(90);

	rmatrix	ModelWorldMatrix;
	D3DXMatrixIdentity(&ModelWorldMatrix);

	if( pAniSet && (pAniSet->GetAnimationType()  == RAniType_Vertex) ) {

		CalcVertexBuffer_VertexAni( frame );

		if(pMesh->m_is_map_object)	
			ModelWorldMatrix = scale_mat*map_rot_mat*(*world_mat);
		else						
			ModelWorldMatrix = scale_mat*(*world_mat);

		nBboxMode = CalcVertexBufferBboxMode_VertexAni;

	}
	else if( pAniSet && m_physique_num) {

		CalcVertexBuffer_Physique(world_mat,frame);

		ModelWorldMatrix = scale_mat*(*world_mat);

		nBboxMode = CalcVertexBufferBboxMode_Physique;

	}
	else {

		if(pMesh->m_is_map_object) { 

			ModelWorldMatrix = result_mat * map_rot_mat;
			CheckAlignMapObject(ModelWorldMatrix);

			nBboxMode = CalcVertexBufferBboxMode_TM_MapObject;
			BBoxMatrix = ModelWorldMatrix;

			ModelWorldMatrix = ModelWorldMatrix * (*world_mat);
		}
		else {

			nBboxMode = CalcVertexBufferBboxMode_TM_Object;

			BBoxMatrix = result_mat;

			ModelWorldMatrix = result_mat * (*world_mat);
		}

	}

	for(int i=0;i<m_point_num;i++) { 
		pVec[i] = m_point_list[i] * ModelWorldMatrix;
	}

	return true;
}

void RMeshNode::CalcVertexBuffer(D3DXMATRIX* world_mat,bool box)
{
	__BP(207,"RMesh::CalcVertexBuffer");

	int nNeedUpdate = 0;

	RMesh* pMesh = m_pBaseMesh;

	CalcVertexBufferBboxMode nBboxMode = CalcVertexBufferBboxMode_None;
	rmatrix BBoxMatrix;

	LPDIRECT3DDEVICE9  dev = RGetDevice();

	D3DXMATRIX result_mat = m_mat_result;
	D3DXMATRIX scale_mat = GetIdentityMatrix();

	if( pMesh->m_isScale ) {
		D3DXMatrixScaling(&scale_mat, pMesh->m_vScale.x, pMesh->m_vScale.y, pMesh->m_vScale.z);
		result_mat = result_mat * scale_mat;
	}

	RAnimation* pAniSet = pMesh->GetNodeAniSet(this);

	int frame = GetNodeAniSetFrame();

	static rmatrix map_rot_mat = RGetRotY(180) * RGetRotX(90);

	rmatrix	ModelWorldMatrix;
	D3DXMatrixIdentity(&ModelWorldMatrix);

	if( pAniSet && (pAniSet->GetAnimationType()  == RAniType_Vertex) ) {
			
		nNeedUpdate = CalcVertexBuffer_VertexAni( frame );

		if(pMesh->m_is_map_object)	ModelWorldMatrix = scale_mat*map_rot_mat*(*world_mat);
		else						ModelWorldMatrix = scale_mat*(*world_mat);

		nBboxMode = CalcVertexBufferBboxMode_VertexAni;

	}
	else if( pAniSet && m_physique_num) {

		CalcVertexBuffer_Physique(world_mat,frame);

		ModelWorldMatrix = scale_mat*(*world_mat);

		nBboxMode = CalcVertexBufferBboxMode_Physique;
		
		nNeedUpdate = 1;
	}
	else {

//		CalcVertexBuffer_Tm(pMeshNode,world_mat,frame,pick);

		if(pMesh->m_is_map_object) { 

			ModelWorldMatrix = result_mat * map_rot_mat;
			CheckAlignMapObject(ModelWorldMatrix);

			nBboxMode = CalcVertexBufferBboxMode_TM_MapObject;
			BBoxMatrix = ModelWorldMatrix;

			ModelWorldMatrix = ModelWorldMatrix * (*world_mat);
		}
		else {

			nBboxMode = CalcVertexBufferBboxMode_TM_Object;

			BBoxMatrix = result_mat;

			ModelWorldMatrix = result_mat * (*world_mat);
		}

	}

	if(box){
		CalcVertexBuffer_Bbox(nBboxMode,BBoxMatrix);//µÎ¹ø°öÇÏ°ÔµÊ,,ºÒÇÊ¿ä..
	}


	m_ModelWorldMatrix = ModelWorldMatrix;

	__EP(207);

	//////////////////////////////////////////////////
	// ¹öÆÛÀÇ »ý¼º°ú °»½Å

	if( nNeedUpdate)
		UpdateNodeBuffer( ); // m_point_list ¿¡ ¸¸µé¾îÁø°É m_pVert ·Î ¿Å±ä´Ù...
}

// ºÎ´ã½º·¯¿îÀÛ¾÷.. 
// ³ªÁß¿¡ ½ºÅ°´×ÇÏ´Â ¹öÅØ½ºµé¸¸ µû·Î Àç°è»ê..

void RMeshNode::CalcVertexNormal(D3DXMATRIX* world_mat)
{
/*
	D3DXPLANE	plane;
	D3DXVECTOR3	vv[3];

	int i=0,j=0;

	if(m_face_num) {

		for(i=0;i<m_face_num;i++) {

			vv[0] = g_point_list[m_face_list[i].m_point_index[0]];
			vv[1] = g_point_list[m_face_list[i].m_point_index[1]];
			vv[2] = g_point_list[m_face_list[i].m_point_index[2]];

			D3DXPlaneFromPoints(&plane,&vv[0],&vv[1],&vv[2]);
			D3DXPlaneNormalize(&plane,&plane);

			m_face_normal[i].x = plane.a;
			m_face_normal[i].y = plane.b;
			m_face_normal[i].z = plane.c;

		}
	}

	///////////////////////////////////////////////////

	static int p_cnt[1000];

	memset(p_cnt,0,sizeof(int)*1000);

	for(i=0;i<m_face_num;i++) {
		for(j=0;j<3;j++) {
			m_point_normal_list[ m_face_list[i].m_point_index[j] ] =
				m_point_normal_list[ m_face_list[i].m_point_index[j] ] + m_face_normal[i];

			p_cnt[ m_face_list[i].m_point_index[j] ]++;
		}
	}

	for(i=0;i<m_point_num;i++) {
		m_point_normal_list[i] = m_point_normal_list[i] / (float)p_cnt[i];
		D3DXVec3Normalize(&m_point_normal_list[i],&m_point_normal_list[i]);
	}
*/
}

int RMeshNode::GetNodeAniSetFrame()
{
	if( m_pBaseMesh->m_pAniSet[1] ) {				// »ó¹Ý½Å ¿¡´Ï¸ÞÀÌ¼ÇÀÌ ÀÖ°í
		if(m_CutPartsType == cut_parts_upper_body) {// ÀÌ³ëµå°¡ »ó¹Ý½Å ³ëµå¶ó¸é
			return m_pBaseMesh->m_frame[1];		// »ó¹Ý½Å ÇÁ·¹ÀÓÀ» »ç¿ë,,,
		}
	}
	return m_pBaseMesh->m_frame[0];
}

float RMeshNode::GetNodeVisValue()
{
	if(m_pAnimationNode) {

		if(m_pAnimationNode->m_vis_cnt) {

			int _nframe = GetNodeAniSetFrame();

			m_vis_alpha = m_pAnimationNode->GetVisValue(_nframe);

			return m_vis_alpha;
		}
	}

	m_vis_alpha = 1.f;

	return m_vis_alpha;
}

bool RMeshNode::isAlphaMtrlNode()
{
	if(m_isAlphaMtrl || m_vis_alpha != 1.f )
		return true;
	return false;
}


_NAMESPACE_REALSPACE2_END
