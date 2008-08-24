
#ifndef __ETSV__
#define __ETSV__
#include <vector>
#include <string>
#define _DEFINE_DEPRECATED_HASH_CLASSES 0
#include <hash_map>
#include <Winsock2.h.>
#include "resource.h"

#pragma pack(1)

using namespace stdext;
using namespace std;

#define MAX_VAR_LEN 100
#define MAX_NAME_LEN 100
#define MAX_IP_LEN 80
#define MAX_VAR MAX_VAR_LEN
#define MAX_COUNTRYNAME_LEN 50
#define MAX_MODNAME_LEN 25
#define MAX_MAPNAME_LEN 40
#define MAX_RCON_LEN 40
#define MAX_PASSWORD_LEN 40
#define MAX_VERSION_LEN 50
#define MAX_GAMETYPENAME_LEN 30

#define MAX_BUDDYNAME_LEN 40

#define	WM_USER_SHELLICON		WM_USER + 1
#define SHELL_CALLBACK			WM_USER+14
#define WM_REFRESHSERVERLIST	WM_USER+15
#define WM_INITVIEWS			WM_USER+16
#define WM_START_PING			WM_USER+17
#define WM_STOP_PING			WM_USER+18
#define WM_GETSERVERLIST_START			WM_USER+19
#define WM_GETSERVERLIST_STOP			WM_USER+20
#define WM_UPDATESERVERLIST_COLUMNS		WM_USER+21
#define WM_START_TRACERT		WM_USER+22

#define WIN_MAINTREEVIEW	0
#define WIN_SERVERLIST		1
#define WIN_BUDDYLIST		2
#define WIN_TABCONTROL		3
#define WIN_MAPPREVIEW		4
#define WIN_PLAYERS			5
#define WIN_RCON			6
#define WIN_RULES			7
#define WIN_LOGGER			8
#define WIN_STATUS			9
#define WIN_PROGRESSBAR		10
#define WIN_PING			11

#define ICO_INFO		8
#define ICO_WARNING		6
#define ICO_EMPTY		-1

#define TOOLBAR_Y_OFFSET 35
#define STATUSBAR_Y_OFFSET 25
#define TABSIZE_Y 22

//#define MAX_VAR 80
//#define MAX_ONLINE_USER 60
#define HOTKEY_ID 40022
#define HOTKEY_ID_CTRL_C 40099

#define WS_EX_LAYERED	0x00080000
#define LWA_COLORKEY	0x00000001
#define LWA_ALPHA		0x00000002
#define ULW_COLORKEY    0x00000001
#define ULW_ALPHA		0x00000002
#define ULW_OPAQUE      0x00000004

typedef BOOL(WINAPI *SLWA)(HWND, COLORREF, BYTE, DWORD);

#define MIN_TRANSPARANCY 30

#define FAVORITE			998
#define PRIVATE				999

#define REDRAWLIST_FAVORITES_PUBLIC		0x00000001
#define REDRAWLIST_FAVORITES_PRIVATE	0x00000002
#define REDRAWLIST_HISTORY				0x00000004
#define FORCE_SCAN_FILTERED				0x00000008
#define REDRAW_SERVERLIST				16

#define MAX_SERVERLIST		17  //This value should be as the last one +1
#define ET_SERVERLIST		0
#define ETQW_SERVERLIST		1
#define Q3_SERVERLIST		2
#define Q4_SERVERLIST		3
#define RTCW_SERVERLIST		4
#define COD_SERVERLIST		5
#define COD2_SERVERLIST		6
#define WARSOW_SERVERLIST	7
#define COD4_SERVERLIST		8
#define CS_SERVERLIST		9
#define CSCZ_SERVERLIST		10
#define CSS_SERVERLIST		11
#define QW_SERVERLIST		12
#define Q2_SERVERLIST		13
#define OPENARENA_SERVERLIST 14
#define HL2_SERVERLIST		15			//Half-Life 2
#define UTERROR_SERVERLIST  16


#define REDRAW_CURRENT_LIST				100
#define SHOW_FAVORITES_PUBLIC			101
#define SHOW_FAVORITES_PRIVATE			102
#define SHOW_HISTORY					103

