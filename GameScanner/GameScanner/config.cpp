
#include "stdafx.h"
#include "config.h"
#include "utilz.h"
#include "gamemanager.h"

#pragma warning(disable : 4995)
#pragma warning(disable : 4996)

extern LRESULT CALLBACK FilterEditor_Dlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern void Default_GameSettings();
extern void Default_Appsettings();
extern DWORD WINAPI CFG_Save(LPVOID lpVoid);
extern string g_sMIRCoutput;
extern HFONT g_hf;
string CFG_g_sMIRCoutputTemp;
extern void ChangeFont(HWND hWnd, HFONT hf);
extern HWND g_hWnd;
string g_cfgScriptTmp;
string g_cfgFilterNameTmp;
//Config dialog vars
#define GAME_CFG_INDEX 7   //This is the helper index in treeview where all games configuration starts from
bool g_bChanged=false;
APP_SETTINGS_NEW AppCFGtemp;
HWND g_hwndTree=NULL;
extern CLogger g_log;
DWORD g_tvIndexCFG=0;
extern HIMAGELIST g_hImageListIcons;
extern HIMAGELIST g_hImageListStates;
extern APP_SETTINGS_NEW AppCFG;
//extern GamesMap GamesInfo;
GamesMap GamesInfoCFG;

extern CGameManager gm;

extern HINSTANCE g_hInst;
extern TCHAR EXE_PATH[_MAX_PATH+_MAX_FNAME];			//Don't write anything to this path
extern TCHAR USER_SAVE_PATH[_MAX_PATH+_MAX_FNAME];     //Path to save settings and server lists
extern TCHAR COMMON_SAVE_PATH[_MAX_PATH+_MAX_FNAME];   //Used for downloading app update and preview images - purpose to share the same data between users.
extern TCHAR EXE_PATH_OLD[_MAX_PATH+_MAX_FNAME];
extern CLanguage g_lang;

typedef struct tag_dlghdr { 
    HWND hwndTab;       // tab control 
    HWND hwndDisplay;   // current child dialog box 
    RECT rcDisplay;     // display rectangle for the tab control 
    DLGTEMPLATE *apRes[GAME_CFG_INDEX+50];  //need to be fixed 50 = GamesInfo().size
} DLGHDR; 

DLGHDR *g_pHdr = NULL;
int g_currSelCfg=-2;
HWND hwndConfDialog=NULL;

void CFG_Apply_General(HWND hDlg)
{
	
	AppCFGtemp.bUseBuddySndNotify = IsDlgButtonChecked(hDlg,IDC_CHECK_USE_WAV_FILE);

	AppCFGtemp.bBuddyNotify = IsDlgButtonChecked(hDlg,IDC_CHECK_BUDDY_NOTIFY);

	GetDlgItemText(hDlg,IDC_EDIT_WAV_FILE,AppCFGtemp.szNotifySoundWAVfile,MAX_PATH);						

	AppCFGtemp.bNoMapResize = IsDlgButtonChecked(hDlg,IDC_CHECK_NO_MAP_RESIZE);

	AppCFGtemp.bCloseOnConnect = IsDlgButtonChecked(hDlg,IDC_CHECK_CONNECT_CLOSE);

	AppCFGtemp.bRegisterWebProtocols = IsDlgButtonChecked(hDlg,IDC_CHECK_REG_WEB_PROTOCOLS);

	if(IsDlgButtonChecked(hDlg,IDC_CHECK1)==BST_CHECKED)
		AppCFGtemp.bAutostart=true;
	else
		AppCFGtemp.bAutostart=false;

	if(IsDlgButtonChecked(hDlg,IDC_CHECK_SHORTNAME)==BST_CHECKED)
		AppCFGtemp.bUseShortCountry=TRUE;
	else
		AppCFGtemp.bUseShortCountry=FALSE;


}
void CFG_Apply_MIRC(HWND hDlg)
{

	if(IsDlgButtonChecked(hDlg,IDC_CHECK_MIRC)==BST_CHECKED)
		AppCFGtemp.bUseMIRC=true;
	else
		AppCFGtemp.bUseMIRC=false;

	 TCHAR tmp[MAX_PATH];	 
	 GetDlgItemText(hDlg,IDC_EDIT_MIRC,tmp,MAX_PATH);
	 CFG_g_sMIRCoutputTemp = tmp;
	 g_sMIRCoutput = CFG_g_sMIRCoutputTemp;

}

void CFG_Apply_Games(int gameID,HWND hDlg)
{
	g_bChanged = false;
	if(gameID!=-1)
	{
		if(IsDlgButtonChecked(hDlg,IDC_CHECK_ACTIVE)==BST_CHECKED)
			GamesInfoCFG[gameID].bActive=TRUE;
		else
			GamesInfoCFG[gameID].bActive=FALSE;

/*		if(IsDlgButtonChecked(hDlg,IDC_CHECK_USE_HTTP)==BST_CHECKED)
			GamesInfoCFG[gameID].bUseHTTPServerList[0] = TRUE;
		else
			GamesInfoCFG[gameID].bUseHTTPServerList[0] = FALSE;
*/
		GetDlgItemText(hDlg,IDC_EDIT_MASTER_SERVER,GamesInfoCFG[gameID].szMasterServerIP[0],MAX_PATH);						

		if(GamesInfoCFG[gameID].bUseHTTPServerList==FALSE)
			SplitIPandPORT(GamesInfoCFG[gameID].szMasterServerIP[0],GamesInfoCFG[gameID].dwMasterServerPORT);
	}
}

void CFG_Apply_Network(HWND hDlg)
{
	g_bChanged = false;						
	TCHAR szTmp[10];
	GetDlgItemText(hDlg,IDC_EDIT_SOCK_TIMEOUT_S,szTmp,sizeof(szTmp));
	
	AppCFGtemp.socktimeout.tv_sec = atoi(szTmp)/1000;
	AppCFGtemp.socktimeout.tv_usec = atoi(szTmp) - (AppCFGtemp.socktimeout.tv_sec*1000);


	if(AppCFGtemp.socktimeout.tv_usec < 100 && AppCFGtemp.socktimeout.tv_sec == 0)
		AppCFGtemp.socktimeout.tv_usec = 100;

	AppCFGtemp.dwThreads = (DWORD)SendMessage(GetDlgItem(hDlg,IDC_SLIDER_THREADS),TBM_GETPOS,0,(LPARAM)0) ; 

	GetDlgItemText(hDlg,IDC_EDIT_CFG_RETRIES,szTmp,sizeof(szTmp));
	AppCFGtemp.dwRetries = atoi(szTmp);

}

