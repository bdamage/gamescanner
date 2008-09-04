// GameScanner.cpp : Defines the entry point for the application.
// Copyright (C) 2007, 2008 Kjell Lloyd 

/*

Set up GUID keys:
Product code:
ETSV 5.x?     {122CD6F9-B1C2-4124-B5B4-5C0B255B74D1}

1.0            {C9C6D743-D7CF-46FD-9366-255F9DFD8442}
v1.0 beta 2    {EAB596BD-45FF-49A2-87CD-7EC3DF77E69C}
v 1.0.1 (1.01) {9FF8932F-7A8C-4654-91CB-5EAE02FB1B38}
v 1.0.2 (1.02) {8F2DE466-8EEB-4D64-8F4A-375553A8D31E}
v 1.0.3 (1.03) {40ED2250-FB02-4183-B2A6-F0A987C2E277}
v 1.0.4 (1.04) {398C9890-FAD1-48DB-A1D0-DE2BBBE661AA}
v 1.0.5 (1.05) {5F677BD9-D1A6-4511-B69C-BD90DD4FCE03}
v 1.0.6 (1.06) {443FDBF3-D249-4510-86A9-93FF61CC8704}
v 1.0.7 (1.07) {CEC4EAF2-8EBA-4C36-8B4F-A69EA0E5AF00}
v 1.0.8        {9989D8EB-84F7-4FC8-9AE2-52CFF0A3DADB} beta
v 1.0.9		   {6EFF1869-9A95-4293-AD16-FCB060E41D56}
v 1.1.0		   {A4883E53-F798-41AB-8251-4B7775CEA4CA}
v 1.1.1        {992C8633-388B-4108-80BE-E860BEBEECE8}

Upgrade code 1.0 - 1.0.9:
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
#include <strsafe.h>
#include "CountryCodes.h"


#pragma comment(lib, "ole32.lib")

//#ifdef _DEBUG
//#pragma comment(lib, "..\\tinyxml\\Release\\tinyxmld.lib")
//#else
//#pragma comment(lib, "..\\tinyxml\\Release\\tinyxml.lib")
//#endif

#pragma comment(lib, "HttpFileDownload.lib")
#pragma comment(lib, "iptocountry.lib")

//#pragma comment(lib, "..\\..\\FreeImage\\FreeImage.lib")

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

using namespace std;

// Global Variables:
HINSTANCE g_hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name


// Forward declarations of functions included in this code module:

BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);


#define MAX_LOADSTRING 100

char TREEVIEW_VERSION[20];

#define ICON_HEIGHT 24
#define ICON_WIDTH	24

#define TIMER_EVENT		 1001
#define TIMER_EVENT_RSS	 1002

#define SCAN_ALL		0
#define SCAN_FILTERED	1
#define SCAN_ALL_GAMES	2


#define ETSV_PURGE_COUNTER 5  //Counter after X timeouts to purge (delete) the server

#define XML_READ_OK		0
#define XML_READ_ERROR	1


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
#define COL_STATUS		13

//if you add something ensure to increase MAX_COLUMNS below
#define MAX_COLUMNS COL_STATUS+1

#pragma comment( user, "Compiled on " __DATE__ " at " __TIME__ ) 
#ifndef _DEBUG
TCHAR szDialogTitle[]="Game Scanner v" APP_VERSION ;
#else
TCHAR szDialogTitle[]="Game Scanner v" APP_VERSION " Compiled on " __DATE__ " at "__TIME__;
#endif


int g_currentGameIdx = ET_SERVERLIST;
int g_currentScanGameIdx = -1;


/****************************************
	Buddy Global vars
*****************************************/
BUDDY_INFO *g_pBIStart=NULL;
DWORD g_tvIndex=0;



BOOL g_bTREELOADED = FALSE;



BOOL bSortColumnAscading[15];
int iLastColumnSortIndex = COL_PLAYERS;

extern BOOL SCANNER_bCloseApp;


bool bFirstTimeSizeCalc= true;
#define BORDER_SIZE 4

string g_sMIRCoutput ="";

_WINDOW_CONTAINER WNDCONT[15];
PLAYERDATA *pCurrentPL=NULL; //a temporary current player list in listview, this will be 
                             //keept when doing a rescan and the user want's to add aplayer to the buddylist for instance.

CRITICAL_SECTION	SCANNER_cs,SCANNER_CSthreadcounter; 
CRITICAL_SECTION	REDRAWLIST_CS; 
CRITICAL_SECTION	LOAD_SAVE_CS; 

int ImageSizeX = 120;
//int oldmaxWidth=0;

HTREEITEM iSelected = NULL;
//TVITEM  tvi;

int tabViewMode = FAVORITE;
void GameTypeFilter_GetSelectedGameType(HTREEITEM hRoot);

HANDLE hCloseEvent = NULL; 

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
char g_szIPtoAdd[260];
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
CLanguage lang;

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
long UpdatePlayerList(LPPLAYERDATA pPlayers);
long UpdateRulesList(LPSERVER_RULES pServerRules);

void TreeView_BuildList();
void OnMinMaxPlayers(HWND hWndParent,bool bMinValueToSet);
void Update_TreeViewCFGFile();
void SaveAll(DWORD dwCloseReason);
SERVER_INFO * Get_CurrentServerListByView();
void ChangeViewStates(UINT uItem);
SERVER_INFO *FindServer(char *str);



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
void OnMinimize(HWND hWnd);
void OnStopScanning();
void OnScanButton();
void SetImageList();
DWORD WINAPI ReScanServerList(LPVOID lpParam);
void StartGame_ConnectToServer(bool connectFromBuddyList);
void Favorite_Remove();
void Favorite_Add(bool manually, char *szIp=NULL);
void FilterUpdate();

DWORD Build_CountryFilter(HTREEITEM hRoot);
void Initialize_CountryFilter();
LRESULT CALLBACK CFG_MainProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
HTREEITEM MainTreeView_AddItem(_TREEITEM ti, HTREEITEM hCurrent, bool active=true);
void TreeView_SetItemText(HTREEITEM hTI, char *szText);
void Select_item_and_all_childs(HTREEITEM hRoot, bool select);
int TreeView_GetSelectionV3();
const char * XML_GetTreeItemStr(TiXmlElement* pNode, const char* attributeName,char *szOutput, DWORD maxBytes);


char EXE_PATH[_MAX_PATH+_MAX_FNAME];			//Don't write anything to this path
char USER_SAVE_PATH[_MAX_PATH+_MAX_FNAME];     //Path to save settings and server lists
char COMMON_SAVE_PATH[_MAX_PATH+_MAX_FNAME];   //Used for downloading app update and preview images - purpose to share the same data between users.
char EXE_PATH_OLD[_MAX_PATH+_MAX_FNAME];


DWORD dwCurrPort = 27960;
bool g_bDoFirstTimeCheckForUpdate=true;

bool bPlayerNameAsc=true,bPlayerClanAsc=true,	bRateAsc = true,bPlayerPingAsc=true;
APP_SETTINGS_NEW AppCFG;


// Global Variables:
SERVER_INFO *g_CurrentSRV = NULL;
SERVER_INFO g_tmpSRV ;
SERVER_INFO g_CurrentSelServer;
bool g_bMinimized=false;
SERVER_INFO *g_PRIVPASSsrv = NULL;
HWND g_PROGRESS_DLG,g_DlgProgress= NULL,g_DlgProgressMsg=NULL;
RECT g_rcDestMapImg;
HWND g_hwndToolbarOptions = NULL;
RECT g_CSRect,g_INFOIconRect;
HWND g_hwndSearchToolbar = NULL, g_hwndSearchCombo= NULL;
HWND g_hwndComboEdit = NULL;
HWND g_hWnd=NULL,g_hwndRibbonBar;

HIMAGELIST g_hImageListIcons = NULL;
HIMAGELIST m_hImageList = NULL;
HIMAGELIST m_hImageListSearchBar = NULL;
HIMAGELIST m_hImageListHot = NULL;
HIMAGELIST m_hImageListDis = NULL;
HIMAGELIST g_hILFlags = NULL;
HFONT g_hf  =NULL;
HFONT g_hf2 = NULL;


char g_currServerIP[128];
bool g_bCancel = false;
bool g_bRunningQuery = false;
bool g_bRunningQueryServerList = false;
int g_iCurrentSelectedServer = -1;
int g_statusIcon = -1;
BOOL g_bRedrawServerListThread = FALSE;
HICON hOnlineIcon = NULL;
HICON hOfflineIcon = NULL;

BOOL g_bControl = FALSE;

HWND g_hwndTabControl = NULL;
HWND g_hwndLogger = NULL,g_hwndStatus = NULL;
HWND g_hwndListViewPlayers=NULL,g_hwndListViewVars=NULL,g_hwndListViewServer=NULL,g_hwndListBuddy=NULL;
HWND g_hwndListViewServerListHeader = NULL;
HWND g_hwndMainTreeCtrl=NULL, g_hwndProgressBar=NULL, hwndPaneV=NULL;
HWND hwndPaneH1=NULL,hwndButtonTest=NULL,hwndButtonOptions=NULL,hwndButtonSearch=NULL;
HWND hwndButtonAddToFavorite=NULL;

HTREEITEM hRootItem=NULL,hRootFiltersItem=NULL,hRootCountryFiltersItem=NULL,hRootEuropeItem=NULL,hFilterPingItem=NULL,hFilterGameTypeItem=NULL;

struct _CountryFilter 
{
	int counter;

	LPARAM lParam[255];
	char szShortCountryName[255][4];

};
_CountryFilter CountryFilter;




GamesMap GamesInfo;

SERVER_CONTAINER SC[17];
//GAME_INFO GamesInfo[GamesInfo.size()+1];
GAME_INFO *currCV = NULL;
char g_szMapName[MAX_PATH];
HWND g_hwndMainRCON=NULL;
HWND g_hwndMainSTATS=NULL;

#define EVENT_PING 1010
UINT_PTR pingTimer;
deque<DWORD> QPing;
typedef deque<DWORD> deQPing;

struct _TRACERT_STRUCT
{
	UCHAR TTL;
	ULONG IpAdress;
	ULONG RTT;
	char szHostName[260];

};

deque<_TRACERT_STRUCT> _TraceRT;
typedef deque<_TRACERT_STRUCT> TraceRT;

void MyDrawFont(HDC hdc, int x, int y, const char *szMessage, int angle)
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
	int i = ListView_GetSelectionMark(g_hwndListViewPlayers);
	
	if(i!=-1)
	{					
		PLAYERDATA *pPlayerData = pCurrentPL;
		for(int x=0;x<i;x++)
			pPlayerData = pPlayerData->pNext;

		if(pPlayerData==NULL)
		{
			SetStatusText(ICO_WARNING,lang.GetString("InvalidPlayer"));
			InvalidateRect(g_hWnd,NULL,TRUE);
			return NULL;
		}	
		return pPlayerData;
	}

	return NULL;
}




void Draw_GraphBackground(HWND hWnd,HDC hDC,UCHAR nGridX,UCHAR nGridY,const char *szTitle,const char* szXLegend, const char* szYLegend,DWORD dwMaxValue,DWORD dwDrawOptions=0);


#define DRAW_GRAPH_X_RIGHT_TO_LEFT 0
#define DRAW_GRAPH_X_LEFT_TO_RIGHT 1

void Draw_GraphBackground(HWND hWnd,HDC hDC,UCHAR nGridX,UCHAR nGridY,const char *szTitle,const char* szXLegend, const char* szYLegend,DWORD dwMaxValue,DWORD dwDrawOptions)
{
	RECT size,rect,rectInner;
	HBRUSH hbrBkgnd=NULL,hbrBar=NULL;
	HPEN hpen, hpenOld;	
	HBRUSH  hbrushOld;
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

	hpen = CreatePen(PS_DOT, 1, RGB(64,64,204));
	hpenOld = (HPEN) SelectObject(hDC, hpen);

	if(nGridX==0)
		nGridX = 5;

	int offsetX = rectInner.right / nGridX;
	int offsetY = rectInner.bottom / nGridY;

	int scale = 10;
	if(dwMaxValue!=0)
		scale = dwMaxValue / nGridY-1;


	char szTxt[20];

	sprintf(szTxt,"%d",0);
	if(dwDrawOptions == DRAW_GRAPH_X_RIGHT_TO_LEFT)	
		MyDrawFont(hDC, rectInner.right, rectInner.bottom+3, szTxt,0);
	else
		MyDrawFont(hDC, rectInner.left, rectInner.bottom+3, szTxt,0);

	for(int i=0; i<nGridX-1;i++)
	{	
		/****************************
		 Draw X Grid
		*****************************/
		if(dwDrawOptions == DRAW_GRAPH_X_RIGHT_TO_LEFT)
		{
			int x = rectInner.right-offsetX*(i+1);
			if(x>rectInner.left)
			{				
				MoveToEx(hDC,x,rectInner.top,NULL);
				LineTo(hDC,x,rectInner.bottom);				
				sprintf(szTxt,"%d",(i+1)*nGridX);
				MyDrawFont(hDC, x, rectInner.bottom+3, szTxt,0);
			}
		} else
		{
			int x = rectInner.left+offsetX*(i+1);

			if(x<rectInner.right)
			{				
				MoveToEx(hDC,x,rectInner.top,NULL);
				LineTo(hDC,x,rectInner.bottom);				
				sprintf(szTxt,"%d",(i+1));
				MyDrawFont(hDC, x, rectInner.bottom+3, szTxt,0);
			}

		}
	}
	for(int i=0; i<nGridY-1;i++)
	{
		/****************************
		 Draw Y Grid
		*****************************/
		if((rectInner.bottom-offsetY*(i+1))>rectInner.top)
		{
			int y = rectInner.bottom-offsetY*(i+1);
			MoveToEx(hDC,rectInner.left,y,NULL);
			LineTo(hDC,rectInner.right,y);
			sprintf(szTxt,"%d",(i+1)*scale);
			MyDrawFont(hDC, rectInner.left-12, y, szTxt,900);
		}

	}

	
	MyDrawFont(hDC, 5, (rectInner.bottom/2)+40, szYLegend,900);
	MyDrawFont(hDC, (rectInner.right / 2), rectInner.bottom+10,szXLegend,0);


	// Do not forget to clean up.
	SelectObject(hDC, hpenOld);

	DeleteObject(hpen);
	SelectObject(hDC, hbrushOld);

	char szText[105];
	size.left = 20;
	size.top = 6;
	sprintf(szText,"%s %s",szTitle,szIPAddressToPing);						
	TextOut(hDC, size.left+2,size.top-1, szText, strlen(szText));
	SetTextColor(hDC,0x00000000);
	if(hbrBkgnd!=NULL)
		DeleteObject(hbrBkgnd);
	if(hbrBar!=NULL)
		DeleteObject(hbrBar);
	
	SelectObject(hDC,original);
}

void Draw_PingStats(HWND hWnd, HDC hDC,UCHAR nGridX,UCHAR nGridY,DWORD dwMaxValue)
{
	RECT rect,size,rectInner;
	GetWindowRect(hWnd,&size);
	
	rect.bottom = (size.bottom - size.top);
	rect.right = (size.right - size.left);
		
	rect.left = X_OFFSET_STATS;
	rect.top = X_OFFSET_STATS;
	rect.bottom-=20;
	rect.right-=20;
	CopyRect(&rectInner,&rect);
			
	//Shrink one pixel for the maind rawing area
	rectInner.left = X_OFFSET_STATS+1;
	rectInner.top = X_OFFSET_STATS+1;
	rectInner.bottom--;
	rectInner.right--;

	int offsetX = (rectInner.right / nGridX) / nGridX;
	int offsetY = rectInner.bottom / nGridY;

	int scale = 10;
	if(dwMaxValue!=0)
		scale = dwMaxValue / nGridY-1;

	int oldX = rectInner.right,
		oldY = rectInner.bottom;

	HPEN hpen = NULL;
	HPEN hpenOld=NULL;	
	HPEN hPenStatus = NULL;
	hpen = CreatePen(PS_DOT, 1, RGB(64,64,204));
	hpenOld = (HPEN) SelectObject(hDC, hpen);
	hPenStatus = CreatePen(PS_SOLID, 1, RGB(0,255,0));
	SelectObject(hDC, hPenStatus);

	deQPing::reverse_iterator iLst;
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
	SelectObject(hDC, hpenOld);
	DeleteObject(hPenStatus);
	DeleteObject(hpen);
}


void Draw_TraceRouteStats(HWND hWnd, HDC hDC,UCHAR nGridX,UCHAR nGridY,DWORD dwMaxValue)
{
	RECT rect,size,rectInner;
	GetWindowRect(hWnd,&size);
	
	rect.bottom = (size.bottom - size.top);
	rect.right = (size.right - size.left);
		
	rect.left = X_OFFSET_STATS;
	rect.top = X_OFFSET_STATS;
	rect.bottom-=20;
	rect.right-=20;
	CopyRect(&rectInner,&rect);
			
	//Shrink one pixel for the maind rawing area
	rectInner.left = X_OFFSET_STATS+1;
	rectInner.top = X_OFFSET_STATS+1;
	rectInner.bottom--;
	rectInner.right--;

	if(nGridX==0)
		nGridX = 5;
	int offsetX = rectInner.right / nGridX;
	int offsetY = rectInner.bottom / nGridY;

	int scale = 10;
	if(dwMaxValue!=0)
		scale = dwMaxValue / nGridX-1;

	int oldX = rectInner.left,
		oldY = rectInner.bottom;
	HPEN hpen = NULL;
	HPEN hpenOld=NULL;	
	HPEN hPenStatus = NULL;
	hpen = CreatePen(PS_DOT, 1, RGB(64,64,204));
	hpenOld = (HPEN) SelectObject(hDC, hpen);
	hPenStatus = CreatePen(PS_SOLID, 1, RGB(0,255,0));
	SelectObject(hDC, hPenStatus);
    SetTextColor(hDC,0x00FFFFFF);
	TraceRT::iterator iLst;
	int i=1;
	for ( iLst = _TraceRT.begin(); iLst != _TraceRT.end(); iLst++ )
	{
		_TRACERT_STRUCT ts = *iLst;
		DWORD dwPingi = ts.RTT;
		MoveToEx(hDC,oldX,oldY,NULL);
		int fact = dwPingi / scale;
		int off=2;	
		if(i%2)
			off=-14;	

		Ellipse(hDC, oldX-3, oldY-3,oldX+3, oldY+3);
        

		int y = fact * offsetY;
		y+= dwPingi-(fact*scale);  //rest

		
		
		int yTot = rectInner.bottom-y;
		if(yTot<rectInner.top)
			yTot = rectInner.top;
		

		int x = rectInner.left+(offsetX*(i));
		
		if(x>rectInner.right)
			x = rectInner.right;

		LineTo(hDC,x,yTot);

		if(strlen(ts.szHostName)>0)
			MyDrawFont(hDC, oldX-30, oldY+off,ts.szHostName,0);	
		else
		{
		    struct in_addr  LastAddr = {0}; 
			LastAddr.s_addr = ts.IpAdress;
			MyDrawFont(hDC, oldX-30, oldY+off,inet_ntoa(LastAddr),0);	
		}

		oldX = x;
		oldY = yTot;
		i++;

	}
	SelectObject(hDC, hpenOld);
	DeleteObject(hPenStatus);
	DeleteObject(hpen);
	SetTextColor(hDC,0x00000000);
}
DWORD WINAPI TraceRoute_Thread(LPVOID lpParam)
{
	SERVER_INFO pSI;
	int n = ListView_GetSelectionMark(g_hwndListViewServer);
	if(n!=-1)
	{
		pSI  = Get_ServerInfoByListViewIndex(currCV,n);
		strcpy(szIPAddressToPing,pSI.szIPaddress);
		TraceRoute(pSI.szIPaddress);
	}
	//InvalidateRect(g_hwndMainSTATS,NULL,TRUE);
	return 0;
}


BOOL g_bPinging=FALSE;

LRESULT CALLBACK STATS_Proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC  hDC;
	RECT size;
	HBRUSH hbrBkgnd=NULL,hbrBar=NULL;
	switch (message)
	{
		case WM_INITDIALOG:
			strcpy(szIPAddressToPing,", right click on a server then click on Network tools->Ping server.");
			ShowWindow(hWnd,SW_HIDE);
			return TRUE;
			break;
		

		case WM_START_TRACERT:
			HANDLE hThread;
			SendMessage(g_hwndMainSTATS,WM_STOP_PING,0,0);

			hThread = CreateThread( NULL, 0, &TraceRoute_Thread, g_hWnd,0, NULL);                
			if (hThread == NULL) 
			{		
			}
			else 
				CloseHandle( hThread );

			break;
		case WM_START_PING:
			{
				QPing.clear();
				int n = ListView_GetSelectionMark(g_hwndListViewServer);
				if(n!=-1)
				{
					g_bPinging = TRUE;
					g_pSIPing  = Get_ServerInfoByListViewIndex(currCV,n);
					strcpy(szIPAddressToPing,g_pSIPing.szIPaddress);
					// TraceRoute(szIPAddressToPing);
					//if(strcmp(szIPAddressToPing,szOldIPAddressToPing)!=0)
					//	QPing.clear();				
					//strcpy(szOldIPAddressToPing,g_pSIPing.szIPaddress);

					SetTimer(hWnd,EVENT_PING,1000,NULL);

				} else
					strcpy(szIPAddressToPing,", Please select a server to ping.");
			}
			break;
	case WM_STOP_PING:
		g_bPinging = FALSE;
		KillTimer(hWnd,EVENT_PING);
		break;
	case WM_TIMER:
		{
			if(wParam==EVENT_PING)
			{
				DWORD pingi;
				MyPing(szIPAddressToPing,pingi);
				QPing.push_back(pingi);
				if(QPing.size()>150)
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
		
		if(g_bPinging==TRUE)
		{
			deQPing::reverse_iterator iLst;
			deQPing::iterator MaxElement;

			MaxElement = max_element ( QPing.begin ( ) , QPing.end ( ) );
			DWORD dwMaxPing = 0;
			if(MaxElement !=QPing.end())
				dwMaxPing = *MaxElement;

			Draw_GraphBackground(hWnd,hDC,12, 6,"Ping response from", "seconds" ,"milliseconds",dwMaxPing*2);
			Draw_PingStats( hWnd, hDC, 12, 6,dwMaxPing*2);
		}
		else
		{
			Draw_GraphBackground(hWnd,hDC, _TraceRT.size(), 10, "Trace route to","hops" ,"milliseconds",350,DRAW_GRAPH_X_LEFT_TO_RIGHT);
			Draw_TraceRouteStats( hWnd, hDC,  _TraceRT.size(), 10,350);
		}
/*
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
*/
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
	
	SERVER_INFO pSIa =  GamesInfo[rSIa.cGAMEINDEX].vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  GamesInfo[rSIb.cGAMEINDEX].vSI.at(rSIb.dwIndex);
	
	if(CUSTCOLUMNS[COL_PLAYERS].bSortAsc)
		return (pSIa.nCurrentPlayers > pSIb.nCurrentPlayers);
	else
		return (pSIa.nCurrentPlayers < pSIb.nCurrentPlayers);
}

bool Sort_Country(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  GamesInfo[rSIa.cGAMEINDEX].vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  GamesInfo[rSIb.cGAMEINDEX].vSI.at(rSIb.dwIndex);

	if(CUSTCOLUMNS[COL_COUNTRY].bSortAsc)
		return (strcmp(pSIa.szCountry , pSIb.szCountry )>0);
	else
	   return (strcmp(pSIa.szCountry , pSIb.szCountry )<0);
}

bool Sort_Ping(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  GamesInfo[rSIa.cGAMEINDEX].vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  GamesInfo[rSIb.cGAMEINDEX].vSI.at(rSIb.dwIndex);
	if(CUSTCOLUMNS[COL_PING].bSortAsc)	
		return (pSIa.dwPing > pSIb.dwPing);	
	else
		return (pSIa.dwPing < pSIb.dwPing);	
}

bool Sort_Map(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  GamesInfo[rSIa.cGAMEINDEX].vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  GamesInfo[rSIb.cGAMEINDEX].vSI.at(rSIb.dwIndex);
	
	if(CUSTCOLUMNS[COL_MAP].bSortAsc)	
		return (StrSorter(pSIa.szMap , pSIb.szMap )>0);
	else
		return (StrSorter(pSIa.szMap , pSIb.szMap )<0);
}

bool Sort_Mod(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  GamesInfo[rSIa.cGAMEINDEX].vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  GamesInfo[rSIb.cGAMEINDEX].vSI.at(rSIb.dwIndex);
	if(CUSTCOLUMNS[COL_MOD].bSortAsc)	
		return (StrSorter(pSIa.szMod , pSIb.szMod )>0);
	else
		return (StrSorter(pSIa.szMod , pSIb.szMod )<0);
}

bool Sort_IP(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  GamesInfo[rSIa.cGAMEINDEX].vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  GamesInfo[rSIb.cGAMEINDEX].vSI.at(rSIb.dwIndex);
	
	if(CUSTCOLUMNS[COL_IP].bSortAsc)
		return (StrSorter(pSIa.szIPaddress , pSIb.szIPaddress )>0);
	else
		return (StrSorter(pSIa.szIPaddress , pSIb.szIPaddress )<0);
}


bool Sort_Punkbuster(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  GamesInfo[rSIa.cGAMEINDEX].vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  GamesInfo[rSIb.cGAMEINDEX].vSI.at(rSIb.dwIndex);
	
	if(CUSTCOLUMNS[COL_PB].bSortAsc)
		return (pSIa.bPunkbuster > pSIb.bPunkbuster );
	else
		return (pSIa.bPunkbuster < pSIb.bPunkbuster );

}

bool Sort_Private(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  GamesInfo[rSIa.cGAMEINDEX].vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  GamesInfo[rSIb.cGAMEINDEX].vSI.at(rSIb.dwIndex);

	if(CUSTCOLUMNS[COL_PRIVATE].bSortAsc)
		return (pSIa.bPrivate > pSIb.bPrivate );	
	else
		return (pSIa.bPrivate < pSIb.bPrivate );	
}

bool Sort_Ranked(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  GamesInfo[rSIa.cGAMEINDEX].vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  GamesInfo[rSIb.cGAMEINDEX].vSI.at(rSIb.dwIndex);
	
	if(CUSTCOLUMNS[COL_RANKED].bSortAsc)
		return (pSIa.cRanked > pSIb.cRanked );	
	else
		return (pSIa.cRanked < pSIb.cRanked );	
}
bool Sort_Bots(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  GamesInfo[rSIa.cGAMEINDEX].vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  GamesInfo[rSIb.cGAMEINDEX].vSI.at(rSIb.dwIndex);
	
	if(CUSTCOLUMNS[COL_BOTS].bSortAsc)
		return (pSIa.cBots > pSIb.cBots );	
	else
		return (pSIa.cBots < pSIb.cBots );	
}

bool Sort_ServerName(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  GamesInfo[rSIa.cGAMEINDEX].vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  GamesInfo[rSIb.cGAMEINDEX].vSI.at(rSIb.dwIndex);

	if(CUSTCOLUMNS[COL_SERVERNAME].bSortAsc)
		return (StrSorter(pSIa.szServerName , pSIb.szServerName )>0);
	 else
		return (StrSorter(pSIa.szServerName , pSIb.szServerName )<0);

}
bool Sort_Status(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  GamesInfo[rSIa.cGAMEINDEX].vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  GamesInfo[rSIb.cGAMEINDEX].vSI.at(rSIb.dwIndex);

	if(CUSTCOLUMNS[COL_STATUS].bSortAsc)
		return (StrSorter(pSIa.szSTATUS , pSIb.szSTATUS )>0);
	 else
		return (StrSorter(pSIa.szSTATUS , pSIb.szSTATUS )<0);

}

bool Sort_Version(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  GamesInfo[rSIa.cGAMEINDEX].vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  GamesInfo[rSIb.cGAMEINDEX].vSI.at(rSIb.dwIndex);

	if(CUSTCOLUMNS[COL_VERSION].bSortAsc)
	{
		return (StrSorter(pSIa.szVersion , pSIb.szVersion )>0);
	}
	
	return (StrSorter(pSIa.szVersion , pSIb.szVersion )<0);
}


bool Sort_GameType(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO pSIa =  GamesInfo[rSIa.cGAMEINDEX].vSI.at(rSIa.dwIndex);
	SERVER_INFO pSIb =  GamesInfo[rSIb.cGAMEINDEX].vSI.at(rSIb.dwIndex);
	if(CUSTCOLUMNS[COL_GAMETYPE].bSortAsc)
		return (pSIa.dwGameType < pSIb.dwGameType);
	else
		return (pSIa.dwGameType > pSIb.dwGameType );
}
void Do_ServerListSortThread(int iColumn)
{
		HANDLE hThread=NULL; 
		DWORD dwThreadIdBrowser=0;				
		hThread = CreateThread( NULL, 0, &Do_ServerListSort, (LPVOID)iColumn,0, &dwThreadIdBrowser);                
		if (hThread == NULL) 
		{
			AddLogInfo(ETSV_WARNING, "CreateThread failed (%d) @ Do_ServerListSortThread\n", GetLastError() ); 
		}
		else 
		{			
			CloseHandle( hThread );
		}
}


DWORD WINAPI Do_ServerListSort(LPVOID column)
{
	if(currCV==NULL)
		return 0;
	int iColumn = (int)column;
	int gameIdx = currCV->cGAMEINDEX;
	
	if(TryEnterCriticalSection(&REDRAWLIST_CS)==FALSE)
	{
		dbg_print("busy RefList");
		return 0;
	}


	//We don't want to do sorting on the current scanning game index due to it is not thread safe.
/*	if((gameIdx==g_currentScanGameIdx) && g_bRunningQueryServerList)
	{
		dbg_print("Skipping sorting during scan.");	
		return ;
	}
*/


	BOOL sortdir = FALSE;
	dbg_print("idx %d\nList view Size %d",gameIdx,GamesInfo[gameIdx].vRefListSI.size());
	if(GamesInfo[gameIdx].vRefListSI.size()>0)
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
			case COL_PB : 
				try
				{	
					sort(GamesInfo[gameIdx].vRefListSI.begin(),GamesInfo[gameIdx].vRefListSI.end(),Sort_Punkbuster); 
				}catch (const exception& e)
				{
					AddLogInfo(ETSV_ERROR,"Vector err exception Details: %s",e.what());
					DebugBreak();
				}
				break;

			case COL_PRIVATE: 
				try
				{	
				sort(GamesInfo[gameIdx].vRefListSI.begin(),GamesInfo[gameIdx].vRefListSI.end(),Sort_Private);
							}catch (const exception& e)
				{
					AddLogInfo(ETSV_ERROR,"Vector err exception Details: %s",e.what());
					DebugBreak();
				}
	
				break;
			case COL_RANKED: 
				try
				{	
					sort(GamesInfo[gameIdx].vRefListSI.begin(),GamesInfo[gameIdx].vRefListSI.end(),Sort_Ranked); 
				}catch (const exception& e)
				{
					AddLogInfo(ETSV_ERROR,"Vector err exception Details: %s",e.what());
					DebugBreak();
				}
				break;
			case COL_SERVERNAME: 
				try
				{	
					sort(GamesInfo[gameIdx].vRefListSI.begin(),GamesInfo[gameIdx].vRefListSI.end(),Sort_ServerName); 
				}catch (const exception& e)
				{
					AddLogInfo(ETSV_ERROR,"Vector err exception Details: %s",e.what());
					DebugBreak();
				}
				break;
			case COL_GAMETYPE: 
				try{
					sort(GamesInfo[gameIdx].vRefListSI.begin(),GamesInfo[gameIdx].vRefListSI.end(),Sort_GameType); 
				}catch (const exception& e)
				{
					AddLogInfo(ETSV_ERROR,"Vector err exception Details: %s",e.what());
					DebugBreak();
				}
				break;

			case COL_MAP: 
				try{
					sort(GamesInfo[gameIdx].vRefListSI.begin(),GamesInfo[gameIdx].vRefListSI.end(),Sort_Map); 
				}catch (const exception& e)
				{
					AddLogInfo(ETSV_ERROR,"Vector err exception Details: %s",e.what());
					DebugBreak();
				}
				break;
			case COL_MOD: 
				try{
					sort(GamesInfo[gameIdx].vRefListSI.begin(),GamesInfo[gameIdx].vRefListSI.end(),Sort_Mod); 
				}catch (const exception& e)
				{
					AddLogInfo(ETSV_ERROR,"Vector err exception Details: %s",e.what());
					DebugBreak();
				}
				break;

			case COL_PLAYERS: 
				try{
					sort(GamesInfo[gameIdx].vRefListSI.begin(),GamesInfo[gameIdx].vRefListSI.end(),Sort_Player);
				}catch (const exception& e)
				{
					AddLogInfo(ETSV_ERROR,"Vector err exception Details: %s",e.what());
					DebugBreak();
				}
				break;
				
			case COL_COUNTRY: sort(GamesInfo[gameIdx].vRefListSI.begin(),GamesInfo[gameIdx].vRefListSI.end(),Sort_Country); break;
			case COL_PING: 
				try{
					sort(GamesInfo[gameIdx].vRefListSI.begin(),GamesInfo[gameIdx].vRefListSI.end(),Sort_Ping); 
				}catch (const exception& e)
				{
					AddLogInfo(ETSV_ERROR,"Vector err exception Details: %s",e.what());
					DebugBreak();
				}
				break;
			case COL_IP: 
				try{
				sort(GamesInfo[gameIdx].vRefListSI.begin(),GamesInfo[gameIdx].vRefListSI.end(),Sort_IP); 
				}catch (const exception& e)
				{
					AddLogInfo(ETSV_ERROR,"Vector err exception Details: %s",e.what());
					DebugBreak();
				}
				break;

			case COL_VERSION: 
				try{
				sort(GamesInfo[gameIdx].vRefListSI.begin(),GamesInfo[gameIdx].vRefListSI.end(),Sort_Version); 
								}catch (const exception& e)
				{
					AddLogInfo(ETSV_ERROR,"Vector err exception Details: %s",e.what());
					DebugBreak();
				}
				break;
			case COL_BOTS: 
				try{
					sort(GamesInfo[gameIdx].vRefListSI.begin(),GamesInfo[gameIdx].vRefListSI.end(),Sort_Bots);
				}catch (const exception& e)
				{
					AddLogInfo(ETSV_ERROR,"Vector err exception Details: %s",e.what());
					DebugBreak();
				}
				 break;
			case COL_STATUS: sort(GamesInfo[gameIdx].vRefListSI.begin(),GamesInfo[gameIdx].vRefListSI.end(),Sort_Status); 
				break;


		}

		ListView_SetHeaderSortImage(g_hwndListViewServer, iColumn,(BOOL) sortdir); //id =iColumn
		ListView_SetItemCount(g_hwndListViewServer,GamesInfo[gameIdx].vRefListSI.size());

	}
	LeaveCriticalSection(&REDRAWLIST_CS);

	return 1;
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
				return (StrSorter(ptr1->szTeam , ptr2->szTeam )>0);
			 else
				return (StrSorter(ptr1->szTeam , ptr2->szTeam )<0);
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
			
			SetWindowText(hDlg,lang.GetString("TitleSetPrivatePass"));
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
BOOL ListView_SL_OnGetDispInfoList(int ctrlid, NMHDR *pNMHDR)
{
	LVITEM *pLVItem;
	NMLVDISPINFO *pLVDI;
	SERVER_INFO *pSrvInf = NULL; 
	GAME_INFO *pGI = currCV;
	if (ctrlid!=IDC_LIST_SERVER)
		return FALSE;
	
	if(pNMHDR==NULL)
		return FALSE;

	pLVDI = (NMLVDISPINFO *)pNMHDR;

	if(pGI->bLockServerList)
		return FALSE;

	pLVItem = &pLVDI->item;
	int size = pGI->vRefListSI.size();
	if (size==0)
		return FALSE;

	if(pLVItem->iItem>=size)
		return FALSE;
	SERVER_INFO pSI;
	ZeroMemory(&pSI,sizeof(SERVER_INFO));
	REF_SERVER_INFO refSI;

	__try{
		refSI = pGI->vRefListSI.at((int)pLVItem->iItem);
		pGI->vSI.at(refSI.dwIndex).dwLVIndex = pLVItem->iItem;
		pSI = pGI->vSI.at(refSI.dwIndex);
	}
	__except(EXCEPTION_ACCESS_VIOLATION == GetExceptionCode())
	{
		dbg_print("Access violation...@ ListView_SL_OnGetDispInfoList");
		return FALSE;
	}
	//std::out_of_range

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
								pLVItem->iImage = 21;  //VAC/Steam icon
							
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
							pLVItem->iImage = Get_GameIcon(pGI->cGAMEINDEX);

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
							strncpy(pLVItem->pszText,Get_GameTypeNameByGameType(pSrvInf->cGAMEINDEX,pSrvInf->dwGameType),pLVItem->cchTextMax);
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
				case COL_STATUS:
					{
						strncpy(pLVItem->pszText,pSrvInf->szSTATUS,pLVItem->cchTextMax);
						return TRUE;
					break;
					}
			} 

		}

	}	
	return TRUE;
}