#define DO_NOTHING				999
#define DO_NOTHING_				0
#define DO_REDRAW_SERVERLIST	1
#define DO_CHECKBOX				2
#define DO_EDIT					3
#define DO_FAV_NO_FILT			4
#define DO_FAV_PUB				5
#define DO_FAV_PRIV				6
#define DO_HISTORY				7
#define DO_GLOBAL_FILTER		8
#define DO_GAME_SPECIFIC_FILTER		9
#define DO_COUNTRY_FILTER		10
#define DO_GLOBAL_EDIT_FILTER	11

//Generic filter defines
#define FILTER_UNKNOWN			0
#define FILTER_PB				1  //Punkbuster
#define FILTER_SHOW_PRIVATE		 2  //Private
#define FILTER_FULL				 4  //Full servers
#define FILTER_EMPTY			8  //Empty
#define FILTER_OFFLINE			16  //Offline
#define FILTER_BOTS				64  //Bots
#define FILTER_HIDE_PRIVATE		128  
#define FILTER_MOD				24
#define FILTER_GAMETYPE			25
#define FILTER_PURE				26
#define FILTER_RANKED			27
#define FILTER_PING				28
#define FILTER_VERSION			29
#define FILTER_MAP				30
#define FILTER_REGION			31  //Used for steam/valve stuff
#define FILTER_FAVORITERS		32  //Favorites
#define FILTER_DEDICATED		33
#define FILTER_MIN_PLY			100
#define FILTER_MAX_PLY			101

#define FILTER_REGION_US_EAST   1
#define FILTER_REGION_US_WEST   2
#define FILTER_REGION_SOUTH_AMERICA   4
#define FILTER_REGION_EUROPE		8
#define FILTER_REGION_ASIA			16
#define FILTER_REGION_AUSTRALIA		32
#define FILTER_REGION_MIDDLE_EAST   64
#define FILTER_REGION_AFRICA0		128
#define FILTER_REGION_ROW			256

#define VERSION_UNKNOWN      0

#define GAMETYPE_UNKNOWN	 0

struct GAMEFILTER{
	string sFriendlyName;
	string sStrValue;
	DWORD dwValue;
	DWORD dwExactMatch;
};


struct RCON_DATA
{
	char szIP[40];
	DWORD dwPort;
	char szPassword[40];
};

struct FILTER_SETTINGS
{
	BOOL bNoFull;
	BOOL bNoPrivate;
	BOOL bOnlyPrivate;
	BOOL bNoEmpty;
	BOOL bPunkbuster;
	BOOL bHideOfflineServers;
	BOOL bRanked;
	BOOL bPure;
	BOOL bNoBots;
	BOOL bDedicated;
	DWORD dwPing;
	DWORD dwGameTypeFilter;
	DWORD dwMod;
	DWORD dwShowServerWithMaxPlayers;
	DWORD dwShowServerWithMinPlayers;
	DWORD dwVersion;
	DWORD dwMap;
	DWORD dwRegion;
	char cActiveMaxPlayer;
	char cActiveMinPlayer;
};

struct COUNTRY_SAVA_DATA
{
	char szCountry[50];
	int bActive;
};


struct APP_SETTINGS_NEW
{
   DWORD dwID; 
   DWORD dwVersion;
   char szET_PATH[MAX_PATH];
   char szET_CMD[MAX_PATH];
   char szEXT_EXE_PATH[MAX_PATH];
   char szEXT_EXE_CMD[MAX_PATH];
   char szEXT_EXE_WINDOWNAME[MAX_PATH];
   BOOL bAutostart;
   BOOL bUseShortCountry;
   BOOL bUse_minimize;
   DWORD dwMinimizeMODKey;
   char cMinimizeKey;
   BOOL bEXTMinimize;
   BOOL bEXTClose;
   char szET_WindowName[MAX_PATH];
   char szMasterServer[128];
   DWORD dwMasterServerPORT;
   char szMasterServerPROTOCOL[4];
   BOOL bLogging;
   BOOL bUse_EXT_APP;
   BOOL bUSE_SCREEN_RESTORE;
   RCON_DATA rconData;
   FILTER_SETTINGS filter;
   FILTER_SETTINGS filterFavorites;
   FILTER_SETTINGS filterMaster;
   char g_cTransparancy;
   struct timeval socktimeout;  //not yet implemented see below
   BOOL bShowBuddyList;
   BOOL bShowMapPreview;
   BOOL bShowRCON;
   BOOL bShowServerRules;
   BOOL bShowPlayerList;
   BOOL bShowServerList;
   BOOL bPlayNotifySound;
   BOOL bSortBuddyAsc;
   BOOL bSortPlayerAsc;
   BOOL bUseColorEncodedFont;
   char cPlayerColumnSort;
   char cBuddyColumnSort;
   char cServerColumnSort;
   char szNotifySoundWAVfile[MAX_PATH];
   char szQ4_PATH[MAX_PATH];
   char szQ4_CMD[MAX_PATH];
   char szETQW_PATH[MAX_PATH];
   char szETQW_CMD[MAX_PATH];
   BOOL bUseCountryFilter;
   BOOL bUseMIRC;
   DWORD dwRetries;
   DWORD dwThreads;
   char szLanguageFilename[MAX_PATH];

};




