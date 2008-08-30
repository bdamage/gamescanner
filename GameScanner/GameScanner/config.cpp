
#include "stdafx.h"
#include "config.h"
#include "utilz.h"


#pragma warning(disable : 4995)
#pragma warning(disable : 4996)

extern void Default_GameSettings();
extern void Default_Appsettings();
extern DWORD WINAPI CFG_Save(LPVOID lpVoid);
extern string g_sMIRCoutput;
extern HFONT g_hf;
string CFG_g_sMIRCoutputTemp;
extern void ChangeFont(HWND hWnd, HFONT hf);
extern HWND g_hWnd;

//Config dialog vars
#define GAME_CFG_INDEX 5   //This is the helper index in treeview where all games configuration starts from
#define C_PAGES 18
bool g_bChanged=false;
APP_SETTINGS_NEW AppCFGtemp;
HWND g_hwndTree=NULL;

DWORD g_tvIndexCFG=0;
extern HIMAGELIST g_hImageListIcons;
extern APP_SETTINGS_NEW AppCFG;
extern GamesMap GamesInfo;
GamesMap GamesInfoCFG;
///extern GAME_INFO GamesInfo[MAX_SERVERLIST+1];
//GAME_INFO GamesInfoCFG[MAX_SERVERLIST+1];

extern HINSTANCE g_hInst;
extern char EXE_PATH[_MAX_PATH+_MAX_FNAME];			//Don't write anything to this path
extern char USER_SAVE_PATH[_MAX_PATH+_MAX_FNAME];     //Path to save settings and server lists
extern char COMMON_SAVE_PATH[_MAX_PATH+_MAX_FNAME];   //Used for downloading app update and preview images - purpose to share the same data between users.
extern char EXE_PATH_OLD[_MAX_PATH+_MAX_FNAME];
extern CLanguage lang;

typedef struct tag_dlghdr { 
    HWND hwndTab;       // tab control 
    HWND hwndDisplay;   // current child dialog box 
    RECT rcDisplay;     // display rectangle for the tab control 
    DLGTEMPLATE *apRes[GAME_CFG_INDEX+MAX_SERVERLIST]; 
} DLGHDR; 

DLGHDR *g_pHdr = NULL;
int g_currSelCfg=-2;
HWND hwndConfDialog=NULL;

void CFG_Apply_General(HWND hDlg)
{
	if(IsDlgButtonChecked(hDlg,IDC_CHECK1)==BST_CHECKED)
		AppCFGtemp.bAutostart=true;
	else
		AppCFGtemp.bAutostart=false;

	if(IsDlgButtonChecked(hDlg,IDC_CHECK_SHORTNAME)==BST_CHECKED)
		AppCFGtemp.bUseShortCountry=TRUE;
	else
		AppCFGtemp.bUseShortCountry=FALSE;

	if(IsDlgButtonChecked(hDlg,IDC_CHECK_MIRC)==BST_CHECKED)
		AppCFGtemp.bUseMIRC=true;
	else
		AppCFGtemp.bUseMIRC=false;

	 char tmp[MAX_PATH];	 
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
			GamesInfoCFG[gameID].bActive=true;
		else
			GamesInfoCFG[gameID].bActive=false;

		if(IsDlgButtonChecked(hDlg,IDC_CHECK_USE_HTTP)==BST_CHECKED)
			GamesInfoCFG[gameID].bUseHTTPServerList = TRUE;
		else
			GamesInfoCFG[gameID].bUseHTTPServerList = FALSE;

		GetDlgItemText(hDlg,IDC_EDIT_MASTER_SERVER,GamesInfoCFG[gameID].szMasterServerIP,MAX_PATH);						
		char szTmp[10];
		GetDlgItemText(hDlg,IDC_EDIT_MASTER_PORT,szTmp,sizeof(szTmp));
		GamesInfoCFG[gameID].dwMasterServerPORT = atoi(szTmp);
		GetDlgItemText(hDlg,IDC_EDIT_PROTOCOL,szTmp,sizeof(szTmp));
		GamesInfoCFG[gameID].dwProtocol= atoi(szTmp);
	}
}