void CFG_Apply_XMPP(HWND hDlg)
{
	g_bChanged = false;

	AppCFGtemp.bXMPP_Active = IsDlgButtonChecked(hDlg,IDC_CHECK_XMPP_ACTIVE);

	GetDlgItemText(hDlg,IDC_EDIT_XMPP_USERNAME,AppCFGtemp.szXMPP_USERNAME,MAX_PATH);
	GetDlgItemText(hDlg,IDC_EDIT_XMPP_PASSWORD,AppCFGtemp.szXMPP_PASSWORD,MAX_PATH);
	GetDlgItemText(hDlg,IDC_EDIT_XMPP_SERVER,AppCFGtemp.szXMPP_SERVER,MAX_PATH);

	TCHAR szTmp[10];
	GetDlgItemText(hDlg,IDC_EDIT_XMPP_PORT,szTmp,sizeof(szTmp));
	AppCFGtemp.dwXMPP_PORT = atoi(szTmp);

}

void CFG_Apply_Ext(HWND hDlg)
{
	g_bChanged = false;

	AppCFGtemp.bUse_EXT_APP = IsDlgButtonChecked(hDlg,IDC_CHECK_EXT_ACTIVE);

	GetDlgItemText(hDlg,IDC_EDIT_EXT_EXE,AppCFGtemp.szEXT_EXE_PATH,MAX_PATH);
	GetDlgItemText(hDlg,IDC_EDIT_EXT_CMD,AppCFGtemp.szEXT_EXE_CMD,MAX_PATH);
	GetDlgItemText(hDlg,IDC_EDIT_EXT_WINDOWNAME,AppCFGtemp.szEXT_EXE_WINDOWNAME,MAX_PATH);

	AppCFGtemp.bUse_EXT_APP2 = IsDlgButtonChecked(hDlg,IDC_CHECK_EXT_ACTIVE2);
	GetDlgItemText(hDlg,IDC_EDIT_EXT_EXE2,AppCFGtemp.szOnReturn_EXE_PATH,MAX_PATH);
	GetDlgItemText(hDlg,IDC_EDIT_EXT_CMD2,AppCFGtemp.szOnReturn_EXE_CMD,MAX_PATH);
}

void CFG_Apply_Minimizer(HWND hDlg)
{
	int i = SendDlgItemMessage (hDlg,IDC_COMBO_MOD,(UINT) CB_GETCURSEL,  0,  0);  

	if(i==0)
		AppCFGtemp.dwMinimizeMODKey = MOD_ALT;
	else if(i==1)
		AppCFGtemp.dwMinimizeMODKey = MOD_CONTROL;
	else if(i==2)
		AppCFGtemp.dwMinimizeMODKey = MOD_SHIFT;	

	if(IsDlgButtonChecked(hDlg,IDC_CHECK2)==BST_CHECKED)
		AppCFGtemp.bUse_minimize =true;
	else
		AppCFGtemp.bUse_minimize=false;
	TCHAR tmp[20];
	GetDlgItemText(hDlg,IDC_EDIT_KEY,tmp,2);
	AppCFGtemp.cMinimizeKey = (TCHAR)toupper((TCHAR)tmp[0]);
	
	if(IsDlgButtonChecked(hDlg,IDC_CHECK_SCR_RESTORE)==BST_CHECKED)
		AppCFGtemp.bUSE_SCREEN_RESTORE  =true;
	else
		AppCFGtemp.bUSE_SCREEN_RESTORE=false;

	GetDlgItemText(hDlg,IDC_EDIT_WINDOWNAME,AppCFGtemp.szET_WindowName,MAX_PATH);
					
}

void CFG_Enable_ApplyBtn(HWND hDlg,BOOL enable)
{
	   EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_APPLY_GENERAL),enable);
	   EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_APPLY_MINIMIZER),enable);
	   EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_APPLY),enable);
	   EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_APPLY_EXT),enable);
	   EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_APPLY_LOOK),enable);
}
void CFG_Apply_Look(HWND hDlg)
{
	AppCFGtemp.g_cTransparancy = (TCHAR)SendMessage(GetDlgItem(hDlg,IDC_SLIDER_TRANS),TBM_GETPOS,0,(LPARAM)0) ; 
}

DWORD CFG_GetGameID(int selectionIndex)
{	
	if(selectionIndex>GAME_CFG_INDEX)
	{
		return selectionIndex - (GAME_CFG_INDEX +1);
	}
	return 0xFFFFFFFF;
}

HTREEITEM TreeView_AddItem(int iImage, const TCHAR *text)
{
	HTREEITEM hCurrent = TreeView_GetSelection(g_hwndTree);

	TVINSERTSTRUCT tvs;
	tvs.item.mask                   =  TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;

	if (!hCurrent)
		tvs.hParent = TVI_ROOT;
	else
		tvs.hParent = hCurrent;

	tvs.item.pszText            = (LPSTR)text;    
	tvs.item.iSelectedImage = tvs.item.iImage = iImage;
	tvs.item.lParam = g_tvIndexCFG++;
	tvs.item.cchTextMax             = lstrlen(tvs.item.pszText) + 1;
	tvs.hInsertAfter = TVI_LAST;

	HTREEITEM hNewItem = TreeView_InsertItem(g_hwndTree, &tvs);
    
	if (hCurrent)
		TreeView_Expand(g_hwndTree, hCurrent, TVE_EXPAND);
	return hNewItem;
}

void CFG_ApplySettings()
{
	int gameID=-1;
	switch(g_currSelCfg)
	{
		case 0: CFG_Apply_General(hwndConfDialog); break;
		case 1: CFG_Apply_Minimizer(hwndConfDialog); break;
		case 2: CFG_Apply_MIRC(hwndConfDialog); break;
		case 3: CFG_Apply_XMPP(hwndConfDialog); break;
		case 4: CFG_Apply_Ext(hwndConfDialog); break;
		case 5: CFG_Apply_Look(hwndConfDialog); break;
		case 6: CFG_Apply_Network(hwndConfDialog); break;

		default:
				gameID = CFG_GetGameID(g_currSelCfg);
			break;

	}
	if(gameID!=-1)
		CFG_Apply_Games(gameID,hwndConfDialog);
}

