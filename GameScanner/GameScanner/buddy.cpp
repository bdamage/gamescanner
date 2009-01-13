


#include "stdafx.h"
#include "buddy.h"
#include "utilz.h"

#pragma warning(disable : 4995)
#pragma warning(disable : 4996)

extern TCHAR EXE_PATH[_MAX_PATH+_MAX_FNAME];			//Don't write anything to this path
extern TCHAR USER_SAVE_PATH[_MAX_PATH+_MAX_FNAME];     //Path to save settings and server lists
extern HWND g_hwndListBuddy;
extern HWND g_hwndListViewPlayers;
extern HWND g_hWnd;
extern LONG_PTR g_wpOrigListBuddyProc;
extern SERVER_INFO *g_CurrentSRV;
extern HINSTANCE g_hInst;
extern SERVER_INFO g_tmpSRV;
extern GamesMap GamesInfo;
extern vecBI BuddyList;
extern APP_SETTINGS_NEW AppCFG;
extern CLanguage g_lang;
extern SERVER_INFO Get_ServerInfoByIndex(GAME_INFO *pGI,int index);
extern void ShowBalloonTip(TCHAR *title,TCHAR *message);
extern UINT Get_GameIcon(UINT GameIndex);
extern void StartGame_ConnectToServer(bool connectFromBuddyList);
extern const TCHAR * ReadCfgStr2(TiXmlElement* pNode, TCHAR *szParamName,TCHAR *szOutputBuffer,int iBuffSize);
extern int ReadCfgInt2(TiXmlElement* pNode, TCHAR *szParamName, int& intVal);
extern BOOL WINAPI EditCopy(TCHAR *pText);
extern PLAYERDATA *Get_PlayerBySelection();
extern _CUSTOM_COLUMN BUDDY_CUSTCOLUMNS[MAX_COLUMNS];

bool bEditBuddyname=false;
//BUDDY_INFO *pEditBuddy;

vecBI::iterator itBuddyEdit;

void OnAddSelectedPlayerToBuddyList()
{
	PLAYERDATA * pPly = Get_PlayerBySelection();
	if(pPly!=NULL)
	{
		SERVER_INFO srv = Get_ServerInfoByIndex(&GamesInfo[pPly->cGAMEINDEX],pPly->dwServerIndex);
		Buddy_AddToList(pPly->szPlayerName,&srv);
		Buddy_UpdateList();
	}													
}



