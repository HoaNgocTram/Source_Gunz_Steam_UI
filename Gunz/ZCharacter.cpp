#include "stdafx.h"

#include "ZApplication.h"
#include "ZGame.h"
#include "ZCharacter.h"
#include "RVisualMeshMgr.h"
#include "RealSpace2.h"
#include "MDebug.h"
#include "MObject.h"
#include "ZPost.h"
#include "ZGameInterface.h"
#include "RBspObject.h"
#include "zshadow.h"
#include "MProfiler.h"
#include "RShaderMgr.h"
#include "ZScreenEffectManager.h"
#include "RDynamicLight.h"
#include "ZConfiguration.h"
#include "MMatchObject.h"
#include "RCollisionDetection.h"
#include "ZEffectStaticMesh.h"
#include "ZEffectAniMesh.h"
#include "ZModule_HPAP.h"
#include "ZModule_Movable.h"
#include "ZModule_Resistance.h"
#include "ZModule_FireDamage.h"
#include "ZModule_ColdDamage.h"
#include "ZModule_LightningDamage.h"
#include "ZModule_PoisonDamage.h"
#include "ZModule_QuestStatus.h"
#include "ZModule_HealOverTime.h"
#include "ZGameConst.h"
#include "RGMain.h"

#ifdef _ANTIHACK
#include "ZGuard.h"
#endif

#define ANGLE_TOLER			.1f
#define ANGLE_SPEED			12.f

#define ROLLBACK_TOLER		20.f

#define ENABLE_CHARACTER_COLLISION

//#ifdef _PUBLISH
#define ENABLE_FALLING_DAMAGE
//#endif

#define IsKeyDown(key) ((GetAsyncKeyState(key) & 0x8000)!=0)
//////////////////////////////////////////////////////////////////////////
//	GLOBAL
//////////////////////////////////////////////////////////////////////////

bool Enable_Cloth	= true;

static DWORD ZGetCharacterOutlineColor(const ZCharacter* pCharacter)
{
	DWORD dwCustomColor = ZGetConfiguration()->GetVideo()->dwOutlineColor;
	if ((dwCustomColor & 0xFF000000) == 0) dwCustomColor |= 0xFF000000;

	ZGame* pGame = ZGetGame();
	if (!pGame) return dwCustomColor;

	ZCharacter* pMyChar = pGame->m_pMyCharacter;
	if (!pMyChar) return dwCustomColor;

	// Màu định nghĩa sẵn
	const DWORD OUTLINE_RED = 0xFFDB0000; // Địch
	const DWORD OUTLINE_BLUE = 0xFF4A7DFF; // Ta & Đồng đội
	const DWORD OUTLINE_GREEN = 0xFF00FF00; // Quest NPC/Đồng đội Quest

	// 1. Nếu là chế độ Quest
	MMATCH_GAMETYPE nGameType = ZGetGameClient()->GetMatchStageSetting()->GetGameType();
	if (nGameType == MMATCH_GAMETYPE_QUEST ||
		nGameType == MMATCH_GAMETYPE_SURVIVAL ||
		nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE) {
		if (pCharacter == pMyChar) return OUTLINE_BLUE;
		return OUTLINE_BLUE;
	}

	// 2. Nếu là Team Play (ClanWar, TDM)
	if (pGame->GetMatch()->IsTeamPlay())
	{
		if (pCharacter->GetTeamID() == pMyChar->GetTeamID())
			return OUTLINE_BLUE;
		else
			return OUTLINE_RED;
	}

	// 3. Các chế độ Solo (Deathmatch, Gladiator)
	if (pCharacter == pMyChar) return dwCustomColor; //0xFF00FFFF; // Màu Cyan cho bản thân cho nổi bật

	return dwCustomColor; // Kẻ thù dùng màu tùy chọn trong Option
}

// Custom: Ghost Mode - Desperate
static ZANIMATIONINFO g_AnimationInfoTableGhost[ZC_STATE_LOWER_END] = { 
 { "" ,true ,true ,false ,false },

 { "idle" ,true ,true ,false ,false },
 { "idle2" ,true ,true ,false ,false },
 { "idle3" ,true ,true ,false ,false },
 { "idle4" ,true ,true ,false ,false },

 { "run" ,true ,true ,false ,false },
 { "runB" ,true ,true ,false ,false },
 { "runL" ,true ,true ,false ,false },
 { "runR" ,true ,true ,false ,false },

 { "jumpU" ,true ,false ,true ,false },
 { "jumpD" ,true ,false ,true ,false },

 { "die" ,true ,false ,false ,false },
 { "die2" ,true ,false ,false ,false },
 { "die3" ,true ,false ,false ,false },
 { "die4" ,true ,false ,false ,false },


 { "runLW" ,false ,true ,true ,false},
 { "runLW_down" ,false ,false ,true ,false},
 { "runW" ,false ,false ,true ,false},
 { "runW_downF" ,false ,false ,true ,false},
 { "runW_downB" ,false ,false ,true ,false},
 { "runRW" ,false ,true ,true ,false},
 { "runRW_down" ,false ,false ,true ,false},


 { "tumbleF" ,false ,false ,true ,false },
 { "tumbleB" ,false ,false ,true ,false },
 { "tumbleR" ,false ,false ,true ,false },
 { "tumbleL" ,false ,false ,true ,false },

 { "bind" ,false ,false ,false ,false },


 { "jumpwallF" ,false ,false ,true ,false },
 { "jumpwallB" ,false ,false ,true ,false },
 { "jumpwallL" ,false ,false ,true ,false },
 { "jumpwallR" ,false ,false ,true ,false },


 { "attack1" ,false ,false ,true ,false },
 { "attack1_ret" ,false ,false ,true ,true },
 { "attack2" ,false ,false ,true ,false },
 { "attack2_ret" ,false ,false ,true ,true },
 { "attack3" ,false ,false ,true ,false },
 { "attack3_ret" ,false ,false ,true ,true },
 { "attack4" ,false ,false ,true ,false },
 { "attack4_ret" ,false ,false ,true ,true },
 { "attack5" ,false ,false ,true ,false },


 { "attack_Jump" ,false ,false ,true ,false },
 { "uppercut" ,false ,false ,true ,false },


 { "guard_start" ,false ,false ,true ,false },
 { "guard_idle" ,false ,false ,false ,false },
 { "guard_block1" ,false ,false ,false ,false },
 { "guard_block1_ret",false ,false ,false ,false },
 { "guard_block2" ,false ,false ,false ,false },
 { "guard_cancel" ,false ,false ,false ,false },

 { "blast" ,false ,false ,false ,false },
 { "blast_fall" ,false ,false ,false ,false },
 { "blast_drop" ,false ,false ,false ,false },
 { "blast_stand" ,false ,false ,false ,false },
 { "blast_airmove" ,false ,false ,false ,false },

 { "blast_dagger" ,false ,false ,false ,false },
 { "blast_drop_dagger",false ,false ,false ,false },

 { "damage" ,false ,false ,false ,false },
 { "damage2" ,false ,false ,false ,false },
 { "damage_down" ,false ,false ,false ,false },

 { "taunt" ,true ,false ,false ,false },
 { "bow" ,true ,false ,false ,false },
 { "wave" ,true ,false ,false ,false },
 { "laugh" ,true ,false ,false ,false },
 { "cry" ,true ,false ,false ,false },
 { "dance" ,true ,false ,false ,false },

 { "cancel" ,false ,false ,false ,false },
 { "charge" ,false ,false ,true ,true },
 { "slash" ,false ,false ,true ,false },
 { "jump_slash1" ,false ,false ,true ,false },
 { "jump_slash2" ,false ,false ,true ,false },


 { "lightning" ,false ,false ,false ,false },
 { "stun" ,false ,false ,true ,false }, // 루프되는 스턴


 { "pit" ,false ,false ,false ,false }, // 나락에서 떨어지는 거
 };

/*
static ZFACECOSTUME g_FaceCostume[MAX_FACE_COSTUME] = 
{
	{ "eq_face_04", "eq_face_001" },
	{ "eq_face_01", "eq_face_001" },
	{ "eq_face_02", "eq_face_001" },
	{ "eq_face_03", "eq_face_001" }
};
*/

//	   ÀÌ¸§  ,Äµ½½°¡´É  ,¹Ýº¹	,ÀÌµ¿°ªÀÌ µé¾ûÜÖ´ÂÁE, ÀÌµ¿°ªÀÌ ½ÃÀÛºÎÅÍ ÀÌ¾ûÝö´ÂÁE

static ZANIMATIONINFO g_AnimationInfoTableLower[ZC_STATE_LOWER_END] = {
	{ ""				,true	,true	,false 	,false },
										   
	{ "idle"			,true	,true	,false 	,false },
	{ "idle2"			,true	,true	,false 	,false },
	{ "idle3"			,true	,true	,false 	,false },
	{ "idle4"			,true	,true	,false 	,false },
										   
	{ "run"				,true	,true	,false 	,false },
	{ "runB"			,true	,true	,false 	,false },
	{ "runL"			,true	,true	,false 	,false },
	{ "runR"			,true	,true	,false 	,false },
										   
	{ "jumpU"			,true	,false	,false 	,false },
	{ "jumpD"			,true	,false	,false 	,false },
										   
	{ "die" 			,true	,false	,false 	,false },
	{ "die2" 			,true	,false	,false 	,false },
	{ "die3" 			,true	,false	,false 	,false },
	{ "die4"			,true	,false	,false 	,false },

	{ "runLW"			,false	,true 	,false	,false},
	{ "runLW_down"		,false	,false	,false	,false},
	{ "runW" 			,false	,false	,true	,false},
	{ "runW_downF"		,false	,false	,false	,false},
	{ "runW_downB"		,false	,false	,false	,false},
	{ "runRW" 			,false	,true 	,false	,false},
	{ "runRW_down"		,false	,false 	,false	,false},

	{ "tumbleF"			,false	,false	,false	,false },   
	{ "tumbleB"			,false	,false	,false	,false },   
	{ "tumbleR"			,false	,false	,false	,false },
	{ "tumbleL"			,false	,false	,false	,false },
									  
	{ "bind"			,false	,false	,false	,false },

	{ "jumpwallF"		,false	,false 	,false	,false },
	{ "jumpwallB"		,false	,false 	,false	,false },
	{ "jumpwallL"		,false	,false 	,false	,false },
	{ "jumpwallR"		,false	,false 	,false	,false },

	{ "attack1"			,false	,false 	,true  	,false },
	{ "attack1_ret"		,false	,false 	,true  	,true  },
	{ "attack2"			,false	,false 	,true  	,false },
	{ "attack2_ret"		,false	,false 	,true  	,true  },
	{ "attack3"			,false	,false 	,true  	,false },
	{ "attack3_ret"		,false	,false 	,true  	,true  },
	{ "attack4"			,false	,false 	,true  	,false },
	{ "attack4_ret"		,false	,false 	,true  	,true  },
	{ "attack5"			,false	,false 	,true  	,false },

	{ "attack_Jump"		,false	,false 	,false	,false },
	{ "uppercut"		,false	,false 	,true	,false },

	{ "guard_start"		,false	,false 	,true 	,false },
	{ "guard_idle"		,false	,false 	,false	,false },
	{ "guard_block1"	,false	,false 	,false	,false },
	{ "guard_block1_ret",false	,false 	,false	,false },
	{ "guard_block2"	,false	,false 	,false	,false },
	{ "guard_cancel"	,false	,false 	,false	,false },
												  
	{ "blast"			,false	,false 	,false 	,false },
	{ "blast_fall"		,false	,false 	,false 	,false },
	{ "blast_drop"		,false	,false 	,false 	,false },
	{ "blast_stand"		,false	,false 	,false 	,false },
	{ "blast_airmove"	,false	,false 	,false 	,false },
											  
	{ "blast_dagger"	 ,false ,false 	,false 	,false },
	{ "blast_drop_dagger",false ,false 	,false 	,false },
											  
	{ "damage"			,false	,false 	,false 	,false },
	{ "damage2"			,false	,false 	,false 	,false },
	{ "damage_down"		,false	,false 	,false 	,false },
											  
	{ "taunt"			,true	,false	,false	,false },
	{ "bow"				,true	,false	,false	,false },
	{ "wave"			,true	,false	,false	,false },
	{ "laugh"			,true	,false	,false	,false },
	{ "cry"				,true	,false	,false	,false },
	{ "dance"			,true	,false	,false	,false },
											  
	{ "cancel"			,false	,false 	,false 	,false },
	{ "charge"			,false	,false 	,true  	,true  },
	{ "slash"			,false	,false 	,true  	,false },
	{ "jump_slash1"		,false	,false 	,false	,false },
	{ "jump_slash2"		,false	,false 	,false	,false },

	{ "lightning"		,false	,false 	,false	,false },
	{ "stun"			,false	,false 	,false	,false },	// ·çÇÁµÇ´Â ½ºÅÏ

	{ "pit"				,false	,false 	,false	,false },	// ³ª¶ô¿¡¼­ ¶³¾ûÝö´Â °Å
};

static ZANIMATIONINFO g_AnimationInfoTableUpper[ZC_STATE_UPPER_END] = {
	{ ""				,true	,true	,false	,false },
											   
	{ "attackS"			,false	,false	,false	,false },
	{ "reload"			,false	,false	,false	,false },
	{ "load"			,false	,false	,false	,false },
											   
	{ "guard_start"		,false	,false 	,false	,false },
	{ "guard_idle"		,false	,false 	,false	,false },
	{ "guard_block1"	,false	,false 	,false	,false },
	{ "guard_block1_ret",false	,false 	,false	,false },
	{ "guard_block2"	,false	,false 	,false	,false },
	{ "guard_cancel"	,false	,false 	,false	,false },
};

// teen ¹öÀEº °­Á¦·Î ¹Ù²ãÁØ´Ù~..
bool CheckTeenVersionMesh(RMesh** ppMesh)
{
	RWeaponMotionType type = eq_weapon_etc;

	type = (*ppMesh)->GetMeshWeaponMotionType();

	if( ZApplication::GetGameInterface()->GetTeenVersion() ) {

		if(type==eq_wd_katana) {
			*ppMesh = ZGetWeaponMeshMgr()->Get( "katana_wood" );
			return true;
		}
		else if(type==eq_ws_dagger) {
			*ppMesh = ZGetWeaponMeshMgr()->Get( "dagger_wood" );
			return true;
		}
		else if(type==eq_wd_sword) {
			*ppMesh = ZGetWeaponMeshMgr()->Get( "sword_wood" );
			return true;
		
		}
		else if(type==eq_wd_blade) {
			*ppMesh = ZGetWeaponMeshMgr()->Get( "blade_wood" );
			return true;
		}
	}

	return false;
}

#ifdef _DYNAMIC
void ChangeCharHair(RVisualMesh* pVMesh, MMatchSex nSex, int nHairIndex)
{
	if ((nHairIndex < 0) || (nHairIndex >= MAX_COSTUME_HAIR)) return;
	if (pVMesh == NULL) return;

	char szMeshName[256];
	if (nSex == MMS_MALE) {
		strcpy(szMeshName, g_szHairMeshName[nHairIndex][MMS_MALE].c_str());
	}
	else {
		strcpy(szMeshName, g_szHairMeshName[nHairIndex][MMS_FEMALE].c_str());
	}

	// ³ªÁß¿¡ hair¸¦ µû·Î ¸¸µé¾î¾ß ÇÒµí..
	pVMesh->SetParts(eq_parts_head, szMeshName, nullptr);
}
#endif

#ifdef _DYNAMIC
void ChangeEquipAvatarParts(RVisualMesh* pVMesh, const unsigned long int* pItemID, MMatchSex nSex, int nHairIndex)
{
	pVMesh->ClearParts();

	char* szMeshName;
	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(pItemID[MMCIP_AVATAR]);
	if (pDesc != NULL)
	{
		if (pDesc->GetDyn() != nullptr)
		{
			RMesh* playerMesh = nullptr;
			if (_stricmp(pVMesh->GetMesh()->GetName(), "heroman1") == 0)
			{
				playerMesh = ZGetMeshMgr()->Get("heroman1");
			}
			else
			{
				playerMesh = ZGetMeshMgr()->Get("herowoman1");
			}
			if (playerMesh->m_parts_mgr->Find(pDesc->m_szDyn) == false)
			{
				std::string szFileName = pDesc->m_szDyn;
				std::string fullPath = "";

				// KIỂM TRA: Nếu trong XML đã viết đường dẫn đầy đủ (có model/) thì dùng luôn
				if (szFileName.find("model/") != std::string::npos)
				{
					fullPath = szFileName;
				}
				else
				{
					// Nếu chỉ là đường dẫn tương đối (custom/...) thì mới cộng thêm folder giới tính
					bool bIsWoman = false;
					if (playerMesh->GetFileName())
					{
						std::string meshName = playerMesh->GetFileName();
						if (meshName.find("woman") != std::string::npos)
							bIsWoman = true;
					}

					if (bIsWoman) fullPath = "model/woman/" + szFileName;
					else          fullPath = "model/man/" + szFileName;
				}

				// Nạp vào manager
				playerMesh->m_parts_mgr->Add((char*)fullPath.c_str());

				// Log kiểm tra để chắc chắn không bị nhân đôi nữa
				// mlog("Final Path: %s\n", fullPath.c_str());
			}
		}
		szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szHeadMeshName;
		if (strlen(szMeshName) > 0)	pVMesh->SetParts(eq_parts_head, szMeshName, pDesc->GetDyn());
		else							ChangeCharHair(pVMesh, nSex, nHairIndex);

		szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szChestMeshName;
		if (strlen(szMeshName) > 0)	pVMesh->SetParts(eq_parts_chest, szMeshName, pDesc->GetDyn());
		else							pVMesh->SetBaseParts(eq_parts_chest);

		szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szHandMeshName;
		if (strlen(szMeshName) > 0)	pVMesh->SetParts(eq_parts_hands, szMeshName, pDesc->GetDyn());
		else							pVMesh->SetBaseParts(eq_parts_hands);

		szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szLegsMeshName;
		if (strlen(szMeshName) > 0)	pVMesh->SetParts(eq_parts_legs, szMeshName, pDesc->GetDyn());
		else							pVMesh->SetBaseParts(eq_parts_legs);

		szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szFeetMeshName;
		if (strlen(szMeshName) > 0)	pVMesh->SetParts(eq_parts_feet, szMeshName, pDesc->GetDyn());
		else							pVMesh->SetBaseParts(eq_parts_feet);
	}

	//pVMesh->SetBaseParts(eq_parts_face);
}
#endif

void ChangeEquipParts(RVisualMesh* pVMesh, const unsigned long int* pItemID)
{
	pVMesh->ClearParts();

	struct _ZPARTSPAIR
	{
		_RMeshPartsType			meshparts;
		MMatchCharItemParts		itemparts;
	};

	static _ZPARTSPAIR PartsPair[] = 
	{
		{eq_parts_head,		MMCIP_HEAD},
		{eq_parts_chest,	MMCIP_CHEST},
		{eq_parts_hands,	MMCIP_HANDS},
		{eq_parts_legs,		MMCIP_LEGS},
		{eq_parts_feet,		MMCIP_FEET}
	};

#ifdef _DYNAMIC
	for (int i = 0; i < 5; i++) {
		if (pItemID[PartsPair[i].itemparts] != 0)
		{
			MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(pItemID[PartsPair[i].itemparts]);
			if (pDesc != NULL)
			{
				if (pDesc->GetDyn() != nullptr)
				{
					RMesh* playerMesh = nullptr;
					if (_stricmp(pVMesh->GetMesh()->GetName(), "heroman1") == 0)
					{
						playerMesh = ZGetMeshMgr()->Get("heroman1");
					}
					else
					{
						playerMesh = ZGetMeshMgr()->Get("herowoman1");
					}
					if (playerMesh->m_parts_mgr->Find(pDesc->m_szDyn) == false)
					{
						std::string szPath = pDesc->m_szDyn;
						std::string fullPath = "";

						// 1. Kiểm tra xem người dùng đã khai báo full đường dẫn chưa
						// Nếu trong XML đã có chữ "model/" thì lấy luôn chuỗi đó
						if (szPath.find("model/") != std::string::npos || szPath.find("model\\") != std::string::npos)
						{
							fullPath = szPath;
						}
						else
						{
							// 2. Nếu chưa có "model/", mới bắt đầu ghép theo giới tính (Logic cũ của mình)
							bool bIsWoman = false;
							if (playerMesh->GetFileName() && std::string(playerMesh->GetFileName()).find("woman") != std::string::npos)
								bIsWoman = true;

							fullPath = bIsWoman ? "model/woman/" + szPath : "model/man/" + szPath;
						}

						// 3. Chuẩn hóa dấu xuyệt (Rất quan trọng cho thư mục sâu)
						std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

						playerMesh->m_parts_mgr->Add((char*)fullPath.c_str());
					}
				}
				pVMesh->SetParts(PartsPair[i].meshparts, pDesc->m_pMItemName->Ref().m_szMeshName, pDesc->GetDyn());
			}
		}
		else {
			pVMesh->SetBaseParts(PartsPair[i].meshparts);
		}
	}

	pVMesh->SetBaseParts(eq_parts_face);
#endif
}

void ChangeCharFace(RVisualMesh* pVMesh, MMatchSex nSex, int nFaceIndex)
{
#ifdef _DYNAMIC
	if ((nFaceIndex < 0) || (nFaceIndex >= MAX_COSTUME_FACE)) return;
	if (pVMesh == NULL) return;

	char szMeshName[256];

	if (nSex == MMS_MALE)
	{
		strcpy(szMeshName, g_szFaceMeshName[nFaceIndex][MMS_MALE].c_str());
	}
	else
	{
		strcpy(szMeshName, g_szFaceMeshName[nFaceIndex][MMS_FEMALE].c_str());
	}

	pVMesh->SetParts(eq_parts_face, szMeshName, nullptr);
#endif
}

void ZChangeCharParts(RVisualMesh* pVMesh, MMatchSex nSex, int nHair, int nFace, const unsigned long int* pItemID)
{
	if (pVMesh == NULL) {
		_ASSERT(0);
		return;
	}

	if( pItemID[MMCIP_AVATAR] != 0 ) {
		ChangeEquipAvatarParts(pVMesh, pItemID, nSex, nHair);
		pVMesh->SetSkipRenderFaceParts(true);
	} 
	else {
		ChangeEquipParts(pVMesh, pItemID);

		if (pItemID[MMCIP_HEAD] == 0) {
			ChangeCharHair(pVMesh, nSex, nHair);
		}

		pVMesh->SetSkipRenderFaceParts(false);

		ChangeCharFace(pVMesh, nSex, nFace);
	}
}


void ZChangeCharPartsNoAvatar(RVisualMesh* pVMesh, MMatchSex nSex, int nHair, int nFace, const unsigned long int* pItemID)
{
	if (pVMesh == NULL) {
		_ASSERT(0);
		return;
	}

	if( pItemID[MMCIP_AVATAR] != 0 ) {
		ChangeEquipAvatarParts(pVMesh, pItemID, nSex, nHair);
		pVMesh->SetSkipRenderFaceParts(true);
	} 
	else {
		ChangeEquipParts(pVMesh, pItemID);		// hair, faceº¸´Ù Àåºñ¸¦ ¸ÕÀE¹Ù²ã¾ß ÇÑ´Ù.
		if (pItemID[MMCIP_HEAD] == 0) {
			ChangeCharHair(pVMesh, nSex, nHair);
		}

		pVMesh->SetSkipRenderFaceParts(false);

		ChangeCharFace(pVMesh, nSex, nFace);
	}
}

void ZChangeCharWeaponMesh(RVisualMesh* pVMesh, unsigned long int nWeaponID)
{
	if( pVMesh ) 
	{
		if (nWeaponID == 0) 
		{
			RWeaponMotionType type = eq_ws_dagger;
			pVMesh->AddWeapon(type , NULL);
			pVMesh->SelectWeaponMotion(type);

			return;	// id°¡ 0ÀÌ¸E¾Æ¹«°Íµµ Âø¿EÏÁE¾Ê´Â´Ù.
		}


		RWeaponMotionType type = eq_weapon_etc;
		MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nWeaponID);
		
		if (pDesc == NULL)
		{
//			_ASSERT(0);
			return;
		}

		RMesh* pMesh = ZGetWeaponMeshMgr()->Get( pDesc->m_pMItemName->Ref().m_szMeshName );

		if( pMesh ) 
		{
			type = pMesh->GetMeshWeaponMotionType();
			CheckTeenVersionMesh(&pMesh);
			pVMesh->AddWeapon(type , pMesh);
			pVMesh->SelectWeaponMotion(type);
		}
	}

}


///////////////////////////////////////////////////////////////////////////////////////
MImplementRTTI(ZCharacter, ZCharacterObject);

ZCharacter::ZCharacter() : ZCharacterObject(), m_DirectionLower(1, 0, 0), m_DirectionUpper(1, 0, 0), m_TargetDir(1, 0, 0)
{ 
	m_nGhost = 0; // Custom: Ghost Mode - Desperate
	m_fPreMaxHP = 0.f;
	m_fPreMaxAP = 0.f;

	m_Status.MakeCrc();

	m_nMoveMode.Set_MakeCrc(MCMM_RUN);
	m_nMode.Set_MakeCrc(MCM_OFFENSIVE);
	m_nState.Set_MakeCrc(MCS_STAND);

	m_nVMID.Set_MakeCrc(-1);
//	m_fLastUpdateTime = 0.f;

	m_fLastShotTime = 0.0f;
	
	m_slotInfo.Ref().m_nSelectSlot = 0;
	m_slotInfo.MakeCrc();

//	m_dwBackUpTime = 0;

	m_killInfo.Ref().m_fLastKillTime = 0;
	m_killInfo.Ref().m_nKillsThisRound = 0;
	m_killInfo.MakeCrc();

	m_damageInfo.Ref().m_LastDamageType = ZD_NONE;
	m_damageInfo.Ref().m_LastDamageWeapon = MWT_NONE;
	m_damageInfo.Ref().m_LastDamageDot = 0.f;
	m_damageInfo.Ref().m_LastDamageDistance = 0.f;
	m_damageInfo.Ref().m_LastThrower = MUID(0,0);
	m_damageInfo.Ref().m_nStunType = ZST_NONE;
	m_damageInfo.Ref().m_dwLastDamagedTime = 0;
	m_damageInfo.MakeCrc();

	m_SpMotion = ZC_STATE_TAUNT;
	
	m_nTeamID.Set_MakeCrc(MMT_ALL);

	m_nBlastType.Set_MakeCrc(0);
	
	m_AniState_Upper.Set_MakeCrc(ZC_STATE_UPPER_NONE);
	m_AniState_Lower.Set_MakeCrc(ZC_STATE_LOWER_NONE);

//	m_pAnimationInfo_Lower=&g_AnimationInfoTableLower[ZC_STATE_LOWER_IDLE1];
//	m_pAnimationInfo_Lower=&g_AnimationInfoTableUpper[ZC_STATE_UPPER_NONE];

	m_pAnimationInfo_Lower=NULL;
	m_pAnimationInfo_Upper=NULL;

	// m_vProxyPosition = m_vProxyDirection = rvector(0.0f,0.0f,0.0f);

/*	for(int i=0;i<6;i++) {
		m_t_parts[i]  = 2;//³²ÀÚ´Â 2¹EÄÚµå¸ðµ¨ÀÌ ÃÊ±â¸ðµ¨..
		m_t_parts2[i] = 0;
	}
*/
	m_fLastReceivedTime=0;

	/*
	m_fAveragePingTime=0;
	m_fAccumulatedTimeError=0;
	m_nTimeErrorCount=0;
	*/

	m_fLastValidTime = 0.0f;
	m_Accel.Set_MakeCrc(rvector(0.0f, 0.0f, 0.0f));
	m_bRendered = false;

	m_nWallJumpDir.Set_MakeCrc(0);
	m_fChargedFreeTime.Set_MakeCrc(0);

	/*
	m_fClimbZ = 0.f;

	m_ClimbDir.x = 0.f;
	m_ClimbDir.y = 0.f;
	m_ClimbDir.z = 0.f;
	*/

	m_RealPositionBefore.Set_MakeCrc( rvector(0.f,0.f,0.f));
	m_AnimationPositionDiff.Set_MakeCrc( rvector(0.f,0.f,0.f));

	m_fGlobalHP = 0.f;
	m_nReceiveHPCount = 0;

	//m_FloorPlane.a = 0.f;
	//m_FloorPlane.b = 0.f;
	//m_FloorPlane.c = 0.f;	
	//m_FloorPlane.d = 0.f;

	m_fAttack1Ratio.Set_MakeCrc(1.f);

	m_nWhichFootSound = 0;

	m_fTimeOffset = 0;
	m_nTimeErrorCount = 0;
	m_fAccumulatedTimeError = 0;

//	RegisterModules();

	m_pModule_HPAP				= new ZModule_HPAP;
	m_pModule_Resistance		= new ZModule_Resistance;
	m_pModule_FireDamage		= new ZModule_FireDamage;
	m_pModule_ColdDamage		= new ZModule_ColdDamage;
	m_pModule_PoisonDamage		= new ZModule_PoisonDamage;
	m_pModule_LightningDamage	= new ZModule_LightningDamage;
	m_pModule_HealOverTime		= new ZModule_HealOverTime;
	m_pModule_QuestStatus = NULL;
	
	AddModule(m_pModule_HPAP);
	AddModule(m_pModule_Resistance);
	AddModule(m_pModule_FireDamage);
	AddModule(m_pModule_ColdDamage);
	AddModule(m_pModule_PoisonDamage);
	AddModule(m_pModule_LightningDamage);
	AddModule(m_pModule_HealOverTime);

	// Äù½ºÆ® °EÃ °ÔÀÓÅ¸ÀÔÀÏ °æ¿E¡¸¸ QuestStatus »ý¼º, µûÓÏ
	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		m_pModule_QuestStatus = new ZModule_QuestStatus();
		AddModule(m_pModule_QuestStatus);
	}

	m_Collision.SetRadius(CHARACTER_RADIUS-2);
	m_Collision.SetHeight(CHARACTER_HEIGHT);

	m_bCharged = new MProtectValue<bool>;
	m_bCharged->Set_MakeCrc(false);
	m_bCharging = new MProtectValue<bool>;
	m_bCharging->Set_MakeCrc(false);

	m_pMdwInvincibleStartTime = new MProtectValue<DWORD>;
	m_pMdwInvincibleStartTime->Set_MakeCrc(0);
	m_pMdwInvincibleDuration = new MProtectValue<DWORD>;
	m_pMdwInvincibleDuration->Set_MakeCrc(0);

	ZCharaterStatusBitPacking uState;
	uState.dwFlagsPublic = 1;
	uState.m_bSniping = false; // Custom: Snipers
	uState.m_bFrozen = false;

	uState.m_bTagger = false; // Custom: Moved it here

	// Custom: Infected
	m_bInfected = false;
	// Custom: DamageTihsRound
	m_nDamageThisRound = 0;
	// Custom: GunGame
	// Description: Properties derived from MatchServer for updating purposes
	memset(m_nGunGameWeaponID, 0, sizeof(m_nGunGameWeaponID));
	m_nGunGameWeaponLevel = 0;

	m_dwLastBasicInfoTime = 0;

/*	m_bAdminHide(false)
	m_bIsLowModel = false;
	m_bDie = false;
	m_bTagger = false;
	//	m_bAutoDir = false;
	m_bStylishShoted = false;
	m_bStun = false;
	m_bDamaged = false;
	m_bFallingToNarak = false;
*/
	m_dwStatusBitPackingValue.Set(uState);

	m_pMUserAndClanName	= new MProtectValue<ZUserAndClanName>;
	m_pMUserAndClanName->Ref().m_szUserName[0] = 0;
	m_pMUserAndClanName->Ref().m_szUserAndClanName[0] = 0;
	m_pMUserAndClanName->MakeCrc();
	//m_szUserName[0]=0;
	//m_szUserAndClanName[0]=0;
	nHunter_LastWeapons = 0;
	m_uidLastTarget = MUID(0, 0);
	MeshesFinishedLoading = false;
	m_bSBTest = false;
	m_dwSBTestStart = 0;

	SetInvincibleTime(0);
#ifdef _HITSCOUNT
	m_nHits = 0;
#endif

