
#include "StdAfx.h"

void Msg_CW_Br(){
	MessageBoxA(NULL,"W-Close\n\nO processo não pode ser fechado! Saindo do Jogo!", phathien.Ten_Cua_So, MB_SERVICE_NOTIFICATION | MB_ICONWARNING);	
ExitProcess(0);
}

void Msg_CW_En(){
	MessageBoxA(NULL,"W-Close\n\nProcess can't be closed! Exiting Game!", phathien.Ten_Cua_So, MB_SERVICE_NOTIFICATION | MB_ICONWARNING);	
ExitProcess(0);
} 

void Msg_CW_Page(){
	Sleep (2000);
    ShellExecute(NULL, "open", phathien.HackSplash_WebSite, NULL, NULL, SW_SHOWNORMAL);
	}


void Close_W_fail(){
	if (phathien.Message_Warning_En == 1 || phathien.Message_Warning_En == 4){
    CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_CW_En),NULL,0,0);
	Sleep(3000); 
	ExitProcess(0);	
}
    if (phathien.Message_Warning_En == 2){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_CW_Br),NULL,0,0);
	Sleep(3000); 
	ExitProcess(0);	
	}
	if (phathien.Message_Warning_En == 3){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_CW_Page),NULL,0,0);
	Sleep(3000); 
	ExitProcess(0);	
	}
	if (phathien.Message_Warning_En == 0){
	ExitProcess(0);
	}
	else
	ExitProcess(0);
}

bool CloseWin(LPCSTR WindowTitle){
HWND WinTitle = FindWindowA(NULL,WindowTitle);
if( WinTitle > 0){
SendMessage(WinTitle, WM_CLOSE,0,0); 
if (phathien.Log_Txt_Hack == 1){	
ofstream out("Xtrap/Log.txt", ios::app);
out << "\nW-Close:     ", out <<   WindowTitle;
out.close();
}
//	 if (phathien.Hack_Log_Upload == 1){
// time_t rawtime;
// struct tm * timeinfo;
// time (&rawtime);
// timeinfo = localtime (&rawtime);
//     ofstream out("Log", ios::app);
//	 out <<"\nLocal Time: ", out << asctime(timeinfo);
//       out <<"W-Close:    ", out << WindowTitle;
//	 out << "\n= = = = = = = = = = = = = = = = = = =";
//	 out.close();
// SetFileAttributes("Log", FILE_ATTRIBUTE_HIDDEN); // Set file as a HIDDEN file
//}
Sleep (2000);
HWND WinTitle = FindWindowA(NULL,WindowTitle);
if( WinTitle > 0){
if (phathien.Log_Txt_Hack == 1){	
ofstream out("Xtrap/Log.txt", ios::app);
out << "\n W-Close:    " <<WindowTitle<<" can't be closed, exiting game!";
out.close();
}
//	 if (phathien.Hack_Log_Upload == 1){
// time_t rawtime;
// struct tm * timeinfo;
// time (&rawtime);
// timeinfo = localtime (&rawtime);
//     ofstream out("Log", ios::app);
//	 out <<"\nLocal Time: ", out << asctime(timeinfo);
//       out <<"W-Close:    " <<WindowTitle<<" can't be closed, exiting game!";
//	 out << "\n= = = = = = = = = = = = = = = = = = =";
//	 out.close();
// SetFileAttributes("Log", FILE_ATTRIBUTE_HIDDEN); // Set file as a HIDDEN file
// Close_W_fail();
//}
	 /*else
	 {
	 Close_W_fail();
}*/
	return false;
	}
}
	return true;
}



////////////////////////////////////////////////////////////////////////////////////////////////
//Window close scan - W-Close 
//Are Case-sensitive - Find it just looking window name or using [Handler 1.5 by Oliver Bock] 
//TitleWindow("xxxx");
////////////////////////////////////////////////////////////////////////////////////////////////
void Close_Winx(){
CloseWin("Cheat Engine 5.0");
}

void CloseWin_(){
	if (phathien.Anti_Kill_Scans == 1)
	{
again:
	CloseWin(); // Antikill
    Close_Winx();
    Sleep(phathien.DWindow_Thoigian);
    goto again;
}
	else
	{
	again2:
    Close_Winx();
    Sleep(phathien.DWindow_Thoigian);
    goto again2;
	}
}

void Close_Window(){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(CloseWin_),NULL,0,0);
}

