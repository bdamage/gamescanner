#pragma once

#include "resource.h"

#include "..\..\tinyxml\tinyxml.h"
DWORD WINAPI StopScanningThread(LPVOID pvoid );
void Load_WindowSizes();
void XML_ReadWindow(TiXmlElement * pNode, _WINDOW_CONTAINER *wc) ;
void XML_WriteWindow(TiXmlElement * parent, char *szParentName, _WINDOW_CONTAINER *wc) ;
LRESULT CALLBACK EnumerateGames_Dlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void UpdateTabControl();
void ClearServerList(int i);
void ListView_InitilizeColumns();
void TreeView_ReBuildList();
int LoadIconIntoImageList(TCHAR*szFilename);
void CleanUpFilesRegistry();
DWORD WINAPI Do_ServerListSort(LPVOID column);
void Do_ServerListSortThread(int iColumn);
DWORD WINAPI  RCONDlgThread(LPVOID lpParam);
void ChangeFont(HWND hWnd,HFONT hf);
LRESULT Draw_ColorEncodedText(RECT rc, LPNMLVCUSTOMDRAW pListDraw , TCHAR *pszText);
LRESULT Draw_ColorEncodedTextQ4(RECT rc, LPNMLVCUSTOMDRAW pListDraw , TCHAR *pszText);
LRESULT Draw_ColorEncodedTextQW(RECT rc, LPNMLVCUSTOMDRAW pListDraw , TCHAR *pszText);
LRESULT Draw_ColorEncodedTextNexuiz(RECT rc, LPNMLVCUSTOMDRAW pListDraw , TCHAR *pszText);
LRESULT Draw_ColorEncodedTextUNICODE(RECT rc, LPNMLVCUSTOMDRAW pListDraw , TCHAR *pszText);
LRESULT Draw_UTF8Text(RECT rc, LPNMLVCUSTOMDRAW pListDraw , char *pszText);
DWORD MyPing(TCHAR *ipaddress, DWORD & dwPing);
int LoadConfig(APP_SETTINGS_NEW &appSettings);
int CALLBACK MyCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort); 
void ListView_SetDefaultColumns();
SERVER_INFO *Get_ServerInfoByListViewIndex(GAME_INFO *pGI,int index);
TCHAR *Get_SelectedServerIP();
void CalcSplitterGripArea();
void Default_Appsettings();
void mysort(int sortBy=3);
LRESULT ListView_SL_CustomDraw (LPARAM lParam);
int AddServerToList(TCHAR *ip, TCHAR *port);
void ClickMouse(VOID);
void Parse_FileServerList(GAME_INFO *pGI,TCHAR *szFilename);
void SetDlgTrans(HWND hwnd,int trans);
void LaunchGame(SERVER_INFO *pSI,GAME_INFO *pGI,int GameInstallIdx=0);
TCHAR GetByCountryNameImageListIndex(TCHAR *szCountry);
void CreateUpdateFile();
LRESULT CALLBACK AddBuddyProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL ExecuteGame(GAME_INFO *pGI,TCHAR *szCmd,int GameInstallIdx=0);
BOOL WINAPI EditCopy(TCHAR *pText);
PLAYERDATA *Get_PlayerBySelection();
void LoadImageList();
void Load_CountryFlags();
int Get_CountryFlagByShortName(const TCHAR *szCC);
const TCHAR * XML_GetTreeItemName(TiXmlElement* pNode,TCHAR *szOutput, DWORD maxBytes);
const TCHAR * XML_GetTreeItemStrValue(TiXmlElement* pNode,TCHAR *szOutput, DWORD maxBytes);
const TCHAR * XML_GetTreeItemStr(TiXmlElement* pNode, const TCHAR* attributeName,TCHAR *szOutput, DWORD maxBytes);
void LoadAllServerList();
DWORD WINAPI LoadAllServerListThread(LPVOID lpVoid);
DWORD WINAPI CFG_Save(LPVOID lpVoid);
int CFG_Load();
void Default_GameSettings();
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
int SetCurrentActiveGame(int index);
int FindFirstActiveGame();
void RefreshServerList();
void OnSize(HWND hwndParent,WPARAM wParam, BOOL bRepaint=TRUE);
//void ResizeListView(HWND g_hwndListViewPlayers, HWND hwndListViewVars, HWND hwndParent, BOOL bRepaint=TRUE);
void tryToMinimizeGame();