#ifdef _ZGUARD
	st_nLastPeerReload = 0;
	st_nLastPeerDash = 0;
	st_nLastSlashTime = 0;
	nLastDashX = 0.f;
	nType = 0;

	m_nKey = 0;
	m_nKeyAlt = 0;

	// --- KHỞI TẠO BIẾN ANTI-MACRO MELEE (CHÉM) ---
	m_nLastQPCTime.QuadPart = 0;
	m_nHistoryIdx = 0;
	m_nMacroWarningCount = 0;
	for (int i = 0; i < 5; i++) {
		m_fIntervalHistory[i] = 0.0;
	}

	// --- KHỞI TẠO BIẾN ANTI-MACRO DASH (LƯỚT) ---
	m_nLastDashQPCTime.QuadPart = 0;
	m_nDashHistoryIdx = 0;
	m_nDashMacroWarningCount = 0;
	for (int i = 0; i < 5; i++) {
		m_fDashIntervalHistory[i] = 0.0;
	}
#endif

}

//void ZCharacter::RegisterModules()
//{
//	ZObject::RegisterModules();
//
//	// ¸ðµEµûÓÏÇÑ´Ù
//	RegisterModule(&m_Module_HPAP);
//	RegisterModule(&m_Module_Movable);
//	RegisterModule(&m_Module_Resistance);
//
//	RegisterModule(&m_Module_FireDamage);
//	RegisterModule(&m_Module_ColdDamage);
//	RegisterModule(&m_Module_PoisonDamage);
//	RegisterModule(&m_Module_LightningDamage);
//}

ZCharacter::~ZCharacter()
{
	//mmemory proxy
	RemoveModule(m_pModule_HPAP);
	RemoveModule(m_pModule_Resistance);
	RemoveModule(m_pModule_FireDamage);
	RemoveModule(m_pModule_ColdDamage);
	RemoveModule(m_pModule_PoisonDamage);
	RemoveModule(m_pModule_LightningDamage);
	RemoveModule(m_pModule_HealOverTime);
	RemoveModule(m_pModule_QuestStatus);

	delete m_pModule_HPAP;
	delete m_pModule_Resistance;
	delete m_pModule_FireDamage;
	delete m_pModule_ColdDamage;
	delete m_pModule_PoisonDamage;
	delete m_pModule_LightningDamage;
	delete m_pModule_HealOverTime;
	
	//mmemory proxy
	SAFE_DELETE(m_bCharged);
	SAFE_DELETE(m_bCharging);
	SAFE_DELETE(m_pMdwInvincibleDuration);
	SAFE_DELETE(m_pMdwInvincibleStartTime);
	//SAFE_DELETE(m_dwStatusBitPackingValue);
	SAFE_DELETE(m_pMUserAndClanName);

	SAFE_DELETE(m_pModule_QuestStatus);

	EmptyHistory();

	Destroy();
}

__forceinline void ZCharacter::InitHPAP()
{	
	m_pModule_HPAP->SetMaxHP(GetMaxHP());
	m_pModule_HPAP->SetMaxAP(GetMaxAP());

	m_pModule_HPAP->SetHP(GetMaxHP());
	m_pModule_HPAP->SetAP(GetMaxAP());

	//if( ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed( MMOD_VANILLA ) )
	//{
	//	if(GetMaxHP() > 125)
	//	{
	//		m_pModule_HPAP->SetMaxHP(125);
	//		m_pModule_HPAP->SetHP(125);
	//	} 
	//	if(GetMaxAP() > 100)
	//	{
	//		m_pModule_HPAP->SetMaxAP(100);
	//		m_pModule_HPAP->SetAP(100);
	//	}

	//	//m_pModule_HPAP->SetMaxHP(125);
	//	//m_pModule_HPAP->SetHP(125);

	//	//m_pModule_HPAP->SetMaxAP(100);
	//	//m_pModule_HPAP->SetAP(100);
	//}

	/*
	m_pModule_HPAP->SetMaxHP(m_Property.fMaxHP.Ref());
	m_pModule_HPAP->SetMaxAP(m_Property.fMaxAP.Ref());

	m_pModule_HPAP->SetHP(m_Property.fMaxHP.Ref());
	m_pModule_HPAP->SetAP(m_Property.fMaxAP.Ref());
	*/
}

void ZCharacter::EmptyHistory()
{
	if (m_bInitialized == false) return;

	while(m_BasicHistory.size())
	{
		delete *m_BasicHistory.begin();
		m_BasicHistory.erase(m_BasicHistory.begin());
	}
	//while(m_HPHistory.size())
	//{
	//	delete *m_HPHistory.begin();
	//	m_HPHistory.erase(m_HPHistory.begin());
	//}
}

void ZCharacter::Stop()
{
	//SetState(ZC_STATE_IDLE);
	SetAnimationLower(ZC_STATE_LOWER_IDLE1);
}

bool ZCharacter::IsTeam(ZCharacter* pChar)
{
	if(pChar) {
		if( pChar->GetTeamID() == GetTeamID()) {
			return true;
		}
	}
	return false;
}

bool ZCharacter::IsMoveAnimation()
{
	return g_AnimationInfoTableLower[m_AniState_Lower.Ref()].bMove;
}

// Custom: Ghost Mode - Desperate
bool ZCharacter::IsMoveGhost()
{
	return g_AnimationInfoTableGhost[m_AniState_Lower.Ref()].bMove;
}

void ZCharacter::SetAnimation(char *AnimationName,bool bEnableCancel,int time)
{
	if (m_bInitialized == false) return;

	SetAnimation(ani_mode_lower,AnimationName,bEnableCancel,time);
}

void ZCharacter::SetAnimation(RAniMode mode,char *AnimationName,bool bEnableCancel,int time)
{
	if (m_bInitialized == false) 
		return;

	if(!m_pVMesh) 
		return;

	if(time) {
		m_pVMesh->SetReserveAnimation(mode,AnimationName,time);
	}
	else {
		m_pVMesh->SetAnimation(mode,AnimationName,bEnableCancel);
	}
}

//#define TRACE_ANIMATION

// ½Ã°£ test À§ÇÑ º¯¼E
DWORD g_dwLastAnimationTime=timeGetTime();

void ZCharacter::SetAnimationLower(ZC_STATE_LOWER nAni)
{
	if (m_bInitialized == false) return;
	if ((IsDie()) && (IsHero())) return;

	if(nAni==m_AniState_Lower.Ref()) return;
	_ASSERT(nAni>=0 && nAni<ZC_STATE_LOWER_END);

	if( nAni< 0 || nAni >= ZC_STATE_LOWER_END )
	{
		return;
	}

//	if(!m_pAnimationInfo_Lower || m_pAnimationInfo_Lower->bEnableCancel)
//	if( m_pVMesh->m_isPlayDone || m_pAnimationInfo_Upper->bEnableCancel )
	//{

	// Custom: Ignore List
	if( ZGetGame()->m_pMyCharacter->GetUID() != this->GetUID() && ZGetGameClient()->IsUserIgnored( this->GetUserName() ) && (nAni == ZC_STATE_TAUNT || nAni == ZC_STATE_CRY || nAni == ZC_STATE_LAUGH) )
		return;
	//else

		m_AniState_Lower.Set_CheckCrc(nAni);
		m_pAnimationInfo_Lower=&g_AnimationInfoTableLower[nAni];
		
		SetAnimation(ani_mode_lower,m_pAnimationInfo_Lower->Name,m_pAnimationInfo_Lower->bEnableCancel,0);

		// ¿òÁ÷ÀÌ´Â ¾Ö´Ï¸ÞÀÌ¼ÇÀÌ ¿¬°áµÉ¶§´Â ¿©±â¿¡ Ãß°¡ÇØÁÖ¾ûÚß ÇÑ´Ù
//		if(IsMoveAnimation())
		{
			if(!g_AnimationInfoTableLower[nAni].bContinuos)
				m_RealPositionBefore.Set_CheckCrc( rvector(0,0,0));
		}

	//m_RealPositionBefore=m_RealPosition=m_Position;


#ifdef TRACE_ANIMATION
	{
		DWORD curtime = timeGetTime();
		if(ZGetGame()->m_pMyCharacter==this)
			mlog("animation - %d %s   - %d frame , interval %d \n",nAni,
			m_pVMesh->GetFrameInfo(ani_mode_lower)->m_pAniSet->GetName(),m_pVMesh->GetFrameInfo(ani_mode_lower)->m_nFrame,
			curtime-g_dwLastAnimationTime);
		g_dwLastAnimationTime = curtime;
	}
#endif

//	m_pVMesh->m_nFrame[0]=0;
	//*/
}

void ZCharacter::SetAnimationUpper(ZC_STATE_UPPER nAni)
{
	if (m_bInitialized == false) return;
	if ((IsDie()) && (IsHero())) return;

	if(nAni==m_AniState_Upper.Ref()) 	return;

	_ASSERT(nAni>=0 && nAni<ZC_STATE_UPPER_END);

	if( nAni< 0 || nAni >= ZC_STATE_UPPER_END )
	{
		return;
	}

#ifdef TRACE_ANIMATION
	{
		DWORD curtime = timeGetTime();
		mlog("upper Animation Index : %d %s @ %d \n", nAni ,g_AnimationInfoTableUpper[nAni].Name,curtime-g_dwLastAnimationTime);
		if(m_AniState_Upper.Ref()==3 && nAni==0)
		{
			bool a=false;
		}
		g_dwLastAnimationTime = curtime;
	}
#endif

//	if(!m_pAnimationInfo_Upper || m_pAnimationInfo_Upper->bEnableCancel)
	//if( m_AniState_Upper != ZC_STATE_UPPER_NONE )
	//{
	//	if( m_pVMesh->m_isPlayDone || m_pAnimationInfo_Upper->bEnableCancel )
	//	{	
			m_AniState_Upper.Set_CheckCrc(nAni);
			m_pAnimationInfo_Upper=&g_AnimationInfoTableUpper[nAni];
			if( m_pAnimationInfo_Upper == NULL || m_pAnimationInfo_Upper->Name == NULL )
			{
				mlog("Fail to Get Animation Info.. Ani Index : [%d]\n", nAni );
				return;
			}
			SetAnimation(ani_mode_upper,m_pAnimationInfo_Upper->Name,m_pAnimationInfo_Upper->bEnableCancel,0);
	//	}
	//}
}

void ZCharacter::UpdateLoadAnimation()
{
	if (m_bInitialized == false) return;

	if(m_pAnimationInfo_Lower)
	{
		SetAnimation(m_pAnimationInfo_Lower->Name,m_pAnimationInfo_Lower->bEnableCancel,0);
		SetAnimationUpper(ZC_STATE_UPPER_NONE);
		SetAnimationUpper(ZC_STATE_UPPER_LOAD);
		m_dwStatusBitPackingValue.Ref().m_bPlayDone_upper = false;
	}
}


// ¸ð¼ÇÀÇ °¨µµ¶§¹®¿¡ ¼ö½Ã·Î °áÁ¤
// ÇÏ¹Ý½Å Ä«¸Þ¶ó¹æÇâÀ¸·Î º¸°£µ¹¸®±E
// »ó¹Ý½Å ÆÄÆ® ¸ñÇ¥ º¤ÅÍ ¼³Á¤

bool CheckVec(rvector& v1,rvector& v2) {
	if( fabs(v1.x - v2.x) < 0.03f )
		if( fabs(v1.y - v2.y) < 0.03f )
			if( fabs(v1.z - v2.z) < 0.03f )
				return false;
	return true;
}

bool g_debug_rot = false;

//Custom: Character Posture
void ZCharacter::UpdateDirection(float fDelta, const v3& Direction)
{
	if (m_bInitialized == false) return;

	if (IsDie())
	{
		m_pVMesh->SetRotXYZ(rvector(0, 0, 0));
		return;
	}
	if (ZGetGame()->GetMatch()->IsClassiMode())
	{
		if ((m_AniState_Lower.Ref() == ZC_STATE_LOWER_IDLE1) ||
			(m_AniState_Lower.Ref() == ZC_STATE_LOWER_RUN_FORWARD) ||
			(m_AniState_Lower.Ref() == ZC_STATE_LOWER_RUN_BACK))
		{

			m_Direction = m_TargetDir;

			rvector targetdir = m_TargetDir;
			targetdir.z = 0;
			Normalize(targetdir);

			rvector dir = m_Accel.Ref();
			dir.z = 0;

			if (Magnitude(dir) < 10.f) {
				dir = targetdir;
			}
			else
				Normalize(dir);

			bool bInversed = false;

			if (DotProduct(targetdir, dir) < -cos(PI_FLOAT / 4.f) + 0.01f)
			{
				dir = -dir;
				bInversed = true;
			}

			float fAngleLower = GetAngleOfVectors(dir, m_DirectionLower);
			rmatrix mat;

#define ROTATION_SPEED	400.f

			if (fAngleLower > 5.f / 180.f * PI_FLOAT)
			{
				D3DXMatrixRotationZ(&mat, max(-ROTATION_SPEED * fDelta / 180.f * PI_FLOAT, -fAngleLower));
				m_DirectionLower = m_DirectionLower * mat;
			}

			if (fAngleLower < -5.f / 180.f * PI_FLOAT)
			{
				D3DXMatrixRotationZ(&mat, min(ROTATION_SPEED * fDelta / 180.f * PI_FLOAT, -fAngleLower));
				m_DirectionLower = m_DirectionLower * mat;
			}

			float fAngle = GetAngleOfVectors(m_TargetDir, m_DirectionLower);

			if (fAngle < -65.f / 180.f * PI_FLOAT)
			{
				fAngle = -65.f / 180.f * PI_FLOAT;
				D3DXMatrixRotationZ(&mat, -65.f / 180.f * PI_FLOAT);
				m_DirectionLower = m_Direction * mat;
			}

			if (fAngle >= 65.f / 180.f * PI_FLOAT)
			{
				fAngle = 65.f / 180.f * PI_FLOAT;
				D3DXMatrixRotationZ(&mat, 65.f / 180.f * PI_FLOAT);
				m_DirectionLower = m_Direction * mat;
			}

			m_pVMesh->SetRotX(-fAngle * 180 / PI_FLOAT * .9f);
			m_pVMesh->SetRotY((m_TargetDir.z + 0.05f) * 50.f);
		}
	}
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bPosture)
	{
		if ((m_AniState_Lower.Ref() == ZC_STATE_LOWER_IDLE1) ||
			(m_AniState_Lower.Ref() == ZC_STATE_LOWER_RUN_FORWARD) ||
			(m_AniState_Lower.Ref() == ZC_STATE_LOWER_RUN_BACK))
		{

			m_Direction = m_TargetDir;

			rvector targetdir = m_TargetDir;
			targetdir.z = 0;
			Normalize(targetdir);

			rvector dir = m_Accel.Ref();
			dir.z = 0;

			if (Magnitude(dir) < 10.f) {
				dir = targetdir;
			}
			else
				Normalize(dir);

			bool bInversed = false;

			if (DotProduct(targetdir, dir) < -cos(PI_FLOAT / 4.f) + 0.01f)
			{
				dir = -dir;
				bInversed = true;
			}

			float fAngleLower = GetAngleOfVectors(dir, m_DirectionLower);
			rmatrix mat;

#define ROTATION_SPEED	400.f

			if (fAngleLower > 5.f / 180.f * PI_FLOAT)
			{
				D3DXMatrixRotationZ(&mat, max(-ROTATION_SPEED * fDelta / 180.f * PI_FLOAT, -fAngleLower));
				m_DirectionLower = m_DirectionLower * mat;
			}

			if (fAngleLower < -5.f / 180.f * PI_FLOAT)
			{
				D3DXMatrixRotationZ(&mat, min(ROTATION_SPEED * fDelta / 180.f * PI_FLOAT, -fAngleLower));
				m_DirectionLower = m_DirectionLower * mat;
			}

			float fAngle = GetAngleOfVectors(m_TargetDir, m_DirectionLower);

			if (fAngle < -65.f / 180.f * PI_FLOAT)
			{
				fAngle = -65.f / 180.f * PI_FLOAT;
				D3DXMatrixRotationZ(&mat, -65.f / 180.f * PI_FLOAT);
				m_DirectionLower = m_Direction * mat;
			}

			if (fAngle >= 65.f / 180.f * PI_FLOAT)
			{
				fAngle = 65.f / 180.f * PI_FLOAT;
				D3DXMatrixRotationZ(&mat, 65.f / 180.f * PI_FLOAT);
				m_DirectionLower = m_Direction * mat;
			}

			m_pVMesh->SetRotX(-fAngle * 180 / PI_FLOAT * .9f);
			m_pVMesh->SetRotY((m_TargetDir.z + 0.05f) * 50.f);
		}
	}
}


void ZCharacter::UpdateMotion(float fDelta)
{
	if (m_bInitialized == false) return;
	if (IsDie()) { 

		m_pVMesh->SetRotXYZ(rvector(0, 0, 0));

		return;
	}


	if ((m_AniState_Lower.Ref() == ZC_STATE_LOWER_IDLE1) ||
		(m_AniState_Lower.Ref() == ZC_STATE_LOWER_RUN_FORWARD) ||
		(m_AniState_Lower.Ref() == ZC_STATE_LOWER_RUN_BACK))
	{

		m_Direction = m_TargetDir;

		rvector targetdir = m_TargetDir;
		targetdir.z = 0;
		Normalize(targetdir);

		rvector dir = m_Accel.Ref();
		dir.z = 0;

		if (Magnitude(dir) < 10.f) {
			dir = targetdir;
		}
		else
			Normalize(dir);

		bool bInversed = false;
		if (DotProduct(targetdir, dir) < -cos(PI_FLOAT / 4.f) + 0.01f)
		{
			dir = -dir;
			bInversed = true;
		}

	
		float fAngleLower = GetAngleOfVectors(dir, m_DirectionLower);
		rmatrix mat;

#define ROTATION_SPEED	400.f


		if (fAngleLower > 5.f / 180.f * PI_FLOAT)
		{
			D3DXMatrixRotationZ(&mat, max(-ROTATION_SPEED * fDelta / 180.f * PI_FLOAT, -fAngleLower));
			m_DirectionLower = m_DirectionLower * mat;
		}

		if (fAngleLower < -5.f / 180.f * PI_FLOAT)
		{
			D3DXMatrixRotationZ(&mat, min(ROTATION_SPEED * fDelta / 180.f * PI_FLOAT, -fAngleLower));
			m_DirectionLower = m_DirectionLower * mat;
		}

		float fAngle = GetAngleOfVectors(m_TargetDir, m_DirectionLower);

		if (fAngle < -65.f / 180.f * PI_FLOAT)
		{
			fAngle = -65.f / 180.f * PI_FLOAT;
			D3DXMatrixRotationZ(&mat, -65.f / 180.f * PI_FLOAT);
			m_DirectionLower = m_Direction * mat;
		}

		if (fAngle >= 65.f / 180.f * PI_FLOAT)
		{
			fAngle = 65.f / 180.f * PI_FLOAT;
			D3DXMatrixRotationZ(&mat, 65.f / 180.f * PI_FLOAT);
			m_DirectionLower = m_Direction * mat;
		}

		m_pVMesh->SetRotX(-fAngle * 180 / PI_FLOAT * .9f);

		m_pVMesh->SetRotY((m_TargetDir.z + 0.05f) * 50.f);
	}
	else
	{
		m_Direction = m_TargetDir;
		m_DirectionLower = m_Direction;
		m_pVMesh->SetRotXYZ(rvector(0, 0, 0));
	}
}

void GetDTM(bool* pDTM,int mode,bool isman)
{
//	³²ÀÚ ¿EÁÂ ¹« ½Ö
//	¿©ÀÚ ÁÂ ¿EÁÂ ¿E

	if(!pDTM) return;

//  ¿©ÀÚµµ ¸ð¼ÇÀÌ °°¾ÆÁ³´Ù..

//	if(isman) {

		     if(mode==0) { pDTM[0]=true; pDTM[1]=false; }
		else if(mode==1) { pDTM[0]=false;pDTM[1]=true;  }
		else if(mode==2) { pDTM[0]=false;pDTM[1]=false; }
		else if(mode==3) { pDTM[0]=true; pDTM[1]=true;  }
/*
	}
	else {

		     if(mode==0) { pDTM[0]=false;pDTM[1]=true;  }
		else if(mode==1) { pDTM[0]=true; pDTM[1]=false; }
		else if(mode==2) { pDTM[0]=false;pDTM[1]=true;  }
		else if(mode==3) { pDTM[0]=true; pDTM[1]=false; }
	}
*/
}

void ZCharacter::CheckDrawWeaponTrack()
{
	if(m_pVMesh==NULL) return;

	bool bDrawTracks = false;//Ä® ±ËÀûÀ» ±×¸±°ÍÀÎ°¡?

	if( ( m_pVMesh->GetSelectWeaponMotionType() == eq_wd_katana ) || 
		( m_pVMesh->GetSelectWeaponMotionType() == eq_wd_sword  ) || 
		( m_pVMesh->GetSelectWeaponMotionType() == eq_wd_blade  ) ) 
	{
		ZC_STATE_LOWER aniState_Lower = m_AniState_Lower.Ref();
		ZC_STATE_UPPER aniState_Upper = m_AniState_Upper.Ref();

		if( (ZC_STATE_LOWER_ATTACK1 <= aniState_Lower && aniState_Lower <= ZC_STATE_LOWER_GUARD_CANCEL) ||
			(ZC_STATE_UPPER_LOAD <= aniState_Upper && aniState_Upper <= ZC_STATE_UPPER_GUARD_CANCEL))
		{
			if(aniState_Upper!=ZC_STATE_UPPER_GUARD_IDLE) {

				// º¹±ÍÇÒ¶§´Â ¾È ±×¸°´Ù
				if(	(aniState_Lower != ZC_STATE_LOWER_ATTACK1_RET) &&
					(aniState_Lower != ZC_STATE_LOWER_ATTACK2_RET) &&
					(aniState_Lower != ZC_STATE_LOWER_ATTACK3_RET) &&
					(aniState_Lower != ZC_STATE_LOWER_ATTACK4_RET))
					// SwordTrail option removed, is no longer necessary.
					bDrawTracks = true;
			}
		}
	}

	// ÀÌµµ·ùÀÏ °æ¿E

	bool bDTM[2];

	bDTM[0] = true;
	bDTM[1] = true;

	bool bMan = IsMan();

	if(m_pVMesh->GetSelectWeaponMotionType() == eq_wd_blade) 
	{
			 if( m_AniState_Lower.Ref() == ZC_STATE_LOWER_ATTACK1 ) GetDTM(bDTM,0,bMan);
		else if( m_AniState_Lower.Ref() == ZC_STATE_LOWER_ATTACK2 ) GetDTM(bDTM,1,bMan);
		else if( m_AniState_Lower.Ref() == ZC_STATE_LOWER_ATTACK3 ) GetDTM(bDTM,2,bMan);
		else if( m_AniState_Lower.Ref() == ZC_STATE_LOWER_ATTACK4 ) GetDTM(bDTM,3,bMan);
	}

	m_pVMesh->SetDrawTracksMotion(0, bDTM[0]);
	m_pVMesh->SetDrawTracksMotion(1, bDTM[1]);

	m_pVMesh->SetDrawTracks(bDrawTracks);

}

// ·»´õ¸µÇÏ¸é¼­ ¸ð¼ÇÀÇ ÇÁ·¹ÀÓ Å¸ÀÓ¿¡ ÀÇÁ¸ÀûÀÎ ¹«±âÀÇ °æ¿E

void ZCharacter::UpdateSpWeapon()
{
	if(!m_pVMesh) return;

	m_pVMesh->UpdateSpWeaponFire();

	if(m_pVMesh->IsAddGrenade()) {//¸ðµÎ°¡ ¾Ë°úÜÖ´Ù~

		rvector vWeapon[2];

		vWeapon[0] = m_pVMesh->GetCurrentWeaponPosition();

		// º®µÚ·Î ´øÁö´Â °æ¿E¡ »ý±â¸E
//		rvector nPos = m_Position + rvector(0,0,100);
		rvector nPos = m_pVMesh->GetBipTypePosition(eq_parts_pos_info_Spine1);// m_Position + rvector(0,0,100);
		rvector nDir = vWeapon[0] - nPos;

		Normalize(nDir);

		RBSPPICKINFO bpi;
		// ¾Æ·¡¸¦ º¸¸é¼­ ´øÁö¸Eº®À» ÅEúÇØ¼­...
//		if(ZGetGame()->GetWorld()->GetBsp()->Pick(m_Position+rvector(0,0,100),m_TargetDir,&bpi))
		if(ZGetGame()->GetWorld()->GetBsp()->Pick(nPos,nDir,&bpi))
		{
			if(D3DXPlaneDotCoord(&(bpi.pInfo->plane),&vWeapon[0])<0){
//				vWeapon[0]=m_Position+rvector(0,0,140);
				vWeapon[0] = bpi.PickPos - nDir;
//				OutputDebugString("º®µÚ·Î °¬´Ù...\n");
			}
		}

		vWeapon[1] = m_TargetDir;//rvector();//½ÃÀÛ¹æÇE
		vWeapon[1].z += 0.1f;//¾à°£À§ÂÊÀ¸·Î..
//		vWeapon[2] = rvector(1,1,1);//¾ÆÁ÷»ç¿EÈÇÔ

		Normalize(vWeapon[1]);

		if(m_UID==ZGetGame()->m_pMyCharacter->m_UID) {

			int type = ZC_WEAPON_SP_GRENADE;//±âº»¼ö·ùÅº
#ifdef _PORTALGUN 1
			MMatchWeaponType wtype = MWT_NONE;
#endif
			RVisualMesh* pWVMesh = m_pVMesh->GetSelectWeaponVMesh();

			if( pWVMesh ) {
				if(pWVMesh->GetMesh()) {
					if(strncmp( pWVMesh->GetMesh()->GetName(), "flashbang", 9) == 0) {
						type = ZC_WEAPON_SP_FLASHBANG;
					}
					else if(strncmp( pWVMesh->GetMesh()->GetName(), "smoke", 5) == 0) {
						type = ZC_WEAPON_SP_SMOKE;
					}
					else if(strncmp( pWVMesh->GetMesh()->GetName(), "tear_gas", 8) == 0) {
						type = ZC_WEAPON_SP_TEAR_GAS;	
					} 
					else if(strncmp( pWVMesh->GetMesh()->GetName(), "trap", 4) == 0) {
						type = ZC_WEAPON_SP_TRAP;
					}
					else if(strncmp( pWVMesh->GetMesh()->GetName(), "dynamite", 8) == 0) {
						type = ZC_WEAPON_SP_DYNAMITE;
					}
					else if (strncmp(pWVMesh->GetMesh()->GetName(), "spy_stungrenade", 15) == 0) {
						type = ZC_WEAPON_SPY_STUNGRENADE;
					}
					else if (wtype == MWT_PORTAL_GUN) {
						type = ZC_WEAPON_SP_PORTAL_GUN;
					}
				}
			}

			int sel_type = GetItems()->GetSelectedWeaponParts();

			ZPostShotSp(/*g_pGame->GetTime(),*/vWeapon[0], vWeapon[1], type, sel_type);
			m_pVMesh->SetAddGrenade(false);
		}
	}
}

#define SLASH_CHARGING_DEFAULT "ef_spirits.elu"
#define SLASH_CHARGED_DEFAULT "ef_spirits.elu_1.elu"
#define SLASH_DEFAULT "ef_slash"

void ZCharacter::InitSlashEquipment()
{
	char defaulSlashChargingName[32] = {};
	char defaulSlashChargedName[32] = {};
	char defaulSlashName[32] = {};

	// set default first
	sprintf(defaulSlashChargingName, SLASH_CHARGING_DEFAULT);
	sprintf(defaulSlashChargedName, SLASH_CHARGED_DEFAULT);
	sprintf(defaulSlashName, SLASH_DEFAULT);

	ZMyItemNode* pItemNode = ZGetMyInfo()->GetItemList()->GetEquipedItem(MMatchCharItemParts(MMCIP_STUN));
	if (pItemNode) {
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(pItemNode->GetItemID());
		if (pItemDesc) {
			sprintf(defaulSlashChargingName, "%s", pItemDesc->m_pMItemName->Ref().m_szMeshName2);
			// check is not empty
			if (defaulSlashChargingName[0] == '\0')
				sprintf(defaulSlashChargingName, SLASH_CHARGING_DEFAULT);

			sprintf(defaulSlashChargedName, "%s", pItemDesc->m_pMItemName->Ref().m_szMeshName3);
			// check is not empty
			if (defaulSlashChargedName[0] == '\0')
				sprintf(defaulSlashChargedName, SLASH_CHARGED_DEFAULT);

			sprintf(defaulSlashName, "%s", pItemDesc->m_pMItemName->Ref().m_szMeshName);
			// check is not empty
			if (defaulSlashName[0] == '\0')
				sprintf(defaulSlashName, SLASH_DEFAULT);
		}
	}

	// Custom: Slash Equip
	sprintf(m_szSlashChargingName, "%s", defaulSlashChargingName);
	sprintf(m_szSlashChargedName, "%s", defaulSlashChargedName);
	sprintf(m_szSlashName, "%s", defaulSlashName);
}

bool ZCharacter::IsMan() 
{
	if(m_pVMesh) {
		if(m_pVMesh->GetMesh()) {
			if(strcmp(m_pVMesh->GetMesh()->GetName(),"heroman1")==0) {
				return true;
			}
		}
	}
	return false;
}

void ZCharacter::OnDraw()
{
	m_bRendered = false;

	if (m_bInitialized == false) return;
	if (!IsVisible()) return;
	if (IsAdminHide()) return;

	// Custom: Snipers
	// --- CUSTOM FIX: Giải kẹt trạng thái Tàng hình ---
	if (m_dwStatusBitPackingValue.Ref().m_bSniping)
	{
		ZItem* pSItem = GetItems()->GetSelectedWeapon();
		// Nếu súng trên tay KHÔNG PHẢI Sniper (đã đổi sang kiếm, súng lục...)
		if (pSItem && pSItem->GetDesc()->m_nWeaponType.Ref() != MWT_SNIFER)
		{
			// Ép cờ về false để các hàm sau vẽ lại nhân vật
			m_dwStatusBitPackingValue.Ref().m_bSniping = false;
		}
		else if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode() && ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter() == this)
		{
			// Nếu thực sự là đang cầm Sniper ngắm thì mới ẩn
			return;
		}
	}

	__BP(39, "ZCharacter::Draw");

	if (m_pVMesh && !Enable_Cloth)	m_pVMesh->DestroyCloth();

	//////////////
	// ±¤¿E¼¼ÆÃ

	//#define SHOW_LIGHT_TEAPOT			// ±¤¿ø¿¡ ÁÖÀEÚ Ç¥½Ã

	if (m_nVMID.Ref() == -1)//ÃÊ±âÈ­µµ ¾ÈµÈ»óÅÂ
	{
		__EP(39);
		return;
	}

	rboundingbox bb;
	bb.vmax = GetPosition() + rvector(50, 50, 190);
	bb.vmin = GetPosition() - rvector(50, 50, 0);
	if (!ZGetGame()->GetWorld()->GetBsp()->IsVisible(bb)) return;
	if (!isInViewFrustum(&bb, RGetViewFrustum())) return;

	__BP(24, "ZCharacter::Draw::Light");

	Draw_SetLight(GetPosition());

	__EP(24);

	if (ZGetGame()->m_bShowWireframe)
	{
		RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
		RGetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		RGetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	}

	//jintriple3 ºñÆ® ÆÐÅ· ¸Þ¸ð¸® ÇÁ·Ï½Ã...
	const ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	// ³ª¶ô¿¡ ¶³¾ûÝú¶§ ¸Ê¿¡ Æ÷±×°¡ ±ò·ÁÀÖÀ¸¸EZBuffer¸¦ ²¨ÁØ´Ù. ³ª¶EÆÇ¶§±â¿¡ Ä³¸¯ÅÍ°¡ °¡·ÁÁö±E¶§¹®
	bool bNarakSetState = ((uStatus.m_bFallingToNarak) && ((ZGetGame()->GetWorld()->IsFogVisible())));
	if (bNarakSetState)
	{
		RGetDevice()->SetRenderState(D3DRS_FOGENABLE, FALSE);
		RSetWBuffer(false);
	}


	///////////////////////////////////////////////////////////////////////
	//	rvector dir = m_Direction;dir.z = 0;

	CheckDrawWeaponTrack();

	auto MaxVisibility = 1.0f;
	// In the skillmap gamemode, all player characters
	// that aren't the player are transparent
	if (!m_bHero && ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DEATHMATCH_SOLO)
		MaxVisibility = 0.4f;

	RGetDevice()->SetRenderState(D3DRS_LIGHTING, TRUE);


	if (IsDie())
	{
#define TRAN_AFTER		3.f		
#define VANISH_TIME		2.f	
		//Custom: Opacity dead By Desperate
		if (ZGetGame()->GetMatch()->IsTeamPlay() || ZGetGameClient()->IsCWChannel() || ZGetGameClient()->IsLadderWarsChannel())
		{
			float fOpacity = max(1.f, min(1.0f, (VANISH_TIME - (ZGetGame()->GetTime() - GetDeadTime() - TRAN_AFTER)) / VANISH_TIME));
			m_pVMesh->SetVisibility(fOpacity);
		}
		else
		{
			float fOpacity = max(0.f, min(1.0f, (VANISH_TIME - (ZGetGame()->GetTime() - GetDeadTime() - TRAN_AFTER)) / VANISH_TIME));
			m_pVMesh->SetVisibility(fOpacity);
		}
	}
	// Custom: Ghost Mode - Desperate
	else if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_GHOST)
	{
		if (GetTeamID() == MMT_RED && IsGhost() == false) m_pVMesh->SetVisibility(0);
		if (GetTeamID() == MMT_RED && IsGhost() == true) m_pVMesh->SetVisibility(0.2);
	}
	else
		if (!m_bHero) m_pVMesh->SetVisibility(1.f);

	__BP(25, "ZCharacter::Draw::VisualMesh::Render");

	//	m_bIsLowModel = true;

	UpdateEnchant();

	bool bGame = ZGetGame() ? true : false;

	if (m_pVMesh)
	{
		ZCharacter* pMyCharacter = ZGetGame()->m_pMyCharacter;
		int nMode = ZGetConfiguration()->GetVideo()->nOutlineMode;
		bool bDrawOutline = false;

		// --- CHECK ĐIỀU KIỆN TRƯỚC KHI VẼ ---
		ZGame* pGame = ZGetGame();
		if (!pGame || !pMyCharacter) return;
		else {
			switch (nMode)
			{
			case ZOUTLINE_ALL: bDrawOutline = true; break;
			case ZOUTLINE_NOT_ME: bDrawOutline = (this != pMyCharacter); break;
			case ZOUTLINE_NOT_MYTEAM:
				// Fix: Phải là Team Game thì mới check Team, nếu Deathmatch thì coi như không cùng team
				if (pGame->GetMatch()->IsTeamPlay())
					bDrawOutline = (this != pMyCharacter) && (GetTeamID() != pMyCharacter->GetTeamID());
				else
					bDrawOutline = (this != pMyCharacter); // Deathmatch thì vẽ hết trừ mình
				break;
			default: bDrawOutline = false; break;
			}
		}

		m_pVMesh->SetOutline(bDrawOutline, ZGetCharacterOutlineColor(this), 1.6f);
	}

	rvector cpos = ZApplication::GetGameInterface()->GetCamera()->GetPosition();
	// cpos = m_vProxyPosition - cpos;
	cpos = GetPosition() - cpos;
	float dist = Magnitude(cpos);

	m_pVMesh->SetClothValue(bGame, 0.0f); //fabs(dist));

	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bLowPoly)
		m_pVMesh->Render(true);
	else
		//m_pVMesh->Render(uStatus.m_bIsLowModel);
	    m_pVMesh->Render(false);