BOOL ListView_PL_OnGetDispInfoList(int ctrlid, NMHDR *pNMHDR)
{
	LVITEM *pLVItem;
	NMLVDISPINFO *pLVDI;

	
	if (ctrlid!=IDC_LIST_PLAYERS)
		return FALSE;
	
	if(pNMHDR==NULL)
		return FALSE;

	pLVDI = (NMLVDISPINFO *)pNMHDR;
	
	pLVItem = &pLVDI->item;

	PLAYERDATA *pPlayerData = pCurrentPL;
	if(pPlayerData!=NULL)
	{
		for(int i=0;i<pLVItem->iItem;i++)
			pPlayerData = pPlayerData->pNext;
	}

	if(pPlayerData==NULL)
		return FALSE;


	char szText[120];

	switch(pLVItem->iSubItem)
		{
			case 0:
				sprintf_s(szText,sizeof(szText)-1,"%d",pLVDI->item.iItem+1);
				pLVItem->pszText= szText;
				return TRUE;
			case 1:
				if(pPlayerData->szTeam!=NULL)
				{				
					char colFiltered[100];				
					if(GamesInfo[pPlayerData->cGAMEINDEX].colorfilter!=NULL)
						pLVItem->pszText = GamesInfo[pPlayerData->cGAMEINDEX].colorfilter(pPlayerData->szTeam,colFiltered,sizeof(colFiltered)-1); 
					else
						pLVItem->pszText = pPlayerData->szTeam;
				
				}
				return TRUE;
			case 2:
				{
				char colFiltered[120];	
				if( pPlayerData->szClanTag!=NULL)
					sprintf_s(szText,sizeof(szText)-1,"%s %s", pPlayerData->szClanTag,pPlayerData->szPlayerName);
				else
					sprintf_s(szText,sizeof(szText)-1,"%s",pPlayerData->szPlayerName);
				if(GamesInfo[pPlayerData->cGAMEINDEX].colorfilter!=NULL)
					pLVItem->pszText = GamesInfo[pPlayerData->cGAMEINDEX].colorfilter(szText,colFiltered,sizeof(colFiltered)-1); 
				else
					pLVItem->pszText = szText;
				return TRUE;
				}

			case 3:
				sprintf_s(szText,sizeof(szText)-1,"%d",pPlayerData->rate);
				pLVItem->pszText = szText;

				return TRUE;
			case 4:
				sprintf_s(szText,sizeof(szText)-1,"%d",pPlayerData->ping);
				pLVItem->pszText = szText;
				return TRUE;
			case 5:
				sprintf_s(szText,sizeof(szText)-1,"%d",pPlayerData->time);
				pLVItem->pszText = szText;
				return TRUE;
			case 6:
				{
				char colFiltered[100];
				
				SERVER_INFO pSI = GamesInfo[pPlayerData->cGAMEINDEX].vSI.at(pPlayerData->dwServerIndex);
				if(GamesInfo[pPlayerData->cGAMEINDEX].colorfilter!=NULL)
					pLVItem->pszText = GamesInfo[pPlayerData->cGAMEINDEX].colorfilter(pSI.szServerName,colFiltered,sizeof(colFiltered)-1); 
				else
					pLVItem->pszText = pSI.szServerName;
				return TRUE;
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
	if(RegCreateKeyEx(hkey, pszRegPath, 0, 0, REG_OPTION_NON_VOLATILE, KEY_READ, 0, &HKey, &dwOpen) == ERROR_SUCCESS) 
		{
			if(RegQueryValueEx(HKey,pszRegKey, 0, &dwType, (LPBYTE)pszOutputString, dwSizeOfBuffer) == ERROR_SUCCESS) 
			{
				RegCloseKey(HKey);		
				return pszOutputString;
			}
		RegCloseKey(HKey);
		}
//	AddGetLastErrorIntoLog("Registry_GetGamePath");
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
	
	if(pSrvInf->dwIndex<=GamesInfo[gametype].vSI.size())	
		g_tmpSRV = GamesInfo[gametype].vSI.at(pSrvInf->dwIndex);
	else
		AddLogInfo(ETSV_ERROR,"Vector request out of scope at %s %d",__FILE__,__LINE__);

	GamesInfo[gametype].GetServerStatus(pSrvInf,&UpdatePlayerList,&UpdateRulesList);

/*	switch(gametype)
	{	


		case ETQW_SERVERLIST:
		case Q4_SERVERLIST:
		{
			Q4_OnServerSelection(pSrvInf,&UpdatePlayerList,&UpdateRulesList);
			break;
		}
		case HL2_SERVERLIST:
		case CS_SERVERLIST:
		case CSCZ_SERVERLIST:
		case CSS_SERVERLIST:
		{			
			STEAM_OnServerSelection(pSrvInf,&UpdatePlayerList,&UpdateRulesList);
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
			Q3_OnServerSelection(pSrvInf,&UpdatePlayerList,&UpdateRulesList);
			break;
		}

	} //end switch
	*/
	return pSrvInf;
}

void OnServerSelection(GAME_INFO *pGI)
{
	SERVER_INFO *pSrvInf = NULL;
	int i = ListView_GetSelectionMark(g_hwndListViewServer);
	if(g_iCurrentSelectedServer==i)
		return;
	g_bRunningQuery = true;

	if(i!=-1)
	{
		g_iCurrentSelectedServer = i;
		SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
	
		ListView_DeleteAllItems(g_hwndListViewVars);
		ListView_DeleteAllItems(g_hwndListViewPlayers);
	
		try
		{
			g_CurrentSelServer = Get_ServerInfoByListViewIndex(currCV,g_iCurrentSelectedServer);
			if(g_CurrentSelServer.dwIP == 0)
				return;				
		}
		catch(const exception& e)
		{
			AddLogInfo(0,"Exception raised at OnServerSelection() Details:%s\n",e.what());
			return;
		}
		pSrvInf = &g_CurrentSelServer;
		if(pSrvInf!=NULL)
		{
			GetServerInfo(g_currentGameIdx,pSrvInf);
			UpdateServerItem(g_iCurrentSelectedServer);
			pGI->vSI.at(g_CurrentSelServer.dwIndex) = g_CurrentSelServer;
			//AddLogInfo(ETSV_DEBUG,"Num visible servers %d of %d\n",pGI->vRefListSI.size(), pGI->vSI.size());
			UpdateCurrentServerUI();
			
		}//if pSrv
	}
	
	UpdateWindow(g_hwndListViewServer);
	UpdateWindow(g_hwndListViewPlayers);
	UpdateWindow(g_hwndListViewVars);

	g_bRunningQuery = false;
}


void OnBuddySelected()
{
	BUDDY_INFO *pBI = NULL;
	
	int i = ListView_GetSelectionMark(g_hwndListBuddy);
	
	g_bRunningQuery = true;

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

						SERVER_INFO pSI = GamesInfo[pBI->cGAMEINDEX].vSI.at((int)pBI->sIndex);
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

	g_bRunningQuery = false;
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
	   strcat_s(path2,515,"\\GameScanner.exe");
   else
	   strcat_s(path2,515,"GameScanner.exe");
	
   sprintf(totpath,"%s %c1",path2,'%');

	//Register protocol
	//--------------------------------------

	for(int i=0; i<GamesInfo.size();i++)
	{
		hkey=NULL;
		sprintf(szBuffer,"%s",GamesInfo[i].szProtocolName);
		ret = RegCreateKeyEx(HKEY_CLASSES_ROOT, szBuffer, 0, NULL,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition ); 
		if(ret==ERROR_SUCCESS)
		{
			sprintf(szBuffer,"URL:%s",GamesInfo[i].szProtocolName);
			ret = RegSetValueEx( hkey, "", 0, REG_SZ, (const unsigned char*)szBuffer, (DWORD)strlen(szBuffer)); 
			memset(szBuffer,0,sizeof(szBuffer));
			ret = RegSetValueEx( hkey, "URL Protocol", 0, REG_SZ, (const unsigned char*)szBuffer, (DWORD)strlen(szBuffer)); 
			RegCloseKey( hkey ); 
		}
		sprintf(szBuffer,"%s\\shell\\open\\command",GamesInfo[i].szProtocolName);
		ret = RegCreateKeyEx(HKEY_CLASSES_ROOT, szBuffer, 0, NULL,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition ); 
		if(ret==ERROR_SUCCESS)
		{
			ret = RegSetValueEx( hkey, "", 0, REG_SZ, (const unsigned char*)totpath, strlen(totpath)); 
			RegCloseKey( hkey ); 
		}
	}
}

void Initialize_GameSettings()
{
	for(int i=0; i<GamesInfo.size(); i++)
	{
		ZeroMemory(&GamesInfo[i],sizeof(GAME_INFO));
		GamesInfo[i].cGAMEINDEX = i;
		GamesInfo[i].iIconIndex =  Get_GameIcon(i);
		GamesInfo[i].vGAME_INST.clear();		
	}
}

#define QW_ENGINE 100
#define Q3_ENGINE 101
#define Q4_ENGINE 102
#define VALVE_ENGINE 103



/***************************************************
	Set up default settings for each game.
****************************************************/
int GetNetEngine(char *szName)
{
	if(strcmp("Q3",szName)==0)
		return Q3_ENGINE;
	else if(strcmp("Q4",szName)==0)
		return Q4_ENGINE;
	else if(strcmp("VALVE",szName)==0)
		return VALVE_ENGINE;
	
	return Q3_ENGINE;  //Q3 as default
}




void Default_GameSettings2()
{
	GAME_INSTALLATIONS gi;
	
	for(int i=0; i<GamesInfo.size(); i++)
	{
		GamesInfo[i].bUseHTTPServerList = FALSE;
		GamesInfo[i].dwViewFlags = 0;
		GamesInfo[i].vGAME_INST.clear();
	}

	CXmlFile xml;
	xml.SetPath(EXE_PATH);	
	if(xml.load("gamedefaults.xml")==XMLFILE_ERROR_LOADING)
		return;
	//TixmlElement *ptempElement = xml.GetElementSafe(xml.m_pRootElement,"Version");
	char szVersion[10];
	xml.GetText(xml.m_pRootElement,"Version",szVersion,sizeof(szVersion));



	TiXmlElement *pGame = xml.GetElementSafe(xml.m_pRootElement,"Game");

	while(pGame!=NULL)
	{
		char szTemp[100];
		GAME_INFO gameinfo;
		
		gameinfo.bActive=0;
		gameinfo.dwTotalServers=0;
		gameinfo.dwScanIdx = 0;
		gameinfo.dwViewFlags = 0;
		
		ZeroMemory(&gameinfo.filter,sizeof(gameinfo.filter));
		ZeroMemory(&gameinfo.szGAME_NAME,sizeof(gameinfo.szGAME_NAME));
		ZeroMemory(&gameinfo.szMasterQueryString,sizeof(gameinfo.szMasterQueryString));
		ZeroMemory(&gameinfo.szMasterServerIP,sizeof(gameinfo.szMasterServerIP));

		ZeroMemory(&szTemp,sizeof(szTemp));
		xml.GetInteger(pGame,"GameIndex",(long*)&gameinfo.cGAMEINDEX);		
		gameinfo.vGAME_INST.clear();
	
		//gameinfo.iIconIndex = Get_GameIcon(gameinfo.cGAMEINDEX);
		xml.GetText(pGame,"Icon",szTemp,sizeof(szTemp)-1);
		gameinfo.iIconIndex  = LoadIconIntoImageList(szTemp);

		xml.GetText(pGame,"Name",gameinfo.szGAME_NAME,sizeof(gameinfo.szGAME_NAME)-1);
		xml.GetText(pGame,"NetEngine",szTemp,sizeof(szTemp)-1);
		gameinfo.GAME_ENGINE = GetNetEngine(szTemp);
		xml.GetText(pGame,"ShortName",gameinfo.szGAME_SHORTNAME,sizeof(gameinfo.szGAME_SHORTNAME)-1);
		xml.GetText(pGame,"Filename",gameinfo.szFilename,sizeof(gameinfo.szFilename)-1);
		xml.GetText(pGame,"WebProtocol",gameinfo.szProtocolName,sizeof(gameinfo.szProtocolName)-1);
		xml.GetInteger(pGame,"ServerDefaultPort",(long*)&gameinfo.dwDefaultPort);
		char szColorEnc[10];
		xml.GetText(pGame,"ColorEncoding",szColorEnc,sizeof(szColorEnc)-1);

		gameinfo.colorfilter = &colorfilter;
		gameinfo.Draw_ColorEncodedText = &Draw_ColorEncodedText;
		if(strcmp(szColorEnc,"QW")==0)
		{
			gameinfo.colorfilter = &colorfilterQW;
			gameinfo.Draw_ColorEncodedText = &Draw_ColorEncodedTextQW;
		} else if(strcmp(szColorEnc,"Q4")==0)
		{
			gameinfo.colorfilter = &colorfilterQ4;
			gameinfo.Draw_ColorEncodedText = &Draw_ColorEncodedTextQ4;
		}

		xml.GetText(pGame,"ServerDefaultPort",gameinfo.szMAP_MAPPREVIEW_PATH,sizeof(gameinfo.szMAP_MAPPREVIEW_PATH)-1);
		xml.GetText(pGame,"ServerInfoQuery",gameinfo.szServerRequestInfo,sizeof(gameinfo.szServerRequestInfo)-1);
		xml.GetText(pGame,"MasterQueryString",gameinfo.szMasterQueryString,sizeof(gameinfo.szMasterQueryString)-1);


		TiXmlElement *ptempMaster = xml.GetElementSafe(pGame,"MasterServers");
		xml.GetInteger(ptempMaster,"UseHTTP",(long*)&gameinfo.bUseHTTPServerList);

		xml.GetText(ptempMaster,"MasterServer",gameinfo.szMasterServerIP,sizeof(gameinfo.szMasterServerIP)-1);

		if(gameinfo.bUseHTTPServerList==FALSE)
			SplitIPandPORT(gameinfo.szMasterServerIP,gameinfo.dwMasterServerPORT);

		char szKey[256];
		char szItem[100];
		char szInstallSuffix[100];
		
		ZeroMemory(szKey,sizeof(szKey));
		ZeroMemory(szItem,sizeof(szItem));
		ZeroMemory(szInstallSuffix,sizeof(szInstallSuffix));
		TiXmlElement *ptempDetection = xml.GetElementSafe(pGame,"Detection");
		TiXmlElement *ptempRegistry = xml.GetElementSafe(ptempDetection,"Registry");
		xml.GetText(ptempRegistry,"RegRoot",szTemp,sizeof(szTemp)-1);
		xml.GetText(ptempRegistry,"RegKey",szKey,sizeof(szKey)-1);
		xml.GetText(ptempRegistry,"RegItem",szItem,sizeof(szItem)-1);
		xml.GetText(ptempRegistry,"InstallSuffix",szInstallSuffix,sizeof(szInstallSuffix)-1);

		char szPath[MAX_PATH*2];
		ZeroMemory(szPath,sizeof(szPath));
		DWORD dwBuffSize = sizeof(szPath);

		HKEY hkey = HKEY_LOCAL_MACHINE;
		if(strcmp(szTemp,"HKEY_LOCAL_MACHINE")==0)
			hkey = HKEY_LOCAL_MACHINE;
		else if (strcmp(szTemp,"HKEY_CURRENT_USER")==0)
			hkey = HKEY_CURRENT_USER;
		else if (strcmp(szTemp,"HKEY_CLASSES_ROOT")==0)
			hkey = HKEY_CLASSES_ROOT;

		Registry_GetGamePath(hkey, szKey,szItem,szPath,&dwBuffSize);
		gameinfo.bActive = false;
		int len = strlen(szPath);
		int len2 = strlen(szInstallSuffix);
		if((len>0) && (len2>0)) //should we add suffix into game path?
		{
			gameinfo.bActive = true;
			strcat_s(szPath,sizeof(szPath),szInstallSuffix);
		}else if(len>0)
			gameinfo.bActive = true;

		TiXmlElement *ptempInstalls = xml.GetElementSafe(pGame,"Installs");

		while(ptempInstalls!=NULL)
		{
			TiXmlElement* pInstall = ptempInstalls->FirstChild("Install")->ToElement();
		
			xml.GetAttribute(pInstall,"Name",szTemp,sizeof(szTemp)-1);
			gi.sName = szTemp;
			if(strlen(szPath)>0)
				gi.szGAME_PATH = szPath;
			else
			{
				xml.GetAttribute(pInstall,"Path",szTemp,sizeof(szTemp)-1);
				gi.szGAME_PATH = szTemp;
			}
				
			xml.GetAttribute(pInstall,"Cmd",szTemp,sizeof(szTemp)-1);
			gi.szGAME_CMD = szTemp;
			xml.GetAttribute(pInstall,"LaunchByMod",szTemp,sizeof(szTemp)-1);
			gi.sMod = szTemp;
			xml.GetAttribute(pInstall,"LaunchByVer",szTemp,sizeof(szTemp)-1);
			gi.sVersion = szTemp;
			gameinfo.vGAME_INST.push_back(gi);

			ptempInstalls = ptempInstalls->NextSiblingElement();
			if(ptempInstalls==NULL)
				break;		
		}

	
		switch(gameinfo.GAME_ENGINE)
		{
			default:
			case Q3_ENGINE:
				{
					gameinfo.GetServersFromMasterServer = &Q3_ConnectToMasterServer;
					gameinfo.GetServerStatus = &Q3_Get_ServerStatus;
					break;
				}
			case Q4_ENGINE:
				{
					gameinfo.GetServersFromMasterServer = &Q4_ConnectToMasterServer;
					gameinfo.GetServerStatus = &Q4_Get_ServerStatus;
					break;
				}
			case VALVE_ENGINE:
				{
					gameinfo.colorfilter = NULL;
					gameinfo.Draw_ColorEncodedText = &Draw_ColorEncodedText;
					gameinfo.GetServersFromMasterServer = &STEAM_ConnectToMasterServer;
					gameinfo.GetServerStatus = &STEAM_Get_ServerStatus;
					break;
				}
		}


		GamesInfo[gameinfo.cGAMEINDEX] = gameinfo;
		pGame = pGame->NextSiblingElement();
		if(pGame==NULL)
			break;		
	}
	//GameEngine dependent
}

void Default_GameSettings()
{
	Default_GameSettings2();
	return;
/*	GAME_INSTALLATIONS gi;

	for(int i=0; i<GamesInfo.size(); i++)
	{
		GamesInfo[i].bUseHTTPServerList = FALSE;
		GamesInfo[i].szGAME_PATH[0]=0; //quick erase
		GamesInfo[i].dwViewFlags = 0;	
		GamesInfo[i].vGAME_INST.clear();
	}
*/
/*
XML prototype: gamedefaults.xml

<Games>
	<Version>1.0</Version>
	<Game>
		<GameIndex>0</GameIndex>
		<Name>Wolfenstein - Enemy Territory</Name>
		<ShortName>W:ET</ShortName>
		<Filename>et.servers</Filename>
		<WebProtocol>et</WebProtocol>
		<MasterServers>
			<MasterServer>etmaster.idsoftware.com:27950</MasterServer>
			<ServerProtocol>84</ServerProtocol>
			<MasterServerQuery>Q3</MasterServerQuery> <!-- Q3, Q4, STEAMv1, STEAMv2, HTTP -->
		</MasterServers>
		<ServerDefaultPort>27960</ServerDefaultPort>
		<MasterQueryString>\xFF\xFF\xFF\xFFgetstatus\n</MasterQueryString>
		<MasterExtendedQueryString></MasterExtendedQueryString>
		<ServerQuery>Q3</ServerQuery>
		<ColorFilter>Q3</ColorFilter>
		<ColorEncoder>Q3</ColorEncoder>
		<Installs>
				<Install name="Name" value="Day of Defeat Source" />
				<Install name="Path" value="HL2.EXE" />
				<Install name="Cmd" value="-applaunch 300" />
				<Install name="LaunchByMod" value="dod" />
				<Install name="LaunchByVer" value="" />
		</Installs>	  
		<Detection>
			<Registry>
				<RegRoot>HKEY_LOCAL_MACHINE</RegRoot>
				<RegKey>SOFTWARE\Activision\Wolfenstein - Enemy Territory</RegKey>
				<RegItem>InstallPath</RegItem>
			</Registry>
			<FileSearch>
				<FileName></FileName>
				<FilePath>
				</FilePath>
			</FileSearch>
		</Detection>
	</Game>
</Games>

*/
/*	int idx = ET_SERVERLIST;
	//GameEngine dependent
	GamesInfo[idx].colorfilter = &colorfilter;
	GamesInfo[idx].Draw_ColorEncodedText = &Draw_ColorEncodedText;
	strcpy(GamesInfo[idx].szServerRequestInfo,"\xFF\xFF\xFF\xFFgetstatus\n");
	GamesInfo[idx].GetServersFromMasterServer = &Q3_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &Q3_Get_ServerStatus;
	GamesInfo[idx].GAME_ENGINE = Q3_ENGINE;
	GamesInfo[idx].cGAMEINDEX = idx;
	GamesInfo[idx].dwMasterServerPORT = 27950;
	GamesInfo[idx].dwProtocol = 84;
	strncpy(GamesInfo[idx].szGAME_NAME,"Wolfenstein - Enemy Territory",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"etmaster.idsoftware.com",MAX_PATH);
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"etmaps",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_CMD,"",MAX_PATH);
	strcpy_s(GamesInfo[idx].szProtocolName,sizeof(GamesInfo[idx].szProtocolName),"et");
	GamesInfo[idx].dwDefaultPort = 27960;
	strcpy(GamesInfo[idx].szMasterQueryString,"");
	
	DWORD dwBuffSize = sizeof(GamesInfo[idx].szGAME_PATH);
	Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Activision\\Wolfenstein - Enemy Territory","InstallPath",GamesInfo[idx].szGAME_PATH,&dwBuffSize);

	if(strlen(GamesInfo[idx].szGAME_PATH)>0)
	{
		GamesInfo[idx].bActive = true;
		strcat_s(GamesInfo[idx].szGAME_PATH,sizeof(GamesInfo[idx].szGAME_PATH),"\\et.exe");
	}
	else
		GamesInfo[idx].bActive = false;
	
	gi.sName = "Default";
	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = GamesInfo[idx].szGAME_CMD;
	GamesInfo[idx].vGAME_INST.push_back(gi);
*/
/*	idx = Q3_SERVERLIST;
	GamesInfo[idx].GetServersFromMasterServer = &Q3_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &Q3_Get_ServerStatus;
	strcpy(GamesInfo[idx].szServerRequestInfo,"\xFF\xFF\xFF\xFFgetstatus\n");
	GamesInfo[idx].cGAMEINDEX = idx;
	GamesInfo[idx].dwMasterServerPORT = 27950;
	GamesInfo[idx].dwProtocol = 68;
	strncpy(GamesInfo[idx].szGAME_NAME,"Quake 3: Arena",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"monster.idsoftware.com",MAX_PATH);
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"q3maps",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_PATH,"Quake3.exe",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_CMD,"",MAX_PATH);
	strcpy(GamesInfo[idx].szProtocolName,"q3");
	GamesInfo[idx].dwDefaultPort = 27960;
	strcpy(GamesInfo[idx].szMasterQueryString,"");
	Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Id\\Quake III Arena","INSTALLPATH",GamesInfo[idx].szGAME_PATH,&dwBuffSize);
	if(strlen(GamesInfo[idx].szGAME_PATH)>0)
		GamesInfo[idx].bActive = true;
	else
		GamesInfo[idx].bActive = false;

	GamesInfo[idx].colorfilter = &colorfilter;
	GamesInfo[idx].Draw_ColorEncodedText = &Draw_ColorEncodedText;

	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = GamesInfo[idx].szGAME_CMD;
	GamesInfo[idx].vGAME_INST.push_back(gi);

	idx = GamesInfo[RTCW_SERVERLIST].cGAMEINDEX = RTCW_SERVERLIST;
	GamesInfo[idx].GetServersFromMasterServer = &Q3_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &Q3_Get_ServerStatus;	
	strcpy(GamesInfo[idx].szServerRequestInfo,"\xFF\xFF\xFF\xFFgetstatus\n");
	GamesInfo[idx].dwMasterServerPORT = 27950;
	GamesInfo[idx].dwProtocol = 60;
	GamesInfo[idx].dwViewFlags = 0;
	strncpy(GamesInfo[idx].szGAME_NAME,"Return To Castle of Wolfenstein",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"wolfmotd.idsoftware.com",MAX_PATH);
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"rtcwmaps",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_PATH,"C:\\Program Files\\Return to Castle Wolfenstein\\WolfMP.exe",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_CMD,"",MAX_PATH);
	strcpy(GamesInfo[idx].szProtocolName,"rtcw");
	GamesInfo[idx].bActive = false;
	GamesInfo[idx].dwDefaultPort = 27960;
	strcpy(GamesInfo[idx].szMasterQueryString,"");
	GamesInfo[idx].colorfilter = &colorfilter;
	GamesInfo[idx].Draw_ColorEncodedText = &Draw_ColorEncodedText;
	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = GamesInfo[idx].szGAME_CMD;
	GamesInfo[idx].vGAME_INST.push_back(gi);


	idx = GamesInfo[Q4_SERVERLIST].cGAMEINDEX = Q4_SERVERLIST;
	GamesInfo[idx].GetServersFromMasterServer = &Q4_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &Q4_Get_ServerStatus;

	GamesInfo[idx].dwMasterServerPORT = 27650;
	GamesInfo[idx].dwProtocol = 0;
	GamesInfo[idx].dwViewFlags = 0;
	strncpy(GamesInfo[idx].szGAME_NAME,"Quake 4",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"q4master.idsoftware.com",MAX_PATH);
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"q4maps",MAX_PATH);
	strcpy(GamesInfo[idx].szProtocolName,"q4");
	strncpy(GamesInfo[idx].szGAME_PATH,"Quake4.exe",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_CMD,"+seta com_allowconsole 1",MAX_PATH);
	Registry_GetGamePath(HKEY_LOCAL_MACHINE, "HKEY_LOCAL_MACHINE\\Software\\Id\\Quake 4","InstallPath",GamesInfo[idx].szGAME_PATH,&dwBuffSize);
	if(strlen(GamesInfo[Q3_SERVERLIST].szGAME_PATH)>0)
		GamesInfo[idx].bActive = true;
	else
		GamesInfo[idx].bActive = false;

	GamesInfo[idx].dwDefaultPort = 28004;
	GamesInfo[idx].colorfilter = &colorfilterQ4;
	GamesInfo[idx].Draw_ColorEncodedText = &Draw_ColorEncodedTextQ4;
 
	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = GamesInfo[idx].szGAME_CMD;
	GamesInfo[idx].vGAME_INST.push_back(gi);


	idx = GamesInfo[ETQW_SERVERLIST].cGAMEINDEX = ETQW_SERVERLIST;
	GamesInfo[idx].GetServersFromMasterServer = &Q4_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &Q4_Get_ServerStatus;

	GamesInfo[idx].dwViewFlags = 0;
	strncpy(GamesInfo[idx].szGAME_NAME,"Enemy Territory - Quake Wars",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"http://etqw-ipgetter.demonware.net/ipgetter/",MAX_PATH);
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"etqwmaps",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_CMD,"+seta com_usefastvidrestart 1 +seta com_allowconsole 1",MAX_PATH);
	dwBuffSize = sizeof(GamesInfo[idx].szGAME_PATH);
	GamesInfo[idx].bUseHTTPServerList = TRUE;

	Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Id\\ET - QUAKE Wars","EXEString",GamesInfo[idx].szGAME_PATH,&dwBuffSize);
	if(strlen(GamesInfo[idx].szGAME_PATH)>0)
		GamesInfo[idx].bActive = true;
	else
		GamesInfo[idx].bActive = false;

	strcpy(GamesInfo[idx].szProtocolName,"etqw");
	GamesInfo[idx].dwDefaultPort = 27733;
	GamesInfo[idx].colorfilter = &colorfilter;
	GamesInfo[idx].Draw_ColorEncodedText = &Draw_ColorEncodedText;
	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = GamesInfo[idx].szGAME_CMD;
	GamesInfo[idx].vGAME_INST.push_back(gi);


	idx = GamesInfo[COD2_SERVERLIST].cGAMEINDEX = COD2_SERVERLIST;
	GamesInfo[idx].GetServersFromMasterServer = &Q3_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &Q3_Get_ServerStatus;

	strcpy(GamesInfo[idx].szServerRequestInfo,"\xFF\xFF\xFF\xFFgetstatus\n");
	GamesInfo[idx].dwViewFlags = 0;
	strncpy(GamesInfo[idx].szGAME_NAME,"Call of Duty 2",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"cod2master.activision.com",MAX_PATH);
	GamesInfo[idx].dwMasterServerPORT = 20710;
	GamesInfo[idx].dwProtocol = 0;
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"cod2maps",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_CMD,"",MAX_PATH);
	dwBuffSize = sizeof(GamesInfo[idx].szGAME_PATH);
	Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Activision\\Call of Duty 2\\","MultiEXEString",GamesInfo[idx].szGAME_PATH,&dwBuffSize);
	if(strlen(GamesInfo[idx].szGAME_PATH)>0)
		GamesInfo[idx].bActive = true;
	else
		GamesInfo[idx].bActive = false;
	strcpy(GamesInfo[idx].szProtocolName,"cod2");
	GamesInfo[idx].dwDefaultPort = 28960;
	strcpy(GamesInfo[idx].szMasterQueryString,"");
	GamesInfo[idx].colorfilter = &colorfilter;
	GamesInfo[idx].Draw_ColorEncodedText = &Draw_ColorEncodedText;
	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = GamesInfo[idx].szGAME_CMD;
	GamesInfo[idx].vGAME_INST.push_back(gi);


	idx = GamesInfo[COD_SERVERLIST].cGAMEINDEX = COD_SERVERLIST;
	GamesInfo[idx].GetServersFromMasterServer = &Q3_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &Q3_Get_ServerStatus;

	strcpy(GamesInfo[idx].szServerRequestInfo,"\xFF\xFF\xFF\xFFgetstatus\n");	
	GamesInfo[idx].dwViewFlags = 0;
	strncpy(GamesInfo[idx].szGAME_NAME,"Call of Duty",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"codmaster.activision.com",MAX_PATH);
	GamesInfo[idx].dwMasterServerPORT = 20510;
	GamesInfo[idx].dwProtocol = 5;
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"codmaps",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_CMD,"",MAX_PATH);
	dwBuffSize = sizeof(GamesInfo[idx].szGAME_PATH);
	Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Activision\\Call of Duty","InstallPath",GamesInfo[idx].szGAME_PATH,&dwBuffSize);
	if(strlen(GamesInfo[idx].szGAME_PATH)>0)
		GamesInfo[idx].bActive = true;
	else
		GamesInfo[idx].bActive = false;
	strcpy(GamesInfo[idx].szProtocolName,"cod");
	GamesInfo[idx].dwDefaultPort = 28960;
	strcpy(GamesInfo[idx].szMasterQueryString,"");
	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = GamesInfo[idx].szGAME_CMD;
	GamesInfo[idx].vGAME_INST.push_back(gi);


	idx = GamesInfo[WARSOW_SERVERLIST].cGAMEINDEX = WARSOW_SERVERLIST;
	GamesInfo[idx].GetServersFromMasterServer = &Q3_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &Q3_Get_ServerStatus;

	strcpy(GamesInfo[idx].szServerRequestInfo,"\xFF\xFF\xFF\xFFgetinfo");
	GamesInfo[idx].iIconIndex = Get_GameIcon(idx);
	GamesInfo[idx].dwViewFlags = 0;
	strncpy(GamesInfo[idx].szGAME_NAME,"Warsow",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"dpmaster.deathmask.net",MAX_PATH);
	GamesInfo[idx].dwMasterServerPORT = 27950;
	GamesInfo[idx].dwProtocol = 10;
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"warsowmaps",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_CMD,"",MAX_PATH);
	dwBuffSize = sizeof(GamesInfo[idx].szGAME_PATH);

	Registry_GetGamePath(HKEY_CLASSES_ROOT, "warsow\\DefaultIcon","",GamesInfo[idx].szGAME_PATH,&dwBuffSize);

	if(strlen(GamesInfo[idx].szGAME_PATH)>0)
	{
		GamesInfo[idx].bActive = true;
		char sztempi[MAX_PATH*2];
		strcpy(sztempi,GamesInfo[idx].szGAME_PATH);
		char* pos = strrchr(sztempi,'\\');
		if(pos!=NULL)
		{
			pos[1]=0;
		}
		sprintf(GamesInfo[idx].szGAME_CMD,"+set fs_usehomedir 1 +set fs_basepath \"%s\"",sztempi);
	}else
	{
		GamesInfo[idx].bActive = false;
		sprintf(GamesInfo[idx].szGAME_CMD,"");
	}
	strcpy(GamesInfo[idx].szMasterQueryString,"Warsow");
	strcpy(GamesInfo[idx].szProtocolName,"warsow");
	GamesInfo[idx].dwDefaultPort = 28960;
	GamesInfo[idx].colorfilter = &colorfilter;
	GamesInfo[idx].Draw_ColorEncodedText = &Draw_ColorEncodedText;
	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = GamesInfo[idx].szGAME_CMD;
	GamesInfo[idx].vGAME_INST.push_back(gi);

	idx = GamesInfo[COD4_SERVERLIST].cGAMEINDEX = COD4_SERVERLIST;
	GamesInfo[idx].GetServersFromMasterServer = &Q3_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &Q3_Get_ServerStatus;

	strcpy(GamesInfo[idx].szServerRequestInfo,"\xFF\xFF\xFF\xFFgetstatus\n");
	GamesInfo[idx].iIconIndex = Get_GameIcon(idx);
	GamesInfo[idx].dwViewFlags = 0;
	strncpy(GamesInfo[idx].szGAME_NAME,"Call of Duty 4",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"cod4master.activision.com",MAX_PATH);
	GamesInfo[idx].dwMasterServerPORT = 20810;
	GamesInfo[idx].dwProtocol = 0;
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"cod4maps",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_CMD,"+set cl_playintro 0 +set ui_skip_titlescreen 1 +set ui_skip_legalscreen 1",MAX_PATH);
	dwBuffSize = sizeof(GamesInfo[idx].szGAME_PATH);
	Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Activision\\Call of Duty 4","EXEStringM",GamesInfo[idx].szGAME_PATH,&dwBuffSize);
	if(strlen(GamesInfo[idx].szGAME_PATH)>0)
		GamesInfo[idx].bActive = true;
	else
		GamesInfo[idx].bActive = false;
	strcpy(GamesInfo[idx].szProtocolName,"cod4");
	GamesInfo[idx].dwDefaultPort = 28960;
	strcpy(GamesInfo[idx].szMasterQueryString,"");
	GamesInfo[idx].colorfilter = &colorfilter;
	GamesInfo[idx].Draw_ColorEncodedText = &Draw_ColorEncodedText;
	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = GamesInfo[idx].szGAME_CMD;
	GamesInfo[idx].vGAME_INST.push_back(gi);


	idx = GamesInfo[CS_SERVERLIST].cGAMEINDEX = CS_SERVERLIST;
	GamesInfo[idx].GetServersFromMasterServer = &STEAM_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &STEAM_Get_ServerStatus;

	GamesInfo[idx].iIconIndex = Get_GameIcon(idx);
	GamesInfo[idx].dwViewFlags = 0;
	strncpy(GamesInfo[idx].szGAME_NAME,"Counter-Strike",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"hl1master.steampowered.com",MAX_PATH);
	GamesInfo[idx].dwMasterServerPORT = 27010;
	GamesInfo[idx].dwProtocol = 0;
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"csmaps",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_CMD,"-game cstrike",MAX_PATH);  //http://developer.valvesoftware.com/wiki/Command_line
	dwBuffSize = sizeof(GamesInfo[idx].szGAME_PATH);
	//Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Activision\\Call of Duty 4","EXEStringM",GamesInfo[CSS_SERVERLIST].szGAME_PATH,&dwBuffSize);
	GamesInfo[idx].bActive = false;
	strcpy(GamesInfo[idx].szProtocolName,"cs");
	GamesInfo[idx].dwDefaultPort = 27015;
	strcpy(GamesInfo[idx].szMasterQueryString,"\\gamedir\\cstrike");
	GamesInfo[idx].pSC = &SC[idx];
	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = GamesInfo[idx].szGAME_CMD;
	GamesInfo[idx].vGAME_INST.push_back(gi);


	idx = GamesInfo[CSCZ_SERVERLIST].cGAMEINDEX = CSCZ_SERVERLIST;
	GamesInfo[idx].GetServersFromMasterServer = &STEAM_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &STEAM_Get_ServerStatus;

	GamesInfo[idx].iIconIndex = Get_GameIcon(idx);
	GamesInfo[idx].dwViewFlags = 0;
	strncpy(GamesInfo[idx].szGAME_NAME,"Counter-Strike: Condition Zero",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"hl1master.steampowered.com",MAX_PATH);
	GamesInfo[idx].dwMasterServerPORT = 27010;
	GamesInfo[idx].dwProtocol = 0;
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"csczmaps",MAX_PATH);
	strncpy(GamesInfo[CS_SERVERLIST].szGAME_CMD,"-game czero",MAX_PATH); 
	dwBuffSize = sizeof(GamesInfo[idx].szGAME_PATH);
	//Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Activision\\Call of Duty 4","EXEStringM",GamesInfo[CSS_SERVERLIST].szGAME_PATH,&dwBuffSize);
	GamesInfo[idx].bActive = false;
	strcpy(GamesInfo[idx].szProtocolName,"cscz");
	GamesInfo[idx].dwDefaultPort = 27015;
	strcpy(GamesInfo[idx].szMasterQueryString,"\\gamedir\\czero");
	GamesInfo[idx].pSC = &SC[idx];
	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = GamesInfo[idx].szGAME_CMD;
	GamesInfo[idx].vGAME_INST.push_back(gi);


	idx = GamesInfo[CSS_SERVERLIST].cGAMEINDEX = CSS_SERVERLIST;
	GamesInfo[idx].GetServersFromMasterServer = &STEAM_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &STEAM_Get_ServerStatus;

	GamesInfo[idx].iIconIndex = Get_GameIcon(idx);
	GamesInfo[idx].dwViewFlags = 0;
	strncpy(GamesInfo[idx].szGAME_NAME,"Counter-Strike: Source",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"hl2master.steampowered.com",MAX_PATH);
	GamesInfo[idx].dwMasterServerPORT = 27011;
	GamesInfo[idx].dwProtocol = 0;
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"cssmaps",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_CMD,"-game cstrike",MAX_PATH);  //http://developer.valvesoftware.com/wiki/Command_line
	dwBuffSize = sizeof(GamesInfo[idx].szGAME_PATH);
	strcpy(GamesInfo[idx].szGAME_PATH,"HL2.EXE");
	GamesInfo[idx].bActive = false;
	strcpy(GamesInfo[idx].szProtocolName,"css");
	GamesInfo[idx].dwDefaultPort = 28960;
	strcpy(GamesInfo[idx].szMasterQueryString,"\\gamedir\\cstrike");
	GamesInfo[idx].pSC = &SC[idx];
	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = GamesInfo[idx].szGAME_CMD;
	GamesInfo[idx].vGAME_INST.push_back(gi);

	
	idx = GamesInfo[QW_SERVERLIST].cGAMEINDEX = QW_SERVERLIST;
	GamesInfo[idx].GetServersFromMasterServer = &Q3_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &Q3_Get_ServerStatus;

	strcpy(GamesInfo[idx].szServerRequestInfo,"\xFF\xFF\xFF\xFFstatus\n");
	GamesInfo[idx].dwMasterServerPORT = 27000;
	GamesInfo[idx].dwProtocol = 0;
	GamesInfo[idx].dwViewFlags = 0;
	strncpy(GamesInfo[idx].szGAME_NAME,"Quake World",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"http://www.quakeservers.net/lists/servers/global.txt",MAX_PATH);  //satan.idsoftware.com:27000
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"qwmaps",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_CMD,"",MAX_PATH);
	GamesInfo[idx].bActive = false;
	strcpy_s(GamesInfo[idx].szGAME_PATH,sizeof(GamesInfo[idx].szGAME_PATH),"quakeworld.exe");
	strcpy_s(GamesInfo[idx].szProtocolName,sizeof(GamesInfo[idx].szProtocolName),"qw");
	GamesInfo[idx].dwDefaultPort = 27960;
	strcpy(GamesInfo[idx].szMasterQueryString,"");
	GamesInfo[idx].bUseHTTPServerList = TRUE;
	GamesInfo[idx].colorfilter = &colorfilterQW;
	GamesInfo[idx].Draw_ColorEncodedText = &Draw_ColorEncodedTextQW;
	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = GamesInfo[idx].szGAME_CMD;
	GamesInfo[idx].vGAME_INST.push_back(gi);

	idx = GamesInfo[Q2_SERVERLIST].cGAMEINDEX = Q2_SERVERLIST;
	GamesInfo[idx].GetServersFromMasterServer = &Q3_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &Q3_Get_ServerStatus;
	strcpy(GamesInfo[idx].szServerRequestInfo,"\xFF\xFF\xFF\xFFstatus\n");
	GamesInfo[idx].dwMasterServerPORT = 27900;
	GamesInfo[idx].dwProtocol = 34;
	GamesInfo[idx].dwViewFlags = 0;
	strncpy(GamesInfo[idx].szGAME_NAME,"Quake 2",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"master.q2servers.com",MAX_PATH); //master.q2servers.com:27900
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"q2maps",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_CMD,"",MAX_PATH);
	GamesInfo[idx].bActive = false;
	strcpy_s(GamesInfo[idx].szGAME_PATH,sizeof(GamesInfo[idx].szGAME_PATH),"quake2.exe");
	strcpy_s(GamesInfo[idx].szProtocolName,sizeof(GamesInfo[idx].szProtocolName),"q2");
	GamesInfo[idx].dwDefaultPort = 27960;
	strcpy(GamesInfo[idx].szMasterQueryString,"");
	GamesInfo[idx].bUseHTTPServerList = FALSE;
	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = GamesInfo[idx].szGAME_CMD;
	GamesInfo[idx].vGAME_INST.push_back(gi);

	idx = GamesInfo[OPENARENA_SERVERLIST].cGAMEINDEX = OPENARENA_SERVERLIST;
	GamesInfo[idx].GetServersFromMasterServer = &Q3_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &Q3_Get_ServerStatus;
	strcpy(GamesInfo[idx].szServerRequestInfo,"\xFF\xFF\xFF\xFFgetstatus\n");	
	GamesInfo[idx].iIconIndex = Get_GameIcon(idx);
	GamesInfo[idx].dwViewFlags = 0;
	strncpy(GamesInfo[idx].szGAME_NAME,"Open Arena",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"dpmaster.deathmask.net",MAX_PATH);
	GamesInfo[idx].dwMasterServerPORT = 27950;
	GamesInfo[idx].dwProtocol = 69;
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"openarenamaps",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_CMD,"",MAX_PATH);
	dwBuffSize = sizeof(GamesInfo[idx].szGAME_PATH);
	GamesInfo[idx].bUseHTTPServerList = FALSE;
	strcpy(GamesInfo[idx].szMasterQueryString,"openarena");
	strcpy(GamesInfo[idx].szProtocolName,"openarena");
	GamesInfo[idx].dwDefaultPort = 28960;	
	strcpy(GamesInfo[idx].szGAME_PATH,"openarena.exe");
	strcpy(GamesInfo[idx].szGAME_CMD,"");
	GamesInfo[idx].bActive = false;
	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = GamesInfo[idx].szGAME_CMD;
	GamesInfo[idx].vGAME_INST.push_back(gi);

//"C:\Program Files\Steam\Steam.exe" -applaunch 300
	idx = GamesInfo[HL2_SERVERLIST].cGAMEINDEX = HL2_SERVERLIST;
	GamesInfo[idx].GetServersFromMasterServer = &STEAM_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &STEAM_Get_ServerStatus;
	GamesInfo[idx].iIconIndex = Get_GameIcon(idx);
	GamesInfo[idx].dwViewFlags = 0;
	strncpy(GamesInfo[idx].szGAME_NAME,"Half-Life 2",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"hl2master.steampowered.com",MAX_PATH);
	GamesInfo[idx].dwMasterServerPORT = 27011;
	GamesInfo[idx].dwProtocol = 0;
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"hf2maps",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_CMD,"-game %MODNAME%",MAX_PATH);  //http://developer.valvesoftware.com/wiki/Command_line
	dwBuffSize = sizeof(GamesInfo[idx].szGAME_PATH);
	strcpy(GamesInfo[idx].szGAME_PATH,"HL2.EXE");
	GamesInfo[idx].bActive = false;
	strcpy(GamesInfo[idx].szProtocolName,"hf2");
	GamesInfo[idx].dwDefaultPort = 28960;
	strcpy(GamesInfo[idx].szMasterQueryString,"");
	GamesInfo[idx].bUseHTTPServerList = FALSE;
	GamesInfo[idx].pSC = &SC[idx];

	Registry_GetGamePath(HKEY_CURRENT_USER, "Software\\Valve\\Steam","SteamExe",GamesInfo[idx].szGAME_PATH,&dwBuffSize);
	if(strlen(GamesInfo[idx].szGAME_PATH)>0)
		GamesInfo[idx].bActive = true;

	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = "-applaunch 300";
	gi.sMod = "dod";
	gi.sName = "Day of Defeat Source";
	GamesInfo[idx].vGAME_INST.push_back(gi);
	gi.szGAME_CMD = "-applaunch 320";
	gi.sMod = "hl2mp";
	gi.sName = "Half-Life 2 Deathmatch";
	GamesInfo[idx].vGAME_INST.push_back(gi);
	gi.szGAME_CMD = "-applaunch 240";
	gi.sMod = "cstrike";
	gi.sName = "Counter-Strike Source";
	GamesInfo[idx].vGAME_INST.push_back(gi);
	gi.szGAME_CMD = "-applaunch 440";
	gi.sMod = "tf";
	gi.sName = "Team Fortress 2";
	GamesInfo[idx].vGAME_INST.push_back(gi);

	idx = GamesInfo[UTERROR_SERVERLIST].cGAMEINDEX = UTERROR_SERVERLIST;
	GamesInfo[idx].GetServersFromMasterServer = &Q3_ConnectToMasterServer;
	GamesInfo[idx].GetServerStatus = &Q3_Get_ServerStatus;
	strcpy(GamesInfo[idx].szServerRequestInfo,"\xFF\xFF\xFF\xFFgetstatus\n");
	GamesInfo[idx].dwMasterServerPORT = 27950;
	GamesInfo[idx].dwProtocol = 68;
	GamesInfo[idx].iIconIndex =  Get_GameIcon(idx);
	GamesInfo[idx].dwViewFlags = 0;
	strncpy(GamesInfo[idx].szGAME_NAME,"Urban Terror",MAX_PATH);
	strncpy(GamesInfo[idx].szMasterServerIP,"master.urbanterror.net",MAX_PATH);
	strncpy(GamesInfo[idx].szMAP_MAPPREVIEW_PATH,"urbanmaps",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_PATH,"q3.exe",MAX_PATH);
	strncpy(GamesInfo[idx].szGAME_CMD,"+fs_game %MODNAME%",MAX_PATH);
	strcpy(GamesInfo[idx].szProtocolName,"urban");
	GamesInfo[idx].bActive = false;
	GamesInfo[idx].dwDefaultPort = 27960;
	GamesInfo[idx].bUseHTTPServerList = FALSE;
	strcpy(GamesInfo[idx].szMasterQueryString,"");
	GamesInfo[idx].pSC = &SC[idx];
	Registry_GetGamePath(HKEY_LOCAL_MACHINE, "SOFTWARE\\Id\\Quake III Arena","INSTALLPATH",GamesInfo[idx].szGAME_PATH,&dwBuffSize);
	GamesInfo[idx].colorfilter = &colorfilter;
	GamesInfo[idx].Draw_ColorEncodedText = &Draw_ColorEncodedText;
	gi.sName = "Default";
	gi.sMod = "";
	gi.sVersion = "";
	gi.szGAME_PATH = GamesInfo[idx].szGAME_PATH;
	gi.szGAME_CMD = GamesInfo[idx].szGAME_CMD;
	GamesInfo[idx].vGAME_INST.push_back(gi);
	strcpy(GamesInfo[idx].szGAME_SHORTNAME,"Urban Terror");
	strcpy(GamesInfo[idx].szFilename,"urbanterror.servers");


	strcpy(GamesInfo[ET_SERVERLIST].szGAME_SHORTNAME,"W:ET");
	strcpy(GamesInfo[ETQW_SERVERLIST].szGAME_SHORTNAME,"ETQW");
	strcpy(GamesInfo[Q3_SERVERLIST].szGAME_SHORTNAME,"Quake 3");			
	strcpy(GamesInfo[Q4_SERVERLIST].szGAME_SHORTNAME,"Quake 4");
	strcpy(GamesInfo[RTCW_SERVERLIST].szGAME_SHORTNAME,"RTCW");
	strcpy(GamesInfo[COD_SERVERLIST].szGAME_SHORTNAME,"CoD");
	strcpy(GamesInfo[COD2_SERVERLIST].szGAME_SHORTNAME,"CoD 2");	
	strcpy(GamesInfo[WARSOW_SERVERLIST].szGAME_SHORTNAME,"Warsow");
	strcpy(GamesInfo[COD4_SERVERLIST].szGAME_SHORTNAME,"CoD 4");
	strcpy(GamesInfo[CS_SERVERLIST].szGAME_SHORTNAME,"CS");
	strcpy(GamesInfo[CSCZ_SERVERLIST].szGAME_SHORTNAME,"CS Zero");
	strcpy(GamesInfo[CSS_SERVERLIST].szGAME_SHORTNAME,"CS Source");
	strcpy(GamesInfo[QW_SERVERLIST].szGAME_SHORTNAME,"Quake World");
	strcpy(GamesInfo[Q2_SERVERLIST].szGAME_SHORTNAME,"Quake 2");
	strcpy(GamesInfo[OPENARENA_SERVERLIST].szGAME_SHORTNAME,"Open Arena");
	strcpy(GamesInfo[HL2_SERVERLIST].szGAME_SHORTNAME,"HF2");

	strcpy(GamesInfo[ET_SERVERLIST].szFilename,"et.servers");
	strcpy(GamesInfo[ETQW_SERVERLIST].szFilename,"etqw.servers");
	strcpy(GamesInfo[Q4_SERVERLIST].szFilename,"q4.servers");
	strcpy(GamesInfo[Q3_SERVERLIST].szFilename,"q3.servers");
	strcpy(GamesInfo[RTCW_SERVERLIST].szFilename,"rtcw.servers");
	strcpy(GamesInfo[COD_SERVERLIST].szFilename,"cod.servers");
	strcpy(GamesInfo[COD2_SERVERLIST].szFilename,"cod2.servers");
	strcpy(GamesInfo[WARSOW_SERVERLIST].szFilename,"warsow.servers");
	strcpy(GamesInfo[COD4_SERVERLIST].szFilename,"cod4.servers");
	strcpy(GamesInfo[CS_SERVERLIST].szFilename,"cs.servers");
	strcpy(GamesInfo[QW_SERVERLIST].szFilename,"qw.servers");
	strcpy(GamesInfo[Q2_SERVERLIST].szFilename,"q2.servers");
	strcpy(GamesInfo[CSCZ_SERVERLIST].szFilename,"cscz.servers");
	strcpy(GamesInfo[CSS_SERVERLIST].szFilename,"css.servers");
	strcpy(GamesInfo[OPENARENA_SERVERLIST].szFilename,"openarena.servers");
	strcpy(GamesInfo[HL2_SERVERLIST].szFilename,"tf2.servers");
	
	RegisterProtocol(EXE_PATH);
	*/
}

void Default_Appsettings()
{

	AddLogInfo(ETSV_INFO,"Settings set to defaults.");
	
	ZeroMemory(&AppCFG,sizeof(APP_SETTINGS_NEW));

	g_sMIRCoutput = "is joining server %SERVERNAME% %GAMENAME% %IP% %PRIVATE%";
	
	AppCFG.dwVersion = 14;
	AppCFG.bAutostart = FALSE;
	AppCFG.bUse_minimize = TRUE;
	AppCFG.dwMinimizeMODKey =MOD_ALT;
	AppCFG.cMinimizeKey = 'Z';
	AppCFG.bLogging = FALSE;
	AppCFG.bUSE_SCREEN_RESTORE = FALSE;

	AppCFG.bUseColorEncodedFont = TRUE;
	AppCFG.bUseShortCountry = FALSE;	

	memset(AppCFG.szEXT_EXE_CMD,0,MAX_PATH);	
	memset(AppCFG.szEXT_EXE_PATH,0,MAX_PATH);
	
	AppCFG.bUse_EXT_APP = FALSE;	
	strcpy(AppCFG.szLanguageFilename,"lang_en.xml");
	strcpy(AppCFG.szEXT_EXE_PATH,"C:\\Program Files\\Teamspeak2_RC2\\TeamSpeak.exe");
	strcpy(AppCFG.szEXT_EXE_CMD,"127.0.0.1?nickname=MyNick?loginname=MyLoginAccount?password=XYZ?channel=Axis");
	strcpy(AppCFG.szEXT_EXE_WINDOWNAME,"TEAMSPEAK 2");
	strcpy(AppCFG.szET_WindowName,"Enemy Territory|Wolfenstein|Quake4|F.E.A.R.|ETQW|Warsow|Call of Duty 4");
	strcpy(AppCFG.szET_CMD,"");

	//Legacy stuff - this should be cleared out from source code...
	AppCFG.filter.bNoEmpty = FALSE;
	AppCFG.filter.bNoFull = FALSE;
	AppCFG.filter.bNoPrivate =FALSE;
	AppCFG.filter.bPunkbuster = FALSE;
	AppCFG.filter.bPure = FALSE;
	AppCFG.filter.bRanked = FALSE;
	AppCFG.filter.bNoBots = FALSE;	
	AppCFG.filter.dwGameTypeFilter = 0;

	//Global filter defaults
	AppCFG.filter.dwPing = 9999;
	AppCFG.filter.dwShowServerWithMaxPlayers = 24;
	AppCFG.filter.dwShowServerWithMinPlayers = 6;
	AppCFG.filter.cActiveMaxPlayer = 0;
	AppCFG.filter.cActiveMinPlayer = 0;

	//Default Network
	AppCFG.dwThreads = 32;
	AppCFG.socktimeout.tv_sec = 1;
	AppCFG.socktimeout.tv_usec  = 0;
	AppCFG.dwRetries = 0;

	AppCFG.g_cTransparancy = 100;


	AppCFG.cBuddyColumnSort = 0; 
	
	AppCFG.bShowBuddyList = TRUE;
	AppCFG.bShowMapPreview = FALSE;
	AppCFG.bShowServerRules = TRUE;
	AppCFG.bShowPlayerList = TRUE;
	
	AppCFG.bSortBuddyAsc = TRUE;
	AppCFG.bPlayNotifySound   = TRUE;
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
	WNDCONT[WIN_MAPPREVIEW].bShow = FALSE;

}

void OnButtonClick_AddServer()
{
	SERVER_INFO *pSrvInf = NULL;

	char ip[100];
	DWORD dwPort=0;
	ZeroMemory(ip,sizeof(ip));
	GetDlgItemText(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD,ip,99);

	if(strlen(ip)==0)
	{
		Favorite_Add(true);
		return;
	}else
		Favorite_Add(true,ip);
}



void OnServerDoubleClick()
{
	int i = ListView_GetSelectionMark(g_hwndListViewServer);
	if(i!=-1)
	{
		SERVER_INFO pSrv;	
		pSrv = Get_ServerInfoByListViewIndex(currCV,i);
		LaunchGame(pSrv,&GamesInfo[g_currentGameIdx]);

	}
}

SERVER_INFO *FindServerByIPandPort(char *szIP, DWORD dwPort)
{
	vSRV_INF::iterator  iLst;
	char destPort[10];
	DWORD dwIP = NetworkNameToIP(szIP,_itoa(dwPort,destPort,10));
	if(currCV->vSI.size()>0)
	{
		for ( iLst = currCV->vSI.begin( ); iLst != currCV->vSI.end( ); iLst++ )
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

	 iResult = find_if(pGI->vSI.begin(), pGI->vSI.end(),duplicate_func);

	 if(iResult == pGI->vSI.end())
		 return -1;

	 return iResult->dwIndex;
}

DWORD AddServer(GAME_INFO *pGI,char *szIP, DWORD dwPort,bool bFavorite)
{
	SERVER_INFO pSI;
	ZeroMemory(&pSI,sizeof(SERVER_INFO));
	char destPort[10];
	
	if(strlen(szIP)<7)
		return 0xFFFFFFFF;
	

	pSI.cGAMEINDEX = pGI->cGAMEINDEX;
	strcpy(pSI.szIPaddress,szIP);
	pSI.dwIP = NetworkNameToIP(szIP,_itoa(dwPort,destPort,10));
	pSI.dwPort = dwPort;

	int iResult =CheckForDuplicateServer(pGI,pSI);
	if(iResult!=-1) //did we get an exsisting server?
	{
		 //If yes then set that server to a favorite
		if(bFavorite)
			pGI->vSI[iResult].cFavorite = 1;
		
		return pGI->vSI[iResult].dwIndex;
	}
		
	//Add a new server into current list!
	
	pSI.dwPing = 9999;
	strcpy(pSI.szShortCountryName,"zz");
	pSI.cCountryFlag = 0;
	pSI.bNeedToUpdateServerInfo = true;
	
	pSI.dwIndex = pGI->vSI.size();
	if(bFavorite)
		pSI.cFavorite = 1;
	
	int hash = pSI.dwIP + pSI.dwPort;
	pGI->shash.insert(Int_Pair(hash,pSI.dwIndex));
	pGI->vSI.push_back(pSI);
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
DWORD TreeView_UncheckAllTypes(char cGameIdx, DWORD dwType)
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
		case FILTER_SHOW_PRIVATE : 
			TreeView_UncheckAllTypes(ti.cGAMEINDEX,128); 
			pGI->filter.bNoPrivate = FALSE; 
			pGI->filter.bOnlyPrivate = TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_HIDE_PRIVATE : 
			TreeView_UncheckAllTypes(ti.cGAMEINDEX,2); 
			pGI->filter.bOnlyPrivate = FALSE; 
			pGI->filter.bNoPrivate = 	TreeView_SwapDWCheckState(tvi,ti.dwState); break;

		case FILTER_FULL    : 	pGI->filter.bNoFull = TreeView_SwapDWCheckState(tvi,ti.dwState); break;			
		case FILTER_EMPTY	: 	 pGI->filter.bNoEmpty = TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_OFFLINE	 : pGI->filter.bHideOfflineServers = 	TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_DEDICATED	 : pGI->filter.bDedicated =	TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_FAVORITERS : break;
		case FILTER_BOTS	: pGI->filter.bNoBots = 	TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		
		case FILTER_PURE : pGI->filter.bPure = 	TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_RANKED : pGI->filter.bRanked = 	TreeView_SwapDWCheckState(tvi,ti.dwState); break;
		case FILTER_MOD	: 
			return   	vTI.at(idx).dwState = TreeView_SwapDWCheckStateOR(tvi,ti, &GamesInfo[ti.cGAMEINDEX].filter.dwMod); break;
		case FILTER_GAMETYPE:
				return vTI.at(idx).dwState = 	TreeView_SwapDWCheckStateOR(tvi,ti,	&GamesInfo[ti.cGAMEINDEX].filter.dwGameTypeFilter); break;
		case FILTER_VERSION:
				return vTI.at(idx).dwState = 	TreeView_SwapDWCheckStateOR(tvi,ti,	&GamesInfo[ti.cGAMEINDEX].filter.dwVersion); break;
		case FILTER_MAP:
				return vTI.at(idx).dwState = 	TreeView_SwapDWCheckStateOR(tvi,ti,	&GamesInfo[ti.cGAMEINDEX].filter.dwMap); break;
		case FILTER_REGION:
				return vTI.at(idx).dwState = 	TreeView_SwapDWCheckStateOR(tvi,ti,	&GamesInfo[ti.cGAMEINDEX].filter.dwRegion); break;

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
		//AddLogInfo(ETSV_DEBUG,"%d %s Action %d",iSel,szBuffer,vTI.at(iSel).dwAction);

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


DWORD TreeView_GetItemStateByType(char cGameIdx,DWORD dwType)
{
	for(UINT i=0;i<vTI.size();i++)
	{
		if((vTI.at(i).dwType == dwType) && (vTI.at(i).cGAMEINDEX == cGameIdx))
			return vTI.at(i).dwState;
	}
	return 0;
}

/*******************************************************
 
 Sets a new value depending of type.
 Return if successfully found the dwType.
	This function will only change the first occurrence, 
	therefore recommendation is that dwType is unique.

********************************************************/
BOOL TreeView_SetDWValueByItemType(DWORD dwType,DWORD dwNewValue,DWORD dwNewState, char*pszNewStrValue)
{
	for(UINT i=0;i<vTI.size();i++)
	{
		if(vTI.at(i).dwType == dwType)
		{
			vTI.at(i).dwValue = dwNewValue;
			vTI.at(i).dwState = dwNewState;
			if(pszNewStrValue!=NULL)
				vTI.at(i).strValue = pszNewStrValue;
			 if(vTI.at(i).dwAction==DO_GLOBAL_EDIT_FILTER)
			 {
				char text[256];
				sprintf(text,"%s %d",vTI.at(i).sName.c_str(),vTI.at(i).dwValue);
				TreeView_SetItemText(vTI.at(i).hTreeItem,text);
				TreeView_SetCheckBoxState(i,vTI.at(i).dwState);
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
/*********************************************************
This function is good for when upgrading the treeview list,
purpose is to restore it original user value.
**********************************************************/

BOOL TreeView_SetFilterGroupCheckState(char gameIdx, DWORD dwType,DWORD dwFilterValue)
{
	BOOL bChanges=FALSE;
	for(UINT i=0;i<vTI.size();i++)
	{

		if((vTI.at(i).cGAMEINDEX == gameIdx) && (vTI.at(i).dwType == dwType))
		{		
			if(vTI.at(i).dwValue & dwFilterValue)
			{
				vTI.at(i).dwState = 1;	
				TreeView_SetCheckBoxState(i,vTI.at(i).dwState);
				bChanges = TRUE;

			}
		}
	}
	return bChanges;
}

BOOL TreeView_SetFilterCheckState(char gameIdx, DWORD dwType,DWORD dwNewState)
{
	BOOL bChanges=FALSE;
	for(UINT i=0;i<vTI.size();i++)
	{

		if((vTI.at(i).cGAMEINDEX == gameIdx) && (vTI.at(i).dwType == dwType))
		{		
				TreeView_SetCheckBoxState(i,dwNewState);
				bChanges = TRUE;
		}
	}
	return bChanges;
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

BOOL TreeView_SetCheckBoxState(int iSel,DWORD dwState)
{
	TVITEM  tvitem;
	ZeroMemory(&tvitem,sizeof(TVITEM));
	tvitem.hItem = vTI.at(iSel).hTreeItem;
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
	
	//AddLogInfo(ETSV_DEBUG,"%d %s Action %d",iSel,szBuffer,vTI.at(iSel).dwAction);

	switch(vTI.at(iSel).dwAction)
	{
		case DO_NOTHING_: return DO_NOTHING; 
		case DO_REDRAW_SERVERLIST: 
			return SetCurrentViewTo(vTI.at(iSel).cGAMEINDEX); 
		case DO_CHECKBOX: 
			{
				TreeView_SetCheckBoxState(iSel,vTI.at(iSel).dwState);
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
			vTI.at(iSel).dwState = Filter_game_specific_edit(&GamesInfo[vTI.at(iSel).cGAMEINDEX],vTI.at(iSel), &tvitem,iSel);
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
	//RedrawServerListThread(&GamesInfo[g_currentGameIdx]);
	Initialize_RedrawServerListThread();
	return DO_NOTHING;
}



void Initialize_CountryFilter()
{
	CountryFilter.counter=0;
	TVITEM  tvitem;
	ZeroMemory(&tvitem,sizeof(TVITEM));
	hRootCountryFiltersItem = TreeView_GetTIByItemType(1001);
	tvitem.hItem = hRootCountryFiltersItem;
	tvitem.mask = TVIF_SELECTEDIMAGE |  TVIF_IMAGE;
	TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );

	if(	tvitem.iImage == 9) //Unchecked image
		AppCFG.bUseCountryFilter = FALSE;
	else
		AppCFG.bUseCountryFilter = TRUE;

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
/*#ifdef _DEBUG
		char szDebugTxt[100];
		sprintf(szDebugTxt,"%d %s - (%d) dwRet = %d  dwReturn = %d",tvitem.iImage,tvitem.pszText,tvitem.lParam,dwRet, dwReturn);
		dbg_print(szDebugTxt);
#endif*/
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

#pragma pack(1) 
typedef struct { 
    BYTE   cbType; 
    BYTE   cbSize; 
    BYTE   pointer; 
    struct in_addr address; 



} RecordRouteOption, *PRecordRouteOption; 


void _stdcall TraceRoute(char *ipaddress ) 
{ 
        char*   SendData = "Hello"; 
        UCHAR   TTL = 0; 
        HANDLE  icmp = IcmpCreateFile(); 
        DWORD   count; 
		unsigned long ipaddr = INADDR_NONE;
		ipaddr = inet_addr(ipaddress);
		if (ipaddr == INADDR_NONE) {
			dbg_print("usage:  IP address\n");
			return ;
		}
	
        DWORD   ReplySize = sizeof(ICMP_ECHO_REPLY) + 
                        sizeof(RecordRouteOption) + 
                        sizeof("Hello") + 8; 
        ICMP_ECHO_REPLY* reply = (PICMP_ECHO_REPLY) malloc(ReplySize); 
        IP_OPTION_INFORMATION info; 
        struct in_addr  LastAddr = {0}; 

		_TraceRT.clear();

        if( (icmp != INVALID_HANDLE_VALUE) && (reply != NULL) ) 
        { 
            dbg_print("Start trace\r\n"); 
            dbg_print( "\tHops\tReply From\tRoundTrip\n" ); 
            while( LastAddr.s_addr != ipaddr ) 
            { 
                memset( &info, 0, sizeof(info) ); 
                info.Ttl = TTL++; 
                count = IcmpSendEcho2( icmp, NULL,NULL,NULL, 
                               ipaddr, SendData, sizeof("Hello"), 
                                &info, reply, ReplySize, 1000 ); 
                if( count ) 
                { 

					
					LastAddr.s_addr = reply->Address; 
						_TRACERT_STRUCT ts;					
						unsigned int addr;
					    struct hostent* remoteHost;
						addr = inet_addr( inet_ntoa(LastAddr));
						remoteHost = gethostbyaddr((char *) &addr, 4, AF_INET);

                        dbg_print("\t%3d:\t%s:\t%3dms %s %s\r\n", 
                                info.Ttl, 
                                inet_ntoa(LastAddr), 
                                reply->RoundTripTime, 
								((reply->DataSize && reply->Data)? reply->Data:" "),((remoteHost==NULL)? " ": remoteHost->h_name) 
                                ); 

						
						
						if( remoteHost!=NULL)
							strcpy(ts.szHostName, remoteHost->h_name);
						else
							ts.szHostName[0]=NULL;
						ts.IpAdress = reply->Address; 
						ts.TTL = info.Ttl;
						ts.RTT = reply->RoundTripTime;
						_TraceRT.push_back(ts);
						InvalidateRect(g_hwndMainSTATS,NULL,TRUE);
                }else{ 
                        // DWORD error = GetLastError(); 
                        // Add error handling if desired 
                }; 
                if( TTL == 0 ) 
                { 
                        //ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData) + 8; 
                        //Causes destination to be unable to reply (reply too small). 
                        dbg_print( "Unreachable within 255 hops, or Program Error\n"); 
                        break; 
                }; 
				if(TTL>20)
				{
					 dbg_print( "TTL> 20 breaking the trace rt\n"); 
					break;
				}
            };// while addr 
            dbg_print("End trace\r\n"); 

        }else{ 
                // report lack of memory or icmp resources 
        };// if icmp 
        free( reply ); 
        IcmpCloseHandle(icmp); 
}

DWORD MyPing(char *ipaddress, DWORD & dwPing)
{
	HANDLE hIcmpFile;
    unsigned long ipaddr = INADDR_NONE;
    DWORD dwRetVal = 0;
    char SendData[] = "Data Buffer";
    LPVOID ReplyBuffer = NULL;
    DWORD ReplySize = 0;
	dwPing = 999;
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
		free(ReplyBuffer);
        return 999;
    
	}
	dwPing = pEchoReply->RoundTripTime;
	free(ReplyBuffer);
    
	return 0;
}    



// Message handler for about box.
LRESULT CALLBACK AddServerProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		{
			SetWindowText(hDlg,lang.GetString("TitleAddIP"));
		if(strlen(g_szIPtoAdd)>0)
		{
			SetDlgItemText(hDlg,IDC_EDIT_IP,g_szIPtoAdd);
			PostMessage(GetDlgItem(hDlg,IDC_EDIT_IP),EM_SETSEL,0,strlen(g_szIPtoAdd));
			PostMessage(GetDlgItem(hDlg,IDC_EDIT_IP),EM_SETSEL,(WPARAM)-1,-1);
		}

		CenterWindow(hDlg);
		SetFocus(GetDlgItem(hDlg,IDC_EDIT_IP));
		//return TRUE;
		}
	break;	

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BUTTON_OK)	
		{
			char ip[80];
			DWORD dwPort=0;

			GetDlgItemText(hDlg,IDC_EDIT_IP,ip,79);
		
			SplitIPandPORT(ip,dwPort);
			
			if(dwPort==0)
				dwPort = GamesInfo[g_currentGameIdx].dwDefaultPort;

			if(AddServer(&GamesInfo[g_currentGameIdx],ip, dwPort,true)!=0xFFFFFFFF)		
			{
				SetStatusText(ICO_INFO,"Added IP %s:%d into %s favorite serverlist.",ip,dwPort,GamesInfo[g_currentGameIdx].szGAME_NAME);
				
			}else
			{
				SetStatusText(ICO_WARNING,lang.GetString("InvalidIP"));
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

DWORD g_TreeLevel=1;
int Tree_ParseChilds(TiXmlElement* childItem, HTREEITEM hTreeItem)
{
	char szName[100];
	_TREEITEM ti;
	if(childItem==NULL)
		return 0;

	g_TreeLevel++;
	for( childItem; childItem; childItem=childItem->NextSiblingElement() )
	{			
		ti.dwAction = 0;
		ti.dwType = 0;
		ti.dwCompare = 0;
		ti.dwState = 0;
		ti.dwValue = 0;
		ti.sName = "No name";

		XML_GetTreeItemName(childItem,szName,sizeof(szName));		
		if(szName!=NULL)
			ti.sName = szName;

		ti.sElementName = childItem->Value();
 
		ti.cGAMEINDEX = (char)XML_GetTreeItemInt(childItem,"game");
		ti.dwType =  XML_GetTreeItemInt(childItem,"type");
	
		ti.dwCompare =  XML_GetTreeItemInt(childItem,"compare");

		if(ti.dwType==1024)
			ti.iIconIndex = Get_GameIcon(ti.cGAMEINDEX);
		else
			ti.iIconIndex = XML_GetTreeItemInt(childItem,"icon");

		ti.dwAction = XML_GetTreeItemInt(childItem,"action");
		ti.dwValue =  XML_GetTreeItemInt(childItem,"value");
		ZeroMemory(szName,sizeof(szName));
		ti.strValue = "zz";	
		XML_GetTreeItemStrValue(childItem,szName,sizeof(szName));
		if(szName!=NULL)
			ti.strValue = szName;

		ti.dwState =  XML_GetTreeItemInt(childItem,"state");
		ti.bExpanded = (bool) XML_GetTreeItemInt(childItem,"expanded");
		ti.dwLevel = g_TreeLevel;
		ti.dwIndex = vTI.size();
		bool active=true;
		if(ti.cGAMEINDEX!=-25)
		{
			if(ti.cGAMEINDEX<GamesInfo.size())
				active = GamesInfo[ti.cGAMEINDEX].bActive;
		}
				
		ti.hTreeItem = MainTreeView_AddItem(&ti,hTreeItem, active);
		vTI.push_back(ti);

		GAMEFILTER gf;
		gf.sFriendlyName = ti.sName;
		gf.sStrValue = ti.strValue;
		gf.dwValue = ti.dwValue;
		gf.dwExactMatch = ti.dwCompare;

		if(ti.sElementName == "GameTypesItems")
			GamesInfo[ti.cGAMEINDEX].vFilterGameType.push_back(gf);
		else if(ti.sElementName == "ModItems")
			GamesInfo[ti.cGAMEINDEX].vFilterMod.push_back(gf);
		else if(ti.sElementName == "Version")
			GamesInfo[ti.cGAMEINDEX].vFilterVersion.push_back(gf);
		else if(ti.sElementName == "Map")
			GamesInfo[ti.cGAMEINDEX].vFilterMap.push_back(gf);

		Tree_ParseChilds(childItem->FirstChildElement(),ti.hTreeItem );

	}
	g_TreeLevel--;
	return 0;
}

UINT g_save_counter=0;  //Ugly hack
DWORD  Save_all_by_level(TiXmlElement *pElemRoot,DWORD dwlevel)
{
	if(g_save_counter>vTI.size())
		return 0;
	
	while(dwlevel==vTI.at(g_save_counter).dwLevel)
	{			
		int iSel = g_save_counter;
		
	//	if(vTI.at(iSel).dwType==1001) //reached to country filter
	//		return 0;


		TiXmlElement * elem = new TiXmlElement( vTI.at(iSel).sElementName.c_str());  

#ifdef _DEBUG	
		char padding[40];
		padding[0]=0;
		for(int i=0;i<dwlevel;i++)
			strcat(padding," ");
		dbg_print("%s %d %d %s %s Action %d level:%d",padding,iSel,dwlevel,vTI.at(iSel).sElementName.c_str(),vTI.at(iSel).sName.c_str(),vTI.at(iSel).dwAction,vTI.at(iSel).dwLevel);
#endif

		elem->SetAttribute("name",vTI.at(iSel).sName.c_str());
		elem->SetAttribute("strval",vTI.at(iSel).strValue.c_str());
		elem->SetAttribute("value",vTI.at(iSel).dwValue);
		elem->SetAttribute("compare",vTI.at(iSel).dwCompare);
		elem->SetAttribute("icon",vTI.at(iSel).iIconIndex);
		elem->SetAttribute("expanded",(UINT)vTI.at(iSel).bExpanded);
		elem->SetAttribute("type",vTI.at(iSel).dwType);
		elem->SetAttribute("state",vTI.at(iSel).dwState);
		elem->SetAttribute("game",vTI.at(iSel).cGAMEINDEX);
		elem->SetAttribute("action",vTI.at(iSel).dwAction);
			
		g_save_counter++;
		pElemRoot->LinkEndChild( elem ); 
		if(g_save_counter>=vTI.size())
			return 0;

		 
		DWORD nextlevel = vTI.at(g_save_counter).dwLevel;
		DWORD lvl=0;
		if(nextlevel>dwlevel)
		{	
			lvl = Save_all_by_level(elem,nextlevel);
			if(lvl!=dwlevel)
				return lvl;
		}		
		if(g_save_counter>=vTI.size())
			return 0;
	}
	return vTI.at(g_save_counter).dwLevel;
}

DWORD  Save_all_by_level_country(TiXmlElement *pElemRoot,DWORD dwlevel)
{
	if(g_save_counter>vTI.size())
		return 0;
	
	while(dwlevel==vTI.at(g_save_counter).dwLevel)
	{			
		int iSel = g_save_counter;
		
		if(vTI.at(iSel).dwType==1001) //reached to country filter
			return 0;


		TiXmlElement * elem = new TiXmlElement( vTI.at(iSel).sElementName.c_str());  

#ifdef _DEBUG	
		char padding[40];
		padding[0]=0;
		for(int i=0;i<dwlevel;i++)
			strcat(padding," ");
		dbg_print("%s %d %d %s %s Action %d level:%d",padding,iSel,dwlevel,vTI.at(iSel).sElementName.c_str(),vTI.at(iSel).sName.c_str(),vTI.at(iSel).dwAction,vTI.at(iSel).dwLevel);
#endif

		elem->SetAttribute("name",vTI.at(iSel).sName.c_str());
		elem->SetAttribute("strval",vTI.at(iSel).strValue.c_str());
		elem->SetAttribute("value",vTI.at(iSel).dwValue);
		elem->SetAttribute("compare",vTI.at(iSel).dwCompare);
		elem->SetAttribute("icon",vTI.at(iSel).iIconIndex);
		elem->SetAttribute("expanded",(UINT)vTI.at(iSel).bExpanded);
		elem->SetAttribute("type",vTI.at(iSel).dwType);
		elem->SetAttribute("state",vTI.at(iSel).dwState);
		elem->SetAttribute("game",vTI.at(iSel).cGAMEINDEX);
		elem->SetAttribute("action",vTI.at(iSel).dwAction);
			
		g_save_counter++;
		pElemRoot->LinkEndChild( elem ); 
		if(g_save_counter>=vTI.size())
			return 0;

		 
		DWORD nextlevel = vTI.at(g_save_counter).dwLevel;
		DWORD lvl=0;
		if(nextlevel>dwlevel)
		{	
			lvl = Save_all_by_level(elem,nextlevel);
			if(lvl!=dwlevel)
				return lvl;
		}		
		if(g_save_counter>=vTI.size())
			return 0;
	}
	return vTI.at(g_save_counter).dwLevel;
}

void TreeView_cleanup()
{
	vTI.size();
	for(int i=0;i<vTI.size();i++)
	{
		vTI.at(i).sElementName.clear();
		vTI.at(i).sName.clear();
		vTI.at(i).strValue.clear();
	}

	vTI.clear();
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
		//	for(int i=0; i<GamesInfo.size(); i++)  //removed since 1.05
		//		ZeroMemory(&GamesInfo[i].filter,sizeof(FILTER_SETTINGS));//removed since 1.05
		}
			
	}

	vTI.clear();
	// save this for later
	hRoot=TiXmlHandle(pElem);

	TiXmlElement* child = hRoot.FirstChild( "TreeItems" ).ToElement();

	g_TreeLevel=1;
	for( child; child; child=child->NextSiblingElement() )
	{
		Tree_ParseChilds(child->FirstChildElement(),NULL);
	}	
	char szBuffer[200];

	//Let's do some resync values, this will help to ensure after an upgrade of the treeview  structure to display correct
	for(int i=0; i<GamesInfo.size();i++)
	{
		GamesInfo[i].hTI = TreeView_GetTIByItemGame(i);

		sprintf(szBuffer,"%s (%d)",GamesInfo[i].szGAME_NAME,GamesInfo[i].dwTotalServers);
		if(GamesInfo[i].hTI!=NULL)
		{
			TreeView_SetItemText(GamesInfo[i].hTI,szBuffer);
			TreeView_SetItemState(g_hwndMainTreeCtrl,GamesInfo[i].hTI,TVIS_BOLD ,TVIS_BOLD);
			TreeView_SetFilterCheckState(i, 1,GamesInfo[i].filter.bPunkbuster);
			TreeView_SetFilterCheckState(i, 2,GamesInfo[i].filter.bOnlyPrivate);
			TreeView_SetFilterCheckState(i, 128,GamesInfo[i].filter.bNoPrivate);
			TreeView_SetFilterCheckState(i, 4,GamesInfo[i].filter.bNoFull);
			TreeView_SetFilterCheckState(i, 8,GamesInfo[i].filter.bNoEmpty);
			TreeView_SetFilterCheckState(i, 64,GamesInfo[i].filter.bNoBots);
			TreeView_SetFilterCheckState(i, 26,GamesInfo[i].filter.bPure);
			TreeView_SetFilterCheckState(i, 27,GamesInfo[i].filter.bRanked);

			TreeView_SetFilterGroupCheckState(i,FILTER_MOD,GamesInfo[i].filter.dwMod);
			TreeView_SetFilterGroupCheckState(i,FILTER_MAP,GamesInfo[i].filter.dwMap);
			TreeView_SetFilterGroupCheckState(i,FILTER_VERSION,GamesInfo[i].filter.dwVersion);
			TreeView_SetFilterGroupCheckState(i,FILTER_GAMETYPE,GamesInfo[i].filter.dwGameTypeFilter);
			TreeView_SetFilterGroupCheckState(i,FILTER_REGION,GamesInfo[i].filter.dwRegion);
		}
	}

	for(int i=0; i<vTI.size(); i++)
	{
		if((vTI.at(i).sElementName == "Ping") && (vTI.at(i).dwValue==AppCFG.filter.dwPing))
		{
			TreeView_SetCheckBoxState(i,1);
		}
		if((vTI.at(i).sName == "Hide offline servers"))
		{
			if(AppCFG.filter.bHideOfflineServers)
				TreeView_SetCheckBoxState(i,1);
			else
				TreeView_SetCheckBoxState(i,0);

		}
	}

	TreeView_SetDWValueByItemType(FILTER_MIN_PLY, AppCFG.filter.dwShowServerWithMinPlayers,AppCFG.filter.cActiveMinPlayer);
	TreeView_SetDWValueByItemType(FILTER_MAX_PLY, AppCFG.filter.dwShowServerWithMaxPlayers,AppCFG.filter.cActiveMaxPlayer);
	
	Initialize_CountryFilter();
	SetFocus(g_hwndMainTreeCtrl);
	return 0;
}

void OnActivate_ServerList(DWORD options)
{
	
	Initialize_RedrawServerListThread();

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



PLAYERDATA * Copy_PlayerToCurrentPL(LPPLAYERDATA &pStartPD, PLAYERDATA *pNewPD)
{
	PLAYERDATA *pTmp=NULL;
	pTmp = pStartPD;
	if(pTmp!=NULL)
		while(pTmp->pNext!=NULL)
			pTmp = pTmp->pNext;

	PLAYERDATA *player = (PLAYERDATA *)calloc(1,sizeof(PLAYERDATA));
	if(player!=NULL)
	{
		memcpy(player,pNewPD,sizeof(PLAYERDATA));
		if(pNewPD->szClanTag!=NULL)
			player->szClanTag = _strdup(pNewPD->szClanTag);
		if(pNewPD->szTeam!=NULL)
			player->szTeam = _strdup(pNewPD->szTeam);		
		if(pNewPD->szPlayerName!=NULL)
			player->szPlayerName = _strdup(pNewPD->szPlayerName);
		player->pNext = NULL;
		if(pTmp==NULL)
			return pStartPD = pTmp = player;
		else
			return 	pTmp->pNext = player;
	}
	return NULL;
}

long DrawCurrentPlayerList(PLAYERDATA *pPlayer)
{
	ListView_DeleteAllItems(g_hwndListViewPlayers);
	int n=0;

	  if(g_hwndListViewPlayers!=NULL)
	  {
		  while (pPlayer!=NULL)
		  {	
			pPlayer = pPlayer->pNext;
			n++;
		  }
		  ListView_SetItemCount(g_hwndListViewPlayers, n);
	  }

	return 0;
}

long UpdatePlayerList(LPPLAYERDATA pPlayers)
{
	if(g_hwndListViewPlayers==NULL)
		return 0xFF;

	if(g_bControl==FALSE)
	{
		UTILZ_CleanUp_PlayerList(pCurrentPL);
		pCurrentPL = NULL;
		ListView_DeleteAllItems(g_hwndListViewPlayers);
	}
	g_bControl=FALSE; //reset
	int c=0;
	PLAYERDATA *pCount=pCurrentPL;
	if(pCount!=NULL)
	{
		while(pCount->pNext!=NULL)
		{
			pCount = pCount->pNext;
			c++;
		}
	}

	int n=c;
	while (pPlayers!=NULL)
	{
		//Potential mem leak, may have to rewrite/improve this part of code
		PLAYERDATA *pPD = Copy_PlayerToCurrentPL(pCurrentPL,pPlayers);  //This will keep a copy of the playerlist during scanning
		pPlayers = pPlayers->pNext;
		
		n++;
	
	}
	ListView_SetItemCount(g_hwndListViewPlayers, n);

/*ListView_SetColumnWidth(g_hwndListViewPlayers,0,LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(g_hwndListViewPlayers,1,LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(g_hwndListViewPlayers,2,LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(g_hwndListViewPlayers,3,LVSCW_AUTOSIZE);
	PostMessage(g_hWnd,WM_SIZE,0,0);
*/	//	ListView_SetColumnWidth(g_hwndListViewPlayers,4,LVSCW_AUTOSIZE);
	return 0;
}

long UpdateRulesList(LPSERVER_RULES pServerRules)
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
	for(int i=0;i<GamesInfo.size();i++)
	{
		OutputDebugString(GamesInfo[i].szGAME_NAME);
		OutputDebugString(" - Cleaning up serverlist, playerlist and hashes etc.\n");
		
		for(long x=0; x<GamesInfo[i].vSI.size();x++)
			UTILZ_CleanUp_PlayerList(GamesInfo[i].vSI.at(x).pPlayerData);

		GamesInfo[i].dwTotalServers = 0;
		GamesInfo[i].vSI.clear();
		GamesInfo[i].vRefListSI.clear();
		GamesInfo[i].vRefScanSI.clear();
		GamesInfo[i].shash.clear();
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
		SetStatusText(ICO_WARNING,lang.GetString("InvalidIP"));
		
		//Try from serverlist if any server selected
		StartGame_ConnectToServer(false);
		return;
	}
	strcpy(srv.szIPaddress,SplitIPandPORT(ip,dwPort));
	srv.dwPort = dwPort;
	strcpy(srv.szPRIVATEPASS,password);
	srv.cGAMEINDEX = -1;
	srv.dwIndex = 999999;
	LaunchGame(srv,&GamesInfo[g_currentGameIdx]);
}

GAME_INFO * Get_CurrentViewByServer(SERVER_INFO* pSrv)
{
	return &GamesInfo[pSrv->cGAMEINDEX];	 
}


int FindFirstActiveGame()
{
	for(int i=0;i<GamesInfo.size();i++)
		if(GamesInfo[i].bActive)
			return i;

		int i = MessageBox(g_hWnd,lang.GetString("MessageNoGamesActive"),"Info",MB_YESNO|MB_ICONINFORMATION|MB_TOPMOST);
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
	OutputDebugString("OnRestore\n");
	//AddLogInfo(ETSV_INFO, "Message WM_SIZE restoring.");
	//Initialize_WindowSizes();
	g_iCurrentSelectedServer = -1;


}

void ListView_SetDefaultColumns()
{
	for(int i=0;i<MAX_COLUMNS;i++)
		ZeroMemory(&CUSTCOLUMNS[i].lvColumn,sizeof(CUSTCOLUMNS[0].lvColumn));
	int idx=0;

	CUSTCOLUMNS[COL_PB].id = COL_PB;
	CUSTCOLUMNS[COL_PB].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_PB].lvColumn.cx = 20;
	CUSTCOLUMNS[COL_PB].sName = lang.GetString("ColumnAntiCheat");
	CUSTCOLUMNS[COL_PB].columnIdx = idx++;
	CUSTCOLUMNS[COL_PB].bActive = TRUE;

	CUSTCOLUMNS[COL_PRIVATE].id = COL_PRIVATE;
	CUSTCOLUMNS[COL_PRIVATE].lvColumn.mask =  LVCF_WIDTH  | LVCF_TEXT;
	CUSTCOLUMNS[COL_PRIVATE].lvColumn.cx = 16;
	CUSTCOLUMNS[COL_PRIVATE].sName = lang.GetString("ColumnPrivate");
	CUSTCOLUMNS[COL_PRIVATE].columnIdx = idx++;
	CUSTCOLUMNS[COL_PRIVATE].bActive = TRUE;
	
	CUSTCOLUMNS[COL_RANKED].id = COL_RANKED;
	CUSTCOLUMNS[COL_RANKED].lvColumn.mask =  LVCF_WIDTH  | LVCF_TEXT;
	CUSTCOLUMNS[COL_RANKED].lvColumn.cx = 16;
	CUSTCOLUMNS[COL_RANKED].sName = lang.GetString("ColumnRanked");
	CUSTCOLUMNS[COL_RANKED].columnIdx = idx++;
	CUSTCOLUMNS[COL_RANKED].bActive = TRUE;

	CUSTCOLUMNS[COL_SERVERNAME].id = COL_SERVERNAME;
	CUSTCOLUMNS[COL_SERVERNAME].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_SERVERNAME].lvColumn.cx = 220;
	CUSTCOLUMNS[COL_SERVERNAME].sName = lang.GetString("ColumnServerName");
	CUSTCOLUMNS[COL_SERVERNAME].columnIdx = idx++;
	CUSTCOLUMNS[COL_SERVERNAME].bActive = TRUE;

	CUSTCOLUMNS[COL_VERSION].id = COL_VERSION;
	CUSTCOLUMNS[COL_VERSION].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_VERSION].lvColumn.cx = 55;
	CUSTCOLUMNS[COL_VERSION].sName = lang.GetString("ColumnVersion");
	CUSTCOLUMNS[COL_VERSION].columnIdx = idx++;
	CUSTCOLUMNS[COL_VERSION].bActive = TRUE;

	CUSTCOLUMNS[COL_GAMETYPE].id = COL_GAMETYPE;
	CUSTCOLUMNS[COL_GAMETYPE].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_GAMETYPE].lvColumn.cx = 55;
	CUSTCOLUMNS[COL_GAMETYPE].sName = lang.GetString("ColumnGameType");
	CUSTCOLUMNS[COL_GAMETYPE].columnIdx = idx++;
	CUSTCOLUMNS[COL_GAMETYPE].bActive = TRUE;

	CUSTCOLUMNS[COL_MAP].id = COL_MAP;
	CUSTCOLUMNS[COL_MAP].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_MAP].lvColumn.cx = 75;
	CUSTCOLUMNS[COL_MAP].sName = lang.GetString("ColumnMap");
	CUSTCOLUMNS[COL_MAP].columnIdx = idx++;
	CUSTCOLUMNS[COL_MAP].bActive = TRUE;

	CUSTCOLUMNS[COL_MOD].id = COL_MOD;
	CUSTCOLUMNS[COL_MOD].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_MOD].lvColumn.cx = 65;
	CUSTCOLUMNS[COL_MOD].sName = lang.GetString("ColumnMod");
	CUSTCOLUMNS[COL_MOD].columnIdx  = idx++;
	CUSTCOLUMNS[COL_MOD].bActive = TRUE;

	CUSTCOLUMNS[COL_BOTS].id = COL_BOTS;
	CUSTCOLUMNS[COL_BOTS].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_BOTS].lvColumn.cx = 20;
	CUSTCOLUMNS[COL_BOTS].sName = lang.GetString("ColumnBots");
	CUSTCOLUMNS[COL_BOTS].columnIdx = idx++;
	CUSTCOLUMNS[COL_BOTS].bActive = TRUE;

	CUSTCOLUMNS[COL_PLAYERS].id = COL_PLAYERS;
	CUSTCOLUMNS[COL_PLAYERS].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_PLAYERS].lvColumn.cx = 90;
	CUSTCOLUMNS[COL_PLAYERS].sName = lang.GetString("ColumnPlayers");
	CUSTCOLUMNS[COL_PLAYERS].columnIdx = idx++;
	CUSTCOLUMNS[COL_PLAYERS].bActive = TRUE;
	CUSTCOLUMNS[COL_PLAYERS].bSortAsc = TRUE;

	CUSTCOLUMNS[COL_COUNTRY].id = COL_COUNTRY;	
	CUSTCOLUMNS[COL_COUNTRY].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	if(AppCFG.bUseShortCountry)
		CUSTCOLUMNS[COL_COUNTRY].lvColumn.cx = 45;
	else
		CUSTCOLUMNS[COL_COUNTRY].lvColumn.cx =115;

	CUSTCOLUMNS[COL_COUNTRY].sName = lang.GetString("ColumnCountry");
	CUSTCOLUMNS[COL_COUNTRY].columnIdx = idx++;
	CUSTCOLUMNS[COL_COUNTRY].bActive = TRUE;

	CUSTCOLUMNS[COL_PING].id = COL_PING;
	CUSTCOLUMNS[COL_PING].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_PING].lvColumn.cx = 45;
	CUSTCOLUMNS[COL_PING].sName = lang.GetString("ColumnPing");
	CUSTCOLUMNS[COL_PING].columnIdx = idx++;
	CUSTCOLUMNS[COL_PING].bActive = TRUE;
	CUSTCOLUMNS[COL_PING].bSortAsc = TRUE;

	CUSTCOLUMNS[COL_IP].id = COL_IP;
	CUSTCOLUMNS[COL_IP].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_IP].lvColumn.cx = 150;
	CUSTCOLUMNS[COL_IP].sName = lang.GetString("ColumnIP");
	CUSTCOLUMNS[COL_IP].columnIdx = idx++;
	CUSTCOLUMNS[COL_IP].bActive = TRUE;
	CUSTCOLUMNS[COL_IP].bSortAsc = TRUE;
	

	CUSTCOLUMNS[COL_STATUS].id = COL_STATUS;
	CUSTCOLUMNS[COL_STATUS].lvColumn.mask =  LVCF_WIDTH | LVCF_TEXT;
	CUSTCOLUMNS[COL_STATUS].lvColumn.cx = 80;
	CUSTCOLUMNS[COL_STATUS].sName = lang.GetString("ColumnStatus");
	CUSTCOLUMNS[COL_STATUS].columnIdx = idx++;
	CUSTCOLUMNS[COL_STATUS].bActive = TRUE;
	CUSTCOLUMNS[COL_STATUS].bSortAsc = TRUE;
	
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



