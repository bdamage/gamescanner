// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#define  APP_VERSION "1.37"

#pragma once
#define _SECURE_SCL 0
#define _SECURE_SCL_THROWS 1


#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
//#define _WIN32_WINNT 0x0600	// Change this to the appropriate value to target other versions of Windows.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later. 
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#include "targetver.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <winuser.h>
#include <iostream>
#include <commctrl.h>
#include <Commdlg.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <shellapi.h>
#include <direct.h>
#include <string>
#include <vector>
#include <queue>
#include <deque>
#include <UxTheme.h>
//#include <tmschema.h>  
#include <vssym32.h>		//Themes
#include <algorithm>
#include <strsafe.h>
#include <functional>
#include <Ddeml.h>
#include <shlobj.h>
#include <string>
#include <iostream>  //for "cerr"
#include <objbase.h>
#include <ole2.h>
#include <iphlpapi.h>
#include <icmpapi.h>  //ping
#include <Mmsystem.h>

#include "structs_defines.h"
#include "utilz.h"
#include "q3.h"
#include "q4.h"
#include "unreal.h"

#include "Xmlconfig.h"
#include "language.h"

#include "..\..\HttpFileDownload\HttpFileDownload.h"
#include "..\..\IPtoCountry\IPtoCountry.h"
#include "..\..\FreeImage\freeimage.h"
#include "..\..\hashlib\src\hashlibpp.h"
#include <tinyxml.h>

// TODO: reference additional headers your program requires here

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
