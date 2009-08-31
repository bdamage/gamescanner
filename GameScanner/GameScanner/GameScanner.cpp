// GameScanner.cpp : Defines the entry point for the application.
// Copyright (C) 2007, 2008 Kjell Lloyd 

/*

Set up GUID keys:
Product code:

v 1.32 {56AE3369-9E26-454A-AD62-88A996A5CA2B}
v 1.33 {00CA389B-8E94-4349-9166-1C6A973B75B5}
v 1.34 {1BF67018-F06A-48B7-BA16-7E25A7D0B43E}
v 1.35 {410AAD30-6890-4413-905A-5D74C29B5D05}
v 1.36 {4D737C75-ECA3-49A1-BF2F-C8826BAF3CDE}
v 1.37 {4BFEE3BD-A6CC-4BF7-B23B-C6B1B4449897}

Upgrade code 1.32 -1.35
{C4D8A4B8-9F37-4265-935A-B7D9ABAA9F3C}

Upgrade code 1.0 - 1.0.9:
{1E1FC67E-A466-4A1F-A278-286B6905C57B}

*/


#include "stdafx.h"
#include "GameScanner.h"

#include "Logger.h"

#include "utilz.h"
#include "dde_wrapper.h"
#include "rcon.h"
#include "scanner.h"
//#include "buddy.h"

#include "CountryCodes.h"
#include "GameManager.h"
#include "TreeViewManager.h"
#include "CVSFile.h"
#include "Timer.h"
#include "ListViewHeader.h"
#include "BuddyManager.h"
#include "ScriptEngine.h"

#pragma comment(lib, "ole32.lib")

//#ifdef _DEBUG
//#pragma comment(lib, "..\\tinyxml\\Release\\tinyxmld.lib")
//#else
//#pragma comment(lib, "..\\tinyxml\\Release\\tinyxml.lib")
//#endif
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

using namespace std;

// Global Variables:
HINSTANCE g_hInst;								// current instance
char szTitle[MAX_LOADSTRING];					// The title bar text
char szWindowClass[MAX_LOADSTRING];			// the main window class name
BOOL bWaitingToSave=FALSE;
BOOL bWaitingToSaveMinimized=FALSE;

//vFILTER_SETS vFilterSetsGlobal;
BOOL g_bBeginEdit = FALSE;
// Forward declarations of functions included in this code module:

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


char NexuizASCII[]=
	{
		' ',' ','-',' ','_','*','+','.',' ',' ',' ',' ','.','>','*','*',
		'[',']','?','?','?',' ',' ',' ','«','»','.',' ','*','-','-','-', //32
		' ','!','"','#','$','%','&','´','(',')','*','+',',','-','.','/',
		'0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
		'@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
		'P','Q','R','S','T','U','V','W','X','Y','Z','[','\\',']','^','_', //96
		'`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
		'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~','<',
		'(','=',')','?','|','?','?','?','?','C','R','§','?','>','*','*',  //160
		'[',']',' ',' ',' ',' ',' ',' ','«','»','*',' ','*','-','-','-',  //176
		' ','!','"','#','$','%','&','´','(',')','*','+',',','-','.','/',  //192
		'0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',  //208
		'@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',  //224
		'P','Q','R','S','T','U','V','W','X','Y','Z','[','\\',']','^','_',  //240
		'\'','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
		'P','Q','R','S','T','U','V','W','X','Y','Z','{','|','}','~','<'
	};

char QuakeWorldASCII[]=
	{ // 0   1   3   4   5   6   7   8   9  10  11  12  13  14  15  16
		'º',' ',' ',' ',' ','º',' ',' ',' ',' ',' ','*',' ','>','*','*',
		'[',']','0','1','2','3','4','5','6','7','8','9','º','-','-','-',  //32
		' ','!','"','#','$','%','&','´','(',')','*','+',',','-','.','/',  //48
		'0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',  //64
		'@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',  //80
		'P','Q','R','S','T','U','V','W','X','Y','Z','[','\\',']','^','_', //96
		'´','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',  //128
		'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~','<',  //144
		'(','=',')','?','|','*','*','*','*','*',' ','*',' ','>','*','*',  //160
		'[',']','0','1','2','3','4','5','6','7','8','9','º','-','-','-',  //176
		' ','!','"','#','$','%','&','´','(',')','*','+',',','-','.','/',  //192
		'0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
		'@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
		'P','Q','R','S','T','U','V','W','X','Y','Z','[','\\',']','^','_',
		'´','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
		'P','Q','R','S','T','U','V','W','X','Y','Z','{','|','}','~','<'
	};

#define MAX_LOADSTRING 100


#define SCAN_ALL		0
#define SCAN_FILTERED	1
#define SCAN_ALL_GAMES	2
#define MONITOR_INTERVAL 5*1000

#define SERVER_PURGE_COUNTER 5  //Counter after X timeouts to purge (delete) the server




#pragma comment( user, "Compiled on " __DATE__ " at " __TIME__ ) 
#ifndef _DEBUG
char szDialogTitle[]="Game Scanner v" APP_VERSION " ";
#else
char szDialogTitle[]="Game Scanner v" APP_VERSION " Compiled on " __DATE__ " at "__TIME__;
#endif


int g_currentGameIdx = ET_SERVERLIST;
int g_currentScanGameIdx = -1;
HTREEITEM hSelectedTreeItem = NULL;

CLogger	log;
CScriptEngine se;
CGameManager	gm(log);
CTreeViewManager tvmgr(log,gm,se);
CListViewHeader *g_LVHeaderSL = NULL;
CIPtoCountry g_IPtoCountry;
CDownload g_download;
CLanguage g_lang(log);
CXmlConfig g_xmlcfg;
CBuddyManager bm(log,gm,g_lang);

UINT_PTR hTimerMonitor=NULL;

/****************************************
	Buddy Global vars
*****************************************/


BOOL g_bWinSizesLoaded = FALSE;

BOOL bSortColumnAscading[15];
int iLastColumnSortIndex = COL_PLAYERS;
extern _COUNTRYCODES CountryCodes[];
extern BOOL SCANNER_bCloseApp;

DWORD g_dwExportDetails=0;
DWORD g_dwExportGameIdx;

bool bFirstTimeSizeCalc= true;
#define BORDER_SIZE 4

string g_sMIRCoutput ="";

_WINDOW_CONTAINER WNDCONT[15];
PLAYERDATA *pCurrentPL=NULL; //a temporary current player list in listview, this will be 
                             //keept when doing a rescan and the user want's to add aplayer to the buddylist for instance.

CRITICAL_SECTION	SCANNER_cs,SCANNER_CSthreadcounter; 
CRITICAL_SECTION	REDRAWLIST_CS; 
CRITICAL_SECTION	LOAD_SAVE_CS; 
CRITICAL_SECTION	SORTING_CS; 

NOTIFYICONDATA structNID;

bool g_bPlayedNotify=false;


//TVITEM  tvi;

int tabViedwMode = FAVORITE;
void GameTypeFilter_GetSelectedGameType(HTREEITEM hRoot);

HANDLE hCloseEvent = NULL; 
RECT g_winClientRC;

extern SERVER_INFO *g_RCONServer;

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


veclpSI g_vMonitorSI;
veclpSI g_vSelectionSI;

DWORD g_ThreadCounter=0;
DWORD g_serverIndex=0;
DWORD g_serverIndexMAX=0;
char g_szIPtoAdd[260];
LONG_PTR g_wpOrigTreeViewProc=NULL;
LONG_PTR g_wpOrigListViewServerProc=NULL;
LONG_PTR g_wpOrigListBuddyProc=NULL;
LONG_PTR g_wpOrigListViewPlayersProc=NULL;
LONG_PTR g_wpOrigLVRulesProc=NULL;
LONG_PTR g_wpOrigCBSearchProc=NULL;
LONG_PTR g_wpOrigSLHeaderProc = NULL;

LONG_PTR g_wpOrigFilterEditorProc=NULL;

HTREEITEM hTIMINPLY=NULL,hTIMAXPLY=NULL;
DEVMODE  userMode, etMode;

bool g_bNormalWindowed = true;
//_CUSTOM_COLUMN CUSTCOLUMNS[MAX_COLUMNS];
_CUSTOM_COLUMN BUDDY_CUSTCOLUMNS[MAX_COLUMNS];


BOOL Sizing = FALSE;

long UpdateServerItem(DWORD index);
long InsertServerItem(GAME_INFO *pGI,SERVER_INFO *pSI);
int TreeView_save();
void ListView_SetHeaderSortImage(HWND listView, int columnIndex, BOOL isAscending);
BOOL DDE_Init();
void OnActivate_ServerList(DWORD options=0);
long UpdatePlayerList(LPPLAYERDATA pPlayers);
long UpdateRulesList(LPSERVER_RULES pServerRules);

void TreeView_BuildList();

void Update_TreeViewCFGFile();
void SaveAll(DWORD dwCloseReason);
SERVER_INFO * Get_CurrentServerListByView();
void ChangeViewStates(UINT uItem);
SERVER_INFO *FindServer(char *str);
BOOL FindServerRule(char *szRule);
BOOL FindPlayers(const char *szPlayer);

_MYTREEITEM g_EditorTI;
_MYTREEITEM g_CopyTI;
_MYTREEITEM g_PasteAtTI;

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

DWORD AddServer(GAME_INFO *pGI,char *szIP, unsigned short usPort,bool bFavorite);

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

LRESULT CALLBACK CFG_MainProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
HTREEITEM TreeView_AddItem(_MYTREEITEM ti, HTREEITEM hCurrent, bool active=true);
void Select_item_and_all_childs(HTREEITEM hRoot, bool select);
//int TreeView_GetSelectionV3(LPARAM lParam);

char EXE_PATH[_MAX_PATH+_MAX_FNAME];			//Don't write anything to this path
char USER_SAVE_PATH[_MAX_PATH+_MAX_FNAME];     //Path to save settings and server lists
char COMMON_SAVE_PATH[_MAX_PATH+_MAX_FNAME];   //Used for downloading app update and preview images - purpose to share the same data between users.
char EXE_PATH_OLD[_MAX_PATH+_MAX_FNAME];


//DWORD dwCurrPort = 27960;
bool g_bDoFirstTimeCheckForUpdate=true;

bool bPlayerNameAsc=true,bPlayerClanAsc=true,	bRateAsc = true,bPlayerPingAsc=true;
APP_SETTINGS_NEW AppCFG;


// Global Variables:
SERVER_INFO *g_CurrentSRV = NULL;

bool g_bMinimized=false;
SERVER_INFO *g_PRIVPASSsrv = NULL;
RECT g_CSRect,g_INFOIconRect;

vecPlyList g_vecPlayerList;
HIMAGELIST m_pDragImage = NULL;
BOOL		m_bLDragging = FALSE;
HTREEITEM	m_hitemDrag,m_hitemDrop;

HWND g_PROGRESS_DLG = NULL,g_DlgProgress = NULL,g_DlgProgressMsg=NULL;
HWND g_hwndToolbarOptions = NULL;
HWND g_hwndSearchToolbar = NULL, g_hwndSearchCombo= NULL;
HWND g_hwndComboEdit = NULL;
HWND g_hWnd=NULL,g_hwndRibbonBar = NULL;

HIMAGELIST g_hImageListIcons = NULL;
HIMAGELIST g_hImageListStates = NULL;
HIMAGELIST m_hImageList = NULL;
HIMAGELIST m_hImageListSearchBar = NULL;
HIMAGELIST m_hImageListHot = NULL;
HIMAGELIST m_hImageListDis = NULL;
HIMAGELIST g_hILFlags = NULL;
HFONT g_hf  = NULL;
HFONT g_hf2 = NULL;
HFONT g_hfScriptEditor = NULL;

BOOL g_bGameRunning = FALSE;

bool g_bCancel = false;
bool g_bRunningQuery = false;
bool g_bRunningQueryServerList = false;
int g_iCurrentSelectedServer = -1;
int g_statusIcon = -1;
BOOL g_bRedrawServerListThread = FALSE;
HICON g_hAppIcon = NULL;
SERVER_INFO g_FastConnectSrv;

BOOL g_bControl = FALSE;

HWND g_hwndTabControl = NULL;
HWND g_hwndLogger = NULL,g_hwndStatus = NULL;
HWND g_hwndListViewPlayers=NULL,g_hwndListViewVars=NULL,g_hwndListViewServer=NULL,g_hwndListBuddy=NULL;
HWND g_hwndListViewServerListHeader = NULL;
HWND g_hwndMainTreeCtrl=NULL, g_hwndProgressBar=NULL, hwndPaneV=NULL;
HWND hwndPaneH1=NULL,hwndButtonTest=NULL,hwndButtonOptions=NULL,hwndButtonSearch=NULL;
HWND hwndButtonAddToFavorite=NULL;




//GamesMap gm.GamesInfo;

GAME_INFO *currCV = NULL;
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
//   if ((toupper(*wild) != toupper(*string)) && (*wild != '?')) {





void MyDrawFont(HDC hdc, int x, int y, const char *szMessage, int angle)
{
	//RECT rc; 
	HFONT hfnt, hfntPrev; 
	HRESULT hr; 
	 
	// Allocate memory for a LOGFONT structure. 	 
	PLOGFONT plf = (PLOGFONT) LocalAlloc(LPTR, sizeof(LOGFONT)); 
	 	 
	// Specify a font typeface name and weight. 	 
	hr = StringCchCopy(plf->lfFaceName, 8, "Verdana");
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
SERVER_INFO *g_pSIPing;


PLAYERDATA *Get_PlayerBySelection()
{
	int i = ListView_GetSelectionMark(g_hwndListViewPlayers);
	
	if(i!=-1)
	{					
		PLAYERDATA *pPlayerData = g_vecPlayerList.at(i);
		if(pPlayerData==NULL)
		{
			SetStatusText(ICO_WARNING,g_lang.GetString("InvalidPlayer"));
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

	FillRect(hDC, &rect, (HBRUSH) hbrBkgnd);   //fill out the main dark blue background (OUTERBOUND background)
			
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
	FillRect(hDC, &rectInner, (HBRUSH) hbrBkgnd);   //Fill INNERBOUND background
			
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

void Draw_PingStats(HWND hWnd, HDC hDC, char nGridX, char nGridY, DWORD dwMaxValue)
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
			
	//Shrink one pixel for the main drawing area
	rectInner.left = X_OFFSET_STATS+1;
	rectInner.top = X_OFFSET_STATS+1;
	rectInner.bottom--;
	rectInner.right--;

	int offsetX = (rectInner.right / nGridX) / nGridX;
	int offsetY = rectInner.bottom / nGridY;

	int scale = 10;
	if(dwMaxValue!=0)
		scale = dwMaxValue / nGridY-1;


	HPEN hpen = NULL;
	HPEN hpenOld=NULL;	
	HPEN hPenStatus = NULL;
	hpen = CreatePen(PS_DOT, 1, RGB(64,64,204));
	hpenOld = (HPEN) SelectObject(hDC, hpen);
	hPenStatus = CreatePen(PS_SOLID, 1, RGB(0,255,0));
	SelectObject(hDC, hPenStatus);

	deQPing::reverse_iterator iLst;
	int oldX = rectInner.right;
	int oldY = rectInner.bottom;
	iLst = QPing.rbegin();
	
	if(iLst != QPing.rend())
	{
		DWORD dwPingi = *iLst;

		int fact = dwPingi / scale;	
		int y = fact * offsetY;
		y+= dwPingi-(fact*scale);  //rest

		int yTot = rectInner.bottom-y;
		if(yTot<rectInner.top)
			yTot = rectInner.top;
		oldY = yTot;
		
		int i=0;
		for ( iLst = QPing.rbegin(); iLst != QPing.rend(); iLst++ )
		{
			
			dwPingi = *iLst;
			MoveToEx(hDC,oldX,oldY,NULL);
			int fact = dwPingi / scale;
				
			y = fact * offsetY;
			y+= dwPingi-(fact*scale);  //rest

			yTot = rectInner.bottom-y;
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
	}
	SelectObject(hDC, hpenOld);
	DeleteObject(hPenStatus);
	DeleteObject(hpen);
}


void Draw_TraceRouteStats(HWND hWnd, HDC hDC,char nGridX,char nGridY,DWORD dwMaxValue)
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
			
	//Shrink one pixel for the main drawing area
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
	SERVER_INFO *pSI;
	int n = ListView_GetSelectionMark(g_hwndListViewServer);
	if(n!=-1)
	{
		pSI  = Get_ServerInfoByListViewIndex(currCV,n);
		strcpy(szIPAddressToPing,pSI->szIPaddress);
		TraceRoute(pSI->szIPaddress);
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
	
	switch (message)
	{
		case WM_INITDIALOG:
			strcpy(szIPAddressToPing,", right click on a server then click on Network tools->Ping server.");
			ShowWindow(hWnd,SW_HIDE);
			return TRUE;
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
					strcpy(szIPAddressToPing,g_pSIPing->szIPaddress);
					// TraceRoute(szIPAddressToPing);
					//if(strcmp(szIPAddressToPing,szOldIPAddressToPing)!=0)
					//	QPing.clear();				
					//strcpy(szOldIPAddressToPing,g_pSIPing->szIPaddress);

					SetTimer(hWnd,EVENT_PING,1000,NULL);

				} else
					strcpy(szIPAddressToPing,", Please select a server to ping.");
			}
			break;
	case WM_SIZE:
	/*	RECT rc;
		GetClientRect(hWnd,&rc);
	hDC = BeginPaint(hWnd, &ps);
		if(hDC==NULL)
			return FALSE;
		
		Rectangle(hDC,  rc.left, rc.top,rc.right,rc.bottom);
		EndPaint(hWnd, &ps);
*/
		return TRUE;

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
			EndPaint(hWnd, &ps);
			return TRUE;
		
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

bool Sort_Player(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	
	SERVER_INFO *pSIa =  rSIa.pServerInfo;
	SERVER_INFO *pSIb =  rSIb.pServerInfo;
	
	if(g_LVHeaderSL->GetColumnSortOrder(COL_PLAYERS))//CUSTCOLUMNS[COL_PLAYERS].bSortAsc)
		return (pSIa->nPlayers > pSIb->nPlayers);
	else
		return (pSIa->nPlayers < pSIb->nPlayers);
}

bool Sort_Country(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO *pSIa =  rSIa.pServerInfo;
	SERVER_INFO *pSIb =  rSIb.pServerInfo;

	if(AppCFG.bUseShortCountry)
	{
		if(g_LVHeaderSL->GetColumnSortOrder(COL_COUNTRY))
			return (strcmp(pSIa->szShortCountryName , pSIb->szShortCountryName )>0);
		else
		   return (strcmp(pSIa->szShortCountryName , pSIb->szShortCountryName )<0);
	} else
	{
		int idxCC_a = Get_CountryFlagByShortName(pSIa->szShortCountryName);
		int idxCC_b = Get_CountryFlagByShortName(pSIb->szShortCountryName);
		
		if(g_LVHeaderSL->GetColumnSortOrder(COL_COUNTRY))
			return (strcmp(CountryCodes[idxCC_a].szCountryName , CountryCodes[idxCC_b].szCountryName )>0);
		else
		   return (strcmp(CountryCodes[idxCC_a].szCountryName, CountryCodes[idxCC_b].szCountryName )<0);
	}
}

bool Sort_Ping(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO *pSIa =  rSIa.pServerInfo;
	SERVER_INFO *pSIb =  rSIb.pServerInfo;
	if(g_LVHeaderSL->GetColumnSortOrder(COL_PING))	
		return (pSIa->dwPing > pSIb->dwPing);	
	else
		return (pSIa->dwPing < pSIb->dwPing);	
}

bool Sort_Map(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO *pSIa =  rSIa.pServerInfo;
	SERVER_INFO *pSIb =  rSIb.pServerInfo;
	
/*	if(pSIa->szMap==NULL)
		return false;
	if(pSIb->szMap==NULL)
		return true;
*/
	if(g_LVHeaderSL->GetColumnSortOrder(COL_MAP))	
		return (CustomStrCmp(pSIa->szMap , pSIb->szMap )>0);
	else
		return (CustomStrCmp(pSIa->szMap , pSIb->szMap )<0);
}

bool Sort_Mod(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO *pSIa =  rSIa.pServerInfo;
	SERVER_INFO *pSIb =  rSIb.pServerInfo;
/*	if(pSIa->szMod==NULL)
		return false;
	if(pSIb->szMod==NULL)
		return true;
*/
	if(g_LVHeaderSL->GetColumnSortOrder(COL_MOD))	
		return (CustomStrCmp(pSIa->szMod , pSIb->szMod )>0);
	else
		return (CustomStrCmp(pSIa->szMod , pSIb->szMod )<0);
}

bool Sort_IP(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO *pSIa =  rSIa.pServerInfo;
	SERVER_INFO *pSIb =  rSIb.pServerInfo;
	
/*if(pSIa->szIPaddress==NULL)
		return false;
	if(pSIb->szIPaddress==NULL)
		return true;
*/
	if(g_LVHeaderSL->GetColumnSortOrder(COL_IP))
		return (CustomStrCmp(pSIa->szIPaddress , pSIb->szIPaddress )>0);
	else
		return (CustomStrCmp(pSIa->szIPaddress , pSIb->szIPaddress )<0);
}


bool Sort_Punkbuster(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO *pSIa =  rSIa.pServerInfo;
	SERVER_INFO *pSIb =  rSIb.pServerInfo;
	
	if(g_LVHeaderSL->GetColumnSortOrder(COL_PB))
		return (pSIa->bPunkbuster > pSIb->bPunkbuster );
	else
		return (pSIa->bPunkbuster < pSIb->bPunkbuster );

}

bool Sort_Private(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO *pSIa =  rSIa.pServerInfo;
	SERVER_INFO *pSIb =  rSIb.pServerInfo;

	if(g_LVHeaderSL->GetColumnSortOrder(COL_PRIVATE))
		return (pSIa->bPrivate > pSIb->bPrivate );	
	else
		return (pSIa->bPrivate < pSIb->bPrivate );	
}

bool Sort_Ranked(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO *pSIa =  rSIa.pServerInfo;
	SERVER_INFO *pSIb =  rSIb.pServerInfo;
	
	if(g_LVHeaderSL->GetColumnSortOrder(COL_RANKED))
		return (pSIa->cRanked > pSIb->cRanked );	
	else
		return (pSIa->cRanked < pSIb->cRanked );	
}
bool Sort_Bots(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO *pSIa =  rSIa.pServerInfo;
	SERVER_INFO *pSIb =  rSIb.pServerInfo;
	
	if(g_LVHeaderSL->GetColumnSortOrder(COL_BOTS))
		return (pSIa->cBots > pSIb->cBots );	
	else
		return (pSIa->cBots < pSIb->cBots );	
}

static BOOL Sort_ServerName(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	if(g_LVHeaderSL->GetColumnSortOrder(COL_SERVERNAME))
		return (CustomStrCmp(rSIa.pServerInfo->szServerName , rSIb.pServerInfo->szServerName )<0);
	return (CustomStrCmp(rSIa.pServerInfo->szServerName , rSIb.pServerInfo->szServerName )>0); //(CustomStrCmp(rSIa.pServerInfo->szServerName , rSIb.pServerInfo->szServerName )>=0)
}

bool Sort_Status(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO *pSIa =  rSIa.pServerInfo;
	SERVER_INFO *pSIb =  rSIb.pServerInfo;

/*	if(pSIa->szSTATUS==NULL)
		return false;
	if(pSIb->szSTATUS==NULL)
		return true;
*/
	if(g_LVHeaderSL->GetColumnSortOrder(COL_STATUS))
		return (CustomStrCmp(pSIa->szSTATUS , pSIb->szSTATUS )>0);
	 else
		return (CustomStrCmp(pSIa->szSTATUS , pSIb->szSTATUS )<0);

}

bool Sort_Version(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO *pSIa =  rSIa.pServerInfo;
	SERVER_INFO *pSIb =  rSIb.pServerInfo;

/*	if(pSIa->szVersion==NULL)
		return false;
	if(pSIb->szVersion==NULL)
		return true;
*/
	if(g_LVHeaderSL->GetColumnSortOrder(COL_VERSION))
		return (CustomStrCmp(pSIa->szVersion , pSIb->szVersion )>0);
	else
		return (CustomStrCmp(pSIa->szVersion , pSIb->szVersion )<0);
}


bool Sort_GameType(REF_SERVER_INFO rSIa, REF_SERVER_INFO rSIb)
{
	SERVER_INFO *pSIa =  rSIa.pServerInfo;
	SERVER_INFO *pSIb =  rSIb.pServerInfo;

/*	if(pSIa->szGameTypeName==NULL)
		return false;
	if(pSIb->szGameTypeName==NULL)
		return true;
*/
	if(g_LVHeaderSL->GetColumnSortOrder(COL_GAMETYPE))
		return (CustomStrCmp(pSIa->szGameTypeName , pSIb->szGameTypeName )>0);
	else
		return (CustomStrCmp(pSIa->szGameTypeName , pSIb->szGameTypeName )<0);
}

void Do_ServerListSortThread(int iColumn)
{
		HANDLE hThread=NULL; 
		DWORD dwThreadIdBrowser=0;				
		hThread = CreateThread( NULL, 0, &Do_ServerListSort, (LPVOID)iColumn,0, &dwThreadIdBrowser);                
		if (hThread == NULL) 
		{
			log.AddLogInfo(GS_LOG_WARNING, "CreateThread failed (%d) @ Do_ServerListSortThread\n", GetLastError() ); 
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
	
	if(TryEnterCriticalSection(&LOAD_SAVE_CS)==FALSE)
	{
		dbg_print("Busy with loading server list.");
		return 0;
	}
	LeaveCriticalSection(&LOAD_SAVE_CS);


	if(gm.GamesInfo[gameIdx].bLockServerList)
		return 0;

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
	//dbg_print("idx %d\nList view Size %d",gameIdx,gm.GamesInfo[gameIdx].vRefListSI.size());
	if(gm.GamesInfo[gameIdx].vRefListSI.size()>0)
	{
	/*	DWORD id=0;
		for(int i=0;i<MAX_COLUMNS;i++)
		{
			if(CUSTCOLUMNS[i].bActive && (CUSTCOLUMNS[i].columnIdx == iColumn))
			{
				id = CUSTCOLUMNS[i].id;
				sortdir = CUSTCOLUMNS[i].bSortAsc;
				
				break;
			}
		}*/
		iLastColumnSortIndex = iColumn;
		sortdir = g_LVHeaderSL->GetColumnSortOrder(iColumn);
		switch(g_LVHeaderSL->GetColumnIdx(iColumn))
		{
			case COL_PB : 
				sort(gm.GamesInfo[gameIdx].vRefListSI.rbegin(),gm.GamesInfo[gameIdx].vRefListSI.rend(),Sort_Punkbuster); 
				break;
			case COL_PRIVATE: 
				sort(gm.GamesInfo[gameIdx].vRefListSI.rbegin(),gm.GamesInfo[gameIdx].vRefListSI.rend(),Sort_Private);
				break;
			case COL_RANKED: 
				sort(gm.GamesInfo[gameIdx].vRefListSI.rbegin(),gm.GamesInfo[gameIdx].vRefListSI.rend(),Sort_Ranked); 
				break;
			case COL_SERVERNAME: 
				std::sort(gm.GamesInfo[gameIdx].vRefListSI.rbegin(),gm.GamesInfo[gameIdx].vRefListSI.rend(),Sort_ServerName); 
				break;
			case COL_GAMETYPE: 
				sort(gm.GamesInfo[gameIdx].vRefListSI.rbegin(),gm.GamesInfo[gameIdx].vRefListSI.rend(),Sort_GameType); 
				break;
			case COL_MAP: 
				sort(gm.GamesInfo[gameIdx].vRefListSI.rbegin(),gm.GamesInfo[gameIdx].vRefListSI.rend(),Sort_Map); 
				break;
			case COL_MOD: 
				sort(gm.GamesInfo[gameIdx].vRefListSI.rbegin(),gm.GamesInfo[gameIdx].vRefListSI.rend(),Sort_Mod); 
				break;
			case COL_PLAYERS: 
				sort(gm.GamesInfo[gameIdx].vRefListSI.rbegin(),gm.GamesInfo[gameIdx].vRefListSI.rend(),Sort_Player);
				break;				
			case COL_COUNTRY: 
				sort(gm.GamesInfo[gameIdx].vRefListSI.rbegin(),gm.GamesInfo[gameIdx].vRefListSI.rend(),Sort_Country); 
				break;
			case COL_PING: 
				sort(gm.GamesInfo[gameIdx].vRefListSI.rbegin(),gm.GamesInfo[gameIdx].vRefListSI.rend(),Sort_Ping); 
				break;
			case COL_IP: 
				sort(gm.GamesInfo[gameIdx].vRefListSI.rbegin(),gm.GamesInfo[gameIdx].vRefListSI.rend(),Sort_IP); 
				break;
			case COL_VERSION: 
				sort(gm.GamesInfo[gameIdx].vRefListSI.rbegin(),gm.GamesInfo[gameIdx].vRefListSI.rend(),Sort_Version); 
				break;
			case COL_BOTS: 
				sort(gm.GamesInfo[gameIdx].vRefListSI.rbegin(),gm.GamesInfo[gameIdx].vRefListSI.rend(),Sort_Bots);
				 break;
			case COL_STATUS: 
				sort(gm.GamesInfo[gameIdx].vRefListSI.rbegin(),gm.GamesInfo[gameIdx].vRefListSI.rend(),Sort_Status); 
				break;
		}

		ListView_SetHeaderSortImage(g_hwndListViewServer, iColumn,(BOOL) sortdir); //id =iColumn
		ListView_SetItemCount(g_hwndListViewServer,gm.GamesInfo[gameIdx].vRefListSI.size());

	}
	LeaveCriticalSection(&REDRAWLIST_CS);

	return 1;
}


DWORD WINAPI PlayNotifySound(LPVOID lpParam )
{
	 g_bPlayedNotify=true;
	 if(AppCFG.bUseBuddySndNotify)
		PlaySound(AppCFG.szNotifySoundWAVfile, 0,SND_ASYNC | SND_FILENAME);
	 return 0;
}

void ShowBalloonTip(char *szTitle, char *szMessage)
{
	
	NOTIFYICONDATA structNID;
	ZeroMemory(&structNID,sizeof(NOTIFYICONDATA));
	structNID.cbSize =  sizeof(NOTIFYICONDATA); 
	structNID.hWnd = (HWND) g_hWnd; 
	structNID.uID = 100123; 
	structNID.uFlags = NIF_INFO| NIF_ICON | NIF_MESSAGE; 
	strcpy(structNID.szTip,"Game Scanner"); //Text of the tooltip 
	structNID.hIcon = g_hAppIcon; 
	structNID.uCallbackMessage = WM_USER_SHELLICON; // user defined message that will be sent as the notification message to the Window Procedure 
	
	structNID.dwInfoFlags = NIIF_INFO; //NIIF_ERROR;  //NIIF_WARNING; 
	structNID.uTimeout = 10000;
	
	strcpy_s(structNID.szInfoTitle,sizeof(structNID.szInfoTitle),szTitle); 
	strcpy_s(structNID.szInfo,sizeof(structNID.szInfo) ,szMessage); 

	//put the actual icon now
	Shell_NotifyIcon(NIM_MODIFY, &structNID); 

}


LRESULT CALLBACK PRIVPASS_Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	
	case WM_INITDIALOG:
		{
			
			SetWindowText(hDlg,g_lang.GetString("TitleSetPrivatePass"));
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

	REF_SERVER_INFO refSI;

	GAME_INFO *pGI = currCV;
	SERVER_INFO *pSrvInf = NULL; 

	if (ctrlid!=IDC_LIST_SERVER)
		return FALSE;
	
	if(pNMHDR==NULL)
		return FALSE;

	pLVDI = (NMLVDISPINFO *)pNMHDR;

	if(pGI->bLockServerList)
		return FALSE;

	pLVItem = &pLVDI->item;
	int size = pGI->vRefListSI.size();
	if(size<=0)
		return FALSE;

	if(pLVItem->iItem>=size)
		return FALSE;

	__try{
		refSI = pGI->vRefListSI.at((int)pLVItem->iItem);
		pGI->vSI.at(refSI.pServerInfo->dwIndex)->dwLVIndex = pLVItem->iItem;
		pSrvInf = refSI.pServerInfo; 
	}
	__except(EXCEPTION_ACCESS_VIOLATION == GetExceptionCode())
	{
		dbg_print("Access violation...@ ListView_SL_OnGetDispInfoList");
		return FALSE;
	}

	char szText[256];
	if(pSrvInf==NULL)
		return FALSE;

	GetServerLock(pSrvInf);

	int i=0;

			switch(g_LVHeaderSL->GetId(pLVItem->iSubItem))
			{
				case COL_PB:
					{
						if(pSrvInf->bPunkbuster) //PB or VAC
						{

							if(gm.GamesInfo[pSrvInf->cGAMEINDEX].GAME_ENGINE == VALVE_ENGINE)
								pLVItem->iImage = 21;  //VAC/Steam icon
							else								
								pLVItem->iImage = 1; //Punkbuster icon							
						}
						i = MAX_COLUMNS;
					break;
					}
				case COL_PRIVATE:
					{
						if (pSrvInf->bPrivate) //Private
							pLVItem->iImage = 4;
						i = MAX_COLUMNS;
					break;
					}					
				case COL_RANKED:
					{
						if (pSrvInf->cRanked) //Ranked only used for ETQW for now
							pLVItem->iImage = 19;
						i = MAX_COLUMNS;
					break;
					}					
				case COL_SERVERNAME:
					{

						if (pSrvInf->cFavorite)
							pLVItem->iImage = 2;
						else
							pLVItem->iImage = gm.Get_GameIcon(pGI->cGAMEINDEX);

						if(pSrvInf->wMonitor > 0 )
							pLVItem->iImage = 29;	

						if(pLVItem->mask & LVIF_TEXT)
						{
							if(pGI->vGAME_SPEC_COL.size()>=COL_SERVERNAME)
							{
								char *szVarValue = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_SERVERNAME).sRuleValue.c_str(),pSrvInf->pServerRules);
								if(szVarValue!=NULL)							
								{	
									if(pGI->colorfilter!=NULL)
									{	
										pGI->colorfilter(pSrvInf->szServerName,szText,sizeof(szText));
										pLVItem->pszText = szText;
									} else
										pLVItem->pszText = pSrvInf->szServerName;

								}
							}
						}
						i = MAX_COLUMNS;
					break;
					}	
				case COL_VERSION:
					{	
						if(pLVItem->mask & LVIF_TEXT)
						{
							if(pGI->vGAME_SPEC_COL.size()>=COL_VERSION)
							{
								char *szVarValue = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_VERSION).sRuleValue.c_str(),pSrvInf->pServerRules);
								if(szVarValue!=NULL)							
									strncpy(pLVItem->pszText,szVarValue,pLVItem->cchTextMax);
							}
						}
						i = MAX_COLUMNS;
					break;
					}	
				case COL_GAMETYPE:
					{
						if(pLVItem->mask & LVIF_TEXT)
						{
							if(pGI->vGAME_SPEC_COL.size()>=COL_GAMETYPE)
							{
								char *szVarValue = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_GAMETYPE).sRuleValue.c_str(),pSrvInf->pServerRules);
								if(szVarValue==NULL)
									szVarValue = Get_RuleValue(_T("gametype"),pSrvInf->pServerRules); //Warsow v0.50 fix

								if(szVarValue!=NULL)							
									strncpy(pLVItem->pszText,szVarValue,pLVItem->cchTextMax);
							}
						}
						i = MAX_COLUMNS;
					break;
					}	
				case COL_MAP:
					{
						if(pLVItem->mask & LVIF_TEXT)
						{
							if(pGI->vGAME_SPEC_COL.size()>=COL_MAP)
							{
								char *szVarValue = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_MAP).sRuleValue.c_str(),pSrvInf->pServerRules);
								if(szVarValue!=NULL)							
									strncpy(pLVItem->pszText,szVarValue,pLVItem->cchTextMax);
							}
						}
						i = MAX_COLUMNS;
					break;
					}	
				case COL_MOD:
					{
						if(pLVItem->mask & LVIF_TEXT)
						{							
							if(pGI->vGAME_SPEC_COL.size()>=COL_MOD)
							{
								char *szVarValue = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_MOD).sRuleValue.c_str(),pSrvInf->pServerRules);
								if(szVarValue!=NULL)							
									strncpy(pLVItem->pszText,szVarValue,pLVItem->cchTextMax);
							}
						}
						i = MAX_COLUMNS;
					break;
					}
				case COL_PLAYERS:
					{			
						if(pLVItem->mask & LVIF_TEXT)
						{
							sprintf_s(szText,sizeof(szText)-1,"%d/%d+(%d)",pSrvInf->nPlayers,pSrvInf->nMaxPlayers,pSrvInf->nPrivateClients);
							strncpy(pLVItem->pszText,szText,pLVItem->cchTextMax);
						}
						i = MAX_COLUMNS;
					break;
					}
				case COL_COUNTRY:
					{
		
						if(pLVItem->mask & LVIF_TEXT)
						{
							int idxCC = Get_CountryFlagByShortName(pSrvInf->szShortCountryName);								
							if(AppCFG.bUseShortCountry)
								strncpy(pLVItem->pszText,CountryCodes[idxCC].szCountryCode,pLVItem->cchTextMax);
							else
								strncpy(pLVItem->pszText,CountryCodes[idxCC].szCountryName,pLVItem->cchTextMax);							
						}
						i = MAX_COLUMNS;
					break;
					}
				case COL_PING:
					{
						sprintf_s(szText,sizeof(szText)-1,"%d",pSrvInf->dwPing);
						if(pLVItem->mask & LVIF_TEXT)
							strncpy(pLVItem->pszText,szText,pLVItem->cchTextMax);
						i = MAX_COLUMNS;
					break;
					}
				case COL_IP:
					{
						sprintf_s(szText,sizeof(szText)-1,"%s:%d",pSrvInf->szIPaddress,pSrvInf->usPort);
						if(pLVItem->mask & LVIF_TEXT)
							strncpy(pLVItem->pszText,szText,pLVItem->cchTextMax);
						i = MAX_COLUMNS;
					break;
					}
				case COL_BOTS:
					{
						sprintf_s(szText,sizeof(szText)-1,"%d",pSrvInf->cBots);
						if(pLVItem->mask & LVIF_TEXT)
							strncpy(pLVItem->pszText,szText,pLVItem->cchTextMax);
						i = MAX_COLUMNS;
					break;
					}
				case COL_STATUS:
					{
						if(pLVItem->mask & LVIF_TEXT)
							if(pSrvInf->szSTATUS!=NULL)
								strncpy(pLVItem->pszText,pSrvInf->szSTATUS,pLVItem->cchTextMax);
						i = MAX_COLUMNS;
					break;
					}
			} 
	
	ReleaseServerLock(pSrvInf);
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

	if(pLVItem->iItem>g_vecPlayerList.size())
		return FALSE;

	PLAYERDATA *pPlayerData = g_vecPlayerList.at(pLVItem->iItem); // pCurrentPL;

	if(pPlayerData==NULL)
		return FALSE;


	char szText[256];
	char colFiltered[256];

	switch(pLVItem->iSubItem)
		{
			case 0:
				sprintf_s(szText,sizeof(szText)-1,"%d",pLVDI->item.iItem+1);
				pLVItem->pszText= szText;
				return TRUE;
			case 1:
				if(pPlayerData->szTeam!=NULL)
				{				
			
					if(gm.GamesInfo[pPlayerData->cGAMEINDEX].colorfilter!=NULL)
						pLVItem->pszText = gm.GamesInfo[pPlayerData->cGAMEINDEX].colorfilter(pPlayerData->szTeam,colFiltered,sizeof(colFiltered)-1); 
					else
						pLVItem->pszText = pPlayerData->szTeam;
				
				}
				return TRUE;
			case 2:
				{
			
				if( pPlayerData->szClanTag!=NULL)
					sprintf_s(szText,sizeof(szText)-1,"%s %s", pPlayerData->szClanTag,pPlayerData->szPlayerName);
				else
					sprintf_s(szText,sizeof(szText)-1,"%s",pPlayerData->szPlayerName);

				if(gm.GamesInfo[pPlayerData->pServerInfo->cGAMEINDEX].colorfilter!=NULL)
					pLVItem->pszText = gm.GamesInfo[pPlayerData->pServerInfo->cGAMEINDEX].colorfilter(szText,colFiltered,sizeof(colFiltered)-1); 
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
				if(gm.GamesInfo[pPlayerData->pServerInfo->cGAMEINDEX].GAME_ENGINE  ==	VALVE_ENGINE)
					sprintf_s(szText,sizeof(szText)-1,"%d.%d",pPlayerData->time/60,(pPlayerData->time - ((pPlayerData->time/60)*60)));
				else
					sprintf_s(szText,sizeof(szText)-1,"%d",pPlayerData->time);
				pLVItem->pszText = szText;
				return TRUE;
			case 6:
				{
			
				
				if(pPlayerData->dwServerIndex==999999)
				{
					if(gm.GamesInfo[pPlayerData->cGAMEINDEX].colorfilter!=NULL)
						pLVItem->pszText = gm.GamesInfo[pPlayerData->cGAMEINDEX].colorfilter(g_FastConnectSrv.szServerName,colFiltered,sizeof(colFiltered)-1); 
					else
						pLVItem->pszText = g_FastConnectSrv.szServerName;
				}
				else
				{
					SERVER_INFO *pSI = pPlayerData->pServerInfo; //gm.GamesInfo[pPlayerData->pServerInfo->cGAMEINDEX].vSI.at(pPlayerData->dwServerIndex);

					if(gm.GamesInfo[pSI->cGAMEINDEX].colorfilter!=NULL)
						pLVItem->pszText = gm.GamesInfo[pSI->cGAMEINDEX].colorfilter(pSI->szServerName,colFiltered,sizeof(colFiltered)-1); 
					else
						pLVItem->pszText = pSI->szServerName;
				}
				return TRUE;
				}				
		}
	return TRUE;
}




/*************************
	Retrieve server info
***************************/

DWORD WINAPI Monitor_ScanThread(LPVOID lpVoid)
{
	KillTimer(g_hWnd,IDT_MONITOR_QUERY_SERVERS);
	hTimerMonitor=NULL;	
	TCHAR sztmpBuffer[256];
	BOOL bSetNewTimer=TRUE;
	dbg_print("Monitor_ScanThread called.");
	for(int i=0; i<g_vMonitorSI.size();i++)
	{
		SERVER_INFO *pSI = g_vMonitorSI.at(i);
		if(pSI!=NULL)
		{
			gm.GamesInfo[pSI->cGAMEINDEX].GetServerStatus(pSI,NULL,NULL);
			if(bWaitingToSave)
				return 0;
			switch(pSI->wMonitor)
			{
				case MONITOR_AUTOJOIN:
					if(pSI->nPlayers<pSI->nMaxPlayers)
					{
						pSI->wMonitor = 0;
						LaunchGame(pSI,&gm.GamesInfo[pSI->cGAMEINDEX]);
						bSetNewTimer = FALSE;
					}
					break;
				case MONITOR_NOTIFY_FREE_SLOTS:
					if(pSI->nPlayers<pSI->nMaxPlayers)
						ShowBalloonTip("Server has free slots!",gm.GamesInfo[pSI->cGAMEINDEX].colorfilter(pSI->szServerName,sztmpBuffer,256));  
					break;
				case MONITOR_NOTIFY_ACTIVITY:
					if(pSI->nPlayers>0)
						ShowBalloonTip("Server has player(s)!",gm.GamesInfo[pSI->cGAMEINDEX].colorfilter(pSI->szServerName,sztmpBuffer,256));  
					break;
			}
		}
			
		Sleep(200);
		if(bWaitingToSave)
			return 0;
	}

	if(bSetNewTimer)
	{
		if(hTimerMonitor==NULL)
			hTimerMonitor = SetTimer(g_hWnd,IDT_MONITOR_QUERY_SERVERS,MONITOR_INTERVAL,0);	
	}
	
	return 0;
}

void Initialize_Monitor()
{
	HANDLE 	hThread = NULL;	
	DWORD dwThreadIdBrowser;
	//dbg_print("Creating Initialize_Monitor thread!");
	
	hThread = CreateThread( NULL, 0, &Monitor_ScanThread, (LPVOID)0,0, &dwThreadIdBrowser);                
	if (hThread == NULL) 
	{
		log.AddLogInfo(GS_LOG_WARNING,"CreateThread failed @ Initialize_Monitor (%d)\n", GetLastError() ); 
	}
	else 
	{
		CloseHandle( hThread );
	}
}

DWORD WINAPI Selection_ScanThread(LPVOID lpVoid)
{
	int n=-1;
	g_vSelectionSI.clear();
	SERVER_INFO *pSI = NULL; 
	ListView_DeleteAllItems(g_hwndListViewVars);
	ListView_DeleteAllItems(g_hwndListViewPlayers);

	if(pCurrentPL!=NULL)
	{
		g_vecPlayerList.clear();
		CleanUp_PlayerList(pCurrentPL);
		pCurrentPL = NULL;
	}	
	SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
	
	while ((n =  ListView_GetNextItem(g_hwndListViewServer,n,LVNI_SELECTED))!=-1)
	{		
		g_bControl = TRUE; //Simulate control press
		pSI = Get_ServerInfoByListViewIndex(currCV,n);		
		gm.GamesInfo[pSI->cGAMEINDEX].GetServerStatus(pSI,&UpdatePlayerList,&UpdateRulesList);		
		ListView_Update(g_hwndListViewServer,pSI->dwLVIndex);
		UpdateCurrentServerUI();
		ListView_SetSelectionMark(g_hwndListViewServer,pSI->dwLVIndex);

	}
	g_bControl = FALSE;
	SetCursor(::LoadCursor(NULL,IDC_ARROW));
	UpdateWindow(g_hwndListViewServer);
	UpdateWindow(g_hwndListViewPlayers);
	UpdateWindow(g_hwndListViewVars);
	return 0;
}



void Initialize_ScanSelection()
{
	HANDLE 	hThread = NULL;	
	DWORD dwThreadIdBrowser;
	dbg_print("Creating Initialize_ScanSelection thread!\n");
	
	hThread = CreateThread( NULL, 0, &Selection_ScanThread, (LPVOID)0,0, &dwThreadIdBrowser);                
	if (hThread == NULL) 
	{
		log.AddLogInfo(GS_LOG_WARNING,"CreateThread failed @ Initialize_ScanSelection (%d)\n", GetLastError() ); 
	}
	else 
	{
		CloseHandle( hThread );
	}
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
	
	
		g_CurrentSRV = Get_ServerInfoByListViewIndex(pGI,g_iCurrentSelectedServer);
		
		g_vSelectionSI.push_back(g_CurrentSRV);

		if(g_CurrentSRV == NULL)
			return;				
	
		gm.GetServerInfo(g_currentGameIdx,g_CurrentSRV);
		UpdateServerItem(g_iCurrentSelectedServer);
		UpdateCurrentServerUI();
		UpdateWindow(g_hwndListViewServer);
		UpdateWindow(g_hwndListViewPlayers);
		UpdateWindow(g_hwndListViewVars);
			
	}
	g_bRunningQuery = false;
}



void RegisterProtocol(char *path)
{
	LONG ret;
	HKEY hkey=NULL;
	DWORD dwDisposition;
	BYTE szBuffer[512];
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

	for(int i=0; i<gm.GamesInfo.size();i++)
	{
		hkey=NULL;
		sprintf((char*)szBuffer,"%s",gm.GamesInfo[i].szWebProtocolName);
		ret = RegCreateKeyEx(HKEY_CLASSES_ROOT,(LPCSTR)&szBuffer, 0, NULL,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition ); 
		if(ret==ERROR_SUCCESS)
		{
			sprintf((PTCHAR)szBuffer,"URL:%s",gm.GamesInfo[i].szWebProtocolName);
			ret = RegSetValueEx( hkey, "", 0, REG_SZ, (PTBYTE)&szBuffer, (DWORD)strlen((PTCHAR)szBuffer)); 
			memset(szBuffer,0,sizeof(szBuffer));
			ret = RegSetValueEx( hkey, "URL Protocol", 0, REG_SZ, (BYTE*)szBuffer, (DWORD)strlen((PTCHAR)szBuffer)); 
			RegCloseKey( hkey ); 
		}
		sprintf((PTCHAR)szBuffer,"%s\\shell\\open\\command",gm.GamesInfo[i].szWebProtocolName);
		ret = RegCreateKeyEx(HKEY_CLASSES_ROOT, (LPCSTR)szBuffer, 0, NULL,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition ); 
		if(ret==ERROR_SUCCESS)
		{
			ret = RegSetValueEx( hkey, "", 0, REG_SZ, (PBYTE)totpath, strlen(totpath)); 
			RegCloseKey( hkey ); 
		}
	}
}









void Default_Appsettings()
{

	//log.AddLogInfo(GS_LOG_INFO,"Settings set to defaults.");
	
	ZeroMemory(&AppCFG,sizeof(APP_SETTINGS_NEW));
                     
	g_sMIRCoutput = "/ame is joining server %SERVERNAME% %GAMENAME% %IP% %PRIVATE%";
	
	AppCFG.bShowMinimizePopUp = TRUE;
	AppCFG.bRegisterWebProtocols = FALSE;
	AppCFG.bAutostart = FALSE;
	AppCFG.bCloseOnConnect = FALSE;
	AppCFG.bUse_minimize = TRUE;
	AppCFG.dwMinimizeMODKey =MOD_ALT;
	AppCFG.cMinimizeKey = 'Z';
	AppCFG.bUSE_SCREEN_RESTORE = FALSE;
	AppCFG.bBuddyNotify = TRUE;
	AppCFG.bUseColorEncodedFont = TRUE;
	AppCFG.bUseShortCountry = FALSE;	
	AppCFG.nWindowState = SW_SHOWNORMAL;


	memset(AppCFG.szEXT_EXE_CMD,0,MAX_PATH);	
	memset(AppCFG.szEXT_EXE_PATH,0,MAX_PATH);

	memset(AppCFG.szOnReturn_EXE_PATH,0,MAX_PATH);	
	memset(AppCFG.szOnReturn_EXE_CMD,0,MAX_PATH);


	AppCFG.bUse_EXT_APP = FALSE;	
	strcpy(AppCFG.szLanguageFilename,"lang_en.xml");
	strcpy(AppCFG.szEXT_EXE_PATH,"C:\\Program Files\\Teamspeak2_RC2\\TeamSpeak.exe");
	strcpy(AppCFG.szEXT_EXE_CMD,"127.0.0.1?nickname=MyNick?loginname=MyLoginAccount?password=XYZ?channel=Axis");
	strcpy(AppCFG.szEXT_EXE_WINDOWNAME,"TEAMSPEAK 2");
	strcpy(AppCFG.szET_WindowName,"Enemy Territory|Wolfenstein|Quake4|F.E.A.R.|ETQW|Warsow|Call of Duty 4|WolfMP");


	//Legacy stuff - this should be cleared out from source code...


	//Default Network
	AppCFG.dwThreads = 64;
	AppCFG.socktimeout.tv_sec = 1;
	AppCFG.socktimeout.tv_usec  = 0;
	AppCFG.dwRetries = 0;

	AppCFG.g_cTransparancy = 100;
	
	AppCFG.dwSleep = 150;

	AppCFG.cBuddyColumnSort = 0; 
	
	AppCFG.bShowBuddyList = TRUE;
	AppCFG.bShowMapPreview = FALSE;
	AppCFG.bShowServerRules = TRUE;
	AppCFG.bShowPlayerList = TRUE;
	
	AppCFG.bSortBuddyAsc = TRUE;
	
	AppCFG.bSortPlayerAsc = TRUE;
	
	AppCFG.bUseCountryFilter = FALSE;
	
	AppCFG.bUseBuddySndNotify = TRUE;
	strcpy(AppCFG.szNotifySoundWAVfile,EXE_PATH);
	strcat(AppCFG.szNotifySoundWAVfile,"\\online.wav");
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
	char ip[100];
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
		SERVER_INFO *pSrv;	
		pSrv = Get_ServerInfoByListViewIndex(currCV,i);
		LaunchGame(pSrv,&gm.GamesInfo[g_currentGameIdx]);

	}
}

SERVER_INFO *FindServerByIPandPort(char *szIP, DWORD dwPort)
{
	vSRV_INF::iterator  iLst;
	char destPort[10];
	SERVER_INFO *g_tmpSRV = NULL;
	DWORD dwIP = NetworkNameToIP(szIP,_itoa(dwPort,destPort,10));
	if(currCV->vSI.size()>0)
	{
		for ( iLst = currCV->vSI.begin( ); iLst != currCV->vSI.end( ); iLst++ )
		{
			g_tmpSRV = *iLst;
			if((g_tmpSRV->dwIP == dwIP) && (g_tmpSRV->usPort == dwPort))
			{
				return g_tmpSRV;
			}
		}
	}
	return NULL;
}


/*
  returns 0xFFFFFFFF (-1), if IP is not satisfied 
  if adding a IP and Favorite=false then if exsistent return 0xFFFFFFFF otherwise the new index
  if adding a IP and favorite=true and it exisist set server as favorite and return the current index

 */

DWORD AddServer(GAME_INFO *pGI,char *szIP, unsigned short usPort,bool bFavorite)
{
	SERVER_INFO *pSI;
	pSI = (SERVER_INFO*)calloc(1,sizeof(SERVER_INFO));
	//ZeroMemory(&pSI,sizeof(SERVER_INFO));
	char destPort[10];
	if(szIP==NULL)
		return 0xFFFFFFFF;

	if(strlen(szIP)<7)
	{
		free(pSI);
		return 0xFFFFFFFF;
	}
	

	pSI->cGAMEINDEX = pGI->cGAMEINDEX;
	strcpy(pSI->szIPaddress,szIP);
	pSI->dwIP = NetworkNameToIP(szIP,_itoa(usPort,destPort,10));
	pSI->usPort = usPort;
	pSI->usQueryPort = usPort;

	int iResult = CheckForDuplicateServer(pGI,pSI);
	if(iResult!=-1) //did we get an exsisting server?
	{
		 //If yes then set that server to a favorite
		if(bFavorite)
			pGI->vSI[iResult]->cFavorite = 1;
		else
		{
			free(pSI);
			return 0xFFFFFFFF;
		}
		free(pSI);
		return pGI->vSI[iResult]->dwIndex;
	}
		
	//Add a new server into current list!
	InitializeCriticalSection(&pSI->csLock);
	pSI->dwPing = 9999;
	strcpy(pSI->szShortCountryName,"zz");
	pSI->bUpdated = 0;
	
	pSI->dwIndex = pGI->vSI.size();
	if(bFavorite)
		pSI->cFavorite = 1;
	
	int hash = pSI->dwIP + pSI->usPort;
	pGI->shash.insert(Int_Pair(hash,pSI->dwIndex));
	pGI->vSI.push_back(pSI);
	InsertServerItem(pGI,pSI);
		
	return pSI->dwIndex;
}



/*
DWORD TreeView_SwapDWCheckStateOR(TVITEM  *pTVitem, _MYTREEITEM ti,DWORD *dwVal)
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
*/
/*
DWORD TreeView_SetDWCheckState (TVITEM  *pTVitem, _MYTREEITEM ti, BOOL active)
{
	if(active) //Is it checked
	{
		pTVitem->iSelectedImage =	pTVitem->iImage = CHECKED_ICON;  //checked image
		tvmgr.vTI.at(ti.dwIndex).dwState = 1;
		ti.dwState = 1;
	}
	else
	{
		pTVitem->iSelectedImage =	pTVitem->iImage = UNCHECKED_ICON;  //unChecked image
		ti.dwState = 0;
		tvmgr.vTI.at(ti.dwIndex).dwState = 0;

	}			
	TreeView_SetItem(g_hwndMainTreeCtrl, pTVitem );
	return ti.dwValue;
}

//Used for group selection such as Ping
DWORD TreeView_UncheckAllTypes(char cGameIdx, DWORD dwType)
{	
	for(UINT i=0;i<tvmgr.vTI.size();i++)
	{
		
		if((tvmgr.vTI.at(i).dwType == dwType) && (tvmgr.vTI.at(i).cGAMEINDEX == cGameIdx) )
		{
			TVITEM  tvitem;
			ZeroMemory(&tvitem,sizeof(TVITEM));
			tvitem.hItem = tvmgr.vTI.at(i).hTreeItem;
			tvitem.mask = TVIF_SELECTEDIMAGE |  TVIF_IMAGE;
			TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );
			TreeView_SetDWCheckState(&tvitem, tvmgr.vTI.at(i), FALSE);		
		}
	}
	return 0;
}
*/

/*
HTREEITEM TreeView_GetTIByItemType(DWORD dwType)
{
	for(UINT i=0;i<tvmgr.vTI.size();i++)
	{
		if(tvmgr.vTI.at(i).dwType == dwType)
			return tvmgr.vTI.at(i).hTreeItem;
	}
	return NULL;
}
*/


/*
DWORD TreeView_GetItemStateByType(char cGameIdx,DWORD dwType)
{
	for(UINT i=0;i<tvmgr.vTI.size();i++)
	{
		if((tvmgr.vTI.at(i).dwType == dwType) && (tvmgr.vTI.at(i).cGAMEINDEX == cGameIdx))
			return tvmgr.vTI.at(i).dwState;
	}
	return 0;
}
*/

/*******************************************************
 
 Sets a new value depending of type.
 Return if successfully found the dwType.
	This function will only change the first occurrence, 
	therefore recommendation is that dwType is unique.

********************************************************/
/*BOOL TreeView_SetDWValueByItemType(DWORD dwType,DWORD dwNewValue,DWORD dwNewState, char*pszNewStrValue)
{
	for(UINT i=0;i<tvmgr.vTI.size();i++)
	{
		if(tvmgr.vTI.at(i).dwType == dwType)
		{
			tvmgr.vTI.at(i).dwValue = dwNewValue;
			tvmgr.vTI.at(i).dwState = dwNewState;
			//if(pszNewStrValue!=NULL)
			//	tvmgr.vTI.at(i).strValue = pszNewStrValue;
			 if(tvmgr.vTI.at(i).dwType==DO_GLOBAL_EDIT_FILTER)
			 {
				char text[256];
				sprintf(text,"%s %d",tvmgr.vTI.at(i).sName.c_str(),tvmgr.vTI.at(i).dwValue);
				TreeView_SetItemText(tvmgr.vTI.at(i).hTreeItem,text);
				TreeView_SetCheckBoxState(i,tvmgr.vTI.at(i).dwState);
			}
			return TRUE;
		}
	}
	return FALSE;
}
*/
/*
BOOL TreeView_SetItemStateByNameAndType(string name,DWORD dwType,DWORD dwNewState)
{
	for(UINT i=0;i<tvmgr.vTI.size();i++)
	{
		if((tvmgr.vTI.at(i).sName == name) && (tvmgr.vTI.at(i).dwType == dwType) )
		{
			tvmgr.vTI.at(i).dwState = dwNewState;
			
			return TRUE;
		}
	}
	return FALSE;
}
*/
/*********************************************************
This function is good for when upgrading the treeview list,
purpose is to restore it's original value set by the user.
**********************************************************/
/*
BOOL TreeView_SetFilterGroupCheckState(char gameIdx, DWORD dwType,DWORD dwFilterValue)
{
	BOOL bChanges=FALSE;
	for(UINT i=0;i<tvmgr.vTI.size();i++)
	{

		if((tvmgr.vTI.at(i).cGAMEINDEX == gameIdx) && (tvmgr.vTI.at(i).dwType == dwType))
		{		
			if(tvmgr.vTI.at(i).dwValue & dwFilterValue)
			{
				tvmgr.vTI.at(i).dwState = 1;	
				TreeView_SetCheckBoxState(i,tvmgr.vTI.at(i).dwState);
				bChanges = TRUE;

			}
		}
	}
	return bChanges;
}
*/
/*
BOOL TreeView_SetFilterCheckState(char gameIdx, DWORD dwType,DWORD dwNewState)
{
	BOOL bChanges=FALSE;
	for(UINT i=0;i<tvmgr.vTI.size();i++)
	{

		if((tvmgr.vTI.at(i).cGAMEINDEX == gameIdx) && (tvmgr.vTI.at(i).dwType == dwType))
		{		
				TreeView_SetCheckBoxState(i,dwNewState);
				bChanges = TRUE;
		}
	}
	return bChanges;
}*/

/*******************************************************
 
 Gets a new value depending of type.
 Return if successfully found the dwType.
	This function will only change the first occurence, 
	therefore recommendation is that dwType is unique.

********************************************************/
/*DWORD TreeView_GetDWValue(DWORD dwType,DWORD dwType)
{
	for(UINT i=0;i<tvmgr.vTI.size();i++)
	{
		if((tvmgr.vTI.at(i).dwType == dwType) && (tvmgr.vTI.at(i).dwType==dwType))
		{		

			return tvmgr.vTI.at(i).dwValue;
		}
	}
	return 0;
}
*/
/*
DWORD TreeView_GetDWStateByGameType(int iGameType, DWORD dwType,DWORD dwType)
{
	for(UINT i=0;i<tvmgr.vTI.size();i++)
	{
		if((tvmgr.vTI.at(i).cGAMEINDEX == iGameType)&& (tvmgr.vTI.at(i).dwType == dwType) && (tvmgr.vTI.at(i).dwType==dwType))
		{		

			return tvmgr.vTI.at(i).dwState;
		}
	}
	return 0;
}
*/



/*
BOOL TreeView_DeleteByHItemTree(HTREEITEM hItemtree)
{
	for(UINT i=0;i<tvmgr.vTI.size();i++)
	{
		if(tvmgr.vTI.at(i).hTreeItem == hItemtree)
		{	
			tvmgr.vTI.erase(tvmgr.vTI.begin()+i);
			return TRUE;
		}
	}
	return FALSE;
}
*/





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
        char   TTL = 0; 
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
			SetWindowText(hDlg,g_lang.GetString("TitleAddIP"));
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
				dwPort = gm.GamesInfo[g_currentGameIdx].dwDefaultPort;

			if(AddServer(&gm.GamesInfo[g_currentGameIdx],ip, dwPort,true)!=0xFFFFFFFF)		
			{
				SetStatusText(ICO_INFO,"Added IP %s:%d into %s favorite serverlist.",ip,dwPort,gm.GamesInfo[g_currentGameIdx].szGAME_NAME);
				
			}else
			{
				SetStatusText(ICO_WARNING,g_lang.GetString("InvalidIP"));
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
	for(int i=0; i<tvmgr.CountryFilter.counter;i++)
	{
		if (tvmgr.CountryFilter.lParam[i]==lParam)
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




//DWORD g_TreeLevel=1;




 void OnActivate()
 {
	if (AppCFG.bUse_EXT_APP2 && g_bGameRunning)
	{
		 if(FindGameWindow()==NULL)
		 {
			if(strlen(AppCFG.szOnReturn_EXE_PATH)>0)
			{		
				ShellExecute(GetDesktopWindow(), "open", AppCFG.szOnReturn_EXE_PATH, AppCFG.szOnReturn_EXE_CMD,NULL, 1);				
				g_bGameRunning = FALSE;
			}
			KillTimer(g_hWnd,IDT_DETECT_GAME);
		 }
	 }
 }

void OnActivate_ServerList(DWORD options)
{
	if(TryEnterCriticalSection(&LOAD_SAVE_CS)==FALSE)
		return;
	LeaveCriticalSection(&LOAD_SAVE_CS);

	Initialize_RedrawServerListThread();
	//if(g_bRunningQueryServerList==false)
	{
		HANDLE hThread=NULL; 
		DWORD dwThreadIdBrowser;
		hThread = NULL;					
		hThread = CreateThread( NULL, 0, &GetServerList, (LPVOID)options,0, &dwThreadIdBrowser);                
		if (hThread == NULL) 
		{
			log.AddLogInfo(GS_LOG_WARNING, "CreateThread failed (%d)\n", GetLastError() ); 
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

		g_vecPlayerList.push_back(player);

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
		g_vecPlayerList.clear();
		CleanUp_PlayerList(pCurrentPL);
		
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
		Copy_PlayerToCurrentPL(pCurrentPL,pPlayers);  //This will keep a copy of the playerlist during scanning
		pPlayers = pPlayers->pNext;
		
		n++;
	
	}
	ListView_SetItemCount(g_hwndListViewPlayers, n);

/*
	ListView_SetColumnWidth(g_hwndListViewPlayers,0,LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(g_hwndListViewPlayers,1,LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(g_hwndListViewPlayers,2,LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(g_hwndListViewPlayers,3,LVSCW_AUTOSIZE);

*/	
	ListView_SetColumnWidth(g_hwndListViewPlayers,6,LVSCW_AUTOSIZE);
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



//Quick connect from the EDIT control
void FastConnect()
{
	char ip[200];
	GetDlgItemText(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD,ip,sizeof(ip)-1);
	ZeroMemory(&g_FastConnectSrv,sizeof(SERVER_INFO));
	DWORD dwPort;	
	if(ip[0]==0)
	{
		SetStatusText(ICO_WARNING,g_lang.GetString("InvalidIP"));
		
		//Try from serverlist if any server selected
		StartGame_ConnectToServer(false);
		return;
	}
	
	char destPort[10];
	size_t n = 0; //strcspn(ip,"01234567890"); //removed since 2.0 beta 18

	char *p=NULL;
	p = strchr(&ip[n],';');
	if(p!=NULL)
		p[0]=0;
	else
	{
		p = strchr(&ip[n],' ');
		if(p!=NULL)
			p[0]=0;		
	}
	char szCustomCmd[200];
	ZeroMemory(szCustomCmd,sizeof(szCustomCmd));
	if(p!=NULL)
		strcpy(szCustomCmd,&p[1]);


	strcpy(g_FastConnectSrv.szIPaddress,SplitIPandPORT(&ip[n],dwPort));

	g_FastConnectSrv.usPort = dwPort;
	//strcpy(g_FastConnectSrv.szPRIVATEPASS,password);
	g_FastConnectSrv.cGAMEINDEX = -1;
	g_FastConnectSrv.dwIndex = 999999;
	g_FastConnectSrv.dwIP = NetworkNameToIP(g_FastConnectSrv.szIPaddress,_itoa(dwPort,destPort,10));

	int iResult = CheckForDuplicateServer(&gm.GamesInfo[g_currentGameIdx],&g_FastConnectSrv);

	if(iResult!=-1) //did we get an exsisting server?
	{
		//then try to launch with exsisting server details... nice if password is set since before.
		SERVER_INFO *pSI = gm.GamesInfo[g_currentGameIdx].vSI.at(iResult);
		LaunchGame(pSI,&gm.GamesInfo[pSI->cGAMEINDEX],0,szCustomCmd);

	} else
	{
		//Let's do quick launch 
		LaunchGame(&g_FastConnectSrv,&gm.GamesInfo[g_currentGameIdx],0,szCustomCmd);
	}
}

GAME_INFO * Get_CurrentViewByServer(SERVER_INFO* pSrv)
{
	return &gm.GamesInfo[pSrv->cGAMEINDEX];	 
}


int FindFirstActiveGame()
{
	for(int i=0;i<gm.GamesInfo.size();i++)
		if(gm.GamesInfo[i].bActive)
			return i;

		int i = MessageBox(g_hWnd,g_lang.GetString("MessageNoGamesActive"),"Info",MB_YESNO|MB_ICONINFORMATION|MB_TOPMOST);
		if(i==IDYES)
			PostMessage(g_hWnd,WM_COMMAND,LOWORD(IDM_SETTINGS),0);

	return 0;
}

void ListView_SetHeaderSortImage(HWND listView, int columnIndex, BOOL isAscending)
{
    HWND header = ListView_GetHeader(listView);
    BOOL isCommonControlVersion6 = TRUE;  //hack to only support XP and Vista
    int columnCount = Header_GetItemCount(header);
    
	//isAscending=!isAscending;


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
	dbg_print("OnRestore");	

	ZeroMemory(g_szIPtoAdd,sizeof(g_szIPtoAdd));
	EnableButtons(TRUE);

	g_vSelectionSI.clear();
	SetInitialViewStates();
	
	SetDlgTrans(g_hWnd,AppCFG.g_cTransparancy);
	
	//TreeView_ReBuildList();

	g_iCurrentSelectedServer = -1;

	bm.UpdateList();
	
	Show_ToolbarButton(IDC_BUTTON_FIND,false);
	Show_ToolbarButton(IDC_DOWNLOAD, false);
	
	OnTabControlSelection();

	if(gm.GamesInfo.size()>0)
	{
		if(gm.GamesInfo[g_currentGameIdx].bActive)
		{
			SetCurrentActiveGame(g_currentGameIdx);
		}
	//	else
	//		SetCurrentActiveGame(FindFirstActiveGame());
	}
	SetStatusText(ICO_INFO,g_lang.GetString("Ready"));

}


void LVHeaderSL_AddDefaultColumns(BOOL bSkipLoad)
{
	g_LVHeaderSL->Reset();
	g_LVHeaderSL->DeleteAllColumns();
	g_LVHeaderSL->AddColumn(20,"ColumnAntiCheat", LVCF_WIDTH | LVCF_TEXT);
	g_LVHeaderSL->AddColumn(16,"ColumnPrivate", LVCF_WIDTH | LVCF_TEXT);
	g_LVHeaderSL->AddColumn(16,"ColumnRanked", LVCF_WIDTH | LVCF_TEXT);
	g_LVHeaderSL->AddColumn(220,"ColumnServerName", LVCF_WIDTH | LVCF_TEXT);
	g_LVHeaderSL->AddColumn(55,"ColumnVersion", LVCF_WIDTH | LVCF_TEXT);
	g_LVHeaderSL->AddColumn(55,"ColumnGameType", LVCF_WIDTH | LVCF_TEXT);
	g_LVHeaderSL->AddColumn(75,"ColumnMap", LVCF_WIDTH | LVCF_TEXT);
	g_LVHeaderSL->AddColumn(65,"ColumnMod", LVCF_WIDTH | LVCF_TEXT);
	g_LVHeaderSL->AddColumn(20,"ColumnBots", LVCF_WIDTH | LVCF_TEXT);
	g_LVHeaderSL->AddColumn(90,"ColumnPlayers", LVCF_WIDTH | LVCF_TEXT);

	int cx=115;
	if(AppCFG.bUseShortCountry)
		cx = 45;

	g_LVHeaderSL->AddColumn(cx,"ColumnCountry", LVCF_WIDTH | LVCF_TEXT);

	g_LVHeaderSL->AddColumn(45,"ColumnPing", LVCF_WIDTH | LVCF_TEXT);
	g_LVHeaderSL->AddColumn(150,"ColumnIP", LVCF_WIDTH | LVCF_TEXT);
	g_LVHeaderSL->AddColumn(80,"ColumnStatus", LVCF_WIDTH | LVCF_TEXT);
	
	g_LVHeaderSL->Update();
	if(bSkipLoad==FALSE)
		g_LVHeaderSL->Load();
	g_LVHeaderSL->UpdateColumnOrder();
}


void DUMP_COL_ORDER()
{
	int order[15];
	int iItems;
dbg_print(" ");	
	if((iItems = SendMessage(g_hwndListViewServerListHeader, HDM_GETITEMCOUNT, 0,0))!=-1)
	{
		ListView_GetColumnOrderArray(g_hwndListViewServer,iItems,order);
		for(int i=0; i<iItems;i++)
			dbg_printNoLF("%d  ",i);	
dbg_print("");
		for(int i=0; i<iItems;i++)
			dbg_printNoLF("%d, ",order[i]);	

	}
	dbg_print("\n");	
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

   ModifyMenu(hmenu,IDM_SETTINGS,MF_BYCOMMAND,IDM_SETTINGS,g_lang.GetString("MenuSettings"));
   ModifyMenu(hmenu,ID_SERVERLIST_PURGEPUBLICSERVLIST,MF_BYCOMMAND,ID_SERVERLIST_PURGEPUBLICSERVLIST,g_lang.GetString("MenuDeleteAllServers"));
   ModifyMenu(hmenu,IDM_EXIT,MF_BYCOMMAND,IDM_EXIT,g_lang.GetString("MenuExit"));
   ModifyMenu(hmenu,ID_BUDDY_ADDFROMPLAYERLIST,MF_BYCOMMAND,ID_BUDDY_ADDFROMPLAYERLIST,g_lang.GetString("MenuAddSelectedPlyToBuddylist"));
   ModifyMenu(hmenu,ID_BUDDY_ADD,MF_BYCOMMAND,ID_BUDDY_ADD,g_lang.GetString("MenuAddNewBuddy"));
   ModifyMenu(hmenu,ID_BUDDY_REMOVE,MF_BYCOMMAND,ID_BUDDY_REMOVE,g_lang.GetString("MenuRemoveBuddy"));

   ModifyMenu(hmenu,ID_VIEW_BUDDYLIST,MF_BYCOMMAND,ID_VIEW_BUDDYLIST,g_lang.GetString("MenuViewBuddy"));
   ModifyMenu(hmenu,ID_VIEW_MAPPREVIEW,MF_BYCOMMAND,ID_VIEW_MAPPREVIEW,g_lang.GetString("MenuViewMap"));
   ModifyMenu(hmenu,ID_VIEW_PLAYERLIST,MF_BYCOMMAND,ID_VIEW_PLAYERLIST ,g_lang.GetString("MenuViewTabs"));

}

BOOL g_bOnCreate = FALSE;

void OnCreate(HWND hwnd, HINSTANCE hInst)
{
	g_bOnCreate = TRUE;
	LV_COLUMN   lvColumn;
	DWORD dwExStyle = 0;	
	ZeroMemory(&lvColumn,sizeof(LV_COLUMN));
	ZeroMemory(&g_CopyTI,sizeof(_MYTREEITEM));
	ZeroMemory(&g_PasteAtTI,sizeof(_MYTREEITEM));
	
	Q3_SetCallbacks(UpdateServerItem, Buddy_CheckForBuddies, NULL);
	Q4_SetCallbacks(UpdateServerItem, Buddy_CheckForBuddies, NULL);
	STEAM_SetCallbacks(UpdateServerItem, Buddy_CheckForBuddies,NULL);

	strcpy(gm.g_szMapName,"unknownmap.png");

	g_CurrentSRV = NULL;
	g_bRedrawServerListThread =  FALSE;
	g_bRunningQueryServerList = false;
	g_bCancel = false;
	//ZeroMemory(&g_CurrentSelServer,sizeof(SERVER_INFO));

	RECT rc;
	GetClientRect(hwnd,&rc);

	WNDCONT[WIN_MAIN].idx = WIN_MAIN;
	WNDCONT[WIN_MAIN].bShow = TRUE;
	WNDCONT[WIN_MAIN].hWnd = hwnd;

	WNDCONT[WIN_MAPPREVIEW].idx = WIN_MAPPREVIEW;


	g_hwndMainTreeCtrl = CreateWindowEx(WS_EX_CLIENTEDGE  ,  WC_TREEVIEW , NULL,

							WS_VISIBLE |WS_CHILDWINDOW|  WS_TABSTOP |  TVS_HASBUTTONS | TVS_EDITLABELS| TVS_LINESATROOT | TVS_HASLINES   , 
							0,TOOLBAR_Y_OFFSET,50, 50, 
							hwnd, (HMENU) IDC_MAINTREE, hInst, NULL);


	WNDCONT[WIN_MAINTREEVIEW].idx = WIN_MAINTREEVIEW;
	WNDCONT[WIN_MAINTREEVIEW].hWnd = g_hwndMainTreeCtrl;

	g_hwndListViewServer = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW , NULL,
							LVS_OWNERDATA | LVS_REPORT | WS_VISIBLE | WS_CHILD | WS_TABSTOP |WS_CLIPCHILDREN | WS_CLIPSIBLINGS ,
							100+BORDER_SIZE,TOOLBAR_Y_OFFSET,200, 100, 
							hwnd, (HMENU) IDC_LIST_SERVER, hInst, NULL);

	dwExStyle = ListView_GetExtendedListViewStyle(g_hwndListViewServer);
	dwExStyle |= LVS_EX_FULLROWSELECT;
	dwExStyle |= LVS_EX_SUBITEMIMAGES; // | LVS_EX_LABELTIP  ;
	dwExStyle |= LVS_EX_DOUBLEBUFFER;
	ListView_SetExtendedListViewStyle(g_hwndListViewServer,dwExStyle);

	WNDCONT[WIN_SERVERLIST].idx = WIN_SERVERLIST;
	WNDCONT[WIN_SERVERLIST].hWnd = g_hwndListViewServer;
	
	g_hwndListBuddy	 = CreateWindowEx(LVS_EX_FULLROWSELECT | WS_EX_CLIENTEDGE , WC_LISTVIEW , NULL,
							LVS_REPORT | WS_VISIBLE |WS_CHILD | WS_TABSTOP ,
							0,0+TOOLBAR_Y_OFFSET+BORDER_SIZE,50, 50, 
							hwnd, (HMENU) IDC_LIST_BUDDY, hInst, NULL);		

	WNDCONT[WIN_BUDDYLIST].idx = WIN_BUDDYLIST;
	WNDCONT[WIN_BUDDYLIST].hWnd = g_hwndListBuddy;


	g_hwndTabControl = CreateWindowEx(0 , WC_TABCONTROL  , NULL,
							WS_VISIBLE |WS_CHILD | WS_TABSTOP | WS_CLIPSIBLINGS,
							100+BORDER_SIZE,200+TOOLBAR_Y_OFFSET+BORDER_SIZE,100, 200, 
							hwnd, (HMENU) IDC_TAB1, hInst, NULL);

	WNDCONT[WIN_TABCONTROL].idx = WIN_TABCONTROL;
	WNDCONT[WIN_TABCONTROL].hWnd = g_hwndTabControl;

	g_hwndListViewPlayers = CreateWindowEx(LVS_EX_SUBITEMIMAGES|LVS_EX_FULLROWSELECT|WS_EX_CLIENTEDGE , WC_LISTVIEW , NULL,
							LVS_OWNERDATA|LVS_REPORT|WS_VISIBLE |WS_CHILD | WS_TABSTOP |WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
							100+BORDER_SIZE,200+TOOLBAR_Y_OFFSET+BORDER_SIZE,100, 200, 
							hwnd, (HMENU) IDC_LIST_PLAYERS, hInst, NULL);

	WNDCONT[WIN_PLAYERS].idx = WIN_PLAYERS;
	WNDCONT[WIN_PLAYERS].hWnd = g_hwndListViewPlayers;
	
	g_hwndListViewVars = CreateWindowEx(LVS_EX_FULLROWSELECT|WS_EX_CLIENTEDGE , WC_LISTVIEW , NULL,
							LVS_REPORT|WS_VISIBLE |WS_CHILD | WS_TABSTOP |WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
							200+BORDER_SIZE*2,200+TOOLBAR_Y_OFFSET+BORDER_SIZE,100-BORDER_SIZE, 200, 
							hwnd, (HMENU) IDC_LIST2, hInst, NULL);

	WNDCONT[WIN_RULES].idx = WIN_RULES;
	WNDCONT[WIN_RULES].hWnd = g_hwndListViewVars;

	int cyVScroll = GetSystemMetrics(SM_CYVSCROLL); 		

	g_hwndLogger = CreateWindowEx(0 , WC_EDIT , NULL, 
							 WS_VSCROLL|WS_BORDER | WS_VISIBLE |  WS_CHILD | ES_LEFT | ES_MULTILINE  | ES_AUTOHSCROLL |  ES_AUTOVSCROLL   ,
								100+BORDER_SIZE,200+TOOLBAR_Y_OFFSET+BORDER_SIZE,100, 200, 
							hwnd, (HMENU) IDC_EDIT_LOGGER, hInst, NULL);

	log.hwndLogger = g_hwndLogger;
	WNDCONT[WIN_LOGGER].idx = WIN_LOGGER;
	WNDCONT[WIN_LOGGER].hWnd = g_hwndLogger;


	g_hwndStatus = CreateWindowEx(0 , WC_EDIT , NULL, 
							WS_VISIBLE |  WS_CHILD | ES_AUTOHSCROLL | ES_READONLY | ES_NOHIDESEL ,
							0,rc.bottom-cyVScroll,rc.right/2, cyVScroll, 
							hwnd, (HMENU) IDC_EDIT_STATUS, hInst, NULL);

	WNDCONT[WIN_STATUS].idx = WIN_STATUS;
	WNDCONT[WIN_STATUS].hWnd = g_hwndStatus;


	g_hwndProgressBar = CreateWindowEx(0, PROGRESS_CLASS,
			 (LPSTR) NULL, WS_CHILD | WS_VISIBLE|PBS_SMOOTH,
			 rc.right/2, rc.bottom-cyVScroll, rc.right/2, cyVScroll, 
			 hwnd, (HMENU) 0, hInst, NULL); 

	WNDCONT[WIN_PROGRESSBAR].idx = WIN_PROGRESSBAR;
	WNDCONT[WIN_PROGRESSBAR].hWnd = g_hwndProgressBar;

	WNDCONT[WIN_RCON].idx = WIN_RCON;
	WNDCONT[WIN_RCON].hWnd = InitRCON(hwnd);
	WNDCONT[WIN_PING].idx = WIN_PING;
	WNDCONT[WIN_PING].hWnd = InitSTATS(hwnd);

	tvmgr.hwndTreeCtrl = g_hwndMainTreeCtrl;

	if(g_bWinSizesLoaded)
	{
		Load_WindowSizes();		
		if(g_bNormalWindowed)
		{
			Update_WindowSizes(0);
			PostMessage(g_hWnd,WM_SIZE,0,0);
			InvalidateRect(g_hWnd,NULL,TRUE);		
		}
	}
	else
	{
		Initialize_WindowSizes();
		CenterWindow(g_hWnd);
	}
	SetImageList();

	TCITEM tci;
	ZeroMemory(&tci,sizeof(tci));
	tci.iImage = 0;
	tci.mask =  TCIF_IMAGE|TCIF_TEXT;
	tci.pszText = (LPSTR)g_lang.GetString("TabPlayers");
	TabCtrl_InsertItem(g_hwndTabControl,0,&tci);
	tci.iImage = 27;
	tci.pszText = (LPSTR)g_lang.GetString("TabRules");
	TabCtrl_InsertItem(g_hwndTabControl,1,&tci);
	tci.iImage = 22;
	tci.pszText = (LPSTR)g_lang.GetString("TabRCON");
	TabCtrl_InsertItem(g_hwndTabControl,2,&tci);
	tci.iImage = 26;
	tci.pszText = (LPSTR)g_lang.GetString("TabNetwork");
	TabCtrl_InsertItem(g_hwndTabControl,3,&tci);
	tci.iImage = 23;
	tci.pszText =  (LPSTR)g_lang.GetString("TabLogger");
	TabCtrl_InsertItem(g_hwndTabControl,4,&tci);



	SetWindowTheme(g_hwndMainTreeCtrl, L"explorer", 0);

	
	dwExStyle = TreeView_GetExtendedStyle(g_hwndMainTreeCtrl);
//	dwExStyle |= TVS_EX_DOUBLEBUFFER;
	TreeView_SetExtendedStyle(g_hwndMainTreeCtrl, WS_EX_LEFT,dwExStyle);
	

	dwExStyle = ListView_GetExtendedListViewStyle(g_hwndListViewPlayers);
	dwExStyle |= LVS_EX_FULLROWSELECT;
	dwExStyle |= LVS_EX_SUBITEMIMAGES;
	dwExStyle |= LVS_EX_DOUBLEBUFFER;	
	ListView_SetExtendedListViewStyle(g_hwndListViewPlayers,dwExStyle);


	dwExStyle = ListView_GetExtendedListViewStyle(g_hwndListViewVars);
	dwExStyle |= LVS_EX_FULLROWSELECT ;
	dwExStyle |= LVS_EX_DOUBLEBUFFER;
	ListView_SetExtendedListViewStyle(g_hwndListViewVars,dwExStyle);
	
	dwExStyle = ListView_GetExtendedListViewStyle(g_hwndListBuddy);
	dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES | LVS_EX_LABELTIP ;
	dwExStyle |= LVS_EX_DOUBLEBUFFER;
	ListView_SetExtendedListViewStyle(g_hwndListBuddy,dwExStyle);

	//initialize the columns
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM ;
	lvColumn.fmt = LVCFMT_LEFT;


	char szPlayerlistColumnString[7][20] = {TEXT("ColumnNo"),TEXT("ColumnTeam"),TEXT("ColumnName"), TEXT("ColumnFrag"), TEXT("ColumnPing"),TEXT("ColumnTime"),TEXT("ColumnServerName")};
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
		lvColumn.pszText = (LPSTR)g_lang.GetString(szPlayerlistColumnString[dd]);
		ListView_InsertColumn(g_hwndListViewPlayers, dd, &lvColumn);
	}
	lvColumn.cx = 150;
	lvColumn.pszText = (LPSTR)g_lang.GetString("ColumnRule");
	ListView_InsertColumn(g_hwndListViewVars, 0, &lvColumn);
	lvColumn.cx = 175;
	lvColumn.pszText = (LPSTR)g_lang.GetString("ColumnValue");
	ListView_InsertColumn(g_hwndListViewVars, 1, &lvColumn);
	
	lvColumn.cx = 120;
	lvColumn.pszText = (LPSTR)g_lang.GetString("ColumnBuddy");
	ListView_InsertColumn(g_hwndListBuddy, 0, &lvColumn);
	lvColumn.cx = 120;
	lvColumn.pszText = (LPSTR)g_lang.GetString("ColumnServerName");
	ListView_InsertColumn(g_hwndListBuddy, 1, &lvColumn);
	lvColumn.cx = 80;
	lvColumn.pszText = (LPSTR)g_lang.GetString("ColumnIP");
	ListView_InsertColumn(g_hwndListBuddy, 2, &lvColumn);

	SendMessage(hwnd,WM_SETICON,ICON_SMALL, (LPARAM)g_hAppIcon);
	
	SetWindowText(hwnd,szDialogTitle);

	g_hwndRibbonBar = TOOLBAR_CreateRebar(hwnd);


	// Subclassing    
	g_wpOrigListViewServerProc = (LONG_PTR) SetWindowLongPtr(g_hwndListViewServer, GWLP_WNDPROC, (LONG_PTR) ListView_SL_SubclassProc); 
	g_wpOrigTreeViewProc = (LONG_PTR) SetWindowLongPtr(g_hwndMainTreeCtrl, GWLP_WNDPROC, (LONG_PTR) TreeView_SubclassProc); 
    g_wpOrigListBuddyProc = (LONG_PTR) SetWindowLongPtr(g_hwndListBuddy, GWLP_WNDPROC, (LONG_PTR) Buddy_ListViewSubclassProc); 
	g_wpOrigListViewPlayersProc = (LONG_PTR) SetWindowLongPtr(g_hwndListViewPlayers, GWLP_WNDPROC, (LONG_PTR) ListViewPlayerSubclassProc); 
	g_wpOrigLVRulesProc =  (LONG_PTR) SetWindowLongPtr(g_hwndListViewVars, GWLP_WNDPROC, (LONG_PTR) ListView_Rules_SubclassProc); 
	
	g_hwndListViewServerListHeader = ListView_GetHeader(g_hwndListViewServer);
	g_wpOrigSLHeaderProc = (LONG_PTR) SetWindowLongPtrW(g_hwndListViewServerListHeader, GWLP_WNDPROC, (LONG_PTR) LV_SL_HeaderSubclassProc); 

//	ListView_SetUnicodeFormat(g_hwndListViewServer,0);
	Header_SetUnicodeFormat(g_hwndListViewServerListHeader,TRUE);

	Load_CountryFlags();

	g_hf = MyCreateFont(hwnd);
	g_hf2 = MyCreateFont(hwnd,14,FW_BOLD,"Verdana");//Courier New");
	g_hfScriptEditor = MyCreateFont(hwnd,14,FW_BOLD,"Courier New");
	
	ChangeMainMenuLanguage(hwnd);
	ChangeFont(hwnd,g_hf);

	if(g_bDoFirstTimeCheckForUpdate)
		PostMessage(g_hWnd,WM_COMMAND,IDM_UPDATE,1);

//-----	
	g_LVHeaderSL = new CListViewHeader(g_hwndListViewServer,"SLLVHeader",g_lang);
	LVHeaderSL_AddDefaultColumns(FALSE);

	SendMessage(g_hwndProgressBar, PBM_SETPOS, (WPARAM) 0, 0); 	

	ZeroMemory(g_szIPtoAdd,sizeof(g_szIPtoAdd));
	EnableButtons(TRUE);

	SetInitialViewStates();
	tvmgr.BuildList(g_hWnd,EXE_PATH,USER_SAVE_PATH);

	if(gm.GamesInfo.size()>0)
		currCV = &gm.GamesInfo[0];
	else
		currCV = NULL;

	LoadAllServerList();
	g_iCurrentSelectedServer = -1;
	bm.Load();
	bm.UpdateList();
	g_bOnCreate = FALSE;
}


void OnClose()
{
	SendMessage(g_hwndMainSTATS,WM_STOP_PING,0,0);
	if(hTimerMonitor!=NULL)
		KillTimer(g_hWnd,IDT_MONITOR_QUERY_SERVERS);
	hTimerMonitor = NULL;
	dbg_print("--- Closing down and cleaning up!\n");

	Shell_NotifyIcon(NIM_DELETE,&structNID);

	g_CurrentSRV=NULL;
	g_bCancel = true;

	SCANNER_bCloseApp = TRUE;
	SaveAll(0xdeadface);

	
	ImageList_Destroy(g_hImageListStates);
	g_hImageListStates= NULL;


	ImageList_Destroy(g_hImageListIcons);
	g_hImageListIcons= NULL;

	ImageList_Destroy(g_hILFlags);
	g_hILFlags = NULL;
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
	DeleteObject(g_hfScriptEditor);
	g_hfScriptEditor = NULL;

}

void OnDestroy(HWND hWnd)
{
	//Clean up and restore subclasses. 
	dbg_print("OnDestroy");
	if(g_hwndListViewServer==NULL)
		return;

	SetWindowLongPtr(g_hwndListViewServer, GWLP_WNDPROC, (LONG_PTR) g_wpOrigListViewServerProc); 
	SetWindowLongPtr(g_hwndListBuddy, GWLP_WNDPROC, (LONG_PTR) g_wpOrigListBuddyProc); 
	SetWindowLongPtr(g_hwndListViewPlayers, GWLP_WNDPROC, (LONG_PTR) g_wpOrigListViewPlayersProc); 
	SetWindowLongPtr(g_hwndMainTreeCtrl, GWLP_WNDPROC, (LONG_PTR) g_wpOrigTreeViewProc); 
	SetWindowLongPtr(g_hwndComboEdit, GWLP_WNDPROC, (LONG_PTR) g_wpOrigCBSearchProc); 
	SetWindowLongPtr(g_hwndListViewServerListHeader, GWLP_WNDPROC, (LONG_PTR) g_wpOrigSLHeaderProc); 
	SetWindowLongPtr(g_hwndListViewVars, GWLP_WNDPROC, (LONG_PTR) g_wpOrigLVRulesProc); 
		
	g_wpOrigLVRulesProc = NULL;
	g_wpOrigSLHeaderProc = NULL;
	g_wpOrigCBSearchProc = NULL;
	g_wpOrigTreeViewProc = NULL;
	g_wpOrigListViewPlayersProc = NULL;
	g_wpOrigListBuddyProc = NULL;
	g_wpOrigListViewServerProc = NULL;

	if(g_wpOrigFilterEditorProc!=NULL)
		MessageBox(NULL,"Error g_wpOrigFilterEditorProc not NULL",NULL,NULL);

	DestroyWindow(g_hwndProgressBar);
	DestroyWindow(g_hwndMainTreeCtrl);
	DestroyWindow(g_hwndListViewVars);
	DestroyWindow(g_hwndListViewPlayers);
	DestroyWindow(g_hwndListBuddy);
	DestroyWindow(g_hwndListViewServer);
	DestroyWindow(g_hwndStatus);			
	DestroyWindow(g_hwndTabControl);
	DestroyWindow(g_hwndLogger);	

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

char * TrimString(char *szIn)
{	
	if(szIn==NULL)
		return NULL;
	char *p=NULL;
	char *s=NULL;
	s = strchr(szIn,'"');	
	p = strrchr(szIn,'"');	
	if(s!=p)
	{
		if(p!=NULL)
			p[0] = 0;
		if(s!=NULL)
			s++;

	}else
	{
		if(s!=NULL)
			s++;
		else
			s = szIn;

	}
	return s;
}

char * ReplaceIllegalchars(char *szIn)
{
	char *p;
	if(szIn==NULL)
		return NULL;
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

void ImportServerList(char* szFilename, char*szPath,GAME_INFO *pGI, DWORD dwImportFlag=0)
{
	char szBuffer[100];
/*	if(pGI->hTI!=NULL)
	{
		sprintf(szBuffer,"%s (Importing...)",pGI->szGAME_NAME);
		TreeView_SetItemText(pGI->hTI,szBuffer);
		TreeView_SetItemState(g_hwndMainTreeCtrl,pGI->hTI,TVIS_BOLD ,TVIS_BOLD);		
	}
*/
	OnStopScanning();
	Sleep(6000);
	char seps[]   = ",\t\n";
	SetCurrentDirectory(szPath);
	
	FILE *fp=NULL;
	dbg_print("Importing serverlist %s ",szFilename);
	fopen_s(&fp,szFilename, "rb");

	char  *next_token1;

	SERVER_INFO	 srv;
	char *szGameType = NULL;
	char *szGameName = NULL;
	char *szIP = NULL;
	char *szByte = NULL;
	char *szPrivatePassword = NULL;
	char *szRCONPassword= NULL;
	char *szFavorite = NULL;
	DWORD idx=pGI->dwTotalServers;
	BOOL bSetIdx=FALSE;
	char buffer[1024];
	int hash;
	if(fp!=NULL)
	{
		while( !feof( fp ) )
		{
			memset(&buffer,0,sizeof(buffer));
			for(int i=0; i<sizeof(buffer);i++)
			{
				
				if(fread(&buffer[i], 1, 1, fp)!=0)
				{


					if(buffer[i]==10)
					{
						szGameName = NULL;
						szIP = NULL;
						szByte = NULL;
						szPrivatePassword = NULL;
						szRCONPassword= NULL;
						szFavorite = NULL;
						next_token1 = NULL;
						memset(&srv,0,sizeof(SERVER_INFO));
						if(dwImportFlag == EXPORT_GAMESCANNER_SL_FILE)
						{
							szGameType= strtok_s( buffer, seps, &next_token1);
							if(szGameType!=NULL)
								srv.cGAMEINDEX =  (char)atoi(szGameType); 
							pGI = &gm.GamesInfo[srv.cGAMEINDEX];
							if(bSetIdx==FALSE)
							{
								bSetIdx = TRUE;
								idx=pGI->dwTotalServers+1;
							}
							szIP = strtok_s( NULL, seps, &next_token1);
						} else
						{
							srv.cGAMEINDEX =  pGI->cGAMEINDEX;
							szIP = strtok_s( buffer, seps, &next_token1);
						}
												
						if(szIP!=NULL)
						{							
							DWORD dwPort=0;					
							SplitIPandPORT(szIP,dwPort);
							srv.usPort = (short)dwPort;
							char destPort[10];
							if(szIP!=NULL)
							{
								strcpy(srv.szIPaddress,szIP);
								srv.dwIP = NetworkNameToIP(szIP,_itoa(srv.usPort,destPort,10));
							}
						}
						
						if(dwImportFlag == EXPORT_GAMESCANNER_SL_FILE)
						{

							szGameName = strtok_s( NULL, seps, &next_token1);
							if(szGameName!=NULL)
								strcpy(srv.szServerName,TrimString(szGameName));

							szPrivatePassword = strtok_s( NULL, seps, &next_token1);
							if(szPrivatePassword!=NULL)
								strcpy(srv.szPRIVATEPASS,TrimString(szPrivatePassword));

							szRCONPassword = strtok_s( NULL, seps, &next_token1);
							if(szRCONPassword!=NULL)
								strcpy(srv.szRCONPASS,TrimString(szRCONPassword));
							
							if(next_token1!=NULL)
							{
								szFavorite = strtok_s( NULL, seps, &next_token1);
								if(szFavorite!=NULL)
									srv.cFavorite = atoi(szFavorite);
							}

						}
						
						srv.usQueryPort = srv.usPort;
						hash = srv.dwIP + srv.usPort;

						if(UTILZ_checkforduplicates(pGI,  hash,srv.dwIP, srv.usPort)==FALSE)
						{
							srv.dwIndex = idx++;

							srv.pPlayerData = NULL;
							srv.pServerRules = NULL;					
							srv.bUpdated = 0;
							strcpy(srv.szShortCountryName,"zz");
							SERVER_INFO *pNewServer = (SERVER_INFO *)calloc(1,sizeof(SERVER_INFO));
							memcpy(pNewServer,&srv,sizeof(SERVER_INFO));
							pGI->shash.insert(Int_Pair(hash,srv.dwIndex));
							pGI->vSI.push_back(pNewServer);			
							pGI->dwTotalServers++;
						}
				
						i = sizeof(buffer)+1;
					}  //endif
				}
			}  //if
		}
		fclose(fp);
	}
	pGI->lastScanTimeStamp = 0;
	if(pGI->hTI!=NULL)
	{
		sprintf(szBuffer,"%s (%d)",pGI->szGAME_NAME,pGI->dwTotalServers);
		tvmgr.SetItemText(pGI->hTI,szBuffer);
		TreeView_SetItemState(g_hwndMainTreeCtrl,pGI->hTI,TVIS_BOLD ,TVIS_BOLD);		
	}
}



void ExportServerList(char* szFilename, char*szPath,GAME_INFO *pGI, DWORD dwExportFlag=0)
{
	SetCurrentDirectory(szPath);

	FILE *fp2=NULL;
	char szFilename2[260];	
	sprintf(szFilename2,"%s",szFilename);
	fopen_s(&fp2,szFilename2, "wb");
	char szBuffer[1024];
	if(fp2!=NULL)
	{
			
		vSRV_INF::iterator  iLst;

		if(pGI->vSI.size()>0)
		{
			bool bWrite=false;
			for ( iLst = pGI->vSI.begin( ); iLst != pGI->vSI.end( ); iLst++ )
			{
		
				SERVER_INFO *pSI = *iLst;
				bWrite=true;
				if(EXPORT_FLAG_FAVORITES & dwExportFlag)
				{
					if (pSI->cFavorite)
						bWrite=true;
					else
						bWrite=false;
				}
				
				if(bWrite)
				{					
					char sep=',';
					if((EXPORT_PLAIN_TEXT & dwExportFlag)==0)
					{
						sprintf_s(szBuffer,sizeof(szBuffer),"%d%c",pSI->cGAMEINDEX,sep);
						fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);	
					}

					sprintf_s(szBuffer,sizeof(szBuffer),"%s:%d",pSI->szIPaddress,pSI->usPort);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);	

					
					if((EXPORT_PLAIN_TEXT & dwExportFlag)==0)
					{
						sprintf_s(szBuffer,sizeof(szBuffer),"%c\"%s\"",sep,ReplaceIllegalchars(pSI->szServerName));
						fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);	

					//	if((EXPORT_FLAG_PRIVATE_PASSWORD & dwExportFlag))
						{
							sprintf_s(szBuffer,sizeof(szBuffer),"%c\"%s\"",sep,pSI->szPRIVATEPASS);
							fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);
						}
					//	if((EXPORT_FLAG_RCON_PASSWORD & dwExportFlag))
						{

							sprintf_s(szBuffer,sizeof(szBuffer),"%c\"%s\"",sep,pSI->szRCONPASS);
							fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);
						}
							sprintf_s(szBuffer,sizeof(szBuffer),"%c%d",sep,pSI->cFavorite);
							fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);

						

					}
					sprintf_s(szBuffer,sizeof(szBuffer),"\n");
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);
			

				}
			}
		}
		fclose(fp2);
	}


}



//TODO rewrite file structure for easier future changes/additional info
void SaveServerList(GAME_INFO *pGI, DWORD dwExportFlag=0)
{
	SetCurrentDirectory(USER_SAVE_PATH);

	FILE *fp2=NULL;
	char szFilename2[260];
	sprintf(szFilename2,"%s.gs",pGI->szFilename);
	fopen_s(&fp2,szFilename2, "wb");
	char szBuffer[1024];
	if(fp2!=NULL)
	{
			
		vSRV_INF::iterator  iLst;

		if(pGI->vSI.size()>0)
		{
			bool bWrite=false;
			for ( iLst = pGI->vSI.begin( ); iLst != pGI->vSI.end( ); iLst++ )
			{
		
				SERVER_INFO *pSI = *iLst;
				bWrite=false;
				if (pSI->cFavorite)
					bWrite=true;
				else if (pSI->cPurge<SERVER_PURGE_COUNTER )
					bWrite=true;
				
				if(bWrite)
				{
					char sep=9;
					sprintf_s(szBuffer,sizeof(szBuffer),"%d%c\"%s\"%c%d%c\"%s\"",pSI->cGAMEINDEX,sep,pSI->szIPaddress,sep,pSI->usPort,sep,pSI->szShortCountryName);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);		
					sprintf_s(szBuffer,sizeof(szBuffer),"%c%d%c\"%s\"%c\"%s\"",sep,pSI->dwIP,sep,pSI->szPRIVATEPASS,sep,pSI->szRCONPASS);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);	
					sprintf_s(szBuffer,sizeof(szBuffer),"%c%d%c%d%c%d",sep,pSI->nMaxPlayers,sep,pSI->nPlayers,sep,pSI->nPrivateClients);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);	
					sprintf_s(szBuffer,sizeof(szBuffer),"%c%d%c%d%c%d%c%d",sep,pSI->bPrivate,sep,pSI->bPunkbuster,sep,0,sep,pSI->cRanked);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);	
					sprintf_s(szBuffer,sizeof(szBuffer),"%c%d%c%d%c%d",sep,pSI->cBots,sep,pSI->cFavorite,sep,pSI->cHistory);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);	
					sprintf_s(szBuffer,sizeof(szBuffer),"%c%d%c%d%c%d%c%d%c",sep,pSI->cPurge,sep,pSI->dwPing,sep,pSI->usQueryPort,sep,0,0x0A);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);
					SERVER_RULES *pszRules=NULL;
					pszRules = pSI->pServerRules;
					while(pszRules!=NULL)
					{
						sprintf_s(szBuffer,sizeof(szBuffer),"\"%s\"%c\"%s\"%c",ReplaceIllegalchars(pszRules->name),sep,ReplaceIllegalchars(pszRules->value),sep);
						fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);
						pszRules = pszRules->pNext;
					}
					sprintf_s(szBuffer,sizeof(szBuffer),"%c",0x0A);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);
					PLAYERDATA *pszPly=NULL;
					pszPly = pSI->pPlayerData;
					char sztmp[2];
					sztmp[0]=0;
					sztmp[1]=0;
					while(pszPly!=NULL)
					{
						sprintf_s(szBuffer,sizeof(szBuffer),"%d%c%d%c%d%c%d%c%d",pszPly->iPlayer,sep,pszPly->rate,sep,pszPly->ping,sep,pszPly->isBot,sep,pszPly->time);
						fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);
						char *pTag = pszPly->szClanTag;
						char *pTeam = pszPly->szTeam;
						
						if(pTag==NULL)
							pTag=sztmp;
						if(pTeam==NULL)
							pTeam=sztmp;

						sprintf_s(szBuffer,sizeof(szBuffer),"%c\"%s\"%c\"%s\"%c\"%s\"%c",sep,ReplaceIllegalchars(pszPly->szPlayerName),sep,pTag,sep,pTeam,sep);
						fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);
						pszPly = pszPly->pNext;
					}
					sprintf_s(szBuffer,sizeof(szBuffer),"%c",0x0A);
					fwrite((const void*)&szBuffer, strlen(szBuffer), 1, fp2);
				}
			}
		}
		fclose(fp2);
	}


}
/*
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
		tvmgr.SetItemText(pGI->hTI,szBuffer);
		TreeView_SetItemState(g_hwndMainTreeCtrl,pGI->hTI,TVIS_BOLD ,TVIS_BOLD);		
	}

	char seps[]   = "\t\n";
	
	FILE *fp=NULL;
	char szFilename2[260];
	sprintf(szFilename2,"%s.csv",pGI->szFilename);
	dbg_print("Loading serverlist %s ",szFilename2);
	fopen_s(&fp,szFilename2, "rb");

	char  *next_token1;

	SERVER_INFO	 *srv;
	
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
	char *szMode = NULL;
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
						srv = (SERVER_INFO*) calloc(1,sizeof(SERVER_INFO));
						//memset(&srv,0,sizeof(SERVER_INFO));
						szGameType= strtok_s( buffer, seps, &next_token1);
						if(szGameType!=NULL)
							srv->cGAMEINDEX =  pGI->cGAMEINDEX; //(char)atoi(szGameType); 
						
						szGameName = strtok_s( NULL, seps, &next_token1);
				//		if(szGameName!=NULL)
				//			strcpy(srv->szServerName,TrimString(szGameName));

						szIP = strtok_s( NULL, seps, &next_token1);
						if(szIP!=NULL)
							strcpy(srv->szIPaddress,TrimString(szIP));
						
						szPort = strtok_s( NULL, seps, &next_token1);
						if(szPort!=NULL)
							srv->usPort = (DWORD)atol(szPort); 

						szShortCountryName = strtok_s( NULL, seps, &next_token1);
						if(szShortCountryName!=NULL)
							strcpy(srv->szShortCountryName,TrimString(szShortCountryName));
						
						szPrivatePassword = strtok_s( NULL, seps, &next_token1);
						if(szPrivatePassword!=NULL)
							strcpy(srv->szPRIVATEPASS,TrimString(szPrivatePassword));

						szRCONPassword = strtok_s( NULL, seps, &next_token1);
						if(szRCONPassword!=NULL)
							strcpy(srv->szRCONPASS,TrimString(szRCONPassword));
						
						szVersion = strtok_s( NULL, seps, &next_token1);
				//		if(szVersion!=NULL)
					//		strcpy(srv->szVersion,TrimString(szVersion));
						
						szMod = strtok_s( NULL, seps, &next_token1);
				//		if(szMod!=NULL)
				//			strcpy(srv->szMod,TrimString(szMod));
						
						szMap = strtok_s( NULL, seps, &next_token1);
					//	if(szMap!=NULL)
					//		strcpy(srv->szMap,TrimString(szMap));
						
						szMaxPlayers = strtok_s( NULL, seps, &next_token1);
						if(szMaxPlayers!=NULL)
							srv->nMaxPlayers = atoi(szMaxPlayers); 

						szCurrentNumberOfPlayers = strtok_s( NULL, seps, &next_token1);
						if(szCurrentNumberOfPlayers!=NULL)
							srv->nPlayers = atoi(szCurrentNumberOfPlayers); 

						szPrivateClients = strtok_s( NULL, seps, &next_token1);
						if(szPrivateClients!=NULL)
							srv->nPrivateClients = atoi(szPrivateClients); 
						
						szPrivate = strtok_s( NULL, seps, &next_token1);
						if(szPrivate!=NULL)
							srv->bPrivate = (char)atoi(szPrivate); 
						
						szByte = strtok_s( NULL, seps, &next_token1);  //Punkbuster or VAC
						if(szByte!=NULL)
							srv->bPunkbuster = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  //Dedicated
	//					if(szByte!=NULL)
	//						srv->bDedicated = (BOOL)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);   //Ranked
						if(szByte!=NULL)
							srv->cRanked = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);   
						if(szByte!=NULL)
							srv->cBots = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);   
						if(szByte!=NULL)
							srv->cFavorite = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);   
						if(szByte!=NULL)
							srv->cHistory = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  
						//if(szByte!=NULL)
						//	srv->cPure = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  
					//	if(szByte!=NULL)
					//		srv->dwGameType = (WORD)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  
//						if(szByte!=NULL)
//							srv->dwMap = (char)atoi(szByte); 
						
						szByte = strtok_s( NULL, seps, &next_token1);  
						if(szByte!=NULL)
							srv->cPurge = (char)atoi(szByte); 
					
						szByte = strtok_s( NULL, seps, &next_token1);  
						if(szByte!=NULL)
							srv->dwPing = (DWORD)atol(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  
						//if(szByte!=NULL)
						//	srv->dwMod = (WORD)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1); 
						if(szByte!=NULL)
							srv->dwIP = (DWORD)atol(szByte); 

						szCountry = strtok_s( NULL, seps, &next_token1);  //not used anymore
						//if(szCountry!=NULL)
						//	strcpy(srv->szCountry,TrimString(szCountry));

						szByte = strtok_s( NULL, seps, &next_token1);  

						if(szByte!=NULL)
						{
							//srv->dwVersion = (char)atol(szByte); 
							szByte = strtok_s( NULL, seps, &next_token1);  
							if(szByte!=NULL)
							{
								//srv->bTV = (char)atol(szByte); 
								szByte = strtok_s( NULL, seps, &next_token1);  
								if(szByte!=NULL)
								{
									srv->usQueryPort = (unsigned short)atol(szByte); 
									szByte = strtok_s( NULL, seps, &next_token1);
									if(szByte!=NULL)
									{
									//	strcpy(srv->szMode,TrimString(szByte));									
										szByte = strtok_s( NULL, seps, &next_token1);  
										//if(szByte!=NULL)
										//	srv->dwMode = (char)atol(szByte); 	
									}
								}								
							}
						}				

						srv->dwIndex = idx++;

						srv->pPlayerData = NULL;
						srv->pServerRules = NULL;					
						srv->bUpdated = 0;
						int hash = srv->dwIP + srv->usPort;
						pGI->shash.insert(Int_Pair(hash,srv->dwIndex));
						pGI->vSI.push_back(srv);			
						pGI->dwTotalServers++;
			
						i = sizeof(buffer)+1;
					}  //endif
				}
			}  //if
		}
		fclose(fp);
	}
	pGI->lastScanTimeStamp = 0;
	if(pGI->hTI!=NULL)
	{
		sprintf(szBuffer,"%s (%d)",pGI->szGAME_NAME,pGI->dwTotalServers);
		tvmgr.SetItemText(pGI->hTI,szBuffer);
		TreeView_SetItemState(g_hwndMainTreeCtrl,pGI->hTI,TVIS_BOLD ,TVIS_BOLD);		
	}
	return 0;
}
*/
//DWORD g_AllocatedSR=0,g_TotalAllocatedSR=0;
//DWORD g_Top1=0,g_nRules=0,g_TopNRules=0;
//string sTopServerName;

DWORD WINAPI LoadServerListV4(LPVOID lpVoid)
{
	
	GAME_INFO *pGI = (GAME_INFO*)lpVoid;
	SetCurrentDirectory(USER_SAVE_PATH);
	pGI->vSI.clear();
	pGI->dwTotalServers	 = 0;		
	DWORD idx=0;
	char szBuffer[100];
	if(pGI->hTI!=NULL)
	{
		sprintf(szBuffer,"%s (Loading...)",pGI->szGAME_NAME);
		tvmgr.SetItemText(pGI->hTI,szBuffer);
		TreeView_SetItemState(g_hwndMainTreeCtrl,pGI->hTI,TVIS_BOLD ,TVIS_BOLD);		
	}
	CCVSFile f;
	char szFilename[260];
	sprintf(szFilename,"%s.gs",pGI->szFilename);
	if(f.Open(szFilename,TRUE)==0)
	{
		long lDummy;

		SERVER_INFO *pSrv = NULL;
		f.NextRecordSet();
		while(!f.EofRecordSet())
		{
			pSrv = (SERVER_INFO*) calloc(1,sizeof(SERVER_INFO));
			if(pSrv==NULL)
			{
				log.AddLogInfo(0,"Out of memory.");
				return 0xdeadbabe;
			}

			
			
			f.ReadRecord((long*)&pSrv->cGAMEINDEX);
			pSrv->cGAMEINDEX =  pGI->cGAMEINDEX; 
			f.ReadRecordString(pSrv->szIPaddress);
			f.ReadRecord((short*)&pSrv->usPort);
			f.ReadRecordString(pSrv->szShortCountryName);
			f.ReadRecord((long*)&pSrv->dwIP);
			f.ReadRecordString(pSrv->szPRIVATEPASS);
			f.ReadRecordString(pSrv->szRCONPASS);
			f.ReadRecord((int*)&pSrv->nMaxPlayers);
			f.ReadRecord((int*)&pSrv->nPlayers);
			f.ReadRecord((int*)&pSrv->nPrivateClients);
			f.ReadRecord((long*)&pSrv->bPrivate);
			f.ReadRecord((long*)&pSrv->bPunkbuster);
			f.ReadRecord((long*)&lDummy);
			f.ReadRecord((int*)&pSrv->cRanked);
			f.ReadRecord((int*)&pSrv->cBots);
			f.ReadRecord((int*)&pSrv->cFavorite);
			f.ReadRecord((int*)&pSrv->cHistory);
			f.ReadRecord((int*)&pSrv->cPurge);
			f.ReadRecord((long*)&pSrv->dwPing);
			f.ReadRecord((short*)&pSrv->usQueryPort);
			
			f.NextRecordSet();
			SERVER_RULES *pCurrentSR=NULL;
			while(!f.EofRecordSet())
			{
			
				if(pSrv->pServerRules==NULL)
					pCurrentSR = pSrv->pServerRules = (SERVER_RULES *)calloc(1,sizeof(SERVER_RULES));		
				else
					pCurrentSR = pCurrentSR->pNext = (SERVER_RULES *)calloc(1,sizeof(SERVER_RULES));
																
				f.ReadRecord(pCurrentSR->name);
				f.ReadRecord(pCurrentSR->value);
			}
			f.NextRecordSet();
			PLAYERDATA *pPlyD=NULL;		
			while(!f.EofRecordSet())
			{		
				
				if(pSrv->pPlayerData==NULL)
					pPlyD = pSrv->pPlayerData = (PLAYERDATA *)calloc(1,sizeof(PLAYERDATA));
				else
					pPlyD = pPlyD->pNext =  (PLAYERDATA *)calloc(1,sizeof(PLAYERDATA));
						
				f.ReadRecord((long*)&pPlyD->iPlayer);
				f.ReadRecord((long*)&pPlyD->rate);
				f.ReadRecord((long*)&pPlyD->ping);
				f.ReadRecord((long*)&pPlyD->isBot);
				f.ReadRecord((long*)&pPlyD->time);

				f.ReadRecord(pPlyD->szPlayerName);
				f.ReadRecord(pPlyD->szClanTag);
				f.ReadRecord(pPlyD->szTeam);
									
				pPlyD->cGAMEINDEX = pSrv->cGAMEINDEX;
				pPlyD->pServerInfo = pSrv;
						
				
			}
			pSrv->szServerName = Get_RuleValue((TCHAR*)gm.GamesInfo[pSrv->cGAMEINDEX].vGAME_SPEC_COL.at(COL_SERVERNAME).sRuleValue.c_str(),pSrv->pServerRules,1);
			pSrv->szMap = Get_RuleValue((TCHAR*)gm.GamesInfo[pSrv->cGAMEINDEX].vGAME_SPEC_COL.at(COL_MAP).sRuleValue.c_str(),pSrv->pServerRules);
			pSrv->szMod = Get_RuleValue((TCHAR*)gm.GamesInfo[pSrv->cGAMEINDEX].vGAME_SPEC_COL.at(COL_MOD).sRuleValue.c_str(),pSrv->pServerRules);
			pSrv->szGameTypeName = Get_RuleValue((TCHAR*)gm.GamesInfo[pSrv->cGAMEINDEX].vGAME_SPEC_COL.at(COL_GAMETYPE).sRuleValue.c_str(),pSrv->pServerRules);
			pSrv->szVersion = Get_RuleValue((TCHAR*)gm.GamesInfo[pSrv->cGAMEINDEX].vGAME_SPEC_COL.at(COL_VERSION).sRuleValue.c_str(),pSrv->pServerRules);


			pSrv->dwIndex = idx++;
			pSrv->timeLastScan = 0x0000000049000000;
			InitializeCriticalSection(&pSrv->csLock);

			
			pSrv->bUpdated = 0;
			int hash = pSrv->dwIP + pSrv->usPort;
			pGI->shash.insert(Int_Pair(hash,pSrv->dwIndex));
			pGI->vSI.push_back(pSrv);			
			pGI->dwTotalServers++;
			f.NextRecordSet();
		}

	}
	pGI->lastScanTimeStamp = 0;
	if(pGI->hTI!=NULL)
	{
		sprintf(szBuffer,"%s (%d)",pGI->szGAME_NAME,pGI->dwTotalServers);
		tvmgr.SetItemText(pGI->hTI,szBuffer);
		TreeView_SetItemState(g_hwndMainTreeCtrl,pGI->hTI,TVIS_BOLD ,TVIS_BOLD);		
	}

 	return 0;
}

/*
DWORD WINAPI LoadServerListV3(LPVOID lpVoid)
{
	GAME_INFO *pGI = (GAME_INFO*)lpVoid;
	SetCurrentDirectory(USER_SAVE_PATH);
	pGI->vSI.clear();
	char szBuffer[100];
	if(pGI->hTI!=NULL)
	{
		sprintf(szBuffer,"%s (Loading...)",pGI->szGAME_NAME);
		tvmgr.SetItemText(pGI->hTI,szBuffer);
		TreeView_SetItemState(g_hwndMainTreeCtrl,pGI->hTI,TVIS_BOLD ,TVIS_BOLD);		
	}

	char seps[]   = "\t\n";
	char seps2[]   = "\"\t";

	FILE *fp=NULL;
	char szFilename2[260];
	sprintf(szFilename2,"%s.gs",pGI->szFilename);
	dbg_print("Loading serverlist %s ",szFilename2);
	fopen_s(&fp,szFilename2, "rb");

	char  *next_token1;

	SERVER_INFO	 *srv;
	
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
	char *szMode = NULL;
	pGI->dwTotalServers	 = 0;
	DWORD idx=0;
	char buffer[2624];
	if(fp!=NULL)
	{
		pGI->vSI.clear();
		while( !feof( fp ) )
		{
			memset(&buffer,0,sizeof(buffer));
			srv = NULL;
			for(int i=0; i<sizeof(buffer);i++)
			{
				
				if(fread(&buffer[i], 1, 1, fp)!=0)
				{

					if(buffer[i]==10)
					{

						next_token1 = NULL;
						srv = (SERVER_INFO*) calloc(1,sizeof(SERVER_INFO));
						//memset(&srv,0,sizeof(SERVER_INFO));
						szGameType= strtok_s( buffer, seps, &next_token1);
						if(szGameType!=NULL)
							srv->cGAMEINDEX =  pGI->cGAMEINDEX; //(char)atoi(szGameType); 


						szIP = strtok_s( NULL, seps2, &next_token1);
						if(szIP!=NULL)
							strcpy(srv->szIPaddress,szIP);
						
						szPort = strtok_s( NULL, seps2, &next_token1);
						if(szPort!=NULL)
							srv->usPort = (DWORD)atol(szPort); 

						szShortCountryName = strtok_s( NULL, seps2, &next_token1);
						if(szShortCountryName!=NULL)
							strcpy(srv->szShortCountryName,szShortCountryName);
						

						szByte = strtok_s( NULL, seps, &next_token1); 
						if(szByte!=NULL)
							srv->dwIP = (DWORD)atol(szByte); 

						szPrivatePassword = strtok_s( NULL, seps, &next_token1);
						if(szPrivatePassword!=NULL)
							strcpy(srv->szPRIVATEPASS,TrimString(szPrivatePassword));

						szRCONPassword = strtok_s( NULL, seps, &next_token1);
						if(szRCONPassword!=NULL)
							strcpy(srv->szRCONPASS,TrimString(szRCONPassword));
						
						szMaxPlayers = strtok_s( NULL, seps, &next_token1);
						if(szMaxPlayers!=NULL)
							srv->nMaxPlayers = atoi(szMaxPlayers); 

						szCurrentNumberOfPlayers = strtok_s( NULL, seps, &next_token1);
						if(szCurrentNumberOfPlayers!=NULL)
							srv->nPlayers = atoi(szCurrentNumberOfPlayers); 

						szPrivateClients = strtok_s( NULL, seps, &next_token1);
						if(szPrivateClients!=NULL)
							srv->nPrivateClients = atoi(szPrivateClients); 
						
						szPrivate = strtok_s( NULL, seps, &next_token1);
						if(szPrivate!=NULL)
							srv->bPrivate = (char)atoi(szPrivate); 

						szByte = strtok_s( NULL, seps, &next_token1);  //Punkbuster or VAC
						if(szByte!=NULL)
							srv->bPunkbuster = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);  //not used
						

						szByte = strtok_s( NULL, seps, &next_token1);   //Ranked
						if(szByte!=NULL)
							srv->cRanked = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);   
						if(szByte!=NULL)
							srv->cBots = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);   
						if(szByte!=NULL)
							srv->cFavorite = (char)atoi(szByte); 

						szByte = strtok_s( NULL, seps, &next_token1);   
						if(szByte!=NULL)
							srv->cHistory = (char)atoi(szByte); 

						
						szByte = strtok_s( NULL, seps, &next_token1);  
						if(szByte!=NULL)
							srv->cPurge = (char)atoi(szByte); 
					
						szByte = strtok_s( NULL, seps, &next_token1);  
						if(szByte!=NULL)
							srv->dwPing = (DWORD)atol(szByte); 


						szByte = strtok_s( NULL, seps, &next_token1);  
						if(szByte!=NULL)
							srv->usQueryPort = (unsigned short)atol(szByte); 
						
						szByte = strtok_s( NULL, seps, &next_token1);  //reserved
						break;
					} //end if 0x0A

				} //end if fread

			} //end for loop

	
			g_AllocatedSR = 0;
			g_nRules = 0;
			if(srv!=NULL)
			{
				srv->pServerRules = NULL;
				memset(&buffer,0,sizeof(buffer));
				for(int i=0; i<sizeof(buffer);i++)
				{
							
					if(fread(&buffer[i], 1, 1, fp)!=0)
					{

						if((buffer[i]==10))
						{
							if(buffer[0]==10)
								break;

							SERVER_RULES *pCurrentSR=NULL;
							next_token1 = NULL;
							char *szRulename = strtok_s( buffer, seps2, &next_token1); 
							while((szRulename!=NULL) && (szRulename[0]!=0x0a))
							{
								char *szRulevalue = NULL;
								if((next_token1[1]=='"') && (next_token1[2]=='"'))
									next_token1+=4;
								else
									szRulevalue	= strtok_s( NULL, seps2, &next_token1);  
								
									

								DWORD dwSRStructSize = sizeof(SERVER_RULES);
								if(srv->pServerRules==NULL)
								{
									
									srv->pServerRules = (SERVER_RULES *)calloc(1,dwSRStructSize);		
									pCurrentSR = srv->pServerRules;			
								}else
								{
									pCurrentSR->pNext = (SERVER_RULES *)calloc(1,dwSRStructSize);
									pCurrentSR = pCurrentSR->pNext;					
								}
								g_AllocatedSR+=dwSRStructSize;
								g_nRules++;
								
								if(szRulename!=NULL)
									pCurrentSR->name = _strdup(TrimString(szRulename));
								
								if(szRulevalue!=NULL)
									pCurrentSR->value = _strdup(TrimString(szRulevalue));
									

								if((next_token1[0]==0x0a) || (next_token1[1]==0x0a))
									break;

								szRulename = strtok_s( NULL, seps2, &next_token1);
							} //end while token
							break;
						} //end if 0x0A
					}  //if fread
				}//end of for loop
				

				g_TotalAllocatedSR+=g_AllocatedSR;

				srv->szServerName = Get_RuleValue((TCHAR*)gm.GamesInfo[srv->cGAMEINDEX].vGAME_SPEC_COL.at(COL_SERVERNAME).sRuleValue.c_str(),srv->pServerRules,1);

				srv->pPlayerData = NULL;	
				memset(&buffer,0,sizeof(buffer));
				for(int i=0; i<sizeof(buffer);i++)
				{
							
					if(fread(&buffer[i], 1, 1, fp)!=0)
					{

						if(buffer[i]==10)
						{
							PLAYERDATA *pPlyD=NULL;
							next_token1 = NULL;
							char *szValue = strtok_s( buffer, seps2, &next_token1); 
							while((szValue!=NULL) && (szValue[0]!=0x0A))
							{					

								if(srv->pPlayerData==NULL)
								{
									srv->pPlayerData = (PLAYERDATA *)calloc(1,sizeof(PLAYERDATA));		
									pPlyD = srv->pPlayerData;			
								}else
								{
									pPlyD->pNext = (PLAYERDATA *)calloc(1,sizeof(PLAYERDATA));
									pPlyD = pPlyD->pNext;					
								}
								if(szValue!=NULL)
									pPlyD->iPlayer = atoi(szValue); 								
								
								szValue = strtok_s( NULL, seps2, &next_token1); 
								if(szValue!=NULL)
									pPlyD->rate = atoi(szValue); 								
								
								szValue = strtok_s( NULL, seps2, &next_token1); 
								if(szValue!=NULL)
									pPlyD->ping = atoi(szValue); 								

								szValue = strtok_s( NULL, seps2, &next_token1); 
								if(szValue!=NULL)
									pPlyD->isBot = (char)atoi(szValue); 		

								szValue = strtok_s( NULL, seps2, &next_token1); 
								if(szValue!=NULL)
									pPlyD->time = (char)atoi(szValue); 								
							
								szValue = strtok_s( NULL, seps2, &next_token1); 
								if(szValue!=NULL)
									pPlyD->szPlayerName= strdup(TrimString(szValue));

								szValue = strtok_s( NULL, seps, &next_token1);
								if(szValue!=NULL)
									pPlyD->szClanTag = strdup(TrimString(szValue));
								
								szValue = strtok_s( NULL, seps, &next_token1);
								if(szValue!=NULL)
									pPlyD->szTeam = strdup(TrimString(szValue));
								
								pPlyD->cGAMEINDEX = srv->cGAMEINDEX;

								pPlyD->pServerInfo = srv;

								if(next_token1[0]==0x0a)
									break;
								szValue = strtok_s( NULL, seps2, &next_token1); 
							} //end while token
							break;
							
						} //end if 0x0A
					}  //if fread
				}//end of for loop

			
		
				
				srv->szMap = Get_RuleValue((TCHAR*)gm.GamesInfo[srv->cGAMEINDEX].vGAME_SPEC_COL.at(COL_MAP).sRuleValue.c_str(),srv->pServerRules);
				srv->szMod = Get_RuleValue((TCHAR*)gm.GamesInfo[srv->cGAMEINDEX].vGAME_SPEC_COL.at(COL_MOD).sRuleValue.c_str(),srv->pServerRules);
				srv->szGameTypeName = Get_RuleValue((TCHAR*)gm.GamesInfo[srv->cGAMEINDEX].vGAME_SPEC_COL.at(COL_GAMETYPE).sRuleValue.c_str(),srv->pServerRules);
				srv->szVersion = Get_RuleValue((TCHAR*)gm.GamesInfo[srv->cGAMEINDEX].vGAME_SPEC_COL.at(COL_VERSION).sRuleValue.c_str(),srv->pServerRules);


				srv->dwIndex = idx++;
				srv->timeLastScan = 0x0000000049000000;
				//time(&srv->timeLastScan);
				InitializeCriticalSection(&srv->csLock);
				//InitializeCriticalSectionAndSpinCount(&srv->csLock,0x80000400);
				
				srv->bUpdated = 0;
				int hash = srv->dwIP + srv->usPort;
				pGI->shash.insert(Int_Pair(hash,srv->dwIndex));
				pGI->vSI.push_back(srv);			
				pGI->dwTotalServers++;
			} //end of srv!=NULL
			

		//	i = sizeof(buffer)+1;
			
		} // end while eof
		fclose(fp);
	}  //	if(fp!=NULL)
	pGI->lastScanTimeStamp = 0;
	if(pGI->hTI!=NULL)
	{
		sprintf(szBuffer,"%s (%d)",pGI->szGAME_NAME,pGI->dwTotalServers);
		tvmgr.SetItemText(pGI->hTI,szBuffer);
		TreeView_SetItemState(g_hwndMainTreeCtrl,pGI->hTI,TVIS_BOLD ,TVIS_BOLD);		
	}

	dbg_print("Biggest mem usage server rules %d",g_Top1);
	dbg_print("Top number of server rules %d (%d)",g_TopNRules,g_Top1/12);
	dbg_print("Server name %s",sTopServerName.c_str());
	dbg_print("Total %d",g_TotalAllocatedSR);

	return 0;
}
*/

void SaveAllServerList()
{
	for(int i=0;i<gm.GamesInfo.size();i++)
		SaveServerList(&gm.GamesInfo[i]);

	//SetStatusText(ICO_INFO,g_lang.GetString("SavedServers"));
}

void LoadAllServerList()
{
	HANDLE hThread;
	DWORD dwThreadIdBrowser;
	hThread = NULL;		
	hThread = CreateThread( NULL, 0, &LoadAllServerListThread, (LPVOID)0,0, &dwThreadIdBrowser);                
	if (hThread == NULL) 
	{
		log.AddLogInfo(GS_LOG_WARNING,"CreateThread failed (%d)\n", GetLastError() ); 
	}
	else 
	{
		CloseHandle( hThread );
	}

}

DWORD WINAPI LoadAllServerListThread(LPVOID lpVoid)
{

	EnableButtons(false);
	EnterCriticalSection(&LOAD_SAVE_CS);

	for(int i=0;i<gm.GamesInfo.size();i++)
	{
		if(gm.GamesInfo[i].vSI.size()==0) //Only try to load if no list exsists (needed for minimizing and restoring)
		{	
			gm.GamesInfo[i].bLockServerList = TRUE;
			CTimer t;
			t.Start();
			LoadServerListV4(&gm.GamesInfo[i]);
			t.Stop();
			t.Print();
			gm.GamesInfo[i].bLockServerList = FALSE;
		}
	}
	
	LeaveCriticalSection(&LOAD_SAVE_CS);

	if(gm.GamesInfo.size()>0)
	{
		if(gm.GamesInfo[g_currentGameIdx].bActive)
			SetCurrentActiveGame(g_currentGameIdx);
		else
			SetCurrentActiveGame(FindFirstActiveGame());

		if((gm.GamesInfo[g_currentGameIdx].vSI.size()>0) && (g_bMinimized == false))
			OnActivate_ServerList(SCAN_FILTERED);
	}
//	if(hTimerMonitor==NULL)
//		hTimerMonitor = SetTimer(g_hWnd,IDT_MONITOR_QUERY_SERVERS,MONITOR_INTERVAL,0);
	EnableButtons(true);
 return 0;
}



void DeleteAllServerLists(HWND hWnd)
{
	int ret = MessageBox(NULL,g_lang.GetString("AskDeleteServerList"),"Warning!",MB_OKCANCEL | MB_ICONWARNING);
	if(ret == IDOK)
	{
		if(g_bRunningQueryServerList==false)
		{
			g_CurrentSRV=NULL;
			ListView_DeleteAllItems(g_hwndListViewVars);
			ListView_DeleteAllItems(g_hwndListViewPlayers);
			ListView_DeleteAllItems(g_hwndListViewServer);

			if(TryEnterCriticalSection(&LOAD_SAVE_CS)==TRUE)
			{
				
				ListView_SetItemCount(g_hwndListViewServer,0);								

				char szBuffer[100];
				for(int i=0;i<gm.GamesInfo.size();i++)
				{
					gm.ClearServerList(i);
					remove(gm.GamesInfo[i].szFilename);
					gm.GamesInfo[i].dwTotalServers = 0;
					sprintf(szBuffer,"%s (%d)",gm.GamesInfo[i].szGAME_NAME,gm.GamesInfo[i].dwTotalServers);
					if(gm.GamesInfo[i].bActive)
						tvmgr.SetItemText(gm.GamesInfo[i].hTI,szBuffer);
				}
				LeaveCriticalSection(&LOAD_SAVE_CS);
				MessageBox(hWnd,g_lang.GetString("DeletedServerList"),"Info",MB_OK);
				return;
			}
			
		}
		MessageBox(hWnd,g_lang.GetString("ErrorDeletingServerList"),"Warning!",MB_OKCANCEL | MB_ICONWARNING);
	}
}

void DeleteServerLists(char cGameIdx)
{
	int ret = MessageBox(NULL,g_lang.GetString("AskDeleteServerList"),"Warning!",MB_OKCANCEL | MB_ICONWARNING);
	if(ret == IDOK)
	{
		if(g_bRunningQueryServerList==false)
		{
			ListView_DeleteAllItems(g_hwndListViewVars);
			ListView_DeleteAllItems(g_hwndListViewPlayers);
			ListView_DeleteAllItems(g_hwndListViewServer);
			ListView_SetItemCount(g_hwndListViewServer,0);
		
			OutputDebugString(gm.GamesInfo[cGameIdx].szGAME_NAME);
			OutputDebugString(" - CLEAN UP SERVERLIST\n");
			
			gm.ClearServerList(cGameIdx);


			g_CurrentSRV=NULL;

			char szBuffer[100];
			remove(gm.GamesInfo[cGameIdx].szFilename);
			gm.GamesInfo[cGameIdx].dwTotalServers = 0;
			sprintf(szBuffer,"%s (%d)",gm.GamesInfo[cGameIdx].szGAME_NAME,gm.GamesInfo[cGameIdx].dwTotalServers);
			if(gm.GamesInfo[cGameIdx].bActive)
				tvmgr.SetItemText(gm.GamesInfo[cGameIdx].hTI,szBuffer);
	
			MessageBox(g_hWnd,g_lang.GetString("DeletedServerList"),"Info",MB_OK);
		
		}else
			MessageBox(g_hWnd,g_lang.GetString("ErrorDeletingServerList"),"Warning!",MB_OKCANCEL | MB_ICONWARNING);
	}
}


/*
	Is called when Add Server button is clicked...
*/


DWORD WINAPI SavingFilesCleaningUpThread(LPVOID pvoid )
{
	if(bWaitingToSave)
		return 0;
	bWaitingToSave = TRUE;
	dbg_print("Entering SavingFilesCleaningUpThread!\n");
	if(g_bRunningQueryServerList==true)
	{
		dbg_print("Waiting for the scanner to close down!\n");
		DWORD dwWaitResult = WaitForSingleObject( hCloseEvent,INFINITE);    // indefinite wait

		switch (dwWaitResult) 
		{
			// Both event objects were signaled.
			case WAIT_OBJECT_0: 
				dbg_print("WAIT_OBJECT_0:  @ SavingFilesCleaningUpThread \n"); 
				break; 
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
				dbg_print("Wait error @ SavingFilesCleaningUpThread\n"); 
	            
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
	gm.ClearAllGameServer();
	LeaveCriticalSection(&LOAD_SAVE_CS);

	g_vecPlayerList.clear();
	CleanUp_PlayerList(pCurrentPL);
	pCurrentPL = NULL;

	bm.Save();
	bm.Clear();
	CFG_Save(0);
	tvmgr.CleanUp();
	g_sMIRCoutput.clear();

	if((DWORD)pvoid!=0x0000FFFF)
	{
		dbg_print("We are gonna close down the app (sending WM_DESTROY)!");
		//DestroyWindow(g_hWnd);
		PostMessage(g_hWnd,WM_DESTROY,(WPARAM)pvoid,0);
	}
	
	

	g_bCancel = false;
	bWaitingToSave = FALSE;
	SCANNER_bCloseApp = FALSE;
	
	return 0x1001;
}



DWORD WINAPI SaveOnMinimizeThread(LPVOID pvoid )
{
	if(bWaitingToSaveMinimized)
		return 0;
	bWaitingToSaveMinimized = TRUE;

	SendMessage(g_hwndMainSTATS,WM_STOP_PING,0,0);

	dbg_print("Entering SaveOnMinimizeThread!\n");
	if(g_bRunningQueryServerList==true)
	{
		dbg_print("Waiting for the scanner to close down!\n");
		DWORD dwWaitResult = WaitForSingleObject( hCloseEvent,INFINITE);    // indefinite wait

		switch (dwWaitResult) 
		{
			// Both event objects were signaled.
			case WAIT_OBJECT_0: 
				dbg_print("WAIT_OBJECT_0: @ SaveOnMinimizeThread\n"); 
				break; 
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
				dbg_print("Wait error @ SaveOnMinimizeThread\n"); 
	            
		}
	}
	
	while(g_bRunningQueryServerList)
	{
		dbg_print("Sleeping because of serverlist is being updated...\n");
		Sleep(300);
	}
	while(TryEnterCriticalSection(&REDRAWLIST_CS)==FALSE)
	{
		dbg_print("Sleeping because redrawing of the server list...\n");
		Sleep(300);
	}
	LeaveCriticalSection(&REDRAWLIST_CS);

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

	bm.Save();
	CFG_Save(0);

	LeaveCriticalSection(&LOAD_SAVE_CS);


	bWaitingToSaveMinimized = FALSE;
	SCANNER_bCloseApp = FALSE;
	dbg_print("SaveOnMinimizeThread: Win state %d",AppCFG.nWindowState);
	return 0x1002;
}


BOOL g_bSaving = FALSE;

DWORD WINAPI CFG_Save(LPVOID lpVoid)
{
	SetCurrentDirectory(USER_SAVE_PATH);
	log.AddLogInfo(GS_LOG_DEBUG,"Saving config");

	if(g_bSaving)
		return 0;

	g_bSaving = TRUE;

	if(tvmgr.m_bTREELOADED)
		tvmgr.Save();


//	FILE *fp=fopen(FILE_ETSERVERCFG, "wb");
//	fwrite((const void*)&AppCFG, sizeof(APP_SETTINGS_NEW), 1, fp);
//	fclose(fp);

	TiXmlDocument doc;  
	TiXmlElement  *MainVersion;
 	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );  
	doc.LinkEndChild( decl );  
	
	TiXmlElement * root = new TiXmlElement( "GScfg" );  
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
	
	for(int i=0;i<gm.GamesInfo.size(); i++)
	{
		TiXmlElement * abc = new TiXmlElement( "Game" );  
		WriteCfgStr(abc, "GameData", "GameName",gm.GamesInfo[i].szGAME_NAME) ;

		
	//	WriteCfgStr(abc, "GameData", "MasterServer",gm.GamesInfo[i].szMasterServerIP[0]);
	//	WriteCfgStr(abc, "GameData", "MapPreview",gm.GamesInfo[i].szMAP_MAPPREVIEW_PATH);
	//	WriteCfgStr(abc, "GameData", "ProtcolName",gm.GamesInfo[i].szWebProtocolName);
		WriteCfgInt(abc, "GameData", "Active",gm.GamesInfo[i].bActive);
		WriteCfgInt(abc, "GameData", "gametype",gm.GamesInfo[i].cGAMEINDEX);
	//	WriteCfgInt(abc, "GameData", "MasterServerPort",gm.GamesInfo[i].dwMasterServerPORT);
		WriteCfgInt(abc, "GameData", "IconIndex",gm.GamesInfo[i].iIconIndex);
	
		
		for(int n=0;n<gm.GamesInfo[i].vGAME_INST.size();n++)
		{
			TiXmlElement * installs = new TiXmlElement( "Installs" ); 
			WriteCfgStr(installs, "Install", "Name",gm.GamesInfo[i].vGAME_INST.at(n).sName.c_str());
			WriteCfgStr(installs, "Install", "Path",gm.GamesInfo[i].vGAME_INST.at(n).szGAME_PATH.c_str());
			WriteCfgStr(installs, "Install", "Cmd",gm.GamesInfo[i].vGAME_INST.at(n).szGAME_CMD.c_str());
		//	WriteCfgStr(installs, "Install", "LaunchByMod",gm.GamesInfo[i].vGAME_INST.at(n).sMod.c_str());
		//	WriteCfgStr(installs, "Install", "LaunchByVer",gm.GamesInfo[i].vGAME_INST.at(n).sVersion.c_str());
			WriteCfgInt(installs, "Install", "ScriptActive",gm.GamesInfo[i].vGAME_INST.at(n).bActiveScript);
			WriteCfgStr(installs, "Install", "ScriptName",gm.GamesInfo[i].vGAME_INST.at(n).sFilterName.c_str());
			WriteCfgStr(installs, "Install", "Script",gm.GamesInfo[i].vGAME_INST.at(n).sScript.c_str());
			abc->LinkEndChild( installs ); 
		}
	
		abcd->LinkEndChild( abc ); 

	}

	TiXmlElement * pElemSort = new TiXmlElement( "Sort" );  
	root->LinkEndChild( pElemSort );  
	
	WriteCfgInt(pElemSort,"SortServerList","LastSortColumn",iLastColumnSortIndex);

	TiXmlElement * xmlWinState = new TiXmlElement( "WindowState" );  
	root->LinkEndChild( xmlWinState );  
	xmlWinState->SetAttribute("value", AppCFG.nWindowState);


	TiXmlElement * pElemWins = new TiXmlElement( "Windows" );  
	root->LinkEndChild( pElemWins );  

	for(int i=0;i<WIN_MAX; i++)
	{			
		XML_WriteWindow(pElemWins, "Window", &WNDCONT[i]);
	}

	TiXmlElement * options = new TiXmlElement( "Options" );  
	root->LinkEndChild( options );  
	WriteCfgInt(options,"General","Transparancy",AppCFG.g_cTransparancy);
	WriteCfgInt(options,"General","MaxScanThreads",AppCFG.dwThreads);
	WriteCfgInt(options,"General","NetworkRetries",AppCFG.dwRetries);
	WriteCfgInt(options,"General","SleepPerScan",AppCFG.dwSleep);
	//---------------------------
	//Filter options
	//---------------------------
	TiXmlElement * filters = new TiXmlElement( "Filters" );  
	root->LinkEndChild( filters );  
	WriteCfgInt(filters,"Filter","HideOfflineServers",AppCFG.filter.bHideOfflineServers);



	TiXmlElement * xmlTVVer = new TiXmlElement( "TreeView" );  
	root->LinkEndChild( xmlTVVer );  
	xmlTVVer->SetAttribute("Version", tvmgr.TREEVIEW_VERSION);


	//---------------------------
	//View options
	//---------------------------
	XML_WriteCfgInt(root,"MapPreviewResize","disable",AppCFG.bNoMapResize);
	XML_WriteCfgInt(root,"CloseOnConnect","enable",AppCFG.bCloseOnConnect);
	XML_WriteCfgInt(root,"RegisterWebProtocols","enable",AppCFG.bRegisterWebProtocols);
	XML_WriteCfgInt(root,"ShortCountryName","enable",AppCFG.bUseShortCountry);
	XML_WriteCfgInt(root,"MapPreview","show",AppCFG.bShowMapPreview);
	XML_WriteCfgInt(root,"AutoStart","enable",AppCFG.bAutostart);
	XML_WriteCfgInt(root,"ShowMinimizePopUp","disable",AppCFG.bShowMinimizePopUp);
	XML_WriteCfgInt(root,"ColorEncodedFont","Enable",AppCFG.bUseColorEncodedFont);
	XML_WriteCfgInt(root,"ServerRules","show",AppCFG.bShowServerRules);
	XML_WriteCfgInt(root,"PlayerList","show",AppCFG.bShowPlayerList);

	XML_WriteCfgInt(root,"LastGameView","index",g_currentGameIdx);


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

	TiXmlElement * xmlSND = new TiXmlElement( "BuddySoundNotify" );  
	root->LinkEndChild( xmlSND );  
	xmlSND->SetAttribute("enable", AppCFG.bUseBuddySndNotify);
	xmlSND->SetAttribute("path", AppCFG.szNotifySoundWAVfile);

	TiXmlElement * xmlBuddyNotify = new TiXmlElement( "BuddyNotify" );  
	root->LinkEndChild( xmlBuddyNotify );  
	xmlBuddyNotify->SetAttribute("enable", AppCFG.bBuddyNotify);

	TiXmlElement * xmlElm2 = new TiXmlElement( "OptionalEXEsettings" );  
	root->LinkEndChild( xmlElm2 );  
	xmlElm2->SetAttribute("enable", AppCFG.bUse_EXT_APP);
	xmlElm2->SetAttribute("path", strlen(AppCFG.szEXT_EXE_PATH)?AppCFG.szEXT_EXE_PATH:"no path set");
	xmlElm2->SetAttribute("cmd", AppCFG.szEXT_EXE_CMD);
	xmlElm2->SetAttribute("WindowName", AppCFG.szEXT_EXE_WINDOWNAME);
	xmlElm2->SetAttribute("CloseOnExitServer", AppCFG.bEXTClose);
	xmlElm2->SetAttribute("MinimizeOnLaunch", AppCFG.bEXTMinimize);

	TiXmlElement * xmlElmOREXE= new TiXmlElement( "OnReturnEXEsettings" );  
	root->LinkEndChild( xmlElmOREXE );  
	xmlElmOREXE->SetAttribute("enable", AppCFG.bUse_EXT_APP2);
	xmlElmOREXE->SetAttribute("path", AppCFG.szOnReturn_EXE_PATH);
	xmlElmOREXE->SetAttribute("cmd", AppCFG.szOnReturn_EXE_CMD);


	TiXmlElement * xmlElm5 = new TiXmlElement( "SocketTimeout" );  
	root->LinkEndChild( xmlElm5 );  
	xmlElm5->SetAttribute("seconds", AppCFG.socktimeout.tv_sec);
	xmlElm5->SetAttribute("useconds", AppCFG.socktimeout.tv_usec);

	TiXmlElement * xmlElmlang = new TiXmlElement( "CurrentLanguage" );  
	root->LinkEndChild( xmlElmlang );  
	xmlElmlang->SetAttribute("filename", AppCFG.szLanguageFilename);

	doc.SaveFile( "config.xml" );
    g_bSaving = FALSE;

	log.AddLogInfo(GS_LOG_DEBUG,"Saving config...DONE!");
	
	return 0;
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
			
}
HANDLE g_hSaveThread=NULL; 

//Returns the thread handle
void SaveAll(DWORD dwCloseReason)
{

	DWORD dwThreadIdBrowser=0;				
	g_hSaveThread = CreateThread( NULL, 0, &SavingFilesCleaningUpThread,(LPVOID)dwCloseReason ,0, &dwThreadIdBrowser);                
	if (g_hSaveThread == NULL) 
	{
		log.AddLogInfo(GS_LOG_WARNING, "CreateThread failed (%d)\n", GetLastError() ); 
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
	SetStatusText(ICO_INFO,g_lang.GetString("StoppingScanner"));
	bWaitingToSave = TRUE;
	dbg_print("Entering SavingFilesCleaningUpThread!\n");
	if(g_bRunningQueryServerList==true)
	{
		dbg_print("Waiting for the scanner to close down!\n");
		DWORD dwWaitResult = WaitForSingleObject(hCloseEvent,INFINITE);    // infinite wait
		switch (dwWaitResult) 
		{
			// Both event objects were signaled.
			case WAIT_OBJECT_0: 
				dbg_print("CloseEvent trigged successfully!\n"); 
				break; 
			case WAIT_ABANDONED:
				dbg_print("WAIT_ABANDONED: %s\n",__FUNCTION__); 
				break;
			case WAIT_TIMEOUT:
				dbg_print("WAIT_TIMEOUT: %s\n",__FUNCTION__); 
				break;
			case WAIT_FAILED:
				dbg_print("WAIT_FAILED: %s\n",__FUNCTION__); 
				break;
			// An error occurred.
			default: 
				dbg_print("Wait error @ WaitForSingleObject(hCloseEvent,INFINITE)\n");        
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
	SetStatusText(ICO_INFO,g_lang.GetString("ScannerStopped"));
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
		log.AddLogInfo(GS_LOG_WARNING, "CreateThread failed (%d)\n", GetLastError() ); 
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

	HANDLE hThread=NULL; 
	DWORD dwThreadIdBrowser=0;	
	Update_WindowSizes(SIZE_MINIMIZED);
	hThread = CreateThread( NULL, 0, &SaveOnMinimizeThread,(LPVOID)0 ,0, &dwThreadIdBrowser);                
	if (hThread == NULL) 
	{
		log.AddLogInfo(GS_LOG_WARNING, "CreateThread SaveOnMinimizeThread failed (%d)\n", GetLastError() ); 
	} else
	{
		CloseHandle( hThread );
	}
	ShowWindow(hWnd,SW_HIDE);
}


int SetCurrentActiveGame(int GameIndex)
{
	try
	{
		gm.ValidateGameIndex(GameIndex);
	}catch(int a)
	{
		log.AddLogInfo(GS_LOG_WARNING, "Error GameIdx <SetCurrentActiveGame> (%d) File:(%s) Line:(%d)\n", GetLastError(),__FILE__,__LINE__ ); 
		MessageBox(g_hWnd,"Game idx out of bounds!","Error",MB_OK);
		throw 2;
	}

	g_currentGameIdx = GameIndex;

	currCV = &gm.GamesInfo[g_currentGameIdx];
	if(g_hwndMainTreeCtrl==NULL)
		return 0;

	if(GameIndex<0)
		return 0;

	//Deselect
	for(int i=0;i<gm.GamesInfo.size();i++)
		if(gm.GamesInfo[i].hTI!=NULL)
			TreeView_SetItemState(g_hwndMainTreeCtrl,gm.GamesInfo[i].hTI,0 , TVIS_SELECTED );


	TreeView_SetItemState(g_hwndMainTreeCtrl,gm.GamesInfo[g_currentGameIdx].hTI,TVIS_SELECTED, TVIS_SELECTED);
	TreeView_Expand(g_hwndMainTreeCtrl, gm.GamesInfo[g_currentGameIdx].hTI, TVE_EXPAND);
	return g_currentGameIdx;
}


//Spider pig... spider pig... does what whatever, spider pig does...
//can he swing from a web, no he can't he is a pig... loooook oooout he is spider pig.. 


int g_waitAniIdx = 0;

void DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, long xStart, 
                           long yStart, long Width, long Height, 
                           long XOffset,COLORREF cTransparentColor) 
{ 
BITMAP     bm; 
COLORREF   cColor; 
HBITMAP    bmAndBack, bmAndObject, bmAndMem, bmSave; 
HBITMAP    bmBackOld, bmObjectOld, bmMemOld, bmSaveOld; 
HDC        hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave; 
POINT      ptSize; 
   hdcTemp = CreateCompatibleDC(hdc); 
   SelectObject(hdcTemp, hBitmap);   // Select the bitmap 
   GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm); 
   ptSize.x = bm.bmWidth;            // Get width of bitmap 
   ptSize.y = bm.bmHeight;           // Get height of bitmap 
   DPtoLP(hdcTemp, &ptSize, 1);      // Convert from device 
                                     // to logical points 
   // Create some DCs to hold temporary data. 
   hdcBack   = CreateCompatibleDC(hdc); 
   hdcObject = CreateCompatibleDC(hdc); 
   hdcMem    = CreateCompatibleDC(hdc); 
   hdcSave   = CreateCompatibleDC(hdc); 
   // Create a bitmap for each DC. DCs are required for a number of 
   // GDI functions. 
   // Monochrome DC 
   bmAndBack   = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL); 
   // Monochrome DC 
   bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL); 
   bmAndMem    = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y); 
   bmSave      = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y); 
   // Each DC must select a bitmap object to store pixel data. 
   bmBackOld   = (HBITMAP)SelectObject(hdcBack, bmAndBack); 
   bmObjectOld = (HBITMAP)SelectObject(hdcObject, bmAndObject); 
   bmMemOld    = (HBITMAP)SelectObject(hdcMem, bmAndMem); 
   bmSaveOld   = (HBITMAP)SelectObject(hdcSave, bmSave); 
   // Set proper mapping mode. 
   SetMapMode(hdcTemp, GetMapMode(hdc)); 
   // Save the bitmap sent here, because it will be overwritten. 
   BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, XOffset, 0, SRCCOPY); 
   // Set the background color of the source DC to the color. 
   // contained in the parts of the bitmap that should be transparent 
   cColor = SetBkColor(hdcTemp, cTransparentColor); 
   // Create the object mask for the bitmap by performing a BitBlt() 
   // from the source bitmap to a monochrome bitmap. 
   BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, XOffset, 0, 
          SRCCOPY); 
   // Set the background color of the source DC back to the original 
   // color. 
   SetBkColor(hdcTemp, cColor); 
   // Create the inverse of the object mask. 
   BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, XOffset, 0, 
          NOTSRCCOPY); 
   // Copy the background of the main DC to the destination. 
   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, xStart, yStart, 
          SRCCOPY); 
   // Mask out the places where the bitmap will be placed. 
   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND); 
   // Mask out the transparent colored pixels on the bitmap. 
   BitBlt(hdcTemp, XOffset, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND); 
   // XOR the bitmap with the background on the destination DC. 
   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, XOffset, 0, SRCPAINT); 
   // Copy the destination to the screen. 
   BitBlt(hdc, xStart, yStart, Width, Height, hdcMem, 0, 0, 
          SRCCOPY); 
   // Place the original bitmap back into the bitmap sent here. 
   BitBlt(hdcTemp, XOffset, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY); 
   // Delete the memory bitmaps. 
   DeleteObject(SelectObject(hdcBack, bmBackOld)); 
   DeleteObject(SelectObject(hdcObject, bmObjectOld)); 
   DeleteObject(SelectObject(hdcMem, bmMemOld)); 
   DeleteObject(SelectObject(hdcSave, bmSaveOld)); 
   // Delete the memory DCs. 
   DeleteDC(hdcMem); 
   DeleteDC(hdcBack); 
   DeleteDC(hdcObject); 
   DeleteDC(hdcSave); 
   DeleteDC(hdcTemp); 
} 
/*
FIBITMAP *dib = FreeImage_Load(FIF_PNG, "test.png", PNG_DEFAULT);
// ...
HBITMAP bitmap = CreateDIBitmap(hDC, FreeImage_GetInfoHeader(dib),
CBM_INIT, FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS);
// ...
FreeImage_Unload(dib);
*/
void Animate()
{
	RECT rcClient;
	HDC hDC = GetDC(g_hWnd);
	GetClientRect(g_hWnd, &rcClient);
	FIBITMAP *dib = NULL;
		char szPath[512];
	sprintf(szPath,"%s\\progressanimation.bmp",EXE_PATH);
	dib = FreeImage_Load(FIF_BMP, szPath, BMP_DEFAULT);
	if(dib!=NULL)
	{
		int xOffset = (g_waitAniIdx*17);

		HBITMAP hBitmap = CreateDIBitmap(hDC, FreeImage_GetInfoHeader(dib),CBM_INIT, FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS);
		
		DrawTransparentBitmap(hDC,  hBitmap, g_INFOIconRect.left,g_INFOIconRect.top, 16, 16, xOffset,0xFFFFFF); 

		DeleteObject(hBitmap);
		FreeImage_Unload(dib);
	}


	g_waitAniIdx++;
	if(g_waitAniIdx>5)
		g_waitAniIdx = 0;

	 ReleaseDC(g_hWnd,hDC);

}

DWORD WINAPI Download_MapshotThread(LPVOID lpParam )
{
	CDownload dl;
	
	char szPath[MAX_PATH+_MAX_FNAME+1];
	char szWWWPath[MAX_PATH+_MAX_FNAME+1];
	wchar_t wcPath[MAX_PATH+_MAX_FNAME+1];

	strcpy(szPath,USER_SAVE_PATH);	
	strcat(szPath,gm.GamesInfo[g_currentGameIdx].szMAP_MAPPREVIEW_PATH);
	ZeroMemory(wcPath,sizeof(wcPath));
	mbstowcs(wcPath,(const char *)szPath,strlen(szPath));	
	SHCreateDirectory(NULL,wcPath);

	strcat(szPath,"\\");
	char *start = strchr(gm.g_szMapName,'/');
	if(start==NULL)
		start = &gm.g_szMapName[0];
	else
		start++;
	
	char *end = strchr(gm.g_szMapName,'.');
	if(end!=NULL)
		end[0]=0;
	strcat(szPath,start);
	strcat(szPath,".jpg");


	
	dl.SetPath(szPath);
	strcpy(szWWWPath,"http://www.bdamage.se/mapshots/");
	strcat(szWWWPath,gm.GamesInfo[g_currentGameIdx].szMAP_MAPPREVIEW_PATH);
	strcat(szWWWPath,"/");
	start = strchr(gm.g_szMapName,'/');
	if(start==NULL)
		start = &gm.g_szMapName[0];
	else
		start++;
	
	end = strchr(gm.g_szMapName,'.');
	if(end!=NULL)
		end[0]=0;

	strcat(szWWWPath,start);
	strcat(szWWWPath,".jpg");
	
	dbg_print("%s",szPath);
	dbg_print("%s\n",szWWWPath);

	if(dl.HttpFileDownload(szWWWPath,szPath,NULL,NULL)!=0)
	{
		strcpy(szPath,USER_SAVE_PATH);	
		strcat(szPath,gm.GamesInfo[g_currentGameIdx].szMAP_MAPPREVIEW_PATH);
		strcat(szPath,"\\");
		start = strchr(gm.g_szMapName,'/');
		if(start==NULL)
			start = &gm.g_szMapName[0];
		else
			start++;
		
		end = strchr(gm.g_szMapName,'.');
		if(end!=NULL)
			end[0]=0;
		strcat(szPath,start);
		strcat(szPath,".png");


		strcpy(szWWWPath,"http://www.bdamage.se/mapshots/");
		strcat(szWWWPath,gm.GamesInfo[g_currentGameIdx].szMAP_MAPPREVIEW_PATH);
		strcat(szWWWPath,"/");
		start = strchr(gm.g_szMapName,'/');
		if(start==NULL)
			start = &gm.g_szMapName[0];
		else
			start++;
		
		end = strchr(gm.g_szMapName,'.');
		if(end!=NULL)
			end[0]=0;

		strcat(szWWWPath,start);
		strcat(szWWWPath,".png");
		dbg_print("%s\n",szWWWPath);
		if(dl.HttpFileDownload(szWWWPath,szPath,NULL,NULL)==0)
			UpdateCurrentServerUI();

	}else
		UpdateCurrentServerUI();
	return 0;
}

void Download_MapshotInit()
{
	
	HANDLE hThread=NULL; 
	DWORD dwThreadIdBrowser;
	hThread = CreateThread( NULL, 0, &Download_MapshotThread, (LPVOID)0,0, &dwThreadIdBrowser);                
	if (hThread == NULL) 
	{
		log.AddLogInfo(GS_LOG_WARNING, "CreateThread failed (%d)\n", GetLastError() ); 
	}
	else 
	{
		SetThreadName( dwThreadIdBrowser, "Download_MapshotThread");
		CloseHandle( hThread );
	}
}

void OnPaint(HDC hDC)
{
	dbg_print("OnPaint");
	POINT pt;
	if(AppCFG.bShowMapPreview)
	{
		SetCurrentDirectory(EXE_PATH);
		FIBITMAP *dib = NULL;
		if(g_CurrentSRV!=NULL)
		{
			__try{

			if(g_CurrentSRV->dwPing == 9999)
				strcpy(gm.g_szMapName,"net.jpg");
			}
			__except(EXCEPTION_ACCESS_VIOLATION == GetExceptionCode())
			{
			// exception handling code
				log.AddLogInfo(GS_LOG_DEBUG,"Access Violation!!! (OnPaint)\n");
			}

		}
		if(strcmp(gm.g_szMapName,"net.jpg")==0)
		{
			dib = FreeImage_Load(FIF_JPEG, gm.g_szMapName, JPEG_DEFAULT);
		}
		else
		{
			char szPath[512+256];
			//Trim
			char *start = strchr(gm.g_szMapName,'/');
			if(start==NULL)
				start = &gm.g_szMapName[0];
			else
				start++;
			
			char *end = strchr(gm.g_szMapName,'.');
			if(end!=NULL)
				end[0]=0;

			//Try different paths
			sprintf(szPath,"%s%s\\%s.jpg",USER_SAVE_PATH,gm.GamesInfo[g_currentGameIdx].szMAP_MAPPREVIEW_PATH,start);
			dib = FreeImage_Load(FIF_JPEG, szPath, JPEG_DEFAULT);
			if(!dib)
			{
				sprintf(szPath,"%s%s\\%s.png",USER_SAVE_PATH,gm.GamesInfo[g_currentGameIdx].szMAP_MAPPREVIEW_PATH,start);
				dib = FreeImage_Load(FIF_PNG, szPath, PNG_DEFAULT);

				if(!dib)
				{
					Download_MapshotInit();
				}
			}

		}
		if(!dib)
			dib = FreeImage_Load(FIF_PNG, "unknownmap.png", PNG_DEFAULT);



		if(dib!=NULL)
		{
			//calc aspect ratio
			float width = FreeImage_GetWidth(dib);
			float height = FreeImage_GetHeight(dib);
			float maxWidth = WNDCONT[WIN_MAPPREVIEW].rSize.right-10; 
			float maxHeight = WNDCONT[WIN_MAPPREVIEW].rSize.bottom;
			int newHeight,newWidth;
			if(maxWidth<maxHeight)
			{
				 newWidth = maxWidth;
				 newHeight = (int)((height / width) * maxWidth);
			}else
			{
				newHeight = maxHeight;
				newWidth = (int)((width / height ) * maxHeight);
			}

			if((newWidth>FreeImage_GetWidth(dib)) && AppCFG.bNoMapResize)
				newWidth = FreeImage_GetWidth(dib);

			if((newHeight>FreeImage_GetHeight(dib)) && AppCFG.bNoMapResize)
				newHeight = FreeImage_GetHeight(dib);

			SetStretchBltMode(hDC, COLORONCOLOR);
			StretchDIBits(hDC, 
				WNDCONT[WIN_MAPPREVIEW].rSize.left, 
				WNDCONT[WIN_MAPPREVIEW].rSize.top, 
				newWidth, 
				newHeight, 
			0, 0, FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),
			FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS, SRCCOPY);
			FreeImage_Unload(dib);

	//	Rectangle(hDC,  WNDCONT[WIN_MAPPREVIEW].rSize.left, WNDCONT[WIN_MAPPREVIEW].rSize.top, WNDCONT[WIN_MAPPREVIEW].rSize.left+WNDCONT[WIN_MAPPREVIEW].rSize.right, WNDCONT[WIN_MAPPREVIEW].rSize.top+WNDCONT[WIN_MAPPREVIEW].rSize.bottom);
		
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
		OnSize(g_hWnd,SIZE_RESTORED,TRUE);
	}							
}


void CalcSplitterGripArea()
{
	
	//log.AddLogInfo(GS_LOG_INFO,"Called CalcSplitterGripArea.");

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

void Load_WindowSizes()
{
	RECT rc;
	CopyRect(&rc,&WNDCONT[WIN_MAIN].rSize);
	dbg_print("Load_WindowSizes %d, %d - %d, %d",rc.left,rc.top,rc.right,rc.bottom);
	rc.bottom-= (STATUSBAR_Y_OFFSET + TOOLBAR_Y_OFFSET); //reduce the size of statusbar from the main window size


	SplitterGripArea[0].tvYPos =  WNDCONT[WIN_MAINTREEVIEW].rSize.bottom ;
	SplitterGripArea[1].tvXPos =  WNDCONT[WIN_MAINTREEVIEW].rSize.right;
	SplitterGripArea[2].tvYPos =  WNDCONT[WIN_SERVERLIST].rSize.bottom ;
	if(AppCFG.bShowBuddyList==FALSE)	
		WNDCONT[WIN_BUDDYLIST].bShow = FALSE;

	if(AppCFG.bShowPlayerList==false)		
	{
		SplitterGripArea[2].tvYPos =  rc.bottom * 1;
		WNDCONT[WIN_PLAYERS].bShow = FALSE;
		WNDCONT[WIN_RULES].bShow = FALSE;
		WNDCONT[WIN_TABCONTROL].bShow = FALSE;
	}
	else
	{
		WNDCONT[WIN_PLAYERS].bShow = TRUE;		
		WNDCONT[WIN_TABCONTROL].bShow = TRUE;
	}
	g_INFOIconRect.top = WNDCONT[WIN_STATUS].rSize.top;
	
	for(int i=0;i<WIN_MAX-1;i++)
	{
		if(WNDCONT[i].idx!=WIN_MAIN)
		{
			MoveWindow(WNDCONT[i].hWnd,WNDCONT[i].rSize.left,WNDCONT[i].rSize.top,WNDCONT[i].rSize.right,WNDCONT[i].rSize.bottom,FALSE);
			ShowWindow(WNDCONT[i].hWnd,WNDCONT[i].bShow);
		}
	}

	if(g_bNormalWindowed)
	{
		SetWindowPos( g_hWnd, NULL,WNDCONT[WIN_MAIN].rSize.left,WNDCONT[WIN_MAIN].rSize.top,WNDCONT[WIN_MAIN].rSize.right - WNDCONT[WIN_MAIN].rSize.left,WNDCONT[WIN_MAIN].rSize.bottom - WNDCONT[WIN_MAIN].rSize.top,SWP_SHOWWINDOW | SWP_NOSIZE ); 
		MoveWindow(g_hWnd,WNDCONT[WIN_MAIN].rSize.left,WNDCONT[WIN_MAIN].rSize.top,WNDCONT[WIN_MAIN].rSize.right - WNDCONT[WIN_MAIN].rSize.left,WNDCONT[WIN_MAIN].rSize.bottom - WNDCONT[WIN_MAIN].rSize.top,FALSE);
		InvalidateRect(g_hWnd,NULL,TRUE);
	}

}

void Initialize_WindowSizes()
{	
	RECT rc;
	GetClientRect(g_hWnd, &rc);
	dbg_print("Initialize_WindowSizes %d, %d - %d, %d",rc.left,rc.top,rc.right,rc.bottom);

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
	
	SetRect(&WNDCONT[WIN_PROGRESSBAR].rSize,WNDCONT[WIN_STATUS].rSize.right+25,(TOOLBAR_Y_OFFSET+rc.bottom)+5,rc.right*0.4,(STATUSBAR_Y_OFFSET-5));

	g_INFOIconRect.top = WNDCONT[WIN_STATUS].rSize.top;

//	dbg_print("WIN_MAINTREEVIEW %d, %d - %d, %d",WNDCONT[WIN_MAINTREEVIEW].rSize.left,WNDCONT[WIN_MAINTREEVIEW].rSize.top,WNDCONT[WIN_MAINTREEVIEW].rSize.right,WNDCONT[WIN_MAINTREEVIEW].rSize.bottom);
//	dbg_print("WIN_SERVERLIST %d, %d - %d, %d",WNDCONT[WIN_SERVERLIST].rSize.left,WNDCONT[WIN_SERVERLIST].rSize.top,WNDCONT[WIN_SERVERLIST].rSize.right,WNDCONT[WIN_SERVERLIST].rSize.bottom);

	InvalidateRect(NULL,NULL,TRUE);
}


void Update_WindowSizes(WPARAM wParam,RECT *pRC)
{
	RECT rc;
	GetClientRect(g_hWnd, &rc);

	if(wParam == SIZE_MINIMIZED)
		return ;

	WINDOWPLACEMENT wp;
	GetWindowPlacement(g_hWnd, &wp);

	int xMax = GetSystemMetrics(SM_CXMAXIMIZED);
	int yMax = GetSystemMetrics(SM_CYMAXIMIZED);


	RECT wrc;
	GetWindowRect(g_hWnd,&wrc);

	if(wrc.left>xMax)
	{
		wrc.left = 0;
		if (wrc.right>xMax)
			wrc.right = xMax;
		if(g_bNormalWindowed)
		{
			SetWindowPos( g_hWnd, NULL,WNDCONT[WIN_MAIN].rSize.left,WNDCONT[WIN_MAIN].rSize.top,WNDCONT[WIN_MAIN].rSize.right - WNDCONT[WIN_MAIN].rSize.left,WNDCONT[WIN_MAIN].rSize.bottom - WNDCONT[WIN_MAIN].rSize.top,SWP_SHOWWINDOW | SWP_NOSIZE ); 
			MoveWindow(g_hWnd,WNDCONT[WIN_MAIN].rSize.left,WNDCONT[WIN_MAIN].rSize.top,WNDCONT[WIN_MAIN].rSize.right - WNDCONT[WIN_MAIN].rSize.left,WNDCONT[WIN_MAIN].rSize.bottom - WNDCONT[WIN_MAIN].rSize.top,FALSE);
			
		}

	}
	CopyRect(&WNDCONT[WIN_MAIN].rSize,&wrc);
	dbg_print("xMax = %d, yMax = %d",xMax,yMax);
	dbg_print("GetWindowPlacement    %d, %d - %d, %d",wp.rcNormalPosition.left,wp.rcNormalPosition.top,wp.rcNormalPosition.right,wp.rcNormalPosition.bottom);
	dbg_print("GetWindowPlacement %d   Min(%d, %d)  Max(%d, %d)",wp.showCmd,wp.ptMinPosition.x,wp.ptMinPosition.y,wp.ptMaxPosition.x,wp.ptMaxPosition.y);
	dbg_print("Update_WindowSizes CR %d, %d - %d, %d",rc.left,rc.top,rc.right,rc.bottom);
	dbg_print("Update_WindowSizes WR %d, %d - %d, %d",wrc.left,wrc.top,wrc.right-wrc.left,wrc.bottom-wrc.top);

	dbg_print("WIN_MAINTREEVIEW %d, %d - %d, %d",WNDCONT[WIN_MAINTREEVIEW].rSize.left,WNDCONT[WIN_MAINTREEVIEW].rSize.top,WNDCONT[WIN_MAINTREEVIEW].rSize.right,WNDCONT[WIN_MAINTREEVIEW].rSize.bottom);
	dbg_print("WIN_SERVERLIST %d, %d - %d, %d",WNDCONT[WIN_SERVERLIST].rSize.left,WNDCONT[WIN_SERVERLIST].rSize.top,WNDCONT[WIN_SERVERLIST].rSize.right,WNDCONT[WIN_SERVERLIST].rSize.bottom);
	dbg_print("WIN_BUDDYLIST %d, %d - %d, %d",WNDCONT[WIN_BUDDYLIST].rSize.left,WNDCONT[WIN_BUDDYLIST].rSize.top,WNDCONT[WIN_BUDDYLIST].rSize.right,WNDCONT[WIN_BUDDYLIST].rSize.bottom);
	dbg_print("------");
	
	if(rc.top <=0)
		rc.top = 0;

	if(rc.left <=0)
		rc.left = 0;

	if(rc.right <=0)
		rc.right = 300;


	if(rc.bottom <=0)
		rc.bottom = 300;



	rc.top = TOOLBAR_Y_OFFSET;
	rc.bottom-= (STATUSBAR_Y_OFFSET + TOOLBAR_Y_OFFSET); //reduce the size of statusbar from the main window size
	
	
	if(WNDCONT[WIN_BUDDYLIST].bShow==FALSE)	
		SplitterGripArea[0].tvYPos = rc.bottom * 1;
	
	if(AppCFG.bShowPlayerList==false)		
		SplitterGripArea[2].tvYPos = rc.bottom * 1;

	SetRect(&WNDCONT[WIN_MAINTREEVIEW].rSize,0,TOOLBAR_Y_OFFSET,SplitterGripArea[1].tvXPos,SplitterGripArea[0].tvYPos);
	SetWindowPos(WNDCONT[WIN_MAINTREEVIEW].hWnd,HWND_TOPMOST,0,TOOLBAR_Y_OFFSET,SplitterGripArea[1].tvXPos,SplitterGripArea[0].tvYPos,SWP_SHOWWINDOW);
	if((WNDCONT[WIN_MAINTREEVIEW].rSize.top<TOOLBAR_Y_OFFSET) || (WNDCONT[WIN_MAINTREEVIEW].rSize.bottom<TOOLBAR_Y_OFFSET+50))
	{
		WNDCONT[WIN_MAINTREEVIEW].rSize.top = TOOLBAR_Y_OFFSET;
		WNDCONT[WIN_MAINTREEVIEW].rSize.bottom = TOOLBAR_Y_OFFSET+50;
	}
	//dbg_print("<WIN_MAINTREEVIEW %d, %d - %d, %d",WNDCONT[WIN_MAINTREEVIEW].rSize.left,WNDCONT[WIN_MAINTREEVIEW].rSize.top,WNDCONT[WIN_MAINTREEVIEW].rSize.right,WNDCONT[WIN_MAINTREEVIEW].rSize.bottom);

	int offSetX = WNDCONT[WIN_MAINTREEVIEW].rSize.right + BORDER_SIZE; //get offset for next window to start at 
	int offSetY = TOOLBAR_Y_OFFSET+WNDCONT[WIN_MAINTREEVIEW].rSize.bottom + BORDER_SIZE; //get offset for next window to start at 

	SetRect(&WNDCONT[WIN_SERVERLIST].rSize,offSetX,TOOLBAR_Y_OFFSET,rc.right-(WNDCONT[WIN_MAINTREEVIEW].rSize.right+BORDER_SIZE),SplitterGripArea[2].tvYPos);
	if(WNDCONT[WIN_SERVERLIST].rSize.top<TOOLBAR_Y_OFFSET || (WNDCONT[WIN_SERVERLIST].rSize.bottom<TOOLBAR_Y_OFFSET+50))
	{
		WNDCONT[WIN_SERVERLIST].rSize.top = TOOLBAR_Y_OFFSET;
		WNDCONT[WIN_SERVERLIST].rSize.bottom = TOOLBAR_Y_OFFSET+50;
	}

	
	SetRect(&WNDCONT[WIN_BUDDYLIST].rSize,0,offSetY,offSetX-BORDER_SIZE,rc.bottom-WNDCONT[WIN_MAINTREEVIEW].rSize.bottom);
	if(WNDCONT[WIN_BUDDYLIST].rSize.top<100)
	{
		WNDCONT[WIN_BUDDYLIST].rSize.top = 100;
		WNDCONT[WIN_BUDDYLIST].rSize.bottom = 150;
	}


	float iShow = 1;
	if(WNDCONT[WIN_MAPPREVIEW].bShow)
		iShow = 0.7f;

	int offSetTabX = (WNDCONT[WIN_SERVERLIST].rSize.right * iShow) ;//- WNDCONT[WIN_MAPPREVIEW].rSize.right; //get offset for next window to start at 
	int offSetMapX = (WNDCONT[WIN_SERVERLIST].rSize.right * 0.3) ;//- WNDCONT[WIN_MAPPREVIEW].rSize.right; //get offset for next window to start at 
	int offSetY2 = WNDCONT[WIN_SERVERLIST].rSize.bottom; //get offset for next window to start at 

	offSetY = TOOLBAR_Y_OFFSET+WNDCONT[WIN_SERVERLIST].rSize.bottom + BORDER_SIZE; //get offset for next window to start at 

	SetRect(&WNDCONT[WIN_TABCONTROL].rSize,offSetX,offSetY,offSetTabX,TABSIZE_Y);
	
	SetRect(&WNDCONT[WIN_PLAYERS].rSize,offSetX,offSetY+TABSIZE_Y,offSetTabX,(rc.bottom-offSetY2)-TABSIZE_Y);
	if(WNDCONT[WIN_PLAYERS].rSize.top<100)
	{
		WNDCONT[WIN_PLAYERS].rSize.top = 100;
		WNDCONT[WIN_PLAYERS].rSize.bottom = 150;
	}
	SetRect(&WNDCONT[WIN_RULES].rSize,offSetX,offSetY+TABSIZE_Y,offSetTabX,(rc.bottom-offSetY2)-TABSIZE_Y);
	SetRect(&WNDCONT[WIN_RCON].rSize,offSetX,offSetY+TABSIZE_Y,offSetTabX,(rc.bottom-offSetY2)-TABSIZE_Y);
	SetRect(&WNDCONT[WIN_LOGGER].rSize,offSetX,offSetY+TABSIZE_Y,offSetTabX,(rc.bottom-offSetY2)-TABSIZE_Y);
	SetRect(&WNDCONT[WIN_PING].rSize,offSetX,offSetY+TABSIZE_Y,offSetTabX,(rc.bottom-offSetY2)-TABSIZE_Y);

	
	SetRect(&WNDCONT[WIN_STATUS].rSize,25,WNDCONT[WIN_BUDDYLIST].rSize.top+WNDCONT[WIN_BUDDYLIST].rSize.bottom+4,(rc.right*0.6)-25,STATUSBAR_Y_OFFSET);
	SetRect(&WNDCONT[WIN_PROGRESSBAR].rSize,
		WNDCONT[WIN_STATUS].rSize.right+25,
		(TOOLBAR_Y_OFFSET+rc.bottom)+5,
		rc.right*0.4,
		STATUSBAR_Y_OFFSET-5);

	

	int MapPreviewoffSetX = offSetX + offSetTabX + BORDER_SIZE; 

	SetRect(&WNDCONT[WIN_MAPPREVIEW].rSize,MapPreviewoffSetX,offSetY+TABSIZE_Y,rc.right - MapPreviewoffSetX,(rc.bottom-offSetY2)-TABSIZE_Y);



	g_INFOIconRect.top = WNDCONT[WIN_STATUS].rSize.top;
/*
	dbg_print("WIN_MAINTREEVIEW %d, %d - %d, %d",WNDCONT[WIN_MAINTREEVIEW].rSize.left,WNDCONT[WIN_MAINTREEVIEW].rSize.top,WNDCONT[WIN_MAINTREEVIEW].rSize.right,WNDCONT[WIN_MAINTREEVIEW].rSize.bottom);
	dbg_print("WIN_BUDDYLIST %d, %d - %d, %d",WNDCONT[WIN_BUDDYLIST].rSize.left,WNDCONT[WIN_BUDDYLIST].rSize.top,WNDCONT[WIN_BUDDYLIST].rSize.right,WNDCONT[WIN_BUDDYLIST].rSize.bottom);
	dbg_print("WIN_STATUS %d, %d - %d, %d",WNDCONT[WIN_STATUS].rSize.left,WNDCONT[WIN_STATUS].rSize.top,WNDCONT[WIN_STATUS].rSize.right,WNDCONT[WIN_STATUS].rSize.bottom);

	dbg_print("WIN_SERVERLIST %d, %d - %d, %d",WNDCONT[WIN_SERVERLIST].rSize.left,WNDCONT[WIN_SERVERLIST].rSize.top,WNDCONT[WIN_SERVERLIST].rSize.right,WNDCONT[WIN_SERVERLIST].rSize.bottom);
	dbg_print("WIN_PING %d, %d - %d, %d",WNDCONT[WIN_PING].rSize.left,WNDCONT[WIN_PING].rSize.top,WNDCONT[WIN_PING].rSize.right,WNDCONT[WIN_PING].rSize.bottom);
	dbg_print("WIN_PLAYERS %d, %d - %d, %d",WNDCONT[WIN_PLAYERS].rSize.left,WNDCONT[WIN_PLAYERS].rSize.top,WNDCONT[WIN_PLAYERS].rSize.right,WNDCONT[WIN_PLAYERS].rSize.bottom);
	dbg_print("WIN_LOGGER %d, %d - %d, %d",WNDCONT[WIN_LOGGER].rSize.left,WNDCONT[WIN_LOGGER].rSize.top,WNDCONT[WIN_LOGGER].rSize.right,WNDCONT[WIN_LOGGER].rSize.bottom);
	dbg_print("WIN_TABCONTROL %d, %d - %d, %d",WNDCONT[WIN_TABCONTROL].rSize.left,WNDCONT[WIN_TABCONTROL].rSize.top,WNDCONT[WIN_TABCONTROL].rSize.right,WNDCONT[WIN_TABCONTROL].rSize.bottom);
	dbg_print("WIN_RULES %d, %d - %d, %d",WNDCONT[WIN_RULES].rSize.left,WNDCONT[WIN_RULES].rSize.top,WNDCONT[WIN_RULES].rSize.right,WNDCONT[WIN_RULES].rSize.bottom);
	dbg_print("WIN_RCON %d, %d - %d, %d",WNDCONT[WIN_RCON].rSize.left,WNDCONT[WIN_RCON].rSize.top,WNDCONT[WIN_RCON].rSize.right,WNDCONT[WIN_RCON].rSize.bottom);

	dbg_print("\nWIN_PROGRESSBAR %d, %d - %d, %d",WNDCONT[WIN_PROGRESSBAR].rSize.left,WNDCONT[WIN_PROGRESSBAR].rSize.top,WNDCONT[WIN_PROGRESSBAR].rSize.right,WNDCONT[WIN_PROGRESSBAR].rSize.bottom);
	dbg_print("\nWIN_MAPPREVIEW %d, %d - %d, %d",WNDCONT[WIN_MAPPREVIEW].rSize.left,WNDCONT[WIN_MAPPREVIEW].rSize.top,WNDCONT[WIN_MAPPREVIEW].rSize.right,WNDCONT[WIN_MAPPREVIEW].rSize.bottom);

	dbg_print("WIN_MAIN %d, %d - %d, %d",WNDCONT[WIN_MAIN].rSize.left,WNDCONT[WIN_MAIN].rSize.top,WNDCONT[WIN_MAIN].rSize.right,WNDCONT[WIN_MAIN].rSize.bottom);
*/
}


void OnSize(HWND hwndParent,WPARAM wParam, BOOL bRepaint)
{
	RECT  rc;
	
	dbg_print("OnSize");
	GetClientRect(hwndParent, &rc);
	
	if(rc.top <=0)
		rc.bottom = 0;

	if(rc.left <=0)
		rc.left = 0;

	if(rc.bottom <=0)
		rc.bottom = 300;

	if(rc.right <=0)
		rc.right = 300;


	rc.top = TOOLBAR_Y_OFFSET;
	rc.bottom-= (STATUSBAR_Y_OFFSET + TOOLBAR_Y_OFFSET);

	g_INFOIconRect.left=2;
	
	g_INFOIconRect.bottom = STATUSBAR_Y_OFFSET;
	g_INFOIconRect.right = g_INFOIconRect.left +20;

	MoveWindow( g_hwndRibbonBar, 
				rc.left,
				rc.top ,
				rc.right,
				40,	
				bRepaint);

	Update_WindowSizes(wParam);

	for(int i=0;i<WIN_MAX;i++)
	{
		if(WNDCONT[i].idx!=WIN_MAIN)
		{
			MoveWindow(WNDCONT[i].hWnd,WNDCONT[i].rSize.left,WNDCONT[i].rSize.top,WNDCONT[i].rSize.right,WNDCONT[i].rSize.bottom,FALSE);
			ShowWindow(WNDCONT[i].hWnd,WNDCONT[i].bShow);
		}


	}
	ShowWindow(g_hwndRibbonBar,SW_SHOWNORMAL);
	
	CalcSplitterGripArea();

	
	if(WNDCONT[WIN_PING].bShow)
		InvalidateRect(WNDCONT[WIN_PING].hWnd,&WNDCONT[WIN_PING].rSize,TRUE);

	ListView_SetColumnWidth(g_hwndListBuddy,2,LVSCW_AUTOSIZE_USEHEADER);

	InvalidateRect(g_hwndSearchToolbar,NULL,TRUE);
	InvalidateRect(g_hwndSearchCombo,NULL,TRUE);
	InvalidateRect(g_hwndComboEdit,NULL,TRUE);
	InvalidateRect(g_hwndRibbonBar,NULL,TRUE);
	InvalidateRect(g_hwndToolbarOptions,NULL,TRUE);
	InvalidateRect(g_hwndSearchCombo,NULL,TRUE);
	InvalidateRect(WNDCONT[WIN_BUDDYLIST].hWnd,&WNDCONT[WIN_BUDDYLIST].rSize,TRUE);

	
}

void UpdateCurrentServerUI()
{
	if(g_CurrentSRV==NULL)
		return;
	RECT rc;
	SetRect(&rc,  WNDCONT[WIN_MAPPREVIEW].rSize.left, WNDCONT[WIN_MAPPREVIEW].rSize.top, WNDCONT[WIN_MAPPREVIEW].rSize.left+WNDCONT[WIN_MAPPREVIEW].rSize.right, WNDCONT[WIN_MAPPREVIEW].rSize.top+WNDCONT[WIN_MAPPREVIEW].rSize.bottom);
	InvalidateRect(g_hWnd,&rc, TRUE);
}


HWND FindGameWindow()
{
	HWND hwndGame=NULL;

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
	
	return hwndGame;
}

void tryToMinimizeGame()
{
	HWND hwndGame=FindGameWindow();
	
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
	int iSize = sizeof(CountryCodes)/(sizeof(CountryCodes->szCountryCode)+sizeof(CountryCodes->szCountryName));
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
	
	int iSize = sizeof(CountryCodes)/(sizeof(CountryCodes->szCountryCode)+sizeof(CountryCodes->szCountryName));

//	g_hILFlags = ImageList_Create(18, 12, ILC_COLOR24,iSize, 1);
	g_hILFlags = ImageList_Create(16, 11, ILC_COLOR24,iSize, 1);

	char szFilename[MAX_PATH+20];

	for(int i=0; i<iSize;i++)
	{
	//	sprintf(szFilename,"%s\\flags\\flag_%s.gif",EXE_PATH,CountryCodes[i].szCountryCode);
		
		sprintf(szFilename,"%s\\flags\\%s.gif",EXE_PATH,CountryCodes[i].szCountryCode);
		_strlwr_s( szFilename ,sizeof(szFilename));

		FIBITMAP *dib = FreeImage_Load(FIF_GIF,szFilename, GIF_DEFAULT);
		if(dib!=NULL)
		{
			HBITMAP hBitmap = CreateDIBitmap(hDC, FreeImage_GetInfoHeader(dib),	CBM_INIT, FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS);
			int idx = ImageList_Add(g_hILFlags,hBitmap,hBitmap);
			if(idx==-1)
				log.AddLogInfo(0,"Error adding %d (%d) flag %s - %s ",i,idx,szFilename,CountryCodes[i].szCountryName);
		
			DeleteObject(hBitmap);
			FreeImage_Unload(dib);
			
		} else
		{
			log.AddLogInfo(0,"Error loading flag %s - %s ",szFilename,CountryCodes[i].szCountryName);
		}
		
	}
	DeleteDC(hDC);
}

int LoadIconIntoImageList(char*szFilename)
{
	int index=7;
	char szTemp[100];

	SetCurrentDirectory(EXE_PATH);
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
	
	dbg_print("LoadImageList");
	HICON hIcon;

	g_hImageListStates = ImageList_Create(16, 16, ILC_COLOR32|ILC_MASK,3, 1);
		
	//dummy load for to fill index 0
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_CHECKBOX));	//9 61 Checkbox
	ImageList_AddIcon(g_hImageListStates, hIcon);
	DestroyIcon(hIcon);

	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_CHECKBOX));	//9 61 Checkbox
	ImageList_AddIcon(g_hImageListStates, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_CHECKEDBOX));	//10 62 CHECKEDBOX
	ImageList_AddIcon(g_hImageListStates, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_TRICHECKEDBOX)); //11 65 Tri Checkedbox
	ImageList_AddIcon(g_hImageListStates, hIcon);

	g_hImageListIcons = ImageList_Create(16, 16, ILC_COLOR32|ILC_MASK,35, 1);

	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_USERS));		    //0 
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_PB));			//1
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_FAVORITES));   //2
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_BUDDY));		//3
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_LOCKED));		//4
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_ERROR));		//5
	ImageList_AddIcon(g_hImageListIcons, hIcon); 
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_WARNING));		//6
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_UNKOWN));		//7
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(ICON_INFO));			//8
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_CHECKBOX));	//9 61 Checkbox
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_CHECKEDBOX));	//10 62 CHECKEDBOX
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_TRICHECKEDBOX)); //11 65 Tri Checkedbox
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_GLOBE));		//12 57 Globe
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_LAN));			//13 58 Lan
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_RULES));	//14 59 App logo
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_FILTER));		//15 60 Filter
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_HISTORY));		//16 64 History
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_FILTER2));	   // 17 82
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_FOLDER));	  //18 75
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_RANKED)); //19 76  Ranked
	ImageList_AddIcon(g_hImageListIcons, hIcon);		
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_BLANK)); //20 77
	ImageList_AddIcon(g_hImageListIcons, hIcon);		
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_VAC)); //21
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_RCON)); //22
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_LOGGER)); //23
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_FONT)); //24
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_PAINT)); //25
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_STATS)); //26
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_RULES)); //27
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_MIRC)); //28
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_ALARM_CLOCK)); //29
	ImageList_AddIcon(g_hImageListIcons, hIcon);
	DestroyIcon(hIcon);
}


void SetImageList()
{
	SendMessage(g_hwndListViewServer, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)g_hImageListIcons);
	SendMessage(g_hwndListBuddy, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)g_hImageListIcons);	
	SendMessage(g_hwndMainTreeCtrl, TVM_SETIMAGELIST , TVSIL_NORMAL, (LPARAM)g_hImageListIcons);
	SendMessage(g_hwndMainTreeCtrl, TVM_SETIMAGELIST , TVSIL_STATE, (LPARAM)g_hImageListStates);
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
	
		TreeView_SelectItem(g_hwndMainTreeCtrl,tvmgr.vTI.at(n).hTreeItem);
		TreeView_SetItemState(g_hwndMainTreeCtrl,tvmgr.vTI.at(n).hTreeItem,TVIS_SELECTED, TVIS_SELECTED);
		tvmgr.OnSelection((LPARAM)lpParam);
		Sleep(1000);
	}

	return 0;
}


DWORD WINAPI GetServerList(LPVOID lpParam )
{
	DWORD options = (DWORD)lpParam;
	DWORD returnCode=0;
	int currGameIdx = g_currentGameIdx;

	Show_StopScanningButton(TRUE);
	ListView_DeleteAllItems(g_hwndListViewVars);
	ListView_DeleteAllItems(g_hwndListViewPlayers);
	ListView_DeleteAllItems(g_hwndListViewServer);

	if(g_bRunningQueryServerList || (gm.GamesInfo[currGameIdx].bLockServerList == TRUE))	
		return 0;

	g_currentScanGameIdx = g_currentGameIdx;
	g_bRunningQueryServerList = true;
	g_bPlayedNotify = false;

	while(1)
	{
		if(TryEnterCriticalSection(&REDRAWLIST_CS)==TRUE)
			break;

		dbg_print("REDRAWLIST_CS is busy!");
		Sleep(500);
	}

	LeaveCriticalSection(&REDRAWLIST_CS);

	if (! ResetEvent(hCloseEvent) ) 
        dbg_print("ResetEvent failed\n");

	HANDLE hThread=NULL; 
	DWORD dwThreadIdBrowser;
		
	if(options==SCAN_ALL_GAMES)
	{
		g_bRunSimulation = TRUE;
		hThread = CreateThread( NULL, 0, &Simulation, (LPVOID)0,0, &dwThreadIdBrowser);                
		if (hThread == NULL) 
		{
			log.AddLogInfo(GS_LOG_WARNING, "CreateThread failed (%d)\n", GetLastError() ); 
		}
		else 
		{
			SetThreadName( dwThreadIdBrowser, "GetServerListThread");
			CloseHandle( hThread );
		}
	}
	DWORD dwStartTick = GetTickCount();
	UINT iGame=0;
nextGame:
	if(options==SCAN_ALL_GAMES)
		currGameIdx = iGame++;
	if(iGame==gm.GamesInfo.size())  //reset
		currGameIdx = iGame = 0;

	int nMasterServer=0;

	SCAN_Set_CALLBACKS(gm.GamesInfo[currGameIdx].GetServerStatus,&UpdateServerItem);

	//Enumurate all master servers
nextMasterServer:
	SetStatusText(gm.GamesInfo[currGameIdx].iIconIndex,g_lang.GetString("StatusReceivingServers"),gm.GamesInfo[currGameIdx].szGAME_NAME);	

	if((DWORD)lpParam==SCAN_FILTERED)
		Initialize_Rescan2(&gm.GamesInfo[currGameIdx],&FilterServerItemV2);
	else
	{
		if(gm.GamesInfo[currGameIdx].bUseHTTPServerList[nMasterServer])
		{
			g_download.SetPath(USER_SAVE_PATH);
			int ret = g_download.HttpFileDownload(gm.GamesInfo[currGameIdx].szMasterServerIP[nMasterServer],"servers.txt",NULL,NULL);
			if(ret!=0)
				goto exitError;
			Parse_FileServerList(&gm.GamesInfo[currGameIdx],"servers.txt");
		}else	
		{	
			if(gm.GamesInfo[currGameIdx].GetServersFromMasterServer!=NULL)
				gm.GamesInfo[currGameIdx].GetServersFromMasterServer(&gm.GamesInfo[currGameIdx],nMasterServer);
			else
			{
				log.AddLogInfo(GS_LOG_ERROR,"%s didn't have a valid funtion to recieve servers.",gm.GamesInfo[currGameIdx].szGAME_NAME);
				goto exitError;
			}
		}
		Initialize_RedrawServerListThread();

		nMasterServer++;
		if(nMasterServer<gm.GamesInfo[currGameIdx].nMasterServers)
			goto nextMasterServer;

		Initialize_Rescan2(&gm.GamesInfo[currGameIdx],NULL);

	}
	


	char szBuffer[100];
	sprintf(szBuffer,"%s (%d)",gm.GamesInfo[currGameIdx].szGAME_NAME,gm.GamesInfo[currGameIdx].dwTotalServers);
	tvmgr.SetItemText(gm.GamesInfo[currGameIdx].hTI,szBuffer);	
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
		log.AddLogInfo(GS_LOG_DEBUG,  "GetServerList was breaked by user.");
		Show_StopScanningButton(FALSE);
		g_currentScanGameIdx = -1;
		g_bRunSimulation = FALSE;
		return 0xFFFF;
	}
	if(gm.GamesInfo[currGameIdx].vSI.size()==0)
		goto exitError;

	SetStatusText(ICO_INFO,g_lang.GetString("StatusReceivingServersDone"),gm.GamesInfo[currGameIdx].szGAME_NAME);
	DWORD dwEndTick = GetTickCount();
	log.AddLogInfo(ICO_INFO,"GetServerList took %d sec",(dwEndTick-dwStartTick)/1000);

	//We don't want to overdraw wrong serverlist
	if(currGameIdx==g_currentGameIdx)
	{
		g_currentScanGameIdx = -1;
		Initialize_RedrawServerListThread();		
	}

	g_bCancel = false;

	//ListView_SortItems(g_hwndListViewServer,MyCompareFunc,COL_NUMPLAYERS);
	//ListView_SetColumnWidth(g_hwndListViewServer,0,LVSCW_AUTOSIZE);
	//ListView_SetColumnWidth(g_hwndListViewServer,1,LVSCW_AUTOSIZE);
	//ListView_SetColumnWidth(g_hwndListViewServer,2,LVSCW_AUTOSIZE);
//	ListView_SetColumnWidth(g_hwndListViewServer,3,LVSCW_AUTOSIZE);
//	ListView_SetColumnWidth(g_hwndListViewServer,4,LVSCW_AUTOSIZE);
	//ListView_SetColumnWidth(g_hwndListViewServer,6,LVSCW_AUTOSIZE);

	//temp fix
	AppCFG.bSortBuddyAsc = !AppCFG.bSortBuddyAsc;
	sort(bm.BuddyList.begin(),bm.BuddyList.end(),Buddy_Sort_ServerName);
	bm.UpdateList();
	goto NoError;

exitError:
	SetStatusText(ICO_WARNING,g_lang.GetString("StatusReceivingServersError"),gm.GamesInfo[currGameIdx].szGAME_NAME);
	returnCode = 0xdead;
NoError:
	time(&gm.GamesInfo[currGameIdx].lastScanTimeStamp);


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
 //  log.AddLogInfo(GS_LOG_DEBUG,  "GetServerList DONE!");
  return returnCode;
 }

bool FilterServerItemV2(SERVER_INFO *srv,GAME_INFO *pGI, vFILTER_SETS *vFilterSets)
{
	DWORD dwFilterFlags = pGI->dwViewFlags;
	DWORD dwReason=0;
	DWORD bForceFavorites = (dwFilterFlags & REDRAWLIST_FAVORITES_PUBLIC);
	DWORD bForceHistory = (dwFilterFlags & REDRAWLIST_HISTORY) ;
	DWORD bForceFavoritesPrivate = dwFilterFlags & REDRAWLIST_FAVORITES_PRIVATE;
	DWORD bRescanFilter = dwFilterFlags & FORCE_SCAN_FILTERED;
	DWORD bWeAreScanning = dwFilterFlags & SCAN_SERVERLIST;
	DWORD bCustomFilterOnly = dwFilterFlags & REDRAWLIST_CUSTOM_FILTER;

	bool returnVal=false;
	if(pGI->dwViewFlags & REDRAW_SERVERLIST)
		bRescanFilter = 0; //force filter when only redrawing the serverlist looks better for user

	if(dwFilterFlags & REDRAW_MONITOR_SERVERS)
	{
		if(srv->wMonitor>0)
			return true;
		
		return false;
	}
	

	time_t currTime;
	time(&currTime);
	
	double seconds = difftime(currTime,srv->timeLastScan);
	
	if((seconds<(60*2)) && bWeAreScanning)
		return false;
				

	returnVal=false;

	if(tvmgr.CountryFilter.counter!=0)
	{
		for(int i=0; i<tvmgr.CountryFilter.counter;i++)
		{
			if(_stricmp(srv->szShortCountryName,tvmgr.CountryFilter.szShortCountryName[i])==0)
			{
				returnVal=true;
				break;
			} 
		}
		if(returnVal==false)
			return false;
	}
	else
		returnVal=true;

	//If scanning filtered (limited number of servers) ignore following filter options:
	if((bRescanFilter==0))
	{
	
		if(AppCFG.filter.bHideOfflineServers && (srv->dwPing==9999))
		{
			if(srv->bUpdated && (bWeAreScanning))
				returnVal=true;
			else
				return false;
		}

		if((srv->cPurge>=SERVER_PURGE_COUNTER) && (srv->cFavorite==0))
			return false;

		if(bForceHistory)
			if(srv->cHistory==0)
				return false;

		BOOL bFilterReturn=TRUE;
		if(bCustomFilterOnly)
			bFilterReturn = se.Execute(srv,pGI,&pGI->vFilterSetsFavorites);
		else
			bFilterReturn = se.Execute(srv,pGI,vFilterSets);
		
		if(bFilterReturn==FALSE)
			return false;

		
		if(pGI->filter.bNoBots)
			if(srv->cBots>0)
				return false;


		if(pGI->filter.bRanked && (srv->cGAMEINDEX == ETQW_SERVERLIST))
			if(srv->cRanked==0)
				return false;

	}

	return returnVal;
}


DWORD WINAPI  RedrawServerListThread(LPVOID pvoid )
{
	if(TryEnterCriticalSection(&REDRAWLIST_CS)==FALSE)
		return 0xDEADBEEF;

	int gameIdx = (int)pvoid;

	GAME_INFO *pGI = &gm.GamesInfo[gameIdx];
	
	pGI->dwViewFlags |= REDRAW_SERVERLIST;
	
	vFILTER_SETS vFS;
	vFS = pGI->vFilterSets;
	vFS.insert(vFS.end(),gm.GetFilterSet(GLOBAL_FILTER).begin(),gm.GetFilterSet(GLOBAL_FILTER).end());
	sort(vFS.begin(),vFS.end(),Sort_Filter_By_GroupName);

	ListView_DeleteAllItems(g_hwndListViewServer);
	pGI->vRefListSI.clear();
	int nSize = pGI->vSI.size();
	UINT idx = nSize;
	if(nSize!=0)
	{
		REF_SERVER_INFO refSI;
		while(idx!=0)
		{
			idx--;
			SERVER_INFO *pSI=NULL;
			pSI = pGI->vSI.at(idx);
			GetServerLock(pSI);
			refSI.pServerInfo = pSI;

			if(FilterServerItemV2(pSI,pGI,&vFS))
				pGI->vRefListSI.push_back(refSI);

			ReleaseServerLock(pSI);
		
		}			
	}
	LeaveCriticalSection(&REDRAWLIST_CS);

	Do_ServerListSort((LPVOID)iLastColumnSortIndex);

	if(pGI->dwViewFlags & REDRAW_SERVERLIST)
		pGI->dwViewFlags ^= REDRAW_SERVERLIST;


	if(pGI->dwViewFlags & FORCE_SCAN_FILTERED)
		pGI->dwViewFlags ^= FORCE_SCAN_FILTERED;

	dbg_print("Created filtered serverlist! View flags %d\n Number of servers in list %d \n",pGI->dwViewFlags,pGI->vRefListSI.size());

	SetStatusText(ICO_INFO,g_lang.GetString("StatusNumVisible"),pGI->szGAME_NAME,pGI->vRefListSI.size(),pGI->vSI.size());
	return 0;
}

//Try to avoid use this func
void Initialize_RedrawServerListThread()
{
	HANDLE hThread;
	DWORD dwThreadIdBrowser;

	if(g_currentGameIdx==-1)
		return;

	dbg_print("Creating RedrawServerListThread thread!\n");
	hThread = NULL;		
	int gameIdx = g_currentGameIdx;
	hThread = CreateThread( NULL, 0, &RedrawServerListThread, (LPVOID)gameIdx,0, &dwThreadIdBrowser);                
	if (hThread == NULL) 
	{
		log.AddLogInfo(GS_LOG_WARNING,"CreateThread failed @ Initialize_RedrawServerListThread (%d)\n", GetLastError() ); 
	}
	else 
	{
		CloseHandle( hThread );
	}
}


void Parse_FileServerList(GAME_INFO *pGI,char *szFilename)
{

	SetCurrentDirectory(USER_SAVE_PATH);
	FILE *fp=fopen(szFilename, "rb");
	int i=0;
	char buff[256];
	DWORD dwPort=0;
	char *pszIP;
	
	if(fp!=NULL)
	{
		while(!feof(fp))
		{		
		//	ZeroMemory(buff, sizeof(buff));
			char *pout = fgets( buff, sizeof(buff), fp );
			if(pout!=NULL)
			{
				pszIP = SplitIPandPORT((char*)&buff,dwPort);					
				DWORD dwRet = AddServer(pGI,pszIP,dwPort,false);
				if(dwRet!=0xFFFFFFFF)
				{
					i++;
					SetStatusText(pGI->iIconIndex,g_lang.GetString("StatusReceivingMaster"),i,pGI->szGAME_NAME);
				}
			}

		}
		fclose(fp);
	} 	
	pGI->dwTotalServers = pGI->vSI.size();
}



void Parse_FileServerListFromGSC(GAME_INFO *pGI,char *szFilename)
{

	SetCurrentDirectory(USER_SAVE_PATH);
	FILE *fp=fopen(szFilename, "rb");
	int i=0;
	char buff[256];
	DWORD dwPort=0;
	char *pszIP;
	char seps[] ={","};
	if(fp!=NULL)
	{
		char *pout = fgets( buff, sizeof(buff), fp ); //skip first row
		while(!feof(fp))
		{		
		//	ZeroMemory(buff, sizeof(buff));
			char *pout = fgets( buff, sizeof(buff), fp );
			if(pout!=NULL)
			{
				
				char *next_token=NULL;
				char *pszIP = strtok_s( buff, seps, &next_token);
				char *port =  strtok_s( NULL, seps, &next_token);
				
				if(port!=NULL)
				{
					dwPort = atoi(port);

					DWORD dwRet = AddServer(pGI,pszIP,dwPort,false);
					if(dwRet!=0xFFFFFFFF)
					{
						i++;
						SetStatusText(pGI->iIconIndex,g_lang.GetString("StatusReceivingMaster"),i,pGI->szGAME_NAME);
					}
				}
			}

		}
		fclose(fp);
	} 	
	pGI->dwTotalServers = pGI->vSI.size();
}


DWORD GSC_ConnectToMasterServer(GAME_INFO *pGI, int iMasterIdx)
{
	CInternet inet;
	
	inet.SetPath(USER_SAVE_PATH);
	inet.CrackURL(pGI->szMasterServerIP[iMasterIdx]);
	inet.URLPost(pGI->szMasterServerIP[iMasterIdx], "servers.csv");
	Parse_FileServerListFromGSC(pGI,"servers.csv");
	return 0;
}


//This is only for reference purpose
long InsertServerItem(GAME_INFO *pGI,SERVER_INFO *pSI)
{
	LVITEM lvItem;
	ZeroMemory(&lvItem, sizeof(LVITEM));

	if(pSI->cGAMEINDEX != g_currentGameIdx)
	{
		//dbg_print("wrong RefList");
		return 2;
	}

	lvItem.iItem =  ListView_GetItemCount(g_hwndListViewServer);
	
	REF_SERVER_INFO refSI;
	//refSI.cGAMEINDEX  = pSI->cGAMEINDEX;
	//refSI.dwIndex = pSI->dwIndex;	
	refSI.pServerInfo = pSI;
	if(TryEnterCriticalSection(&REDRAWLIST_CS)==FALSE)
	{
		dbg_print("busy RefList");
		return 1;
	}
	pGI->vRefListSI.push_back(refSI);
	LeaveCriticalSection(&REDRAWLIST_CS);

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

//	sprintf(portstr, "%d", (int)port);
	
	_itoa_s(port,portstr,10);

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

#define FONT_MAX_WIDTH 16

LRESULT Draw_ColorEncodedText(RECT rc, LPNMLVCUSTOMDRAW pListDraw , char *pszText)
{
	HDC  hDC =  pListDraw->nmcd.hdc;
	HBRUSH  hbrSel = CreateSolidBrush( RGB(0x28,0x2c,0x28)); 														
	FillRect(hDC, &rc, (HBRUSH) hbrSel);
	if(hbrSel!=NULL)
		DeleteObject(hbrSel);
	if( pListDraw->nmcd.uItemState & CDIS_SELECTED) // (CDIS_SELECTED | CDIS_FOCUS))
	{
		HBRUSH hbrSel2= NULL;
		pListDraw->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT); //RGB(255, 255, 255);
		hbrSel2 = CreateSolidBrush( GetSysColor(COLOR_HIGHLIGHT)); //RGB(51,153,250)); 																
		FillRect(hDC, &rc, (HBRUSH) hbrSel2);
		if(hbrSel2!=NULL)
			DeleteObject(hbrSel2);
	}
	if(pszText==NULL)
	{			
		SelectObject(hDC,g_hf);	
		return (CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT );
	}
	BYTE ncharWidth;
	SelectObject(hDC,g_hf2);		
	ABC abc[256];
	LPABC pAbc = abc;
	GetCharABCWidths(hDC,(UINT)0, (UINT) 255,pAbc);

	char *pText;

	rc.left+=20;
//	SetBkColor(hDC, RGB(0x28,0x2c,0x28));
//	SetTextColor(hDC,pListDraw->clrText);
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
		ExtTextOut(hDC,rc.left,rc.top,ETO_CLIPPED , &rc,pText, 1,NULL); 
	
		ncharWidth = abc[(unsigned char)pszText[i]].abcA + abc[(unsigned char)pszText[i]].abcB + abc[(unsigned char)pszText[i]].abcC;
		if(ncharWidth>FONT_MAX_WIDTH)
			ncharWidth = FONT_MAX_WIDTH;
		rc.left+=ncharWidth;

	}
	SelectObject(hDC,g_hf);	
	return  (CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT);
}

LRESULT Draw_ColorEncodedTextUNICODE(RECT rc, LPNMLVCUSTOMDRAW pListDraw , char *pszText)
{
	HDC  hDC =  pListDraw->nmcd.hdc;
	if(pszText==NULL)
		return (CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT );
	HBRUSH hbrSel= NULL;
	hbrSel = CreateSolidBrush( RGB(0x28,0x2c,0x28)); 														
	FillRect(hDC, &rc, (HBRUSH) hbrSel);

	if( pListDraw->nmcd.uItemState & CDIS_SELECTED) 
	{
		HBRUSH hbrSel2= NULL;
		pListDraw->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT); //RGB(255, 255, 255);
		hbrSel2 = CreateSolidBrush( GetSysColor(COLOR_HIGHLIGHT)); //RGB(51,153,250)); 																
		FillRect(hDC, &rc, (HBRUSH) hbrSel2);
		if(hbrSel2!=NULL)
			DeleteObject(hbrSel2);
	}

	SelectObject(hDC,g_hf2);
			
	ABC abc[256];
	LPABC pAbc = abc;
	GetCharABCWidths(hDC,       (UINT)0, (UINT) 255,pAbc);


	char buffer[512];
	ZeroMemory(buffer,512);
	int ret = wcstombs((char*)buffer,(WCHAR*)pszText,512);


	rc.left+=20;
	//rc.top+=2;
	COLORREF col = RGB(255,255,255) ;
		
	SetTextColor(hDC,col);
	if(ret==0)
		ExtTextOut(hDC,rc.left,rc.top,ETO_CLIPPED, &rc,pszText, strlen(pszText),NULL); 
	else		
		ExtTextOutW(hDC,rc.left,rc.top,ETO_CLIPPED, &rc,(LPCWSTR)buffer, wcslen((LPCWSTR)buffer),NULL); 


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

	if( pListDraw->nmcd.uItemState & CDIS_SELECTED) // (CDIS_SELECTED | CDIS_FOCUS))
	{
		HBRUSH hbrSel2= NULL;
		pListDraw->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT); //RGB(255, 255, 255);
		hbrSel2 = CreateSolidBrush( GetSysColor(COLOR_HIGHLIGHT)); //RGB(51,153,250)); 																
		FillRect(hDC, &rc, (HBRUSH) hbrSel2);
		if(hbrSel2!=NULL)
			DeleteObject(hbrSel2);;
	}
	if(pszText==NULL)
	{	
		if(hbrSel!=NULL)
			DeleteObject(hbrSel);
		SelectObject(hDC,g_hf);	
		return (CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT );
	}

	int ncharWidth;
	SelectObject(hDC,g_hf2);	
	ABC abc[256];
	LPABC pAbc = abc;
	GetCharABCWidths(hDC,(UINT)0, (UINT) 255,pAbc);  //supports true type font
	char *pText;
	rc.left+=20;
	rc.top+=2;
	COLORREF col = RGB(255,255,255) ;

	for(int i=0;i<strlen(pszText);i++)
	{
		if((pszText[i]=='^') && (pszText[i+1]=='i') )//Q4 ^idm0 and ^idm1 icons filter
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

		ExtTextOut(hDC,rc.left,rc.top,ETO_CLIPPED, &rc,pText, 1,NULL); 
		
	    ncharWidth = abc[(unsigned char)pszText[i]].abcA + abc[(unsigned char)pszText[i]].abcB + abc[(unsigned char)pszText[i]].abcC;
		if(ncharWidth>FONT_MAX_WIDTH)
			ncharWidth = FONT_MAX_WIDTH;
		rc.left+=ncharWidth;

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
	
	if( pListDraw->nmcd.uItemState & CDIS_SELECTED) 
	{
		HBRUSH hbrSel2= NULL;
		pListDraw->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT); //RGB(255, 255, 255);
		hbrSel2 = CreateSolidBrush( GetSysColor(COLOR_HIGHLIGHT)); //RGB(51,153,250)); 																
		FillRect(hDC, &rc, (HBRUSH) hbrSel2);
		if(hbrSel2!=NULL)
			DeleteObject(hbrSel2);
	}
	if(pszText==NULL)
	{	
		if(hbrSel!=NULL)
			DeleteObject(hbrSel);
		SelectObject(hDC,g_hf);	
		return (CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT );
	}

	int ncharWidth;
	SelectObject(hDC,g_hf2);			
	ABC abc[256];
	LPABC pAbc = abc;
	GetCharABCWidths(hDC,       (UINT)0, (UINT) 255,pAbc);
	char *pText;
	rc.left+=20;
	rc.top+=2;
	unsigned char uc[2];
	uc[1]=0;

	for(int i=0;i<strlen(pszText);i++)
	{
		COLORREF col = RGB(255,255,255) ;
		uc[0] = (unsigned char)QuakeWorldASCII[(unsigned char)pszText[i]];

		if(pszText[i]<'!')
			col = RGB(166,103,42);
		else if(pszText[i]>='!' && pszText[i]<=160)
			col = RGB(109,109,109);
		else if(pszText[i]>160 && pszText[i]<=176)	
			col = RGB(166,103,42);
		else if(pszText[i]>176)
			col = RGB(111,77,52);
	
		SetTextColor(hDC,col);
		pText = (char*)&uc[0];
		ExtTextOut(hDC,rc.left,rc.top,ETO_CLIPPED, &rc,pText, 1,NULL); 
		ncharWidth = (abc[uc[0]].abcA + abc[uc[0]].abcB + abc[uc[0]].abcC) ;

		if(ncharWidth>FONT_MAX_WIDTH)
			ncharWidth = FONT_MAX_WIDTH;
		rc.left+=ncharWidth;

	}
	SelectObject(hDC,g_hf);								
	if(hbrSel!=NULL)
		DeleteObject(hbrSel);

	return  (CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT );
}

int charHexToDec(char c)
{
	if(tolower(c)>=97)
		return tolower(c)-87;
	else
		return c-48;
}

LRESULT Draw_ColorEncodedTextNexuiz(RECT rc, LPNMLVCUSTOMDRAW pListDraw , char *pszText)
{
	HDC  hDC =  pListDraw->nmcd.hdc;
	HBRUSH hbrSel= NULL;
	if(pszText==NULL)
		return (CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT );
	
	hbrSel = CreateSolidBrush( RGB(0x28,0x2c,0x28)); 														
	FillRect(hDC, &rc, (HBRUSH) hbrSel);
	
	if( pListDraw->nmcd.uItemState & CDIS_SELECTED) 
	{
		pListDraw->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT); //RGB(255, 255, 255);
		hbrSel = CreateSolidBrush( GetSysColor(COLOR_HIGHLIGHT)); //RGB(51,153,250)); 																
		FillRect(hDC, &rc, (HBRUSH) hbrSel);
	}
	int ncharWidth;
	SelectObject(hDC,g_hf2);
	//GecharWidth32(hDC, (UINT) 0, (UINT) 0, &ncharWidth); 				
	ABC abc[256];
	LPABC pAbc = abc;
	GetCharABCWidths(hDC,       (UINT)0, (UINT) 255,pAbc);
	char *pText;
	rc.left+=20;
	rc.top+=2;
	unsigned char uc[2];
	uc[1]=0;
	COLORREF col = RGB(255,255,255) ;
	for(int i=0;i<strlen(pszText);i++)
	{
	
		uc[0] = (unsigned char)NexuizASCII[(unsigned char)pszText[i]];
		if(pszText[i]=='^')
		{
			if(pszText[i+1]=='x')//^xFFF or 
			{

				int r = charHexToDec(pszText[i+2]);
				int g = charHexToDec(pszText[i+3]);
				int b = charHexToDec(pszText[i+4]);
				col = RGB(17*r,17*g,17*b);
				i+=4;
				continue;
			}else
			{
				col = GetColor(pszText[i+1]);
			}
			i++;
			if(pszText[i]!='^') // this fixes these kind of names with double ^^
				continue;
		} 	
		SetTextColor(hDC,col);
		pText = (char*)&uc[0];
		ExtTextOut(hDC,rc.left,rc.top,ETO_CLIPPED, &rc,pText, 1,NULL); 
		ncharWidth = abc[uc[0]].abcA + abc[uc[0]].abcB + abc[uc[0]].abcC;
		if(ncharWidth>FONT_MAX_WIDTH)
			ncharWidth = FONT_MAX_WIDTH;
		rc.left+=ncharWidth;

	}
	SelectObject(hDC,g_hf);								
	if(hbrSel!=NULL)
		DeleteObject(hbrSel);

	return  (CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT );
}

LRESULT Draw_UTF8Text(RECT rc, LPNMLVCUSTOMDRAW pListDraw , char *pszText)
{
	HDC  hDC =  pListDraw->nmcd.hdc;
	HBRUSH hbrSel= NULL;
	if(pszText==NULL)
		return (CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT );
	
	hbrSel = CreateSolidBrush( RGB(0x28,0x2c,0x28)); 														
	FillRect(hDC, &rc, (HBRUSH) hbrSel);

	if( pListDraw->nmcd.uItemState & CDIS_SELECTED) 
	{
		pListDraw->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT); //RGB(255, 255, 255);
		hbrSel = CreateSolidBrush( GetSysColor(COLOR_HIGHLIGHT)); //RGB(51,153,250)); 																
		FillRect(hDC, &rc, (HBRUSH) hbrSel);
	}

	SelectObject(hDC,g_hf2);
	
	rc.left+=20;
	rc.top+=2;

	COLORREF col = RGB(255,255,255) ;

	UTF8toMB(pszText,pszText); //Counter strike source

	SetTextColor(hDC,col);
	ExtTextOut(hDC,rc.left,rc.top,ETO_CLIPPED, &rc,pszText, strlen(pszText),NULL); 

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

	int percentage =  ((float)xCurrentValue /  (float)xMaxValue) * 100.0f ;
	if(percentage>100.0f)
		percentage = 100;

	if(percentage<0)  //can't be less than zero
		percentage = 0;
	
	Rectangle(hDC, rect.left, rect.top-1,rect.right-1, rect.bottom+1); 
	rect.left++;
	int maxPixel = (rect.right-1) - (rect.left);
	int pixelX = (percentage * maxPixel) / 100;
	pixelX--;
	rect.right = rect.left + pixelX; 

	
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
	SERVER_INFO *pSI = NULL;
	int    nItem = static_cast<int>( pListDraw->nmcd.dwItemSpec );
	switch(pListDraw->nmcd.dwDrawStage)
	{
		
		case CDDS_PREPAINT:		
			lResult= (CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYSUBITEMDRAW);
			break;
		case CDDS_ITEMPREPAINT:
			{				
				if(nItem%2 == 0)
					pListDraw->clrTextBk = RGB(202, 221,250);
				else
					pListDraw->clrTextBk = RGB(255, 255,255);
	
				if(pListDraw->nmcd.hdr.idFrom == IDC_LIST_SERVER)
				{		
					pSI = Get_ServerInfoByListViewIndex(currCV,nItem); 
					GetServerLock(pSI);
					if(pSI != NULL) //Quick and dirty fix if server index is out of range
					{
						if(pSI->bUpdated==false)
							pListDraw->clrText   = RGB(140, 140, 140);
						else
							pListDraw->clrText   = RGB(0, 0, 0);
						
						if(pSI->dwPing==9999)
							pListDraw->clrText   = RGB(255, 0, 0);
					}					
				}
			}
			lResult = (  CDRF_NOTIFYSUBITEMDRAW );
			break;
		
		case (CDDS_ITEMPREPAINT|CDDS_SUBITEM):
			{
				pListDraw->clrText   = RGB(0, 0, 0);	

				if(pListDraw->nmcd.hdr.idFrom == IDC_LIST_SERVER)
				{
					pSI = Get_ServerInfoByListViewIndex(currCV,nItem); 
					GetServerLock(pSI);					
					
					if(pSI == NULL) //Quick and dirty fix if server index is out of range
						break;
															
						//do some default stuff
						if(pSI->dwPing==9999)
							pListDraw->clrText   = RGB(255, 0, 0);
						else
						{
							if(pSI->bUpdated==false)
								pListDraw->clrText   = RGB(140, 140, 140);
							else
								pListDraw->clrText   = RGB(0, 0, 0);
						}
						HDC  hDC =  pListDraw->nmcd.hdc;							
						RECT rc;
						HBRUSH hbrBkgnd=NULL;
						HBRUSH hbrBkgnd2=NULL;
						iRow = (int)pListDraw->nmcd.dwItemSpec;	

						ListView_GetSubItemRect(g_hwndListViewServer,nItem,pListDraw->iSubItem,LVIR_BOUNDS,&rc);


						if(pListDraw->iSubItem==g_LVHeaderSL->GetColumnIdx(COL_PLAYERS))
						{
							char szText[50];
							sprintf_s(szText,sizeof(szText),"%d/%d+(%d)",pSI->nPlayers,pSI->nMaxPlayers,pSI->nPrivateClients);
							
							hbrBkgnd = CreateSolidBrush(RGB(202, 221,250)); 
							hbrBkgnd2 = CreateSolidBrush(RGB(255, 255,255)); 

							if(pListDraw->iSubItem==0)
								ListView_GetSubItemRect(g_hwndListViewServer,nItem,pListDraw->iSubItem,LVIR_LABEL,&rc);

							if(iRow%2 == 0)
							{
							//	pListDraw->clrTextBk = RGB(202, 221,250);
								FillRect(hDC, &rc, (HBRUSH) hbrBkgnd);
							}else
							{
								FillRect(hDC, &rc, (HBRUSH) hbrBkgnd2);
							}

							Draw_ShadedRect(hDC,rc,szText,pSI->nPlayers,pSI->nMaxPlayers);

							if(hbrBkgnd2!=NULL)	
								DeleteObject(hbrBkgnd2);

							if(hbrBkgnd!=NULL)
								DeleteObject(hbrBkgnd);

							if(iRow%2 == 0)
								pListDraw->clrTextBk = RGB(202, 221,250);
							else
								pListDraw->clrTextBk = RGB(255, 255,255);


							lResult =  (CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT);
							if(pListDraw->iSubItem==0)
								lResult=  CDRF_SKIPDEFAULT; //(CDRF_NEWFONT | CDRF_NOTIFYPOSTPAINT | CDRF_SKIPDEFAULT); 
							break;
						}
						else if(pListDraw->iSubItem==g_LVHeaderSL->GetColumnIdx(COL_COUNTRY))
						{
							HBRUSH hbrSel=NULL;							
							int idxCC = Get_CountryFlagByShortName(pSI->szShortCountryName);
											

							COLORREF colRef;
							if(iRow%2 == 0)
								colRef = RGB(202, 221,250);
							else
								colRef = RGB(255, 255,255);
							
							hbrBkgnd = CreateSolidBrush(colRef); 	
							FillRect(hDC, &rc, (HBRUSH) hbrBkgnd);
					
							colRef = pListDraw->clrTextBk ;
							//dbg_print("state %d",pListDraw->nmcd.uItemState );
							if(pListDraw->nmcd.uItemState & ( CDIS_SELECTED |CDIS_FOCUS))
							{
								pListDraw->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT);
								pListDraw->clrTextBk = GetSysColor(COLOR_HIGHLIGHT);
							}
							ImageList_Draw(g_hILFlags,idxCC,hDC,rc.left+1,rc.top+2,ILD_NORMAL);

							SetBkColor(hDC,pListDraw->clrTextBk);
							SetTextColor(hDC,pListDraw->clrText);								
							rc.left+=20;
							//rc.top+=2;
							
							if(AppCFG.bUseShortCountry)
								ExtTextOut(hDC,rc.left,rc.top,ETO_CLIPPED | ETO_OPAQUE, &rc,CountryCodes[idxCC].szCountryCode, strlen(CountryCodes[idxCC].szCountryCode),NULL); 
							else
								ExtTextOut(hDC,rc.left,rc.top,ETO_CLIPPED | ETO_OPAQUE, &rc,CountryCodes[idxCC].szCountryName, strlen(CountryCodes[idxCC].szCountryName),NULL); 
											
							if(hbrBkgnd!=NULL)	
								DeleteObject(hbrBkgnd);
							if(hbrSel!=NULL)
								DeleteObject(hbrSel);

							if(iRow%2 == 0)
								colRef = RGB(202, 221,250);
							else
								colRef = RGB(255, 255,255);

							pListDraw->clrTextBk   = colRef;
							pListDraw->clrText   = GetSysColor(COLOR_MENUTEXT);
							lResult = (CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT );
							break;
						}
						else if(pListDraw->iSubItem==g_LVHeaderSL->GetColumnIdx(COL_SERVERNAME))
						{
							if(AppCFG.bUseColorEncodedFont)
							{
								HDC  hDC =  pListDraw->nmcd.hdc;
								RECT rc;								
								ListView_GetSubItemRect(g_hwndListViewServer,nItem,pListDraw->iSubItem,LVIR_BOUNDS,&rc);								

								if(pListDraw->iSubItem==0)
									ListView_GetSubItemRect(g_hwndListViewServer,nItem,pListDraw->iSubItem,LVIR_LABEL,&rc);
								
							
								if(gm.GamesInfo[pSI->cGAMEINDEX].vGAME_SPEC_COL.size()>=COL_SERVERNAME)
								{
										char *szVarValue = pSI->szServerName;//Get_RuleValue((TCHAR*)gm.GamesInfo[pSI->cGAMEINDEX].vGAME_SPEC_COL.at(COL_SERVERNAME).sRuleValue.c_str(),pSI->pServerRules,1);							

										if(gm.GamesInfo[pSI->cGAMEINDEX].Draw_ColorEncodedText!=NULL)
											gm.GamesInfo[pSI->cGAMEINDEX].Draw_ColorEncodedText(rc, pListDraw , szVarValue);
										else
											Draw_ColorEncodedText(rc, pListDraw , szVarValue);
								}


								COLORREF colRef;
								if(iRow%2 == 0)
									colRef = RGB(202, 221,250);
								else
									colRef = RGB(255, 255,255);

								pListDraw->clrTextBk   = colRef;

								lResult =  ( CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT );
							}
							if(pSI->cFavorite)
								ImageList_Draw(g_hImageListIcons,2,hDC,rc.left+1,rc.top+2,ILD_NORMAL|ILD_TRANSPARENT);
							else
								ImageList_Draw(g_hImageListIcons,gm.Get_GameIcon(pSI->cGAMEINDEX),hDC,rc.left+1,rc.top+2,ILD_NORMAL|ILD_TRANSPARENT);
							
							if((pSI->wMonitor >0))
								ImageList_Draw(g_hImageListIcons,29,hDC,rc.left+1,rc.top+2,ILD_NORMAL|ILD_TRANSPARENT);
							break;


						}
					lResult =  CDRF_NEWFONT; 					
					break;
				} 
			}
			break;
	default:
		break;
	}
	ReleaseServerLock(pSI);
	return lResult;
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
							PLAYERDATA *pPlayerData = g_vecPlayerList.at(nItem);

							RECT rc;								
							if(pPlayerData!=NULL)
							{
								char szText[256];
								if( pPlayerData->szClanTag!=NULL)
									sprintf_s(szText,sizeof(szText)-1,"%s %s", pPlayerData->szClanTag,pPlayerData->szPlayerName);
								else
									sprintf_s(szText,sizeof(szText)-1,"%s",pPlayerData->szPlayerName);

								ListView_GetSubItemRect(g_hwndListViewPlayers,nItem,pListDraw->iSubItem,LVIR_BOUNDS,&rc);								
								if(gm.GamesInfo[pPlayerData->cGAMEINDEX].Draw_ColorEncodedText!=NULL)
									return gm.GamesInfo[pPlayerData->cGAMEINDEX].Draw_ColorEncodedText(rc, pListDraw ,szText);// pPlayerData->szPlayerName);
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

bool PlayerList_Sort_Name(PLAYERDATA* a, PLAYERDATA* b)
{
	if(AppCFG.bPlayerNameAsc)
		return (CustomStrCmp(a->szPlayerName,b->szPlayerName)>0);
	else
		return (CustomStrCmp(a->szPlayerName,b->szPlayerName)<0);
}

bool PlayerList_Sort_ServerName(PLAYERDATA* a, PLAYERDATA* b)
{
	if(AppCFG.bSortPlayerServerNameAsc)
		return (CustomStrCmp(a->pServerInfo->szServerName,b->pServerInfo->szServerName)>0);
	else
		return (CustomStrCmp(a->pServerInfo->szServerName,b->pServerInfo->szServerName)<0);
}

bool PlayerList_Sort_Rate(PLAYERDATA* a, PLAYERDATA* b)
{
	if(bRateAsc)
		return (a->rate>b->rate);
	else
		return (a->rate<b->rate);
}

bool PlayerList_Sort_Ping(PLAYERDATA* a, PLAYERDATA* b)
{
	if(bPlayerPingAsc)
		return (a->ping > b->ping);
	else
		return (a->ping < b->ping);
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

void OnSearchFieldChange()
{
	TCHAR szSearchTmp[256];
	ZeroMemory(szSearchTmp,sizeof(szSearchTmp));
	GetDlgItemText(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD,szSearchTmp,sizeof(szSearchTmp)-sizeof(TCHAR));
	GetDlgItemText(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD,szSearchTmp,99);
	if(strlen(szSearchTmp)>0)
	{
		Show_ToolbarButton(IDC_BUTTON_FIND,true);
		int searchFor = 0;

		if(strstr(szSearchTmp,"/player")!=NULL)
			searchFor = SEARCH_PLAYER;
		else if(strstr(szSearchTmp,"/rule")!=NULL)
			searchFor = SEARCH_RULE;
		switch(searchFor)
		{		
			case SEARCH_RULE: 
				FindServerRule(szSearchTmp);
				break;
			case SEARCH_PLAYER: 
				FindPlayers(szSearchTmp);
				break;
			default:
				FindServer(szSearchTmp);
				break;
		}
		SetDlgItemText(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD,szSearchTmp);					
		SetCursor(::LoadCursor(NULL,IDC_ARROW));
		ShowCursor(TRUE);
		SendDlgItemMessage (g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD, CB_SETEDITSEL, 0,MAKELPARAM(strlen(szSearchTmp),strlen(szSearchTmp))); 	
	}
	else
	{
		Show_ToolbarButton(IDC_BUTTON_FIND,false);
		int gameIdx = g_currentGameIdx;
		RedrawServerListThread((LPVOID)gameIdx);
	}
}


SERVER_INFO *FindServer(char *str)
{	
	char copy1[256];
	char copy2[256];
	char szTempBuffert[256];

	SendDlgItemMessage(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD, CB_SHOWDROPDOWN, FALSE, 0); 
	SendDlgItemMessage (g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD, CB_RESETCONTENT, 0, 0); 
	DWORD counter=0;
	if(str==NULL)
		return NULL;

	if(strlen(str)<1)
		return NULL;

	ZeroMemory(copy1,sizeof(copy1));
	strncpy(copy1,str,sizeof(copy1)-1);


	if(TryEnterCriticalSection(&LOAD_SAVE_CS)==FALSE)
	{
		dbg_print("Busy with loading server list.");
		return 0;
	}
	LeaveCriticalSection(&LOAD_SAVE_CS);

	GAME_INFO *pGI = (GAME_INFO *)&gm.GamesInfo[g_currentGameIdx];
	vSRV_INF::iterator  iLst;

	if(pGI->vSI.size()==0)
		return NULL;

	if(TryEnterCriticalSection(&REDRAWLIST_CS)==FALSE)
	{
		dbg_print("busy RefList");
		return 0;
	}

	_strlwr_s( copy1,sizeof(copy1));

	ListView_DeleteAllItems(g_hwndListViewServer);

	
	pGI->vRefListSI.clear();
	
	if(str[0]>='A' &&  str[0]<='z')
	{
		for ( iLst = pGI->vSI.begin( ); iLst != pGI->vSI.end( ); iLst++ )
		{		
			SERVER_INFO *pSI = *iLst;
			pGI->colorfilter(pSI->szServerName,szTempBuffert,sizeof(szTempBuffert));
			strncpy(copy2,szTempBuffert,sizeof(copy2));
			int l = strlen(copy2);
			if(copy2!=NULL || l!=0)	
			{	
				_strlwr_s( copy2,sizeof(copy2));
				if(strstr(copy2,copy1)!=NULL)
				{
					REF_SERVER_INFO refSI;
					refSI.pServerInfo = pSI;
					pGI->vRefListSI.push_back(refSI);
					counter++;
				}
			}		
		}

	} else //could it be a IP address search?
	{	
		for ( iLst = pGI->vSI.begin( ); iLst != pGI->vSI.end( ); iLst++ )
		{
			SERVER_INFO *pSI = *iLst;
			if(strstr(pSI->szIPaddress,str)!=NULL)
			{								
				REF_SERVER_INFO refSI;
				refSI.pServerInfo = pSI;
				pGI->vRefListSI.push_back(refSI);
				counter++;
			}else 
			{
				sprintf(szTempBuffert,"%s:%d",pSI->szIPaddress,pSI->usPort);
				if(strstr(szTempBuffert,str)!=NULL)
				{
					REF_SERVER_INFO refSI;
					refSI.pServerInfo = pSI;
					pGI->vRefListSI.push_back(refSI);
					counter++;
				}
			}
		}
	}
	if(pGI->vRefListSI.size()==0)
	{
		size_t n = strcspn(str,"01234567890");
		char p = str[n];
		if(n>0)
		{
			for ( iLst = pGI->vSI.begin( ); iLst != pGI->vSI.end( ); iLst++ )
			{
				SERVER_INFO *pSI = *iLst;
				if(strstr(&str[n],pSI->szIPaddress)!=NULL)
				{								
					REF_SERVER_INFO refSI;
					refSI.pServerInfo = pSI;
					pGI->vRefListSI.push_back(refSI);
					counter++;
				}else 
				{
					sprintf(szTempBuffert,"%s:%d",pSI->szIPaddress,pSI->usPort);
					if(strstr(&str[n],szTempBuffert)!=NULL)
					{
						REF_SERVER_INFO refSI;
						refSI.pServerInfo = pSI;
						pGI->vRefListSI.push_back(refSI);
						counter++;
					}
				}
			}
		}
	}

	LeaveCriticalSection(&REDRAWLIST_CS);

/*	int count = SendDlgItemMessage(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD, (UINT) CB_GETCOUNT, 0, 0);  

	if(count>0)
		SendDlgItemMessage(g_hwndSearchToolbar,IDC_COMBOBOXEX_CMD, CB_SHOWDROPDOWN, TRUE, 0); 
*/
 // if((str[0]!='/') && (strlen(str)<strlen("rule")))
	Do_ServerListSortThread(iLastColumnSortIndex);
	//Do_ServerListSort((LPVOID)iLastColumnSortIndex);	
	//ListView_SetItemCount(g_hwndListViewServer,pGI->vRefListSI.size());		


	return NULL;
}

BOOL FindServerRule(char *szRule)
{	
	char copy1[256];

	BOOL bFoundRules=FALSE;

	DWORD counter=0;
	if(szRule==NULL)
		return bFoundRules;

	if(strlen(szRule)<strlen("/rule "))
		return bFoundRules;

	ZeroMemory(copy1,sizeof(copy1));
	strncpy(copy1,szRule,sizeof(copy1)-1);


	if(TryEnterCriticalSection(&LOAD_SAVE_CS)==FALSE)
	{
		dbg_print("Busy with loading server list.");
		return 0;
	}
	LeaveCriticalSection(&LOAD_SAVE_CS);

	GAME_INFO *pGI = (GAME_INFO *)&gm.GamesInfo[g_currentGameIdx];
	vSRV_INF::iterator  iLst;

	if(pGI->vSI.size()==0)
		return NULL;

	if(TryEnterCriticalSection(&REDRAWLIST_CS)==FALSE)
	{
		dbg_print("busy RefList");
		return 0;
	}

	_strlwr_s( copy1,sizeof(copy1));

	ListView_DeleteAllItems(g_hwndListViewServer);
	
	pGI->vRefListSI.clear();
	char rulename[256];
	ZeroMemory(rulename,256);
	szRule+=strlen("/rule ");
	size_t n = strcspn(szRule,"=");
	strncpy(rulename,szRule,n);
	
	if(n!=strlen(szRule))
	{
		for ( iLst = pGI->vSI.begin( ); iLst != pGI->vSI.end( ); iLst++ )
		{
			SERVER_INFO *pSI = *iLst;
			char *szVarValue = Get_RuleValue(rulename,pSI->pServerRules);
			if(szVarValue!=NULL)							
			{	
				if(strstr(&szRule[n+1],szVarValue)!=NULL)
				{								
					REF_SERVER_INFO refSI;
					refSI.pServerInfo = pSI;
					pGI->vRefListSI.push_back(refSI);
					counter++;
					bFoundRules = TRUE;
				}
			}
		}
	}


	LeaveCriticalSection(&REDRAWLIST_CS);
	Do_ServerListSortThread(iLastColumnSortIndex);
	return NULL;
}



BOOL FindPlayers(const char *szPlayer)
{	
	char copy1[256];
	BOOL bFoundPlayers = FALSE;
	DWORD counter=0;
	if(szPlayer==NULL)
		return bFoundPlayers;

	if(strlen(szPlayer)<strlen("/player "))
		return bFoundPlayers;

	ZeroMemory(copy1,sizeof(copy1));
	strncpy(copy1,szPlayer,sizeof(copy1)-1);

	//Go through a couple of safety checks

	if(TryEnterCriticalSection(&LOAD_SAVE_CS)==FALSE)
	{
		dbg_print("Busy with loading server list.");
		return bFoundPlayers;
	}
	LeaveCriticalSection(&LOAD_SAVE_CS);

	GAME_INFO *pGI = (GAME_INFO *)&gm.GamesInfo[g_currentGameIdx];
	vSRV_INF::iterator  iLst;

	if(pGI->vSI.size()==0)
		return bFoundPlayers;

	if(TryEnterCriticalSection(&REDRAWLIST_CS)==FALSE)
	{
		dbg_print("busy RefList");
		return bFoundPlayers;
	}

	_strlwr_s( copy1,sizeof(copy1));
	pGI->vRefListSI.clear();

	const char *pszName = strchr(szPlayer,' ');	
	if(pszName!=NULL)
	{
		g_vecPlayerList.clear();
		CleanUp_PlayerList(pCurrentPL);
		pCurrentPL = NULL;
		ListView_DeleteAllItems(g_hwndListViewPlayers);
		pszName++; //skip initial white space
		for ( iLst = pGI->vSI.begin( ); iLst != pGI->vSI.end( ); iLst++ )
		{
			SERVER_INFO *pSI = *iLst;
			PLAYERDATA *pd = pSI->pPlayerData;
			while(pd!=NULL)
			{
				if(pd->szPlayerName!=NULL)
				{
					if(strstr(pd->szPlayerName,pszName)!=NULL)
					{								
						REF_SERVER_INFO refSI;
						refSI.pServerInfo = pSI;
						pGI->vRefListSI.push_back(refSI);
						counter++;
						if(pCurrentPL==NULL)
							pCurrentPL = Copy_PlayerToCurrentPL(pCurrentPL,pd);  //This will keep a copy of the playerlist during scanning							
						else 
							Copy_PlayerToCurrentPL(pCurrentPL,pd);  //This will keep a copy of the playerlist during scanning							
						ListView_SetItemCount(g_hwndListViewPlayers, counter);
						bFoundPlayers = TRUE;
					}
				}
				pd = pd->pNext;
			}
		}
	}

	LeaveCriticalSection(&REDRAWLIST_CS);
	Do_ServerListSortThread(iLastColumnSortIndex);
	//ListView_SetItemCount(g_hwndListViewServer,pGI->vRefListSI.size());		

	return bFoundPlayers;
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
	char szCMD[80];		
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
					
					PLAYERDATA* ply = Get_PlayerBySelection();		
					if(ply!=NULL)
					{
						sprintf_s(szCMD,sizeof(szCMD),"say ^1WARNED! ^w ^6%s ^1WARNED!",ply->szPlayerName);
						PLAYERDATA* ply = Get_PlayerBySelection();											
						RCON_Connect(ply->pServerInfo);
						RCON_SendCmd(ply->pServerInfo,ply->pServerInfo->szRCONPASS,szCMD); 
						RCON_Read(ply->pServerInfo);
						RCON_Disconnect();
					}
				}
				break;
				case ID_MUTE_PLAYER:
				{							
					PLAYERDATA* ply = Get_PlayerBySelection();		
					if(ply!=NULL)
					{
						sprintf_s(szCMD,sizeof(szCMD),"mute %s",ply->szPlayerName);
						g_RCONServer = ply->pServerInfo;
						RCON_Connect(ply->pServerInfo);
						RCON_SendCmd(ply->pServerInfo,ply->pServerInfo->szRCONPASS,szCMD); 
						RCON_Read(ply->pServerInfo);
						RCON_Disconnect();
					}
				}
				break;
				case ID_UNMUTE_PLAYER:
				{
					PLAYERDATA* ply = Get_PlayerBySelection();		
					if(ply!=NULL)
					{
						sprintf_s(szCMD,sizeof(szCMD),"unmute %s",ply->szPlayerName);
						g_RCONServer = ply->pServerInfo;
						RCON_Connect(ply->pServerInfo);
						RCON_SendCmd(ply->pServerInfo,ply->pServerInfo->szRCONPASS,szCMD); 
						RCON_Read(ply->pServerInfo);
						RCON_Disconnect();
					}
				}
				break;
				case ID_KICK_PLAYER:
				{
					PLAYERDATA* ply = Get_PlayerBySelection();		
					if(ply!=NULL)
					{	
						int ret = MessageBox(NULL,g_lang.GetString("AskRconKickPlayer"),ply->szPlayerName,MB_YESNO);
						if(ret==IDYES)
						{
							sprintf_s(szCMD,sizeof(szCMD),"kick %s",ply->szPlayerName);
							RCON_Connect(ply->pServerInfo);
							RCON_SendCmd(ply->pServerInfo,ply->pServerInfo->szRCONPASS,szCMD); 
							RCON_Read(ply->pServerInfo);
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
					bm.OnAddSelectedPlayerToBuddyList();											
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
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_ADD,g_lang.GetString("MenuAddPlayerToBuddyList"));			
				switch(g_currentGameIdx)
				{
					case RTCW_SERVERLIST:
					case Q4_SERVERLIST:
					case Q3_SERVERLIST:
					case COD_SERVERLIST:
					case COD2_SERVERLIST:					
					case ET_SERVERLIST:
					case ETQW_SERVERLIST:
						InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_YAWN_PLAYER,g_lang.GetString("MenuYAWNPlayer"));			
					break;
				}

				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_KICK_PLAYER,g_lang.GetString("MenuKick"));			
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_WARN_PLAYER,g_lang.GetString("MenuWarn"));			
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_MUTE_PLAYER,g_lang.GetString("MenuMute"));			
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_UNMUTE_PLAYER,g_lang.GetString("MenuUnMute"));			
												
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


LRESULT CALLBACK EnumerateGames_Dlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{
			CenterWindow(hDlg);
			HWND hwndGamesCombo = GetDlgItem(hDlg,IDC_COMBO_GAMES);

			SetWindowText(hDlg,g_lang.GetString("EnumerateGamesTitle"));	
			if(lParam==1)
				SetDlgItemText(hDlg,IDC_STATIC_LABEL,g_lang.GetString("ExportServerListLabel"));	
			else
				SetDlgItemText(hDlg,IDC_STATIC_LABEL,g_lang.GetString("ImportServerListLabel"));	

			for(int i=0;i<gm.GamesInfo.size();i++)
			{
				SendMessage((HWND) hwndGamesCombo,(UINT) CB_ADDSTRING,(WPARAM) 0, (LPARAM)gm.GamesInfo[i].szGAME_NAME);  
			}
		
		
		//return TRUE;
		}
		break;
	case WM_COMMAND:

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			if(LOWORD(wParam) == IDOK)
			{
							
				char szGameName[256];
				HWND hwndGamesCombo = GetDlgItem(hDlg,IDC_COMBO_GAMES);
			
				DWORD  dwidx = SendMessage((HWND) hwndGamesCombo,(UINT) CB_GETCURSEL,(WPARAM) 0, (LPARAM)0);  
				SendMessage((HWND) hwndGamesCombo,(UINT) CB_GETLBTEXT,(WPARAM)dwidx , (LPARAM)szGameName);  
				for(int i=0;i<gm.GamesInfo.size();i++)
				{
					if(strcmp(gm.GamesInfo[i].szGAME_NAME,	szGameName)==0)
					{
						g_dwExportGameIdx = i;
						break;
					}
				}
			}

			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}



string g_sEditRuleValue;
string g_sEditRuleValueLabel;

LRESULT CALLBACK EditValue_Dlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndEdit;
	char szTemp[256];
	switch (message)
	{
		case WM_INITDIALOG:
		{
			CenterWindow(hDlg);
			hwndEdit = GetDlgItem(hDlg,IDC_EDIT_MINMAX);
			SetWindowText(hDlg,g_lang.GetString("TitleEditValue"));				
			SetDlgItemText(hDlg,IDC_STATIC_EDIT_LABEL,g_sEditRuleValueLabel.c_str());	

			SetDlgItemText(hDlg,IDC_EDIT_MINMAX,g_sEditRuleValue.c_str());	
			SetFocus(GetDlgItem(hDlg,IDC_EDIT_MINMAX));	
			PostMessage(GetDlgItem(hDlg,IDC_EDIT_MINMAX),EM_SETSEL,0,g_sEditRuleValue.length());
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
				g_sEditRuleValue = szTemp;

			}

			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}


void OnEditRuleValue(HWND hWndParent, char *szRulename, char *szRuleValue)
{
	string rulename;
	rulename = szRulename;
	g_sEditRuleValue = szRuleValue;
	g_sEditRuleValueLabel = szRulename;
	g_sEditRuleValueLabel.append(": ");

	INT_PTR result = DialogBoxParam(g_hInst, (LPCTSTR)IDD_DLG_SET_VALUE, hWndParent, (DLGPROC)EditValue_Dlg,0);					

	if(result==IDOK)
	{
		char szCMD[256];		
		sprintf_s(szCMD,sizeof(szCMD),"%s %s",rulename.c_str(),g_sEditRuleValue.c_str());
		if(RCON_Connect(g_CurrentSRV)==0)
		{		
			int ret = RCON_SendCmd(g_CurrentSRV,g_CurrentSRV->szRCONPASS,szCMD); 
			if(ret!=0)
				MessageBox(g_hWnd,g_lang.GetString("RCONErrorCommand"),"Info",MB_OK);
			RCON_Disconnect();
			gm.GetServerInfo(g_currentGameIdx,g_CurrentSRV);
		} else
		{
			MessageBox(g_hWnd,g_lang.GetString("RCONErrorConnecting"),"Info",MB_OK);
		}
	}
}

LRESULT APIENTRY ListView_Rules_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 
	HMENU hPopMenu;
	char szRuleName[40];
	char szRuleValueName[256];
	int n=-1;
	DWORD wmId;
	DWORD wmEvent;

	if(uMsg == WM_COMMAND)
	 {
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_COPY_RULE:
				{
					n = ListView_GetSelectionMark(g_hwndListViewVars);
					ListView_GetItemText(g_hwndListViewVars,n,0,szRuleName,sizeof(szRuleName)-1);
					ListView_GetItemText(g_hwndListViewVars,n,1,szRuleValueName,sizeof(szRuleValueName)-1);
					if(n!=-1)
					{		
						g_sEditRuleValueLabel = szRuleName;
						g_sEditRuleValueLabel.append(" ");
						g_sEditRuleValueLabel.append(szRuleValueName);
						EditCopy((TCHAR*)g_sEditRuleValueLabel.c_str());
					}
					break;
				}
				case IDM_EDIT_RULE:
				{
					n = ListView_GetSelectionMark(g_hwndListViewVars);
					ListView_GetItemText(g_hwndListViewVars,n,0,szRuleName,sizeof(szRuleName)-1);
					ListView_GetItemText(g_hwndListViewVars,n,1,szRuleValueName,sizeof(szRuleValueName)-1);
					if(n!=-1)
					{				
						OnEditRuleValue(g_hWnd,szRuleName,szRuleValueName);

					}
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
			n = ListView_GetSelectionMark(g_hwndListViewVars);
			if(n!=-1)
			{
				//place the window/menu there if needed 						
				hPopMenu = CreatePopupMenu();
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_EDIT_RULE,g_lang.GetString("MenuEditRule"));			
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_COPY_RULE,g_lang.GetString("MenuCopyRuleToClipBoard"));			
												
				//workaround for microsoft bug, to hide menu w/o selecting
				SetForegroundWindow(hwnd);
				TrackPopupMenu(hPopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,lpClickPoint.x, lpClickPoint.y,0,hwnd,NULL);
				SendMessage(hwnd,WM_NULL,0,0);
				DestroyMenu(hPopMenu); 

			}
			return 0;
		}				
    return CallWindowProc((WNDPROC)g_wpOrigLVRulesProc, hwnd, uMsg,  wParam, lParam); 
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
					vecBI::iterator vecBI_it = find(bm.BuddyList.begin(),bm.BuddyList.end(),(DWORD)lvItem.lParam);
					if( vecBI_it != bm.BuddyList.end())
					{
						BUDDY_INFO BI = (BUDDY_INFO)*vecBI_it;								
						if(BI.sIndex!=-1)
						{
							SERVER_INFO *pSI = gm.GamesInfo[BI.cGAMEINDEX].vSI.at((int)BI.sIndex);						
							LaunchGame(pSI,&gm.GamesInfo[pSI->cGAMEINDEX]);
						}						
					}
			
				}
			}else
				MessageBox(NULL,g_lang.GetString("MessageBuddyNotOnline"),"Info",MB_OK);
		} else
			MessageBox(NULL,g_lang.GetString("MessageErrorConnecting"),"Info",MB_OK);
	}
	else  //From Favorites or the Masterlist
	{	
		n = ListView_GetSelectionMark(g_hwndListViewServer);
		if(n!=-1)
		{

			SERVER_INFO *pSrv;
			pSrv =  Get_ServerInfoByListViewIndex(currCV,n);
			LaunchGame(pSrv,&gm.GamesInfo[g_currentGameIdx]);
		}
	}

}


//if not sucessfull a zerofilled SERVER_INFO is returned!!
//This function should be rewritten...
SERVER_INFO *Get_ServerInfoByListViewIndex(GAME_INFO *pGI,int index)
{	

	REF_SERVER_INFO refSI;

	if(pGI==NULL)
		return NULL; //return NULL srv

	if(index<pGI->vRefListSI.size())
	{
		__try
		{				
			refSI = pGI->vRefListSI.at(index);
		}
		__except(EXCEPTION_ACCESS_VIOLATION == GetExceptionCode())
		{
			log.AddLogInfo(GS_LOG_ERROR,"Access Violation @ GetServerInfoByListIndex %s!");
			return NULL;
		}		
	} else
		return NULL;
	
	return refSI.pServerInfo; //gotcha... :)
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
	  MessageBox(NULL,g_lang.GetString("SelectServerAtConnectionRCON"),"Info!",MB_ICONINFORMATION|MB_OK); 

	}
}


void OnBeginDrag(NMHDR* pNMHDR)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	m_hitemDrag = pNMTreeView->itemNew.hItem;
	m_hitemDrop = NULL;

	
	m_pDragImage = TreeView_CreateDragImage(g_hwndMainTreeCtrl,m_hitemDrag);  // get the image list for dragging
	// CreateDragImage() returns NULL if no image list
	// associated with the tree view control
	if( !m_pDragImage )
		return;

	m_bLDragging = TRUE;
	ImageList_BeginDrag(m_pDragImage,0, -15,-15);
	POINT pt = pNMTreeView->ptDrag;
	ClientToScreen(g_hWnd, &pt );
	ImageList_DragEnter(NULL, pt.x,pt.y);
	SetCapture(g_hwndMainTreeCtrl);

}

;
LRESULT APIENTRY TreeView_SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 

	HMENU hPopMenu;
	if(uMsg == WM_MOUSEMOVE)
	{
		if(g_bRunningQuery)
			SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
		HTREEITEM	hitem;
	

		if (m_bLDragging)
		{
			POINT pt;
			pt.x =  LOWORD(lParam);
			pt.y =  HIWORD(lParam);
			POINT pt2;
			pt2.x =  LOWORD(lParam);
			pt2.y =  HIWORD(lParam);
			ClientToScreen(g_hWnd, &pt2 );
			ImageList_DragMove(pt2.x-15,pt2.y-15);

	
			TVHITTESTINFO tvHTTI;

			ZeroMemory(&tvHTTI,sizeof(TVHITTESTINFO));
			tvHTTI.pt = pt;
			dbg_print("point: x:%d  y:%d",tvHTTI.pt.x,tvHTTI.pt.y);
			if ((hitem = TreeView_HitTest(g_hwndMainTreeCtrl,&tvHTTI)) != NULL)
			{
				
				DWORD action1 = tvmgr.vTI.at(tvmgr.GetIndex(hitem)).dwType;
				DWORD action2 = tvmgr.vTI.at(tvmgr.GetIndex(m_hitemDrag)).dwType;
				if(((action1>1) && (action1<13))  || (action2<13) || (action2==23) )
				{
					SetCursor(LoadCursor(NULL, IDC_NO));
				}
				else
					SetCursor(::LoadCursor(NULL,IDC_ARROW));

				ImageList_DragShowNolock(FALSE);
				TreeView_SelectDropTarget(g_hwndMainTreeCtrl,hitem);
				m_hitemDrop = hitem;
				HTREEITEM prev=TreeView_GetPrevVisible(hwnd,hitem);
				HTREEITEM next=TreeView_GetNextVisible(hwnd,hitem);
				TreeView_EnsureVisible(hwnd,prev);		
				TreeView_EnsureVisible(hwnd,next);				
				ImageList_DragShowNolock(TRUE);
			}
		}
	}
	else if(uMsg == WM_LBUTTONUP)
	{
		if (m_bLDragging)
		{
				m_bLDragging = FALSE;
				ImageList_DragLeave(hwnd);
				ImageList_EndDrag();
				ReleaseCapture();

				 ImageList_Destroy( m_pDragImage);
				 m_pDragImage = NULL;

				// Remove drop target highlighting
				TreeView_SelectDropTarget(hwnd,NULL);

				if( m_hitemDrag == m_hitemDrop )
					return TRUE;

				// If Drag item is an ancestor of Drop item then return
				HTREEITEM htiParent = m_hitemDrop;
				while( (htiParent = TreeView_GetParent(hwnd, htiParent )) != NULL )
				{
					if( htiParent == m_hitemDrag ) 
						return TRUE;
				}
				
				

				TreeView_Expand(hwnd, m_hitemDrop, TVE_EXPAND ) ;
				int idx = tvmgr.GetIndex(m_hitemDrag);
				if((tvmgr.vTI.at(idx).dwType>12) && (tvmgr.vTI.at(idx).dwType!=23) )
				{					
					HTREEITEM htiNew = TreeView_MoveItem(m_hitemDrag,m_hitemDrop);
					TreeView_SelectItem( hwnd,htiNew );
				}
			}

	//	OnTreeViewSelectionChanged(lParam);
	}
	else if(uMsg == WM_KEYDOWN)
	{
		switch((wParam))
		{		
			case VK_F2:OnRenameFilter(g_hWnd);
				return TRUE;
			case VK_RSHIFT:
			case VK_LSHIFT:tvmgr.m_bShiftKeyPressed = TRUE;
				return TRUE;
		} 
	}
	else if(uMsg == WM_KEYUP)
	{
		switch((wParam))
		{		
			case VK_F2:OnRenameFilter(g_hWnd);
				return TRUE;
			
			case VK_RSHIFT:
			case VK_LSHIFT:tvmgr.m_bShiftKeyPressed = FALSE;
				return TRUE;
		} 
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
				case IDM_LAUNCH_GAME_ONLY:
					ExecuteGame(currCV,"");
				break;
				case IDM_DELETE_SERVERLIST:
					DeleteServerLists(gm.GamesInfo[g_currentGameIdx].cGAMEINDEX);
				break;
				case IDM_FILTER_ADD:
					OnAddFilter(g_hWnd);
				break;
				case IDM_FILTER_EDIT:		
					OnEditFilter(g_hWnd);
				break;
				case IDM_FILTER_DELETE:		
					OnDelFilter(g_hWnd);
				break;
				case IDM_FILTER_RENAME:
					OnRenameFilter(g_hWnd);
				break;
				case IDM_FILTER_NEW_FOLDER:
					OnAddNewFolder(g_hWnd);
				break;		
				case IDM_FILTER_DELETE_FOLDER:
					OnDeleteFolder(g_hWnd);
				break;
				case IDM_FILTER_COPY:
					OnCopyFilter();
				break;
				case IDM_FILTER_PASTE:
					OnPasteFilter();
				break;
			}
		}
 		else if(uMsg == WM_RBUTTONDOWN)
		{
			//get mouse cursor position x and y as lParam has the message itself 
			POINT lpClickPoint;
			HMENU hSubPopMenu;

			tvmgr.g_bMouseMenuClick = TRUE;
			SendMessage( hwnd, WM_LBUTTONDOWN, wParam, lParam );	
			
			HTREEITEM hCurrent = TreeView_GetSelection(hwnd);
			if(hCurrent==NULL)
				return 0;
			GetCursorPos(&lpClickPoint);
			hPopMenu = CreatePopupMenu();
			
			hSubPopMenu = CreatePopupMenu();

			TVITEM  tvitem;
			ZeroMemory(&tvitem,sizeof(TVITEM));
			tvitem.hItem = hCurrent;
			tvitem.mask = TVIF_PARAM;
			TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );
			DWORD dwType = tvmgr.vTI.at(tvitem.lParam).dwType;
			char szText[100];
			sprintf_s(szText,sizeof(szText),g_lang.GetString("MenuFindInternetServers"),gm.GamesInfo[g_currentGameIdx].szGAME_NAME);
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_SCAN,szText);
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_LAUNCH_GAME_ONLY,g_lang.GetString("MenuLaunchGameOnly"));
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_ADDIP,g_lang.GetString("MenuAddNewIPToFav"));
			sprintf_s(szText,sizeof(szText),g_lang.GetString("MenuDeleteServerList"),gm.GamesInfo[g_currentGameIdx].szGAME_NAME);
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_DELETE_SERVERLIST,szText);			
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_SEPARATOR,0,"");

			DWORD dwflag = MF_BYPOSITION|MF_STRING|MF_GRAYED;
			if((dwType>=13 && dwType<=40) || (dwType==0))
				dwflag = MF_BYPOSITION|MF_STRING;

			InsertMenu(hPopMenu,0xFFFFFFFF,dwflag,IDM_FILTER_NEW_FOLDER,"Add new folder");
			if(dwType==0)
				InsertMenu(hPopMenu,0xFFFFFFFF,dwflag,IDM_FILTER_DELETE_FOLDER,"Delete folder");


			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_SEPARATOR,0,"");
			dwflag = MF_BYPOSITION|MF_STRING|MF_GRAYED;
			if((dwType>=13 && dwType<=40) || (dwType==0))
				dwflag = MF_BYPOSITION|MF_STRING;

			InsertMenu(hPopMenu,0xFFFFFFFF,dwflag,IDM_FILTER_ADD,g_lang.GetString("FilterAdd"));
			InsertMenu(hPopMenu,0xFFFFFFFF,dwflag,IDM_FILTER_RENAME,"Rename                      (F2)");

			if((dwType==0) || (dwType==23))			
				dwflag = MF_BYPOSITION|MF_STRING|MF_GRAYED;

			InsertMenu(hPopMenu,0xFFFFFFFF,dwflag,IDM_FILTER_EDIT,g_lang.GetString("FilterEdit"));
			InsertMenu(hPopMenu,0xFFFFFFFF,dwflag,IDM_FILTER_DELETE,g_lang.GetString("FilterDel"));
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_SEPARATOR,0,"");
			InsertMenu(hPopMenu,0xFFFFFFFF,dwflag,IDM_FILTER_COPY,"Copy filter");
			InsertMenu(hPopMenu,0xFFFFFFFF,dwflag,IDM_FILTER_PASTE,"Paste filter");
						
			//workaround for microsoft bug, to hide menu w/o selecting
			SetForegroundWindow(hwnd);
			TrackPopupMenu(hPopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,lpClickPoint.x, lpClickPoint.y,0,hwnd,NULL);
			SendMessage(hwnd,WM_NULL,0,0);
			DestroyMenu(hPopMenu);
			DestroyMenu(hSubPopMenu);
			tvmgr.g_bMouseMenuClick = FALSE;
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



LRESULT APIENTRY LV_SL_HeaderSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	static HMENU LV_Header_PopMenu = NULL;
	 if(uMsg == WM_COMMAND)
	 {
		DWORD wmId;
		DWORD wmEvent;
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		if(wmId == IDM_RESTORE_COLUMNS)
		{
			g_LVHeaderSL->ResetColumnOrder();
			LVHeaderSL_AddDefaultColumns(TRUE);
			PostMessage(g_hWnd,WM_REFRESHSERVERLIST,0,0);
			return TRUE;
		}
						
		MENUITEMINFO mii;
		memset(&mii,0,sizeof(MENUITEMINFO));
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_STATE;

		if(GetMenuItemInfo(LV_Header_PopMenu,wmId,FALSE,&mii))
		{				
			if(MFS_CHECKED == (mii.fState  & MFS_CHECKED) )
			{
				g_LVHeaderSL->SetActive(wmId,FALSE);
				mii.fState = MFS_UNCHECKED;
			} else
			{
				g_LVHeaderSL->SetActive(wmId,TRUE);
				mii.fState = MFS_CHECKED;
			}
			SetMenuItemInfo(LV_Header_PopMenu,wmId,FALSE,&mii);				
			
		}
		if(LV_Header_PopMenu != NULL)
		{
			DestroyMenu(LV_Header_PopMenu);	
			LV_Header_PopMenu = NULL;
		}
		return TRUE;
	 }
	else if(uMsg == WM_RBUTTONDOWN)
	{
			//get mouse cursor position x and y as lParam has the message itself 
			POINT lpClickPoint;
			GetCursorPos(&lpClickPoint);
			LV_Header_PopMenu = CreatePopupMenu();
			MENUITEMINFO mii;
			for(int i=0;i<g_LVHeaderSL->GetSize();i++)
			{
				ZeroMemory(&mii,sizeof(MENUITEMINFO));
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_STRING | MIIM_ID |   MIIM_STATE | MIIM_FTYPE;
				mii.fType = MFT_STRING ;
				mii.wID = i;
				mii.dwTypeData = (LPSTR) g_lang.GetString(g_LVHeaderSL->GetColumnName(i));
				if(g_LVHeaderSL->IsActive(i))
					mii.fState = MFS_CHECKED;
				else
					mii.fState = MFS_UNCHECKED;
				InsertMenuItem(LV_Header_PopMenu,0,FALSE,&mii);
			}
			InsertMenu(LV_Header_PopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_RESTORE_COLUMNS,g_lang.GetString("MenuRestoreColumns"));
			
			SetForegroundWindow(hwnd);
			TrackPopupMenu(LV_Header_PopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,lpClickPoint.x, lpClickPoint.y,0,hwnd,NULL);
			SendMessage(hwnd,WM_NULL,0,0);
			return 0;
	}
	return CallWindowProcW((WNDPROC)g_wpOrigSLHeaderProc, hwnd, uMsg,  wParam, lParam);  
}

char *Get_SelectedServerIP()
{
	int n=-1;
	n = ListView_GetSelectionMark(g_hwndListViewServer);
	if(n!=-1)
	{
		SERVER_INFO *pSI = Get_ServerInfoByListViewIndex(currCV,n);
		if(pSI!=NULL)
		{
			sprintf(gm.g_currServerIP,"%s:%d",pSI->szIPaddress,pSI->usPort);	
			return gm.g_currServerIP;
		}
	}
	return NULL;
}
HBITMAP hBmp;


DWORD WINAPI  Menu_SL_Thread(LPVOID hWnd , WPARAM wParam, LPARAM lParam)
{
	POINT lpClickPoint;
	HMENU hPopMenu = NULL;
	HMENU hSubPopMenu = NULL;
	HMENU hSubForceLaunchPopMenu= NULL;
	HMENU hSubClipboardPopMenu = NULL;		
	HMENU hSubMonitorPopMenu = NULL;
	IMITEMIMAGE  imi; 
	GetCursorPos(&lpClickPoint);
	hPopMenu = CreatePopupMenu();
	SendMessage( (HWND)hWnd, WM_LBUTTONDOWN, wParam, lParam );	

	hSubPopMenu = CreatePopupMenu();
	hSubForceLaunchPopMenu = CreatePopupMenu();
	hSubClipboardPopMenu  = CreatePopupMenu();
	hSubMonitorPopMenu  = CreatePopupMenu();

	int n=-1;
	n = ListView_GetSelectionMark(g_hwndListViewServer);
	if(n!=-1)
	{			
		SERVER_INFO *pSI = Get_ServerInfoByListViewIndex(currCV,n); 

		AppendMenu(hPopMenu,MF_BYPOSITION|MF_STRING,IDM_CONNECT,g_lang.GetString("MenuConnect"));
		AppendMenu(hPopMenu,MF_BYPOSITION|MF_STRING,IDM_LAUNCH_GAME_ONLY,g_lang.GetString("MenuLaunchGameOnly"));
		AppendMenu(hPopMenu,MF_POPUP|MF_BYPOSITION|MF_STRING,(UINT_PTR)hSubForceLaunchPopMenu,g_lang.GetString("MenuForceLaunch"));
		for(int x=0; x<currCV->vGAME_INST.size();x++)
			AppendMenu(hSubForceLaunchPopMenu,MF_BYPOSITION|MF_STRING,36000+x,currCV->vGAME_INST.at(x).sName.c_str());

		AppendMenu(hPopMenu,MF_BYPOSITION|MF_STRING,IDM_CONNECT_AS_SPEC,"Connect As Spectator");
	
		AppendMenu(hPopMenu,MF_BYPOSITION|MF_SEPARATOR,0,"");
	
		if(pSI->cFavorite==0)
			AppendMenu(hPopMenu,MF_BYPOSITION|MF_STRING,IDM_ADDIP,g_lang.GetString("MenuAddServerToFavorites"));
		else
			AppendMenu(hPopMenu,MF_BYPOSITION|MF_STRING,IDM_ADDIP,g_lang.GetString("MenuRemoveFromFavorites"));

		AppendMenu(hPopMenu,MF_BYPOSITION|MF_STRING,IDM_DELETE,g_lang.GetString("MenuDeleteServer"));


		AppendMenu(hPopMenu,MF_BYPOSITION|MF_STRING,IDM_PRIVPASS,g_lang.GetString("MenuSetPrivatePass"));
					
		AppendMenu(hPopMenu,MF_BYPOSITION|MF_STRING,IDM_COPYIP,g_lang.GetString("MenuCopyIP"));

		AppendMenu(hPopMenu,MF_BYPOSITION|MF_STRING,ID_YAWN_SERVER,g_lang.GetString("MenuYAWN"));
	
		AppendMenu(hPopMenu,MF_BYPOSITION|MF_SEPARATOR,0,"");

		AppendMenu(hPopMenu,MF_POPUP|MF_BYPOSITION|MF_STRING,(UINT_PTR)hSubMonitorPopMenu,g_lang.GetString("MenuMonitor"));
		DWORD dwFlag = MF_BYPOSITION|MF_STRING;
		if((pSI->wMonitor & MONITOR_AUTOJOIN))
			dwFlag = MF_CHECKED|MF_BYPOSITION|MF_STRING;

		AppendMenu(hSubMonitorPopMenu,dwFlag,IDM_MONITOR_AUTO_JOIN,g_lang.GetString("MenuMonitorAutoJoin"));

		if((pSI->wMonitor & MONITOR_NOTIFY_FREE_SLOTS))
			dwFlag = MF_CHECKED|MF_BYPOSITION|MF_STRING;
		else
			dwFlag = MF_BYPOSITION|MF_STRING;

		AppendMenu(hSubMonitorPopMenu,dwFlag,IDM_MONITOR_NOTIFY_WHEN_SLOT_FREE,g_lang.GetString("MenuMonitorNotifyFreeSlots"));
		if((pSI->wMonitor & MONITOR_NOTIFY_ACTIVITY))
			dwFlag = MF_CHECKED|MF_BYPOSITION|MF_STRING;
		else
			dwFlag = MF_BYPOSITION|MF_STRING;
		AppendMenu(hSubMonitorPopMenu,dwFlag,IDM_MONITOR_NOTIFY_WHEN_ACTIVITY,g_lang.GetString("MenuMonitorNotifyActivity"));

		
		//ImageMenu_SetStyle(OFFICE2007);
	}										



	AppendMenu(hPopMenu,MF_BYPOSITION|MF_STRING,IDM_OPTIONS_RCON,g_lang.GetString("MenuRCON"));



	if(g_bRunningQueryServerList==false)
		AppendMenu(hPopMenu,MF_BYPOSITION|MF_STRING,IDM_REFRESH,g_lang.GetString("MenuRefresh"));
	else
		AppendMenu(hPopMenu,MF_BYPOSITION|MF_STRING|MF_GRAYED,IDM_REFRESH,g_lang.GetString("MenuRefresh"));				

	AppendMenu(hPopMenu,MF_BYPOSITION|MF_STRING,IDM_REFRESH_SELECTED,g_lang.GetString("MenuRefreshSelected"));
	
	AppendMenu(hPopMenu,MF_BYPOSITION|MF_SEPARATOR,0,"");

	AppendMenu(hPopMenu,MF_POPUP|MF_BYPOSITION|MF_STRING,(UINT_PTR)hSubClipboardPopMenu,g_lang.GetString("MenuCopyToClipboard"));
	AppendMenu(hSubClipboardPopMenu,MF_BYPOSITION|MF_STRING,IDM_COPY_VERSION,g_lang.GetString("ColumnVersion"));
	AppendMenu(hSubClipboardPopMenu,MF_BYPOSITION|MF_STRING,IDM_COPY_MODNAME,g_lang.GetString("ColumnMod"));


	AppendMenu(hPopMenu,MF_BYPOSITION|MF_SEPARATOR,0,"");
	AppendMenu(hPopMenu,MF_POPUP|MF_BYPOSITION|MF_STRING,(UINT_PTR)hSubPopMenu,g_lang.GetString("MenuNetworkTools"));			
	AppendMenu(hSubPopMenu,MF_BYPOSITION|MF_STRING,IDM_PING_SERVER,g_lang.GetString("MenuPing"));
	AppendMenu(hSubPopMenu,MF_BYPOSITION|MF_STRING,IDM_TRACEROUTE_SERVER,g_lang.GetString("MenuTraceRoute"));
	
	AppendMenu(hPopMenu,MF_BYPOSITION|MF_STRING,IDM_FILTER_DEBUG,"Debug filter on selected server");

	
	ImageMenu_CreatePopup((HWND)hWnd, hPopMenu); 
	imi.mask = IMIMF_LOADFROMRES|IMIMF_ICON; 
	imi.hInst = GetModuleHandle(NULL); 
	imi.itemID = IDM_ADDIP; 
	imi.imageStr = MAKEINTRESOURCE(IDI_ICON_FAVORITES); 
	ImageMenu_SetItemImage(&imi); 
	imi.itemID = IDM_DELETE; 
	imi.imageStr = MAKEINTRESOURCE(IDI_ICON_ERROR); 
	ImageMenu_SetItemImage(&imi); 
	imi.itemID = (UINT)hSubMonitorPopMenu; 
	imi.imageStr = MAKEINTRESOURCE(IDI_ICON_ALARM_CLOCK); 
	ImageMenu_SetItemImage(&imi); 

	imi.itemID = IDM_OPTIONS_RCON; 
	imi.imageStr = MAKEINTRESOURCE(IDI_ICON_RCON); 
	ImageMenu_SetItemImage(&imi); 

	//SetForegroundWindow((HWND)hWnd);
//	TrackPopupMenu(hPopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,lpClickPoint.x, lpClickPoint.y,0,(HWND)hWnd,NULL);

	//TrackPopupMenu(popupMenu, ...); 
	TrackPopupMenu(hPopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,lpClickPoint.x, lpClickPoint.y,0,(HWND)hWnd,NULL);
	//Remember to call it before calling DestroyMenu! 
	ImageMenu_Remove(hPopMenu); 

	SendMessage((HWND)hWnd,WM_NULL,0,0);
	DestroyMenu(hSubMonitorPopMenu);		
	DestroyMenu(hSubPopMenu);
	DestroyMenu(hSubForceLaunchPopMenu);
	DestroyMenu(hSubClipboardPopMenu);
	DestroyMenu(hPopMenu);

	DeleteObject(hBmp);
	return 0;
}

void OnAddFavorites()
{
	int n=-1;
	SERVER_INFO *pSI = NULL; 
	
	while ((n =  ListView_GetNextItem(g_hwndListViewServer,n,LVNI_SELECTED))!=-1)
	{		
		pSI = Get_ServerInfoByListViewIndex(currCV,n);	
		if(pSI!=NULL)
		{
			pSI->cFavorite =! pSI->cFavorite;							
			currCV->vSI.at((int)pSI->dwIndex) = pSI;
			UpdateServerItem(n);
		}
	}
}
void OnDeleteServers()
{
	int n=-1;
	SERVER_INFO *pSI = NULL; 
	
	while ((n =  ListView_GetNextItem(g_hwndListViewServer,n,LVNI_SELECTED))!=-1)
	{		
		pSI = Get_ServerInfoByListViewIndex(currCV,n);	
		if(pSI!=NULL)
		{
			pSI->cPurge = 255;
		}
	}
	RedrawServerListThread((LPVOID)g_currentGameIdx);
}

void OnMonitorNotifyFreeSlots()
{
	int n=-1;
	SERVER_INFO *pSI = NULL; 
	
	while ((n =  ListView_GetNextItem(g_hwndListViewServer,n,LVNI_SELECTED))!=-1)
	{		
		pSI = Get_ServerInfoByListViewIndex(currCV,n);	
		if(pSI!=NULL)
		{
			if(pSI->wMonitor & MONITOR_NOTIFY_FREE_SLOTS)
			{
				pSI->wMonitor ^= MONITOR_NOTIFY_FREE_SLOTS;
				g_vMonitorSI.erase(find(g_vMonitorSI.begin(),g_vMonitorSI.end(),pSI));
			}
			else
			{
				pSI->wMonitor |= MONITOR_NOTIFY_FREE_SLOTS;
				g_vMonitorSI.push_back(pSI);
				if(hTimerMonitor==NULL)
					hTimerMonitor = SetTimer(g_hWnd,IDT_MONITOR_QUERY_SERVERS,MONITOR_INTERVAL,0);	
			}
			ListView_Update(g_hwndListViewServer,n);
		}
	}
}

void OnMonitorNotifyActivity()
{
	int n=-1;
	SERVER_INFO *pSI = NULL; 
	
	while ((n =  ListView_GetNextItem(g_hwndListViewServer,n,LVNI_SELECTED))!=-1)
	{		
		pSI = Get_ServerInfoByListViewIndex(currCV,n);	
		if(pSI!=NULL)
		{
			if(pSI->wMonitor & MONITOR_NOTIFY_ACTIVITY)
			{
				pSI->wMonitor ^= MONITOR_NOTIFY_ACTIVITY;
				g_vMonitorSI.erase(find(g_vMonitorSI.begin(),g_vMonitorSI.end(),pSI));
			}
			else
			{

				pSI->wMonitor |= MONITOR_NOTIFY_ACTIVITY;
				g_vMonitorSI.push_back(pSI);
				if(hTimerMonitor==NULL)
					hTimerMonitor = SetTimer(g_hWnd,IDT_MONITOR_QUERY_SERVERS,MONITOR_INTERVAL,0);	
			}
			ListView_Update(g_hwndListViewServer,n);
		}
	}
}


void OnMonitorNotifyAutoJoin()
{
	int n=-1;
	SERVER_INFO *pSI = NULL; 
	
	while ((n =  ListView_GetNextItem(g_hwndListViewServer,n,LVNI_SELECTED))!=-1)
	{		
		pSI = Get_ServerInfoByListViewIndex(currCV,n);	
		if(pSI!=NULL)
		{			
			if(pSI->wMonitor & MONITOR_AUTOJOIN)
			{
				pSI->wMonitor ^= MONITOR_AUTOJOIN;
				g_vMonitorSI.erase(find(g_vMonitorSI.begin(),g_vMonitorSI.end(),pSI));
			}
			else
			{
				pSI->wMonitor |= MONITOR_AUTOJOIN;
				g_vMonitorSI.push_back(pSI);
				if(hTimerMonitor==NULL)
					hTimerMonitor = SetTimer(g_hWnd,IDT_MONITOR_QUERY_SERVERS,MONITOR_INTERVAL,0);	

			}							
			ListView_Update(g_hwndListViewServer,n);		
		}
	}
}


LRESULT APIENTRY ListView_SL_SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 

	HMENU hPopMenu = NULL;
	HMENU hSubPopMenu = NULL;
	HMENU hSubForceLaunchPopMenu= NULL;
	HMENU hSubClipboardPopMenu = NULL;		
	HMENU hSubMonitorPopMenu = NULL;
	
	switch(uMsg)
	{
		case WM_KILLFOCUS:
			{
			//	int n = ListView_GetSelectionMark(g_hwndListViewServer);
			//	ListView_SetSelectedColumn(g_hwndListViewServer,n);
				UnregisterHotKey(g_hwndListViewServer,HOTKEY_ID_CTRL_C);
				return 0;
			}
		case WM_SAVE_SERVERLIST_COLUMN_STATE:
			g_LVHeaderSL->Update();
			g_LVHeaderSL->Save();
			g_LVHeaderSL->DUMP_COL_ORDER();
			return TRUE;

		case WM_NOTIFY:
			{
				NMHEADER *pNMheader;  //List View Header Control
				pNMheader = (LPNMHEADER)lParam;
		
				switch(pNMheader->hdr.code)
				{		
					case HDN_BEGINTRACK:
					case HDN_BEGINTRACKW:
					
						return g_LVHeaderSL->OnBeginTrack(lParam);
					case HDN_TRACK:
					case HDN_ENDTRACKW:
					case HDN_ENDTRACK:
						{
							g_LVHeaderSL->OnEndTrack(lParam);
						}
						break;
					case HDN_BEGINDRAG:

						return FALSE;
					case HDN_ENDDRAG:
						{
							g_LVHeaderSL->OnEndDrag();
							PostMessage(g_hWnd,WM_REFRESHSERVERLIST,0,0);
							//PostMessage(g_hWnd,WM_UPDATESERVERLIST_COLUMNS,0,0);
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
				SetStatusText(ICO_INFO,g_lang.GetString("IPAddedToClipBoard"),pszIP);
			}else
			{
				SetStatusText(ICO_WARNING,g_lang.GetString("ServerCopyToClipBoard"));
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
			//Enumerate game intalls if any of those to be forced launched
			for(UINT x=0; x<currCV->vGAME_INST.size();x++)
			{
				if(wmId == 36000+x)
				{
					int i = ListView_GetSelectionMark(g_hwndListViewServer);
					if(i!=-1)
					{
						SERVER_INFO *pSrv;	
						pSrv = Get_ServerInfoByListViewIndex(currCV,i);
						LaunchGame(pSrv,&gm.GamesInfo[g_currentGameIdx],x,NULL,TRUE);
					}
				}
			}

			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_FILTER_DEBUG:
					{
						int n = ListView_GetSelectionMark(g_hwndListViewServer);
						if(n!=-1)
						{
							SERVER_INFO *pSI = Get_ServerInfoByListViewIndex(currCV,n); 	
							if(pSI!=NULL)
							{
								vFILTER_SETS vFilterSets;
								//vFilterSets = gm.GamesInfo[pSI->cGAMEINDEX].vFilterSets;
								vFilterSets.insert(vFilterSets.end(),gm.GetFilterSet(GLOBAL_FILTER).begin(),gm.GetFilterSet(GLOBAL_FILTER).end());

								FilterServerItemV2(pSI,&gm.GamesInfo[pSI->cGAMEINDEX],&vFilterSets);
							}
						}
					}
				break;
				case IDM_SCAN:				OnActivate_ServerList();			break;
				case IDM_OPTIONS_RCON:		OnRCON();							break;
				case IDM_REFRESH:			OnActivate_ServerList();			break;
				case IDM_CONNECT:			StartGame_ConnectToServer(false);	break;
				case IDM_CONNECT_AS_SPEC:
					{
						int i = ListView_GetSelectionMark(g_hwndListViewServer);
						if(i!=-1)
						{
							SERVER_INFO *pSrv;	
							pSrv = Get_ServerInfoByListViewIndex(currCV,i);
							LaunchGame(pSrv,&gm.GamesInfo[g_currentGameIdx],0,"+spectator 1",TRUE);

						}
					}
						break;
				case IDM_REFRESH_SELECTED:	Initialize_ScanSelection();			break;
				case IDM_PRIVPASS:
					{
						int i = ListView_GetSelectionMark(g_hwndListViewServer);
						if(i==-1)
						{
							MessageBox(NULL,g_lang.GetString("ErrorPrivatePassword"),"Info!",MB_ICONINFORMATION|MB_OK); 
							return TRUE;
						}
						g_PRIVPASSsrv = Get_ServerInfoByListViewIndex(currCV,i);						
						if(g_PRIVPASSsrv!=NULL)
							DialogBox(g_hInst, (LPCTSTR)IDD_DLG_SETPRIVPASS, g_hWnd, (DLGPROC)PRIVPASS_Proc);									

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

				case IDM_DELETE:	OnDeleteServers();			break;
				case IDM_ADDIP:		OnAddFavorites();			break;
				case IDM_PING_SERVER:
					{
						OnTabControlSelection(3);
						SendMessage(g_hwndMainSTATS,WM_START_PING,0,0);					
					}
					break;
				case IDM_TRACEROUTE_SERVER:
					{						
						OnTabControlSelection(3);
						SendMessage(g_hwndMainSTATS,WM_START_TRACERT,0,0);

					}
					break;
				case IDM_MONITOR_AUTO_JOIN:	OnMonitorNotifyAutoJoin();				break;
				case IDM_MONITOR_NOTIFY_WHEN_ACTIVITY:	OnMonitorNotifyActivity();	break;
				case IDM_MONITOR_NOTIFY_WHEN_SLOT_FREE:	OnMonitorNotifyFreeSlots();	break;
				case IDM_COPY_VERSION:
					{
						int n = ListView_GetSelectionMark(g_hwndListViewServer);
						if(n!=-1)
						{
							SERVER_INFO *pSI = Get_ServerInfoByListViewIndex(currCV,n);								
							EditCopy(pSI->szVersion);

						}
						else
							MessageBox(hwnd,g_lang.GetString("ErrorServerCopyToClipBoard"),NULL,MB_OK);
					}
				break;	
				case IDM_COPY_MODNAME:
					{
						int n=-1;
						n = ListView_GetSelectionMark(g_hwndListViewServer);
						if(n!=-1)
						{
							SERVER_INFO *pSI = Get_ServerInfoByListViewIndex(currCV,n);								
							EditCopy(pSI->szMod);

						}
						else
							MessageBox(hwnd,g_lang.GetString("ErrorServerCopyToClipBoard"),NULL,MB_OK);
					}
					break;
				case IDM_COPYIP:
					{
						char *pszIP = Get_SelectedServerIP();
						if(pszIP!=NULL)
							EditCopy(pszIP);
						else
							MessageBox(hwnd,g_lang.GetString("ErrorServerCopyToClipBoard"),NULL,MB_OK);
					}
				break;				

			}
		}
 		else if(uMsg == WM_RBUTTONDOWN)
		{
			//get mouse cursor position x and y as lParam has the message itself 
	
			//Init_SL_Menu(hwnd);
			Menu_SL_Thread(hwnd, wParam, lParam );
			return 0;
		}	

    return CallWindowProcW((WNDPROC)g_wpOrigListViewServerProc, hwnd, uMsg,  wParam, lParam);  
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
            (cch + 1) * sizeof(char)); 
        if (hglbCopy == NULL) 
        { 
            CloseClipboard(); 
            return FALSE; 
        } 
 
        // Lock the handle and copy the text to the buffer. 
 
        lptstrCopy = (LPTSTR)GlobalLock(hglbCopy); 
        memcpy(lptstrCopy,(LPTSTR)pText,cch); 
        lptstrCopy[cch] = (char) 0;    // null character 
        GlobalUnlock(hglbCopy); 
 
        // Place the handle on the clipboard. 
 
        SetClipboardData(CF_TEXT, hglbCopy); 
    } 
 
    // Close the clipboard. 
 
    CloseClipboard(); 
 
    return TRUE; 
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
			log.AddLogInfo(GS_LOG_DEBUG,WETFolder);
			log.AddLogInfo(GS_LOG_DEBUG,LoadLocation);
			hret = ShellExecute(NULL, "open", LoadLocation, szCmd,WETFolder, 1);

			if((int)hret<=32)
			{
				MessageBox(NULL,g_lang.GetString("ErrorLaunchingGame"),"Error!",MB_OK);
				return FALSE;
			}
			return TRUE;
		}
	}

	MessageBox(NULL,g_lang.GetString("ErrorLaunchingGame"),"Error!",MB_OK);	

	return FALSE;
}

void OnTabControlSelection(int iSelection)
{
	
	int iSel = iSelection;
	if( iSel==-1)
		iSel = TabCtrl_GetCurSel(g_hwndTabControl);

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
	TabCtrl_SetCurSel(g_hwndTabControl,iSel);
}

BOOL OnTreeViewSelectionChanged(LPARAM lParam)
{
	g_iCurrentSelectedServer=-1;
	int i =  tvmgr.OnSelection(lParam);
	switch(i)
	{
		case DO_NOTHING:
			return TRUE;
		case SHOW_CUSTOM_FAVORITE_FILTER:
			gm.GamesInfo[g_currentGameIdx].dwViewFlags = REDRAWLIST_CUSTOM_FILTER;
			break;
		case SHOW_FAVORITES_PUBLIC:
			gm.GamesInfo[g_currentGameIdx].dwViewFlags = REDRAWLIST_FAVORITES_PUBLIC ;
			break;								
		case SHOW_FAVORITES_PRIVATE:
			gm.GamesInfo[g_currentGameIdx].dwViewFlags = REDRAWLIST_FAVORITES_PRIVATE ;
			break;	
		case SHOW_HISTORY:
			gm.GamesInfo[g_currentGameIdx].dwViewFlags = REDRAWLIST_HISTORY;
			break;
		case SHOW_MONITOR:
			gm.GamesInfo[g_currentGameIdx].dwViewFlags = REDRAW_MONITOR_SERVERS;
			break;			
		case REDRAW_CURRENT_LIST: 
			gm.GamesInfo[g_currentGameIdx].dwViewFlags = 0;
			break;						
		default:  //a game is selected
			{
				gm.GamesInfo[g_currentGameIdx].dwViewFlags = 0;
				if(gm.GamesInfo[g_currentGameIdx].vSI.size()==0) //if empty let's download server list from master
				{
					OnActivate_ServerList();
					return TRUE;
				} else
				{
					time_t currTime;
					time(&currTime);
					double seconds = difftime(currTime,gm.GamesInfo[g_currentGameIdx].lastScanTimeStamp);
					dbg_print("Seconds since last scan %f",seconds);
				//	if((seconds>(60*5)) || (seconds==0))
					{					
						gm.GamesInfo[g_currentGameIdx].dwViewFlags |= FORCE_SCAN_FILTERED;
						OnActivate_ServerList(SCAN_FILTERED);
						return TRUE;
					}				
				}
			}
			break;

	} //end switch
	Initialize_RedrawServerListThread();
	return TRUE;
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
					if(pNMHdr->hwndFrom == g_hwndTabControl)
						OnTabControlSelection();
					break;
				}

			//case TBN_DROPDOWN:
			//	OnScanningOptions(hWnd);
			//break;
		
			case LVN_GETDISPINFO:
				if(lpnmia->hdr.hwndFrom == g_hwndListViewServer)
					return ListView_SL_OnGetDispInfoList(wParam,(NMHDR*)lParam);
				else if(lpnmia->hdr.hwndFrom == g_hwndListViewPlayers)
					return  ListView_PL_OnGetDispInfoList(wParam, (NMHDR*)lParam);

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
							lpttt->lpszText = (LPSTR)g_lang.GetString("ScanAll"); //MAKEINTRESOURCE(IDS_REFRESH); 
							break; 
						case IDM_SCAN_FILTERED:
							lpttt->lpszText = (LPSTR)g_lang.GetString("ScanFiltered"); //MAKEINTRESOURCE(IDS_REFRESH_FILTER); 
							break;
						case IDM_SETTINGS: 
							lpttt->lpszText = (LPSTR)g_lang.GetString("SettingsButton"); // MAKEINTRESOURCE(IDS_OPTIONS); 
							break; 
						case IDC_BUTTON_ADD_SERVER: 
							lpttt->lpszText =(LPSTR)g_lang.GetString("AddToFavorites"); // MAKEINTRESOURCE(IDS_ADD_TO_FAVORITES); 
							break; 
						case IDC_BUTTON_QUICK_CONNECT: 
							lpttt->lpszText = (LPSTR)g_lang.GetString("QuickConnect"); //MAKEINTRESOURCE(IDS_FAST_CONNECT); 
							break; 
						case IDC_BUTTON_FIND: 
							lpttt->lpszText =(LPSTR)g_lang.GetString("SearchButton"); // MAKEINTRESOURCE(IDS_SEARCH); 
							break; 
						case IDC_DOWNLOAD: 
							lpttt->lpszText =(LPSTR)g_lang.GetString("DownloadButton"); // MAKEINTRESOURCE(IDS_DOWNLOAD); 
							break; 
						case IDM_FONT_COLOR: 
							lpttt->lpszText = (LPSTR)g_lang.GetString("FontButton"); //MAKEINTRESOURCE(IDS_COLOR_FONT); 
							break; 
						case ID_BUDDY_ADD:
							lpttt->lpszText =(LPSTR)g_lang.GetString("AddNewBuddyButton"); // MAKEINTRESOURCE(IDS_ADD_NEW_BUDDY); 
							break; 

					} 
					break; 
				} 
		}

		switch(LOWORD(wParam))
		{
			case IDC_MAINTREE:
				{
				  if(pnmtv->hdr.code == TVN_BEGINDRAG )
				  {
						OnBeginDrag(pNMHdr);
						return TRUE;
				  }

 

				if(pnmtv->hdr.code == TVN_BEGINLABELEDIT)
				  {
					TVITEM  tvitem;
					ZeroMemory(&tvitem,sizeof(TVITEM));
					tvitem.hItem = TreeView_GetSelection(g_hwndMainTreeCtrl);
					tvitem.mask = TVIF_PARAM;
					TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );
					DWORD dwType = tvmgr.vTI.at(tvitem.lParam).dwType;
					if(((dwType>=13 && dwType<=40) || (dwType==0)) && g_bBeginEdit)
						hEdit=TreeView_GetEditControl(g_hwndMainTreeCtrl);
					else
					{
						g_bBeginEdit = FALSE;
						return TRUE;
					}
					g_bBeginEdit = FALSE;
				  }

				  if(pnmtv->hdr.code== TVN_ENDLABELEDIT)
				  {
					  	TVITEM  tvi;
					char Text[256]="";
					ZeroMemory(&tvi,sizeof(TVITEM));
					tvi.mask = TVIF_TEXT |TVIF_PARAM;
					tvi.cchTextMax = 256;
					tvi.hItem=TreeView_GetSelection(g_hwndMainTreeCtrl);
					SendDlgItemMessage(hWnd,IDC_MAINTREE,TVM_GETITEM,0,(WPARAM)&tvi);
					GetWindowText(hEdit, Text, sizeof(Text));
					tvi.pszText=Text;
					SendDlgItemMessage(hWnd,IDC_MAINTREE,TVM_SETITEM,0,(WPARAM)&tvi);
					tvmgr.vTI.at(tvi.lParam).sName = Text;
					g_bBeginEdit = FALSE;
				  }
					LPNMHDR lpnmh = (LPNMHDR) lParam;
					if(lpnmh->code == NM_CLICK)
					{
						OnTreeViewSelectionChanged(lParam);
						return FALSE;
					}
				    if(pnmtv->hdr.code== TVN_ITEMEXPANDED)
					{
						UINT action = pnmtv->action;

						int i = tvmgr.GetIndex(pnmtv->itemNew.hItem);						
						if(i!=-1)
						{
							if(action==TVE_EXPAND)
								tvmgr.vTI.at(i).bExpanded = true;
							else if(action==TVE_COLLAPSE)
							{
								tvmgr.vTI.at(i).bExpanded = false;
								tvmgr.SetAllChildItemExpand(i, false);
							}
							
						//	log.AddLogInfo(GS_LOG_DEBUG,"Expanded %s Action %d",tvmgr.vTI.at(i).sName.c_str(),action);
						}
					}

					if(pnmtv->hdr.code == TVN_SELCHANGED || pnmtv->hdr.code == TVN_SELCHANGING)
					{
						if(pnmtv->action != TVC_BYMOUSE )
							return FALSE;
						 
					//	OnTreeViewSelectionChanged(lParam);
					//	return FALSE;
						
					}					
				}
		
			
		  default:
		{
			if((lpnmia->hdr.code == NM_SETFOCUS) && (lpnmia->hdr.hwndFrom == g_hwndListViewServer))
				{
					if (!RegisterHotKey(g_hwndListViewServer, HOTKEY_ID_CTRL_C, MOD_CONTROL, 0x43))
						log.AddLogInfo(GS_LOG_WARNING,"Couldn't register CTRL+V hotkey.");							
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
					bm.OnBuddySelected();
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
					return ListView_SL_CustomDraw(lParam);					
				else  if(lpnmia->hdr.code == NM_CUSTOMDRAW && (lpnmia->hdr.hwndFrom == g_hwndMainTreeCtrl))
					return TreeView_CustomDraw(lParam);					
				else  if(lpnmia->hdr.code == LVN_COLUMNCLICK && lpnmia->hdr.hwndFrom == g_hwndListViewServer)
				{		
				NMLISTVIEW* lstvw;
					lstvw = (NMLISTVIEW*) lParam; 
/*					for(int i=0;i<MAX_COLUMNS;i++)
					{
						if(CUSTCOLUMNS[i].columnIdx == lstvw->iSubItem)
						{						
							CUSTCOLUMNS[i].bSortAsc = ! CUSTCOLUMNS[i].bSortAsc;
							break;
						}
					}	
					*/
					g_LVHeaderSL->SwapSortAsc(lstvw->iSubItem);
					Do_ServerListSortThread(lstvw->iSubItem);
					return TRUE;
				}else if(lpnmia->hdr.code == LVN_COLUMNCLICK && lpnmia->hdr.hwndFrom == g_hwndListBuddy)
				{
					NMLISTVIEW* lstvw = (NMLISTVIEW*) lParam; 
				
					AppCFG.bSortBuddyAsc = !AppCFG.bSortBuddyAsc;
					switch(lstvw->iSubItem)
					{
						case 0:sort(bm.BuddyList.begin(),bm.BuddyList.end(),Buddy_Sort_Name); break;
						case 1:sort(bm.BuddyList.begin(),bm.BuddyList.end(),Buddy_Sort_ServerName); break;
					}				
					
					bm.UpdateList();
					return TRUE;
				}else if(lpnmia->hdr.code == LVN_COLUMNCLICK && lpnmia->hdr.hwndFrom == g_hwndListViewPlayers)
				{
					NMLISTVIEW* lstvw  = (NMLISTVIEW*) lParam; 	
					bPlayerClanAsc= !bPlayerClanAsc;
					switch(lstvw->iSubItem)
					{
						case 2:AppCFG.bPlayerNameAsc = !AppCFG.bPlayerNameAsc; sort(g_vecPlayerList.begin(),g_vecPlayerList.end(),PlayerList_Sort_Name); break;
						case 3:bRateAsc = !bRateAsc; sort(g_vecPlayerList.begin(),g_vecPlayerList.end(),PlayerList_Sort_Rate); break;
						case 4:bPlayerPingAsc= !bPlayerPingAsc; sort(g_vecPlayerList.begin(),g_vecPlayerList.end(),PlayerList_Sort_Ping); break;
						case 6:AppCFG.bSortPlayerServerNameAsc = !AppCFG.bSortPlayerServerNameAsc; sort(g_vecPlayerList.begin(),g_vecPlayerList.end(),PlayerList_Sort_ServerName); break;
					}	
					
					ListView_SetItemCount(g_hwndListViewPlayers, g_vecPlayerList.size());					
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

	g_hAppIcon = LoadIcon(hInstance,(LPCTSTR)MAKEINTRESOURCE(IDI_GAMESCANNER)); 

 	GetModuleFileName ( NULL, EXE_PATH, MAX_PATH*2 );
	char *p = strrchr(EXE_PATH,'\\');
	if(p!=NULL)
		p[0]=0;


	char *pszCmd=NULL;
	pszCmd = strchr(lpCmdLine,'/');
	if(pszCmd!=NULL)
	{
		if(strcmp(pszCmd,"/portable")==0)
			strcpy(USER_SAVE_PATH,	EXE_PATH);
		 else
			SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,SHGFP_TYPE_CURRENT,USER_SAVE_PATH);
	} else
		SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,SHGFP_TYPE_CURRENT,USER_SAVE_PATH);

	//hack for GS 2.x beta program
//	strcpy(USER_SAVE_PATH,	EXE_PATH);
	
	SHGetFolderPath(NULL,CSIDL_COMMON_APPDATA,NULL,SHGFP_TYPE_CURRENT,COMMON_SAVE_PATH);
	 
	strcat(USER_SAVE_PATH,"\\GameScannerData\\");
	strcat(COMMON_SAVE_PATH,"\\GameScannerData\\");
	
	wchar_t wcPath[MAX_PATH];

	ZeroMemory(wcPath,sizeof(wcPath));
	mbstowcs(wcPath,(const char *)USER_SAVE_PATH,strlen(USER_SAVE_PATH));	
	SHCreateDirectory(NULL,wcPath);

	ZeroMemory(wcPath,sizeof(wcPath));
	mbstowcs(wcPath,(const char *)COMMON_SAVE_PATH,strlen(COMMON_SAVE_PATH));	
	SHCreateDirectory(NULL,wcPath);

	//ShGetKnownFolderPath 
	//g_lang = new CLanguage(log);

	log.SetLogPath(USER_SAVE_PATH);
	SetCurrentDirectory(USER_SAVE_PATH);

	//LOGGER_Init();

	log.AddLogInfo(GS_LOG_INFO,"Initializing Game Scanner version "APP_VERSION);
	log.AddLogInfo(GS_LOG_INFO,"Executable directory: %s",EXE_PATH);
	log.AddLogInfo(GS_LOG_INFO,"User Data directory: %s",USER_SAVE_PATH);	
	//log.AddLogInfo(GS_LOG_INFO,"Common Data directory: %s",COMMON_SAVE_PATH);	
	log.AddLogInfo(GS_LOG_INFO,"Cmd line input %s",lpCmdLine);

	g_IPtoCountry.SetPath(EXE_PATH);	
	char szPath[512];
	strcpy(szPath,EXE_PATH);
	strcat(szPath,"\\languages");
	g_lang.SetPath(szPath);
	g_xmlcfg.SetPath(EXE_PATH);


	g_hInst = hInstance; // Store instance handle in our global variable


	//Do the conversion of the IP to country database 
//#ifdef CONVERTIPDATABASE
	SetCurrentDirectory(EXE_PATH);
	
	if(g_IPtoCountry.ConvertDatabase()==0)
		log.AddLogInfo(GS_LOG_INFO,"Updated IP to Country file.");
	SetCurrentDirectory(USER_SAVE_PATH);
//#endif
	
	//fnIPtoCountryInit();
	g_IPtoCountry.LoadTable();

	memset((void*)&etMode,0,sizeof(DEVMODE));

	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR)
	{
	  log.AddLogInfo(GS_LOG_INFO,"Error at WSAStartup()");
	}

	LoadImageList();
	CFG_Load();

	if(AppCFG.bAutostart)  //auto run up on boot
		AddAutoRun(EXE_PATH);
	else
		RemoveAutoRun();

	pszCmd = strchr(lpCmdLine,'/');
	if(pszCmd!=NULL)
	{
		if(strcmp(pszCmd,"/tasktray")==0)
		{
			g_bMinimized = true;
			g_bNormalWindowed = false;
		}
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
			srv.usPort = dwPort;
			
			char *cutoff;
			cutoff = strchr(proto,':');
			if(cutoff!=NULL)
			{
			//	Setup_Appsettings();
				cutoff[0]=0;
				for(char i=0;i<gm.GamesInfo.size(); i++)
				{
					if(strcmp(proto,gm.GamesInfo[i].szWebProtocolName)==0)
					{
						srv.cGAMEINDEX = i;	
						srv.dwIndex = 999999;  //work around to skip sync
						if(srv.usPort ==0)
							srv.usPort = gm.GamesInfo[i].dwDefaultPort;

						LaunchGame(&srv,&gm.GamesInfo[i]);
					}
				}
				return 0;
			}

		}
	}
	HWND hwndGS =  FindWindow(szWindowClass,szDialogTitle );
	//close multiple windows
	if(hwndGS!=NULL)
	{

		PostMessage(hwndGS,WM_COMMAND,LOWORD(IDM_OPEN),0);
		Sleep(500);
		SetForegroundWindow ((HWND) (((DWORD)hwndGS) | 0x01));  

		//MessageBox(NULL,g_lang.GetString("MessageInstance"),"Alert",MB_OK);
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

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		log.AddLogInfo(GS_LOG_INFO,"Error InitInstance");
		return FALSE;
	}

	CleanUpFilesRegistry();
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_ETSERVERVIEWER);
	//register hot key
	if(AppCFG.bUse_minimize)
	{		
		if (!RegisterHotKey(NULL, HOTKEY_ID, AppCFG.dwMinimizeMODKey , AppCFG.cMinimizeKey))
		{
			MessageBox(NULL,g_lang.GetString("ErrorRegHotkey"),"Hotkey error",NULL);			
			AppCFG.bUse_minimize = false;
		}
	}

	

	structNID.cbSize = sizeof(NOTIFYICONDATA); 
	structNID.hWnd = (HWND) g_hWnd; 
	structNID.uID = 100123; //ID of the icon that willl appear in the system tray 
	structNID.uFlags =  NIF_ICON | NIF_MESSAGE | NIF_TIP; 
	strcpy(structNID.szTip,"Game Scanner");
	structNID.hIcon = g_hAppIcon; 
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
			log.AddLogInfo(GS_LOG_INFO,"Error adding task tray icon. Please report this back to the developer.");
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

	dbg_print("Wait for save thread to finish ");
	DWORD dwEvent = WaitForSingleObject(g_hSaveThread, INFINITE);
	switch (dwEvent) 
	{
		case WAIT_OBJECT_0: 
			dbg_print("Success WaitForSingleObject @ func: %s\n",__FUNCTION__); 
			break; 
		case WAIT_TIMEOUT:
			dbg_print("Wait timed out. %s\n",__FUNCTION__); 
			break;
		case WAIT_ABANDONED:
				dbg_print("WAIT_ABANDONED: %s\n",__FUNCTION__); 
			break;
		default: 
			{
				dbg_print("Wait error @  WaitForSingleObject(g_hSaveThread, INFINITE)\n"); 
			}         
	}

	if(g_LVHeaderSL!=NULL)
		delete g_LVHeaderSL;

	CloseHandle( g_hSaveThread );
	dbg_print("Done.\n");

	DeleteCriticalSection(&REDRAWLIST_CS);
	DeleteCriticalSection(&LOAD_SAVE_CS);
	DeleteCriticalSection(&SCANNER_CSthreadcounter);
	DeleteCriticalSection(&SCANNER_cs);

	g_vecPlayerList.clear();
	CleanUp_PlayerList(pCurrentPL);
	

	if(AppCFG.bUse_minimize)
		UnregisterHotKey(NULL, HOTKEY_ID);
	

	CloseHandle(hCloseEvent);
	DestroyIcon(g_hAppIcon);
	WSACleanup();

	DestroyAcceleratorTable(hAccelTable); 
	log.AddLogInfo(GS_LOG_INFO,"Exit app..");

	
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
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);

	wcex.hbrBackground	= (HBRUSH) GetSysColorBrush(COLOR_3DFACE);//hBrush;

	wcex.lpszMenuName	= (LPCTSTR)IDC_GAMESCANNER;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

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


void LaunchGame(SERVER_INFO *pSI,GAME_INFO *pGI,int GameInstallIdx, char *szCustomCmd,BOOL bForceLaunch)
{
	char CommandParameters[512];
	ZeroMemory(CommandParameters,512);
	int typeRecognized = -1;

	if(pSI==NULL)
		return;

	if(pSI->pServerRules==NULL)
		gm.GetServerInfo(g_currentGameIdx,pSI);

	if(pSI->bPrivate==1)
	{
		g_PRIVPASSsrv = pSI;
		int ret = DialogBox(g_hInst, (LPCTSTR)IDD_DLG_SETPRIVPASS, g_hWnd, (DLGPROC)PRIVPASS_Proc);		
		if(ret!=IDOK)
			return;

	}

	if(pSI->nPlayers>=pSI->nMaxPlayers)
	{
		int retFullServer = MessageBox(g_hWnd,"The server is full would you like to connect anyway? (Ignore)\nOr add the server to try to auto join? (Retry)\nOtherwise abort.","Full server",MB_ABORTRETRYIGNORE | MB_ICONINFORMATION);
		if(retFullServer==IDRETRY)
		{
			if(pSI->wMonitor & MONITOR_AUTOJOIN)
			{
				//pSI->wMonitor ^= MONITOR_AUTOJOIN;
				//g_vMonitorSI.erase(find(g_vMonitorSI.begin(),g_vMonitorSI.end(),pSI));
			}
			else
			{
				pSI->wMonitor |= MONITOR_AUTOJOIN;
				g_vMonitorSI.push_back(pSI);
			}							
			int n=-1;
			if ((n =  ListView_GetNextItem(g_hwndListViewServer,n,LVNI_SELECTED))!=-1)
				ListView_Update(g_hwndListViewServer,n);
			return;
		}else if(retFullServer==IDABORT)
			return;
	}


	pSI->cHistory++;
	
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
	int iFirstDefaultInstallIdx=-1;
	BOOL bLaunchCondition=FALSE;
	if(GameInstallIdx==0 && bForceLaunch==FALSE)
	{
		for(int x=0; x<pGI->vGAME_INST.size();x++)
		{
			vFILTER_SETS vFilterSetsTemp;
			if(pGI->vGAME_INST.at(x).bActiveScript)
			{
				int iResult = se.CompileFilter(vFilterSetsTemp,pGI->vGAME_INST.at(x).sScript.c_str(),"temp","launchcond");
				if(vFilterSetsTemp.size()>0)
				{
					if(se.Execute(pSI,pGI,&vFilterSetsTemp)==TRUE)
					{
						GameInstallIdx = x;
						bLaunchCondition = TRUE;
						break;
					}
				}
			} else if (iFirstDefaultInstallIdx==-1)
				iFirstDefaultInstallIdx = x;
		}
	}

	if(bForceLaunch==FALSE)
	{
		if((bLaunchCondition==FALSE) && (iFirstDefaultInstallIdx!=-1)) 
			GameInstallIdx = iFirstDefaultInstallIdx; //First install with no active script
		else if((bLaunchCondition==FALSE) && (iFirstDefaultInstallIdx==-1))
		{
			SetStatusText(ICO_WARNING,"Server didn't pass any launch condition. Check your launch scripts or game path is missing.");
			return;
		}
	}


	string cmd;
	if(pGI->vGAME_INST.size()>0)
	{
		cmd = pGI->vGAME_INST.at(GameInstallIdx).szGAME_CMD;

		ReplaceStrInStr(cmd,"%MODNAME%",pSI->szMod);
		ReplaceStrInStr(cmd,"%FS_GAME%",pSI->szFS_GAME);

		char szTempPath[MAX_PATH*2];
		strcpy(szTempPath,pGI->vGAME_INST.at(GameInstallIdx).szGAME_PATH.c_str());
		char *p=strrchr(szTempPath,'\\');
			if(p!=NULL)
				p[0]=0;
		ReplaceStrInStr(cmd,"%GAMEPATH%",szTempPath);
	}

	if(strstr(cmd.c_str(),"applaunch")!=NULL)  //quick steam fix cmd has to be pre-concated
	{
		if(strlen(pSI->szPRIVATEPASS)>0)
			sprintf(CommandParameters,"%s +connect %s:%d +password %s ",cmd.c_str(),pSI->szIPaddress,pSI->usPort,pSI->szPRIVATEPASS);					
		else
			sprintf(CommandParameters,"%s +connect %s:%d ",cmd.c_str(),pSI->szIPaddress,pSI->usPort);
	}
	else
	{
		if(strlen(pSI->szPRIVATEPASS)>0)
			sprintf(CommandParameters,"+connect %s:%d +password %s %s ",pSI->szIPaddress,pSI->usPort,pSI->szPRIVATEPASS,cmd.c_str());					
		else
			sprintf(CommandParameters,"+connect %s:%d %s ",pSI->szIPaddress,pSI->usPort,cmd.c_str());					
	}


	if(szCustomCmd!=NULL) //This is used by the fast connect from the search field for custom command typical pasted from irc or similar
		strcat(CommandParameters,szCustomCmd);

	//log.AddLogInfo(0,CommandParameters);

	if(ExecuteGame(pGI,CommandParameters,GameInstallIdx))
	{
		if(hTimerMonitor!=NULL)
		{
			KillTimer(g_hWnd,IDT_MONITOR_QUERY_SERVERS);
			for(int iMon=0; iMon<g_vMonitorSI.size();iMon++)
			{
				SERVER_INFO *pSI = g_vMonitorSI.at(iMon);
				if(pSI!=NULL)
					pSI->wMonitor = 0;
			}
			g_vMonitorSI.clear();
			hTimerMonitor=NULL;
		}
		//A Successfull launch
		WINDOWPLACEMENT wp;
		GetWindowPlacement(g_hWnd, &wp);

		AppCFG.nWindowState = wp.showCmd;
		ShowWindow(g_hWnd,SW_MINIMIZE);
//		PostMessage(g_hWnd,WM_SIZE,SIZE_MINIMIZED,0);
		//OnMinimize(g_hWnd);
		if(AppCFG.bUseMIRC)
		{
			//Notify mIRC which server user will join
			DDE_Init();
  			char szMsg[350];
			
			string mircoutput;
			mircoutput = g_sMIRCoutput;
			int a = g_sMIRCoutput.find("/amsg");
			if(a==-1)
			{
				a = g_sMIRCoutput.find("/msg");
				if(a==-1)
					a = g_sMIRCoutput.find("/me");
					if(a==-1)
						a = g_sMIRCoutput.find("/ame");
						if(a==-1)
							mircoutput.insert(0,"/ame ");
			}

			char colfilter[120];
			colorfilter(pSI->szServerName,colfilter,119);
			ReplaceStrInStr(mircoutput,"%SERVERNAME%",colfilter);
			
			wsprintf(szMsg,"%s:%d",pSI->szIPaddress,pSI->usPort);	
			ReplaceStrInStr(mircoutput,"%IP%",szMsg);		
			ReplaceStrInStr(mircoutput,"%GAMENAME%",gm.GamesInfo[pSI->cGAMEINDEX].szGAME_NAME);
			
			if(pSI->bPrivate)
				ReplaceStrInStr(mircoutput,"%PRIVATE%","Private");
			else
				ReplaceStrInStr(mircoutput,"%PRIVATE%","Public");

			DDE_Send((char*)mircoutput.c_str());
			DDE_DeInit();
		}
		if(AppCFG.bCloseOnConnect)
			PostMessage(g_hWnd,WM_CLOSE,0,0);
		if (AppCFG.bUse_EXT_APP2)
		{
			KillTimer(g_hWnd,IDT_DETECT_GAME);
			SetTimer(g_hWnd,IDT_DETECT_GAME,10*1000,0);
		}
	}
				
}


BOOL DetectAnyRunningGame()
{

	if(FindGameWindow()!=NULL)
	{
		dbg_print("Game launched and detected!");
		return g_bGameRunning = TRUE;
	} 
	dbg_print("No known running games!");
	return FALSE;	
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
	
	Show_ToolbarButton(IDC_DOWNLOAD, false);
	BOOL bAnyUpdates=FALSE;
	SetCurrentDirectory(USER_SAVE_PATH);
	int ret = 0;

	if(IsServerAlive("www.cludden.se")==false)
	{
		log.AddLogInfo(0,"< Update Server is down! >");
		return ret;
	}
	OSVERSIONINFO OSversion;
		
	OSversion.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	::GetVersionEx(&OSversion);
	log.AddLogInfo(0,"OS Version: %d.%d",OSversion.dwMajorVersion,OSversion.dwMinorVersion);

	remove("update.xml");

	char szBuff[512];
	sprintf(szBuff,"http://www.bdamage.se/download/checkversion2.php?version=%s&osversion=%d.%d",APP_VERSION,OSversion.dwMajorVersion,OSversion.dwMinorVersion);

	g_download.SetPath(USER_SAVE_PATH);

	g_download.HttpFileDownload(szBuff,"update.xml",g_DlgProgress,g_DlgProgressMsg);

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
		log.AddLogInfo(ICO_INFO,"Error checking for new version (XML corrupt)!",szVersion);
		return 0;
	}
	pElement->FirstChild()->ToElement();

	if(pElement!=NULL)
	{
		char version[30];
		char newversion[30];
		strcpy(version,APP_VERSION);
		
		DWORD dwMajor,dwMinor,dwMaintain;
		DWORD dwNewMajor,dwNewMinor,dwNewMaintain;
		char *pMaj = version;
		char *pt = strchr(version,'.');
		pt[0] = 0;
		char *pMin = pt+1;

		pt = strchr(pMin,'.');
		pt[0] = 0;
		char *pMain = pt+1;
		dwMajor = atol(pMaj);
		dwMinor = atol(pMin);
		dwMaintain = atol(pMain);


		

		ReadCfgStr2(pElement , "Version",szVersion,sizeof(szVersion));
		strcpy(newversion,szVersion);
		pMaj = newversion;
		pt = strchr(newversion,'.');
		pt[0] = 0;
		pMin = pt+1;
		pt = strchr(pMin,'.');
		pt[0] = 0;
		pMain = pt+1;
		dwNewMajor = atol(pMaj);
		dwNewMinor = atol(pMin);
		dwNewMaintain = atol(pMain);

		if ((dwNewMajor>=dwMajor && dwNewMinor>=dwMinor && dwNewMaintain>dwMaintain) \
		|| (dwNewMajor>=dwMajor && dwNewMinor>dwMinor) \
		|| (dwNewMajor>dwMajor) )
			
		{
			Show_ToolbarButton(IDC_DOWNLOAD, true);
			//EnableDownloadLink(TRUE);
			SetStatusText(ICO_INFO,g_lang.GetString("StatusNewVersion"),szVersion);
			log.AddLogInfo(ICO_INFO,"New version %s detected!",szVersion);
			bAnyUpdates=TRUE;
			PostMessage(g_hWnd,WM_COMMAND,IDC_DOWNLOAD,0);
		} else
		{
//			log.AddLogInfo(ICO_INFO,"No new version detected!");
			if((int)lpParam!=1) //silent?
				MessageBox(g_hWnd,g_lang.GetString("MessageNoNewVersion"),"Info",MB_OK);
		}

	}
	
	g_bDoFirstTimeCheckForUpdate=false;
	return 0;
}

DWORD WINAPI AutomaticDownloadUpdateSetUp(LPVOID lpParam)
{
	SetCurrentDirectory(USER_SAVE_PATH);
 
	hashwrapper *myWrapper = NULL;
	TiXmlHandle hRoot(NULL);
	TiXmlHandle hDoc(NULL);//&doc);
	TiXmlDocument doc("update.xml");
	if (!doc.LoadFile()) 
		goto failed;

	HANDLE hThread;
	hThread = CreateThread( NULL, 0, &ProgressGUI_Thread, g_hWnd,0, NULL);                
	if (hThread == NULL) 
	{
		log.AddLogInfo(GS_LOG_WARNING, "CreateThread failed  <AutomaticDownloadUpdateSetUp> (%d) File:(%s) Line:(%d)\n", GetLastError(),__FILE__,__LINE__ ); 
	}
	else 
	{
		dbg_print("CreateThread  success AutomaticDownloadUpdateSetUp");
		CloseHandle( hThread );
	}
	Sleep(500);

	g_download.SetPath(USER_SAVE_PATH);

	hDoc = &doc;
	TiXmlElement* pElem;


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

				ReadCfgStr2(pChild ,"FileName",szFileName,sizeof(szFileName));
				ReadCfgStr2(pChild, "Dest",szDestPath,sizeof(szDestPath));
				ReadCfgStr2(pChild, "MD5",szMD5,sizeof(szMD5));
				ReadCfgInt2(pChild, "exec",(int&)iExec);					
				remove(szFileName);
				sprintf_s(szBuff,sizeof(szBuff),"http://www.bdamage.se/download/%s",szFileName);
				SetStatusText(ICO_INFO,"Downloading update!");
				if(strcmp(szDestPath,"ExePath")==0)
				{
					g_download.SetPath(EXE_PATH);
					SetCurrentDirectory(EXE_PATH);
				}else
				{
					g_download.SetPath(USER_SAVE_PATH);
					SetCurrentDirectory(USER_SAVE_PATH);
				}
				iRet = g_download.HttpFileDownload(szBuff,szFileName,g_DlgProgress,g_DlgProgressMsg);

				 try
				 {


					std::string hash2 = myWrapper->getHashFromFile(szFileName);
					log.AddLogInfo(0,"Comparing hashing %s = %s", hash2.c_str(),szMD5);	
					if(_stricmp(hash2.c_str(),szMD5)!=0)
					{
						log.AddLogInfo(0,"Mismatch hashing %s = %s", hash2.c_str(),szMD5);	
						goto failed;
					}
					else
					{
						if(iExec)
						{
							log.AddLogInfo(0,"Executing %s", szFileName);	
							if(strcmp(szDestPath,"ExePath")==0)
								ShellExecute(g_hWnd, "open",szFileName, NULL, EXE_PATH,SW_NORMAL);	
							else
								ShellExecute(g_hWnd, "open",szFileName, NULL, USER_SAVE_PATH,SW_NORMAL);	
							PostMessage(g_hWnd,WM_CLOSE,0,0);
						}

					}

				 }
				 catch(hlException &e)
				 {					 
					 log.AddLogInfo(0,"Error hashing file: %d %s", e.error_number(), e.erro_message().c_str());									   
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
	int ret = MessageBox(NULL,g_lang.GetString("ErrorDownloadingUpdate"),"Update error",MB_YESNO);				
	if(ret==IDYES)
	{
		ShellExecute(NULL,NULL,"http://www.bdamage.se/",NULL,NULL,SW_SHOWNORMAL);

	}		
	return 2;
}

void ErrorExit(LPTSTR lpszFunction) 
{ 
    char szBuf[80]; 
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

/*
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
				SetWindowText(hDlg,g_lang.GetString("TitleSetMinValue"));				
				bSettingMax=FALSE;
			}
			else
			{
				dwMaxMin = &AppCFG.filter.dwShowServerWithMaxPlayers;
				_itoa(AppCFG.filter.dwShowServerWithMaxPlayers,szTemp,10);	
				SetWindowText(hDlg,g_lang.GetString("TitleSetMaxValue"));
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
					TreeView_SetDWValueByItemType(FILTER_MAX_PLY,*dwMaxMin,TreeView_GetItemStateByType(-1,FILTER_MAX_PLY));
				else
					TreeView_SetDWValueByItemType(FILTER_MIN_PLY,*dwMaxMin,TreeView_GetItemStateByType(-1,FILTER_MIN_PLY));

			}

			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
*/
int nEditScriptIndex;
BOOL g_bAddNewFilter=FALSE;

/**********************************************************

hInsertAfter can be HTREEITEM or:
TVI_FIRST
Inserts the item at the beginning of the list.
TVI_LAST
Inserts the item at the end of the list.
TVI_ROOT
Add the item as a root item.
TVI_SORT
Inserts the item into the list in alphabetical order.

************************************************************/




HTREEITEM TreeView_InsertNewItem(HTREEITEM hParent, HTREEITEM hInsertAfter,_MYTREEITEM *ti)
{
	TVINSERTSTRUCT tvs;
	memset(&tvs,0,sizeof(TVINSERTSTRUCT));
	tvs.item.mask                   = TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_IMAGE  | TVIF_TEXT;
	
	tvs.item.iImage                 = ti->iIconIndex;
	tvs.item.iSelectedImage         = ti->iIconIndex;
	tvs.item.pszText            = (LPSTR)ti->sName.c_str();
	tvs.item.lParam = ti->dwIndex+1;

	tvs.hParent = hParent;
	tvs.hInsertAfter = hInsertAfter;

	HTREEITEM hNewItem = TreeView_InsertItem(g_hwndMainTreeCtrl, &tvs);

	if(hNewItem!=NULL)
	{
		ti->hTreeItem = hNewItem;		
		ti->dwIndex ++;
		
		for(int i=ti->dwIndex; i<tvmgr.vTI.size()-1;i++)
		{					
			tvmgr.vTI.at(i).dwIndex++;
			TVITEM  tvitem;
			ZeroMemory(&tvitem,sizeof(TVITEM));
			tvitem.hItem = tvmgr.vTI.at(i).hTreeItem;
			tvitem.mask = TVIF_PARAM ;
			tvitem.lParam = tvmgr.vTI.at(i).dwIndex;
			TreeView_SetItem(g_hwndMainTreeCtrl,&tvitem);
		
	/*		char text[256];
		 if(ti->dwType==DO_GLOBAL_EDIT_FILTER)
			 sprintf(text,"%s %d",tvmgr.vTI.at(i).sName.c_str(),ti->dwValue);
		 else
			 sprintf(text,"%s %d",tvmgr.vTI.at(i).sName.c_str(),tvmgr.vTI.at(i).dwIndex);

		 TreeView_SetItemText(tvmgr.vTI.at(i).hTreeItem,text);*/
		}
	}
	TreeView_Expand(g_hwndMainTreeCtrl, hParent, TVE_EXPAND);

	return hNewItem;
}



HTREEITEM TreeView_MoveItem(HTREEITEM hitemDrag,HTREEITEM hitemDrop)
{
	TVINSERTSTRUCT tvs;
	memset(&tvs,0,sizeof(TVINSERTSTRUCT));
	char szBuffer[100];

	tvs.item.mask   =  TVIF_SELECTEDIMAGE | TVIF_IMAGE  | TVIF_TEXT;
	tvs.item.pszText = szBuffer;
	tvs.item.cchTextMax = sizeof(szBuffer);
	tvs.item.hItem = hitemDrag;
	
	TreeView_GetItem(g_hwndMainTreeCtrl,&tvs.item);
	
	int dragidx = tvmgr.GetIndex(hitemDrag);
	int dropidx = tvmgr.GetIndex(hitemDrop);
	_MYTREEITEM drag = tvmgr.vTI.at(dragidx);
	_MYTREEITEM drop = tvmgr.vTI.at(dropidx);
	tvs.item.mask   =  TVIF_PARAM|TVIF_SELECTEDIMAGE | TVIF_IMAGE  | TVIF_TEXT;
	tvs.item.lParam = dragidx;

	if((drop.dwType==23) || (drop.dwType==0) )
	{
		drag.dwLevel = drop.dwLevel+1;
		tvs.hParent = hitemDrop;
		tvs.hInsertAfter = TVI_FIRST;
	}
	else
	{
		drag.dwLevel = drop.dwLevel;
		tvs.hParent = TreeView_GetParent(g_hwndMainTreeCtrl,hitemDrop);
		tvs.hInsertAfter = hitemDrop;
	}
	
	HTREEITEM hNewItem = TreeView_InsertItem(g_hwndMainTreeCtrl, &tvs);
	drag.hTreeItem = hNewItem;

	if(drag.dwLevel<3)
		drag.dwLevel = 3;
	drag.pGI = drop.pGI;
	drag.cGAMEINDEX = drop.cGAMEINDEX;

	tvmgr.vTI.insert(tvmgr.vTI.begin()+dropidx+1,drag);
	tvmgr.DeleteByHTI(hitemDrag);
	TreeView_DeleteItem(g_hwndMainTreeCtrl,m_hitemDrag);

	tvmgr.ReIndex();

	return hNewItem;

}






int countOfChar(const char*szText,char c)
{
	int i=0;
	int count=0;
	while(szText[i]!=NULL || (i>strlen(szText)))
		if(szText[i++]==c)
			count++;

	return count;
}
HWND g_hWndFilterEditorhDlg=NULL;
int g_startLine =0;
int g_startMaxLines =0;

void updateLineNo(HWND hWnd,int startLine)
{
	char sztemp[400];
	ZeroMemory(sztemp,sizeof(sztemp));

	int nTextLen = GetWindowTextLength(GetDlgItem(g_hWndFilterEditorhDlg,IDC_EDIT_FILTER));
	char *szText = (char*) calloc(nTextLen+1,sizeof(char));
	GetDlgItemText(g_hWndFilterEditorhDlg,IDC_EDIT_FILTER,szText,nTextLen);
	
	int i = countOfChar(szText,0xd);
	int c=startLine;
	while(c<=i)
	{
		char sztmp[10];
		sprintf(sztmp,"%2.2d\r\n",++c);
		strcat(sztemp,sztmp);
	}
	SetDlgItemText(hWnd,IDC_EDIT_LINE_NO,sztemp);
	if(szText!=NULL)
		free(szText);
}


LRESULT APIENTRY FilterEditor_EditSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
	 if(uMsg == WM_KEYDOWN)
	 {
		if(wParam==VK_RETURN)
		{
			
			int line = SendMessage(GetDlgItem(g_hWndFilterEditorhDlg,IDC_EDIT_FILTER),   (UINT) EM_LINEFROMCHAR,    -1, 0);
			if(line>0)
			{
				line = line +1;
				int nmaxlines = SendMessage(GetDlgItem(g_hWndFilterEditorhDlg,IDC_EDIT_FILTER),   (UINT) EM_GETLINECOUNT,    0, 0);

			}
		}
			
	 }
	 if(uMsg==WM_VSCROLL)
	 {
		 if(LOWORD(wParam)==SB_THUMBTRACK)
		 {
			 g_startLine = HIWORD(wParam);
		 }
		 if(LOWORD(wParam)==SB_LINEDOWN)
		 {
			 g_startLine++;
			 int nmaxlines = SendMessage(GetDlgItem(g_hWndFilterEditorhDlg,IDC_EDIT_FILTER),   (UINT) EM_GETLINECOUNT,    0, 0)-29;
			 if(g_startLine>nmaxlines)
				 g_startLine = nmaxlines;
				 
		 }
		 if(LOWORD(wParam)==SB_LINEUP)
		 {
			 g_startLine--;
			 if(g_startLine<0)
				 g_startLine = 0;
		 }
		updateLineNo(g_hWndFilterEditorhDlg,g_startLine);
	 }
	 if(uMsg == WM_PASTE)
	 {
		int nBar = GetScrollPos(GetDlgItem(g_hWndFilterEditorhDlg,IDC_EDIT_FILTER),SB_VERT);
		g_startLine = nBar;
		updateLineNo(g_hWndFilterEditorhDlg,g_startLine);
	 }
	if(uMsg == WM_KEYUP)
	{			
		if(wParam==VK_F1)
		{
			return TRUE;
		}
		else if(wParam==VK_RETURN)
		{
			int nlines = SendMessage(GetDlgItem(g_hWndFilterEditorhDlg,IDC_EDIT_FILTER),   (UINT) EM_GETLINECOUNT,    0, 0);
			if(nlines>=29)
				g_startLine++;

			updateLineNo(g_hWndFilterEditorhDlg,g_startLine);
			return TRUE;
		}
		else if(wParam==VK_BACK)
		{
			int nlines = SendMessage(GetDlgItem(g_hWndFilterEditorhDlg,IDC_EDIT_FILTER),   (UINT) EM_GETLINECOUNT,    0, 0);				
			if(g_startLine+29>29)
				g_startLine--;
			updateLineNo(g_hWndFilterEditorhDlg,g_startLine);
			return TRUE;
		}
	}				
    return CallWindowProc((WNDPROC)g_wpOrigFilterEditorProc, hWnd, uMsg,  wParam, lParam); 
}

BOOL isValidGroupString(const char *szGroup)
{
	int i=0;
	while(szGroup[i]!=0)
		if((szGroup[i]>='a' && szGroup[i]<='z') || (szGroup[i]>='A' && szGroup[i]<='Z'))
			i++;
		else
			return FALSE;


	return TRUE;
}

int GetDlgItemTextToString(HWND hWnd,int nID, string &pOutString)
{			
	int nTextLen = GetWindowTextLength(GetDlgItem(hWnd,nID))+1;
	char *szTemp = (char*) calloc(nTextLen+1,sizeof(char));
	GetDlgItemText(hWnd,nID,szTemp,nTextLen);
	pOutString = szTemp;
					
	if(szTemp!=NULL)
		free(szTemp);
	return 0;
}


BOOL g_bEditFilterLaunchScript=FALSE;
LRESULT CALLBACK FilterEditor_Dlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	switch (message)
	{
		case WM_INITDIALOG:
		{
			CenterWindow(hDlg);	
			SetFontToDlgItem(hDlg,g_hfScriptEditor,IDC_EDIT_FILTER);
			SetFontToDlgItem(hDlg,g_hfScriptEditor,IDC_EDIT_LINE_NO);
			SetWindowText(hDlg,g_lang.GetString("TitleEditFilter"));						
			nEditScriptIndex = lParam;
			g_hWndFilterEditorhDlg = hDlg;
			g_bEditFilterLaunchScript=FALSE;
			g_startLine =0;
		

			if(lParam==-1) //used for config exec launch condition
			{
				g_bEditFilterLaunchScript = TRUE;
				g_EditorTI.sName = "My condition";
				g_EditorTI.sScript = "";
				g_EditorTI.sElementName = "LaunchCondition";
				EnableWindow(GetDlgItem(hDlg,IDC_EDIT_GROUP),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_CHECK_GROUP),FALSE);
				SetDlgItemText(hDlg,IDC_EDIT_GROUP,g_EditorTI.sElementName.c_str());	
				SetDlgItemText(hDlg,IDC_EDIT_FILTERNAME,g_EditorTI.sName.c_str());	
				SetDlgItemText(hDlg,IDC_EDIT_FILTER,g_EditorTI.sScript.c_str());	
			}
			else if(lParam==-2) //used for config exec launch condition
			{
				g_bEditFilterLaunchScript = TRUE;
				g_EditorTI.sElementName = "LaunchCondition";

				EnableWindow(GetDlgItem(hDlg,IDC_CHECK_GROUP),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_EDIT_GROUP),FALSE);
				SetDlgItemText(hDlg,IDC_EDIT_GROUP,g_EditorTI.sElementName.c_str());	
				SetDlgItemText(hDlg,IDC_EDIT_FILTERNAME,g_EditorTI.sName.c_str());	
				SetDlgItemText(hDlg,IDC_EDIT_FILTER,g_EditorTI.sScript.c_str());	

			}
			else if(g_bAddNewFilter)
			{
				g_EditorTI = tvmgr.vTI.at(lParam);
				g_EditorTI.sName = "My filter";
				g_EditorTI.sScript = "";
				//g_EditorTI.sElementName = "CustomFilter";
				SetDlgItemText(hDlg,IDC_EDIT_GROUP,g_EditorTI.sElementName.c_str());	
				SetDlgItemText(hDlg,IDC_EDIT_FILTERNAME,g_EditorTI.sName.c_str());	
				SetDlgItemText(hDlg,IDC_EDIT_FILTER,g_EditorTI.sScript.c_str());	
				if((g_EditorTI.dwType==DO_CUSTOM_MODIFIER) || (g_EditorTI.dwType==0) || (g_EditorTI.dwType==DO_GLOBAL_FILTER_PARENT))
					CheckDlgButton(hDlg,IDC_CHECK_GROUP,FALSE);	
				else
					CheckDlgButton(hDlg,IDC_CHECK_GROUP,TRUE);	

							
			} else
			{		
				SetDlgItemText(hDlg,IDC_EDIT_GROUP,tvmgr.vTI.at(lParam).sElementName.c_str());	
				SetDlgItemText(hDlg,IDC_EDIT_FILTERNAME,tvmgr.vTI.at(lParam).sName.c_str());
				SetDlgItemText(hDlg,IDC_EDIT_FILTER,tvmgr.vTI.at(lParam).sScript.c_str());
				if(tvmgr.vTI.at(lParam).dwType==DO_CUSTOM_MODIFIER)
					CheckDlgButton(hDlg,IDC_CHECK_GROUP,FALSE);	
				else
					CheckDlgButton(hDlg,IDC_CHECK_GROUP,TRUE);	

				if(tvmgr.vTI.at(lParam).dwType==DO_CUSTOM_FAVORITE_FILTER)
					EnableWindow(GetDlgItem(hDlg,IDC_CHECK_GROUP),FALSE);
			}
			 updateLineNo(hDlg,g_startLine);
			


			SetFocus(GetDlgItem(hDlg,IDC_EDIT_FILTER));
			g_wpOrigFilterEditorProc =  (LONG_PTR) SetWindowLongPtr(GetDlgItem(hDlg,IDC_EDIT_FILTER), GWLP_WNDPROC, (LONG_PTR) FilterEditor_EditSubclassProc); 

		}
		break;
	case WM_COMMAND:
		if(HIWORD(wParam)==EN_VSCROLL)
		{
		
			int nBar = GetScrollPos(GetDlgItem(hDlg,IDC_EDIT_FILTER),SB_VERT);

			g_startLine = nBar;
			updateLineNo(g_hWndFilterEditorhDlg,g_startLine);
			dbg_print("EN_VSCROLL %d = %d",g_startLine,nBar);
			break;
		}
		if(LOWORD(wParam) == IDC_BUTTON_TEST_FILTER)
		{						
			string sTempScript;
			string sTempScriptName;
			vFILTER_SETS vFilterSetsTemp;

			GetDlgItemTextToString(hDlg,IDC_EDIT_FILTERNAME,sTempScriptName);
			GetDlgItemTextToString(hDlg,IDC_EDIT_FILTER,sTempScript);
			int iResult = se.CompileFilter(vFilterSetsTemp,sTempScript.c_str(),sTempScriptName.c_str(),"test");
			se.Display_Result(iResult,TRUE);
			break;
		}
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			if(LOWORD(wParam) == IDOK)
			{
				string sTempScript;
				string sTempGroupName;
				string sTempScriptName;
				vFILTER_SETS vFilterSetsTemp;
				
				GetDlgItemTextToString(hDlg,IDC_EDIT_FILTERNAME,sTempScriptName);
				GetDlgItemTextToString(hDlg,IDC_EDIT_FILTER,sTempScript);					
				GetDlgItemTextToString(hDlg,IDC_EDIT_GROUP,sTempGroupName);				
				if(isValidGroupString(sTempGroupName.c_str())==FALSE)
				{
					MessageBox(hDlg,"Group name is not valid.\nIt can only contain alpha chars:\na-z or A-Z only.","Error",MB_OK);					
					return TRUE;
				}

				int iResult = se.CompileFilter(vFilterSetsTemp,sTempScript.c_str(),sTempScriptName.c_str(),sTempGroupName.c_str());
				if(se.Display_Result(iResult,FALSE)==FALSE)
					return TRUE;

				if(g_bEditFilterLaunchScript)
				{
					g_EditorTI.sName = sTempScriptName;
					g_EditorTI.sScript = sTempScript;

				}
				else if(g_bAddNewFilter)
				{
					g_EditorTI.sName = sTempScriptName;
					g_EditorTI.sElementName = sTempGroupName;
					g_EditorTI.sScript = sTempScript;

					if ((g_EditorTI.dwType==0) || (g_EditorTI.dwType==23))  //Game specific filter parent
					{	
						g_EditorTI.dwLevel++;
						g_EditorTI.iIconIndex = 9;		
						TreeView_InsertNewItem(g_EditorTI.hTreeItem,TVI_FIRST,&g_EditorTI);
					}
					else
						TreeView_InsertNewItem(TreeView_GetParent(g_hwndMainTreeCtrl,g_EditorTI.hTreeItem),g_EditorTI.hTreeItem,&g_EditorTI);


					if(IsDlgButtonChecked(hDlg,IDC_CHECK_GROUP)==BST_CHECKED)
						g_EditorTI.dwType = DO_CUSTOM_GROUP_FILTER;
					else
						g_EditorTI.dwType = DO_CUSTOM_MODIFIER;


				//	TreeView_InsertNewItem(&g_EditorTI);
					if(nEditScriptIndex>=0)
						tvmgr.vTI.insert(tvmgr.vTI.begin()+nEditScriptIndex+1,g_EditorTI);
					else
						MessageBox(hDlg,"Error editing script.\n Please report this bug.\ncode 1","Script editor",MB_OK);
				}else
				{
					string sOldFiltername;
					if(nEditScriptIndex>=0)
					{	
						sOldFiltername = tvmgr.vTI.at(nEditScriptIndex).sName;
						tvmgr.vTI.at(nEditScriptIndex).sScript = sTempScript;
						tvmgr.vTI.at(nEditScriptIndex).sName = sTempScriptName;
						tvmgr.vTI.at(nEditScriptIndex).sElementName = sTempGroupName;
						tvmgr.SetItemText(tvmgr.vTI.at(nEditScriptIndex).hTreeItem,tvmgr.vTI.at(nEditScriptIndex).sName.c_str());
						
						if(tvmgr.vTI.at(nEditScriptIndex).dwType!=DO_CUSTOM_FAVORITE_FILTER)
						{
							if(IsDlgButtonChecked(hDlg,IDC_CHECK_GROUP)==BST_CHECKED)
								tvmgr.vTI.at(nEditScriptIndex).dwType = DO_CUSTOM_GROUP_FILTER;
							else
								tvmgr.vTI.at(nEditScriptIndex).dwType = DO_CUSTOM_MODIFIER;
						}
						
						if(tvmgr.vTI.at(nEditScriptIndex).dwType!=GLOBAL_FILTER && tvmgr.vTI.at(nEditScriptIndex).cGAMEINDEX!=-1)
						{
							int idx = tvmgr.vTI.at(nEditScriptIndex).cGAMEINDEX;
							try
							{
								gm.ValidateGameIndex(idx);
							}catch(int a)
							{
								log.AddLogInfo(GS_LOG_WARNING, "Error GameIdx <FilterEditor_Dlg> (%d) File:(%s) Line:(%d)\n", GetLastError(),__FILE__,__LINE__ ); 													
								MessageBox(g_hWnd,"Game idx out of bounds!","Error",MB_OK);								
							}
							tvmgr.Filter_update(&gm.GamesInfo[idx],tvmgr.vTI.at(nEditScriptIndex),sOldFiltername.c_str());
						}
						else
							tvmgr.Filter_update(NULL,tvmgr.vTI.at(nEditScriptIndex),sOldFiltername.c_str());
					}else
					{
						MessageBox(hDlg,"Error editing script.\n Please report this bug.\ncode 2","Script editor",MB_OK);
					}

				}

			}
			
			SetWindowLong(GetDlgItem(hDlg,IDC_EDIT_FILTER), GWLP_WNDPROC, (LONG) g_wpOrigFilterEditorProc); 
			g_wpOrigFilterEditorProc = NULL;

			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void OnRenameFilter(HWND hWndParent)
{
	HTREEITEM hSelectedTreeItem = TreeView_GetSelection(g_hwndMainTreeCtrl);

	if(hSelectedTreeItem!=NULL)
	{
		SetFocus(g_hwndMainTreeCtrl);
		g_bBeginEdit = TRUE;
		HWND hWnd = TreeView_EditLabel(g_hwndMainTreeCtrl,hSelectedTreeItem);
		
	}
}
void OnDelFilter(HWND hWndParent)
{
	HTREEITEM hTreeItem = TreeView_GetSelection(g_hwndMainTreeCtrl);

	if(hTreeItem!=NULL)
	{
		int ret = MessageBox(g_hWnd,"Do you want to delete selected filter?","Game Scanner",MB_YESNO | MB_ICONWARNING);
		if(ret == IDYES)
		{
			TVITEM  tvitem;
			ZeroMemory(&tvitem,sizeof(TVITEM));
			char szBuffer[100];
			tvitem.hItem = hTreeItem;
			tvitem.cchTextMax = sizeof(szBuffer);
			tvitem.pszText = szBuffer;
			tvitem.mask = TVIF_PARAM | TVIF_TEXT;
			TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );
			
			g_EditorTI = tvmgr.vTI.at((int)tvitem.lParam);

			TreeView_DeleteItem(g_hwndMainTreeCtrl,hTreeItem);
		
		//	if((tvmgr.vTI.at(tvitem.lParam).dwType>12) && (tvmgr.vTI.at(tvitem.lParam).dwType<20) && (tvmgr.vTI.at(tvitem.lParam).cGAMEINDEX!=GLOBAL_FILTER))
	//			Filter_Remove(&gm.GamesInfo[tvmgr.vTI.at(tvitem.lParam).cGAMEINDEX].vFilterSets,tvmgr.vTI.at(tvitem.lParam).sName.c_str());
	//		else
			int GameIdx = tvmgr.vTI.at(tvitem.lParam).cGAMEINDEX;

			tvmgr.Filter_Remove(gm.GetFilterSet(GameIdx),tvmgr.vTI.at(tvitem.lParam).sName.c_str());

			tvmgr.DeleteByHTI(hTreeItem);	
			tvmgr.ReIndex();
			Initialize_RedrawServerListThread();
		}

	}
	
}

void DeleteAllChilds(HTREEITEM hSibling)
{
	while(hSibling!=NULL)
	{	
		HTREEITEM hNextSibling = TreeView_GetNextSibling( g_hwndMainTreeCtrl, hSibling );
		tvmgr.DeleteByHTI(hSibling);
		TreeView_DeleteItem(g_hwndMainTreeCtrl, hSibling );		
		hSibling = hNextSibling;
	}
}

void OnDeleteFolder(HWND hWndParent)
{
	

	HTREEITEM hTreeItem = TreeView_GetSelection(g_hwndMainTreeCtrl);
 
	if(hTreeItem!=NULL)
	{
		int ret = MessageBox(g_hWnd,"Do you want to delete selected folder?","Game Scanner",MB_YESNO | MB_ICONWARNING);
		if(ret == IDYES)
		{
			HTREEITEM hChild = TreeView_GetChild( g_hwndMainTreeCtrl, hTreeItem );
			DeleteAllChilds(hChild);
			tvmgr.DeleteByHTI(hTreeItem);
			TreeView_DeleteItem(g_hwndMainTreeCtrl, hTreeItem );		
			tvmgr.ReIndex();
		}
	}

}

void OnAddNewFolder(HWND hWndParent)
{
	
	g_EditorTI.sName = "New Folder";
	g_EditorTI.sElementName = "MyFilter";
	g_EditorTI.sScript = "";
	g_EditorTI.iIconIndex = 18;
	g_EditorTI.dwType = 0;
	g_EditorTI.dwState = 0;

	HTREEITEM hTreeItem = TreeView_GetSelection(g_hwndMainTreeCtrl);
 
	if(hTreeItem!=NULL)
	{
		TVITEM  tvitem;
		ZeroMemory(&tvitem,sizeof(TVITEM));
		char szBuffer[100];
		tvitem.hItem = hTreeItem;
		tvitem.cchTextMax = sizeof(szBuffer);
		tvitem.pszText = szBuffer;
		tvitem.mask = TVIF_HANDLE|TVIF_PARAM | TVIF_TEXT | TVIF_SELECTEDIMAGE |  TVIF_IMAGE;
		TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );
		int iSel = (int)tvitem.lParam;
		
		tvitem.mask = TVIF_HANDLE;
		
		HTREEITEM hParent = TreeView_GetParent(g_hwndMainTreeCtrl, tvitem.hItem );
		
		g_EditorTI.cGAMEINDEX = tvmgr.vTI.at(iSel).cGAMEINDEX;
		g_EditorTI.dwLevel = tvmgr.vTI.at(iSel).dwLevel;
		g_EditorTI.dwIndex = iSel;
		g_EditorTI.hTreeItem = TreeView_InsertNewItem(hParent,TVI_FIRST,&g_EditorTI);
		tvmgr.vTI.insert(tvmgr.vTI.begin()+iSel+1,g_EditorTI);
		tvmgr.ReIndex();
	}

}

void OnEditFilter(HWND hWndParent)
{
	HTREEITEM hTreeItem = TreeView_GetSelection(g_hwndMainTreeCtrl);
 
	if(hTreeItem!=NULL)
	{
		TVITEM  tvitem;
		ZeroMemory(&tvitem,sizeof(TVITEM));
		char szBuffer[100];
		tvitem.hItem = hTreeItem;
		tvitem.cchTextMax = sizeof(szBuffer);
		tvitem.pszText = szBuffer;
		tvitem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_SELECTEDIMAGE |  TVIF_IMAGE;
		TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );

		int iSel = (int)tvitem.lParam;

		int ret = DialogBoxParam(g_hInst, (LPCTSTR)IDD_DLG_EDIT_FILTER, hWndParent, (DLGPROC)FilterEditor_Dlg,iSel);
		if(ret==IDOK)
			Initialize_RedrawServerListThread();
	}
}

void OnAddFilter(HWND hWndParent)
{
	g_bAddNewFilter = TRUE;
	OnEditFilter(hWndParent);
	g_bAddNewFilter = FALSE;
}

void OnPasteFilter()
{
	HTREEITEM hTreeItem = TreeView_GetSelection(g_hwndMainTreeCtrl);
 
	if((hTreeItem!=NULL) && (g_CopyTI.dwType>10))
	{
		TVITEM  tvitem;
		ZeroMemory(&tvitem,sizeof(TVITEM));
	
		tvitem.hItem = hTreeItem;
		tvitem.mask = TVIF_PARAM ;
		TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );

		g_PasteAtTI = tvmgr.vTI.at(tvitem.lParam);
	
		g_CopyTI.cGAMEINDEX = g_PasteAtTI.cGAMEINDEX;

		if (g_PasteAtTI.dwType==0)  //Game specific filter parent
		{	
			g_CopyTI.dwLevel++;
			TreeView_InsertNewItem(g_CopyTI.hTreeItem,TVI_FIRST,&g_CopyTI);

		}
		else if (g_PasteAtTI.dwType==23) //Global filter parent
		{		
			g_CopyTI.dwLevel++;			
			TreeView_InsertNewItem(g_CopyTI.hTreeItem,TVI_FIRST,&g_CopyTI);
		}else

			TreeView_InsertNewItem(TreeView_GetParent(g_hwndMainTreeCtrl,g_PasteAtTI.hTreeItem),g_PasteAtTI.hTreeItem,&g_CopyTI);

		tvmgr.vTI.insert(tvmgr.vTI.begin()+tvitem.lParam+1,g_CopyTI);
		tvmgr.ReIndex();
	}
}

void OnCopyFilter()
{
	HTREEITEM hTreeItem = TreeView_GetSelection(g_hwndMainTreeCtrl);
 
	if(hTreeItem!=NULL)
	{
		
		TVITEM  tvitem;
		ZeroMemory(&tvitem,sizeof(TVITEM));
	
		tvitem.hItem = hTreeItem;
		tvitem.mask = TVIF_PARAM ;
		TreeView_GetItem(g_hwndMainTreeCtrl, &tvitem );

		g_CopyTI = tvmgr.vTI.at(tvitem.lParam);
	}
}

#define TOOLBAR_SIZE_X 16
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
                            0,  0, (TOOLBAR_SIZE_X+2)*4, 0,
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
		::SendMessage(hwndTB, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);
	
		::SendMessage(hwndTB, TB_SETBITMAPSIZE, 0, MAKELONG(TOOLBAR_SIZE_X, TOOLBAR_SIZE_X));
		//::SendMessage(hwndTB, TB_SETBITMAPSIZE, 0, MAKELONG(24, 24));

		int iNumButtons = 10;
		COLORREF crMask = RGB(255,0,255);

		HBITMAP hbm = NULL;
		if(TOOLBAR_SIZE_X>16)
			hbm = ::LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_TOOLBAR_N));
		else
			hbm = ::LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_TOOLBAR_N_16));

		BITMAP bm = {0};

		::GetObject(hbm, sizeof(BITMAP), &bm);
		int iImageWidth  = TOOLBAR_SIZE_X; //bm.bmWidth / iNumButtons;
		int iImageHeight = bm.bmHeight;

		m_hImageList = ImageList_Create(iImageWidth, iImageHeight, ILC_COLOR32 , iNumButtons, 0);
		ImageList_AddMasked(m_hImageList, hbm, crMask);
		::DeleteObject(hbm);

		SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)m_hImageList);
		
		if(TOOLBAR_SIZE_X>16)
			hbm = ::LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_TOOLBAR_H));		
		else
			hbm = ::LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_TOOLBAR_H_16));		

		m_hImageListHot = ImageList_Create(iImageWidth, iImageHeight, ILC_COLOR32 , iNumButtons, 0);		
		ImageList_AddMasked(m_hImageListHot, hbm, crMask);
		::DeleteObject(hbm);
		SendMessage(hwndTB, TB_SETHOTIMAGELIST, 0, (LPARAM)m_hImageListHot);
						

		m_hImageListDis = ImageList_Create(iImageWidth, iImageHeight, ILC_COLOR32 | ILC_MASK, iNumButtons, 0);
		if(TOOLBAR_SIZE_X>16)
			hbm = ::LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_TOOLBAR_D));
		else
			hbm = ::LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_TOOLBAR_D_16));
		ImageList_AddMasked(m_hImageListDis, hbm, crMask);
		::DeleteObject(hbm);
		SendMessage(hwndTB, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)m_hImageListDis);
	
		int iImages = 0;
		TBBUTTON tbb;

	   	ZeroMemory(&tbb, sizeof(TBBUTTON));

		tbb.iBitmap = 0;
		tbb.idCommand = IDM_REFRESH;
		tbb.fsState = TBSTATE_ENABLED;
		tbb.fsStyle = TBSTYLE_BUTTON ; //;
		//		tbb.fsStyle = TBSTYLE_SEP;
		::SendMessage(hwndTB, TB_ADDBUTTONS, 1, (LPARAM)&tbb);

		tbb.iBitmap = 7;
		tbb.idCommand = IDM_SCAN_FILTERED;
		tbb.fsState = TBSTATE_ENABLED;
		tbb.fsStyle = TBSTYLE_BUTTON ;
		//		tbb.fsStyle = TBSTYLE_SEP;
		::SendMessage(hwndTB, TB_ADDBUTTONS, 1, (LPARAM)&tbb);


		tbb.iBitmap = 1;
		tbb.idCommand = IDM_SETTINGS;
		tbb.fsState = TBSTATE_ENABLED;
		tbb.fsStyle = BTNS_BUTTON ;//| BTNS_DROPDOWN;
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
            (1 * HIWORD(dwBaseUnits)) / 8, 
            (160 * LOWORD(dwBaseUnits)) / 4, 
            (4 * HIWORD(dwBaseUnits)) / 8, 
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
                            0, 0, 28, TOOLBAR_SIZE_X+4,
                            hWndParent, 
                            NULL, 
                            g_hInst, 
                            NULL); 


	if(hwndTB==NULL)
		log.AddLogInfo(0,"TOOLBAR_CreateSearchToolBar create failed!!");

   

		::SendMessage(hwndTB, TB_SETPARENT, (WPARAM)(HWND) hWndParent, 0);

		// We must send this message before sending the TB_ADDBITMAP or TB_ADDBUTTONS message
		::SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

		// allows buttons to have a separate dropdown arrow
		// Note: TBN_DROPDOWN notification is sent by a toolbar control when the user clicks a dropdown button
	//	::SendMessage(hwndTB, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);
	
		::SendMessage(hwndTB, TB_SETBITMAPSIZE, 0, MAKELONG(TOOLBAR_SIZE_X, TOOLBAR_SIZE_X));


		SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)m_hImageList);
		SendMessage(hwndTB, TB_SETHOTIMAGELIST, 0, (LPARAM)m_hImageListHot);
		SendMessage(hwndTB, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)m_hImageListDis);

	
		TBBUTTON tbb;

		ZeroMemory(&tbb, sizeof(TBBUTTON));

		tbb.iBitmap = 330;
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
                           0,0,280,24,
                           hwndOwner,
                           NULL,
                           g_hInst,
                           NULL);
   
	if(!hwndRB)
	{		
		log.AddLogInfo(GS_LOG_WARNING, "Create Rebar failed  <TOOLBAR_CreateRebar>"); 
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

	rbBand.lpText     = (LPSTR)g_lang.GetString("Search");
	rbBand.hwndChild  = hwndCSTB;
	rbBand.cxMinChild = 315;   
	rbBand.cxIdeal    = 445;//415;
	rbBand.cx         = 460;//435;
   
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
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, g_hInst, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   g_hWnd = hWnd;
 
   if(g_bNormalWindowed)
   {
	   	ShowWindow(hWnd,AppCFG.nWindowState);

//		ShowWindow(hWnd, nCmdShow);
   }
   UpdateWindow(hWnd);

   return TRUE;
}


void CleanUpFilesRegistry()
{
	SetCurrentDirectory(USER_SAVE_PATH);
	remove("ETSVsetup.msi");
	remove("GameScanner.msi");
}

int CFG_Load()
{

	Default_Appsettings();
	gm.Default_GameSettings();

	//log.AddLogInfo(GS_LOG_INFO,"CFG_Load");

	g_lang.loadFile("lang_en.xml");

	/*
//New xml wrapper code test
	TiXmlElement* pXmlElem;
	g_xmlcfg.SetPath(USER_SAVE_PATH);
	g_xmlcfg.load("config.xml");
	pXmlElem = g_xmlcfg.GetElementSafe(g_xmlcfg.m_pRootElement,"Versions");
	char szCfgVersion[20];
	g_xmlcfg.GetText(pXmlElem,"MainVersion",szCfgVersion,sizeof(szCfgVersion));

	TiXmlElement* pGamesElem;
	TiXmlElement* pGameElem;
	TiXmlElement* pGameData;
	TiXmlElement* pInstalls;
	TiXmlElement* pInstall;
	pGamesElem = g_xmlcfg.GetElementSafe(g_xmlcfg.m_pRootElement,"Games");
	pGameElem = g_xmlcfg.GetElementSafe(pGamesElem,"Game");
	pInstalls = g_xmlcfg.GetElementSafe(pGameElem,"Installs");
	pInstall = g_xmlcfg.GetElementSafe(pInstalls,"Install");
	char szBuffTemp[200];
	g_xmlcfg.GetAttribute(pInstall,"Name",szBuffTemp,sizeof(szBuffTemp));
*/

	SetCurrentDirectory(USER_SAVE_PATH);
	TiXmlDocument doc("config.xml");
	if (!doc.LoadFile()) 
	{
	//	ListView_SetDefaultColumns();
		return 1;
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
	pElem=hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem) 
		return 1;
	const char *szP;
	szP = pElem->Value(); //= GScfg

	// save this for later
	hRoot=TiXmlHandle(pElem);
	int intVal=0;

	pElem=hRoot.FirstChild("CurrentLanguage").Element();
	if (pElem)
		strcpy(AppCFG.szLanguageFilename,pElem->Attribute("filename"));		

	g_lang.loadFile(AppCFG.szLanguageFilename);
	//ListView_SetDefaultColumns();

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
	
	pElem=hRoot.FirstChild("BuddySoundNotify").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("enable",&intVal);
		AppCFG.bUseBuddySndNotify = intVal;
		if(pElem->Attribute("path")!=NULL)
			strcpy_s(AppCFG.szNotifySoundWAVfile,sizeof(AppCFG.szNotifySoundWAVfile), pElem->Attribute("path"));
	} 
	pElem=hRoot.FirstChild("BuddyNotify").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("enable",&intVal);
		AppCFG.bBuddyNotify  = intVal;
		
	} 
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


	pElem=hRoot.FirstChild("OnReturnEXEsettings").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("enable",&intVal);
		AppCFG.bUse_EXT_APP2 = intVal;
		if(pElem->Attribute("path")!=NULL)
			strcpy(AppCFG.szOnReturn_EXE_PATH,pElem->Attribute("path"));

		if(pElem->Attribute("cmd")!=NULL)
			strcpy(AppCFG.szOnReturn_EXE_CMD,pElem->Attribute("cmd"));

	} else //set defualt value
		AppCFG.bUse_EXT_APP2 = FALSE;



	pElem=hRoot.FirstChild("ShowMinimizePopUp").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("disable",&intVal);
		AppCFG.bShowMinimizePopUp = (BOOL) intVal;
	} else //set defualt value
		AppCFG.bShowMinimizePopUp = TRUE;


	pElem=hRoot.FirstChild("CloseOnConnect").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("enable",&intVal);
		AppCFG.bCloseOnConnect = (BOOL) intVal;
	} else //set defualt value
		AppCFG.bCloseOnConnect = FALSE;

	pElem=hRoot.FirstChild("RegisterWebProtocols").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("enable",&intVal);
		AppCFG.bRegisterWebProtocols = (BOOL) intVal;
	} else //set defualt value
		AppCFG.bRegisterWebProtocols = FALSE;


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


	pElem=hRoot.FirstChild("LastGameView").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("index",&intVal);
		g_currentGameIdx   = intVal;		
	} else //set default value
		g_currentGameIdx  = 0;
	

	
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"HideOfflineServers",(int&)AppCFG.filter.bHideOfflineServers);
	AppCFG.bNoMapResize = XML_GetTreeItemInt(hRoot.FirstChild("MapPreviewResize").ToElement(),"disable");

/*	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"NumMaxPlayers",(int&)AppCFG.filter.dwShowServerWithMaxPlayers);
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"NumMinPlayers",(int&)AppCFG.filter.dwShowServerWithMinPlayers);
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"NumMaxPlayersActive",(int&)AppCFG.filter.cActiveMaxPlayer);
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"NumMinPlayersActive",(int&)AppCFG.filter.cActiveMinPlayer);
	ReadCfgInt(hRoot.FirstChild("Filters").FirstChild().ToElement(),"Ping",(int&)AppCFG.filter.dwPing);
	*/
	ReadCfgInt(hRoot.FirstChild("Options").FirstChild().ToElement(),"Transparancy",(int&)AppCFG.g_cTransparancy);
	ReadCfgInt(hRoot.FirstChild("Options").FirstChild().ToElement(),"MaxScanThreads",(int&)AppCFG.dwThreads);
	ReadCfgInt(hRoot.FirstChild("Options").FirstChild().ToElement(),"NetworkRetries",(int&)AppCFG.dwRetries);
	ReadCfgInt(hRoot.FirstChild("Options").FirstChild().ToElement(),"SleepPerScan",(int&)AppCFG.dwSleep);

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


	ReadCfgInt(hRoot.FirstChild("Sort").FirstChild().ToElement(),"LastSortColumn",iLastColumnSortIndex);


	pElem=hRoot.FirstChild("WindowState").Element();
	if (pElem)
	{
		pElem->QueryIntAttribute("value",&AppCFG.nWindowState);

	} else //set defualt value
		AppCFG.nWindowState = SW_SHOWNORMAL;


	TiXmlElement * pElemWindows;
	pElemWindows=hRoot.FirstChild("Windows").ToElement();
	if(pElemWindows!=NULL)
	{
		TiXmlNode*  pNode = pElemWindows->FirstChild("Window");

		int idx=0;
		for( pNode; pNode; pNode=pNode->NextSiblingElement())
		{
			if(pNode!=NULL)
			{
				XML_ReadWindow(pNode->ToElement(), &WNDCONT[idx]);
				g_bWinSizesLoaded = TRUE;
				idx++;
			}
		}
	
	}

	char szTemp[MAX_PATH*4];
	char temp[MAX_PATH*4];

	TiXmlElement* pElement;	
	pElement=hRoot.FirstChild("Games").ToElement();
	if(pElement!=NULL)
	{
		if(pElement->FirstChild()!=NULL)  //No games found in config!
		{
			pElement = pElement->FirstChild()->ToElement();
			if(pElement!=NULL)
			{
				for(int i=0;i<gm.GamesInfo.size();i++)
				{
					TiXmlElement* pNode = pElement->FirstChild()->ToElement();
					if(pNode!=NULL)
					{

						ReadCfgStr(pNode, "GameName",temp,MAX_PATH);
						if(strlen(temp)>0)
							strcpy(gm.GamesInfo[i].szGAME_NAME,temp);
						
	
							TiXmlNode* pInstallTags = pElement->FirstChild("Installs");
							
							if( pInstallTags!=NULL)
							{
								TiXmlElement* pInstalls = pInstallTags->ToElement();
								if(pInstalls!=NULL)
									gm.GamesInfo[i].vGAME_INST.clear();

								while(pInstalls!=NULL)
								{
									TiXmlElement* pInstall = pInstalls->FirstChild("Install")->ToElement();
						
									ReadCfgStr(pInstall, "Name",szTemp,MAX_PATH); 
									GAME_INSTALLATIONS gi;							

									gi.sName = szTemp;

									ReadCfgStr(pInstall, "Path",temp,MAX_PATH);
									gi.szGAME_PATH = temp;

									ReadCfgStr(pInstall, "Cmd",temp,MAX_PATH);
									gi.szGAME_CMD = temp;

									ReadCfgStr(pInstall, "ScriptName",temp,MAX_PATH);
									gi.sFilterName = temp;		
									ReadCfgStr(pInstall, "Script",temp,MAX_PATH*4);
									gi.sScript = temp;		
									int itmp=0;
									ReadCfgInt(pInstall,"ScriptActive",itmp);
									gi.bActiveScript = (BOOL)itmp;

									gm.GamesInfo[i].vGAME_INST.push_back(gi);
									pInstalls = pInstalls->NextSiblingElement();
									if(pInstalls==NULL)
										break;
									
								}
							}	
						ReadCfgInt(pNode, "Active",(int&)gm.GamesInfo[i].bActive);

						if(pElement!=NULL)
							pElement = pElement->NextSiblingElement();
						if(pElement==NULL)
							break;
					} else
						break;
				}
			}
		}
	}
	if(AppCFG.bRegisterWebProtocols)
		RegisterProtocol(EXE_PATH);

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
	InvalidateRect(hWnd,NULL,TRUE);
	//InvalidateRect(WNDCONT[WIN_PING].hWnd,&WNDCONT[WIN_PING].rSize,TRUE);
	OnSize(hWnd,0,TRUE);
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



void OnExportServers(HWND hWnd)
{
	TCHAR szFile[260];
	OPENFILENAME ofn;
	INT_PTR result = DialogBoxParam(g_hInst, (LPCTSTR)ID_DLG_ENUMERATE_GAMES, hWnd, (DLGPROC)EnumerateGames_Dlg,1);					

	if(result==IDOK)
	{


		memset(&ofn,0,sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof (OPENFILENAME);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFilter = NULL;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "All\0*.*\0Text file\0*.txt\0Game Scanner file\0*.gs\0";
		ofn.nFilterIndex = 2;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST ;

		if(GetSaveFileName(&ofn))
		{
			char szPath[256];
			strcpy(szPath,ofn.lpstrFile);
			char *pFilename = strrchr(szPath,'\\');
			pFilename[0]=0;
			pFilename++;
			if(ofn.nFilterIndex==2)
				g_dwExportDetails = EXPORT_PLAIN_TEXT;
			else 
				g_dwExportDetails = EXPORT_GAMESCANNER_SL_FILE;

			ExportServerList(pFilename,szPath,&gm.GamesInfo[g_dwExportGameIdx],g_dwExportDetails);
			MessageBox(hWnd,g_lang.GetString("ExportDone"),"Game Scanner",MB_OK);
		}
	}

}

void OnImportServers(HWND hWnd)
{
	TCHAR szFile[260];
	OPENFILENAME ofn;
		memset(&ofn,0,sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof (OPENFILENAME);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFilter = NULL;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "All\0*.*\0Text file\0*.txt\0Game Scanner file\0*.gs\0";
		ofn.nFilterIndex = 2;
		ofn.lpstrFileTitle = "Import serverlist";
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST ;

		if(GetOpenFileName(&ofn))
		{
			char szPath[256];
			strcpy(szPath,ofn.lpstrFile);
			char *pFilename = strrchr(szPath,'\\');
			pFilename[0]=0;
			pFilename++;
			DWORD dwImportFlag=0;
			if(strstr(pFilename,".gs")!=0)
			{
				ImportServerList(pFilename,szPath,&gm.GamesInfo[0],EXPORT_GAMESCANNER_SL_FILE);
				MessageBox(hWnd,g_lang.GetString("ImportDone"),"Game Scanner",MB_OK);
			} else
			{
				//Manually import server files
				INT_PTR result = DialogBoxParam(g_hInst, (LPCTSTR)ID_DLG_ENUMERATE_GAMES, hWnd, (DLGPROC)EnumerateGames_Dlg,0);					
				
				if(result==IDOK)
				{
					ImportServerList(pFilename,szPath,&gm.GamesInfo[g_dwExportGameIdx],EXPORT_PLAIN_TEXT);
					MessageBox(hWnd,g_lang.GetString("ImportDone"),"Game Scanner",MB_OK);
				}
			}
				
		}



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
		case WM_ACTIVATE:
			switch(wParam)
			{
				//case WA_INACTIVE: dbg_print("WA_INACTIVE");	break;
				case WA_ACTIVE: /*dbg_print("WA_ACTIVATE");*/ OnActivate();	break;
				case WA_CLICKACTIVE: /*dbg_print("WA_CLICKACTIVE");*/	 OnActivate();	 break;
			} 
			break;
		case WM_REINIT_COUNTRYFILTER:
			tvmgr.Initialize_CountryFilter();
			break;
		case WM_GETSERVERLIST_START:
			//Initialize_GetServerListThread(SCAN_ALL_GAMES);
			break;
		case WM_GETSERVERLIST_STOP:

			break;
		case WM_REFRESHSERVERLIST:
			Initialize_RedrawServerListThread();
			return TRUE;
		case WM_CREATE:
			OnCreate(hWnd,g_hInst);			
		break;
		case WM_TIMER:
		{
			switch (wParam) 
			{ 
				case IDT_1SECOND: 
					Animate();
					break;
				case IDT_DETECT_GAME:
					DetectAnyRunningGame();
					break;
				case IDT_MONITOR_QUERY_SERVERS:
					Initialize_Monitor();
					break;
				
			}
			break;
		}
		case WM_NOTIFY:		return OnNotify(hWnd,  message,  wParam,  lParam);
		case WM_SIZE:
	   {			 
		dbg_print("WM_SIZE %d",wParam);

		if(wParam==SIZE_MAXIMIZED)
			AppCFG.nWindowState = SW_SHOWMAXIMIZED;
		else if(g_bOnCreate==FALSE)
			AppCFG.nWindowState = SW_SHOWNORMAL;

		if(wParam==SIZE_RESTORED)
		{
			 OnRestore();
			 PostMessage(g_hWnd,WM_REINIT_COUNTRYFILTER,0,0);
		}
		else if (wParam==SIZE_MINIMIZED)
		{			
			 g_bMinimized=true;
			 OnMinimize(g_hWnd);
			 if(AppCFG.bShowMinimizePopUp) //Show for first time only
			 {
				 ShowBalloonTip("Notification","Game Scanner is still running.\nDouble click on the task tray icon to\nopen it again.");
				 AppCFG.bShowMinimizePopUp = FALSE;
			 }
			 return 0;
		 }
		dbg_print("WM_SIZE: Win state %d",AppCFG.nWindowState);
		OnSize(hWnd,wParam);
		if(wParam==SIZE_RESTORED)
			 InvalidateRect(hWnd,NULL,TRUE);

		return 0;
	   }
	case WM_LBUTTONDOWN:	OnLeftMouseButtonDown( hWnd, wParam, lParam);		break;
	case WM_MOUSEMOVE:		OnMouseMove(hWnd,  wParam, lParam);					break;
	case WM_LBUTTONUP:		OnLeftMouseButtonUp( hWnd,  wParam, lParam);		break;
	case WM_HOTKEY:
		if ((wParam == HOTKEY_ID))
			tryToMinimizeGame();
		break;		
	case WM_USER_SHELLICON: 
		{
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
							OnSearchFieldChange();

						}
						//return FALSE;
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
							strcpy(gm.g_currServerIP,ip);
							//dwCurrPort = dwPort;	
							g_CurrentSRV = FindServerByIPandPort(ip,dwPort);
					
							ListView_DeleteAllItems(g_hwndListViewVars);
							ListView_DeleteAllItems(g_hwndListViewPlayers);
							gm.GetServerInfo(g_currentGameIdx,g_CurrentSRV);
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
							 
					ShowWindow(hWnd, SW_RESTORE);
				
					SetForegroundWindow(hWnd);

					if(g_bMinimized==false)
					{
						ShowWindow(hWnd, SW_SHOW);
						ShowWindow(hWnd, SW_SHOWNORMAL);	
						return TRUE;					
					}
					g_bMinimized = false;
					return 0;
				}
				case IDM_SCAN_ALL_GAMES:
					gm.GamesInfo[g_currentGameIdx].dwViewFlags = 0;
					OnActivate_ServerList(SCAN_ALL_GAMES);
					break;
				case IDM_SCAN_FILTERED:
					gm.GamesInfo[g_currentGameIdx].dwViewFlags |= FORCE_SCAN_FILTERED;
					OnActivate_ServerList(SCAN_FILTERED);
					break;
				case IDM_SCAN:  //Toolbar
					OnActivate_ServerList(SCAN_ALL);
				break;
				case IDM_REFRESH:  //Toolbar
				case ID_OPTIONS_REFRESH: 
					gm.GamesInfo[g_currentGameIdx].dwViewFlags = 0;
					OnScanButton();							
				break;
				case IDC_DOWNLOAD:
					{
						if(MessageBox(hWnd,g_lang.GetString("AskToUpdate"),"Update Game Scanner",MB_YESNO)==IDYES)
						{
							HANDLE hThread;
							hThread = CreateThread( NULL, 0, &AutomaticDownloadUpdateSetUp, g_hWnd,0, NULL);                
							if (hThread == NULL) 
							{
								log.AddLogInfo(GS_LOG_WARNING, "CreateThread failed  <AutomaticDownloadUpdateSetUp> (%d) File:(%s) Line:(%d)\n", GetLastError(),__FILE__,__LINE__ ); 
							}
							else 
							{
								dbg_print("CreateThread  success AutomaticDownloadUpdateSetUp");
								CloseHandle( hThread );
							}
						}
					}				
				break;
				case IDC_BUTTON_QUICK_CONNECT:		FastConnect();							break;				
				case IDC_BUTTON_ADD_SERVER:			OnButtonClick_AddServer();				break;
				case IDC_BUTTON_FIND:				OnSearchFieldChange();					break;
				case ID_IMPORT_SERVERS:				OnImportServers(hWnd);					break;
				case ID_EXPORT_ALL_SERVERS:			OnExportServers(hWnd);					break;
				case IDM_OPTIONS_RCON:				OnRCON();								break;
				case ID_BUDDY_ADD:					bm.Buddy_Add(g_hInst,g_hWnd,true);		break;
				case ID_BUDDY_REMOVE:				bm.Remove();							break;
				case ID_BUDDY_ADDFROMPLAYERLIST:	bm.Buddy_Add(g_hInst,g_hWnd,false);		break;
				case ID_FAVORITES_ADDIP:			Favorite_Add(true);						break;
				case ID_HELP_DOCUMENTATIONFAQ:		ShellExecute(NULL,NULL,"http://www.bdamage.se/documentation.html",NULL,NULL,SW_SHOWNORMAL);	break;
				case ID_SERVERLIST_PURGEPUBLICSERVLIST:		DeleteAllServerLists(hWnd);		break;
				case IDM_UPDATE:
					{
						HANDLE hThread;
						hThread = CreateThread( NULL, 0, &CheckForUpdates, (LPVOID)lParam,0, NULL);      //lParam = 1 = silent = no messageboxes          
						if (hThread == NULL) 
						{
							log.AddLogInfo(GS_LOG_WARNING, "CreateThread failed  <CheckForUpdates> (%d) File:(%s) Line:(%d)\n", GetLastError(),__FILE__,__LINE__ ); 
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
						ChangeViewStates(LOWORD(wParam));
					break;
				case IDM_SETTINGS:
					{
						DialogBox(g_hInst, (LPCTSTR)IDD_DIALOG_CONFIG, g_hWnd, (DLGPROC)CFG_MainProc);
					
						OnSize(g_hWnd,SIZE_RESTORED,TRUE);
						tvmgr.ReBuildList();							
						SetDlgTrans(hWnd,AppCFG.g_cTransparancy);
						
						currCV = &gm.GamesInfo[g_currentGameIdx];		//restore currCV pointer									
						//Do we need to change view after configuring?
						if(gm.GamesInfo[g_currentGameIdx].bActive==false)
						{
							ListView_DeleteAllItems(g_hwndListViewServer);
							ListView_DeleteAllItems(g_hwndListViewPlayers);
							SetCurrentActiveGame(FindFirstActiveGame());
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
			return 0L;
		case WM_CLOSE:
			{	
				dbg_print("WM_CLOSE");
				OnClose();
				ShowWindow(hWnd, SW_HIDE);	
				break;
			}
		case WM_DESTROY:
			{
				dbg_print("WM_DESTROY");
				OnDestroy(hWnd);
				PostQuitMessage(0);
				break;
			}
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
	log.AddLogInfo(0,"Error reading XML tag name (XML_GetTreeItemName)");
	return NULL;
}




/*****************************************************************

XML example:

 <ChildItem name="Internet servers" strval="" value="0" compare="0" icon="12" expanded="0" type="0" state="0" game="0" action="1" />

Usage:
 ti.dwValue =  XML_GetTreeItemInt(childItem,"value");

******************************************************************/


int XML_GetTreeItemInt(TiXmlElement* pNode, const char* attributeName)
{
	int value=0;
	if(pNode!=NULL)
	{
		int ret = pNode->QueryIntAttribute(attributeName,&value);	
		if(ret!=TIXML_NO_ATTRIBUTE)
			return value;
	}
	log.AddLogInfo(GS_LOG_WARNING,"Error finding XML attributename = %s",attributeName);

	return 0;
}

/*****************************************************************

XML example:

  <GameData name="FilterMod" value="0" />

Usage:
  ReadCfgInt(pNode, "FilterMod",(int&)gm.GamesInfo[i].filter.ddwMod);

******************************************************************/

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
			return XML_READ_OK;					
		}
	}
	log.AddLogInfo(0,"Error reading XML tag %s",szParamName);
	return XML_READ_ERROR;
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
	log.AddLogInfo(0,"Error reading XML tag %s",szParamName);
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

void XML_WriteCfgInt(TiXmlElement * root, char *szParentName,char *szAttributeName,int value) 
{
	TiXmlElement * elem;
	elem = new TiXmlElement(szParentName );  
	elem->SetAttribute(szAttributeName,value);
	root->LinkEndChild( elem );  
}

void XML_ReadWindow(TiXmlElement * pNode, _WINDOW_CONTAINER *wc) 
{
	if(pNode!=NULL)
	{
		wc->idx = XML_GetTreeItemInt(pNode, "idx");
		wc->bShow = (bool)XML_GetTreeItemInt(pNode, "show");
		wc->rSize.left = XML_GetTreeItemInt(pNode, "x");
		wc->rSize.top = XML_GetTreeItemInt(pNode, "y");
		wc->rSize.right= XML_GetTreeItemInt(pNode, "sx");
		wc->rSize.bottom = XML_GetTreeItemInt(pNode, "sy");
	}
}

void XML_WriteWindow(TiXmlElement * parent, char *szParentName, _WINDOW_CONTAINER *wc) 
{
	TiXmlElement * elem;
	elem = new TiXmlElement(szParentName );  	
	elem->SetAttribute("idx",wc->idx);
	elem->SetAttribute("show",wc->bShow);
	elem->SetAttribute("x",wc->rSize.left);
	elem->SetAttribute("y",wc->rSize.top);
	elem->SetAttribute("sx",wc->rSize.right);
	elem->SetAttribute("sy",wc->rSize.bottom);
	parent->LinkEndChild( elem );  
}


void WriteCfgStr(TiXmlElement * root, char *szParentName, char *szParamName,const char *value) 
{
	TiXmlElement * filter;
	filter = new TiXmlElement(szParentName );  
	filter->SetAttribute("name",szParamName);
	filter->SetAttribute("value",value);
	root->LinkEndChild( filter );  
}