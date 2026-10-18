#include "stdafx.h"
#include "CriticalSection.h"

CriticalSection::CriticalSection()
{
	InitializeCriticalSection(&_cs);
}

void CriticalSection::Enter()
{
	EnterCriticalSection(&_cs);
}

void CriticalSection::Exit()
{
	LeaveCriticalSection(&_cs);
}

CriticalSection::~CriticalSection()
{
	DeleteCriticalSection(&_cs);
}