LRESULT CALLBACK CFG_MainProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{
		
			g_tvIndexCFG = 0;
			g_hwndTree = GetDlgItem(hDlg,IDC_TREE_CONF);

			SendMessage(g_hwndTree, TVM_SETIMAGELIST , TVSIL_NORMAL, (LPARAM)g_hImageListIcons);
			SendMessage(g_hwndTree, TVM_SETIMAGELIST , TVSIL_STATE, (LPARAM)g_hImageListStates);
			HTREEITEM hNewItem;
			hNewItem = TreeView_AddItem(27,g_lang.GetString("ConfigGeneral"));
			hNewItem = TreeView_AddItem(15,g_lang.GetString("ConfigMinimizer"));
			hNewItem = TreeView_AddItem(28,"mIRC");
			hNewItem = TreeView_AddItem(3,"Account (XMPP)");
			hNewItem = TreeView_AddItem(16,g_lang.GetString("ConfigExtExe"));

			hNewItem = TreeView_AddItem(25,g_lang.GetString("ConfigGraphic"));
			hNewItem = TreeView_AddItem(13,g_lang.GetString("ConfigNetwork"));
			hNewItem = TreeView_AddItem(20 ,g_lang.GetString("ConfigGames"));
			if (hNewItem)
				TreeView_Select(g_hwndTree, hNewItem, TVGN_CARET);

			for(UINT i=0;i<gm.GamesInfo.size();i++)
				hNewItem = TreeView_AddItem(gm.GamesInfo[i].iIconIndex,gm.GamesInfo[i].szGAME_SHORTNAME);

			TreeView_Select(g_hwndTree, NULL, TVGN_CARET);

			for(UINT i=0; i<gm.GamesInfo.size();i++)
			{
					GamesInfoCFG[i].bActive = gm.GamesInfo[i].bActive;
					GamesInfoCFG[i].bUseHTTPServerList[0] = gm.GamesInfo[i].bUseHTTPServerList[0];
					GamesInfoCFG[i].dwMasterServerPORT = gm.GamesInfo[i].dwMasterServerPORT;
					strcpy(GamesInfoCFG[i].szGAME_NAME, gm.GamesInfo[i].szGAME_NAME);
					strcpy(GamesInfoCFG[i].szMasterServerIP[0], gm.GamesInfo[i].szMasterServerIP[0]);					
					GamesInfoCFG[i].vGAME_INST = gm.GamesInfo[i].vGAME_INST;								
			}
			memcpy(&AppCFGtemp,&AppCFG,sizeof(APP_SETTINGS_NEW));
			CFG_g_sMIRCoutputTemp = g_sMIRCoutput;
			SetDlgItemText(hDlg,IDOK,g_lang.GetString("Ok"));
			SetDlgItemText(hDlg,IDC_BUTTON_DEFAULT,g_lang.GetString("SetDefault"));
			SetDlgItemText(hDlg,IDCANCEL,g_lang.GetString("Cancel"));
			
			CenterWindow(hDlg);
			CFG_OnTabbedDialogInit(hDlg) ;
			return TRUE;			
		}	

  case WM_NOTIFY: 
	  {
		NMTREEVIEW *lpnmtv;
		lpnmtv = (LPNMTREEVIEW)lParam;

		switch (wParam) 
		{ 
			case IDC_TREE_CONF:
				{
					NMTREEVIEW *pnmtv;
					pnmtv = (LPNMTREEVIEW) lParam;
					if((lpnmtv->hdr.code  == TVN_SELCHANGED)  )
					{
						if((g_bChanged==true) && (pnmtv->action == TVC_BYMOUSE))
							CFG_ApplySettings();

						CFG_OnSelChanged(hDlg);
					}
				}
			break;

		} 
		break; 
	  }

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))				
			{
				case  IDCANCEL:	
					LocalFree(g_pHdr);
					EndDialog(hDlg, LOWORD(wParam)); 
				return TRUE;
				case IDOK:
				{					
					CFG_ApplySettings();
					
					if(AppCFGtemp.bAutostart)
						AddAutoRun(EXE_PATH);
					else
						RemoveAutoRun();
					
					if(AppCFGtemp.bUse_minimize)
					{	
						UnregisterHotKey(NULL, HOTKEY_ID);
						if (!RegisterHotKey(NULL, HOTKEY_ID, AppCFGtemp.dwMinimizeMODKey ,AppCFGtemp.cMinimizeKey))
						{
							//probably already registred
							MessageBox(NULL,g_lang.GetString("ErrorRegHotkey"),"Hotkey error",NULL);
						}
					}else
					{
						UnregisterHotKey(NULL, HOTKEY_ID);
					}
					
					
					for(UINT i=0; i<gm.GamesInfo.size();i++)
					{
						gm.GamesInfo[i].bActive = GamesInfoCFG[i].bActive;
						gm.GamesInfo[i].bUseHTTPServerList[0] = GamesInfoCFG[i].bUseHTTPServerList[0];
						gm.GamesInfo[i].dwMasterServerPORT = GamesInfoCFG[i].dwMasterServerPORT;					
						strcpy(gm.GamesInfo[i].szMasterServerIP[0], GamesInfoCFG[i].szMasterServerIP[0]);
						gm.GamesInfo[i].vGAME_INST = GamesInfoCFG[i].vGAME_INST;	
					}

					memcpy(&AppCFG,&AppCFGtemp,sizeof(APP_SETTINGS_NEW));
					//memcpy(&GI,&GamesInfoCFG,sizeof(GAME_INFO)*GamesInfo.size());
				//	ZeroMemory(&GamesInfoCFG,sizeof(GAME_INFO)*GamesInfo.size());
					ZeroMemory(&AppCFGtemp,sizeof(APP_SETTINGS_NEW));
					HANDLE hThread=NULL; 
					DWORD dwThreadIdBrowser=0;				
					hThread = CreateThread( NULL, 0, &CFG_Save,(LPVOID)0 ,0, &dwThreadIdBrowser);                
					if (hThread == NULL) 
					{
						g_log.AddLogInfo(GS_LOG_WARNING, "CreateThread CFG_Save failed (%d)\n", GetLastError() ); 
					} else
					{
						CloseHandle( hThread );
					}

			//		CFG_Save(0);
					LocalFree(g_pHdr);
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}
				case IDC_BUTTON_DEFAULT:
				{				
					Default_Appsettings();
				//	Default_GameSettings();
					memcpy(&AppCFGtemp,&AppCFG,sizeof(APP_SETTINGS_NEW));
					//memcpy(&GamesInfoCFG,&GI,sizeof(GAME_INFO)*GamesInfo.size());
					//GamesInfoCFG = GamesInfo;
					g_currSelCfg=-1;
					CFG_OnSelChanged(hDlg); 
					return TRUE;
				}			

				break;
			}
		}
	}
	
	return FALSE;
}



VOID WINAPI CFG_OnTabbedDialogInit(HWND hwndDlg) 
	{ 
	g_pHdr = (DLGHDR *) LocalAlloc(LPTR, sizeof(DLGHDR)); 
   
	g_currSelCfg = -2;
    // Save a pointer to the DLGHDR structure. 
    SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG) g_pHdr); 
 
	g_pHdr->hwndTab = GetDlgItem(hwndDlg,IDC_TREE_CONF); 
 
    // Lock the resources for the three child dialog boxes. 
    g_pHdr->apRes[0] = DoLockDlgRes(MAKEINTRESOURCE(IDD_CFG1)); 
    g_pHdr->apRes[1] = DoLockDlgRes(MAKEINTRESOURCE(IDD_CFG2)); 
	g_pHdr->apRes[2] = DoLockDlgRes(MAKEINTRESOURCE(IDD_CFG_MIRC));
	g_pHdr->apRes[3] = DoLockDlgRes(MAKEINTRESOURCE(IDD_CFG_XMPP));
	g_pHdr->apRes[4] = DoLockDlgRes(MAKEINTRESOURCE(IDD_CFG4)); 
	g_pHdr->apRes[5] = DoLockDlgRes(MAKEINTRESOURCE(IDD_CFG5)); 
	g_pHdr->apRes[6] = DoLockDlgRes(MAKEINTRESOURCE(IDD_CFG_NET)); 
	g_pHdr->apRes[7] = DoLockDlgRes(MAKEINTRESOURCE(IDD_CFG3_NEW)); 
	
	g_currSelCfg=-1;
    CFG_OnSelChanged(hwndDlg); 
} 
	
