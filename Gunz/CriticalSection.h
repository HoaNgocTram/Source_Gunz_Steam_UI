#ifndef _CRITICALSECTION_H
#define _CRITICALSECTION_H

#pragma once

#include <windows.h>

class CriticalSection
{
public:
	CriticalSection();
	~CriticalSection();

	void Enter();
	void Exit();
private:
	CRITICAL_SECTION _cs;
};
#endif // !_CRITICALSECTION_H
