#pragma once
#include "stdafx.h"
#include <string>

std::string UrlEncode(const std::string& value);
std::string MakeCharNameToken(const std::string& name);
const char* GetMyCharName();
void OpenGachaInWebview();
void OpenBattlePassInWebview();
void OpenMarketInWebview();
void OpenLadderRewardInWebview();
