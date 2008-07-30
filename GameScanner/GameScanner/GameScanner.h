#pragma once

#include "resource.h"
#include "structs_defines.h"

DWORD WINAPI GetServerListThread(LPVOID lpParam );
DWORD WINAPI  RCONDlgThread(LPVOID lpParam);

DWORD MyPing(char *ipaddress, DWORD & dwPing);
int LoadConfig(APP_SETTINGS_NEW &appSettings);
int CALLBACK MyCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort); 
void ListView_SetDefaultColumns();
SERVER_INFO Get_ServerInfoByIndex(int index);
char *Get_SelectedServerIP();
void CalcSplitterGripArea();
void Default_Appsettings();
void mysort(int sortBy=3);
int AddServerToList(char *ip, char *port);
void ClickMouse(VOID);
void Parse_FileServerList(GAME_INFO *pGI,char *szFilename);
void SetDlgTrans(HWND hwnd,int trans);
void LaunchGame(SERVER_INFO pSI,GAME_INFO *pGI);
char GetByCountryNameImageListIndex(char *szCountry);
void CreateUpdateFile();
LRESULT CALLBACK AddBuddyProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL ExecuteGame(GAME_INFO *pGI,char *szCmd);
BOOL WINAPI EditCopy(char *pText);
PLAYERDATA *Get_PlayerBySelection();
void LoadImageList();
void Load_CountryFlags();
int Get_CountryFlagByShortName(const char *szCC);
const char * XML_GetTreeItemName(TiXmlElement* pNode,char *szOutput, DWORD maxBytes);
const char * XML_GetTreeItemStrValue(TiXmlElement* pNode,char *szOutput, DWORD maxBytes);
const char * XML_GetTreeItemStr(TiXmlElement* pNode, const char* attributeName,char *szOutput, DWORD maxBytes);
void LoadAllServerList();
void CFG_Save();
int CFG_Load();
int TreeView_load();
LRESULT TreeView_CustomDraw(LPARAM lParam);
HWND WINAPI TOOLBAR_CreateRebar(HWND hwndOwner);
DWORD WINAPI AutomaticDownloadUpdateSetUp(LPVOID lpParam);
// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);

BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	MainProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PRIVPASS_Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
int SetCurrentViewTo(int index);
int FindFirstActiveGame();
void RefreshServerList();
void OnSize(HWND hwndParent, BOOL bRepaint=TRUE);
//void ResizeListView(HWND g_hwndListViewPlayers, HWND hwndListViewVars, HWND hwndParent, BOOL bRepaint=TRUE);
void tryToMinimizeGame();

void Show_StopScanningButton(BOOL show);
void Show_ToolbarButton(int id, bool show);

char getPlayerTypeNEW(int index);

LRESULT APIENTRY DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) ;
char* ReCalcTextCB();
//LRESULT ListViewNotifyNEW(HWND hWnd,  WPARAM wParam, LPARAM lParam);
void Initialize_RedrawServerListThread();
BOOL TreeView_SetItemCheckState(char gameIdx, DWORD dwType,DWORD dwNewState);
BOOL TreeView_SetFilterGroupCheckState(char gameIdx, DWORD dwType,DWORD dwFilterValue);
BOOL TreeView_SetCheckBoxState(int iSel,DWORD dwState);
BOOL TreeView_SetDWValueByItemType(DWORD dwType,DWORD dwNewValue,DWORD dwNewState, char*pszNewStrValue=NULL);

DWORD TreeView_GetItemStateByType(char cGameIdx,DWORD dwType);
DWORD WINAPI  QueryThread(LPVOID lpParam );
DWORD WINAPI  PlayNotify(LPVOID lpParam );
DWORD WINAPI GetServerList(LPVOID lpParam );
DWORD WINAPI ProgressGUI_Thread(LPVOID lpParam);
DWORD WINAPI CheckForUpdates(LPVOID lpParam);
//DWORD WINAPI  RefreshServerListThread(LPVOID lpParam);
DWORD WINAPI  GetServerStatusThread(LPVOID lpParam);
//bool FilterServerItem(LPARAM *lp, DWORD dwFlags); //bool bForceFavorites=false, bool bForceHistory=false);
bool FilterServerItemV2(LPARAM *lp, GAME_INFO *pGI); //bool bForceFavorites=false, bool bForceHistory=false);
DWORD WINAPI RedrawServerList(LPSERVERINFO pServerInfo,DWORD dwFilterFlags=0);// bool bForceFavorites=false, bool bForceHistory=false,bool bForceFavoritesPrivate=false);
DWORD WINAPI  RedrawServerListThread(LPVOID pvoid );
void ShowBalloonTip(char *title,char *message);
void _stdcall TraceRoute(char *ipaddress );
LRESULT CALLBACK AddServerProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void InitShowServer();

int StrSorter(char *a, char *b);
int GetPingFilter();
void SetPingFilter();
bool IsServerAlive(char *host);

void mysortBuddy(int sortBy);
 

void ErrorExit(LPTSTR lpszFunction);
void SetSelectedTaskTrayServer(int index);
//void ListIsBusyMessage(char *szText);
void EnableButtons(bool active);
//void	AddLogInfo(int color, char *lpszText, ...);
DWORD	WINAPI  MainDlgThread(LPVOID lpParam);

void Initialize_WindowSizes();
void Update_WindowSizes();

LRESULT APIENTRY ListViewPlayerSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ComboCountriesSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ComboListCountriesSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ListViewServerListSubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY LV_SL_HeaderSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) ;
LRESULT APIENTRY ListViewBuddySubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam,     LPARAM lParam) ;
LRESULT APIENTRY ButtonPaneVSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ButtonPaneH1SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ButtWindProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ButtOptionsWindProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ButtSearchWindProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY ButtAddToFavoriteWindProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY TreeView_SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) ;
LRESULT APIENTRY ComboBox_SearchSubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) ;

//void EnableDownloadLink(BOOL bEnable);
void UpdateCurrentServerUI();
void SetInitialViewStates();
DWORD WINAPI  RefreshServerListThread_v2(LPVOID lpParam);
//void EnableFilterOptions(BOOL active);
void CheckAll(bool state);

DWORD WINAPI  UpdateNotify(LPVOID lpParam);


void Load_Countryfilter(char *filename);
void Save_Countryfilter(char *filename);

void dbg_dumpbuf(const char *file, const void *buf, size_t size);
//void parseServerData(char *packet, DWORD packlen);
char* parseData(char *toBuff,char*buff);
char* parseString(char *toBuff, char*buff);
//void Q3_ParsePlayerData(char*packet,PLAYER_DATA *ppd, DWORD packlen);
SOCKET getsock(const char *host, unsigned short port, int family, int socktype, int protocol);

#define getsockudp(host,port) getsock(host, port, AF_INET, SOCK_DGRAM, IPPROTO_UDP)
#define getsocktcp(host,port) getsock(host, port, AF_INET, SOCK_STREAM, IPPROTO_TCP)