//	DrawOutline();

	bool bPrevSilhouette = RMesh::m_bSilhouette;
	float fPrevSilhouetteLength = RMesh::m_fSilhouetteLength;
	DWORD dwPrevSilhouetteColor = RMesh::m_dwSilhouetteColor;

	if (bGame) {
		int nOutlineMode = ZGetConfiguration()->GetVideo()->nOutlineMode;
		bool bDrawOutline = false;

		switch (nOutlineMode) {
		case ZOUTLINE_ALL:
			bDrawOutline = true;
			break;
		case ZOUTLINE_NOT_ME:
			bDrawOutline = !IsMyCharacter(this);
			break;
		case ZOUTLINE_NOT_MYTEAM:
			bDrawOutline = !IsMyCharacter(this) && !(ZGetGame()->m_pMyCharacter && IsSameTeam(ZGetGame()->m_pMyCharacter));
			break;
		case ZOUTLINE_DISABLE:
		default:
			bDrawOutline = false;
			break;
		}

		static bool bLoggedOnce = false;
		if (!bLoggedOnce) {
			mlog("Outline debug: mode=%d bDrawOutline=%d mbUsingShader=%d\n",
				nOutlineMode, bDrawOutline, RShaderMgr::mbUsingShader);
			bLoggedOnce = true;
		}

		RMesh::m_bSilhouette = bDrawOutline;
		RMesh::m_fSilhouetteLength = 1.0f;
		RMesh::m_dwSilhouetteColor = ZGetConfiguration()->GetVideo()->dwOutlineColor;
	}
	//m_pVMesh->Render(uStatus.m_bIsLowModel);
	//if (bGame) {
	//	RMesh::m_bSilhouette = bPrevSilhouette;
	//	RMesh::m_fSilhouetteLength = fPrevSilhouetteLength;
	//	RMesh::m_dwSilhouetteColor = dwPrevSilhouetteColor;
	//}

	m_bRendered = m_pVMesh->IsRender();	// ½ÇÁ¦ ·»´õ¸µ µÇ¾ú´Â°¡


	if (m_pVMesh->IsRenderWeapon() && (m_pVMesh->GetVisibility() > 0.05f))
	{
		DrawEnchant(m_AniState_Lower.Ref(), m_bCharged->Ref());	//mmemory proxy
	}


	if (bNarakSetState)
	{
		RSetWBuffer(true);
	}

	__EP(25);


	__EP(39);
}

void ZCharacter::DrawFriendOutlineXray()
{
	ZCharacter* pMyCharacter = ZGetGame()->m_pMyCharacter;
	ZGame* pGame = ZGetGame();
	if (!pGame || !pMyCharacter || !m_pVMesh) return;

	bool bIsTeamGame = pGame->GetMatch()->IsTeamPlay() || (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_QUEST);
	bool bIsFriend = (bIsTeamGame && GetTeamID() == pMyCharacter->GetTeamID() && this != pMyCharacter);

	if (bIsFriend)
	{
		// Tắt độ sâu để nhìn xuyên tường
		RGetDevice()->SetRenderState(D3DRS_ZENABLE, FALSE);
		RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

		// Render cái viền thôi
		if (m_pVMesh->GetMesh()) {
			m_pVMesh->GetMesh()->RenderOutline((D3DXMATRIX*)&m_pVMesh->GetWorldMat());
		}

		RGetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);
		RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	}
}


bool ZCharacter::CheckDrawGrenade()
{
	if(m_Items.GetSelectedWeapon()==NULL) return false;

	if( m_pVMesh ) {
		if( m_pVMesh->GetSelectWeaponMotionType()==eq_wd_grenade ) {
			if( m_Items.GetSelectedWeapon()->GetBulletCurrMagazine() ) {
				return true;
			}
		}
	}
	return false;
}

bool ZCharacter::Pick(rvector& pos,rvector& dir, RPickInfo* pInfo)
{
	if (m_bInitialized==false) return false ;

	return ZObject::Pick(pos, dir, pInfo);
}

/*
bool ZCharacter::Pick(int x,int y,rvector* v,float* f)
{
	RPickInfo info;
	bool hr = Pick(x,y,&info);
	*v = info.vOut;
	*f = info.t;
	return hr;
}
*/
/*
bool ZCharacter::Pick(int x,int y,RPickInfo* pInfo) {


	if(m_pVMesh) {
		return m_pVMesh->Pick(x,y,pInfo);
	}

	return false;
}
*/

#define GRAVITY_CONSTANT	2500.f			// Áß·ÂÀÇ ¿µÇE
#define DAMAGE_VELOCITY		1700.f			// ¼Óµµ°¡ ÀÌ ÀÌ»óµÇ¸EÆú¸µµ¥¹ÌÁö¸¦ ¹Þ´Â´Ù.
#define MAX_FALL_SPEED		3000.f			// ÃÖ´E³«ÇÏ¼Óµµ
#define MAX_FALL_DAMAGE		50.f			// ÃÖ´Eµ¥¹ÌÁE
#define BLASTED_KNOCKBACK_RATIO	3.f			// ¶ç¿öÁø»óÅÂ¿¡¼­ÀÇ KnockbackÀ» °úÀE¹è¼E

void ZCharacter::UpdateHeight(float fDelta)
{
	//jintriple3 ¸Þ¸ð¸® ÇÁ·Ï½Ã...ºñÆ® ÆÐÅ·..
	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	if (uStatus.m_bFallingToNarak) return;

	uStatus.m_bJumpUp=(GetVelocity().z>0);

	if(GetVelocity().z<0 && GetDistToFloor()>35.f)	// °è´Ü ³»·Á°¥¶§ Á¤µµ´Â ±×³É ³»·Á°£´Ù.
	{
		if(!uStatus.m_bJumpDown) {
//			m_fFallHeight = m_Position.z;
			uStatus.m_bJumpDown=true;
			uStatus.m_bJumpUp = false;
		}
	}

	if(!uStatus.m_bWallJump)
	{
		//if(GetVelocity().z<1.f && GetDistToFloor()<1.f)
		//{
		//	if(!m_bLand )
		if(m_pModule_Movable->isLanding())
		{
			if(GetPosition().z + 100.f < m_pModule_Movable->GetFallHeight())
			{
//				m_fFallHeight = m_Position.z;
				float fSpeed=fabs(GetVelocity().z);
				if(fSpeed>DAMAGE_VELOCITY)
				{
					float fDamage = MAX_FALL_DAMAGE * ( fSpeed-DAMAGE_VELOCITY) / (MAX_FALL_SPEED-DAMAGE_VELOCITY) ;
#ifdef	ENABLE_FALLING_DAMAGE
					//				DamagedFalling(fDamage);
#endif

				}

				RBspObject* r_map = ZGetGame()->GetWorld()->GetBsp();

				// Á¡ÇÁ ÂøÁö½Ã ¸ÕÁE.
				rvector vPos = GetPosition();
				rvector vDir = rvector(0.f,0.f,-1.f);
				vPos.z += 50.f;

				RBSPPICKINFO pInfo;

				if(r_map->Pick(vPos,vDir,&pInfo)) {
/*
					if( g_pGame->m_waters.CheckSpearing( vPos+rvector(0.f,0.f,200.f), pInfo.PickPos, 250, 0.3 ) )	{
								

					}
					else {
*/						
					vPos = pInfo.PickPos;

					vDir.x = pInfo.pInfo->plane.a;
					vDir.y = pInfo.pInfo->plane.b;
					vDir.z = pInfo.pInfo->plane.c;

					ZGetEffectManager()->AddLandingEffect(vPos,vDir);//³»ºÎ¿¡¼­ ¿É¼Ç¿¡ µû¶ó~

					///////////////////////////////////////////////////////////
					// ÂøÁEsound .. ÂøÁE½ÃÁ¡À» Á¤È®ÇÏ°Ô ÇÏ·Á¸E..

					AniFrameInfo* pInfo = m_pVMesh->GetFrameInfo(ani_mode_lower);
					RAniSoundInfo* pSInfo = &pInfo->m_SoundInfo;// &m_pVMesh->m_SoundInfo[0];

					if(pSInfo->Name[0]) {
						pSInfo->isPlay = true;
						UpdateSound();
//							ZApplication::GetSoundEngine()->PlaySound(pSInfo->Name,vPos.x,vPos.y,vPos.z);
					}
					else {//º®Á¡ÇÁÈÄ »ç¿ûÑåµûÜº µûÓÏµÇ¾EÀÖÁE¾Ê´Ù..
						strcpy(pSInfo->Name,"man_jump");
						pSInfo->isPlay = true;
						UpdateSound();
					}

//						}
				}

			}
		}
	}

	return;
}

int ZCharacter::GetSelectWeaponDelay(MMatchItemDesc* pSelectItemDesc)
{
//	Ä®ÀÇ °æ¿EÂ ¸ð¼ÇÀÇ ±æÀÌ°¡ Æ÷ÇÔµÇ¾EÀÖ´Ù..
//	¹«±âÅ¸ÀÙº°·Î ÇÏ³ª¸¸ µûÓÏµÇ¾EÀÖÀ¸´Ï±E±âº»°ø°Ý ¸ð¼ÇÀÇ ±æÀÌ¸¦ ±âÁØÀ¸·Î ÇÑ´Ù..

	if(!pSelectItemDesc) return 0;

	int nReturnDelay = pSelectItemDesc->m_nDelay.Ref();

	// Custom: Force Melee Delay room mod - chỉ override khi có giá trị hợp lệ,
	// nếu mod active nhưng value=-1 (checkbox bật nhưng ô trống) thì giữ nguyên delay gốc của vũ khí.
	if (pSelectItemDesc->m_nType.Ref() == MMIT_MELEE &&
		ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_FORCEMELEEDELAY))
	{
		int nForce = ZGetGameClient()->GetMatchStageSetting()->GetModifierValue(MMOD_FORCEMELEEDELAY);
		if (nForce >= 100 && nForce <= 1000)
			nReturnDelay = nForce;
	}

	if(pSelectItemDesc->m_nType.Ref() != MMIT_MELEE)
		return 0;

	switch(pSelectItemDesc->m_nWeaponType.Ref())
	{
		case MWT_DAGGER:		// attackS °ø°Ý ±æÀÌ¸¸Å­... man_dagger_standshot.elu.ani±âÁØ..
			nReturnDelay -= 266;
			break;

		case MWT_DUAL_DAGGER:	// attack1 °ø°Ý ±æÀÌ¸¸Å­.... man_2hdagger_standshot_1.elu.ani
			nReturnDelay -= 299;
			break;

		case MWT_KATANA:		//	attack1 °ø°Ý ±æÀÌ¸¸Å­....man_knife_standshot_1.elu.ani
			nReturnDelay -= 299;
			break;

		case MWT_DOUBLE_KATANA:	//	attack1 °ø°Ý ±æÀÌ¸¸Å­.... man_blade_standshot_1.elu.ani 
			nReturnDelay -= 299;
			break;

		case MWT_GREAT_SWORD:	// attack1 °ø°Ý ±æÀÌ¸¸Å­.... man_sword_standshot_1.elu.ani
			nReturnDelay -= 399;
			break;

		case MWT_SPYCASE:
			nReturnDelay -= 299; // spycase standshot, idk if this is the correct delay or not though.
			break;

		default:
			_ASSERT(0);
			break;
	}

	return nReturnDelay;
/*
	½Ç½Ã°£À¸·Î °Ë»öÇÒ ÇÊ¿ä¾øÀ»°Í °°´Ù..

	RAnimation* pAniSet = m_pVMesh->GetFrameInfo( ani_mode_upper )->m_pAniSet;

	if( pAniSet==NULL )
		pAniSet = m_pVMesh->GetFrameInfo( ani_mode_lower )->m_pAniSet;

	if( pAniSet ) {
		if( pAniSet->m_pAniData ) {
			DWORD _ms  = pAniSet->m_pAniData->m_max_frame / 4.8f;
			nWeaponDelay -= _ms;
		}
	}
*/
}
//¹öÇÁÁ¤º¸ÀÓ½ÃÁÖ¼® 
/*
void ZCharacter::UpdateBuff()
{
	static unsigned int nPreviousTime = 0;

	unsigned int nCurrentTime = GetTickCount();
	if( nCurrentTime - nPreviousTime >= 1000 ) 
	{
		ZBuffSummary* pBuffSummary = GetCharacterBuff()->GetBuffSummary();
		OnHealing(this, pBuffSummary->GetDoteHP(), pBuffSummary->GetDoteAP());

		nPreviousTime = nCurrentTime;
	}
}
*/
void ZCharacter::UpdateSpeed()
{
	//{
	//	if (m_pVMesh == NULL) return;
	//	float speed = static_cast<float>(4.8f);	//speedgame

	//	if (GetItems() && GetItems()->GetSelectedWeapon() && GetItems()->GetSelectedWeapon()->GetDesc()) {
	//		if (GetItems()->GetSelectedWeapon()->GetDesc()->m_nType.Ref() == MMIT_MELEE) {
	//			ZC_STATE_LOWER aniState_Lower = m_AniState_Lower.Ref();
	//			if ((aniState_Lower == ZC_STATE_LOWER_ATTACK1) || (aniState_Lower == ZC_STATE_LOWER_ATTACK1_RET) ||
	//				(aniState_Lower == ZC_STATE_LOWER_ATTACK2) || (aniState_Lower == ZC_STATE_LOWER_ATTACK2_RET) ||
	//				(aniState_Lower == ZC_STATE_LOWER_ATTACK3) || (aniState_Lower == ZC_STATE_LOWER_ATTACK3_RET) ||
	//				(aniState_Lower == ZC_STATE_LOWER_ATTACK4) || (aniState_Lower == ZC_STATE_LOWER_ATTACK4_RET) ||
	//				(aniState_Lower == ZC_STATE_LOWER_ATTACK5) || (aniState_Lower == ZC_STATE_LOWER_JUMPATTACK) ||
	//				(m_AniState_Upper.Ref() == ZC_STATE_UPPER_SHOT)) {

	//				MMatchItemDesc* pRangeDesc = GetItems()->GetSelectedWeapon()->GetDesc();

	//				int  nWeaponDelay = GetSelectWeaponDelay(pRangeDesc);

	//				int max_frame = 0;

	//				max_frame = m_pVMesh->GetMaxFrame(ani_mode_upper);

	//				if (max_frame == 0)
	//					max_frame = m_pVMesh->GetMaxFrame(ani_mode_lower);

	//				if (max_frame) {

	//					int _time = (int)(max_frame / speed);

	//					int as = _time + nWeaponDelay;

	//					if (as < 1)	as = 1;

	//					float fas = 0.f;

	//					fas = (_time / (float)(as));

	//					m_fAttack1Ratio.Set_CheckCrc(fas);

	//					speed = 4.8f * m_fAttack1Ratio.Ref();
	//				}

	//				if (speed < 0.1f)
	//					speed = 0.1f;

	//			}
	//		}
	//	}

	//	if (m_AniState_Upper.Ref() == ZC_STATE_UPPER_LOAD) {
	//		speed = speed * 1.2f;
	//	}

	//	m_pVMesh->SetSpeed(speed, speed);
	//}
	{
		if (m_pVMesh == NULL) return;

		float speed = 4.8f;
		float speed_upper = 4.8f;

		if (GetItems() && GetItems()->GetSelectedWeapon() && GetItems()->GetSelectedWeapon()->GetDesc())
		{
			if (GetItems()->GetSelectedWeapon()->GetDesc()->m_nType.Ref() == MMIT_MELEE)
			{
				ZC_STATE_LOWER aniState_Lower = m_AniState_Lower.Ref();
				if ((aniState_Lower == ZC_STATE_LOWER_ATTACK1) || (aniState_Lower == ZC_STATE_LOWER_ATTACK1_RET) ||
					(aniState_Lower == ZC_STATE_LOWER_ATTACK2) || (aniState_Lower == ZC_STATE_LOWER_ATTACK2_RET) ||
					(aniState_Lower == ZC_STATE_LOWER_ATTACK3) || (aniState_Lower == ZC_STATE_LOWER_ATTACK3_RET) ||
					(aniState_Lower == ZC_STATE_LOWER_ATTACK4) || (aniState_Lower == ZC_STATE_LOWER_ATTACK4_RET) ||
					(aniState_Lower == ZC_STATE_LOWER_ATTACK5) || (aniState_Lower == ZC_STATE_LOWER_JUMPATTACK) ||
					(m_AniState_Upper.Ref() == ZC_STATE_UPPER_SHOT))
				{
					MMatchItemDesc* pRangeDesc = GetItems()->GetSelectedWeapon()->GetDesc();
					int  nWeaponDelay = GetSelectWeaponDelay(pRangeDesc);
					int max_frame = 0;
					max_frame = m_pVMesh->GetMaxFrame(ani_mode_upper);

					if (max_frame == 0)
						max_frame = m_pVMesh->GetMaxFrame(ani_mode_lower);
					if (max_frame)
					{
						int _time = (int)(max_frame / 4.8f);
						int as = _time + nWeaponDelay;
						if (as < 1)	as = 1;
						float fas = 0.f;
						fas = (_time / (float)(as));
						m_fAttack1Ratio.Set_CheckCrc(fas);
						speed = 4.8f * m_fAttack1Ratio.Ref();
					}

					if (speed < 0.1f)
						speed = 0.1f;

				}
			}
		}
		if (m_AniState_Upper.Ref() == ZC_STATE_UPPER_LOAD)
		{
			speed = 4.8f * 1.2f;
			speed_upper = 4.8f * 1.2f;
		}
		m_pVMesh->SetSpeed(speed, speed);
	}
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_TURBO)
	{
		if (m_pVMesh == NULL) return;
		float speed = 5.4f;
		if (GetItems() && GetItems()->GetSelectedWeapon() && GetItems()->GetSelectedWeapon()->GetDesc())
		{
			if (GetItems()->GetSelectedWeapon()->GetDesc()->m_nType.Ref() == MMIT_MELEE)
			{
				ZC_STATE_LOWER aniState_Lower = m_AniState_Lower.Ref();
				if ((aniState_Lower == ZC_STATE_LOWER_ATTACK1) || (aniState_Lower == ZC_STATE_LOWER_ATTACK1_RET) ||
					(aniState_Lower == ZC_STATE_LOWER_ATTACK2) || (aniState_Lower == ZC_STATE_LOWER_ATTACK2_RET) ||
					(aniState_Lower == ZC_STATE_LOWER_ATTACK3) || (aniState_Lower == ZC_STATE_LOWER_ATTACK3_RET) ||
					(aniState_Lower == ZC_STATE_LOWER_ATTACK4) || (aniState_Lower == ZC_STATE_LOWER_ATTACK4_RET) ||
					(aniState_Lower == ZC_STATE_LOWER_ATTACK5) || (aniState_Lower == ZC_STATE_LOWER_JUMPATTACK) ||
					(m_AniState_Upper.Ref() == ZC_STATE_UPPER_SHOT))
				{
					MMatchItemDesc* pRangeDesc = GetItems()->GetSelectedWeapon()->GetDesc();
					int  nWeaponDelay = GetSelectWeaponDelay(pRangeDesc);
					int max_frame = 0;
					max_frame = m_pVMesh->GetMaxFrame(ani_mode_upper);
					if (max_frame == 0)
						max_frame = m_pVMesh->GetMaxFrame(ani_mode_lower);
					if (max_frame)
					{
						int _time = (int)(max_frame / speed);
						int as = _time + nWeaponDelay;
						if (as < 1)	as = 1;
						float fas = 0.f;
						fas = (_time / (float)(as));
						m_fAttack1Ratio.Set_CheckCrc(fas);
						speed = 4.9f * m_fAttack1Ratio.Ref();
					}
					if (speed < 0.1f)
						speed = 0.1f;
				}
			}
		}
		if (m_AniState_Upper.Ref() == ZC_STATE_UPPER_LOAD)
		{
			speed = speed * 1.3f;
		}
		m_pVMesh->SetSpeed(speed, speed);
	}

	if (ZGetGame()->GetMatch()->IsClassiMode())
	{
		if (m_pVMesh == NULL) return;

		float speed = 4.8f;
		float speed_upper = 4.8f;

		if (GetItems() && GetItems()->GetSelectedWeapon() && GetItems()->GetSelectedWeapon()->GetDesc())
		{
			if (GetItems()->GetSelectedWeapon()->GetDesc()->m_nType.Ref() == MMIT_MELEE)
			{
				ZC_STATE_LOWER aniState_Lower = m_AniState_Lower.Ref();
				if ((aniState_Lower == ZC_STATE_LOWER_ATTACK1) || (aniState_Lower == ZC_STATE_LOWER_ATTACK1_RET) ||
					(aniState_Lower == ZC_STATE_LOWER_ATTACK2) || (aniState_Lower == ZC_STATE_LOWER_ATTACK2_RET) ||
					(aniState_Lower == ZC_STATE_LOWER_ATTACK3) || (aniState_Lower == ZC_STATE_LOWER_ATTACK3_RET) ||
					(aniState_Lower == ZC_STATE_LOWER_ATTACK4) || (aniState_Lower == ZC_STATE_LOWER_ATTACK4_RET) ||
					(aniState_Lower == ZC_STATE_LOWER_ATTACK5) || (aniState_Lower == ZC_STATE_LOWER_JUMPATTACK) ||
					(m_AniState_Upper.Ref() == ZC_STATE_UPPER_SHOT))
				{
					MMatchItemDesc* pRangeDesc = GetItems()->GetSelectedWeapon()->GetDesc();
					int  nWeaponDelay = GetSelectWeaponDelay(pRangeDesc);
					int max_frame = 0;
					max_frame = m_pVMesh->GetMaxFrame(ani_mode_upper);

					if (max_frame == 0)
						max_frame = m_pVMesh->GetMaxFrame(ani_mode_lower);
					if (max_frame) 
					{
						int _time = (int)(max_frame / 4.8f);
						int as = _time + nWeaponDelay;
						if (as < 1)	as = 1;
						float fas = 0.f;
						fas = (_time / (float)(as));
						m_fAttack1Ratio.Set_CheckCrc(fas);
						speed = 4.8f * m_fAttack1Ratio.Ref();
					}

					if (speed < 0.1f)
						speed = 0.1f;

				}
			}
		}
		if (m_AniState_Upper.Ref() == ZC_STATE_UPPER_LOAD)
		{
			speed = 4.8f * 1.2f;
			speed_upper = 4.8f * 1.2f;
		}
		m_pVMesh->SetSpeed(speed, speed);
	}

}
static DWORD meshupdatetime = 0;
void ZCharacter::OnUpdate(float fDelta)
{
	if (m_bInitialized == false) return;
	if (!IsVisible()) return;

	if (m_pVMesh) {
		m_pVMesh->SetVisibility(1.f);
		m_pVMesh->Frame();
	}

	meshupdatetime += fDelta * 1000;

	//todo: create a callback instead of this lazy check
	if (meshupdatetime >= 20000 && MeshesFinishedLoading == false)
	{
		MeshesFinishedLoading = true;
		unsigned long meshparts[MMCIP_END];
		for (int i = 0; i < MMCIP_END; ++i)
		{
			meshparts[i] = this->GetItems()->GetItem(static_cast<MMatchCharItemParts>(i))->GetDescID();
		}
		ZChangeCharParts(m_pVMesh, GetProperty()->nSex, GetProperty()->nHair, GetProperty()->nFace, meshparts);
		MeshesFinishedLoading = false;
	}

	UpdateSound();
	UpdateSpeed();

	//Custom: Character Posture
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bPosture)
	{
		UpdateDirection(fDelta, m_TargetDir);
		UpdateMotion(fDelta);
	}
	if (ZGetGame()->GetMatch()->IsClassiMode())
	{
		UpdateDirection(fDelta, m_TargetDir);
		UpdateMotion(fDelta);
	}
	else
	{
		UpdateMotion(fDelta);
	}

	if (m_pVMesh && Enable_Cloth && m_pVMesh->isChestClothMesh())
	{
		if (IsDie())
		{
			rvector force = rvector(0, 0, -150);
			m_pVMesh->UpdateForce(force);
			m_pVMesh->SetClothState(CHARACTER_DIE_STATE);
		}
		else
		{
			rvector force = -GetVelocity() * 0.15;
			force.z += -90;
			m_pVMesh->UpdateForce(force);
		}
	}

	rvector vRot(0.0f, 0.0f, 0.0f);
	rvector vProxyPosition(0.0f, 0.0f, 0.0f);
	rvector vProxyDirection(0.0f, 0.0f, 0.0f);


	ZObserver* pObserver = ZGetCombatInterface()->GetObserver();
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bPosture)
	{
		v3 ProxyDirection;
		if (pObserver->IsVisible())
		{
			UpdateDirection(fDelta, m_TargetDir);
			m_vProxyPosition = m_Position.Ref();
			ProxyDirection = m_DirectionLower;

			v3 dir = ProxyDirection;
		}
		else
		{
			rvector _vDir;
			if (!GetHistory(&vProxyPosition, &_vDir, ZGetGame()->GetTime()))
				return;

			if (ZGetGame() && ZGetConfiguration()->GetEtc()->bPosture)
			{
				vProxyDirection = m_DirectionLower;

				float fAngle = GetAngleOfVectors(_vDir, vProxyDirection);

				vRot.x = -fAngle * 180 / PI_FLOAT * .9f;
				vRot.y = (_vDir.z + 0.05f) * 50.f;
				vRot.z = 0.f;

				m_pVMesh->SetRotXYZ(vRot);
			}
			else
				ProxyDirection = _vDir;
			UpdateDirection(fDelta, _vDir);
		}
	}
	if (ZGetGame()->GetMatch()->IsClassiMode())
	{
		v3 ProxyDirection;
		if (pObserver->IsVisible())
		{
			UpdateDirection(fDelta, m_TargetDir);
			m_vProxyPosition = m_Position.Ref();
			ProxyDirection = m_DirectionLower;

			v3 dir = ProxyDirection;
		}
		else
		{
			rvector _vDir;
			if (!GetHistory(&vProxyPosition, &_vDir, ZGetGame()->GetTime()))
				return;

			if (ZGetGame()->GetMatch()->IsClassiMode())
			{
				vProxyDirection = m_DirectionLower;

				float fAngle = GetAngleOfVectors(_vDir, vProxyDirection);

				vRot.x = -fAngle * 180 / PI_FLOAT * .9f;
				vRot.y = (_vDir.z + 0.05f) * 50.f;
				vRot.z = 0.f;

				m_pVMesh->SetRotXYZ(vRot);
			}
			else
				ProxyDirection = _vDir;
			UpdateDirection(fDelta, _vDir);
		}
	}
	else if (pObserver->IsVisible())
	{
		rvector _vDir;
		if (!GetHistory(&vProxyPosition, &_vDir, ZGetGame()->GetTime() - pObserver->GetDelay()))
			return;

		vProxyDirection = m_DirectionLower;

		float fAngle = GetAngleOfVectors(_vDir, vProxyDirection);

		vRot.x = -fAngle * 180 / pi * .9f;
		vRot.y = (_vDir.z + 0.05f) * 50.f;
		vRot.z = 0.f;

		m_pVMesh->SetRotXYZ(vRot);
	}
	else
	{
		vProxyPosition = GetPosition();
		vProxyDirection = m_DirectionLower;
	}

	if (IsDie()) {
		vProxyDirection = m_Direction;
	}

	vProxyDirection.z = 0;
	Normalize(vProxyDirection);

	if (m_nVMID.Ref() == -1) return;		

	D3DXMATRIX world;
	MakeWorldMatrix(&world, rvector(0, 0, 0), vProxyDirection, rvector(0, 0, 1));

	rvector MeshPosition;

	// Custom: Ghost Mode - Desperate
	if (IsMoveGhost()) 
	{
		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_GHOST) 
			{
				m_dwStatusBitPackingValue.Ref().m_bGhost = true;
				 GhostTime = timeGetTime(); 
			}
	}

	if (IsMoveAnimation())		
	{
		rvector footposition = m_pVMesh->GetFootPosition();

		rvector RealPosition = footposition * world;


		if (m_AniState_Lower.Ref() == ZC_STATE_LOWER_RUN_WALL)
		{

			rvector headpos = rvector(0.f, 0.f, 0.f);

			if (m_pVMesh) {

				AniFrameInfo* pAniLow = m_pVMesh->GetFrameInfo(ani_mode_lower);
				AniFrameInfo* pAniUp = m_pVMesh->GetFrameInfo(ani_mode_upper);

				m_pVMesh->GetMesh()->SetAnimation(pAniLow->m_pAniSet, pAniUp->m_pAniSet);
				m_pVMesh->GetMesh()->SetFrame(pAniLow->m_nFrame, pAniUp->m_nFrame);
				m_pVMesh->GetMesh()->SetMeshVis(m_pVMesh->GetVisibility());
				m_pVMesh->GetMesh()->SetVisualMesh(m_pVMesh);

				m_pVMesh->GetMesh()->RenderFrame();

				RMeshNode* pNode = NULL;

				pNode = m_pVMesh->GetMesh()->FindNode(eq_parts_pos_info_Head);

				if (pNode) {

					headpos.x = pNode->m_mat_result._41;
					headpos.y = pNode->m_mat_result._42;
					headpos.z = pNode->m_mat_result._43;
				}
			}

			rvector rootpos = 0.5f * (footposition + headpos) * world;

			MeshPosition = vProxyPosition + rvector(0, 0, 90) - rootpos;
		}
		else
			MeshPosition = vProxyPosition - RealPosition;

		m_AnimationPositionDiff.Set_CheckCrc(footposition - m_RealPositionBefore.Ref());

		m_AnimationPositionDiff.Set_CheckCrc(m_AnimationPositionDiff.Ref() * world);

		m_RealPositionBefore.Set_CheckCrc(footposition);

	}
	else
	MeshPosition = vProxyPosition;

	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();


	MakeWorldMatrix(&world, MeshPosition, vProxyDirection, rvector(0, 0, 1));
	m_pVMesh->SetWorldMatrix(world);

	rvector cpos = ZApplication::GetGameInterface()->GetCamera()->GetPosition();
	cpos = vProxyPosition - cpos;
	float dist = Magnitude(cpos);

	uStatus.m_bIsLowModel = (fabs(dist) > 3000.f);
	if (uStatus.m_bFallingToNarak) uStatus.m_bIsLowModel = false;	

	uStatus.m_bDamaged = false;

	CheckLostConn();

	if (m_bCharging->Ref() && (m_AniState_Lower.Ref() != ZC_STATE_CHARGE && m_AniState_Lower.Ref() != ZC_STATE_LOWER_ATTACK1)) {
		m_bCharging->Set_CheckCrc(false);
	}

	if (m_bCharged->Ref() && ZGetGame()->GetTime() > m_fChargedFreeTime.Ref()) {
		m_bCharged->Set_CheckCrc(false);
	}


	UpdateSpWeapon();
}

