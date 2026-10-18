#include "stdafx.h"
#include "ZWorldObject.h"
#include "ZMap.h"
#ifdef _MOVINGFLOOR
MapObjectCollision::MapObjectCollision()
{
	//TODO: make use of collisiontype
	CollisionType = CT_CYLINDER;
	Height = 0;
	Width = 0;
	Radius = 0;
	Collidable = false;
}

MapObjectCollision::~MapObjectCollision()
{

}

//todo: fetch information from a xml for the map
ZWorldObject::ZWorldObject() noexcept
{
	VisualMesh = nullptr;
	LastMoveDiff = rvector(0, 0, 0);
}

ZWorldObject::~ZWorldObject() noexcept
{
	delete VisualMesh;
	VisualMesh = nullptr;
	RMesh* playerMesh = nullptr;

	//remove model from memory, will be loaded again next time it's needed
//	ZGetMeshMgr()->Del((char*)Model.c_str());
	playerMesh = ZGetMeshMgr()->Get("heroman1");
	playerMesh = ZGetMeshMgr()->Get("herowoman1");
}

bool ZWorldObject::InitWithMesh(WorldObject const& worldObj)
{
	Name = worldObj.name;
	Model = worldObj.model;

	char szMapPath[64] = "";
	ZGetCurrMapPath(szMapPath);

	char szBuf[256];

	sprintf(szBuf, "%s%s/", szMapPath, ZGetGameClient()->GetMatchStageSetting()->GetMapName());
	std::string meshpath = szBuf;
	meshpath.append(Model);

	RMesh* pMesh = ZGetMeshMgr()->Get((char*)worldObj.name.c_str());

	if (pMesh == nullptr)
	{
		ZGetMeshMgr()->Add((char*)meshpath.c_str(), (char*)worldObj.name.c_str(), false, true);
	}


	pMesh = ZGetMeshMgr()->Get((char*)worldObj.name.c_str());
	if (pMesh == nullptr)
		return false;

	pMesh->ReloadAnimation();

	VisualMesh = new RVisualMesh;
	VisualMesh->Create(pMesh);

	if (worldObj.animation.empty() == false)
	{
		std::string meshpath = szBuf;
		meshpath.append(worldObj.animation.c_str());
		RAnimation* pAni = VisualMesh->GetMesh()->m_ani_mgr.AddGameLoad((char*)worldObj.animation.c_str(), (char*)meshpath.c_str(), -1, 0);
		pAni->SetAnimationLoopType(AnimationLoopType::RAniLoopType_Loop);
		VisualMesh->SetAnimation(worldObj.animation.c_str());
	}

	VisualMesh->SetVisibility(1.f);
	VisualMesh->GetMesh()->SetTextureRenderOnOff(true);
	VisualMesh->SetCheckViewFrustum(false);
	VisualMesh->SetScale((D3DXVECTOR3)(worldObj.scale));

	//collision
	{
		SetCollidable(worldObj.collidable);
		SetCollRadius(worldObj.collradius);
		SetCollWidth(worldObj.collwidth);
		SetCollHeight(worldObj.collheight);
		SetCollisionType(worldObj.collisiontype);
	}

	StartPosition = worldObj.position;
	CurrPosition = StartPosition;
	Direction = worldObj.direction;
	Sound = worldObj.sound;

	rmatrix mat = GetWorldMatrix();
	VisualMesh->SetWorldMatrix(mat);


	return true;
}

void ZWorldObject::Update(float elapsed)
{

}

void ZWorldObject::Draw()
{
	if (VisualMesh == nullptr || VisualMesh->GetMesh()->m_isMeshLoaded == false)
		return;

	LPDIRECT3DDEVICE9 dev = RGetDevice();

	if (RAdvancedGraphics::nMultiSampling > D3DMULTISAMPLE_NONE)
		dev->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);

	//need to update the frame at every draw call
	VisualMesh->Frame();
	VisualMesh->Render();

	if (RAdvancedGraphics::nMultiSampling > D3DMULTISAMPLE_NONE)
	{
		RGetDevice()->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
	}
}



//TODO: fill this in
bool ZWorldObject::Pick(rvector& pos, rvector& dir, RBSPPICKINFO* pOut)
{
	if (IsCollidable() == false)
	{
		return false;
	}
	rvector diff = GetPosition() - pos;
	diff.z = 0;

	// 나중에 radius상수값으로 된것 Object의 멤버변수로 고치자
	float objDistance = 0;
	if (GetCollisionType() == CT_CYLINDER)
		objDistance = GetCollRadius();
	else
		objDistance = GetCollWidth();

	//todo: improve some more but fixes the teleportation bug
	if (Magnitude(diff) < objDistance && fabs(CurrPosition.z - pos.z) < (GetCollHeight() + CHARACTER_HEIGHT))
	{
		if (pos.z + CHARACTER_HEIGHT >= CurrPosition.z + GetCollHeight())
			return true;

		return false;
	}
	return false;
}

bool ZWorldObject::OnCheckWallHang(rvector const& pos, rvector const& dir, bool const& initial)
{

	if (IsStandingOn(pos) == true)
		return false;

	//todo: tweak this better
	if (IsCollidable() == false)
	{
		return false;
	}

	rvector diff = GetPosition() - pos;
	diff.z = 0;

	float objDistance = 0;
	if (GetCollisionType() == CT_CYLINDER)
		objDistance = GetCollRadius();
	else
		objDistance = GetCollWidth();

	float heightdiff = fabs(pos.z - CurrPosition.z);

	//if player is close enough to the object, but not too high, return true to hang.
	if (Magnitude(diff) < objDistance + 100) //add 100 for sword stab
	{
		if(heightdiff <= GetCollHeight())
		return true;
	}
	return false;
}

bool ZWorldObject::IsStandingOn(rvector const& pos)
{
	if (IsCollidable() == false)
	{
		return false;
	}
	rvector diff = GetPosition() - pos;
	diff.z = 0;

	// 나중에 radius상수값으로 된것 Object의 멤버변수로 고치자
	float objDistance = 0;
	if (GetCollisionType() == CT_CYLINDER)
		objDistance = GetCollRadius();
	else
		objDistance = GetCollWidth();

	
	//todo: improve some more but fixes the teleportation bug
	if (Magnitude(diff) < objDistance && fabs(CurrPosition.z - pos.z) < (GetCollHeight() + CHARACTER_HEIGHT))
	{
		if (pos.z + CHARACTER_HEIGHT >= CurrPosition.z + GetCollHeight())
			return true;

		return false;
	}
	return false;
}
#endif
