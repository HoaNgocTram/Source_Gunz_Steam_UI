
#include "stdafx.h"
//VM-Scan splash////////////////////////////////////////////////////////////
void Msg_VM_Br(){
	MessageBoxA(NULL,"VM-Scan\n\nConteúdo suspeito detectado!", phathien.Ten_Cua_So, MB_SERVICE_NOTIFICATION | MB_ICONWARNING);
	ExitProcess(0);
}
void Msg_VM_En(){
	MessageBoxA(NULL,"VM-Scan\n\nAn illegal choice haas been detected!", phathien.Ten_Cua_So, MB_SERVICE_NOTIFICATION | MB_ICONWARNING);
	ExitProcess(0);
}

void Msg_VM_Page(){
	Sleep (2000);
    ShellExecute(NULL, "open", phathien.HackSplash_WebSite, NULL, NULL, SW_SHOWNORMAL);
	}

void Attack_Value(){
	 if (phathien.Log_Txt_Hack == 1){
ofstream out("Xtrap/Log.txt", ios::app);
out << "\nVM-Scan:     ", out <<   "Virtual memory Value changed!"; 
out.close();
}
    if (phathien.Message_Warning_En == 1){
    CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_VM_En),NULL,0,0);
	Sleep(3000); 
	ExitProcess(0);	
}
    if (phathien.Message_Warning_En == 2){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_VM_Br),NULL,0,0);
	Sleep(3000); 
	ExitProcess(0);	
	}
	if (phathien.Message_Warning_En == 3){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_VM_Page),NULL,0,0);
	Sleep(3000); 
	ExitProcess(0);	
	}
	if (phathien.Message_Warning_En == 4){
    CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Kill_From_Message_Warning),NULL,0,0); 
    MessageBoxA(NULL, "Virtual memory Value changed!", "VM-Scan", MB_SERVICE_NOTIFICATION | MB_ICONSTOP); 
    ExitProcess(0);
}
	if (phathien.Message_Warning_En == 0){
	ExitProcess(0);
	}
	else
	ExitProcess(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Virtual Memory protection - VM-Scan 

//How to:
// Use HxD.exe to get address that you want to protect
// For example, run mxmain(fake).exe, open mxmain(fake).exe with HxD (virtual memory), search for string Gasmask, or important value that need be protected on virtual memory.  
// Select G, press CTRL+E (or right click) Select bloc.  That is your addrees. 0x449081
// Turn On phathien.Memory_Buffer_L on START.cpp to get address buffer, add buffer on detector, turn off phathien.Memory_Buffer_L. 
// Change values on memory with HxD to test. (unsigned short) protect 5 hex from start point adrees
////////////////////////////////////////////////////////////////////////////////////////////////

// PROTECT MAIN VIRTUAL MEMORY ///////////////////////////////////////
bool Virt()
{
// GET BUFFER MESSAGE
if (phathien.Memory_Buffer_P == 1){
char cbuffer[10];

//WARNING!  Add an address that not exist on Mxmain.exe will crash Mxmain.exe

sprintf (cbuffer, "%d", (*(unsigned short*)0x449081));  //Put ADDRESS here, this is for [phathien.Memory_Buffer_P]                    
MessageBoxA(NULL,(LPCSTR) cbuffer, "Memory_Buffer_P___",MB_ICONWARNING);
ExitProcess(0);	
}

//WARNING!  Add an address that not exist on Mxmain.exe will crash Mxmain.exe
	
//DETECTOR - Protect 5 hex from address start point
  //((*(unsigned short*)0xADDRESS != BUFFER)
if ((*(unsigned short*)0x449081 != 29537) || 
 // (*(unsigned short*)0x449081 != 29537) || 
 // (*(unsigned short*)0x449081 != 29537) || 
 //	(*(unsigned short*)0x449081 != 29537) || 
 //	(*(unsigned short*)0x449081 != 29537) || 
    (*(unsigned short*)0x449081 != 29537)){ 
	
		Attack_Value();  //ExitProcess(0);
		return true;	
	}
	else
	{
		return false;	
	}
}


void VirtA(){	
	if (phathien.Anti_Kill_Scans == 1)
	{
again:
	DVirtualM(); //Antikill
	Virt();
	Sleep (phathien.Virtual_Mem_occours);   
    goto again;
	}
	else
	{
again2:
	Virt();
   	Sleep (phathien.Virtual_Mem_occours);   
    goto again2;
	}
}
	
void Virtual_Attack(){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(VirtA),NULL,0,0);
}