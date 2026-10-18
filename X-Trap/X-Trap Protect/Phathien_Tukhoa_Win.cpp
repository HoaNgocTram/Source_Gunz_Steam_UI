
#include "StdAfx.h"


void Msg_WIN_Br(){	
	MessageBoxA(NULL,"HW-Scan\n\nConteúdo suspeito detectado!", phathien.Ten_Cua_So, MB_SERVICE_NOTIFICATION | MB_ICONWARNING);
	ExitProcess(0);
}
void Msg_WIN_En(){	
	MessageBoxA(NULL,"HW-Scan\n\nAn illegal choice haas been detected!", phathien.Ten_Cua_So, MB_SERVICE_NOTIFICATION | MB_ICONWARNING);
	ExitProcess(0);
}
void Msg_WIN_Page(){
	Sleep (2000);
    ShellExecute(NULL, "open", phathien.HackSplash_WebSite, NULL, NULL, SW_SHOWNORMAL);
	}

BOOL CALLBACK heurprocess(HWND hwnd, LPARAM lParam){
    static TCHAR x[40];
    GetWindowText(hwnd, x, 40);

////////////////////////////////////////////////////////////////////////////////////////////////
//Heuristic Window name scan - HW-scan 
//Are Case-sensitive - Find it just looking window name or using [Handler 1.5 by Oliver Bock] 
//_tcsstr(x,"xxx") Just one word, without empty space
////////////////////////////////////////////////////////////////////////////////////////////////

if (_tcsstr(x,"[F?Ø???]")   || //Black box
    _tcsstr(x,"[BlackBox]")	    ||
	_tcsstr(x,"Task Manager") ||
	_tcsstr(x,"BYPASS")	    ||
	_tcsstr(x,"ByPass")		||
	_tcsstr(x,"byPass")		||
	_tcsstr(x,"Bypass")		||
	_tcsstr(x,"CAPOTE")	    ||
	_tcsstr(x,"Capote")	    ||
	_tcsstr(x,"capote")	    ||	
	_tcsstr(x,"CATASTROPHE")	||
	_tcsstr(x,"Catastrophe")	||
	_tcsstr(x,"catastrophe")	||
	_tcsstr(x,"DUPAR")		||
	_tcsstr(x,"Dupar")		||
	_tcsstr(x,"dupar")		||
	_tcsstr(x,"DUPER")		||
	_tcsstr(x,"Duper")		||
	_tcsstr(x,"duper")		||
	_tcsstr(x,"SPEED")      ||
	_tcsstr(x,"Speed")      ||
	_tcsstr(x,"Speed")      ||
	_tcsstr(x,"HIDE")		||
	_tcsstr(x,"Hide")		||
	_tcsstr(x,"hide")		||
	_tcsstr(x,"TOOLZ")		||
	_tcsstr(x,"Toolz")		||
	_tcsstr(x,"toolz")		||
	_tcsstr(x,"INJECT")	    ||
	_tcsstr(x,"Inject")	    ||
	_tcsstr(x,"inject")	    ||
	_tcsstr(x,"INJECTOR")	||
	_tcsstr(x,"Injector")	||
	_tcsstr(x,"injector")	||
	_tcsstr(x,"OLLY")		||
	_tcsstr(x,"Olly")		||
	_tcsstr(x,"olly")		||
	_tcsstr(x,"HXD")         ||
	_tcsstr(x,"HxD")         ||
	_tcsstr(x,"hxd")         ||
	_tcsstr(x,"Emulator:")   ||   //SolidProxy Stealth mode
	_tcsstr(x,"Style30_Theme1")  || //darkterro gamehacks
	_tcsstr(x, "Value") ||
	_tcsstr(x, "Processes") ||
	_tcsstr(x, "Confirmation") ||
	_tcsstr(x, "Change address") ||
	_tcsstr(x, "Open existing file") ||
	_tcsstr(x, "Process List") ||
	_tcsstr(x, "Pointer scan") ||
	_tcsstr(x, "Pointerscanner scanoptions") ||
	_tcsstr(x, "WIN64AST 1.1 beta7") ||
	_tcsstr(x, "WIN64AST") ||
	_tcsstr(x, "#### Privado #### AutoPot ####") ||
	_tcsstr(x, "AP & AC v.1 [Frozen]") ||
	_tcsstr(x, "MuAway Simpler Hack") ||
	_tcsstr(x, "ΑuŦØ pØŦIØИ - CØмBØ") ||
	_tcsstr(x, "ΑuŦØ pØŦiØИ - CØмBØ") ||
	_tcsstr(x, "Multi~Cheat v.2 By Frozen [TEAMGAMEHACKS]") ||
	//_tcsstr(x, "Info") ||
	_tcsstr(x, "Commands and Macros") ||
	//_tcsstr(x, "AC Tool 5.4.0") ||
	//_tcsstr(x, "AC Tool") ||
	_tcsstr(x, "Extreme Injector") ||
	_tcsstr(x, "Injecton") ||
	_tcsstr(x, "Mendes DLL Injector") ||
	_tcsstr(x, "DLL Injector") ||
	_tcsstr(x, "Inject DLL") ||
	_tcsstr(x, "Select a process") ||
	_tcsstr(x, "Memory Viewer") ||
	_tcsstr(x, "Hardcore Injector") ||
	_tcsstr(x, "RemoteDLL") ||
	_tcsstr(x, "Sea of Thieves Injector") ||
	_tcsstr(x, "Superject") ||
	_tcsstr(x, "Ki Injector") ||
	_tcsstr(x, "DLInjector") ||
	_tcsstr(x, "CheatSquad Injector") ||
	_tcsstr(x, "DrokJector") ||
	_tcsstr(x, "Auto DLL Injector") ||
	_tcsstr(x, "AlisaAlis Injector") ||
	_tcsstr(x, "Winject") ||
	_tcsstr(x, "INJECTEASY") ||
	_tcsstr(x, "DllRifle") ||
	//_tcsstr(x, "Process") ||
	_tcsstr(x, "CC Dll Injector") ||
	_tcsstr(x, "CLImE_DllInjector") ||
	_tcsstr(x, "K^2's DLL Injector") ||
	_tcsstr(x, "HEX-EN Injector") ||
	_tcsstr(x, "MicroNukes Epic Injector") ||
	_tcsstr(x, "Chong Dis Khi Hack Click truoc ") ||
	_tcsstr(x, "Macro Flooding Tool Mundo Jacar") ||
	_tcsstr(x, "Ultimate Global MuOnline Cheat II") ||
	_tcsstr(x, "JewelDropBeta 0.1") ||
	_tcsstr(x, "Mega-Hack MuMad Desenvolvido Especialmente para WebHacke By: LoukoWH") ||
	_tcsstr(x, "OverClock MU v0.2") ||
	_tcsstr(x, "cheat darkterro") ||
	_tcsstr(x, "WildProxy ") ||
	_tcsstr(x, "Speed Hack by Ziomal") ||
	_tcsstr(x, "ByAnonGr") ||
	_tcsstr(x, "Hit Count") ||
	_tcsstr(x, "HaHaMu Bot 1.16 Translator v 1.1") ||
	_tcsstr(x, "MerryChristmas HackFree By Froz") ||
	_tcsstr(x, "VolksTurbinas") ||
	_tcsstr(x, "Philiphinas Cheat II") ||
	_tcsstr(x, "FireCheats - By PiR@DeX -FREE ") ||
	_tcsstr(x, "Extreme Injector v3.7.3 - by ma") ||
	_tcsstr(x, "Calculadora Item-Set Index Game") ||
	_tcsstr(x, "Youre the Best? (Beta)") ||
	_tcsstr(x, "HEROES  XTREME X6") ||
	_tcsstr(x, "16 Heroes") ||
	_tcsstr(x, "18 AUTO COMBO") ||
	_tcsstr(x, "Speed Hack Simplifier 1.3 Test") ||
	_tcsstr(x, "Game Speed Changer") ||
	_tcsstr(x, "5moLL 1nJect0r V1= smoll_ice2@hotmail.com") ||
	_tcsstr(x, "Baixaki_Hot Virtual Keyboard_27") ||
	_tcsstr(x, "MMSNIFF 1.51") ||
	_tcsstr(x, "SmartSniff 1.40") ||
	_tcsstr(x, "MMSNIFF 1.40") ||
	_tcsstr(x, "CE - Kernelmoduleunloader - V6") ||
	_tcsstr(x, "MuInvasi0n By DarkTerro") ||
	_tcsstr(x, "Suco Hack Mu Season4 1.05D") ||
	_tcsstr(x, "#### Privado #### AutoPot ####.") ||
	_tcsstr(x, "[Cheat SkyDark] By D4rkTerro") ||
	_tcsstr(x, "TGH Explosion") ||
	_tcsstr(x, "OllyDBG_CiM's_Edition") ||
	_tcsstr(x, "Catastrophe RC1 by Astrogod") ||
	_tcsstr(x, "Gold & Items Duplicacion Hack-Rat Studios 1.0 BT") ||
	_tcsstr(x, "Ultimate Cheat Public Release - 2009 - By ZergNM") ||
	_tcsstr(x, "SEASON 8 - EP3 - CHEAT - BY DAR") ||
	_tcsstr(x, "House Edition") ||
	_tcsstr(x, "D3SCENEINJECTOR") ||
	_tcsstr(x, "WPePro 0.9a") ||
	_tcsstr(x, "SR_RODRIGO Mu ZhyperMU eX700 Pl") ||
	_tcsstr(x, "Season 2 Proxy 0.8 =beta=") ||
	_tcsstr(x, "Hit Count Updater for MuProxy v 2.6 - v1.1") ||
	_tcsstr(x, "IDA") ||
	_tcsstr(x,"debug")){

if (phathien.Log_Txt_Hack == 1){
ofstream out("Xtrap/Log.txt", ios::app);
out << "\nX-TRAP:    ", out << x;    // printf ("Found Hack = %s " , windowName);
out.close();
}
// if (phathien.Hack_Log_Upload == 1){
// time_t rawtime;
// struct tm * timeinfo;
// time (&rawtime);
// timeinfo = localtime (&rawtime);
//     ofstream out("Log", ios::app);
//	 out <<"\nLocal Time: ", out << asctime(timeinfo);
//       out <<"HW-Scan:    ", out << x;
//	 out << "\n= = = = = = = = = = = = = = = = = = =";
//	 out.close();
// SetFileAttributes("Log", FILE_ATTRIBUTE_HIDDEN); // Set file as a HIDDEN file
//}
    if (phathien.Message_Warning_En == 1){
    CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_WIN_En),NULL,0,0);
	Sleep(3000); 
	ExitProcess(0);	
}
    if (phathien.Message_Warning_En == 2){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_WIN_Br),NULL,0,0);
	Sleep(500); 
	ExitProcess(0);
	}
	if (phathien.Message_Warning_En == 3){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_WIN_Page),NULL,0,0);
	Sleep(3000); 
	ExitProcess(0);	
	}
	if (phathien.Message_Warning_En == 4){
    CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Kill_From_Message_Warning),NULL,0,0); 
    MessageBoxA(NULL, x, "X-TRAP", MB_SERVICE_NOTIFICATION | MB_ICONSTOP); 
    ExitProcess(0);
	}
	if (phathien.Message_Warning_En == 0){
	ExitProcess(0);	
	}
	else
	ExitProcess(0);				
    return FALSE;
    }
	//cout <<"true";
	return TRUE;
}


void heuristica(){
	EnumWindows(heurprocess, NULL);
}

void heuristico(){
if (phathien.Anti_Kill_Scans == 1)
	{
again:
    DHeuriWin(); //Antikill
    heuristica();
    Sleep (phathien.Phathien_TuKhoa_Win_Thoigian);
    goto again;
}
else
{
	again2:
    heuristica();
    Sleep (phathien.Phathien_TuKhoa_Win_Thoigian);
	goto again2;
}
}

void HeuristicW(){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(heuristico),NULL,0,0);
}