void ZCharacter::CheckLostConn()
{
	//jintriple3 ºñÆ® ÆÐÅ· ¸Þ¸ð¸® ÇÁ·Ï½Ã...
	ZCharaterStatusBitPacking & uStatus =m_dwStatusBitPackingValue.Ref();
	// ¸¶Áö¸·À¸·Î µ¥ÀÌÅÍ¸¦ ¹ÞÀºÁE1ÃÊ°¡ Áö³µÀ¸¸E¸Ó¸®¿¡ ¾ÆÀÌÄÜ ¶ç¿öÁÖÀÚ
	if (ZGetGame()->GetTime()-m_fLastReceivedTime > 1.f)
	{
		if(!uStatus.m_bLostConEffect)
		{
			uStatus.m_bLostConEffect=true;
			ZGetEffectManager()->AddLostConIcon(this);
		}
		SetVelocity(rvector(0,0,0));
	}else
		uStatus.m_bLostConEffect=false;
}

float ZCharacter::GetMoveSpeedRatio()
{
	float fRatio = 1.f;

	MMatchItemDesc* pMItemDesc = GetSelectItemDesc();

	if(pMItemDesc)
	{
		if( pMItemDesc->m_nLimitSpeed.Ref() <=100)		//Á¤»EÀ¯ÀúµE.
			fRatio = pMItemDesc->m_nLimitSpeed.Ref()/100.f;
		else					//ÇØÄ¿µE..ÁE» °³ ´À·ÁÁú²¬..¤»¤»¤»¤»
			fRatio = 0.1f;
	}

	// Custom: Slow down user while scoping
	if( ZGetGame()->m_pMyCharacter )
	{
		if( ZGetGame()->m_pMyCharacter->m_statusFlags.Ref().m_bSniferMode )
			fRatio *= (60/100.0f);
	}

	// Custom: Speed room mod
	if( ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed( MMOD_SPEED ) && !ZGetGameClient()->GetMatchStageSetting()->IsQuestDrived()
		&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		int nPercent = ZGetGameClient()->GetMatchStageSetting()->GetModifierValue( MMOD_SPEED );

		if( nPercent < 0 || nPercent > 300 )
			nPercent = 100;

		fRatio *= (nPercent/100.0f);
	}
	if (ZGetGame()->GetMatch()->IsTraining() && GetRGMain().TrainingSettings.Speed) 
	{
		int nPercent = 135.0f;
		fRatio *= (nPercent / 100.0f);
	}
	// Custom: RollTheDice Speed Roll
	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_ROLLTHEDICE))
	{
		if(ZGetGame()->GetRolledDice() == 7)
		{
			int nPercent = 125.0f;
			fRatio *= (nPercent/100.0f);
		}

		if(ZGetGame()->GetRolledDice() == 8)
		{
			int nPercent = 75.0f;
			fRatio *= (nPercent/100.0f);
		}
	}
	if (ZGetGame()->GetMatch()->IsTurboMode())
	{
		int nPercent = 115.0f;
		fRatio *= (nPercent / 100.0f);
	}
	if( ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_INFECTED )
	{
		if( m_bInfected )
			fRatio *= (120/100.0f);
	}
	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_GHOST) // Custom: Ghost Mode
	{
		if (GetTeamID() == MMT_RED)
		{
			fRatio *= (140 / 100.0f);
		}
	}

	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_FPS))
	{
		if( this->GetUID() == ZGetGame()->m_pMyCharacter->GetUID() && ZGetGame()->m_pMyCharacter->m_bSprint )
		{
			int nSprintFactor = ZGetGame()->m_pMyCharacter->m_nSprintFactor;
			fRatio *= (nSprintFactor/100.0f);
		}
	}
#ifdef _PAINTMODE
	if (ZGetGame()->GetMatch()->IsPaintBall())
	{
		if (this->GetUID() == ZGetGame()->m_pMyCharacter->GetUID() && ZGetGame()->m_pMyCharacter->m_bSprint)
		{
			int nSprintFactor = ZGetGame()->m_pMyCharacter->m_nSprintFactor;
			fRatio *= (nSprintFactor / 100.0f);
		}
	}
#endif
	//Speed Hack Call
	if (ZGetGame()->m_pMyCharacter->GetStatus().Ref().isSpeed == 1)
	{
		int nPercent = 160.0f;
		fRatio *= (nPercent / 50.0f);
	}
	//if (ZGetGame() && ZGetConfiguration()->GetEtc()->bSlow)
	//{
	//	int nPercent = 50.0f;
	//	fRatio *= (nPercent / 200.0f);
	//}
	return m_pModule_Movable->GetMoveSpeedRatio()*fRatio;
}

// °¨¼ÓÀº mycharacter¿Í °øÅE¸·Î ¾´´Ù
void ZCharacter::UpdateVelocity(float fDelta)
{
	rvector dir = rvector(GetVelocity().x, GetVelocity().y, 0);
	float fSpeed = Magnitude(dir);
	Normalize(dir);

	float fRatio = GetMoveSpeedRatio();

	float max_speed = MAX_SPEED * fRatio;

	// ÃÖ´EÓµµ ÀÌ»óÀÌ¸EÃÖ´EÓµµ¿¡ ¸ÂÃá´Ù..
	if(fSpeed > max_speed) fSpeed = max_speed;

	//jintriple3 ¸Þ¸ð¸® ÇÁ·Ï½Ã...ºñÆ® ÆÐÅ·..
	const ZCharaterStatusBitPacking & uStatus = m_dwStatusBitPackingValue.Ref();

	bool bTumble= !IsDie() && (uStatus.m_bTumble ||
		(ZC_STATE_LOWER_TUMBLE_FORWARD<=m_AniState_Lower.Ref() && m_AniState_Lower.Ref()<=ZC_STATE_LOWER_TUMBLE_LEFT));

	if(uStatus.m_bLand && !uStatus.m_bWallJump && !bTumble)
	{
		// ´Þ¸®´Â ¼Óµµ ÀÌ»óÀÌ¸EºE£°Ô °¨¼ÓÇÑ´Ù.
		rvector forward=m_TargetDir;
		forward.z=0;
		Normalize(forward);

		// ÃÖ´EªÀ» ºñÀ²·Î Á¦¾ûãÑ´Ù.
		float run_speed = RUN_SPEED * fRatio;
		float back_speed = BACK_SPEED * fRatio;
		float stop_formax_speed = STOP_FORMAX_SPEED * (1/fRatio);  

		if(DotProduct(forward,dir)>cosf(10.f* PI_FLOAT /180.f))	//Custom: Character Posture
		{
			if(fSpeed>run_speed)
				fSpeed=max(fSpeed-stop_formax_speed*fDelta,run_speed);
		}
		else
		{
			if(fSpeed>back_speed)
				fSpeed=max(fSpeed-stop_formax_speed*fDelta,back_speed);
		}
	}

//	if(fSpeed>200)
//		int k=0;
	// ¾Æ¹«Å°µµ ¾È´­·ÁÀÖÀ¸¸E°¨¼ÓÇÑ´Ù.
	if(IS_ZERO(Magnitude(m_Accel.Ref())) && (uStatus.m_bLand && !uStatus.m_bWallJump && !uStatus.m_bWallJump2 && !bTumble ))
	{
		if( uStatus.m_bBlast && m_nBlastType.Ref() == 1) {
			// ´EÅ ´E¬ °ø°Ý¹ÞÀº »óÅÂ¶ó¸EÁ¦¿Ü..
		}
		else {
			fSpeed = max(fSpeed -STOP_SPEED * fDelta, 0);
		}
	}

	SetVelocity(dir.x * fSpeed, dir.y * fSpeed, GetVelocity().z);
}

void ZCharacter::UpdateAnimation()
{
	if (m_bInitialized==false) return;
	SetAnimationLower(ZC_STATE_LOWER_IDLE1);
}

//bool ZCharacter::Move(rvector &diff)
//{
//	if (m_bInitialized==false) return false;
//	if (!IsVisible()) return false;
//
//#ifdef ENABLE_CHARACTER_COLLISION
#define CHARACTER_COLLISION_DIST	70.f
//	// Ä³¸¯ÅÍ³¢¸®ÀÇ Ãæµ¹Ã¼Å©¸¦ ÇÑ´Ù.
///*
//	rvector dir=m_Position+diff-m_Position;
//	//float t = ColTest(m_Position, dir, );
//	rplane pln;
//	//float t = SweepTest(rsphere(m_Position, 100.0f), dir, rsphere(rvector(0.0f,0.0f,0.0f), 100.0f), &pln);
//
//	rvector pos22 = m_Position + rvector(0.0f, 0.0f, 60.0f);
//	rcapsule cap = rcapsule(rvector(0.0f,0.0f,0.0f), rvector(0.0f, 0.0f, 180.0f), 100.0f);
//
//	float t = SweepTest(rsphere(pos22, 100.0f), dir, cap, &pln);
//	if (t < 1.0f) {
//		return false;
//	}
//*/
//	if(!IsDie())
//	{
//		ZObjectManager *pcm=&g_pGame->m_ObjectManager;
//		for (ZObjectManager::iterator itor = pcm->begin(); itor != pcm->end(); ++itor)
//		{
//			ZObject* pObject = (*itor).second;
//			if (pObject != this && pObject->IsCollideable())
//			{
//				rvector pos=pObject->m_Position;
//				rvector dir=m_Position+diff-pos;
//				dir.z=0;
//				float fDist=Magnitude(dir);
//				if(fDist<CHARACTER_COLLISION_DIST && fabs(pos.z-m_Position.z)<180.f)
//				{
//					// °ÅÀÇ °°ÀºÀ§Ä¡¿¡ ÀÖ´Â °æ¿E. ÇÑÂÊ¹æÇâÀ¸·Î ¹Ð¸²
//					if(fDist<1.f)
//					{
//						pos.x+=1.f;
//						dir=m_Position-pos;
//					}
//
//					if(DotProduct(dir,diff)<0)	// ´E°¡±ûÛöÁö´Â ¹æÇâÀÌ¸E
//					{
//						Normalize(dir);
//						rvector newthispos=pos+dir*(CHARACTER_COLLISION_DIST+1.f);
//
//						rvector newdiff=newthispos-m_Position;
//						diff.x=newdiff.x;
//						diff.y=newdiff.y;
//
//					}
//				}
//			}
//		}
//	}
//#endif
//	// ÀÌ·¸°ÔµÇ¸E195cm Á¤µµ±ûÝE¸øÁö³ª°£´Ù.
//	
//	rvector origin,targetpos;
//	rplane impactplane;
//
//	origin=m_Position+rvector(0,0,120);
//	targetpos=origin+diff;
//	m_bAdjusted=ZGetGame()->GetWorld()->GetBsp()->CheckWall(origin,targetpos,CHARACTER_RADIUS,60,RCW_CYLINDER,0,&impactplane);
//
//	/*
//	// Áö±Ý ÅÎ¿¡°É·Á ¿Ã¶ó°¡´Â »óÈ²ÀÌ ¾Æ´Ï¶ó¸EÅÎ¿¡ °É¸°°ÍÀÎÁEÃ¼Å©.
//
//	// Á¶°Ç 1. ¸·Çô¼­ °ÅÀÇ ¸ø¿òÁ÷ÀÌ´Â »óÅÂÀÌ¸E°ÅÀÇ À§¾Æ·¡·Î´Â ¿òÁ÷ÀÌÁE¾ÊÀ»¶§
//
//	if(Magnitude(GetVelocity())>0.01f)
//	{
//		if(!m_bClimb && 
//			//Magnitude(targetpos-origin)<0.1f && 
//			m_bAdjusted &&
//			fabs(GetVelocity().z)<1.f )
//		{
//			// 2. ÇöÀEµó°úÜÖ´Â ¹Ù´ÚÀÌ °ÅÀÇ ¼öÁ÷ÀÎ ¹Ù´ÚÀÎ°¡
//			// 2. ÇöÀEºÎµúÈE¸éÀÌ °ÅÀÇ ¼öÁ÷ÀÎ°¡ !
//			if(D3DXPlaneDotNormal(&impactplane,&rvector(0,0,1))<0.01f)
//			{
//				// 4. À§·Î 50cm ¸¸Å­ °¥¼öÀÖ´Â°¡ ? 
//				rvector uptest_origin=m_Position+rvector(0,0,90);
//				rvector uptest_targetpos=uptest_origin+rvector(0,0,50);
//				rplane testimpactplane;
//
//				bool bAdjust=ZGetGame()->GetWorld()->GetBsp()->CheckWall(uptest_origin,uptest_targetpos,CHARACTER_RADIUS,89,RCW_CYLINDER,0,&testimpactplane);
//				if(!bAdjust || Magnitude(uptest_targetpos-uptest_origin)>45.f)
//				{
//					rvector forwardtest_origin=uptest_targetpos;
////					rvector dir=-rvector(impactplane.a,impactplane.b,impactplane.c);
//					rvector dir=diff;
//					Normalize(dir);
//					rvector forwardtest_targetpos=forwardtest_origin+30.f*dir;
//
//					bool bAdjust=ZGetGame()->GetWorld()->GetBsp()->CheckWall(forwardtest_origin,forwardtest_targetpos,CHARACTER_RADIUS,89,RCW_CYLINDER,0,&testimpactplane);
//					// 3. 50cm ¸¸Å­ À§¿¡¼­ º®¹æÇâÀ¸·Î ÁøÇàÇÏ¸E°¥¼öÀÖ´Â°¡ ? 
//					if(!bAdjust || (Magnitude(forwardtest_targetpos-forwardtest_origin)>20.f && impactplane!=testimpactplane) )
//					{
//						m_bClimb=true;
//						m_ClimbDir=GetVelocity();
//						Normalize(m_ClimbDir);
//						m_fClimbZ=m_Position.z;
//						m_ClimbPlane=impactplane;
//						return m_bAdjusted;
//					}
//				}
//			}
//		}
//	}
//	*/
//
//	if(m_bAdjusted)
//	{
//		m_fLastAdjustedTime=g_pGame->GetTime();
//	}
//
//	diff=targetpos-origin;
//
//	m_Position+=diff;
////	m_RealPosition+=diff;
////	m_RealPositionBefore+=diff;
//
//	return m_bAdjusted;
//}

/*
	ÀÓ½Ã ->	È®ÀEÇÊ¿E¾øÀ¸¸E¸â¹ö·Î ¿Å±â°Å³ª ,
	Ä¿Áú°Í °°À¸¸EÆÄÀÏ°E®¸¦ Æ÷ÇÔÇÏ´Â class ¸¸µé±E.
*/

#define ST_MAX_WEAPON 200
#define ST_MAX_PARTS  200

struct WeaponST {
	int		id;
	char*	name;
	RWeaponMotionType weapontype;//¹«±âÁ¾·E
};

// µûÓÏ ÀÌ¸§ , Å¸ÀÙ..
/*
WeaponST g_WeaponST[ ST_MAX_WEAPON ] = {
//	{ 0,"w_none",	eq_weapon_etc},
//	{ 0,"katana01",	eq_ws_pistol },
//	{ 1,"rifle01",	eq_ws_pistol },
	{ 0,"pistol01",	eq_ws_pistol },
	{ 1,"pistol02",	eq_ws_pistol },
//	{ 0,"katana01",	eq_ws_pistol },
//	{ 1,"rifle01",	eq_ws_pistol },
	{ 2,"katana01",	eq_wd_katana },
	{ 3,"rifle01",	eq_wd_rifle  },

};
*/

// µûÓÏ mesh_id , µûÓÏÀÌ¸§ , ¸ð¼Ç ¿¬°EÅ¸ÀÙ

WeaponST g_WeaponST[ ST_MAX_WEAPON ] = {
	{ 0,"pistol01",	eq_wd_katana },
	{ 1,"pistol02",	eq_wd_katana },
	{ 2,"katana01",	eq_wd_katana },
	{ 3,"rifle01",	eq_wd_rifle  },
};

void ZCharacter::OnSetSlot(int nSlot,int WeaponID)
{
	
}

void ZCharacter::SetTargetDir(rvector vTarget) {

	Normalize(vTarget);
	m_TargetDir = vTarget;
//	m_dwBackUpTime = timeGetTime();
}

void ZCharacter::OnChangeSlot(int nSlot)
{
	if (m_bInitialized==false) return;
	if(nSlot < 0 || nSlot > ZC_SLOT_END-1) 
		return;

	if( m_pVMesh ) {


		MEMBER_SET_CHECKCRC(m_slotInfo, m_nSelectSlot, nSlot);

		int nWeaponID = m_slotInfo.Ref().m_Slot[nSlot].m_WeaponID;

		int SelModelID  = nWeaponID;
		int SelToggleID = nWeaponID;

		RWeaponMotionType type = eq_weapon_etc;

		if( nWeaponID != -1 )
			type = g_WeaponST[ nWeaponID ].weapontype;

		// ÀÌ¸§À¸·Î °Ë»öÇÏ±E.

		RMesh* pMesh = ZGetWeaponMeshMgr()->GetFast(SelModelID);

		if( pMesh ) {

			if( SelModelID == -1 ) {
				m_pVMesh->RemoveWeapon(type);
			}
			else  {
				CheckTeenVersionMesh(&pMesh);
				m_pVMesh->AddWeapon(type , pMesh);
				m_pVMesh->SelectWeaponMotion(type);
			}
		}
	}
}

// °¡ÁE¹«±âÁß¿¡¼­¸¸ ¼±ÅÃÇÏ°Ô µÈ´Ù...

void ZCharacter::OnChangeWeapon(char* WeaponModelName)
{
	// ÃÊ±E¹«±â°¡ Æ²·ÁÁö´Â ¹®Á¦ÀÏ¼öµµ..
	// ·ÎµùÀÌ ¾ÈµÇ¾ú´Ù¸E¸ð¼ÇÀº µ¿±âÈ­°¡ ¾ÈµÈ´Ù? À¯ÀúµéÀÇ ·Îµù¼Óµµ¿¡ µû¶EÆ²·ÁÁú¼öµµ?

	if(m_bInitialized==false) 
		return;

	if( m_pVMesh ) {

		RWeaponMotionType type = eq_weapon_etc;

		RMesh* pMesh = ZGetWeaponMeshMgr()->Get( WeaponModelName );

		if( pMesh ) {

			type = pMesh->GetMeshWeaponMotionType();

			CheckTeenVersionMesh(&pMesh);

			m_pVMesh->AddWeapon(type , pMesh);
			m_pVMesh->SelectWeaponMotion(type);
			UpdateLoadAnimation();//»óÅÂ´Â ±×´EÎÁö¸¸ ¹«±â°¡¹Ù²ûÚúÀ¸´Ï ¸ð¼ÇÆÄÀÏÀÌ ¹Ù²ûÚûÚßÇÑ´Ù..
		}

		if (eq_wd_katana == type || eq_wd_spycase == type)
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("fx_blade_sheath",GetPosition(),IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySoundSheath(m_Items.GetSelectedWeapon()->GetDesc(),GetPosition(),IsObserverTarget());
#endif
		}
		else if( (eq_wd_dagger == type) || (eq_ws_dagger == type) )
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("fx_dagger_sheath",GetPosition(),IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("fx_dagger_sheath",GetPosition(), IsObserverTarget());
#endif
		}
		else if( eq_wd_sword == type )
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("fx_dagger_sheath",GetPosition(),IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("fx_dagger_sheath",GetPosition(), IsObserverTarget());
#endif
		}
		else if( eq_wd_blade == type )
		{
#ifdef _BIRDSOUND
			ZGetSoundEngine()->PlaySoundCharacter("fx_dagger_sheath",GetPosition(),IsObserverTarget());
#else
			ZGetSoundEngine()->PlaySound("fx_dagger_sheath",GetPosition(), IsObserverTarget());
#endif
		}
	}
	
}

// µðÀÚÀÌ³Ê ÀÛ¾E¶§¹®¿¡... Debug ¸ðµå¿¡¼­¸¸ »ç¿E

char* GetPartsNextName(RMeshPartsType ptype,RVisualMesh* pVMesh,bool bReverse)
{

	static bool bFirst = true;
	static vector<RMeshNode*> g_table[6*2];
	static int g_parts[6*2];

	if(bFirst) {

		RMesh* pMesh = ZGetMeshMgr()->Get("heroman1");//¿øÇÏ´Â ¸ðµ¨À» ºÙ¿©ÁÖ±E.

		if(pMesh) { //man

			pMesh->GetPartsNode( eq_parts_chest,g_table[0]);
			pMesh->GetPartsNode( eq_parts_head ,g_table[1]);
			pMesh->GetPartsNode( eq_parts_hands,g_table[2]);
			pMesh->GetPartsNode( eq_parts_legs ,g_table[3]);
			pMesh->GetPartsNode( eq_parts_feet ,g_table[4]);
			pMesh->GetPartsNode( eq_parts_face ,g_table[5]);
		}

		pMesh = ZGetMeshMgr()->Get("herowoman1");//¿øÇÏ´Â ¸ðµ¨À» ºÙ¿©ÁÖ±E.
		
		if(pMesh) { //woman

			pMesh->GetPartsNode( eq_parts_chest,g_table[6]);
			pMesh->GetPartsNode( eq_parts_head ,g_table[7]);
			pMesh->GetPartsNode( eq_parts_hands,g_table[8]);
			pMesh->GetPartsNode( eq_parts_legs ,g_table[9]);
			pMesh->GetPartsNode( eq_parts_feet ,g_table[10]);
			pMesh->GetPartsNode( eq_parts_face ,g_table[11]);
		}

		bFirst = false;
	}

	int mode = 0;

		 if(ptype==eq_parts_chest)	mode = 0;
	else if(ptype==eq_parts_head)	mode = 1;
	else if(ptype==eq_parts_hands)	mode = 2;
	else if(ptype==eq_parts_legs)	mode = 3;
	else if(ptype==eq_parts_feet)	mode = 4;
	else if(ptype==eq_parts_face)	mode = 5;
	else return NULL;

	// isman()

	if(pVMesh) {
		if(pVMesh->GetMesh()) {
			if(strcmp(pVMesh->GetMesh()->GetName(),"heroman1")!=0) {
				mode +=6;
			}
		}
	}

	if(bReverse) { // ¿ª¼ø°Ë»E.

		g_parts[mode]--;

		if(g_parts[mode] < 0) {
			g_parts[mode] = (int)g_table[mode].size()-1;
		}

	}
	else {

		g_parts[mode]++;

		if(g_parts[mode] > (int)g_table[mode].size()-1) {
			g_parts[mode] = 0;
		}
	}

	return g_table[mode][g_parts[mode]]->GetName();
}

void ZCharacter::OnChangeParts(RMeshPartsType partstype,int PartsID)
{
#ifndef _PUBLISH
	//if (m_bInitialized==false) return;
	//if( m_pVMesh ) {

	//	// ±âÈ¹¼­¿¡ Á¤ÇØÁE°úÜ¯ÇÑ ÀÌ¸§ÀÌ ÀÖ¾ûÚß ÇÑ´Ù..

	//	if(partstype > eq_parts_etc && partstype < eq_parts_left_pistol) {
	//	
	//		if(PartsID == 0) { // clear
	//			m_pVMesh->SetBaseParts( partstype );
	//		}
	//		else {

	//			char* Name = NULL;

	//			if(MEvent::GetCtrlState()) {
	//				Name = GetPartsNextName( partstype,m_pVMesh ,true);//ÀÌÀEÊ...
	//			}
	//			else {
	//				Name = GetPartsNextName( partstype,m_pVMesh ,false);
	//			}

	//			if(Name)
	//				m_pVMesh->SetParts( partstype, Name );
	//		}
	//	}
	//}

	//if(Enable_Cloth)
	//	m_pVMesh->ChangeChestCloth(1.f,1);
#endif
}


void ZCharacter::OnAttack(int type,rvector& pos)
{

}

float ZCharacter::GetMaxHP()
{
	// Custom: Fixed negative HP/AP and changed to suit vanilla mode
	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_VANILLA))
	{
		if (m_Property.fMaxHP.Ref() > 125)
			return 125.f;

		//return max(0, m_pModule_HPAP->GetMaxHP());
	}
	return max(0, /*floor(*/m_Property.fMaxHP.Ref());//);
	//return m_Property.fMaxHP.Ref();
	
	/* //¹öÇÁÁ¤º¸ÀÓ½ÃÁÖ¼®  
	float fRatio = GetHP() / m_fPreMaxHP; //<- m_fPreMaxHP°¡ 0ÀÎ °æ¿EÀÖÀ½ ¼öÁ¤ ÇÊ¿E
	float fCurMaxHP = m_Property.fMaxHP.Ref() + GetCharacterBuff()->GetBuffSummary()->GetHP();
	
	if( fCurMaxHP != m_fPreMaxHP ) {
		SetMaxHP(fCurMaxHP);
		m_fPreMaxHP = fCurMaxHP;
		SetHP(fCurMaxHP * fRatio);

#ifdef _DEBUG
		mlog("ZCharacter::GetMaxHP() - preHP(%f), preMaxHP(%f), curHP(%f), curMaxHP(%f)\n", GetHP(), m_fPreMaxHP, fCurMaxHP * fRatio, fCurMaxHP);
#endif
	}

	return fCurMaxHP;*/
}

float ZCharacter::GetMaxAP()
{
	// Custom: Fixed negative HP/AP and changed to suit vanilla mode
	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_VANILLA))
	{
		if (m_Property.fMaxAP.Ref() > 100)
			return 100.f;

	//	return max(0, m_pModule_HPAP->GetMaxAP());
	}
	// Custom: 0 AP RoomTAG Event
	/*if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_EVENTMODE))
	{
		if (m_Property.fMaxAP.Ref() > 0)
			return 0.f;

		//	return max(0, m_pModule_HPAP->GetMaxAP());
	}*/
	return max(0, /*floor(*/m_Property.fMaxAP.Ref());/*);*/
	//return m_Property.fMaxAP.Ref();
	
	/* //¹öÇÁÁ¤º¸ÀÓ½ÃÁÖ¼®
	float fRatio = GetAP() / m_fPreMaxAP; //<- m_fPreMaxAp°¡ 0ÀÎ °æ¿EÀÖÀ½ ¼öÁ¤ ÇÊ¿E
	float fCurMaxAP = m_Property.fMaxAP.Ref() + GetCharacterBuff()->GetBuffSummary()->GetAP();

	if( fCurMaxAP != m_fPreMaxAP ) {
		SetMaxAP(fCurMaxAP);
		m_fPreMaxAP = fCurMaxAP;
		SetAP(fCurMaxAP * fRatio);

#ifdef _DEBUG
		mlog("ZCharacter::GetMaxAP() - preAP(%f), preMaxAP(%f), curAP(%f), curMaxAP(%f)\n", GetAP(), m_fPreMaxAP, fCurMaxAP * fRatio, fCurMaxAP);
#endif
	}

	return fCurMaxAP;*/
}

float ZCharacter::GetHP()	
{
	return m_pModule_HPAP->GetHP(); 
	// Custom: Round down
	//return floor(m_pModule_HPAP->GetHP()); 
}

float ZCharacter::GetAP()	
{
	return m_pModule_HPAP->GetAP(); 
	// Custom: Round up
	//return floor(m_pModule_HPAP->GetAP()); 
}

void ZCharacter::InitAccumulationDamage()
{
	m_pModule_HPAP->InitAccumulationDamage();
}
float ZCharacter::GetAccumulationDamage()	{ 
	return m_pModule_HPAP->GetAccumulationDamage(); 
}

void ZCharacter::EnableAccumulationDamage(bool bAccumulationDamage)	{ 
	m_pModule_HPAP->EnableAccumulationDamage(bAccumulationDamage); 
}

// ¸ðµEÄ³¸¯ÅÍÀÇ HP/AP ÀÇ º¯°æÀº ÀÌ Æò¼ÇÀ» ÅEØ ÀÌ·ç¾ûÝø´Ù
//void ZCharacter::SetHP(float nHP)
//{ 
//	m_pModule_HPAP->SetHP(nHP);
//
//	CHECK_RETURN_CALLSTACK(SetHP);
//}
//
//void ZCharacter::SetAP(float nAP)
//{ 
//	m_pModule_HPAP->SetAP(nAP); 
//
//	CHECK_RETURN_CALLSTACK(SetAP);
//}

void ZCharacter::Die()
{
	OnDie();
}

void ZCharacter::OnDie()
{
	if (m_bInitialized==false) 
	{
		CHECK_RETURN_CALLSTACK(OnDie);
		return;
	}
	if (!IsVisible())
	{
		CHECK_RETURN_CALLSTACK(OnDie);
		return;
	}
/*
	if ((GetStateLower() != ZC_STATE_LOWER_DIE1) && (GetStateLower() != ZC_STATE_LOWER_DIE2))
	{
		if(DotProduct(m_Direction,m_LastDamageDir)<0)
			SetAnimationLower(ZC_STATE_LOWER_DIE1);
		else
			SetAnimationLower(ZC_STATE_LOWER_DIE2);
	}
	if (GetStateUpper() != ZC_STATE_UPPER_NONE)
	{
		SetAnimationUpper(ZC_STATE_UPPER_NONE);
	}
*/
	//jintriple3 ¸Þ¸ð¸® ÇÁ·Ï½Ã...ºñÆ® ÆÐÅ·..
	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	if (uStatus.m_bChatEffect) uStatus.m_bChatEffect = false;

#ifdef _ICONCHAT
	if (uStatus.m_bChatVoice) uStatus.m_bChatVoice = false;
	if (m_Status.Ref().isTalking = true) m_Status.Ref().isTalking = false; 
#endif
	uStatus.m_bDie = true;
	m_Collision.SetCollideable(false);
	uStatus.m_bPlayDone = false;
	

	if ( uStatus.m_bFallingToNarak == true)
	{
		if (m_Property.nSex == MMS_MALE)
			ZGetSoundEngine()->PlaySound("fx2/MAL10",GetPosition(),IsObserverTarget());
		else
			ZGetSoundEngine()->PlaySound("fx2/FEM10",GetPosition(),IsObserverTarget());
	}
	else
	{
		if (m_Property.nSex == MMS_MALE)
			ZGetSoundEngine()->PlaySound("fx2/MAL08",GetPosition(),IsObserverTarget());
		else
			ZGetSoundEngine()->PlaySound("fx2/FEM08",GetPosition(),IsObserverTarget());
	}
	//prevent players from bugging duels by suicidig 
	if (this->GetUID() == ZGetGame()->m_pMyCharacter->GetUID())
		ZGetGame()->CancelSuicide();

	CHECK_RETURN_CALLSTACK(OnDie);
}

