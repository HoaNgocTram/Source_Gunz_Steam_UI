#include "stdafx.h"
#include "ZAntiHack.h"
#include "ZGameClient.h"

ZAntiHack::ZAntiHack(void)
{
	nMeeleCount = 0;
	nMassiveCount = 0;
	nShotCount = 0;
	nFlipCount = 0;
}

ZAntiHack::~ZAntiHack(void)
{
	m_LastShotAttack.clear(); 
	m_LastMeeleAttack.clear(); 
	m_LastMassiveAttack.clear(); 
	m_LastFlipAttack.clear();
	m_Blocked.clear(); 
}


bool ZAntiHack::CheckBlock(MUID Attacker) 
{
	if(m_Blocked.count(Attacker) == 1)
	{
		Blocked::iterator itr = m_Blocked.find(Attacker);
		if(itr->second >= 2)
			return true;
		else
			return false;
	}
	return false;
}

void ZAntiHack::AddMassive(MUID Attacker) 
{
	if(m_LastMassiveAttack.size() >=6) { m_LastMassiveAttack.clear(); nMassiveCount = 0; }
	ZAntiHackInfoNode* info = new ZAntiHackInfoNode;
	info->Attacker = Attacker;
	info->time = timeGetTime();
	CheckMassive(info->time, Attacker);
	m_LastMassiveAttack.insert(ZAntiHackMap::value_type(nMassiveCount++, info));
}

void ZAntiHack::AddFlip(MUID Attacker) 
{
	if(m_LastFlipAttack.size() >=6) {  m_LastFlipAttack.clear(); nFlipCount = 0; }
	ZAntiHackInfoNode* info = new ZAntiHackInfoNode;
	info->Attacker = Attacker;
	info->time = timeGetTime();
	CheckFlip(info->time, Attacker);
	m_LastFlipAttack.insert(ZAntiHackMap::value_type(nFlipCount++, info));
}

void ZAntiHack::AddMelee(int itemid, MUID Attacker)  
{
	if(m_LastMeeleAttack.size() >= 8) { m_LastMeeleAttack.clear(); nMeeleCount = 0; }
	ZAntiHackInfoNode* info = new ZAntiHackInfoNode;
	info->Attacker = Attacker;
	info->time = timeGetTime();
	info->itemid = itemid;
	CheckMelee(info->time, Attacker);
	m_LastMeeleAttack.insert(ZAntiHackMap::value_type(nMeeleCount++, info));
}

void ZAntiHack::AddShot(int itemid, MUID Attacker) 
{
	if(m_LastShotAttack.size() >=16) { m_LastShotAttack.clear(); nShotCount = 0;}
	ZAntiHackInfoNode* info = new ZAntiHackInfoNode;
	info->Attacker = Attacker;
	info->time = timeGetTime();
	info->itemid = itemid;
	CheckShot(info->time, Attacker);
	m_LastShotAttack.insert(ZAntiHackMap::value_type(nShotCount++, info));
}

void ZAntiHack::CheckShot(unsigned long time, MUID Attacker)
{
	ZAntiHackMap::reverse_iterator it;
	for ( it=m_LastShotAttack.rbegin() ; it != m_LastShotAttack.rend(); it++ )
	{
		ZAntiHackInfoNode* info = (*it).second;
		if(info && info->Attacker == Attacker)
		{
			MMatchItemDesc* pItem = MGetMatchItemDescMgr()->GetItemDesc(info->itemid);
			if(pItem && (time - info->time) <= pItem->m_nDelay.Ref())
			{
				if(m_Blocked.count(Attacker) == 1)
				{
					Blocked::iterator itr = m_Blocked.find(Attacker);
					itr->second++;
					break;
				} else {
					m_Blocked.insert(Blocked::value_type(Attacker, 0));
					break;
				}
			}
		}
	}
}

void ZAntiHack::CheckFlip(unsigned long time, MUID Attacker)
{
	ZAntiHackMap::reverse_iterator it;
	for ( it=m_LastFlipAttack.rbegin() ; it != m_LastFlipAttack.rend(); it++ )
	{
		ZAntiHackInfoNode* info = (*it).second;
		if(info && info->Attacker == Attacker)
		{
			if((time - info->time) <= 350)
			{
				if(m_Blocked.count(Attacker) == 1)
				{
					Blocked::iterator itr = m_Blocked.find(Attacker);
					itr->second++;
					break;
				} else {
					m_Blocked.insert(Blocked::value_type(Attacker, 0));
					break;
				}
			}
		}
	}
}

void ZAntiHack::CheckMassive(unsigned long time, MUID Attacker)
{
	ZAntiHackMap::reverse_iterator it;
	for ( it=m_LastMassiveAttack.rbegin() ; it != m_LastMassiveAttack.rend(); it++ )
	{
		ZAntiHackInfoNode* info = (*it).second;
		if(info && info->Attacker == Attacker)
		{
			if((time - info->time) <= 600)
			{
				if(m_Blocked.count(Attacker) == 1)
				{
					Blocked::iterator itr = m_Blocked.find(Attacker);
					itr->second++;
					break;
				} else {
					m_Blocked.insert(Blocked::value_type(Attacker, 0));
					break;
				}
			}
		}
	}
}

void ZAntiHack::CheckMelee(unsigned long time, MUID Attacker)
{
	ZAntiHackMap::reverse_iterator it;
	for ( it=m_LastMeeleAttack.rbegin() ; it != m_LastMeeleAttack.rend(); it++ )
	{
		ZAntiHackInfoNode* info = (*it).second;
		if(info && info->Attacker == Attacker)
		{
			MMatchItemDesc* pItem = MGetMatchItemDescMgr()->GetItemDesc(info->itemid);
			if(pItem && (time - info->time) <= (pItem->m_nDelay.Ref()-50))
			{
				if(m_Blocked.count(Attacker) == 1)
				{
					Blocked::iterator itr = m_Blocked.find(Attacker);
					itr->second++;
					break;
				} else {
					m_Blocked.insert(Blocked::value_type(Attacker, 0));
					break;
				}
			}
		}
	}
}