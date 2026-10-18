#include "stdafx.h"
#include "ZActorActionManager.h"
#include "ZActorAction.h"


ZActorActionManager::~ZActorActionManager()
{
	Clear();
}

bool ZActorActionManager::ReadXml(MZFileSystem* pFileSystem, const char* szFileName)
{
	MXmlDocument xmlIniData;
	xmlIniData.Create();

	char* buffer;
	MZFile mzf;

	if (pFileSystem) {
		if (!mzf.Open(szFileName, pFileSystem)) {
			if (!mzf.Open(szFileName)) {
				xmlIniData.Destroy();
				return false;
			}
		}
	}
	else {

		if (!mzf.Open(szFileName)) {

			xmlIniData.Destroy();
			return false;
		}
	}

	buffer = new char[mzf.GetLength() + 1];
	buffer[mzf.GetLength()] = 0;

	mzf.Read(buffer, mzf.GetLength());

	bool bResult = ParseXmlFromStr(buffer);

	delete[] buffer;
	mzf.Close();

	return bResult;
}

bool ZActorActionManager::ParseXmlFromStr(const char* szXmlContent)
{
	Clear();

	if (!szXmlContent) return NULL;

	MXmlDocument xmlDocument;
	xmlDocument.Create();

	if (!xmlDocument.LoadFromMemory((char*)szXmlContent))
	{
		xmlDocument.Destroy();
		return NULL;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlDocument.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	bool bResult = true;

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, "ACTION"))
		{
			ZActorAction* pAction = new ZActorAction;

			if (!ParseXml_Action(pAction, chrElement))
			{
				bResult = false;
				delete pAction;
				break;
			}

			if (!AddAction(pAction))
			{
				mlog("ERROR: xml action node name duplicated.\n");
				delete pAction;
				bResult = false;
				break;
			}
		}
	}

	xmlDocument.Destroy();

	// 중간에 파싱이 실패하면 읽었던 내용을 다 버린다
	if (!bResult)
		Clear();

	return bResult;
}

bool ZActorActionManager::ParseXml_Action(ZActorAction* pOutAction, MXmlElement& elem)
{
	char szTagName[256];
	char szTemp[256];

	elem.GetAttribute(szTemp, "name");
	if (0 == strcmp(szTemp, ""))
	{
		mlog("ERROR: xml action node has no name attr.\n");
		return false;
	}
	pOutAction->SetName(szTemp);

	elem.GetAttribute(szTemp, "animation");
	pOutAction->SetAnimationName(szTemp);

	elem.GetAttribute(szTemp, "movinganimation");
	if (0 == _stricmp("true", szTemp))
		pOutAction->SetMovingAnimation(true);

	MXmlElement chrElement;
	int iCount = elem.GetChildNodeCount();
	for (int i = 0; i < iCount; i++)
	{
		chrElement = elem.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, "MELEESHOT"))
		{
			ZActorActionMeleeShot* pMeleeShot = new ZActorActionMeleeShot;
			if (!ParseXml_ActionMeleeShot(pMeleeShot, chrElement)) {
				delete pMeleeShot;
				return false;
			}
			pOutAction->SetMeleeShot(pMeleeShot);
		}
		else if (!_stricmp(szTagName, "RANGESHOT"))
		{
			ZActorActionRangeShot* pRangeShot = new ZActorActionRangeShot;
			if (!ParseXml_ActionRangeShot(pRangeShot, chrElement)) {
				delete pRangeShot;
				return false;
			}
			pOutAction->AddRangeShot(pRangeShot);
		}
		else if (!_stricmp(szTagName, "GRENADESHOT"))
		{
			ZActorActionGrenadeShot* pGrenadeShot = new ZActorActionGrenadeShot;
			if (!ParseXml_ActionGrenadeShot(pGrenadeShot, chrElement)) {

				delete pGrenadeShot;
				return false;
			}
			pOutAction->AddGrenadeShot(pGrenadeShot);
		}
		else if (!_stricmp(szTagName, "SOUND"))
		{
			ZActorActionSound* pSound = new ZActorActionSound;
			if (!ParseXml_ActionSound(pSound, chrElement)) {

				delete pSound;
				return false;
			}
			pOutAction->AddSound(pSound);
		}
		else if (!_stricmp(szTagName, "EFFECT"))
		{
			ZActorActionEffect* pEffect = new ZActorActionEffect;
			if (!ParseXml_ActionEffect(pEffect, chrElement)) {

				delete pEffect;
				return false;
			}
			pOutAction->AddEffect(pEffect);
		}
		else if (!_stricmp(szTagName, "SUMMON"))
		{
			ZActorActionSummon* pSummon = new ZActorActionSummon;
			if (!ParseXml_ActionSummon(pSummon, chrElement)) {

				delete pSummon;
				return false;
			}
			pOutAction->AddSummon(pSummon);
		}
	}
	return true;
}

