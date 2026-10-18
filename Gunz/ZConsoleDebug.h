#pragma once
#ifdef _DEVMODE
class ZConsoleDebug
{
public:

	void DebugConsole();
	void LogConsole(char* input);
    #define zMyBuildVersion "1"
	bool OnConsole = false;
};
#endif