struct PLAYERDATA
{
	
	char iPlayer;  //Player number or index
	DWORD ping;
	DWORD rate;
	DWORD time;
	DWORD dwServerIndex;
	char *szPlayerName;
	char *szClanTag;
	BYTE ClanTagPos;
	BYTE isBot;
	char cGAMEINDEX;
	char szServerName[MAX_NAME_LEN];
	PLAYERDATA *pNext;
};

typedef PLAYERDATA* LPPLAYERDATA;

struct SERVER_RULES
{
	char *name;
	char *value;
	SERVER_RULES *pNext;
};
typedef SERVER_RULES* LPSERVER_RULES;
struct SERVER_INFO
{	
	DWORD dwIndex;
	DWORD dwLVIndex; //index of the ListView for on screen update.
	char szServerName[MAX_NAME_LEN];
	char szIPaddress[MAX_IP_LEN];
	DWORD dwIP;
	DWORD dwPort;
	char szMap[MAX_MAPNAME_LEN];
	DWORD dwMap;
	int nCurrentPlayers;
	int nMaxPlayers;	
	char szCountry[MAX_COUNTRYNAME_LEN];
	char szMod[MAX_MODNAME_LEN];
	DWORD wMod;						//Used for faster filtering
	char szVersion[MAX_VERSION_LEN];
	DWORD dwVersion;                  //Used for faster filtering
	DWORD dwGameType;
	DWORD dwPing;
	DWORD dwAvgPing;	
	char bPrivate;
	char bPunkbuster;
	BYTE cPurge;          //Purge counter
	char bNeedToUpdateServerInfo;
	char bUpdated;		
	BYTE nPrivateClients;
	char cCountryFlag;	
	BYTE cFavorite;  
	BYTE cHistory;  //History.... connected to this server.
	BYTE cGAMEINDEX;  //RTCW, ET, Quake 4....
	char cLAN;
	char szGameTypeName[MAX_GAMETYPENAME_LEN];
	BYTE bDedicated;
	char cPure;
	char cLocked;
	char cRanked;
	BYTE cBots;
	char szShortCountryName[3];	
	char szRCONPASS[MAX_RCON_LEN];
	char szPRIVATEPASS[MAX_PASSWORD_LEN];
	char szSTATUS[40];

	PLAYERDATA *pPlayerData;
	SERVER_RULES *pServerRules;
};
typedef SERVER_INFO* LPSERVERINFO;

struct _WINDOW_CONTAINER{
	int idx;
	BOOL bShow;
	HWND hWnd;
	RECT rSize;
	RECT rMinSize;
};
typedef _WINDOW_CONTAINER * LPWNDCONT;

typedef vector<SERVER_INFO> vSRV_INF;

typedef vector<GAMEFILTER> vGF;

//For virtual list
struct REF_SERVER_INFO
{	
	DWORD dwIndex;
	char cGAMEINDEX;  //RTCW, ET, Quake 4....
};
typedef REF_SERVER_INFO* LPREFSERVER_INF;


struct GAME_INSTALLATIONS
{
	string sName;
	string szGAME_PATH;
	string szGAME_CMD;
	string sVersion;
	string sMod;
};