void ChangeFont(HWND hWnd,HFONT hf)
{
	SetFontToDlgItem(hWnd,hf,IDC_TAB1);
	SetFontToDlgItem(hWnd,hf,IDC_LIST_SERVER);
	SetFontToDlgItem(hWnd,hf,IDC_EDIT_STATUS);
	SetFontToDlgItem(hWnd,hf,IDC_CUSTOM2);
	SetFontToDlgItem(hWnd,hf,IDC_EDIT_LOGGER);
	SetFontToDlgItem(hWnd,hf,IDC_LIST_BUDDY);
	SetFontToDlgItem(hWnd,hf,IDC_LIST_PLAYERS);
	SetFontToDlgItem(g_hwndSearchToolbar,g_hf,IDC_COMBOBOXEX_CMD);
}

void ChangeMainMenuLanguage(HWND hWnd)
{
	HMENU hmenu;
	hmenu = GetMenu(hWnd); 

   ModifyMenu(hmenu,IDM_SETTINGS,MF_BYCOMMAND,IDM_SETTINGS,lang.GetString("MenuSettings"));
   ModifyMenu(hmenu,ID_SERVERLIST_PURGEPUBLICSERVLIST,MF_BYCOMMAND,ID_SERVERLIST_PURGEPUBLICSERVLIST,lang.GetString("MenuDeleteAllServers"));
   ModifyMenu(hmenu,IDM_EXIT,MF_BYCOMMAND,IDM_EXIT,lang.GetString("MenuExit"));
   ModifyMenu(hmenu,ID_BUDDY_ADDFROMPLAYERLIST,MF_BYCOMMAND,ID_BUDDY_ADDFROMPLAYERLIST,lang.GetString("MenuAddSelectedPlyToBuddylist"));
   ModifyMenu(hmenu,ID_BUDDY_ADD,MF_BYCOMMAND,ID_BUDDY_ADD,lang.GetString("MenuAddNewBuddy"));
   ModifyMenu(hmenu,ID_BUDDY_REMOVE,MF_BYCOMMAND,ID_BUDDY_REMOVE,lang.GetString("MenuRemoveBuddy"));

   ModifyMenu(hmenu,ID_VIEW_BUDDYLIST,MF_BYCOMMAND,ID_VIEW_BUDDYLIST,lang.GetString("MenuViewBuddy"));
   ModifyMenu(hmenu,ID_VIEW_MAPPREVIEW,MF_BYCOMMAND,ID_VIEW_MAPPREVIEW,lang.GetString("MenuViewMap"));
   ModifyMenu(hmenu,ID_VIEW_PLAYERLIST,MF_BYCOMMAND,ID_VIEW_PLAYERLIST ,lang.GetString("MenuViewTabs"));

}