// CFG_OnSelChanged - processes the TCN_SELCHANGE notification. 
// hwndDlg - handle to the parent dialog box. 
VOID WINAPI CFG_OnSelChanged(HWND hwndDlg) 
{ 
	 DLGHDR *pHdr = (DLGHDR *) GetWindowLong( hwndDlg, GWLP_USERDATA); 
		 
	 if(pHdr==NULL)
		 return;

	HTREEITEM hTreeItem = TreeView_GetSelection(pHdr->hwndTab);
	
	TVITEM tvitem;
	memset(&tvitem,0,sizeof(TVITEM));
	tvitem.hItem = hTreeItem;
	tvitem.mask = TVIF_PARAM | TVIF_TEXT;

	TreeView_GetItem(g_hwndTree, &tvitem );

	UINT iSel = (UINT)tvitem.lParam;

	if((UINT)g_currSelCfg==iSel)
		return;

	if(iSel==-1)
		iSel = 0;
	if(iSel==GAME_CFG_INDEX)
		return;
	g_currSelCfg = iSel ;

	if(iSel>GAME_CFG_INDEX)
		iSel--;

    // Destroy the current child dialog box, if any. 
    if (pHdr->hwndDisplay != NULL) 
        DestroyWindow(pHdr->hwndDisplay); 
 
	if(iSel>=GAME_CFG_INDEX+gm.GamesInfo.size()) 
		return;
	else if(iSel>GAME_CFG_INDEX)
		iSel = GAME_CFG_INDEX;
	

	if(pHdr->apRes[iSel]==NULL)
		return;

    // Create the new child dialog box. 
    pHdr->hwndDisplay = CreateDialogIndirect(g_hInst, pHdr->apRes[iSel], hwndDlg, (DLGPROC)CFG_OnSelChangedProc); 

	ShowWindow(pHdr->hwndDisplay,SW_SHOWNORMAL);
} 
// CFG_OnChildDialogInit - Positions the child dialog box to fall 
//     within the display area of the tab control. 
VOID WINAPI CFG_OnChildDialogInit(HWND hwndDlg) 
{ 
    HWND hwndParent = GetParent(hwndDlg); 
    DLGHDR *pHdr = (DLGHDR *) GetWindowLongPtr( 
        hwndParent, GWLP_USERDATA); 
   
	SetWindowPos(hwndDlg, HWND_TOP, 
        pHdr->rcDisplay.left+165, pHdr->rcDisplay.top+5, 
        400, 370, SWP_NOSIZE); 
} 

HFONT FAR PASCAL CFG_SelectFont( void ) 
{ 
    CHOOSEFONT cf; 
    LOGFONT lf; 
    HFONT hfont; 
 
    // Initialize members of the CHOOSEFONT structure. 
 
    cf.lStructSize = sizeof(CHOOSEFONT); 
    cf.hwndOwner = (HWND)NULL; 
    cf.hDC = (HDC)NULL; 
    cf.lpLogFont = &lf; 
    cf.iPointSize = 0; 
    cf.Flags = CF_SCREENFONTS; 
    cf.rgbColors = RGB(0,0,0); 
    cf.lCustData = 0L; 
    cf.lpfnHook = (LPCFHOOKPROC)NULL; 
    cf.lpTemplateName = (LPSTR)NULL; 
    cf.hInstance = (HINSTANCE) NULL; 
    cf.lpszStyle = (LPSTR)NULL; 
    cf.nFontType = SCREEN_FONTTYPE; 
    cf.nSizeMin = 0; 
    cf.nSizeMax = 0; 
 
    // Display the CHOOSEFONT common-dialog box. 
 
    ChooseFont(&cf); 
 
    // Create a logical font based on the user's 
    // selection and return a handle identifying 
    // that font. 
	lf.lfOutPrecision =0;
	lf.lfCharSet = 0;
	lf.lfClipPrecision =0;
	lf.lfEscapement = 0;
	lf.lfPitchAndFamily = 0;
	lf.lfQuality =0;


    hfont = CreateFontIndirect(cf.lpLogFont); 
    return (hfont); 
} 

HWND g_hwndScrollTrans = NULL;
HWND g_hwndScrollThreads = NULL;
HWND g_hwndSleepSlider = NULL;
int CFG_editexeIdx;
extern _MYTREEITEM g_EditorTI;

