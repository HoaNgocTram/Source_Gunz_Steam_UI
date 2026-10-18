#pragma once
#include <discord_register.h>
#include <discord_rpc.h>
#include <Windows.h>
class ZDiscord
{
public:
	void OnRunDiscord();
	void OnUpdateDiscord();
	bool SetStateDiscord(GunzState nState);
protected:
	GunzState			m_nStateDiscord;

};