// ºÎÈ° - ÀÌ°ÍÀº °ÔÀÓ·E¡ µû¶E´Þ¶óÁE¼öµµ ÀÖ´Ù.
void ZCharacter::Revival()
{
	if (m_bInitialized==false) return;
//	if (!IsVisible()) return;

	InitStatus();

	//jintriple3 ¸Þ¸ð¸® ÇÁ·Ï½Ã...ºñÆ® ÆÐÅ·..
	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	// Custom: Fix for unscoping!
	uStatus.m_bSniping = false;
	// Custom: Allow players to move again after dying
	uStatus.m_bFrozen = false;
	// Custom: Damage counter

	m_fDamageCaused = 0.f;

	if (this == ZGetGame()->m_pMyCharacter)
	{
		// Custom: Disable sniper scope (if observer didn't disable) after respawning
		ZCombatInterface* ci=ZGetCombatInterface();
		if (ci)
			ci->OnGadgetOff();

		// Custom: Bugfix for all gamemodes (force camera re-initialize)
		ZCamera* pCamera = ZGetGameInterface()->GetCamera();
		pCamera->Init();
	}

	uStatus.m_bDie = false;
	m_Collision.SetCollideable(true);
#ifdef _SPEC 1
	if(IsAdminHide() || GetTeamID() == MMT_SPECTATOR)
		uStatus.m_bDie = true;
#else
	if (IsAdminHide())
		uStatus.m_bDie = true;
#endif
	SetAnimationLower(ZC_STATE_LOWER_IDLE1);
	// Custom: Prevent players from doing that weird bug where they're invisible.
	if (this->GetUID() == ZGetGame()->m_pMyCharacter->GetUID())
		ZGetGame()->CancelSuicide();

	CHECK_RETURN_CALLSTACK(Revival);
}



void ZCharacter::SetDirection(rvector& dir)
{
	m_Direction = dir;
	m_DirectionLower = dir;
	m_DirectionUpper = dir;
	m_TargetDir = dir;
}

/*
void ZCharacter::SetAnimationForce(ZC_STATE nState, ZC_STATE_SUB nStateSub)
{
	m_State = nState;
	m_StateSub = nStateSub;

	char szName[256];
	if (nState != ZC_STATE_IDLE)
		strcpy(szName, g_AnimationInfoSubRunTable[nStateSub].Name);
	else strcpy(szName, "idle");

	SetAnimation(szName, g_AnimationInfoTable[m_State].bEnableCancel, 0);

	if(m_State == ZC_STATE_ATTACK) 
	{
		//¾Æ·¡¸ð¼ÇÀº ³öµÎ°E.»óÃ¼¸¸
		SetAnimation(ani_mode_upper,"attackS",true,0);
	}
}
*/

void ZCharacter::OnKnockback(rvector& dir, float fForce)
{
	// ³²ÀÇ Ä³¸¯ÅÍ´Â ³Ë¹éÀ» ¾ø¾Ø´Ù
	if(IsHero())
	{
		// Custom: Infected game mode
		float fForceMod = fForce;

		if (m_bInfected)
			fForceMod *= 15.f;

		ZCharacterObject::OnKnockback(dir,fForceMod);
	}
}

//void ZCharacter::OnDamagedFalling(float fDamage)
//{
//	if (m_bInitialized==false) return;
//
//	m_LastAttacker = m_UID;
//	m_LastDamageType = ZD_FALLING;
//	m_LastDamageDir = m_Direction;
//
//	SetHP(GetHP() - fDamage);
//}
//
//#define SPLASH_DAMAGE_RATIO	.4f		// ½ºÇÃ·¡½Ã µ¥¹ÌÁE°EEE
//
//void ZCharacter::OnDamagedKatanaSplash(ZCharacter* pAttacker,float fDamageRange)
//{
//	if (m_bInitialized==false) return;
//	if (!IsVisible()) return;
//
//	// °Å¸®¿¡ µû¶ó¼­ µ¥¹ÌÁö~
//	bool bCanAttack = g_pGame->IsAttackable(pAttacker,this);
//
//	if (bCanAttack) m_LastAttacker= pAttacker->m_UID;
//	m_LastDamageType = ZD_KATANA_SPLASH;
//	m_LastDamageDir = m_Position-pAttacker->m_Position;
//	Normalize(m_LastDamageDir);
//
//	int damage = 0;
//
//	MMatchItemDesc* pDesc = pAttacker->GetItems()->GetSelectedWeapon()->GetDesc();
//	if (pDesc == NULL)
//	{
//		_ASSERT(0);
//		return;
//	}
//
//	if (bCanAttack) {
//
//#define SLASH_DAMAGE	3		// °­º£±âµ¥¹ÌÁE= ÀÏ¹Ý°ø°ÝÀÇ x SLASH_DAMAGE
//		damage = (int) pDesc->m_nDamage * fDamageRange * SLASH_DAMAGE;
//
//		OnDamage(damage, SPLASH_DAMAGE_RATIO);
//		//SetHP(GetStatus()->nHP - damage);
//	}
//}
//
//void ZCharacter::OnDamagedGrenade(MUID uidOwner, rvector& dir, float fDamage, int nTeamID)
//{
//	if (m_bInitialized==false) return;
//	if (!IsVisible()) return;
//
//	// °ø°ÝÇÑ »ç¶÷ÀÌ ³ª°¬¾ûÑµ ÀÌ¹Ì ÀÖ´Â ¼ö·ùÅºÀº ÅÍÁ®¾ßÇÑ´Ù
////	if (pAttacker == NULL) return;
//
//	// Áö±Ý µ¥¹ÌÁö¸¦ ÁÙ¼EÀÖ´Â »óÈ²ÀÎ°¡ ?
//	bool bCanAttack = g_pGame->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY;
//
//	if (bCanAttack) {
//
//		m_LastAttacker= uidOwner;
//		m_LastDamageType = ZD_EXPLOSION;
//		m_LastDamageDir = dir;
//
//		ZCharacter* pOwnerCharacter = g_pGame->m_CharacterManager.Find( uidOwner );
//
//		bool bForce = this == pOwnerCharacter;// ÀÚ½ÅÀº ÆÀÇÃÀÌ¶óµµ µ¥¹ÌÁö¸¦ ÀÔ´Â´Ù.
//
//		float fRatio = 1.0f;
//
////		MMatchWeaponType wtype = z_wd_grenade;//pAttacker->m_pVMesh->GetSelectWeaponType();
//
//		fRatio = ZItem::GetPiercingRatio( MWT_FRAGMENTATION , eq_parts_chest );//¼ö·ùÅº°E·ÎÄÏ ±¸ºÐ¾ø´Ù..
//
//		if(bForce || GetTeamID()!=nTeamID)
//			OnDamage((int)fDamage,fRatio);
//
//		if( pOwnerCharacter == g_pGame->m_pMyCharacter )
//			g_pGame->m_pMyCharacter->HitSound();
//	}
//}

//void ZCharacter::OnKnockback(ZItem *pItem, rvector& pos, rvector& dir, int nHitCount)
//{
//	if (m_bInitialized==false) return;
//	if (!IsVisible() || IsDie()) return;
//
//	float fKnockBackForce = 0.0f;
//	bool bMelee = false;
//	bool bKnockBackSkip = false;
//
//	if(nHitCount != -1) {	// Shotgun ÀÓ...
//		if(nHitCount > 5) {	// 6¹ß±ûÝö¸¸ KnockBack ¿¡ ¿µÇâÀ» ÁØ´Ù..
//			bKnockBackSkip = true;
//		}
//	}
//
//	MMatchItemDesc* pDesc = pItem->GetDesc();
//	if (pDesc->m_nType == MMIT_MELEE) {
//		fKnockBackForce = 0.0f;
//		bMelee = true;
//	}
//	else if (pDesc->m_nType == MMIT_RANGE) {
//
//		if (pDesc->m_pEffect != NULL)
//		{
//			fKnockBackForce = (float)(pDesc->m_pEffect->m_nKnockBack);
//		}
//		else
//		{
//			//_ASSERT(0);
//			fKnockBackForce = 0.0f;
//			fKnockBackForce = 200.0f;
//		}
//	}
//	else {
//		_ASSERT(0);
//		return;
//	}
//
//	float fRatio = 1.0f;
//
//	// KnockBack
//	if (IsHero())
//	{
//		if(bKnockBackSkip==false) {
//
//			if(m_bBlast || m_bBlastFall) {	// ¶°ÀÖÀ»¶§ ³Ë¹E
//				rvector vKnockBackDir = dir;
//				Normalize(vKnockBackDir);
//				vKnockBackDir *= (fKnockBackForce * BLASTED_KNOCKBACK_RATIO);
//				vKnockBackDir.x = vKnockBackDir.x * 0.2f;
//				vKnockBackDir.y = vKnockBackDir.y * 0.2f;
//				SetVelocity(vKnockBackDir);
////				SetAccel(vKnockBackDir);
//			} else {	// ±×³É ³Ë¹E
//				rvector vKnockBackDir = dir;
//				Normalize(vKnockBackDir);
//				if (bMelee) vKnockBackDir = rvector(0.0f, 0.0f, 1.0f);
//				KnockBack(vKnockBackDir, fKnockBackForce);
//			}
//		}
//	}
//}

void ZCharacter::UpdateSound()
{
	if (m_bInitialized==false) return;
	if(m_pVMesh) {

		char szSndName[128];
		RMATERIAL* pMaterial = NULL;
		RBSPPICKINFO bpi;
		if(ZGetGame()->GetWorld()->GetBsp()->Pick(GetPosition()+rvector(0,0,100),rvector(0,0,-1),&bpi)) {
			pMaterial = ZGetGame()->GetWorld()->GetBsp()->GetMaterial(bpi.pNode, bpi.nIndex);
		}


		//	¹ßÀÚ±¹ ¼Ò¸® ÇÏµåÄÚµE!    

		AniFrameInfo* pInfo = m_pVMesh->GetFrameInfo(ani_mode_lower);

		int nFrame = pInfo->m_nFrame;//m_pVMesh->m_nFrame[ani_mode_lower];

		int nCurrFoot = 0;

#define FRAME(x) int(float(x)/30.f*4800.f)
		if(m_AniState_Lower.Ref()==ZC_STATE_LOWER_RUN_FORWARD ||	// ÀÏ¹ÝÀûÀÎ °È±E
			m_AniState_Lower.Ref()==ZC_STATE_LOWER_RUN_BACK) {
			
			if(FRAME(8) < nFrame && nFrame < FRAME(18) )	// ´E« 8, 18 ÇÁ·¹ÀÓ¿¡¼­ ¼Ò¸®°¡ ³­´Ù
				nCurrFoot = 1;
		}

		if(m_AniState_Lower.Ref()==ZC_STATE_LOWER_RUN_WALL_LEFT ||	// º®´Þ¸±¶§
			m_AniState_Lower.Ref()==ZC_STATE_LOWER_RUN_WALL_RIGHT ) {

			if (nFrame < FRAME(9) ) nCurrFoot = 1;
			else if (nFrame < FRAME(17) ) nCurrFoot = 0;
			else if (nFrame < FRAME(24) ) nCurrFoot = 1;
			else if (nFrame < FRAME(32) ) nCurrFoot = 0;
			else if (nFrame < FRAME(40) ) nCurrFoot = 1;
			else if (nFrame < FRAME(48) ) nCurrFoot = 0;
			else if (nFrame < FRAME(55) ) nCurrFoot = 1;
		}

		if(m_AniState_Lower.Ref()==ZC_STATE_LOWER_RUN_WALL ) {	// ¾ÕÀ¸·Î º®Å»¶§

			if (nFrame < FRAME(8) ) nCurrFoot = 1;
			else if (nFrame < FRAME(16) ) nCurrFoot = 0;
			else if (nFrame < FRAME(26) ) nCurrFoot = 1;
			else if (nFrame < FRAME(40) ) nCurrFoot = 0;
		}

		//	Áö±Ý ¿Þ¹ßÀÎÁE¿À¸¥¹ßÀÎÁEÆÇ´ÜÇÏ°E¹ßÀÌ ¹Ù²ûÔE¼Ò¸®¸¦ ³½´Ù
		if(m_nWhichFootSound!=nCurrFoot && pMaterial) {	
			if(m_nWhichFootSound==0)
			{	
				// ¿Þ¹ß
				rvector pos = m_pVMesh->GetLFootPosition();
				char *szSndName=ZGetGame()->GetSndNameFromBsp("man_fs_l", pMaterial);

#ifdef _BIRDSOUND
				ZApplication::GetSoundEngine()->PlaySoundCharacter(szSndName,pos,IsObserverTarget());
#else
				ZApplication::GetSoundEngine()->PlaySound(szSndName,pos,IsObserverTarget());
#endif
			}else
			{
				rvector pos = m_pVMesh->GetRFootPosition();
				char *szSndName=ZGetGame()->GetSndNameFromBsp("man_fs_r", pMaterial);
#ifdef _BIRDSOUND
				ZApplication::GetSoundEngine()->PlaySoundCharacter(szSndName,pos,IsObserverTarget());
#else
				ZApplication::GetSoundEngine()->PlaySound(szSndName,pos,IsObserverTarget());
#endif
			}
			m_nWhichFootSound=nCurrFoot;
		}
         
		RAniSoundInfo* pSInfo;
		RAniSoundInfo* pSInfoTable[2];

		rvector p;

		AniFrameInfo* pAniLow = m_pVMesh->GetFrameInfo(ani_mode_lower);
		AniFrameInfo* pAniUp  = m_pVMesh->GetFrameInfo(ani_mode_upper);

		pSInfoTable[0] = &pAniLow->m_SoundInfo;
		pSInfoTable[1] = &pAniUp->m_SoundInfo;

		for(int i=0;i<2;i++) {//»óÇÏ

			pSInfo = pSInfoTable[i];//&m_pVMesh->m_SoundInfo[i];

			if(pSInfo->isPlay) 
			{
				p = pSInfo->Pos;

//				if(strcmp(pSInfo->Name,"fx_dash")==0)
//					int k=0;

				if(pMaterial)	// picking ¼º°øÇßÀ¸¸E
				{
					strcpy(szSndName, ZGetGame()->GetSndNameFromBsp(pSInfo->Name, pMaterial));

					int nStr = (int)strlen( szSndName );
					strncpy( m_pSoundMaterial, szSndName + ( nStr - 6 ), 7 );  

					ZApplication::GetSoundEngine()->PlaySoundElseDefault(szSndName,pSInfo->Name,p,IsObserverTarget());
				}
				else {
					m_pSoundMaterial[0] = 0;

					strcpy(szSndName, pSInfo->Name);
#ifdef _BIRDSOUND
					ZApplication::GetSoundEngine()->PlaySoundCharacter(szSndName,p,IsObserverTarget());
#else
					ZApplication::GetSoundEngine()->PlaySound(szSndName,p,IsObserverTarget());
#endif
				}

				pSInfo->Clear();
			}
		}
	}

	//jintriple3 ºñÆ® ÆÐÅ· ¸Þ¸ð¸® ÇÁ·Ï½Ã...
	ZCharaterStatusBitPacking & uStatus =m_dwStatusBitPackingValue.Ref();
	// TODO : onscream À¸·Î Á¤¸®ÇÏÀÚ
	// Custom: Changed hurt sounds for infected game mode
	if ( ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_INFECTED && uStatus.m_bDamaged && (!IsDie()) && m_bInfected )
	{
		DWORD currTime = timeGetTime();

		if ( (m_damageInfo.Ref().m_dwLastDamagedTime + 5000) < currTime)
		{
			int nRandId = rand() % 6 + 1;

			char szSndName[128];
			memset( szSndName, 0, sizeof( szSndName ) );
			sprintf_s( szSndName, "zombie_pain%d", nRandId );

			ZGetSoundEngine()->PlaySound(szSndName, GetPosition(), IsObserverTarget());

			MEMBER_SET_CHECKCRC(m_damageInfo, m_dwLastDamagedTime, currTime);
		}

		uStatus.m_bDamaged = false;
		return;
	}

	if ( uStatus.m_bDamaged && (!IsDie()) && (GetHP() < 30.f))
	{
		DWORD currTime = timeGetTime();

		if ( (m_damageInfo.Ref().m_dwLastDamagedTime + 5000) < currTime)
		{
			// Male
			if(GetProperty()->nSex==MMS_MALE)
			{
				int nRetVal = ZGetSoundEngine()->PlaySound("fx2/MAL06", GetPosition(), IsObserverTarget());
				if ( nRetVal == 0)
					ZGetSoundEngine()->PlaySound("ooh_male", GetPosition(), IsObserverTarget());
			}

			// Female
			else			
			{
 				int nRetVal = ZGetSoundEngine()->PlaySound("fx2/FEM06", GetPosition(), IsObserverTarget());
				if ( nRetVal == 0)
					ZGetSoundEngine()->PlaySound("ooh_female", GetPosition(), IsObserverTarget());
			}

			MEMBER_SET_CHECKCRC(m_damageInfo, m_dwLastDamagedTime, currTime);
		}

		uStatus.m_bDamaged = false;
	}
}

bool ZCharacter::DoingStylishMotion()
{
	if ((m_AniState_Lower.Ref() >= ZC_STATE_LOWER_RUN_WALL_LEFT) && 
		(m_AniState_Lower.Ref() <= ZC_STATE_LOWER_JUMP_WALL_BACK))
	{
		return true;
	}

	return false;
}

void ZCharacter::UpdateStylishShoted()
{
	//jintriple3 ¸Þ¸ð¸® ÇÁ·Ï½Ã...ºñÆ® ÆÐÅ·..
	ZCharaterStatusBitPacking & uStatus = m_dwStatusBitPackingValue.Ref();

	if (DoingStylishMotion())
	{
		uStatus.m_bStylishShoted = true;
	}
	else
	{
		uStatus.m_bStylishShoted = false;
	}
}

//void ZCharacter::InitHPAP()
//{
//	m_pModule_HPAP->SetMaxHP(m_Property.fMaxHP.GetData());
//	m_pModule_HPAP->SetMaxAP(m_Property.fMaxAP.GetData());
//
//	m_pModule_HPAP->SetHP(m_Property.fMaxHP.GetData());
//	m_pModule_HPAP->SetAP(m_Property.fMaxAP.GetData());
//	// HP, AP ÃÊ±âÈ­
//	//SetHP(m_Property.fMaxHP.GetData());
//	//SetAP(m_Property.fMaxAP.GetData());
//	CHECK_RETURN_CALLSTACK(InitHPAP);
//}

void ZCharacter::InitItemBullet()
{
	int nBulletSpare, nBulletCurrMagazine;
	// ÃÑ¾Ë ÃÊ±âÈ­
	if (!m_Items.GetItem(MMCIP_PRIMARY)->IsEmpty())
	{
		nBulletSpare = m_Items.GetItem(MMCIP_PRIMARY)->GetDesc()->m_nMaxBullet.Ref();
		nBulletCurrMagazine = m_Items.GetItem(MMCIP_PRIMARY)->GetDesc()->m_nMagazine.Ref();

		m_Items.GetItem(MMCIP_PRIMARY)->InitBullet(nBulletSpare, nBulletCurrMagazine);
	}
	if (!m_Items.GetItem(MMCIP_SECONDARY)->IsEmpty())
	{
		nBulletSpare = m_Items.GetItem(MMCIP_SECONDARY)->GetDesc()->m_nMaxBullet.Ref();
		nBulletCurrMagazine = m_Items.GetItem(MMCIP_SECONDARY)->GetDesc()->m_nMagazine.Ref();

		m_Items.GetItem(MMCIP_SECONDARY)->InitBullet(nBulletSpare, nBulletCurrMagazine);
	}

	for (int i = MMCIP_CUSTOM1; i < MMCIP_CUSTOM3 + 3; i++)
	{

		if (!m_Items.GetItem(MMatchCharItemParts(i))->IsEmpty())
		{
			MMatchItemDesc* pDesc = m_Items.GetItem(MMatchCharItemParts(i))->GetDesc();

			// ÀÏ¹Ý Ä¿½ºÅÒ ¾ÆÀÌÅÛ(¼ö·ùÅº È¸º¹Å¶ µE
			if (!pDesc->IsSpendableItem())
			{
				// Ä¿½ºÅÒ ¹«±â´Â ÀçÀåÀE°³³äÀÌ ¾øÀ¸¹Ç·Î ¿©ºÐÀÇ Åº¾Ë¼ö¸¦ ¼¼ÆÃÇÑ´Ù±âº¸´Ù´Â,
				// È­¸é¿¡ "ÇöÀçÀåÀE®/ÃÖ´EåÀE®"°¡ Ç¥½ÃµÇµµ·Ï °ªÀ» ÁÙ »ÓÀÌ´Ù.
				nBulletSpare = pDesc->m_nMaxBullet.Ref();
				nBulletCurrMagazine = pDesc->m_nMagazine.Ref();

				m_Items.GetItem(MMatchCharItemParts(i))->InitBullet(nBulletSpare, nBulletCurrMagazine);
			}
			// ¼Ò¸ð¼º ¾ÆÀÌÅÛ
			else
			{
				nBulletCurrMagazine = pDesc->m_nMagazine.Ref();

				if (GetUID() != ZGetMyUID())
					nBulletSpare = MAX_SPENDABLE_ITEM_COUNT;	//peerÀÇ ¾ÆÀÌÅÛ¼ÒÁö·®À» ¾Ë ¼E¾ø¾ûØ­..
				else
				{
					nBulletSpare = 0;
					ZMyItemNode* pItemNode = ZGetMyInfo()->GetItemList()->GetEquipedItem(MMatchCharItemParts(i));
					if (pItemNode)
					{
						if (pItemNode->GetItemCount() >= nBulletCurrMagazine)
							nBulletSpare = pItemNode->GetItemCount() - nBulletCurrMagazine;
						else
						{
							nBulletCurrMagazine = pItemNode->GetItemCount();
							nBulletSpare = 0;
						}
					}
					else
					{
						nBulletCurrMagazine = 0;
						nBulletSpare = 0;
					}
				}

				// ¼Ò¸ð¼º ¾ÆÀÌÅÛÀº "ÇöÀçÀåÀE®/ÀÎº¥¿¡³²Àº·®"ÀÌ Ç¥½ÃµÇµµ·Ï °ªÀ» ÁØ´Ù
				m_Items.GetItem(MMatchCharItemParts(i))->InitBullet(nBulletSpare, nBulletCurrMagazine);
			}
		}
	}
}

void ZCharacter::InitStatus()
{
	InitHPAP();
	InitItemBullet();

	//jintriple3 ºñÆ® ÆÐÅ· ¸Þ¸ð¸® ÇÁ·Ï½Ã...
	ZCharaterStatusBitPacking & uStatus =m_dwStatusBitPackingValue.Ref();

	// Custom: Moved to object initialization
	//uStatus.m_bTagger		= false;
	uStatus.m_bCommander	= false;
	uStatus.m_bDie			= false;	// ¾ÆÁEÁ¤º¸°¡ ¾È¿Â Ä³¸¯ÅÍ´Â ÀáÁ¤ÀûÀ¸·Î »EÒ´Ù°E»ý°¢ÇÏ±â·ÎÇÔ

	// Custom: Snipers
	uStatus.m_bSniping = false;
	uStatus.m_bFrozen = false;

	uStatus.m_bStylishShoted	= false;
	uStatus.m_bStun				= false;

	uStatus.m_bBlast		= false;
	uStatus.m_bBlastFall	= false;
	uStatus.m_bBlastDrop	= false;
	uStatus.m_bBlastStand	= false;
	uStatus.m_bBlastAirmove = false;

	uStatus.m_bSpMotion			= false;
	uStatus.m_bLostConEffect	= false;
	uStatus.m_bChatEffect		= false;

#ifdef _ICONCHAT
	uStatus.m_bChatVoice		= false;
#endif

	uStatus.m_bFallingToNarak	= false;
	
#ifdef _SPEC 1
	if(IsAdminHide() || GetTeamID() == MMT_SPECTATOR) 
	{
		uStatus.m_bDie = true;
		SetHP(0);
		SetAP(0);
		SetVisible(false);
	}
#else
	if (IsAdminHide())
	{
		uStatus.m_bDie = true;
		SetVisible(false);
	}
#endif
	SetVelocity(0,0,0);
//	m_fLastUpdateTime=g_pGame->GetTime();

	m_Collision.SetCollideable(true);
//	m_bAutoDir = false;
	m_SpMotion = ZC_STATE_TAUNT;

//	m_bClimb = false;

	m_fLastReceivedTime=0;
	/*
	m_fAveragePingTime=0;
	m_fAccumulatedTimeError=0;
	m_nTimeErrorCount=0;
	*/

	m_killInfo.CheckCrc();
	m_killInfo.Ref().m_fLastKillTime = 0;
	m_killInfo.Ref().m_nKillsThisRound = 0;
	m_killInfo.MakeCrc();

	MEMBER_SET_CHECKCRC(m_damageInfo, m_LastDamageType, ZD_NONE);
	SetLastThrower(MUID(0,0), 0.0f);

	EmptyHistory();

	/*
	for(int i=0;i<ZCI_END;i++)
	{
		m_fIconStartTime[i]=-HP_SCALE;
	}
	*/

	// Á×À»¶§ Åõ¸úãØÁ³À¸¹Ç·Î..
	if(m_pVMesh) {
		m_pVMesh->SetVisibility(1);
	}

	m_bCharged->Set_CheckCrc(false);	//mmemory proxy
	m_bCharging->Set_CheckCrc(false);	//mmemory proxy

#ifndef _PUBLISH
	char szLog[128];
	sprintf(szLog, "ZCharacter::InitStatus() - %s(%u) Initialized \n", 
		GetProperty()->GetName(), m_UID.Low);
	OutputDebugString(szLog);
#endif

	InitModuleStatus();
}

//´çÀEº¸¿©ÁÖ±EÀ§ÇÑ°É·Î »ç¿EÑ´Ù..
void ZCharacter::TestChangePartsAll()
{
	if( IsMan() ) {

		OnChangeParts(eq_parts_chest,0);
		OnChangeParts(eq_parts_head	,0);
		OnChangeParts(eq_parts_hands,0);
		OnChangeParts(eq_parts_legs	,0);
		OnChangeParts(eq_parts_feet	,0);
		OnChangeParts(eq_parts_face	,0);

	}
	else {

		OnChangeParts(eq_parts_chest,0);
		OnChangeParts(eq_parts_head	,0);
		OnChangeParts(eq_parts_hands,0);
		OnChangeParts(eq_parts_legs	,0);
		OnChangeParts(eq_parts_feet	,0);
		OnChangeParts(eq_parts_face	,0);
	}
}

#define AddText(s) { str.Add(#s,false); str.Add(" :",false); str.Add(s);}
#define AddTextEnum(s,e) {str.Add(#s,false); str.Add(" :",false); str.Add(#e);}

void ZCharacter::OutputDebugString_CharacterState()
{
	return;

	RDebugStr str;

	str.Add("//////////////////////////////////////////////////////////////" );

	AddText( m_bInitialized );
	AddText( m_bHero );

	AddText( m_nVMID.Ref() );

	AddText( m_UID.High );
	AddText( m_UID.Low  );
	AddText( m_nTeamID.Ref() );

	str.AddLine();

	str.Add("######  m_Property  #######\n");


	AddText( m_Property.GetName() );
	AddText( m_Property.nSex );//±âÅ¸»ý·«

	str.AddLine();

	str.Add("######  m_Status  #######\n");

	AddText( GetHP() );
	AddText( GetAP() );
	AddText( m_Status.Ref().nLife );
//	AddText( m_Status.nScore );
	AddText( m_Status.Ref().nKills );
	AddText( m_Status.Ref().nDeaths );
	AddText( m_Status.Ref().nLoadingPercent );
	AddText( m_Status.Ref().nCombo );
	AddText( m_Status.Ref().nMaxCombo );
	AddText( m_Status.Ref().nAllKill );
	AddText( m_Status.Ref().nExcellent );
	AddText( m_Status.Ref().nFantastic );
	AddText( m_Status.Ref().nHeadShot );
	AddText( m_Status.Ref().nUnbelievable );

#ifdef _ICONCHAT
	AddText( m_Status.Ref().isTalking);
#endif

	str.AddLine();

	str.Add("######  m_Items  #######\n");


	ZItem* pItem = m_Items.GetSelectedWeapon();

	// ¼±ÅÃµÈ ¹«±E
#define IF_SITEM_ENUM(a)		if(a==m_Items.GetSelectedWeaponType())		{ AddTextEnum(m_Items.GetSelectedWeaponType(),a); }
#define ELSE_IF_SITEM_ENUM(a)	else if(a==m_Items.GetSelectedWeaponType())	{ AddTextEnum(m_Items.GetSelectedWeaponType(),a); }

	IF_SITEM_ENUM(MMCIP_HEAD)
	ELSE_IF_SITEM_ENUM(MMCIP_CHEST)
	ELSE_IF_SITEM_ENUM(MMCIP_HANDS)
	ELSE_IF_SITEM_ENUM(MMCIP_LEGS)
	ELSE_IF_SITEM_ENUM(MMCIP_FEET)
	ELSE_IF_SITEM_ENUM(MMCIP_FINGERL)
	ELSE_IF_SITEM_ENUM(MMCIP_FINGERR)
	ELSE_IF_SITEM_ENUM(MMCIP_AVATAR)
	ELSE_IF_SITEM_ENUM(MMCIP_MELEE)
	ELSE_IF_SITEM_ENUM(MMCIP_PRIMARY)
	ELSE_IF_SITEM_ENUM(MMCIP_SECONDARY)
	ELSE_IF_SITEM_ENUM(MMCIP_CUSTOM1)
	ELSE_IF_SITEM_ENUM(MMCIP_CUSTOM2)
	ELSE_IF_SITEM_ENUM(MMCIP_CUSTOM3)
	ELSE_IF_SITEM_ENUM(MMCIP_COMMUNITY1)
	ELSE_IF_SITEM_ENUM(MMCIP_COMMUNITY2)
	ELSE_IF_SITEM_ENUM(MMCIP_LONGBUFF1)
	ELSE_IF_SITEM_ENUM(MMCIP_LONGBUFF2)

	//jintriple3 ¸Þ¸ð¸® ÇÁ·Ï½Ã...ºñÆ® ÆÐÅ·..
	ZCharaterStatusBitPacking & uStatus = m_dwStatusBitPackingValue.Ref();

	AddText( uStatus.m_bDie );
	AddText( uStatus.m_bStylishShoted );
	AddText( IsVisible() );
	AddText( uStatus.m_bStun );
	AddText( m_damageInfo.Ref().m_nStunType );
	AddText( uStatus.m_bPlayDone );

	AddText( m_killInfo.Ref().m_nKillsThisRound );
	AddText( m_killInfo.Ref().m_fLastKillTime );

	str.AddLine(1);

#define IF_LD_ENUM(a)		if(a==m_damageInfo.Ref().m_LastDamageType)			{ AddTextEnum(m_damageInfo.Ref().m_LastDamageType,a); }
#define ELSE_IF_LD_ENUM(a)	else if(a==m_damageInfo.Ref().m_LastDamageType)		{ AddTextEnum(m_damageInfo.Ref().m_LastDamageType,a); }

	IF_LD_ENUM(ZD_NONE)
	ELSE_IF_LD_ENUM(ZD_BULLET)
	ELSE_IF_LD_ENUM(ZD_MELEE)
	ELSE_IF_LD_ENUM(ZD_FALLING)
	ELSE_IF_LD_ENUM(ZD_EXPLOSION)
	ELSE_IF_LD_ENUM(ZD_BULLET_HEADSHOT)
	ELSE_IF_LD_ENUM(ZD_KATANA_SPLASH)
	ELSE_IF_LD_ENUM(ZD_HEAL)
	ELSE_IF_LD_ENUM(ZD_REPAIR)
/*
		 if(m_LastDamageType==ZD_NONE)			{	AddTextEnum(m_LastDamageType,ZD_NONE);}
	else if(m_LastDamageType==ZD_BULLET)		{	AddTextEnum(m_LastDamageType,ZD_BULLET);}
*/
	AddText( m_damageInfo.Ref().m_LastDamageDir );
	AddText( GetSpawnTime() );

	AddText( m_fLastValidTime );
	AddText( GetDistToFloor() );
//	AddText( m_FloorPlane ); // rplane
	AddText( uStatus.m_bLand );
	AddText( uStatus.m_bWallJump );
	AddText( m_nWallJumpDir.Ref() );
	AddText( uStatus.m_bJumpUp );
	AddText( uStatus.m_bJumpDown );
	AddText( uStatus.m_bWallJump2 );
	AddText( uStatus.m_bBlast );
	AddText( uStatus.m_bBlastFall );
	AddText( uStatus.m_bBlastDrop );
	AddText( uStatus.m_bBlastStand );
	AddText( uStatus.m_bBlastAirmove );
	AddText( uStatus.m_bSpMotion );
//	AddText( m_SpMotion );

	AddText( m_bDynamicLight );
	AddText( m_iDLightType );
	AddText( m_fLightLife );
	AddText( m_vLightColor );
	AddText( m_fTime );
	AddText( m_bLeftShot );
//	AddText( m_bClimb );
// 	AddText( m_ClimbDir );
//	AddText( m_fClimbZ );
//	AddText( m_ClimbPlane );// rplane
//	AddText( m_pshadow );
//	AddText( m_pCloth );
//	AddText( m_BasicHistory );
//	AddText( m_HPHistory );

	AddText( m_TargetDir );
	AddText( GetPosition() );
	AddText( m_Direction );
	AddText( m_DirectionLower );
	AddText( m_DirectionUpper );
	AddText( m_RealPositionBefore.Ref() );
//	AddText( GetVelocity() );
	AddText( m_Accel.Ref() );

	str.AddLine(1);

	// Ç®¾ûØ­ Ãâ·Â~

	// »óÃ¼ ¿¡´Ï¸ÞÀÌ¼Ç »óÅÂ

#define IF_Upper_ENUM(a)		if(a==m_AniState_Upper.Ref())		{ AddTextEnum(m_AniState_Upper.Ref(),a); }
#define ELSE_IF_Upper_ENUM(a)	else if(a==m_AniState_Upper.Ref())	{ AddTextEnum(m_AniState_Upper.Ref(),a); }

#define IF_Lower_ENUM(a)		if(a==m_AniState_Lower.Ref())		{ AddTextEnum(m_AniState_Lower.Ref(),a); }
#define ELSE_IF_Lower_ENUM(a)	else if(a==m_AniState_Lower.Ref())	{ AddTextEnum(m_AniState_Lower.Ref(),a); }

		 IF_Upper_ENUM(ZC_STATE_UPPER_NONE)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_SHOT)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_RELOAD)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_LOAD)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_START)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_IDLE)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_BLOCK1)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_BLOCK1_RET)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_BLOCK2)
	ELSE_IF_Upper_ENUM(ZC_STATE_UPPER_GUARD_CANCEL)

