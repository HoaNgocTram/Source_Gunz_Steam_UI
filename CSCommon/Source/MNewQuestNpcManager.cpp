#include "stdafx.h"
#include "MNewQuestNpcManager.h"


void MNewQuestNpcManager::AddNpcObject(MUID uidNpc, MUID uidController, int nCustomSpawnIndex, int nSpawnIndex, bool bBoss, std::string actorType, int nDropItemId)
{
	if (uidNpc.IsInvalid() || uidController.IsInvalid()) { _ASSERT(0); return; }

	MNewQuestNpcObject* pNewNPC = new MNewQuestNpcObject(uidNpc);
	pNewNPC->AssignControl(uidController);
	pNewNPC->SetDropItemId(nDropItemId);
	pNewNPC->SetActorType(actorType);
	pNewNPC->SetIndex(nSpawnIndex);
	pNewNPC->SetCustomIndex(nCustomSpawnIndex);
	pNewNPC->SetBoss(bBoss);
	m_mapNpc[uidNpc] = pNewNPC;
}

void MNewQuestNpcManager::DeleteNpcObject(MUID uidNpc)
{
	if (uidNpc.IsInvalid()) { _ASSERT(0); return; }

	ItorNpc it = m_mapNpc.find(uidNpc);
	if (it == m_mapNpc.end())
		return;

	delete it->second;
	m_mapNpc.erase(it);
}

MNewQuestNpcObject* MNewQuestNpcManager::GetNpc( MUID uid )
{
	if (uid.IsInvalid()) { _ASSERT(0); return NULL; }

	ItorNpc it = m_mapNpc.find(uid);
	if (it == m_mapNpc.end())
		return NULL;

	return it->second;
}

void MNewQuestNpcManager::SetController(MUID uidNpc, MUID uidChar)
{
	ItorNpc it = m_mapNpc.find(uidNpc);
	if (it == m_mapNpc.end())
		return;

	it->second->AssignControl(uidChar);
}

MNewQuestNpcManager::vecNPCUid MNewQuestNpcManager::GetNPCByController(MUID uidChar)
{
	vecNPCUid newList;
	for(ItorNpc it = m_mapNpc.begin(); it != m_mapNpc.end(); ++it)
	{
		if(it->second->GetController() == uidChar)
		{
			newList.push_back(it->second->GetUID());
		}
	}
	return newList;
}