void OnCreate(HWND hwnd, HINSTANCE hInst)
{
	AddLogInfo(ETSV_INFO,"Called OnCreate...");

	LV_COLUMN   lvColumn;

	ZeroMemory(&lvColumn,sizeof(LV_COLUMN));
			
	Q3_SetCallbacks(UpdateServerItem, Buddy_CheckForBuddies, NULL);//&InsertServerItem);
	Q4_SetCallbacks(UpdateServerItem, Buddy_CheckForBuddies, NULL);//&InsertServerItem);
	STEAM_SetCallbacks(UpdateServerItem, Buddy_CheckForBuddies,NULL);// &InsertServerItem);

	g_CurrentSRV = NULL;
	g_bRedrawServerListThread =  FALSE;
	g_bRunningQueryServerList = false;
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

	g_hwndListViewPlayers = CreateWindowEx(LVS_EX_SUBITEMIMAGES|LVS_EX_FULLROWSELECT|WS_EX_CLIENTEDGE , WC_LISTVIEW , NULL,
							LVS_OWNERDATA|LVS_REPORT|WS_VISIBLE |WS_CHILD | WS_TABSTOP |WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
							100+BORDER_SIZE,200+TOOLBAR_Y_OFFSET+BORDER_SIZE,100, 200, 
							hwnd, (HMENU) IDC_LIST_PLAYERS, hInst, NULL);

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

	SetImageList();


	AddLogInfo(ETSV_INFO,"Called All CreateWindowEx.");

	TCITEM tci;
	ZeroMemory(&tci,sizeof(tci));
	tci.iImage = 0;
	tci.mask =  TCIF_IMAGE|TCIF_TEXT;
	tci.pszText = (LPSTR)lang.GetString("TabPlayers");
	TabCtrl_InsertItem(g_hwndTabControl,0,&tci);
	tci.iImage = 27;
	tci.pszText = (LPSTR)lang.GetString("TabRules");
	TabCtrl_InsertItem(g_hwndTabControl,1,&tci);
	tci.iImage = 22;
	tci.pszText = (LPSTR)lang.GetString("TabRCON");
	TabCtrl_InsertItem(g_hwndTabControl,2,&tci);
	tci.iImage = 26;
	tci.pszText = (LPSTR)lang.GetString("TabNetwork");
	TabCtrl_InsertItem(g_hwndTabControl,3,&tci);
	tci.iImage = 23;
	tci.pszText =  (LPSTR)lang.GetString("TabLogger");
	TabCtrl_InsertItem(g_hwndTabControl,4,&tci);

	DWORD dwExStyle = 0;

	dwExStyle = TreeView_GetExtendedStyle(g_hwndMainTreeCtrl);
	//dwExStyle |= (TVS_EX_DOUBLEBUFFER | TVS_EX_FADEINOUTEXPANDOS);
	TreeView_SetExtendedStyle(g_hwndMainTreeCtrl, WS_EX_LEFT,dwExStyle);
	
	SetWindowTheme(g_hwndMainTreeCtrl, L"explorer", 0);
	//SetWindowTheme(g_hwndListViewServer, L"explorer", 0);

	dwExStyle = ListView_GetExtendedListViewStyle(g_hwndListViewPlayers);
	dwExStyle |= LVS_EX_FULLROWSELECT ;
	dwExStyle |= LVS_EX_SUBITEMIMAGES;
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


	TCHAR szPlayerlistColumnString[7][20] = {TEXT("ColumnNo"),TEXT("ColumnTeam"),TEXT("ColumnName"), TEXT("ColumnFrag"), TEXT("ColumnPing"),TEXT("ColumnTime"),TEXT("ColumnServerName")};
	lvColumn.cx = 30;
	int dd;
	for(dd = 0; dd <7; dd++)
	{
		if(dd==2)
			lvColumn.cx = 180;
		else if(dd==0)
			lvColumn.cx = 30;
		else if(dd>5)
			lvColumn.cx = 180;
		else
			lvColumn.cx = 60;
		lvColumn.pszText = (LPSTR)lang.GetString(szPlayerlistColumnString[dd]);
		ListView_InsertColumn(g_hwndListViewPlayers, dd, &lvColumn);
	}
	lvColumn.cx = 150;
	lvColumn.pszText = (LPSTR)lang.GetString("ColumnRule");
	ListView_InsertColumn(g_hwndListViewVars, 0, &lvColumn);
	lvColumn.cx = 175;
	lvColumn.pszText = (LPSTR)lang.GetString("ColumnValue");
	ListView_InsertColumn(g_hwndListViewVars, 1, &lvColumn);
	
	lvColumn.cx = 120;
	lvColumn.pszText = (LPSTR)lang.GetString("ColumnBuddy");
	ListView_InsertColumn(g_hwndListBuddy, 0, &lvColumn);
	lvColumn.cx = 120;
	lvColumn.pszText = (LPSTR)lang.GetString("ColumnServerName");
	ListView_InsertColumn(g_hwndListBuddy, 1, &lvColumn);
	lvColumn.cx = 80;
	lvColumn.pszText = (LPSTR)lang.GetString("ColumnIP");
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

	g_hf = MyCreateFont(hwnd);
	g_hf2 = MyCreateFont(hwnd,14,FW_BOLD,"Verdana");//Courier New");
	

	ChangeMainMenuLanguage(hwnd);

	ChangeFont(hwnd,g_hf);
	
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
	g_hf = NULL;
	DeleteObject(g_hf2);
	g_hf2=NULL;

	OutputDebugString("Cleaning up filter Gametypes, Map, Mod and Versions.\n");
	for(int i=0;i<GamesInfo.size();i++)
	{
		GamesInfo[i].vFilterGameType.clear();
		GamesInfo[i].vFilterMap.clear();
		GamesInfo[i].vFilterMod.clear();
		GamesInfo[i].vFilterVersion.clear();
	}

	UTILZ_CleanUp_PlayerList(pCurrentPL);
	pCurrentPL = NULL;
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
	//AddLogInfo(ETSV_INFO,"Initialize main dialog.");
	ZeroMemory(g_szIPtoAdd,sizeof(g_szIPtoAdd));
	EnableButtons(TRUE);
	//EnableDownloadLink(FALSE);	
	if(g_bDoFirstTimeCheckForUpdate)
		PostMessage(g_hWnd,WM_COMMAND,IDM_UPDATE,1);
	
	CenterWindow(g_hWnd);

	g_nCurrentSortMode = COL_PLAYERS;

	SetInitialViewStates();
	
	SetDlgTrans(hwnd,AppCFG.g_cTransparancy);
	
	TreeView_BuildList();


	currCV = &GamesInfo[0];

	LoadAllServerList();

	g_iCurrentSelectedServer = -1;

	ListView_InitilizeColumns();

	Buddy_Load(g_pBIStart);
	Buddy_UpdateList(g_pBIStart);

	Show_ToolbarButton(IDC_BUTTON_FIND,false);
	Show_ToolbarButton(IDC_DOWNLOAD, false);
	//RedrawServerList(Get_CurrentServerListByView());
	
	ShowWindow(g_hwndLogger,SW_HIDE);
	ShowWindow(g_hwndMainRCON,SW_HIDE);	
	ShowWindow(g_hwndListViewVars,SW_HIDE);	
	ShowWindow(g_hwndMainSTATS,SW_HIDE);	
	TabCtrl_SetCurSel(g_hwndTabControl,0);
	ShowWindow(g_hwndListViewPlayers,SW_SHOW);	

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

		if(pGI->vSI.size()>0)
		{
			bool bWrite=false;
			for ( iLst = pGI->vSI.begin( ); iLst != pGI->vSI.end( ); iLst++ )
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
					sprintf_s(szBuffer,sizeof(szBuffer),"%c%d%c%d%c%d%c%d",sep,pSI.dwGameType,sep,pSI.dwMap,sep,pSI.cPurge,sep,pSI.dwPing);
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
DWORD WINAPI LoadServerListV2(LPVOID lpVoid)
{
	GAME_INFO *pGI = (GAME_INFO*)lpVoid;
	SetCurrentDirectory(USER_SAVE_PATH);
	pGI->vSI.clear();
	char szBuffer[100];
	if(pGI->hTI!=NULL)
	{
		sprintf(szBuffer,"%s (Loading...)",pGI->szGAME_NAME);
		TreeView_SetItemText(pGI->hTI,szBuffer);
		TreeView_SetItemState(g_hwndMainTreeCtrl,pGI->hTI,TVIS_BOLD ,TVIS_BOLD);		
	}

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
		pGI->vSI.clear();
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
						if(szGameType!=NULL)
							srv.cGAMEINDEX =  pGI->cGAMEINDEX; //(char)atoi(szGameType); 
						
						szGameName = strtok_s( NULL, seps, &next_token1);
						if(szGameName!=NULL)
							strcpy(srv.szServerName,TrimString(szGameName));

						szIP = strtok_s( NULL, seps, &next_token1);
						if(szIP!=NULL)
							strcpy(srv.szIPaddress,TrimString(szIP));
						
						szPort = strtok_s( NULL, seps, &next_token1);
						if(szPort!=NULL)
							srv.dwPort = (DWORD)atol(szPort); 

						szShortCountryName = strtok_s( NULL, seps, &next_token1);
						if(szShortCountryName!=NULL)
							strcpy(srv.szShortCountryName,TrimString(szShortCountryName));
						
						szPrivatePassword = strtok_s( NULL, seps, &next_token1);
						if(szPrivatePassword!=NULL)
							strcpy(srv.szPRIVATEPASS,TrimString(szPrivatePassword));

						szRCONPassword = strtok_s( NULL, seps, &next_token1);
						if(szRCONPassword!=NULL)
							strcpy(srv.szRCONPASS,TrimString(szRCONPassword));
						
						szVersion = strtok_s( NULL, seps, &next_token1);
						if(szVersion!=NULL)
							strcpy(srv.szVersion,TrimString(szVersion));
						
						szMod = strtok_s( NULL, seps, &next_token1);
						if(szMod!=NULL)
							strcpy(srv.szMod,TrimString(szMod));
						
						szMap = strtok_s( NULL, seps, &next_token1);
						if(szMap!=NULL)
							strcpy(srv.szMap,TrimString(szMap));
						
						szMaxPlayers = strtok_s( NULL, seps, &next_token1);
						if(szMaxPlayers!=NULL)
							srv.nMaxPlayers = atoi(szMaxPlayers); 

						szCurrentNumberOfPlayers = strtok_s( NULL, seps, &next_token1);
						if(szCurrentNumberOfPlayers!=NULL)
							srv.nCurrentPlayers = atoi(szCurrentNumberOfPlayers); 

						szPrivateClients = strtok_s( NULL, seps, &next_token1);
						if(szPrivateClients!=NULL)
							srv.nPrivateClients = atoi(szPrivateClients); 
						
						szPrivate = strtok_s( NULL, seps, &next_token1);
						if(szPrivate!=NULL)
							srv.bPrivate = (char)atoi(szPrivate); 
						
						szByte = strtok_s( NULL, seps, &next_token1);  //Punkbuster or VAC
						if(szByte!=NULL)
							srv.bPunkbuster = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  //Dedicated
						if(szByte!=NULL)
							srv.bDedicated = (BOOL)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);   //Ranked
						if(szByte!=NULL)
							srv.cRanked = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);   
						if(szByte!=NULL)
							srv.cBots = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);   
						if(szByte!=NULL)
							srv.cFavorite = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);   
						if(szByte!=NULL)
							srv.cHistory = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  
						if(szByte!=NULL)
							srv.cPure = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  
						if(szByte!=NULL)
							srv.dwGameType = (WORD)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  
						if(szByte!=NULL)
							srv.dwMap = (char)atoi(szByte); 
						
						szByte = strtok_s( NULL, seps, &next_token1);  
						if(szByte!=NULL)
							srv.cPurge = (char)atoi(szByte); 
					
						szByte = strtok_s( NULL, seps, &next_token1);  
						if(szByte!=NULL)
							srv.dwPing = (DWORD)atol(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  
						if(szByte!=NULL)
							srv.wMod = (WORD)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1); 
						if(szByte!=NULL)
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
						srv.bUpdated = 0;
						srv.bNeedToUpdateServerInfo = true;
						int hash = srv.dwIP + srv.dwPort;
						pGI->shash.insert(Int_Pair(hash,srv.dwIndex));
						pGI->vSI.push_back(srv);			
						pGI->dwTotalServers++;
	
		
						i = sizeof(buffer)+1;
					}  //endif
				}
			}  //if
		}
		fclose(fp);
	}
	if(pGI->hTI!=NULL)
	{
		sprintf(szBuffer,"%s (%d)",pGI->szGAME_NAME,pGI->dwTotalServers);
		TreeView_SetItemText(pGI->hTI,szBuffer);
		TreeView_SetItemState(g_hwndMainTreeCtrl,pGI->hTI,TVIS_BOLD ,TVIS_BOLD);		
	}
	return 0;
}


