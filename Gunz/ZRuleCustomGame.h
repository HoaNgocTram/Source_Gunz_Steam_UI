#ifndef _ZRULE_CUSTOMGAME_H
#define _ZRULE_CUSTOMGAME_H

#include "ZRule.h"
#include "ZRuleDeathMatch.h"

//Custom: Rule Custom Game

class ZRuleVampire : public ZRuleSoloDeathMatch
{
public:
	ZRuleVampire(ZMatch* pMatch);
	virtual ~ZRuleVampire();
};

class ZRuleSoloClassic : public ZRuleSoloDeathMatch
{
public:
	ZRuleSoloClassic(ZMatch* pMatch);
	virtual ~ZRuleSoloClassic();
};


class ZRuleTeamClassic : public ZRuleTeamDeathMatch
{
public:
	ZRuleTeamClassic(ZMatch* pMatch);
	virtual ~ZRuleTeamClassic();
};

class ZRuleModeStaff : public ZRule
{
public:
	ZRuleModeStaff(ZMatch* pMatch);
	virtual ~ZRuleModeStaff();
};


#endif