LRESULT CALLBACK  CFG_EditInstall_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR szFile[260];
	switch(uMsg)
	{
		case WM_INITDIALOG:
			{
				CFG_editexeIdx = lParam;
				int gameID = CFG_GetGameID(g_currSelCfg);	
				GAME_INSTALLATIONS gi;
				gi = GamesInfoCFG[gameID].vGAME_INST.at(lParam);

				g_cfgScriptTmp.clear();
				g_cfgFilterNameTmp.clear();

				g_EditorTI.sScript = gi.sScript;
				g_EditorTI.sName  = gi.sFilterName;


				CheckDlgButton(hDlg,IDC_CHECK_CONDITION,gi.bActiveScript);			
				SetDlgItemText(hDlg,IDC_EDIT_SCRIPTNAME,gi.sFilterName.c_str());

				SetDlgItemText(hDlg,IDC_EDIT_CFG_PROPNAME,gi.sName.c_str());
				SetDlgItemText(hDlg,IDC_EDIT_PATH,gi.szGAME_PATH.c_str());
				SetDlgItemText(hDlg,IDC_EDIT_CMD,gi.szGAME_CMD.c_str());
		//		SetDlgItemText(hDlg,IDC_EDIT_CFG_MOD,gi.sMod.c_str());
		//		SetDlgItemText(hDlg,IDC_EDIT_CFG_VERSION,gi.sVersion.c_str());
				break;
			}
		case WM_COMMAND:
			{
				switch (LOWORD(wParam))
	            {
					case IDC_BUTTON_EDIT_SCRIPT:
						DialogBoxParam(g_hInst, (LPCTSTR)IDD_DLG_EDIT_FILTER, hDlg, (DLGPROC)FilterEditor_Dlg,-2);
						g_cfgScriptTmp = g_EditorTI.sScript;
						g_cfgFilterNameTmp = g_EditorTI.sName;
						SetDlgItemText(hDlg,IDC_EDIT_SCRIPTNAME,g_cfgFilterNameTmp.c_str());
					break;
					case IDC_BUTTON_CFG_EXE_CANCEL:
						EndDialog(hDlg,0);
						break;
					case IDC_BUTTON_EXE_CFG_OK:
						{
							int gameID = CFG_GetGameID(g_currSelCfg);
							TCHAR szTemp[MAX_PATH*2];
							GAME_INSTALLATIONS gi;
							GetDlgItemText(hDlg,IDC_EDIT_CFG_PROPNAME,szTemp,MAX_PATH);
							if(strlen(szTemp)==0)
							{
								MessageBox(hDlg,g_lang.GetString("NoName"),"Error saving",MB_OK);
								return TRUE;
							}
							if(IsDlgButtonChecked(hDlg,IDC_CHECK_CONDITION)==BST_CHECKED)
								gi.bActiveScript=TRUE;
							else
								gi.bActiveScript=FALSE;

							gi.sFilterName = g_cfgFilterNameTmp;
							gi.sScript = g_cfgScriptTmp;

							gi.sName = szTemp;
							GetDlgItemText(hDlg,IDC_EDIT_PATH,szTemp,MAX_PATH);
							gi.szGAME_PATH = szTemp;
							GetDlgItemText(hDlg,IDC_EDIT_CMD,szTemp,MAX_PATH*2);
							gi.szGAME_CMD = szTemp;
							GamesInfoCFG[gameID].vGAME_INST.at(CFG_editexeIdx) = gi;
							EndDialog(hDlg,0);
						}
						break;

					case IDC_BUTTON_ET_PATH:
					{	
						 
						OPENFILENAME ofn;
						memset(&ofn,0,sizeof(OPENFILENAME));
						ofn.lStructSize = sizeof (OPENFILENAME);
						ofn.hwndOwner = hDlg;
						ofn.lpstrFilter = NULL;
						ofn.lpstrFile = szFile;

						// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
						// use the contents of szFile to initialize itself.
						//
						ofn.lpstrFile[0] = '\0';
						ofn.nMaxFile = sizeof(szFile);
						ofn.lpstrFilter = "All\0*.*\0Executable\0*.exe\0";
						ofn.nFilterIndex = 2;
						ofn.lpstrFileTitle = NULL;
						ofn.nMaxFileTitle = 0;
						ofn.lpstrInitialDir = NULL;
						int gameID=-1;
						gameID = CFG_GetGameID(g_currSelCfg);

						if(gameID!=-1)
						{
							if(GamesInfoCFG[gameID].vGAME_INST.size()>0)
								ofn.lpstrInitialDir = GamesInfoCFG[gameID].vGAME_INST[CFG_editexeIdx].szGAME_PATH.c_str();
						}
						ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

						if(GetOpenFileName(&ofn))
							SetDlgItemText(hDlg,IDC_EDIT_PATH,ofn.lpstrFile);

						return TRUE;					
					}

				}
				break;
			}

	}
	return FALSE;
}



LRESULT CALLBACK  CFG_AddNewInstall_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR szFile[360];
	switch(uMsg)
	{
		case WM_INITDIALOG:
			{
				g_cfgScriptTmp.clear();
				g_cfgFilterNameTmp.clear();
				SetWindowText(hDlg,g_lang.GetString("TitleNewInstall"));
				SetDlgItemText(hDlg,IDC_BUTTON_EXE_CFG_OK,g_lang.GetString("Save"));
				SetDlgItemText(hDlg,IDC_BUTTON_CFG_EXE_CANCEL,g_lang.GetString("Cancel"));
				SetDlgItemText(hDlg,IDC_STATIC_ADDNEWINSTALL,g_lang.GetString("AddNewInstall"));
				SetDlgItemText(hDlg,IDC_STATIC_EXE,g_lang.GetString("ColumnExePath"));
				SetDlgItemText(hDlg,IDC_STATIC_COMMAND,g_lang.GetString("ColumnCommand"));
				SetDlgItemText(hDlg,IDC_EDIT_CFG_PROPNAME,"Default");
				int gameID = CFG_GetGameID(g_currSelCfg);
				if(gm.GamesInfo[gameID].vGAME_INST_DEFAULT.size()>0)
				{
					SetDlgItemText(hDlg,IDC_EDIT_CMD,gm.GamesInfo[gameID].vGAME_INST_DEFAULT.at(0).szGAME_CMD.c_str());
				}
				
				break;
			}
		case WM_COMMAND:
			{
				switch (LOWORD(wParam))
	            {
					case IDC_BUTTON_EDIT_SCRIPT:
						DialogBoxParam(g_hInst, (LPCTSTR)IDD_DLG_EDIT_FILTER, hDlg, (DLGPROC)FilterEditor_Dlg,-1);
						g_cfgScriptTmp = g_EditorTI.sScript;
						g_cfgFilterNameTmp = g_EditorTI.sName;
						SetDlgItemText(hDlg,IDC_EDIT_SCRIPTNAME,g_cfgFilterNameTmp.c_str());
					break;
					case IDC_BUTTON_CFG_EXE_CANCEL:
						EndDialog(hDlg,0);
						break;
					case IDC_BUTTON_EXE_CFG_OK:
						{
							int gameID = CFG_GetGameID(g_currSelCfg);
							TCHAR szTemp[MAX_PATH*2];
							GAME_INSTALLATIONS gi;
							GetDlgItemText(hDlg,IDC_EDIT_CFG_PROPNAME,szTemp,MAX_PATH);
							if(strlen(szTemp)==0)
							{
								MessageBox(hDlg,g_lang.GetString("NoName"),"Error saving",MB_OK);
								return TRUE;
							}
							if(IsDlgButtonChecked(hDlg,IDC_CHECK_CONDITION)==BST_CHECKED)
								gi.bActiveScript=TRUE;
							else
								gi.bActiveScript=FALSE;

							gi.sFilterName = g_cfgFilterNameTmp;
							gi.sScript = g_cfgScriptTmp;
							gi.sName = szTemp;
							GetDlgItemText(hDlg,IDC_EDIT_PATH,szTemp,MAX_PATH*2);
							gi.szGAME_PATH = szTemp;
							GetDlgItemText(hDlg,IDC_EDIT_CMD,szTemp,MAX_PATH*2);
							gi.szGAME_CMD = szTemp;

//USE below function for above code!!!
//GetDlgItemTextToString(HWND hWnd,int nID, string &pOutString)
							GamesInfoCFG[gameID].vGAME_INST.push_back(gi);

							EndDialog(hDlg,0);
						}
						break;

					case IDC_BUTTON_ET_PATH:
					{	
						 
						OPENFILENAME ofn;
						memset(&ofn,0,sizeof(OPENFILENAME));
						ofn.lStructSize = sizeof (OPENFILENAME);
						ofn.hwndOwner = hDlg;
						ofn.lpstrFilter = NULL;
						ofn.lpstrFile = szFile;

						// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
						// use the contents of szFile to initialize itself.
						//
						ofn.lpstrFile[0] = '\0';
						ofn.nMaxFile = sizeof(szFile);
						ofn.lpstrFilter = "All\0*.*\0Executable\0*.exe\0";
						ofn.nFilterIndex = 2;
						ofn.lpstrFileTitle = NULL;
						ofn.nMaxFileTitle = 0;
						ofn.lpstrInitialDir = NULL;
						int gameID=-1;
						gameID = CFG_GetGameID(g_currSelCfg);

						if(gameID!=-1)
						{
								if(GamesInfoCFG[gameID].vGAME_INST.size()>0)
									ofn.lpstrInitialDir = GamesInfoCFG[gameID].vGAME_INST[CFG_editexeIdx].szGAME_PATH.c_str();
						}
						ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

						if(GetOpenFileName(&ofn))
							SetDlgItemText(hDlg,IDC_EDIT_PATH,ofn.lpstrFile);

						return TRUE;					
					}

				}
				break;
			}

	}
	return FALSE;
}

