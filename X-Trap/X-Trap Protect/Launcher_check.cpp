
#include "StdAfx.h"

void Msg_La_Br(){
	MessageBoxA(NULL,"L-Scan\n\nO Launcher deve permanecer ativo!", phathien.Ten_Cua_So, MB_SERVICE_NOTIFICATION | MB_ICONWARNING);
	ExitProcess(0);
}
void Msg_La_En(){
	MessageBoxA(NULL,"L-Scan\n\nLauncher are not running on system!", phathien.Ten_Cua_So, MB_SERVICE_NOTIFICATION | MB_ICONWARNING);
ExitProcess(0);
}
void Msg_La_Page(){
	Sleep (2000);
    ShellExecute(NULL, "open", phathien.HackSplash_WebSite, NULL, NULL, SW_SHOWNORMAL);
	}


void Fecha_ativa(){
	if (phathien.Log_Txt_Hack == 1){	
    ofstream out("Xtrap/Log.txt", ios::app);
    out << "\nAL-Scan:    ", out <<phathien.LauncherWindowName<< " are not running on system!";
	out.close();
	}
//	 if (phathien.Hack_Log_Upload == 1){
// time_t rawtime;
// struct tm * timeinfo;
// time (&rawtime);
// timeinfo = localtime (&rawtime);
//     ofstream out("Log", ios::app);
//	 out <<"\nLocal Time: ", out << asctime(timeinfo);
//       out <<"AL-Scan:    ", out <<phathien.LauncherWindowName<< " are not running on system!";
//	 out << "\n= = = = = = = = = = = = = = = = = = =";
//	 out.close();
// SetFileAttributes("Log", FILE_ATTRIBUTE_HIDDEN); // Set file as a HIDDEN file
//}
    if (phathien.Message_Warning_En == 1){
    CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_La_En),NULL,0,0);
	Sleep(3000);
	ExitProcess(0);	
}
    if (phathien.Message_Warning_En == 2){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_La_Br),NULL,0,0);
	Sleep(3000);
	ExitProcess(0);	
}
	if (phathien.Message_Warning_En == 3){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_La_Page),NULL,0,0);
	Sleep(3000); 
	ExitProcess(0);	
}
	if (phathien.Message_Warning_En == 4){
    CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Kill_From_Message_Warning),NULL,0,0); 
    MessageBoxA(NULL, phathien.LauncherWindowName, "AL-Scan", MB_SERVICE_NOTIFICATION | MB_ICONSTOP); 
    ExitProcess(0);
}
	if (phathien.Message_Warning_En == 0){
	ExitProcess(0);
	}
	else
	ExitProcess(0);
}

void Launcher_Name(){
	HWND WinTitle = FindWindowA(NULL, phathien.LauncherWindowName);
	if( WinTitle == 0){
	Fecha_ativa();
	}
	HWND WinClasse = FindWindowExA(NULL,NULL,phathien.LauncherClassName,NULL);
	if( WinClasse == 0){ 
	Fecha_ativa();
	//WinExec(phathien.Nome_do_Main, SW_SHOWNORMAL); // START Mxmain.exe
}
}

void LA_Name(){
	if (phathien.Anti_Kill_Scans == 1)
	{
again:
	DLauncher(); //Antikill
    Launcher_Name();
    Sleep(phathien.Launcher_Check_Thoigian);
    goto again;
}
	else
	{
again2:
    Launcher_Name();
    Sleep(phathien.Launcher_Check_Thoigian);
    goto again2;
}
}

void LA_On_byName(){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(LA_Name),NULL,0,0);	
}