bool ZActorActionManager::ParseXml_ActionShotCommonAttributes(ZActorActionShotCommon* pOutShot, MXmlElement& elem)
{
	float fDamage;
	int nDelay, nPierce, nKnockback;
	string strSound;
	string strTemp;

	elem.GetAttribute(&nDelay, "delay", 0);

	if (!elem.GetAttribute(&fDamage, "damage"))
	{
		mlog("ERROR: xml action shot node has no damage attr.\n");
		return false;
	}

	elem.GetAttribute(&nPierce, "pierce", 50);
	if (!(0 <= nPierce && nPierce <= 100))
	{
		mlog("ERROR: xml action shot node has invalid pierce attr.\n");
		return false;
	}

	elem.GetAttribute(&nKnockback, "knockback", 0);
	if (nKnockback < 0)
	{
		mlog("ERROR: xml action shot node has invalid knockback attr.\n");
		return false;
	}

	elem.GetAttribute(&strSound, "sound", "");


	elem.GetAttribute(&strTemp, "uppercut", "");
	if (0 == _stricmp(strTemp.c_str(), "true"))
		pOutShot->SetUpperCut(true);
	else
		pOutShot->SetUpperCut(false);

	elem.GetAttribute(&strTemp, "thrust", "");
	if (0 == _stricmp(strTemp.c_str(), "true"))
		pOutShot->SetThrust(true);
	else
		pOutShot->SetThrust(false);

	pOutShot->SetDelay(nDelay * 0.001f);
	pOutShot->SetDamage(fDamage);
	pOutShot->SetPierce(nPierce * 0.01f);
	pOutShot->SetKnockbackForce(nKnockback);
	pOutShot->SetSound(strSound.c_str());

	return true;
}

bool ZActorActionManager::ParseXml_ActionMeleeShot(ZActorActionMeleeShot* pOutMeleeShot, MXmlElement& elem)
{
	if (!ParseXml_ActionShotCommonAttributes(pOutMeleeShot, elem))
		return false;

	float fAngle, fRange;

	if (!elem.GetAttribute(&fAngle, "angle"))
	{
		mlog("ERROR: xml action melee shot node has no angle attr.\n");
		return false;
	}
	if (!elem.GetAttribute(&fRange, "range"))
	{
		mlog("ERROR: xml action melee shot node has no range attr.\n");
		return false;
	}

	pOutMeleeShot->SetRange(fRange);
	pOutMeleeShot->SetAngle(fAngle);

	return true;
}