LRESULT CALLBACK Buddy_AddBuddyProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{
	switch (message)
	{
	case WM_INITDIALOG:
		{
			SetDlgItemText(hDlg,IDC_STATIC_BUDDY_NAME,g_lang.GetString("BuddyName"));
			CenterWindow(hDlg);
			if(bEditBuddyname)
			{

			//	if(pEditBuddy!=NULL)
				{
					if(itBuddyEdit->cMatchOnColorEncoded)
						CheckDlgButton(hDlg,IDC_CHECK_NO_COLOR_MATCH,BST_CHECKED);
					else			
						CheckDlgButton(hDlg,IDC_CHECK_NO_COLOR_MATCH,BST_UNCHECKED);

					if(itBuddyEdit->cMatchExact)
						CheckDlgButton(hDlg,IDC_CHECK_EXACT_MATCH,BST_CHECKED);
					else
						CheckDlgButton(hDlg,IDC_CHECK_EXACT_MATCH,BST_UNCHECKED);

					PostMessage(GetDlgItem(hDlg,IDC_EDIT_NICKNAME_FILTER),EM_SETSEL,0,strlen(itBuddyEdit->szPlayerName));
					PostMessage(GetDlgItem(hDlg,IDC_EDIT_NICKNAME_FILTER),EM_SETSEL,(WPARAM)-1,-1);
				}
				SetWindowText(hDlg,g_lang.GetString("TitleEditBuddy")); 
				SetDlgItemText(hDlg,IDC_EDIT_NICKNAME_FILTER,(PTCHAR)itBuddyEdit->szPlayerName);
			}
			else
			{
				SetWindowText(hDlg,g_lang.GetString("TitleAddBuddy")); 
			}
			SetFocus(GetDlgItem(hDlg,IDC_EDIT_NICKNAME_FILTER));
				
			//return TRUE;
			}
		break;
	case WM_COMMAND:

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			if(LOWORD(wParam) == IDOK)
			{
				TCHAR szBuddy[100];
			
				GetDlgItemText(hDlg,IDC_EDIT_NICKNAME_FILTER,szBuddy,sizeof(szBuddy)-1);
				if(!bEditBuddyname)
				{					
					Buddy_AddToList(szBuddy,NULL);
					Buddy_UpdateList();
				}else if(bEditBuddyname)
				{
					strcpy(itBuddyEdit->szPlayerName,szBuddy);
					itBuddyEdit->cMatchOnColorEncoded = IsDlgButtonChecked(hDlg,IDC_CHECK_NO_COLOR_MATCH);
					itBuddyEdit->cMatchExact = IsDlgButtonChecked(hDlg,IDC_CHECK_EXACT_MATCH);
				}

			}
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}



void Buddy_Save()
{
	if(BuddyList.size()==0)
		return;

	SetCurrentDirectory(USER_SAVE_PATH);

	TiXmlDocument doc;  
	TiXmlElement  *MainVersion;
 	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );  
	doc.LinkEndChild( decl );  
 
	TiXmlElement * root = new TiXmlElement( "BuddiesRoot" );  
	doc.LinkEndChild( root );  

	TiXmlElement * versions = new TiXmlElement( "Versions" );  
	root->LinkEndChild( versions );  

	MainVersion = new TiXmlElement("MainVersion");
	MainVersion->LinkEndChild(new TiXmlText(APP_VERSION));
	versions->LinkEndChild(MainVersion);
	//---------------------------
	//Filter options
	//---------------------------
	TiXmlElement * buddies = new TiXmlElement( "Buddies" );  
	root->LinkEndChild( buddies );  
	

	for(int i=0; i<BuddyList.size();i++)
	//	while(pBI!=NULL)
	{		
		BUDDY_INFO BI = BuddyList.at(i);
		if(BI.bRemove==false)
		{
			TiXmlElement * buddy = new TiXmlElement( "Buddy" );
			buddies->LinkEndChild( buddy ); 
		
			TiXmlElement *name = new TiXmlElement("Name");
			name->LinkEndChild(new TiXmlText(BI.szPlayerName));
			buddy->LinkEndChild(name);	

			TiXmlElement *clan = new TiXmlElement("Clan");
			clan->LinkEndChild(new TiXmlText(BI.szClan));
			buddy->LinkEndChild(clan);	

			TCHAR szBoolean[5];
			TiXmlElement *EM = new TiXmlElement("ExactMatch");
			if(BI.cMatchExact)
				strcpy(szBoolean,"1");
			else
				strcpy(szBoolean,"0");

			EM->LinkEndChild(new TiXmlText(szBoolean));
			buddy->LinkEndChild(EM);	

			TiXmlElement *MONCE = new TiXmlElement("MatchOnColorEncoded");
			if(BI.cMatchOnColorEncoded)
				strcpy(szBoolean,"1");
			else
				strcpy(szBoolean,"0");
			MONCE->LinkEndChild(new TiXmlText(szBoolean));
			buddy->LinkEndChild(MONCE);	

			TiXmlElement *LSIP = new TiXmlElement("LastSeenIP");
			LSIP->LinkEndChild(new TiXmlText(BI.szLastSeenIPaddress));
			buddy->LinkEndChild(LSIP);	

			TCHAR num[5];
			
			TiXmlElement *LSGI = new TiXmlElement("LastSeenGameIdx");
			LSGI->LinkEndChild(new TiXmlText(_itoa(BI.cGAMEINDEX,num,10)));
			buddy->LinkEndChild(LSGI);	


			TiXmlElement *LSSN = new TiXmlElement("LastSeenServerName");
			LSSN->LinkEndChild(new TiXmlText(BI.szLastSeenServerName));
			buddy->LinkEndChild(LSSN);	
		
		}
		
	}

	doc.SaveFile("Buddies.xml");

	OutputDebugString(_T("Saved buddylist.\n"));
}


void Buddy_Clear()
{
	BuddyList.clear();
}

