#ifndef _MAIN_H
#define _MAIN_H

#include "../CSCommon/MFeatureDefine.h"
#pragma warning(disable:4099)

#ifdef _DEBUG

#pragma comment(lib,"cmld.lib")
#pragma comment(lib,"mint2d.lib")
#pragma comment(lib,"RealSoundd.lib")
#pragma comment(lib,"RealSpace2d.lib")
#pragma comment(lib,"CSCommond.lib")
#pragma comment(lib,"SafeUDPd.lib")
#pragma comment(lib,"dxerr.lib")

#else // Release

#ifdef _M_X64
#pragma comment(lib,"cml64.lib")
#pragma comment(lib,"Mint264.lib")
#pragma comment(lib,"RealSound64.lib")
#pragma comment(lib,"RealSpace2.lib")
#pragma comment(lib,"CSCommonE.lib")
#pragma comment(lib,"SafeUDP64.lib")
#else
#pragma comment(lib,"cml.lib")
#pragma comment(lib,"mint2.lib")
#pragma comment(lib,"RealSound.lib")
#pragma comment(lib,"RealSpace2.lib")
#pragma comment(lib,"CSCommon.lib")
#pragma comment(lib,"SafeUDP.lib")
#endif
#pragma comment(lib,"dxerr.lib")

#endif // _DEBUG

// Voice chat libs
#ifdef _VOICE_CHAT
#pragma comment(lib,"portaudio.lib")
#pragma comment(lib,"opus.lib")
#ifndef _M_X64
#pragma comment(lib,"celt.lib")
#pragma comment(lib,"silk_common.lib")
#pragma comment(lib,"silk_fixed.lib")
#pragma comment(lib,"silk_float.lib")
#endif
#endif

// System libs
#pragma comment(lib,"winmm.lib")

#endif // _MAIN_H