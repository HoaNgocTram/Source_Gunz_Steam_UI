#include "stdafx.h"
#include "ZConsoleDebug.h"
#include <TlHelp32.h>
#include <iostream>
#include <stdio.h>
#include "MDebug.h"
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#ifdef _DEVMODE
using namespace std;

void ZConsoleDebug::DebugConsole()
{
	if (OnConsole)
		return;

	char zsLogger[64];
	char zsDT[64];

	AllocConsole();

	freopen("CONOUT$", "w", stdout);
	sprintf(zsDT, "%s -%s", __DATE__, __TIME__);
	sprintf(zsLogger, "[%s] [GunZ Debug Console] ::Version :: %s ", zsDT, zMyBuildVersion);
	cout << zsLogger << endl;
	mlog(zsLogger);
}

void ZConsoleDebug::LogConsole(char* input)
{
	if (!OnConsole) 
		return;

	char zsTransDerm[64];
	sprintf(zsTransDerm, "[Debug] (" __DATE__" " __TIME__"):: %s\n",input);
	std::cout << zsTransDerm << std::endl;	
}
#endif