bool ZActorActionManager::ParseXml_ActionRangeShot(ZActorActionRangeShot* pOutRangeShot, MXmlElement& elem)
{
	if (!ParseXml_ActionShotCommonAttributes(pOutRangeShot, elem))
		return false;

	string strMeshName, strDirMod, strPosParts, strDirTarget;
	float fSpeed, fCollRadius;
	int nZAxis, nYAxis;
	rvector vDirMod;
	_RMeshPartsPosInfoType posPartsType = eq_parts_pos_info_end;

	elem.GetAttribute(&strMeshName, "mesh", "");

	if (!elem.GetAttribute(&fSpeed, "speed"))
	{
		mlog("ERROR: xml action range shot node has no speed attr.\n");
		return false;
	}

	elem.GetAttribute(&fCollRadius, "collradius");
	/*todok del collradius 이제는 선택적으로 할 수 있다
	if (!elem.GetAttribute(&fCollRadius, "collradius"))
	{
		mlog("ERROR: xml action range shot node has no collradius attr.\n");
		return false;
	}*/

	elem.GetAttribute(&strDirMod, "dirmod", "0 0 0");
	if (3 != sscanf(strDirMod.c_str(), "%f %f %f", &vDirMod.x, &vDirMod.y, &vDirMod.z))
	{
		mlog("ERROR: xml action range shot node has invalid dirmod attr.\n");
		return false;
	}

	if (elem.GetAttribute(&strPosParts, "posparts"))
	{
		if (0 == _stricmp(strPosParts.c_str(), "head"))			posPartsType = eq_parts_pos_info_Head;
		else if (0 == _stricmp(strPosParts.c_str(), "lhand"))		posPartsType = eq_parts_pos_info_LHand;
		else if (0 == _stricmp(strPosParts.c_str(), "rhand"))		posPartsType = eq_parts_pos_info_RHand;
		else
		{
			mlog("ERROR: xml action range shot node has invalid posparts attr.\n");
			return false;
		}
	}

	if (elem.GetAttribute(&nZAxis, "zaxis", 0))
	{
		pOutRangeShot->SetZAxis(nZAxis);
	}

	if (elem.GetAttribute(&nYAxis, "yaxis", 0))
	{
		pOutRangeShot->SetYAxis(nYAxis);
	}

	if (elem.GetAttribute(&strDirTarget, "dirTarget"))
	{
		if (0 == _stricmp(strDirTarget.c_str(), "true"))
			pOutRangeShot->SetDirTarget(true);
		else
			pOutRangeShot->SetDirTarget(false);
	}

	pOutRangeShot->SetMeshName(strMeshName.c_str());
	pOutRangeShot->SetSpeed(fSpeed);
	pOutRangeShot->SetCollRadius(fCollRadius);
	pOutRangeShot->SetDirectionMod(vDirMod);
	pOutRangeShot->SetPosPartsType(posPartsType);

	return true;
}