/*
	 	 if(m_AniState_Upper==ZC_STATE_UPPER_NONE)				{ AddTextEnum(m_AniState_Upper,ZC_STATE_UPPER_NONE); }
	else if(m_AniState_Upper==ZC_STATE_UPPER_SHOT)				{ AddTextEnum(m_AniState_Upper,ZC_STATE_UPPER_SHOT); }
*/
	// ÇÏÃ¼ ¿¡´Ï¸ÞÀÌ¼Ç »óÅÂ

		 IF_Lower_ENUM(ZC_STATE_LOWER_NONE)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_IDLE1)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_IDLE2)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_IDLE3)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_IDLE4)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_FORWARD)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_BACK)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_LEFT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_RIGHT)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_UP)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_DOWN)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_DIE1)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_DIE2)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_DIE3)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_DIE4)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_LEFT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_LEFT_DOWN)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_DOWN)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_RIGHT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_RUN_WALL_RIGHT_DOWN)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_TUMBLE_FORWARD)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_TUMBLE_BACK)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_TUMBLE_RIGHT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_TUMBLE_LEFT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BIND)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_WALL_FORWARD)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_WALL_BACK)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_WALL_LEFT)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMP_WALL_RIGHT)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK1)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK1_RET)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK2)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK2_RET)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK3)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK3_RET)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK4)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK4_RET)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_ATTACK5)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_JUMPATTACK)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_UPPERCUT)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_START)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_IDLE)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_BLOCK1)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_BLOCK1_RET)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_BLOCK2)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_GUARD_CANCEL)

	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST_FALL)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST_DROP)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST_STAND)
	ELSE_IF_Lower_ENUM(ZC_STATE_LOWER_BLAST_AIRMOVE)

	ELSE_IF_Lower_ENUM(ZC_STATE_DAMAGE)
	ELSE_IF_Lower_ENUM(ZC_STATE_DAMAGE2)
	ELSE_IF_Lower_ENUM(ZC_STATE_DAMAGE_DOWN)

	ELSE_IF_Lower_ENUM(ZC_STATE_TAUNT)
	ELSE_IF_Lower_ENUM(ZC_STATE_BOW)
	ELSE_IF_Lower_ENUM(ZC_STATE_WAVE)
	ELSE_IF_Lower_ENUM(ZC_STATE_LAUGH)
	ELSE_IF_Lower_ENUM(ZC_STATE_CRY)
	ELSE_IF_Lower_ENUM(ZC_STATE_DANCE)

/*
	 	 if(m_AniState_Lower==ZC_STATE_LOWER_NONE)				{ AddTextEnum(m_AniState_Lower,ZC_STATE_LOWER_NONE); }
	else if(m_AniState_Lower==ZC_STATE_LOWER_IDLE1)				{ AddTextEnum(m_AniState_Lower,ZC_STATE_LOWER_IDLE1); }
*/

	// À§ÀÇ »óÅÂ¿Í ´Ù¸¦ ¼EÀÖ´ÂÁEÁ¶»ç~

	if(m_pAnimationInfo_Upper) {
		AddText(m_pAnimationInfo_Upper->Name);
	}

	if(m_pAnimationInfo_Lower) {
		AddText(m_pAnimationInfo_Lower->Name);
	}

	str.AddLine(1);

		 if(m_nMoveMode.Ref()==MCMM_WALK)	{ AddTextEnum(m_nMoveMode.Ref(),MCMM_WALK); }
	else if(m_nMoveMode.Ref()==MCMM_RUN)	{ AddTextEnum(m_nMoveMode.Ref(),MCMM_RUN);  }

		 if(m_nMode.Ref()==MCM_PEACE)		{ AddTextEnum(m_nMode.Ref(),MCM_PEACE);	  }
	else if(m_nMode.Ref()==MCM_OFFENSIVE)	{ AddTextEnum(m_nMode.Ref(),MCM_OFFENSIVE); }

		 if(m_nState.Ref()==MCS_STAND)		{ AddTextEnum(m_nState.Ref(),MCS_STAND);	}
	else if(m_nState.Ref()==MCS_SIT)		{ AddTextEnum(m_nState.Ref(),MCS_SIT);	}
	else if(m_nState.Ref()==MCS_DEAD)		{ AddTextEnum(m_nState.Ref(),MCS_DEAD);	}

	str.AddLine(1);

//	AddText( m_bAdjusted );
//	AddText( m_fLastAdjustedTime );

	/*
	AddText( m_bAutoDir );
	AddText( m_bLeftMoving );
	AddText( m_bRightMoving );

	AddText( m_bForwardMoving );
	*/
	AddText( uStatus.m_bBackMoving );
	AddText( m_fLastReceivedTime );
//	AddText( m_fLastUpdateTime );

//	AddText( m_fAveragePingTime );
//	AddText( m_nTimeErrorCount );
//	AddText( m_TimeErrors[20] );
//	AddText( m_fAccumulatedTimeError );
	AddText( m_fGlobalHP );
	AddText( m_nReceiveHPCount );

//	AddText( m_dwBackUpTime );
	AddText( m_slotInfo.Ref().m_nSelectSlot );

//	AddText( m_Slot[ZC_SLOT_END] );

	str.PrintLog();

//	m_pVMesh ÀÇ »óÅÂµµ Ãâ·Â~

	if(m_pVMesh) {
		m_pVMesh->OutputDebugString_CharacterState();
	}
}

void ZCharacter::TestToggleCharacter()
{
	if(m_pVMesh->GetMesh()) {

		RMesh* pMesh = NULL;

		if( strcmp(m_pVMesh->GetMesh()->GetName(),"heroman1")==0 ) {
			pMesh = ZGetMeshMgr()->Get("herowoman1");//¿øÇÏ´Â ¸ðµ¨À» ºÙ¿©ÁÖ±E.
			m_pVMesh->SetMesh(pMesh);
			m_pVMesh->ClearParts();//³²³à°¡ Æ²·Á¼­.
			TestChangePartsAll();
		}
		else {
			pMesh = ZGetMeshMgr()->Get("heroman1");//¿øÇÏ´Â ¸ðµ¨À» ºÙ¿©ÁÖ±E.
			m_pVMesh->SetMesh(pMesh);
			m_pVMesh->ClearParts();
			TestChangePartsAll();
		}
	}
}

void ZCharacter::InfectCharacter(bool bFirst)
{
#define ZOMBIE_MELEE_ITEMID			6000000
#define ZOMBIE_AVATAR_M_ITEMID		6000001
#define ZOMBIE_AVATAR_F_ITEMID		6000002
#define ZOMBIE_HP					500.f
#define ZOMBIE_MINOR_HP				250.f
#define ZOMBIE_AP					0.f

	if (m_bInfected)
		return;

	if (!m_pVMesh->GetMesh())
		return;

	m_bInfected = true;

	// lol scales dont work
	//m_pVMesh->SetScale(D3DXVECTOR3(m_pVMesh->GetScale().x*1.5f, m_pVMesh->GetScale().y*1.5f, m_pVMesh->GetScale().z*1.5f));

	unsigned long nEquipedItemDesc[MMCIP_END], nEquipedItemCount[MMCIP_END];
	memset(nEquipedItemDesc, 0, sizeof( nEquipedItemDesc ));
	memset(nEquipedItemCount, 0, sizeof( nEquipedItemCount ));

	nEquipedItemDesc[MMCIP_AVATAR] = (m_Property.nSex == MMS_MALE) ? ZOMBIE_AVATAR_M_ITEMID : ZOMBIE_AVATAR_F_ITEMID;
	nEquipedItemDesc[MMCIP_MELEE] = ZOMBIE_MELEE_ITEMID;

	nEquipedItemCount[MMCIP_AVATAR] = 1;
	nEquipedItemCount[MMCIP_MELEE] = 1;

	ZChangeCharParts(m_pVMesh, m_Property.nSex, m_Property.nHair, m_Property.nFace, nEquipedItemDesc);

	for (int i = 0; i < MMCIP_END; i++)
	{
		m_Items.EquipItem(MMatchCharItemParts(i), nEquipedItemDesc[i], nEquipedItemCount[i]);
	}

	// change the weapon
	ChangeWeapon(MMCIP_MELEE);

	MMatchItemDesc* pSelectedItemDesc = m_Items.GetSelectedWeapon()->GetDesc();

	if (pSelectedItemDesc==NULL)
	{
		m_Items.SelectWeapon(MMCIP_MELEE);
	}
	else
		OnChangeWeapon(pSelectedItemDesc->m_pMItemName->Ref().m_szMeshName);

	SetMaxHP(bFirst ? ZOMBIE_HP : ZOMBIE_MINOR_HP);
	SetHP(bFirst ? ZOMBIE_HP : ZOMBIE_MINOR_HP);
	m_Property.fMaxHP.Set(bFirst ? ZOMBIE_HP : ZOMBIE_MINOR_HP);
	m_Property.fMaxHP.MakeCrc();
	SetMaxAP(ZOMBIE_AP);
	SetAP(ZOMBIE_AP);
	m_Property.fMaxAP.Set(ZOMBIE_AP);
	m_Property.fMaxAP.MakeCrc();
}

void ZCharacter::InitMesh()
{
	//char szMeshName[64];
	//if (m_Property.nSex == MMS_MALE)
	//{
	//	strcpy(szMeshName, "heroman1");
	//}
	//else
	//{
	//	strcpy(szMeshName, "herowoman1");
	//}
	//_ASSERT(m_pVMesh == NULL);
	//m_pVMesh = new RVisualMesh;
	//int nVMID = m_pVMesh->InitCharacterMesh(this, szMeshName);

	//m_nVMID.Set_CheckCrc(nVMID);

	RMesh* pMesh;

	char szMeshName[64];
	if (m_Property.nSex == MMS_MALE)
	{
		strcpy(szMeshName, "heroman1");
	}
	else
	{
		strcpy(szMeshName, "herowoman1");
	}

	pMesh = ZGetMeshMgr()->Get(szMeshName);//¿øÇÏ´Â ¸ðµ¨À» ºÙ¿©ÁÖ±E.

	if (!pMesh) {
		mlog("AddCharacter ¿øÇÏ´Â ¸ðµ¨À» Ã£À»¼E¾øÀ½\n");
	}

	int nVMID = ZGetGame()->m_VisualMeshMgr.Add(pMesh);

	if (nVMID == -1) {
		mlog("AddCharacter Ä³¸¯ÅÍ »ý¼º ½ÇÆÐ\n");
	}

	m_nVMID.Set_CheckCrc(nVMID);

	RVisualMesh* pVMesh = ZGetGame()->m_VisualMeshMgr.GetFast(nVMID);
	SetVisualMesh(pVMesh);

}


void ZCharacter::ChangeLowPolyModel()
{
	if(m_pVMesh==NULL)
		return;

	char szMeshName[64];

	bool cloth_model = false;

	if( m_pVMesh ) {
		cloth_model = m_pVMesh->IsClothModel();
	}

	if ( cloth_model ) {
		strcpy(szMeshName, "heroman_low1");
	}
	else {
		strcpy(szMeshName, "heroman_low2");
	}

	RMesh* pLowMesh = ZGetMeshMgr()->Get(szMeshName);//¿øÇÏ´Â ¸ðµ¨À» ºÙ¿©ÁÖ±E.

	// Custom: No low poly models, ugly as fck. (Model Low Poly)
	m_pVMesh->SetLowPolyModel(pLowMesh);
}

bool ZCharacter::IsAdminName()
{
	if(m_MInitialInfo.Ref().nUGradeID == MMUG_EVENTMASTER ||
		m_MInitialInfo.Ref().nUGradeID == MMUG_DEVELOPER ||
		m_MInitialInfo.Ref().nUGradeID == MMUG_ADMIN ||
		m_MInitialInfo.Ref().nUGradeID == MMUG_EVENTTEAM ||
		m_MInitialInfo.Ref().nUGradeID == MMUG_GAMEMASTER ||
		m_MInitialInfo.Ref().nUGradeID == MMUG_MANAGER)
		return true;
	return false;
}

// Custom: Ghost Mode - Desperate
bool ZCharacter::IsGhost()
{ 
	if (GhostTime + 600 < timeGetTime())
	{
		m_dwStatusBitPackingValue.Ref().m_bGhost = false;
	}
	if (m_dwStatusBitPackingValue.Ref().m_bGhost) return true;
	return false;
}

#ifdef _VIPGRADES
bool ZCharacter::IsVIP1Name()
{
	if (m_MInitialInfo.Ref().nUGradeID == MMUG_VIP1)
		return true;
	return false;
}
bool ZCharacter::IsVIP2Name()
{
	if (m_MInitialInfo.Ref().nUGradeID == MMUG_VIP2)
		return true;
	return false;
}
bool ZCharacter::IsVIP3Name()
{
	if (m_MInitialInfo.Ref().nUGradeID == MMUG_VIP3)
		return true;
	return false;
}
bool ZCharacter::IsVIP4Name()
{
	if (m_MInitialInfo.Ref().nUGradeID == MMUG_VIP4)
		return true;
	return false;
}
bool ZCharacter::IsVIP5Name()
{
	if (m_MInitialInfo.Ref().nUGradeID == MMUG_VIP5)
		return true;
	return false;
}
bool ZCharacter::IsVIP6Name()
{
	if (m_MInitialInfo.Ref().nUGradeID == MMUG_VIP6)
		return true;
	return false;
}
bool ZCharacter::IsVIP7Name()
{
	if (m_MInitialInfo.Ref().nUGradeID == MMUG_VIP7)
		return true;
	return false;
}
#endif

#ifdef _EVENTGRD
bool ZCharacter::Event()
{
	if (m_MInitialInfo.Ref().nUGradeID == MMUG_STAR)
		return true;
	return false;
}
bool ZCharacter::IsStaffName()
{
	if (m_MInitialInfo.Ref().nUGradeID == MMUG_HIDE_ADMIN)
		return true;
	return false;
}
bool ZCharacter::IsEvent1Name()
{
	if (m_MInitialInfo.Ref().nUGradeID == MMUG_EVENT1)
		return true;
	return false;
}
bool ZCharacter::IsEvent2Name()
{
	if (m_MInitialInfo.Ref().nUGradeID == MMUG_EVENT2)
		return true;
	return false;
}
bool ZCharacter::IsEvent3Name()
{
	if (m_MInitialInfo.Ref().nUGradeID == MMUG_EVENT3)
		return true;
	return false;
}
bool ZCharacter::IsEvent4Name()
{
	if (m_MInitialInfo.Ref().nUGradeID == MMUG_EVENT4)
		return true;
	return false;
}
bool ZCharacter::IsEvent5Name()
{
	if (m_MInitialInfo.Ref().nUGradeID == MMUG_BOOSTER)
		return true;
	return false;
}
bool ZCharacter::IsEvent6Name()
{
	if (m_MInitialInfo.Ref().nUGradeID == MMUG_WINTOUR)
		return true;
	return false;
}
#endif

void ZCharacter::InitProperties()
{
	const MTD_CharInfo* pCharInfo = &m_MInitialInfo.Ref();

	m_Property.SetName(pCharInfo->szName);
	m_Property.SetClanName(pCharInfo->szClanName);
	m_Property.nSex		= (MMatchSex)pCharInfo->nSex;
	m_Property.nHair	= pCharInfo->nHair;
	m_Property.nFace	= pCharInfo->nFace;
	m_Property.nLevel	= pCharInfo->nLevel;
	
	float fAddedAP = DEFAULT_CHAR_AP;			///< AP ÀçÁ¶Á¤
	for (int i = 0; i < MMCIP_END; i++) 
	{

		if (m_Items.GetItem(MMatchCharItemParts(i)) && !m_Items.GetItem(MMatchCharItemParts(i))->IsEmpty()) {
			//ÇÙ ¹æ¾ûÛEÀÓ½ÃÄÚµE
			// Custom: Bypass AP limit of 40 (MAIET trap check)
			//if(m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nAP.Ref() > 40) {
			//	m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nAP.Ref() = 0; 
			//}
			fAddedAP += m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nAP.Ref();
		}
	}
	
	float fAddedHP = DEFAULT_CHAR_HP;			///< HP ÀçÁ¶Á¤
	for (int i = 0; i < MMCIP_END; i++) 
	{

		if (m_Items.GetItem(MMatchCharItemParts(i)) && !m_Items.GetItem(MMatchCharItemParts(i))->IsEmpty()) {
			fAddedHP += m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nHP.Ref();
		}
	}

	// Custom: Ghost Mode - Desperate
	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_FORCEHP) || ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_GHOST)
	{ 
		 if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_GHOST && GetTeamID() == MMT_RED) 
		 {
			 m_Property.fMaxHP.Set_CheckCrc(150); 
			 m_fPreMaxHP = 150; 
			 m_Property.fMaxAP.Set_CheckCrc(140); 
			 m_fPreMaxAP = 140;
		 }
		else
		 {
			m_Property.fMaxHP.Set_CheckCrc(120); 
			m_fPreMaxHP = 120; 
			m_Property.fMaxAP.Set_CheckCrc(110); 
			m_fPreMaxAP = 110; 
		 }
	} 

	m_Property.fMaxAP.Set_CheckCrc(pCharInfo->nAP + fAddedAP);
	m_Property.fMaxHP.Set_CheckCrc(pCharInfo->nHP + fAddedHP);


	m_fPreMaxHP = pCharInfo->nHP + fAddedHP;
	m_fPreMaxAP = pCharInfo->nAP + fAddedAP;

	//Avatar Vampire By Desperate
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_VAMPIRE)
	{
		if (GetProperty()->nSex == MMS_FEMALE)
		{
			m_Items.EquipItem(MMCIP_AVATAR, 530510);
		}
		else
		{
			m_Items.EquipItem(MMCIP_AVATAR, 530010);
		}
		m_Items.EquipItem(MMCIP_FINGERL, 525005);
	}
#ifdef _PAINTMODE
	if (ZGetGame()->GetMatch()->IsPaintBall())
	{
		if (GetProperty()->nSex == MMS_FEMALE)
		{
			m_Items.EquipItem(MMCIP_AVATAR, 900659);
		}
		else
		{
			m_Items.EquipItem(MMCIP_AVATAR, 900658);
		}
	}
#endif

#ifdef _PAINTMODE
	if (ZGetGame()->GetMatch()->IsPaintBall())
	{
		m_Property.fMaxHP.Set_CheckCrc(100);
		m_fPreMaxHP = 100;
		m_Property.fMaxAP.Set_CheckCrc(100);
		m_fPreMaxAP = 100;
	}
#endif
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_VAMPIRE)
	{
		m_Property.fMaxHP.Set_CheckCrc(130);
		m_fPreMaxHP = 130;
		m_Property.fMaxAP.Set_CheckCrc(130);
		m_fPreMaxAP = 130;
	}
	if (ZGetGame()->GetMatch()->IsClassiMode())
	{
		m_Property.fMaxHP.Set_CheckCrc(100);
		m_fPreMaxHP = 100;
		m_Property.fMaxAP.Set_CheckCrc(100);
		m_fPreMaxAP = 100;
	}
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SNIPERMODE)
	{
		if (GetProperty()->nSex == MMS_MALE)
		{
			m_Items.EquipItem(MMCIP_AVATAR, 530024);
		}
		else
		{
			m_Items.EquipItem(MMCIP_AVATAR, 530524);
		}
		m_Items.EquipItem(MMCIP_PRIMARY, 10002);
		m_Items.EquipItem(MMCIP_FINGERL, 525004);
		m_Items.EquipItem(MMCIP_FINGERR, 525008);
	}
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_MODE_STAFF)
	{
		m_Property.fMaxHP.Set_CheckCrc(5000);
		m_fPreMaxHP = 5000;
		m_Property.fMaxAP.Set_CheckCrc(5000);
		m_fPreMaxAP = 5000;
	}
	if(GetUserGrade() == MMUG_DEVELOPER) 
	{
		m_pMUserAndClanName->CheckCrc();
		// Custom: Unmask names, use sprintf_s
		strcpy(m_pMUserAndClanName->Ref().m_szUserName,m_Property.GetName());
		if(strlen(m_Property.GetClanName()) != 0)
			sprintf_s(m_pMUserAndClanName->Ref().m_szUserAndClanName,"%s(%s)",m_Property.GetName(),m_Property.GetClanName());
		else
			sprintf_s(m_pMUserAndClanName->Ref().m_szUserAndClanName,"%s",m_Property.GetName());
		//strcpy(m_pMUserAndClanName->Ref().m_szUserName, ZMsg(MSG_WORD_DEVELOPER));
		//strcpy(m_pMUserAndClanName->Ref().m_szUserAndClanName, ZMsg(MSG_WORD_DEVELOPER));
		m_pMUserAndClanName->MakeCrc();
	}
	else if(GetUserGrade() == MMUG_ADMIN) {
		m_pMUserAndClanName->CheckCrc();
		// Custom: Unmask names, use sprintf_s
		strcpy(m_pMUserAndClanName->Ref().m_szUserName,m_Property.GetName());
		if(strlen(m_Property.GetClanName()) != 0)
			sprintf_s(m_pMUserAndClanName->Ref().m_szUserAndClanName,"%s(%s)",m_Property.GetName(),m_Property.GetClanName());
		else
			sprintf_s(m_pMUserAndClanName->Ref().m_szUserAndClanName,"%s",m_Property.GetName());
		//strcpy(m_pMUserAndClanName->Ref().m_szUserName,ZMsg(MSG_WORD_ADMIN));
		//strcpy(m_pMUserAndClanName->Ref().m_szUserAndClanName,ZMsg(MSG_WORD_ADMIN));
		m_pMUserAndClanName->MakeCrc();
	}
	else {
		m_pMUserAndClanName->CheckCrc();
		strcpy(m_pMUserAndClanName->Ref().m_szUserName,m_Property.GetName());
		if(strlen(m_Property.GetClanName()) != 0)
			sprintf_s(m_pMUserAndClanName->Ref().m_szUserAndClanName,"%s(%s)",m_Property.GetName(),m_Property.GetClanName());
		else
			sprintf_s(m_pMUserAndClanName->Ref().m_szUserAndClanName,"%s",m_Property.GetName());
		m_pMUserAndClanName->MakeCrc();
	}

	MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(GetUID());

	//jintriple3 ¸Þ¸ð¸® ÇÁ·Ï½Ã...ºñÆ® ÆÐÅ·..
	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	if (pObjCache && IsAdminGrade(pObjCache->GetUGrade()) && 
		pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide) || pObjCache && pObjCache->GetUGrade() == MMUG_GAMEMASTER &&
		pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide))
		uStatus.m_bAdminHide = true;
	else
		uStatus.m_bAdminHide = false;

	//Custom: ForceHPAP
	//int newhp = 0;
	//int newap = 0;
	//if (sscanf(ZGetGameClient()->GetStageName(), "[HP=%i] [AP=%i]", &newhp, &newap))
	//{
	//	if (newhp > 0)
	//	{
	//		m_Property.fMaxHP.Set_CheckCrc(newhp);
	//		m_fPreMaxHP = newhp;
	//	}
	//	if (newap > 0)
	//	{
	//		m_Property.fMaxAP.Set_CheckCrc(newap);
	//		m_fPreMaxAP = newap;
	//	}
	//	else
	//	{
	//		m_Property.fMaxHP.Set_CheckCrc(pCharInfo->nHP + fAddedHP);
	//		m_fPreMaxHP = pCharInfo->nHP + fAddedHP;
	//		m_Property.fMaxAP.Set_CheckCrc(pCharInfo->nAP + fAddedAP);
	//		m_fPreMaxAP = pCharInfo->nAP + fAddedAP;
	//	}
	//}

// Room Mod: Force HP/AP
	{
		auto* pSetting = ZGetGameClient()->GetMatchStageSetting();
		if (pSetting->IsModifierUsed(MMOD_FORCEHP))
		{
			int nHP = pSetting->GetModifierValue(MMOD_FORCEHP);
			if (nHP > 0)
			{
				m_Property.fMaxHP.Set_CheckCrc((float)nHP);
				m_fPreMaxHP = (float)nHP;
			}
			// nHP == -1 means [hpap] without value -> keep character default
		}
		if (pSetting->IsModifierUsed(MMOD_FORCEAP))
		{
			int nAP = pSetting->GetModifierValue(MMOD_FORCEAP);
			if (nAP > 0)
			{
				m_Property.fMaxAP.Set_CheckCrc((float)nAP);
				m_fPreMaxAP = (float)nAP;
			}
		}
	}
}

bool ZCharacter::Create(MTD_CharInfo* pCharInfo/*, MTD_CharBuffInfo* pCharBuffInfo*/)
{
	_ASSERT(!m_bInitialized);

	//memcpy(&m_MInitialInfo.GetData(),pCharInfo,sizeof(MTD_CharInfo));
	m_MInitialInfo.Set(*pCharInfo);
	m_MInitialInfo.MakeCrc();


	//¹öÇÁÁ¤º¸ÀÓ½ÃÁÖ¼® SetCharacterBuff(pCharBuffInfo);		///< ÄÉ¸¯ÅÍ ¹öÇÁ ¼¼ÆÃ	
	for (int i = 0; i < MMCIP_END; i++) 
	{
		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_VAMPIRE)
		{
			m_Items.EquipItem(MMCIP_PRIMARY, 506007);
			m_Items.EquipItem(MMCIP_SECONDARY, 504005);
			m_Items.EquipItem(MMCIP_MELEE, 502006);
			m_Items.EquipItem(MMCIP_CUSTOM1, 550054);
			m_Items.EquipItem(MMCIP_CUSTOM2, 550053);
		}
#ifdef _PAINTMODE
		if (ZGetGame()->GetMatch()->IsPaintBall())
		{
			m_Items.EquipItem(MMCIP_MELEE, 900660);
			m_Items.EquipItem(MMCIP_PRIMARY, 900656);
			m_Items.EquipItem(MMCIP_SECONDARY, 900657);
		}
#endif
		if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_SNIPERMODE)
		{
			m_Items.EquipItem(MMCIP_PRIMARY, 10002);
			m_Items.EquipItem(MMCIP_FINGERL, 525004);
			m_Items.EquipItem(MMCIP_FINGERR, 525008);
		}
		else if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_MODE_STAFF)
		{
			if (GetProperty()->nSex == MMS_MALE)
			{
				m_Items.EquipItem(MMCIP_CHEST, 399999);
			}
			else
			{
				m_Items.EquipItem(MMCIP_CHEST, 399998);
			}
			m_Items.EquipItem(MMCIP_PRIMARY, 600334);
			m_Items.EquipItem(MMCIP_SECONDARY, 600337);
			m_Items.EquipItem(MMCIP_MELEE, 600329);
		}

		m_Items.EquipItem(MMatchCharItemParts(i), pCharInfo->nEquipedItemDesc[i], pCharInfo->nEquipedItemCount[i]);
	}

	InitProperties();
	InitMesh();

	m_bInitialized = true;
	m_bInitialized_DebugRegister = true;

	SetAnimationLower(ZC_STATE_LOWER_IDLE1);
	SetAnimationUpper(ZC_STATE_UPPER_NONE);

	InitMeshParts();

	CreateShadow();
	
	m_pSoundMaterial[0] = 0;

	if(Enable_Cloth) {
		m_pVMesh->ChangeChestCloth(1.f,1);
	}

	ChangeLowPolyModel();

	m_dwStatusBitPackingValue.Ref().m_bIsLowModel = false;
	SetVisible(true);

	m_fAttack1Ratio.Set_CheckCrc(1.f);

	ZGetEmblemInterface()->AddClanInfo(GetClanID());
#ifdef _SWORDCOLOR 1
	//Custom: Effect Massive By Desperate
	//MMatchItemDesc* pDesc = m_Items.GetDesc(MMCIP_MELEE);
	//if (pDesc)
	//{
	//	switch (pDesc->m_nID)
	//	{
	//	case 515000:
	//		GetRGMain().SetSwordColor(GetUID(), 0xFF81AAB2);
	//		break;
	//	case 515001:
	//		GetRGMain().SetSwordColor(GetUID(), 0xFFFF4D00);
	//		break;
	//	case 515002:
	//		GetRGMain().SetSwordColor(GetUID(), 0xFF00A2FF);
	//		break;
	//	case 515003:
	//		GetRGMain().SetSwordColor(GetUID(), 0xFF00BA44);
	//		break;
	//	default:
	//		GetRGMain().SetSwordColor(GetUID(), 0xFFFFFFFF);
	//		break;
	//	}
	//}
#endif

#ifdef _TRAILCOLOR
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bTrailCheck)
	{
		GetRGMain().SetSwordColor(ZGetGame()->m_pMyCharacter->GetUID(), ZGetConfiguration()->GetEtc()->bTrailColors);
	}
	else
	{
		switch (Z_ETC_TRAILBOXCOLOR)
		{
		case 0:
			GetRGMain().SetSwordColor(GetUID(), 0xFFFFFFFF); // NORMAL 460859
			break;
		case 1:
			GetRGMain().SetSwordColor(GetUID(), 0xFFFF0000); // RED
			break;
		case 2:
			GetRGMain().SetSwordColor(GetUID(), 0xFF0377FC); // BLUE
			break;
		case 3:
			GetRGMain().SetSwordColor(GetUID(), 0xFF2FFF00); // GREEN
			break;
		case 4:
			GetRGMain().SetSwordColor(GetUID(), 0xFF00F5F5); // CYAN
			break;
		case 5:
			GetRGMain().SetSwordColor(GetUID(), 0xFFF5CC00); // YELLOW
			break;
		case 6:
			GetRGMain().SetSwordColor(GetUID(), 0xFF9F00F5); // PURPLE
			break;
		case 7:
			GetRGMain().SetSwordColor(GetUID(), 0xFFFF6600); // ORANGE
			break;
		case 8:
			GetRGMain().SetSwordColor(GetUID(), 0xFFC26EA3); // PINK
			break;
		default:
			GetRGMain().SetSwordColor(GetUID(), 0xFFFFFFFF); // NORMAL
			break;
		}
	}
#endif
	return true;
}
void ZCharacter::Destroy()
{
	if(m_bInitialized) {
		ZGetEmblemInterface()->DeleteClanInfo(GetClanID());
	}

	m_bInitialized = false;
	m_bInitialized_DebugRegister = false;

	DestroyShadow();
}