void SaveAllServerList()
{
	for(int i=0;i<GamesInfo.size();i++)
		SaveServerList(&GamesInfo[i]);
}

void LoadAllServerList()
{
	HANDLE hThread;
	DWORD dwThreadIdBrowser;
	hThread = NULL;		
	hThread = CreateThread( NULL, 0, &LoadAllServerListThread, (LPVOID)0,0, &dwThreadIdBrowser);                
	if (hThread == NULL) 
	{
		AddLogInfo(ETSV_WARNING,"CreateThread failed (%d)\n", GetLastError() ); 
	}
	else 
	{
		CloseHandle( hThread );
	}

}

DWORD WINAPI LoadAllServerListThread(LPVOID lpVoid)
{
	EnterCriticalSection(&LOAD_SAVE_CS);

	for(int i=0;i<GamesInfo.size();i++)
	{
		if(GamesInfo[i].vSI.size()==0) //Only try to load if no list exsists (needed for minimizing and restoring)
		{	
			GamesInfo[i].bLockServerList = TRUE;
			LoadServerListV2(&GamesInfo[i]);
			GamesInfo[i].bLockServerList = FALSE;
		}
	}
	
	LeaveCriticalSection(&LOAD_SAVE_CS);

	if(GamesInfo[g_currentGameIdx].bActive)
		SetCurrentViewTo(g_currentGameIdx);
	else
		SetCurrentViewTo(FindFirstActiveGame());

	if(GamesInfo[g_currentGameIdx].vSI.size()>0)
		OnActivate_ServerList(SCAN_FILTERED);
		
 return 0;
}



void DeleteAllServerLists()
{
	int ret = MessageBox(NULL,lang.GetString("AskDeleteServerList"),"Warning!",MB_OKCANCEL | MB_ICONWARNING);
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
			for(int i=0;i<GamesInfo.size();i++)
			{
				remove(GamesInfo[i].szFilename);
				GamesInfo[i].dwTotalServers = 0;
				sprintf(szBuffer,"%s (%d)",GamesInfo[i].szGAME_NAME,GamesInfo[i].dwTotalServers);
				if(GamesInfo[i].bActive)
					TreeView_SetItemText(GamesInfo[i].hTI,szBuffer);
			}
			MessageBox(NULL,lang.GetString("DeletedServerList"),"Info",MB_OK);
		
		}else
			MessageBox(NULL,lang.GetString("ErrorDeletingServerList"),"Warning!",MB_OKCANCEL | MB_ICONWARNING);
	}
}