/*
Reference server info, this will only point to the full server info container 
using Gametype var and index combined.
*/
typedef vector<REF_SERVER_INFO> vREF_SRV_INF;
typedef vector<REF_SERVER_INFO> vREF_SRV_INF;
typedef pair <int, int> Int_Pair;
typedef hash_multimap <int, int> serverhash;
// create an empty hash_multimap hmp0 of key type integer
typedef vector<GAME_INSTALLATIONS> vGAME_INSTALLS;

struct SERVER_CONTAINER
{
	vSRV_INF vSI;
	vREF_SRV_INF vRefListSI;
	vREF_SRV_INF vRefScanSI;
	serverhash shash;
	vGF	 vFilterMod;
	vGF	 vFilterGameType;
	vGF	 vFilterMap;
	vGF	 vFilterVersion;
	vGAME_INSTALLS vGAME_INST;
};

struct GAME_INFO
{
	char cGAMEINDEX;
	//SERVER_INFO *pStartServerInfo;
	char szGAME_NAME[MAX_PATH];
	char szGAME_SHORTNAME[MAX_PATH];
	char szGAME_PATH[MAX_PATH];
	char szGAME_CMD[MAX_PATH*2];
	char szLaunchByVersion[MAX_PATH];
	char szLaunchByMod[MAX_PATH];
	char szMAP_MAPPREVIEW_PATH[MAX_PATH];
	char szMAP_YAWN_PATH[MAX_PATH];
	char szMAP_SPLATTERLADDER_PATH[MAX_PATH];
	char szMAP_OTHER_WEBPATH_PATH[MAX_PATH];   //ie ETQW stats webserver...	
	char szMasterServerIP[MAX_PATH];	
	char szProtocolName[5];
	char szGameTypeSearch[15];
	char szQueryString[MAX_PATH];
	BOOL bUseHTTPServerList;
	int bActive;
	DWORD dwViewFlags;
	DWORD dwMasterServerPORT;
	DWORD dwDefaultPort;
	DWORD dwProtocol;
	DWORD dwTotalServers;
	HTREEITEM hTI;
	int iIconIndex;
	char szFilename[MAX_PATH];
	DWORD dwGameTypeFilter;
	BOOL bLockServerList;
	SERVER_CONTAINER *pSC;
	DWORD dwScanIdx;
	char szServerRequestInfo[30]; //0xFF 0xFF etc getInfo
	FILTER_SETTINGS filter;
    char *(*colorfilter)(const char *szIn, char *szOut,int length);
	LRESULT (*Draw_ColorEncodedText)(RECT rc, LPNMLVCUSTOMDRAW pListDraw , char *pszText);
	time_t lastScanTimeStamp;

};




struct BUDDY_INFO
{
	char szPlayerName[MAX_NAME_LEN];
	char szClan[MAX_NAME_LEN];
	char szServerName[MAX_NAME_LEN];
	char szIPaddress[MAX_IP_LEN];
	int cMatchExact;
	int cMatchOnColorEncoded;
	char cGAMEINDEX;
	char cUnused1;
	int sIndex;
	bool bRemove;
	char szLastSeenServerName[MAX_NAME_LEN];
	char szLastSeenIPaddress[MAX_IP_LEN];
	SERVER_INFO *pSERVER;
	BUDDY_INFO *pNext;
};

typedef BUDDY_INFO* LPBUDDY_INFO;

struct _TREEITEM
{
	HTREEITEM hTreeItem;
	std::string sName;
	std::string sElementName;
	std::string strValue;
	DWORD dwValue;
	DWORD dwType;
	DWORD dwCompare;
	DWORD dwState;
	DWORD dwAction;
	DWORD dwLevel; //XML hierachy
	int iIconIndex;
	bool bExpanded;
	bool bVisible;
	DWORD dwIndex;
	GAME_INFO *pGI;
	char cGAMEINDEX;
};


struct _CUSTOM_COLUMN
{
	unsigned short id;
	std::string sName;
	LV_COLUMN   lvColumn;
	BOOL bActive;
	BOOL bSortAsc;
	short columnIdx;
	short columnIdxToSave;
};

inline bool operator == (SERVER_INFO pSIa, SERVER_INFO pSIb)
{
	return ((pSIa.dwIP == pSIb.dwIP) && (pSIa.dwPort == pSIb.dwPort));
}


#endif
