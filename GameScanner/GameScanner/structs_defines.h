
#ifndef __ETSV__
#define __ETSV__
#include <vector>
#include <string>
#define _DEFINE_DEPRECATED_HASH_CLASSES 0
#include <hash_map>
#include <Winsock2.h.>
#include "resource.h"


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

#define MAX_SERVERLIST		16  //This value should be as the last one +1
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
#define FILTER_UNKNOWN	 0
#define FILTER_PB			 1  //Punkbuster
#define FILTER_SHOW_PRIVATE		 2  //Private
#define FILTER_FULL			 4  //Full servers
#define FILTER_EMPTY		 8  //Empty
#define FILTER_OFFLINE		16  //Offline
#define FILTER_FAVORITERS		32  //Favorites
#define FILTER_BOTS				64  //Bots
#define FILTER_HIDE_PRIVATE		128  
#define FILTER_MOD				24
#define FILTER_GAMETYPE			25
#define FILTER_PURE				26
#define FILTER_RANKED			27
#define FILTER_PING				28
#define FILTER_VERSION			29
#define FILTER_MAP				30
#define FILTER_MIN_PLY			100
#define FILTER_MAX_PLY			101


#define FILTER_2	   256	
#define FILTER_3	   512  
#define FILTER_4		  1024  
#define FILTER_5	  2048  
#define FILTER_6		  4096  
#define FILTER_7	  8192   
#define FILTER_8	  16384  
#define FILTER_9	  32768  
#define FILTER_10	  65535  

#define MOD_ET_UNKNOWN		 0
#define MOD_ET_ETMAIN		 1  
#define MOD_ET_ETPRO		 2  
#define MOD_ET_JAYMOD		 4  
#define MOD_ET_NOQUARTER	 8  
#define MOD_ET_ETPUB		16  
#define MOD_ET_TCETEST		32


#define MOD_ETQW_UNKNOWN	 0
#define MOD_ETQW_MAIN		 1  
#define MOD_ETQW_PRO		 2  
#define MOD_ETQW_WOW		 4  


#define MOD_COD4_UNKNOWN	 0
#define MOD_COD4_MAIN		 1  
#define MOD_COD4_ZOMBIE		 2  
#define MOD_COD4_PAM		 4  
#define MOD_COD4_WARFARE	 8 
#define MOD_COD4_AWE		 16

#define VERSION_UNKNOWN      0
#define VER_ET_UNKNOWN		 0
#define VER_ET_255			 1  
#define VER_ET_260			 2  
#define VER_ET_260B			 4  


#define GAMETYPE_UNKNOWN	 0
#define GAMETYPE_DM			 1  //free for all deatch match 
#define GAMETYPE_SW			 2  //StopWatch
#define GAMETYPE_LMS		 4  //Last Man Standing
#define GAMETYPE_TDM		 8  //Team deatch match
#define GAMETYPE_OBJ		16  //Objective
#define GAMETYPE_CAMP		32  //Campaign
#define GAMETYPE_CTF		64  //Capture the Flag
#define GAMETYPE_CA		   128  //Clan Arena
#define GAMETYPE_DUEL	   256	//Duel
#define GAMETYPE_FFA	   512  //Free for all
#define GAMETYPE_SD		  1024  //Cod2 Search & Destroy
#define GAMETYPE_ZOM	  2048  //cod
#define GAMETYPE_HQ		  4096  //CoD 2 & CoD 4 King of The Hill / HeadQuarters??
#define GAMETYPE_unused	  8192   //
#define GAMETYPE_SAB	  16384  //CoD 4 Sabotage
#define GAMETYPE_DOM	  32768  //CoD 4 Domination
#define GAMETYPE_OTHER	  65535  //All Others :)


#define GAMETYPE_Q3_DM		 1  //Free for all / Deatch Match 
#define GAMETYPE_Q3_FFA		 2  //Free for all
#define GAMETYPE_Q3_CTF		 4  //Capture the flag
#define GAMETYPE_Q3_TDM		 8  //Team deatch match
#define GAMETYPE_Q3_CA		16  //Clan Arean
#define GAMETYPE_Q3_DUEL	32  //Duel