int Buddy_Load()
{

	DWORD dwBuddyID = 0;
	SetCurrentDirectory(USER_SAVE_PATH);
	
	TiXmlDocument doc("Buddies.xml");
	if (!doc.LoadFile()) 
	{
		AddLogInfo(GS_LOG_INFO,"Error loading buddies.xml file or file didn't exsist.");
		return 1;
	}


		TiXmlHandle hDoc(&doc);
		TiXmlElement* pElem;
		TiXmlHandle hRoot(0);
		pElem=hDoc.FirstChildElement().Element();
		// should always have a valid root but handle gracefully if it does
		if (!pElem) 
			return 1;
		const TCHAR *szP;
		szP = pElem->Value(); //Should return BuddiesRoot

		// save this for later
		hRoot=TiXmlHandle(pElem);

		//Default values
		TiXmlElement* pElement;
		TCHAR szVersion[50];

		pElement=hRoot.FirstChild("Versions").ToElement();
		pElement = pElement->FirstChild()->ToElement();
		if(pElement!=NULL)
		{
			ReadCfgStr2(pElement , "MainVersion",szVersion,sizeof(szVersion));		
		}
	

		TiXmlElement* pBuddy;
		pBuddy=hRoot.FirstChild("Buddies").ToElement();
		if(pBuddy!=NULL)
		{
			if(pBuddy->FirstChild()!=NULL)
			{
				pBuddy = pBuddy->FirstChild()->ToElement();
				while(pBuddy!=NULL)
				{

					if(pBuddy!=NULL)
					{
						BUDDY_INFO bi;
						ZeroMemory(&bi,sizeof(BUDDY_INFO));
						TiXmlElement* pBud = pBuddy->FirstChild()->ToElement();

						ReadCfgStr2(pBud , "Name",bi.szPlayerName,sizeof(bi.szPlayerName));				
						ReadCfgStr2(pBud , "Clan",bi.szClan,sizeof(bi.szClan));
						ReadCfgStr2(pBud , "LastSeenServerName",bi.szLastSeenServerName,sizeof(bi.szLastSeenServerName));
						ReadCfgStr2(pBud , "LastSeenIP",bi.szLastSeenIPaddress,sizeof(bi.szLastSeenIPaddress));
						ReadCfgInt2(pBud , "MatchOnColorEncoded",(int&)bi.cMatchOnColorEncoded);
						ReadCfgInt2(pBud , "ExactMatch",(int&)bi.cMatchExact);						
						ReadCfgInt2(pBud , "LastSeenGameIdx",(int&)bi.cGAMEINDEX);

						bi.dwID = dwBuddyID++;
					
						bi.sIndex = -1;  //reset

						BuddyList.push_back(bi);

					}
					pBuddy = pBuddy->NextSiblingElement();
				}
			}else
			{
				AddLogInfo(GS_LOG_ERROR,"No buddies found.");
			}
		}else
			AddLogInfo(GS_LOG_ERROR,"Error loading buddy xml data.");

	//pBI = pFirstBI;
	return 0;
}

void Buddy_AdvertiseBuddyIsOnline(BUDDY_INFO *pBI, SERVER_INFO *pServerInfo)
{
	if(pBI==NULL)
		return;
	if(pServerInfo==NULL)
		return;

	vecBI::iterator it = Buddy_FindBuddyInfoByID(pBI->dwID);
	if(it!=BuddyList.end())
	{	
		strcpy_s(it->szServerName,sizeof(pBI->szServerName),pServerInfo->szServerName);
		it->cGAMEINDEX = pServerInfo->cGAMEINDEX;
		it->sIndex = (int) pServerInfo->dwIndex;  //have to change the Buddy index to a new var that can hold bigger numbers such as DWORD
	}
	HWND hwndLV = g_hwndListBuddy;

	LV_FINDINFO lvfi;
	TCHAR szText[250];
	memset(&lvfi,0,sizeof(LV_FINDINFO));
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = (LPARAM)pBI->dwID;
	int index = ListView_FindItem(hwndLV , -1,  &lvfi); 

	if(index!=-1)
	{
		LVITEM item;
		item.mask = LVIF_TEXT | LVIF_IMAGE;
		item.iItem = index;
		memset(szText,0,sizeof(szText));
	
		if(GamesInfo[pBI->cGAMEINDEX].colorfilter!=NULL)
		{					
			GamesInfo[pBI->cGAMEINDEX].colorfilter(pBI->szServerName,szText,249);
			item.pszText = szText;
			item.cchTextMax = (int)strlen(szText);
		}
		else
		{
			item.pszText = pBI->szServerName;
			item.cchTextMax = (int)strlen(pBI->szServerName);
		}

		item.iSubItem = 1;
		item.iImage = Get_GameIcon(pBI->cGAMEINDEX);
		ListView_SetItem(g_hwndListBuddy,&item);
		sprintf(szText,"%s:%d",pServerInfo->szIPaddress,pServerInfo->usPort);
		strcpy(pBI->szIPaddress,szText);
		ListView_SetItemText(g_hwndListBuddy,index ,2,szText);
	}
	if(GamesInfo[pBI->cGAMEINDEX].colorfilter!=NULL)
		GamesInfo[pBI->cGAMEINDEX].colorfilter(pBI->szServerName,szText,249);
	else
		strcpy(szText,pBI->szPlayerName);

	ShowBalloonTip("A buddy is online!",szText);		
}

