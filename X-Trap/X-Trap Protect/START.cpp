
#include "StdAfx.h"
phathienR phathien;
HANDLE m_DllModule;
extern  "C"  __declspec(dllexport) void __cdecl GasMask_5()  // Hàm API (init) từ dll
{
//string title = _T("Mxmain");  // Đặt tên cửa sổ ứng dụng bảng điều khiển (chỉ dành cho mục đích cấu hình!)
//SetConsoleTitle(title.c_str());

	//////////////////////////////////////////////////////////////////////////
	//CODEX.DLL GENERAL CONFIGURATION ////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

//Load TEST.cpp file
	phathien.TESTE       				= 0; // On/Off - Bắt đầu một tệp cpp thử nghiệm để kiểm tra mã mới

//Load Splash screen image (/Xtrap/Protect.bmd)
	phathien.Splash_Screen				= 1; // On/Off - Load splash image (/Xtrap/Protect.bmd)

//Splash messages
	phathien.Ten_Cua_So			= "X-TRAP"; // // [CHOICE!] Splash message Server name
	phathien.Message_Warning_En          = 4; // [CHOICE!] Hack detect splash message: [Options: 0,1,2,3,4]  0 = silent, 1 = Engish, 2 = Portuguese 3 = Open php webpage with splash image and make a Log.txt file with Date / ip, 4 = Message with hack name detected (for configuration purposes only)
	phathien.HackSplash_WebSite          = "https://gunz.vn/X-TRAP/HackSplash/Splash.php";

//Log file [LƯU Ý !] Đây chỉ là mục đích cấu hình!
	phathien.Log_Txt_Hack                = 1; // On/Off - tạo log trong client  ("Xtrap/Log.txt")
//if (phathien.Log_Txt_Hack == 1){
//MessageBoxA(NULL,"Public HACK DETECTION LOG are active in START.cpp!\n\nWARNING! Log file are Just for configuration purposes!\n\nThink about! Players dont need to know any system DETAILS.\n\nYou can disable this message on START.cpp adding lines intersection: '//' on both lines.","CONFIGURATION MESSAGE", MB_SERVICE_NOTIFICATION | MB_ICONWARNING);}

	//////////////////////////////////////////////////////////////////////////
    //X-TRAP protection and configuration /////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

// File Names verification	
	phathien.Check_File_Names 	        = 0; // On/Off - Check Gunz.exe, Launcher.exe, X-Trap.dll tên tập tin tồn tại bên trong thư mục. Kiểm tra xem tên Gunz và dll có bị thay đổi khi khởi động hay không. Kiểm tra xem tên cửa sổ chính có bị thay đổi không.
	phathien.DVerifica_occours           = 543; // Xảy ra mỗi X mili giây. LƯU Ý! Nếu tính năng này hoạt động thì giá trị này không thể lớn hơn 1000 ]
//Main name and data:
	phathien.Nome_do_Main				= "Gunz.exe";          // Main name 
	phathien.MainWindowName              = "Gunz";              // Tiện ích Handler.exe >> Main WindowName 
	phathien.MainClassName               = "Gunz";  // Tiện ích Handler.exe >> Main Classname
//Anti-Hack dll Name:
	phathien.Ten_Dll                 = "X-TRAP.dll";           // Anti-hack dll name
//Launcher name and data:
//Launcher alwais on:
	phathien.Launcher_Check              = 0; // On/Off - Kiểm tra xem Launcher có luôn bật bằng Launcher Wintitle/Winclass không 
	phathien.Launcher_Check_Thoigian           = 539; //Xảy ra mỗi X mili giây. LƯU Ý! Nếu tính năng này hoạt động thì giá trị này không thể lớn hơn 1000 ]
	phathien.Ten_Launcher	      	= "Launcher.exe";            // Launcher name
	phathien.LauncherWindowName		    = "GUNZ Launcher";              // Tiện ích Handler.exe >> Launcher WindowName 
	phathien.LauncherClassName           = "#32770";  // Tiện ích Handler.exe >> Main Classname  

//GaMeGuard.exe protection 
	phathien.GameGuard_Exe_Protection    = 0; // On/Off - Đây là một biện pháp bảo vệ GameGuard.exe đang hoạt động nhằm ngăn chặn việc tiêu diệt hoặc đóng băng các trình phát hiện hack.
	phathien.CRC_GameGuard  		        = 0; // On/Off - GameGuard.exe file CRC Check 
	phathien.CRC_GameGuard_Value         = 0x43faab90; // CRC check for GameGuard.exe file
	phathien.GameGuard_Value           	= "GameGuard.exe"; // GameGuard.exe file: Location / name / extension 
//Dll injection Protection
	phathien.Inject_dll_Protection       = 0; // On/Off - Đây là tính năng bảo vệ inject DLL giúp tiêu diệt hoặc đóng băng các trình phát hiện hack.
	phathien.CRC_Inject_dll 		        = 0; // On/Off - c.dll file CRC Check - Sử dụng nếu bạn không tạo gói Molebox
	phathien.CRC_Inject_dll_Value        = 0x022b003e; // CRC check for c.dll file (không sử dụng)
	phathien.Inject_Dll_Value            = "c.dll"; // Inject.dll file: Location / name / extension
//Các victim exe do Windows cung cấp, tính năng này chỉ hoạt động trên Windows x32, vì [c.dll] là x32 - Nhận victim windows bằng cách sử dụng: START.cpp >> phathien.Show_Windows_Window_list    = 1;
	phathien.Injected_Window_1	      	= "Start Menu";            // "Start Menu"   (7x32, XPx32)
	phathien.Injected_Window_2	      	= "Default IME";           // "Default IME" Theo dõi ctrl alt del (7x32, XPx32), Có thể: "Battery Meter" (only 7x32),/ "Calculator"(7x32,XPx32) / "Windows Media Player" (7x32,XPx32).
	phathien.Injected_Window_3	      	= "LauncherV5";            // "Cửa sổ Launcher trò chơi của bạn"(Tất cả các phiên bản) BẬT, tính năng Launcher luôn BẬT
//Exe victim khởi động với hệ thống windows    
	phathien.Copy_exe_to_Windows_Startup = 0; // On/Off - Sao chép StartUp.exe vào thư mục khởi động Windows (khởi động bằng windows) và đổi tên thành SounDriver.exe
	phathien.CRC_StartUp  		        = 0; // On/Off - StartUp.exe file CRC Check 
	phathien.CRC_StartUp_Value           = 0x70ab5cfb; // CRC kiểm tra StartUp.exe file
	phathien.StartUp_Original_Name       = "StartUp.exe"; //tệp nằm trong thư mục trò chơi và sẽ được sao chép/đổi tên vào thư mục khởi động Windows
	phathien.StartUp_Address_NewName     = "C:/Documents and Settings/All Users/Start Menu/Programs/Startup/SoundDriver.exe"; //địa chỉ cho tất cả người dùng, tất cả các phiên bản windows (không thay đổi địa chỉ, chọn bất kỳ tên nào bạn muốn)
	
//Configuration and test
	phathien.Show_Windows_Window_list    = 0; // On/Off - Hiển thị (trên ứng dụng bảng điều khiển giả gunz.exe) danh sách tên cửa sổ hệ thống windows để định cấu hình trình inject dll
	phathien.Test_Main_Win_Class_PID     = 0; // On/Off - Kiểm tra xem các giá trị Gunz.exe: classwindow, classname và PID có đúng không
	phathien.Test_Launcher_Win_Class_PID = 0; // On/Off - Kiểm tra xem các giá trị Launcher: classwindow, classname và PID có chính xác không
	phathien.GG_Close_Game_immediately   = 0; // [alwais ON = 1]  1 = Nếu bất kỳ Gameguard nào dừng vì bất kỳ lý do gì, trò chơi sẽ đóng NGAY LẬP TỨC,  0 = Hiển thị thông báo (chỉ dành cho người dùng cấu hình!)
	
	//////////////////////////////////////////////////////////////////////////
	//Anti-hack Detectors ////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////     
	phathien.TAM_DUNG_TAT_CA						= 0;     // [alwais OFF = 0] On/Off - =1; = TẠM DỪNG TẤT CẢ HOẠT ĐỘNG CỦA ANTIHACK (Chỉ sử dụng cho mục đích cấu hình)
//Hide tools detector
	phathien.Phathien_Hide_Toolz        			= 0;     // On/Off - Công cụ ẩn / Trình phát hiện CheatEngine - Bạn phải tự chịu rủi ro khi sử dụng!
	phathien.PHHide_Thoigian            			= 536;   // Xảy ra mỗi X mili giây. LƯU Ý! Nếu tính năng này hoạt động thì giá trị này không thể lớn hơn 1000 ]
//Dump Detector	
	phathien.Phathien_Dump							= 0;     // On/Off - Phát hiện entrypoint ứng dụng và 16 HexDump đầu tiên	1
	phathien.Disable_Windows_ASLR					= 0;	 // On/Off - Vô hiệu hóa windows ASLR cho TẤT CẢ người dùng máy tính (trên xp). [ LƯU Ý! Sử dụng có nguy cơ crash! ] Tính năng này cho phép kết xuất để đọc các bản hack được tạo bằng ASLR, mô tả: Ngẫu nhiên bố cục không gian địa chỉ (ASLR) là một quy trình bảo vệ bộ nhớ dành cho các hệ điều hành (OS) nhằm bảo vệ chống lại các cuộc tấn công tràn bộ đệm bằng cách chọn ngẫu nhiên vị trí nơi các tệp thực thi hệ thống được lưu trữ được nạp vào bộ nhớ.
	phathien.PHDump_Thoigian						= 547;   // Xảy ra mỗi X mili giây. LƯU Ý! Nếu tính năng này hoạt động thì giá trị này không thể lớn hơn 1000 ]
//Window name Detector
	phathien.Phathien_Window						= 0;     // On/Off - Đóng GAME khi phát hiện Hack WindowName	1
	phathien.Close_Window            				= 0;     // On/Off - Đóng cửa sổ HACK, nếu đóng cửa sổ HACK không thành công, nó sẽ thoát GAME. Sử dụng để đóng các ứng dụng "cụ thể". Điều đó có thể làm hỏng hệ thống của bạn.
	phathien.DWindow_Thoigian						= 536;   // Xảy ra mỗi X mili giây. LƯU Ý! Nếu tính năng này hoạt động thì giá trị này không thể lớn hơn 1000 ]
//Class name Detector	                 
	phathien.Phathien_ClassName						= 0;     // On/Off - Đóng GAME khi phát hiện Hack ClassName
	phathien.Close_ClassName         				= 0;     // On/Off - Đóng HACK ClassName, nếu tắt HACK ClassName không thành công, nó sẽ thoát GAME. Sử dụng để đóng các ứng dụng "đặc biệt". Điều này có thể làm hỏng hệ thống của bạn. Trình phát hiện này rất tốt để phát hiện hack tốc độ "chung" được tạo bằng VB. Ví dụ: ["WindowsForms10.Window.8.app.0xxxxxx]
	phathien.DClassName_Thoigian					= 538;   // Xảy ra mỗi X mili giây. LƯU Ý! Nếu tính năng này hoạt động thì giá trị này không thể lớn hơn 1000 ]
//PID Detector
	phathien.Phathien_PID			     			= 0;     // On/Off - Phát hiện ứng dụng theo {name.extension}	1
	phathien.DPID_Thoigian		     				= 556;   // Xảy ra mỗi X mili giây. LƯU Ý! Nếu tính năng này hoạt động thì giá trị này không thể lớn hơn 1000 ]
//Heuristic Detector by Window
	phathien.Phathien_TuKhoa_Win					= 0;     // On/Off -Phát hiện các từ xấu, phân biệt chữ hoa chữ thường và quét trình duyệt	1
	phathien.Phathien_TuKhoa_Win_Thoigian			= 526;   // Xảy ra mỗi X mili giây. LƯU Ý! Nếu tính năng này hoạt động thì giá trị này không thể lớn hơn 1000 ]
//Heuristic Detector by Classname
	phathien.Phathien_TuKhoa_Class					= 0;     // On/Off -Phát hiện các từ xấu, phân biệt chữ hoa chữ thường và quét trình duyệt
	phathien.Phathien_TuKhoa_Class_Thoigian			= 546;   // Xảy ra mỗi X mili giây. LƯU Ý! Nếu tính năng này hoạt động thì giá trị này không thể lớn hơn 1000 ]
//Heuristic Detector generic
	phathien.Phathien_TuKhoa						= 0;     // On/Off -Phát hiện các từ xấu, phân biệt chữ hoa chữ thường và quét trình duyệt
	phathien.Phathien_TuKhoa_Thoigian				= 536;   // Xảy ra mỗi X mili giây. LƯU Ý! Nếu tính năng này hoạt động thì giá trị này không thể lớn hơn 1000 ]
//Heuristic Detector Non-Case sensitive	
	phathien.Phathien_TuKhoa_NC						= 0;     // On/Off - Phát hiện các từ xấu, KHÔNG phân biệt chữ hoa chữ thường và KHÔNG quét trình duyệt web
	phathien.Phathien_TuKhoa_NC_Thoigian			= 545;   // Xảy ra mỗi X mili giây. LƯU Ý! Nếu tính năng này hoạt động thì giá trị này không thể lớn hơn 1000 ]
//Speed Detector by Tickcount
	phathien.Phathien_Speed_Tickcount				= 0;     // On/Off - Hack tốc độ và phát hiện đóng băng (tạm dừng) hệ thống
	phathien.Phathien_Speed_Thoigian				= 500;   // [ LƯU Ý! ] Không thay đổi số này!
//Speed Detector by Performance
	phathien.Phathien_Speed_Hethong					= 0;     // On/Off - Tốc độ hệ thống của máy người chơi (có bất thường hay không)
	phathien.Phathien_Speed_Hethong_Thoigian		= 559;   // Xảy ra mỗi X mili giây. LƯU Ý! Nếu tính năng này hoạt động thì giá trị này không thể lớn hơn 1000 ]


	//////////////////////////////////////////////////////////////////////////
	//phát hiện Inject DLL ///////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	phathien.Phathien_Inject						= 0;     // On/Off - Phát hiện dll tiêm (bởi dll {name.extension})	1
	phathien.Phathien_Inject_Thoigian				= 556;   // Xảy ra mỗi X mili giây. LƯU Ý! Nếu tính năng này hoạt động thì giá trị này không thể lớn hơn 1000 ]

	
	//////////////////////////////////////////////////////////////////////////
	// Anti-kill /////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////// 

	//Anti KILL bằng số lần quét hoạt động [LƯU Ý!] đôi khi AKS-Scan có thể có [ExitProcess] do Internet bị trễ trên >> (BAN SERVER / BAN HWID) (Hoạt động không ổn định)
	phathien.Anti_Kill_Scans						= 0;     // On/Off - Phát hiện Quét, điều này hoàn toàn dựa trên thời gian xảy ra, vì vậy, đừng thay đổi bất kỳ cài đặt thời gian nào nếu nó BẬT!
	phathien.Anti_Kill_Thoigian						= 2500;  // [LƯU Ý!] Giá trị này cần gấp 2 lần so với bình thường
		

	//////////////////////////////////////////////////////////////////////////
	//virtual memorty protection /////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////	
	
	//Virtual Memory Protection
	phathien.Virtual_Memory_Protect					= 0;     // On/Off - bảo vệ mã khỏi quá trình tự động hóa trên bộ nhớ ảo (cần được định cấu hình) trên [Virtual_Memory.cpp]
	phathien.Memory_Buffer_P						= 0;     // On/Off - [CẤU HÌNH!] Hộp thông báo có bộ đệm địa chỉ để cấu hình! Cần cấu hình các giá trị trên: Phathien_Virtual_Memory.cpp
	phathien.Virtual_Mem_occours					= 735;   // Xảy ra mỗi X mili giây. LƯU Ý! Nếu tính năng này hoạt động thì giá trị này không thể lớn hơn 1000 ]

	//Xây dựng lại dữ liệu Gunz.exe
	phathien.Rebuild_Main_Data						= 0;    // On/Off - Cần cấu hình Rebuild_Main_Data.cpp nếu không hệ thống sẽ gặp sự cố! (Lỗi thấy mẹ mà lười xóa nên để đó)
	
	
	//////////////////////////////////////////////////////////////////////////
	//Serial / version / ip configuration ////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//Load Data from dll
	phathien.phathien_serial_version				= 0;     // On/Off - Menset Ip/Serial/Phiên bản, không có X-Trap.dll Gunz.exe sẽ không thực hiện bất kỳ chức năng xác thực nào.
	phathien.Serial									= "Your_Serial_Here";	// (16) Your server serial
	phathien.Version								= "YourV";				// (5)  Your server version
    phathien.IP										= "103.207.37.150";	    //      Your server Ip
		
	
	//////////////////////////////////////////////////////////////////////////
	// Files CRC Check ///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	phathien.Ativa_CRC_Geral             = 1;  // [luôn BẬT = 1] = Cái này cho phép kiểm tra CRC cho tất cả các tệp được BẬT
	//////////////////////////////////////////////////////////////////////////

	phathien.CRC_Launcher			    = 0;          // On/Off = CRC Launcher
	phathien.CRC_LauncherEXE	    	= 0xc0f938ae; // Launcher

	phathien.CRC_ProtectBMP		    	= 1;          // On/Off = CRC ảnh Protect.bmd 
	phathien.CRC_Protect  				= 0x512e818a; //

	phathien.CRC_System					= 0;          // On/Off = CRC system.mrs 
	phathien.CRC_SystemMRS				= 0x0908aab1; // 

	phathien.CRC_Gunz					= 1;          // On/Off = CRC Gunz.exe
	phathien.CRC_GunzEXE                = 0xf70a9693; // Gunz.exe win10
	
	phathien.CRC_Client_MRS				= 0;          // On/Off = CRC tất cả các files MRS trong client
	phathien.CRC_Interface				= 0x5339965e;			// Interface.mrs
	phathien.CRC_Maps					= 0xc3e83648;			// Maps.mrs
	phathien.CRC_MapsChallenge			= 0x0847c665;			// MapsChallenge.mrs
	phathien.CRC_model					= 0x7cc0297c;			// model.mrs
	phathien.CRC_Quest					= 0x76487ad9;			// Quest.mrs
	phathien.CRC_sfx					= 0xb3b0cf70;			// sfx.mrs
	phathien.CRC_Sound					= 0x98fb5fca;           // Sound.mrs

	//////////////////////////////////////////////////////////////////////////
	// Load External files ///////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	phathien.LoadThemDLL    			= 0; // On/Off - chạy những dll cho phép (trong Xtrap folder), ví dụ discord.dll ...
    phathien.Ten_Dll		         	= "Xtrap/discord.dll"; //
}


	//////////////////////////////////////////////////////////////////////////
	// END OF X-TRAP CONFIGURATION ////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////


 
BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
	                        
	                          switch (ul_reason_for_call){
						      case DLL_PROCESS_ATTACH: m_DllModule = hModule;							 			  	
							  GasMask_5(); //API funciton

							  //Splash Screen /////////////////////////////////////////////////////////////	
							  if(phathien.Splash_Screen == 1)
							  {
							  SplashShow();
							  }	

							  //ESSENSSIALS /////////////////////////////////////////////////////////////							  
							  if(phathien.GameGuard_Exe_Protection || phathien.Inject_dll_Protection == 1)
							  {
                              LoadSingleInstance();  // Chỉ cho phép một phiên bản Gunz.exe						  
							  }							

							  if (phathien.Disable_Windows_ASLR == 1){
	                          Exists_Registry(); //ENABLE / DISABLE Để ghi vào windows registry					  
							  }
							  
							  if(phathien.Copy_exe_to_Windows_Startup == 1){			  
							  Exists_Startup(); //ENABLE / DISABLE để copy vào Windows startup folder
							  }				  							 
							  	
							   // CRC CHECK FOR ALL FILES
							  if(phathien.Ativa_CRC_Geral == 1) 
							  {							 			 	 
							  _beginthread( MainThread, 0, NULL  );
							  }				
							  
							  //GAME-GUARD ////////////////////////////////////////////////////////
							  if(phathien.GameGuard_Exe_Protection == 1)
							  {
							  GameGuard_Protection();
							  }

							  if(phathien.Inject_dll_Protection)
							  {
							  Dll_Protection();
							  DenyAdm();
							  }								 				 
							  
							  //TESTS ///////////////////////////////////////////////////////////////////////////
							  if(phathien.Test_Main_Win_Class_PID == 1)
							  {
							  CheckConfigFile();    // Kiểm tra xem tệp cấu hình cho tất cả các loại phát hiện Gameguard có ổn không!
							  }	

							  if(phathien.Test_Launcher_Win_Class_PID == 1)
							  {
							  CheckConfigFileL();    // Kiểm tra xem tệp cấu hình cho tất cả các loại phát hiện Gameguard có ổn không!
							  }			

							  if(phathien.TESTE == 1)
							  {
							  TESTES();
							  }	
							 	   	
							  //BAN ///////////////////////////////////////////////////////////////////////////								 
							  //if(phathien.Hack_Log_Upload == 1)
							  //{
							  //Hack_Log_Up();
							  //}	
							  //
							  //if(phathien.Hardware_Ban == 1)
							  //{
							  //Mac_Load();
							  //}	

							  //if(phathien.Server_Ban == 1 && phathien.Anti_Kill_Scans == 0) 
							  //{
							  //HTTP_Connect(); //Đi thẳng tới BAN SERVER, tạo chủ đề mới. (cái này bật thì bị phát hiện virus ?????? chắc do giao thức HTTP)
							  //} 		
							  
							  //ANTI-HACK  ///////////////////////////////////////////////////////////////////////////
							  if(phathien.TAM_DUNG_TAT_CA == 1) 
							  {	
							  cout <<"[START.cpp > phathien.TAM_DUNG_TAT_CA] Turned ON!"<<endl;
							  cout <<"All Anti-hack and anti-kill PAUSED!"<<endl;
							  if (phathien.Log_Txt_Hack == 1){	
     ofstream out("Xtrap/Log.txt", ios::app);
     out <<"[START.cpp > phathien.TAM_DUNG_TAT_CA] Turned ON! All Anti-hack and anti-kill PAUSED!";
	out.close();
	}
	// if (phathien.Hack_Log_Upload == 1){
 //time_t rawtime;
 //struct tm * timeinfo;
 //time (&rawtime);
 //timeinfo = localtime (&rawtime);
 //    ofstream out("Log", ios::app);
	// out <<"\nLocal Time: ", out << asctime(timeinfo);
 //    out <<"[START.cpp > phathien.TAM_DUNG_TAT_CA] Turned ON! All Anti-hack detectors and anti-kill by scan are PAUSED!";
	// out << "\n= = = = = = = = = = = = = = = = = = =";
	// out.close();
 //SetFileAttributes("Log", FILE_ATTRIBUTE_HIDDEN); // Set file as a HIDDEN file	 
	//                          }
							  goto Pause_Detectors;  //START PAUSE ALL DETECTORS 
							  } 							  
							  
							  if(phathien.Check_File_Names == 1)
							  {
							  Nomes();
							  }	 
							  							    							  				  							
							  if(phathien.Launcher_Check == 1)
							  {
							  LA_On_byName();
							  }

							  if(phathien.Phathien_ClassName == 1)
							  {
							  CNProtection();
							  }	
							  
							  if(phathien.Close_ClassName == 1)
							  {
							  Close_Class();
							  }

							  if(phathien.Phathien_Window == 1)
							  {
							  DwProtection();
							  }

							  if(phathien.Close_Window == 1)
							  {
							  Close_Window();
							  }					  
							  
							  if(phathien.Phathien_TuKhoa_NC == 1)
							  {
							  Heuristica2();
							  }

							  if(phathien.Phathien_TuKhoa == 1)
							  {
							  HProtection();
							  }

							  if(phathien.Phathien_TuKhoa_Win == 1)
							  {
							  HeuristicW();
							  }

							  if(phathien.Phathien_TuKhoa_Class == 1)
							  {
							  Heuristicclass();
							  }

							  if(phathien.Phathien_Dump == 1)
							  {
							  ProtectionDump();
							  }

							  if(phathien.Phathien_PID == 1)
							  {
							  DetectID();
							  }					  
							  				  				  		
							  if(phathien.Phathien_Inject == 1)		                    	
		                      {
	                          Dll_Inject();
	                          }								
							  
							  if(phathien.Phathien_Hide_Toolz == 1)
							  {
							  Hide_T();
							  } 

							  if(phathien.Phathien_Speed_Hethong  == 1)
							  {
							  SPerformance();
							  }

							  if(phathien.Phathien_Speed_Tickcount == 1)
							  {
							  DetectSpeed();
							  }
							  Pause_Detectors:  //END PAUSE ALL 
		  
							  // MEMORY ////////////////////////////////////////////////////								  							  													 
							  if(phathien.phathien_serial_version == 1)
							  {
							  Serial_e_Etc();
							  }	

							  if(phathien.Virtual_Memory_Protect == 1)
							  {
							  Virtual_Attack();
							  }	

							  if(phathien.Rebuild_Main_Data == 1)
							  {
							  RebuildData();
							  }				

							  //ANTI-KILL ////////////////////////////////////////////////////
							  if(phathien.Anti_Kill_Scans == 1 && phathien.TAM_DUNG_TAT_CA == 0 )
							  {
							  Anti_kill1();
							  //Anti_kill2();
							  }

							  //LOAD DLL FILES ////////////////////////////////////////////////////
							  if(phathien.LoadThemDLL == 1)
							  {
							  LoadLibraryA(phathien.Ten_Dll);
							  }			
							 
						  break;
						  case DLL_THREAD_ATTACH:
						  case DLL_THREAD_DETACH:							
						  case DLL_PROCESS_DETACH:		
						  break;
						  }	
						  return TRUE;						
}
					
					