void CFG_Enumerate_installations(HWND hDlg,int GameId)
{
	HWND hwndLVexes;
	hwndLVexes = GetDlgItem(hDlg,IDC_LIST_CFG_EXES);

	LVITEM lvItem;
	ZeroMemory(&lvItem, sizeof(LVITEM));

	vGAME_INSTALLS::reverse_iterator  iLst;
	lvItem.mask = LVIF_TEXT ;	
	lvItem.iSubItem = 0;

	ListView_DeleteAllItems(hwndLVexes);

	for ( iLst = GamesInfoCFG[GameId].vGAME_INST.rbegin(); iLst != GamesInfoCFG[GameId].vGAME_INST.rend( ); iLst++ )
	{
	
		GAME_INSTALLATIONS gi = *iLst;
		if(gi.sName.length()>0)
			lvItem.pszText = (LPSTR)gi.sName.c_str();
		else
			lvItem.pszText = "Default";

		ListView_InsertItem( hwndLVexes,&lvItem);
		ListView_SetItemText(hwndLVexes,lvItem.iItem,1,(LPSTR)gi.sFilterName.c_str());
		ListView_SetItemText(hwndLVexes,lvItem.iItem,2,(LPSTR)gi.szGAME_PATH.c_str());
		ListView_SetItemText(hwndLVexes,lvItem.iItem,3,(LPSTR)gi.szGAME_CMD.c_str());

	}

}

