#include "stdafx.h"
#include "ZRuleCustomGame.h"

//Custom: Rule Custom Game

//Vampire Mode
ZRuleVampire::ZRuleVampire(ZMatch* pMatch) : ZRuleSoloDeathMatch(pMatch){
}

ZRuleVampire::~ZRuleVampire(){
}

//Mode Staff
ZRuleModeStaff::ZRuleModeStaff(ZMatch* pMatch) : ZRule(pMatch){
}
ZRuleModeStaff::~ZRuleModeStaff(){
}

//Classic Modes
ZRuleSoloClassic::ZRuleSoloClassic(ZMatch* pMatch) : ZRuleSoloDeathMatch(pMatch){
}
ZRuleSoloClassic::~ZRuleSoloClassic(){
}
ZRuleTeamClassic::ZRuleTeamClassic(ZMatch* pMatch) : ZRuleTeamDeathMatch(pMatch){
}
ZRuleTeamClassic::~ZRuleTeamClassic(){
}