#include "stdafx.h"
#include "MActorDef.h"


MActorDefManager::~MActorDefManager()
{
	Clear();
}

bool MActorDefManager::ReadXml(MZFileSystem* pFileSystem, const char* szFileName)
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

bool MActorDefManager::ParseXmlFromStr(const char* szXmlContent)
{
	Clear();

	if (!szXmlContent) return NULL;

	MXmlDocument xmlDocument;
	xmlDocument.Create();

	if (!xmlDocument.LoadFromMemory(szXmlContent))
	{
		xmlDocument.Destroy();
		return NULL;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];
	char szTemp[256];
	int nTemp;
	float fTemp;

	rootElement = xmlDocument.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	bool bResult = true;

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!stricmp(szTagName, "ACTOR"))
		{
			MActorDef* pActorDef = new MActorDef;

			chrElement.GetAttribute(szTemp, "name");
			pActorDef->SetName(szTemp);
			chrElement.GetAttribute(szTemp, "ai.fsm");
			pActorDef->SetFsmName(szTemp);
			chrElement.GetAttribute(szTemp, "model");
			pActorDef->SetModelName(szTemp);

			chrElement.GetAttribute(&nTemp, "max_hp");
			pActorDef->SetMaxHp(nTemp);
			chrElement.GetAttribute(&nTemp, "max_ap");
			pActorDef->SetMaxAp(nTemp);

			chrElement.GetAttribute(&fTemp, "collision.radius");
			pActorDef->SetCollisionRadius(fTemp);
			chrElement.GetAttribute(&fTemp, "collision.height");
			pActorDef->SetCollisionHeight(fTemp);

			chrElement.GetAttribute(&fTemp, "scale", 1.0f);
			pActorDef->SetScale(fTemp);

			chrElement.GetAttribute(&fTemp, "speed");
			pActorDef->SetSpeed(fTemp);
			chrElement.GetAttribute(&fTemp, "rotspeed");
			pActorDef->SetRotSpeed(fTemp);
			chrElement.GetAttribute(&fTemp, "groggyRecoverPerSec");
			pActorDef->SetGroggyRecoverySpeed(fTemp);

			chrElement.GetAttribute(szTemp, "neverblasted", "false");
			if (0 == stricmp(szTemp, "false"))
				pActorDef->SetNeverBlasted(false);
			else if (0 == stricmp(szTemp, "true"))
				pActorDef->SetNeverBlasted(true);
			else {
				mlog("ERROR: xml actor node has invalid value for neverblasted attr.\n");
				_ASSERT(0);
				bResult = false;
			}

			chrElement.GetAttribute(szTemp, "boss", "false");
			if (0 == stricmp(szTemp, "true"))
				pActorDef->SetBoss(true);
			else
				pActorDef->SetBoss(false);


			if (chrElement.GetAttribute(szTemp, "neverblasted.sound", ""))
			{
				pActorDef->SetNeverBlastedSound(szTemp);
			}

			chrElement.GetAttribute(szTemp, "forced.collup120", "false");
			if (0 == _stricmp(szTemp, "false"))
				pActorDef->SetCollup120(false);
			else if (0 == _stricmp(szTemp, "true"))
				pActorDef->SetCollup120(true);

			chrElement.GetAttribute(szTemp, "meshpicking", "false");
			if (0 == _stricmp(szTemp, "false"))
				pActorDef->SetMeshPicking(false);
			else if (0 == _stricmp(szTemp, "true"))
				pActorDef->SetMeshPicking(true);

			if (chrElement.GetAttribute(szTemp, "noshadow", "true"))
			{
				if (0 == _stricmp(szTemp, "true"))
					pActorDef->SetNoShadow(true);
				else
					pActorDef->SetNoShadow(false);
			}
			else
			{
				pActorDef->SetNoShadow(false);
			}
			if (chrElement.GetAttribute(szTemp, "type", ""))
			{
				pActorDef->SetActorType(szTemp);
			}
			// 에러 체크
			if (0 == strcmp(pActorDef->GetName(), "")) {
				mlog("ERROR: xml actor node has no name attr.\n"); _ASSERT(0); bResult = false;
			}
			if (0 == strcmp(pActorDef->GetFsmName(), "")) {
				mlog("ERROR: xml actor node has no fsm name attr.\n"); _ASSERT(0); bResult = false;
			}
			if (0 == strcmp(pActorDef->GetModelName(), "")) {
				mlog("ERROR: xml actor node has no name attr.\n"); _ASSERT(0); bResult = false;
			}
			if (0 == pActorDef->GetMaxHp()) {
				mlog("ERROR: xml actor node has no max_hp attr.\n"); _ASSERT(0); bResult = false;
			}
			if (0 == pActorDef->GetCollisionRadius()) {
				mlog("ERROR: xml actor node has no collision.radius attr.\n"); _ASSERT(0); bResult = false;
			}
			if (0 == pActorDef->GetCollisionHeight()) {
				mlog("ERROR: xml actor node has no collision.height attr.\n"); _ASSERT(0); bResult = false;
			}
			if (0 == pActorDef->GetSpeed()) {
				mlog("ERROR: xml actor node has no speed attr.\n"); _ASSERT(0); bResult = false;
			}
			if (0 == pActorDef->GetRotSpeed()) {
				mlog("ERROR: xml actor node has no rotspeed attr.\n"); _ASSERT(0); bResult = false;
			}

			if (!bResult)
			{
				delete pActorDef;
				break;
			}

			// 추가
			if (!AddActorDef(pActorDef))
			{
				mlog("ERROR: xml actor node name duplicated.");
				delete pActorDef;
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

MActorDef* MActorDefManager::GetDef(const char* szActorName)
{
	ItorActorDef it = m_mapActorDef.find(szActorName);
	if (it != m_mapActorDef.end())
		return it->second;
	return NULL;
}

MActorDef* MActorDefManager::GetDefIndex(int index)
{
	ItorActorDef it = m_mapActorDef.begin();
	int i = 0;
	for (it = m_mapActorDef.begin(); it != m_mapActorDef.end(); it++)
	{
		i++;
		if (index == i)
			return (*it).second;
	}
	return NULL;
}

bool MActorDefManager::AddActorDef(MActorDef* pDef)
{
	// 이미 존재하는 이름이면 실패
	if (GetDef(pDef->GetName()))
		return false;

	m_mapActorDef[pDef->GetName()] = pDef;
	return true;
}

void MActorDefManager::Clear()
{
	for (ItorActorDef it = m_mapActorDef.begin(); it != m_mapActorDef.end(); ++it)
		delete it->second;
	m_mapActorDef.clear();
}