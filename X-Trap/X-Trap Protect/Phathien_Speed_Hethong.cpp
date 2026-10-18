
#include "stdafx.h"

void Msg_Sf_Br(){
	MessageBoxA(NULL,"FS-Scan\n\nConteúdo suspeito detectado!", phathien.Ten_Cua_So, MB_SERVICE_NOTIFICATION | MB_ICONWARNING);
ExitProcess(0);
} 

void Msg_Sf_En(){
	MessageBoxA(NULL,"FS-Scan\n\nAn illegal choice haas been detected!", phathien.Ten_Cua_So, MB_SERVICE_NOTIFICATION | MB_ICONWARNING);
ExitProcess(0);
}

void Msg_Sf_Page(){
	Sleep (2000);
    ShellExecute(NULL, "open", phathien.HackSplash_WebSite, NULL, NULL, SW_SHOWNORMAL);
	}


void Speed_perf(){
//cout << *(unsigned long*)QueryPerformanceCounter <<endl;
if( *(unsigned long*)QueryPerformanceCounter != 2337669003 ){
if (phathien.Log_Txt_Hack == 1){
ofstream out("Xtrap/Log.txt", ios::app);
out << "\nX-TRAP:     ", out <<"Your computer's processing speed is not stable!";
out.close();
}
//if (phathien.Hack_Log_Upload == 1){
// time_t rawtime;
// struct tm * timeinfo;
// time (&rawtime);
// timeinfo = localtime (&rawtime);
//     ofstream out("Log", ios::app);
//	 out <<"\nLocal Time: ", out << asctime(timeinfo);
//     out <<"nProtect GameGuard:    ", out <<"Your computer's processing speed is not stable!";
//	 out << "\n= = = = = = = = = = = = = = = = = = =";
//	 out.close();
// SetFileAttributes("Log", FILE_ATTRIBUTE_HIDDEN); // Set file as a HIDDEN file
//}
    if (phathien.Message_Warning_En == 1){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_Sf_En),NULL,0,0);
  	Sleep(3000); 
	ExitProcess(0);	
}
	if (phathien.Message_Warning_En == 2){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_Sf_Br),NULL,0,0);
  	Sleep(3000); 
	ExitProcess(0);	 
}
    if (phathien.Message_Warning_En == 3){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_Sf_Page),NULL,0,0);
	Sleep(3000); 
	ExitProcess(0);	
}
	if (phathien.Message_Warning_En == 4){
    CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Kill_From_Message_Warning),NULL,0,0); 
    MessageBoxA(NULL, "Your computer's processing speed is not stable!", "X-TRAP", MB_SERVICE_NOTIFICATION | MB_ICONSTOP); 
    ExitProcess(0);
}
	if (phathien.Message_Warning_En == 0){
	ExitProcess(0);
	}
	else
	ExitProcess(0);
}
}


void performance(){
	if (phathien.Anti_Kill_Scans == 1)
	{
again:
	DSPerf(); //Antikill
    Speed_perf();
    Sleep(phathien.Phathien_Speed_Hethong_Thoigian );
    goto again;
}
	else
	{
again2:
    Speed_perf();
    Sleep(phathien.Phathien_Speed_Hethong_Thoigian);
    goto again2;
}
}

void SPerformance(){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(performance),NULL,0,0);
	}