void DeleteServerLists(char cGameIdx)
{
	int ret = MessageBox(NULL,lang.GetString("AskDeleteServerList"),"Warning!",MB_OKCANCEL | MB_ICONWARNING);
	if(ret == IDOK)
	{
		if(g_bRunningQueryServerList==false)
		{
			ListView_DeleteAllItems(g_hwndListViewVars);
			ListView_DeleteAllItems(g_hwndListViewPlayers);
			ListView_DeleteAllItems(g_hwndListViewServer);
			ListView_SetItemCount(g_hwndListViewServer,0);
		
			OutputDebugString(GamesInfo[cGameIdx].szGAME_NAME);
			OutputDebugString(" - CLEAN UP SERVERLIST\n");
				
			GamesInfo[cGameIdx].dwTotalServers = 0;
			GamesInfo[cGameIdx].vSI.clear();
			GamesInfo[cGameIdx].vRefListSI.clear();
			GamesInfo[cGameIdx].vRefScanSI.clear();
			GamesInfo[cGameIdx].shash.clear();

			g_CurrentSRV=NULL;

			char szBuffer[100];
			remove(GamesInfo[cGameIdx].szFilename);
			GamesInfo[cGameIdx].dwTotalServers = 0;
			sprintf(szBuffer,"%s (%d)",GamesInfo[cGameIdx].szGAME_NAME,GamesInfo[cGameIdx].dwTotalServers);
			if(GamesInfo[cGameIdx].bActive)
				TreeView_SetItemText(GamesInfo[cGameIdx].hTI,szBuffer);
	
			MessageBox(NULL,lang.GetString("DeletedServerList"),"Info",MB_OK);
		
		}else
			MessageBox(NULL,lang.GetString("ErrorDeletingServerList"),"Warning!",MB_OKCANCEL | MB_ICONWARNING);
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
	
	while(g_bRunningQueryServerList)
	{
		dbg_print("Sleeping...\n");
		Sleep(300);
	}

	while(1)
	{
		if(TryEnterCriticalSection(&LOAD_SAVE_CS)==FALSE)
		{
			OutputDebugString("Waiting for Loading to finish.\n"); 
			Sleep(1000);
			
		}else
			break;
	}
		

	SaveAllServerList();

	dbg_print("Cleaning up all linked list!\n");
	
	ClearAllServerLinkedList();
	

	LeaveCriticalSection(&LOAD_SAVE_CS);

	Buddy_Save(g_pBIStart);
	Buddy_Clear(g_pBIStart);
	g_pBIStart = NULL;
	
	//Save_CountryFilter();
	CFG_Save(0);
	if((DWORD)pvoid!=0x0000FFFF)
		PostMessage(g_hWnd,WM_DESTROY,(WPARAM)pvoid,0);
	
	TreeView_cleanup();
	g_sMIRCoutput.clear();
	g_bCancel = false;
	bWaitingToSave = FALSE;
	SCANNER_bCloseApp = FALSE;
	
	return 0x1001;
}
DWORD WINAPI CFG_Save(LPVOID lpVoid)
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
	comment->SetValue(" \nSettings for Game Scanner\n\nONLY CHANGE VALUES IF YOU KNOW WHAT YOU ARE DOING!\n " );  	
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
	
	for(int i=0;i<GamesInfo.size(); i++)
	{
		TiXmlElement * abc = new TiXmlElement( "Game" );  
		WriteCfgStr(abc, "GameData", "GameName",GamesInfo[i].szGAME_NAME) ;

		//WriteCfgStr(abc, "GameData", "yawn",GamesInfo[i].szMAP_YAWN_PATH);
		WriteCfgStr(abc, "GameData", "MasterServer",GamesInfo[i].szMasterServerIP);
		WriteCfgStr(abc, "GameData", "MapPreview",GamesInfo[i].szMAP_MAPPREVIEW_PATH);
		//WriteCfgStr(abc, "GameData", "webpath",GamesInfo[i].szMAP_OTHER_WEBPATH_PATH);
		//WriteCfgStr(abc, "GameData", "splatterladder",GamesInfo[i].szMAP_SPLATTERLADDER_PATH);
		WriteCfgStr(abc, "GameData", "ProtcolName",GamesInfo[i].szProtocolName);
		WriteCfgInt(abc, "GameData", "Active",GamesInfo[i].bActive);
		WriteCfgInt(abc, "GameData", "gametype",GamesInfo[i].cGAMEINDEX);
		WriteCfgInt(abc, "GameData", "MasterServerPort",GamesInfo[i].dwMasterServerPORT);
		//WriteCfgInt(abc, "GameData", "Protocol",GamesInfo[i].dwProtocol);
		WriteCfgInt(abc, "GameData", "IconIndex",GamesInfo[i].iIconIndex);
		WriteCfgInt(abc, "GameData", "FilterMod",GamesInfo[i].filter.dwMod);
		WriteCfgInt(abc, "GameData", "FilterRegion",GamesInfo[i].filter.dwRegion);
		WriteCfgInt(abc, "GameData", "FilterVersion",GamesInfo[i].filter.dwVersion);
		WriteCfgInt(abc, "GameData", "FilterMap",GamesInfo[i].filter.dwMap);
		WriteCfgInt(abc, "GameData", "FilterGameType",GamesInfo[i].filter.dwGameTypeFilter);
		WriteCfgInt(abc, "GameData", "FilterHideFull",GamesInfo[i].filter.bNoFull);
		WriteCfgInt(abc, "GameData", "FilterHideEmpty",GamesInfo[i].filter.bNoEmpty);
		WriteCfgInt(abc, "GameData", "FilterPunkbuster",GamesInfo[i].filter.bPunkbuster);
		WriteCfgInt(abc, "GameData", "FilterHideOffline",GamesInfo[i].filter.bHideOfflineServers);
		WriteCfgInt(abc, "GameData", "FilterPure",GamesInfo[i].filter.bPure);
		WriteCfgInt(abc, "GameData", "FilterRanked",GamesInfo[i].filter.bRanked);
		WriteCfgInt(abc, "GameData", "FilterHideBots",GamesInfo[i].filter.bNoBots);
		WriteCfgInt(abc, "GameData", "FilterNoPrivate",GamesInfo[i].filter.bNoPrivate);
		WriteCfgInt(abc, "GameData", "FilterOnlyPrivate",GamesInfo[i].filter.bOnlyPrivate);
		WriteCfgInt(abc, "GameData", "FilterDedicated",GamesInfo[i].filter.bDedicated);
		
		for(int n=0;n<GamesInfo[i].vGAME_INST.size();n++)
		{
			TiXmlElement * installs = new TiXmlElement( "Installs" ); 
			WriteCfgStr(installs, "Install", "Name",GamesInfo[i].vGAME_INST.at(n).sName.c_str());
			WriteCfgStr(installs, "Install", "Path",GamesInfo[i].vGAME_INST.at(n).szGAME_PATH.c_str());
			WriteCfgStr(installs, "Install", "Cmd",GamesInfo[i].vGAME_INST.at(n).szGAME_CMD.c_str());
			WriteCfgStr(installs, "Install", "LaunchByMod",GamesInfo[i].vGAME_INST.at(n).sMod.c_str());
			WriteCfgStr(installs, "Install", "LaunchByVer",GamesInfo[i].vGAME_INST.at(n).sVersion.c_str());
			abc->LinkEndChild( installs ); 
		}
	
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
		//WriteCfgStr(pElemSortIdx, "Column", "strval",CUSTCOLUMNS[i].sName.c_str()) ;		
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

	TiXmlElement * xmlColorFont = new TiXmlElement( "ColorEncodedFont" );  
	root->LinkEndChild( xmlColorFont );  
	xmlColorFont->SetAttribute("Enable", AppCFG.bUseColorEncodedFont);

	TiXmlElement * xmlServerRules = new TiXmlElement( "ServerRules" );  
	root->LinkEndChild( xmlServerRules );  
	xmlServerRules->SetAttribute("show", AppCFG.bShowServerRules);

	TiXmlElement * xmlPlayerList = new TiXmlElement( "PlayerList" );  
	root->LinkEndChild( xmlPlayerList );  
	xmlPlayerList->SetAttribute("show", AppCFG.bShowPlayerList);

	TiXmlElement * xmlLastTabView = new TiXmlElement( "LastGameView" );  
	root->LinkEndChild( xmlLastTabView );  
	xmlLastTabView->SetAttribute("index", g_currentGameIdx);

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
	xmlmIRC->SetAttribute("command", g_sMIRCoutput.c_str());

	
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
	
	TiXmlElement * xmlElmRet = new TiXmlElement( "NetworkRetries" );  
	root->LinkEndChild( xmlElmRet );  
	xmlElmRet->SetAttribute("value", AppCFG.dwRetries);

	TiXmlElement * xmlElmlang = new TiXmlElement( "CurrentLanguage" );  
	root->LinkEndChild( xmlElmlang );  
	xmlElmlang->SetAttribute("filename", AppCFG.szLanguageFilename);

	doc.SaveFile( "config.xml" );
	AddLogInfo(ETSV_DEBUG,"Saving config...DONE!");
	return 0;
}

void SetInitialViewStates()
{
	OutputDebugString("SetInitialViewStates\n");
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
	{
		mii.fState = MFS_CHECKED;
	}
	else	
	{
		mii.fState = MFS_UNCHECKED;
	}

	SetMenuItemInfo(hMenu,	ID_VIEW_BUDDYLIST,FALSE,&mii);
	
	if(AppCFG.bShowPlayerList)
		mii.fState = MFS_CHECKED;
	else	
		mii.fState = MFS_UNCHECKED;
	SetMenuItemInfo(hMenu,	ID_VIEW_PLAYERLIST,FALSE,&mii);

	OnSize(g_hWnd);					
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
	
	while(g_bRunningQueryServerList)
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

	CFG_Save(0);

	ShowWindow(hWnd,SW_HIDE);
	//Removed since ver 5.41
	//ShowBalloonTip("Notification","ET Server Viewer is still running.\nDouble click to activate ETSV\nRight click to exit the application.");
}

int SetCurrentViewTo(int index)
{
	g_currentGameIdx = index;
	//Clear old
	//if(currCV!=NULL)
	//	currCV->vRefListSI.clear();

	currCV = &GamesInfo[g_currentGameIdx];
	if(g_hwndMainTreeCtrl==NULL)
		return 0;
	//Deselect
	for(int i=0;i<GamesInfo.size();i++)
		TreeView_SetItemState(g_hwndMainTreeCtrl,GamesInfo[i].hTI,0 , TVIS_SELECTED );


	TreeView_SetItemState(g_hwndMainTreeCtrl,GamesInfo[g_currentGameIdx].hTI,TVIS_SELECTED, TVIS_SELECTED);
	TreeView_Expand(g_hwndMainTreeCtrl, GamesInfo[g_currentGameIdx].hTI, TVE_EXPAND);
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
	
	sprintf(szPath,"%s\\%s\\",COMMON_SAVE_PATH,GamesInfo[gameindex].szMAP_MAPPREVIEW_PATH);
	mbstowcs(wcPath,(const char *)szPath,strlen(szPath));		
	SHCreateDirectory(NULL,wcPath);
	
	sprintf(szDestPath,"%s%s.png",szPath,g_TrimmedSSFileName);
	sprintf(szBuff,"http://www.cludden.se/braindamage/download/screenshots/%s/%s",GamesInfo[gameindex].szProtocolName,szDestPath);


	//HFD_SetPath(szPath);

	int iRet = HttpFileDownload(szBuff,szDestPath,g_DlgProgress,g_DlgProgressMsg);
	if(iRet!=0)
	{
		SetCurrentDirectory(szPath);
		remove(szDestPath);
		
		sprintf(szDestPath,"%s%s.jpg",szPath,g_TrimmedSSFileName);
		sprintf(szBuff,"http://www.cludden.se/braindamage/download/screenshots/%s/%s",GamesInfo[gameindex].szProtocolName,szDestPath);
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
			sprintf(szPath,"%s%s\\%s.jpg",COMMON_SAVE_PATH,GamesInfo[g_currentGameIdx].szMAP_MAPPREVIEW_PATH,start);
			dib = FreeImage_Load(FIF_JPEG, szPath, JPEG_DEFAULT);
			if(!dib)
			{
				sprintf(szPath,"%s%s\\%s.png",COMMON_SAVE_PATH,GamesInfo[g_currentGameIdx].szMAP_MAPPREVIEW_PATH,start);
				dib = FreeImage_Load(FIF_PNG, szPath, PNG_DEFAULT);

				if(!dib)
				{

					sprintf(szPath,"%s\\%s\\%s.jpg",EXE_PATH,GamesInfo[g_currentGameIdx].szMAP_MAPPREVIEW_PATH,start);
					dib = FreeImage_Load(FIF_JPEG, szPath, JPEG_DEFAULT);

					if(!dib)
					{
						sprintf(szPath,"%s\\%s\\%s.png",EXE_PATH,GamesInfo[g_currentGameIdx].szMAP_MAPPREVIEW_PATH,start);
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

		Rectangle(hDC,  g_rcDestMapImg.left, g_rcDestMapImg.top,g_rcDestMapImg.right,g_rcDestMapImg.bottom);

		if(dib!=NULL)
		{
			SetStretchBltMode(hDC, COLORONCOLOR);
			StretchDIBits(hDC, 
				g_rcDestMapImg.left, 
				g_rcDestMapImg.top, 
				g_rcDestMapImg.right-g_rcDestMapImg.left, 
				g_rcDestMapImg.bottom-g_rcDestMapImg.top, 
			0, 0, FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),
			FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS, SRCCOPY);
			FreeImage_Unload(dib);
		}

		
	} 

	pt.y = g_INFOIconRect.top; //rc.bottom;
	pt.x = g_INFOIconRect.left;//rc.left+2;

	if(g_hImageListIcons!=NULL)
	{
		ImageList_Draw(g_hImageListIcons,g_statusIcon,hDC,pt.x,pt.y,ILD_TRANSPARENT);
		//InvalidateRect(g_hWnd,&g_INFOIconRect,FALSE);
	}
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
					WNDCONT[WIN_BUDDYLIST].bShow = FALSE;
				break;
				case ID_VIEW_PLAYERLIST: 
					AppCFG.bShowPlayerList = FALSE;
					SplitterGripArea[2].tvYPos =  rc.bottom * 1;
					WNDCONT[WIN_TABCONTROL].bShow = FALSE;
					WNDCONT[WIN_PLAYERS].bShow = FALSE;
					WNDCONT[WIN_RCON].bShow = FALSE;
					WNDCONT[WIN_RULES].bShow = FALSE;
					WNDCONT[WIN_LOGGER].bShow = FALSE;
					WNDCONT[WIN_PING].bShow = FALSE;
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
				case ID_VIEW_SERVERRULES: 
					AppCFG.bShowServerRules = true;
				break;
				case ID_VIEW_BUDDYLIST: 

					SplitterGripArea[0].tvYPos = ((rc.bottom ) *  0.65f)  ;
					AppCFG.bShowBuddyList = TRUE;
					WNDCONT[WIN_BUDDYLIST].bShow = TRUE;
				break;
				case ID_VIEW_PLAYERLIST: 
					AppCFG.bShowPlayerList = true;
					WNDCONT[WIN_TABCONTROL].bShow = TRUE;
					WNDCONT[WIN_PLAYERS].bShow = TRUE;
					WNDCONT[WIN_RCON].bShow = FALSE;
					WNDCONT[WIN_RULES].bShow = FALSE;
					WNDCONT[WIN_LOGGER].bShow = FALSE;
					WNDCONT[WIN_PING].bShow = FALSE;
					
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
	OutputDebugString("Initialize_WindowSizes\n");
	RECT rc;
	GetClientRect(g_hWnd, &rc);

	rc.top = TOOLBAR_Y_OFFSET;
	rc.bottom-= (STATUSBAR_Y_OFFSET + TOOLBAR_Y_OFFSET); //reduce the size of statusbar from the main window size

	float fBuddyScale=0;
	if(AppCFG.bShowBuddyList)	
		fBuddyScale = 0.4f;
	else
		WNDCONT[WIN_BUDDYLIST].bShow = FALSE;
	

	SetRect(&WNDCONT[WIN_MAINTREEVIEW].rSize,0,TOOLBAR_Y_OFFSET,rc.right*0.2,rc.bottom*(1.0f-fBuddyScale));

	int offSetX = WNDCONT[WIN_MAINTREEVIEW].rSize.right + BORDER_SIZE; //get offset for next window to start at 
	int offSetY = TOOLBAR_Y_OFFSET+WNDCONT[WIN_MAINTREEVIEW].rSize.bottom + BORDER_SIZE; //get offset for next window to start at 

	SplitterGripArea[0].tvYPos = offSetY ;
//	SplitterGripArea[0].tvXPos = offSetX + TOOLBAR_Y_OFFSET ;
	SplitterGripArea[1].tvXPos = offSetX;

	SplitterGripArea[2].tvYPos = offSetY ;

	if(AppCFG.bShowPlayerList==false)		
	{
		SplitterGripArea[2].tvYPos =  rc.bottom * 1;
		WNDCONT[WIN_PLAYERS].bShow = FALSE;
		WNDCONT[WIN_RULES].bShow = FALSE;
		WNDCONT[WIN_TABCONTROL].bShow = FALSE;
	}
	else
	{
		SplitterGripArea[2].tvYPos =  rc.bottom * 0.6;
		WNDCONT[WIN_PLAYERS].bShow = TRUE;		
		WNDCONT[WIN_TABCONTROL].bShow = TRUE;
	}


	SetRect(&WNDCONT[WIN_SERVERLIST].rSize,offSetX,TOOLBAR_Y_OFFSET,rc.right-WNDCONT[WIN_MAINTREEVIEW].rSize.right,SplitterGripArea[2].tvYPos);


	SetRect(&WNDCONT[WIN_BUDDYLIST].rSize,0,offSetY,offSetX,rc.bottom*fBuddyScale);
	
	int iShow = rc.right * 1;
	if(AppCFG.bShowMapPreview)
		iShow =  rc.right * 0.4;

	SetRect(&WNDCONT[WIN_TABCONTROL].rSize,offSetX,offSetY,iShow,TABSIZE_Y);
	
	SetRect(&WNDCONT[WIN_PLAYERS].rSize,offSetX,offSetY+TABSIZE_Y,iShow,(rc.bottom*0.4)-TABSIZE_Y);
	SetRect(&WNDCONT[WIN_RULES].rSize,offSetX,offSetY+TABSIZE_Y,iShow,(rc.bottom*0.4)-TABSIZE_Y);
	SetRect(&WNDCONT[WIN_RCON].rSize,offSetX,offSetY+TABSIZE_Y,iShow,(rc.bottom*0.4)-TABSIZE_Y);
	SetRect(&WNDCONT[WIN_PING].rSize,offSetX,offSetY+TABSIZE_Y,iShow,(rc.bottom*0.4)-TABSIZE_Y);
	SetRect(&WNDCONT[WIN_LOGGER].rSize,offSetX,offSetY+TABSIZE_Y,iShow,(rc.bottom*0.4)-TABSIZE_Y);
	
	offSetX = WNDCONT[WIN_MAINTREEVIEW].rSize.right + WNDCONT[WIN_TABCONTROL].rSize.right; //get offset for next window to start at 


	SetRect(&WNDCONT[WIN_MAPPREVIEW].rSize,offSetX,offSetY,rc.right*0.2,rc.bottom*0.4);


	SetRect(&WNDCONT[WIN_STATUS].rSize,25,offSetY+WNDCONT[WIN_BUDDYLIST].rSize.bottom+BORDER_SIZE+4,(rc.right*0.6)-25,STATUSBAR_Y_OFFSET);
	
	SetRect(&WNDCONT[WIN_PROGRESSBAR].rSize,
		WNDCONT[WIN_STATUS].rSize.right+25,
		(TOOLBAR_Y_OFFSET+rc.bottom)+5,
		rc.right*0.4,
		(STATUSBAR_Y_OFFSET-5));

	g_INFOIconRect.top = WNDCONT[WIN_STATUS].rSize.top;
}




void Update_WindowSizes()
{
	OutputDebugString("Update_WindowSizes\n");
	RECT rc;
	GetClientRect(g_hWnd, &rc);

	rc.top = TOOLBAR_Y_OFFSET;
	rc.bottom-= (STATUSBAR_Y_OFFSET + TOOLBAR_Y_OFFSET); //reduce the size of statusbar from the main window size

	if(WNDCONT[WIN_BUDDYLIST].bShow==FALSE)	
		SplitterGripArea[0].tvYPos = ((rc.bottom ) * 1);
	
	if(AppCFG.bShowPlayerList==false)		
		SplitterGripArea[2].tvYPos =  rc.bottom * 1;

	SetRect(&WNDCONT[WIN_MAINTREEVIEW].rSize,0,TOOLBAR_Y_OFFSET,SplitterGripArea[1].tvXPos,SplitterGripArea[0].tvYPos);

	int offSetX = WNDCONT[WIN_MAINTREEVIEW].rSize.right + BORDER_SIZE; //get offset for next window to start at 
	int offSetY = TOOLBAR_Y_OFFSET+WNDCONT[WIN_MAINTREEVIEW].rSize.bottom + BORDER_SIZE; //get offset for next window to start at 

	SetRect(&WNDCONT[WIN_SERVERLIST].rSize,offSetX,TOOLBAR_Y_OFFSET,rc.right-(WNDCONT[WIN_MAINTREEVIEW].rSize.right+BORDER_SIZE),SplitterGripArea[2].tvYPos);


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
	
	SetRect(&WNDCONT[WIN_STATUS].rSize,25,WNDCONT[WIN_BUDDYLIST].rSize.top+WNDCONT[WIN_BUDDYLIST].rSize.bottom+4,(rc.right*0.6)-25,STATUSBAR_Y_OFFSET);
	SetRect(&WNDCONT[WIN_PROGRESSBAR].rSize,
		WNDCONT[WIN_STATUS].rSize.right+25,
		(TOOLBAR_Y_OFFSET+rc.bottom)+5,
		rc.right*0.4,
		STATUSBAR_Y_OFFSET-5);

	
	
	g_rcDestMapImg.left = rc.right-(ImageSizeX+BORDER_SIZE); 
	g_rcDestMapImg.top = (rc.bottom - ImageSizeX) +  (TOOLBAR_Y_OFFSET + BORDER_SIZE);
	g_rcDestMapImg.right = g_rcDestMapImg.left + ImageSizeX;
	g_rcDestMapImg.bottom = g_rcDestMapImg.top + ImageSizeX;
	SetRect(&WNDCONT[WIN_MAPPREVIEW].rSize,g_rcDestMapImg.left,g_rcDestMapImg.top,g_rcDestMapImg.right,g_rcDestMapImg.bottom);
	g_INFOIconRect.top = WNDCONT[WIN_STATUS].rSize.top;

}



//void ResizeListView(HWND g_hwndListViewPlayers,HWND g_hwndListViewVars, HWND hwndParent, BOOL bRepaint)
void OnSize(HWND hwndParent, BOOL bRepaint)
{
	RECT  rc;
	
	dbg_print("OnSize");
	GetClientRect(hwndParent, &rc);
	rc.top = TOOLBAR_Y_OFFSET;
	rc.bottom-= (STATUSBAR_Y_OFFSET + TOOLBAR_Y_OFFSET);

	//GetClientRect(hwndParent, &g_INFOIconRect);
	g_INFOIconRect.left=2;
	
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

int LoadIconIntoImageList(char*szFilename)
{
	int index=7;
	char szTemp[100];
	strcpy(szTemp,".\\GameIcons\\");
	strcat(szTemp,szFilename);

	HICON hIcon = (HICON) LoadImage(NULL,szTemp,IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),LR_LOADFROMFILE);
	if(hIcon!=NULL)
	{	
		index = ImageList_AddIcon(g_hImageListIcons, hIcon);
		if(index==-1)
			index=7;
		DestroyIcon(hIcon);
	}
	return index;
}

void LoadImageList()
{

	if(g_hImageListIcons!=NULL)
		return;
	dbg_print("Image list\n");

	g_hImageListIcons = ImageList_Create(16, 16, ILC_COLOR32|ILC_MASK,35, 1);

	HICON hIcon;
	int i=0;
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_USERS));		    //0 
	i = ImageList_AddIcon(g_hImageListIcons, hIcon);
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


	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_VAC)); //21
	ImageList_AddIcon(g_hImageListIcons, hIcon);

	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_RCON)); //22
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_LOGGER)); //23
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_FONT)); //24
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_PAINT)); //25
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_STATS)); //26
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_RULES)); //27
	ImageList_AddIcon(g_hImageListIcons, hIcon);

/*

	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_QUAKE)); //21 52         ddd
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_ETQW)); //22 56 ET QW ICONd     ddd
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_RTCW)); //23 63 RTCW       ddddd
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_Q3)); //24 74            dd
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_WARSOW)); //25 78                  dd
	ImageList_AddIcon(g_hImageListIcons, hIcon);	
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_COD)); //26 80
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_COD2)); //27 79         dd
	ImageList_AddIcon(g_hImageListIcons, hIcon);	
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_COD4)); //28 81     ddd
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_CS)); //29          dd
	ImageList_AddIcon(g_hImageListIcons, hIcon);

	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_CSS)); //31  ddd
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_USERS)); //32  ddd
	ImageList_AddIcon(g_hImageListIcons, hIcon);

*/
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

BOOL g_bRunSimulation=FALSE;

DWORD WINAPI Simulation(LPVOID lpParam )
{
	Sleep(1000);
	while(g_bRunSimulation)
	{
		UINT max = TreeView_GetCount(g_hwndMainTreeCtrl);

		UINT n = rand() % max;
	
		TreeView_SelectItem(g_hwndMainTreeCtrl,vTI.at(n).hTreeItem);
		TreeView_SetItemState(g_hwndMainTreeCtrl,vTI.at(n).hTreeItem,TVIS_SELECTED, TVIS_SELECTED);
		TreeView_GetSelectionV3();
		Sleep(1000);
	}

	return 0;
}

//7Used for debug???

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
//	if(currGameIdx==g_currentGameIdx)
//		Initialize_RedrawServerListThread();
	
	if(g_bRunningQueryServerList)	
		return 0;

	g_currentScanGameIdx = g_currentGameIdx;
	g_bRunningQueryServerList = true;

	if (! ResetEvent(hCloseEvent) ) 
        dbg_print("ResetEvent failed\n");

	time(&GamesInfo[currGameIdx].lastScanTimeStamp);

	HANDLE hThread=NULL; 
	DWORD dwThreadIdBrowser;
		
	if(options==SCAN_ALL_GAMES)
	{
		g_bRunSimulation = TRUE;
		hThread = CreateThread( NULL, 0, &Simulation, (LPVOID)0,0, &dwThreadIdBrowser);                
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

	int iGame=0;
nextGame:
	if(options==SCAN_ALL_GAMES)
		currGameIdx = iGame++;
	if(iGame==GamesInfo.size())  //reset
		currGameIdx = iGame = 0;

	SetStatusText(GamesInfo[currGameIdx].iIconIndex,lang.GetString("StatusReceivingServers"),GamesInfo[currGameIdx].szGAME_NAME);

	SCAN_Set_CALLBACKS(GamesInfo[currGameIdx].GetServerStatus,&UpdateServerItem);

	if((DWORD)lpParam==SCAN_FILTERED)
		Initialize_Rescan2(&GamesInfo[currGameIdx],&FilterServerItemV2);
	else
	{
		if(GamesInfo[currGameIdx].bUseHTTPServerList)
		{
			HFD_SetPath(USER_SAVE_PATH);
			int ret = HttpFileDownload(GamesInfo[currGameIdx].szMasterServerIP,"servers.txt",NULL,NULL);
			if(ret!=0)
				goto exitError;
			Parse_FileServerList(&GamesInfo[currGameIdx],"servers.txt");
		}else	
		{	
			if(GamesInfo[currGameIdx].GetServersFromMasterServer!=NULL)
				GamesInfo[currGameIdx].GetServersFromMasterServer(&GamesInfo[currGameIdx]);
			else
			{
				AddLogInfo(ETSV_ERROR,"%s didn't have a valid funtion to recieve servers.",GamesInfo[currGameIdx].szGAME_NAME);
				goto exitError;
			}
		}
		Initialize_RedrawServerListThread();
		Initialize_Rescan2(&GamesInfo[currGameIdx],NULL);
	}

	char szBuffer[100];
	sprintf(szBuffer,"%s (%d)",GamesInfo[currGameIdx].szGAME_NAME,GamesInfo[currGameIdx].dwTotalServers);
	TreeView_SetItemText(GamesInfo[currGameIdx].hTI,szBuffer);	
	if((options==SCAN_ALL_GAMES) && (g_bCancel==false))
		goto nextGame;

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
		Show_StopScanningButton(FALSE);
		g_currentScanGameIdx = -1;
		g_bRunSimulation = FALSE;
		return 0xFFFF;
	}
	if(GamesInfo[currGameIdx].vSI.size()==0)
		goto exitError;

	SetStatusText(ICO_INFO,lang.GetString("StatusReceivingServersDone"),GamesInfo[currGameIdx].szGAME_NAME);
	
	//We don't want to overdraw wrong serverlist
	if(currGameIdx==g_currentGameIdx)
	{
		g_currentScanGameIdx = -1;
		//Modified since ver 5.25		
		Initialize_RedrawServerListThread();
		//RedrawServerListThread(&GamesInfo[currGameIdx]);
		
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
	SetStatusText(ICO_WARNING,lang.GetString("StatusReceivingServersError"),GamesInfo[currGameIdx].szGAME_NAME);
NoError:
   g_bRunSimulation = FALSE;
   g_currentScanGameIdx = -1;
   Show_StopScanningButton(FALSE);
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



bool FilterServerItemV2(LPARAM *lp,GAME_INFO *pGI)
{
	DWORD dwFilterFlags = pGI->dwViewFlags;

	DWORD bForceFavorites = (dwFilterFlags & REDRAWLIST_FAVORITES_PUBLIC);
	DWORD bForceHistory = (dwFilterFlags & REDRAWLIST_HISTORY) ;
	DWORD bForceFavoritesPrivate = dwFilterFlags & REDRAWLIST_FAVORITES_PRIVATE;
	DWORD bRescanFilter = dwFilterFlags & FORCE_SCAN_FILTERED;

	SERVER_INFO *srv=NULL;
	bool returnVal=false;

	__try
	{
		srv = (SERVER_INFO*)lp;

		if(pGI->dwViewFlags & REDRAW_SERVERLIST)
			bRescanFilter = 0; //force filter when only redrawing the serverlist looks better for user

		//If scanning filtered (limited number of servers) ignore following filter options:
		if((bRescanFilter==0))
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
			{
				if(srv->bNeedToUpdateServerInfo)
					returnVal=true;
				else
					return false;
			}
		
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
		
			if(pGI->filter.bNoBots)
				if(srv->cBots>0)
					return false;

			if(pGI->filter.dwMap>0)
			{
				returnVal=false;
				DWORD val = 1;
				for(int i=0;i<pGI->vFilterMap.size();i++)
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
				for(int i=0;i<pGI->vFilterVersion.size();i++)
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
		}

		if(pGI->filter.bRanked && (srv->cGAMEINDEX == ETQW_SERVERLIST))
			if(srv->cRanked==0)
				return false;

		if(pGI->filter.bPure)
			if(srv->cPure==0)
				return false;

		if(pGI->filter.bDedicated)
			if(srv->bDedicated==0)
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
		for(int i=0;i<pGI->vFilterGameType.size();i++)
		{
			if(pGI->filter.dwGameTypeFilter & val)
			{
				DWORD result = (srv->dwGameType & val);
		
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
		for(int i=0;i<pGI->vFilterMod.size();i++)
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

		returnVal=false;

		if(CountryFilter.counter!=0)
		{
			for(int i=0; i<CountryFilter.counter;i++)
			{
				if(_stricmp(srv->szShortCountryName,CountryFilter.szShortCountryName[i])==0)//if(srv->cCountryFlag == CountryFilter.countryIndex[i])
				{
					returnVal=true;
					break;
				} 
			}
		}
		else
			returnVal=true;

	}
	__except(EXCEPTION_ACCESS_VIOLATION == GetExceptionCode())
	{
		// exception handling code
		dbg_print("Access Violation!!! @ FilterServerItem(...)\n");	
		return false;
	}

	if(returnVal==false)
		return false;
	return true;
}


DWORD WINAPI  RedrawServerListThread(LPVOID pvoid )
{
//	if(g_bRedrawServerListThread==TRUE)
//		return 0xDEADFACE;

	if(TryEnterCriticalSection(&REDRAWLIST_CS)==FALSE)
		return 0xDEADFACE;

	int gameIdx = (int)pvoid;

	GAME_INFO *pGI = &GamesInfo[gameIdx];//(GAME_INFO *)pvoid;
	
	vSRV_INF::iterator  iLst;

	pGI->dwViewFlags |= REDRAW_SERVERLIST;
	
//	SendMessage(g_hwndMainSTATS,WM_STOP_PING,0,0);
	dbg_print("View flags: %d \n idx %d \nList Size %d",pGI->dwViewFlags,gameIdx,pGI->vSI.size());
	ListView_DeleteAllItems(g_hwndListViewServer);
	pGI->vRefListSI.clear();
	if(pGI->vSI.size()>0)
	{
		try{
			for ( iLst = pGI->vSI.begin( ); iLst != pGI->vSI.end( ); iLst++ )
			{
				SERVER_INFO pSI = *iLst;
			
				REF_SERVER_INFO refSI;
				refSI.dwIndex = pSI.dwIndex;
				refSI.cGAMEINDEX = pSI.cGAMEINDEX;

				if(FilterServerItemV2((LPARAM*)&pSI,pGI))
				{
					pGI->vRefListSI.push_back(refSI);
				}
			}			
		}catch (const exception& e)
		{
			AddLogInfo(ETSV_ERROR,"Vector err exception Details: %s",e.what());
			DebugBreak();
		}
	}
	LeaveCriticalSection(&REDRAWLIST_CS);
	Do_ServerListSort((LPVOID)iLastColumnSortIndex);

	if(pGI->dwViewFlags & REDRAW_SERVERLIST)
		pGI->dwViewFlags ^= REDRAW_SERVERLIST;


	if(pGI->dwViewFlags & FORCE_SCAN_FILTERED)
		pGI->dwViewFlags ^= FORCE_SCAN_FILTERED;


	dbg_print("Created filtered serverlist! View flags %d\n Number of servers in list %d \n",pGI->dwViewFlags,pGI->vRefListSI.size());

	//ListView_SetItemCount(g_hwndListViewServer,pGI->vRefListSI.size());

	
	return 0;
}

//Try to avoid use this func
void Initialize_RedrawServerListThread()
{
		HANDLE hThread;
		DWORD dwThreadIdBrowser;
//		AddLogInfo(ETSV_DEBUG,  "executing InitilizeRedrawServerListThread");	

		if(g_currentGameIdx==-1) //since v5.41
			return;

	//	if(bRunningRefreshThread==FALSE)
		{
	//		bRunningRefreshThread = TRUE;
			dbg_print("Creating RedrawServerListThread thread!\n");
			hThread = NULL;		
			int gameIdx = g_currentGameIdx;
			hThread = CreateThread( NULL, 0, &RedrawServerListThread, (LPVOID)gameIdx,0, &dwThreadIdBrowser);                
			if (hThread == NULL) 
			{
				AddLogInfo(ETSV_WARNING,"CreateThread failed (%d)\n", GetLastError() ); 
			}
			else 
			{
				CloseHandle( hThread );
			}
		}
//		AddLogInfo(ETSV_DEBUG,  "executing InitilizeRedrawServerListThread done");
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
	pGI->dwTotalServers = pGI->vSI.size();
}


char Get_GameIcon(char index)
{
	return GamesInfo[index].iIconIndex;
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
		case OPENARENA_SERVERLIST:
			return 41;
		case HL2_SERVERLIST:
			return 42;
		case UTERROR_SERVERLIST:
			return 43;
		default:
			return 7;  //unkown icon
	}

}
//This is only for reference purpose
long InsertServerItem(GAME_INFO *pGI,SERVER_INFO pSI)
{
	LVITEM lvItem;
	ZeroMemory(&lvItem, sizeof(LVITEM));

	if(pSI.cGAMEINDEX != g_currentGameIdx)
	{
		//dbg_print("wrong RefList");
		return 2;
	}

	lvItem.iItem =  ListView_GetItemCount(g_hwndListViewServer);
	

	REF_SERVER_INFO refSI;


	refSI.cGAMEINDEX  = pSI.cGAMEINDEX;
	refSI.dwIndex = pSI.dwIndex;	
	if(TryEnterCriticalSection(&REDRAWLIST_CS)==FALSE)
	{
		dbg_print("busy RefList");
		return 1;
	}
	pGI->vRefListSI.push_back(refSI);
	LeaveCriticalSection(&REDRAWLIST_CS);

	//return 0; //Game scanner 1.0
	

//	ListView_SetItemCount(g_hwndListViewServer,currCV->vRefListSI.size());

	lvItem.mask = LVIF_IMAGE ;	
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

struct ET_COLOR_CODES
{
	COLORREF color;
};

ET_COLOR_CODES ET_CC[]=
{
	0x00000000, // space
	RGB(0xFF,00,00), //! 
	0x00000000, //"
	RGB(0x7f,00,0x7f), //#
	RGB(0x00,0x7f,0xff), //$
	RGB(0x66,0x10,0xbe), //%
	RGB(0x33,0x99,0xcc), //&
	RGB(0xae,0xd9,0xae), //'
	RGB(0x03,0x63,0x33), //(
	RGB(0xf2,0x03,0x33), //)
	RGB(0x9d,0x9d,0x9d), //*
	RGB(0x99,0x33,0x00),//+
	0x00,       //,
	RGB(0x7f,00,00),//-
	RGB(0x27,0x2b,0x27),//.
	RGB(0xd6,0xd7,0x6e),// /
	0x00,// 0
	RGB(0xFF,0x00,0x00),// 1
	RGB(0x00,0xFF,0x00),// 2
	RGB(0xc9,0xca,0x09),// 3
	RGB(0x00,0x00,0xff),// 4
	RGB(0x00,0xff,0xff),// 5
	RGB(0x4a,0x2c,0x4a),// 6
	RGB(0xff,0xff,0xff),// 7
	RGB(0x78,0x50,0x28),// 8
	RGB(0x7f,0x7f,0x7f),// 9
	RGB(0xac,0xad,0xac),// :
	0x00,// ;
	RGB(0x00,0x7f,0x00),// <
	RGB(0x7f,0x7f,0x00),// =
	RGB(0x02,0x02,0x79),// >
	RGB(0x79,0x02,0x02),// ?
	RGB(0x35,0x36,0x36),// @
	RGB(0x34,0x31,0x26),// A
	RGB(0x04,0x74,0x74),// B
	RGB(0x74,0x05,0x74),// C
	RGB(0x1f,0x3a,0x4f),// D
	RGB(0x7f,0x00,0xff),// E
	RGB(0x33,0x99,0xcc),// F
	RGB(0xcc,0xff,0xcc),// G
	RGB(0x00,0x66,0x33),// H
	RGB(0xff,0x00,0x33),// I
	RGB(0x27,0x2b,0x27),// J
	RGB(0x99,0x33,0x00),// K
	RGB(0x99,0x33,0x00),// L
	RGB(0x75,0x76,0x2f),// M
	RGB(0xe4,0xe4,0xac),// N
	RGB(0xe5,0xe5,0x75),// O
	RGB(0x06,0x07,0x06),// P  //black?
	RGB(0xff,0x00,0x00),// Q
	RGB(0x00,0xff,0x00),// R
	RGB(0xf1,0xf1,0x02),// S
	RGB(0x0a,0x0b,0xc4),// T
	RGB(0x02,0xf1,0xf1),// U
	RGB(0xf1,0x02,0xf1),// V
	RGB(0xff,0xff,0xff),// W
	RGB(0xe4,0x74,0x04),// X
	RGB(0x79,0x79,0x79),// Y
	RGB(0xbf,0xbf,0xbf),// Z
	RGB(0xae,0xae,0xae),// [
	RGB(0x02,0x79,0x02), // /*\*/
	RGB(0x6a,0x6b,0x14), // ]
	0x00, // ^
	RGB(0x7f,00,00),//_
	0x00000000,//´

};



//Fast look up
COLORREF GetColor(char inC)
{
	if(inC<0)
		return 0;
	char c = inC;
	if(isalpha(inC))
	 c = _tolower(inC);
	int len = sizeof(ET_CC)/sizeof(DWORD);
	if((c-' ')<len)
		return ET_CC[c-' '].color;
	return 0x0000000;
}

LRESULT Draw_ColorEncodedText(RECT rc, LPNMLVCUSTOMDRAW pListDraw , char *pszText)
{
	HDC  hDC =  pListDraw->nmcd.hdc;
	HBRUSH hbrSel= NULL;
	hbrSel = CreateSolidBrush( RGB(0x28,0x2c,0x28)); 														
	FillRect(hDC, &rc, (HBRUSH) hbrSel);

	if( pListDraw->nmcd.uItemState & ( CDIS_SELECTED))
	{
		pListDraw->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT); //RGB(255, 255, 255);
		hbrSel = CreateSolidBrush( GetSysColor(COLOR_HIGHLIGHT)); //RGB(51,153,250)); 																
		FillRect(hDC, &rc, (HBRUSH) hbrSel);
	}
	int nCharWidth;
	SelectObject(hDC,g_hf2);
	//GetCharWidth32(hDC, (UINT) 0, (UINT) 0, &nCharWidth); 				
	ABC abc[256];
	LPABC pAbc = abc;
	GetCharABCWidths(hDC,       (UINT)0, (UINT) 255,pAbc);
	char *pText;

	rc.left+=20;
	//rc.top+=2;
	COLORREF col = RGB(255,255,255) ;
	for(int i=0;i<strlen(pszText);i++)
	{
		
		if(pszText[i]=='^')
		{
			col = GetColor(pszText[i+1]);
			i++;
			if(pszText[i]!='^') // this fixes these kind of names with double ^^
				continue;
		}
		
		SetTextColor(hDC,col);
		pText = &pszText[i];		
		
		ExtTextOut(hDC,rc.left,rc.top,0, &rc,pText, 1,NULL); 
	
		nCharWidth = abc[pszText[i]].abcA + abc[pszText[i]].abcB + abc[pszText[i]].abcC;
		rc.left+=nCharWidth;

	}
	SelectObject(hDC,g_hf);								
	if(hbrSel!=NULL)
		DeleteObject(hbrSel);

	return   (CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT);
}

LRESULT Draw_ColorEncodedTextQ4(RECT rc, LPNMLVCUSTOMDRAW pListDraw , char *pszText)
{
	HDC  hDC =  pListDraw->nmcd.hdc;
	HBRUSH hbrSel= NULL;
	hbrSel = CreateSolidBrush( RGB(0x28,0x2c,0x28)); 														
	FillRect(hDC, &rc, (HBRUSH) hbrSel);

	if( pListDraw->nmcd.uItemState & ( CDIS_SELECTED))
	{
		pListDraw->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT); //RGB(255, 255, 255);
		hbrSel = CreateSolidBrush( GetSysColor(COLOR_HIGHLIGHT)); //RGB(51,153,250)); 																
		FillRect(hDC, &rc, (HBRUSH) hbrSel);
	}
	int nCharWidth;
	SelectObject(hDC,g_hf2);
	//GetCharWidth32(hDC, (UINT) 0, (UINT) 0, &nCharWidth); 				
	ABC abc[256];
	LPABC pAbc = abc;
	GetCharABCWidths(hDC,       (UINT)0, (UINT) 255,pAbc);  //supports true type font
	char *pText;
	rc.left+=20;
	rc.top+=2;
	COLORREF col = RGB(255,255,255) ;

	for(int i=0;i<strlen(pszText);i++)
	{
		if((pszText[i]=='^') && (pszText[i+1]=='i') ) //&& (pszText[i+2]=='d') )//&& (isNumeric(pszText[i+4]))) //Q4 ^idm0 and ^idm1 icons
		{
			i+=4;
			continue;
		}
		else if((pszText[i]=='^') && (pszText[i+1]=='c') )
		{
			int r = pszText[i+2]-48;
			int g = pszText[i+3]-48;
			int b = pszText[i+4]-48;
			col = RGB(28*r,28*g,28*b) ;

			i+=4;
			continue;
		}
		else if(pszText[i]=='^')
		{
			col = GetColor(pszText[i+1]);

			i++;
			continue;
		}
		SetTextColor(hDC,col);
		pText = &pszText[i];

		ExtTextOut(hDC,rc.left,rc.top,0, &rc,pText, 1,NULL); 
	    nCharWidth = abc[pszText[i]].abcA + abc[pszText[i]].abcB + abc[pszText[i]].abcC;
		rc.left+=nCharWidth;

	}
	SelectObject(hDC,g_hf);								
	if(hbrSel!=NULL)
		DeleteObject(hbrSel);

	return   (CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT);
}
LRESULT Draw_ColorEncodedTextQW(RECT rc, LPNMLVCUSTOMDRAW pListDraw , char *pszText)
{
	HDC  hDC =  pListDraw->nmcd.hdc;
	HBRUSH hbrSel= NULL;
	hbrSel = CreateSolidBrush( RGB(0x28,0x2c,0x28)); 														
	FillRect(hDC, &rc, (HBRUSH) hbrSel);

	if( pListDraw->nmcd.uItemState & ( CDIS_SELECTED))
	{
		pListDraw->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT); //RGB(255, 255, 255);
		hbrSel = CreateSolidBrush( GetSysColor(COLOR_HIGHLIGHT)); //RGB(51,153,250)); 																
		FillRect(hDC, &rc, (HBRUSH) hbrSel);
	}
	int nCharWidth;
	SelectObject(hDC,g_hf2);
	//GetCharWidth32(hDC, (UINT) 0, (UINT) 0, &nCharWidth); 				
	ABC abc[256];
	LPABC pAbc = abc;
	GetCharABCWidths(hDC,       (UINT)0, (UINT) 255,pAbc);
	char *pText;
	rc.left+=20;
	rc.top+=2;
			UCHAR uc[2];
			uc[1]=0;

	for(int i=0;i<strlen(pszText);i++)
	{
		COLORREF col = RGB(255,255,255) ;
		uc[0] = (UCHAR)pszText[i];
			switch(uc[0])
			{
				case 16:
				{
					uc[0] = '[';
					col = RGB(0xff,0xf9,0x80);
					break;
				}
				case 17:
				{
					uc[0] = ']';
					col = RGB(0xff,0xf9,0x80);
					break;
				}
				case 28:
					uc[0] = '*';
					break;

			}
		if((unsigned char)uc[0]>127)
		{
			switch(uc[0])
			{

				case 156:
				{
					uc[0] = '*';
					col = RGB(0xfa,0xbd,0x8e);
					break;
				}
				case 157:
				{

					uc[0] = '<';
					col = RGB(0xfa,0xbd,0x8e);
					break;
				}
				case 159:
					{
					uc[0] = '>';
					col = RGB(0xfa,0xbd,0x8e);
					break;
				}
				default:
				{
					if((unsigned char)uc[0]>225)
					{
						uc[0]-=160;
						col = RGB(0xfa,0xbd,0x8e);
					}
					else if((unsigned char)uc[0]>193)
					{
						uc[0]-=128;
						col = RGB(255,255,255); 
					}
					else if((unsigned char)uc[0]>169)
					{
						uc[0]-=104;
						col = RGB(0xff,0xf9,0x80);
					}
					else //if((unsigned char)szInText[i]>193)
					{
						uc[0]-=67;
						col = RGB(0xff,0xf9,0x80);
					}
					break;
				}
			}
	
		}
		SetTextColor(hDC,col);
		pText = (char*)&uc[0];
		ExtTextOut(hDC,rc.left,rc.top,0, &rc,pText, 1,NULL); 
		nCharWidth = abc[uc[0]].abcA + abc[uc[0]].abcB + abc[uc[0]].abcC;
		rc.left+=nCharWidth;

	}
	SelectObject(hDC,g_hf);								
	if(hbrSel!=NULL)
		DeleteObject(hbrSel);

	return  (CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT );
}

