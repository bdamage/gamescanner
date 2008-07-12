// GameScanner.cpp : Defines the entry point for the application.
// Copyright (C) 2007, 2008 Kjell Lloyd 

/*
Version 1.0.0

Product code:
{C9C6D743-D7CF-46FD-9366-255F9DFD8442}

Upgrade code:
{1E1FC67E-A466-4A1F-A278-286B6905C57B}

*/

#include "stdafx.h"

#include "GameScanner.h"

#include "utilz.h"
#include "dde_wrapper.h"
#include "rcon.h"
#include "q3.h"
#include "q4.h"
#include "steam.h"
#include "scanner.h"
#include "buddy.h"

#include "CountryCodes.h"


#pragma comment(lib, "ole32.lib")

#ifdef _DEBUG
#pragma comment(lib, "tinyxmld.lib")
#else
#pragma comment(lib, "tinyxml.lib")
#endif

#pragma comment(lib, "HttpFileDownload.lib")
#pragma comment(lib, "iptocountry.lib")

#pragma comment(lib, "..\\..\\FreeImage\\FreeImage.lib")

#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Winmm.lib")  //Playsound
#pragma comment(lib, "Iphlpapi.lib")  //ICMP



#pragma warning( disable : 4244 ) 
#pragma warning( disable : 4018 ) 
#pragma warning( disable : 4800 ) 
#pragma warning( disable : 4996 ) 

#pragma warning(disable : 4267)
#pragma warning(disable : 4995)
#pragma warning(disable : 4002)

#pragma warning(disable : 4995)
#pragma warning(disable : 4996)

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE g_hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name


// Forward declarations of functions included in this code module:

BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

#define MAX_LOADSTRING 100

char TREEVIEW_VERSION[20];

#define ICON_HEIGHT 24
#define ICON_WIDTH	24

#define TIMER_EVENT		 1001
#define TIMER_EVENT_RSS	 1002

#define CFG_VER			0x3403   //Configure file version
#define SERVERLIST_VER	0x2002   //Configure file version
#define SERVERSLISTFILE_VER	0x1005   //Configure file version


#define ICO_INFO		8
#define ICO_WARNING		6
#define ICO_EMPTY		-1

#define TOOLBAR_Y_OFFSET 35
#define STATUSBAR_Y_OFFSET 25
#define TABSIZE_Y 22

#define SCAN_ALL		0
#define SCAN_FILTERED	1
#define SCAN_ALL_GAMES	2

#define ETSV_PURGE_COUNTER 5  //Counter after X timeouts to purge (delete) the server

#define REDRAWLIST_FAVORITES_PUBLIC		0x00000001
#define REDRAWLIST_FAVORITES_PRIVATE	0x00000002
#define REDRAWLIST_HISTORY				0x00000004
#define REDRAWLIST_SCAN_FILTERED		0x00000008

int g_currentGameIdx = ET_SERVERLIST;


/****************************************
	Buddy Global vars
*****************************************/
BUDDY_INFO *g_pBIStart=NULL;


DWORD g_tvIndex=0;

HIMAGELIST g_hILFlags = NULL;

BOOL g_bTREELOADED = FALSE;

#define COL_PB			0
#define COL_PRIVATE		1
#define COL_RANKED		2
#define COL_SERVERNAME	3
#define COL_VERSION		4
#define COL_GAMETYPE	5
#define COL_MAP			6
#define COL_MOD			7
#define COL_BOTS		8
#define COL_PLAYERS		9
#define COL_COUNTRY		10
#define COL_PING		11
#define COL_IP			12

//if you add something ensure to increase MAX_COLUMNS below
#define MAX_COLUMNS COL_IP+1


BOOL bSortColumnAscading[15];
int iLastColumnSortIndex = COL_PLAYERS;

extern BOOL g_bQ4;
extern BOOL SCANNER_bCloseApp;


bool bFirstTimeSizeCalc= true;
#define BORDER_SIZE 4

struct _WINDOW_CONTAINER{
	int idx;
	BOOL bShow;
	HWND hWnd;
	RECT rSize;
	RECT rMinSize;
};
typedef _WINDOW_CONTAINER * LPWNDCONT;
_WINDOW_CONTAINER WNDCONT[10];



#pragma comment( user, "Compiled on " __DATE__ " at " __TIME__ ) 

#ifndef _DEBUG
TCHAR szDialogTitle[]="Game Scanner v" APP_VERSION " beta 2 ";
#else
TCHAR szDialogTitle[]="Game Scanner v" APP_VERSION " beta 2 Compiled on " __DATE__ " at "__TIME__;
#endif


//HHOOK hhook = NULL;

using namespace std;

int ImageSizeX = 120;
//int oldmaxWidth=0;

HTREEITEM iSelected ;
TVITEM  tvi;

int tabViewMode = FAVORITE;
void GameTypeFilter_GetSelectedGameType(HTREEITEM hRoot);

HANDLE hCloseEvent; 

struct _splitter
{
	BOOL bHidden;
	BOOL bSizing;
	BOOL bMoveVertical;
	BOOL bMoveHorizontal;
	RECT hit;
	int tvXPos;
	int tvYPos;
};
typedef _splitter SPLITTER, *LPSPLITTER;

SPLITTER SplitterGripArea[5];

BOOL g_bIsVisible=FALSE;

DWORD g_ThreadCounter=0;
DWORD g_serverIndex=0;
DWORD g_serverIndexMAX=0;

DWORD g_nCurrentSortMode = COL_PLAYERS;
LONG_PTR g_wpOrigTreeViewProc=NULL;
LONG_PTR g_wpOrigListViewServerProc=NULL;
LONG_PTR g_wpOrigListBuddyProc=NULL;
LONG_PTR g_wpOrigListViewPlayersProc=NULL;
LONG_PTR g_wpOrigCBSearchProc=NULL;
LONG_PTR g_wpOrigSLHeaderProc = NULL;

HTREEITEM hTIMINPLY=NULL,hTIMAXPLY=NULL;
DEVMODE  userMode, etMode;

typedef vector<_TREEITEM> _vecTI;

_vecTI vTI; //Vector Tree Item
bool g_bNormalWindowed = true;
_CUSTOM_COLUMN CUSTCOLUMNS[MAX_COLUMNS];


BOOL Sizing = FALSE;

long UpdateServerItem(DWORD index);
char Get_GameIcon(char index);
long InsertServerItem(GAME_INFO *pGI,SERVER_INFO pSI);
int TreeView_GetIndexByHItemTree(HTREEITEM hItemtree);
int TreeView_SetAllChildItemExpand(int startIdx, bool expand);
int TreeView_save();
void ListView_SetHeaderSortImage(HWND listView, int columnIndex, BOOL isAscending);
BOOL DDE_Init();
void OnActivate_ServerList(DWORD options=0);
bool bMainWindowsRunning=false;
long UpdatePlayerListQ3(PLAYERDATA *pQ4ply);
long UpdatePlayerListQ4(PLAYERDATA *pQ4ply);
long UpdateRulesList(SERVER_RULES *pServerRules);

void TreeView_BuildList();
void OnMinMaxPlayers(HWND hWndParent,bool bMinValueToSet);
void Update_TreeViewCFGFile();
void SaveAll(DWORD dwCloseReason);
SERVER_INFO * Get_CurrentServerListByView();
void ChangeViewStates(UINT uItem);
SERVER_INFO *FindServer(char *str);

LRESULT ListView_CustomDraw (LPARAM lParam);

LRESULT CALLBACK CFG_OnSelChangedProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void FastConnect();
DWORD FilterServerList(bool bPrivateOnly=false);

char * ReadCfgStr(TiXmlElement* pNode, char *szParamName,char *szOutputBuffer,int iBuffSize);
const char * ReadCfgStr2(TiXmlElement* pNode, char *szParamName,char *szOutputBuffer,int iBuffSize);
int ReadCfgInt(TiXmlElement* pNode, char *szParamName, int& intVal);
int ReadCfgInt2(TiXmlElement* pNode, char *szParamName, int& intVal);
void WriteCfgStr(TiXmlElement * root, char *szParentName, char *szParamName,const char *value) ;
void WriteCfgInt(TiXmlElement * root, char *szParentName, char *szParamName,int value) ;
int XML_GetTreeItemInt(TiXmlElement* pNode, const char* attributeName);
const char * XML_GetTreeItemStr(TiXmlElement* pNode, const char* attributeName,char *szOutput, DWORD maxBytes);
const char * XML_GetTreeItemStrValue(TiXmlElement* pNode,char *szOutput, DWORD maxBytes);
const char * XML_GetTreeItemName(TiXmlElement* pNode,char *szOutput, DWORD maxBytes);

int OwnerDrawnComboBox_Country(LPDRAWITEMSTRUCT lpDrawItemStruct, HDC dc);
void ClearAllServerLinkedList();
DWORD AddServer(GAME_INFO *pGI,char *szIP, DWORD dwPort,bool bFavorite);

void UpdateItem(LPARAM *lp);
void OnRestore();
void OnCreate(HWND hwnd);
void OnInitialize_MainDlg(HWND hwnd);
void OnClose();
void OnRCON();
void OnPaint(HDC hDC);

void SetImageList();
DWORD WINAPI ReScanServerList(LPVOID lpParam);
void StartGame_ConnectToServer(bool connectFromBuddyList);
void Favorite_Remove();
void Favorite_Add(bool manually);
void FilterUpdate();
char *SplitIPandPORT(char *szIPport,DWORD &port);


char EXE_PATH[_MAX_PATH+_MAX_FNAME];			//Don't write anything to this path
char USER_SAVE_PATH[_MAX_PATH+_MAX_FNAME];     //Path to save settings and server lists
char COMMON_SAVE_PATH[_MAX_PATH+_MAX_FNAME];   //Used for downloading app update and preview images - purpose to share the same data between users.
char EXE_PATH_OLD[_MAX_PATH+_MAX_FNAME];


DWORD dwCurrPort = 27960;
bool bDoFirstTimeCheckForUpdate=true;

bool	 bPlayerNameAsc=true,bPlayerClanAsc=true,	bRateAsc = true,bPlayerPingAsc=true;

RECT rcClient;

HBITMAP hbmOldBuffer;
HBITMAP hbmBuffer;
HDC hdcBuffer;

APP_SETTINGS_NEW AppCFG;


// Global Variables:
SERVER_INFO *g_CurrentSRV = NULL;
SERVER_INFO g_tmpSRV ;
SERVER_INFO g_CurrentSelServer;
bool g_bMinimized=false;
SERVER_INFO *g_PRIVPASSsrv;
HWND g_PROGRESS_DLG,g_DlgProgress= NULL,g_DlgProgressMsg=NULL;
RECT g_rcDestMapImg;
HWND g_hwndToolbarOptions = NULL;
RECT g_CSRect,g_INFOIconRect;
HWND g_hwndSearchToolbar = NULL, g_hwndSearchCombo= NULL;
HWND g_hwndComboEdit = NULL;
HWND g_hWnd=NULL,g_hwndRibbonBar;

HIMAGELIST g_hImageListIcons = NULL;


HIMAGELIST m_hImageList,m_hImageListSearchBar;
HIMAGELIST m_hImageListHot;
HIMAGELIST m_hImageListDis;

void OnMinimize(HWND hWnd);
void OnStopScanning();
void OnScanButton();

HFONT g_hf;
char g_currServerIP[128];

bool g_bAnimateTaskTray = false;
bool g_bCancel = false;
bool bRunningQuery = false;
bool g_bRunningQueryServerList = false;
DWORD dwThreadIdRefreshSrvLst;
int g_iCurrentSelectedServer = -1;
int g_statusIcon = -1;
bool bRefreshBrowserList=false;
BOOL 	bRunningRefreshThread = FALSE;
HICON hOnlineIcon,hOfflineIcon;;

HICON hMainIcon;

HWND g_hwndTabControl = NULL;
HWND g_hwndLogger = NULL,g_hwndStatus = NULL;
HWND g_hwndListViewPlayers=NULL,g_hwndListViewVars=NULL,g_hwndListViewServer=NULL,g_hwndListBuddy=NULL;
HWND g_hwndListViewServerListHeader = NULL;
HWND g_hwndMainTreeCtrl=NULL, g_hwndProgressBar=NULL, hwndPaneV=NULL;
HWND hwndPaneH1=NULL,hwndButtonTest=NULL,hwndButtonOptions=NULL,hwndButtonSearch=NULL;
HWND hwndButtonAddToFavorite=NULL;
LRESULT CALLBACK CFG_MainProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

HTREEITEM hRootItem=NULL,hRootFiltersItem=NULL,hRootCountryFiltersItem=NULL,hRootEuropeItem=NULL,hFilterPingItem=NULL,hFilterGameTypeItem=NULL;
DWORD Build_CountryFilter(HTREEITEM hRoot);
void Initialize_CountryFilter();

struct _CountryFilter 
{
	int counter;
	char countryIndex[255];
	LPARAM lParam[255];
	char szShortCountryName[255][4];
	DWORD dwMinPing;
	DWORD dwGameType;
	DWORD dwReserved1;
	char cReserved2;	
	BOOL bReserved3;
};
_CountryFilter CountryFilter;

HTREEITEM MainTreeView_AddItem(_TREEITEM ti, HTREEITEM hCurrent, bool active=true);
//HTREEITEM MainTreeView_AddItem(int iImageIndex, HTREEITEM hCurrent, const char *text, bool expand=false,bool active=true);
void TreeView_SetItemText(HTREEITEM hTI, char *szText);
void Select_item_and_all_childs(HTREEITEM hRoot, bool select);
int TreeView_GetSelectionV3();
const char * XML_GetTreeItemStr(TiXmlElement* pNode, const char* attributeName,char *szOutput, DWORD maxBytes);


TCHAR szPlayerlistColumnString[5][20] = {TEXT("#"),TEXT("Team"),TEXT("Name"), TEXT("Rate/XP"), TEXT("Ping")};

SERVER_CONTAINER SC[MAX_SERVERLIST];
GAME_INFO GI[MAX_SERVERLIST+1];

GAME_INFO *currCV;

char g_szMapName[MAX_PATH];

HWND g_hwndMainRCON=NULL;
HWND g_hwndMainSTATS=NULL;

#define EVENT_PING 1010
UINT_PTR pingTimer;

deque<DWORD> QPing;
typedef deque<DWORD> deQPing;


void MyDrawFont(HDC hdc, int x, int y, char *szMessage,int angle)
{
	//RECT rc; 
	HFONT hfnt, hfntPrev; 
	HRESULT hr; 
	 
	// Allocate memory for a LOGFONT structure. 	 
	PLOGFONT plf = (PLOGFONT) LocalAlloc(LPTR, sizeof(LOGFONT)); 
	 	 
	// Specify a font typeface name and weight. 	 
	hr = StringCchCopy(plf->lfFaceName, 6, "Arial");
	if (FAILED(hr))
	{
		return;
	// TODO: write error handler
	}

	plf->lfWeight = FW_NORMAL; 
	
	SetBkMode(hdc, TRANSPARENT); 
	plf->lfHeight = 12;
	plf->lfEscapement = angle; 
	plf->lfOrientation = angle;
	hfnt = CreateFontIndirect(plf); 
	hfntPrev =(HFONT) SelectObject(hdc, hfnt);
		
	//
	// The StringCchLength call is fitted to the lpszRotate string
	//
	TextOut(hdc, x , y , szMessage, strlen(szMessage)); 
	SelectObject(hdc, hfntPrev); 
	DeleteObject(hfnt); 
	
	// Reset the background mode to its default. 	 
	SetBkMode(hdc, OPAQUE); 
	 
	// Free the memory allocated for the LOGFONT structure. 
	 
	LocalFree((LOCALHANDLE) plf); 
}

#define X_OFFSET_STATS 30
char szIPAddressToPing[100];
char szOldIPAddressToPing[50];
SERVER_INFO g_pSIPing;


PLAYERDATA *Get_PlayerBySelection()
{
	LVITEM lvItem;
	
	int i = ListView_GetSelectionMark(g_hwndListViewPlayers);
	
	if(i!=-1)
	{			
		memset(&lvItem,0,sizeof(LVITEM));
		PLAYERDATA *pPly=NULL;	
		lvItem.mask =  LVIF_PARAM ; 
		lvItem.iItem = i;
		lvItem.iSubItem = 0;		
		if(ListView_GetItem( g_hwndListViewPlayers, &lvItem))
		{
			pPly = (PLAYERDATA*)lvItem.lParam;	
			
			if(pPly==NULL)
			{
				SetStatusText(1,"Not a valid player!");
				InvalidateRect(g_hWnd,NULL,TRUE);
				return NULL;
			}
		}
		return pPly;
	}

	return NULL;
}



LRESULT CALLBACK STATS_Proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC  hDC;
	RECT size,rect,rectInner;
	HBRUSH hbrBkgnd=NULL,hbrBar=NULL;
	HPEN hpen, hpenOld;
	HPEN hPenStatus;
	HBRUSH  hbrushOld;
	switch (message)
	{
		case WM_INITDIALOG:
			strcpy(szIPAddressToPing,", right click on a server then click on Network tools->Ping server.");
			ShowWindow(hWnd,SW_HIDE);
			return TRUE;
			break;
		
		case WM_START_PING:
			{
				QPing.clear();
				int n = ListView_GetSelectionMark(g_hwndListViewServer);
				if(n!=-1)
				{
				
					g_pSIPing  = Get_ServerInfoByIndex(n);
					strcpy(szIPAddressToPing,g_pSIPing.szIPaddress);
					//if(strcmp(szIPAddressToPing,szOldIPAddressToPing)!=0)
					//	QPing.clear();				
					//strcpy(szOldIPAddressToPing,g_pSIPing.szIPaddress);

					SetTimer(hWnd,EVENT_PING,1000,NULL);

				} else
					strcpy(szIPAddressToPing,", Please select a server to ping.");
			}
			break;
	case WM_STOP_PING:
		KillTimer(hWnd,EVENT_PING);
		break;
	case WM_TIMER:
		{
			if(wParam==EVENT_PING)
			{
				

					DWORD pingi =  MyPing(szIPAddressToPing);
					QPing.push_back(pingi);
					if(QPing.size()>50)
						QPing.pop_front();

					if(pingi==999)
						strcpy(szIPAddressToPing,", sorry no response from server.");

				GetClientRect(hWnd,&size);
				InvalidateRect(hWnd,&size,TRUE);
			}
		}
		break;

	case WM_PAINT:
		{
			hDC = BeginPaint(hWnd, &ps);
			if(hDC==NULL)
				return FALSE;
		
			HGDIOBJ original = NULL;
			original = SelectObject(hDC,GetStockObject(DC_PEN));
			GetWindowRect(hWnd,&size);
		
			hbrBkgnd = CreateSolidBrush(RGB(0, 0,64)); 
			hbrushOld = (HBRUSH)SelectObject(hDC, hbrBkgnd);

			SetDCPenColor(hDC,RGB(192,192,255));
			SetBkColor(hDC,    RGB(0, 0,64));
			rect.left = 2;
			rect.top = 2;
			
			rect.bottom = (size.bottom - size.top);
			rect.right = (size.right - size.left);
			FillRect(hDC, &rect, (HBRUSH) hbrBkgnd);   //fill out the main dark blue background
			
			rect.left = X_OFFSET_STATS;
			rect.top = X_OFFSET_STATS;
			rect.bottom-=20;
			rect.right-=20;
			
			
			Rectangle(hDC, rect.left, rect.top ,rect.right, rect.bottom); 
			
			CopyRect(&rectInner,&rect);
			
			//Shrink one pixel for the maind rawing area
			rectInner.left = X_OFFSET_STATS+1;
			rectInner.top = X_OFFSET_STATS+1;
			rectInner.bottom--;
			rectInner.right--;
			FillRect(hDC, &rectInner, (HBRUSH) hbrBkgnd);
			
			SetTextColor(hDC,0x00FFFFFF);

			hPenStatus = CreatePen(PS_SOLID, 1, RGB(0,255,0));

			hpen = CreatePen(PS_DOT, 1, RGB(64,64,204));
			hpenOld = (HPEN) SelectObject(hDC, hpen);
		
			int offsetX = rectInner.right / 6;
			int offsetY = rectInner.bottom / 6;
			deQPing::reverse_iterator iLst;
			deQPing::iterator MaxElement;

			MaxElement = max_element ( QPing.begin ( ) , QPing.end ( ) );
			DWORD dwMaxPing = 0;
			if(MaxElement !=QPing.end())
				dwMaxPing = *MaxElement;
			
			int scale = 10;
			if(dwMaxPing!=0)
				scale = dwMaxPing / 5;


			for(int i=0; i<5;i++)
			{	
				char szTxt[10];

				if((rectInner.right-offsetX*(i+1))>rectInner.left)
				{
					int x = rectInner.right-offsetX*(i+1);
					MoveToEx(hDC,x,rectInner.top,NULL);
					LineTo(hDC,x,rectInner.bottom);
					
					sprintf(szTxt,"%d",(i+1)*6);
					MyDrawFont(hDC, x, rectInner.bottom+3, szTxt,0);
				}
				if((rectInner.bottom-offsetY*(i+1))>rectInner.top)
				{
					int y = rectInner.bottom-offsetY*(i+1);
					MoveToEx(hDC,rectInner.left,y,NULL);
					LineTo(hDC,rectInner.right,y);

					sprintf(szTxt,"%d",(i+1)*scale);
					MyDrawFont(hDC, rectInner.left-12, y, szTxt,900);
				}

			}
			SelectObject(hDC, hPenStatus);

			offsetX = offsetX/6;
			
			int oldX = rectInner.right,
				oldY = rectInner.bottom;

			

			int i=0;
			for ( iLst = QPing.rbegin(); iLst != QPing.rend(); iLst++ )
			{
				
				DWORD dwPingi = *iLst;
				MoveToEx(hDC,oldX,oldY,NULL);
				int fact = dwPingi / scale;
					
				int y = fact * offsetY;
				y+= dwPingi-(fact*scale);  //rest

				int yTot = rectInner.bottom-y;
				if(yTot<rectInner.top)
					yTot = rectInner.top;
				
				int x = rectInner.right-(offsetX*(i+1));
				if(x<rectInner.left)
					x = rectInner.left;
				
				LineTo(hDC,x,yTot);
				oldX = x;
				oldY = yTot;
				i++;

			}
		

			// Do not forget to clean up.
			SelectObject(hDC, hpenOld);
			DeleteObject(hPenStatus);
			DeleteObject(hpen);
			SelectObject(hDC, hbrushOld);
		
			char szText[100];
		
			sprintf(szText,"Ping response from %s",szIPAddressToPing);
			
			
			MyDrawFont(hDC, 5, (rectInner.bottom/2)+40, "milliseconds",900);

			MyDrawFont(hDC, rectInner.right / 2, rectInner.bottom+10, "seconds",0);
			size.left = 20;
			size.top = 6;

			TextOut(hDC, size.left+2,size.top-1, szText, strlen(szText));
			SetTextColor(hDC,0x00000000);
			if(hbrBkgnd!=NULL)
				DeleteObject(hbrBkgnd);
			if(hbrBar!=NULL)
				DeleteObject(hbrBar);
			
			SelectObject(hDC,original);

			EndPaint(hWnd, &ps);
	
		}
		break;
	case WM_CLOSE:
		KillTimer(hWnd,EVENT_PING);
		break;

	case WM_COMMAND:

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			if(LOWORD(wParam) == IDOK)
			{
			
				return TRUE;
			}	
			EndDialog(hWnd, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}




HWND InitRCON(HWND hwndParent)
{
	DLGTEMPLATE *apRes = DoLockDlgRes(MAKEINTRESOURCE(IDD_DLG_RCON));
    g_hwndMainRCON = CreateDialogIndirect(g_hInst, apRes, hwndParent, (DLGPROC)RCON_Proc); 
	ShowWindow(g_hwndMainRCON,SW_HIDE);
	return g_hwndMainRCON;
}

//Create PING window
HWND InitSTATS(HWND hwndParent)
{
	DLGTEMPLATE *apRes = DoLockDlgRes(MAKEINTRESOURCE(IDD_DIALOG_STATS));
    g_hwndMainSTATS = CreateDialogIndirect(g_hInst, apRes, hwndParent, (DLGPROC)STATS_Proc); 
	
	return g_hwndMainSTATS;
}

int StrSorter(char *a, char *b)
{
	int i=0,i2=0;
	char upperA;
	char upperB;

	while(a[i]==32) //trim spaces
		i++;

	while(b[i2]==32)  //trim spaces
		i2++;

	if(a==NULL)
		return 0;

	while(1)
	{
		if(a[i]==0 && b[i2]==0)
			return 0;
		else if(a[i]==0)
			return -1;
		else if(b[i2]==0)
			return 1;


		upperA = a[i];

		if(__isascii(a[i]))
			if(islower(a[i]))
				upperA =_toupper(a[i]);
		
		upperB = b[i2];

		if(__isascii(b[i2]))
			if(islower(b[i2]))
				upperB = _toupper(b[i2]);

		if(upperA> upperB)
			return 1;
		else if(upperA < upperB)
			return -1;
		
		i++;
		i2++;
	}

	return 0;
}

bool Sort_Player(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  currCV->pSC->vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  currCV->pSC->vSI.at(rSIb.dwIndex);
	
	if(CUSTCOLUMNS[COL_PLAYERS].bSortAsc)
		return (pSIa.nCurrentPlayers > pSIb.nCurrentPlayers);
	else
		return (pSIa.nCurrentPlayers < pSIb.nCurrentPlayers);
}

bool Sort_Country(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  currCV->pSC->vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  currCV->pSC->vSI.at(rSIb.dwIndex);

	if(CUSTCOLUMNS[COL_COUNTRY].bSortAsc)
		return (strcmp(pSIa.szCountry , pSIb.szCountry )>0);
	else
	   return (strcmp(pSIa.szCountry , pSIb.szCountry )<0);
}

bool Sort_Ping(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  currCV->pSC->vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  currCV->pSC->vSI.at(rSIb.dwIndex);
	if(CUSTCOLUMNS[COL_PING].bSortAsc)	
		return (pSIa.dwPing > pSIb.dwPing);	
	else
		return (pSIa.dwPing < pSIb.dwPing);	
}

bool Sort_Map(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  currCV->pSC->vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  currCV->pSC->vSI.at(rSIb.dwIndex);
	
	if(CUSTCOLUMNS[COL_MAP].bSortAsc)	
		return (StrSorter(pSIa.szMap , pSIb.szMap )>0);
	else
		return (StrSorter(pSIa.szMap , pSIb.szMap )<0);
}

bool Sort_Mod(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  currCV->pSC->vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  currCV->pSC->vSI.at(rSIb.dwIndex);
	if(CUSTCOLUMNS[COL_MOD].bSortAsc)	
		return (StrSorter(pSIa.szMod , pSIb.szMod )>0);
	else
		return (StrSorter(pSIa.szMod , pSIb.szMod )<0);
}

bool Sort_IP(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  currCV->pSC->vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  currCV->pSC->vSI.at(rSIb.dwIndex);
	
	if(CUSTCOLUMNS[COL_IP].bSortAsc)
		return (StrSorter(pSIa.szIPaddress , pSIb.szIPaddress )>0);
	else
		return (StrSorter(pSIa.szIPaddress , pSIb.szIPaddress )<0);
}


bool Sort_Punkbuster(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  currCV->pSC->vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  currCV->pSC->vSI.at(rSIb.dwIndex);
	
	if(CUSTCOLUMNS[COL_PB].bSortAsc)
		return (pSIa.bPunkbuster > pSIb.bPunkbuster );
	else
		return (pSIa.bPunkbuster < pSIb.bPunkbuster );

}

bool Sort_Private(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  currCV->pSC->vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  currCV->pSC->vSI.at(rSIb.dwIndex);

	if(CUSTCOLUMNS[COL_PRIVATE].bSortAsc)
		return (pSIa.bPrivate > pSIb.bPrivate );	
	else
		return (pSIa.bPrivate < pSIb.bPrivate );	
}

bool Sort_Ranked(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  currCV->pSC->vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  currCV->pSC->vSI.at(rSIb.dwIndex);
	
	if(CUSTCOLUMNS[COL_RANKED].bSortAsc)
		return (pSIa.cRanked > pSIb.cRanked );	
	else
		return (pSIa.cRanked < pSIb.cRanked );	
}
bool Sort_Bots(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  currCV->pSC->vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  currCV->pSC->vSI.at(rSIb.dwIndex);
	
	if(CUSTCOLUMNS[COL_BOTS].bSortAsc)
		return (pSIa.cBots > pSIb.cBots );	
	else
		return (pSIa.cBots < pSIb.cBots );	
}

bool Sort_ServerName(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  currCV->pSC->vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  currCV->pSC->vSI.at(rSIb.dwIndex);

	if(CUSTCOLUMNS[COL_SERVERNAME].bSortAsc)
		return (StrSorter(pSIa.szServerName , pSIb.szServerName )>0);
	 else
		return (StrSorter(pSIa.szServerName , pSIb.szServerName )<0);

}
bool Sort_Version(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  currCV->pSC->vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  currCV->pSC->vSI.at(rSIb.dwIndex);

	if(CUSTCOLUMNS[COL_VERSION].bSortAsc)
	{
		return (StrSorter(pSIa.szVersion , pSIb.szVersion )>0);
	}
	
	return (StrSorter(pSIa.szVersion , pSIb.szVersion )<0);
}


bool Sort_GameType(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  currCV->pSC->vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  currCV->pSC->vSI.at(rSIb.dwIndex);
	if(CUSTCOLUMNS[COL_GAMETYPE].bSortAsc)
		return (pSIa.cGameTypeCVAR < pSIb.cGameTypeCVAR);
	else
		return (pSIa.cGameTypeCVAR > pSIb.cGameTypeCVAR );
}

void Do_ServerListSort(int iColumn)
{
	GAME_INFO *localcurrCV = currCV;
	BOOL sortdir = FALSE;
	if(localcurrCV->pSC->vRefListSI.size()>0)
	{
		DWORD id=0;
		for(int i=0;i<MAX_COLUMNS;i++)
		{
			if(CUSTCOLUMNS[i].bActive && (CUSTCOLUMNS[i].columnIdx == iColumn))
			{
				id = CUSTCOLUMNS[i].id;
				sortdir = CUSTCOLUMNS[i].bSortAsc;
				iLastColumnSortIndex = iColumn;
				break;
			}
		}

		switch(id)
		{
			case COL_PB : sort(localcurrCV->pSC->vRefListSI.begin(),localcurrCV->pSC->vRefListSI.end(),Sort_Punkbuster); break;
			case COL_PRIVATE: sort(localcurrCV->pSC->vRefListSI.begin(),localcurrCV->pSC->vRefListSI.end(),Sort_Private); break;
			case COL_RANKED: sort(localcurrCV->pSC->vRefListSI.begin(),localcurrCV->pSC->vRefListSI.end(),Sort_Ranked); break;
			case COL_SERVERNAME: sort(localcurrCV->pSC->vRefListSI.begin(),localcurrCV->pSC->vRefListSI.end(),Sort_ServerName); break;
			case COL_GAMETYPE: sort(localcurrCV->pSC->vRefListSI.begin(),localcurrCV->pSC->vRefListSI.end(),Sort_GameType); break;
			case COL_MAP: sort(localcurrCV->pSC->vRefListSI.begin(),localcurrCV->pSC->vRefListSI.end(),Sort_Map); break;
			case COL_MOD: sort(localcurrCV->pSC->vRefListSI.begin(),localcurrCV->pSC->vRefListSI.end(),Sort_Mod); break;
			case COL_PLAYERS: sort(localcurrCV->pSC->vRefListSI.begin(),localcurrCV->pSC->vRefListSI.end(),Sort_Player); break;
			case COL_COUNTRY: sort(localcurrCV->pSC->vRefListSI.begin(),localcurrCV->pSC->vRefListSI.end(),Sort_Country); break;
			case COL_PING: sort(localcurrCV->pSC->vRefListSI.begin(),localcurrCV->pSC->vRefListSI.end(),Sort_Ping); break;
			case COL_IP: sort(localcurrCV->pSC->vRefListSI.begin(),localcurrCV->pSC->vRefListSI.end(),Sort_IP); break;
			case COL_VERSION: sort(localcurrCV->pSC->vRefListSI.begin(),localcurrCV->pSC->vRefListSI.end(),Sort_Version); break;
			case COL_BOTS: sort(localcurrCV->pSC->vRefListSI.begin(),localcurrCV->pSC->vRefListSI.end(),Sort_Bots); break;


		}
		ListView_SetHeaderSortImage(g_hwndListViewServer, iColumn,(BOOL) sortdir); //id =iColumn
		ListView_SetItemCount(g_hwndListViewServer,localcurrCV->pSC->vRefListSI.size());
	
	}

}

int CALLBACK MyComparePlayerFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	PLAYERDATA *ptr1=NULL;
	PLAYERDATA *ptr2=NULL;

	ptr1 = (PLAYERDATA*)lParam1;
	ptr2 = (PLAYERDATA*)lParam2;

	switch(lParamSort)
	{
		case 1:
			{
			if(bPlayerClanAsc)
				return (StrSorter(ptr1->szClanTag , ptr2->szClanTag )>0);
			 else
				return (StrSorter(ptr1->szClanTag , ptr2->szClanTag )<0);
			}
		 break;

		case 2:
			{
			if(bPlayerNameAsc)
				return (StrSorter(ptr1->szPlayerName , ptr2->szPlayerName )>0);
			 else
				return (StrSorter(ptr1->szPlayerName , ptr2->szPlayerName )<0);
			}
		 break;
			case 3:
			{
			if(bRateAsc)
				return (ptr1->rate > ptr2->rate);
			 else
				return (ptr1->rate < ptr2->rate);
			}
		 break;
			case 4:
			{
			if(bPlayerPingAsc)
				return (ptr1->ping > ptr2->ping);
			 else
				return (ptr1->ping < ptr2->ping);
			}
		 break;

	}

	return 0;

}


void ShowBalloonTip(char *title,char *message)
{
	
	NOTIFYICONDATA structNID;

	structNID.cbSize = sizeof(NOTIFYICONDATA); // sizeof the struct in bytes 
	//structNID.hWnd = (HWND) hWnd; //handle of the window which will process this app. messages 
	structNID.uID = 100123; //ID of the icon that willl appear in the system tray 
	structNID.uFlags = NIF_INFO| NIF_ICON | NIF_MESSAGE; // | NIF_TIP; //ORing of all the flags 
	strcpy(structNID.szTip,"ET Server Viewer"); //Text of the tooltip 
//	structNID.hIcon = hOfflineIcon; // handle of the Icon to be displayed, obtained from LoadIcon 
	structNID.uCallbackMessage = WM_USER_SHELLICON; // user defined message that will be sent as the notification message to the Window Procedure 

	structNID.dwInfoFlags = NIIF_INFO; //NIIF_ERROR;  //NIIF_WARNING; 
	structNID.uTimeout = 10000;
	
	strcpy(structNID.szInfoTitle,title); 
	strcpy(structNID.szInfo ,message); 

	//put the actual icon now
	Shell_NotifyIcon(NIM_MODIFY, &structNID); 

}


LRESULT CALLBACK PRIVPASS_Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	
	case WM_INITDIALOG:
		{
			
			CenterWindow(hDlg);

			SetFocus(GetDlgItem(hDlg,IDC_EDIT_PASS));

			if(g_PRIVPASSsrv->szPRIVATEPASS!=NULL)
			{
				SetDlgItemText(hDlg,IDC_EDIT_PASS,g_PRIVPASSsrv->szPRIVATEPASS);
				PostMessage(GetDlgItem(hDlg,IDC_EDIT_PASS),EM_SETSEL,0,strlen(g_PRIVPASSsrv->szPRIVATEPASS));
				PostMessage(GetDlgItem(hDlg,IDC_EDIT_PASS),EM_SETSEL,(WPARAM)-1,-1);
			}

		//	return TRUE;
		}
		break;

	case WM_COMMAND:

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			if(LOWORD(wParam) == IDOK)
			{
				GetDlgItemText(hDlg,IDC_EDIT_PASS,g_PRIVPASSsrv->szPRIVATEPASS,sizeof(g_PRIVPASSsrv->szPRIVATEPASS)-1);
			}
			
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}


//Virtual list - had to use vector instead of linked list using LPARAM refering to a memory as 
//index based is the only way to achieve this :(
BOOL OnGetDispInfoList(int ctrlid, NMHDR *pNMHDR)
{
	LVITEM *pLVItem;
	NMLVDISPINFO *pLVDI;
	SERVER_INFO *pSrvInf = NULL; 
	
	if (ctrlid!=IDC_LIST_SERVER)
		return FALSE;
	
	if(pNMHDR==NULL)
		return FALSE;

	pLVDI = (NMLVDISPINFO *)pNMHDR;
	
	pLVItem = &pLVDI->item;
	int size = currCV->pSC->vRefListSI.size();
	if (size==0)
		return FALSE;

	if(size<pLVItem->iItem)
		return FALSE;
	SERVER_INFO pSI;
	ZeroMemory(&pSI,sizeof(SERVER_INFO));
	REF_SERVER_INFO refSI;
	try
	{	
		refSI = currCV->pSC->vRefListSI.at((int)pLVItem->iItem);
		pSI = currCV->pSC->vSI.at(refSI.dwIndex);
	}
	catch(const exception& e)
	{
		AddLogInfo(0,"Vector err exception Details: %s",e.what());
		return FALSE;
	}

	pSrvInf = &pSI;

	char szText[120];
	if(pSrvInf==NULL)
		return FALSE;


	//This support dynamic columns 
	for(int i=0;i<MAX_COLUMNS;i++)
	{
		//Enumerate columns 
		if(CUSTCOLUMNS[i].bActive &&  (pLVItem->iSubItem == CUSTCOLUMNS[i].columnIdx ))
		{
		//	dbg_print("Updating column idx=%d %s",CUSTCOLUMNS[i].columnIdx,CUSTCOLUMNS[i].sName.c_str());
			switch(CUSTCOLUMNS[i].id) //CUSTCOLUMNS[i].id)
			{
				case COL_PB:
					{
						if(pSrvInf->bPunkbuster) //PB or VAC
						{
							if(pSrvInf->cGAMEINDEX<CS_SERVERLIST)
								pLVItem->iImage = 1; //Punkbuster icon
							else
								pLVItem->iImage = 30;  //VAC/Steam icon
							
						}
						return TRUE;
					break;
					}
				case COL_PRIVATE:
					{
						if (pSrvInf->bPrivate) //Private
							pLVItem->iImage = 4;
						return TRUE;
					break;
					}					
				case COL_RANKED:
					{
						if (pSrvInf->cRanked) //Ranked only used for ETQW for now
							pLVItem->iImage = 19;
						return TRUE;
					break;
					}					
				case COL_SERVERNAME:
					{
						if (pSrvInf->cFavorite)
							pLVItem->iImage = 2;

						else
							pLVItem->iImage = Get_GameIcon(currCV->cGAMEINDEX);

						char colFiltered[100];
						colorfilterQ4(pSrvInf->szServerName,colFiltered,sizeof(colFiltered));
						pLVItem->pszText = colFiltered;
						return TRUE;
						//strncpy(pLVItem->pszText,szText,pLVItem->cchTextMax);
					break;
					}	
				case COL_VERSION:
					{	
						strncpy(pLVItem->pszText,pSrvInf->szVersion,pLVItem->cchTextMax);
						return TRUE;
					break;
					}	
				case COL_GAMETYPE:
					{
						if(strlen(pSrvInf->szGameTypeName)==0)	
							strncpy(pLVItem->pszText,Get_GameTypeNameByGameType(pSrvInf->cGAMEINDEX,pSrvInf->cGameTypeCVAR),pLVItem->cchTextMax);
						else
							strncpy(pLVItem->pszText,pSrvInf->szGameTypeName,pLVItem->cchTextMax);
						return TRUE;
					break;
					}	
				case COL_MAP:
					{
						strncpy(pLVItem->pszText,pSrvInf->szMap,pLVItem->cchTextMax);
						return TRUE;
					break;
					}	
				case COL_MOD:
					{
						strncpy(pLVItem->pszText,pSrvInf->szMod,pLVItem->cchTextMax);
						return TRUE;
					break;
					}
				case COL_PLAYERS:
					{			
						sprintf_s(szText,sizeof(szText)-1,"%d/%d+(%d)",pSrvInf->nCurrentPlayers,pSrvInf->nMaxPlayers,pSrvInf->nPrivateClients);
						strncpy(pLVItem->pszText,szText,pLVItem->cchTextMax);
						return TRUE;
					break;
					}
				case COL_COUNTRY:
					{
						pLVItem->iImage = pSrvInf->cCountryFlag;
						strncpy(pLVItem->pszText,pSrvInf->szCountry,pLVItem->cchTextMax);
						return TRUE;
					break;
					}
				case COL_PING:
					{
						sprintf_s(szText,sizeof(szText)-1,"%d",pSrvInf->dwPing);
						strncpy(pLVItem->pszText,szText,pLVItem->cchTextMax);
						return TRUE;
					break;
					}
				case COL_IP:
					{
						sprintf_s(szText,sizeof(szText)-1,"%s:%d",pSrvInf->szIPaddress,pSrvInf->dwPort);
						strncpy(pLVItem->pszText,szText,pLVItem->cchTextMax);
						return TRUE;
					break;
					}
				case COL_BOTS:
					{
						sprintf_s(szText,sizeof(szText)-1,"%d",pSrvInf->cBots);
						strncpy(pLVItem->pszText,szText,pLVItem->cchTextMax);
						return TRUE;
					break;
					}
			} 

		}

	}	
	return TRUE;
}



/************************************************
 Will output the result into pszOutputString and size in bytes
 Unsuccefull result equals to NULL
**************************************************/
char *Registry_GetGamePath(HKEY hkey,char *pszRegPath,char *pszRegKey,char *pszOutputString,DWORD *dwSizeOfBuffer)
{
	HKEY HKey;
	DWORD dwOpen=0,dwType=REG_SZ;
	if(RegCreateKeyEx(hkey, pszRegPath, 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &HKey, &dwOpen) == ERROR_SUCCESS) 
		{
			if(RegQueryValueEx(HKey,pszRegKey, 0, &dwType, (LPBYTE)pszOutputString, dwSizeOfBuffer) == ERROR_SUCCESS) 
			{
				RegCloseKey(HKey);		
				return pszOutputString;
			}
		RegCloseKey(HKey);
		}
	pszOutputString = NULL;
	dwSizeOfBuffer=0;
	return NULL;
}

/*************************
	Retrieve server info
***************************/
SERVER_INFO * GetServerInfo(int gametype,SERVER_INFO *pSrvInf)
{
	g_CurrentSRV = NULL;
	if(pSrvInf==NULL)
		return NULL;

	strcpy(g_currServerIP,pSrvInf->szIPaddress);
	strcpy(g_szMapName,pSrvInf->szMap);
	dwCurrPort = pSrvInf->dwPort;	
	g_CurrentSRV = pSrvInf;
	
	if(pSrvInf->dwIndex<=GI[gametype].pSC->vSI.size())	
		g_tmpSRV = GI[gametype].pSC->vSI.at(pSrvInf->dwIndex);
	else
		AddLogInfo(ETSV_ERROR,"Vector request out of scope at %s %d",__FILE__,__LINE__);

	switch(gametype)
	{	

		case Q4_SERVERLIST:
		{
			g_bQ4 = TRUE;
			Q4_OnServerSelection(pSrvInf,&UpdatePlayerListQ3,&UpdateRulesList);
			break;
		}
		case ETQW_SERVERLIST:
		{
			g_bQ4 = FALSE;
			Q4_OnServerSelection(pSrvInf,&UpdatePlayerListQ3,&UpdateRulesList);
			break;
		}
		case CS_SERVERLIST:
		case CSCZ_SERVERLIST:
		case CSS_SERVERLIST:
		{			
			STEAM_OnServerSelection(pSrvInf,&UpdatePlayerListQ3,&UpdateRulesList);
			break;
		}
		case WARSOW_SERVERLIST:
		case COD_SERVERLIST:
		case COD2_SERVERLIST:
		case COD4_SERVERLIST:
		case Q3_SERVERLIST:
		case RTCW_SERVERLIST:
		case ET_SERVERLIST:
		default:
		{
			Q3_OnServerSelection(pSrvInf,&UpdatePlayerListQ3,&UpdateRulesList);
			break;
		}

	} //end switch
	return pSrvInf;
}

void OnServerSelected(GAME_INFO *pGI)
{
	SERVER_INFO *pSrvInf = NULL;
	//SERVER_INFO pSI;
	
	int i = ListView_GetSelectionMark(g_hwndListViewServer);
	if(g_iCurrentSelectedServer==i)
		return;
	bRunningQuery = true;


	if(i!=-1)
	{
		g_iCurrentSelectedServer = i;
		SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
	
		ListView_DeleteAllItems(g_hwndListViewVars);
		ListView_DeleteAllItems(g_hwndListViewPlayers);
	
		try
		{
			g_CurrentSelServer = Get_ServerInfoByIndex(g_iCurrentSelectedServer);
			if(g_CurrentSelServer.dwIP == 0)
				return;				
		}
		catch(const exception& e)
		{
			AddLogInfo(0,"Exception raised at OnServerSelected() Details:%s\n",e.what());
			return;
		}
		pSrvInf = &g_CurrentSelServer;
		if(pSrvInf!=NULL)
		{
			GetServerInfo(g_currentGameIdx,pSrvInf);
			UpdateServerItem(g_iCurrentSelectedServer);
			pGI->pSC->vSI.at(g_CurrentSelServer.dwIndex) = g_CurrentSelServer;
			//AddLogInfo(ETSV_DEBUG,"Num visible servers %d of %d\n",pGI->pSC->vRefListSI.size(), pGI->pSC->vSI.size());
			UpdateCurrentServerUI();
			
		}//if pSrv
	}
	
	UpdateWindow(g_hwndListViewServer);
	UpdateWindow(g_hwndListViewPlayers);
	UpdateWindow(g_hwndListViewVars);

	bRunningQuery = false;
}


void OnBuddySelected()
{
	BUDDY_INFO *pBI = NULL;
	
	int i = ListView_GetSelectionMark(g_hwndListBuddy);
	
	bRunningQuery = true;

	dbg_print("Onbuddy\n");
	if(i!=-1)
	{
		SetCursor(LoadCursor(NULL, IDC_APPSTARTING));

		LVITEM lvItem;
		memset(&lvItem,0,sizeof(LVITEM));	
		lvItem.mask =  LVIF_PARAM ; 
		lvItem.iItem = i;
		lvItem.iSubItem = 0;		
		if(ListView_GetItem( g_hwndListBuddy, &lvItem))
		{
			ListView_DeleteAllItems(g_hwndListViewVars);
			ListView_DeleteAllItems(g_hwndListViewPlayers);

			pBI = (BUDDY_INFO*)lvItem.lParam;	
			
			if((pBI!=NULL))
			{
				vSRV_INF::iterator  iLst;
		
				if(pBI->sIndex!=-1)
				{
					try
					{

						SERVER_INFO pSI = GI[pBI->cGAMEINDEX].pSC->vSI.at((int)pBI->sIndex);
						GetServerInfo(pBI->cGAMEINDEX,&pSI);
		
					}
					catch(const exception& e)
					{						
						AddLogInfo(0,"Access Violation!!! %s (OnBuddySelected)\n",e.what());
					}
					UpdateCurrentServerUI();
				}

	
			
				
			}//if pSrv
		}
	}	

	bRunningQuery = false;
}

void RegisterProtocol(char *path)
{
	LONG ret;
	HKEY hkey=NULL;
	DWORD dwDisposition;
	char szBuffer[512];
	char totpath[_MAX_PATH+_MAX_FNAME],path2[_MAX_PATH+_MAX_FNAME];
  

	//copy before modify
	strcpy_s(path2,515,path);
	//check if it is in root only.. C:\?
   if(strlen(path2)>3)	
	   strcat_s(path2,515,"\\ETServerViewer.exe");
   else
	   strcat_s(path2,515,"ETServerViewer.exe");
	
   sprintf(totpath,"%s %c1",path2,'%');

	//Register protocol
	//--------------------------------------

	for(int i=0; i<MAX_SERVERLIST;i++)
	{
		hkey=NULL;
		sprintf(szBuffer,"%s",GI[i].szProtocolName);
		ret = RegCreateKeyEx(HKEY_CLASSES_ROOT, szBuffer, 0, NULL,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition ); 
		if(ret==ERROR_SUCCESS)
		{
			sprintf(szBuffer,"URL:%s",GI[i].szProtocolName);
			ret = RegSetValueEx( hkey, "", 0, REG_SZ, (const unsigned char*)szBuffer, (DWORD)strlen(szBuffer)); 
			memset(szBuffer,0,sizeof(szBuffer));
			ret = RegSetValueEx( hkey, "URL Protocol", 0, REG_SZ, (const unsigned char*)szBuffer, (DWORD)strlen(szBuffer)); 
			RegCloseKey( hkey ); 
		}
		sprintf(szBuffer,"%s\\shell\\open\\command",GI[i].szProtocolName);
		ret = RegCreateKeyEx(HKEY_CLASSES_ROOT, szBuffer, 0, NULL,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition ); 
		if(ret==ERROR_SUCCESS)
		{
			ret = RegSetValueEx( hkey, "", 0, REG_SZ, (const unsigned char*)totpath, strlen(totpath)); 
			RegCloseKey( hkey ); 
		}
	}
}


/***************************************************
	Set up default settings for each game.
****************************************************/
void Default_GameSettings()
{

	for(int i=0; i<MAX_SERVERLIST; i++)
	{
		ZeroMemory(&GI[i],sizeof(GAME_INFO));
		GI[i].cGAMEINDEX = i;
		GI[i].pSC = &SC[i];
		GI[i].iIconIndex =  Get_GameIcon(i);

	}
	GI[ET_SERVERLIST].cGAMEINDEX = ET_SERVERLIST;
	GI[ET_SERVERLIST].dwMasterServerPORT = 27950;
	GI[ET_SERVERLIST].dwProtocol = 84;
	GI[ET_SERVERLIST].dwViewFlags = 0;
	strncpy(GI[ET_SERVERLIST].szGAME_NAME,"Wolfenstein - Enemy Territory",MAX_PATH);
	strncpy(GI[ET_SERVERLIST].szMasterServerIP,"etmaster.idsoftware.com",MAX_PATH);
	strncpy(GI[ET_SERVERLIST].szMAP_MAPPREVIEW_PATH,"etmaps",MAX_PATH);
	strncpy(GI[ET_SERVERLIST].szGAME_CMD,"",MAX_PATH);
	DWORD dwBuffSize = sizeof(GI[ET_SERVERLIST].szGAME_PATH);
	Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Activision\\Wolfenstein - Enemy Territory","InstallPath",GI[ET_SERVERLIST].szGAME_PATH,&dwBuffSize);

	if(strlen(GI[ET_SERVERLIST].szGAME_PATH)>0)
		GI[ET_SERVERLIST].bActive = true;
	else
		GI[ET_SERVERLIST].bActive = false;
	
	GI[ET_SERVERLIST].pSC = &SC[ET_SERVERLIST];

//	MyFindFile();

	strcat_s(GI[ET_SERVERLIST].szGAME_PATH,sizeof(GI[ET_SERVERLIST].szGAME_PATH),"\\et.exe");
	strcpy_s(GI[ET_SERVERLIST].szProtocolName,sizeof(GI[ET_SERVERLIST].szProtocolName),"et");
	
	GI[ET_SERVERLIST].dwDefaultPort = 27960;
	strcpy(GI[ET_SERVERLIST].szQueryString,"");

	GI[Q3_SERVERLIST].cGAMEINDEX = Q3_SERVERLIST;

	GI[Q3_SERVERLIST].dwMasterServerPORT = 27950;
	GI[Q3_SERVERLIST].dwProtocol = 68;
	GI[Q3_SERVERLIST].iIconIndex =  Get_GameIcon(Q3_SERVERLIST);
	GI[Q3_SERVERLIST].dwViewFlags = 0;
	strncpy(GI[Q3_SERVERLIST].szGAME_NAME,"Quake 3: Arena",MAX_PATH);
	strncpy(GI[Q3_SERVERLIST].szMasterServerIP,"monster.idsoftware.com",MAX_PATH);
	strncpy(GI[Q3_SERVERLIST].szMAP_MAPPREVIEW_PATH,"q3maps",MAX_PATH);
	strncpy(GI[Q3_SERVERLIST].szGAME_PATH,"Quake3.exe",MAX_PATH);
	strncpy(GI[Q3_SERVERLIST].szGAME_CMD,"",MAX_PATH);
	strcpy(GI[Q3_SERVERLIST].szProtocolName,"q3");
	GI[Q3_SERVERLIST].bActive = false;
	GI[Q3_SERVERLIST].dwDefaultPort = 27960;
	strcpy(GI[Q3_SERVERLIST].szQueryString,"");
	GI[Q3_SERVERLIST].pSC = &SC[Q3_SERVERLIST];
//	Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Activision\\Wolfenstein - Enemy Territory","InstallPath",GI[Q3_SERVERLIST].szGAME_PATH,&dwBuffSize);


	GI[RTCW_SERVERLIST].cGAMEINDEX = RTCW_SERVERLIST;
	GI[RTCW_SERVERLIST].dwMasterServerPORT = 27950;
	GI[RTCW_SERVERLIST].dwProtocol = 60;
	GI[RTCW_SERVERLIST].iIconIndex = Get_GameIcon(RTCW_SERVERLIST);
	GI[RTCW_SERVERLIST].dwViewFlags = 0;
	strncpy(GI[RTCW_SERVERLIST].szGAME_NAME,"Return To Castle of Wolfenstein",MAX_PATH);
	strncpy(GI[RTCW_SERVERLIST].szMasterServerIP,"wolfmotd.idsoftware.com",MAX_PATH);
	strncpy(GI[RTCW_SERVERLIST].szMAP_MAPPREVIEW_PATH,"rtcwmaps",MAX_PATH);
	strncpy(GI[RTCW_SERVERLIST].szGAME_PATH,"C:\\Program Files\\Return to Castle Wolfenstein\\WolfMP.exe",MAX_PATH);
	strncpy(GI[RTCW_SERVERLIST].szGAME_CMD,"",MAX_PATH);
	strcpy(GI[RTCW_SERVERLIST].szProtocolName,"rtcw");
	GI[RTCW_SERVERLIST].bActive = false;
	GI[RTCW_SERVERLIST].dwDefaultPort = 27960;
	strcpy(GI[RTCW_SERVERLIST].szQueryString,"");
	GI[RTCW_SERVERLIST].pSC = &SC[RTCW_SERVERLIST];

	GI[Q4_SERVERLIST].cGAMEINDEX = Q4_SERVERLIST;
	GI[Q4_SERVERLIST].dwMasterServerPORT = 27650;
	GI[Q4_SERVERLIST].dwProtocol = 0;
	GI[Q4_SERVERLIST].iIconIndex = Get_GameIcon(Q4_SERVERLIST);
	GI[Q4_SERVERLIST].dwViewFlags = 0;
	strncpy(GI[Q4_SERVERLIST].szGAME_NAME,"Quake 4",MAX_PATH);
	strncpy(GI[Q4_SERVERLIST].szMasterServerIP,"q4master.idsoftware.com",MAX_PATH);
	strncpy(GI[Q4_SERVERLIST].szMAP_MAPPREVIEW_PATH,"q4maps",MAX_PATH);
	strcpy(GI[Q4_SERVERLIST].szProtocolName,"q4");
	strncpy(GI[Q4_SERVERLIST].szGAME_PATH,"Quake4.exe",MAX_PATH);
	strncpy(GI[Q4_SERVERLIST].szGAME_CMD,"+seta com_allowconsole 1",MAX_PATH);
	Registry_GetGamePath(HKEY_LOCAL_MACHINE, "HKEY_LOCAL_MACHINE\\Software\\Id\\Quake 4","InstallPath",GI[Q4_SERVERLIST].szGAME_PATH,&dwBuffSize);
	GI[Q4_SERVERLIST].bActive = false;
	GI[Q4_SERVERLIST].dwDefaultPort = 28004;
	GI[Q4_SERVERLIST].pSC = &SC[Q4_SERVERLIST];

	GI[ETQW_SERVERLIST].cGAMEINDEX = ETQW_SERVERLIST;
	GI[ETQW_SERVERLIST].iIconIndex = Get_GameIcon(ETQW_SERVERLIST);
	GI[ETQW_SERVERLIST].dwViewFlags = 0;
	strncpy(GI[ETQW_SERVERLIST].szGAME_NAME,"Enemy Territory - Quake Wars",MAX_PATH);
	strncpy(GI[ETQW_SERVERLIST].szMasterServerIP,"http://etqw-ipgetter.demonware.net/ipgetter/",MAX_PATH);
	strncpy(GI[ETQW_SERVERLIST].szMAP_MAPPREVIEW_PATH,"etqwmaps",MAX_PATH);
	strncpy(GI[ETQW_SERVERLIST].szGAME_CMD,"+seta com_usefastvidrestart 1 +seta com_allowconsole 1",MAX_PATH);
	dwBuffSize = sizeof(GI[ETQW_SERVERLIST].szGAME_PATH);
	GI[ETQW_SERVERLIST].pSC = &SC[ETQW_SERVERLIST];

	
	Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Id\\ET - QUAKE Wars","EXEString",GI[ETQW_SERVERLIST].szGAME_PATH,&dwBuffSize);
	if(strlen(GI[ETQW_SERVERLIST].szGAME_PATH)>0)
		GI[ETQW_SERVERLIST].bActive = true;
	else
		GI[ETQW_SERVERLIST].bActive = false;

	strcpy(GI[ETQW_SERVERLIST].szProtocolName,"etqw");
	GI[ETQW_SERVERLIST].dwDefaultPort = 27733;

	GI[COD2_SERVERLIST].cGAMEINDEX = COD2_SERVERLIST;
	GI[COD2_SERVERLIST].iIconIndex = Get_GameIcon(COD2_SERVERLIST);
	GI[COD2_SERVERLIST].dwViewFlags = 0;
	strncpy(GI[COD2_SERVERLIST].szGAME_NAME,"Call of Duty 2",MAX_PATH);
	strncpy(GI[COD2_SERVERLIST].szMasterServerIP,"cod2master.activision.com",MAX_PATH);
	GI[COD2_SERVERLIST].dwMasterServerPORT = 20710;
	GI[COD2_SERVERLIST].dwProtocol = 0;
	strncpy(GI[COD2_SERVERLIST].szMAP_MAPPREVIEW_PATH,"cod2maps",MAX_PATH);
	strncpy(GI[COD2_SERVERLIST].szGAME_CMD,"",MAX_PATH);
	dwBuffSize = sizeof(GI[COD2_SERVERLIST].szGAME_PATH);
	//Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Id\\ETQW Beta 2","EXEString",GI[ETQW_SERVERLIST].szGAME_PATH,&dwBuffSize);
	//if(strlen(GI[ETQW_SERVERLIST].szGAME_PATH)>0)
		GI[COD2_SERVERLIST].bActive = false;
	//else
	//	GI[ETQW_SERVERLIST].bActive = false;
	strcpy(GI[COD2_SERVERLIST].szProtocolName,"cod2");
	GI[COD2_SERVERLIST].dwDefaultPort = 28960;
	strcpy(GI[COD2_SERVERLIST].szQueryString,"");
	GI[COD2_SERVERLIST].pSC = &SC[COD2_SERVERLIST];

	GI[COD_SERVERLIST].cGAMEINDEX = COD_SERVERLIST;
	GI[COD_SERVERLIST].iIconIndex = Get_GameIcon(COD_SERVERLIST);
	GI[COD_SERVERLIST].dwViewFlags = 0;
	strncpy(GI[COD_SERVERLIST].szGAME_NAME,"Call of Duty",MAX_PATH);
	strncpy(GI[COD_SERVERLIST].szMasterServerIP,"codmaster.activision.com",MAX_PATH);
	GI[COD_SERVERLIST].dwMasterServerPORT = 20510;
	GI[COD_SERVERLIST].dwProtocol = 5;
	strncpy(GI[COD_SERVERLIST].szMAP_MAPPREVIEW_PATH,"codmaps",MAX_PATH);
	strncpy(GI[COD_SERVERLIST].szGAME_CMD,"",MAX_PATH);
	dwBuffSize = sizeof(GI[COD_SERVERLIST].szGAME_PATH);
	//Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Activision\\Call of Duty","EXEString",GI[ETQW_SERVERLIST].szGAME_PATH,&dwBuffSize);
	//if(strlen(GI[ETQW_SERVERLIST].szGAME_PATH)>0)
		GI[COD_SERVERLIST].bActive = false;
	//else
	//	GI[ETQW_SERVERLIST].bActive = false;
	strcpy(GI[COD_SERVERLIST].szProtocolName,"cod");
	GI[COD_SERVERLIST].dwDefaultPort = 28960;
	strcpy(GI[COD_SERVERLIST].szQueryString,"");
	GI[COD_SERVERLIST].pSC = &SC[COD_SERVERLIST];


	GI[WARSOW_SERVERLIST].cGAMEINDEX = WARSOW_SERVERLIST;

	GI[WARSOW_SERVERLIST].iIconIndex = Get_GameIcon(WARSOW_SERVERLIST);
	GI[WARSOW_SERVERLIST].dwViewFlags = 0;
	strncpy(GI[WARSOW_SERVERLIST].szGAME_NAME,"Warsow",MAX_PATH);
	strncpy(GI[WARSOW_SERVERLIST].szMasterServerIP,"dpmaster.deathmask.net",MAX_PATH);
	GI[WARSOW_SERVERLIST].dwMasterServerPORT = 27950;
	GI[WARSOW_SERVERLIST].dwProtocol = 10;
	strncpy(GI[WARSOW_SERVERLIST].szMAP_MAPPREVIEW_PATH,"warsowmaps",MAX_PATH);
	strncpy(GI[WARSOW_SERVERLIST].szGAME_CMD,"",MAX_PATH);
	dwBuffSize = sizeof(GI[WARSOW_SERVERLIST].szGAME_PATH);

	//"C:\Program Files\Warsow\warsow_x86.exe" +set fs_usehomedir 1 +set fs_basepath "C:/Program Files/Warsow"
	strcpy(GI[WARSOW_SERVERLIST].szGAME_PATH,"C:\\Program Files\\Warsow\\warsow_x86.exe");
	strcpy(GI[WARSOW_SERVERLIST].szGAME_CMD,"+set fs_usehomedir 1 +set fs_basepath \"C:/Program Files/Warsow\"");

#ifdef _DEBUG
	GI[WARSOW_SERVERLIST].bActive = true;
#else
	GI[WARSOW_SERVERLIST].bActive = false;
#endif


	strcpy(GI[WARSOW_SERVERLIST].szQueryString,"Warsow");
	strcpy(GI[WARSOW_SERVERLIST].szProtocolName,"warsow");
	GI[WARSOW_SERVERLIST].dwDefaultPort = 28960;
	GI[WARSOW_SERVERLIST].pSC = &SC[WARSOW_SERVERLIST];



	GI[COD4_SERVERLIST].cGAMEINDEX = COD4_SERVERLIST;
	GI[COD4_SERVERLIST].iIconIndex = Get_GameIcon(COD4_SERVERLIST);
	GI[COD4_SERVERLIST].dwViewFlags = 0;
	strncpy(GI[COD4_SERVERLIST].szGAME_NAME,"Call of Duty 4",MAX_PATH);
	strncpy(GI[COD4_SERVERLIST].szMasterServerIP,"cod4master.activision.com",MAX_PATH);
	GI[COD4_SERVERLIST].dwMasterServerPORT = 20810;
	GI[COD4_SERVERLIST].dwProtocol = 0;
	strncpy(GI[COD4_SERVERLIST].szMAP_MAPPREVIEW_PATH,"cod4maps",MAX_PATH);
	strncpy(GI[COD4_SERVERLIST].szGAME_CMD,"+set cl_playintro 0 +set ui_skip_titlescreen 1 +set ui_skip_legalscreen 1",MAX_PATH);
	dwBuffSize = sizeof(GI[COD4_SERVERLIST].szGAME_PATH);

	//strcpy(GI[COD4_SERVERLIST].szGAME_PATH,"C:\\Program Files\\Activision\\Call of Duty 4 - Modern Warfare\\iw3mp.exe");
	
	Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Activision\\Call of Duty 4","EXEStringM",GI[COD4_SERVERLIST].szGAME_PATH,&dwBuffSize);
	if(strlen(GI[COD4_SERVERLIST].szGAME_PATH)>0)
		GI[COD4_SERVERLIST].bActive = true;
	else
		GI[COD4_SERVERLIST].bActive = false;
	strcpy(GI[COD4_SERVERLIST].szProtocolName,"cod4");
	GI[COD4_SERVERLIST].dwDefaultPort = 28960;
	strcpy(GI[COD4_SERVERLIST].szQueryString,"");
	GI[COD4_SERVERLIST].pSC = &SC[COD4_SERVERLIST];


	GI[CS_SERVERLIST].cGAMEINDEX = CS_SERVERLIST;
	GI[CS_SERVERLIST].iIconIndex = Get_GameIcon(CS_SERVERLIST);
	GI[CS_SERVERLIST].dwViewFlags = 0;
	strncpy(GI[CS_SERVERLIST].szGAME_NAME,"Counter-Strike",MAX_PATH);
	strncpy(GI[CS_SERVERLIST].szMasterServerIP,"hl1master.steampowered.com",MAX_PATH);
	GI[CS_SERVERLIST].dwMasterServerPORT = 27010;
	GI[CS_SERVERLIST].dwProtocol = 0;
	strncpy(GI[CS_SERVERLIST].szMAP_MAPPREVIEW_PATH,"csmaps",MAX_PATH);
	strncpy(GI[CS_SERVERLIST].szGAME_CMD,"-game cstrike",MAX_PATH);  //http://developer.valvesoftware.com/wiki/Command_line
	dwBuffSize = sizeof(GI[CS_SERVERLIST].szGAME_PATH);

	

	//Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Activision\\Call of Duty 4","EXEStringM",GI[CSS_SERVERLIST].szGAME_PATH,&dwBuffSize);
	GI[CS_SERVERLIST].bActive = false;
	strcpy(GI[CS_SERVERLIST].szProtocolName,"cs");
	GI[CS_SERVERLIST].dwDefaultPort = 27015;
	strcpy(GI[CS_SERVERLIST].szQueryString,"\\gamedir\\cstrike");
	GI[CS_SERVERLIST].pSC = &SC[CS_SERVERLIST];


	GI[CSCZ_SERVERLIST].cGAMEINDEX = CSCZ_SERVERLIST;
	GI[CSCZ_SERVERLIST].iIconIndex = Get_GameIcon(CSCZ_SERVERLIST);
	GI[CSCZ_SERVERLIST].dwViewFlags = 0;
	strncpy(GI[CSCZ_SERVERLIST].szGAME_NAME,"Counter-Strike: Condition Zero",MAX_PATH);
	strncpy(GI[CSCZ_SERVERLIST].szMasterServerIP,"hl1master.steampowered.com",MAX_PATH);
	GI[CSCZ_SERVERLIST].dwMasterServerPORT = 27010;
	GI[CSCZ_SERVERLIST].dwProtocol = 0;
	strncpy(GI[CSCZ_SERVERLIST].szMAP_MAPPREVIEW_PATH,"csczmaps",MAX_PATH);
	strncpy(GI[CS_SERVERLIST].szGAME_CMD,"-game czero",MAX_PATH); 
	dwBuffSize = sizeof(GI[CSCZ_SERVERLIST].szGAME_PATH);
	//Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Activision\\Call of Duty 4","EXEStringM",GI[CSS_SERVERLIST].szGAME_PATH,&dwBuffSize);
	GI[CSCZ_SERVERLIST].bActive = false;
	strcpy(GI[CSCZ_SERVERLIST].szProtocolName,"cscz");
	GI[CSCZ_SERVERLIST].dwDefaultPort = 27015;
	strcpy(GI[CSCZ_SERVERLIST].szQueryString,"\\gamedir\\czero");
	GI[CSCZ_SERVERLIST].pSC = &SC[CSCZ_SERVERLIST];

	GI[CSS_SERVERLIST].cGAMEINDEX = CSS_SERVERLIST;
	GI[CSS_SERVERLIST].iIconIndex = Get_GameIcon(CSS_SERVERLIST);
	GI[CSS_SERVERLIST].dwViewFlags = 0;
	strncpy(GI[CSS_SERVERLIST].szGAME_NAME,"Counter-Strike: Source",MAX_PATH);
	strncpy(GI[CSS_SERVERLIST].szMasterServerIP,"hl2master.steampowered.com",MAX_PATH);
	GI[CSS_SERVERLIST].dwMasterServerPORT = 27011;
	GI[CSS_SERVERLIST].dwProtocol = 0;
	strncpy(GI[CSS_SERVERLIST].szMAP_MAPPREVIEW_PATH,"cssmaps",MAX_PATH);
	strncpy(GI[CSS_SERVERLIST].szGAME_CMD,"-game cstrike",MAX_PATH);  //http://developer.valvesoftware.com/wiki/Command_line
	dwBuffSize = sizeof(GI[CSS_SERVERLIST].szGAME_PATH);
	strcpy(GI[CSS_SERVERLIST].szGAME_PATH,"HL2.EXE");
	GI[CSS_SERVERLIST].bActive = false;

	//Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Activision\\Call of Duty 4","EXEStringM",GI[CSS_SERVERLIST].szGAME_PATH,&dwBuffSize);
	
	strcpy(GI[CSS_SERVERLIST].szProtocolName,"css");
	GI[CSS_SERVERLIST].dwDefaultPort = 28960;
	strcpy(GI[CSS_SERVERLIST].szQueryString,"\\gamedir\\cstrike");
	GI[CSS_SERVERLIST].pSC = &SC[CSS_SERVERLIST];


	GI[QW_SERVERLIST].cGAMEINDEX = QW_SERVERLIST;
	GI[QW_SERVERLIST].dwMasterServerPORT = 27000;
	GI[QW_SERVERLIST].dwProtocol = 0;
	GI[QW_SERVERLIST].dwViewFlags = 0;
	strncpy(GI[QW_SERVERLIST].szGAME_NAME,"Quake World",MAX_PATH);
	strncpy(GI[QW_SERVERLIST].szMasterServerIP,"http://www.quakeservers.net/lists/servers/global.txt",MAX_PATH);  //satan.idsoftware.com:27000
	strncpy(GI[QW_SERVERLIST].szMAP_MAPPREVIEW_PATH,"qwmaps",MAX_PATH);
	strncpy(GI[QW_SERVERLIST].szGAME_CMD,"",MAX_PATH);
	GI[QW_SERVERLIST].bActive = false;
	strcat_s(GI[QW_SERVERLIST].szGAME_PATH,sizeof(GI[QW_SERVERLIST].szGAME_PATH),"quakeworld.exe");
	strcpy_s(GI[QW_SERVERLIST].szProtocolName,sizeof(GI[QW_SERVERLIST].szProtocolName),"qw");
	GI[QW_SERVERLIST].dwDefaultPort = 27960;
	strcpy(GI[QW_SERVERLIST].szQueryString,"");
	GI[QW_SERVERLIST].bUseHTTPServerList = TRUE;


	GI[Q2_SERVERLIST].cGAMEINDEX = Q2_SERVERLIST;
	GI[Q2_SERVERLIST].dwMasterServerPORT = 27900;
	GI[Q2_SERVERLIST].dwProtocol = 34;
	GI[Q2_SERVERLIST].dwViewFlags = 0;
	strncpy(GI[Q2_SERVERLIST].szGAME_NAME,"Quake 2",MAX_PATH);
	strncpy(GI[Q2_SERVERLIST].szMasterServerIP,"master.q2servers.com",MAX_PATH); //master.q2servers.com:27900
	strncpy(GI[Q2_SERVERLIST].szMAP_MAPPREVIEW_PATH,"q2maps",MAX_PATH);
	strncpy(GI[Q2_SERVERLIST].szGAME_CMD,"",MAX_PATH);
	GI[Q2_SERVERLIST].bActive = false;
	strcat_s(GI[Q2_SERVERLIST].szGAME_PATH,sizeof(GI[Q2_SERVERLIST].szGAME_PATH),"quake2.exe");
	strcpy_s(GI[Q2_SERVERLIST].szProtocolName,sizeof(GI[Q2_SERVERLIST].szProtocolName),"q2");
	GI[Q2_SERVERLIST].dwDefaultPort = 27960;
	strcpy(GI[Q2_SERVERLIST].szQueryString,"");
	GI[Q2_SERVERLIST].bUseHTTPServerList = FALSE;

	strcpy(GI[ET_SERVERLIST].szFilename,"et.servers");
	strcpy(GI[ETQW_SERVERLIST].szFilename,"etqw.servers");
	strcpy(GI[Q4_SERVERLIST].szFilename,"q4.servers");
	strcpy(GI[Q3_SERVERLIST].szFilename,"q3.servers");
	strcpy(GI[RTCW_SERVERLIST].szFilename,"rtcw.servers");
	strcpy(GI[COD_SERVERLIST].szFilename,"cod.servers");
	strcpy(GI[COD2_SERVERLIST].szFilename,"cod2.servers");
	strcpy(GI[WARSOW_SERVERLIST].szFilename,"warsow.servers");
	strcpy(GI[COD4_SERVERLIST].szFilename,"cod4.servers");
	strcpy(GI[CS_SERVERLIST].szFilename,"cs.servers");
	strcpy(GI[QW_SERVERLIST].szFilename,"qw.servers");
	strcpy(GI[Q2_SERVERLIST].szFilename,"q2.servers");
	strcpy(GI[CSCZ_SERVERLIST].szFilename,"cscz.servers");
	strcpy(GI[CSS_SERVERLIST].szFilename,"css.servers");

	RegisterProtocol(EXE_PATH);
}

void Default_Appsettings()
{

	AddLogInfo(ETSV_INFO,"Settings set to defaults.");
	Default_GameSettings();
	
	ZeroMemory(&AppCFG,sizeof(APP_SETTINGS_NEW));

	AppCFG.dwID = CFG_VER;  
	AppCFG.dwVersion = 14;
	AppCFG.bAutostart = FALSE;
	AppCFG.bUse_ETpro_path = TRUE;
	AppCFG.bUse_minimize = TRUE;
	AppCFG.dwMinimizeMODKey =MOD_ALT;
	AppCFG.cMinimizeKey = 'Z';
	AppCFG.bViewModeAdvance = FALSE;
	AppCFG.bLogging = FALSE;
	AppCFG.bUSE_SCREEN_RESTORE = FALSE;
	AppCFG.filter.dwGameTypeFilter = 0;

	AppCFG.dwThreads = 32;
	AppCFG.bUseFilterOnPing = FALSE;

	memset(AppCFG.szEXT_EXE_CMD,0,MAX_PATH);	
	memset(AppCFG.szEXT_EXE_PATH,0,MAX_PATH);
	
	AppCFG.bUse_EXT_APP = FALSE;

	strcpy(AppCFG.szEXT_EXE_PATH,"C:\\Program Files\\Teamspeak2_RC2\\TeamSpeak.exe");
	strcpy(AppCFG.szEXT_EXE_CMD,"127.0.0.1?nickname=MyNick?loginname=MyLoginAccount?password=XYZ?channel=Axis");
	strcpy(AppCFG.szEXT_EXE_WINDOWNAME,"TEAMSPEAK 2");
	strcpy(AppCFG.szET_WindowName,"Enemy Territory|Wolfenstein|Quake4|F.E.A.R.|ETQW|Warsow");
	strcpy(AppCFG.szET_CMD,"");

	AppCFG.filter.bNoEmpty = FALSE;
	AppCFG.filter.bNoFull = FALSE;
	AppCFG.filter.bNoPrivate =FALSE;
	AppCFG.filter.bPunkbuster = FALSE;
	AppCFG.filter.bPure = FALSE;
	AppCFG.filter.bRanked = FALSE;
	AppCFG.filter.dwPing = 0;
	AppCFG.filter.bNoBots = FALSE;

	AppCFG.filter.dwShowServerWithMaxPlayers = 0;
	AppCFG.filter.dwShowServerWithMinPlayers = 0;
	AppCFG.filter.cActiveMaxPlayer = 0;
	AppCFG.filter.cActiveMinPlayer = 0;

	AppCFG.socktimeout.tv_sec = 1;
	AppCFG.socktimeout.tv_usec  = 0;
	AppCFG.dwRetries = 0;

	AppCFG.g_cTransparancy = 100;

	AppCFG.filter.dwPing = 9999;
	AppCFG.filter.dwPing = AppCFG.filter.dwPing;
	AppCFG.cBuddyColumnSort = 0; 
	AppCFG.bSortBuddyAsc = TRUE;
	AppCFG.bShowBuddyList = TRUE;
	AppCFG.bShowMapPreview = FALSE;

	//AppCFG.bUseFilterOnFavorites = true;
	AppCFG.bPlayNotifySound   = TRUE;
	AppCFG.bShowServerRules = TRUE;
	AppCFG.bShowPlayerList = TRUE;
	AppCFG.bSortPlayerAsc = TRUE;
	
	AppCFG.bUseCountryFilter = FALSE;

	strcpy(AppCFG.szNotifySoundWAVfile,"notify.wav");
	AppCFG.bUseMIRC = FALSE;

	//setup which windows should show as default
	WNDCONT[WIN_MAINTREEVIEW].bShow = TRUE;
	WNDCONT[WIN_SERVERLIST].bShow = TRUE;
	WNDCONT[WIN_BUDDYLIST].bShow = TRUE;
	WNDCONT[WIN_TABCONTROL].bShow = TRUE;
	WNDCONT[WIN_PLAYERS].bShow = TRUE;
	WNDCONT[WIN_RULES].bShow = FALSE;
	WNDCONT[WIN_LOGGER].bShow = FALSE;
	WNDCONT[WIN_STATUS].bShow = TRUE;
	WNDCONT[WIN_PROGRESSBAR].bShow = TRUE;
	WNDCONT[WIN_RCON].bShow = FALSE;
	WNDCONT[WIN_PING].bShow = FALSE;
	WNDCONT[WIN_MAPPREVIEW].bShow = TRUE;

}

void OnButtonClick_AddServer()
{
	SERVER_INFO *pSrvInf = NULL;
	SERVER_INFO pSI;
	char ip[100];
	DWORD dwPort=0;
	ZeroMemory(ip,sizeof(ip));
	GetDlgItemText(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD,ip,99);

	if(strlen(ip)==0)
	{
		Favorite_Add(true);
		return;
	}


	SplitIPandPORT(ip,dwPort);

	if(dwPort==0)
		dwPort = GI[g_currentGameIdx].dwDefaultPort;
	DWORD ret = AddServer(&GI[g_currentGameIdx],ip,  dwPort,true);
	if(ret!=0xFFFFFFFF)	
	{
		SetStatusText(ICO_INFO,"Added IP %s:%d into %s favorite serverlist.",ip,dwPort,GI[g_currentGameIdx].szGAME_NAME);
		
	   g_iCurrentSelectedServer = ret;

		try
		{
			pSI = Get_ServerInfoByIndex(g_iCurrentSelectedServer);
			if(pSI.dwIP == 0)
				return;
			
		}
		catch(const exception& e)
		{
			AddLogInfo(0,"Exception raised at OnServerSelected() Details: %s\n",e.what());
			return;
		}
		pSrvInf = &pSI;
		if(pSrvInf!=NULL)
		{
			GetServerInfo(g_currentGameIdx,pSrvInf);
			UpdateServerItem(g_iCurrentSelectedServer);
			currCV->pSC->vSI.at(pSI.dwIndex) = pSI;			
			UpdateCurrentServerUI();
			
		}//if pSrv

	}else
	{
		SetStatusText(ICO_WARNING,"Invalid IP address!");
	}
}



void OnServerDoubleClick()
{
	int i = ListView_GetSelectionMark(g_hwndListViewServer);
	if(i!=-1)
	{
		SERVER_INFO pSrv;	
		pSrv = Get_ServerInfoByIndex(i);
		LaunchGame(pSrv,&GI[g_currentGameIdx]);

	}
}

SERVER_INFO *FindServerByIPandPort(char *szIP, DWORD dwPort)
{
	vSRV_INF::iterator  iLst;
	char destPort[10];
	DWORD dwIP = NetworkNameToIP(szIP,_itoa(dwPort,destPort,10));
	if(currCV->pSC->vSI.size()>0)
	{
		for ( iLst = currCV->pSC->vSI.begin( ); iLst != currCV->pSC->vSI.end( ); iLst++ )
		{
			g_tmpSRV = *iLst;
			if((g_tmpSRV.dwIP == dwIP) && (g_tmpSRV.dwPort == dwPort))
			{
				return &g_tmpSRV;
			}
		}
	}
	return NULL;
}


SERVER_INFO g_pSIduplicate;
bool duplicate_func(SERVER_INFO pSIa) //This function is not multithread safe!!
{
		if((pSIa.dwIP == g_pSIduplicate.dwIP) && (pSIa.dwPort == g_pSIduplicate.dwPort))
			return true;
	return false;
}

/************************************************************ 
	Check if the server exsist in the current view list.

	This function is not multithread safe!!
*************************************************************/
int CheckForDuplicateServer(GAME_INFO *pGI, SERVER_INFO pSI)
{
	vSRV_INF::iterator  iResult;

	memcpy(&g_pSIduplicate,&pSI,sizeof(SERVER_INFO));

	 iResult = find_if(pGI->pSC->vSI.begin(), pGI->pSC->vSI.end(),duplicate_func);

	 if(iResult == pGI->pSC->vSI.end())
		 return -1;

	 return iResult->dwIndex;
}

DWORD AddServer(GAME_INFO *pGI,char *szIP, DWORD dwPort,bool bFavorite)
{
	SERVER_INFO pSI;
	ZeroMemory(&pSI,sizeof(SERVER_INFO));
	char destPort[10];
	
	if(strlen(szIP)<1)
		return 0xFFFFFFFF;
	
	pSI.cGAMEINDEX = pGI->cGAMEINDEX;
	strcpy(pSI.szIPaddress,szIP);
	pSI.dwIP = NetworkNameToIP(szIP,_itoa(dwPort,destPort,10));
	pSI.dwPort = dwPort;


	int iResult =CheckForDuplicateServer(pGI,pSI);
	if(iResult!=-1) //do we got an exsisting server?
	{
		 //If yes then set that server to a favorite
		if(bFavorite)
			pGI->pSC->vSI[iResult].cFavorite = 1;
		
		return pGI->pSC->vSI[iResult].dwIndex;
	}
		
	//Add a new server to current list!
	
	pSI.dwPing = 9999;
	pSI.cCountryFlag = 0;
	pSI.bNeedToUpdateServerInfo = true;
	pSI.dwIndex = pGI->pSC->vSI.size();
	if(bFavorite)
		pSI.cFavorite = 1;
	
	pGI->pSC->vSI.push_back(pSI);
	InsertServerItem(pGI,pSI);
		
	return pSI.dwIndex;
}


#define UNCHECKED_ICON 9
#define CHECKED_ICON 10
#define GRAY_CHECKED_ICON 11


DWORD TreeView_SwapDWCheckState(TVITEM  *pTVitem, DWORD &dwValue)
{
	if(pTVitem->iImage == CHECKED_ICON) //Is it checked
	{
		pTVitem->iSelectedImage =	pTVitem->iImage = UNCHECKED_ICON;  //Unchecked image
		dwValue = 0;		
	}
	else
	{
		pTVitem->iSelectedImage =	pTVitem->iImage = CHECKED_ICON;  //Checked image
		dwValue = 1;
	}			
	
	TreeView_SetItem(g_hwndMainTreeCtrl, pTVitem );
	return dwValue;
}

DWORD TreeView_SwapDWCheckStateOR(TVITEM  *pTVitem, _TREEITEM ti,DWORD *dwVal)
{
	DWORD dwState;
	if(pTVitem->iImage == CHECKED_ICON) //Is it checked
	{
		pTVitem->iSelectedImage =	pTVitem->iImage = UNCHECKED_ICON;  //Unchecked image
		*dwVal^=ti.dwValue;
		dwState = 0;
	}
	else
	{
		pTVitem->iSelectedImage =	pTVitem->iImage = CHECKED_ICON;  //Checked image
		*dwVal|=ti.dwValue;
				
		dwState = 1;
	}			
	TreeView_SetItem(g_hwndMainTreeCtrl, pTVitem );
	return dwState;
}

DWORD TreeView_SetDWCheckState (TVITEM  *pTVitem, _TREEITEM ti, BOOL active)
{
	if(active) //Is it checked
	{
		pTVitem->iSelectedImage =	pTVitem->iImage = CHECKED_ICON;  //checked image
		vTI.at(ti.dwIndex).dwState = 1;
		ti.dwState = 1;
	}
	else
	{
		pTVitem->iSelectedImage =	pTVitem->iImage = UNCHECKED_ICON;  //unChecked image
		ti.dwState = 0;
		vTI.at(ti.dwIndex).dwState = 0;

	}			
	TreeView_SetItem(g_hwndMainTreeCtrl, pTVitem );
	return ti.dwValue;
}

//Used for group selection such as Ping
int TreeView_UncheckAllTypes(char cGameIdx, DWORD dwType)
{	
	for(UINT i=0;i<vTI.size();i++)
	{
		
		if((vTI.at(i).dwType == dwType) && (vTI.at(i).cGAMEINDEX == cGameIdx) )
		{
			TVITEM  tvitem;
			ZeroMemory(&tvitem,sizeof(TVITEM));
			tvitem.hItem = vTI.at(i).hTreeItem;
			tvitem.mask = TVIF_SELECTEDIMAGE |  TVIF_IMAGE;
			TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );
			TreeView_SetDWCheckState(&tvitem, vTI.at(i), FALSE);		
		}
	}
	return 0;
}



DWORD Filter_global_edit(_TREEITEM ti, TVITEM *tvi)
{

	switch(ti.dwType)
	{
	/*	case FILTER_PB		: AppCFG.filter.bPunkbuster = TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_SHOW_PRIVATE	:   AppCFG.filter.bOnlyPrivate = TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_FULL    :  AppCFG.filter.bNoFull = TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_EMPTY	:  AppCFG.filter.bNoEmpty = TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		*/
		case FILTER_OFFLINE	 :  AppCFG.filter.bHideOfflineServers = 	TreeView_SwapDWCheckState(tvi,ti.dwState); break;
//		case FILTER_FAVORITERS : break;
//		case FILTER_BOTS	: AppCFG.filter.bNoBots = 	TreeView_SwapDWCheckState(tvi,ti.dwState); break;
//		case FILTER_HIDE_PRIVATE :  AppCFG.filter.bNoPrivate = 	TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_PING : 	
			 
			TreeView_UncheckAllTypes(-25,ti.dwType); 
			if(TreeView_SwapDWCheckState(tvi,ti.dwState)==1)
				AppCFG.filter.dwPing = ti.dwValue;
			else
				AppCFG.filter.dwPing = 0;
			break;
		case FILTER_MIN_PLY : 
				AppCFG.filter.dwShowServerWithMinPlayers = 	ti.dwValue; 
				AppCFG.filter.cActiveMinPlayer = (char) TreeView_SwapDWCheckState(tvi,ti.dwState); 
				
				break;
		case FILTER_MAX_PLY : 
				AppCFG.filter.dwShowServerWithMaxPlayers = 	ti.dwValue; 
				AppCFG.filter.cActiveMaxPlayer = (char)TreeView_SwapDWCheckState(tvi,ti.dwState); 
			break;
		case 0: break;
		default: return 0;

	}	
	return ti.dwState;
}

int Filter_game_specific_edit(GAME_INFO *pGI,_TREEITEM ti, TVITEM *tvi,int idx)
{
	switch(ti.dwType)
	{
		case FILTER_PB		: pGI->filter.bPunkbuster = TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_SHOW_PRIVATE : TreeView_UncheckAllTypes(ti.cGAMEINDEX,128); pGI->filter.bNoPrivate = FALSE; pGI->filter.bOnlyPrivate = TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_HIDE_PRIVATE : TreeView_UncheckAllTypes(ti.cGAMEINDEX,2); pGI->filter.bOnlyPrivate = FALSE; pGI->filter.bNoPrivate = 	TreeView_SwapDWCheckState(tvi,ti.dwState); break;

		case FILTER_FULL    : 	pGI->filter.bNoFull = TreeView_SwapDWCheckState(tvi,ti.dwState); break;			
		case FILTER_EMPTY	: 	 pGI->filter.bNoEmpty = TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_OFFLINE	 : pGI->filter.bHideOfflineServers = 	TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_FAVORITERS : break;
		case FILTER_BOTS	: pGI->filter.bNoBots = 	TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		
		case FILTER_PURE : pGI->filter.bPure = 	TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_RANKED : pGI->filter.bRanked = 	TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_MOD	: 
			return   	vTI.at(idx).dwState = TreeView_SwapDWCheckStateOR(tvi,ti, &GI[ti.cGAMEINDEX].filter.dwMod); break;
		case FILTER_GAMETYPE:
				return vTI.at(idx).dwState = 	TreeView_SwapDWCheckStateOR(tvi,ti,	&GI[ti.cGAMEINDEX].filter.dwGameTypeFilter); break;
		case FILTER_VERSION:
				return vTI.at(idx).dwState = 	TreeView_SwapDWCheckStateOR(tvi,ti,	&GI[ti.cGAMEINDEX].filter.dwVersion); break;
		case FILTER_MAP:
				return vTI.at(idx).dwState = 	TreeView_SwapDWCheckStateOR(tvi,ti,	&GI[ti.cGAMEINDEX].filter.dwMap); break;
		case 0: break;
		default: return 0;
	}	
	return ti.dwState;
}
void Select_all_childs(HTREEITEM hRoot, bool selected)
{
	TVITEM  tvitem;
	if(hRoot==NULL)
		return;
	char szBuffer[100];
	memset(&tvitem,0,sizeof(TVITEM));
	tvitem.hItem = hRoot;
	tvitem.cchTextMax = sizeof(szBuffer);
	tvitem.pszText = szBuffer;
	tvitem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_SELECTEDIMAGE |  TVIF_IMAGE;

	while(hRoot!=NULL)
	{
		tvitem.hItem = hRoot;

		TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem);
		int iSel = (int)tvitem.lParam;
		AddLogInfo(ETSV_DEBUG,"%d %s Action %d",iSel,szBuffer,vTI.at(iSel).dwAction);

		if(selected==false)
			tvitem.iSelectedImage =	tvitem.iImage = UNCHECKED_ICON;  //Unchecked image
		else
			tvitem.iSelectedImage =	tvitem.iImage = CHECKED_ICON;  //Checked image
		
		vTI.at(iSel).dwState = tvitem.iSelectedImage - UNCHECKED_ICON; //Set value
		TreeView_SetItem(g_hwndMainTreeCtrl, &tvitem );
		Select_all_childs(TreeView_GetNextItem( g_hwndMainTreeCtrl, hRoot, TVGN_CHILD),selected);
		hRoot =TreeView_GetNextItem( g_hwndMainTreeCtrl, hRoot, TVGN_NEXT);			
		
	}
}


HTREEITEM TreeView_GetTIByItemType(DWORD dwType)
{
	for(UINT i=0;i<vTI.size();i++)
	{
		if(vTI.at(i).dwType == dwType)
			return vTI.at(i).hTreeItem;
	}
	return NULL;
}

/*******************************************************
 
 Sets a new value depending of type.
 Return if successfully found the dwType.
	This function will only change the first occurrence, 
	therefore recommendation is that dwType is unique.

********************************************************/
BOOL TreeView_SetDWValueByItemType(DWORD dwType,DWORD dwNewValue)
{
	for(UINT i=0;i<vTI.size();i++)
	{
		if(vTI.at(i).dwType == dwType)
		{
			vTI.at(i).dwValue = dwNewValue;
			
			 if(vTI.at(i).dwAction==DO_GLOBAL_EDIT_FILTER)
			 {
				char text[256];
				sprintf(text,"%s %d",vTI.at(i).sName.c_str(),vTI.at(i).dwValue);
				TreeView_SetItemText(vTI.at(i).hTreeItem,text);
			}
			return TRUE;
		}
	}
	return FALSE;
}

BOOL TreeView_SetItemStateByNameAndType(string name,DWORD dwType,DWORD dwNewState)
{
	for(UINT i=0;i<vTI.size();i++)
	{
		if((vTI.at(i).sName == name) && (vTI.at(i).dwType == dwType) )
		{
			vTI.at(i).dwState = dwNewState;
			
			return TRUE;
		}
	}
	return FALSE;
}
/*******************************************************
 
 Gets a new value depending of type.
 Return if successfully found the dwType.
	This function will only change the first occurrence, 
	therefore recommendation is that dwType is unique.

********************************************************/
DWORD TreeView_GetDWValue(DWORD dwType,DWORD dwAction)
{
	for(UINT i=0;i<vTI.size();i++)
	{
		if((vTI.at(i).dwType == dwType) && (vTI.at(i).dwAction==dwAction))
		{		

			return vTI.at(i).dwValue;
		}
	}
	return 0;
}

DWORD TreeView_GetDWStateByGameType(int iGameType, DWORD dwType,DWORD dwAction)
{
	for(UINT i=0;i<vTI.size();i++)
	{
		if((vTI.at(i).cGAMEINDEX == iGameType)&& (vTI.at(i).dwType == dwType) && (vTI.at(i).dwAction==dwAction))
		{		

			return vTI.at(i).dwState;
		}
	}
	return 0;
}

HTREEITEM TreeView_GetTIByItemGame(char game)
{
	for(UINT i=0;i<vTI.size();i++)
	{
		//Will return first occurens of game (= game root)
		if(vTI.at(i).cGAMEINDEX == game)
			return vTI.at(i).hTreeItem;
	}
	return NULL;
}

int TreeView_GetIndexByHItemTree(HTREEITEM hItemtree)
{
	for(UINT i=0;i<vTI.size();i++)
	{
		if(vTI.at(i).hTreeItem == hItemtree)
			return i;
	}
	return -1;
}




int TreeView_SetAllChildItemExpand(int startIdx, bool expand)
{
	DWORD dwLevel=0;
	dwLevel = vTI.at(startIdx).dwLevel;
	for(UINT i=startIdx;i<vTI.size();i++)
	{
		vTI.at(i).bExpanded = expand;
		if(vTI.at(i).dwLevel <dwLevel)
			break;
			
	}
	return -1;
}

BOOL TreeView_SetCheckBoxState(HTREEITEM hTreeItem,int iSel,DWORD dwState)
{
	TVITEM  tvitem;
	ZeroMemory(&tvitem,sizeof(TVITEM));
	tvitem.hItem = hTreeItem;
	tvitem.mask = TVIF_SELECTEDIMAGE |  TVIF_IMAGE;

	if(dwState) //Is it checked
		tvitem.iSelectedImage =	tvitem.iImage = CHECKED_ICON;  //Checked image
	else
		tvitem.iSelectedImage =	tvitem.iImage = UNCHECKED_ICON;  //Unchecked image
	
	vTI.at(iSel).dwState = tvitem.iSelectedImage - UNCHECKED_ICON ; //set the value
	return TreeView_SetItem(g_hwndMainTreeCtrl, &tvitem );	
}

int TreeView_GetSelectionV3()
{
	HTREEITEM hTreeItem = TreeView_GetSelection(g_hwndMainTreeCtrl);
    iSelected = hTreeItem;
	if(hTreeItem==NULL)
	{
		AddLogInfo(ETSV_DEBUG,  "Inside TreeView_GetSelection2 hTreeItem=NULL");
		return DO_NOTHING;
	}
	TVITEM  tvitem;
	ZeroMemory(&tvitem,sizeof(TVITEM));
	char szBuffer[100];
	tvitem.hItem = hTreeItem;
	tvitem.cchTextMax = sizeof(szBuffer);
	tvitem.pszText = szBuffer;
	tvitem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_SELECTEDIMAGE |  TVIF_IMAGE;
	TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );

	int iSel = (int)tvitem.lParam;
	
	AddLogInfo(ETSV_DEBUG,"%d %s Action %d",iSel,szBuffer,vTI.at(iSel).dwAction);

	switch(vTI.at(iSel).dwAction)
	{
		case DO_NOTHING_: return DO_NOTHING; 
		case DO_REDRAW_SERVERLIST: 
			return SetCurrentViewTo(vTI.at(iSel).cGAMEINDEX); 
		case DO_CHECKBOX: 
			{
				TreeView_SetCheckBoxState(hTreeItem,iSel,vTI.at(iSel).dwState);
			}	
			break; 
		case DO_EDIT:
			{
			//HWND hwndEditCtrl = TreeView_EditLabel(g_hwndMainTreeCtrl,&tvitem);
			break;
			}
		case DO_FAV_NO_FILT	: SetCurrentViewTo(vTI.at(iSel).cGAMEINDEX); return DO_NOTHING;
		case DO_FAV_PUB	: SetCurrentViewTo(vTI.at(iSel).cGAMEINDEX); return SHOW_FAVORITES_PUBLIC;
		case DO_FAV_PRIV : SetCurrentViewTo(vTI.at(iSel).cGAMEINDEX); return SHOW_FAVORITES_PRIVATE;
		case DO_HISTORY : SetCurrentViewTo(vTI.at(iSel).cGAMEINDEX); return SHOW_HISTORY;
		case DO_GAME_SPECIFIC_FILTER:		
			vTI.at(iSel).dwState = Filter_game_specific_edit(&GI[vTI.at(iSel).cGAMEINDEX],vTI.at(iSel), &tvitem,iSel);
			break;
		case DO_GLOBAL_EDIT_FILTER:
		case DO_GLOBAL_FILTER:
			vTI.at(iSel).dwState = Filter_global_edit(vTI.at(iSel), &tvitem);
			break;
		
		case DO_COUNTRY_FILTER:
			{
				HTREEITEM hChild;
				hChild = TreeView_GetNextItem(g_hwndMainTreeCtrl, tvitem.hItem, TVGN_CHILD);
				if(vTI.at(iSel).dwState)
				{   //Is it checked
					tvitem.iSelectedImage =	tvitem.iImage = UNCHECKED_ICON;  //Unchecked image
					Select_all_childs(hChild,false);
				}
				else
				{
					tvitem.iSelectedImage =	tvitem.iImage = CHECKED_ICON;  //Checked image
					Select_all_childs(hChild,true);
				}
				vTI.at(iSel).dwState = tvitem.iSelectedImage - UNCHECKED_ICON ; //set the state
				TreeView_SetItem(g_hwndMainTreeCtrl, &tvitem );
				
				hRootCountryFiltersItem = TreeView_GetTIByItemType(1001);
				Initialize_CountryFilter();
				//Build_CountryFilter();
			break;
				
			}

	}

	TreeView_SelectItem(g_hwndMainTreeCtrl,NULL);	
	SetCurrentViewTo(g_currentGameIdx);
	RedrawServerListThread(&GI[g_currentGameIdx]);
	//Initialize_RedrawServerListThread();
	return DO_NOTHING;
}



void Initialize_CountryFilter()
{
	CountryFilter.counter=0;
	memset(&CountryFilter.countryIndex,0,sizeof(CountryFilter.countryIndex));

	TVITEM  tvitem;
	memset(&tvitem,0,sizeof(TVITEM));
	hRootCountryFiltersItem = TreeView_GetTIByItemType(1001);
	tvitem.hItem = hRootCountryFiltersItem;
	tvitem.mask = TVIF_SELECTEDIMAGE |  TVIF_IMAGE;
	TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );

	if(	tvitem.iImage == 9) //Unchecked image
		AppCFG.bUseCountryFilter = false;
	else
		AppCFG.bUseCountryFilter = true;

	dbg_print("Entering Build_CountryFilter() func.");
	Build_CountryFilter(hRootCountryFiltersItem);
}

DWORD Build_CountryFilter(HTREEITEM hRoot)
{
	
	HTREEITEM hCurrent=NULL;
	HTREEITEM hChild=NULL;
	char szBuffer[55];
	TVITEM  tvitem;
	DWORD dwRet = 0;
	DWORD dwReturn = 0;
	memset(&tvitem,0,sizeof(TVITEM));
	tvitem.hItem = hRoot;
	tvitem.pszText = szBuffer;
	tvitem.cchTextMax = sizeof(szBuffer);
	tvitem.mask = TVIF_SELECTEDIMAGE |  TVIF_IMAGE | TVIF_TEXT | TVIF_PARAM;

	hCurrent = hRoot;
	hChild = TreeView_GetNextItem( g_hwndMainTreeCtrl, hRoot, TVGN_CHILD);
	if(hChild!=NULL)
	{
		dwRet = Build_CountryFilter(hChild);
		if(dwRet>0)
		{
			dwReturn = 1;
			tvitem.hItem = hRoot;
			TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );
			if(	tvitem.iImage != CHECKED_ICON)
			{
				tvitem.iImage  = GRAY_CHECKED_ICON;
				TreeView_SetItem(g_hwndMainTreeCtrl, &tvitem );
			}
		} else
		{
			tvitem.hItem = hRoot;
			TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );
			if(	tvitem.iImage == GRAY_CHECKED_ICON)
			{
				tvitem.iImage  = UNCHECKED_ICON;
				TreeView_SetItem(g_hwndMainTreeCtrl, &tvitem );
			}

		}
	} 

	while(hCurrent!=NULL)
	{

		tvitem.hItem = hCurrent;
		TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );
#ifdef _DEBUG
//		char szDebugTxt[100];
//		sprintf(szDebugTxt,"%d %s - (%d) dwRet = %d  dwReturn = %d\n",tvitem.iImage,tvitem.pszText,tvitem.lParam,dwRet, dwReturn);
//		dbg_print(szDebugTxt);
#endif
		if(	tvitem.iImage == CHECKED_ICON) //Checked country
		{

			dwReturn = 1;
			int iSel = (int)tvitem.lParam;
			strncpy(CountryFilter.szShortCountryName[CountryFilter.counter],vTI.at(iSel).strValue.c_str(),4);
			CountryFilter.counter++;
		}
		
		hCurrent =TreeView_GetNextItem( g_hwndMainTreeCtrl, hCurrent, TVGN_NEXT);
		if(hCurrent!=NULL)
		{
			hChild = TreeView_GetNextItem( g_hwndMainTreeCtrl, hCurrent, TVGN_CHILD);
			if(hChild!=NULL)
			{
				dwRet = Build_CountryFilter(hChild);
				if(dwRet>0)
				{		
					dwReturn = 1;
					tvitem.hItem = hCurrent;
					TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );
					if(	tvitem.iImage != CHECKED_ICON)
					{
						tvitem.iImage  = 11;
						TreeView_SetItem(g_hwndMainTreeCtrl, &tvitem );
					} 

				}else
				{
					tvitem.hItem = hCurrent;
					TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );
					if(	tvitem.iImage == GRAY_CHECKED_ICON)
					{
						tvitem.iImage  = UNCHECKED_ICON;
						TreeView_SetItem(g_hwndMainTreeCtrl, &tvitem );
					}

				}
			}
		}
		
	}
	return dwReturn;
}



DWORD MyPing(char *ipaddress)
{
	HANDLE hIcmpFile;
    unsigned long ipaddr = INADDR_NONE;
    DWORD dwRetVal = 0;
    char SendData[] = "Data Buffer";
    LPVOID ReplyBuffer = NULL;
    DWORD ReplySize = 0;
	
    // Validate the parameters

    ipaddr = inet_addr(ipaddress);
    if (ipaddr == INADDR_NONE) {
        dbg_print("usage:  IP address\n");
        return 999;
    }
    
    hIcmpFile = IcmpCreateFile();
    if (hIcmpFile == INVALID_HANDLE_VALUE) {
        dbg_print("\tUnable to open handle.\n");
        dbg_print("IcmpCreatefile returned error: %ld\n", GetLastError() );
        return 999;
    }    

    ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
    ReplyBuffer = (VOID*) malloc(ReplySize);
    if (ReplyBuffer == NULL) {
        dbg_print("\tUnable to allocate memory\n");
        return 999;
    }    
    
    PICMP_ECHO_REPLY pEchoReply;
    dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), 
        NULL, ReplyBuffer, ReplySize, 1000);

    if (dwRetVal != 0) {
        pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
        struct in_addr ReplyAddr;
        ReplyAddr.S_un.S_addr = pEchoReply->Address;
        printf("\tSent icmp message to %s\n", ipaddress);
        if (dwRetVal > 1) {
            dbg_print("\tReceived %ld icmp message responses\n", dwRetVal);
            dbg_print("\tInformation from the first response:\n"); 
        }    
        else {    
            dbg_print("\tReceived %ld icmp message response\n", dwRetVal);
            dbg_print("\tInformation from this response:\n"); 
        }    
        dbg_print("\t  Received from %s\n", inet_ntoa( ReplyAddr ) );
        dbg_print("\t  Status = %ld\n", 
            pEchoReply->Status);
        dbg_print("\t  Roundtrip time = %ld milliseconds\n", 
            pEchoReply->RoundTripTime);
    }
    else {

        dbg_print("\tCall to IcmpSendEcho failed.\n");
        dbg_print("\tIcmpSendEcho returned error: %ld\n", GetLastError() );
        return 999;
    }
    return  pEchoReply->RoundTripTime;
}    



// Message handler for about box.
LRESULT CALLBACK AddServerProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		
		//SetDlgItemText(hDlg,IDC_EDIT_PORT,"27960");
		CenterWindow(hDlg);
		SetFocus(GetDlgItem(hDlg,IDC_EDIT_IP));
		//return TRUE;
	break;	

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BUTTON_OK)	
		{
			char ip[80];
			DWORD dwPort=0;

			GetDlgItemText(hDlg,IDC_EDIT_IP,ip,79);
		
			SplitIPandPORT(ip,dwPort);
			
			if(dwPort==0)
				dwPort = GI[g_currentGameIdx].dwDefaultPort;

			if(AddServer(&GI[g_currentGameIdx],ip, dwPort,true)!=0xFFFFFFFF)
			//if(Q4_AddServer(GI[g_currentGameIdx].pStartServerInfo,ip,dwPort,GI[g_currentGameIdx].cGAMEINDEX,InsertServerItemQ4,true)!=-1)
			{
				SetStatusText(ICO_INFO,"Added IP %s:%d into %s favorite serverlist.",ip,dwPort,GI[g_currentGameIdx].szGAME_NAME);
				
			}else
			{
				SetStatusText(ICO_WARNING,"Invalid IP address!",ip);
			}		
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		else if(LOWORD(wParam) == IDC_BUTTON_CANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}		
	
		break;
	}
	return FALSE;
}


bool ShouldWeSelect(LPARAM lParam)
{
	for(int i=0; i<CountryFilter.counter;i++)
	{
		if (CountryFilter.lParam[i]==lParam)
			return true;
	}
	return false;
}

void Select_item_by_lparam(HTREEITEM hRoot)
{

	char szBuffer[55];
	HTREEITEM hCurrent=NULL;
	HTREEITEM hChild=NULL;
	TVITEM  tvitem;
	memset(&tvitem,0,sizeof(TVITEM));
	tvitem.pszText = szBuffer;
	tvitem.cchTextMax = sizeof(szBuffer);

	tvitem.hItem = hRoot;
	tvitem.mask = TVIF_SELECTEDIMAGE |  TVIF_IMAGE | TVIF_PARAM | TVIF_TEXT;

	hChild = TreeView_GetNextItem( g_hwndMainTreeCtrl, hRoot, TVGN_CHILD);
	if(hChild!=NULL)
		Select_item_by_lparam(hChild);
	
	hCurrent = hRoot;
	while(hCurrent!=NULL)
	{
		tvitem.hItem = hCurrent;
		TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );
#ifdef _DEBUG
		char szDebugTxt[100];	
		sprintf(szDebugTxt,"%d %s - (%d)\n",tvitem.iImage,tvitem.pszText,tvitem.lParam);
		dbg_print(szDebugTxt);
#endif

		if( ShouldWeSelect(tvitem.lParam))
		{
			tvitem.iSelectedImage =	tvitem.iImage = CHECKED_ICON;  //checked image			
			TreeView_SetItem(g_hwndMainTreeCtrl, &tvitem );
		}

		hCurrent =TreeView_GetNextItem( g_hwndMainTreeCtrl, hCurrent, TVGN_NEXT);
		
		if(hCurrent!=NULL)
		{
			hChild = TreeView_GetNextItem( g_hwndMainTreeCtrl, hCurrent, TVGN_CHILD);
			if(hChild!=NULL)
			{
				Select_item_by_lparam(hChild);
				tvitem.hItem = hChild;
				TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );
#ifdef _DEBUG
				char szDebugTxt[100];				
				sprintf(szDebugTxt,">%d %s - (%d)\n",tvitem.iImage,tvitem.pszText,tvitem.lParam);
				dbg_print(szDebugTxt);
#endif
				if( ShouldWeSelect(tvitem.lParam))
				{
					tvitem.iSelectedImage =	tvitem.iImage = CHECKED_ICON;  //checked image			
					TreeView_SetItem(g_hwndMainTreeCtrl, &tvitem );
				}
			}
		}
		
	}
}



void Select_item_and_all_childs(HTREEITEM hRoot, bool selected)
{
	HTREEITEM hCurrent=NULL;
	HTREEITEM hChild=NULL;
	TVITEM  tvitem;
	memset(&tvitem,0,sizeof(TVITEM));
	tvitem.hItem = hRoot;
	tvitem.mask = TVIF_SELECTEDIMAGE |  TVIF_IMAGE ;

	hChild = TreeView_GetNextItem( g_hwndMainTreeCtrl, hRoot, TVGN_CHILD);
	if(hChild!=NULL)
	{
		Select_item_and_all_childs(hChild,selected);
		hCurrent = hChild;
	}
	else
		hCurrent = hRoot;

	while(hCurrent!=NULL)
	{
		if(selected==false)
			tvitem.iSelectedImage =	tvitem.iImage = UNCHECKED_ICON;  //Unchecked image
		else
			tvitem.iSelectedImage =	tvitem.iImage = CHECKED_ICON;  //Checked image
		
		TreeView_SetItem(g_hwndMainTreeCtrl, &tvitem );
		if(hChild!=NULL)
			hCurrent =TreeView_GetNextItem( g_hwndMainTreeCtrl, hCurrent, TVGN_NEXT);
		else
			hCurrent = NULL;
		tvitem.hItem = hCurrent;
		if(hCurrent!=NULL)
			Select_item_and_all_childs(hCurrent,selected);
		
	}
}

void TreeView_SetItemText(HTREEITEM hTI, char *szText)
{
	TVITEM  tvitem;
	memset(&tvitem,0,sizeof(TVITEM));
	tvitem.hItem = hTI;
	tvitem.mask = TVIF_TEXT ;
	tvitem.cchTextMax = strlen(szText);
	tvitem.pszText = szText;
	TreeView_SetItem(g_hwndMainTreeCtrl, &tvitem );

}

HTREEITEM MainTreeView_AddItem(_TREEITEM *ti,HTREEITEM hCurrent,bool active)
							
{

	 int iImageIndex = ti->iIconIndex+ti->dwState;
    // char *text = ti->sName.c_str();
	 bool expand = ti->bExpanded;
	
	 char text[256];
	 if(ti->dwAction==DO_GLOBAL_EDIT_FILTER)
		 sprintf(text,"%s %d",ti->sName.c_str(),ti->dwValue);
	 else
		sprintf(text,"%s",ti->sName.c_str());

	if(active==false)
	{
		g_tvIndex++;

		return NULL;
	}
		
	TVINSERTSTRUCT tvs;
	memset(&tvs,0,sizeof(TVINSERTSTRUCT));
	tvs.item.mask                   = TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
	if (!hCurrent)
	{
		tvs.hParent = TVI_ROOT;
		tvs.item.pszText            = (LPSTR)text;    
	}
	else
	{
		tvs.hParent = hCurrent;
		tvs.item.pszText            = (LPSTR)text;
	}
	
	tvs.item.lParam = g_tvIndex++;
	tvs.item.cchTextMax             = lstrlen(tvs.item.pszText) + 1;

	tvs.hInsertAfter = TVI_LAST;
	tvs.item.iImage                 = iImageIndex;
	tvs.item.iSelectedImage         = iImageIndex;//62;
	HTREEITEM hNewItem = TreeView_InsertItem(g_hwndMainTreeCtrl, &tvs);
	//TreeView_SetCheckState(g_hwndMainTreeCtrl,hNewItem,TRUE);
    
	if (hCurrent && expand)
		TreeView_Expand(g_hwndMainTreeCtrl, hCurrent, TVE_EXPAND);
	return hNewItem;
}

DWORD level=1;
int Tree_ParseChilds(TiXmlElement* childItem, HTREEITEM hTreeItem)
{
	char szName[50];
	_TREEITEM ti;
	if(childItem==NULL)
		return 0;

	level++;
	for( childItem; childItem; childItem=childItem->NextSiblingElement() )
	{	
		

		ti.sName = "No name";
		XML_GetTreeItemName(childItem,szName,sizeof(szName));		
		if(szName!=NULL)
			ti.sName = szName;

		ti.sElementName = childItem->Value();

 
		ti.cGAMEINDEX = (char)XML_GetTreeItemInt(childItem,"game");
		ti.dwType =  XML_GetTreeItemInt(childItem,"type");

		if(ti.dwType==1024)
			ti.iIconIndex = Get_GameIcon(ti.cGAMEINDEX);
		else
			ti.iIconIndex = XML_GetTreeItemInt(childItem,"icon");

		ti.dwAction = XML_GetTreeItemInt(childItem,"action");
		ti.dwValue =  XML_GetTreeItemInt(childItem,"value");
		ZeroMemory(szName,sizeof(szName));
		XML_GetTreeItemStrValue(childItem,szName,sizeof(szName));
		ti.strValue = "zz";
		if(szName!=NULL)
			ti.strValue = szName;

		ti.dwState =  XML_GetTreeItemInt(childItem,"state");
		ti.bExpanded = (bool) XML_GetTreeItemInt(childItem,"expanded");
		ti.dwLevel = level;
		ti.dwIndex = vTI.size()+1;
		bool active=true;
		if(ti.cGAMEINDEX!=-25)
		{
			if(ti.cGAMEINDEX<MAX_SERVERLIST)
				active = GI[ti.cGAMEINDEX].bActive;
		}
				
		ti.hTreeItem = MainTreeView_AddItem(&ti,hTreeItem, active);
		vTI.push_back(ti);
		if(ti.sElementName == "GameTypesItems")
		{
			GAMEFILTER gf;
			gf.sFriendlyName = ti.sName;
			gf.sStrValue = ti.strValue;
			gf.dwValue = ti.dwValue;
			GI[ti.cGAMEINDEX].pSC->vFilterGameType.push_back(gf);
		}
		if(ti.sElementName == "ModItems")
		{
			GAMEFILTER gf;
			gf.sFriendlyName = ti.sName;
			gf.sStrValue = ti.strValue;
			gf.dwValue = ti.dwValue;
			GI[ti.cGAMEINDEX].pSC->vFilterMod.push_back(gf);
		}
		if(ti.sElementName == "Version")
		{
			GAMEFILTER gf;
			gf.sFriendlyName = ti.sName;
			gf.sStrValue = ti.strValue;
			gf.dwValue = ti.dwValue;
			GI[ti.cGAMEINDEX].pSC->vFilterVersion.push_back(gf);
		}
		if(ti.sElementName == "Map")
		{
			GAMEFILTER gf;
			gf.sFriendlyName = ti.sName;
			gf.sStrValue = ti.strValue;
			gf.dwValue = ti.dwValue;
			GI[ti.cGAMEINDEX].pSC->vFilterMap.push_back(gf);
		}
		Tree_ParseChilds(childItem->FirstChildElement(),ti.hTreeItem );

	}
	level--;

	return 0;
}

//Ugly hack
UINT g_save_counter=0;
DWORD  Save_all_by_level(TiXmlElement *pElemRoot,DWORD dwlevel)
{
	if(g_save_counter>vTI.size())
		return 0;
	
	while(dwlevel==vTI.at(g_save_counter).dwLevel)
	{
			
		int iSel = g_save_counter;

		TiXmlElement * elem = new TiXmlElement( vTI.at(iSel).sElementName.c_str());  
	
	//	char padding[40];
	//	padding[0]=0;
	//	for(int i=0;i<dwlevel;i++)
	//		strcat(padding," ");
	//	AddLogInfo(ETSV_DEBUG,"%s %d %d %s %s Action %d level:%d",padding,iSel,dwlevel,vTI.at(iSel).sElementName.c_str(),vTI.at(iSel).sName.c_str(),vTI.at(iSel).dwAction,vTI.at(iSel).dwLevel);
		
		elem->SetAttribute("name",vTI.at(iSel).sName.c_str());
		elem->SetAttribute("strval",vTI.at(iSel).strValue.c_str());
		elem->SetAttribute("value",vTI.at(iSel).dwValue);
		elem->SetAttribute("icon",vTI.at(iSel).iIconIndex);
		elem->SetAttribute("expanded",(UINT)vTI.at(iSel).bExpanded);
		elem->SetAttribute("type",vTI.at(iSel).dwType);
		elem->SetAttribute("state",vTI.at(iSel).dwState);
		elem->SetAttribute("game",vTI.at(iSel).cGAMEINDEX);
		elem->SetAttribute("action",vTI.at(iSel).dwAction);
			
		g_save_counter++;
		if(g_save_counter>=vTI.size())
		{
				pElemRoot->LinkEndChild( elem );  

			return 0;
		}
		pElemRoot->LinkEndChild( elem );  
		DWORD nextlevel = vTI.at(g_save_counter).dwLevel;
		DWORD lvl=0;
		if(nextlevel>dwlevel)
		{	lvl = Save_all_by_level(elem,nextlevel);

			if(lvl!=dwlevel)
				return lvl;
		}

		
		if(g_save_counter>=vTI.size())
			return 0;		
		
		
	}
	return vTI.at(g_save_counter).dwLevel;
}

int TreeView_save()
{
	AddLogInfo(ETSV_WARNING,"Saving treeview state in progress...");
	TiXmlDocument doc;  
 	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );  
	doc.LinkEndChild( decl );  
 
	TiXmlElement * root = new TiXmlElement( "TreeViewCFG" );  //TreeView config
	root->SetAttribute("Version",TREEVIEW_VERSION);

	TiXmlComment * comment = new TiXmlComment();
	comment->SetValue("Settings for Game Scanner treeview." );  
	root->LinkEndChild( comment );  

	TiXmlElement * TreeItems = new TiXmlElement( "TreeItems" );  

	HTREEITEM hRoot = TreeView_GetRoot(g_hwndMainTreeCtrl);
	
	
	g_save_counter=0;
	if(!vTI.empty())
		Save_all_by_level(TreeItems,vTI.at(g_save_counter).dwLevel);

	//Save_all_childs(TreeItems,hRoot);


	root->LinkEndChild( TreeItems );  
	doc.LinkEndChild( root );  

	char szFilePath[_MAX_PATH+_MAX_FNAME];
	ZeroMemory(szFilePath,sizeof(szFilePath));
	strncpy(szFilePath,USER_SAVE_PATH,strlen(USER_SAVE_PATH));
	strcat(szFilePath,"treeviewcfg.xml");
	SetCurrentDirectory(USER_SAVE_PATH);

	if(doc.SaveFile(szFilePath))
		AddLogInfo(ETSV_WARNING,"Success saving Treeview XML file!");
	else
		AddLogInfo(ETSV_WARNING,"Error saving Treeview XML file!");

	AddLogInfo(ETSV_WARNING,"Saving treeview state in progress... DONE!");
	return 0;
}

//Returns 0 if a new version has detected otherwise non zero.
int TreeView_CheckForUpdate(const char *szCurrentVersion)
{
		
	char szFilePath[_MAX_PATH+_MAX_FNAME];
	ZeroMemory(szFilePath,sizeof(szFilePath));
	strncpy(szFilePath,EXE_PATH,strlen(EXE_PATH));
	strcat(szFilePath,"\\treeviewcfg.new");
	AddLogInfo(ETSV_INFO,"Trying to load TreeView.new file from %s",szFilePath);
	SetCurrentDirectory(EXE_PATH);
	//TiXmlDocument doc(szFilePath);		
	TiXmlDocument doc(szFilePath);
	if (!doc.LoadFile(szFilePath)) 
	{
		SetCurrentDirectory(USER_SAVE_PATH);
		AddLogInfo(ETSV_ERROR,"Error loading TreeView.new from %s",szFilePath);
		return 1;
	}

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;


	pElem=hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{
		AddLogInfo(ETSV_ERROR,"Error finding first element in treeviewcfg.NEW file.");
		return 1;
	}

	const char *szP;
	szP = pElem->Value(); //= ETSV_ti_cfg
	char  szVersion[20];
	ZeroMemory(szVersion,sizeof(szVersion));
	XML_GetTreeItemStr(pElem, "Version",szVersion,sizeof(szVersion)-1);
	AddLogInfo(ETSV_INFO,"Detected treeview.NEW cfg file version is %s ",szVersion);
	if(szVersion!=NULL)
	{
		if(szCurrentVersion!=NULL)
		{
			if(strcmp(szVersion,szCurrentVersion)>0)
			{
			
				AddLogInfo(ETSV_INFO,"New version detected %s != %s",szVersion,szCurrentVersion);
				strcpy(TREEVIEW_VERSION,szVersion);
			}
			else
				return 2;
		}else
		{
			AddLogInfo(ETSV_INFO,"New version detected",szVersion);
			strcpy(TREEVIEW_VERSION,szVersion);
		}
		
		return 0; //New version detected
	}
	return 1;

}

void TreeView_BuildList()
{   
	g_tvIndex = 0;	
	TreeView_DeleteAllItems(g_hwndMainTreeCtrl);
	TreeView_load();
	return;
}

int TreeView_load()
{
	char szFilePath[_MAX_PATH+_MAX_FNAME];
	ZeroMemory(szFilePath,sizeof(szFilePath));
	strncpy(szFilePath,USER_SAVE_PATH,strlen(USER_SAVE_PATH));
	strcat(szFilePath,"treeviewcfg.xml");
	//strcat(szFilePath,"treetest.xml");//"treeviewcfg.xml");
	AddLogInfo(ETSV_INFO,"Trying to load TreeView from %s",szFilePath);
	SetCurrentDirectory(USER_SAVE_PATH);
	TiXmlDocument doc(szFilePath);
	if (!doc.LoadFile()) 
	{
		AddLogInfo(ETSV_ERROR,"Error loading config file for TreeView from USER_SAVE_PATH (%s)",szFilePath);

		ZeroMemory(szFilePath,sizeof(szFilePath));
		strncpy(szFilePath,EXE_PATH,strlen(EXE_PATH));
		strcat(szFilePath,"\\treeviewcfg.xml");
		SetCurrentDirectory(EXE_PATH);
	
		if (!doc.LoadFile(szFilePath)) 
		{
			SetCurrentDirectory(USER_SAVE_PATH);
			AddLogInfo(ETSV_ERROR,"Second error loading config file for TreeView from EXE_PATH (%s)",szFilePath);
			ZeroMemory(szFilePath,sizeof(szFilePath));
			strncpy(szFilePath,EXE_PATH,strlen(EXE_PATH));
			strcat(szFilePath,"\\treeviewcfg.new");
			SetCurrentDirectory(EXE_PATH);
			
			if (!doc.LoadFile(szFilePath)) 
			{
					AddLogInfo(ETSV_ERROR,"Error loading default TreeView file from EXE_PATH (%s)",szFilePath);
				return 1;
			}
		}
		SetCurrentDirectory(USER_SAVE_PATH);
		AddLogInfo(ETSV_INFO,"Success loading TreeView config file from USER_SAVE_PATH (%s)",szFilePath);
	}

	g_bTREELOADED = TRUE;

	TiXmlHandle hDoc(&doc);
	
	TiXmlDocument docNew;

	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);


	pElem=hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem)
	{
		AddLogInfo(ETSV_ERROR,"Error finding first element in treeviewcfg.xml file.");
		DebugBreak();
		return 1;
	}

	const char *szP;
	szP = pElem->Value(); //= ETSV_ti_cfg
	
	ZeroMemory(TREEVIEW_VERSION,sizeof(TREEVIEW_VERSION));

	XML_GetTreeItemStr(pElem, "Version",TREEVIEW_VERSION,sizeof(TREEVIEW_VERSION)-1);

	if(strlen(TREEVIEW_VERSION)>0)
		AddLogInfo(ETSV_INFO,"Current TreeView CFG file version is %s.",TREEVIEW_VERSION);
		
	if(TreeView_CheckForUpdate(TREEVIEW_VERSION)==0)
	{
		//new version detected
		ZeroMemory(szFilePath,sizeof(szFilePath));
		strncpy(szFilePath,EXE_PATH,strlen(EXE_PATH));
		strcat(szFilePath,"\\treeviewcfg.new");
		SetCurrentDirectory(EXE_PATH);		
		if(!docNew.LoadFile(szFilePath))
		{
			AddLogInfo(ETSV_ERROR,"Error loading NEW treeviewcfg file.");
			//continue with the old one
		}else
		{
			TiXmlHandle hDocNew(&docNew);
			pElem=hDocNew.FirstChildElement().Element();
			if (!pElem)
			{
				AddLogInfo(ETSV_ERROR,"Error finding first element in NEW treeviewcfg file.");
				return 1;
		
			}
			//Clear all old filtering settings... ugly hack.
			for(int i=0; i<MAX_SERVERLIST; i++)
				ZeroMemory(&GI[i].filter,sizeof(FILTER_SETTINGS));
		}
			
	}

	vTI.clear();
	// save this for later
	hRoot=TiXmlHandle(pElem);

	TiXmlElement* child = hRoot.FirstChild( "TreeItems" ).ToElement();

	level=1;
	for( child; child; child=child->NextSiblingElement() )
	{
		Tree_ParseChilds(child->FirstChildElement(),NULL);
	}	
	char szBuffer[200];

	for(int i=0; i<MAX_SERVERLIST;i++)
	{
		GI[i].hTI = TreeView_GetTIByItemGame(i);

		sprintf(szBuffer,"%s (%d)",GI[i].szGAME_NAME,GI[i].dwTotalServers);
		if(GI[i].hTI!=NULL)
		{
			TreeView_SetItemText(GI[i].hTI,szBuffer);
			TreeView_SetItemState(g_hwndMainTreeCtrl,GI[i].hTI,TVIS_BOLD ,TVIS_BOLD);
			GI[i].filter.bHideOfflineServers = TreeView_GetDWStateByGameType(i,1,9);
			GI[i].filter.bNoBots = TreeView_GetDWStateByGameType(i,64,9);
			GI[i].filter.bNoEmpty = TreeView_GetDWStateByGameType(i,8,9);
			GI[i].filter.bNoFull = TreeView_GetDWStateByGameType(i,4,9);
			GI[i].filter.bNoPrivate = TreeView_GetDWStateByGameType(i,128,9);
			GI[i].filter.bOnlyPrivate = TreeView_GetDWStateByGameType(i,2,9);
			GI[i].filter.bPunkbuster = TreeView_GetDWStateByGameType(i,1,9);
			GI[i].filter.bPure = TreeView_GetDWStateByGameType(i,26,9);
			GI[i].filter.bRanked = TreeView_GetDWStateByGameType(i,27,9);
		}
	}
	 /* AppCFG.filter.bNoPrivate = 	TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_PING : 	
	
                <Ping name="Ping 200" strval="" value="200" icon="9" expanded="0" type="28" state="0" game="-25" action="8" />
                <Ping name="Ping 150" strval="" value="150" icon="9" expanded="0" type="28" state="0" game="-25" action="8" />
                <Ping name="Ping 100" strval="" value="100" icon="9" expanded="0" type="28" state="0" game="-25" action="8" />
                <Ping name="Ping 50" strval="" value="50" icon="9" expanded="0" type="28" state="0" game="-25" action="8" />
*/
	for(int i=0; i<vTI.size(); i++)
	{
		if((vTI.at(i).sElementName == "Ping") && (vTI.at(i).dwValue==AppCFG.filter.dwPing))
		{
			TreeView_SetCheckBoxState(vTI.at(i).hTreeItem,i,vTI.at(i).dwState);
		}
	}

	AppCFG.filter.bHideOfflineServers = TreeView_GetDWValue(16,8);
	AppCFG.filter.dwShowServerWithMinPlayers = TreeView_GetDWValue(100,11);
	AppCFG.filter.dwShowServerWithMaxPlayers = TreeView_GetDWValue(101,11);

	Initialize_CountryFilter();
	SetFocus(g_hwndMainTreeCtrl);
	return 0;
}



void Initialize_GetServerListThread(DWORD options)
{

	HANDLE hThread=NULL; 
	DWORD dwThreadIdBrowser;
	hThread = NULL;					
	hThread = CreateThread( NULL, 0, &GetServerListThread, (LPVOID)options,0, &dwThreadIdBrowser);                
	if (hThread == NULL) 
	{
		AddLogInfo(ETSV_WARNING, "CreateThread failed (%d)\n", GetLastError() ); 
	}
	else 
	{
		SetThreadName( dwThreadIdBrowser, "GetServerListThread");
		CloseHandle( hThread );
	}
}




void OnActivate_ServerList(DWORD options)
{
	
	//if(g_bRunningQueryServerList==false)
	{
		HANDLE hThread=NULL; 
		DWORD dwThreadIdBrowser;
		hThread = NULL;					
		hThread = CreateThread( NULL, 0, &GetServerList, (LPVOID)options,0, &dwThreadIdBrowser);                
		if (hThread == NULL) 
		{
			AddLogInfo(ETSV_WARNING, "CreateThread failed (%d)\n", GetLastError() ); 
		}
		else 
		{
			SetThreadName( dwThreadIdBrowser, "GetServerList");
			CloseHandle( hThread );
		}
	}
}


PLAYERDATA *pCurrentPL=NULL; //Current player list in listview

PLAYERDATA * Copy_PlayerToCurrentPL(LPPLAYERDATA &pStartPD, PLAYERDATA *pNewPD)
{
	PLAYERDATA *pTmp=NULL;
	pTmp = pStartPD;
	if(pTmp!=NULL)
		while(pTmp->pNext!=NULL)
			pTmp = pTmp->pNext;

	PLAYERDATA *player = (PLAYERDATA *)calloc(1,sizeof(PLAYERDATA));
	memcpy(player,pNewPD,sizeof(PLAYERDATA));
	if(pNewPD->szClanTag!=NULL)
		player->szClanTag = _strdup(pNewPD->szClanTag);
	if(pNewPD->szPlayerName!=NULL)
		player->szPlayerName = _strdup(pNewPD->szPlayerName);
	player->pNext = NULL;
	if(pTmp==NULL)
		return pStartPD = pTmp = player;
	else
		return 	pTmp->pNext = player;
}


long UpdatePlayerListQ3(PLAYERDATA *pQ4ply)
{
	UTILZ_CleanUp_PlayerList(pCurrentPL);
	pCurrentPL = NULL;
	LVITEM item;
	ZeroMemory(&item, sizeof(LVITEM));
	item.mask = LVIF_TEXT |  LVIF_PARAM;
	item.iSubItem = 0;
	ListView_DeleteAllItems(g_hwndListViewPlayers);
	int n=0;
	char num[10];
	  if(g_hwndListViewPlayers!=NULL)
	  {
		  while (pQ4ply!=NULL)
		  {
			item.mask = LVIF_TEXT | LVIF_PARAM;
			//ZeroMemory(&item, sizeof(LVITEM));
			//item.mask = LVIF_TEXT ;
			item.iItem=n;
			sprintf(num,"%d",n+1);
			item.pszText = num;
	
			//Potential mem leak, may have to rewrite/improve this part of code
			PLAYERDATA *pPD = Copy_PlayerToCurrentPL(pCurrentPL,pQ4ply);  //This will keep a copy of the playerlist during scanning

			item.lParam = (LPARAM)pPD; //pQ4ply;	

			ListView_InsertItem( g_hwndListViewPlayers,&item);
			char colFiltered[100];
			
			if(pQ4ply->szClanTag!=NULL)
				ListView_SetItemText(g_hwndListViewPlayers,item.iItem,1,colorfilter(pQ4ply->szClanTag,colFiltered,sizeof(colFiltered)-1));
			
			ListView_SetItemText(g_hwndListViewPlayers,item.iItem,2,colorfilter(pQ4ply->szPlayerName,colFiltered,sizeof(colFiltered)-1));

			sprintf(colFiltered,"%d",pQ4ply->rate);
			ListView_SetItemText(g_hwndListViewPlayers,item.iItem,3,colFiltered);
				
			sprintf(colFiltered,"%d",pQ4ply->ping);
			ListView_SetItemText(g_hwndListViewPlayers,item.iItem,4,colFiltered);
			
	
			pQ4ply = pQ4ply->pNext;
			//ListView_SetItemCount(g_hwndListViewPlayers, n);
			n++;
		  }
	  }
/*ListView_SetColumnWidth(g_hwndListViewPlayers,0,LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(g_hwndListViewPlayers,1,LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(g_hwndListViewPlayers,2,LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(g_hwndListViewPlayers,3,LVSCW_AUTOSIZE);
	PostMessage(g_hWnd,WM_SIZE,0,0);
*/	//	ListView_SetColumnWidth(g_hwndListViewPlayers,4,LVSCW_AUTOSIZE);
	return 0;
}

long UpdateRulesList(SERVER_RULES *pServerRules)
{
	LVITEM item;
	item.iSubItem = 0;

	int n=0;
	char num[4];
	  if(g_hwndListViewVars!=NULL)
	  {
		  while (pServerRules!=NULL)
		  {
			if(pServerRules->name!=NULL)
			{
				ZeroMemory(&item, sizeof(LVITEM));
				item.mask = LVIF_TEXT | LVIF_PARAM;
				item.iItem=n;
				sprintf(num,"%d",n+1);
				item.pszText = pServerRules->name;
				item.cchTextMax = (int)strlen(pServerRules->name);
				item.lParam = (LPARAM)pServerRules;

				ListView_InsertItem( g_hwndListViewVars,&item);
		
				ListView_SetItemText(g_hwndListViewVars,item.iItem,1,pServerRules->value);
				n++;
			} 
			pServerRules = pServerRules->pNext;
			//ListView_SetItemCount(g_hwndListViewPlayers, n);
			
		  }
	  }

	return 0;
}


void ClearAllServerLinkedList()
{
	for(int i=0;i<MAX_SERVERLIST;i++)
	{
		OutputDebugString(GI[i].szGAME_NAME);
		OutputDebugString(" - CLEAN UP SERVERLIST\n");
		
		GI[i].dwTotalServers = 0;
		GI[i].pSC->vSI.clear();
		GI[i].pSC->vRefListSI.clear();
		GI[i].pSC->vRefScanSI.clear();
	}
}

//Quick connect from the EDIT control
void FastConnect()
{
	char ip[200],password[50];

	GetDlgItemText(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD,ip,199);
	GetDlgItemText(g_hwndRibbonBar,IDC_EDIT_QUICK_PASSWORD,password,49);
	char *p=NULL;
	p = strchr(ip,' ');
	if(p!=NULL)
		p[0]=0;		

	SERVER_INFO srv;
	ZeroMemory(&srv,sizeof(SERVER_INFO));
	DWORD dwPort;	
	if(ip[0]==0)
	{
		SetStatusText(ICO_WARNING,"Invalid IP address!");
		
		//Try from serverlist if any server selected
		StartGame_ConnectToServer(false);
		return;
	}
	strcpy(srv.szIPaddress,SplitIPandPORT(ip,dwPort));
	srv.dwPort = dwPort;
	strcpy(srv.szPRIVATEPASS,password);
	srv.cGAMEINDEX = -1;
	srv.dwIndex = 999999;
	LaunchGame(srv,&GI[g_currentGameIdx]);
}

GAME_INFO * Get_CurrentViewByServer(SERVER_INFO* pSrv)
{
	return &GI[pSrv->cGAMEINDEX];	 
}


int FindFirstActiveGame()
{
	for(int i=0;i<MAX_SERVERLIST;i++)
		if(GI[i].bActive)
			return i;

		int i = MessageBox(g_hWnd,"You need to configure atleast one active game.\nDo you want to to configure it now?","Info",MB_YESNO|MB_ICONINFORMATION|MB_TOPMOST);
		if(i==IDYES)
			PostMessage(g_hWnd,WM_COMMAND,LOWORD(IDM_SETTINGS),0);

	return 0;
}

void ListView_SetHeaderSortImage(HWND listView, int columnIndex, BOOL isAscending)
{
    HWND header = ListView_GetHeader(listView);
    BOOL isCommonControlVersion6 = TRUE;  //hack to only support XP and Vista
    int columnCount = Header_GetItemCount(header);
    
	isAscending=!isAscending;


    for (int i = 0; i<columnCount; i++)
    {
        HDITEM hi = {0};
        hi.mask = HDI_FORMAT | (isCommonControlVersion6 ? 0 : HDI_BITMAP);
        Header_GetItem(header, i, &hi);
        
        //Set sort image to this column
        if (i == columnIndex)
        {
            if (isCommonControlVersion6)
            {
                hi.fmt &= ~(HDF_SORTDOWN|HDF_SORTUP);
                hi.fmt |= isAscending ? HDF_SORTUP : HDF_SORTDOWN;
            }
            else
            {
                UINT bitmapID = isAscending ? IDB_UPARROW : IDB_DOWNARROW;
                
                //If there's a bitmap, let's delete it.
                if (hi.hbm)
                    DeleteObject(hi.hbm);
                
                hi.fmt |= HDF_BITMAP|HDF_BITMAP_ON_RIGHT;
                hi.hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(bitmapID), IMAGE_BITMAP, 0,0, LR_LOADMAP3DCOLORS);
            }
			
        }
        //Remove sort image (if exists)
        //from other columns.
        else
        {
            if (isCommonControlVersion6)
                hi.fmt &= ~(HDF_SORTDOWN|HDF_SORTUP);
            else
            {
                //If there's a bitmap, let's delete it.
                if (hi.hbm)
                    DeleteObject(hi.hbm);
                
                hi.mask &= ~HDI_BITMAP;
                hi.fmt &= ~(HDF_BITMAP|HDF_BITMAP_ON_RIGHT);
            }
        }
        
        Header_SetItem(header, i, &hi);
    }
}

void OnRestore()
{
	//AddLogInfo(ETSV_INFO, "Message WM_SIZE restoring.");
	Initialize_WindowSizes();
	g_iCurrentSelectedServer = -1;
	ShowWindow(g_hwndLogger,SW_HIDE);
	ShowWindow(g_hwndMainRCON,SW_HIDE);	
	ShowWindow(g_hwndListViewVars,SW_HIDE);	

}

void ListView_SetDefaultColumns()
{
	for(int i=0;i<MAX_COLUMNS;i++)
		ZeroMemory(&CUSTCOLUMNS[i].lvColumn,sizeof(CUSTCOLUMNS[0].lvColumn));
	int idx=0;

	CUSTCOLUMNS[COL_PB].id = COL_PB;
	CUSTCOLUMNS[COL_PB].lvColumn.mask =  LVCF_WIDTH;
	CUSTCOLUMNS[COL_PB].lvColumn.cx = 20;
	CUSTCOLUMNS[COL_PB].sName = "PB";
	CUSTCOLUMNS[COL_PB].columnIdx = idx++;
	CUSTCOLUMNS[COL_PB].bActive = TRUE;

	CUSTCOLUMNS[COL_PRIVATE].id = COL_PRIVATE;
	CUSTCOLUMNS[COL_PRIVATE].lvColumn.mask =  LVCF_WIDTH;
	CUSTCOLUMNS[COL_PRIVATE].lvColumn.cx = 16;
	CUSTCOLUMNS[COL_PRIVATE].sName = "Private";
	CUSTCOLUMNS[COL_PRIVATE].columnIdx = idx++;
	CUSTCOLUMNS[COL_PRIVATE].bActive = TRUE;
	
	CUSTCOLUMNS[COL_RANKED].id = COL_RANKED;
	CUSTCOLUMNS[COL_RANKED].lvColumn.mask =  LVCF_WIDTH;
	CUSTCOLUMNS[COL_RANKED].lvColumn.cx = 16;
	CUSTCOLUMNS[COL_RANKED].sName = "Ranked";
	CUSTCOLUMNS[COL_RANKED].columnIdx = idx++;
	CUSTCOLUMNS[COL_RANKED].bActive = TRUE;

	CUSTCOLUMNS[COL_SERVERNAME].id = COL_SERVERNAME;
	CUSTCOLUMNS[COL_SERVERNAME].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_SERVERNAME].lvColumn.cx = 220;
	CUSTCOLUMNS[COL_SERVERNAME].sName = "Servername";
	CUSTCOLUMNS[COL_SERVERNAME].columnIdx = idx++;
	CUSTCOLUMNS[COL_SERVERNAME].bActive = TRUE;

	CUSTCOLUMNS[COL_VERSION].id = COL_VERSION;
	CUSTCOLUMNS[COL_VERSION].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_VERSION].lvColumn.cx = 55;
	CUSTCOLUMNS[COL_VERSION].sName = "Version";
	CUSTCOLUMNS[COL_VERSION].columnIdx = idx++;
	CUSTCOLUMNS[COL_VERSION].bActive = TRUE;

	CUSTCOLUMNS[COL_GAMETYPE].id = COL_GAMETYPE;
	CUSTCOLUMNS[COL_GAMETYPE].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_GAMETYPE].lvColumn.cx = 55;
	CUSTCOLUMNS[COL_GAMETYPE].sName = "Game Type";
	CUSTCOLUMNS[COL_GAMETYPE].columnIdx = idx++;
	CUSTCOLUMNS[COL_GAMETYPE].bActive = TRUE;

	CUSTCOLUMNS[COL_MAP].id = COL_MAP;
	CUSTCOLUMNS[COL_MAP].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_MAP].lvColumn.cx = 75;
	CUSTCOLUMNS[COL_MAP].sName = "Map";
	CUSTCOLUMNS[COL_MAP].columnIdx = idx++;
	CUSTCOLUMNS[COL_MAP].bActive = TRUE;

	CUSTCOLUMNS[COL_MOD].id = COL_MOD;
	CUSTCOLUMNS[COL_MOD].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_MOD].lvColumn.cx = 65;
	CUSTCOLUMNS[COL_MOD].sName = "Mod";
	CUSTCOLUMNS[COL_MOD].columnIdx  = idx++;
	CUSTCOLUMNS[COL_MOD].bActive = TRUE;

	CUSTCOLUMNS[COL_BOTS].id = COL_BOTS;
	CUSTCOLUMNS[COL_BOTS].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_BOTS].lvColumn.cx = 20;
	CUSTCOLUMNS[COL_BOTS].sName = "Bots";
	CUSTCOLUMNS[COL_BOTS].columnIdx = idx++;
	CUSTCOLUMNS[COL_BOTS].bActive = TRUE;

	CUSTCOLUMNS[COL_PLAYERS].id = COL_PLAYERS;
	CUSTCOLUMNS[COL_PLAYERS].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_PLAYERS].lvColumn.cx = 90;
	CUSTCOLUMNS[COL_PLAYERS].sName = "Players";
	CUSTCOLUMNS[COL_PLAYERS].columnIdx = idx++;
	CUSTCOLUMNS[COL_PLAYERS].bActive = TRUE;

	CUSTCOLUMNS[COL_COUNTRY].id = COL_COUNTRY;	
	CUSTCOLUMNS[COL_COUNTRY].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	if(AppCFG.bUseShortCountry)
		CUSTCOLUMNS[COL_COUNTRY].lvColumn.cx = 45;
	else
		CUSTCOLUMNS[COL_COUNTRY].lvColumn.cx =115;

	CUSTCOLUMNS[COL_COUNTRY].sName = "Country";
	CUSTCOLUMNS[COL_COUNTRY].columnIdx = idx++;
	CUSTCOLUMNS[COL_COUNTRY].bActive = TRUE;

	CUSTCOLUMNS[COL_PING].id = COL_PING;
	CUSTCOLUMNS[COL_PING].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_PING].lvColumn.cx = 45;
	CUSTCOLUMNS[COL_PING].sName = "Ping";
	CUSTCOLUMNS[COL_PING].columnIdx = idx++;
	CUSTCOLUMNS[COL_PING].bActive = TRUE;
	
	CUSTCOLUMNS[COL_IP].id = COL_IP;
	CUSTCOLUMNS[COL_IP].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_IP].lvColumn.cx = 150;
	CUSTCOLUMNS[COL_IP].sName = "IP";
	CUSTCOLUMNS[COL_IP].columnIdx = idx++;
	CUSTCOLUMNS[COL_IP].bActive = TRUE;

}

void ListView_InitilizeColumns()
{	
	for(int i = 0;i<MAX_COLUMNS;i++)
		ListView_DeleteColumn(g_hwndListViewServer, 0 );
				
	short idx = 0;
	for(int i=0;i<MAX_COLUMNS;i++)
	{
		for(int x=0;x<MAX_COLUMNS;x++)
		{
			if(CUSTCOLUMNS[x].bActive && (CUSTCOLUMNS[x].columnIdx==i))
			{
				CUSTCOLUMNS[x].columnIdxToSave = CUSTCOLUMNS[x].columnIdx = idx;
				CUSTCOLUMNS[x].lvColumn.pszText = (LPSTR)CUSTCOLUMNS[x].sName.c_str();
				ListView_InsertColumn(g_hwndListViewServer,CUSTCOLUMNS[x].columnIdx, &CUSTCOLUMNS[x].lvColumn);
				dbg_print("Init pos %d = %d %d %s",x,CUSTCOLUMNS[x].columnIdx,CUSTCOLUMNS[x].columnIdxToSave,CUSTCOLUMNS[x].sName.c_str());
			
				idx++;
			}
		}
	}	
	for(int i = 0;i<MAX_COLUMNS;i++)
	{	
		if(CUSTCOLUMNS[i].bActive==FALSE)
		{
			CUSTCOLUMNS[i].columnIdxToSave = CUSTCOLUMNS[i].columnIdx = idx;
			idx++;			
		}
	}	
	dbg_print("exit ListView_InitilizeColumns()\n");
}

void ListView_ReInitializeColumns()
{	
	short idx = 0;
	for(int i = 0;i<MAX_COLUMNS;i++)
		ListView_DeleteColumn(g_hwndListViewServer, 0 );
	
	for(int x = 0;x<MAX_COLUMNS;x++)
	{
		for(int i = 0;i<MAX_COLUMNS;i++)
		{	
			//CUSTCOLUMNS[i].columnIdx = CUSTCOLUMNS[i].columnIdxToSave;
			if(CUSTCOLUMNS[i].bActive && (CUSTCOLUMNS[i].columnIdx == x))
			{
				CUSTCOLUMNS[i].columnIdx = idx;
				CUSTCOLUMNS[i].lvColumn.pszText = (LPSTR) CUSTCOLUMNS[i].sName.c_str();

				dbg_print("ReInit pos: %d = %d %d %s ",x,CUSTCOLUMNS[i].columnIdx, CUSTCOLUMNS[i].columnIdxToSave,CUSTCOLUMNS[i].sName.c_str());	
				ListView_InsertColumn(g_hwndListViewServer,idx, &CUSTCOLUMNS[i].lvColumn);
				idx++;	
			}
			
		}
	}
	//give inactive columns a rest index value	
	for(int i = 0;i<MAX_COLUMNS;i++)
	{	
		if(CUSTCOLUMNS[i].bActive==FALSE)
		{
			CUSTCOLUMNS[i].columnIdxToSave = CUSTCOLUMNS[i].columnIdx = idx;
			idx++;			
		}
	}	
}


void UpdateColumnsPosAfterDrag()
{
	int iItems,	*lpiArray;
	// Get memory for buffer.

	if((iItems = SendMessage(g_hwndListViewServerListHeader, HDM_GETITEMCOUNT, 0,0))!=-1)
	{
		if(!(lpiArray =(int*) calloc(iItems,sizeof(int))))
		{
			MessageBox(NULL, "Out of memory.","Error", MB_OK);
		}
		else
		{
			SendMessage(g_hwndListViewServerListHeader, HDM_GETORDERARRAY , iItems,(LPARAM)lpiArray);
			int c=0;
			for(int i=0;i<iItems;i++)
			{
				//dbg_print("Before pos %d %d %s",i,CUSTCOLUMNS[lpiArray[i]].columnIdx,CUSTCOLUMNS[lpiArray[i]].sName.c_str());
				for(int x=0;x<MAX_COLUMNS;x++)	
				{
					if(CUSTCOLUMNS[x].bActive && (CUSTCOLUMNS[x].columnIdx==lpiArray[i] ))
					{
						CUSTCOLUMNS[x].columnIdxToSave = c;
						dbg_print("After  pos [%d]=%d colidx=%d %d %s",i,lpiArray[i],CUSTCOLUMNS[x].columnIdx,CUSTCOLUMNS[x].columnIdxToSave,CUSTCOLUMNS[x].sName.c_str());
						c++;
					}
				}

			
				
			}
		//	for(int i=0;i<MAX_COLUMNS;i++)
				for(int x=0;x<MAX_COLUMNS;x++)					
						dbg_print("After pos %d = %d %d %s",x,CUSTCOLUMNS[x].columnIdx,CUSTCOLUMNS[x].columnIdxToSave,CUSTCOLUMNS[x].sName.c_str());

			PostMessage(g_hWnd,WM_REFRESHSERVERLIST,0,0);
			free(lpiArray);
		}
	}
}





void OnCreate(HWND hwnd, HINSTANCE hInst)
{
	AddLogInfo(ETSV_INFO,"Called OnCreate...");

	LV_COLUMN   lvColumn;

	ZeroMemory(&lvColumn,sizeof(LV_COLUMN));
			
	Q3_SetCallbacks(UpdateServerItem, Buddy_CheckForBuddies, &InsertServerItem);
	Q4_SetCallbacks(UpdateServerItem, Buddy_CheckForBuddies, &InsertServerItem);
	STEAM_SetCallbacks(UpdateServerItem, Buddy_CheckForBuddies, &InsertServerItem);

	g_CurrentSRV = NULL;
	bRunningRefreshThread = FALSE;
	g_bRunningQueryServerList = false;
	bRefreshBrowserList = false;
	g_bCancel = false;
	ZeroMemory(&g_CurrentSelServer,sizeof(SERVER_INFO));

	RECT rc;
	GetClientRect(hwnd,&rc);

	g_hwndMainTreeCtrl = CreateWindowEx(WS_EX_CLIENTEDGE  ,  WC_TREEVIEW , NULL,

							WS_VISIBLE |WS_CHILDWINDOW|  WS_TABSTOP |  TVS_HASBUTTONS |  TVS_LINESATROOT | TVS_HASLINES |  TVS_DISABLEDRAGDROP  , 
							0,TOOLBAR_Y_OFFSET,50, 50, 
							hwnd, (HMENU) IDC_MAINTREE, hInst, NULL);


	WNDCONT[WIN_MAINTREEVIEW].hWnd = g_hwndMainTreeCtrl;



	g_hwndListViewServer = CreateWindowEx(LVS_EX_SUBITEMIMAGES|LVS_EX_FULLROWSELECT|WS_EX_CLIENTEDGE , WC_LISTVIEW , NULL,
							 LVS_OWNERDATA|LVS_REPORT|WS_VISIBLE |WS_CHILD | WS_TABSTOP |WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
							100+BORDER_SIZE,TOOLBAR_Y_OFFSET,200, 100, 
							hwnd, (HMENU) IDC_LIST_SERVER, hInst, NULL);

	WNDCONT[WIN_SERVERLIST].hWnd = g_hwndListViewServer;
	
	g_hwndListBuddy	 = CreateWindowEx(LVS_EX_FULLROWSELECT|WS_EX_CLIENTEDGE , WC_LISTVIEW , NULL,
							LVS_REPORT | WS_VISIBLE |WS_CHILD | WS_TABSTOP ,
							0,0+TOOLBAR_Y_OFFSET+BORDER_SIZE,50, 50, 
							hwnd, (HMENU) IDC_LIST_BUDDY, hInst, NULL);		

	WNDCONT[WIN_BUDDYLIST].hWnd = g_hwndListBuddy;

	g_hwndTabControl = CreateWindowEx(0 , WC_TABCONTROL  , NULL,
							WS_VISIBLE |WS_CHILD | WS_TABSTOP | WS_CLIPSIBLINGS,
							100+BORDER_SIZE,200+TOOLBAR_Y_OFFSET+BORDER_SIZE,100, 200, 
							hwnd, (HMENU) IDC_TAB1, hInst, NULL);


	WNDCONT[WIN_TABCONTROL].hWnd = g_hwndTabControl;

	g_hwndListViewPlayers = CreateWindowEx(LVS_EX_FULLROWSELECT|WS_EX_CLIENTEDGE , WC_LISTVIEW , NULL,
							LVS_REPORT|WS_VISIBLE |WS_CHILD | WS_TABSTOP |WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
							100+BORDER_SIZE,200+TOOLBAR_Y_OFFSET+BORDER_SIZE,100, 200, 
							hwnd, (HMENU) IDC_LIST1, hInst, NULL);

	WNDCONT[WIN_PLAYERS].hWnd = g_hwndListViewPlayers;
	
	g_hwndListViewVars = CreateWindowEx(LVS_EX_FULLROWSELECT|WS_EX_CLIENTEDGE , WC_LISTVIEW , NULL,
							LVS_REPORT|WS_VISIBLE |WS_CHILD | WS_TABSTOP |WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
							200+BORDER_SIZE*2,200+TOOLBAR_Y_OFFSET+BORDER_SIZE,100-BORDER_SIZE, 200, 
							hwnd, (HMENU) IDC_LIST2, hInst, NULL);

	WNDCONT[WIN_RULES].hWnd = g_hwndListViewVars;

	int cyVScroll = GetSystemMetrics(SM_CYVSCROLL); 		

	g_hwndLogger = CreateWindowEx(0 , WC_EDIT , NULL, 
							 WS_VSCROLL|WS_BORDER | WS_VISIBLE |  WS_CHILD | ES_LEFT | ES_MULTILINE  | ES_AUTOHSCROLL |  ES_AUTOVSCROLL   ,
								100+BORDER_SIZE,200+TOOLBAR_Y_OFFSET+BORDER_SIZE,100, 200, 
							hwnd, (HMENU) IDC_EDIT_LOGGER, hInst, NULL);

	WNDCONT[WIN_LOGGER].hWnd = g_hwndLogger;


	g_hwndStatus = CreateWindowEx(0 , WC_EDIT , NULL, 
							WS_VISIBLE |  WS_CHILD | ES_AUTOHSCROLL | ES_READONLY | ES_NOHIDESEL ,
							0,rc.bottom-cyVScroll,rc.right/2, cyVScroll, 
							hwnd, (HMENU) IDC_EDIT_STATUS, hInst, NULL);

	WNDCONT[WIN_STATUS].hWnd = g_hwndStatus;


	g_hwndProgressBar = CreateWindowEx(0, PROGRESS_CLASS,
			 (LPSTR) NULL, WS_CHILD | WS_VISIBLE|PBS_SMOOTH,
			 rc.right/2, rc.bottom-cyVScroll, rc.right/2, cyVScroll, 
			 hwnd, (HMENU) 0, hInst, NULL); 
	
	WNDCONT[WIN_PROGRESSBAR].hWnd = g_hwndProgressBar;

	WNDCONT[WIN_RCON].hWnd = InitRCON(hwnd);
	WNDCONT[WIN_PING].hWnd = InitSTATS(hwnd);

	Initialize_WindowSizes();

	LoadImageList();
	SetImageList();


	AddLogInfo(ETSV_INFO,"Called All CreateWindowEx.");

	TCITEM tci;
	ZeroMemory(&tci,sizeof(tci));
	tci.iImage = 32;
	tci.mask =  TCIF_IMAGE|TCIF_TEXT;
	tci.pszText = "Players";
	TabCtrl_InsertItem(g_hwndTabControl,0,&tci);
	tci.iImage = 38;
	tci.pszText = "Server Rules";
	TabCtrl_InsertItem(g_hwndTabControl,1,&tci);
	tci.iImage = 33;
	tci.pszText = "RCON";
	TabCtrl_InsertItem(g_hwndTabControl,2,&tci);
	tci.iImage = 37;
	tci.pszText = "Ping";
	TabCtrl_InsertItem(g_hwndTabControl,3,&tci);
	tci.iImage = 34;
	tci.pszText = "Logger";
	TabCtrl_InsertItem(g_hwndTabControl,4,&tci);



	DWORD dwExStyle = 0;

	dwExStyle = TreeView_GetExtendedStyle(g_hwndMainTreeCtrl);
	//dwExStyle |= (TVS_EX_DOUBLEBUFFER | TVS_EX_FADEINOUTEXPANDOS);
	TreeView_SetExtendedStyle(g_hwndMainTreeCtrl, WS_EX_LEFT,dwExStyle);
	
	SetWindowTheme(g_hwndMainTreeCtrl, L"explorer", 0);
	//SetWindowTheme(g_hwndListViewServer, L"explorer", 0);

	dwExStyle = ListView_GetExtendedListViewStyle(g_hwndListViewPlayers);
	dwExStyle |= LVS_EX_FULLROWSELECT ;
	ListView_SetExtendedListViewStyle(g_hwndListViewPlayers,dwExStyle);


	dwExStyle = ListView_GetExtendedListViewStyle(g_hwndListViewVars);
	dwExStyle |= LVS_EX_FULLROWSELECT ;
	ListView_SetExtendedListViewStyle(g_hwndListViewVars,dwExStyle);
	

	dwExStyle = ListView_GetExtendedListViewStyle(g_hwndListViewServer);
	dwExStyle |= LVS_EX_FULLROWSELECT ;
	dwExStyle |= LVS_EX_SUBITEMIMAGES; // | LVS_EX_LABELTIP  ;
	dwExStyle |= LVS_EX_DOUBLEBUFFER;

	ListView_SetExtendedListViewStyle(g_hwndListViewServer,dwExStyle);

	dwExStyle = ListView_GetExtendedListViewStyle(g_hwndListBuddy);
	dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES | LVS_EX_LABELTIP ;
	ListView_SetExtendedListViewStyle(g_hwndListBuddy,dwExStyle);

	//initialize the columns
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM ;
	lvColumn.fmt = LVCFMT_LEFT;


	lvColumn.cx = 30;
	int dd;
	for(dd = 0; dd <5; dd++)
	{
		if(dd==2)
			lvColumn.cx = 125;
		else if(dd==0)
			lvColumn.cx = 30;
		else
			lvColumn.cx = 60;
		lvColumn.pszText = szPlayerlistColumnString[dd];
		ListView_InsertColumn(g_hwndListViewPlayers, dd, &lvColumn);
	}
	lvColumn.cx = 150;
	lvColumn.pszText = "Rule";
	ListView_InsertColumn(g_hwndListViewVars, 0, &lvColumn);
	lvColumn.cx = 175;
	lvColumn.pszText = "Value";
	ListView_InsertColumn(g_hwndListViewVars, 1, &lvColumn);
	
	lvColumn.cx = 120;
	lvColumn.pszText = "Buddy";
	ListView_InsertColumn(g_hwndListBuddy, 0, &lvColumn);
	lvColumn.cx = 120;
	lvColumn.pszText = "Server Name";
	ListView_InsertColumn(g_hwndListBuddy, 1, &lvColumn);
	lvColumn.cx = 80;
	lvColumn.pszText = "IP";
	ListView_InsertColumn(g_hwndListBuddy, 2, &lvColumn);

	SendMessage(hwnd,WM_SETICON,ICON_SMALL, (LPARAM)hOnlineIcon);

	
	SetWindowText(hwnd,szDialogTitle);

	g_hwndRibbonBar = TOOLBAR_CreateRebar(hwnd);

	 long style = GetWindowLong(g_hwndListViewServerListHeader,GWL_STYLE);
	 style |= HDS_DRAGDROP;
	 SetWindowLong(g_hwndListViewServerListHeader,GWL_STYLE,style);


	AddLogInfo(ETSV_INFO,"Creating Subclasses");

	// Subclassing    
	g_wpOrigListViewServerProc = (LONG_PTR) SetWindowLongPtr(g_hwndListViewServer, GWLP_WNDPROC, (LONG_PTR) ListViewServerListSubclassProc); 
	g_wpOrigTreeViewProc = (LONG_PTR) SetWindowLongPtr(g_hwndMainTreeCtrl, GWLP_WNDPROC, (LONG_PTR) TreeView_SubclassProc); 
    g_wpOrigListBuddyProc = (LONG_PTR) SetWindowLongPtr(g_hwndListBuddy, GWLP_WNDPROC, (LONG_PTR) Buddy_ListViewSubclassProc); 
	g_wpOrigListViewPlayersProc = (LONG_PTR) SetWindowLongPtr(g_hwndListViewPlayers, GWLP_WNDPROC, (LONG_PTR) ListViewPlayerSubclassProc); 

	g_hwndListViewServerListHeader = ListView_GetHeader(g_hwndListViewServer);
	g_wpOrigSLHeaderProc = (LONG_PTR) SetWindowLongPtr(g_hwndListViewServerListHeader, GWLP_WNDPROC, (LONG_PTR) LV_SL_HeaderSubclassProc); 

	Load_CountryFlags();

	g_hf = SetFont(hwnd,IDC_LIST_SERVER);
	SetFontToDlgItem(hwnd,g_hf,IDC_EDIT_STATUS);
	SetFontToDlgItem(hwnd,g_hf,IDC_CUSTOM2);
	SetFontToDlgItem(g_hwndSearchToolbar,g_hf,IDC_COMBOBOXEX_CMD);
	SetFontToDlgItem(hwnd,g_hf,IDC_EDIT_LOGGER);

	ShowWindow(g_hwndLogger,SW_HIDE);
	ShowWindow(g_hwndMainRCON,SW_HIDE);	
	ShowWindow(g_hwndMainSTATS,SW_HIDE);	
	
	SendMessage(hwnd,WM_GETSERVERLIST_START,0, 0);
	
	AddLogInfo(ETSV_INFO,"Exit OnCreate...");
}


void OnClose()
{
 
	dbg_print("--- Closing down and cleaning up!\n");
	
	g_CurrentSRV=NULL;
	g_bCancel = true;

	SCANNER_bCloseApp = TRUE;
	SaveAll(0xdeadface);

	ImageList_Destroy(g_hImageListIcons);
	g_hImageListIcons= NULL;

	ImageList_Destroy(m_hImageList);
	m_hImageList = NULL;
	ImageList_Destroy(m_hImageListHot);
	m_hImageListHot = NULL;
	ImageList_Destroy(m_hImageListDis);
	m_hImageListDis = NULL;

	DeleteObject(g_hf);

	UTILZ_CleanUp_PlayerList(pCurrentPL);
	bMainWindowsRunning=false;
}

void OnDestroy(HWND hWnd)
{
	//Clean up and restore subclasses. 
	SetWindowLongPtr(g_hwndListViewServer, GWLP_WNDPROC, (LONG_PTR) g_wpOrigListViewServerProc); 
	SetWindowLongPtr(g_hwndListBuddy, GWLP_WNDPROC, (LONG_PTR) g_wpOrigListBuddyProc); 
	SetWindowLongPtr(g_hwndListViewPlayers, GWLP_WNDPROC, (LONG_PTR) g_wpOrigListViewPlayersProc); 
	SetWindowLongPtr(g_hwndMainTreeCtrl, GWLP_WNDPROC, (LONG_PTR) g_wpOrigTreeViewProc); 
	SetWindowLongPtr(g_hwndComboEdit, GWLP_WNDPROC, (LONG_PTR) g_wpOrigCBSearchProc); 
	SetWindowLongPtr(g_hwndListViewServerListHeader, GWLP_WNDPROC, (LONG_PTR) g_wpOrigSLHeaderProc); 
	g_wpOrigSLHeaderProc = NULL;
	g_wpOrigCBSearchProc = NULL;
	g_wpOrigTreeViewProc = NULL;
	g_wpOrigListViewPlayersProc = NULL;
	g_wpOrigListBuddyProc = NULL;
	g_wpOrigListViewServerProc = NULL;

	DestroyWindow(g_hwndProgressBar);
	DestroyWindow(g_hwndMainTreeCtrl);
	DestroyWindow(g_hwndListViewVars);
	DestroyWindow(g_hwndListViewPlayers);
	DestroyWindow(g_hwndListBuddy);
	DestroyWindow(g_hwndListViewServer);
	DestroyWindow(g_hwndStatus);			
	DestroyWindow(g_hwndTabControl);
	DestroyWindow(g_hwndLogger);	
	DestroyWindow(hWnd);
	g_hwndLogger = NULL;
	g_hwndTabControl = NULL;
	g_hwndListViewPlayers = NULL;
	g_hwndListViewVars = NULL;
	g_hwndListViewServer = NULL;
	g_hwndListBuddy = NULL;
	g_hwndMainTreeCtrl = NULL;
	g_hwndProgressBar = NULL;
	g_hwndStatus = NULL;		
}

void OnInitialize_MainDlg(HWND hwnd)
{
	ListView_DeleteAllItems(g_hwndListViewServer);
	ListView_DeleteAllItems(g_hwndListViewPlayers);
	SendMessage(g_hwndProgressBar, PBM_SETPOS, (WPARAM) 0, 0); 	

	strcpy(g_szMapName,"unknownmap.png");
	AddLogInfo(ETSV_INFO,"Initialize main dialog.");

	EnableButtons(TRUE);
	//EnableDownloadLink(FALSE);	
	PostMessage(g_hWnd,WM_COMMAND,IDM_UPDATE,0);
	
	CenterWindow(g_hWnd);

	g_nCurrentSortMode = COL_PLAYERS;

	SetInitialViewStates();
	
	SetDlgTrans(hwnd,AppCFG.g_cTransparancy);
	LoadAllServerList();

	 g_iCurrentSelectedServer = -1;

	CountryFilter.counter = 0;
//	Load_CountryFilter();
	
	ListView_InitilizeColumns();
	TreeView_BuildList();
	
	if(GI[g_currentGameIdx].bActive)
		SetCurrentViewTo(g_currentGameIdx);
	else
		SetCurrentViewTo(FindFirstActiveGame());

	Buddy_Load(g_pBIStart);
	Buddy_UpdateList(g_pBIStart);

	Show_ToolbarButton(IDC_BUTTON_FIND,false);
	Show_ToolbarButton(IDC_DOWNLOAD, false);
	//RedrawServerList(Get_CurrentServerListByView());
	
	SetStatusText(ICO_INFO,"Initialization done.");
}

char * TrimString(char *szIn)
{	
	if(szIn==NULL)
		return NULL;
	char *p;
	p = strrchr(szIn,'"');	
	if(p!=NULL)
		p[0] = 0;
	return &szIn[1];
}

char * ReplaceIllegalChars(char *szIn)
{
	char *p;
	int i = 0;
	p = szIn;
	while(p[i]!=0)
	{
		if(p[i]==0x0a)
			p[i] = 0x20;
		else if(p[i]==0x0d)
			p[i] = 0x20;
		else if(p[i]==0x09)
			p[i] = 0x20;
		i++;
	}
	return szIn;
}

//TODO rewrite file structure for easier future changes/additional info
void SaveServerList(GAME_INFO *pGI)
{

	//SetStatusText(ICO_INFO,"Saving...");

	//dbg_print("Saving serverlist... %s");
	SetCurrentDirectory(USER_SAVE_PATH);
	FILE *fp=NULL;
	FILE *fp2=NULL;
//	fopen_s(&fp,pGI->szFilename, "wb");
	char szFilename2[260];
	sprintf(szFilename2,"%s.csv",pGI->szFilename);
	fopen_s(&fp2,szFilename2, "wb");

	if(fp2!=NULL)
	{
			
		vSRV_INF::iterator  iLst;

		if(pGI->pSC->vSI.size()>0)
		{
			bool bWrite=false;
			for ( iLst = pGI->pSC->vSI.begin( ); iLst != pGI->pSC->vSI.end( ); iLst++ )
			{
		
				SERVER_INFO pSI = *iLst;//currCV->vSI.at((int)pLVItem->iItem);
		
				//memcpy(pSI.cIdentifier,"SERV",4);
				//pSI.dwVersion = SERVERSLISTFILE_VER;
				bWrite=false;
				if (pSI.cPurge<ETSV_PURGE_COUNTER )
					bWrite=true;
				else if (pSI.cFavorite)
					bWrite=true;


				if(bWrite)
				{

					//fwrite((const void*)&pSI, sizeof(SERVER_INFO), 1, fp);	
					char szBuffer[512];
					char sep=9;
					sprintf_s(szBuffer,sizeof(szBuffer),"%d%c\"%s\"",pSI.cGAMEINDEX,sep,ReplaceIllegalChars(pSI.szServerName));
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);	
					sprintf_s(szBuffer,sizeof(szBuffer),"%c\"%s\"%c%d%c\"%s\"",sep,pSI.szIPaddress,sep,pSI.dwPort,sep,pSI.szShortCountryName);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);	
					sprintf_s(szBuffer,sizeof(szBuffer),"%c\"%s\"%c\"%s\"%c\"%s\"%c\"%s\"",sep,pSI.szPRIVATEPASS,sep,pSI.szRCONPASS,sep,pSI.szVersion,sep,pSI.szMod);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);	
					sprintf_s(szBuffer,sizeof(szBuffer),"%c\"%s\"%c%d%c%d%c%d",sep,pSI.szMap,sep,pSI.nMaxPlayers,sep,pSI.nCurrentPlayers,sep,pSI.nPrivateClients);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);	
					sprintf_s(szBuffer,sizeof(szBuffer),"%c%d%c%d%c%d%c%d",sep,pSI.bPrivate,sep,pSI.bPunkbuster,sep,pSI.bDedicated,sep,pSI.cRanked);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);	
					sprintf_s(szBuffer,sizeof(szBuffer),"%c%d%c%d%c%d%c%d",sep,pSI.cBots,sep,pSI.cFavorite,sep,pSI.cHistory,sep,pSI.cPure);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);	
					sprintf_s(szBuffer,sizeof(szBuffer),"%c%d%c%d%c%d%c%d",sep,pSI.cGameTypeCVAR,sep,pSI.dwMap,sep,pSI.cPurge,sep,pSI.dwPing);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);	
					sprintf_s(szBuffer,sizeof(szBuffer),"%c%d%c%d",sep,pSI.wMod,sep,pSI.dwIP);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);	
					sprintf_s(szBuffer,sizeof(szBuffer),"%c\"%s\"%c%d\n",sep,pSI.szCountry,sep,pSI.dwVersion);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);	
				}
			}
		}
	//	fclose(fp);
		fclose(fp2);
	}

	SetStatusText(ICO_INFO,"Saving... DONE!");
}
//TODO rewrite file structure for easier future changes/additional info
void LoadServerListV2(GAME_INFO *pGI)
{
	SetCurrentDirectory(USER_SAVE_PATH);

	char seps[]   = "\t\n";

	FILE *fp=NULL;
	char szFilename2[260];
	sprintf(szFilename2,"%s.csv",pGI->szFilename);
	dbg_print("Loading serverlist %s ",szFilename2);
	fopen_s(&fp,szFilename2, "rb");


	char  *next_token1;

	SERVER_INFO	 srv;
	
	char *szGameType = NULL;
	char *szGameName = NULL;
	char *szIP = NULL;
	char *szPort = NULL;
	char *szShortCountryName = NULL;
	char *szByte = NULL;
	char *szPrivatePassword = NULL;
	char *szRCONPassword= NULL;
	char *szVersion  = NULL;
	char *szMod = NULL;
	char *szMap = NULL;
	char *szMaxPlayers = NULL;
	char *szCurrentNumberOfPlayers= NULL;
	char *szPrivateClients  = NULL;
	char *szPrivate = NULL;
	char *szCountry = NULL;
	pGI->dwTotalServers	 = 0;
	DWORD idx=0;
	char buffer[1024];
	if(fp!=NULL)
	{
		pGI->pSC->vSI.clear();
		while( !feof( fp ) )
		{
			memset(&buffer,0,sizeof(buffer));
			for(int i=0; i<sizeof(buffer);i++)
			{
				
				if(fread(&buffer[i], 1, 1, fp)!=0)
				{

					if(buffer[i]==10)
					{

						next_token1 = NULL;
						memset(&srv,0,sizeof(SERVER_INFO));
						szGameType= strtok_s( buffer, seps, &next_token1);
						srv.cGAMEINDEX =(char)atoi(szGameType); 
						
						szGameName = strtok_s( NULL, seps, &next_token1);
						strcpy(srv.szServerName,TrimString(szGameName));

						szIP = strtok_s( NULL, seps, &next_token1);
						strcpy(srv.szIPaddress,TrimString(szIP));
						
						szPort = strtok_s( NULL, seps, &next_token1);
						srv.dwPort = (DWORD)atol(szPort); 

						szShortCountryName = strtok_s( NULL, seps, &next_token1);
						strcpy(srv.szShortCountryName,TrimString(szShortCountryName));
						
						szPrivatePassword = strtok_s( NULL, seps, &next_token1);
						strcpy(srv.szPRIVATEPASS,TrimString(szPrivatePassword));

						szRCONPassword = strtok_s( NULL, seps, &next_token1);
						strcpy(srv.szRCONPASS,TrimString(szRCONPassword));
						
						szVersion = strtok_s( NULL, seps, &next_token1);
						strcpy(srv.szVersion,TrimString(szVersion));
						
						szMod = strtok_s( NULL, seps, &next_token1);
						strcpy(srv.szMod,TrimString(szMod));
						
						szMap = strtok_s( NULL, seps, &next_token1);
						strcpy(srv.szMap,TrimString(szMap));
						
						szMaxPlayers = strtok_s( NULL, seps, &next_token1);
						srv.nMaxPlayers = atoi(szMaxPlayers); 

						szCurrentNumberOfPlayers = strtok_s( NULL, seps, &next_token1);
						srv.nCurrentPlayers = atoi(szCurrentNumberOfPlayers); 

						szPrivateClients = strtok_s( NULL, seps, &next_token1);
						srv.nPrivateClients = atoi(szPrivateClients); 
						
						szPrivate = strtok_s( NULL, seps, &next_token1);
						srv.bPrivate = (char)atoi(szPrivate); 
						
						szByte = strtok_s( NULL, seps, &next_token1);  //Punkbuster or VAC
						srv.bPunkbuster = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  //Dedicated
						srv.bDedicated = (BOOL)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);   //Ranked
						srv.cRanked = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);   
						srv.cBots = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);   
						srv.cFavorite = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);   
						srv.cHistory = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  
						srv.cPure = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  
						srv.cGameTypeCVAR = (WORD)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  
						srv.dwMap = (char)atoi(szByte); 
						
						szByte = strtok_s( NULL, seps, &next_token1);  
						srv.cPurge = (char)atoi(szByte); 
					
						szByte = strtok_s( NULL, seps, &next_token1);  
						srv.dwPing = (DWORD)atol(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  
						srv.wMod = (WORD)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  
						srv.dwIP = (DWORD)atol(szByte); 


						szCountry = strtok_s( NULL, seps, &next_token1);
						if(szCountry!=NULL)
							strcpy(srv.szCountry,TrimString(szCountry));

						szByte = strtok_s( NULL, seps, &next_token1);  
						if(szByte!=NULL)
							srv.dwVersion = (char)atol(szByte); 

						srv.dwIndex = idx++;

						srv.pPlayerData = NULL;
						srv.pServerRules = NULL;
						srv.cLocked = 0;
						srv.bUpdated = 0;

						pGI->pSC->vSI.push_back(srv);
			
						pGI->dwTotalServers++;
						i = sizeof(buffer)+1;
					}  //endif
				}
			}  //if
		}
		fclose(fp);
	}

}

//TODO rewrite file structure for easier future changes/additional info
SERVER_INFO *LoadServerList(GAME_INFO *pGI)
{
	SERVER_INFO	 srv;
	SERVER_INFO	 *firstsrv=NULL;

	dbg_print("Loading serverlist %s",pGI->szFilename); 
	
	SetCurrentDirectory(USER_SAVE_PATH);
	FILE *fp=NULL;
	fopen_s(&fp,pGI->szFilename, "rb");

	if(fp==NULL)
	{
		//For backward compatability, so try to find the server list at the same path as the EXE file.
		SetCurrentDirectory(EXE_PATH);
		fp=fopen(pGI->szFilename, "rb");
	}


	pGI->dwTotalServers	 = 0;
	DWORD idx=0;
	if(fp!=NULL)
	{
		pGI->pSC->vSI.clear();
		while( !feof( fp ) )
		{
		
			memset(&srv,0,sizeof(SERVER_INFO));
			if(fread(&srv, sizeof(SERVER_INFO), 1, fp)!=0)	
			{
			/*	if(srv.cIdentifier[0]!='S' || srv.cIdentifier[1]!='E' || (srv.dwVersion != SERVERSLISTFILE_VER))
				{
					MessageBox(NULL,"Fatal error while loading serverlist!\nReason could be wrong version of file or corrupt file.","Error!",MB_OK|MB_ICONERROR);
					fclose(fp);
					char szFilename[50];
					sprintf(szFilename,"%s_backup",pGI->szFilename);
					remove(szFilename);
					rename(pGI->szFilename,szFilename);
					return NULL;

				}*/
			
				srv.dwIndex = idx++;

				srv.pPlayerData = NULL;
				srv.pServerRules = NULL;
				srv.cLocked = 0;
				srv.bUpdated = 0;

				pGI->pSC->vSI.push_back(srv);
	
				pGI->dwTotalServers++;
			}

		}
		fclose(fp);
	}

	return firstsrv;
}



void SaveAllServerList()
{
	for(int i=0;i<MAX_SERVERLIST;i++)
		SaveServerList(&GI[i]);
}

void LoadAllServerList()
{
	for(int i=0;i<MAX_SERVERLIST;i++)
		if(GI[i].pSC->vSI.size()==0) //Only try to load if no list exsists (needed for minimizing and restoring)
		{
			GI[i].pSC->vSI.clear();
			//LoadServerList(&GI[i]);
			LoadServerListV2(&GI[i]);
		}
}


void DeleteAllServerLists()
{
	int ret = MessageBox(NULL,"Are you sure you want to delete all servers including favorites?","Warning!",MB_OKCANCEL | MB_ICONWARNING);
	if(ret == IDOK)
	{
		if(g_bRunningQueryServerList==false)
		{
			g_CurrentSRV=NULL;
			ListView_DeleteAllItems(g_hwndListViewVars);
			ListView_DeleteAllItems(g_hwndListViewPlayers);
			ListView_DeleteAllItems(g_hwndListViewServer);
			ClearAllServerLinkedList();
			ListView_SetItemCount(g_hwndListViewServer,0);
		
			char szBuffer[100];
			for(int i=0;i<MAX_SERVERLIST;i++)
			{
				remove(GI[i].szFilename);
				GI[i].dwTotalServers = 0;
				sprintf(szBuffer,"%s (%d)",GI[i].szGAME_NAME,GI[i].dwTotalServers);
				if(GI[i].bActive)
					TreeView_SetItemText(GI[i].hTI,szBuffer);
			}
			MessageBox(NULL,"Master serverlist deleted!","Info",MB_OK);
		
		}else
			MessageBox(NULL,"You can't delete while scanning/updating servers!","Warning!",MB_OKCANCEL | MB_ICONWARNING);
	}
}



/*
	Is called when Add Server button is clicked...
*/
BOOL bWaitingToSave=FALSE;
DWORD WINAPI SavingFilesCleaningUpThread(LPVOID pvoid )
{
	if(bWaitingToSave)
		return 0;
	bWaitingToSave = TRUE;
	dbg_print("Entering SavingFilesCleaningUpThread!\n");
	if(g_bRunningQueryServerList==true)
	{
		dbg_print("Waiting for the scanner to close down!\n");
		DWORD dwWaitResult = WaitForSingleObject( 
			hCloseEvent,
			INFINITE);    // indefinite wait

		switch (dwWaitResult) 
		{
			// Both event objects were signaled.
		//	case WAIT_OBJECT_0: 
		//		dbg_print("WAIT_OBJECT_0:\n"); 
		//		break; 
			case WAIT_ABANDONED:
				dbg_print("WAIT_ABANDONED: \n"); 
				break;
			case WAIT_TIMEOUT:
				dbg_print("WAIT_TIMEOUT:\n"); 
				break;
			case WAIT_FAILED:
				dbg_print("WAIT_FAILED:\n"); 
				break;

			// An error occurred.
			default: 
				dbg_print("Wait error\n"); 
	            
		}
	}
	
	while(bRunningRefreshThread)
	{
		dbg_print("Sleeping...\n");
		Sleep(300);
	}

	SaveAllServerList();

	dbg_print("Cleaning up all linked list!\n");
	ClearAllServerLinkedList();

	Buddy_Save(g_pBIStart);
	Buddy_Clear(g_pBIStart);
	g_pBIStart = NULL;
	
	//Save_CountryFilter();
	CFG_Save();
	if((DWORD)pvoid!=0x0000FFFF)
		PostMessage(g_hWnd,WM_DESTROY,(WPARAM)pvoid,0);
	
	g_bCancel = false;
	bWaitingToSave = FALSE;
	SCANNER_bCloseApp = FALSE;
	
	return 0x1001;
}
void CFG_Save()
{

	SetCurrentDirectory(USER_SAVE_PATH);
	AddLogInfo(ETSV_DEBUG,"Saving config");

	if(g_bTREELOADED)
		TreeView_save();


//	FILE *fp=fopen(FILE_ETSERVERCFG, "wb");
//	fwrite((const void*)&AppCFG, sizeof(APP_SETTINGS_NEW), 1, fp);
//	fclose(fp);

	TiXmlDocument doc;  
	TiXmlElement  *MainVersion;
 	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );  
	doc.LinkEndChild( decl );  
 
	TiXmlElement * root = new TiXmlElement( "ETSVcfg" );  
	doc.LinkEndChild( root );  

	TiXmlComment * comment = new TiXmlComment();
	comment->SetValue(" \nSettings for ET Server Viewer\n\nONLY CHANGE VALUES IF YOU KNOW WHAT YOU ARE DOING!\n " );  
	
	root->LinkEndChild( comment );  

	TiXmlElement * versions = new TiXmlElement( "Versions" );  
	root->LinkEndChild( versions );  

	MainVersion = new TiXmlElement("MainVersion");
	MainVersion->LinkEndChild(new TiXmlText(APP_VERSION));
	versions->LinkEndChild(MainVersion);
	//---------------------------
	//Filter options
	//---------------------------
	TiXmlElement * abcd = new TiXmlElement( "Games" );  
	root->LinkEndChild( abcd );  
	
	for(int i=0;i<MAX_SERVERLIST; i++)
	{
		TiXmlElement * abc = new TiXmlElement( "Game" );  
		WriteCfgStr(abc, "GameData", "GameName",GI[i].szGAME_NAME) ;
		WriteCfgStr(abc, "GameData", "Cmd",GI[i].szGAME_CMD);
		WriteCfgStr(abc, "GameData", "Path",GI[i].szGAME_PATH);
		WriteCfgStr(abc, "GameData", "yawn",GI[i].szMAP_YAWN_PATH);
		WriteCfgStr(abc, "GameData", "MasterServer",GI[i].szMasterServerIP);
		WriteCfgStr(abc, "GameData", "MapPreview",GI[i].szMAP_MAPPREVIEW_PATH);
		WriteCfgStr(abc, "GameData", "webpath",GI[i].szMAP_OTHER_WEBPATH_PATH);
		WriteCfgStr(abc, "GameData", "splatterladder",GI[i].szMAP_SPLATTERLADDER_PATH);
		WriteCfgStr(abc, "GameData", "ProtcolName",GI[i].szProtocolName);
		WriteCfgInt(abc, "GameData", "Active",GI[i].bActive);
		WriteCfgInt(abc, "GameData", "gametype",GI[i].cGAMEINDEX);
		WriteCfgInt(abc, "GameData", "MasterServerPort",GI[i].dwMasterServerPORT);
		WriteCfgInt(abc, "GameData", "Protocol",GI[i].dwProtocol);
		WriteCfgInt(abc, "GameData", "IconIndex",GI[i].iIconIndex);
		WriteCfgInt(abc, "GameData", "FilterMod",GI[i].filter.dwMod);
		WriteCfgInt(abc, "GameData", "FilterGameType",GI[i].filter.dwGameTypeFilter);
		WriteCfgInt(abc, "GameData", "FilterHideFull",GI[i].filter.bNoFull);
		WriteCfgInt(abc, "GameData", "FilterHideEmpty",GI[i].filter.bNoEmpty);
		WriteCfgInt(abc, "GameData", "FilterPunkbuster",GI[i].filter.bPunkbuster);
		WriteCfgInt(abc, "GameData", "FilterHideOffline",GI[i].filter.bHideOfflineServers);
		WriteCfgInt(abc, "GameData", "FilterPure",GI[i].filter.bPure);
		WriteCfgInt(abc, "GameData", "FilterRanked",GI[i].filter.bRanked);
		WriteCfgInt(abc, "GameData", "FilterHideBots",GI[i].filter.bNoBots);
		WriteCfgInt(abc, "GameData", "FilterNoPrivate",GI[i].filter.bNoPrivate);
		WriteCfgInt(abc, "GameData", "FilterOnlyPrivate",GI[i].filter.bOnlyPrivate);
		abcd->LinkEndChild( abc ); 

	}

	TiXmlElement * pElemSort = new TiXmlElement( "Sort" );  
	root->LinkEndChild( pElemSort );  
	
	for(int i=0;i<MAX_COLUMNS; i++)
	{
		
		TiXmlElement * pElemSortIdx = new TiXmlElement( "ColumnsServerList" );  
		WriteCfgInt(pElemSortIdx, "Column", "Active",(int)CUSTCOLUMNS[i].bActive);	
		WriteCfgInt(pElemSortIdx, "Column", "id",(int)CUSTCOLUMNS[i].id);
		WriteCfgInt(pElemSortIdx, "Column", "Ascending",(int)CUSTCOLUMNS[i].bSortAsc);	 
		WriteCfgInt(pElemSortIdx, "Column", "columnIdx",(int)CUSTCOLUMNS[i].columnIdxToSave);
		WriteCfgInt(pElemSortIdx, "Column", "cx",(int)CUSTCOLUMNS[i].lvColumn.cx);	
		WriteCfgStr(pElemSortIdx, "Column", "strval",CUSTCOLUMNS[i].sName.c_str()) ;		
		pElemSort->LinkEndChild( pElemSortIdx ); 
		dbg_print("col idx %d",CUSTCOLUMNS[i].columnIdxToSave);
	}
	WriteCfgInt(pElemSort,"SortServerList","LastSortColumn",iLastColumnSortIndex);

	TiXmlElement * options = new TiXmlElement( "Options" );  
	root->LinkEndChild( options );  
	WriteCfgInt(options,"General","Transparancy",AppCFG.g_cTransparancy);
	WriteCfgInt(options,"General","MaxScanThreads",AppCFG.dwThreads);

	//---------------------------
	//Filter options
	//---------------------------
	TiXmlElement * filters = new TiXmlElement( "Filters" );  
	root->LinkEndChild( filters );  
	WriteCfgInt(filters,"Filter","FilterPing",AppCFG.bUseFilterOnPing);
	WriteCfgInt(filters,"Filter","HideOfflineServers",AppCFG.filter.bHideOfflineServers);

	WriteCfgInt(filters,"Filter","NumMaxPlayers",AppCFG.filter.dwShowServerWithMaxPlayers);
	WriteCfgInt(filters,"Filter","NumMinPlayers",AppCFG.filter.dwShowServerWithMinPlayers);
	WriteCfgInt(filters,"Filter","NumMaxPlayersActive",AppCFG.filter.cActiveMaxPlayer);
	WriteCfgInt(filters,"Filter","NumMinPlayersActive",AppCFG.filter.cActiveMinPlayer);
	WriteCfgInt(filters,"Filter","Ping",AppCFG.filter.dwPing);


	TiXmlElement * xmlTVVer = new TiXmlElement( "TreeView" );  
	root->LinkEndChild( xmlTVVer );  
	xmlTVVer->SetAttribute("Version", TREEVIEW_VERSION);


	//---------------------------
	//View options
	//---------------------------

	TiXmlElement * xmlSCN = new TiXmlElement( "ShortCountryName" );  
	root->LinkEndChild( xmlSCN );  
	xmlSCN->SetAttribute("enable", AppCFG.bUseShortCountry);

	TiXmlElement * xmlMapPreview = new TiXmlElement( "MapPreview" );  
	root->LinkEndChild( xmlMapPreview );  
	xmlMapPreview->SetAttribute("show", AppCFG.bShowMapPreview);

	TiXmlElement * xmlServerRules = new TiXmlElement( "ServerRules" );  
	root->LinkEndChild( xmlServerRules );  
	xmlServerRules->SetAttribute("show", AppCFG.bShowServerRules);

	TiXmlElement * xmlPlayerList = new TiXmlElement( "PlayerList" );  
	root->LinkEndChild( xmlPlayerList );  
	xmlPlayerList->SetAttribute("show", AppCFG.bShowPlayerList);

	TiXmlElement * xmlLastTabView = new TiXmlElement( "LastGameView" );  
	root->LinkEndChild( xmlLastTabView );  
	xmlLastTabView->SetAttribute("index", AppCFG.lastTabView);

	TiXmlElement * xmlAutoStart = new TiXmlElement( "AutoStart" );  
	root->LinkEndChild( xmlAutoStart );  
	xmlAutoStart->SetAttribute("enable", AppCFG.bAutostart);

	TiXmlElement * xmlMinimize = new TiXmlElement( "Minimize" );  
	root->LinkEndChild( xmlMinimize );  
	xmlMinimize->SetAttribute("enable", AppCFG.bUse_minimize);
	xmlMinimize->SetAttribute("Key", AppCFG.cMinimizeKey?AppCFG.cMinimizeKey:'Z');
	xmlMinimize->SetAttribute("ModKey", AppCFG.dwMinimizeMODKey?AppCFG.dwMinimizeMODKey:MOD_ALT);
	xmlMinimize->SetAttribute("WindowNames",AppCFG.szET_WindowName);

	TiXmlElement * xmlBuddylist = new TiXmlElement( "BuddyList" );  
	root->LinkEndChild( xmlBuddylist );  
	xmlBuddylist->SetAttribute("show", AppCFG.bShowBuddyList);
	xmlBuddylist->SetAttribute("SortAscending", AppCFG.bSortBuddyAsc );
	xmlBuddylist->SetAttribute("ColumnSort", AppCFG.cBuddyColumnSort);

	TiXmlElement * xmlmIRC = new TiXmlElement( "mIRC" );  
	root->LinkEndChild( xmlmIRC );  
	xmlmIRC->SetAttribute("enable", AppCFG.bUseMIRC);

	
	TiXmlElement * xmlElm2 = new TiXmlElement( "OptionalEXEsettings" );  
	root->LinkEndChild( xmlElm2 );  
	xmlElm2->SetAttribute("enable", AppCFG.bUse_EXT_APP);
	xmlElm2->SetAttribute("path", strlen(AppCFG.szEXT_EXE_PATH)?AppCFG.szEXT_EXE_PATH:"no path set");
	xmlElm2->SetAttribute("cmd", AppCFG.szEXT_EXE_CMD);
	xmlElm2->SetAttribute("WindowName", AppCFG.szEXT_EXE_WINDOWNAME);
	xmlElm2->SetAttribute("CloseOnExitServer", AppCFG.bEXTClose);
	xmlElm2->SetAttribute("MinimizeOnLaunch", AppCFG.bEXTMinimize);

	TiXmlElement * xmlElm5 = new TiXmlElement( "SocketTimeout" );  
	root->LinkEndChild( xmlElm5 );  
	xmlElm5->SetAttribute("seconds", AppCFG.socktimeout.tv_sec);
	xmlElm5->SetAttribute("useconds", AppCFG.socktimeout.tv_usec);

	doc.SaveFile( "config.xml" );

	AddLogInfo(ETSV_DEBUG,"Saving config...DONE!");
}

void SetInitialViewStates()
{
	HMENU hMenu = GetMenu(g_hWnd);					
	MENUITEMINFO mii;
	memset(&mii,0,sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;

	if(AppCFG.bShowMapPreview)
		mii.fState = MFS_CHECKED;
	else	
		mii.fState = MFS_UNCHECKED;
	SetMenuItemInfo(hMenu,ID_VIEW_MAPPREVIEW,FALSE,&mii);

	if(AppCFG.bShowBuddyList)
		mii.fState = MFS_CHECKED;
	else	
		mii.fState = MFS_UNCHECKED;
	SetMenuItemInfo(hMenu,	ID_VIEW_BUDDYLIST,FALSE,&mii);
	
	if(AppCFG.bShowPlayerList)
		mii.fState = MFS_CHECKED;
	else	
		mii.fState = MFS_UNCHECKED;
	SetMenuItemInfo(hMenu,	ID_VIEW_PLAYERLIST,FALSE,&mii);

	OnSize(g_hWnd);					
}



//split/parse ip & port string indata=1.1.1.1:27960
char *SplitIPandPORT(char *szIPport,DWORD &port)
{
	port = 0;
	if(strlen(szIPport)>0)
	{
		char *p=NULL,*r=NULL;
	
		p = strchr(szIPport,':');
		
		//Added since v5.41
		r = strrchr(szIPport,'/');  //reverse find
		if(r!=NULL)
			r[0]=0;

		if(p!=NULL)
		{
			p[0]=0;
			p++;
			port = atoi(p);		
		}
		return szIPport;
	}
	return NULL;
}

void SaveAll(DWORD dwCloseReason)
{
	HANDLE hThread=NULL; 
	DWORD dwThreadIdBrowser=0;				
	hThread = CreateThread( NULL, 0, &SavingFilesCleaningUpThread,(LPVOID)dwCloseReason ,0, &dwThreadIdBrowser);                
	if (hThread == NULL) 
	{
		AddLogInfo(ETSV_WARNING, "CreateThread failed (%d)\n", GetLastError() ); 
	} else
	{
		CloseHandle( hThread );
	}
}

void OnScanButton()
{
	if(g_bRunningQueryServerList!=true)
		OnActivate_ServerList();
	else
		OnStopScanning();
}


//Doeas what it sounds and should be running as a own thread... ;)
DWORD WINAPI StopScanningThread(LPVOID pvoid )
{
	g_CurrentSRV=NULL;
	g_bCancel = true;

	SCANNER_bCloseApp = TRUE;

	if(bWaitingToSave)
		return 0;

	EnableButtons(false);
	SetStatusText(ICO_INFO,"Stopping scanner!");
	bWaitingToSave = TRUE;
	dbg_print("Entering SavingFilesCleaningUpThread!\n");
	if(g_bRunningQueryServerList==true)
	{
		dbg_print("Waiting for the scanner to close down!\n");
		DWORD dwWaitResult = WaitForSingleObject(hCloseEvent,INFINITE);    // infinite wait
		switch (dwWaitResult) 
		{
			// Both event objects were signaled.
		//	case WAIT_OBJECT_0: 
		//		dbg_print("WAIT_OBJECT_0:\n"); 
		//		break; 
			case WAIT_ABANDONED:
				dbg_print("WAIT_ABANDONED: \n"); 
				break;
			case WAIT_TIMEOUT:
				dbg_print("WAIT_TIMEOUT:\n"); 
				break;
			case WAIT_FAILED:
				dbg_print("WAIT_FAILED:\n"); 
				break;

			// An error occurred.
			default: 
				dbg_print("Wait error\n");        
		}
	}
	
	while(bRunningRefreshThread)
	{
		dbg_print("Sleeping...\n");
		Sleep(300);
	}
	g_bCancel = false;
	SCANNER_bCloseApp = FALSE;
	bWaitingToSave = FALSE;
	SetStatusText(ICO_INFO,"Scanner stopped!");
	EnableButtons(true);
	return 0;
}

//Creates the thread that will signal to stop game servers scanning
void OnStopScanning()
{
	HANDLE hThread=NULL; 
	DWORD dwThreadIdBrowser=0;				
	hThread = CreateThread( NULL, 0, &StopScanningThread,(LPVOID)0 ,0, &dwThreadIdBrowser);                
	if (hThread == NULL) 
	{
		AddLogInfo(ETSV_WARNING, "CreateThread failed (%d)\n", GetLastError() ); 
	} else
	{
		SetThreadName( dwThreadIdBrowser, "StopScanningThread");
		CloseHandle( hThread );
	}
}

void OnMinimize(HWND hWnd)
{
	g_CurrentSRV=NULL;

	g_bMinimized=true;

	SaveAllServerList();
	Buddy_Save(g_pBIStart);
	Buddy_Clear(g_pBIStart);
	g_pBIStart = NULL;

	CFG_Save();

	ShowWindow(hWnd,SW_HIDE);
	//Removed since ver 5.41
	//ShowBalloonTip("Notification","ET Server Viewer is still running.\nDouble click to activate ETSV\nRight click to exit the application.");
}

int SetCurrentViewTo(int index)
{
	g_currentGameIdx = index;
	AppCFG.lastTabView = g_currentGameIdx;
	//Clear old
	if(currCV!=NULL)
		currCV->pSC->vRefListSI.clear();
	currCV = &GI[g_currentGameIdx];
	if(g_hwndMainTreeCtrl==NULL)
		return 0;
	//Deselect
	for(int i=0;i<MAX_SERVERLIST;i++)
		TreeView_SetItemState(g_hwndMainTreeCtrl,GI[i].hTI,0 , TVIS_SELECTED );


	TreeView_SetItemState(g_hwndMainTreeCtrl,GI[g_currentGameIdx].hTI,TVIS_SELECTED, TVIS_SELECTED);
	TreeView_Expand(g_hwndMainTreeCtrl, GI[g_currentGameIdx].hTI, TVE_EXPAND);
	return g_currentGameIdx;
}


//Spider pig... spider pig... does what whatever, spider pig does...
//can he swing from a web, no he can't he is a pig... loooook oooout he is spider pig.. 

char g_TrimmedSSFileName[25];

DWORD WINAPI Download_ScreenshotThread(LPVOID lpParam)
{
	char gameindex = (char)lpParam;
	char szBuff[512];
	char szDestPath[MAX_PATH+100];
	char szPath[MAX_PATH];
	wchar_t wcPath[MAX_PATH];

	//SetCurrentDirectory(COMMON_SAVE_PATH);
	HANDLE hThread;
	hThread = CreateThread( NULL, 0, &ProgressGUI_Thread, g_hWnd,0, NULL);                
	if (hThread == NULL) 
	{
		AddLogInfo(ETSV_WARNING, "CreateThread failed  <AutomaticDownloadUpdateSetUp> (%d) File:(%s) Line:(%d)\n", GetLastError(),__FILE__,__LINE__ ); 
	}
	else 
	{
		dbg_print("CreateThread  success AutomaticDownloadUpdateSetUp");
		CloseHandle( hThread );
	}
	Sleep(500);

	

	ZeroMemory(wcPath,sizeof(wcPath));
	ZeroMemory(szPath,sizeof(szPath));
	
	sprintf(szPath,"%s\\%s\\",COMMON_SAVE_PATH,GI[gameindex].szMAP_MAPPREVIEW_PATH);
	mbstowcs(wcPath,(const char *)szPath,strlen(szPath));		
	SHCreateDirectory(NULL,wcPath);
	
	sprintf(szDestPath,"%s%s.png",szPath,g_TrimmedSSFileName);
	sprintf(szBuff,"http://www.cludden.se/braindamage/download/screenshots/%s/%s",GI[gameindex].szProtocolName,szDestPath);


	//HFD_SetPath(szPath);

	int iRet = HttpFileDownload(szBuff,szDestPath,g_DlgProgress,g_DlgProgressMsg);
	if(iRet!=0)
	{
		SetCurrentDirectory(szPath);
		remove(szDestPath);
		
		sprintf(szDestPath,"%s%s.jpg",szPath,g_TrimmedSSFileName);
		sprintf(szBuff,"http://www.cludden.se/braindamage/download/screenshots/%s/%s",GI[gameindex].szProtocolName,szDestPath);
		iRet = HttpFileDownload(szBuff,szDestPath,g_DlgProgress,g_DlgProgressMsg);
		if(iRet!=0)
		{
			SetCurrentDirectory(szPath);
			remove(szDestPath);
		}
	}
	//SetCurrentDirectory(USER_SAVE_PATH);
	PostMessage(g_PROGRESS_DLG,WM_CLOSE,0,0);
	return 0;
}


void Download_Screenshot(char GameIndex, char *szPreviewFileName)
{
	strcpy(g_TrimmedSSFileName,szPreviewFileName);
	HANDLE hThread;
	hThread = CreateThread( NULL, 0, &Download_ScreenshotThread, (LPVOID)GameIndex,0, NULL);                
	if (hThread == NULL) 
	{
		AddLogInfo(ETSV_WARNING, "CreateThread failed  <Download_ScreenshotThread> (%d) File:(%s) Line:(%d)\n", GetLastError(),__FILE__,__LINE__ ); 
	}
	else 
	{
		dbg_print("CreateThread  success Download_ScreenshotThread");
		CloseHandle( hThread );
	}
}

void OnPaint(HDC hDC)
{
	POINT pt;	

	if(AppCFG.bShowMapPreview)
	{
		SetCurrentDirectory(EXE_PATH);
		FIBITMAP *dib = NULL;
		if(g_CurrentSRV!=NULL)
		{
			__try{

			if(g_CurrentSRV->dwPing == 9999)
				strcpy(g_szMapName,"net.jpg");
			}
			__except(EXCEPTION_ACCESS_VIOLATION == GetExceptionCode())
			{
			// exception handling code
				AddLogInfo(ETSV_DEBUG,"Access Violation!!! (OnPaint)\n");
			}

		}
		if(strcmp(g_szMapName,"net.jpg")==0)
		{
			dib = FreeImage_Load(FIF_JPEG, g_szMapName, JPEG_DEFAULT);
		}
		else
		{
			char szPath[512];
			//Trim
			char *start = strchr(g_szMapName,'/');
			if(start==NULL)
				start = &g_szMapName[0];
			else
				start++;
			
			char *end = strchr(g_szMapName,'.');
			if(end!=NULL)
				end[0]=0;

			//Try different paths
			sprintf(szPath,"%s%s\\%s.jpg",COMMON_SAVE_PATH,GI[g_currentGameIdx].szMAP_MAPPREVIEW_PATH,start);
			dib = FreeImage_Load(FIF_JPEG, szPath, JPEG_DEFAULT);
			if(!dib)
			{
				sprintf(szPath,"%s%s\\%s.png",COMMON_SAVE_PATH,GI[g_currentGameIdx].szMAP_MAPPREVIEW_PATH,start);
				dib = FreeImage_Load(FIF_PNG, szPath, PNG_DEFAULT);

				if(!dib)
				{

					sprintf(szPath,"%s\\%s\\%s.jpg",EXE_PATH,GI[g_currentGameIdx].szMAP_MAPPREVIEW_PATH,start);
					dib = FreeImage_Load(FIF_JPEG, szPath, JPEG_DEFAULT);

					if(!dib)
					{
						sprintf(szPath,"%s\\%s\\%s.png",EXE_PATH,GI[g_currentGameIdx].szMAP_MAPPREVIEW_PATH,start);
						dib = FreeImage_Load(FIF_PNG, szPath, PNG_DEFAULT);
						
						/**********************************************
							Time to try to download the screenshot!!
						***********************************************/
					//	Download_Screenshot(g_currentGameIdx,start);
					}
				}
			}


		}
		if(!dib)
			dib = FreeImage_Load(FIF_PNG, "unknownmap.png", PNG_DEFAULT);

		SetStretchBltMode(hDC, COLORONCOLOR);
		StretchDIBits(hDC, 
			g_rcDestMapImg.left, 
			g_rcDestMapImg.top, 
			g_rcDestMapImg.right-g_rcDestMapImg.left, 
			g_rcDestMapImg.bottom-g_rcDestMapImg.top, 
		0, 0, FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),
		FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS, SRCCOPY);
		FreeImage_Unload(dib);

	//	Rectangle(hDC,  g_rcDestMapImg.left, g_rcDestMapImg.top,g_rcDestMapImg.left+5, g_rcDestMapImg.top+5);
	} 



	pt.y = g_INFOIconRect.top; //rc.bottom;
	pt.x = g_INFOIconRect.left;//rc.left+2;

//	if(g_statusIcon!=ICO_EMPTY)
	if(g_hImageListIcons!=NULL)
		ImageList_Draw(g_hImageListIcons,g_statusIcon,hDC,pt.x,pt.y,ILD_TRANSPARENT);

	//InvalidateRect(g_hWnd,&g_INFOIconRect,TRUE);
	SetCurrentDirectory(USER_SAVE_PATH);

}

void ChangeViewStates(UINT uItem)
{	
	HMENU hMenu = GetMenu(g_hWnd);					
	MENUITEMINFO mii;
	memset(&mii,0,sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	
	RECT rc;
	GetClientRect(g_hWnd, &rc);
	rc.top = TOOLBAR_Y_OFFSET;
	rc.bottom-= (STATUSBAR_Y_OFFSET + TOOLBAR_Y_OFFSET);

	if(GetMenuItemInfo(hMenu,uItem,FALSE,&mii))
	{
	//	ErrorExit("GetMenuItemInfo");
		if(MFS_CHECKED == (mii.fState  & MFS_CHECKED) )
		{
			mii.fState = MFS_UNCHECKED;

			switch(uItem)
			{
				case ID_VIEW_MAPPREVIEW: 
					AppCFG.bShowMapPreview = FALSE; 
					WNDCONT[WIN_MAPPREVIEW].bShow = FALSE;
				break;
				case ID_VIEW_BUDDYLIST:
				
					SplitterGripArea[0].tvYPos = ((rc.bottom ) * 1)  ;
					AppCFG.bShowBuddyList = FALSE;
				break;
				case ID_VIEW_PLAYERLIST: 
					AppCFG.bShowPlayerList = FALSE;
					if(!AppCFG.bShowPlayerList && !AppCFG.bShowServerRules)
					{
			
						SplitterGripArea[2].tvYPos =  rc.bottom * 1;
					}
				break;

			}

		} else
		{
			mii.fState = MFS_CHECKED;
			switch(uItem)
			{
				case ID_VIEW_MAPPREVIEW: 
					WNDCONT[WIN_MAPPREVIEW].bShow = TRUE;
					AppCFG.bShowMapPreview = true; 
				break;
				case ID_VIEW_SERVERRULES: AppCFG.bShowServerRules = true;
				break;
				case ID_VIEW_BUDDYLIST: 

					SplitterGripArea[0].tvYPos = ((rc.bottom ) *  0.65f)  ;
					AppCFG.bShowBuddyList = TRUE;
				break;
				case ID_VIEW_PLAYERLIST: 
					AppCFG.bShowPlayerList = true;
	
					SplitterGripArea[2].tvYPos =  rc.bottom * 0.5f;
				break;
			}
		}
		SetMenuItemInfo(hMenu,uItem,FALSE,&mii);
		OnSize(g_hWnd);
	}							
}


void CalcSplitterGripArea()
{
	
	//AddLogInfo(ETSV_INFO,"Called CalcSplitterGripArea.");

	DWORD wndStyle;
	int cyVScroll = 0;
	int cyXScroll = 0;
	int cyVScrollLV = 0;
	int cyXScrollLV = 0;    

	wndStyle = GetWindowLong(g_hwndMainTreeCtrl, GWL_STYLE);
	if ((wndStyle & WS_HSCROLL)> 0 )	
		cyVScroll = GetSystemMetrics(SM_CYVSCROLL); 

	if ((wndStyle & WS_VSCROLL)> 0 )
		cyXScroll = GetSystemMetrics(SM_CXHSCROLL); 

	RECT rc,r1,r2;
	SplitterGripArea[0].bMoveVertical = FALSE;
	SplitterGripArea[0].bMoveHorizontal = TRUE;
	SplitterGripArea[0].bSizing = FALSE;
	GetClientRect(g_hWnd,&rc);
	GetClientRect(g_hwndMainTreeCtrl,&r1);
	int yPos = r1.bottom + TOOLBAR_Y_OFFSET+3 + cyVScroll;
	int xPos = r1.right + cyXScroll;
	
	if(SplitterGripArea[0].tvYPos>rc.bottom-(TOOLBAR_Y_OFFSET+STATUSBAR_Y_OFFSET))
		SplitterGripArea[0].tvYPos=rc.bottom-(TOOLBAR_Y_OFFSET+(STATUSBAR_Y_OFFSET*2));

	SetRect(&SplitterGripArea[0].hit,0,yPos,xPos,yPos+(BORDER_SIZE+2));

	//Vertical Splitter gripper right to TreeView
	SplitterGripArea[1].bSizing = FALSE;
	SplitterGripArea[1].bMoveVertical = TRUE;
	SplitterGripArea[1].bMoveHorizontal = FALSE;
	SetRect(&SplitterGripArea[1].hit,r1.right+BORDER_SIZE + cyXScroll,TOOLBAR_Y_OFFSET,r1.right+BORDER_SIZE+4+cyXScroll,rc.bottom-25);


	//Horizontal spliiter gripper under ServerList 
	GetClientRect(g_hwndListViewServer,&r2);
	SplitterGripArea[2].bMoveVertical = FALSE;
	SplitterGripArea[2].bMoveHorizontal = TRUE;
	SplitterGripArea[2].bSizing = FALSE;
	
	
	wndStyle = GetWindowLong(g_hwndListViewServer, GWL_STYLE);
	cyXScrollLV = 0;
	cyVScrollLV = 0;
	if ((wndStyle & WS_HSCROLL)> 0 )
		cyVScrollLV = GetSystemMetrics(SM_CYVSCROLL); 
	if ((wndStyle & WS_VSCROLL)> 0 )
		cyXScrollLV = GetSystemMetrics(SM_CXHSCROLL); 
	
	yPos = r2.bottom + TOOLBAR_Y_OFFSET+3 + cyVScrollLV;
	xPos = r1.right + BORDER_SIZE*2;

	if(SplitterGripArea[2].tvYPos>rc.bottom-(TOOLBAR_Y_OFFSET+STATUSBAR_Y_OFFSET))
		SplitterGripArea[2].tvYPos=rc.bottom-(TOOLBAR_Y_OFFSET+(STATUSBAR_Y_OFFSET*2));

	SetRect(&SplitterGripArea[2].hit,xPos+cyXScroll,yPos,xPos+r2.right+BORDER_SIZE+cyXScrollLV,yPos+(BORDER_SIZE+2));
}

void Initialize_WindowSizes()
{

	RECT rc;
	GetClientRect(g_hWnd, &rc);

	rc.top = TOOLBAR_Y_OFFSET;
	rc.bottom-= (STATUSBAR_Y_OFFSET + TOOLBAR_Y_OFFSET); //reduce the size of statusbar from the main window size

	float fBuddyScale=0;
	if(WNDCONT[WIN_BUDDYLIST].bShow)
		fBuddyScale = 0.4f;

	SetRect(&WNDCONT[WIN_MAINTREEVIEW].rSize,0,TOOLBAR_Y_OFFSET,rc.right*0.2,rc.bottom*(1.0f-fBuddyScale));

	int offSetX = WNDCONT[WIN_MAINTREEVIEW].rSize.right + BORDER_SIZE; //get offset for next window to start at 
	int offSetY = TOOLBAR_Y_OFFSET+WNDCONT[WIN_MAINTREEVIEW].rSize.bottom + BORDER_SIZE; //get offset for next window to start at 

	SplitterGripArea[0].tvYPos = offSetY ;
//	SplitterGripArea[0].tvXPos = offSetX + TOOLBAR_Y_OFFSET ;
	SplitterGripArea[1].tvXPos = offSetX;

	SetRect(&WNDCONT[WIN_SERVERLIST].rSize,offSetX,TOOLBAR_Y_OFFSET,rc.right-WNDCONT[WIN_MAINTREEVIEW].rSize.right,rc.bottom*0.6);


	SetRect(&WNDCONT[WIN_BUDDYLIST].rSize,0,offSetY,offSetX,rc.bottom*fBuddyScale);
	
	int iShow = rc.right * 1;
	if(WNDCONT[WIN_MAPPREVIEW].bShow)
		iShow =  rc.right * 0.4;

	SetRect(&WNDCONT[WIN_TABCONTROL].rSize,offSetX,offSetY,iShow,TABSIZE_Y);
	
	SetRect(&WNDCONT[WIN_PLAYERS].rSize,offSetX,offSetY+TABSIZE_Y,iShow,(rc.bottom*0.4)-TABSIZE_Y);
	SetRect(&WNDCONT[WIN_RULES].rSize,offSetX,offSetY+TABSIZE_Y,iShow,(rc.bottom*0.4)-TABSIZE_Y);
	SetRect(&WNDCONT[WIN_RCON].rSize,offSetX,offSetY+TABSIZE_Y,iShow,(rc.bottom*0.4)-TABSIZE_Y);
	SetRect(&WNDCONT[WIN_PING].rSize,offSetX,offSetY+TABSIZE_Y,iShow,(rc.bottom*0.4)-TABSIZE_Y);
	SetRect(&WNDCONT[WIN_LOGGER].rSize,offSetX,offSetY+TABSIZE_Y,iShow,(rc.bottom*0.4)-TABSIZE_Y);
	
	offSetX = WNDCONT[WIN_MAINTREEVIEW].rSize.right + WNDCONT[WIN_TABCONTROL].rSize.right; //get offset for next window to start at 


	SetRect(&WNDCONT[WIN_MAPPREVIEW].rSize,offSetX,offSetY,rc.right*0.2,rc.bottom*0.4);
	SplitterGripArea[2].tvYPos = offSetY ;

	SetRect(&WNDCONT[WIN_STATUS].rSize,25,offSetY+WNDCONT[WIN_BUDDYLIST].rSize.bottom+BORDER_SIZE+2,(rc.right*0.6)-25,STATUSBAR_Y_OFFSET);
	SetRect(&WNDCONT[WIN_PROGRESSBAR].rSize,WNDCONT[WIN_STATUS].rSize.right+25,offSetY+WNDCONT[WIN_BUDDYLIST].rSize.bottom+BORDER_SIZE,rc.right*0.4,STATUSBAR_Y_OFFSET);
}




void Update_WindowSizes()
{

	RECT rc;
	GetClientRect(g_hWnd, &rc);

	rc.top = TOOLBAR_Y_OFFSET;
	rc.bottom-= (STATUSBAR_Y_OFFSET + TOOLBAR_Y_OFFSET); //reduce the size of statusbar from the main window size

	if(WNDCONT[WIN_BUDDYLIST].bShow==FALSE)	
		SplitterGripArea[0].tvYPos = ((rc.bottom ) * 1);
	

	SetRect(&WNDCONT[WIN_MAINTREEVIEW].rSize,0,TOOLBAR_Y_OFFSET,SplitterGripArea[1].tvXPos,SplitterGripArea[0].tvYPos);

	int offSetX = WNDCONT[WIN_MAINTREEVIEW].rSize.right + BORDER_SIZE; //get offset for next window to start at 
	int offSetY = TOOLBAR_Y_OFFSET+WNDCONT[WIN_MAINTREEVIEW].rSize.bottom + BORDER_SIZE; //get offset for next window to start at 

	SetRect(&WNDCONT[WIN_SERVERLIST].rSize,offSetX,TOOLBAR_Y_OFFSET,rc.right-WNDCONT[WIN_MAINTREEVIEW].rSize.right,SplitterGripArea[2].tvYPos);


	SetRect(&WNDCONT[WIN_BUDDYLIST].rSize,0,offSetY,offSetX-BORDER_SIZE,rc.bottom-WNDCONT[WIN_MAINTREEVIEW].rSize.bottom);


	float iShow = 1;
	if(WNDCONT[WIN_MAPPREVIEW].bShow)
		iShow = 0.7f;

	int offSetTabX = (WNDCONT[WIN_SERVERLIST].rSize.right * iShow) ;//- WNDCONT[WIN_MAPPREVIEW].rSize.right; //get offset for next window to start at 
	int offSetMapX = (WNDCONT[WIN_SERVERLIST].rSize.right * 0.3) ;//- WNDCONT[WIN_MAPPREVIEW].rSize.right; //get offset for next window to start at 
	int offSetY2 = WNDCONT[WIN_SERVERLIST].rSize.bottom; //get offset for next window to start at 

	offSetY = TOOLBAR_Y_OFFSET+WNDCONT[WIN_SERVERLIST].rSize.bottom + BORDER_SIZE; //get offset for next window to start at 



	SetRect(&WNDCONT[WIN_TABCONTROL].rSize,offSetX,offSetY,offSetTabX,TABSIZE_Y);
	
	SetRect(&WNDCONT[WIN_PLAYERS].rSize,offSetX,offSetY+TABSIZE_Y,offSetTabX,(rc.bottom-offSetY2)-TABSIZE_Y);
	SetRect(&WNDCONT[WIN_RULES].rSize,offSetX,offSetY+TABSIZE_Y,offSetTabX,(rc.bottom-offSetY2)-TABSIZE_Y);
	SetRect(&WNDCONT[WIN_RCON].rSize,offSetX,offSetY+TABSIZE_Y,offSetTabX,(rc.bottom-offSetY2)-TABSIZE_Y);
	SetRect(&WNDCONT[WIN_LOGGER].rSize,offSetX,offSetY+TABSIZE_Y,offSetTabX,(rc.bottom-offSetY2)-TABSIZE_Y);
	SetRect(&WNDCONT[WIN_PING].rSize,offSetX,offSetY+TABSIZE_Y,offSetTabX,(rc.bottom-offSetY2)-TABSIZE_Y);

	int MapPreviewoffSetX = WNDCONT[WIN_MAINTREEVIEW].rSize.right + WNDCONT[WIN_TABCONTROL].rSize.right; //get offset for next window to start at 


	SetRect(&WNDCONT[WIN_MAPPREVIEW].rSize,MapPreviewoffSetX,offSetY,offSetMapX,(rc.bottom-offSetY2)-TABSIZE_Y);
	
	SetRect(&WNDCONT[WIN_STATUS].rSize,25,WNDCONT[WIN_BUDDYLIST].rSize.top+WNDCONT[WIN_BUDDYLIST].rSize.bottom+2,(rc.right*0.6)-25,STATUSBAR_Y_OFFSET);
	SetRect(&WNDCONT[WIN_PROGRESSBAR].rSize,WNDCONT[WIN_STATUS].rSize.right+25,WNDCONT[WIN_STATUS].rSize.top,rc.right*0.4,STATUSBAR_Y_OFFSET);

	
	
	g_rcDestMapImg.left = rc.right-(ImageSizeX+BORDER_SIZE); 
	g_rcDestMapImg.top = (rc.bottom - ImageSizeX) +  (TOOLBAR_Y_OFFSET + BORDER_SIZE);
	g_rcDestMapImg.right = g_rcDestMapImg.left + ImageSizeX;
	g_rcDestMapImg.bottom = g_rcDestMapImg.top + ImageSizeX;
	SetRect(&WNDCONT[WIN_MAPPREVIEW].rSize,g_rcDestMapImg.left,g_rcDestMapImg.top,g_rcDestMapImg.right,g_rcDestMapImg.bottom);


}



//void ResizeListView(HWND g_hwndListViewPlayers,HWND g_hwndListViewVars, HWND hwndParent, BOOL bRepaint)
void OnSize(HWND hwndParent, BOOL bRepaint)
{
	RECT  rc;
	
	dbg_print("OnSize");
	GetClientRect(hwndParent, &rc);
	rc.top = TOOLBAR_Y_OFFSET;
	rc.bottom-= (STATUSBAR_Y_OFFSET + TOOLBAR_Y_OFFSET);

	GetClientRect(hwndParent, &g_INFOIconRect);
	g_INFOIconRect.left+=2;
	g_INFOIconRect.top = g_INFOIconRect.bottom-STATUSBAR_Y_OFFSET+BORDER_SIZE+1;
	g_INFOIconRect.bottom = STATUSBAR_Y_OFFSET;
	g_INFOIconRect.right = g_INFOIconRect.left +20;

	MoveWindow( g_hwndRibbonBar, 
				rc.left,
				rc.top ,
				rc.right,
				40,	
				bRepaint);


	Update_WindowSizes();

	for(int i=0;i<13;i++)
	{
		MoveWindow(WNDCONT[i].hWnd,WNDCONT[i].rSize.left,WNDCONT[i].rSize.top,WNDCONT[i].rSize.right,WNDCONT[i].rSize.bottom,bRepaint);
		ShowWindow(WNDCONT[i].hWnd,WNDCONT[i].bShow);
	}
	ShowWindow(g_hwndRibbonBar,TRUE);
	CalcSplitterGripArea();
	InvalidateRect(WNDCONT[WIN_PING].hWnd,&WNDCONT[WIN_PING].rSize,TRUE);
	ListView_SetColumnWidth(g_hwndListBuddy,2,LVSCW_AUTOSIZE_USEHEADER);

}

void UpdateCurrentServerUI()
{
	if(g_CurrentSRV==NULL)
		return;

	//SetDlgItemText(g_hwndRibbonBar,IDC_EDIT_QUICK_PASSWORD,g_CurrentSRV->szPRIVATEPASS);
	
	InvalidateRect(g_hWnd,&g_rcDestMapImg, TRUE);

}


void tryToMinimizeGame()
{
	HWND hwndGame=NULL;

	hwndGame=NULL;
	char seps[]={";|"};
	char *token;
	char szTmp[MAX_PATH];
	strcpy(szTmp,AppCFG.szET_WindowName);
	token = strtok(szTmp, seps ); // C4996	
	while( token != NULL )
	{
	
	  hwndGame = FindWindow(NULL,token); 	
	  if(hwndGame!=NULL)
		  break;
	  // Get next token: 
	  token = strtok( NULL, seps ); // C4996
	}
	
	if (hwndGame)
	{
		WINDOWPLACEMENT wp;
		GetWindowPlacement(hwndGame, &wp);
		if (wp.showCmd == SW_SHOWMINIMIZED)
		{
			if(AppCFG.bUSE_SCREEN_RESTORE )
				SetScreenResolution(etMode);
			
			EnableWindow(hwndGame, TRUE);
			SetWindowLong(hwndGame, GWL_STYLE, WS_MAXIMIZE);
			ShowWindow(hwndGame, SW_MAXIMIZE);
			SetActiveWindow(hwndGame);
			Sleep(100);
			ClickMouse();
		}
		else
		{
			if (etMode.dmPelsWidth==0)
				etMode = GetScreenResolution();

			ShowWindow(hwndGame, SW_MINIMIZE);
			EnableWindow(hwndGame, FALSE);
			
			if(AppCFG.bUSE_SCREEN_RESTORE )
				SetScreenResolution(userMode);

			SetWindowLong(hwndGame, GWL_STYLE, WS_MINIMIZE);
			Sleep(100);
			ClickMouse();
		}
	} else
	{
		SetScreenResolution(userMode);
	}

}

int Get_CountryFlagByShortName(const char *szCC)
{	
	int iSize = sizeof(CountryCodes)/3;
	for(int i=0; i<iSize;i++)
	{
		if(_stricmp(CountryCodes[i].szCountryCode,szCC)==0)
			return i;
	}
	return iSize-1;
}

void Load_CountryFlags()
{
	
	HDC hDC =CreateDC("DISPLAY", NULL, NULL, NULL); 
	if(hDC==NULL)
		return;
	
	int iSize = sizeof(CountryCodes)/3;

	g_hILFlags = ImageList_Create(18, 12, ILC_COLOR24,iSize, 1);

	char szFilename[MAX_PATH+20];

	for(int i=0; i<iSize;i++)
	{
		sprintf(szFilename,"%s\\flags\\flag_%s.gif",EXE_PATH,CountryCodes[i].szCountryCode);

		FIBITMAP *dib = FreeImage_Load(FIF_GIF,szFilename, GIF_DEFAULT);
		if(dib!=NULL)
		{
			HBITMAP hBitmap = CreateDIBitmap(hDC, FreeImage_GetInfoHeader(dib),	CBM_INIT, FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS);
			ImageList_Add(g_hILFlags,hBitmap,hBitmap);
		
			DeleteObject(hBitmap);
			FreeImage_Unload(dib);
		}
	}
	DeleteDC(hDC);
}


void LoadImageList()
{

	if(g_hImageListIcons!=NULL)
		return;
	dbg_print("Image list\n");

	g_hImageListIcons = ImageList_Create(16, 16, ILC_COLOR32|ILC_MASK,35, 1);

	HICON hIcon;

	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_ET));		    //0 ET
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_PB));			//1
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_FAVORITES));   //2
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_BUDDY));		//3
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_LOCKED));		//4
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_ERROR));		//5
	ImageList_AddIcon(g_hImageListIcons, hIcon); 
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_WARNING));		//6
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_UNKOWN));		//7
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(ICON_INFO));			//8
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_CHECKBOX));	//9 61 Checkbox
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_CHECKEDBOX));	//10 62 CHECKEDBOX
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_TRICHECKEDBOX)); //11 65 Tri Checkedbox
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_GLOBE));		//12 57 Globe
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_LAN));			//13 58 Lan
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_APP_LOGO));	//14 59 App logo
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_FILTER));		//15 60 Filter
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_HISTORY));		//16 64 History
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_FILTER2));	   // 17 82
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_FOLDER));	  //18 75
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_RANKED)); //19 76  Ranked
	ImageList_AddIcon(g_hImageListIcons, hIcon);		
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_BLANK)); //20 77
	ImageList_AddIcon(g_hImageListIcons, hIcon);		
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_QUAKE)); //21 52
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_ETQW)); //22 56 ET QW ICON
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_RTCW)); //23 63 RTCW
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_Q3)); //24 74
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_WARSOW)); //25 78
	ImageList_AddIcon(g_hImageListIcons, hIcon);	
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_COD)); //26 80
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_COD2)); //27 79
	ImageList_AddIcon(g_hImageListIcons, hIcon);	
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_COD4)); //28 81
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_CS)); //29
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_VAC)); //30
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_CSS)); //31
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_USERS)); //32
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_RCON)); //33
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_LOGGER)); //34
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_FONT)); //35
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_PAINT)); //36
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_STATS)); //37
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_RULES)); //38
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_QUAKE1)); //39
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_QUAKE2)); //40
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);

}


void SetImageList()
{
	SendMessage(g_hwndListViewServer, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)g_hImageListIcons);
	SendMessage(g_hwndListBuddy, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)g_hImageListIcons);	
	SendMessage(g_hwndMainTreeCtrl, TVM_SETIMAGELIST , TVSIL_NORMAL, (LPARAM)g_hImageListIcons);
	TabCtrl_SetImageList(g_hwndTabControl,g_hImageListIcons);

}

//simple check to see if the update server is online
bool IsServerAlive(char *host)
{
  SOCKET pSocket;
  SendMessage(g_DlgProgress, PBM_STEPIT, (WPARAM) 0, 0); 

  pSocket =  getsockudp(host ,80); 
  SendMessage(g_DlgProgress, PBM_STEPIT, (WPARAM) 0, 0); 

  if(INVALID_SOCKET==pSocket)
  {
	  return false;
  }
	SendMessage(g_DlgProgress, PBM_STEPIT, (WPARAM) 0, 0); 

	return true;
}





DWORD WINAPI GetServerList(LPVOID lpParam )
{
	DWORD options = (DWORD)lpParam;

	int currGameIdx = g_currentGameIdx;
	
	//EnableButtons(FALSE);
	Show_StopScanningButton(TRUE);
	ListView_DeleteAllItems(g_hwndListViewVars);
	ListView_DeleteAllItems(g_hwndListViewPlayers);
	ListView_DeleteAllItems(g_hwndListViewServer);

	AddLogInfo(ETSV_DEBUG,  "executing GetServerList");
	
	//We don't want to overdraw wrong serverlist
	if(currGameIdx==g_currentGameIdx)
		RedrawServerListThread(&GI[currGameIdx]); //Modifed version since 5.25

	if(g_bRunningQueryServerList)	
		return 0;


	g_bRunningQueryServerList = true;

	if (! ResetEvent(hCloseEvent) ) 
        dbg_print("ResetEvent failed\n");

	
	int iGame=0;
nextGame:
	if(options==SCAN_ALL_GAMES)
		currGameIdx = iGame++;
	if(iGame==MAX_SERVERLIST)  //reset
		currGameIdx = iGame = 0;

	SetStatusText(ICO_INFO,"Receiving %s servers...",GI[currGameIdx].szGAME_NAME);

	switch(GI[currGameIdx].cGAMEINDEX)
	{
		case ETQW_SERVERLIST:
			{
				g_bQ4 = FALSE;	
				HFD_SetPath(USER_SAVE_PATH);
				int ret = HttpFileDownload(GI[currGameIdx].szMasterServerIP,"etqwservers.txt",NULL,NULL);
				if(ret!=0)
					goto exitError;
				
				//SetStatusText(ICO_INFO,"Downloading %s servers... Done!",GI[currGameIdx].szGAME_NAME);
				//ETQW_ParseServerList();				
				Parse_FileServerList(&GI[currGameIdx],"etqwservers.txt");

				SCAN_Set_CALLBACKS(&Q4_Get_ServerStatus,&UpdateServerItem);				
				if((DWORD)lpParam==SCAN_FILTERED)
					Initialize_Rescan2(&GI[ETQW_SERVERLIST],&FilterServerItemV2);
				else
					Initialize_Rescan2(&GI[ETQW_SERVERLIST],NULL);
			}
		break;
		case Q4_SERVERLIST:
			{
				g_bQ4 = TRUE;
				SCAN_Set_CALLBACKS(&Q4_Get_ServerStatus,&UpdateServerItem);				
				if((DWORD)lpParam==SCAN_FILTERED)
					Initialize_Rescan2(&GI[Q4_SERVERLIST],&FilterServerItemV2);
				else
				{
					Q4_ConnectToMasterServer(&GI[currGameIdx]);			
					Initialize_Rescan2(&GI[Q4_SERVERLIST],NULL);
				}
			}
			break;
		case CSCZ_SERVERLIST:
		case CS_SERVERLIST:
		case CSS_SERVERLIST:
			{
				
				SCAN_Set_CALLBACKS(&STEAM_Get_ServerStatus,&UpdateServerItem);
				
				if((DWORD)lpParam==SCAN_FILTERED)
					Initialize_Rescan2(&GI[currGameIdx],&FilterServerItemV2);
				else
				{
					
					STEAM_ConnectToMasterServer(&GI[currGameIdx]);
					Initialize_Rescan2(&GI[currGameIdx],NULL);
					
				}
	
			}
			break;
		case Q2_SERVERLIST:
			{
				
				if(GI[currGameIdx].bUseHTTPServerList)
				{
					HFD_SetPath(USER_SAVE_PATH);
					int ret = HttpFileDownload(GI[currGameIdx].szMasterServerIP,"q2servers.txt",NULL,NULL);
					if(ret!=0)
						goto exitError;
					Parse_FileServerList(&GI[currGameIdx],"q2servers.txt");
				}
				SCAN_Set_CALLBACKS(&Q3_Get_ServerStatus,&UpdateServerItem);
				if((DWORD)lpParam==SCAN_FILTERED)
					Initialize_Rescan2(&GI[currGameIdx],&FilterServerItemV2);
				else
				{
					if(GI[currGameIdx].bUseHTTPServerList == FALSE)				
						Q3_ConnectToMasterServer(&GI[currGameIdx]);
					Initialize_Rescan2(&GI[currGameIdx],NULL);
				}

			}
			break;
		case QW_SERVERLIST:
			{
				HFD_SetPath(USER_SAVE_PATH);
				int ret = HttpFileDownload(GI[currGameIdx].szMasterServerIP,"qwservers.txt",NULL,NULL);
				if(ret!=0)
					goto exitError;
				Parse_FileServerList(&GI[currGameIdx],"qwservers.txt");
				SCAN_Set_CALLBACKS(&Q3_Get_ServerStatus,&UpdateServerItem);
				if((DWORD)lpParam==SCAN_FILTERED)
					Initialize_Rescan2(&GI[currGameIdx],&FilterServerItemV2);
				else
					Initialize_Rescan2(&GI[currGameIdx],NULL);
			}
			break;

		default:
			{
				SCAN_Set_CALLBACKS(&Q3_Get_ServerStatus,&UpdateServerItem);
				if((DWORD)lpParam==SCAN_FILTERED)
					Initialize_Rescan2(&GI[currGameIdx],&FilterServerItemV2);
				else
				{
					Q3_ConnectToMasterServer(&GI[currGameIdx]);
					Initialize_Rescan2(&GI[currGameIdx],NULL);
				}

			}

			break;
	}
	char szBuffer[100];
	sprintf(szBuffer,"%s (%d)",GI[currGameIdx].szGAME_NAME,GI[currGameIdx].dwTotalServers);
	TreeView_SetItemText(GI[currGameIdx].hTI,szBuffer);	
	if((options==SCAN_ALL_GAMES) && (g_bCancel==false))
		goto nextGame;

	Show_StopScanningButton(FALSE);

	SendMessage(g_hwndProgressBar, PBM_SETPOS, (WPARAM) 0, 0); 

	if(g_bCancel)
	{
		g_bCancel = false;

		g_bRunningQueryServerList = false;
		if (! SetEvent(hCloseEvent) ) 
		{
			dbg_print("SetEvent failed!\n");
	      
		}
		AddLogInfo(ETSV_DEBUG,  "cancel GetServerList <<<<<");
		return 0xFFFF;
	}
	if(GI[currGameIdx].pSC->vSI.size()==0)
		goto exitError;

	SetStatusText(ICO_INFO,"Receiving %s servers... DONE!",GI[currGameIdx].szGAME_NAME);
	
	//We don't want to overdraw wrong serverlist
	if(currGameIdx==g_currentGameIdx)
	{
		//Modified since ver 5.25		
		//Initilize_RedrawServerListThread();
		RedrawServerListThread(&GI[currGameIdx]);
		
	}

	g_bCancel = false;



	//ListView_SortItems(g_hwndListViewServer,MyCompareFunc,COL_NUMPLAYERS);
	//ListView_SetColumnWidth(g_hwndListViewServer,0,LVSCW_AUTOSIZE);
	//ListView_SetColumnWidth(g_hwndListViewServer,1,LVSCW_AUTOSIZE);
	//ListView_SetColumnWidth(g_hwndListViewServer,2,LVSCW_AUTOSIZE);
//	ListView_SetColumnWidth(g_hwndListViewServer,3,LVSCW_AUTOSIZE);
//	ListView_SetColumnWidth(g_hwndListViewServer,4,LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(g_hwndListViewServer,6,LVSCW_AUTOSIZE);
	Buddy_UpdateList(g_pBIStart);
	goto NoError;

exitError:
	SetStatusText(ICO_INFO,"Error receiving %s servers... UNSUCCESSFULL!",GI[currGameIdx].szGAME_NAME);
NoError:
   g_bRunningQueryServerList = false;
   if (! SetEvent(hCloseEvent) ) 
    {
        dbg_print("SetEvent failed!\n");
      
    }
   EnableButtons(TRUE);
   SendMessage(g_hwndProgressBar, PBM_SETPOS, (WPARAM) 0, 0); 
   AddLogInfo(ETSV_DEBUG,  "GetServerList DONE!");
  return 0x00FF00FF;
 }



DWORD WINAPI GetServerListThread(LPVOID lpParam )
{
	DWORD options = (DWORD)lpParam;
	BOOL bError=FALSE;
	int currGameIdx = g_currentGameIdx;
	int iGame=0;

	AddLogInfo(ETSV_DEBUG,  "Entering GetServerListThread(..)");

	if (! ResetEvent(hCloseEvent) ) 
       dbg_print("ResetEvent failed\n");

	

nextGame:
	
	currGameIdx = iGame;
	if(GI[currGameIdx].bActive==FALSE)
	{		
			iGame++;
			Sleep(100);
			goto nextGame;
	}
	iGame++;

	if(iGame>MAX_SERVERLIST)  //reset
		currGameIdx = iGame = 0;

	SetStatusText(ICO_INFO,"Receiving %s servers...",GI[currGameIdx].szGAME_NAME);

	switch(GI[currGameIdx].cGAMEINDEX)
	{
		case ETQW_SERVERLIST:
			{
				g_bQ4 = FALSE;	
				HFD_SetPath(USER_SAVE_PATH);
				int ret = HttpFileDownload(GI[currGameIdx].szMasterServerIP,"etqwservers.txt",NULL,NULL);
				if(ret!=0)
				{
					bError = TRUE;
					goto exitLoop;
				}
				
				Parse_FileServerList(&GI[currGameIdx],"etqwservers.txt");			
			}
			break;
		case Q4_SERVERLIST:
			{
				g_bQ4 = TRUE;
				Q4_ConnectToMasterServer(&GI[currGameIdx]);			
			}
			break;
		case CSCZ_SERVERLIST:
		case CS_SERVERLIST:
		case CSS_SERVERLIST:
			{				
				STEAM_ConnectToMasterServer(&GI[currGameIdx]);
			}
			break;
		default:
			{
				Q3_ConnectToMasterServer(&GI[currGameIdx]);

			}
			break;
	}
	
	char szBuffer[100];
	sprintf(szBuffer,"%s (%d)",GI[currGameIdx].szGAME_NAME,GI[currGameIdx].dwTotalServers);
	TreeView_SetItemText(GI[currGameIdx].hTI,szBuffer);


	if((options==SCAN_ALL_GAMES) && (g_bCancel==false))
		goto nextGame;

	if(g_bCancel)
	{
		g_bCancel = false;

		if (! SetEvent(hCloseEvent) ) 
			dbg_print("SetEvent failed!\n");

		AddLogInfo(ETSV_DEBUG,  "Cancel GetServerListThread!");
		return 0xFFFF;
	}

	if(GI[currGameIdx].pSC->vSI.size()==0) //Well no new servers
		bError = TRUE;

	SetStatusText(ICO_INFO,"Receiving %s servers... DONE!",GI[currGameIdx].szGAME_NAME);
	
	g_bCancel = false;



exitLoop:
	if(bError)
		SetStatusText(ICO_INFO,"Error receiving %s servers... UNSUCCESSFULL!",GI[currGameIdx].szGAME_NAME);

   if (! SetEvent(hCloseEvent) ) 
        dbg_print("SetEvent failed!\n");      

   AddLogInfo(ETSV_DEBUG,  "GetServerList DONE!");
  return 0x00FF00FF;
 }

bool FilterServerItemV2(LPARAM *lp,GAME_INFO *pGI)
{
	DWORD dwFilterFlags = pGI->dwViewFlags;

	DWORD bForceFavorites = (dwFilterFlags & REDRAWLIST_FAVORITES_PUBLIC);
	DWORD bForceHistory = (dwFilterFlags & REDRAWLIST_HISTORY) ;
	DWORD bForceFavoritesPrivate = dwFilterFlags & REDRAWLIST_FAVORITES_PRIVATE;
	DWORD bRescanFilter = dwFilterFlags & REDRAWLIST_SCAN_FILTERED;

	SERVER_INFO *srv=NULL;
	bool returnVal=false;

	__try
	{
		srv = (SERVER_INFO*)lp;

		//If scanning filtered (limited number of servers) ignore following filter options:
		if(bRescanFilter==0)
		{

			
			if(bForceFavorites)
				if(srv->cFavorite==0 || srv->bPrivate==1)
					return false;
				else if(srv->cFavorite==1 || srv->bPrivate==0)
						return true;

			//Show only Favorites that is Private!?
			if(bForceFavoritesPrivate)
			{
				if((srv->bPrivate) && (srv->cFavorite))
					return true;//returnVal=true;
				else
					return false;	
			}

			if(AppCFG.filter.bHideOfflineServers && (srv->dwPing==9999))
				return false;
		
			if(AppCFG.filter.dwPing>0)
			{
				
				if(srv->dwPing>AppCFG.filter.dwPing)
					return false;
				//if(pGI->filter.bUseFilterOnPing && (srv->dwPing>CountryFilter.dwMinPing))
				//	return false;
			}

			if(pGI->filter.bNoEmpty && (srv->nCurrentPlayers==0))
				return false;

			if(pGI->filter.bNoFull && (srv->nCurrentPlayers>=srv->nMaxPlayers))
				return false;

			if(AppCFG.filter.cActiveMinPlayer && (srv->nCurrentPlayers<(int)AppCFG.filter.dwShowServerWithMinPlayers))
				return false;

			if(AppCFG.filter.cActiveMaxPlayer && (srv->nCurrentPlayers>(int)AppCFG.filter.dwShowServerWithMaxPlayers))
				return false;
		}

		if(pGI->filter.bNoBots)
			if(srv->cBots>0)
				return false;

		if(pGI->filter.bRanked && (srv->cGAMEINDEX == ETQW_SERVERLIST))
			if(srv->cRanked==0)
				return false;

		if(pGI->filter.bPure)
			if(srv->cPure==0)
				return false;


		if((srv->cPurge>=ETSV_PURGE_COUNTER) && (srv->cFavorite==0))
			return false;

		if(bForceHistory)
			if(srv->cHistory==0)
				return false;



		if(pGI->filter.bPunkbuster)
			if(srv->bPunkbuster==0)
				return false;
		if(pGI->filter.bNoPrivate)
			if(srv->bPrivate)
				return false;
		
		if(pGI->filter.bOnlyPrivate)
			if(srv->bPrivate)
				returnVal=true;
			else
				return false;


	if(pGI->filter.dwGameTypeFilter>0)
	{
		returnVal=false;
		DWORD val = 1;
		for(int i=0;i<pGI->pSC->vFilterGameType.size();i++)
		{
			if(pGI->filter.dwGameTypeFilter & val)
			{
				DWORD result = (srv->cGameTypeCVAR & val);
		
				if(result) 
				{
					returnVal=true;
					break;
				}
			}
			
			val=val*2;
		}

		if(returnVal==false)
			return false;
	}

	if(pGI->filter.dwMod>0)
	{
		returnVal=false;
		DWORD val = 1;
		for(int i=0;i<pGI->pSC->vFilterMod.size();i++)
		{
			if(pGI->filter.dwMod & val)
			{
				DWORD result = (srv->wMod & val);
		
				if(result) 
				{
					returnVal=true;
					break;
				}
			}
			
			val=val*2;
		}

		if(returnVal==false)
			return false;
	}
	if(pGI->filter.dwMap>0)
	{
		returnVal=false;
		DWORD val = 1;
		for(int i=0;i<pGI->pSC->vFilterMap.size();i++)
		{
			if(pGI->filter.dwMap & val)
			{
				DWORD result = (srv->dwMap & val);
		
				if(result) 
				{
					returnVal=true;
					break;
				}
			}
			
			val=val*2;
		}

		if(returnVal==false)
			return false;
	}
	if(pGI->filter.dwVersion>0)
	{
		returnVal=false;
		DWORD val = 1;
		for(int i=0;i<pGI->pSC->vFilterVersion.size();i++)
		{
			if(pGI->filter.dwVersion & val)
			{
				DWORD result = (srv->dwVersion & val);
		
				if(result) 
				{
					returnVal=true;
					break;
				}
			}
			
			val=val*2;
		}

		if(returnVal==false)
			return false;
	}
		returnVal=false;

	//	if(AppCFG.bUseCountryFilter)
		{
			for(int i=0; i<CountryFilter.counter;i++)
			{
				if(_stricmp(srv->szShortCountryName,CountryFilter.szShortCountryName[i])==0)//if(srv->cCountryFlag == CountryFilter.countryIndex[i])
				{
					returnVal=true;
					break;
				} else if((srv->cCountryFlag==0) && (CountryFilter.countryIndex[i]==98))
				{
					returnVal=true;
					break;
				}
			}
		}

		if(CountryFilter.counter==0)
			returnVal=true;


	}
	__except(EXCEPTION_ACCESS_VIOLATION == GetExceptionCode())
	{
		// exception handling code
		dbg_print("Access Violation!!! @ FilterServerItem(...)\n");	
		return false;
	}
	srv->bHide = !returnVal;
	if(returnVal==false)
		return false;
	return true;
}


DWORD WINAPI  RedrawServerListThread(LPVOID pvoid )
{
	if(bRunningRefreshThread==TRUE)
		return 0xFF;
	
	bRunningRefreshThread = TRUE;
	
	GAME_INFO *pGI = (GAME_INFO *)pvoid;
	vSRV_INF::iterator  iLst;

//	SendMessage(g_hwndMainSTATS,WM_STOP_PING,0,0);

	ListView_DeleteAllItems(g_hwndListViewServer);
	pGI->pSC->vRefListSI.clear();
	if(pGI->pSC->vSI.size()>0)
	{
		for ( iLst = pGI->pSC->vSI.begin( ); iLst != pGI->pSC->vSI.end( ); iLst++ )
		{
			SERVER_INFO pSI = *iLst;
		
			REF_SERVER_INFO refSI;
			refSI.dwIndex = pSI.dwIndex;
			refSI.cGAMEINDEX = pSI.cGAMEINDEX;

			if(FilterServerItemV2((LPARAM*)&pSI,pGI))
			{
				pGI->pSC->vRefListSI.push_back(refSI);
			}
		}
		dbg_print("Created filtered serverlist!\n");

		Do_ServerListSort(iLastColumnSortIndex);
	}
	
	ListView_SetItemCount(g_hwndListViewServer,pGI->pSC->vRefListSI.size());
	
	bRunningRefreshThread=FALSE;
	return 0;
}

//Try to avoid use this func
void Initialize_RedrawServerListThread()
{
		HANDLE hThread;
		DWORD dwThreadIdBrowser;
		AddLogInfo(ETSV_DEBUG,  "executing InitilizeRedrawServerListThread");	

		if(g_currentGameIdx==-1) //since v5.41
			return;

	//	if(bRunningRefreshThread==FALSE)
		{
	//		bRunningRefreshThread = TRUE;
			dbg_print("Creating RedrawServerListThread thread!\n");
			hThread = NULL;		
	
			hThread = CreateThread( NULL, 0, &RedrawServerListThread, (LPVOID)&GI[g_currentGameIdx],0, &dwThreadIdBrowser);                
			if (hThread == NULL) 
			{
				AddLogInfo(ETSV_WARNING,"CreateThread failed (%d)\n", GetLastError() ); 
			}
			else 
			{
				CloseHandle( hThread );
			}
		}
		AddLogInfo(ETSV_DEBUG,  "executing InitilizeRedrawServerListThread done");
}


void Parse_FileServerList(GAME_INFO *pGI,char *szFilename)
{

	SetCurrentDirectory(USER_SAVE_PATH);
	FILE *fp=fopen(szFilename, "rb");
	int i=0;
	char buff[81];
	DWORD dwPort=0;
	char *pszIP;
	
	if(fp!=NULL)
	{
		while(!feof(fp))
		{				
			fscanf_s( fp, "%s", buff, 81 );
			pszIP = SplitIPandPORT((char*)&buff,dwPort);					
			AddServer(pGI,pszIP,dwPort,false);
			i++;
		}
		fclose(fp);
	} 	
	pGI->dwTotalServers = pGI->pSC->vSI.size();
}


char Get_GameIcon(char index)
{
	switch(index)
	{
		case ETQW_SERVERLIST: 
			return 22;  //ETQW icon
			break;
		case Q4_SERVERLIST:
			return 21;  //Quake 4 icon
			break;
		case ET_SERVERLIST: 
			return 0;  //ETQW icon
			break;
		case Q3_SERVERLIST:
			return 24;  //Quake icon
			break;
		case RTCW_SERVERLIST: 
			return 23; 
			break;
		case COD_SERVERLIST: 
			return 26;  
			break;
		case COD2_SERVERLIST: 
			return 27; 
		case COD4_SERVERLIST: 
			return 28; 
			break;
		case WARSOW_SERVERLIST: 
			return 25; 
			break;
		case CS_SERVERLIST:
		case CSCZ_SERVERLIST:
			return 29;
		case CSS_SERVERLIST:
			return 31;
		case QW_SERVERLIST:
			return 39;
		case Q2_SERVERLIST:
			return 40;

		default:
			return 7;  //unkown icon
	}

}
long InsertServerItem(GAME_INFO *pGI,SERVER_INFO pSI)
{
	LVITEM lvItem;
	ZeroMemory(&lvItem, sizeof(LVITEM));
	lvItem.iItem =  ListView_GetItemCount(g_hwndListViewServer);
	
	REF_SERVER_INFO refSI;
	refSI.cGAMEINDEX  = pSI.cGAMEINDEX;
	refSI.dwIndex = pSI.dwIndex;
	pGI->pSC->vRefListSI.push_back(refSI);

	return 0; //Game scanner 1.0
	

//	ListView_SetItemCount(g_hwndListViewServer,currCV->vSIFiltered.size());

	lvItem.mask = LVIF_IMAGE ;	
	if(pSI.bPunkbuster)
		lvItem.iImage = 1; //Punkbuster
	else
		lvItem.iImage = 77; //empty
	ListView_InsertItem( g_hwndListViewServer,&lvItem);

	return 0;
}



long UpdateServerItem(DWORD index)
{
	ListView_Update(g_hwndListViewServer,index);
//	ListView_SetItemCount(g_hwndListViewServer,currCV->vSIFiltered.size());//,LVSICF_NOSCROLL);
	return 0;
}



SOCKET getsock(const char *host, unsigned short port, int family, int socktype, int protocol) {
	struct addrinfo hints={0};
	struct addrinfo *info, *iter;
	char portstr[32];

	sprintf(portstr, "%d", (int)port);

	hints.ai_family=family;
	hints.ai_socktype=socktype;
	hints.ai_protocol=protocol;

	if(getaddrinfo(host, portstr, &hints, &info)) {

		return INVALID_SOCKET;
	}

	for(iter=info; iter!=NULL; iter=iter->ai_next) {
		SOCKET sock=socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
		if(sock==INVALID_SOCKET) continue;

		if(connect(sock, iter->ai_addr, (int)iter->ai_addrlen)!=SOCKET_ERROR) {
			freeaddrinfo(info);

			return sock;
		}
		else closesocket(sock);
	}
	freeaddrinfo(info);
	return INVALID_SOCKET;
}


LRESULT ListView_CustomDraw (LPARAM lParam)
{
	LRESULT lResult = CDRF_DODEFAULT;	
	int iRow=0;
	LPNMLVCUSTOMDRAW pListDraw = (LPNMLVCUSTOMDRAW)lParam;
	switch(pListDraw->nmcd.dwDrawStage)
	{
	//	case CDDS_POSTPAINT:
	//		return (CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYSUBITEMDRAW); //CDRF_NOTIFYPOSTPAINT; //lResult;
		case CDDS_PREPAINT:
			return (CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYSUBITEMDRAW);
		case CDDS_ITEMPREPAINT:
			{
				iRow = (int)pListDraw->nmcd.dwItemSpec;
				if(iRow%2 == 0)
					pListDraw->clrTextBk = RGB(202, 221,250);
	
				if(pListDraw->nmcd.hdr.idFrom == IDC_LIST_SERVER)
				{		
					try{
						SERVER_INFO pSI = Get_ServerInfoByIndex(iRow); //currCV->pSC->vSIFiltered.at((int)iRow);
						if(pSI.dwIP != 0) //Quick and dirty fix if server index is out of range
						{
							if(pSI.bUpdated==false)
								pListDraw->clrText   = RGB(140, 140, 140);
							else
								pListDraw->clrText   = RGB(0, 0, 0);
							
							if(pSI.dwPing==9999)
								pListDraw->clrText   = RGB(255, 0, 0);
						}
				
					}
					catch(const exception& e)
					{						
						AddLogInfo(0,"Access Violation!!! (ListView_CustomDraw) part 1 %s\n",e.what());
					}
				}
			}
			return (  CDRF_NOTIFYSUBITEMDRAW );
			break;
	/*	case CDDS_ITEMPOSTPAINT:
		{
	
			return lResult; //CDRF_NOTIFYSUBITEMDRAW ;//lResult;
		}
		break;
*/
		case CDDS_ITEMPREPAINT|CDDS_SUBITEM:
			{
	
				int    nItem = static_cast<int>( pListDraw->nmcd.dwItemSpec );
				SERVER_INFO pSI;
				pListDraw->clrText   = RGB(0, 0, 0);
			
					if(pListDraw->nmcd.hdr.idFrom == IDC_LIST_SERVER)
					{
						try
						{
							pSI = Get_ServerInfoByIndex(nItem); //currCV->pSC->vSIFiltered.at((int)nItem);

							HDC  pDC =  pListDraw->nmcd.hdc;
							RECT rect,rectFull;

							if(pListDraw->iSubItem==CUSTCOLUMNS[COL_PLAYERS].columnIdx)
							{
								iRow = (int)pListDraw->nmcd.dwItemSpec;
						
								if(iRow%2 == 0)
								{
									pListDraw->clrTextBk = RGB(202, 221,250);
							//		return CDRF_NEWFONT; //CDRF_NEWFONT;
								}

								ListView_GetSubItemRect(g_hwndListViewServer,nItem,pListDraw->iSubItem,LVIR_BOUNDS,&rect);
								ListView_GetSubItemRect(g_hwndListViewServer,nItem,pListDraw->iSubItem,LVIR_BOUNDS,&rectFull);
						
								//ListView_GetItemIndexRect( g_hwndListViewServer,&lvi,pListDraw->iSubItem , LVIR_BOUNDS,&rect);
								rect.top+=3;
															
								float percentage =  ((float)pSI.nCurrentPlayers /  (float)pSI.nMaxPlayers) * 100.0f ;
								if(percentage>100.0f)
									percentage = 100;

								HBRUSH hbrBkgnd=NULL,hbrBar=NULL;
			
								
								//percentage/=2;
								if(percentage<0)
									percentage =0;

								rect.bottom-=3;
							//	SetROP2(pDC, R2_NOTXORPEN ); 
							
								hbrBkgnd = CreateSolidBrush(RGB(202, 221,250)); 

								if(iRow%2 == 0)
									FillRect(pDC, &rectFull, (HBRUSH) hbrBkgnd);

								Rectangle(pDC, rect.left-1, rect.top-1,rect.right-1, rect.bottom+1); 

								int maxPixel = rect.right-1 - rect.left-1;
								int pixelX = (percentage * maxPixel) / 100;
								rect.right = rect.left +pixelX; // percentage; //rect.left + pSI.nCurrentPlayers+2;
								RECT rect2;
								CopyRect(&rect2,&rect);	        

								for(int i=0; i<pixelX;i++)
								{
									int r = i*2;
									int g = 225-i*2;
									if(g<0)
										g=0;
									if(r>255)
										r = 255;
									hbrBar = CreateSolidBrush(RGB(r, g,0)); 
									rect.right = rect.left+1; //percentage; 								
									FillRect(pDC, &rect, (HBRUSH) hbrBar);
									DeleteObject(hbrBar);
									rect.left++;
								}
								CopyRect(&rect,&rect2);
								//(COLOR_HIGHLIGHTTEXT));
				//				SetBkMode(hdc, TRANSPARENT);
								char szText[50];
							//	ExcludeClipRect(pDC,rect.left,rect.top,rect.right,rect.bottom);
								sprintf_s(szText,sizeof(szText),"%d/%d+(%d)",pSI.nCurrentPlayers,pSI.nMaxPlayers,pSI.nPrivateClients);
							
								SetTextColor(pDC,0x00FFFFFF);
								
								TextOut(pDC, rect.left+2,rect.top-1, szText, strlen(szText));
								SetTextColor(pDC,0x0000000);
								ExcludeClipRect(pDC,rect.left,rect.top,rect.right,rect.bottom);
								TextOut(pDC, rect.left+2,rect.top-1, szText, strlen(szText));
							
								SetTextColor(pDC,0x000);
								if(hbrBkgnd!=NULL)
									DeleteObject(hbrBkgnd);
								if(hbrBar!=NULL)
									DeleteObject(hbrBar);


								return CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT  ;
								
							}
							else if(pListDraw->iSubItem==CUSTCOLUMNS[COL_COUNTRY].columnIdx)
							{
								HDC  hDC =  pListDraw->nmcd.hdc;
								RECT rc;
								HBRUSH hbrBkgnd,hbrSel;
								ListView_GetSubItemRect(g_hwndListViewServer,nItem,pListDraw->iSubItem,LVIR_BOUNDS,&rc);								
								int idxCC = Get_CountryFlagByShortName(pSI.szShortCountryName);
								hbrBkgnd = CreateSolidBrush(RGB(202, 221,250)); 

								iRow = (int)pListDraw->nmcd.dwItemSpec;

								if(iRow%2 == 0)
									FillRect(pDC, &rc, (HBRUSH) hbrBkgnd);
								
								if(pSI.dwPing==9999)
								{
									pListDraw->clrText   = RGB(255, 0, 0);
								}
								else
								{
									if(pSI.bUpdated==false)
										pListDraw->clrText   = RGB(140, 140, 140);
									else
										pListDraw->clrText   = RGB(0, 0, 0);
								}
								
								if(pListDraw->nmcd.uItemState & ( CDIS_SELECTED))
								{
									pListDraw->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT); //RGB(255, 255, 255);
									hbrSel = CreateSolidBrush( GetSysColor(COLOR_HIGHLIGHT)); //RGB(51,153,250)); 																
									FillRect(pDC, &rc, (HBRUSH) hbrSel);
								}
								ImageList_Draw(g_hILFlags,idxCC,hDC,rc.left+1,rc.top+2,ILD_NORMAL|ILD_TRANSPARENT);
																
								SetBkColor(hDC,pListDraw->clrTextBk);
								SetTextColor(hDC,pListDraw->clrText);								
								SetBkMode(hDC, TRANSPARENT);

								rc.left+=20;
								rc.top+=2;
								ExtTextOut(hDC,rc.left,rc.top,0, &rc,pSI.szCountry, strlen(pSI.szCountry),NULL);
								if(AppCFG.bUseShortCountry)
									TextOut(hDC, rc.left,rc.top, pSI.szShortCountryName, strlen(pSI.szShortCountryName));
								else
									ExtTextOut(hDC,rc.left,rc.top,0, &rc,pSI.szCountry, strlen(pSI.szCountry),NULL); //TextOut(hDC, rc.left+20,rc.top+2, pSI.szCountry, strlen(pSI.szCountry));
									
								DeleteObject(hbrBkgnd);

								return   CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT  ;
							}

							if(pSI.bUpdated==false)
								pListDraw->clrText   = RGB(140, 140, 140);
							else
								pListDraw->clrText   = RGB(0, 0, 0);
							
							if(pSI.dwPing==9999)
								pListDraw->clrText   = RGB(255, 0, 0);
							return  CDRF_NEWFONT; //;
							
							

						}
						catch(const exception& e)
						{
						// exception handling code
								AddLogInfo(0,"Access Violation!!! (ListView_CustomDraw) %s\n",e.what());
						}
					} 
						
					
				
				return  CDRF_DODEFAULT; //CDRF_NEWFONT;// CDRF_DODEFAULT;// CDRF_NEWFONT; //( CDRF_NOTIFYPOSTPAINT );
			}
			break;
	default:
		break;
	}
	return CDRF_DODEFAULT;
}

LRESULT TreeView_CustomDraw(LPARAM lParam)
{
	LRESULT lResult = CDRF_DODEFAULT;
	LPNMTVCUSTOMDRAW pCustomDraw = (LPNMTVCUSTOMDRAW)lParam;
	switch(pCustomDraw->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			return (CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYITEMDRAW);

		case CDDS_ITEMPREPAINT:
			{
				if (!(pCustomDraw->nmcd.uItemState & ( CDIS_FOCUS)))
				{
			
					if(pCustomDraw->nmcd.uItemState & ( CDIS_SELECTED))
					{
						pCustomDraw->clrText   = RGB(0, 0, 0);
						pCustomDraw->clrTextBk = RGB(202, 221,250);
	

					}
				}
			}
		case CDDS_ITEMPOSTPAINT:
		{
 		
			if (!(pCustomDraw->nmcd.uItemState & ( CDIS_FOCUS)))
			{

				if(pCustomDraw->nmcd.uItemState & ( CDIS_SELECTED))
				{
					pCustomDraw->clrText   = RGB(0, 0, 0);
					pCustomDraw->clrTextBk = RGB(202, 221,250);
	

				}
			}
			
			return lResult;
		}
		break;
	default:
		break;
	}
	return CDRF_DODEFAULT;
}



SERVER_INFO *FindServer(char *str)
{
	
	char copy1[100];//copy2[50];
	char copy2[120];
	char buff[120];

	SendDlgItemMessage(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD, CB_SHOWDROPDOWN, FALSE, 0); 
	SendDlgItemMessage (g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD, CB_RESETCONTENT, 0, 0); 
	DWORD counter=0;
	if(str==NULL)
		return NULL;

	if(strlen(str)<1)
		return NULL;

	ZeroMemory(copy1,sizeof(copy1));
	strncpy(copy1,str,49);

	_strlwr_s( copy1,sizeof(copy1));
		
	GAME_INFO *pGI = (GAME_INFO *)&GI[g_currentGameIdx];
	vSRV_INF::iterator  iLst;

	if(pGI->pSC->vSI.size()==0)
		return NULL;

	ListView_DeleteAllItems(g_hwndListViewServer); //5.44
	pGI->pSC->vRefListSI.clear();

	if(str[0]>='A' &&  str[0]<='z')
	{
		for ( iLst = pGI->pSC->vSI.begin( ); iLst != pGI->pSC->vSI.end( ); iLst++ )
		{
		
			SERVER_INFO pSI = *iLst;
			
			strncpy(copy2,colorfilter(pSI.szServerName,buff,sizeof(buff)),118);
			int l = strlen(copy2);
			if(copy2!=NULL || l!=0)	
			{
	
				_strlwr_s( copy2,sizeof(copy2));
				if(strstr(copy2,copy1)!=NULL)
				{
				//	sprintf_s(szText,198,"%s:%d - %s",pSI.szIPaddress,pSI.dwPort,buff);
					//SendDlgItemMessage (g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD, CB_ADDSTRING, 0, (LPARAM)szText); 
				//	SendDlgItemMessage (g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD, CB_INSERTSTRING, counter, (LPARAM)szText); 


		
					REF_SERVER_INFO refSI;
					refSI.dwIndex = pSI.dwIndex;
					refSI.cGAMEINDEX = pSI.cGAMEINDEX;
					pGI->pSC->vRefListSI.push_back(refSI);

				
					counter++;
				}
			}		
		}

	} else //could it be a IP address search?
	{
		char szIPPORT[50];
		
		for ( iLst = pGI->pSC->vSI.begin( ); iLst != pGI->pSC->vSI.end( ); iLst++ )
		{
			SERVER_INFO pSI = *iLst;//currCV->vSI.at((int)pLVItem->iItem);
			
			
			if(strstr(pSI.szIPaddress,str)!=NULL)
			{				
				//colorfilter(pSI.szServerName,buff,sizeof(buff));
				//sprintf_s(szText,198,"%s:%d - %s",pSI.szIPaddress,pSI.dwPort,buff);
				//SendDlgItemMessage (g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD, CB_INSERTSTRING, counter, (LPARAM)szText); 
				
				REF_SERVER_INFO refSI;
				refSI.dwIndex = pSI.dwIndex;
				refSI.cGAMEINDEX = pSI.cGAMEINDEX;
				pGI->pSC->vRefListSI.push_back(refSI);

				counter++;
			}else 
			{
				sprintf(szIPPORT,"%s:%d",pSI.szIPaddress,pSI.dwPort);
				if(strstr(szIPPORT,str)!=NULL)
				{
					REF_SERVER_INFO refSI;
					refSI.dwIndex = pSI.dwIndex;
					refSI.cGAMEINDEX = pSI.cGAMEINDEX;
					pGI->pSC->vRefListSI.push_back(refSI);

					counter++;

				}
			}
		}
	}

	int count = SendDlgItemMessage(  g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD,     (UINT) CB_GETCOUNT, 0,     0);  

	if(count>0)
		SendDlgItemMessage(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD, CB_SHOWDROPDOWN, TRUE, 0); 

	Do_ServerListSort(iLastColumnSortIndex);	
	ListView_SetItemCount(g_hwndListViewServer,pGI->pSC->vRefListSI.size());		


	return NULL;
}


void EnableButtons(bool active)
{
  //  EnableWindow( GetDlgItem(g_hWnd, IDC_BUTTON_JOIN),active);  //Join button

	::SendMessage(g_hwndToolbarOptions,	TB_ENABLEBUTTON, (WPARAM)IDM_SCAN_FILTERED, (LPARAM)MAKELONG(active,0)); 
	::SendMessage(g_hwndToolbarOptions,	TB_ENABLEBUTTON, (WPARAM)IDM_REFRESH, (LPARAM)MAKELONG(active,0)); 

}

void Show_StopScanningButton(BOOL show)
{
	TBBUTTONINFO tbbi;
	ZeroMemory(&tbbi, sizeof(TBBUTTONINFO));
	tbbi.cbSize = sizeof(TBBUTTONINFO);
	tbbi.dwMask = TBIF_IMAGE;
	::SendMessage(g_hwndToolbarOptions, TB_GETBUTTONINFO, IDM_REFRESH, (LPARAM)&tbbi);

	if(show)
		tbbi.iImage = 5;
	else
		tbbi.iImage = 0;
	::SendMessage(g_hwndToolbarOptions, TB_SETBUTTONINFO, IDM_REFRESH, (LPARAM)&tbbi);

	::SendMessage(g_hwndToolbarOptions,	TB_ENABLEBUTTON, (WPARAM)IDM_SCAN_FILTERED, (LPARAM)MAKELONG(!show,0)); 



}

void Show_ToolbarButton(int id, bool show)
{
	
	::SendMessage(g_hwndSearchToolbar,	TB_HIDEBUTTON, (WPARAM)id, (LPARAM)MAKELONG(!show,0)); 
	UpdateWindow(g_hwndSearchCombo);
}

bool bPlayingNotify=false;

DWORD WINAPI PlayNotify(LPVOID lpParam )
{
	 bPlayingNotify=true;
	 PlaySound(AppCFG.szNotifySoundWAVfile, 0, SND_FILENAME);
	 bPlayingNotify=false;
	 return 0;
}


LRESULT APIENTRY ComboBox_SearchSubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{

	HANDLE hglb;
	LPCSTR lptstr;
	if(uMsg == WM_COMMAND)
	 {
			DWORD wmId;
			DWORD wmEvent;
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			switch (wmId)
			{
				case ID_CTRL_V:
					{
						  if (IsClipboardFormatAvailable(CF_TEXT)) 
						  {
							if (OpenClipboard(hwnd)) 
							{
					 
								hglb = GetClipboardData(CF_TEXT); 
								if (hglb != NULL) 
								{ 
									lptstr = (LPCSTR)GlobalLock(hglb); 
									if (lptstr != NULL) 
									{ 
										SetDlgItemText(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD,lptstr);
										PostMessage(g_hWnd,WM_COMMAND,MAKELONG(IDC_COMBOBOXEX_CMD,CBN_EDITCHANGE),0);
										GlobalUnlock(hglb); 
									} 
								} 
								CloseClipboard();
							}
						  }
					}
				break;
			}

	 }
	 return CallWindowProc((WNDPROC)g_wpOrigCBSearchProc, hwnd, uMsg,  wParam, lParam); 
}



LRESULT APIENTRY ListViewPlayerSubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 
	HMENU hPopMenu;
	 if(uMsg == WM_COMMAND)
	 {
			DWORD wmId;
			DWORD wmEvent;
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				case ID_WARN_PLAYER:

						{
							int n=-1;
							char szPlayerName[MAX_BUDDYNAME_LEN];
							n = ListView_GetSelectionMark(g_hwndListViewPlayers);
							ListView_GetItemText(g_hwndListViewPlayers,n,2,szPlayerName,sizeof(szPlayerName)-1);
							if(n!=-1)
							{				
								char szCMD[80];		

								sprintf_s(szCMD,sizeof(szCMD),"say ^1WARN! ^wBe nice ^6%s",szPlayerName);
								RCON_Connect(g_CurrentSRV);
								RCON_SendCmd(g_CurrentSRV->szRCONPASS,szCMD); 
								RCON_Disconnect();
							}
						}
						break;
				case ID_MUTE_PLAYER:

						{
							int n=-1;
							char szPlayerName[MAX_BUDDYNAME_LEN];
							n = ListView_GetSelectionMark(g_hwndListViewPlayers);
							ListView_GetItemText(g_hwndListViewPlayers,n,2,szPlayerName,sizeof(szPlayerName)-1);
							if(n!=-1)
							{				
								char szCMD[80];		

								sprintf_s(szCMD,sizeof(szCMD),"mute %s",szPlayerName);
								RCON_Connect(g_CurrentSRV);
								RCON_SendCmd(g_CurrentSRV->szRCONPASS,szCMD); 
								RCON_Disconnect();
							}
						}
					break;
				case ID_UNMUTE_PLAYER:

						{
							int n=-1;
							char szPlayerName[MAX_BUDDYNAME_LEN];
							n = ListView_GetSelectionMark(g_hwndListViewPlayers);
							ListView_GetItemText(g_hwndListViewPlayers,n,2,szPlayerName,sizeof(szPlayerName)-1);
							if(n!=-1)
							{				
								char szCMD[80];		

								sprintf_s(szCMD,sizeof(szCMD),"unmute %s",szPlayerName);
								RCON_Connect(g_CurrentSRV);
								RCON_SendCmd(g_CurrentSRV->szRCONPASS,szCMD); 
								RCON_Disconnect();
							}
						}
					break;
				case ID_KICK_PLAYER:

						{
							int n=-1;
							char szPlayerName[MAX_BUDDYNAME_LEN];
							n = ListView_GetSelectionMark(g_hwndListViewPlayers);
							ListView_GetItemText(g_hwndListViewPlayers,n,2,szPlayerName,sizeof(szPlayerName)-1);
							if(n!=-1)
							{				
								char szCMD[80];		
								int ret = MessageBox(NULL,"Are you sure you want to kick player?!",szPlayerName,MB_YESNO);
								if(ret==IDYES)
								{
									sprintf_s(szCMD,sizeof(szCMD),"kick %s",szPlayerName);
									RCON_Connect(g_CurrentSRV);
									RCON_SendCmd(g_CurrentSRV->szRCONPASS,szCMD); 
									RCON_Disconnect();
								}
							}
						}
					break;
				case ID_STATS_PLAYER:
					{
							int n=-1;
							char szPlayerName[MAX_BUDDYNAME_LEN];
							n = ListView_GetSelectionMark(g_hwndListViewPlayers);
							ListView_GetItemText(g_hwndListViewPlayers,n,2,szPlayerName,sizeof(szPlayerName)-1);
							if(n!=-1)
							{
								char szURL[256];
								sprintf(szURL,"http://stats.enemyterritory.com/profile/%s",szPlayerName);								
								ShellExecute(NULL,NULL,szURL,NULL,NULL,SW_SHOWNORMAL);
								
							}
					break;
					}
				case ID_YAWN_PLAYER:
						{
							int n=-1;
							char szPlayerName[MAX_BUDDYNAME_LEN];
							n = ListView_GetSelectionMark(g_hwndListViewPlayers);
							ListView_GetItemText(g_hwndListViewPlayers,n,2,szPlayerName,sizeof(szPlayerName)-1);
							if(n!=-1)
							{
								char szURL[512];
							
								switch(g_currentGameIdx)
								{				
									default:
									case ET_SERVERLIST:	
										sprintf(szURL,"http://www.yawn.be/findPlayer.yawn?nick=%s&pbGuid=&etproGuid=&gamecode=ET",szPlayerName);
										break;								
									case Q4_SERVERLIST:
										sprintf(szURL,"http://www.yawn.be/findPlayer.yawn?nick=%s&pbGuid=&etproGuid=&gamecode=Q4",szPlayerName);
										break;
									case Q3_SERVERLIST:
										sprintf(szURL,"http://www.yawn.be/findPlayer.yawn?nick=%s&pbGuid=&gamecode=Q3",szPlayerName);
										break;
									case RTCW_SERVERLIST:
										sprintf(szURL,"http://www.yawn.be/findPlayer.yawn?nick=%s&pbGuid=&gamecode=RTCW",szPlayerName);
										break;
									case ETQW_SERVERLIST:
										sprintf(szURL,"http://www.yawn.be/findPlayer.yawn?nick=%s&pbGuid=&gamecode=ETQW",szPlayerName);
										break;

								}								
								ShellExecute(NULL,NULL,szURL,NULL,NULL,SW_SHOWNORMAL);
							}

							break;
						}
					case IDM_ADD:
						{
							OnAddSelectedPlayerToBuddyList();											
						}
					break;
			}
		}
 		else if(uMsg == WM_RBUTTONDOWN)
		{
			//get mouse cursor position x and y as lParam has the message itself 
			POINT lpClickPoint;
			GetCursorPos(&lpClickPoint);
			int n=-1;
			n = ListView_GetSelectionMark(g_hwndListViewPlayers);
			if(n!=-1)
			{
				//place the window/menu there if needed 						
				hPopMenu = CreatePopupMenu();
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_ADD,"&Add to buddy list");			
				switch(g_currentGameIdx)
				{
					case RTCW_SERVERLIST:
					case Q4_SERVERLIST:
					case Q3_SERVERLIST:
					case COD_SERVERLIST:
					case COD2_SERVERLIST:					
					case ET_SERVERLIST:
					case ETQW_SERVERLIST:
						InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_YAWN_PLAYER,"Search player at YAWn!");			
					break;
				}
				switch(g_currentGameIdx)
				{
					case ETQW_SERVERLIST:
						InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_STATS_PLAYER,"Search player at ETQW stats!");			
					break;
				}

				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_KICK_PLAYER,"Kick");			
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_WARN_PLAYER,"Warn");			
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_MUTE_PLAYER,"Mute");			
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_UNMUTE_PLAYER,"UnMute");			
												
				//workaround for microsoft bug, to hide menu w/o selecting
				SetForegroundWindow(hwnd);
				TrackPopupMenu(hPopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,lpClickPoint.x, lpClickPoint.y,0,hwnd,NULL);
				SendMessage(hwnd,WM_NULL,0,0);
				DestroyMenu(hPopMenu); 

			}
			return 0;
		}				
    return CallWindowProc((WNDPROC)g_wpOrigListViewPlayersProc, hwnd, uMsg,  wParam, lParam); 
}




void StartGame_ConnectToServer(bool connectFromBuddyList)
{
	int n=-1;
	char szIP[128];

	if(connectFromBuddyList)
	{
		n = ListView_GetSelectionMark(g_hwndListBuddy);
		ListView_GetItemText(g_hwndListBuddy,n,2,szIP,sizeof(szIP)-1);
		if(n!=-1)  //Valid selection?
		{
			if(strlen(szIP)>0)
			{
				LVITEM lvItem;
				memset(&lvItem,0,sizeof(LVITEM));
				lvItem.mask =  LVIF_PARAM ; 
				lvItem.iItem = n;
				lvItem.iSubItem = 0;		
				if(ListView_GetItem( g_hwndListBuddy, &lvItem))
				{
					BUDDY_INFO *pBI;

					pBI = (BUDDY_INFO*)lvItem.lParam;	
					if(pBI==NULL)
						return;

					SERVER_INFO pSI = GI[pBI->cGAMEINDEX].pSC->vSI.at(pBI->sIndex);
					LaunchGame(pSI,&GI[pSI.cGAMEINDEX]);
			
				}
			}else
				MessageBox(NULL,"Selected buddy isn't playing\non any server!","Info",MB_OK);
		} else
			MessageBox(NULL,"Please select a server\nbefore trying to connect!","Info",MB_OK);
	}
	else  //From Favorites or the Masterlist
	{	
		n = ListView_GetSelectionMark(g_hwndListViewServer);
		if(n!=-1)
		{

			SERVER_INFO pSrv;
			pSrv =  Get_ServerInfoByIndex(n);//currCV->pSC->vSIFiltered.at(n);
			LaunchGame(pSrv,&GI[g_currentGameIdx]);
		}
	}

}

int Get_ServerInfoByListIndex(int index)
{
	REF_SERVER_INFO refSI;
	refSI.dwIndex = 0;

	if(currCV->pSC->vRefListSI.size()>0)
	{
		try
		{				
			refSI = currCV->pSC->vRefListSI.at(index);
		}	catch(const exception& e)
		{
			AddLogInfo(ETSV_ERROR,"Error at GetServerInfoByListIndex %s!",e.what());
			return -1;
		}		

	} else
		return -1;
	return refSI.dwIndex;
}

SERVER_INFO Get_ServerInfoByIndex(int index)
{

	SERVER_INFO srv;
	ZeroMemory(&srv,sizeof(SERVER_INFO));
	int idx = Get_ServerInfoByListIndex(index);
	if(idx==-1)
	{
		dbg_print("> Error at Get_ServerInfoByIndex index = %d",index);
		return srv;
	}
		//DebugBreak();
	try
	{

		srv = currCV->pSC->vSI.at(idx);
	}catch(std::out_of_range)
	{
		//DebugBreak();
		AddLogInfo(0,"Vector: Out of range");
	}catch(const exception& e)
	{
		//DebugBreak();
		AddLogInfo(ETSV_ERROR,"<<<<<<< Error at Get_ServerInfoByIndex out of vector scope! %s >>>>>>>>",e.what());
	}

	return srv;
}

void Favorite_Add(bool manually)
{
	LVITEM lvItem;
	if(manually)
	{
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ADD_SERVER), NULL,(DLGPROC)AddServerProc);
		SetStatusText(ICO_INFO,"Added server to Favorites!");
		RedrawServerListThread(&GI[g_currentGameIdx]);
	
	} else
	{

		char szPORT[20];
		int i = ListView_GetSelectionMark(g_hwndListViewServer);
		g_iCurrentSelectedServer = -1;
		if(i!=-1)
		{
			
			memset(&lvItem,0,sizeof(LVITEM));
			SERVER_INFO *pSrv=NULL;	
			lvItem.mask =  LVIF_PARAM ; 
			lvItem.iItem = i;
			lvItem.iSubItem = 3;		
			if(ListView_GetItem( g_hwndListViewServer, &lvItem))
			{
				pSrv = (SERVER_INFO*)lvItem.lParam;	
				
				if(pSrv==NULL)
				{
					SetStatusText(ICO_WARNING,"Not a valid server!");
					InvalidateRect(g_hWnd,NULL,TRUE);
					return;
				}
				pSrv->cFavorite = 1;

				strcpy(g_currServerIP,pSrv->szIPaddress);
				dwCurrPort = pSrv->dwPort;		
			}
			g_iCurrentSelectedServer = i;
			_itoa(pSrv->dwPort,szPORT,10);

			//To favorite icon
			memset(&lvItem,0,sizeof(LVITEM));
			lvItem.iItem = i;
			lvItem.iSubItem = 3;
			lvItem.mask = LVIF_IMAGE  ;// |LVIF_INDENT;
			lvItem.iImage = 2;  //Favorite icon
			ListView_SetItem(g_hwndListViewServer,&lvItem);

		}
	}
}


SERVER_INFO g_thisRCONServer;

void OnRCON()
{

	int i = ListView_GetSelectionMark(g_hwndListViewServer);
	if(i!=-1)
	{
		TabCtrl_SetCurSel(g_hwndTabControl,1);
		ShowWindow(g_hwndMainRCON,SW_SHOW);
		ShowWindow(g_hwndMainSTATS,SW_HIDE);
		ShowWindow(g_hwndLogger,SW_HIDE);
		ShowWindow(g_hwndListViewPlayers,SW_HIDE);
		SendMessage(g_hwndMainSTATS,WM_STOP_PING,0,0);
		//g_thisRCONServer = Get_ServerInfoByIndex(i);	
		//RCON_Start(hInst,g_hWnd,&g_thisRCONServer);
		//currCV->pSC->vSI.at((int)g_thisRCONServer.dwIndex) = g_thisRCONServer;
	
	} else
	{
	  MessageBox(NULL,"Please select a server before connecting to Remote Console!","Info!",MB_ICONINFORMATION|MB_OK); 

	}
}


LRESULT APIENTRY TreeView_SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 

	HMENU hPopMenu;
	if(uMsg == WM_MOUSEMOVE)
	{
		if(bRunningQuery)
			SetCursor(LoadCursor(NULL, IDC_APPSTARTING));

	}
	else if(uMsg == WM_COMMAND)
	 {
			DWORD wmId;
			DWORD wmEvent;
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:

			switch (wmId)
			{
				case IDM_SCAN:
					OnActivate_ServerList();
				break;
				case IDM_REFRESH: 
					RedrawServerListThread(&GI[g_currentGameIdx]);
				break;
				case IDM_ADDIP:
					Favorite_Add(true);
				break;
				case ID_FOO_MINIMUMPLAYERS:
					OnMinMaxPlayers(g_hWnd,false);
				break;
				case ID_FOO_MAXIMUMPLAYERS:
					OnMinMaxPlayers(g_hWnd,true);
				break;
				case IDM_LAUNCH_GAME_ONLY:
					ExecuteGame(currCV,"");
				break;
				

			}
		}
 		else if(uMsg == WM_RBUTTONDOWN)
		{
			//get mouse cursor position x and y as lParam has the message itself 
			POINT lpClickPoint;
			HTREEITEM hCurrent = TreeView_GetSelection(hwnd);
			if(hCurrent==NULL)
				return 0;
			GetCursorPos(&lpClickPoint);
			hPopMenu = CreatePopupMenu();
			HMENU hSubPopMenu;
			hSubPopMenu = CreatePopupMenu();
	
				//place the window/menu there if needed 						

			char szText[100];
			sprintf_s(szText,sizeof(szText),"Find %s internet servers.",GI[g_currentGameIdx].szGAME_NAME);

			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_SCAN,szText);
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_LAUNCH_GAME_ONLY,"Launch game only.");
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_ADDIP,"&Add IP to Favorites");
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_FOO_MINIMUMPLAYERS,"&Modify minimum players a server must have.");
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_FOO_MAXIMUMPLAYERS,"&Modify maximum players a server must have.");

			//workaround for microsoft bug, to hide menu w/o selecting
			SetForegroundWindow(hwnd);
			TrackPopupMenu(hPopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,lpClickPoint.x, lpClickPoint.y,0,hwnd,NULL);
			SendMessage(hwnd,WM_NULL,0,0);
			DestroyMenu(hPopMenu);
			DestroyMenu(hSubPopMenu);
			return TRUE;
		}	
	 
    return CallWindowProc((WNDPROC)g_wpOrigTreeViewProc, hwnd, uMsg,  wParam, lParam);  
} 

HBITMAP LoadIconAsBitmap(HINSTANCE hResourceDll, UINT nIDResource)
{
	HICON hIcon = (HICON)::LoadImage(hResourceDll, MAKEINTRESOURCE(nIDResource),IMAGE_ICON, 10, 10, LR_LOADTRANSPARENT);


	if (hIcon)
	{
		ICONINFO iconInfo;
		::GetIconInfo(hIcon, & iconInfo);
		::DeleteObject(iconInfo.hbmMask); 
		::DestroyIcon(hIcon);
		return iconInfo.hbmColor;
	}

return NULL;
}

HMENU LV_Header_PopMenu = NULL;

LRESULT APIENTRY LV_SL_HeaderSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{

	switch(uMsg)
	{
		case WM_NOTIFY:
			{
				NMHEADER *pNMheader;  //List View Header Control
				pNMheader = (LPNMHEADER)lParam;
				//if(pNMheader->hdr.hwndFrom==
				switch(pNMheader->hdr.code)
				{
					case HDN_BEGINTRACK:
					case HDN_ENDTRACK:
						{
							 HDITEM *pHDitem;
							 pHDitem = pNMheader->pitem;
	//						 CUSTCOLUMNS[0].lvColumn.cx
						}
						break;

				}
			}
	}

	 if(uMsg == WM_COMMAND)
	 {
		DWORD wmId;
		DWORD wmEvent;
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		if(wmId == IDM_RESTORE_COLUMNS)
		{
			ListView_SetDefaultColumns();
			ListView_InitilizeColumns();
			return TRUE;
		}
		for(int i=0;i<MAX_COLUMNS;i++)
		{
			if (wmId==CUSTCOLUMNS[i].id)
			{							
				MENUITEMINFO mii;
				memset(&mii,0,sizeof(MENUITEMINFO));
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_STATE;

				if(GetMenuItemInfo(LV_Header_PopMenu,wmId,FALSE,&mii))
				{				
					if(MFS_CHECKED == (mii.fState  & MFS_CHECKED) )
					{
						CUSTCOLUMNS[i].bActive = FALSE;
						mii.fState = MFS_UNCHECKED;
					} else
					{
						CUSTCOLUMNS[i].bActive = TRUE;
						mii.fState = MFS_CHECKED;
					}
					SetMenuItemInfo(LV_Header_PopMenu,wmId,FALSE,&mii);
					ListView_ReInitializeColumns();					
					
				}
				DestroyMenu(LV_Header_PopMenu);	
				return TRUE;
			}
		}
		return FALSE;
	 }
	if(uMsg == WM_RBUTTONDOWN)
	{
			//get mouse cursor position x and y as lParam has the message itself 
			POINT lpClickPoint;
			GetCursorPos(&lpClickPoint);
			LV_Header_PopMenu = CreatePopupMenu();
			MENUITEMINFO mii;
			for(int i=0;i<MAX_COLUMNS;i++)
			{
				ZeroMemory(&mii,sizeof(MENUITEMINFO));
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_STRING | MIIM_ID |   MIIM_STATE | MIIM_FTYPE;
				mii.fType = MFT_STRING ;
				mii.wID = CUSTCOLUMNS[i].id;
				mii.dwTypeData = (LPSTR) CUSTCOLUMNS[i].sName.c_str();	
				if(CUSTCOLUMNS[i].bActive)
					mii.fState = MFS_CHECKED;
				else
					mii.fState = MFS_UNCHECKED;
				InsertMenuItem(LV_Header_PopMenu,0,FALSE,&mii);
			}
			InsertMenu(LV_Header_PopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_RESTORE_COLUMNS,"Restore Columns to Default");
			
			SetForegroundWindow(hwnd);
			TrackPopupMenu(LV_Header_PopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,lpClickPoint.x, lpClickPoint.y,0,hwnd,NULL);
			SendMessage(hwnd,WM_NULL,0,0);
			//DestroyMenu(LV_Header_PopMenu);	//is being destroy by
			return 0;
	}
	return CallWindowProc((WNDPROC)g_wpOrigSLHeaderProc, hwnd, uMsg,  wParam, lParam);  
}


HBITMAP hBmp;
LRESULT APIENTRY ListViewServerListSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 
	HMENU hPopMenu;
	POINT lpClickPoint;
	HMENU hSubPopMenu;		
	
	switch(uMsg)
	{
		case WM_NOTIFY:
			{
				NMHEADER *pNMheader;  //List View Header Control
				pNMheader = (LPNMHEADER)lParam;
				//if(pNMheader->hdr.hwndFrom==
				switch(pNMheader->hdr.code)
				{					
					case HDN_ENDTRACKA:
					case HDN_ENDTRACKW:
						{
							 HDITEM *pHDitem;
							 pHDitem = pNMheader->pitem;
							for(int i=0;i<MAX_COLUMNS;i++)
							{
								if(CUSTCOLUMNS[i].bActive && (CUSTCOLUMNS[i].columnIdx==pNMheader->iItem))
								{
									CUSTCOLUMNS[i].lvColumn.cx = pNMheader->pitem->cxy;
								}
							}
	//						 CUSTCOLUMNS[0].lvColumn.cx
						}
						break;
					case HDN_BEGINDRAG:
						{
							int iItems,	*lpiArray;
							// Get memory for buffer.
							if((iItems = SendMessage(pNMheader->hdr.hwndFrom, HDM_GETITEMCOUNT, 0,0))!=-1)
							{
								if(!(lpiArray =(int*) calloc(iItems,sizeof(int))))
								{
									MessageBox(hwnd, "Out of memory.","Error", MB_OK);
								}
								else
								{
									SendMessage(pNMheader->hdr.hwndFrom, HDM_GETORDERARRAY , iItems,(LPARAM)lpiArray);
									int c=0;
									for(int i=0;i<MAX_COLUMNS;i++)
									{
										if(CUSTCOLUMNS[i].bActive)
										{
																				
											dbg_print("BD Before pos %d %s",CUSTCOLUMNS[lpiArray[c]].id,CUSTCOLUMNS[lpiArray[c]].sName.c_str());
											c++;
										}
									}
									free(lpiArray);
								}
							}
							return FALSE;
						}			
					break;
					case HDN_ENDDRAG:
						{
							PostMessage(g_hWnd,WM_UPDATESERVERLIST_COLUMNS,0,0);
							return FALSE;
						}
						break;
				}
			}
	}
	if(uMsg == WM_HOTKEY)		
	{
		if (wParam == HOTKEY_ID_CTRL_C)
		{
			int n=-1;
			char szIP[40];
			n = ListView_GetSelectionMark(g_hwndListViewServer);
		
			ListView_GetItemText(g_hwndListViewServer,n,10,szIP,sizeof(szIP)-1);
			if(n!=-1)
			{				
				EditCopy(szIP);
				SetStatusText(ICO_INFO,"IP address %s added to clipboard!",szIP);
			}else
			{
				SetStatusText(ICO_WARNING,"Please select a server before trying to copy to clipboard!");
				InvalidateRect(g_hWnd,NULL,TRUE);
				return 0;
			}
		}
	}
	else if(uMsg == WM_KEYUP)
	{
		if((wParam==VK_UP) || (wParam==VK_DOWN))
		{
			OnServerSelected(currCV);
			return TRUE;
		}


	}
	else if(uMsg == WM_MOUSEMOVE)
	{
		if(bRunningQuery)
			SetCursor(LoadCursor(NULL, IDC_APPSTARTING));

	}
		
	else if(uMsg == WM_COMMAND)
	 {
			DWORD wmId;
			DWORD wmEvent;
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 

			// Parse the menu selections:
			switch (wmId)
			{
				case ID_OPTIONS_RCON:
					{
						OnRCON();
					}
				break;
				case IDM_PRIVPASS:
					{
						int i = ListView_GetSelectionMark(g_hwndListViewServer);
						if(i==-1)
						{
							MessageBox(NULL,"Please select a server before setting a private password!","Info!",MB_ICONINFORMATION|MB_OK); 
							return TRUE;
						}
						SERVER_INFO pSI = Get_ServerInfoByIndex(i);
						g_PRIVPASSsrv = &pSI;

						DialogBox(g_hInst, (LPCTSTR)IDD_DLG_SETPRIVPASS, g_hWnd, (DLGPROC)PRIVPASS_Proc);	
			
						currCV->pSC->vSI.at((int)pSI.dwIndex) = pSI;

					}
				break;
				case ID_YAWN_SERVER:
					{
						char szURL[512];
						char szIP[40];
						int n = ListView_GetSelectionMark(g_hwndListViewServer);
									
						ListView_GetItemText(g_hwndListViewServer,n,10,szIP,sizeof(szIP)-1);
						if(n!=-1)
						{
							switch(g_currentGameIdx)
							{				
								default:
								case ET_SERVERLIST:	
									sprintf_s(szURL,sizeof(szURL),"http://www.yawn.be/findServer.yawn?hostname=&serverAddress=%s&version=&protocol=&modid=0&game=ET&action=",szIP);
									break;								
								case Q4_SERVERLIST:
									sprintf_s(szURL,sizeof(szURL),"http://www.yawn.be/findServer.yawn?hostname=&serverAddress=%s&version=&protocol=&modid=0&game=Q4&action=",szIP);
									break;
								case ETQW_SERVERLIST:
									sprintf_s(szURL,sizeof(szURL),"http://etqw.splatterladder.com/?mod=serverlist&phrase=%s",szIP);
									break;


							}
							ShellExecute(NULL,NULL,szURL,NULL,NULL,SW_SHOWNORMAL);
						}
					}
					break;
				case IDM_REFRESH: 
					OnActivate_ServerList();
					break;
				case IDM_DELETE:
				
					break;
				case IDM_ADDIP:
					{

						int n = ListView_GetSelectionMark(g_hwndListViewServer);
						if(n!=-1)
						{
							SERVER_INFO pSI = Get_ServerInfoByIndex(n); 						
							pSI.cFavorite =! pSI.cFavorite;							
							currCV->pSC->vSI.at((int)pSI.dwIndex) = pSI;
							UpdateServerItem(n);

						}
					}
				break;
				case ID_TT_SERVER1:
					{

						TabCtrl_SetCurSel(g_hwndTabControl,3);
						ShowWindow(g_hwndMainRCON,SW_HIDE);
						ShowWindow(g_hwndMainSTATS,SW_SHOW);
						ShowWindow(g_hwndLogger,SW_HIDE);
						ShowWindow(g_hwndListViewPlayers,SW_HIDE);
						
						SendMessage(g_hwndMainSTATS,WM_START_PING,0,0);
						
					}
					break;
				case ID_TT_SERVER2:
					{
						int n=-1;
						char szIP[40];				
						n = ListView_GetSelectionMark(g_hwndListViewServer);
									
						ListView_GetItemText(g_hwndListViewServer,n,10,szIP,sizeof(szIP)-1);
						ShellExecute(NULL,"open","tracert.exe",szIP,NULL,SW_SHOWNORMAL);
					}
					break;

				case IDM_COPYIP:
					{
						int n=-1;
						
						n = ListView_GetSelectionMark(g_hwndListViewServer);
						if(n!=-1)
						{
							char szIP[50];
							SERVER_INFO pSI = Get_ServerInfoByIndex(n);
							sprintf(szIP,"%s:%d",pSI.szIPaddress,pSI.dwPort);
							EditCopy(szIP);
						}else
							MessageBox(hwnd,"Please select a server\nbefore trying to copy it to clipboard!",NULL,MB_OK);
					}
				break;
				case IDM_CONNECT:
					StartGame_ConnectToServer(false);					
				break;
			}
		}
 		else if(uMsg == WM_RBUTTONDOWN)
		{
			//get mouse cursor position x and y as lParam has the message itself 
			GetCursorPos(&lpClickPoint);
			hPopMenu = CreatePopupMenu();
		
			hSubPopMenu = CreatePopupMenu();
			int n=-1;
			n = ListView_GetSelectionMark(g_hwndListViewServer);
			if(n!=-1)
			{			
				//place the window/menu there if needed 						
				hBmp = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP_CONNECT));

				//hBmp = (HBITMAP)::LoadImage(hInst, MAKEINTRESOURCE(ICON_FLAG_10),IMAGE_ICON, 16, 16, LR_LOADTRANSPARENT);
				//hBmp =	LoadIconAsBitmap(hInst, IDI_ICON4);
				MENUITEMINFO mii;

				ZeroMemory(&mii,sizeof(MENUITEMINFO));
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_STRING | MIIM_ID |  MIIM_CHECKMARKS | MIIM_STATE | MIIM_FTYPE;
				mii.fType = MFT_STRING ;
				mii.wID = IDM_CONNECT;
				mii.hbmpUnchecked = NULL;
				mii.hbmpChecked = hBmp;
				mii.dwTypeData = "Connect";
				mii.cch = strlen("Connect");
				mii.fState = MFS_CHECKED | MFS_DEFAULT;
				InsertMenuItem(hPopMenu,IDM_CONNECT,FALSE,&mii);

				SERVER_INFO pSI = Get_ServerInfoByIndex(n); 						
				
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_LAUNCH_GAME_ONLY,"Launch game only.");

				if(pSI.cFavorite==0)
					InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_ADDIP,"Add Server to favorites");
				else
					InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_ADDIP,"Remove Server from favorites");
	
				//		InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_DELETE,"&Delete");
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_PRIVPASS,"Set Private Pass");
			
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_COPYIP,"Copy &IP to text                  (Ctrl+C)");

				if(g_currentGameIdx==ETQW_SERVERLIST)
					InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_YAWN_SERVER,"Search server at Splatterladder!");
				else
					InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_YAWN_SERVER,"Search server at YAWn!");

	
			}										

				if(g_bRunningQueryServerList==false)
				{
				
					InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_OPTIONS_RCON,"RCON");
					InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_REFRESH,"&Refresh");
				}
				else
				{
					InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING|MF_GRAYED,ID_OPTIONS_RCON,"RCON");
					InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING|MF_GRAYED,IDM_REFRESH,"&Refresh");				
				}

				InsertMenu(hPopMenu,0xFFFFFFFF,MF_POPUP|MF_BYPOSITION|MF_STRING,(UINT_PTR)hSubPopMenu,"Network tools");
				InsertMenu(hSubPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_TT_SERVER1,"Ping server");
				InsertMenu(hSubPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_TT_SERVER2,"Trace route server");

			
				
				SetForegroundWindow(hwnd);
				TrackPopupMenu(hPopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,lpClickPoint.x, lpClickPoint.y,0,hwnd,NULL);
				SendMessage(hwnd,WM_NULL,0,0);
				DestroyMenu(hPopMenu);
				DestroyMenu(hSubPopMenu);
				DeleteObject(hBmp);
	
			return 0;
		}	

    return CallWindowProc((WNDPROC)g_wpOrigListViewServerProc, hwnd, uMsg,  wParam, lParam);  
} 
 
BOOL WINAPI EditCopy(char *pText) 
{ 
    LPTSTR  lptstrCopy; 
    HGLOBAL hglbCopy; 
    int cch; 
 
    if (!OpenClipboard(g_hWnd)) 
        return FALSE; 
    EmptyClipboard(); 

    if (pText!=NULL) 
    { 
		cch = strlen(pText);
        if (cch==0)     // zero length
        {   
            CloseClipboard();                   // selection 
            return FALSE; 
        } 
 
 
        // Allocate a global memory object for the text. 
 
        hglbCopy = GlobalAlloc(GMEM_MOVEABLE, 
            (cch + 1) * sizeof(TCHAR)); 
        if (hglbCopy == NULL) 
        { 
            CloseClipboard(); 
            return FALSE; 
        } 
 
        // Lock the handle and copy the text to the buffer. 
 
        lptstrCopy = (LPTSTR)GlobalLock(hglbCopy); 
        memcpy(lptstrCopy,(LPTSTR)pText,cch); 
        lptstrCopy[cch] = (TCHAR) 0;    // null character 
        GlobalUnlock(hglbCopy); 
 
        // Place the handle on the clipboard. 
 
        SetClipboardData(CF_TEXT, hglbCopy); 
    } 
 
    // Close the clipboard. 
 
    CloseClipboard(); 
 
    return TRUE; 
}


void Sync_ServerInfo(SERVER_INFO pSI)
{
	GAME_INFO *pGI = &GI[pSI.cGAMEINDEX];
	pGI->pSC->vSI.at(pSI.dwIndex) = pSI;
}

BOOL ExecuteGame(GAME_INFO *pGI,char *szCmd)
{
	char LoadLocation[512],  WETFolder[512];
	HINSTANCE hret=NULL;
	if(strlen(pGI->szGAME_PATH)>0)
	{			
		strcpy(WETFolder,pGI->szGAME_PATH);
		char* pos = strrchr(WETFolder,'\\');
		if(pos!=NULL)
		{
			pos[1]=0;
		}
		strcpy(LoadLocation,pGI->szGAME_PATH);
		AddLogInfo(ETSV_DEBUG,LoadLocation);
		hret = ShellExecute(NULL, "open", LoadLocation, szCmd,WETFolder, 1);

		if((int)hret<=32)
		{
			MessageBox(NULL,"Couldn't execute game.\nReason: Install path may not be correct or doesn't exsist.\nSolution: Go to the Options then setup the game paths correct.","Error!",MB_OK);
			return FALSE;
		}
		return TRUE;
	}
	else
		MessageBox(NULL,"Invalid game path!\nReason: Install path may not be correct or doesn't exsist.\nSolution: Go to the Options then setup the game paths correct.","Error!",MB_OK);	

	return FALSE;
}

HWND hEdit = NULL;
LRESULT OnNotify(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	    
		NMITEMACTIVATE *lpnmia;
		lpnmia = (LPNMITEMACTIVATE)lParam;
  
		NMTREEVIEW *pnmtv;
		pnmtv = (LPNMTREEVIEW) lParam;

		NMHDR *pNMHdr;
		pNMHdr = (LPNMHDR) lParam;

		NMTOOLBAR *lpnmtb;
	    lpnmtb = (LPNMTOOLBAR) lParam;

	
	
		switch (((LPNMHDR) lParam)->code) 
		{ 

			case TCN_SELCHANGE:		//Tab Control Notification
				{
					NMHDR *lpnmhdr;
					lpnmhdr = (LPNMHDR) lParam; 
					if(lpnmhdr->hwndFrom = g_hwndTabControl)
					{
						int iSel =  TabCtrl_GetCurSel(g_hwndTabControl);
						if(iSel==0)
						{
							WNDCONT[WIN_PLAYERS].bShow = TRUE;
							WNDCONT[WIN_RULES].bShow = FALSE;
							WNDCONT[WIN_RCON].bShow = FALSE;
							WNDCONT[WIN_PING].bShow = FALSE;
							WNDCONT[WIN_LOGGER].bShow = FALSE;
							ShowWindow(g_hwndListViewVars,SW_HIDE);
							ShowWindow(g_hwndMainRCON,SW_HIDE);		
							ShowWindow(g_hwndMainSTATS,SW_HIDE);
							ShowWindow(g_hwndLogger,SW_HIDE);
							ShowWindow(g_hwndListViewPlayers,SW_SHOW);
							SendMessage(g_hwndMainSTATS,WM_STOP_PING,0,0);
							
						}
						else if (iSel==1)
						{
							WNDCONT[WIN_PLAYERS].bShow = FALSE;
							WNDCONT[WIN_RULES].bShow = TRUE;
							WNDCONT[WIN_RCON].bShow = FALSE;
							WNDCONT[WIN_PING].bShow = FALSE;
							WNDCONT[WIN_LOGGER].bShow = FALSE;
							ShowWindow(g_hwndListViewVars,SW_SHOW);
							ShowWindow(g_hwndMainRCON,SW_HIDE);
							ShowWindow(g_hwndMainSTATS,SW_HIDE);
							ShowWindow(g_hwndLogger,SW_HIDE);
							ShowWindow(g_hwndListViewPlayers,SW_HIDE);
							SendMessage(g_hwndMainSTATS,WM_STOP_PING,0,0);
						}
						else if (iSel==2)
						{
							WNDCONT[WIN_PLAYERS].bShow = FALSE;
							WNDCONT[WIN_RULES].bShow = FALSE;
							WNDCONT[WIN_RCON].bShow = TRUE;
							WNDCONT[WIN_PING].bShow = FALSE;
							WNDCONT[WIN_LOGGER].bShow = FALSE;
							ShowWindow(g_hwndListViewVars,SW_HIDE);
							ShowWindow(g_hwndMainRCON,SW_SHOW);
							ShowWindow(g_hwndMainSTATS,SW_HIDE);
							ShowWindow(g_hwndLogger,SW_HIDE);
							ShowWindow(g_hwndListViewPlayers,SW_HIDE);
							SendMessage(g_hwndMainSTATS,WM_STOP_PING,0,0);
						}
						else if (iSel==3)
						{
							WNDCONT[WIN_PLAYERS].bShow = FALSE;
							WNDCONT[WIN_RULES].bShow = FALSE;
							WNDCONT[WIN_RCON].bShow = FALSE;
							WNDCONT[WIN_PING].bShow = TRUE;
							WNDCONT[WIN_LOGGER].bShow = FALSE;
							ShowWindow(g_hwndListViewVars,SW_HIDE);
							ShowWindow(g_hwndMainRCON,SW_HIDE);
							ShowWindow(g_hwndMainSTATS,SW_SHOW);
							ShowWindow(g_hwndLogger,SW_HIDE);
							ShowWindow(g_hwndListViewPlayers,SW_HIDE);

						}

						else if (iSel==4)
						{
							WNDCONT[WIN_PLAYERS].bShow = FALSE;
							WNDCONT[WIN_RULES].bShow = FALSE;
							WNDCONT[WIN_RCON].bShow = FALSE;
							WNDCONT[WIN_PING].bShow = FALSE;
							WNDCONT[WIN_LOGGER].bShow = TRUE;
							ShowWindow(g_hwndListViewVars,SW_HIDE);
							ShowWindow(g_hwndMainRCON,SW_HIDE);
							ShowWindow(g_hwndMainSTATS,SW_HIDE);
							ShowWindow(g_hwndLogger,SW_SHOW);
							ShowWindow(g_hwndListViewPlayers,SW_HIDE);
							SendMessage(g_hwndMainSTATS,WM_STOP_PING,0,0);
						}

					}
					break;
				}

			//case TBN_DROPDOWN:
			//	OnScanningOptions(hWnd);
			//break;
			case LVN_GETDISPINFO:
				if(lpnmia->hdr.hwndFrom == g_hwndListViewServer)
					return OnGetDispInfoList(wParam,(NMHDR*)lParam);
				return FALSE;
			break;
			case TTN_GETDISPINFO: 
				{ 
				LPTOOLTIPTEXT lpttt; 
				UINT_PTR idButton;
				lpttt = (LPTOOLTIPTEXT) lParam; 
				lpttt->hinst = g_hInst; 

				// Specify the resource identifier of the descriptive 
				// text for the given button. 
				idButton = lpttt->hdr.idFrom; 
					switch (idButton) 
					{ 
						case IDM_REFRESH: 
							lpttt->lpszText = MAKEINTRESOURCE(IDS_REFRESH); 
							break; 
						case IDM_SCAN_FILTERED:
							lpttt->lpszText = MAKEINTRESOURCE(IDS_REFRESH_FILTER); 
							break;
						case IDM_SETTINGS: 
							lpttt->lpszText = MAKEINTRESOURCE(IDS_OPTIONS); 
							break; 
						case IDC_BUTTON_ADD_SERVER: 
							lpttt->lpszText = MAKEINTRESOURCE(IDS_ADD_TO_FAVORITES); 
							break; 
						case IDC_BUTTON_QUICK_CONNECT: 
							lpttt->lpszText = MAKEINTRESOURCE(IDS_FAST_CONNECT); 
							break; 
						case IDC_BUTTON_FIND: 
							lpttt->lpszText = MAKEINTRESOURCE(IDS_SEARCH); 
							break; 
						case IDC_DOWNLOAD: 
							lpttt->lpszText = MAKEINTRESOURCE(IDS_DOWNLOAD); 
							break; 
					} 
					break; 
				} 
		}

		switch(LOWORD(wParam))
		{
			case IDC_MAINTREE:
				{

			
				  if(pnmtv->hdr.code == TVN_BEGINLABELEDIT)
				  {
					hEdit=TreeView_GetEditControl(g_hwndMainTreeCtrl);
				  }

				  if(pnmtv->hdr.code== TVN_ENDLABELEDIT)
				  {
					char Text[256]="";
					tvi.hItem=iSelected;
					SendDlgItemMessage(hWnd,IDC_MAINTREE,TVM_GETITEM,0,
									  (WPARAM)&tvi);
					GetWindowText(hEdit, Text, sizeof(Text));
					tvi.pszText=Text;
					SendDlgItemMessage(hWnd,IDC_MAINTREE,TVM_SETITEM,0,
									  (WPARAM)&tvi);
				  }
					
				    if(pnmtv->hdr.code== TVN_ITEMEXPANDED)
					{
						UINT action = pnmtv->action;

						int i = TreeView_GetIndexByHItemTree(pnmtv->itemNew.hItem);						
						if(i!=-1)
						{
							if(action==TVE_EXPAND)
								vTI.at(i).bExpanded = true;
							else if(action==TVE_COLLAPSE)
							{
								vTI.at(i).bExpanded = false;
								TreeView_SetAllChildItemExpand(i, false);
							}
							
						//	AddLogInfo(ETSV_DEBUG,"Expanded %s Action %d",vTI.at(i).sName.c_str(),action);
						}
					}

					if(pnmtv->hdr.code == TVN_SELCHANGED)
					{
						if(pnmtv->action != TVC_BYMOUSE )
							return FALSE;
						int i =  TreeView_GetSelectionV3();
						switch(i)
						{
							case DO_NOTHING:
								return TRUE;
								break;
							case SHOW_FAVORITES_PUBLIC:
								GI[g_currentGameIdx].dwViewFlags = REDRAWLIST_FAVORITES_PUBLIC;
								break;								
							case SHOW_FAVORITES_PRIVATE:
								GI[g_currentGameIdx].dwViewFlags = REDRAWLIST_FAVORITES_PRIVATE;
								break;	
							case SHOW_HISTORY:
								GI[g_currentGameIdx].dwViewFlags = REDRAWLIST_HISTORY;
								break;
							case REDRAW_CURRENT_LIST : 
								GI[g_currentGameIdx].dwViewFlags = 0;
								break;						
							default:  //a game is selected
								{
									GI[g_currentGameIdx].dwViewFlags = 0;
									if(GI[g_currentGameIdx].pSC->vSI.size()==0) //if empty let's download server list from master
									{
										OnActivate_ServerList();
										return TRUE;
									}		
								}
								break;

						} //end switch
						RedrawServerListThread(&GI[g_currentGameIdx]);
						return TRUE;
					}					
				}
		
			
		  default:
		{
			if((lpnmia->hdr.code == NM_SETFOCUS) && (lpnmia->hdr.hwndFrom == g_hwndListViewServer))
				{
					if (!RegisterHotKey(g_hwndListViewServer, HOTKEY_ID_CTRL_C, MOD_CONTROL, 0x43))
						AddLogInfo(ETSV_WARNING,"Couldn't register CTRL+V hotkey.");
					else
						AddLogInfo(ETSV_INFO, "Register CTRL+V hotkey.");
			
				}
				else if((lpnmia->hdr.code == NM_KILLFOCUS) && (lpnmia->hdr.hwndFrom == g_hwndListViewServer))
				{
					//Unregister copy short key
					dbg_print("Unregister CTRL+V hotkey.");
					UnregisterHotKey(g_hwndListViewServer,HOTKEY_ID_CTRL_C);
				}
				else if((lpnmia->hdr.code == NM_DBLCLK) && (lpnmia->hdr.hwndFrom == g_hwndListViewServer))
				{
					OnServerDoubleClick();
					return TRUE;
					
				}
				else if((lpnmia->hdr.code == NM_CLICK) && (lpnmia->hdr.hwndFrom == g_hwndListViewServer))
				{
					OnServerSelected(currCV);
					return TRUE;

				} 
				else if((lpnmia->hdr.code == NM_CLICK) && (lpnmia->hdr.hwndFrom == g_hwndListBuddy))
				{
					OnBuddySelected();
					return TRUE;					
				}
				else if((lpnmia->hdr.code == NM_RETURN) && (lpnmia->hdr.hwndFrom == g_hwndListViewServer))
				{
					OnServerSelected(currCV);					
					
				}
				else  if(lpnmia->hdr.code == NM_CUSTOMDRAW && (lpnmia->hdr.hwndFrom != g_hwndMainTreeCtrl))
				{					
					return ListView_CustomDraw(lParam);					
				}
				else  if(lpnmia->hdr.code == NM_CUSTOMDRAW && (lpnmia->hdr.hwndFrom == g_hwndMainTreeCtrl))
				{					
					return TreeView_CustomDraw(lParam);					
				}
				else  if(lpnmia->hdr.code == LVN_COLUMNCLICK && lpnmia->hdr.hwndFrom == g_hwndListViewServer)
				{		


					NMLISTVIEW* lstvw;
					lstvw = (NMLISTVIEW*) lParam; 
					for(int i=0;i<MAX_COLUMNS;i++)
					{
						if(CUSTCOLUMNS[i].columnIdx == lstvw->iSubItem)
						{						
							CUSTCOLUMNS[i].bSortAsc = ! CUSTCOLUMNS[i].bSortAsc;
							break;
						}
					}
					//bSortColumnAscading[lstvw->iSubItem] = ! bSortColumnAscading[lstvw->iSubItem];
					Do_ServerListSort(lstvw->iSubItem);


					return TRUE;
				}else if(lpnmia->hdr.code == LVN_COLUMNCLICK && lpnmia->hdr.hwndFrom == g_hwndListBuddy)
				{
					NMLISTVIEW* lstvw;
					lstvw = (NMLISTVIEW*) lParam; 
				
					AppCFG.bSortBuddyAsc = !AppCFG.bSortBuddyAsc;
					return TRUE;
				}else if(lpnmia->hdr.code == LVN_COLUMNCLICK && lpnmia->hdr.hwndFrom == g_hwndListViewPlayers)
				{

					NMLISTVIEW* lstvw;
					lstvw = (NMLISTVIEW*) lParam; 					
					
					bPlayerNameAsc= !bPlayerNameAsc;
					bPlayerClanAsc= !bPlayerClanAsc;
					bRateAsc = !bRateAsc;
					bPlayerPingAsc= !bPlayerPingAsc;
					ListView_SortItems(g_hwndListViewPlayers,MyComparePlayerFunc,lstvw->iSubItem);						
					return TRUE;
				}
			

			break;
		  }

		 }
	return FALSE;
}



int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPTSTR    lpCmdLine,int nCmdShow)
{
	
	MSG msg;
	HACCEL hAccelTable;

	InitCommonControls(); 

  INITCOMMONCONTROLSEX icex;
 
// Ensure that the common control DLL is loaded. 
   icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
   icex.dwICC  = ICC_LINK_CLASS | ICC_TREEVIEW_CLASSES;
   InitCommonControlsEx(&icex);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_GAMESCANNER, szWindowClass, MAX_LOADSTRING);


	hOfflineIcon = LoadIcon(hInstance,(LPCTSTR)MAKEINTRESOURCE(IDI_GAMESCANNER)); //ICON_APP_LOGO)); 
	hOnlineIcon = LoadIcon(hInstance,(LPCTSTR)MAKEINTRESOURCE(IDI_ICON_TASKTRAY)); 


	//GetCurrentDirectory(512,EXE_PATH);

 	GetModuleFileName ( NULL, EXE_PATH, MAX_PATH );
	char *p = strrchr(EXE_PATH,'\\');
	if(p!=NULL)
		p[0]=0;

	SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,SHGFP_TYPE_CURRENT,USER_SAVE_PATH);
	
	SHGetFolderPath(NULL,CSIDL_COMMON_APPDATA,NULL,SHGFP_TYPE_CURRENT,COMMON_SAVE_PATH);
	 
	strcat(USER_SAVE_PATH,"\\GameScanner\\");
	strcat(COMMON_SAVE_PATH,"\\GameScanner\\");
	
	wchar_t wcPath[MAX_PATH];

	ZeroMemory(wcPath,sizeof(wcPath));
	mbstowcs(wcPath,(const char *)USER_SAVE_PATH,strlen(USER_SAVE_PATH));	
	SHCreateDirectory(NULL,wcPath);

	ZeroMemory(wcPath,sizeof(wcPath));
	mbstowcs(wcPath,(const char *)COMMON_SAVE_PATH,strlen(COMMON_SAVE_PATH));	
	SHCreateDirectory(NULL,wcPath);


	//ShGetKnownFolderPath 
	SetLogPath(USER_SAVE_PATH);
	SetCurrentDirectory(USER_SAVE_PATH);

	//	bool installed = IsInstalled(ETSV_VERSION);

	remove("Log_previous_start.htm");
	rename("Log.htm","Log_previous_start.htm");
	AddLogInfo(ETSV_INFO,"Initilizing Game Scanner...");
	AddLogInfo(ETSV_INFO,"Version %s",APP_VERSION);
	AddLogInfo(ETSV_INFO,"Executable directory: %s",EXE_PATH);
	AddLogInfo(ETSV_INFO,"User Data directory: %s",USER_SAVE_PATH);	
	AddLogInfo(ETSV_INFO,"Common Data directory: %s",COMMON_SAVE_PATH);	
	AddLogInfo(ETSV_INFO,"Cmd line input %s",lpCmdLine);

	IPC_SetPath(EXE_PATH);

	//Do the conversion of the IP to country database 
//#ifdef CONVERTIPDATABASE
	SetCurrentDirectory(EXE_PATH);
	
	if(fnConvertDatabase()==0)
		AddLogInfo(ETSV_INFO,"Updated IP to Country file.");
	SetCurrentDirectory(USER_SAVE_PATH);
//#endif
	
	fnIPtoCountryInit();
	
	memset((void*)&etMode,0,sizeof(DEVMODE));

	CFG_Load();
		
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR)
	{
	  AddLogInfo(ETSV_INFO,"Error at WSAStartup()");
	}

	if(AppCFG.bAutostart)  //auto run up on boot
		SelfInstall(EXE_PATH);
	else
		UnInstall();

	char *pszCmd=NULL;
	pszCmd = strchr(lpCmdLine,'/');
	if(pszCmd!=NULL)
	{
		if(strcmp(pszCmd,"/tasktray")==0)
			g_bNormalWindowed = false;
		 else
			g_bNormalWindowed = true;
	}

	SetCurrentDirectory(USER_SAVE_PATH);

	if(strlen(lpCmdLine)>0)
	{

		char *p,*q;
		char proto[256];
		strcpy(proto,lpCmdLine);
		p = strchr(lpCmdLine,'/');
		if(p!=NULL)
		{

			p+=2;
			q = strchr(p,'/');
			if(q != NULL)
				q[0]=0;
			SERVER_INFO srv;
			ZeroMemory(&srv,sizeof(SERVER_INFO));
			DWORD dwPort;			
			strcpy(srv.szIPaddress,SplitIPandPORT(p,dwPort));
			srv.dwPort = dwPort;
			
			char *cutoff;
			cutoff = strchr(proto,':');
			if(cutoff!=NULL)
			{
			//	Setup_Appsettings();
				cutoff[0]=0;
				for(char i=0;i<MAX_SERVERLIST; i++)
				{
					if(strcmp(proto,GI[i].szProtocolName)==0)
					{
						srv.cGAMEINDEX = i;	
						srv.dwIndex = 999999;  //work around to skip sync
						if(srv.dwPort ==0)
							srv.dwPort = GI[i].dwDefaultPort;

						LaunchGame(srv,&GI[i]);
					}
				}
				return 0;
			}

		}
	}
	
	//close multiple windows
	if(FindWindow(szWindowClass,szDialogTitle )!=NULL)
	{

		MessageBox(NULL,"Please exit current W:ET Server Viewer\nbefore running the new version!","Alert",MB_OK);
		return 0;
	}

	hCloseEvent = CreateEvent( NULL, TRUE, TRUE,"CloseEvent"); 

    if (hCloseEvent == NULL) 
    { 
        dbg_print("CreateEvent failed\n");   
    }

	userMode = GetScreenResolution();

	MyRegisterClass(hInstance);
	AddLogInfo(ETSV_INFO,"Registered class");

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		AddLogInfo(ETSV_INFO,"Error InitInstance");
		return FALSE;
	}
	AddLogInfo(ETSV_INFO,"InitInstance...");

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_ETSERVERVIEWER);
	//register hot key
	if(AppCFG.bUse_minimize)
	{		
		if (!RegisterHotKey(NULL, HOTKEY_ID, AppCFG.dwMinimizeMODKey , AppCFG.cMinimizeKey))
		{
			MessageBox(0, "Couldn't register hotkey!\nProbably due to another application is running as a minimzer.\nETSV will disable minimze function & continue.\nYou can enable it from the settings if neccessary.", NULL, NULL);
			AppCFG.bUse_minimize = false;
		}
		else
		{
			AddLogInfo(ETSV_INFO,"Registring hotkey success. (%4.4X  %C)",AppCFG.dwMinimizeMODKey ,AppCFG.cMinimizeKey);
		}

	}

	
	NOTIFYICONDATA structNID;
	structNID.cbSize = sizeof(NOTIFYICONDATA); 
	structNID.hWnd = (HWND) g_hWnd; 
	structNID.uID = 100123; //ID of the icon that willl appear in the system tray 
	structNID.uFlags =  NIF_ICON | NIF_MESSAGE | NIF_TIP; 
	strcpy(structNID.szTip,"Game Scanner");
	structNID.hIcon = hOnlineIcon; 
	structNID.uCallbackMessage = WM_USER_SHELLICON;

	int try_count=1;
tryagain:
	//put the actual icon now
	if(Shell_NotifyIcon(NIM_ADD, &structNID)==FALSE)
	{
		Sleep(6000*try_count);
		try_count++;
		if(try_count>3)
		{
			AddLogInfo(ETSV_INFO,"Error adding task tray icon. Please report this back to the developer.");
		}
		goto tryagain;
	}

	 
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{

			if (msg.message == WM_HOTKEY)
			{
			
				if (msg.wParam != HOTKEY_ID_CTRL_C)
					msg.hwnd = g_hWnd;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}
	}


	if(AppCFG.bUse_minimize)
		UnregisterHotKey(NULL, HOTKEY_ID);
	
	Shell_NotifyIcon(NIM_DELETE,&structNID);
	CloseHandle(hCloseEvent);
	DestroyIcon(hOfflineIcon);
	DestroyIcon(hOnlineIcon);
	WSACleanup();

	DestroyAcceleratorTable(hAccelTable); 
	AddLogInfo(ETSV_INFO,"Exit app..");
	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			=  CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0; 
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_NODISKVIEWER);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);

	wcex.hbrBackground	= (HBRUSH) GetSysColorBrush(COLOR_3DFACE);//hBrush;

	wcex.lpszMenuName	= (LPCTSTR)IDC_GAMESCANNER;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_ICON1);

	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK PROGRESS_Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	g_PROGRESS_DLG  = hDlg;
	switch (message)
	{
		case WM_INITDIALOG:
			{
				g_DlgProgress= NULL;
				g_DlgProgressMsg=NULL;
				g_DlgProgressMsg = GetDlgItem(hDlg,IDC_STATIC_TEXT);
				g_DlgProgress = GetDlgItem(hDlg,IDC_PROGRESS1);
				SendMessage(g_DlgProgress, PBM_SETRANGE, (WPARAM) 0,MAKELPARAM(0, 50)); 
				SendMessage(g_DlgProgress, PBM_SETSTEP, (WPARAM) 1, 0); 
				SendMessage(g_DlgProgress, PBM_SETPOS, (WPARAM) 0, 0); 
				SendMessage(g_DlgProgress, PBM_STEPIT, (WPARAM) 0, 0); 
				CenterWindow(hDlg);
	
				return TRUE;
			}
			break;
		case WM_CLOSE:
			
			g_DlgProgress= NULL;
			g_DlgProgressMsg=NULL;
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
			break;
	}
	return FALSE;

}

void LaunchGame(SERVER_INFO pSI,GAME_INFO *pGI)
{
	char CommandParameters[512];	
	int typeRecognized = -1;

	//simple autodetect to try to recognize which game to launch (used for quick launch)
	for(int i=0; i<MAX_SERVERLIST;i++)
	{
		
		if((GI[i].dwDefaultPort==pSI.dwPort) && GI[i].bActive)
		{
			typeRecognized=GI[i].cGAMEINDEX;
			break;
		}
	}
	
	if(typeRecognized==-1)
		GetServerInfo(g_currentGameIdx,&pSI);
	else
	{
		GetServerInfo(typeRecognized,&pSI);

		dbg_print("Game detected: ");
		dbg_print(GI[typeRecognized].szGAME_NAME);
		dbg_print("\n");
	}
	if(pSI.bPrivate==1)
	{
		g_PRIVPASSsrv = &pSI;
		DialogBox(g_hInst, (LPCTSTR)IDD_DLG_SETPRIVPASS, g_hWnd, (DLGPROC)PRIVPASS_Proc);		
	}


	pSI.cHistory++;
	
	if(pSI.dwIndex!=999999)
		Sync_ServerInfo(pSI);

	if(strlen(AppCFG.szEXT_EXE_PATH)>0)
	{
		HWND hHwndEXT=NULL;
		hHwndEXT = FindWindow(NULL,AppCFG.szEXT_EXE_WINDOWNAME); 
		//we don't want to launch the external app twice..
		if (hHwndEXT==NULL && AppCFG.bUse_EXT_APP)
		{
			ShellExecute(GetDesktopWindow(), "open", AppCFG.szEXT_EXE_PATH, AppCFG.szEXT_EXE_CMD,NULL, 1);
			Sleep(500);
		}
	}
	
		sprintf(CommandParameters,"+connect %s:%d +password %s %s",pSI.szIPaddress,pSI.dwPort,pSI.szPRIVATEPASS,pGI->szGAME_CMD);					
	
		if(ExecuteGame(pGI,CommandParameters))
		{
			//A Successfull launch
			PostMessage(g_hWnd,WM_CLOSE,0xdead,0);  //Minimize ETSV
			
			if(AppCFG.bUseMIRC)
			{
				//Notify mIRC which server user will join
				DDE_Init();
	  			char szMsg[350];
				wsprintf(szMsg,"/ame joing server %s (%s IP %s:%d %s)",pSI.szServerName,pGI->szProtocolName,pSI.szIPaddress,pSI.dwPort,pSI.bPrivate?"Private":"");
				DDE_Send(szMsg);
				DDE_DeInit();
			}
		}
				
}


// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
		case WM_INITDIALOG:
		{	
			return TRUE;

		}
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
		}

		case WM_NOTIFY:
		{
			NMHDR *pNMHdr;
			pNMHdr = (LPNMHDR) lParam;

			switch (pNMHdr->code)
			{
				case NM_RETURN:
				case NM_CLICK:
				{
					PNMLINK pNMLink = (PNMLINK) lParam;
				
					if (!wcscmp (pNMLink->item.szID,L"idHomePage"))
					{
						ShellExecute(NULL,NULL,"http://www.bdamage.se/",NULL,NULL,SW_SHOWNORMAL);
					}
					else if (!wcscmp (pNMLink->item.szID,L"idHelpMenu"))
					{
						// TODO: Launch Help Menu...
					}
					break;
				}
			}
		}
		break;


	}
	return FALSE;
}


DWORD WINAPI ProgressGUI_Thread(LPVOID lpParam)
{
	DialogBox(g_hInst, (LPCTSTR)IDD_DLG_PROGRESS, NULL, (DLGPROC)PROGRESS_Proc);					
	return 0;
}

DWORD dwThreadIdProgress;

DWORD WINAPI CheckForUpdates(LPVOID lpParam)
{
	BOOL bAnyUpdates=FALSE;
	SetCurrentDirectory(USER_SAVE_PATH);
	int ret = 0;

	if(IsServerAlive("www.cludden.se")==false)
	{
		AddLogInfo(0,"< Update Server is down!\n >");
		return ret;
	}
	OSVERSIONINFO OSversion;
		
	OSversion.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	::GetVersionEx(&OSversion);
	AddLogInfo(0,"OS Version: %d.%d",OSversion.dwMajorVersion,OSversion.dwMinorVersion);

	remove("update.xml");

	char szBuff[512];
	sprintf(szBuff,"http://www.cludden.se/braindamage/download/checkversion.php?version=%s&osversion=%d.%d",APP_VERSION,OSversion.dwMajorVersion,OSversion.dwMinorVersion);

	HFD_SetPath(USER_SAVE_PATH);

	HttpFileDownload(szBuff,"update.xml",g_DlgProgress,g_DlgProgressMsg);

	TiXmlDocument doc("update.xml");
	if (!doc.LoadFile()) 
		return 1;


	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);
	pElem=hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem) 
		return 1;
	const char *szP;
	szP = pElem->Value(); //= ETSVcfg

	// save this for later
	hRoot=TiXmlHandle(pElem);

	TiXmlElement* pElement;

	char szVersion[50];

	pElement=hRoot.FirstChild("Version").ToElement();
	pElement->FirstChild()->ToElement();

	if(pElement!=NULL)
	{
		ReadCfgStr2(pElement , "Version",szVersion,sizeof(szVersion));
		if(strcmp(szVersion,APP_VERSION)>0)
		{
			Show_ToolbarButton(IDC_DOWNLOAD, true);
			//EnableDownloadLink(TRUE);
			SetStatusText(ICO_INFO,"*** New version detected! ***");
			AddLogInfo(ICO_INFO,"*** New version detected! ***");
			bAnyUpdates=TRUE;
		} else
		{
			AddLogInfo(ICO_INFO,"No new version detected!");
			SetStatusText(ICO_INFO,"No new version detected!");
		}

	}
	
	bDoFirstTimeCheckForUpdate=false;
	return 0;
}

DWORD WINAPI AutomaticDownloadUpdateSetUp(LPVOID lpParam)
{
	SetCurrentDirectory(USER_SAVE_PATH);
 
	hashwrapper *myWrapper = NULL;

	TiXmlDocument doc("update.xml");
	if (!doc.LoadFile()) 
		goto failed;

	HANDLE hThread;
	hThread = CreateThread( NULL, 0, &ProgressGUI_Thread, g_hWnd,0, NULL);                
	if (hThread == NULL) 
	{
		AddLogInfo(ETSV_WARNING, "CreateThread failed  <AutomaticDownloadUpdateSetUp> (%d) File:(%s) Line:(%d)\n", GetLastError(),__FILE__,__LINE__ ); 
	}
	else 
	{
		dbg_print("CreateThread  success AutomaticDownloadUpdateSetUp");
		CloseHandle( hThread );
	}
	Sleep(500);

	HFD_SetPath(USER_SAVE_PATH);

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(NULL);

	pElem=hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem) 
		goto failed;

	const char *szP;
	szP = pElem->Value(); //= ETSVcfg

	// save this for later
	hRoot=TiXmlHandle(pElem);
	//Default values

	TiXmlElement* pElement;
	TiXmlElement* pChild;
	char szBuff[512];
	int iRet;
	char szFileName[256];
	char szMD5[256];
	char szDestPath[50];
	int iExec=0;

	 myWrapper = new md5wrapper();

	pElement=hRoot.FirstChild("File").ToElement();
	if(pElement!=NULL)
	{
		pElement->FirstChild()->ToElement();
		if(pElement!=NULL)
		{
			
			for(int i=0;i<10;i++)
			{
	
				pChild = pElement->FirstChild()->ToElement();	
				if(pChild==NULL)
					break;

				ReadCfgStr2(pChild , "FileName",szFileName,sizeof(szFileName));
				ReadCfgStr2(pChild, "Dest",szDestPath,sizeof(szDestPath));
				ReadCfgStr2(pChild, "MD5",szMD5,sizeof(szMD5));
				ReadCfgInt2(pChild, "exec",(int&)iExec);					
				remove(szFileName);
				sprintf_s(szBuff,sizeof(szBuff),"http://www.cludden.se/braindamage/download/%s",szFileName);
				SetStatusText(ICO_INFO,"Downloading update!");
				iRet = HttpFileDownload(szBuff,szFileName,g_DlgProgress,g_DlgProgressMsg);

				 try
				 {
					SetCurrentDirectory(USER_SAVE_PATH);
					std::string hash2 = myWrapper->getHashFromFile(szFileName);
					AddLogInfo(0,"Comparing hashing %s = %s", hash2.c_str(),szMD5);	
					if(_stricmp(hash2.c_str(),szMD5)!=0)
					{
						AddLogInfo(0,"Mismatch hashing %s = %s", hash2.c_str(),szMD5);	
						goto failed;
					}
					else
					{
						if(iExec)
						{
							AddLogInfo(0,"Executing %s", szFileName);	
							ShellExecute(g_hWnd, "open",szFileName, NULL, USER_SAVE_PATH,SW_NORMAL);	
							PostMessage(g_hWnd,WM_CLOSE,0,0);
						}

					}

				 }
				 catch(hlException &e)
				 {					 
					 AddLogInfo(0,"Error hashing file: %d %s", e.error_number(), e.erro_message().c_str());									   
				 }

				pElement = pElement->NextSiblingElement();
				if(pElement==NULL)
					break;
			}// 	for(int i...
		}
	}

	PostMessage(g_PROGRESS_DLG,WM_CLOSE,0,0);
	delete myWrapper;
	return 0;


failed:
	if(myWrapper!=NULL)
		delete myWrapper;

	PostMessage(g_PROGRESS_DLG,WM_CLOSE,0,0);
	int ret = MessageBox(NULL,"New ET Server Viewer version has been detected!\nError downloading file(s).\nTake me to the website and download manually!","Update error",MB_YESNO);				
	if(ret==IDYES)
	{
		ShellExecute(NULL,NULL,"http://www.bdamage.se/",NULL,NULL,SW_SHOWNORMAL);

	}		
	return 2;
}

void ErrorExit(LPTSTR lpszFunction) 
{ 
    TCHAR szBuf[80]; 
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    wsprintf(szBuf, 
        "%s failed with error %d: %s", 
        lpszFunction, dw, lpMsgBuf); 
 
    MessageBox(NULL, szBuf, "Error", MB_OK); 

    LocalFree(lpMsgBuf);
  
}
DWORD *dwMaxMin;
BOOL bSettingMax=FALSE;
LRESULT CALLBACK MINMAX_Dlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndEdit;
	char szTemp[50];
	switch (message)
	{
	case WM_INITDIALOG:
		{
		CenterWindow(hDlg);
		hwndEdit = GetDlgItem(hDlg,IDC_EDIT_MINMAX);

		if(lParam==0)
		{

			_itoa(AppCFG.filter.dwShowServerWithMinPlayers,szTemp,10);	
			dwMaxMin = &AppCFG.filter.dwShowServerWithMinPlayers;
		
			SetWindowText(hDlg,"Set minimum value");
			bSettingMax=FALSE;
		}
		else
		{
			dwMaxMin = &AppCFG.filter.dwShowServerWithMaxPlayers;
			_itoa(AppCFG.filter.dwShowServerWithMaxPlayers,szTemp,10);	
			SetWindowText(hDlg,"Set maximum value");
			bSettingMax=TRUE;
		}

		

		SetDlgItemText(hDlg,IDC_EDIT_MINMAX,szTemp);
		
		SetFocus(GetDlgItem(hDlg,IDC_EDIT_MINMAX));
		
	
		PostMessage(GetDlgItem(hDlg,IDC_EDIT_MINMAX),EM_SETSEL,0,strlen(szTemp));
		PostMessage(GetDlgItem(hDlg,IDC_EDIT_MINMAX),EM_SETSEL,(WPARAM)-1,-1);
		
		

		//return TRUE;
		}
		break;
	case WM_COMMAND:

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			if(LOWORD(wParam) == IDOK)
			{
							
				GetDlgItemText(hDlg,IDC_EDIT_MINMAX,szTemp,sizeof(szTemp)-1);
				*dwMaxMin= atoi(szTemp);
				if(bSettingMax)
					TreeView_SetDWValueByItemType(FILTER_MAX_PLY,*dwMaxMin);
				else
					TreeView_SetDWValueByItemType(FILTER_MIN_PLY,*dwMaxMin);

			}

			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}



void OnMinMaxPlayers(HWND hWndParent,bool bMinValueToSet)
{
	int ret =(int) DialogBoxParam(g_hInst, (LPCTSTR)IDD_DLG_MAXMIN_PLY, hWndParent, (DLGPROC)MINMAX_Dlg,bMinValueToSet);					

	if(ret == IDOK)
	{
//		TreeView_BuildList();
	}

}
HWND TOOLBAR_CreateOptionsToolBar(HWND hWndParent)
{
    RECT rcClient;  // dimensions of client area 
    HWND hwndTB;    

    GetClientRect(hWndParent, &rcClient); 
    hwndTB = CreateWindowEx(0,
                            TOOLBARCLASSNAME ,
                            NULL,
                            WS_VISIBLE |WS_CLIPSIBLINGS | WS_CHILD |  
							TBSTYLE_TOOLTIPS | TBSTYLE_FLAT |
							CCS_NODIVIDER | CCS_NORESIZE, 
                            0,  0, 26*4, 0,
                            hWndParent, 
                            NULL, 
                            g_hInst, 
                            NULL); 
	
	
		if(hwndTB==NULL)
		{
				AddGetLastErrorIntoLog("TOOLBAR_CreateOptionsToolBar");
			return NULL;
		}
			

		g_hwndToolbarOptions = hwndTB;

		::SendMessage(hwndTB, TB_SETPARENT, (WPARAM)(HWND) hWndParent, 0);

		// We must send this message before sending the TB_ADDBITMAP or TB_ADDBUTTONS message
		::SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

		// allows buttons to have a separate dropdown arrow
		// Note: TBN_DROPDOWN notification is sent by a toolbar control when the user clicks a dropdown button
	//	::SendMessage(hwndTB, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);
	
		::SendMessage(hwndTB, TB_SETBITMAPSIZE, 0, MAKELONG(24, 24));

		int iNumButtons = 8;
		COLORREF crMask = RGB(255,0,255);

		HBITMAP hbm = ::LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_TOOLBAR_N));
		BITMAP bm = {0};

		::GetObject(hbm, sizeof(BITMAP), &bm);
		int iImageWidth  = 24; //bm.bmWidth / iNumButtons;
		int iImageHeight = bm.bmHeight;

		m_hImageList = ImageList_Create(iImageWidth, iImageHeight, ILC_COLOR24 | ILC_MASK, iNumButtons, 0);
		ImageList_AddMasked(m_hImageList, hbm, crMask);
		::DeleteObject(hbm);

		SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)m_hImageList);
					
		hbm = ::LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_TOOLBAR_H));		
		m_hImageListHot = ImageList_Create(iImageWidth, iImageHeight, ILC_COLOR24 | ILC_MASK, iNumButtons, 0);		
		ImageList_AddMasked(m_hImageListHot, hbm, crMask);
		::DeleteObject(hbm);
		SendMessage(hwndTB, TB_SETHOTIMAGELIST, 0, (LPARAM)m_hImageListHot);
						

		m_hImageListDis = ImageList_Create(iImageWidth, iImageHeight, ILC_COLOR24 | ILC_MASK, iNumButtons, 0);
		hbm = ::LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_TOOLBAR_D));
		ImageList_AddMasked(m_hImageListDis, hbm, crMask);
		::DeleteObject(hbm);
		SendMessage(hwndTB, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)m_hImageListDis);
	
		int iImages = 0;
		TBBUTTON tbb;

	   	ZeroMemory(&tbb, sizeof(TBBUTTON));

		tbb.iBitmap = iImages++;
		tbb.idCommand = IDM_REFRESH;
		tbb.fsState = TBSTATE_ENABLED;
		tbb.fsStyle = TBSTYLE_BUTTON ; //| TBSTYLE_DROPDOWN;
		//		tbb.fsStyle = TBSTYLE_SEP;
		::SendMessage(hwndTB, TB_ADDBUTTONS, 1, (LPARAM)&tbb);

		tbb.iBitmap = 7;
		tbb.idCommand = IDM_SCAN_FILTERED;
		tbb.fsState = TBSTATE_ENABLED;
		tbb.fsStyle = TBSTYLE_BUTTON ;
		//		tbb.fsStyle = TBSTYLE_SEP;
		::SendMessage(hwndTB, TB_ADDBUTTONS, 1, (LPARAM)&tbb);


		tbb.iBitmap = iImages++;
		tbb.idCommand = IDM_SETTINGS;
		tbb.fsState = TBSTATE_ENABLED;
		tbb.fsStyle = TBSTYLE_BUTTON;
		::SendMessage(hwndTB, TB_ADDBUTTONS, 1, (LPARAM)&tbb);
	
    return hwndTB;
}

HWND WINAPI TOOLBAR_CreateSearchComboBox(HWND hwndParent)
{
	HWND hwndCB;
	DWORD dwBaseUnits =  GetDialogBaseUnits(); 

	  //add a band that contains a combobox
   hwndCB = CreateWindowEx(   0, 
            TEXT("Combobox"), 
            NULL,
            WS_VISIBLE |  WS_CHILD | WS_TABSTOP |WS_VSCROLL |
            WS_CLIPCHILDREN | WS_CLIPSIBLINGS | 
            CBS_AUTOHSCROLL | CBS_DROPDOWN ,
            (2 * LOWORD(dwBaseUnits)) / 4, 
            (2 * HIWORD(dwBaseUnits)) / 8, 
            (120 * LOWORD(dwBaseUnits)) / 4, 
            (5 * HIWORD(dwBaseUnits)) / 8, 
            hwndParent, 
            (HMENU)IDC_COMBOBOXEX_CMD, 
            g_hInst, 
            NULL);

	if(hwndCB!=NULL)
	{		
		g_hwndComboEdit = FindWindowEx( hwndCB, NULL, "Edit", NULL );  
		g_wpOrigCBSearchProc = (LONG_PTR) SetWindowLongPtr(g_hwndComboEdit, GWLP_WNDPROC, (LONG_PTR) ComboBox_SearchSubclassProc); 
		InvalidateRect(g_hwndComboEdit,NULL,TRUE);

	} else
	{
		AddGetLastErrorIntoLog("TOOLBAR_CreateSearchComboBox");
	}
   return hwndCB;
}


HWND TOOLBAR_CreateSearchToolBar(HWND hWndParent)
{
    RECT rcClient; 
    HWND hwndTB;   

    GetClientRect(hWndParent, &rcClient); 
    hwndTB = CreateWindowEx(0,
                            TOOLBARCLASSNAME ,
                            NULL,
                            WS_VISIBLE |WS_CLIPSIBLINGS | WS_CHILD |
							TBSTYLE_TOOLTIPS | TBSTYLE_FLAT |
							CCS_NODIVIDER | CCS_NORESIZE , 
                            0, 0, 28, 28,
                            hWndParent, 
                            NULL, 
                            g_hInst, 
                            NULL); 


	if(hwndTB==NULL)
		AddLogInfo(0,"TOOLBAR_CreateSearchToolBar create failed!!");

   

		::SendMessage(hwndTB, TB_SETPARENT, (WPARAM)(HWND) hWndParent, 0);

		// We must send this message before sending the TB_ADDBITMAP or TB_ADDBUTTONS message
		::SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

		// allows buttons to have a separate dropdown arrow
		// Note: TBN_DROPDOWN notification is sent by a toolbar control when the user clicks a dropdown button
	//	::SendMessage(hwndTB, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);
	
		::SendMessage(hwndTB, TB_SETBITMAPSIZE, 0, MAKELONG(24, 24));


		SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)m_hImageList);
		SendMessage(hwndTB, TB_SETHOTIMAGELIST, 0, (LPARAM)m_hImageListHot);
		SendMessage(hwndTB, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)m_hImageListDis);

	
		TBBUTTON tbb;

		ZeroMemory(&tbb, sizeof(TBBUTTON));

		tbb.iBitmap = 260;
		tbb.fsStyle = BTNS_SEP;
		tbb.fsState = TBSTATE_ENABLED;
		::SendMessage(hwndTB, TB_ADDBUTTONS, 1, (LPARAM)&tbb);

		tbb.iBitmap = 3;
		tbb.idCommand = IDC_BUTTON_FIND;
		tbb.fsState = TBSTATE_ENABLED;
		tbb.fsStyle = BTNS_BUTTON;
		::SendMessage(hwndTB, TB_ADDBUTTONS, 1, (LPARAM)&tbb);
		tbb.iBitmap = 0;

		tbb.idCommand = 0;
		tbb.fsState = TBSTATE_ENABLED;
		tbb.fsStyle = TBSTYLE_SEP;
		::SendMessage(hwndTB, TB_ADDBUTTONS, 1, (LPARAM)&tbb);

		tbb.iBitmap = 2;
		tbb.idCommand = IDC_BUTTON_ADD_SERVER;
		tbb.fsState = TBSTATE_ENABLED;
		tbb.fsStyle = BTNS_BUTTON;		
		::SendMessage(hwndTB, TB_ADDBUTTONS, 1, (LPARAM)&tbb);

		tbb.iBitmap = 6;
		tbb.idCommand = IDC_BUTTON_QUICK_CONNECT;
		tbb.fsState = TBSTATE_ENABLED;
		tbb.fsStyle = TBSTYLE_BUTTON;		
		::SendMessage(hwndTB, TB_ADDBUTTONS, 1, (LPARAM)&tbb);		 
		
		tbb.iBitmap = 4;
		tbb.idCommand = IDC_DOWNLOAD;
		tbb.fsState = TBSTATE_ENABLED;
		tbb.fsStyle = TBSTYLE_BUTTON;		
		::SendMessage(hwndTB, TB_ADDBUTTONS, 1, (LPARAM)&tbb);	

#ifdef _DEBUG
		tbb.iBitmap = 7;
		tbb.idCommand = IDM_SCAN_ALL_GAMES;
		tbb.fsState = TBSTATE_ENABLED;
		tbb.fsStyle = TBSTYLE_BUTTON;		
		::SendMessage(hwndTB, TB_ADDBUTTONS, 1, (LPARAM)&tbb);	

#endif


		g_hwndSearchToolbar = hwndTB;
		g_hwndSearchCombo = TOOLBAR_CreateSearchComboBox(hwndTB);
	
    return hwndTB;
}


HWND WINAPI TOOLBAR_CreateRebar(HWND hwndOwner)
{
   REBARINFO     rbi;
   REBARBANDINFO rbBand;
   HWND   hwndTB, hwndRB;
   DWORD  dwBtnSize;
   INITCOMMONCONTROLSEX icex;  
   icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
   icex.dwICC   = ICC_COOL_CLASSES|ICC_BAR_CLASSES;
   InitCommonControlsEx(&icex);
 

   hwndRB = CreateWindowEx(WS_EX_TOOLWINDOW,
                           REBARCLASSNAME,
                           NULL,
                           WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|
                           WS_CLIPCHILDREN|RBS_VARHEIGHT| 
                            CCS_NODIVIDER ,
                           0,0,200,24,
                           hwndOwner,
                           NULL,
                           g_hInst,
                           NULL);
   
	if(!hwndRB)
	{		
		AddLogInfo(ETSV_WARNING, "Create Rebar failed  <TOOLBAR_CreateRebar>"); 
		return NULL;
	}


	// Initialize and send the REBARINFO structure.
	ZeroMemory(&rbi,sizeof(REBARINFO));
	rbi.cbSize = sizeof(REBARINFO);  // Required when using this structure.
	rbi.fMask  = 0;
	rbi.himl   = (HIMAGELIST)NULL;
	if(!SendMessage(hwndRB, RB_SETBARINFO, 0, (LPARAM)&rbi))
	{
		AddGetLastErrorIntoLog("Failed: SendMessage(hwndRB, RB_SETBARINFO");
	  return NULL;
	}
	AddGetLastErrorIntoLog("SendMessage(hwndRB, RB_SETBARINFO");
	// Initialize structure members that both bands will share.

	ZeroMemory(&rbBand,sizeof(REBARBANDINFO));
	rbBand.cbSize = sizeof(REBARBANDINFO);  // Required
	rbBand.fMask  =  RBIM_IMAGELIST | 
				   RBBIM_STYLE | RBBIM_CHILD  | RBBIM_CHILDSIZE |RBBIM_SIZE;

	rbBand.fStyle = RBBS_CHILDEDGE  |  RBBS_NOGRIPPER  ;

	hwndTB = TOOLBAR_CreateOptionsToolBar(hwndRB);
	if(hwndTB==NULL)
		AddGetLastErrorIntoLog("Failed: TOOLBAR_CreateOptionsToolBar");

	 // Get the height of the toolbar.
	dwBtnSize = SendMessage(hwndTB, TB_GETBUTTONSIZE, 0,0);
	
	// Set values unique to the band with the toolbar.
	rbBand.lpText     = "Options";  //Not used
	rbBand.hwndChild  = hwndTB;
	rbBand.cxMinChild = LOWORD(dwBtnSize)*3; //24*4;
	rbBand.cyMinChild = HIWORD(dwBtnSize);
	rbBand.cx         = LOWORD(dwBtnSize)*3;
	rbBand.cyChild	  = HIWORD(dwBtnSize);
	// Add the band that has the toolbar.
	SendMessage(hwndRB, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);


	ZeroMemory(&rbBand,sizeof(REBARBANDINFO));
	rbBand.cbSize = sizeof(REBARBANDINFO);  // Required

	rbBand.fMask  = RBBIM_TEXT | RBIM_IMAGELIST |  
				    RBBIM_STYLE | RBBIM_CHILD  | RBBIM_CHILDSIZE | RBBIM_SIZE;
	rbBand.fStyle = RBBS_NOGRIPPER;
	
	HWND hwndCSTB = TOOLBAR_CreateSearchToolBar(hwndRB);

	rbBand.lpText     = "Search";
	rbBand.hwndChild  = hwndCSTB;
	rbBand.cxMinChild = 315;   
	rbBand.cxIdeal    = 415;
	rbBand.cx         = 435;
   
	// Add the band that has the combo box.
	SendMessage(hwndRB, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);

   return (hwndRB);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   g_hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   g_hWnd = hWnd;
 
   if(g_bNormalWindowed)
   {
	ShowWindow(hWnd, nCmdShow);
   }
   UpdateWindow(hWnd);

   return TRUE;
}



int CFG_Load()
{
	memset(&AppCFG,0,sizeof(APP_SETTINGS_NEW));

	Default_Appsettings();
	Default_GameSettings();
	ListView_SetDefaultColumns();

	SetCurrentDirectory(USER_SAVE_PATH);
	TiXmlDocument doc("config.xml");
	if (!doc.LoadFile()) 
	{
		return 1;
	}

	AppCFG.bUseFilterOnPing = FALSE;
	AppCFG.bUseFilterOnFavorites = FALSE;
	AppCFG.bUseShortCountry = FALSE;	
	AppCFG.filter.bPunkbuster = FALSE;
	AppCFG.filter.bNoEmpty = FALSE;
	AppCFG.filter.bNoFull = FALSE;
	AppCFG.filter.bNoPrivate = FALSE;
	AppCFG.filter.bPure = FALSE;
	AppCFG.filter.bRanked = FALSE;
	AppCFG.filter.dwShowServerWithMaxPlayers = 0;
	AppCFG.filter.dwShowServerWithMinPlayers = 0;
	AppCFG.filter.cActiveMaxPlayer = 0;
	AppCFG.filter.cActiveMinPlayer = 0;
	 
	if(AppCFG.filter.dwPing == 0)
	{
			AppCFG.filter.dwPing = 9999;	
	}
	if(AppCFG.socktimeout.tv_sec<1)
			AppCFG.socktimeout.tv_sec = 1;
	
	if(AppCFG.socktimeout.tv_sec>6)
			AppCFG.socktimeout.tv_sec = 6;

	if(AppCFG.socktimeout.tv_usec>1000)
		AppCFG.socktimeout.tv_usec  = 1000;

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);
//	TiXmlNode *pNode;

	// block: name
	{
		pElem=hDoc.FirstChildElement().Element();
		// should always have a valid root but handle gracefully if it does
		if (!pElem) return 1;
		const char *szP;
		szP = pElem->Value(); //= ETSVcfg

		// save this for later
		hRoot=TiXmlHandle(pElem);
	}
	int intVal=0;

	pElem=hRoot.FirstChild("BuddyList").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("show",&intVal);
		if(intVal)
			AppCFG.bShowBuddyList = TRUE;
		else
			AppCFG.bShowBuddyList = FALSE;
		pElem->QueryIntAttribute("SortAscending",&intVal);
		if(intVal)
			AppCFG.bSortBuddyAsc = TRUE;
		else
			AppCFG.bSortBuddyAsc = FALSE;

		if(pElem->QueryIntAttribute("ColumnSort",&intVal)!=TIXML_NO_ATTRIBUTE)
			AppCFG.cBuddyColumnSort  = (char)intVal;
		else
			AppCFG.cBuddyColumnSort = 0; 
	} else
	{
		AppCFG.cBuddyColumnSort = 0; 
		AppCFG.bSortBuddyAsc = true;
		AppCFG.bShowBuddyList = true;
	}

	pElem=hRoot.FirstChild("MapPreview").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("show",&intVal);
		WNDCONT[WIN_MAPPREVIEW].bShow = AppCFG.bShowMapPreview = intVal;
	} else //set defualt value
		WNDCONT[WIN_MAPPREVIEW].bShow = AppCFG.bShowMapPreview = FALSE;

	pElem=hRoot.FirstChild("mIRC").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("enable",&intVal);
		AppCFG.bUseMIRC = intVal;
	} else //set defualt value
		AppCFG.bUseMIRC = FALSE;


	pElem=hRoot.FirstChild("ShortCountryName").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("enable",&intVal);
		AppCFG.bUseShortCountry = (BOOL) intVal;
	} else //set defualt value
		AppCFG.bUseShortCountry = FALSE;


	pElem=hRoot.FirstChild("ServerRules").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("show",&intVal);
		AppCFG.bShowServerRules  = intVal;
	} else //set defualt value
		AppCFG.bShowServerRules = TRUE;
	
	pElem=hRoot.FirstChild("PlayerList").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("show",&intVal);
		AppCFG.bShowPlayerList   = intVal;

		pElem->QueryIntAttribute("SortAscending",&intVal);
		AppCFG.bShowPlayerList = intVal;
		if(pElem->QueryIntAttribute("ColumnSort",&intVal)!=TIXML_NO_ATTRIBUTE)
			AppCFG.cPlayerColumnSort   = (char)intVal;
		else
			AppCFG.cPlayerColumnSort = 0; 

	} else //set default value
	{
		AppCFG.bShowPlayerList = TRUE;
		AppCFG.bSortPlayerAsc = TRUE;
	}
	
	pElem=hRoot.FirstChild("Minimize").Element();
	if (pElem)
	{
		if(pElem->QueryIntAttribute("enable",&intVal)!=TIXML_NO_ATTRIBUTE)
			AppCFG.bUse_minimize   = intVal;

		if(pElem->QueryIntAttribute("Key",&intVal)!=TIXML_NO_ATTRIBUTE)
			AppCFG.cMinimizeKey = (char)intVal;
		 
		if(pElem->QueryIntAttribute("ModKey",&intVal)!=TIXML_NO_ATTRIBUTE)
			AppCFG.dwMinimizeMODKey = intVal;

		if(pElem->Attribute("WindowNames")!=NULL)
			strcpy(AppCFG.szET_WindowName,pElem->Attribute("WindowNames"));

	} 	

	pElem=hRoot.FirstChild("BuddyNotify").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("enable",&intVal);
		AppCFG.bPlayNotifySound   = intVal;
		strcpy(AppCFG.szNotifySoundWAVfile,pElem->Attribute("wav_file"));

	} else //set default value
		AppCFG.bPlayNotifySound = true;

	pElem=hRoot.FirstChild("LastGameView").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("index",&intVal);
		g_currentGameIdx   = intVal;		
	} else //set default value
		g_currentGameIdx  = 0;
	

	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"FilterPing",(int&)AppCFG.bUseFilterOnPing);
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"HideOfflineServers",(int&)AppCFG.filter.bHideOfflineServers);

//	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"NoEmpty",(int&)AppCFG.filter.bNoEmpty);
//	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"NoFull",(int&)AppCFG.filter.bNoFull);
//	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"NoPrivate",(int&)AppCFG.filter.bNoPrivate);
//	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"OnlyPrivate",(int&)AppCFG.filter.bOnlyPrivate);
//	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"Punkbuster",(int&)AppCFG.filter.bPunkbuster);
//	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"OnlyRanked",(int&)AppCFG.filter.bRanked);
//	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"Pure",(int&)AppCFG.filter.bPure);
//	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"NoBots",(int&)AppCFG.filter.bNoBots);
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"NumMaxPlayers",(int&)AppCFG.filter.dwShowServerWithMaxPlayers);
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"NumMinPlayers",(int&)AppCFG.filter.dwShowServerWithMinPlayers);
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"NumMaxPlayersActive",(int&)AppCFG.filter.cActiveMaxPlayer);
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"NumMinPlayersActive",(int&)AppCFG.filter.cActiveMinPlayer);
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"Ping",(int&)AppCFG.filter.dwPing);

	
	ReadCfgInt(hRoot.FirstChild("Options").FirstChild().ToElement(),"Transparancy",(int&)AppCFG.g_cTransparancy);
	ReadCfgInt(hRoot.FirstChild("Options").FirstChild().ToElement(),"MaxScanThreads",(int&)AppCFG.dwThreads);



	pElem=hRoot.FirstChild("SocketTimeout").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("seconds",&intVal);
		AppCFG.socktimeout.tv_sec   = intVal;		
		pElem->QueryIntAttribute("useconds",&intVal);
		AppCFG.socktimeout.tv_usec   = intVal;		

	} else //set default value
	{
		AppCFG.socktimeout.tv_sec   = 1;	
		AppCFG.socktimeout.tv_usec   = 0;	
	}

	TiXmlElement * pElemSort;
	pElemSort=hRoot.FirstChild("Sort").ToElement();
	if(pElemSort!=NULL)
	{
		TiXmlNode*  pNode = pElemSort->FirstChild("ColumnsServerList");
		if(pNode!=NULL)
		{
			TiXmlElement * pElemSortIdx = pNode->ToElement();
			if(pElemSortIdx!=NULL)
			{
				for(int i=0;i<MAX_COLUMNS; i++)
				{		
					
					if(pElemSortIdx->FirstChild()!=NULL)
					{
						TiXmlElement * pElemSortValue = pElemSortIdx->FirstChild()->ToElement();
						ReadCfgInt(pElemSortValue, "Active",(int&)CUSTCOLUMNS[i].bActive);
						ReadCfgInt(pElemSortValue, "id",(int&)CUSTCOLUMNS[i].id);
						ReadCfgInt(pElemSortValue, "Ascending",(int&)CUSTCOLUMNS[i].bSortAsc);
						ReadCfgInt(pElemSortValue, "columnIdx",(int&)CUSTCOLUMNS[i].columnIdx);
						ReadCfgInt(pElemSortValue, "cx",(int&)CUSTCOLUMNS[i].lvColumn.cx);
						CUSTCOLUMNS[i].columnIdxToSave = CUSTCOLUMNS[i].columnIdx;

						char szOutput[50];
						if(XML_GetTreeItemStrValue(pElemSortValue,szOutput, sizeof(szOutput)-1)!=NULL)
							CUSTCOLUMNS[i].sName = szOutput;
						pElemSortIdx = pElemSortIdx->NextSiblingElement();
						
						if(pElemSortIdx==NULL)
							break;
					}
				}
			}
		}
	}
	ReadCfgInt(hRoot.FirstChild("Sort").FirstChild().ToElement(),"LastSortColumn",iLastColumnSortIndex);

	TiXmlElement* pElement;	
	pElement=hRoot.FirstChild("Games").ToElement();
	if(pElement!=NULL)
	{
		pElement = pElement->FirstChild()->ToElement();
		if(pElement!=NULL)
		{
			for(int i=0;i<MAX_SERVERLIST;i++)
			{
				TiXmlElement* pNode = pElement->FirstChild()->ToElement();
				if(pNode!=NULL)
				{
					ReadCfgStr(pNode, "GameName",GI[i].szGAME_NAME,MAX_PATH);
					ReadCfgStr(pNode, "Path",GI[i].szGAME_PATH,MAX_PATH);
					ReadCfgStr(pNode, "Cmd",GI[i].szGAME_CMD,sizeof(GI[i].szGAME_CMD));
				//	ReadCfgStr( pNode, "yawn",GI[i].szMAP_YAWN_PATH,sizeof(GI[i].szMAP_YAWN_PATH));
					ReadCfgStr( pNode, "MapPreview",GI[i].szMAP_MAPPREVIEW_PATH,sizeof(GI[i].szMAP_MAPPREVIEW_PATH));
					ReadCfgStr( pNode, "MasterServer",GI[i].szMasterServerIP,sizeof(GI[i].szMasterServerIP));		
					ReadCfgInt( pNode, "MasterServerPort",(int&)GI[i].dwMasterServerPORT);
					ReadCfgInt( pNode, "Protocol",(int&)GI[i].dwProtocol);		
					ReadCfgInt(pNode, "Active",(int&)GI[i].bActive);
					ReadCfgInt(pNode, "FilterMod",(int&)GI[i].filter.dwMod);
					ReadCfgInt(pNode, "FilterGameType",(int&)GI[i].filter.dwGameTypeFilter);					
					ReadCfgInt(pNode, "FilterHideFull",(int&)GI[i].filter.bNoFull);
					ReadCfgInt(pNode, "FilterHideEmpty",(int&)GI[i].filter.bNoEmpty);
					ReadCfgInt(pNode, "FilterPunkbuster",(int&)GI[i].filter.bPunkbuster);
					ReadCfgInt(pNode, "FilterHideOffline",(int&)GI[i].filter.bHideOfflineServers);
					ReadCfgInt(pNode, "FilterPure",(int&)GI[i].filter.bPure);
					ReadCfgInt(pNode, "FilterRanked",(int&)GI[i].filter.bRanked);
					ReadCfgInt(pNode, "FilterHideBots",(int&)GI[i].filter.bNoBots);
					ReadCfgInt(pNode, "FilterNoPrivate",(int&)GI[i].filter.bNoPrivate);
					ReadCfgInt(pNode, "FilterOnlyPrivate",(int&)GI[i].filter.bOnlyPrivate);					
					pElement = pElement->NextSiblingElement();
					if(pElement==NULL)
						break;
				} else
					break;
			}
		}
	}

	dbg_print("\nLoaded config\n");	
	return 0;

}

void  OnMouseMove(HWND hWnd, WPARAM wParam,LPARAM lParam)
{
	POINT pt;
	HDC hdc;
	RECT rc;
	hdc = GetDC(hWnd);
	GetClientRect(hWnd,&rc);
	pt.x = LOWORD(lParam); 
	pt.y = HIWORD(lParam); 

	//   DrawFocusRect(hdc, &SplitterGripArea[1].hit);

	if(Sizing)
	{
		if(wParam == MK_LBUTTON)   
		{
			for(int i=0;i<3;i++)
			{

				if(SplitterGripArea[i].bMoveHorizontal && SplitterGripArea[i].bSizing)
				{
					DrawFocusRect(hdc, &SplitterGripArea[i].hit);
					if((pt.y<rc.bottom-(TOOLBAR_Y_OFFSET+STATUSBAR_Y_OFFSET)) && (pt.y>TOOLBAR_Y_OFFSET+20) )
					{
						SplitterGripArea[i].hit.top = pt.y;
						SplitterGripArea[i].hit.bottom = pt.y + BORDER_SIZE;
					}
					DrawFocusRect(hdc, &SplitterGripArea[i].hit);
					SetCursor(LoadCursor(NULL, IDC_SIZENS));

				} else if(SplitterGripArea[i].bMoveVertical &&SplitterGripArea[i].bSizing)
				{
					DrawFocusRect(hdc, &SplitterGripArea[i].hit);
					if(pt.x>30)
					{
						SplitterGripArea[i].hit.left = pt.x;
						SplitterGripArea[i].hit.right = pt.x + BORDER_SIZE;
					}
					DrawFocusRect(hdc, &SplitterGripArea[i].hit);
					SetCursor(LoadCursor(NULL, IDC_SIZEWE));
				}

			}
		}
		InvalidateRect(WNDCONT[WIN_PING].hWnd,&WNDCONT[WIN_PING].rSize,TRUE);
	} 
	else 
	   
	{
		for(int i=0;i<3;i++)
		{
		   if(PtInRect(&SplitterGripArea[i].hit,pt))
		   {
			   if(SplitterGripArea[i].bMoveHorizontal)
					SetCursor(LoadCursor(NULL, IDC_SIZENS));
			   else
					SetCursor(LoadCursor(NULL, IDC_SIZEWE));
		   }
		}
	}
	ReleaseDC(hWnd, hdc);
}

void OnLeftMouseButtonUp(HWND hWnd, WPARAM wParam,LPARAM lParam)
{
   POINT pt;
   RECT rc;
   HDC hdc;
   GetClientRect(hWnd,&rc);
   hdc = GetDC(hWnd);
   pt.x = LOWORD(lParam); 
   pt.y = HIWORD(lParam); 

	if(Sizing)
	{
		for(int i=0;i<3;i++)
		{
			if(SplitterGripArea[i].bMoveHorizontal && SplitterGripArea[i].bSizing)
			{
				if(pt.y>rc.bottom-(TOOLBAR_Y_OFFSET+STATUSBAR_Y_OFFSET) && (pt.y>TOOLBAR_Y_OFFSET+20))
					pt.y = rc.bottom-(TOOLBAR_Y_OFFSET+STATUSBAR_Y_OFFSET);
					SplitterGripArea[i].tvYPos = pt.y-TOOLBAR_Y_OFFSET;

			} else if(SplitterGripArea[i].bMoveVertical &&SplitterGripArea[i].bSizing)
			{
				if(pt.x>30)
					SplitterGripArea[i].tvXPos = pt.x+2;
				else
					SplitterGripArea[i].tvXPos = 30;
			}
			SplitterGripArea[i].bSizing = FALSE;
		}
		
		ReleaseCapture();
		Sizing = FALSE;
	}
	ReleaseDC(hWnd, hdc);	
	InvalidateRect(WNDCONT[WIN_PING].hWnd,&WNDCONT[WIN_PING].rSize,TRUE);
	InvalidateRect(hWnd,NULL,TRUE);
	PostMessage(hWnd, WM_SIZE, 0xFFFF, 0);
}

void OnLeftMouseButtonDown(HWND hWnd, WPARAM wParam,LPARAM lParam)
{
   HDC     hdc;
   hdc = GetDC(hWnd);
   POINT pt;

   pt.x = LOWORD(lParam); 
   pt.y = HIWORD(lParam); 
    
   for(int i=0;i<3;i++)
   {
	   if(PtInRect(&SplitterGripArea[i].hit,pt))
		{	
			DrawFocusRect(hdc, &SplitterGripArea[i].hit);
			SplitterGripArea[i].bSizing = TRUE;
			Sizing = TRUE;
			SetCapture(hWnd);
		}
   }
  	
 //DrawFocusRect(hdc, &SplitterGripArea[0].hit);
   ReleaseDC(hWnd, hdc);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	POINT lpClickPoint;
	g_hWnd = hWnd;
	HMENU hPopMenu;

	switch (message) 
	{
		case WM_UPDATESERVERLIST_COLUMNS:
			UpdateColumnsPosAfterDrag();
		break;
		case WM_GETSERVERLIST_START:
			//Initialize_GetServerListThread(SCAN_ALL_GAMES);
			break;
		case WM_GETSERVERLIST_STOP:

			break;

		case WM_INITVIEWS:
			Update_WindowSizes();
			PostMessage(g_hWnd,WM_SIZE,0,0);
			return TRUE;

		case WM_REFRESHSERVERLIST:
			RedrawServerListThread(&GI[g_currentGameIdx]);
			return TRUE;
		case WM_CREATE:
		{
			  //add a band that contains a combobox			
			OnCreate(hWnd,g_hInst);	
			
			OnInitialize_MainDlg(hWnd);
			PostMessage(hWnd, WM_SIZE, 0, 0);
			PostMessage(g_hWnd,WM_REFRESHSERVERLIST,0,0);
			
			return TRUE;
		break;
		}
	case WM_NOTIFY:
		{
			return OnNotify(hWnd,  message,  wParam,  lParam);
				
		}
		break;
	case WM_SIZE:
	   {
		   
		 if(wParam==SIZE_RESTORED)
		 {
			 OnRestore();
			// CalcSplitterGripArea();
		 }
		 else if (wParam==SIZE_MINIMIZED)
		 {
			 g_bMinimized=TRUE;
			 PostMessage(g_hWnd,WM_CLOSE,0xdead,0);
			
			 //Removed since v5.41
			 //ShowBalloonTip("Notification","ET Server Viewer has still running.\nDouble click on the task tray icon to\nopen it again.");
			return TRUE;
		 }

		OnSize(hWnd);
		UpdateWindow(g_hWnd);
	   }
      break;
   case WM_LBUTTONDOWN: OnLeftMouseButtonDown( hWnd, wParam, lParam);
	   break;
	case WM_MOUSEMOVE:  OnMouseMove(hWnd,  wParam, lParam);
	   break;
	case WM_LBUTTONUP:	OnLeftMouseButtonUp( hWnd,  wParam, lParam);
	   break;
	case WM_HOTKEY:
		if ((wParam == HOTKEY_ID))
			tryToMinimizeGame();
		break;		
			
	case WM_USER_SHELLICON: 
		
		switch(LOWORD(lParam)) 
		{ 
				case WM_LBUTTONDBLCLK: // to open the GUI of the Application 
					//DialogBox(hInst, MAKEINTRESOURCE(IDD_FORMVIEW), NULL,(DLGPROC) MainProc);
					PostMessage(hWnd,WM_COMMAND,IDM_OPEN,0);
			
					break; 

				case WM_RBUTTONDOWN: 
					//get mouse cursor position x and y as lParam has the message itself 
					GetCursorPos(&lpClickPoint);
		
					//place the window/menu there if needed 
								
					hPopMenu = CreatePopupMenu();
					InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_OPEN,"&Open");
					InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_ABOUT,"&About");
					InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_EXIT,"&Exit");
										
						//workaround for microsoft bug, to hide menu w/o selecting
					SetForegroundWindow(hWnd);
					TrackPopupMenu(hPopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,lpClickPoint.x, lpClickPoint.y,0,hWnd,NULL);
					SendMessage(hWnd,WM_NULL,0,0);
                    DestroyMenu(hPopMenu);
					return TRUE; 
			} 
			break; 
	
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 


		switch(wmEvent)
			{
				
				case CBN_EDITCHANGE:
					{
						if(wmId==IDC_COMBOBOXEX_CMD) 
						{

							char ip[100];
							GetDlgItemText(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD,ip,99);
							if(strlen(ip)>0)
							{
								Show_ToolbarButton(IDC_BUTTON_FIND,true);
								FindServer(ip);
								SetDlgItemText(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD,ip);
					
								SetCursor(::LoadCursor(NULL,IDC_ARROW));
								ShowCursor(TRUE);
								SendDlgItemMessage (g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD, CB_SETEDITSEL, 0,MAKELPARAM(strlen(ip),strlen(ip))); 
							
							}
							else
							{
								Show_ToolbarButton(IDC_BUTTON_FIND,false);
								RedrawServerListThread(&GI[g_currentGameIdx]);
							}
						}
						return FALSE;
						break;
					}

				case CBN_SELENDOK:
				{
				
					if (wmId==IDC_COMBOBOXEX_CMD)
					{
						char ip[300];
						DWORD dwPort;
					
						int i = SendDlgItemMessage(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD,CB_GETCURSEL ,0,0);
						SendDlgItemMessage(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD,CB_GETLBTEXT ,i,(LPARAM)ip);
						
						//GetDlgItemText(g_hWnd,IDC_COMBOBOXEX_CMD,ip,99);
													
						if(SplitIPandPORT(ip,dwPort)!=NULL)
						{
							strcpy(g_currServerIP,ip);
							dwCurrPort = dwPort;	
							g_CurrentSRV = FindServerByIPandPort(ip,dwPort);
					
							ListView_DeleteAllItems(g_hwndListViewVars);
							ListView_DeleteAllItems(g_hwndListViewPlayers);
							GetServerInfo(g_currentGameIdx,g_CurrentSRV);
							UpdateServerItem(g_CurrentSRV->dwIndex);
							UpdateCurrentServerUI();
							
							//PostMessage(g_hWnd,WM_USER_GETSERVERINFO,0,(LPARAM)NULL);
						}
						Show_ToolbarButton(IDC_BUTTON_FIND,true);
					}
				
				break;
				}
				
			}//End wmEvent switch
			// Parse the menu selections:
			switch (wmId)
			{

				case IDM_OPEN:
				{		
					if(g_bMinimized==false)
						return TRUE;
					//ShowWindow(hWnd, SW_SHOWMAXIMIZED);
					OnInitialize_MainDlg(g_hWnd);
					//ShowWindow(g_hWnd, 	 SW_SHOW); 
					ShowWindow(g_hWnd, 	 SW_SHOW);
					ShowWindow(g_hWnd, SW_SHOWMAXIMIZED);
					
					if(g_bMinimized)
						PostMessage(g_hWnd,WM_INITVIEWS,0,0);
					g_bMinimized= FALSE;

					PostMessage(g_hWnd,WM_REFRESHSERVERLIST,0,0);
					return TRUE;
					break;
				}
				case IDM_SCAN_ALL_GAMES:
					OnActivate_ServerList(SCAN_ALL_GAMES);
					break;
				case IDM_SCAN_FILTERED:
					OnActivate_ServerList(SCAN_FILTERED);
					break;

				case IDM_SCAN:  //Toolbar
					OnActivate_ServerList(SCAN_ALL);
				break;
				case IDM_REFRESH:  //Toolbar
				case ID_OPTIONS_REFRESH: 
					OnScanButton();							
				break;
				case IDC_DOWNLOAD:
					{
						HANDLE hThread;
						hThread = CreateThread( NULL, 0, &AutomaticDownloadUpdateSetUp, g_hWnd,0, NULL);                
						if (hThread == NULL) 
						{
							AddLogInfo(ETSV_WARNING, "CreateThread failed  <AutomaticDownloadUpdateSetUp> (%d) File:(%s) Line:(%d)\n", GetLastError(),__FILE__,__LINE__ ); 
						}
						else 
						{
							dbg_print("CreateThread  success AutomaticDownloadUpdateSetUp");
							CloseHandle( hThread );
						}
					}
				
				break;
				case IDC_BUTTON_QUICK_CONNECT:	
					FastConnect();
				break;				
				case IDC_BUTTON_ADD_SERVER:
					OnButtonClick_AddServer();
				break;
				case IDC_BUTTON_FIND:
				{
					char str[100];
					GetDlgItemText(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD,str,99);
					FindServer(str);
				}
				break;
				case ID_OPTIONS_RCON:
					 OnRCON();
					break;
				case ID_BUDDY_ADD:
					Buddy_Add(true);
				break;
				case ID_BUDDY_REMOVE:
					Buddy_Remove();
				break;
				case ID_BUDDY_ADDFROMPLAYERLIST:
					Buddy_Add(false);
				break;
				case ID_FAVORITES_ADDIP:
					Favorite_Add(true);
				break;

				case ID_HELP_DOCUMENTATIONFAQ:
					ShellExecute(NULL,NULL,"http://etsv.cludden.se/page.php?3",NULL,NULL,SW_SHOWNORMAL);
					break;
				case ID_SERVERLIST_PURGEPUBLICSERVLIST:
					{	
						DeleteAllServerLists();
						return TRUE;
					} //ID_SERVERLIST_PURGEPUBLICSERVLIST
				break;
				case IDM_UPDATE:
					{
						HANDLE hThread;
						hThread = CreateThread( NULL, 0, &CheckForUpdates, g_hWnd,0, NULL);                
						if (hThread == NULL) 
						{
							AddLogInfo(ETSV_WARNING, "CreateThread failed  <CheckForUpdates> (%d) File:(%s) Line:(%d)\n", GetLastError(),__FILE__,__LINE__ ); 
						}
						else 
						{
							dbg_print("Launched CheckForUpdates thread.");
							CloseHandle( hThread );
						}
					}
					
				break;
				case ID_VIEW_MAPPREVIEW :						
				case ID_VIEW_SERVERRULES:
				case ID_VIEW_BUDDYLIST:  
				case ID_VIEW_PLAYERLIST: 
			//		oldmaxWidth++;  //work around trick to recalculate width & heights
					ChangeViewStates(LOWORD(wParam));
					break;
				case IDM_SETTINGS:
					{
						DialogBox(g_hInst, (LPCTSTR)IDD_DIALOG_CONFIG, g_hWnd, (DLGPROC)CFG_MainProc);
						
						OnSize(g_hWnd);
						TreeView_BuildList();
						SetDlgTrans(hWnd,AppCFG.g_cTransparancy);
						
						//Do we need to change view after configuring?
						if(GI[g_currentGameIdx].bActive==false)
						{
							ListView_DeleteAllItems(g_hwndListViewServer);
							ListView_DeleteAllItems(g_hwndListViewPlayers);
							SetCurrentViewTo(FindFirstActiveGame());
						}

						return TRUE;
					}
				break;
			case IDM_ABOUT:
				DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				break;
		
			case IDM_EXIT:

				if(g_hWnd!=NULL)
				{
					OnClose();
					ShowWindow(hWnd, SW_HIDE);
				}
				break;
			}
			break;
		case WM_PAINT:
			if(hWnd==NULL)	
				return TRUE;
			hdc = BeginPaint(hWnd, &ps);
			if(hdc==NULL)
				return FALSE;
			OnPaint(hdc);
			EndPaint(hWnd, &ps);
			
			break;

		case WM_CLOSE:
			{	
				if(wParam==0xdead)
				{
					OnMinimize(hWnd);
					return TRUE;
				}
				OnClose();
				ShowWindow(hWnd, SW_HIDE);	
				return TRUE;
				break;
			}
		case WM_DESTROY:
			{
				OnDestroy(hWnd);
				PostQuitMessage(0);
			}
			break;


		default: 
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	
	return 0;
}


#define XML_READ_OK		0
#define XML_READ_ERROR	1




const char * XML_GetTreeItemName(TiXmlElement* pNode,char *szOutput, DWORD maxBytes)
{
	const char *pName=pNode->Attribute("name");
	if(pName!=NULL)
	{
		strcpy_s(szOutput,maxBytes,pName);
		return szOutput;
	}
	AddLogInfo(0,"Error reading XML tag name (XML_GetTreeItemName)");
	return NULL;
}


const char * XML_GetTreeItemStrValue(TiXmlElement* pNode,char *szOutput, DWORD maxBytes)
{
	const char *pValue = pNode->Attribute("strval");

		if(pValue!=NULL)
		{
			strcpy_s(szOutput,maxBytes,pValue);
			return szOutput;
		}
	AddLogInfo(0,"Error reading XML tag strval");
	return NULL;
}

const char * XML_GetTreeItemStr(TiXmlElement* pNode, const char* attributeName,char *szOutput, DWORD maxBytes)
{
	if(pNode!=NULL)
	{
		const char *pValue = pNode->Attribute(attributeName);
		if(pValue!=NULL)
		{
			strcpy_s(szOutput,maxBytes,pValue);
			return szOutput;
		}
	}
	AddLogInfo(ETSV_WARNING,"Error finding XML attributename = %s",attributeName);
	return NULL;
}

int XML_GetTreeItemInt(TiXmlElement* pNode, const char* attributeName)
{
	int value=0;
	if(pNode!=NULL)
	{
		int ret = pNode->QueryIntAttribute(attributeName,&value);	
		if(ret!=TIXML_NO_ATTRIBUTE)
			return value;
	}
	AddLogInfo(ETSV_WARNING,"Error finding XML attributename = %s",attributeName);

	return 0;
}

int ReadCfgInt(TiXmlElement* pNode, char *szParamName, int& intVal)
{	
	for( pNode; pNode; pNode=pNode->NextSiblingElement())
	{		
		const char *pName=pNode->Attribute("name");
		if(pName==NULL)
			continue;
	
		if(strcmp(szParamName,pName)==0)
		{
			const char *pValue=pNode->Attribute("value");
			intVal = atoi(pValue);
			//pNode->QueryIntAttribute("value",&intVal); // If this fails, original value is left as-is	
			return XML_READ_OK;					
		}
	}
	AddLogInfo(0,"Error reading XML tag %s",szParamName);
	return XML_READ_ERROR;
}

int ReadCfgInt2(TiXmlElement* pNode, char *szParamName, int& intVal)
{	
	for( pNode; pNode; pNode=pNode->NextSiblingElement())
	{		
		const char *pName=pNode->Value();
		if(pName==NULL)
			continue;
	
		if(strcmp(szParamName,pName)==0)
		{
			const char *pValue = pNode->FirstChild()->Value();
			
			intVal = atoi(pValue);
			
			return XML_READ_OK	;					
		}
	}
	AddLogInfo(0,"Error reading XML tag %s",szParamName);
	return XML_READ_ERROR;
}

const char * ReadCfgStr2(TiXmlElement* pNode, char *szParamName,char *szOutputBuffer,int iBuffSize)
{
	if(szOutputBuffer==NULL)
		return NULL;
	ZeroMemory(szOutputBuffer,iBuffSize);
	for( pNode; pNode; pNode=pNode->NextSiblingElement())
	{
		
		const char *pName=pNode->Value();
		if(pName==NULL)
			continue;
		
		
		if(strcmp(szParamName,pName)==0)
		{
			if(pNode->FirstChild()!=NULL)
			{
				const char *pValue = pNode->FirstChild()->Value();
			//int len = strlen(pValue);
			//strcpy(szOutputBuffer,pValue);
				strncpy(szOutputBuffer,pValue,iBuffSize);
				return szOutputBuffer;					
			}
			return NULL;
		}
	
	}
	AddLogInfo(0,"Error reading XML tag %s",szParamName);
	return NULL;
}

char * ReadCfgStr(TiXmlElement* pNode, char *szParamName,char *szOutputBuffer,int iBuffSize)
{
	if(szOutputBuffer==NULL)
		return NULL;
	ZeroMemory(szOutputBuffer,iBuffSize);
	for( pNode; pNode; pNode=pNode->NextSiblingElement())
	{
		
		const char *pName=pNode->Attribute("name");
		if(pName==NULL)
			continue;
		
		if(strcmp(szParamName,pName)==0)
		{
			const char *pValue = pNode->Attribute("value"); // If this fails, original value is left as-is
			//int len = strlen(pValue);
			//strcpy(szOutputBuffer,pValue);
			strncpy(szOutputBuffer,pValue,iBuffSize);
			return szOutputBuffer;					
		}
	}
	AddLogInfo(0,"Error reading XML tag %s",szParamName);
	return NULL;
}

void WriteCfgInt(TiXmlElement * root, char *szParentName, char *szParamName,int value) 
{
	TiXmlElement * filter;
	filter = new TiXmlElement(szParentName );  
	filter->SetAttribute("name",szParamName);
	filter->SetAttribute("value",value);
	root->LinkEndChild( filter );  
}

void WriteCfgStr(TiXmlElement * root, char *szParentName, char *szParamName,const char *value) 
{
	TiXmlElement * filter;
	filter = new TiXmlElement(szParentName );  
	filter->SetAttribute("name",szParamName);
	filter->SetAttribute("value",value);
	root->LinkEndChild( filter );  
}