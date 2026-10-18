
#include "StdAfx.h"

void Msg_PC_Br(){
	MessageBoxA(NULL,"PID-Scan\n\nConteúdo suspeito detectado!", phathien.Ten_Cua_So, MB_SERVICE_NOTIFICATION | MB_ICONWARNING);	
ExitProcess(0);
} 

void Msg_PC_En(){
	MessageBoxA(NULL,"PID-Scan\n\nAn Phat hien can thiep game!", phathien.Ten_Cua_So, MB_SERVICE_NOTIFICATION | MB_ICONWARNING);	
ExitProcess(0);
}

void Msg_PC_Page(){
	Sleep (2000);
    ShellExecute(NULL, "open", phathien.HackSplash_WebSite, NULL, NULL, SW_SHOWNORMAL);
	}


	void GetProcId(char* ProcName){
    PROCESSENTRY32 pe32;
    HANDLE hSnapshot = NULL;

	pe32.dwSize = sizeof( PROCESSENTRY32 );
    hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

    if( Process32First( hSnapshot, &pe32 )){
        do{
            if( strcmp( pe32.szExeFile, ProcName ) == 0 )
            {
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);  // Close detected process
			TerminateProcess(hProcess,NULL);                                               // Close detected process
    if (phathien.Log_Txt_Hack == 1){	
    ofstream out("Xtrap/Log.txt", ios::app);
    out << "\nX-TRAP:   ", out << ProcName;
}
// if (phathien.Hack_Log_Upload == 1){
// time_t rawtime;
// struct tm * timeinfo;
// time (&rawtime);
// timeinfo = localtime (&rawtime);
//     ofstream out("Log", ios::app);
//	 out <<"\nLocal Time: ", out << asctime(timeinfo);
//       out <<"PID-Scan:   ", out << ProcName;
//	 out << "\n= = = = = = = = = = = = = = = = = = =";
//	 out.close();
// SetFileAttributes("Log", FILE_ATTRIBUTE_HIDDEN); // Set file as a HIDDEN file
//}
    if (phathien.Message_Warning_En == 1){
    CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_PC_En),NULL,0,0);
	Sleep(3000); 
	ExitProcess(0);	
}
    if (phathien.Message_Warning_En == 2){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_PC_Br),NULL,0,0);
	Sleep(3000); 
	ExitProcess(0);
}
   if (phathien.Message_Warning_En == 3){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Msg_PC_Page),NULL,0,0);
	Sleep(3000); 
	ExitProcess(0);	
}
   if (phathien.Message_Warning_En == 4){
    CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(Kill_From_Message_Warning),NULL,0,0); 
    MessageBoxA(NULL, ProcName, "X-TRAP", MB_SERVICE_NOTIFICATION | MB_ICONSTOP);
    ExitProcess(0);
}
	else 
	ExitProcess(0);	

            }
        }while( Process32Next( hSnapshot, &pe32 ) );
    }
    if( hSnapshot != INVALID_HANDLE_VALUE )
        CloseHandle( hSnapshot );   
}

////////////////////////////////////////////////////////////////////////////////////////////////
//Process ID - PID-Scan 
//Are Case-sensitive - Find it using: Windows Task Manger/table processes (ctrl + alt + del)
//GetProcId("xxxx.exe");
//GetProcId("XXXX.EXE"); 
////////////////////////////////////////////////////////////////////////////////////////////////