void CFG_Apply_Network(HWND hDlg)
{

	g_bChanged = false;
							
	char szTmp[10];
	GetDlgItemText(hDlg,IDC_EDIT_SOCK_TIMEOUT_S,szTmp,sizeof(szTmp));
	
	AppCFGtemp.socktimeout.tv_sec = atoi(szTmp)/1000;
	AppCFGtemp.socktimeout.tv_usec = atoi(szTmp) - (AppCFGtemp.socktimeout.tv_sec*1000);

	AppCFGtemp.dwThreads = (DWORD)SendMessage(GetDlgItem(hDlg,IDC_SLIDER_THREADS),TBM_GETPOS,0,(LPARAM)0) ; 

	GetDlgItemText(hDlg,IDC_EDIT_CFG_RETRIES,szTmp,sizeof(szTmp));
	AppCFGtemp.dwRetries = atoi(szTmp);
				

}

void CFG_Apply_Ext(HWND hDlg)
{
	g_bChanged = false;
	if(IsDlgButtonChecked(hDlg,IDC_CHECK_EXT_ACTIVE)==BST_CHECKED)
		AppCFGtemp.bUse_EXT_APP =true;
	else
		AppCFGtemp.bUse_EXT_APP=false;

	GetDlgItemText(hDlg,IDC_EDIT_EXT_EXE,AppCFGtemp.szEXT_EXE_PATH,MAX_PATH);
	GetDlgItemText(hDlg,IDC_EDIT_EXT_CMD,AppCFGtemp.szEXT_EXE_CMD,MAX_PATH);
	GetDlgItemText(hDlg,IDC_EDIT_EXT_WINDOWNAME,AppCFGtemp.szEXT_EXE_WINDOWNAME,MAX_PATH);
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
	char tmp[20];
	GetDlgItemText(hDlg,IDC_EDIT_KEY,tmp,2);
	AppCFGtemp.cMinimizeKey = (char)toupper((char)tmp[0]);
	
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
	AppCFGtemp.g_cTransparancy = (char)SendMessage(GetDlgItem(hDlg,IDC_SLIDER_TRANS),TBM_GETPOS,0,(LPARAM)0) ; 
}

DWORD CFG_GetGameID(int selectionIndex)
{
	
	if(selectionIndex>GAME_CFG_INDEX)
	{
		return selectionIndex - (GAME_CFG_INDEX +1);
	}
	return 0xFFFFFFFF;
}

HTREEITEM TreeView_AddItem(int iImage, const char *text)
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