void ZCharacter::InitMeshParts()
{
	RMeshPartsType mesh_parts_type;

	if (m_pVMesh)
	{
		ZItem* pAvatarItem = GetItems()->GetItem(MMCIP_AVATAR);
		if (pAvatarItem == NULL) {
			_ASSERT(0);
			return;
		}

		if (pAvatarItem && pAvatarItem->IsEmpty()) {
			for (int i = 0; i < MMCIP_END; i++) {
				switch (MMatchCharItemParts(i))
				{
				case MMCIP_HEAD:	mesh_parts_type = eq_parts_head;	break;
				case MMCIP_CHEST:	mesh_parts_type = eq_parts_chest;	break;
				case MMCIP_HANDS:	mesh_parts_type = eq_parts_hands;	break;
				case MMCIP_LEGS:	mesh_parts_type = eq_parts_legs;	break;
				case MMCIP_FEET:	mesh_parts_type = eq_parts_feet;	break;
				default: continue;
				}
#ifdef _DYNAMIC
				if (!GetItems()->GetItem(MMatchCharItemParts(i))->IsEmpty())
				{
					MMatchItemDesc* itemDesc = GetItems()->GetItem(MMatchCharItemParts(i))->GetDesc();
					if (itemDesc->GetDyn() != nullptr)
					{
						RMesh* playerMesh = nullptr;
						if (_stricmp(m_pVMesh->GetMesh()->GetName(), "heroman1") == 0)
						{
							playerMesh = ZGetMeshMgr()->Get("heroman1");
						}
						else
						{
							playerMesh = ZGetMeshMgr()->Get("herowoman1");
						}
						if (playerMesh->m_parts_mgr->Find(itemDesc->m_szDyn) == false)
						{
							std::string szInputPath = itemDesc->m_szDyn; // Chuỗi từ XML
							std::string fullPath = "";

							// 1. Kiểm tra xem trong XML đã có tiền tố "model/" chưa
							if (szInputPath.find("model/") != std::string::npos || szInputPath.find("model\\") != std::string::npos)
							{
								// Nếu có rồi thì dùng luôn, không ghép thêm gì nữa
								fullPath = szInputPath;
							}
							else
							{
								// 2. Nếu chưa có "model/", mới bắt đầu ghép theo giới tính (Logic cũ)
								bool bIsWoman = false;
								if (playerMesh->GetFileName())
								{
									std::string meshName = playerMesh->GetFileName();
									if (meshName.find("woman") != std::string::npos)
									{
										bIsWoman = true;
									}
								}

								if (bIsWoman)
									fullPath = "model/woman/" + szInputPath;
								else
									fullPath = "model/man/" + szInputPath;
							}

							// 3. CHUẨN HÓA DẤU XUYỆT (Cực kỳ quan trọng để Engine không bị ngáo khi nạp thư mục sâu)
							// Chuyển hết \ thành /
							for (auto& c : fullPath) { if (c == '\\') c = '/'; }

							// 4. Nạp vào Parts Manager
							playerMesh->m_parts_mgr->Add((char*)fullPath.c_str());

							// Cập nhật trạng thái
							MeshesFinishedLoading = false;
							meshupdatetime = 0;

							// Nên bật dòng này để check mlog xem nó nạp đường dẫn gì nhé
							// mlog("ZCharacter::AddParts: Final Path = %s\n", fullPath.c_str());
						}
					}
					m_pVMesh->SetParts(mesh_parts_type, itemDesc->m_pMItemName->Ref().m_szMeshName, itemDesc->m_szDyn);
				}
				else {
					m_pVMesh->SetBaseParts(mesh_parts_type);
				}
			}
#endif
			// ¸Ó¸® ¾ÆÀÌÅÛÀÌ ¾øÀ¸¸é ¸Ó¸®Ä«¶ôÀÌ ÀÔÇôÁø´Ù. Áö±ÝÀº ¼­·Î ¸Þ½¬¸¦ °øÀ¯ÇÔ
			if (GetItems()->GetItem(MMCIP_HEAD)->IsEmpty()) {
				ChangeCharHair(m_pVMesh, m_Property.nSex, m_Property.nHair);
			}

			m_pVMesh->SetSkipRenderFaceParts(false);

			ChangeCharFace(m_pVMesh, m_Property.nSex, m_Property.nFace);
		}
		else {
			char* szMeshName;
			MMatchItemDesc* pDesc = pAvatarItem->GetDesc();
			if (pDesc != NULL) {
				m_pVMesh->ClearParts();
#ifdef _DYNAMIC
				szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szHeadMeshName;
				if (strlen(szMeshName) > 0)	m_pVMesh->SetParts(eq_parts_head, szMeshName, pDesc->GetDyn());
				else							ChangeCharHair(m_pVMesh, m_Property.nSex, m_Property.nHair);

				szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szChestMeshName;
				if (strlen(szMeshName) > 0)	m_pVMesh->SetParts(eq_parts_chest, szMeshName, pDesc->GetDyn());
				else							m_pVMesh->SetBaseParts(eq_parts_chest);

				szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szHandMeshName;
				if (strlen(szMeshName) > 0)	m_pVMesh->SetParts(eq_parts_hands, szMeshName, pDesc->GetDyn());
				else							m_pVMesh->SetBaseParts(eq_parts_hands);

				szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szLegsMeshName;
				if (strlen(szMeshName) > 0)	m_pVMesh->SetParts(eq_parts_legs, szMeshName, pDesc->GetDyn());
				else							m_pVMesh->SetBaseParts(eq_parts_legs);

				szMeshName = pDesc->m_pAvatarMeshName->Ref().m_szFeetMeshName;
				if (strlen(szMeshName) > 0)	m_pVMesh->SetParts(eq_parts_feet, szMeshName, pDesc->GetDyn());
				else							m_pVMesh->SetBaseParts(eq_parts_feet);
#endif
				m_pVMesh->SetSkipRenderFaceParts(true);
			}
		}
	}


	SetAnimationUpper(ZC_STATE_UPPER_NONE);
	SetAnimationLower(ZC_STATE_LOWER_IDLE1);

	// Ã³À½¿¡ Áã´Â ¾ÆÀÌÅÛ
	// Custom: FPS mod
	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_FPS))
	{
		if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_INFECTED)
		{
			if (m_bInfected)
			{
				if (!m_Items.GetItem(MMCIP_MELEE)->IsEmpty())			ChangeWeapon(MMCIP_MELEE);
			}
			else
			{
				// Change to primary/secondary
				if (!m_Items.GetItem(MMCIP_PRIMARY)->IsEmpty())			ChangeWeapon(MMCIP_PRIMARY);
				else if (!m_Items.GetItem(MMCIP_SECONDARY)->IsEmpty())	ChangeWeapon(MMCIP_SECONDARY);
			}
		}
		else
		{
			// Change to primary/secondary
			if (!m_Items.GetItem(MMCIP_PRIMARY)->IsEmpty())			ChangeWeapon(MMCIP_PRIMARY);
			else if (!m_Items.GetItem(MMCIP_SECONDARY)->IsEmpty())	ChangeWeapon(MMCIP_SECONDARY);
		}
	}
	else if (!ZGetGame()->GetMatch()->IsRuleGladiator())
	{
		if (!m_Items.GetItem(MMCIP_PRIMARY)->IsEmpty())			ChangeWeapon(MMCIP_PRIMARY);
		else if (!m_Items.GetItem(MMCIP_SECONDARY)->IsEmpty())	ChangeWeapon(MMCIP_SECONDARY);
		else if (!m_Items.GetItem(MMCIP_MELEE)->IsEmpty())		ChangeWeapon(MMCIP_MELEE);
		else if (!m_Items.GetItem(MMCIP_CUSTOM1)->IsEmpty())	ChangeWeapon(MMCIP_CUSTOM1);
		else if (!m_Items.GetItem(MMCIP_CUSTOM2)->IsEmpty())	ChangeWeapon(MMCIP_CUSTOM2);
		else if (!m_Items.GetItem(MMCIP_CUSTOM3)->IsEmpty())	ChangeWeapon(MMCIP_CUSTOM3);

		else ChangeWeapon(MMCIP_PRIMARY);
	}
	else
	{
		if (!m_Items.GetItem(MMCIP_MELEE)->IsEmpty()) ChangeWeapon(MMCIP_MELEE);
		else if (!m_Items.GetItem(MMCIP_CUSTOM1)->IsEmpty()) ChangeWeapon(MMCIP_CUSTOM1);
		else if (!m_Items.GetItem(MMCIP_CUSTOM2)->IsEmpty()) ChangeWeapon(MMCIP_CUSTOM2);
		else if (!m_Items.GetItem(MMCIP_CUSTOM3)->IsEmpty()) ChangeWeapon(MMCIP_CUSTOM3);

		else ChangeWeapon(MMCIP_PRIMARY);

	}
}

void ZCharacter::ChangeWeapon(MMatchCharItemParts nParts, bool bReSelect)
{
	if (!bReSelect) {
		if (m_Items.GetSelectedWeaponParts() == nParts)
			return;
	}

	// Custom: MMCIP_END bugfix
	if( nParts < 0 || nParts >= MMCIP_END )
	{
		return;
	}
	if (m_Items.GetItem(nParts) == NULL) return;
	if (m_Items.GetItem(nParts)->GetDesc() == NULL) return;

	if (ZGetGame()->GetMatch()->IsRuleGladiator())
	{
		if ((nParts == MMCIP_PRIMARY) || (nParts == MMCIP_SECONDARY)) {
			return;
		}
	}
	//Custom: No change weapon on gadget snifer
	if (ZGetGame()->m_pMyCharacter->m_statusFlags.Ref().m_bSniferMode)
	{
		if ((nParts == MMCIP_PRIMARY) || (nParts == MMCIP_SECONDARY || (nParts == MMCIP_MELEE) || nParts == MMCIP_CUSTOM1 || nParts == MMCIP_CUSTOM2 || nParts == MMCIP_CUSTOM3)) 
		{
			return;
		}
	}
	// Custom: FPS mod
	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_FPS))
	{
		if( (!m_bInfected && nParts == MMCIP_MELEE) || nParts == MMCIP_CUSTOM1 || nParts == MMCIP_CUSTOM2 || nParts == MMCIP_CUSTOM3)
			return;
	}

	MMatchCharItemParts BackupParts = m_Items.GetSelectedWeaponParts();

	m_Items.SelectWeapon(nParts);

	if(m_Items.GetSelectedWeapon()==NULL) return;

	MMatchItemDesc* pSelectedItemDesc = m_Items.GetSelectedWeapon()->GetDesc();

	if (pSelectedItemDesc==NULL) {
		m_Items.SelectWeapon(BackupParts);
		return;
	}

#ifdef _BULLETBAR
	if (nParts == MMCIP_PRIMARY)
	{
		m_Status.Ref().nMaxCurrBullet = m_Items.GetItem(MMCIP_PRIMARY)->GetDesc()->m_nMagazine.Ref();
	}
	if (nParts == MMCIP_SECONDARY)
	{
		m_Status.Ref().nMaxCurrBullet = m_Items.GetItem(MMCIP_SECONDARY)->GetDesc()->m_nMagazine.Ref();
	}
	if (nParts == MMCIP_CUSTOM1)
	{
		m_Status.Ref().nMaxCurrBullet = m_Items.GetItem(MMCIP_CUSTOM1)->GetDesc()->m_nMagazine.Ref();
	}
	if (nParts == MMCIP_CUSTOM2)
	{
		m_Status.Ref().nMaxCurrBullet = m_Items.GetItem(MMCIP_CUSTOM2)->GetDesc()->m_nMagazine.Ref();
	}
	if (nParts == MMCIP_CUSTOM3)
	{
		m_Status.Ref().nMaxCurrBullet = m_Items.GetItem(MMCIP_CUSTOM3)->GetDesc()->m_nMagazine.Ref();
	}
#endif

	OnChangeWeapon(pSelectedItemDesc->m_pMItemName->Ref().m_szMeshName);

	if(nParts!=MMCIP_MELEE)
		m_bCharged->Set_CheckCrc(false);

	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();
	if (uStatus.m_bChatEffect) uStatus.m_bChatEffect = false;

#ifdef _ICONCHAT
	if (uStatus.m_bChatVoice) uStatus.m_bChatVoice = false; 
	if (m_Status.Ref().isTalking = true) m_Status.Ref().isTalking = false; 
#endif

}

//#define _CHECKVALIDSHOTLOG

bool ZCharacter::CheckValidShotTime(int nItemID, float fTime, ZItem* pItem)
{
#ifdef _CHECKVALIDSHOTLOG
	char szTime[32]; _strtime(szTime);
	char szLog[256];
#endif

	if (GetLastShotItemID() == nItemID)
	{
		if (fTime - GetLastShotTime() + 0.005f < (float)pItem->GetDesc()->m_nDelay.Ref() / 1000.0f)
		{

			MMatchWeaponType nWeaponType = pItem->GetDesc()->m_nWeaponType.Ref();
			if ((MWT_DAGGER <= nWeaponType && nWeaponType <= MWT_DOUBLE_KATANA || nWeaponType == MWT_SPYCASE)
				&& (fTime - GetLastShotTime() >= 0.23f))
			{
				// continue Valid... (Ä®ÁEÁ¤È®ÇÑ ½Ã°£ÃøÁ¤ÀÌ ¾ûÓÁ¿E¸ÅÁ÷³Ñ¹ö»ç¿E
			}
			else if ((nWeaponType == MWT_DOUBLE_KATANA || nWeaponType == MWT_DUAL_DAGGER)
				&& (fTime - GetLastShotTime() >= 0.099f)) //±âÁ¸ÀÇ 0.11ÀÏ¶§ ÁÖÇ÷·ÉÀÇ 2¹øÂ° °ø°ÝÀÌ ¾È¸ÔÈ÷´Â °æ¿E¡ ÀÚÁÖ ¹ß»ý..
															//¼öÄ¡¸¦ Á¶ÀýÇØºÃ´Âµ¥ 0.1ÀÏ¶§´Â ¾ÆÁÖ ºE£°Ô ´©¸¦¶§ °°Àº Çö»E¹ß°ß..
															//±×·¡¼­ 0.099·Î ¸ÂÃè´Ù. 
			{
				// continue Valid... (Ä®ÁEÁ¤È®ÇÑ ½Ã°£ÃøÁ¤ÀÌ ¾ûÓÁ¿E¸ÅÁ÷³Ñ¹ö»ç¿E
			}
			else
			{
				// ºÒ¹ýÀûÀÎ ¼ÓµµÀÎ°æ¿ì
#ifdef _CHECKVALIDSHOTLOG
				sprintf(szLog, "IGNORE>> [%s] (%u:%u) Interval(%0.2f) Delay(%0.2f) \n",
					szTime, GetUID().High, GetUID().Low, fTime - GetLastShotTime(), (float)pItem->GetDesc()->m_nDelay / 1000.0f);
				OutputDebugString(szLog);
#endif
				m_dwIsValidTime = -10;
				return false;
			}
		}
			}

#ifdef _CHECKVALIDSHOTLOG
	sprintf(szLog, "[%s] (%u:%u) %u(%f)\n",
		szTime, GetUID().High, GetUID().Low, nItemID, fTime);
	OutputDebugString(szLog);
#endif
	m_dwIsValidTime = FOR_DEBUG_REGISTER;
	return true;
		}

bool ZCharacter::IsObserverTarget()
{
	if (ZGetCombatInterface()->GetObserver()->GetTargetCharacter() == this)
	{
		return true;
	}

	return false;
}
/*//¹öÇÁÁ¤º¸ÀÓ½ÃÁÖ¼® 
void ZCharacter::SetCharacterBuff(MTD_CharBuffInfo* pCharBuffInfo)
{
	if( pCharBuffInfo == NULL ) {
		_ASSERT(0);
		return;
	}

	m_CharacterBuff.Clear();

	for(int i = 0; i < MAX_CHARACTER_SHORT_BUFF_COUNT; i++) {
		m_CharacterBuff.SetShortBuff(i, pCharBuffInfo->ShortBuffInfo[i].uidBuff, pCharBuffInfo->ShortBuffInfo[i].nBuffID, 
			timeGetTime(), pCharBuffInfo->ShortBuffInfo[i].nBuffPeriodRemainder );
	}

	ApplyBuffEffect();
}
*/
void ZCharacter::OnDamagedAnimation(ZObject *pAttacker,int type)
{
	// Ä®À» ¸ÂÀ¸¸EÀá½Ã ¸ØÃá´Ù.  ´©¿öÀÖÀ»¶§ »©°E.
	if(pAttacker==NULL)
		return;

	//jintriple3 ºñÆ® ÆÐÅ· ¸Þ¸ð¸® ÇÁ·Ï½Ã...
	ZCharaterStatusBitPacking & uStatus = m_dwStatusBitPackingValue.Ref();

	if(!uStatus.m_bBlastDrop && !uStatus.m_bBlastDrop)
	{
		rvector dir = GetPosition()-pAttacker->GetPosition();
		Normalize(dir);

		uStatus.m_bStun = true;
		SetVelocity(0,0,0);

		float fRatio = GetMoveSpeedRatio();

		// ½ºÅÏÈ¿°E¾ø¾Ö·Á ¸Ó¸®ÈçµE¾Ö´Ï¸ÞÀÌ¼Ç ¾ø¾Ý´Ù. - ¹öµE
		if(type==SEM_WomanSlash5 || type==SEM_ManSlash5)
		{
			AddVelocity( dir * MAX_SPEED * fRatio );
			MEMBER_SET_CHECKCRC(m_damageInfo, m_nStunType, ZST_SLASH);	// ¸Ó¸® Èçµå´Â ¾Ö´Ï¸ÞÀÌ¼Ç »°´Ù

			ZCharacterObject* pCObj = MDynamicCast(ZCharacterObject, pAttacker);

			if(pCObj) {
				ZC_ENCHANT etype = pCObj->GetEnchantType();
				if( etype == ZC_ENCHANT_LIGHTNING )//¶óÀÌÆ®´×ÀÇ °æ¿Eµ¥¹ÌÁEÀÔÀº ¸ð¼Ç..
					MEMBER_SET_CHECKCRC(m_damageInfo, m_nStunType, ZST_LIGHTNING);
			}

			// °­º£±â·Î ´Ù½Ã ½ºÅÏÀ» ºÎÈ° - dubble
//			m_nStunType = 1;

		} else {
			AddVelocity( dir * RUN_SPEED * fRatio );
			MEMBER_SET_CHECKCRC(m_damageInfo, m_nStunType, (ZSTUNTYPE)((type) %2));
			if(type<=SEM_ManSlash4)
				MEMBER_SET_CHECKCRC(m_damageInfo, m_nStunType, (ZSTUNTYPE)(1 - m_damageInfo.Ref().m_nStunType));
		}
		
		uStatus.m_bPlayDone = false;
	}
}

// ÇÇ¾EDead¸Þ¼¼Áö¿¡ ¸ÂÃEDead ¾Ö´Ï¸ÞÀÌ¼ÇÀ» ¿¬±E
// - ½ÇÁ¦·Î Á×´Â °Í ÆÇÁ¤Àº ¼­¹ö¿¡¼­ Á÷Á¢ ¹Þ¾Æ¿Â ¸Þ¼¼Áö¸¦ °¡Áö°Eµû·Î Ã³¸®ÇÑ´Ù
void ZCharacter::ActDead()
{
	if (m_bInitialized==false)	return;
	if (!IsVisible())			return;

	rvector vDir = m_damageInfo.Ref().m_LastDamageDir;
	vDir.z = 0.f;
	Normalize(vDir);
	vDir.z = 0.6f;//¾à°£À§ÂÊÀ¸·Î
	Normalize(vDir);

	float fForce = 1.f;

	bool bKnockBack = false;

	SetDeadTime(ZGetGame()->GetTime());

	//jintriple3 ¸Þ¸ð¸® ÇÁ·Ï½Ã...ºñÆ® ÆÐÅ·..
	ZCharaterStatusBitPacking & uStatus = m_dwStatusBitPackingValue.Ref();

	if (uStatus.m_bChatEffect) uStatus.m_bChatEffect = false;

#ifdef _ICONCHAT
	if (uStatus.m_bChatVoice) uStatus.m_bChatVoice = false;
	if (m_Status.Ref().isTalking = true) m_Status.Ref().isTalking = false; 
#endif

	if ((GetStateLower() != ZC_STATE_LOWER_DIE1) && (GetStateLower() != ZC_STATE_LOWER_DIE2) && 
		(GetStateLower() != ZC_STATE_LOWER_DIE3) && (GetStateLower() != ZC_STATE_LOWER_DIE4) )
	{
		ZC_STATE_LOWER lower_motion;

		float dot = m_damageInfo.Ref().m_LastDamageDot;

		switch(m_damageInfo.Ref().m_LastDamageWeapon) 
		{
		// melee
		case MWT_DAGGER:
		case MWT_DUAL_DAGGER: 
		case MWT_KATANA:
		case MWT_GREAT_SWORD:
		case MWT_DOUBLE_KATANA:
		case MWT_SPYCASE:
			bKnockBack = false;
			break;

		case MWT_PISTOL:
		case MWT_PISTOLx2:
		case MWT_REVOLVER:
		case MWT_REVOLVERx2:
		case MWT_SMG:
		case MWT_SMGx2:
		case MWT_RIFLE:
		case MWT_SNIFER:
			// °Å¸®°¡ 8mº¸´Ù °¡±ûÛEE
			if( m_damageInfo.Ref().m_LastDamageDistance < 800.f )
			{
				// 400 ~ 900
				fForce = 300 + (1.f-(m_damageInfo.Ref().m_LastDamageDistance/800.f)) * 500.f;

				bKnockBack = true;
			}
			break;

		case MWT_SHOTGUN:
		case MWT_SAWED_SHOTGUN:
		case MWT_MACHINEGUN:
			// °Å¸®°¡ 10mº¸´Ù °¡±ûÛEE
			if( m_damageInfo.Ref().m_LastDamageDistance < 1000.f )
			{
				// 500 ~ 1000
				fForce = 400 + (1.f-(m_damageInfo.Ref().m_LastDamageDistance/1000.f)) * 500.f;

				bKnockBack = true;
			}
			break;

		case MWT_ROCKET:
		case MWT_FRAGMENTATION:
			// ³¯¶ó°¡´Â Ã³¸®ÇÊ¿E
			fForce = 600.f;
			bKnockBack = true;

			break;

#ifdef _PORTALGUN 1
		case MWT_PORTAL_GUN:
			fForce = 5000.f;
			bKnockBack = true;
#endif
		
		case MWT_TRAP:	
		case MWT_TRAP_SPY:
			{
				bKnockBack = false;
			}			
			break;

		case MWT_DYNAMITYE:
		case MWT_STUN_GRENADE_SPY:
			{
				fForce = 600.f;
				bKnockBack = true;
			}
			break;

		default:
			lower_motion = ZC_STATE_LOWER_DIE1;
			break;

		}

		if(m_damageInfo.Ref().m_LastDamageType == ZD_BULLET_HEADSHOT) {
			bKnockBack = true;
			fForce = 700.f;
		}

		if(bKnockBack) {
			// zcharacterÀÇ ³Ë¹éÀº ¾øÀ¸¹Ç·Î..
			ZObject::OnKnockback(vDir, fForce );
		}

		if(bKnockBack) {

			if(dot<0)	lower_motion = ZC_STATE_LOWER_DIE3;
			else		lower_motion = ZC_STATE_LOWER_DIE4;
		}
		else {

			if(dot<0)	lower_motion = ZC_STATE_LOWER_DIE1;
			else		lower_motion = ZC_STATE_LOWER_DIE2;
		}

		// ³ª¶E¿¡´Ï¸ÞÀÌ¼Ç 
		if (GetPosition().z <= DIE_CRITICAL_LINE)
		{
			lower_motion = ZC_STATE_PIT;

			m_dwStatusBitPackingValue.Ref().m_bFallingToNarak = true;
		}
		SetAnimationLower(lower_motion);

//		mlog("dir : %f %f %f / %d : force %f / dist %f \n",vDir.x,vDir.y,vDir.z,lower_motion,fForce,m_LastDamageDistance);
	}

	if (GetStateUpper() != ZC_STATE_UPPER_NONE )
	{
		SetAnimationUpper(ZC_STATE_UPPER_NONE);
	}

	// excellent ÆÇÁ¤
#define EXCELLENT_TIME	3.0f
	ZCharacter *pLastAttacker = (ZCharacter*) ZGetCharacterManager()->Find(GetLastAttacker());
	if(pLastAttacker && pLastAttacker!=this)
	{
		if( ZGetGame()->GetTime()-pLastAttacker->m_killInfo.Ref().m_fLastKillTime < EXCELLENT_TIME 
			&& ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL
			&& ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT 
#ifdef _MAGICBOX
			&& ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DROPMAGIC &&
#endif
			ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_SPY)
		{
			MEMBER_SET_CHECKCRC(pLastAttacker->GetStatus(), nExcellent, pLastAttacker->GetStatus().Ref().nExcellent+1)
			pLastAttacker->AddIcon(ZCI_EXCELLENT);
		}

		MEMBER_SET_CHECKCRC(pLastAttacker->m_killInfo, m_fLastKillTime, ZGetGame()->GetTime());

		// fantastic ÆÇÁ¤
		if(!uStatus.m_bLand && GetDistToFloor()>200.f && ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL
			&& ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT && 
#ifdef _MAGICBOX
			ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DROPMAGIC &&
#endif
			ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_SPY)
		{
			MEMBER_SET_CHECKCRC(pLastAttacker->GetStatus(), nFantastic, pLastAttacker->GetStatus().Ref().nFantastic+1)
			pLastAttacker->AddIcon(ZCI_FANTASTIC);
		}

		// unbelievable ÆÇÁ¤
		if(pLastAttacker && ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL
			&& ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT && 
#ifdef _MAGICBOX
			ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DROPMAGIC &&
#endif
			ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_SPY)
		{
			m_killInfo.CheckCrc();
			ZCharacter::KillInfo& lastAttackerKillInfo = m_killInfo.Ref();
			lastAttackerKillInfo.m_nKillsThisRound++;
			if(lastAttackerKillInfo.m_nKillsThisRound==3)
				MEMBER_SET_CHECKCRC(pLastAttacker->GetStatus(), nUnbelievable, pLastAttacker->GetStatus().Ref().nUnbelievable+1);
			if(lastAttackerKillInfo.m_nKillsThisRound>=3)
			{
				pLastAttacker->AddIcon(ZCI_UNBELIEVABLE);
			}
			m_killInfo.MakeCrc();
		}
	}
}

void ZCharacter::AddIcon(int nIcon)
{
	if(nIcon<0 || nIcon>=6) return;

	ZGetEffectManager()->AddCharacterIcon(this,nIcon);

	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if(pTargetCharacter == this)
	{
		ZGetScreenEffectManager()->AddPraise(nIcon);
	}

	/*
	float fDelta=g_pGame->GetTime()-m_fIconStartTime[nIcon];
	if(fDelta<CHARACTER_ICON_DELAY) return;	// ³ªÅ¸³ª ÀÖÀ»¶§ ³ª¿ÔÀ¸¸E¹«½Ã
		
	m_fIconStartTime[nIcon]=g_pGame->GetTime();
	*/
}
/*
float ZCharacter::GetIconStartTime(int nIcon)
{
	if(nIcon<0 || nIcon>=5) return -HP_SCALE;

	return m_fIconStartTime[nIcon];
}
*/

void ZCharacter::ToggleClothSimulation()
{
	if(!m_pVMesh) return;

	if( Enable_Cloth )
		m_pVMesh->ChangeChestCloth(1.f,1);
	else
		m_pVMesh->DestroyCloth();
}					  

bool ZCharacter::Save(ZFile *file)
{
	size_t n;

	n=zfwrite(&m_bHero,sizeof(m_bHero),1,file);
	if(n!=1) return false;

	n=zfwrite(&m_MInitialInfo.Ref(),sizeof(m_MInitialInfo.Ref()),1,file);
	if(n!=1) return false;

	n=zfwrite(&m_UID,sizeof(m_UID),1,file);
	if(n!=1) return false;

	ZCharacterProperty_Old oldProperty;
	m_Property.CopyToOldStruct(oldProperty);
	n=zfwrite(&oldProperty,sizeof(oldProperty),1,file);
	if(n!=1) return false;

	float fHP = m_pModule_HPAP->GetHP();
	n=zfwrite(&fHP,sizeof(float),1,file);
	if(n!=1) return false;

	float fAP = m_pModule_HPAP->GetAP();
	n=zfwrite(&fAP,sizeof(float),1,file);
	if(n!=1) return false;

	n=zfwrite(&m_Status.Ref(),sizeof(ZCharacterStatus),1,file);
	if(n!=1) return false;

	if(!m_Items.Save(file)) return false;

	n=zfwrite((void*)&GetPosition(),sizeof(GetPosition()),1,file);
	if(n!=1) return false;

	n=zfwrite(&m_Direction,sizeof(m_Direction),1,file);
	if(n!=1) return false;

	n=zfwrite(&m_nTeamID.Ref(),sizeof(m_nTeamID.Ref()),1,file);
	if(n!=1) return false;

	//jintriple3 ¸Þ¸ð¸® ÇÁ·Ï½Ã...ºñÆ® ÆÐÅ·..
	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	bool bDie = uStatus.m_bDie;
	n=zfwrite(&bDie,sizeof(bDie),1,file);
	if(n!=1) return false;

	bool bAdminHide = uStatus.m_bAdminHide;
	n=zfwrite(&bAdminHide,sizeof(bAdminHide),1,file);
	if(n!=1) return false;

	return true;
}

bool ZCharacter::Load(ZFile *file,int nVersion)
{
	size_t n;

	// m_bHero ¿Í initial info ´Â ¾Õ¿¡¼­ createÇÏ±EÀE¡ ÀÐ¾ú´Ù.
//	n=fread(&m_InitialInfo,sizeof(m_InitialInfo),1,file);
//	if(n!=1) return false;

	n=zfread(&m_UID,sizeof(m_UID),1,file);
	if(n!=1) return false;

	ZCharacterProperty_Old oldProperty;
	n=zfread(&oldProperty,sizeof(oldProperty),1,file);
	if(n!=1) return false;
	m_Property.CopyFromOldStruct(oldProperty);

	InitHPAP();

	float fHP;
	n=zfread(&fHP,sizeof(float),1,file);
	if(n!=1) return false;
	m_pModule_HPAP->SetHP(fHP);

	float fAP;
	n=zfread(&fAP,sizeof(float),1,file);
	if(n!=1) return false;
	m_pModule_HPAP->SetAP(fAP);
	if (nVersion <= 9)
	{
		n = zfread(&m_Status.Ref(), sizeof(ZCharacterStatus_Old), 1, file);
		if (n != 1) return false;
		m_Status.MakeCrc();
	}
	else
	{
		n = zfread(&m_Status.Ref(), sizeof(ZCharacterStatus), 1, file);
		if (n != 1) return false;
		m_Status.MakeCrc();
	}

	if(!m_Items.Load(file, nVersion)) return false;

	/*
	MMatchCharItemParts nSelectedWeapon=m_Items.GetSelectedWeaponParts();
	m_Items.ResetItems();

	ChangeWeapon(nSelectedWeapon);
	*/

	rvector pos;
	n=zfread(&pos,sizeof(pos),1,file);
	if(n!=1) return false;
	SetPosition(pos);

	n=zfread(&m_Direction,sizeof(m_Direction),1,file);
	if(n!=1) return false;

	MMatchTeam teamID;
	n=zfread(&teamID,sizeof(teamID),1,file);
	if(n!=1) return false;
	m_nTeamID.Set_CheckCrc(teamID);

	//jintriple3 ¸Þ¸ð¸® ÇÁ·Ï½Ã...ºñÆ® ÆÐÅ·..
	ZCharaterStatusBitPacking& uStatus = m_dwStatusBitPackingValue.Ref();

	bool bDie;
	n=zfread(&bDie,sizeof(bDie),1,file);
	uStatus.m_bDie = bDie;
	if(n!=1) return false;

	if(nVersion>=2) {
		bool bAdminHide;
		n=zfread(&bAdminHide,sizeof(bAdminHide),1,file);
		uStatus.m_bAdminHide = bAdminHide;
		if(n!=1) return false;
	}

	return true;
}

/*
void ZCharacter::DrawForce(bool bDrawShadow)
{
	Draw();

	// ±×¸²ÀÚ ÂEE
	if (bDrawShadow)
	{
		ZShadow::predraw();
		DrawShadow();
		ZShadow::postdraw();
	}
}
*/

void ZCharacter::OnLevelDown()
{
	m_Property.nLevel--;
}
void ZCharacter::OnLevelUp()
{
	m_Property.nLevel++;
	ZGetEffectManager()->AddLevelUpEffect(this);
}

void ZCharacter::LevelUp()
{
	OnLevelUp();
	CHECK_RETURN_CALLSTACK(LevelUp);
}
void ZCharacter::LevelDown()
{
	OnLevelDown();
	CHECK_RETURN_CALLSTACK(LevelDown);
}