long Buddy_CheckForBuddies(PLAYERDATA *pPlayers, SERVER_INFO *pServerInfo)
{
	//BUDDY_INFO *pBI = g_pBIStart;
	PLAYERDATA *pStart = pPlayers;

	if(BuddyList.size()==0)
		return 0;

	while(pPlayers!=NULL)
	{
		//&BI = g_&BIStart;
		
		//while(&BI!=NULL)
		for(unsigned int i=0;i<BuddyList.size();i++)
		{			
			
			BUDDY_INFO BI = BuddyList.at(i);

			if(BI.bRemove)
				break;
			if(pPlayers->szPlayerName==NULL)
				return 0;
			if((BI.cMatchExact) && (BI.cMatchOnColorEncoded==0))
			{
				TCHAR cf[100],cf2[100];

				if(GamesInfo[pServerInfo->cGAMEINDEX].GAME_ENGINE!= VALVE_ENGINE)
				{
					colorfilter(pPlayers->szPlayerName,cf,sizeof(cf));
					colorfilter(BI.szPlayerName,cf2,sizeof(cf2));
				}else
				{
					ZeroMemory(cf,sizeof(cf));
					ZeroMemory(cf2,sizeof(cf2));
					strncpy(cf,BI.szPlayerName,sizeof(cf)-1);
					strncpy(cf2,pPlayers->szPlayerName,sizeof(cf2)-1);
				}

				if(strcmp(cf,cf2)==0)
				{
					Buddy_AdvertiseBuddyIsOnline(&BI, pServerInfo);
					return 1;
				}
			}else if((BI.cMatchExact) && (BI.cMatchOnColorEncoded))
			{
			//	dbg_print("%s == %s",BI.szPlayerName,pPlayers->szPlayerName);
				
				if(strcmp(BI.szPlayerName,pPlayers->szPlayerName)==0)
				{
					Buddy_AdvertiseBuddyIsOnline(&BI, pServerInfo);
					return 1;
				} 			
			}else if(BI.cMatchOnColorEncoded)
			{
			//	dbg_print("%s == %s",BI.szPlayerName,pPlayers->szPlayerName);
				
				if(strstr(BI.szPlayerName,pPlayers->szPlayerName)!=NULL)
				{
					Buddy_AdvertiseBuddyIsOnline(&BI, pServerInfo);
					return 1;
				} 			
			}
			else
			{
				if(_stricmp(BI.szPlayerName,pPlayers->szPlayerName)==0)
				{
					Buddy_AdvertiseBuddyIsOnline(&BI, pServerInfo);
					return 1;
				} else
				{
					//Try without color codes
					TCHAR cf[100],cf2[100];

					if(GamesInfo[pServerInfo->cGAMEINDEX].GAME_ENGINE!= VALVE_ENGINE)
					{
						colorfilter(pPlayers->szPlayerName,cf,sizeof(cf));
						colorfilter(BI.szPlayerName,cf2,sizeof(cf2));
					}else
					{
						ZeroMemory(cf,sizeof(cf));
						ZeroMemory(cf2,sizeof(cf2));
						strncpy(cf,BI.szPlayerName,sizeof(cf)-1);
						strncpy(cf2,pPlayers->szPlayerName,sizeof(cf2)-1);
					}
					if(_stricmp(cf,cf2)==0)
					{
						Buddy_AdvertiseBuddyIsOnline(&BI, pServerInfo);
						return 1;
					}//Try to find FILTERED name of the current online FILTERED playername
					else if (strstr(cf,cf2)!=NULL)
					{
						Buddy_AdvertiseBuddyIsOnline(&BI, pServerInfo);
						return 1;
					} else
					{

						//Try to find playername with lower case
						TCHAR copy1[100],copy2[100];
						strcpy(copy1,cf);
						strcpy(copy2,cf2);
						_strlwr_s( copy1 , 99);
						if(strlen(copy2)>0)
							_strlwr_s( copy2 , 99);

						if(strstr(copy1,copy2)!=NULL)
						{
							Buddy_AdvertiseBuddyIsOnline(&BI, pServerInfo);				
							return 1;
						}
					}
				}
			}
		
		}
		pPlayers = pPlayers->pNext;
	}

	return 0;
}