LRESULT CALLBACK CFG_MainProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{
		
			g_tvIndexCFG = 0;
			g_hwndTree = GetDlgItem(hDlg,IDC_TREE_CONF);

			SendMessage(g_hwndTree, TVM_SETIMAGELIST , TVSIL_NORMAL, (LPARAM)g_hImageListIcons);
			HTREEITEM hNewItem;
			hNewItem = TreeView_AddItem(34,lang.GetString("ConfigGeneral"));
			hNewItem = TreeView_AddItem(17,lang.GetString("ConfigMinimizer"));
			hNewItem = TreeView_AddItem(16,lang.GetString("ConfigExtExe"));
			hNewItem = TreeView_AddItem(36,lang.GetString("ConfigGraphic"));
			hNewItem = TreeView_AddItem(13,lang.GetString("ConfigNetwork"));
			hNewItem = TreeView_AddItem(15 ,lang.GetString("ConfigGames"));
			if (hNewItem)
				TreeView_Select(g_hwndTree, hNewItem, TVGN_CARET);

			for(int i=0;i<MAX_SERVERLIST;i++)
				hNewItem = TreeView_AddItem(GamesInfo[i].iIconIndex,GamesInfo[i].szGAME_SHORTNAME);

			TreeView_Select(g_hwndTree, NULL, TVGN_CARET);
				
			GamesInfoCFG = GamesInfo;
			memcpy(&AppCFGtemp,&AppCFG,sizeof(APP_SETTINGS_NEW));
		//	memcpy(&GamesInfoCFG,&GI,sizeof(GAME_INFO)*MAX_SERVERLIST);
			CFG_g_sMIRCoutputTemp = g_sMIRCoutput;
			SetDlgItemText(hDlg,IDOK,lang.GetString("Ok"));
			SetDlgItemText(hDlg,IDC_BUTTON_DEFAULT,lang.GetString("SetDefault"));
			SetDlgItemText(hDlg,IDCANCEL,lang.GetString("Cancel"));
			
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
						{

							int gameID=-1;
							switch(g_currSelCfg)
							{
								case 0: CFG_Apply_General(hwndConfDialog); break;
								case 1: CFG_Apply_Minimizer(hwndConfDialog); break;
								case 2: CFG_Apply_Ext(hwndConfDialog); break;
								case 3: CFG_Apply_Look(hwndConfDialog); break;
								case 4: CFG_Apply_Network(hwndConfDialog); break;

								default:
										gameID = CFG_GetGameID(g_currSelCfg);
									break;

							}
							if(gameID!=-1)
								CFG_Apply_Games(gameID,hwndConfDialog);
						} 
						CFG_OnSelChanged(hDlg) ;

							
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
					int gameID=-1;
					switch(g_currSelCfg)
					{
						case 0: CFG_Apply_General(hwndConfDialog); break;
						case 1: CFG_Apply_Minimizer(hwndConfDialog); break;
						case 2: CFG_Apply_Ext(hwndConfDialog); break;
						case 3: CFG_Apply_Look(hwndConfDialog); break;
						case 4: CFG_Apply_Network(hwndConfDialog); break;
						default:
								gameID = CFG_GetGameID(g_currSelCfg);

					}
					if(gameID!=-1)
						CFG_Apply_Games(gameID,hwndConfDialog);
					
					if(AppCFGtemp.bAutostart)
						SelfInstall(EXE_PATH);
					else
						UnInstall();
					
					if(AppCFGtemp.bUse_minimize)
					{	
						//removed since 2.2.0 beta added back since 3.0.1
						UnregisterHotKey(NULL, HOTKEY_ID);
						if (!RegisterHotKey(NULL, HOTKEY_ID, AppCFGtemp.dwMinimizeMODKey ,AppCFGtemp.cMinimizeKey))
						{
							//probably already registred
							MessageBox(NULL,lang.GetString("ErrorRegHotkey"),"Hotkey error",NULL);
						}
					}else
					{
						UnregisterHotKey(NULL, HOTKEY_ID);
					}
						
					GamesInfo = GamesInfoCFG;
					memcpy(&AppCFG,&AppCFGtemp,sizeof(APP_SETTINGS_NEW));
					//memcpy(&GI,&GamesInfoCFG,sizeof(GAME_INFO)*MAX_SERVERLIST);
				//	ZeroMemory(&GamesInfoCFG,sizeof(GAME_INFO)*MAX_SERVERLIST);
					ZeroMemory(&AppCFGtemp,sizeof(APP_SETTINGS_NEW));
					/*HANDLE hThread=NULL; 
					DWORD dwThreadIdBrowser=0;				
					hThread = CreateThread( NULL, 0, &CFG_Save,(LPVOID)0 ,0, &dwThreadIdBrowser);                
					if (hThread == NULL) 
					{
						AddLogInfo(ETSV_WARNING, "CreateThread CFG_Save failed (%d)\n", GetLastError() ); 
					} else
					{
						CloseHandle( hThread );
					}*/

					CFG_Save(0);
					LocalFree(g_pHdr);
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}
				case IDC_BUTTON_DEFAULT:
				{				
					Default_Appsettings();
					Default_GameSettings();
					memcpy(&AppCFGtemp,&AppCFG,sizeof(APP_SETTINGS_NEW));
					//memcpy(&GamesInfoCFG,&GI,sizeof(GAME_INFO)*MAX_SERVERLIST);
					GamesInfoCFG = GamesInfo;
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
    SetWindowLong(hwndDlg, GWLP_USERDATA, (LONG) g_pHdr); 
 
	g_pHdr->hwndTab = GetDlgItem(hwndDlg,IDC_TREE_CONF); 
 
    // Lock the resources for the three child dialog boxes. 
    g_pHdr->apRes[0] = DoLockDlgRes(MAKEINTRESOURCE(IDD_CFG1)); 
    g_pHdr->apRes[1] = DoLockDlgRes(MAKEINTRESOURCE(IDD_CFG2)); 
	g_pHdr->apRes[2] = DoLockDlgRes(MAKEINTRESOURCE(IDD_CFG4)); 
	g_pHdr->apRes[3] = DoLockDlgRes(MAKEINTRESOURCE(IDD_CFG5)); 
	g_pHdr->apRes[4] = DoLockDlgRes(MAKEINTRESOURCE(IDD_CFG_NET)); 
	g_pHdr->apRes[5] = DoLockDlgRes(MAKEINTRESOURCE(IDD_CFG3_NEW)); 
	
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

	int iSel = (int)tvitem.lParam;

	if(g_currSelCfg==iSel)
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
 
	if(iSel>=GAME_CFG_INDEX+MAX_SERVERLIST) //C_PAGES
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
	lf.lfCharSet =0;
	lf.lfClipPrecision =0;
	lf.lfEscapement = 0;
	lf.lfPitchAndFamily = 0;
	lf.lfQuality =0;


    hfont = CreateFontIndirect(cf.lpLogFont); 
    return (hfont); 
} 