void ClasseCheckPross(){ 
	// PID Detector are Case-sensitive!
	GetProcId("ollydbg.exe*32");
	GetProcId("ollydbg.exe");
	GetProcId("-.exe");
	GetProcId("ProcessHacker.exe");
	GetProcId("PCHunter.exe");
	GetProcId("PCHunter32.exe");
	GetProcId("PCHunter64.exe");
	GetProcId("bvkhex.exe");
    GetProcId("cheatengine-x86_64.exe");
	GetProcId("HxD.exe");
	GetProcId("procexp2.exe");
	GetProcId("Hide Toolz3.3.3.exe");
	GetProcId("SbieSvc.exe");    // < sandbox 
	GetProcId("SbieSvc*32.exe"); // < sandbox 
	GetProcId("SbieSvc*32.exe"); // < sandbox 
	GetProcId("SbieCtrl.exe");   // < sandbox 
	GetProcId("Start.exe");      // < sandbox
	GetProcId("#### Privado #### AutoPot ####.exe");
	GetProcId("AP TGH Frozen.exe");
	GetProcId("AUTOPOT LVL1.exe");
	GetProcId("ΑuŦØ pØŦIØИ - CØмBØ.exe");
	GetProcId("shine.exe");
	GetProcId("perfmon.exe");
	GetProcId("Taskmgr.exe");
	GetProcId("TitanEditor.exe.exe");
	GetProcId("i6188.exe");
	GetProcId("MuMaker.exe");
	GetProcId("PowerTool.exe");
	GetProcId("uknow-without dbk.exe");
	GetProcId("JasonGWAPO.exe");
	GetProcId("StevenBOBO.exe");
	GetProcId("02 TEAM_GH XXX x86_64.exe");
	GetProcId("TEAM_GH XXX x86_64.exe");
	GetProcId("CE.exe");
	GetProcId("Rzpnk.exe");
	GetProcId("dbk32.exe");
	GetProcId("RzSynapse.exe");
	GetProcId("ssss.exe");
	GetProcId("Jeferson (Over-Hide).exe");
	GetProcId("NMsvc.exe");
	GetProcId("NetworkInventoryExplorer.exe");
	GetProcId("NetMonitor.exe");
	GetProcId("NetScan.exe");
	GetProcId("pautomation.exe");
	GetProcId("prckiller.exe");
	GetProcId("MuProxy.exe");
	GetProcId("Vzla Engine.exe");
	GetProcId("Xtreme Mu X6.exe");
	GetProcId("WpeSpy-v1.0.exe");
	GetProcId("OLLYDBGh.exe");
	GetProcId("OllyDBG_X-Colly.exe");
	GetProcId("Hook.exe");
	GetProcId("PulangltIog.exe");
	GetProcId("Muted.exe");
	GetProcId("## Injector ##.exe");
	GetProcId("06 ## Injector ##.exe");
	GetProcId("Devilz - xDmgr.exe");
	GetProcId("OllyDBG_CiM's_Edition (3).exe");
	GetProcId("VIP 1.1.exe");
	GetProcId("LibHook.exe");
	GetProcId("Globe Broadband.exe");
	GetProcId("H4sTyMU.exe");
	GetProcId("sandboxie_5240.exe");
	GetProcId("658 PORTAL.exe");
	GetProcId("MotoSpeed Gaming MouseV60.exe");
	GetProcId("SpeedMUVN.exe");
	GetProcId("08 inject hacker [mu].exe");
	GetProcId("inject hacker [mu].exe");
	GetProcId("standalonephase1.exe");
	GetProcId("Process_Hider.exe");
	GetProcId("xtreme_mu_x6_0dc-3b1___.exe");
	GetProcId("EX700PsHackPack.exe");
	GetProcId("KeyTweak.exe");
	GetProcId("DragAndClick.exe");
	GetProcId("FileDate.exe");
	GetProcId("programme test.exe");
	GetProcId("GetAhInfo.exe");
	GetProcId("Inject for All.exe");
	GetProcId("GameClicker.exe");
	GetProcId("WinHide.exe");
	GetProcId("RzDriverInstaller.exe");
	GetProcId("SupperMan.exe");
	GetProcId("Window-Title-Changer.exe");
	GetProcId("RzDriverInstaller.exe");
	GetProcId("RazerServiceCommon.exe");
	GetProcId("ProcessMagic.exe");
	GetProcId("Chaos-WKE64.exe");
	GetProcId("GameManagerServiceStartup.exe");
	GetProcId("651 GameManagerServiceStartup.exe");
	GetProcId("sinJect.exe");
	GetProcId("RzUpdateManager.resources.exe");
	GetProcId("Pirate Speed.exe");
	GetProcId("ColorClicker.exe");
	GetProcId("RzUnlockAgent.exe");
	GetProcId("RzUpdate.exe");
	GetProcId("Dokan.exe");
	//GetProcId("CefSharp.BrowserSubprocess.exe");
	GetProcId("uninstallhlpr.exe");
	GetProcId("SandboxiePortable.exe");
	GetProcId("ToolMuSS6.exe");
	GetProcId("428 PB.exe");
	GetProcId("PB.exe");
	GetProcId("systemcallsignal.exe");
	GetProcId("PCHunterPortable.exe");
	GetProcId("GameOwner.exe");
	//GetProcId("GameManagerService.exe");
	GetProcId("Mixer Engine-i386.exe.exe");
	GetProcId("cheatengine-i386.exe");
	GetProcId("Games Hacks-titeeeee.exe");
	GetProcId("hahaha.exe");
	GetProcId("titeeeee.exe");
	GetProcId("PERU-i386.exe");
	GetProcId("Chile-i386.exe");
	GetProcId("Argentina-i386.exe");
	GetProcId("Internet Explorer-Chile-Argentina-PERU-TAPUZ-ShirShir-i386.exe");
	GetProcId("ShirShir-i386.exe");
	GetProcId("MouseClicker.exe");
	GetProcId("xelerator.v2.0-patch.exe");
	GetProcId("MacroServiceWnd.exe");
	GetProcId("Macro Recorder.exe");
	GetProcId("RazerIngameEngine.exe");
	GetProcId("AutoInject.exe");
	GetProcId("RzBrowserApplet.exe");
	GetProcId("RzRBSApplet.exe");
	GetProcId("1009 AutoPotion.exe");
	GetProcId("AutoPotion.exe");
	GetProcId("Suppend.exe");
	GetProcId("TeamHack.com AutoCombo.exe");
	GetProcId("15 AP TGH Frozen - ATUAL 2016.exe");
	GetProcId("AP TGH FROZEN 2016.exe");
	GetProcId("AP TGH Frozen - ATUAL 2016.exe");
	GetProcId("Pluguin Cheat engine.exe");
	GetProcId("EvilHookv1.exe");
	GetProcId("RzCefRenderProcess.exe");
	GetProcId("1018 AutoPotion.exe");
	GetProcId("MUAutoClicker.exe");
	GetProcId("Heroes Xtreme X6.exe");
	GetProcId("RzUISdk.exe");
	GetProcId("FlooderMuServer.exe");
	GetProcId("Au3Info.exe");
	GetProcId("1004 originalMuStorm.exe");
	GetProcId("Winject .exe");
	GetProcId("19 Cliente.exe");
	GetProcId("ThienDuong VangEm.exe");
	GetProcId("MAX JUNIOR - TGH.exe");
	GetProcId("Creepy Modz DLL Injector.exe");
	GetProcId("DarkSpy_En.exe");
	GetProcId("45 Speed 97D By Hidan.exe");
	GetProcId("Speed 97D By Hidan.exe");
	GetProcId("Welcome to Hell - S9.exe");
	GetProcId("ChupaVM.exe");
	GetProcId("Devilz - xD.exe");
	GetProcId("Bandcan12.exe");
	GetProcId("DARKTERRO 99B+S2.exe");
	GetProcId("471 Trainer especial by Delphi.exe");
	GetProcId("Trainer especial by Delphi.exe");
	GetProcId("SupendMain.exe");
	GetProcId("Playerbmdnguyen.exe");
	GetProcId("MHPDetect64.exe");
	GetProcId("Black Box hack by Garrix.exe");
	GetProcId("MultiHack by KingHack.exe");
	GetProcId("667 Black Box hack by afro 2015.exe");
	GetProcId("Black Box hack by afro 2015.exe");
	GetProcId("BlackBox by afroo 2016.exe");
	GetProcId("banvip.exe");
	GetProcId("123qa.exe");
	GetProcId("standalonephase2.exe");
	GetProcId("KHOCMUON Hack.exe");
	GetProcId("TOOL MU SEASON 6.exe");
	GetProcId("Mu Cracker zombie.exe");
	GetProcId("VuNguyen Ver 1.0.exe");
	GetProcId("Vu Nguyen.exe");
	GetProcId("Vinh Tu.exe");
	GetProcId("PipeServer.exe");
	GetProcId("TanMui.exe");
	GetProcId("GameScannerCommon.exe");
	GetProcId("VinhTu2.exe");
	GetProcId("Le Vinh Tu Jav.exe");
	GetProcId("AnhThuy.exe");
	GetProcId("N_CUONG.exe");
	GetProcId("hack-nah-VinhTu2-Cuongexe.exe");
	GetProcId("Cuongexe.exe");
	GetProcId("To Ten Thanh.exe");
	GetProcId("Tu cmn suong.exe");
	GetProcId("VuNguyen Ver 1.0.exe");
}

void PC_Scan(){
	if (phathien.Anti_Kill_Scans == 1)
	{
again:
	DProcID(); //Antikill
    ClasseCheckPross();
    Sleep(phathien.DPID_Thoigian);
    goto again;
}
	else
	{
again2:
    ClasseCheckPross();
    Sleep(phathien.DPID_Thoigian);
    goto again2;
}
}

void DetectID(){
	CreateThread(NULL,NULL,LPTHREAD_START_ROUTINE(PC_Scan),NULL,0,0);
	}