//returns anything between 0 - 0xFFFFFFFF-1
//0xFFFFFFFF = unsuccessfull
DWORD Buddy_GetBuddyIDBySelection()
{
	int n = ListView_GetSelectionMark(g_hwndListBuddy);
	if(n != -1)
	{
		LVITEM lvItem;
		memset(&lvItem,0,sizeof(LVITEM));
		lvItem.mask =  LVIF_PARAM ;
		lvItem.iItem = n;
		lvItem.iSubItem = 0;
		if(ListView_GetItem( g_hwndListBuddy, &lvItem))
		{
			return (DWORD)lvItem.lParam;
			
		}
	}
	return 0xFFFFFFFF;
}

//verify with iterator with vecBI_it!=BuddyList.end() to check if it exsist
vecBI::iterator Buddy_FindBuddyInfoByID(DWORD dwID)
{
	vecBI::iterator vecBI_it = find(BuddyList.begin(),BuddyList.end(),(DWORD)dwID);
	return vecBI_it;
}
			

BOOL Buddy_AddToList(TCHAR *szName,SERVER_INFO *pServer)
{
	BUDDY_INFO BI;
	ZeroMemory(&BI,sizeof(BUDDY_INFO));
	strcpy(BI.szPlayerName,szName);

	if(pServer!=NULL)
	{
		TCHAR szIP[MAX_IP_LEN];
		sprintf(szIP,"%s:%d",pServer->szIPaddress,pServer->usPort);
		strcpy(BI.szServerName,pServer->szServerName);
		strcpy(BI.szIPaddress,szIP);
		strcpy(BI.szLastSeenIPaddress,szIP);
		BI.cGAMEINDEX = pServer->cGAMEINDEX;
		BI.sIndex = pServer->dwIndex;
	}

	BuddyList.push_back(BI);
	return TRUE;
}