HWND g_hwndScrollTrans;
HWND g_hwndScrollThreads;
int CFG_editexeIdx;

LRESULT CALLBACK  CFG_EditInstall_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char szFile[260];
	switch(uMsg)
	{
		case WM_INITDIALOG:
			{
				CFG_editexeIdx = lParam;
				int gameID = CFG_GetGameID(g_currSelCfg);	
				GAME_INSTALLATIONS gi;
				gi = GamesInfoCFG[gameID].pSC->vGAME_INST.at(lParam);

				SetDlgItemText(hDlg,IDC_EDIT_CFG_PROPNAME,gi.sName.c_str());
				SetDlgItemText(hDlg,IDC_EDIT_PATH,gi.szGAME_PATH.c_str());
				SetDlgItemText(hDlg,IDC_EDIT_CMD,gi.szGAME_CMD.c_str());
				SetDlgItemText(hDlg,IDC_EDIT_CFG_MOD,gi.sMod.c_str());
				SetDlgItemText(hDlg,IDC_EDIT_CFG_VERSION,gi.sVersion.c_str());
				break;
			}
		case WM_COMMAND:
			{
				switch (LOWORD(wParam))
	            {
					case IDC_BUTTON_CFG_EXE_CANCEL:
						EndDialog(hDlg,0);
						break;
					case IDC_BUTTON_EXE_CFG_OK:
						{
							int gameID = CFG_GetGameID(g_currSelCfg);
							char szTemp[MAX_PATH*2];
							GAME_INSTALLATIONS gi;
							GetDlgItemText(hDlg,IDC_EDIT_CFG_PROPNAME,szTemp,MAX_PATH);
							if(strlen(szTemp)==0)
							{
								MessageBox(hDlg,"Ensure that a Name is entered.","Error saving",MB_OK);
								return TRUE;
							}

							gi.sName = szTemp;
							GetDlgItemText(hDlg,IDC_EDIT_PATH,szTemp,MAX_PATH);
							gi.szGAME_PATH = szTemp;
							GetDlgItemText(hDlg,IDC_EDIT_CMD,szTemp,MAX_PATH*2);
							gi.szGAME_CMD = szTemp;
							GetDlgItemText(hDlg,IDC_EDIT_CFG_VERSION,szTemp,MAX_PATH*2);
							gi.sVersion = szTemp;
							GetDlgItemText(hDlg,IDC_EDIT_CFG_MOD,szTemp,MAX_PATH*2);
							gi.sMod = szTemp;
							GamesInfoCFG[gameID].pSC->vGAME_INST.at(CFG_editexeIdx) = gi;
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

							ofn.lpstrInitialDir = GamesInfoCFG[gameID].szGAME_PATH;
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
	char szFile[260];
	switch(uMsg)
	{
		case WM_INITDIALOG:
			{
				SetWindowText(hDlg,lang.GetString("TitleNewInstall"));
				SetDlgItemText(hDlg,IDC_BUTTON_EXE_CFG_OK,lang.GetString("Save"));
				SetDlgItemText(hDlg,IDC_BUTTON_CFG_EXE_CANCEL,lang.GetString("Cancel"));
				SetDlgItemText(hDlg,IDC_STATIC_ADDNEWINSTALL,lang.GetString("AddNewInstall"));
				SetDlgItemText(hDlg,IDC_STATIC_EXE,lang.GetString("ColumnExePath"));
				SetDlgItemText(hDlg,IDC_STATIC_COMMAND,lang.GetString("ColumnCommand"));
				SetDlgItemText(hDlg,IDC_STATIC_VERSION,lang.GetString("CFGTextVersion"));
				SetDlgItemText(hDlg,IDC_STATIC_MOD,lang.GetString("CFGTextMod"));
				
				break;
			}
		case WM_COMMAND:
			{
				switch (LOWORD(wParam))
	            {
					case IDC_BUTTON_CFG_EXE_CANCEL:
						EndDialog(hDlg,0);
						break;
					case IDC_BUTTON_EXE_CFG_OK:
						{
							int gameID = CFG_GetGameID(g_currSelCfg);
							char szTemp[MAX_PATH*2];
							GAME_INSTALLATIONS gi;
							GetDlgItemText(hDlg,IDC_EDIT_CFG_PROPNAME,szTemp,MAX_PATH);
							if(strlen(szTemp)==0)
							{
								MessageBox(hDlg,"Ensure that a Name is entered.","Error saving",MB_OK);
								return TRUE;
							}
							gi.sName = szTemp;
							GetDlgItemText(hDlg,IDC_EDIT_PATH,szTemp,MAX_PATH);
							gi.szGAME_PATH = szTemp;
							GetDlgItemText(hDlg,IDC_EDIT_CMD,szTemp,MAX_PATH*2);
							gi.szGAME_CMD = szTemp;
							GetDlgItemText(hDlg,IDC_EDIT_CFG_VERSION,szTemp,MAX_PATH*2);
							gi.sVersion = szTemp;
							GetDlgItemText(hDlg,IDC_EDIT_CFG_MOD,szTemp,MAX_PATH*2);
							gi.sMod = szTemp;
							GamesInfoCFG[gameID].pSC->vGAME_INST.push_back(gi);

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

							ofn.lpstrInitialDir = GamesInfoCFG[gameID].szGAME_PATH;
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

	for ( iLst = GamesInfoCFG[GameId].pSC->vGAME_INST.rbegin(); iLst != GamesInfoCFG[GameId].pSC->vGAME_INST.rend( ); iLst++ )
	{
	
		GAME_INSTALLATIONS gi = *iLst;//currCV->vSI.at((int)pLVItem->iItem);
		if(gi.sName.length()>0)
			lvItem.pszText = (LPSTR)gi.sName.c_str();
		else
			lvItem.pszText = "Default";

		ListView_InsertItem( hwndLVexes,&lvItem);
		ListView_SetItemText(hwndLVexes,lvItem.iItem,1,(LPSTR)gi.sVersion.c_str());
		ListView_SetItemText(hwndLVexes,lvItem.iItem,2,(LPSTR)gi.sMod.c_str());
		ListView_SetItemText(hwndLVexes,lvItem.iItem,3,(LPSTR)gi.szGAME_PATH.c_str());
		ListView_SetItemText(hwndLVexes,lvItem.iItem,4,(LPSTR)gi.szGAME_CMD.c_str());

	}

}

LRESULT CALLBACK CFG_OnSelChangedProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char szText[256];
	char tmp[20];
	char szFile[260];
	
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


				if(AppCFGtemp.bAutostart)
					CheckDlgButton(hDlg,IDC_CHECK1,BST_CHECKED);
				else			
					CheckDlgButton(hDlg,IDC_CHECK1,BST_UNCHECKED);

				if(AppCFGtemp.bUseShortCountry)
					CheckDlgButton(hDlg,IDC_CHECK_SHORTNAME,BST_CHECKED);
				else			
					CheckDlgButton(hDlg,IDC_CHECK_SHORTNAME,BST_UNCHECKED);
				
				LVCOLUMN lvColumn;
				//initialize the columns
				lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM ;
				lvColumn.fmt = LVCFMT_LEFT;

				HWND hwndLVexes;
				hwndLVexes = GetDlgItem(hDlg,IDC_LIST_CFG_EXES);

				lvColumn.cx = 80;
				lvColumn.pszText = (LPSTR)lang.GetString("ColumnName");
				ListView_InsertColumn(hwndLVexes, 0, &lvColumn);
				lvColumn.cx = 120;
				lvColumn.pszText = (LPSTR)lang.GetString("ColumnLaunchByVer");
				ListView_InsertColumn(hwndLVexes, 1, &lvColumn);
				lvColumn.pszText = (LPSTR)lang.GetString("ColumnLaunchByMod");
				ListView_InsertColumn(hwndLVexes, 2, &lvColumn);
				lvColumn.cx = 180;
				lvColumn.pszText =  (LPSTR)lang.GetString("ColumnExePath"); 
				ListView_InsertColumn(hwndLVexes, 3, &lvColumn);
				lvColumn.pszText =  (LPSTR)lang.GetString("ColumnCommand"); 
				ListView_InsertColumn(hwndLVexes, 4, &lvColumn);

				DWORD dwExStyle=0;
				dwExStyle = ListView_GetExtendedListViewStyle(hwndLVexes);
				dwExStyle |= LVS_EX_FULLROWSELECT |  LVS_EX_LABELTIP ;
				ListView_SetExtendedListViewStyle(hwndLVexes,dwExStyle);			

				if(AppCFGtemp.bUse_minimize)
					CheckDlgButton(hDlg,IDC_CHECK2,BST_CHECKED);
				else
					CheckDlgButton(hDlg,IDC_CHECK2,BST_UNCHECKED);


			if(AppCFGtemp.bUSE_SCREEN_RESTORE )
				CheckDlgButton(hDlg,IDC_CHECK_SCR_RESTORE,BST_CHECKED);				

			
			sprintf(szText,"%d",(AppCFGtemp.socktimeout.tv_sec*1000)+AppCFGtemp.socktimeout.tv_usec);
			SetDlgItemText(hDlg,IDC_EDIT_SOCK_TIMEOUT_S,szText);
			
			sprintf(szText,"%d",AppCFGtemp.dwRetries);
			SetDlgItemText(hDlg,IDC_EDIT_CFG_RETRIES,szText);


			SetDlgItemText(hDlg,IDC_EDIT_MIRC,g_sMIRCoutput.c_str());
		

			if(AppCFGtemp.bUse_EXT_APP )
				CheckDlgButton(hDlg,IDC_CHECK_EXT_ACTIVE,BST_CHECKED);
			else
				CheckDlgButton(hDlg,IDC_CHECK_EXT_ACTIVE,BST_UNCHECKED);

			if(AppCFGtemp.bUseMIRC)
				CheckDlgButton(hDlg,IDC_CHECK_MIRC,BST_CHECKED);
			else			
				CheckDlgButton(hDlg,IDC_CHECK_MIRC,BST_UNCHECKED);


			lang.EnumerateLanguage();
		   for( map<string,string>::iterator ii=lang.m_Languages.begin(); ii!=lang.m_Languages.end(); ++ii)
		   {			   
			   const char *psz = (*ii).first.c_str() ;
			   SendMessage(GetDlgItem(hDlg,IDC_COMBO_LANG),   (UINT) CB_ADDSTRING, 0, (LPARAM)psz );  			
		   }
		   
		   SendMessage(GetDlgItem(hDlg,IDC_COMBO_LANG),   (UINT) CB_SELECTSTRING, 0, (LPARAM)lang.m_strCurrentLang.c_str() );  			

			char szTmp[10];
			int gameID=-1;
			gameID = CFG_GetGameID(g_currSelCfg);
			if(gameID!=-1)
			{
				sprintf(szText,lang.GetString("CFGSettingsTitle"),GamesInfoCFG[gameID].szGAME_NAME);
				SetWindowText(GetDlgItem(hDlg,IDC_STATIC_GAME_NAME),szText);

				SetDlgItemText(hDlg,IDC_EDIT_MASTER_SERVER,GamesInfoCFG[gameID].szMasterServerIP);
				SetDlgItemText(hDlg,IDC_EDIT_MASTER_PORT,_itoa(GamesInfoCFG[gameID].dwMasterServerPORT,szTmp,10));
				SetDlgItemText(hDlg,IDC_EDIT_PROTOCOL,_itoa(GamesInfoCFG[gameID].dwProtocol,szTmp,10));

				if(GamesInfoCFG[gameID].bUseHTTPServerList)
					CheckDlgButton(hDlg,IDC_CHECK_USE_HTTP,BST_CHECKED);
				else
					CheckDlgButton(hDlg,IDC_CHECK_USE_HTTP,BST_UNCHECKED);
				
				if(GamesInfoCFG[gameID].bActive)
					CheckDlgButton(hDlg,IDC_CHECK_ACTIVE,BST_CHECKED);
				else
					CheckDlgButton(hDlg,IDC_CHECK_ACTIVE,BST_UNCHECKED);


				CFG_Enumerate_installations(hDlg,gameID);
			}
			SetDlgItemText(hDlg,IDC_BUTTON_ADD_INSTALL,lang.GetString("CFGAddInstall"));
			SetDlgItemText(hDlg,IDC_BUTTON_EDIT_INSTALL,lang.GetString("CFGEditInstall"));
			SetDlgItemText(hDlg,IDC_BUTTON_DELETE_INSTALL,lang.GetString("CFGDeleteInstall"));



			SetDlgItemText(hDlg,IDC_EDIT_EXT_EXE,AppCFGtemp.szEXT_EXE_PATH);
			SetDlgItemText(hDlg,IDC_EDIT_EXT_CMD,AppCFGtemp.szEXT_EXE_CMD);
			SetDlgItemText(hDlg,IDC_EDIT_EXT_WINDOWNAME,AppCFGtemp.szEXT_EXE_WINDOWNAME);
			SetDlgItemText(hDlg,IDC_EDIT_WINDOWNAME,AppCFGtemp.szET_WindowName);

			SetDlgItemText(hDlg,IDC_STATIC_INSTALL_PATH,EXE_PATH);		

			g_hwndScrollTrans = GetDlgItem(hDlg,IDC_SLIDER_TRANS);
			SendMessage(g_hwndScrollTrans,TBM_SETRANGE,TRUE,(LPARAM)MAKELONG(MIN_TRANSPARANCY, 100)) ; //AppCFG.g_cTransparancy);
			SendMessage(g_hwndScrollTrans,TBM_SETPOS,TRUE,(LPARAM)AppCFGtemp.g_cTransparancy ) ; //AppCFG.g_cTransparancy);
			SetDlgTrans(hDlg,AppCFGtemp.g_cTransparancy);

			g_hwndScrollThreads =  GetDlgItem(hDlg,IDC_SLIDER_THREADS);
			SendMessage( g_hwndScrollThreads,TBM_SETRANGE,TRUE,(LPARAM)MAKELONG(1, 256)) ;
			SendMessage(g_hwndScrollThreads,TBM_SETPOS,TRUE,(LPARAM)AppCFGtemp.dwThreads) ; 

			char sztemp[20];		
			SetDlgItemText(hDlg,IDC_STATIC_THREAD,_itoa(AppCFGtemp.dwThreads ,sztemp,10));
			break;
		}
		case WM_VSCROLL:
		{
			if(lParam==(LPARAM)g_hwndScrollTrans)
			{
				int i = (char)SendMessage(GetDlgItem(hDlg,IDC_SLIDER_TRANS),TBM_GETPOS,0,(LPARAM)0) ; 
				SetDlgTrans(GetParent(hDlg),i);
			} else if (lParam==(LPARAM)g_hwndScrollThreads)
			{
				DWORD i = (DWORD)SendMessage(GetDlgItem(hDlg,IDC_SLIDER_THREADS),TBM_GETPOS,0,(LPARAM)0) ; 
				char sztemp[20];
				AppCFGtemp.dwThreads = i;
				SetDlgItemText(hDlg,IDC_STATIC_THREAD,_itoa(i,sztemp,10));
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
							char sztemp[200];
							int idx=   SendMessage(GetDlgItem(hDlg,IDC_COMBO_LANG),   (UINT) CB_GETCURSEL, 0, (LPARAM)0 );  
							SendMessage(GetDlgItem(hDlg,IDC_COMBO_LANG),   (UINT) CB_GETLBTEXT, idx, (LPARAM)sztemp );
							strcpy(AppCFGtemp.szLanguageFilename,lang.m_Languages[sztemp].c_str());
							lang.loadFile(lang.m_Languages[sztemp].c_str());
							MessageBox(NULL,lang.GetString("AlertRestartRequired"),"Alert!",MB_OK);
							 
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
								GamesInfoCFG[gameID].pSC->vGAME_INST.erase(GamesInfoCFG[gameID].pSC->vGAME_INST.begin()+n);
								CFG_Enumerate_installations(hDlg,gameID);
							}
						}
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
					ofn.lpstrFilter = "All\0*.*\0ET exe\0*.exe\0";
					ofn.nFilterIndex = 2;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					int gameID=-1;
					gameID = CFG_GetGameID(g_currSelCfg);

					if(gameID!=-1)
					{

						ofn.lpstrInitialDir = GamesInfoCFG[gameID].szGAME_PATH;
					}
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

					if(GetOpenFileName(&ofn))
						SetDlgItemText(hDlg,IDC_EDIT_PATH,ofn.lpstrFile);

					return TRUE;					
				}
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
	
	
			}
			break;
	}

	return FALSE;
}