/********************************************************************************
   Homebrewn progressbar that will display a green to red shaded rectangle.

   HDC hDC - the HDC to be drawn on.
   RECT rectFull - size of the rectangle
   char *pszText - text to be on top of the shaded rect
   int xCurrentValue - the current status of a value, this will help to calcualte 
					   percentage (%) of the progress. 
   int xMaxValue - the max value, see above for details.
************************************************************************************/
void Draw_ShadedRect(HDC hDC,RECT rectFull,char *pszText,int xCurrentValue,int xMaxValue)
{
	RECT rect;
	HBRUSH hbrBar=NULL;
	RECT rectTemp;

	CopyRect(&rect,&rectFull);

	//Shrink the rectangle
	rect.top+=3;
	rect.bottom-=3;								

	float percentage =  ((float)xCurrentValue /  (float)xMaxValue) * 100.0f ;
	if(percentage>100.0f)
		percentage = 100;

	if(percentage<0)  //can't be less than zero
		percentage = 0;
	Rectangle(hDC, rect.left-1, rect.top-1,rect.right-1, rect.bottom+1); 

	int maxPixel = (rect.right-1) - (rect.left-1);
	int pixelX = (percentage * maxPixel) / 100;
	rect.right = rect.left +pixelX; // percentage; //rect.left + pSI.nCurrentPlayers+2;
	
	CopyRect(&rectTemp,&rect);
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
		FillRect(hDC, &rect, (HBRUSH) hbrBar);
		DeleteObject(hbrBar);
		rect.left++;
	}
	CopyRect(&rect,&rectTemp);						
	
	SetTextColor(hDC,0x00FFFFFF);								
	TextOut(hDC, rect.left+2,rect.top-1, pszText, strlen(pszText));
	SetTextColor(hDC,0x0000000);
	ExcludeClipRect(hDC,rect.left,rect.top,rect.right,rect.bottom);
	TextOut(hDC, rect.left+2,rect.top-1, pszText, strlen(pszText));

	if(hbrBar!=NULL)
		DeleteObject(hbrBar);
}

//ServerList custom draw
LRESULT ListView_SL_CustomDraw (LPARAM lParam)
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
					SERVER_INFO pSI = Get_ServerInfoByListViewIndex(currCV,iRow); 
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
			}
			return (  CDRF_NOTIFYSUBITEMDRAW );
		
		case (CDDS_ITEMPREPAINT|CDDS_SUBITEM):
			{
	 
				int    nItem = static_cast<int>( pListDraw->nmcd.dwItemSpec );
				SERVER_INFO pSI;
				pListDraw->clrText   = RGB(0, 0, 0);	

				if(pListDraw->nmcd.hdr.idFrom == IDC_LIST_SERVER)
				{
					pSI = Get_ServerInfoByListViewIndex(currCV,nItem); //currCV->vSIFiltered.at((int)nItem);
					
					if(pSI.dwIP == 0) //Quick and dirty fix if server index is out of range
						return CDRF_DODEFAULT;
															
						//do some default stuff
						if(pSI.dwPing==9999)
							pListDraw->clrText   = RGB(255, 0, 0);
						else
						{
							if(pSI.bUpdated==false)
								pListDraw->clrText   = RGB(140, 140, 140);
							else
								pListDraw->clrText   = RGB(0, 0, 0);
						}
						HDC  hDC =  pListDraw->nmcd.hdc;							
						RECT rc;
						HBRUSH hbrBkgnd=NULL;
						iRow = (int)pListDraw->nmcd.dwItemSpec;	

						ListView_GetSubItemRect(g_hwndListViewServer,nItem,pListDraw->iSubItem,LVIR_BOUNDS,&rc);

						if(pListDraw->iSubItem==CUSTCOLUMNS[COL_PLAYERS].columnIdx)
						{
							char szText[50];
							sprintf_s(szText,sizeof(szText),"%d/%d+(%d)",pSI.nCurrentPlayers,pSI.nMaxPlayers,pSI.nPrivateClients);
							
							hbrBkgnd = CreateSolidBrush(RGB(202, 221,250)); 
							if(iRow%2 == 0)
							{
								pListDraw->clrTextBk = RGB(202, 221,250);
								FillRect(hDC, &rc, (HBRUSH) hbrBkgnd);
							}

							Draw_ShadedRect(hDC,rc,szText,pSI.nCurrentPlayers,pSI.nMaxPlayers);

							if(hbrBkgnd!=NULL)
								DeleteObject(hbrBkgnd);

							return (CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT);							
						}
						else if(pListDraw->iSubItem==CUSTCOLUMNS[COL_COUNTRY].columnIdx)
						{
							HBRUSH hbrSel=NULL;							
							int idxCC = Get_CountryFlagByShortName(pSI.szShortCountryName);
							hbrBkgnd = CreateSolidBrush(RGB(202, 221,250)); 

							if(iRow%2 == 0)
								FillRect(hDC, &rc, (HBRUSH) hbrBkgnd);
						
							if(pListDraw->nmcd.uItemState & ( CDIS_SELECTED))
							{
								pListDraw->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT); //RGB(255, 255, 255);
								hbrSel = CreateSolidBrush( GetSysColor(COLOR_HIGHLIGHT)); //RGB(51,153,250)); 																
								FillRect(hDC, &rc, (HBRUSH) hbrSel);
							}
							ImageList_Draw(g_hILFlags,idxCC,hDC,rc.left+1,rc.top+2,ILD_NORMAL|ILD_TRANSPARENT);
															
							//SetBkColor(hDC,pListDraw->clrTextBk);
							SetTextColor(hDC,pListDraw->clrText);								
							SetBkMode(hDC, TRANSPARENT);

							rc.left+=20;
							rc.top+=2;
							if(AppCFG.bUseShortCountry)
								ExtTextOut(hDC,rc.left,rc.top,0, &rc,pSI.szShortCountryName, strlen(pSI.szShortCountryName),NULL); 
							else
								ExtTextOut(hDC,rc.left,rc.top,0, &rc,pSI.szCountry, strlen(pSI.szCountry),NULL); 
								
							DeleteObject(hbrBkgnd);
							if(hbrSel!=NULL)
								DeleteObject(hbrSel);
							return   (CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT);
						}
						else if(pListDraw->iSubItem==CUSTCOLUMNS[COL_SERVERNAME].columnIdx)
						{
							if(AppCFG.bUseColorEncodedFont)
							{
								HDC  hDC =  pListDraw->nmcd.hdc;
								RECT rc;								
								ListView_GetSubItemRect(g_hwndListViewServer,nItem,pListDraw->iSubItem,LVIR_BOUNDS,&rc);								
								if(GamesInfo[pSI.cGAMEINDEX].Draw_ColorEncodedText!=NULL)
									GamesInfo[pSI.cGAMEINDEX].Draw_ColorEncodedText(rc, pListDraw , pSI.szServerName);
								else
									Draw_ColorEncodedText(rc, pListDraw , pSI.szServerName);

								if(pSI.cFavorite)
									ImageList_Draw(g_hImageListIcons,2,hDC,rc.left+1,rc.top+2,ILD_NORMAL|ILD_TRANSPARENT);
								else
									ImageList_Draw(g_hImageListIcons,Get_GameIcon(pSI.cGAMEINDEX),hDC,rc.left+1,rc.top+2,ILD_NORMAL|ILD_TRANSPARENT);
								return  ( CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT);
							}

						}
					return  CDRF_NEWFONT; 					
				} 
			}
			break;
	default:
		break;
	}
	return CDRF_DODEFAULT;
}


//Player list custom draw
LRESULT ListView_PL_CustomDraw(LPARAM lParam)
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
			break;
		case CDDS_ITEMPREPAINT:
			{			
				iRow = (int)pListDraw->nmcd.dwItemSpec;
				if(iRow%2 == 0)
					pListDraw->clrTextBk = RGB(202, 221,250);	
			}
			return (CDRF_NOTIFYSUBITEMDRAW );
			break;
		case CDDS_ITEMPREPAINT|CDDS_SUBITEM:
			{
				int nItem = static_cast<int>( pListDraw->nmcd.dwItemSpec );
				pListDraw->clrText   = RGB(0, 0, 0);	
				if(pListDraw->nmcd.hdr.idFrom == IDC_LIST_PLAYERS)
				{
					if(AppCFG.bUseColorEncodedFont)
					{
						if(pListDraw->iSubItem==2)
						{
							PLAYERDATA *pPlayerData = pCurrentPL;
							if(pPlayerData!=NULL)
							{
								for(int i=0;i<nItem;i++)
									pPlayerData = pPlayerData->pNext;
								
								HDC  hDC = pListDraw->nmcd.hdc;							
								RECT rc;								
								if(pPlayerData!=NULL)
								{
									ListView_GetSubItemRect(g_hwndListViewPlayers,nItem,pListDraw->iSubItem,LVIR_BOUNDS,&rc);								
									if(GamesInfo[pPlayerData->cGAMEINDEX].Draw_ColorEncodedText!=NULL)
										return GamesInfo[pPlayerData->cGAMEINDEX].Draw_ColorEncodedText(rc, pListDraw , pPlayerData->szPlayerName);
								}
							}
						}
					}
					return  CDRF_NEWFONT;					
				}
				
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
				break;
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
		
	GAME_INFO *pGI = (GAME_INFO *)&GamesInfo[g_currentGameIdx];
	vSRV_INF::iterator  iLst;

	if(pGI->vSI.size()==0)
		return NULL;

	ListView_DeleteAllItems(g_hwndListViewServer); //5.44
	pGI->vRefListSI.clear();

	if(str[0]>='A' &&  str[0]<='z')
	{
		for ( iLst = pGI->vSI.begin( ); iLst != pGI->vSI.end( ); iLst++ )
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
					pGI->vRefListSI.push_back(refSI);

				
					counter++;
				}
			}		
		}

	} else //could it be a IP address search?
	{
		char szIPPORT[50];
		
		for ( iLst = pGI->vSI.begin( ); iLst != pGI->vSI.end( ); iLst++ )
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
				pGI->vRefListSI.push_back(refSI);

				counter++;
			}else 
			{
				sprintf(szIPPORT,"%s:%d",pSI.szIPaddress,pSI.dwPort);
				if(strstr(szIPPORT,str)!=NULL)
				{
					REF_SERVER_INFO refSI;
					refSI.dwIndex = pSI.dwIndex;
					refSI.cGAMEINDEX = pSI.cGAMEINDEX;
					pGI->vRefListSI.push_back(refSI);

					counter++;

				}
			}
		}
	}

	int count = SendDlgItemMessage(  g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD,     (UINT) CB_GETCOUNT, 0,     0);  

	if(count>0)
		SendDlgItemMessage(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD, CB_SHOWDROPDOWN, TRUE, 0); 

	Do_ServerListSortThread(iLastColumnSortIndex);
	//Do_ServerListSort((LPVOID)iLastColumnSortIndex);	
	//ListView_SetItemCount(g_hwndListViewServer,pGI->vRefListSI.size());		


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
								int ret = MessageBox(NULL,lang.GetString("AskRconKickPlayer"),szPlayerName,MB_YESNO);
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
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_ADD,lang.GetString("MenuAddPlayerToBuddyList"));			
				switch(g_currentGameIdx)
				{
					case RTCW_SERVERLIST:
					case Q4_SERVERLIST:
					case Q3_SERVERLIST:
					case COD_SERVERLIST:
					case COD2_SERVERLIST:					
					case ET_SERVERLIST:
					case ETQW_SERVERLIST:
						InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_YAWN_PLAYER,lang.GetString("MenuYAWNPlayer"));			
					break;
				}

				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_KICK_PLAYER,lang.GetString("MenuKick"));			
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_WARN_PLAYER,lang.GetString("MenuWarn"));			
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_MUTE_PLAYER,lang.GetString("MenuMute"));			
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_UNMUTE_PLAYER,lang.GetString("MenuUnMute"));			
												
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

					SERVER_INFO pSI = GamesInfo[pBI->cGAMEINDEX].vSI.at(pBI->sIndex);
					LaunchGame(pSI,&GamesInfo[pSI.cGAMEINDEX]);
			
				}
			}else
				MessageBox(NULL,lang.GetString("MessageBuddyNotOnline"),"Info",MB_OK);
		} else
			MessageBox(NULL,lang.GetString("MessageErrorConnecting"),"Info",MB_OK);
	}
	else  //From Favorites or the Masterlist
	{	
		n = ListView_GetSelectionMark(g_hwndListViewServer);
		if(n!=-1)
		{

			SERVER_INFO pSrv;
			pSrv =  Get_ServerInfoByListViewIndex(currCV,n);
			LaunchGame(pSrv,&GamesInfo[g_currentGameIdx]);
		}
	}

}


//if not sucessfull a zerofilled SERVER_INFO is returned!!
//This function should be rewritten...
SERVER_INFO Get_ServerInfoByListViewIndex(GAME_INFO *pGI,int index)
{	
	SERVER_INFO srv;
	REF_SERVER_INFO refSI;
	ZeroMemory(&srv,sizeof(SERVER_INFO));
	
	refSI.dwIndex = 0;
	srv.cGAMEINDEX = 0;
	if(pGI==NULL)
		return srv; //return NULL srv

	if(index<pGI->vRefListSI.size())
	{
		__try
		{				
			refSI = pGI->vRefListSI.at(index);
		}
		__except(EXCEPTION_ACCESS_VIOLATION == GetExceptionCode())
		{
			AddLogInfo(ETSV_ERROR,"Access Violation @ GetServerInfoByListIndex %s!");
			return srv;
		}		
	} else
		return srv;
	//continue if successfull
	__try
	{
		srv = pGI->vSI.at(refSI.dwIndex);
	}
	__except(EXCEPTION_ACCESS_VIOLATION == GetExceptionCode())
	{
		AddLogInfo(ETSV_ERROR,"Access Violation...@ GetServerInfoByListIndex");		
	}
	return srv; //gotcha... :)
}

SERVER_INFO Get_ServerInfoByIndex(GAME_INFO *pGI,int index)
{	
	SERVER_INFO srv;
	ZeroMemory(&srv,sizeof(SERVER_INFO));
	srv.cGAMEINDEX = 0;
	__try{

		srv = pGI->vSI.at(index);
	}
	__except(EXCEPTION_ACCESS_VIOLATION == GetExceptionCode())
	{
		OutputDebugString("Access violation...@ Get_ServerInfoByListViewIndex");		
	}
	return srv;
}


void Favorite_Add(bool manually,char *szIP)
{
	if(manually)
	{
		if(szIP!=NULL)
			strcpy(g_szIPtoAdd,szIP);
		else
		{
			char *pszIP = Get_SelectedServerIP();
			if(pszIP!=NULL)
				strcpy(g_szIPtoAdd,pszIP);
			else
				g_szIPtoAdd[0]=0;
		}

		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ADD_SERVER), NULL,(DLGPROC)AddServerProc);		
		int gameIdx = g_currentGameIdx;
		RedrawServerListThread((LPVOID)gameIdx);
	}
}


SERVER_INFO g_thisRCONServer;

void OnRCON()
{

	int i = ListView_GetSelectionMark(g_hwndListViewServer);
	if(i!=-1)
	{
		TabCtrl_SetCurSel(g_hwndTabControl,2);
		ShowWindow(g_hwndListViewPlayers,SW_HIDE);	
		ShowWindow(g_hwndMainRCON,SW_SHOW);
		ShowWindow(g_hwndMainSTATS,SW_HIDE);
		ShowWindow(g_hwndLogger,SW_HIDE);
	
		SendMessage(g_hwndMainSTATS,WM_STOP_PING,0,0);
		SendMessage(g_hwndMainRCON,WM_COMMAND,ID_RCON_CONNECT,0);
	} else
	{
	  MessageBox(NULL,lang.GetString("SelectServerAtConnectionRCON"),"Info!",MB_ICONINFORMATION|MB_OK); 

	}
}


LRESULT APIENTRY TreeView_SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 

	HMENU hPopMenu;
	if(uMsg == WM_MOUSEMOVE)
	{
		if(g_bRunningQuery)
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
					RedrawServerListThread((LPVOID)g_currentGameIdx);
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
				case IDM_DELETE_SERVERLIST:
					DeleteServerLists(GamesInfo[g_currentGameIdx].cGAMEINDEX);
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
			sprintf_s(szText,sizeof(szText),lang.GetString("MenuFindInternetServers"),GamesInfo[g_currentGameIdx].szGAME_NAME);
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_SCAN,szText);
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_LAUNCH_GAME_ONLY,lang.GetString("MenuLaunchGameOnly"));
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_ADDIP,lang.GetString("MenuAddNewIPToFav"));
			sprintf_s(szText,sizeof(szText),lang.GetString("MenuDeleteServerList"),GamesInfo[g_currentGameIdx].szGAME_NAME);
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_DELETE_SERVERLIST,szText);			
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_FOO_MINIMUMPLAYERS,lang.GetString("MenuModifyMinPly"));
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_FOO_MAXIMUMPLAYERS,lang.GetString("MenuModifyMaxPly"));

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

char *Get_SelectedServerIP()
{
	int n=-1;
	n = ListView_GetSelectionMark(g_hwndListViewServer);
	if(n!=-1)
	{
		SERVER_INFO pSI = Get_ServerInfoByListViewIndex(currCV,n);
		sprintf(g_currServerIP,"%s:%d",pSI.szIPaddress,pSI.dwPort);	
		return g_currServerIP;
	}
	return NULL;
}

HBITMAP hBmp;
LRESULT APIENTRY ListViewServerListSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 
	HMENU hPopMenu;
	POINT lpClickPoint;
	HMENU hSubPopMenu,hSubForceLaunchPopMenu,hSubClipboardPopMenu;		
	
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
			char *pszIP = Get_SelectedServerIP();
			if(pszIP!=NULL)
			{				
				EditCopy(pszIP);
				SetStatusText(ICO_INFO,lang.GetString("IPAddedToClipBoard"),pszIP);
			}else
			{
				SetStatusText(ICO_WARNING,lang.GetString("ServerCopyToClipBoard"));
				return 0;
			}
		}
	}
	else if(uMsg == WM_KEYUP)
	{
		if((wParam==VK_UP) || (wParam==VK_DOWN))
		{
			OnServerSelection(currCV);
			return TRUE;
		}
	}
	else if(uMsg == WM_MOUSEMOVE)
	{
		if(g_bRunningQuery)
			SetCursor(LoadCursor(NULL, IDC_APPSTARTING));

	}
		
	else if(uMsg == WM_COMMAND)
	 {
			DWORD wmId;
			DWORD wmEvent;
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 

		
				for(int x=0; x<currCV->vGAME_INST.size();x++)
					if(wmId == 36000+x)
					{
						int i = ListView_GetSelectionMark(g_hwndListViewServer);
						if(i!=-1)
						{
							SERVER_INFO pSrv;	
							pSrv = Get_ServerInfoByListViewIndex(currCV,i);
							LaunchGame(pSrv,&GamesInfo[g_currentGameIdx],x);
						}
					}

			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_SCAN:
					OnActivate_ServerList();
				break;
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
							MessageBox(NULL,lang.GetString("ErrorPrivatePassword"),"Info!",MB_ICONINFORMATION|MB_OK); 
							return TRUE;
						}
						SERVER_INFO pSI = Get_ServerInfoByListViewIndex(currCV,i);
						g_PRIVPASSsrv = &pSI;

						DialogBox(g_hInst, (LPCTSTR)IDD_DLG_SETPRIVPASS, g_hWnd, (DLGPROC)PRIVPASS_Proc);	
			
						currCV->vSI.at((int)pSI.dwIndex) = pSI;

					}
				break;
				case ID_YAWN_SERVER:
					{
						char szURL[512];
						char *pszIP = Get_SelectedServerIP();
						if(pszIP!=NULL)
						{
							switch(g_currentGameIdx)
							{				
								default:
								case ET_SERVERLIST:	
									sprintf_s(szURL,sizeof(szURL),"http://www.yawn.be/findServer.yawn?hostname=&serverAddress=%s&version=&protocol=&modid=0&game=ET&action=",pszIP);
									break;								
								case Q3_SERVERLIST:
									sprintf_s(szURL,sizeof(szURL),"http://www.yawn.be/findServer.yawn?hostname=&serverAddress=%s&version=&protocol=&modid=0&game=Q3&action=",pszIP);
									break;
								case Q4_SERVERLIST:
									sprintf_s(szURL,sizeof(szURL),"http://www.yawn.be/findServer.yawn?hostname=&serverAddress=%s&version=&protocol=&modid=0&game=Q4&action=",pszIP);
									break;
								case ETQW_SERVERLIST:
									sprintf_s(szURL,sizeof(szURL),"http://etqw.splatterladder.com/?mod=serverlist&phrase=%s",pszIP);
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
							SERVER_INFO pSI = Get_ServerInfoByListViewIndex(currCV,n); 						
							pSI.cFavorite =! pSI.cFavorite;							
							currCV->vSI.at((int)pSI.dwIndex) = pSI;
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
						
						TabCtrl_SetCurSel(g_hwndTabControl,3);
						ShowWindow(g_hwndMainRCON,SW_HIDE);
						ShowWindow(g_hwndMainSTATS,SW_SHOW);
						ShowWindow(g_hwndLogger,SW_HIDE);
						ShowWindow(g_hwndListViewPlayers,SW_HIDE);	
						SendMessage(g_hwndMainSTATS,WM_START_TRACERT,0,0);

					}
					break;

				case IDM_COPY_VERSION:
					{
						int n=-1;
						n = ListView_GetSelectionMark(g_hwndListViewServer);
						if(n!=-1)
						{
							SERVER_INFO pSI = Get_ServerInfoByListViewIndex(currCV,n);								
							EditCopy(pSI.szVersion);

						}
						else
							MessageBox(hwnd,lang.GetString("ErrorServerCopyToClipBoard"),NULL,MB_OK);
					}
				break;	
				case IDM_COPY_MODNAME:
					{
						int n=-1;
						n = ListView_GetSelectionMark(g_hwndListViewServer);
						if(n!=-1)
						{
							SERVER_INFO pSI = Get_ServerInfoByListViewIndex(currCV,n);								
							EditCopy(pSI.szMod);

						}
						else
							MessageBox(hwnd,lang.GetString("ErrorServerCopyToClipBoard"),NULL,MB_OK);
					}
					break;
				case IDM_COPYIP:
					{
						char *pszIP = Get_SelectedServerIP();
						if(pszIP!=NULL)
							EditCopy(pszIP);
						else
							MessageBox(hwnd,lang.GetString("ErrorServerCopyToClipBoard"),NULL,MB_OK);
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
			hSubForceLaunchPopMenu = CreatePopupMenu();
			hSubClipboardPopMenu  = CreatePopupMenu();
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
				mii.dwTypeData = (LPSTR)lang.GetString("MenuConnect");
				//mii.cch = strlen("Connect");
				mii.fState = MFS_CHECKED | MFS_DEFAULT;
				InsertMenuItem(hPopMenu,IDM_CONNECT,FALSE,&mii);

				SERVER_INFO pSI = Get_ServerInfoByListViewIndex(currCV,n); 						
				
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_LAUNCH_GAME_ONLY,lang.GetString("MenuLaunchGameOnly"));
				
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_POPUP|MF_BYPOSITION|MF_STRING,(UINT_PTR)hSubForceLaunchPopMenu,lang.GetString("MenuForceLaunch"));
				for(int x=0; x<currCV->vGAME_INST.size();x++)
					InsertMenu(hSubForceLaunchPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,36000+x,currCV->vGAME_INST.at(x).sName.c_str());

			
			
				if(pSI.cFavorite==0)
					InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_ADDIP,lang.GetString("MenuAddServerToFavorites"));
				else
					InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_ADDIP,lang.GetString("MenuRemoveFromFavorites"));
	
				//		InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_DELETE,"&Delete");
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_PRIVPASS,lang.GetString("MenuSetPrivatePass"));
			
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_COPYIP,lang.GetString("MenuCopyIP"));

				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_YAWN_SERVER,lang.GetString("MenuYAWN"));

	
			}										

			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_OPTIONS_RCON,lang.GetString("MenuRCON"));

			if(g_bRunningQueryServerList==false)
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_REFRESH,lang.GetString("MenuRefresh"));
			else
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING|MF_GRAYED,IDM_REFRESH,lang.GetString("MenuRefresh"));				

			InsertMenu(hPopMenu,0xFFFFFFFF,MF_POPUP|MF_BYPOSITION|MF_STRING,(UINT_PTR)hSubClipboardPopMenu,lang.GetString("MenuCopyToClipboard"));
			InsertMenu(hSubClipboardPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_COPY_VERSION,lang.GetString("ColumnVersion"));
			InsertMenu(hSubClipboardPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_COPY_MODNAME,lang.GetString("ColumnMod"));

			InsertMenu(hPopMenu,0xFFFFFFFF,MF_POPUP|MF_BYPOSITION|MF_STRING,(UINT_PTR)hSubPopMenu,lang.GetString("MenuNetworkTools"));
			InsertMenu(hSubPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_TT_SERVER1,lang.GetString("MenuPing"));
			InsertMenu(hSubPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_TT_SERVER2,lang.GetString("MenuTraceRoute"));
			
			SetForegroundWindow(hwnd);
			TrackPopupMenu(hPopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,lpClickPoint.x, lpClickPoint.y,0,hwnd,NULL);
			SendMessage(hwnd,WM_NULL,0,0);
			DestroyMenu(hPopMenu);
			DestroyMenu(hSubPopMenu);
			DestroyMenu(hSubForceLaunchPopMenu);
			DestroyMenu(hSubClipboardPopMenu);

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
	GAME_INFO *pGI = &GamesInfo[pSI.cGAMEINDEX];
	pGI->vSI.at(pSI.dwIndex) = pSI;
}