LRESULT CALLBACK CFG_OnSelChangedProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR szText[256];
	TCHAR tmp[20];
	TCHAR szFile[260];
	
	hwndConfDialog = hDlg;
	switch(uMsg)
    {
		case WM_INITDIALOG:
			{
			
				g_bChanged = false;
				CFG_OnChildDialogInit(hDlg);
		 		
				SendDlgItemMessage (hDlg,IDC_COMBO_MOD, CB_ADDSTRING, 0, (LPARAM)"ALT");  
				SendDlgItemMessage (hDlg,IDC_COMBO_MOD, CB_ADDSTRING, 0 ,(LPARAM)"CONTROL");  
				SendDlgItemMessage (hDlg,IDC_COMBO_MOD, CB_ADDSTRING, 0, (LPARAM)"SHIFT");  
			
				if(AppCFGtemp.dwMinimizeMODKey == MOD_ALT)
					SendDlgItemMessage (hDlg,IDC_COMBO_MOD, CB_SETCURSEL, 0, 0);  
				else if(AppCFGtemp.dwMinimizeMODKey == MOD_CONTROL)
					SendDlgItemMessage (hDlg,IDC_COMBO_MOD, CB_SETCURSEL, 1, 0);  
				else if(AppCFGtemp.dwMinimizeMODKey == MOD_SHIFT)
					SendDlgItemMessage (hDlg,IDC_COMBO_MOD, CB_SETCURSEL, 2, 0);  

				sprintf(tmp,"%c",AppCFGtemp.cMinimizeKey);
				SetDlgItemText(hDlg,IDC_EDIT_KEY,tmp);
				SendDlgItemMessage (hDlg,IDC_EDIT_KEY, EM_SETLIMITTEXT,1, 0);  

		
				CheckDlgButton(hDlg,IDC_CHECK_CONNECT_CLOSE,AppCFGtemp.bCloseOnConnect);
				CheckDlgButton(hDlg,IDC_CHECK1,AppCFGtemp.bAutostart);
				CheckDlgButton(hDlg,IDC_CHECK_SHORTNAME,AppCFGtemp.bUseShortCountry);
				CheckDlgButton(hDlg,IDC_CHECK_REG_WEB_PROTOCOLS,AppCFGtemp.bRegisterWebProtocols);
				CheckDlgButton(hDlg,IDC_CHECK_NO_MAP_RESIZE,AppCFGtemp.bNoMapResize);
		

				LVCOLUMN lvColumn;
				//initialize the columns
				lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM ;
				lvColumn.fmt = LVCFMT_LEFT;

				HWND hwndLVexes;
				hwndLVexes = GetDlgItem(hDlg,IDC_LIST_CFG_EXES);

				lvColumn.cx = 80;
				lvColumn.pszText = (LPSTR)g_lang.GetString("ColumnName");
				ListView_InsertColumn(hwndLVexes, 0, &lvColumn);
				lvColumn.cx = 120;
				lvColumn.pszText = (LPSTR)g_lang.GetString("LaunchByScript");
				ListView_InsertColumn(hwndLVexes, 1, &lvColumn);
				lvColumn.cx = 180;
				lvColumn.pszText =  (LPSTR)g_lang.GetString("ColumnExePath"); 
				ListView_InsertColumn(hwndLVexes, 3, &lvColumn);
				lvColumn.pszText =  (LPSTR)g_lang.GetString("ColumnCommand"); 
				ListView_InsertColumn(hwndLVexes, 4, &lvColumn);

				DWORD dwExStyle=0;
				dwExStyle = ListView_GetExtendedListViewStyle(hwndLVexes);
				dwExStyle |= LVS_EX_FULLROWSELECT |  LVS_EX_LABELTIP ;
				ListView_SetExtendedListViewStyle(hwndLVexes,dwExStyle);			

			
			CheckDlgButton(hDlg,IDC_CHECK2,AppCFGtemp.bUse_minimize);
			CheckDlgButton(hDlg,IDC_CHECK_SCR_RESTORE,AppCFGtemp.bUSE_SCREEN_RESTORE );				

			
			sprintf(szText,"%d",(AppCFGtemp.socktimeout.tv_sec*1000)+AppCFGtemp.socktimeout.tv_usec);
			SetDlgItemText(hDlg,IDC_EDIT_SOCK_TIMEOUT_S,szText);
			
			sprintf(szText,"%d",AppCFGtemp.dwRetries);
			SetDlgItemText(hDlg,IDC_EDIT_CFG_RETRIES,szText);


			CheckDlgButton(hDlg,IDC_CHECK_XMPP_ACTIVE,AppCFGtemp.bXMPP_Active);
			SetDlgItemText(hDlg,IDC_EDIT_XMPP_USERNAME,AppCFGtemp.szXMPP_USERNAME);
			SetDlgItemText(hDlg,IDC_EDIT_XMPP_PASSWORD,AppCFGtemp.szXMPP_PASSWORD);
			SetDlgItemText(hDlg,IDC_EDIT_XMPP_SERVER,AppCFGtemp.szXMPP_SERVER);
			

			CheckDlgButton(hDlg,IDC_CHECK_BUDDY_NOTIFY,AppCFGtemp.bBuddyNotify);
			CheckDlgButton(hDlg,IDC_CHECK_USE_WAV_FILE,AppCFGtemp.bUseBuddySndNotify);
			SetDlgItemText(hDlg,IDC_EDIT_WAV_FILE,AppCFGtemp.szNotifySoundWAVfile);						

			SetDlgItemText(hDlg,IDC_EDIT_MIRC,g_sMIRCoutput.c_str());
					
			CheckDlgButton(hDlg,IDC_CHECK_EXT_ACTIVE,AppCFGtemp.bUse_EXT_APP);
			CheckDlgButton(hDlg,IDC_CHECK_EXT_ACTIVE2,AppCFGtemp.bUse_EXT_APP2);
			CheckDlgButton(hDlg,IDC_CHECK_MIRC,AppCFGtemp.bUseMIRC);
	

		   g_lang.EnumerateLanguage();
		   for( map<string,string>::iterator ii=g_lang.m_Languages.begin(); ii!=g_lang.m_Languages.end(); ++ii)
		   {			   
			   const TCHAR *psz = (*ii).first.c_str() ;
			   SendMessage(GetDlgItem(hDlg,IDC_COMBO_LANG),   (UINT) CB_ADDSTRING, 0, (LPARAM)psz );  			
		   }
		   
		   SendMessage(GetDlgItem(hDlg,IDC_COMBO_LANG),   (UINT) CB_SELECTSTRING, 0, (LPARAM)g_lang.m_strCurrentLang.c_str() );  			
		
			int gameID=-1;
			gameID = CFG_GetGameID(g_currSelCfg);
			if(gameID!=-1)
			{
				sprintf(szText,g_lang.GetString("CFGSettingsTitle"),GamesInfoCFG[gameID].szGAME_NAME);
				SetWindowText(GetDlgItem(hDlg,IDC_STATIC_GAME_NAME),szText);
				
				TCHAR szTempMaster[200];
				if(GamesInfoCFG[gameID].bUseHTTPServerList==FALSE)
					sprintf(szTempMaster,"%s:%d",GamesInfoCFG[gameID].szMasterServerIP[0],GamesInfoCFG[gameID].dwMasterServerPORT);
				else
					sprintf(szTempMaster,"%s",GamesInfoCFG[gameID].szMasterServerIP[0]);
	
				SetDlgItemText(hDlg,IDC_EDIT_MASTER_SERVER,szTempMaster);//GamesInfoCFG[gameID].szMasterServerIP[0]);

			
				CheckDlgButton(hDlg,IDC_CHECK_USE_HTTP,GamesInfoCFG[gameID].bUseHTTPServerList[0]);								
				CheckDlgButton(hDlg,IDC_CHECK_ACTIVE,GamesInfoCFG[gameID].bActive);

				sprintf(szText,"Example web protocol: %s://10.0.0.1:%d",gm.GamesInfo[gameID].szWebProtocolName,gm.GamesInfo[gameID].dwDefaultPort);
				SetDlgItemText(hDlg,IDC_STATIC_WEB_PROTOCOL,szText);


				CFG_Enumerate_installations(hDlg,gameID);
			}
			SetDlgItemText(hDlg,IDC_BUTTON_ADD_INSTALL,g_lang.GetString("CFGAddInstall"));
			SetDlgItemText(hDlg,IDC_BUTTON_EDIT_INSTALL,g_lang.GetString("CFGEditInstall"));
			SetDlgItemText(hDlg,IDC_BUTTON_DELETE_INSTALL,g_lang.GetString("CFGDeleteInstall"));

			SetDlgItemText(hDlg,IDC_EDIT_EXT_EXE,AppCFGtemp.szEXT_EXE_PATH);
			SetDlgItemText(hDlg,IDC_EDIT_EXT_CMD,AppCFGtemp.szEXT_EXE_CMD);
			SetDlgItemText(hDlg,IDC_EDIT_EXT_WINDOWNAME,AppCFGtemp.szEXT_EXE_WINDOWNAME);
			SetDlgItemText(hDlg,IDC_EDIT_WINDOWNAME,AppCFGtemp.szET_WindowName);

			SetDlgItemText(hDlg,IDC_EDIT_EXT_EXE2,AppCFGtemp.szOnReturn_EXE_PATH);
			SetDlgItemText(hDlg,IDC_EDIT_EXT_CMD2,AppCFGtemp.szOnReturn_EXE_CMD);


			SetDlgItemText(hDlg,IDC_STATIC_INSTALL_PATH,EXE_PATH);		

			g_hwndScrollTrans = GetDlgItem(hDlg,IDC_SLIDER_TRANS);
			SendMessage(g_hwndScrollTrans,TBM_SETRANGE,TRUE,(LPARAM)MAKELONG(MIN_TRANSPARANCY, 100)) ; //AppCFG.g_cTransparancy);
			SendMessage(g_hwndScrollTrans,TBM_SETPOS,TRUE,(LPARAM)AppCFGtemp.g_cTransparancy ) ; //AppCFG.g_cTransparancy);
			SetDlgTrans(hDlg,AppCFGtemp.g_cTransparancy);



			 g_hwndSleepSlider =  GetDlgItem(hDlg,IDC_SLIDER_SLEEP_VAL);
			SendMessage(g_hwndSleepSlider,TBM_SETRANGE,TRUE,(LPARAM)MAKELONG(0, 2000)) ;
			SendMessage(g_hwndSleepSlider,TBM_SETPOS,TRUE,(LPARAM)AppCFGtemp.dwSleep) ; 
			TCHAR sztemp[100];
			sprintf(sztemp,"Sleep time is %d ms",AppCFGtemp.dwSleep);
			SetDlgItemText(hDlg,IDC_STATIC_SLEEP_VAL,sztemp);


			g_hwndScrollThreads =  GetDlgItem(hDlg,IDC_SLIDER_THREADS);
			SendMessage( g_hwndScrollThreads,TBM_SETRANGE,TRUE,(LPARAM)MAKELONG(1, 256)) ;
			SendMessage(g_hwndScrollThreads,TBM_SETPOS,TRUE,(LPARAM)AppCFGtemp.dwThreads) ; 

				
			SetDlgItemText(hDlg,IDC_STATIC_THREAD,_itoa(AppCFGtemp.dwThreads ,sztemp,10));
			break;
		}
		case WM_VSCROLL:
		{
			if(lParam==(LPARAM)g_hwndScrollTrans)
			{
				int i = (TCHAR)SendMessage(GetDlgItem(hDlg,IDC_SLIDER_TRANS),TBM_GETPOS,0,(LPARAM)0) ; 
				SetDlgTrans(GetParent(hDlg),i);
			} else if (lParam==(LPARAM)g_hwndScrollThreads)
			{
				DWORD i = (DWORD)SendMessage(GetDlgItem(hDlg,IDC_SLIDER_THREADS),TBM_GETPOS,0,(LPARAM)0) ; 
				TCHAR sztemp[20];
				AppCFGtemp.dwThreads = i;
				SetDlgItemText(hDlg,IDC_STATIC_THREAD,_itoa(i,sztemp,10));
			}	
			else if (lParam==(LPARAM)g_hwndSleepSlider)
			{
				DWORD i = (DWORD)SendMessage(GetDlgItem(hDlg,IDC_SLIDER_SLEEP_VAL),TBM_GETPOS,0,(LPARAM)0) ; 
				TCHAR sztemp[100];
				AppCFG.dwSleep = i;
				AppCFGtemp.dwSleep = i;
				sprintf(sztemp,"Sleep time is %d ms",i);
				SetDlgItemText(hDlg,IDC_STATIC_SLEEP_VAL,sztemp);
			}
			return FALSE;
		}
		case WM_COMMAND:

			switch(HIWORD(wParam))		
			{
				case CBN_SELCHANGE:
					{
						if(LOWORD(wParam)==IDC_COMBO_LANG)
						{							
							TCHAR sztemp[200];
							int idx=   SendMessage(GetDlgItem(hDlg,IDC_COMBO_LANG),   (UINT) CB_GETCURSEL, 0, (LPARAM)0 );  
							SendMessage(GetDlgItem(hDlg,IDC_COMBO_LANG),   (UINT) CB_GETLBTEXT, idx, (LPARAM)sztemp );
							strcpy(AppCFGtemp.szLanguageFilename,g_lang.m_Languages[sztemp].c_str());
							g_lang.loadFile(g_lang.m_Languages[sztemp].c_str());
							MessageBox(NULL,g_lang.GetString("AlertRestartRequired"),"Alert!",MB_OK);							 
						}
					}
					break;

				case CBN_CLOSEUP :
					g_bChanged = true;
				
					break;
				case BN_CLICKED:					
					break;
				case EN_CHANGE:
				{
					g_bChanged = true;								
				}
			}
			
			switch (LOWORD(wParam))
            {
				case IDC_CHECK_USE_HTTP:
					g_bChanged = true;
					break;
				case IDC_BUTTON_SEL_FONT:
					HFONT hf;
					hf = CFG_SelectFont();
					DeleteObject(g_hf);				
					g_hf = hf;
					ChangeFont(g_hWnd, g_hf);
					break;
				case IDCANCEL:	
					EndDialog(hDlg, 0);
				break;
				case IDC_BUTTON_ADD_INSTALL:
					{
						DialogBox(g_hInst, (LPCTSTR)IDD_ADD_NEW_GAME_INSTALLATION, hDlg, (DLGPROC)CFG_AddNewInstall_Proc);				
						int gameID = CFG_GetGameID(g_currSelCfg);			
						CFG_Enumerate_installations(hDlg,gameID);
					}
					break;
				case IDC_BUTTON_EDIT_INSTALL:
					{
						int n;
						n = ListView_GetSelectionMark(GetDlgItem(hDlg,IDC_LIST_CFG_EXES));
						if(n!=-1)
						{
							DialogBoxParam(g_hInst, (LPCTSTR)IDD_ADD_NEW_GAME_INSTALLATION, hDlg, (DLGPROC)CFG_EditInstall_Proc,n);				
							int gameID = CFG_GetGameID(g_currSelCfg);			
							CFG_Enumerate_installations(hDlg,gameID);
						}
					}
					break;
				case IDC_BUTTON_DELETE_INSTALL:
					{
						int n;
						n = ListView_GetSelectionMark(GetDlgItem(hDlg,IDC_LIST_CFG_EXES));
						if(n!=-1)
						{
									
							int gameID = CFG_GetGameID(g_currSelCfg);	
							if(gameID!=-1)
							{
								GamesInfoCFG[gameID].vGAME_INST.erase(GamesInfoCFG[gameID].vGAME_INST.begin()+n);
								CFG_Enumerate_installations(hDlg,gameID);
							}
						}
					}
					break;

				case IDC_EXT_EXE:
				{
			
					OPENFILENAME ofn;
					memset(&ofn,0,sizeof(OPENFILENAME));
					ofn.lStructSize = sizeof (OPENFILENAME);
					ofn.hwndOwner = hDlg;
					ofn.lpstrFilter = NULL;
					ofn.lpstrFile = szFile;
					ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = sizeof(szFile);
					ofn.lpstrFilter = "All\0*.*\0External exe\0*.exe\0";
					ofn.nFilterIndex = 2;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.lpstrInitialDir = AppCFGtemp.szEXT_EXE_PATH;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

					if(GetOpenFileName(&ofn))
						SetDlgItemText(hDlg,IDC_EDIT_EXT_EXE,ofn.lpstrFile);

					return TRUE;
				}
	
				case IDC_EXT_EXE2:
				{
			
					OPENFILENAME ofn;
					memset(&ofn,0,sizeof(OPENFILENAME));
					ofn.lStructSize = sizeof (OPENFILENAME);
					ofn.hwndOwner = hDlg;
					ofn.lpstrFilter = NULL;
					ofn.lpstrFile = szFile;
					ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = sizeof(szFile);
					ofn.lpstrFilter = "All\0*.*\0External exe\0*.exe\0";
					ofn.nFilterIndex = 2;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.lpstrInitialDir = AppCFGtemp.szOnReturn_EXE_PATH;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

					if(GetOpenFileName(&ofn))
						SetDlgItemText(hDlg,IDC_EDIT_EXT_EXE2,ofn.lpstrFile);

					return TRUE;
				}

				case IDC_BUTTON_WAV_FILE:
				{
			
					OPENFILENAME ofn;
					memset(&ofn,0,sizeof(OPENFILENAME));
					ofn.lStructSize = sizeof (OPENFILENAME);
					ofn.hwndOwner = hDlg;
					ofn.lpstrFilter = NULL;
					ofn.lpstrFile = szFile;
					ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = sizeof(szFile);
					ofn.lpstrFilter = "All\0*.*\0Wav\0*.wav\0";
					ofn.nFilterIndex = 2;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.lpstrInitialDir = AppCFGtemp.szNotifySoundWAVfile;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

					if(GetOpenFileName(&ofn))
						SetDlgItemText(hDlg,IDC_EDIT_WAV_FILE,ofn.lpstrFile);

					return TRUE;
				}
			}
			break;
	}

	return FALSE;
}