RMesh *ZCharacter::GetWeaponMesh(MMatchCharItemParts parts)
{
	ZItem *pWeapon = m_Items.GetItem(parts);

	if(!pWeapon) return NULL;

	if( pWeapon->GetDesc()==NULL ) return NULL;

	RMesh* pMesh = ZGetWeaponMeshMgr()->Get( pWeapon->GetDesc()->m_pMItemName->Ref().m_szMeshName );
	return pMesh;
}

bool ZCharacter::IsRunWall()
{
	ZC_STATE_LOWER s = m_AniState_Lower.Ref();

	if( ( s == ZC_STATE_LOWER_RUN_WALL_LEFT ) || 
		( s == ZC_STATE_LOWER_RUN_WALL_LEFT_DOWN ) || 
		( s == ZC_STATE_LOWER_RUN_WALL ) || 
		( s == ZC_STATE_LOWER_RUN_WALL_DOWN_FORWARD ) || 
		( s == ZC_STATE_LOWER_RUN_WALL_DOWN ) || 
		( s == ZC_STATE_LOWER_RUN_WALL_RIGHT ) || 
		( s == ZC_STATE_LOWER_RUN_WALL_RIGHT_DOWN ) ||
		( s == ZC_STATE_LOWER_JUMP_WALL_FORWARD ) ||
		( s == ZC_STATE_LOWER_JUMP_WALL_BACK ) ||
		( s == ZC_STATE_LOWER_JUMP_WALL_LEFT ) ||
		( s == ZC_STATE_LOWER_JUMP_WALL_RIGHT ) ) {
		return true;
	}
	return false;
}

bool ZCharacter::IsMeleeWeapon()
{
	ZItem* pItem = m_Items.GetSelectedWeapon();

	if(pItem) {
		if(pItem->GetDesc()) {
			if(pItem->GetDesc()->m_nType.Ref() == MMIT_MELEE) {		
				return true;
			}
		}
	}

	return false;
}

bool ZCharacter::IsCollideable()
{
	//jintriple3 ºñÆ® ÆÐÅ· ¸Þ¸ð¸® ÇÁ·Ï½Ã...
	const ZCharaterStatusBitPacking & uStatus = m_dwStatusBitPackingValue.Ref();

	if (m_Collision.IsCollideable())
	{
		return ((!IsDie() && !uStatus.m_bBlastDrop));
	}

	return m_Collision.IsCollideable();
}

bool ZCharacter::IsAttackable()
{
	// Á×¾ûÜÖÀ¸¸E°ø°ÝÇÒ ¼E¾ø´Ù.
	if (IsDie()) return false;
	return true;
}

float ZCharacter::ColTest(const rvector& pos, const rvector& vec, float radius, rplane* out)
{
	return SweepTest(rsphere(pos, radius), vec, rsphere(GetPosition(), CHARACTER_COLLISION_DIST), out);
}

// ¹æ¾ûÝßÀÎ°¡ ?
bool ZCharacter::IsGuard()
{
	return ((ZC_STATE_LOWER_GUARD_IDLE<=m_AniState_Lower.Ref() && m_AniState_Lower.Ref()<=ZC_STATE_LOWER_GUARD_BLOCK2) ||
		(ZC_STATE_UPPER_GUARD_IDLE<=m_AniState_Upper.Ref() && m_AniState_Upper.Ref()<=ZC_STATE_UPPER_GUARD_BLOCK2));
}
void ZCharacter::InitRound()
{
	// Custom: Slash Equip Init
	InitSlashEquipment();

	{
		MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(GetUID());
		if (pCache && pCache->GetCostume())
		{
			unsigned long int nItemID = pCache->GetCostume()->nEquipedItemID[MMCIP_JJANG];
			if (nItemID != 0)
			{
				MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
				if (pDesc)
				{
					const char* szMesh = pDesc->m_pMItemName->Ref().m_szMeshName;
					ZGetEffectManager()->AddStarEffect(this, szMesh);
				}
			}
		}
	}

	// Custom: Infected
	//if (m_pVMesh)
	//	m_pVMesh->ClearScale();

	m_bInfected = false;

	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_INFECTED)
	{
		const MTD_CharInfo* pCharInfo = &m_MInitialInfo.Ref();

		for (int i = 0; i < MMCIP_END; i++) {	///< ¾ÆÀÌÅÛ ¼¼ÆÃ
			m_Items.EquipItem(MMatchCharItemParts(i), pCharInfo->nEquipedItemDesc[i], pCharInfo->nEquipedItemCount[i]);
		}
	
		float fAddedAP = DEFAULT_CHAR_AP;			///< AP ÀçÁ¶Á¤
		for (int i = 0; i < MMCIP_END; i++) {
			if (!m_Items.GetItem(MMatchCharItemParts(i))->IsEmpty()) {
				//ÇÙ ¹æ¾ûÛEÀÓ½ÃÄÚµE
				// Custom: Bypass AP limit of 40 (MAIET trap check)
				//if(m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nAP.Ref() > 40) {
				//	m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nAP.Ref() = 0; 
				//}
				fAddedAP += m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nAP.Ref();
			}
		}
	
		float fAddedHP = DEFAULT_CHAR_HP;			///< HP ÀçÁ¶Á¤
		for (int i = 0; i < MMCIP_END; i++) {
			if (!m_Items.GetItem(MMatchCharItemParts(i))->IsEmpty()) {
				fAddedHP += m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nHP.Ref();
			}
		}

		m_Property.fMaxAP.Set_CheckCrc(pCharInfo->nAP + fAddedAP);
		m_Property.fMaxHP.Set_CheckCrc(pCharInfo->nHP + fAddedHP);

		m_fPreMaxHP = pCharInfo->nHP + fAddedHP;
		m_fPreMaxAP = pCharInfo->nAP + fAddedAP;

		ZItem* pMeleeItem = m_Items.GetItem(MMCIP_MELEE);
		if (pMeleeItem != NULL && pMeleeItem->GetDesc() != NULL)
		{
			OnChangeWeapon(pMeleeItem->GetDesc()->m_pMItemName->Ref().m_szMeshName);
			ChangeWeapon(MMCIP_MELEE);
		}

		InitMeshParts();
	}

	// Custom: Ghost Mode - reset item each round for ghost
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_GHOST && GetTeamID() == MMT_RED)
	{
		unsigned long nEquipedItemDesc[MMCIP_END], nEquipedItemCount[MMCIP_END];
		memset(nEquipedItemDesc, 0, sizeof(nEquipedItemDesc));
		memset(nEquipedItemCount, 0, sizeof(nEquipedItemCount));

		nEquipedItemDesc[MMCIP_PRIMARY] = NULL;
		nEquipedItemDesc[MMCIP_SECONDARY] = NULL;
		nEquipedItemCount[MMCIP_PRIMARY] = 1;
		nEquipedItemCount[MMCIP_SECONDARY] = 1;

		for (int i = 8; i <= 9; i++)
		{
			m_Items.EquipItem(MMatchCharItemParts(i), nEquipedItemDesc[i], nEquipedItemCount[i]);
		}

		ZItem* pMeleeItem = m_Items.GetItem(MMCIP_MELEE);
		if (pMeleeItem != NULL && pMeleeItem->GetDesc() != NULL)
		{
			OnChangeWeapon(pMeleeItem->GetDesc()->m_pMItemName->Ref().m_szMeshName);
			ChangeWeapon(MMCIP_MELEE);
		}
	}
}
#ifdef _UPCHARCMD
bool ZCharacter::Load(MTD_CharInfo* pCharInfo)
{
	if (!pCharInfo)
	{
		mlog("Unable to Load Character. =()NULL;");
		return false;
	}

	MMatchCharItemParts nSelectedWeapon = m_Items.GetSelectedWeaponParts();

	for (int i = 0; i < MMCIP_END; i++)
	{
		string szItemType =
			i == MMatchCharItemParts::MMCIP_MELEE ? "Melee" :
			i == MMatchCharItemParts::MMCIP_PRIMARY ? "Primary" :
			i == MMatchCharItemParts::MMCIP_SECONDARY ? "Secondary" :
			i == MMatchCharItemParts::MMCIP_CUSTOM1 ? "Custom 1" :
			i == MMatchCharItemParts::MMCIP_CUSTOM2 ? "Custom 2" :
			i == MMatchCharItemParts::MMCIP_CUSTOM3 ? "Custom 3" :
			"other";
		if (i > MMatchCharItemParts::MMCIP_FINGERR && i < MMatchCharItemParts::MMCIP_AVATAR)
		{
			if (pCharInfo->nEquipedItemCount[i] > 0)
			{
				m_Items.EquipItem(MMatchCharItemParts(i), pCharInfo->nEquipedItemDesc[i], pCharInfo->nEquipedItemCount[i]);
				m_MInitialInfo.CheckCrc();
				GetCharInfo()->nEquipedItemDesc[i] = pCharInfo->nEquipedItemDesc[i];
				m_MInitialInfo.MakeCrc();

				m_MInitialInfo.CheckCrc();
				GetCharInfo()->nEquipedItemCount[i] = pCharInfo->nEquipedItemCount[i];
				m_MInitialInfo.MakeCrc();

				m_Items.Reload();

				MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(pCharInfo->nEquipedItemDesc[i]);

				if (!pDesc)
					continue;

				string szItemName =
					strlen(pDesc->m_pMItemName->Ref().m_szItemName) > 0 ?
					string(pDesc->m_pMItemName->Ref().m_szItemName) :
					"nill";

				ZGetGameClient()->OutputMessage_Sub("%s : %s", szItemType.c_str(), szItemName.c_str());
			}
		}
	}

	ChangeWeapon(nSelectedWeapon);

	InitItemBullet();
	InitHPAP();
	return true;
}
#endif
ZOBJECTHITTEST ZCharacter::HitTest( const rvector& origin, const rvector& to, float fTime, rvector *pOutPos )
{
	
	rvector footpos,headpos,characterdir;
	if (ZObject::GetHistory(&footpos, &characterdir, fTime))
	{
		footpos.z += 5.f;
		if (IsRendered())
		{
			headpos = GetVisualMesh()->GetHeadPosition();
			headpos.z+= 5.f; 
		}else
			headpos=footpos+rvector(0,0,180);		

		rvector rootpos = (footpos + headpos)*0.5f;

		rvector nearest = GetNearestPoint(headpos, origin, to);
		float fDist = Magnitude(nearest - headpos);
		float fDistToChar = Magnitude(nearest - origin);

		rvector ap,cp;
#ifdef _ANTIHACK
		if (fDist < ZGuard::HIT_HEAD_15) {
			if (pOutPos) *pOutPos = nearest;
			return ZOH_HEAD;
		}
		else {
			rvector dir = to - origin;
			Normalize(dir);

			rvector rootdir = (rootpos - headpos);
			Normalize(rootdir);
			float fDist = GetDistanceBetweenLineSegment(origin, to, headpos + ZGuard::HIT_LEGS_20 * rootdir, rootpos - ZGuard::HIT_LEGS_20 * rootdir, &ap, &cp);
			if (fDist < ZGuard::HIT_BODY_30) {
				rvector ap2cp = ap - cp;
				float fap2cpsq = D3DXVec3LengthSq(&ap2cp);
				float fdiff = sqrtf(ZGuard::HIT_BODY_30 * ZGuard::HIT_BODY_30 - fap2cpsq);

				if (pOutPos) *pOutPos = ap - dir * fdiff;;
				return ZOH_BODY;
			}
			else {
				float fDist = GetDistanceBetweenLineSegment(origin, to, rootpos - ZGuard::HIT_LEGS_20 * rootdir, footpos, &ap, &cp);
				if (fDist < ZGuard::HIT_BODY_30) {
					rvector ap2cp = ap - cp;
					float fap2cpsq = D3DXVec3LengthSq(&ap2cp);
					float fdiff = sqrtf(ZGuard::HIT_BODY_30 * ZGuard::HIT_BODY_30 - fap2cpsq);

					if (pOutPos) *pOutPos = ap - dir * fdiff;;
					return ZOH_LEGS;
				}
			}
		}
	}
	return ZOH_NONE;
#else
		if (fDist < 15.f)
		{
			if (pOutPos) *pOutPos = nearest;
			return ZOH_HEAD;
		}
		else
		{
			rvector dir = to - origin;
			Normalize(dir);

			rvector rootdir=(rootpos-headpos);
			Normalize(rootdir);
			float fDist=GetDistanceBetweenLineSegment(origin,to,headpos+20.f*rootdir,rootpos-20.f*rootdir,&ap,&cp);
			if(fDist<30)	
			{
				rvector ap2cp=ap-cp;
				float fap2cpsq=D3DXVec3LengthSq(&ap2cp);
				float fdiff=sqrtf(30.f*30.f-fap2cpsq);

				if(pOutPos) *pOutPos = ap-dir*fdiff;;
				return ZOH_BODY;
			}
			else
			{
				float fDist=GetDistanceBetweenLineSegment(origin,to,rootpos-20.f*rootdir,footpos,&ap,&cp);
				if(fDist<30)	
				{
					rvector ap2cp=ap-cp;
					float fap2cpsq=D3DXVec3LengthSq(&ap2cp);
					float fdiff=sqrtf(30.f*30.f-fap2cpsq);

					if(pOutPos) *pOutPos = ap-dir*fdiff;;
					return ZOH_LEGS;
				}
			}
		}
	}
	return ZOH_NONE;
#endif
}

//void ZCharacter::OnDamagedMelee(ZObject* pAttacker, float fDamage, float fPiercingRatio, MMatchWeaponType weaponType, int nMeleeType)
//{
//	if (m_bInitialized==false) return;
//	if (!IsVisible() || IsDie()) return;
//
//	bool bCanAttack = g_pGame->IsAttackable(pAttacker,this);
//
//	rvector dir = GetPosition() - pAttacker->GetPosition();
//	Normalize(dir);
//
//// lastAttacker ¾²ÁE¾Ê°EÀÖ´Ù
////	if (bCanAttack) m_LastAttacker= pAttacker->m_UID;
//	m_LastDamageDir = dir;
//	m_LastDamageType = ZD_KATANA;
//	m_LastDamageWeapon = weaponType;
//	m_LastDamageDot = DotProduct( m_Direction,dir );
//	m_LastDamageDistance = Magnitude(GetPosition() - pAttacker->GetPosition());
//
//	// hp, ap °è»E
//	if (bCanAttack)
//	{
//		ZObject::OnDamagedMelee(pAttacker,fDamage,fPiercingRatio,weaponType,nMeleeType);
//		
//		if( pAttacker == g_pGame->m_pMyCharacter )
//			g_pGame->m_pMyCharacter->HitSound();
//	}
//
//	OnDamagedAnimation(pAttacker,nMeleeType);
//}
//
//void ZCharacter::OnDamagedRange( ZObject* pAttacker, float fDamage, float fPiercingRatio, MMatchWeaponType weaponType)
//{
//	if (m_bInitialized==false) return;
//	if (!IsVisible() || IsDie()) return;
//
//	bool bCanAttack = g_pGame->IsAttackable(pAttacker,this);
//
//	rvector dir = GetPosition() - pAttacker->GetPosition();
//	Normalize(dir);
//
//// lastAttacker ¾²ÁE¾Ê°EÀÖ´Ù
////	if (bCanAttack) m_LastAttacker= pAttacker->m_UID;
//	m_LastDamageDir = dir;
//	m_LastDamageType = (partstype==eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET
//	m_LastDamageWeapon = weaponType;
//	m_LastDamageDot = DotProduct( m_Direction,dir );
//	m_LastDamageDistance = Magnitude(GetPosition() - pAttacker->GetPosition());
//
//	// hp, ap °è»E
//	if (bCanAttack)
//	{
//		ZObject::OnDamagedRange(pAttacker,fDamage,fPiercingRatio,weaponType);
//		
//		if( pAttacker == g_pGame->m_pMyCharacter )
//			g_pGame->m_pMyCharacter->HitSound();
//	}
//}
//

//jintriple3 ????? ???????? ??y ????E???E????


void ZCharacter::OnDamagedAPlayer(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeTpye)
{
	if (damageType == ZD_MELEE)
	{
		OnDamaged(pAttacker, srcPos, damageType, weaponType, fDamage, fPiercingRatio, nMeleeTpye);
		return;
	}

	if (pAttacker != NULL && fDamage > 0)
	{
		if (pAttacker == ZGetGame()->m_pMyCharacter && this != pAttacker && !ZGetGame()->m_pMyCharacter->IsDie())
		{
			void* pBlobArray = MMakeBlobArray(sizeof(MTD_AntiLeadN), 1);
			void* pBlobElement = MGetBlobArrayElement(pBlobArray, 0);

			MTD_AntiLeadN shotInfo;
			shotInfo.fDamage = fDamage;
			shotInfo.fPosX = srcPos.x;
			shotInfo.fPosY = srcPos.y;
			shotInfo.fPosZ = srcPos.z;
			shotInfo.nDamageType = damageType;
			shotInfo.fRatio = fPiercingRatio;
			shotInfo.nLowId = GetUID().Low;
			shotInfo.nWeaponType = weaponType;

			memcpy(pBlobElement, &shotInfo, sizeof(MTD_AntiLeadN));
			//ZGetGameClient()->GetPeerPacketCrypter().Encrypt((char*)pBlobElement, sizeof(MTD_AntiLeadN));
			ZPOSTCMDLEAD(MC_GUNZ_ANTI_LEAD_N, GetUID(), MCommandParameterBlob(pBlobArray, MGetBlobArraySize(pBlobArray)));
		}
	}
}
void ZCharacter::OnDamagedAPlayer(vector<MTD_AntiLeadN*> vShotInfo)
{
	if (vShotInfo.size() > 0)
	{
		if (ZGetGameClient()->GetPlayerUID().Low != GetUID().Low)
		{
			void* pBlobArray = MMakeBlobArray(sizeof(MTD_AntiLeadN), vShotInfo.size());

			for (int i = 0; i < vShotInfo.size(); ++i)
			{
				MTD_AntiLeadN* pShot = vShotInfo[i];
				void* pElement = MGetBlobArrayElement(pBlobArray, i);
				memcpy(pElement, pShot, sizeof(MTD_AntiLeadN));
				//ZGetGameClient()->GetPeerPacketCrypter().Encrypt((char*)pElement, sizeof(MTD_AntiLeadN));
			}

			ZPOSTCMDLEAD(MC_GUNZ_ANTI_LEAD_N, GetUID(), MCommandParameterBlob(pBlobArray, MGetBlobArraySize(pBlobArray)));
		}
	}
}

void ZCharacter::OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	if (m_bInitialized==false) 
		PROTECT_DEBUG_REGISTER(m_bInitialized_DebugRegister == false)
			return;
	bool bDebugRegister  = !IsVisible() || IsDie();
	if (!IsVisible() || IsDie()) 
		PROTECT_DEBUG_REGISTER(bDebugRegister)
			return;

	// ÀÚ±â°¡ ½EÆø¹ß µ¥¹ÌÁE& ³ª¶Eµ¥¹ÌÁö´Â ¹«Á¶°Ç ¸Ô´Â´Ù
	bool bCanAttack = ZGetGame()->CanAttack(pAttacker,this) || (pAttacker==this && (damageType==ZD_EXPLOSION || damageType==ZD_FALLING));
	bDebugRegister = ZGetGame()->CanAttack(pAttacker,this) || (pAttacker==this && (damageType==ZD_EXPLOSION || damageType==ZD_FALLING));
    bool bReturnValue = ZGetGameTypeManager()->IsTeamExtremeGame(ZGetGame()->GetMatch()->GetMatchType());

	// Custom: Ghost Mode - Desperate
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_GHOST) 
	{
		m_Property.nGhost.Set_CheckCrc(1); 
		m_nGhost = 1; 
		GhostTime = timeGetTime();
	} 

	// Custom: CTF
	if ( ZGetGameTypeManager()->IsTeamExtremeGame(ZGetGame()->GetMatch()->GetMatchType()) )
	{
		PROTECT_DEBUG_REGISTER(bReturnValue)
		{
			if( damageType != ZD_FALLING)
			{
				bCanAttack &= !IsInvincible();
				// jintriple3 디버그 레지스터 해킹 방지를 위해 한번 더 실행했다..코드 옵티마이즈를 건너뛰기 위해서..ㅜㅜ
				bDebugRegister &= !IsInvincible();
			}
		}
	}

	rvector dir = GetPosition() - srcPos;
	Normalize(dir);

	// Custom: Vampire packet
	if((ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_VAMPIRE) && !ZGetGameClient()->GetMatchStageSetting()->IsQuestDrived()
		&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		if( pAttacker != NULL && pAttacker->GetUID() != ZGetMyUID() && this->GetUID() == ZGetMyUID() )
			ZPostVampire( pAttacker->GetUID(), fDamage );
	}

#ifdef _LADDERWARSPACKETS
	if (ZGetGameClient()->IsLadderWarsChannel())
	{
		if (pAttacker != NULL && pAttacker->GetUID() != ZGetMyUID() && this->GetUID() == ZGetMyUID())
			ZPostLadderWars(pAttacker->GetUID(), fDamage);
	}
#endif
	m_damageInfo.CheckCrc();
	m_damageInfo.Ref().m_LastDamageDir = dir;
	m_damageInfo.Ref().m_LastDamageType = damageType;
	m_damageInfo.Ref().m_LastDamageWeapon = weaponType;
	m_damageInfo.Ref().m_LastDamageDot = DotProduct( m_Direction,dir );
	m_damageInfo.Ref().m_LastDamageDistance = Magnitude(GetPosition() - srcPos);
	m_damageInfo.MakeCrc();

	// hp, ap °è»E
	//jintriple3 µð¹ö±× ·¹Áö½ºÅÍ ÇÙ ¹æÁö¿EÄÚµE...
	if(!bCanAttack)
		PROTECT_DEBUG_REGISTER(!bDebugRegister)
			return;


	ZObject::OnDamaged(pAttacker,srcPos,damageType,weaponType,fDamage,fPiercingRatio,nMeleeType);

	if(damageType==ZD_MELEE) OnDamagedAnimation(pAttacker,nMeleeType);

	m_dwStatusBitPackingValue.Ref().m_bDamaged = true;
#ifdef _KILLFEED
	SetWeaponDamaged((MMatchWeaponType)weaponType);
	ZPostLastWeaponUsed(weaponType);
	m_nKillShotWeapon = weaponType;
#endif

#ifdef _CHATOUTPUT_ENABLE_CHAR_DAMAGE_INFO_	// ³»ºÎºôµå¿¡¼­ ´EÌÁEÁ¤º¸ Ãâ·Â
	char szDamagePrint[256];
	// Custom: Changed damage debug print string
	sprintf(szDamagePrint, "%s was hit (dmg: %.0f) - Remaining HP[%2.0f] AP[%2.0f]", GetUserName(), fDamage, GetHP(), GetAP());
	//sprintf(szDamagePrint, "%s¿¡°Ô ´EÌÁö[%2.1f], ³²Àº HP[%2.1f] AP[%2.1f]", GetUserName(), fDamage, GetHP(), GetAP());

	// Custom: Removed ugly shotgun dmg spam on chat feed
	//if (weaponType == MWT_SHOTGUN || weaponType == MWT_SAWED_SHOTGUN)
		return;

	ZChatOutput(MCOLOR(255, 100, 100), szDamagePrint);
#endif

}

void ZCharacter::OnScream()
{
	if(GetProperty()->nSex==MMS_MALE)
		ZGetSoundEngine()->PlaySound("ooh_male",GetPosition(),IsObserverTarget());
	else			
		ZGetSoundEngine()->PlaySound("ooh_female",GetPosition(),IsObserverTarget());
}

void ZCharacter::OnMeleeGuardSuccess()
{
	ZGetSoundEngine()->PlaySound("fx_guard",GetPosition(),IsObserverTarget());
}

void ZCharacter::OnShot()
{
	//jintriple3 ºñÆ® ÆÐÅ· ¸Þ¸ð¸® ÇÁ·Ï½Ã...
	ZCharaterStatusBitPacking & uStatus =m_dwStatusBitPackingValue.Ref();
	if (uStatus.m_bChatEffect) uStatus.m_bChatEffect = false;

#ifdef _ICONCHAT
	if (uStatus.m_bChatVoice) uStatus.m_bChatVoice = false;
	if (m_Status.Ref().isTalking = true) m_Status.Ref().isTalking = false;  
#endif

}


void ZCharacter::SetInvincibleTime(int nDuration)
{
//	m_dwInvincibleStartTime = gaepanEncode(timeGetTime(), 4);
//	m_dwInvincibleDuration = gaepanEncode(nDuration, 5);
	m_pMdwInvincibleStartTime->Set_CheckCrc(gaepanEncode(timeGetTime(), 4));
	m_pMdwInvincibleDuration->Set_CheckCrc(gaepanEncode(nDuration, 5));
}

bool ZCharacter::IsInvincible()
{
//	return ((int)timeGetTime() < (gaepanDecode(m_dwInvincibleStartTime, 4) + gaepanDecode(m_dwInvincibleDuration, 5)));
	return ((int)timeGetTime() < (gaepanDecode(m_pMdwInvincibleStartTime->Ref(), 4) + gaepanDecode(m_pMdwInvincibleDuration->Ref(), 5)));
}

void ZCharacter::ShiftFugitiveValues()
{
	m_pModule_HPAP->ShiftFugitiveValues();

	m_Property.nameCharClan.ShiftHeapPos_CheckCrc();
	m_Property.fMaxHP.ShiftHeapPos_CheckCrc();
	m_Property.fMaxAP.ShiftHeapPos_CheckCrc();

	m_Status.ShiftHeapPos_CheckCrc();
	m_MInitialInfo.ShiftHeapPos_CheckCrc();

	// ÀÌ¸§À» Ã£¾Æ ±ÙÃ³ÀÇ ´Ù¸¥ ¸â¹öº¯¼öµéÀ» Ã£´Â °ÍÀ» ¹æÁE
	m_pMUserAndClanName->ShiftHeapPos_CheckCrc();

	m_killInfo.ShiftHeapPos_CheckCrc();
	m_damageInfo.ShiftHeapPos_CheckCrc();
	m_slotInfo.ShiftHeapPos_CheckCrc();

	// °ÔÀÓ ½ÃÀÛ½Ã ÀÌ ±¸Á¶Ã¼´Â 00 00 00 00 00 64 00 00 00 00 00 ÀÌ¶ó´Â ¸Å¿E°Ë»öÀÌ ½¬¿EÆÐÅÏÀ» °¡Áö°EÀÖ´Ù
	m_Status.ShiftHeapPos_CheckCrc();

	m_pMdwInvincibleStartTime->ShiftHeapPos_CheckCrc();
	m_pMdwInvincibleDuration->ShiftHeapPos_CheckCrc();

	m_dwStatusBitPackingValue.ShiftHeapPos();

	m_bCharged->ShiftHeapPos_CheckCrc();
	m_bCharging->ShiftHeapPos_CheckCrc();

	m_fChargedFreeTime.ShiftHeapPos_CheckCrc();
	m_nWallJumpDir.ShiftHeapPos_CheckCrc();
	m_nBlastType.ShiftHeapPos_CheckCrc();

	m_RealPositionBefore.ShiftHeapPos_CheckCrc();
	m_AnimationPositionDiff.ShiftHeapPos_CheckCrc();
	m_Accel.ShiftHeapPos_CheckCrc();

	m_AniState_Upper.ShiftHeapPos_CheckCrc();
	m_AniState_Lower.ShiftHeapPos_CheckCrc();

	m_nVMID.ShiftHeapPos_CheckCrc();
	m_nTeamID.ShiftHeapPos_CheckCrc();

	m_nMoveMode.ShiftHeapPos_CheckCrc();
	m_nMode.ShiftHeapPos_CheckCrc();
	m_nState.ShiftHeapPos_CheckCrc();

	m_fAttack1Ratio.ShiftHeapPos_CheckCrc();
	m_slotInfo.ShiftHeapPos_CheckCrc();
}

void ZCharacter::ApplyBuffEffect()
{

}
void ZCharacter::SpyHealthBonus(int nHPAP)
{
	const MTD_CharInfo* pCharInfo = &m_MInitialInfo.Ref();

	float fAddedAP = 0;			///< AP ÀçÁ¶Á¤ // DEFAULT_CHAR_AP = 0
	for (int i = 0; i < MMCIP_END; i++) {
		if (!m_Items.GetItem(MMatchCharItemParts(i))->IsEmpty()) {
			if (m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nAP.Ref() > 40) {
				m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nAP.Ref() = 0;
			}
			fAddedAP += m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nAP.Ref();
		}
	}

	float fAddedHP = 100;			///< HP ÀçÁ¶Á¤ DEFAULT_CHAR_HP
	for (int i = 0; i < MMCIP_END; i++) {
		if (!m_Items.GetItem(MMatchCharItemParts(i))->IsEmpty()) {
			fAddedHP += m_Items.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nHP.Ref();
		}
	}

	m_Property.fMaxAP.Set_CheckCrc(pCharInfo->nAP + fAddedAP + (float)nHPAP);
	m_Property.fMaxHP.Set_CheckCrc(pCharInfo->nHP + fAddedHP + (float)nHPAP);


	m_fPreMaxHP = pCharInfo->nHP + fAddedHP;
	m_fPreMaxAP = pCharInfo->nAP + fAddedAP;


	InitHPAP();
}

void ZCharacter::InitSpyWeaponBullet()
{
	for (int i = MMCIP_MELEE; i <= MMCIP_CUSTOM2; i++)
	{
		ZItem* pItem = m_Items.GetItem((MMatchCharItemParts)i);
		if (pItem->IsEmpty()) continue;

		// spy item ID check.
		if (MMatchSpyMode::IsSpyItem((int)pItem->GetDescID()))
		{
			pItem->SetBulletCurrMagazine(pItem->GetItemCount());
			pItem->SetBulletSpare(pItem->GetBulletCurrMagazine());
		}
	}
}

void ZCharacter::DistributeSpyItem(vector<MMatchSpyItem>& vt)
{
	if (GetTeamID() == MMT_RED)
	{
		m_Items.EquipItem(MMCIP_MELEE, 0, 0);
		m_Items.EquipItem(MMCIP_PRIMARY, 0, 0);
		m_Items.EquipItem(MMCIP_SECONDARY, 0, 0);
		m_Items.EquipItem(MMCIP_CUSTOM1, 0, 0);
		m_Items.EquipItem(MMCIP_CUSTOM2, 0, 0);
	}
	else
	{
		m_Items.EquipItem(MMCIP_CUSTOM1, 0, 0);
		m_Items.EquipItem(MMCIP_CUSTOM2, 0, 0);
	}

	for (vector<MMatchSpyItem>::iterator i = vt.begin(); i != vt.end(); i++)
	{
		MMatchSpyItem* pItem = &(*i);

		MMatchCharItemParts nParts = MMCIP_END;
		switch (pItem->nItemID)
		{
		case 601001:	// flashbang
			nParts = MMCIP_SECONDARY;	break;
		case 601002:	// smoke
			nParts = MMCIP_CUSTOM2;		break;
		case 601003:	// frozen field
			nParts = MMCIP_CUSTOM1;		break;
		case 601004:	// stun grenade
			nParts = MMCIP_CUSTOM1;		break;
		case 601007:	// landmine
			nParts = MMCIP_CUSTOM2;		break;
		case 601006:	// spycase
			nParts = MMCIP_MELEE;		break;
		default:
			_ASSERT(0);					break;
		}

#ifdef _DYNAMIC
		m_Items.EquipItem(nParts, pItem->nItemID, pItem->nItemCount, MMS_END /*false*/);
#endif
	}

	InitSpyWeaponBullet();

	ChangeWeapon(MMCIP_MELEE, true);
}

void ZCharacter::TakeoutSpyItem()
{
	const MTD_CharInfo* pCharInfo = &m_MInitialInfo.Ref();

	for (int i = 0; i < MMCIP_END; i++)
	{
		m_Items.EquipItem(MMatchCharItemParts(i), pCharInfo->nEquipedItemDesc[i], pCharInfo->nEquipedItemCount[i]);
	}

	InitItemBullet();

	ChangeWeapon(MMCIP_MELEE, true);
}
int gaepanEncode(int a, int depth)
{
	if (depth > 0)
	{
		int t = a ^ 0xa56b21;
		t += 516912;
		t = gaepanEncode(t, depth-1);
		return t;
	}
	return a;
}

int gaepanDecode(int a, int depth)
{
	if (depth > 0)
	{
		int t = gaepanDecode(a, depth-1);
		t -= 516912;
		t ^= 0xa56b21;
		return t;
	}
	return a;
}

