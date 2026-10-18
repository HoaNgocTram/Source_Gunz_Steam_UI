#ifndef _ZANTIHACK_H
#define _ZANTIHACK_H

#include <string>
#include <map>
#include "MMatchGlobal.h"
using namespace std;

class ZAntiHackInfoNode
{
public:
	unsigned long time;
	MUID Attacker;
	unsigned int itemid;
	ZAntiHackInfoNode() {
		time			= 0;
		Attacker		= MUID(0,0);
		itemid			= 0;
	}
};
typedef	map<int, ZAntiHackInfoNode*> ZAntiHackMap;
typedef	map<MUID, int> Blocked;

class ZAntiHack 
{
public:
	ZAntiHack(void);
	~ZAntiHack(void);
	bool CheckBlock(MUID Attacker);
	void AddMassive(MUID Attacker);
	void AddFlip(MUID Attacker);
	void AddMelee(int itemid, MUID Attacker);
	void AddShot(int itemid, MUID Attacker);
private:
	void CheckShot(unsigned long time, MUID Attacker);
	void CheckFlip(unsigned long time, MUID Attacker);
	void CheckMassive(unsigned long time, MUID Attacker);
	void CheckMelee(unsigned long time, MUID Attacker);
protected:
	int nMeeleCount, nMassiveCount, nShotCount, nFlipCount;
	ZAntiHackMap m_LastMeeleAttack;
	ZAntiHackMap m_LastMassiveAttack;
	ZAntiHackMap m_LastShotAttack;
	ZAntiHackMap m_LastFlipAttack;
	Blocked m_Blocked;
};
#endif