void Buddy_UpdateList()
{
	
	ListView_DeleteAllItems(g_hwndListBuddy);

	LVITEM item;
	ZeroMemory(&item, sizeof(LVITEM));
	item.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM ;

	int n=0;
	for(unsigned int i=0; i<BuddyList.size();i++)
	{
		BUDDY_INFO BI = BuddyList.at(i);

		if(BI.bRemove == false)
		{
			TCHAR cf[100];
			ZeroMemory(&item, sizeof(LVITEM));
			item.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM ;
			item.iSubItem = 0;
			item.iImage = 3;
			item.iItem = n;
			
			if(GamesInfo[BI.cGAMEINDEX].colorfilter!=NULL)
			{					
				GamesInfo[BI.cGAMEINDEX].colorfilter(BI.szPlayerName,cf,99);
				item.pszText = cf;
				item.cchTextMax = (int)strlen(cf);
			}
			else
			{
				item.pszText = BI.szPlayerName;
				item.cchTextMax = (int)strlen(BI.szPlayerName);

			}
			
			item.lParam = (LPARAM)BI.dwID;
			ListView_InsertItem( g_hwndListBuddy,&item);
			
			//BUG!!
			if(strlen(BI.szServerName)>0)
			{
				ZeroMemory(&item, sizeof(LVITEM));
				item.mask = LVIF_IMAGE | LVIF_TEXT; 
				GamesInfo[BI.cGAMEINDEX].colorfilter(BI.szServerName,cf,sizeof(cf));
				//ListView_SetItemText(g_hwndListBuddy,item.iItem ,1,cf);
				item.iItem = n;
				item.iSubItem = 1;
				item.pszText = cf;
				item.cchTextMax = (int)strlen(cf);
				item.iImage = Get_GameIcon(BI.cGAMEINDEX);

				ListView_SetItem(g_hwndListBuddy,&item);
				ListView_SetItemText(g_hwndListBuddy,item.iItem ,2,BI.szIPaddress);
			}
			n++;
		}		
	}
	ListView_SetColumnWidth(g_hwndListBuddy,0,LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(g_hwndListBuddy,1,100);
	ListView_SetColumnWidth(g_hwndListBuddy,2,LVSCW_AUTOSIZE);
}


void Buddy_Add(bool manually)
{
	if(manually)
	{
		DialogBox(g_hInst, (LPCTSTR)IDD_DLG_ADD_BUDDY, g_hWnd, (DLGPROC)Buddy_AddBuddyProc);

	}else
	{
		OnAddSelectedPlayerToBuddyList();
	}
}

/*******************************
Remove buddy by listview index.
********************************/
void Buddy_Remove()
{
	
	vecBI::iterator it = Buddy_FindBuddyInfoByID(Buddy_GetBuddyIDBySelection());
	if(it!=BuddyList.end())
		it->bRemove = true;
//	pEditBuddy = Buddy_GetBuddyInfoBySelection();
//	pEditBuddy->bRemove = true;
	Buddy_UpdateList();

}




LRESULT APIENTRY Buddy_ListViewSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
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
				case IDM_EDIT_BUDDY:
					{
						bEditBuddyname=true;
						//pEditBuddy = Buddy_GetBuddyInfoBySelection();
						itBuddyEdit = Buddy_FindBuddyInfoByID(Buddy_GetBuddyIDBySelection());
						if(itBuddyEdit!=BuddyList.end())
						{						
							DialogBox(g_hInst, (LPCTSTR)IDD_DLG_ADD_BUDDY, g_hWnd, (DLGPROC)Buddy_AddBuddyProc);
						}
						
						bEditBuddyname=false;
						Buddy_UpdateList();
					}
				break;
				case IDM_REFRESH:				
				break;
				case IDM_ADD:
					Buddy_Add(true);					
				break;
				case IDM_DELETE:
					Buddy_Remove();
				break;
				case IDM_COPYIP:
					{
						int n=-1;
						TCHAR szIP[40];
						n = ListView_GetSelectionMark(g_hwndListBuddy);
						ListView_GetItemText(g_hwndListBuddy,n,2,szIP,sizeof(szIP)-1);
						if(n!=-1)
						{
							EditCopy(szIP);
						}
						else
							MessageBox(hwnd,g_lang.GetString("ErrorServerCopyToClipBoard"),NULL,MB_OK);
					}
				break;
				case IDM_CONNECT:
					StartGame_ConnectToServer(true);
				break;
			}
		}
		else if(uMsg==WM_LBUTTONDBLCLK)
		{
			PostMessage(hwnd,WM_COMMAND,LOWORD(IDM_CONNECT),0);
	    }
 		else if(uMsg == WM_RBUTTONDOWN)
		{
			//get mouse cursor position x and y as lParam has the message itself 
			POINT lpClickPoint;
			GetCursorPos(&lpClickPoint);
			hPopMenu = CreatePopupMenu();
			int n=-1;
			n = ListView_GetSelectionMark(g_hwndListBuddy);
			if(n!=-1)
			{
				//place the window/menu there if needed 						
				
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_CONNECT,g_lang.GetString("MenuConnect"));				
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_DELETE,g_lang.GetString("Remove"));
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_EDIT_BUDDY,g_lang.GetString("Edit"));
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_COPYIP,g_lang.GetString("MenuCopyIP"));
			}
						
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_ADD,g_lang.GetString("Add"));			
				//InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_REFRESH,"&Refresh");
												
				//workaround for microsoft bug, to hide menu w/o selecting
				SetForegroundWindow(hwnd);
				TrackPopupMenu(hPopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,lpClickPoint.x, lpClickPoint.y,0,hwnd,NULL);
				SendMessage(hwnd,WM_NULL,0,0);
				DestroyMenu(hPopMenu);
			return 0;
		}				
    return CallWindowProc((WNDPROC)g_wpOrigListBuddyProc, hwnd, uMsg,  wParam, lParam); 
} 

bool Buddy_Sort_Name(BUDDY_INFO BIa, BUDDY_INFO BIb)
{
	if(AppCFG.bSortBuddyAsc)
		return (CustomStrCmp(BIa.szPlayerName,BIb.szPlayerName)>0);
	else
		return (CustomStrCmp(BIa.szPlayerName,BIb.szPlayerName)<0);

}

bool Buddy_Sort_ServerName(BUDDY_INFO BIa, BUDDY_INFO BIb)
{
	if(AppCFG.bSortBuddyAsc)
		return (CustomStrCmp(BIa.szServerName,BIb.szServerName)>0);
	else
		return (CustomStrCmp(BIa.szServerName,BIb.szServerName)<0);

}