BOOL ExecuteGame(GAME_INFO *pGI,char *szCmd,int GameInstallIdx)
{
	char LoadLocation[512],  WETFolder[512];
	HINSTANCE hret=NULL;
	if(pGI->vGAME_INST.size()>0)
	{
		if(pGI->vGAME_INST.at(GameInstallIdx).szGAME_PATH.length()>0)
		{			
			strcpy(WETFolder,pGI->vGAME_INST.at(GameInstallIdx).szGAME_PATH.c_str());
			char* pos = strrchr(WETFolder,'\\');
			if(pos!=NULL)
			{
				pos[1]=0;
			}
			strcpy(LoadLocation,pGI->vGAME_INST.at(GameInstallIdx).szGAME_PATH.c_str());
			AddLogInfo(ETSV_DEBUG,WETFolder);
			AddLogInfo(ETSV_DEBUG,LoadLocation);
			hret = ShellExecute(NULL, "open", LoadLocation, szCmd,WETFolder, 1);

			if((int)hret<=32)
			{
				MessageBox(NULL,lang.GetString("ErrorLaunchingGame"),"Error!",MB_OK);
				return FALSE;
			}
			return TRUE;
		}
	}

	MessageBox(NULL,lang.GetString("ErrorLaunchingGame"),"Error!",MB_OK);	

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
					return ListView_SL_OnGetDispInfoList(wParam,(NMHDR*)lParam);
				else if(lpnmia->hdr.hwndFrom == g_hwndListViewPlayers)
				{

					return  ListView_PL_OnGetDispInfoList(wParam, (NMHDR*)lParam);
				}
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
							lpttt->lpszText = (LPSTR)lang.GetString("ScanAll"); //MAKEINTRESOURCE(IDS_REFRESH); 
							break; 
						case IDM_SCAN_FILTERED:
							lpttt->lpszText = (LPSTR)lang.GetString("ScanFiltered"); //MAKEINTRESOURCE(IDS_REFRESH_FILTER); 
							break;
						case IDM_SETTINGS: 
							lpttt->lpszText = (LPSTR)lang.GetString("SettingsButton"); // MAKEINTRESOURCE(IDS_OPTIONS); 
							break; 
						case IDC_BUTTON_ADD_SERVER: 
							lpttt->lpszText =(LPSTR)lang.GetString("AddToFavorites"); // MAKEINTRESOURCE(IDS_ADD_TO_FAVORITES); 
							break; 
						case IDC_BUTTON_QUICK_CONNECT: 
							lpttt->lpszText = (LPSTR)lang.GetString("QuickConnect"); //MAKEINTRESOURCE(IDS_FAST_CONNECT); 
							break; 
						case IDC_BUTTON_FIND: 
							lpttt->lpszText =(LPSTR)lang.GetString("SearchButton"); // MAKEINTRESOURCE(IDS_SEARCH); 
							break; 
						case IDC_DOWNLOAD: 
							lpttt->lpszText =(LPSTR)lang.GetString("DownloadButton"); // MAKEINTRESOURCE(IDS_DOWNLOAD); 
							break; 
						case IDM_FONT_COLOR: 
							lpttt->lpszText = (LPSTR)lang.GetString("FontButton"); //MAKEINTRESOURCE(IDS_COLOR_FONT); 
							break; 
						case ID_BUDDY_ADD:
							lpttt->lpszText =(LPSTR)lang.GetString("AddNewBuddyButton"); // MAKEINTRESOURCE(IDS_ADD_NEW_BUDDY); 
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

		/*		  if(pnmtv->hdr.code== TVN_ENDLABELEDIT)
				  {
					char Text[256]="";
					tvi.hItem=iSelected;
					SendDlgItemMessage(hWnd,IDC_MAINTREE,TVM_GETITEM,0,
									  (WPARAM)&tvi);
					GetWindowText(hEdit, Text, sizeof(Text));
					tvi.pszText=Text;
					SendDlgItemMessage(hWnd,IDC_MAINTREE,TVM_SETITEM,0,
									  (WPARAM)&tvi);
				  }*/
					
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
								GamesInfo[g_currentGameIdx].dwViewFlags = REDRAWLIST_FAVORITES_PUBLIC ;
								break;								
							case SHOW_FAVORITES_PRIVATE:
								GamesInfo[g_currentGameIdx].dwViewFlags = REDRAWLIST_FAVORITES_PRIVATE ;
								break;	
							case SHOW_HISTORY:
								GamesInfo[g_currentGameIdx].dwViewFlags = REDRAWLIST_HISTORY;
								break;
							case REDRAW_CURRENT_LIST : 
								GamesInfo[g_currentGameIdx].dwViewFlags = 0;
								break;						
							default:  //a game is selected
								{
									GamesInfo[g_currentGameIdx].dwViewFlags = 0;
									if(GamesInfo[g_currentGameIdx].vSI.size()==0) //if empty let's download server list from master
									{
										OnActivate_ServerList();
										return TRUE;
									} else
									{
										time_t currTime;
										time(&currTime);
										double seconds = difftime(currTime,GamesInfo[g_currentGameIdx].lastScanTimeStamp);
										dbg_print("Seconds since last scan %f",seconds);
										if(seconds>(60*5))
										{
										
											GamesInfo[g_currentGameIdx].dwViewFlags |= FORCE_SCAN_FILTERED;
											OnActivate_ServerList(SCAN_FILTERED);
											return TRUE;
										}
									
									}
								}
								break;

						} //end switch
						Initialize_RedrawServerListThread();
						//RedrawServerListThread(&GamesInfo[g_currentGameIdx]);
						return TRUE;
					}					
				}
		
			
		  default:
		{
			if((lpnmia->hdr.code == NM_SETFOCUS) && (lpnmia->hdr.hwndFrom == g_hwndListViewServer))
				{
					if (!RegisterHotKey(g_hwndListViewServer, HOTKEY_ID_CTRL_C, MOD_CONTROL, 0x43))
						AddLogInfo(ETSV_WARNING,"Couldn't register CTRL+V hotkey.");							
				}
				else if((lpnmia->hdr.code == NM_KILLFOCUS) && (lpnmia->hdr.hwndFrom == g_hwndListViewServer))
				{
					//Unregister copy short key
					UnregisterHotKey(g_hwndListViewServer,HOTKEY_ID_CTRL_C);
				}
				else if((lpnmia->hdr.code == NM_DBLCLK) && (lpnmia->hdr.hwndFrom == g_hwndListViewServer))
				{
					OnServerDoubleClick();
					return TRUE;
					
				}
				else if((lpnmia->hdr.code == NM_CLICK) && (lpnmia->hdr.hwndFrom == g_hwndListViewServer))
				{
					USHORT sKey = GetAsyncKeyState(VK_CONTROL);
					if(sKey==0x8001)
						g_bControl = TRUE;
					else
						g_bControl= FALSE;

					OnServerSelection(currCV);
					return TRUE;

				} 
				else if((lpnmia->hdr.code == NM_CLICK) && (lpnmia->hdr.hwndFrom == g_hwndListBuddy))
				{
					OnBuddySelected();
					return TRUE;					
				}
				else if((lpnmia->hdr.code == NM_RETURN) && (lpnmia->hdr.hwndFrom == g_hwndListViewServer))
				{
					OnServerSelection(currCV);
					return TRUE;					
				}
				else  if(lpnmia->hdr.code == NM_CUSTOMDRAW && (lpnmia->hdr.hwndFrom == g_hwndListViewPlayers))
					return ListView_PL_CustomDraw(lParam);	 
				else  if(lpnmia->hdr.code == NM_CUSTOMDRAW && (lpnmia->hdr.hwndFrom != g_hwndMainTreeCtrl))
				{					
					return ListView_SL_CustomDraw(lParam);					
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
					
					Do_ServerListSortThread(lstvw->iSubItem);


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
	LOGGER_Init();

	AddLogInfo(ETSV_INFO,"Initilizing Game Scanner...");
	AddLogInfo(ETSV_INFO,"Version %s",APP_VERSION);
	AddLogInfo(ETSV_INFO,"Executable directory: %s",EXE_PATH);
	AddLogInfo(ETSV_INFO,"User Data directory: %s",USER_SAVE_PATH);	
	AddLogInfo(ETSV_INFO,"Common Data directory: %s",COMMON_SAVE_PATH);	
	AddLogInfo(ETSV_INFO,"Cmd line input %s",lpCmdLine);

	IPC_SetPath(EXE_PATH);
		
	lang.SetPath(EXE_PATH);

	g_hInst = hInstance; // Store instance handle in our global variable
	LoadImageList();
	CFG_Load();

	//Do the conversion of the IP to country database 
//#ifdef CONVERTIPDATABASE
	SetCurrentDirectory(EXE_PATH);
	
	if(fnConvertDatabase()==0)
		AddLogInfo(ETSV_INFO,"Updated IP to Country file.");
	SetCurrentDirectory(USER_SAVE_PATH);
//#endif
	
	fnIPtoCountryInit();
	
	memset((void*)&etMode,0,sizeof(DEVMODE));

	

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
				for(char i=0;i<GamesInfo.size(); i++)
				{
					if(strcmp(proto,GamesInfo[i].szProtocolName)==0)
					{
						srv.cGAMEINDEX = i;	
						srv.dwIndex = 999999;  //work around to skip sync
						if(srv.dwPort ==0)
							srv.dwPort = GamesInfo[i].dwDefaultPort;

						LaunchGame(srv,&GamesInfo[i]);
					}
				}
				return 0;
			}

		}
	}
	
	//close multiple windows
	if(FindWindow(szWindowClass,szDialogTitle )!=NULL)
	{

		MessageBox(NULL,lang.GetString("MessageInstance"),"Alert",MB_OK);
		return 0;
	}
	InitializeCriticalSection(&SCANNER_cs);
	InitializeCriticalSection(&SCANNER_CSthreadcounter);					
	InitializeCriticalSection(&REDRAWLIST_CS);
	InitializeCriticalSection(&LOAD_SAVE_CS);

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
	CleanUpFilesRegistry();
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_ETSERVERVIEWER);
	//register hot key
	if(AppCFG.bUse_minimize)
	{		
		if (!RegisterHotKey(NULL, HOTKEY_ID, AppCFG.dwMinimizeMODKey , AppCFG.cMinimizeKey))
		{
			MessageBox(NULL,lang.GetString("ErrorRegHotkey"),"Hotkey error",NULL);			
			AppCFG.bUse_minimize = false;
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
	DeleteCriticalSection(&REDRAWLIST_CS);
	DeleteCriticalSection(&LOAD_SAVE_CS);
	DeleteCriticalSection(&SCANNER_CSthreadcounter);
	DeleteCriticalSection(&SCANNER_cs);

	 Q4_CleanUp_PlayerList(pCurrentPL);
	

	if(AppCFG.bUse_minimize)
		UnregisterHotKey(NULL, HOTKEY_ID);
	
	Shell_NotifyIcon(NIM_DELETE,&structNID);
	CloseHandle(hCloseEvent);
	DestroyIcon(hOfflineIcon);
	DestroyIcon(hOnlineIcon);
	WSACleanup();

	DestroyAcceleratorTable(hAccelTable); 
	AddLogInfo(ETSV_INFO,"Exit app..");
	LOGGER_DeInit();
	
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

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
/***************************************

Usage: ReplaceStrInStr(myString,"%IP%","127.0.0.1");
Return: TRUE if successfull.

****************************************/
BOOL ReplaceStrInStr(string &strToReplace,const char *szReplace,const char *szReplaceWith)
{
	string::size_type offset;
	offset = strToReplace.find(szReplace);
	if(offset!=-1)
	{
		strToReplace.insert(offset,szReplaceWith);
		offset = strToReplace.find(szReplace);
		strToReplace.erase(offset,strlen(szReplace));
		return TRUE;
	}
	return FALSE;
}


void LaunchGame(SERVER_INFO pSI,GAME_INFO *pGI,int GameInstallIdx)
{
	char CommandParameters[512];
	ZeroMemory(CommandParameters,512);
	int typeRecognized = -1;

	//simple autodetect to try to recognize which game to launch (used for quick launch)
	for(int i=0; i<GamesInfo.size();i++)
	{
		
		if((GamesInfo[i].dwDefaultPort==pSI.dwPort) && GamesInfo[i].bActive)
		{
			typeRecognized=GamesInfo[i].cGAMEINDEX;
			break;
		}
	}
	
	if(typeRecognized==-1)
		GetServerInfo(g_currentGameIdx,&pSI);
	else
	{
		GetServerInfo(typeRecognized,&pSI);

		dbg_print("Game detected: ");
		dbg_print(GamesInfo[typeRecognized].szGAME_NAME);
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

	for(int x=0; x<pGI->vGAME_INST.size();x++)
	{
		if(pSI.szMod!=NULL)
		{
			if(pGI->vGAME_INST.at(x).sMod.length()>0)
			{
				if(strcmp(pSI.szMod,pGI->vGAME_INST.at(x).sMod.c_str())==0)
				{
					GameInstallIdx = x;
					break;
				}
			}
		}
		else if((pSI.szVersion!=NULL) && (pGI->vGAME_INST.at(x).sVersion.length()>0)) 
		{
			if(strcmp(pSI.szVersion,pGI->vGAME_INST.at(x).sVersion.c_str())==0)
			{
				GameInstallIdx = x;
				break;
			}
		}
	}
	string cmd;
	if(pGI->vGAME_INST.size()>0)
	{
		cmd = pGI->vGAME_INST.at(GameInstallIdx).szGAME_CMD;

		ReplaceStrInStr(cmd,"%MODNAME%",pSI.szMod);
		char szTempPath[MAX_PATH*2];
		strcpy(szTempPath,pGI->vGAME_INST.at(GameInstallIdx).szGAME_PATH.c_str());
		char *p=strrchr(szTempPath,'\\');
			if(p!=NULL)
				p[0]=0;
		ReplaceStrInStr(cmd,"%GAMEPATH%",szTempPath);
	}

	if(strstr(cmd.c_str(),"applaunch")!=NULL)  //quick steam fix cmd has to be pre-merged
	{
		if(strlen(pSI.szPRIVATEPASS)>0)
			sprintf(CommandParameters,"%s +connect %s:%d +password %s",cmd.c_str(),pSI.szIPaddress,pSI.dwPort,pSI.szPRIVATEPASS);					
		else
			sprintf(CommandParameters,"%s +connect %s:%d",cmd.c_str(),pSI.szIPaddress,pSI.dwPort);					

	}
	else
	{
		if(strlen(pSI.szPRIVATEPASS)>0)
			sprintf(CommandParameters,"+connect %s:%d +password %s %s",pSI.szIPaddress,pSI.dwPort,pSI.szPRIVATEPASS,cmd.c_str());					
		else
			sprintf(CommandParameters,"+connect %s:%d %s",pSI.szIPaddress,pSI.dwPort,cmd.c_str());					
	}
	AddLogInfo(0,CommandParameters);

	if(ExecuteGame(pGI,CommandParameters,GameInstallIdx))
	{
		//A Successfull launch
		PostMessage(g_hWnd,WM_CLOSE,0xdead,0);  //Minimize ETSV
		
		if(AppCFG.bUseMIRC)
		{
			//Notify mIRC which server user will join
			DDE_Init();
  			char szMsg[350];
			
			string mircoutput;
			mircoutput = g_sMIRCoutput;
			mircoutput.insert(0,"/ame ");

			char colfilter[120];
			colorfilter(pSI.szServerName,colfilter,119);
			ReplaceStrInStr(mircoutput,"%SERVERNAME%",colfilter);
			
			wsprintf(szMsg,"%s:%d",pSI.szIPaddress,pSI.dwPort);	
			ReplaceStrInStr(mircoutput,"%IP%",szMsg);		
			ReplaceStrInStr(mircoutput,"%GAMENAME%",GamesInfo[pSI.cGAMEINDEX].szGAME_NAME);
			
			if(pSI.bPrivate)
				ReplaceStrInStr(mircoutput,"%PRIVATE%","Private");
			else
				ReplaceStrInStr(mircoutput,"%PRIVATE%","Public");

			DDE_Send((char*)mircoutput.c_str());
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
	if(pElement==NULL)
	{
		AddLogInfo(ICO_INFO,"Error checking for new version (XML corrupt)!",szVersion);
		return 0;
	}
	pElement->FirstChild()->ToElement();

	if(pElement!=NULL)
	{
		ReadCfgStr2(pElement , "Version",szVersion,sizeof(szVersion));
		if(strcmp(szVersion,APP_VERSION)>0)
		{
			Show_ToolbarButton(IDC_DOWNLOAD, true);
			//EnableDownloadLink(TRUE);
			SetStatusText(ICO_INFO,lang.GetString("StatusNewVersion"),szVersion);
			AddLogInfo(ICO_INFO,"New version %s detected!",szVersion);
			bAnyUpdates=TRUE;
			PostMessage(g_hWnd,WM_COMMAND,IDC_DOWNLOAD,0);
		} else
		{
			AddLogInfo(ICO_INFO,"No new version detected!");
			//SetStatusText(ICO_INFO,"No new version detected!");
			if((int)lpParam!=1) //silent?
				MessageBox(g_hWnd,lang.GetString("MessageNoNewVersion"),"Info",MB_OK);
		}

	}
	
	g_bDoFirstTimeCheckForUpdate=false;
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
	int ret = MessageBox(NULL,lang.GetString("ErrorDownloadingUpdate"),"Update error",MB_YESNO);				
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
				SetWindowText(hDlg,lang.GetString("TitleSetMinValue"));				
				bSettingMax=FALSE;
			}
			else
			{
				dwMaxMin = &AppCFG.filter.dwShowServerWithMaxPlayers;
				_itoa(AppCFG.filter.dwShowServerWithMaxPlayers,szTemp,10);	
				SetWindowText(hDlg,lang.GetString("TitleSetMaxValue"));
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
					TreeView_SetDWValueByItemType(FILTER_MAX_PLY,*dwMaxMin,TreeView_GetItemStateByType(-25,FILTER_MAX_PLY));
				else
					TreeView_SetDWValueByItemType(FILTER_MIN_PLY,*dwMaxMin,TreeView_GetItemStateByType(-25,FILTER_MIN_PLY));

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

		int iNumButtons = 10;
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
		
		tbb.iBitmap = 9;
		tbb.idCommand = ID_BUDDY_ADD;
		tbb.fsState = TBSTATE_ENABLED;
		tbb.fsStyle = TBSTYLE_BUTTON;		
		::SendMessage(hwndTB, TB_ADDBUTTONS, 1, (LPARAM)&tbb);	

		tbb.iBitmap = 8;
		tbb.idCommand = IDM_FONT_COLOR;
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

	rbBand.lpText     = (LPSTR)lang.GetString("Search");
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


//*************************************************************
//
//  RegDelnodeRecurse()
//
//  Purpose:    Deletes a registry key and all it's subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL RegDelnodeRecurse (HKEY hKeyRoot, LPTSTR lpSubKey)
{
    LPTSTR lpEnd;
    LONG lResult;
    DWORD dwSize;
    TCHAR szName[MAX_PATH];
    HKEY hKey;
    FILETIME ftWrite;

    // First, see if we can delete the key without having
    // to recurse.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);
	AddGetLastErrorIntoLog("RegDelnodeRecurse");
    if (lResult == ERROR_SUCCESS) 
        return TRUE;

    lResult = RegOpenKeyEx (hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) 
    {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            printf("Key not found.\n");
            return TRUE;
        } 
        else {
            printf("Error opening key.\n");
            return FALSE;
        }
    }

    // Check for an ending slash and add one if it is missing.

    lpEnd = lpSubKey + lstrlen(lpSubKey);

    if (*(lpEnd - 1) != TEXT('\\')) 
    {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    // Enumerate the keys

    dwSize = MAX_PATH;
    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                           NULL, NULL, &ftWrite);

    if (lResult == ERROR_SUCCESS) 
    {
        do {

            StringCchCopy (lpEnd, MAX_PATH*2, szName);

            if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
                break;
            }

            dwSize = MAX_PATH;

            lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                                   NULL, NULL, &ftWrite);

        } while (lResult == ERROR_SUCCESS);
    }

    lpEnd--;
    *lpEnd = TEXT('\0');

    RegCloseKey (hKey);

    // Try again to delete the key.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS) 
        return TRUE;

    return FALSE;
}

//*************************************************************
//
//  RegDelnode()
//
//  Purpose:    Deletes a registry key and all it's subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL RegDelnode (HKEY hKeyRoot, LPTSTR lpSubKey)
{
    TCHAR szDelKey[2 * MAX_PATH];

    StringCchCopy (szDelKey, MAX_PATH*2, lpSubKey);
    return RegDelnodeRecurse(hKeyRoot, szDelKey);

}

void CleanUpFilesRegistry()
{
	SetCurrentDirectory(USER_SAVE_PATH);
	remove("ETSVsetup.msi");

 //   RegDelnode(HKEY_LOCAL_MACHINE,TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{122CD6F9-B1C2-4124-B5B4-5C0B255B74D1}"));
//	RegDelnode(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{CEC4EAF2-8EBA-4C36-8B4F-A69EA0E5AF00}");

}

int CFG_Load()
{
	Initialize_GameSettings();		

	Default_GameSettings();
	Default_Appsettings();

	lang.loadFile("lang_en.xml");

	SetCurrentDirectory(USER_SAVE_PATH);
	TiXmlDocument doc("config.xml");
	if (!doc.LoadFile()) 
	{
		ListView_SetDefaultColumns();
		return 1;
	}
	 
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

	pElem=hRoot.FirstChild("CurrentLanguage").Element();
	if (pElem)
		strcpy(AppCFG.szLanguageFilename,pElem->Attribute("filename"));		

	lang.loadFile(AppCFG.szLanguageFilename);
	ListView_SetDefaultColumns();

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
		if(pElem->Attribute("command")!=NULL)
			g_sMIRCoutput = pElem->Attribute("command");
	} else //set defualt value
		AppCFG.bUseMIRC = FALSE;

	pElem=hRoot.FirstChild("OptionalEXEsettings").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("enable",&intVal);
		AppCFG.bUse_EXT_APP = intVal;
		if(pElem->Attribute("path")!=NULL)
			strcpy(AppCFG.szEXT_EXE_PATH,pElem->Attribute("path"));

		if(pElem->Attribute("cmd")!=NULL)
			strcpy(AppCFG.szEXT_EXE_CMD,pElem->Attribute("cmd"));

		if(pElem->Attribute("WindowName")!=NULL)
			strcpy(AppCFG.szEXT_EXE_WINDOWNAME,pElem->Attribute("WindowName"));

	} else //set defualt value
		AppCFG.bUse_EXT_APP = FALSE;

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

	pElem=hRoot.FirstChild("ColorEncodedFont").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("Enable",&intVal);
		AppCFG.bUseColorEncodedFont  = intVal;
	} else //set defualt value
		AppCFG.bUseColorEncodedFont = TRUE;

	pElem=hRoot.FirstChild("AutoStart").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("enable",&intVal);
		AppCFG.bAutostart  = intVal;
	} else //set defualt value
		AppCFG.bAutostart = FALSE;


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
	

	
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"HideOfflineServers",(int&)AppCFG.filter.bHideOfflineServers);
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"NumMaxPlayers",(int&)AppCFG.filter.dwShowServerWithMaxPlayers);
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"NumMinPlayers",(int&)AppCFG.filter.dwShowServerWithMinPlayers);
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"NumMaxPlayersActive",(int&)AppCFG.filter.cActiveMaxPlayer);
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"NumMinPlayersActive",(int&)AppCFG.filter.cActiveMinPlayer);
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"Ping",(int&)AppCFG.filter.dwPing);
	ReadCfgInt(hRoot.FirstChild("Options").FirstChild().ToElement(),"Transparancy",(int&)AppCFG.g_cTransparancy);
	ReadCfgInt(hRoot.FirstChild("Options").FirstChild().ToElement(),"MaxScanThreads",(int&)AppCFG.dwThreads);
	ReadCfgInt(hRoot.FirstChild("NetworkRetries").FirstChild().ToElement(),"MaxScanThreads",(int&)AppCFG.dwRetries);

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
				int NoColumnsCheck=0;
				for(int i=0;i<MAX_COLUMNS; i++)
				{		
					
					if(pElemSortIdx->FirstChild()!=NULL)
					{
						TiXmlElement * pElemSortValue = pElemSortIdx->FirstChild()->ToElement();
						int val=0;
						if(ReadCfgInt(pElemSortValue, "Active",(int&)val)==XML_READ_OK)
							CUSTCOLUMNS[i].bActive = val;
						if(CUSTCOLUMNS[i].bActive)
							NoColumnsCheck++;
							
						if(ReadCfgInt(pElemSortValue, "id",(int&)val)==XML_READ_OK)
							CUSTCOLUMNS[i].id = val;

						if(ReadCfgInt(pElemSortValue, "Ascending",(int&)val)==XML_READ_OK)
							CUSTCOLUMNS[i].bSortAsc = val;

						if(ReadCfgInt(pElemSortValue, "columnIdx",(int&)val)==XML_READ_OK)
							CUSTCOLUMNS[i].columnIdx = val;

						ReadCfgInt(pElemSortValue, "cx",(int&)CUSTCOLUMNS[i].lvColumn.cx);
						CUSTCOLUMNS[i].columnIdxToSave = CUSTCOLUMNS[i].columnIdx;

						//char szOutput[50];
						//if(ReadCfgStr(pElemSortValue,"strval",szOutput, sizeof(szOutput)-1)!=NULL)
						//	CUSTCOLUMNS[i].sName = szOutput;
						pElemSortIdx = pElemSortIdx->NextSiblingElement();
						
						if(pElemSortIdx==NULL)
							break;
					}
				}
				if(NoColumnsCheck==0)
				{
					ListView_SetDefaultColumns(); //sanity check
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
			for(int i=0;i<GamesInfo.size();i++)
			{
				TiXmlElement* pNode = pElement->FirstChild()->ToElement();
				if(pNode!=NULL)
				{
					char temp[MAX_PATH];
					ReadCfgStr(pNode, "GameName",temp,MAX_PATH);
					if(strlen(temp)>0)
						strcpy(GamesInfo[i].szGAME_NAME,temp);
					
					
					if(ReadCfgStr(pNode, "Path",temp,MAX_PATH)!=NULL) //old changed since ver 1.08
					{
						
						GamesInfo[i].vGAME_INST.clear();
						GAME_INSTALLATIONS gi;
						gi.sName = "Default";
						gi.szGAME_PATH = temp;
						

						ReadCfgStr(pNode, "Cmd",temp,MAX_PATH);
						gi.szGAME_PATH = temp;
						ReadCfgStr(pNode, "LaunchByVer",temp,MAX_PATH);
						gi.sVersion = temp;
						ReadCfgStr(pNode, "LaunchByMod",temp,MAX_PATH);
						gi.sMod = temp;

						GamesInfo[i].vGAME_INST.push_back(gi);
					} else
					{
						TiXmlNode* pInstallTags = pElement->FirstChild("Installs");
						
						if( pInstallTags!=NULL)
						{
							TiXmlElement* pInstalls = pInstallTags->ToElement();
							if(pInstalls!=NULL)
								GamesInfo[i].vGAME_INST.clear();

							while(pInstalls!=NULL)
							{
								TiXmlElement* pInstall = pInstalls->FirstChild("Install")->ToElement();
								char szTemp[MAX_PATH];
								ReadCfgStr(pInstall, "Name",szTemp,MAX_PATH); 
								GAME_INSTALLATIONS gi;							

								gi.sName = szTemp;

								ReadCfgStr(pInstall, "Path",temp,MAX_PATH);
								gi.szGAME_PATH = temp;

								ReadCfgStr(pInstall, "Cmd",temp,MAX_PATH);
								gi.szGAME_CMD = temp;

								ReadCfgStr(pInstall, "LaunchByVer",temp,MAX_PATH);
								gi.sVersion = temp;

								ReadCfgStr(pInstall, "LaunchByMod",temp,MAX_PATH);
								gi.sMod = temp;		

								GamesInfo[i].vGAME_INST.push_back(gi);
								pInstalls = pInstalls->NextSiblingElement();
								if(pInstalls==NULL)
									break;
								
							}
						}
					}
					
				//	ReadCfgStr( pNode, "yawn",GamesInfo[i].szMAP_YAWN_PATH,sizeof(GamesInfo[i].szMAP_YAWN_PATH));
					ReadCfgStr( pNode, "MapPreview",GamesInfo[i].szMAP_MAPPREVIEW_PATH,sizeof(GamesInfo[i].szMAP_MAPPREVIEW_PATH));
					ReadCfgStr( pNode, "MasterServer",GamesInfo[i].szMasterServerIP,sizeof(GamesInfo[i].szMasterServerIP));		
					ReadCfgInt( pNode, "MasterServerPort",(int&)GamesInfo[i].dwMasterServerPORT);
					//ReadCfgInt( pNode, "Protocol",(int&)GamesInfo[i].dwProtocol);		
					ReadCfgInt(pNode, "Active",(int&)GamesInfo[i].bActive);
					ReadCfgInt(pNode, "FilterMod",(int&)GamesInfo[i].filter.dwMod);
					ReadCfgInt(pNode, "FilterVersion",(int&)GamesInfo[i].filter.dwVersion);
					ReadCfgInt(pNode, "FilterMap",(int&)GamesInfo[i].filter.dwMap);
					ReadCfgInt(pNode, "FilterRegion",(int&)GamesInfo[i].filter.dwRegion);
					ReadCfgInt(pNode, "FilterGameType",(int&)GamesInfo[i].filter.dwGameTypeFilter);					
					ReadCfgInt(pNode, "FilterHideFull",(int&)GamesInfo[i].filter.bNoFull);
					ReadCfgInt(pNode, "FilterHideEmpty",(int&)GamesInfo[i].filter.bNoEmpty);
					ReadCfgInt(pNode, "FilterPunkbuster",(int&)GamesInfo[i].filter.bPunkbuster);
					ReadCfgInt(pNode, "FilterHideOffline",(int&)GamesInfo[i].filter.bHideOfflineServers);
					ReadCfgInt(pNode, "FilterPure",(int&)GamesInfo[i].filter.bPure);
					ReadCfgInt(pNode, "FilterRanked",(int&)GamesInfo[i].filter.bRanked);
					ReadCfgInt(pNode, "FilterHideBots",(int&)GamesInfo[i].filter.bNoBots);
					ReadCfgInt(pNode, "FilterNoPrivate",(int&)GamesInfo[i].filter.bNoPrivate);
					ReadCfgInt(pNode, "FilterOnlyPrivate",(int&)GamesInfo[i].filter.bOnlyPrivate);					
					ReadCfgInt(pNode, "FilterDedicated",(int&)GamesInfo[i].filter.bDedicated);					
					pElement = pElement->NextSiblingElement();
					if(pElement==NULL)
						break;
				} else
					break;
			}
		}
	}

	dbg_print("Loaded config.\n");	
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
			Initialize_WindowSizes();
			//Update_WindowSizes();
			PostMessage(g_hWnd,WM_SIZE,0,0);
			return TRUE;

		case WM_REFRESHSERVERLIST:
			Initialize_RedrawServerListThread();
			//RedrawServerListThread(&GamesInfo[g_currentGameIdx]);
			return TRUE;
		case WM_CREATE:
		{
			  //add a band that contains a combobox			
			OnCreate(hWnd,g_hInst);	
			
			OnInitialize_MainDlg(hWnd);
			PostMessage(hWnd, WM_SIZE, 0, 0);
	
			
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
			 g_bMinimized=true;
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
								int gameIdx = g_currentGameIdx;
								RedrawServerListThread((LPVOID)gameIdx);
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
					ShowWindow(g_hWnd, SW_RESTORE);
					
					if(g_bMinimized)
						PostMessage(g_hWnd,WM_INITVIEWS,0,0);
					g_bMinimized= FALSE;

					PostMessage(g_hWnd,WM_REFRESHSERVERLIST,0,0);
					return TRUE;
					break;
				}
				case IDM_SCAN_ALL_GAMES:
					GamesInfo[g_currentGameIdx].dwViewFlags = 0;
					OnActivate_ServerList(SCAN_ALL_GAMES);
					break;
				case IDM_SCAN_FILTERED:
					GamesInfo[g_currentGameIdx].dwViewFlags |= FORCE_SCAN_FILTERED;
					OnActivate_ServerList(SCAN_FILTERED);
					break;
				case IDM_SCAN:  //Toolbar
					OnActivate_ServerList(SCAN_ALL);
				break;
				case IDM_REFRESH:  //Toolbar
				case ID_OPTIONS_REFRESH: 
					GamesInfo[g_currentGameIdx].dwViewFlags = 0;
					OnScanButton();							
				break;
				case IDC_DOWNLOAD:
					{
						if(MessageBox(hWnd,lang.GetString("AskToUpdate"),"Update Game Scanner",MB_YESNO)==IDYES)
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
						hThread = CreateThread( NULL, 0, &CheckForUpdates, (LPVOID)lParam,0, NULL);      //lParam = 1 = silent = no messageboxes          
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
				case IDM_FONT_COLOR:
					{
						AppCFG.bUseColorEncodedFont = !AppCFG.bUseColorEncodedFont;
						int gameIdx = g_currentGameIdx;
						RedrawServerListThread((LPVOID)gameIdx);
						DrawCurrentPlayerList(pCurrentPL);
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
						
						currCV = &GamesInfo[g_currentGameIdx];		//restore currCV pointer			

						OnSize(g_hWnd);
						TreeView_BuildList();
						SetDlgTrans(hWnd,AppCFG.g_cTransparancy);
						
							

						//Do we need to change view after configuring?
						if(GamesInfo[g_currentGameIdx].bActive==false)
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
	AddLogInfo(0,"Error reading XML tag strval (XML_GetTreeItemStrValue)");
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