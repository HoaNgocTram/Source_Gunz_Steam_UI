
#include "stdafx.h"
#include "Splash.h"


void SplashShow(){
    CSplash splash1(TEXT(".\\Xtrap/Protect.bmp"), RGB(128, 128, 128)); //".\\Xtrap/Protect.bmp"
    splash1.ShowSplash();
    Sleep(3000);
    splash1.CloseSplash();
}


