// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef _STDAFX_H
#define _STDAFX_H

#define _KILLFEED 1
#define _CHATBACKGROUND 1

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#undef GetClassName
#undef CreateEvent
#undef GetUserName
#undef GetObject
#undef MoveFile
#undef DeleteFile
#undef CreateFile
#undef CreateDirectory

// ¿©±â´Ù include
#include "winsock2.h"
#include "MWidget.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "rapidxml_print.hpp"
#endif