bool ZActorActionManager::ParseXml_ActionGrenadeShot(ZActorActionGrenadeShot* pOutGrenadeShot, MXmlElement& elem)
{
	float fDamage;
	int nDelay, nPierce, nGrenadetype, nItemID, nZaxis, nYaxis, nForce;
	string strPosParts, strPosMod, strDirMod, strSound;
	_RMeshPartsPosInfoType posPartsType = eq_parts_pos_info_end;
	rvector vPosMod, vDirMod;

	if (!elem.GetAttribute(&nDelay, "delay", 2000))
		mlog("ERROR: xml action grenade node has no delay attr.\n");

	if (!elem.GetAttribute(&fDamage, "damage", 0))
		mlog("ERROR: xml action grenade node has no damage attr.\n");

	if (!elem.GetAttribute(&nPierce, "pierce", 0))
		mlog("ERROR: xml action grenade node has no damage attr.\n");

	if (!elem.GetAttribute(&nGrenadetype, "grenadetype", 0))
		mlog("ERROR: xml action grenade node has no grenadetype attr.\n");

	if (!elem.GetAttribute(&nItemID, "itemid"))
	{
		mlog("ERROR: xml action grenade node has no itemid attr.\n");
		return false;
	}

	if (!elem.GetAttribute(&nZaxis, "zaxis", 0))
		mlog("ERROR: xml action grenade node has no zaxis attr.\n");

	if (!elem.GetAttribute(&nYaxis, "yaxis", 0))
		mlog("ERROR: xml action grenade node has no yaxis attr.\n");

	if (!elem.GetAttribute(&nForce, "force", 0))
		mlog("ERROR: xml action grenade node has no force attr.\n");

	if (elem.GetAttribute(&strPosParts, "posparts"))
	{
		if (0 == _stricmp(strPosParts.c_str(), "head"))			posPartsType = eq_parts_pos_info_Head;
		else if (0 == _stricmp(strPosParts.c_str(), "lhand"))		posPartsType = eq_parts_pos_info_LHand;
		else if (0 == _stricmp(strPosParts.c_str(), "rhand"))		posPartsType = eq_parts_pos_info_RHand;
		else if (0 == _stricmp(strPosParts.c_str(), ""))           posPartsType = eq_parts_pos_info_end; //no clue... fix later
	}
	if (elem.GetAttribute(&strPosMod, "posmod", "0 0 0"))
	{
		if (3 != sscanf(strPosMod.c_str(), "%f %f %f", &vPosMod.x, &vPosMod.y, &vPosMod.z))
		{
			mlog("ERROR: xml action grenade shot node has invalid posmod attr.\n");
			return false;
		}
	}
	if (elem.GetAttribute(&strDirMod, "dirmod", "0 0 0"))
	{
		if (3 != sscanf(strDirMod.c_str(), "%f %f %f", &vDirMod.x, &vDirMod.y, &vDirMod.z))
		{
			mlog("ERROR: xml action grenade shot node has invalid dirmod attr.\n");
			return false;
		}
	}
	elem.GetAttribute(&strSound, "sound", "");


	pOutGrenadeShot->SetDelay(nDelay * 0.001f);
	pOutGrenadeShot->SetDamage(fDamage);
	pOutGrenadeShot->SetPierce(nPierce * 0.01f);
	pOutGrenadeShot->SetGrenadeType(nGrenadetype);
	pOutGrenadeShot->SetItemID(nItemID);
	pOutGrenadeShot->SetZAxis(nZaxis);
	pOutGrenadeShot->SetYAxis(nYaxis);
	pOutGrenadeShot->SetForce(nForce);
	pOutGrenadeShot->SetPosPartsInfo(posPartsType);
	pOutGrenadeShot->SetPosModPosition(vPosMod);
	pOutGrenadeShot->SetDirModDirection(vDirMod);
	pOutGrenadeShot->SetSound(strSound.c_str());

	return true;
}


bool ZActorActionManager::ParseXml_ActionEffect(ZActorActionEffect* pOutEffect, MXmlElement& elem)
{
	//if (!ParseXml_ActionShotCommonAttributes(pOutRangeShot, elem))
	//	return false;
	int nDelay;
	string strMeshName, strDirMod, strPosParts, strPosMod, strScale;
	rvector vDirMod, vPosMod, vScale;
	_RMeshPartsPosInfoType posPartsType = eq_parts_pos_info_end;
	int nZAxis;
	elem.GetAttribute(&nDelay, "delay", 0);
	elem.GetAttribute(&strMeshName, "mesh", "");
	elem.GetAttribute(&strScale, "scale", "0 0 0");
	if (1 != sscanf(strScale.c_str(), "%f", &vScale.x)) //Only 1 value stored in XML
	{
		mlog("ERROR: xml action Effect Scale\n");
		return false;
	}
	else
	{
		vScale.y = vScale.x;
		vScale.z = vScale.x;
	}
	elem.GetAttribute(&strDirMod, "dirmod", "0 0 0");
	if (3 != sscanf(strDirMod.c_str(), "%f %f %f", &vDirMod.x, &vDirMod.y, &vDirMod.z))
	{
		mlog("ERROR: xml action Effect Dirmod\n");
		return false;
	}
	elem.GetAttribute(&strPosMod, "posmod", "0 0 0");
	if (3 != sscanf(strPosMod.c_str(), "%f %f %f", &vPosMod.x, &vPosMod.y, &vPosMod.z))
	{
		mlog("ERROR: xml action Effect Posmod\n");
		return false;
	}
	if (elem.GetAttribute(&strPosParts, "posparts"))
	{
		if (0 == _stricmp(strPosParts.c_str(), "head"))			posPartsType = eq_parts_pos_info_Head;
		else if (0 == _stricmp(strPosParts.c_str(), "lhand"))		posPartsType = eq_parts_pos_info_LHand;
		else if (0 == _stricmp(strPosParts.c_str(), "rhand"))		posPartsType = eq_parts_pos_info_RHand;
		else if (0 == _stricmp(strPosParts.c_str(), ""))			posPartsType = eq_parts_pos_info_end; //CHECK THIS
		else
		{
			mlog("ERROR: xml action range shot node has invalid posparts attr.\n");
			return false;
		}
	}

	if (elem.GetAttribute(&nZAxis, "zaxis"))
	{
		pOutEffect->SetZAxis(nZAxis);
	}
	pOutEffect->SetMeshName(strMeshName.c_str());
	pOutEffect->SetDelay(nDelay * 0.001f);
	pOutEffect->SetDirectionMod(vDirMod);
	pOutEffect->SetPosMod(vPosMod);
	pOutEffect->SetPosPartsType(posPartsType);
	pOutEffect->SetScale(vScale);

	return true;
}