void Show_StopScanningButton(BOOL show);
void Show_ToolbarButton(int id, bool show);

TCHAR getPlayerTypeNEW(int index);

LRESULT APIENTRY DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) ;
TCHAR* ReCalcTextCB();
//LRESULT ListViewNotifyNEW(HWND hWnd,  WPARAM wParam, LPARAM lParam);
void Initialize_RedrawServerListThread();
BOOL TreeView_SetItemCheckState(TCHAR gameIdx, DWORD dwType,DWORD dwNewState);
BOOL TreeView_SetFilterGroupCheckState(TCHAR gameIdx, DWORD dwType,DWORD dwFilterValue);
BOOL TreeView_SetCheckBoxState(int iSel,DWORD dwState);
BOOL TreeView_SetDWValueByItemType(DWORD dwType,DWORD dwNewValue,DWORD dwNewState, TCHAR*pszNewStrValue=NULL);

DWORD TreeView_GetItemStateByType(TCHAR cGameIdx,DWORD dwType);
DWORD WINAPI GetServerList(LPVOID lpParam );
DWORD WINAPI ProgressGUI_Thread(LPVOID lpParam);
DWORD WINAPI CheckForUpdates(LPVOID lpParam);
SERVER_INFO *Get_ServerInfoByIndex(GAME_INFO *pGI,int index);

DWORD WINAPI  GetServerStatusThread(LPVOID lpParam);

bool FilterServerItemV2(SERVER_INFO *lp, GAME_INFO *pGI); 
DWORD WINAPI RedrawServerList(LPSERVERINFO pServerInfo,DWORD dwFilterFlags=0);// bool bForceFavorites=false, bool bForceHistory=false,bool bForceFavoritesPrivate=false);
DWORD WINAPI  RedrawServerListThread(LPVOID pvoid );
void ShowBalloonTip(TCHAR *title,TCHAR *message);
void _stdcall TraceRoute(TCHAR *ipaddress );
LRESULT CALLBACK AddServerProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void InitShowServer();

int GetPingFilter();
void SetPingFilter();
bool IsServerAlive(TCHAR *host);

void mysortBuddy(int sortBy);
 

void ErrorExit(LPTSTR lpszFunction);
void SetSelectedTaskTrayServer(int index);
//void ListIsBusyMessage(TCHAR *szText);
void EnableButtons(bool active);
//void	AddLogInfo(int color, TCHAR *lpszText, ...);
DWORD	WINAPI  MainDlgThread(LPVOID lpParam);

void Initialize_WindowSizes();
void Update_WindowSizes(RECT *pRC=NULL);

LRESULT APIENTRY ListView_Rules_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
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


void Load_Countryfilter(TCHAR *filename);
void Save_Countryfilter(TCHAR *filename);

void dbg_dumpbuf(const TCHAR *file, const void *buf, size_t size);
//void parseServerData(TCHAR *packet, DWORD packlen);
TCHAR* parseData(TCHAR *toBuff,TCHAR*buff);
TCHAR* parseString(TCHAR *toBuff, TCHAR*buff);
//void Q3_ParsePlayerData(TCHAR*packet,PLAYER_DATA *ppd, DWORD packlen);
SOCKET getsock(const TCHAR *host, unsigned short port, int family, int socktype, int protocol);

#define getsockudp(host,port) getsock(host, port, AF_INET, SOCK_DGRAM, IPPROTO_UDP)
#define getsocktcp(host,port) getsock(host, port, AF_INET, SOCK_STREAM, IPPROTO_TCP)