#define GAMETYPE_ET_UNUSED1		 1  //??
#define GAMETYPE_ET_SW			 2  //StopWatch
#define GAMETYPE_ET_LMS			 4  //Last Man Standing
#define GAMETYPE_ET_OBJ			 8  //Objective
#define GAMETYPE_ET_CAMP		16  //Campaign


#define GAMETYPE_COD_DUEL		  256	//Duel
#define GAMETYPE_COD_FFA		  512  //Free for all
#define GAMETYPE_COD_SD			  1024  //Cod2 Search & Destroy
#define GAMETYPE_COD_ZOM		  2048  //cod
#define GAMETYPE_COD_HQ			  4096  //CoD 2 & CoD 4 King of The Hill / HeadQuarters??
#define GAMETYPE_COD_UNUSED1	  8192   //
#define GAMETYPE_COD_SAB		  16384  //CoD 4 Sabotage
#define GAMETYPE_COD_DOM		  32768  //CoD 4 Domination



struct GAMEFILTER{
	string sFriendlyName;
	string sStrValue;
	DWORD dwValue;
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
	DWORD dwPing;
	DWORD dwGameTypeFilter;
	DWORD dwMod;
	DWORD dwShowServerWithMaxPlayers;
	DWORD dwShowServerWithMinPlayers;
	DWORD dwVersion;
	DWORD dwMap;
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
   BOOL bUse_ETpro_path;
   DWORD dwMinimizeMODKey;
   char cMinimizeKey;
   BOOL bViewModeAdvance;
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
   BOOL bUseFilterOnFavorites;
   BOOL bUseFilterOnPing;
   char szNotifySoundWAVfile[MAX_PATH];
   char szQ4_PATH[MAX_PATH];
   char szQ4_CMD[MAX_PATH];
   char szETQW_PATH[MAX_PATH];
   char szETQW_CMD[MAX_PATH];
   BOOL bUseCountryFilter;
   BOOL bUseMIRC;
   DWORD dwRetries;
   DWORD dwThreads;
	
};




struct PLAYERDATA
{
	
	char iPlayer;  //Player number or index
	DWORD ping;
	DWORD rate;
	char *szPlayerName;
	char *szClanTag;
	BYTE ClanTagPos;
	BYTE isBot;
	//SERVER_INFO  *pCurrentServer;
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
	DWORD dwPing;
	DWORD dwAvgPing;	
	char bPrivate;
	char bPunkbuster;
	char cPurge;          //Purge counter
	char bNeedToUpdateServerInfo;
	char bUpdated;		
	char szRCONPASS[MAX_RCON_LEN];
	char szPRIVATEPASS[MAX_PASSWORD_LEN];
	char nPrivateClients;
	char cCountryFlag;	
	char cFavorite;  
	char cHistory;  //History.... connected to this server.
	char cGAMEINDEX;  //RTCW, ET, Quake 4....
	char cLAN;
	DWORD cGameTypeCVAR;
	char szGameTypeName[MAX_GAMETYPENAME_LEN];
	BOOL bDedicated;
	char cPure;
	char cLocked;
	char cRanked;
	char cBots;
	char szShortCountryName[4];	
	char bHide;  //Hide from the server list this will be set to tru when it didn't pass the filter test
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



/*
Reference server info, this will only point to the full server info container 
using Gametype var and index combined.
*/
typedef vector<REF_SERVER_INFO> vREF_SRV_INF;
typedef vector<REF_SERVER_INFO> vREF_SRV_INF;
typedef pair <int, int> Int_Pair;
typedef hash_multimap <int, int> serverhash;
// create an empty hash_multimap hmp0 of key type integer


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
};


struct GAME_INFO
{
	char cGAMEINDEX;
	//SERVER_INFO *pStartServerInfo;
	char szGAME_NAME[MAX_PATH];
	char szGAME_PATH[MAX_PATH];
	char szGAME_CMD[MAX_PATH];
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
	SERVER_CONTAINER *pSC;
	DWORD dwScanIdx;
	char szServerRequestInfo[30]; //0xFF 0xFF etc getInfo
	FILTER_SETTINGS filter;
	

};


struct PLAYER_DATA
{
	char szPlayerName[MAX_NAME_LEN];
	char szPlayerNameFilter[MAX_NAME_LEN];
	char szScore[12];
	char szPing[10];
	char szTeam[25];
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