bool ZActorActionManager::ParseXml_ActionSound(ZActorActionSound* pOutEffect, MXmlElement& elem)
{
	int nDelay;
	string strSoundPath;

	elem.GetAttribute(&nDelay, "delay", 0);
	elem.GetAttribute(&strSoundPath, "sound", "");

	pOutEffect->SetSoundPath(strSoundPath.c_str());
	pOutEffect->SetDelay(nDelay * 0.001f);
	return true;
}

bool ZActorActionManager::ParseXml_ActionSummon(ZActorActionSummon* pOutEffect, MXmlElement& elem)
{
	string strSummonName;
	int nRoute, nDelay;
	float fRange, fAngle;
	char szDropTable[256];
	char szSummon[256];
	bool bAdjustPlayerNum;
	elem.GetAttribute(&strSummonName, "name", "");
	elem.GetAttribute(&nDelay, "delay", 0);
	elem.GetAttribute(&fRange, "range", 0);
	elem.GetAttribute(&fAngle, "angle", 0);

	if (elem.GetAttribute(szSummon, "adjustplayernum"))
	{
		if (0 == _stricmp("true", szSummon))
			bAdjustPlayerNum = true;
		else
			bAdjustPlayerNum = false;
	}
	if (elem.GetAttribute(&nRoute, "route", 0))
	{
		pOutEffect->SetRoute(nRoute);
	}
	if (elem.GetAttribute(szDropTable, "drop"))
	{
		pOutEffect->SetDropTableID(szDropTable);
	}

	pOutEffect->SetSummonName(strSummonName.c_str());
	pOutEffect->SetDelay(nDelay * 0.001f);
	pOutEffect->SetRange(fRange);
	pOutEffect->SetAngle(fAngle);
	pOutEffect->SetAdjustPlayerNum(bAdjustPlayerNum);
	return true;
}

bool ZActorActionManager::AddAction(ZActorAction* pAction)
{
	// 중복 이름이면 실패
	ItorAction it = m_mapAction.find(pAction->GetName());
	if (it != m_mapAction.end())
		return false;

	m_mapAction[pAction->GetName()] = pAction;
	return true;
}

void ZActorActionManager::Clear()
{
	for (ItorAction it = m_mapAction.begin(); it != m_mapAction.end(); ++it)
		delete it->second;
	m_mapAction.clear();
}

ZActorAction* ZActorActionManager::GetAction(const char* szName)
{
	ItorAction it = m_mapAction.find(szName);
	if (it != m_mapAction.end())
		return it->second;

	return NULL;
}

int ZActorActionManager::GetIndexOfAction(IActorAction* pAction)
{
	int idx = 0;
	for (ItorAction it = m_mapAction.begin(); it != m_mapAction.end(); ++it)
	{
		if (it->second == pAction)
			return idx;
		++idx;
	}
	return -1;
}

ZActorAction* ZActorActionManager::GetActionByIndex(int idx)
{
	int numAction = (int)m_mapAction.size();
	if (idx < 0 || numAction <= idx)
		return NULL;

	ItorAction it = m_mapAction.begin();
	for (int i = 0; i < idx; ++i)
		++it;

	return it->second;
}