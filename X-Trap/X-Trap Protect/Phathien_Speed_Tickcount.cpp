
#include "StdAfx.h"

void Msg_S_Br(){
	MessageBoxA(NULL,"ST-Scan\n\nConteúdo suspeito detectado!", phathien.Ten_Cua_So, MB_SERVICE_NOTIFICATION | MB_ICONWARNING);
ExitProcess(0);
} 

void Msg_S_En(){
	MessageBoxA(NULL,"ST-Scan\n\nAn illegal choice haas been detected!", phathien.Ten_Cua_So, MB_SERVICE_NOTIFICATION | MB_ICONWARNING);
ExitProcess(0);
}

void Msg_S_Page(){
	Sleep (2000);
    ShellExecute(NULL, "open", phathien.HackSplash_WebSite, NULL, NULL, SW_SHOWNORMAL);
	}



void tickplash(){
	if (phathien.Log_Txt_Hack == 1){
ofstream out("Xtrap/Log.txt", ios::app);
out << "\nX-TRAP:    ", out <<"Speed Hack or System Freeze detected!";
out.close();
}
//if (phathien.Hack_Log_Upload == 1){
// time_t rawtime;
// struct tm * timeinfo;
// time (&rawtime);
// timeinfo = localtime (&rawtime);
//     ofstream out("Log", ios::app);
//	 out <<"\nLocal Time: ", out << asctime(timeinfo);
//     out <<"ST-Scan:    ", out <<"Speed Hack or System Freeze detected!";
//	 out << "\n= = = = = = = = = = = = = = = = = = =";
//	 out.close();
// SetFileAttributes("Log", FILE_ATTRIBUTE_HIDDEN); // Set file as a HIDDEN file
//}
    if (phathien.Message_Warning_En == 1){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_S_En),NULL,0,0);
  	Sleep(3000); 
	ExitProcess(0);	
}
	if (phathien.Message_Warning_En == 2){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_S_Br),NULL,0,0);
  	Sleep(3000); 
	ExitProcess(0);	 
}
    if (phathien.Message_Warning_En == 3){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_S_Page),NULL,0,0);
	Sleep(3000); 
	ExitProcess(0);	
}
	if (phathien.Message_Warning_En == 4){
    CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Kill_From_Message_Warning),NULL,0,0); 
    MessageBoxA(NULL, "Speed Hack detected!", "X-TRAP", MB_SERVICE_NOTIFICATION | MB_ICONSTOP); 
    ExitProcess(0);
}
	if (phathien.Message_Warning_En == 0){
	ExitProcess(0);
	}
	else
	ExitProcess(0);
}


int ispidi = 0;
void Speed_Scanner(){ 
again2:
	int timer = ::GetTickCount();
again:
	DSpeed();//Antikill 	   	
    Sleep(phathien.Phathien_Speed_Thoigian); 
	ispidi++;
	//cout << isp <<endl;
	if (ispidi != 20){
	goto again;
	}
	else
	{
ispidi = 0;
int timeElapsed = ::GetTickCount() - timer; 
//printf ( "GetTickCount Speed time are: %d \n", timeElapsed) ;
if ("%d", timeElapsed > 10500){ // velocidade máxima (em milisegundos) que não deve ser ultrapassada
tickplash();
	}
	else
	{
    goto again2;
	}
}
}

void DetectSpeed(){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Speed_Scanner),NULL,0,0);	
}













