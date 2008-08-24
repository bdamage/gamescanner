


#include "stdafx.h"
#include "buddy.h"
#include "utilz.h"

#pragma warning(disable : 4995)
#pragma warning(disable : 4996)

extern char EXE_PATH[_MAX_PATH+_MAX_FNAME];			//Don't write anything to this path
extern char USER_SAVE_PATH[_MAX_PATH+_MAX_FNAME];     //Path to save settings and server lists
extern HWND g_hwndListBuddy;
extern HWND g_hwndListViewPlayers;
extern HWND g_hWnd;
extern LONG_PTR g_wpOrigListBuddyProc;
extern BUDDY_INFO *g_pBIStart;
extern SERVER_INFO *g_CurrentSRV;
extern HINSTANCE g_hInst;
extern SERVER_INFO g_tmpSRV;
extern GAME_INFO GI[MAX_SERVERLIST+1];
extern CLanguage lang;
extern SERVER_INFO Get_ServerInfoByIndex(GAME_INFO *pGI,int index);
extern void ShowBalloonTip(char *title,char *message);
extern char Get_GameIcon(char index);
extern void StartGame_ConnectToServer(bool connectFromBuddyList);
extern const char * ReadCfgStr2(TiXmlElement* pNode, char *szParamName,char *szOutputBuffer,int iBuffSize);
extern int ReadCfgInt2(TiXmlElement* pNode, char *szParamName, int& intVal);
extern BOOL WINAPI EditCopy(char *pText);
extern PLAYERDATA *Get_PlayerBySelection();

bool bEditBuddyname=false;
BUDDY_INFO *pEditBuddy;

void OnAddSelectedPlayerToBuddyList()
{
	PLAYERDATA * pPly = Get_PlayerBySelection();
	if(pPly!=NULL)
	{
		SERVER_INFO srv = Get_ServerInfoByIndex(&GI[pPly->cGAMEINDEX],pPly->dwServerIndex);
		Buddy_AddToList(g_pBIStart,pPly->szPlayerName,&srv);
		Buddy_UpdateList(g_pBIStart);
	}													
}



LRESULT CALLBACK Buddy_AddBuddyProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{
	switch (message)
	{
	case WM_INITDIALOG:
		{
		CenterWindow(hDlg);
		if(bEditBuddyname)
		{

			if(pEditBuddy!=NULL)
			{
				if(pEditBuddy->cMatchOnColorEncoded)
					CheckDlgButton(hDlg,IDC_CHECK_NO_COLOR_MATCH,BST_CHECKED);
				else			
					CheckDlgButton(hDlg,IDC_CHECK_NO_COLOR_MATCH,BST_UNCHECKED);

				if(pEditBuddy->cMatchExact)
					CheckDlgButton(hDlg,IDC_CHECK_EXACT_MATCH,BST_CHECKED);
				else
					CheckDlgButton(hDlg,IDC_CHECK_EXACT_MATCH,BST_UNCHECKED);

				PostMessage(GetDlgItem(hDlg,IDC_EDIT_NICKNAME_FILTER),EM_SETSEL,0,strlen(pEditBuddy->szPlayerName));
				PostMessage(GetDlgItem(hDlg,IDC_EDIT_NICKNAME_FILTER),EM_SETSEL,(WPARAM)-1,-1);
			}

			SetWindowText(hDlg,"Edit buddy..."); 
			SetDlgItemText(hDlg,IDC_EDIT_NICKNAME_FILTER,pEditBuddy->szPlayerName);

		}
		else
			SetWindowText(hDlg,"Add buddy..."); 
		
			SetFocus(GetDlgItem(hDlg,IDC_EDIT_NICKNAME_FILTER));

			
		//return TRUE;
		}
		break;
	case WM_COMMAND:

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			if(LOWORD(wParam) == IDOK)
			{
				char szBuddy[100];

				GetDlgItemText(hDlg,IDC_EDIT_NICKNAME_FILTER,szBuddy,sizeof(szBuddy)-1);
				if(!bEditBuddyname)
				{					
					Buddy_AddToList(g_pBIStart,szBuddy,NULL);
					Buddy_UpdateList(g_pBIStart);
				}else if(bEditBuddyname)
				{
					strcpy(pEditBuddy->szPlayerName,szBuddy);
					if(IsDlgButtonChecked(hDlg,IDC_CHECK_NO_COLOR_MATCH)==BST_CHECKED)
						pEditBuddy->cMatchOnColorEncoded=1;
					else
						pEditBuddy->cMatchOnColorEncoded=0;

					if(IsDlgButtonChecked(hDlg,IDC_CHECK_EXACT_MATCH)==BST_CHECKED)
						pEditBuddy->cMatchExact=1;
					else
						pEditBuddy->cMatchExact=0;
				}

				pEditBuddy =NULL;
			
			}
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}



void Buddy_Save(BUDDY_INFO *pBI)
{
	if(pBI==NULL)
		return;

	SetCurrentDirectory(USER_SAVE_PATH);
	//FILE *fp=fopen("buddies.dat", "wb");
	//if(fp!=NULL)
	//{
	//	while(pBI!=NULL)
	//	{		
	//		if(pBI->bRemove==false)
	//			fwrite((const void*)pBI, sizeof(BUDDY_INFO), 1, fp);

	//		pBI = pBI->pNext;
	//	}
	//	fclose(fp);
	//}


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
	
	while(pBI!=NULL)
	{		
		if(pBI->bRemove==false)
		{
			TiXmlElement * buddy = new TiXmlElement( "Buddy" );
			buddies->LinkEndChild( buddy ); 
		
			TiXmlElement *name = new TiXmlElement("Name");
			name->LinkEndChild(new TiXmlText(pBI->szPlayerName));
			buddy->LinkEndChild(name);	

			TiXmlElement *clan = new TiXmlElement("Clan");
			clan->LinkEndChild(new TiXmlText(pBI->szClan));
			buddy->LinkEndChild(clan);	

			char szBoolean[5];
			TiXmlElement *EM = new TiXmlElement("ExactMatch");
			if(pBI->cMatchExact)
				strcpy(szBoolean,"1");
			else
				strcpy(szBoolean,"0");

			EM->LinkEndChild(new TiXmlText(szBoolean));
			buddy->LinkEndChild(EM);	

			TiXmlElement *MONCE = new TiXmlElement("MatchOnColorEncoded");
			if(pBI->cMatchOnColorEncoded)
				strcpy(szBoolean,"1");
			else
				strcpy(szBoolean,"0");
			MONCE->LinkEndChild(new TiXmlText(szBoolean));
			buddy->LinkEndChild(MONCE);	

			TiXmlElement *LSIP = new TiXmlElement("LastSeenIP");
			LSIP->LinkEndChild(new TiXmlText(pBI->szLastSeenIPaddress));
			buddy->LinkEndChild(LSIP);	

			char num[5];
			
			TiXmlElement *LSGI = new TiXmlElement("LastSeenGameIdx");
			LSGI->LinkEndChild(new TiXmlText(_itoa(pBI->cGAMEINDEX,num,10)));
			buddy->LinkEndChild(LSGI);	


			TiXmlElement *LSSN = new TiXmlElement("LastSeenServerName");
			LSSN->LinkEndChild(new TiXmlText(pBI->szLastSeenServerName));
			buddy->LinkEndChild(LSSN);	
		
		}
		pBI = pBI->pNext;
	}

	doc.SaveFile("Buddies.xml");

	OutputDebugString("Saved buddylist.\n");
}


void Buddy_Clear(LPBUDDY_INFO pBI)
{
	if(pBI==NULL)
		return;
	if(pBI->pNext !=NULL)
		Buddy_Clear(pBI->pNext);

	free(pBI);
	pBI = NULL;
}

int Buddy_Load(LPBUDDY_INFO &pBI)
{
	BUDDY_INFO *pCurrentBI=NULL;
	BUDDY_INFO *pFirstBI=NULL;
	BUDDY_INFO *ptempBI=NULL;
	
	Buddy_Clear(pBI);
	pBI = NULL;

	AddLogInfo(ETSV_INFO,"Loading buddies.xml info.");
	SetCurrentDirectory(USER_SAVE_PATH);
	
	TiXmlDocument doc("Buddies.xml");
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
		szP = pElem->Value(); //Should return BuddiesRoot

		// save this for later
		hRoot=TiXmlHandle(pElem);

		//Default values
		TiXmlElement* pElement;
		char szVersion[50];

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
						TiXmlElement* pBud = pBuddy->FirstChild()->ToElement();
						ptempBI = (BUDDY_INFO*)malloc(sizeof(BUDDY_INFO));	
						ZeroMemory(ptempBI,sizeof(BUDDY_INFO));
						
						ReadCfgStr2(pBud , "Name",ptempBI->szPlayerName,sizeof(ptempBI->szPlayerName));				
						ReadCfgStr2(pBud , "Clan",ptempBI->szClan,sizeof(ptempBI->szClan));
						ReadCfgStr2(pBud , "LastSeenServerName",ptempBI->szLastSeenServerName,sizeof(ptempBI->szLastSeenServerName));
						ReadCfgStr2(pBud , "LastSeenIP",ptempBI->szLastSeenIPaddress,sizeof(ptempBI->szLastSeenIPaddress));
						ReadCfgInt2(pBud , "MatchOnColorEncoded",(int&)ptempBI->cMatchOnColorEncoded);
						ReadCfgInt2(pBud , "ExactMatch",(int&)ptempBI->cMatchExact);
						
						ReadCfgInt2(pBud , "LastSeenGameIdx",(int&)ptempBI->sIndex);
						
						ptempBI->sIndex = -1;  //reset

						if(pFirstBI==NULL)
							pFirstBI = pCurrentBI = ptempBI;
						else
							pCurrentBI = pCurrentBI->pNext = ptempBI;
					}
					pBuddy = pBuddy->NextSiblingElement();
				}
			}else
			{
				AddLogInfo(ETSV_ERROR,"No buddies found.");
			}
		}else
			AddLogInfo(ETSV_ERROR,"Error loading buddy xml data.");

	pBI = pFirstBI;
	return 0;
}

void Buddy_AdvertiseBuddyIsOnline(BUDDY_INFO *pBI, SERVER_INFO *pServerInfo)
{
	if(pBI==NULL)
		return;
	if(pServerInfo==NULL)
		return;

	strcpy_s(pBI->szServerName,sizeof(pBI->szServerName),pServerInfo->szServerName);
	
	pBI->pSERVER = NULL; //pServerInfo;
	pBI->cGAMEINDEX = pServerInfo->cGAMEINDEX;
	pBI->sIndex = (int) pServerInfo->dwIndex;  //have to change the Buddy index to a new var that can hold bigger numbers such as DWORD
	
	HWND hwndLV = g_hwndListBuddy;

	LV_FINDINFO lvfi;
	char szText[100];
	memset(&lvfi,0,sizeof(LV_FINDINFO));
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = (LPARAM)pBI;
	int index = ListView_FindItem(hwndLV , -1,  &lvfi); 

	if(index!=-1)
	{
		LVITEM item;
		item.mask = LVIF_TEXT | LVIF_IMAGE;
		item.iItem = index;
		memset(szText,0,sizeof(szText));
		colorfilter(pBI->szServerName,szText,99);
		item.pszText = szText;
		item.cchTextMax = strlen(szText);
		item.iSubItem = 1;
		item.iImage = Get_GameIcon(pBI->cGAMEINDEX);
		ListView_SetItem(g_hwndListBuddy,&item);

		//ListView_SetItemText(g_hwndListBuddy,index ,1,szText);
		sprintf(szText,"%s:%d",pServerInfo->szIPaddress,pServerInfo->dwPort);
		strcpy(pBI->szIPaddress,szText);
		ListView_SetItemText(g_hwndListBuddy,index ,2,szText);
	}
	if(pBI->cGAMEINDEX != CSS_SERVERLIST)
	{
		colorfilter(pBI->szPlayerName,szText,99);
		ShowBalloonTip("A buddy is online!",szText);		
	}else
	{
		ShowBalloonTip("A buddy is online!",pBI->szPlayerName);
	}
}

long Buddy_CheckForBuddies(PLAYERDATA *pPlayers, SERVER_INFO *pServerInfo)
{
	BUDDY_INFO *pBI = g_pBIStart;
	PLAYERDATA *pStart = pPlayers;
	if(pBI==NULL)
		return 0;
	while(pPlayers!=NULL)
	{
		pBI = g_pBIStart;
		while(pBI!=NULL)
		{			

			if(pBI->bRemove)
				break;
			if(pPlayers->szPlayerName==NULL)
				return 0;
			if((pBI->cMatchExact) && (pBI->cMatchOnColorEncoded==0))
			{
				char cf[100],cf2[100];

				if(pServerInfo->cGAMEINDEX!=CSS_SERVERLIST)
				{
					colorfilter(pPlayers->szPlayerName,cf,sizeof(cf));
					colorfilter(pBI->szPlayerName,cf2,sizeof(cf2));
				}else
				{
					ZeroMemory(cf,sizeof(cf));
					ZeroMemory(cf2,sizeof(cf2));
					strncpy(cf,pBI->szPlayerName,sizeof(cf)-1);
					strncpy(cf2,pPlayers->szPlayerName,sizeof(cf2)-1);
				}

				if(strcmp(cf,cf2)==0)
				{
					Buddy_AdvertiseBuddyIsOnline(pBI, pServerInfo);
					return 1;
				}
			}else if((pBI->cMatchExact) && (pBI->cMatchOnColorEncoded))
			{
			//	dbg_print("%s == %s",pBI->szPlayerName,pPlayers->szPlayerName);
				
				if(strcmp(pBI->szPlayerName,pPlayers->szPlayerName)==0)
				{
					Buddy_AdvertiseBuddyIsOnline(pBI, pServerInfo);
					return 1;
				} 			
			}else if(pBI->cMatchOnColorEncoded)
			{
			//	dbg_print("%s == %s",pBI->szPlayerName,pPlayers->szPlayerName);
				
				if(strstr(pBI->szPlayerName,pPlayers->szPlayerName)!=NULL)
				{
					Buddy_AdvertiseBuddyIsOnline(pBI, pServerInfo);
					return 1;
				} 			
			}
			else
			{
				if(_stricmp(pBI->szPlayerName,pPlayers->szPlayerName)==0)
				{
					Buddy_AdvertiseBuddyIsOnline(pBI, pServerInfo);
					return 1;
				} else
				{
					//Try without color codes
					char cf[100],cf2[100];

					if(pServerInfo->cGAMEINDEX!=CSS_SERVERLIST)
					{
						colorfilter(pPlayers->szPlayerName,cf,sizeof(cf));
						colorfilter(pBI->szPlayerName,cf2,sizeof(cf2));
					}else
					{
						ZeroMemory(cf,sizeof(cf));
						ZeroMemory(cf2,sizeof(cf2));
						strncpy(cf,pBI->szPlayerName,sizeof(cf)-1);
						strncpy(cf2,pPlayers->szPlayerName,sizeof(cf2)-1);
					}
					if(_stricmp(cf,cf2)==0)
					{
						Buddy_AdvertiseBuddyIsOnline(pBI, pServerInfo);
						return 1;
					}//Try to find FILTERED name of the current online FILTERED playername
					else if (strstr(cf,cf2)!=NULL)
					{
						Buddy_AdvertiseBuddyIsOnline(pBI, pServerInfo);
						return 1;
					} else
					{

						//Try to find playername with lower case
						char copy1[100],copy2[100];
						strcpy(copy1,cf);
						strcpy(copy2,cf2);
						_strlwr_s( copy1 , 99);
						if(strlen(copy2)>0)
							_strlwr_s( copy2 , 99);

						if(strstr(copy1,copy2)!=NULL)
						{
							Buddy_AdvertiseBuddyIsOnline(pBI, pServerInfo);				
							return 1;
						}
					}
				}
			}
			pBI = pBI->pNext;
		}
		pPlayers = pPlayers->pNext;
	}

	return 0;
}




BUDDY_INFO *Buddy_GetBuddyInfoBySelection()
{
	int n = ListView_GetSelectionMark(g_hwndListBuddy);
	if(n != -1)
	{
		LVITEM lvItem;
		memset(&lvItem,0,sizeof(LVITEM));
		lvItem.mask =  LVIF_PARAM ;//LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE ;
		lvItem.iItem = n;
		lvItem.iSubItem = 0;
		if(ListView_GetItem( g_hwndListBuddy, &lvItem))
		{
			return (BUDDY_INFO*)lvItem.lParam;
			
		}
	}
	return NULL;
}

BUDDY_INFO *Buddy_AddToList(LPBUDDY_INFO &pBI,char *szName,SERVER_INFO *pServer)
{
	BUDDY_INFO* pCurrentBI = NULL;
	BUDDY_INFO* pTempBI = NULL;
	pTempBI = pBI;

	//Move to the last server and continue from there
	while(pTempBI!=NULL)
	{
		if(pTempBI->pNext!=NULL)
			pTempBI = pTempBI->pNext;
		else
			break;		
	}
	
	pCurrentBI = (BUDDY_INFO*)malloc(sizeof(BUDDY_INFO));
	ZeroMemory(pCurrentBI,sizeof(BUDDY_INFO));

	if(pTempBI!=NULL)
		pTempBI->pNext = pCurrentBI;
	else
		pBI = pTempBI = pCurrentBI; //Ok we had to create a startSI

	strcpy(pCurrentBI->szPlayerName,szName);
	pCurrentBI->pSERVER = pServer;
	
	if(pServer!=NULL)
	{
		char szIP[MAX_IP_LEN];
		sprintf(szIP,"%s:%d",pServer->szIPaddress,pServer->dwPort);
		strcpy(pCurrentBI->szServerName,pServer->szServerName);
		strcpy(pCurrentBI->szIPaddress,szIP);
		strcpy(pCurrentBI->szLastSeenIPaddress,szIP);

		pCurrentBI->cGAMEINDEX = pServer->cGAMEINDEX;
		pCurrentBI->sIndex = pServer->dwIndex;

	}
	return pBI;
}

void Buddy_UpdateList(BUDDY_INFO *pBI)
{
	
	ListView_DeleteAllItems(g_hwndListBuddy);

	LVITEM item;
	ZeroMemory(&item, sizeof(LVITEM));
	item.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM ;

	int i=0;
	while(pBI!=NULL)
	{
		if(pBI->bRemove == false)
		{
			char cf[100];
			ZeroMemory(&item, sizeof(LVITEM));
			item.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM ;
			item.iSubItem = 0;
			item.iImage = 3;
			item.iItem = i;
			
			if(GI[pBI->cGAMEINDEX].colorfilter!=NULL)
			{
			
				GI[pBI->cGAMEINDEX].colorfilter(pBI->szPlayerName,cf,99);
				item.pszText = cf;
				item.cchTextMax = (int)strlen(cf);
			}
			else
			{
				item.pszText = pBI->szPlayerName;
				item.cchTextMax = (int)strlen(pBI->szPlayerName);

			}

			//colorfilter(pBI->szPlayerName,cf,99);			
			
			item.lParam = (LPARAM)pBI;
			ListView_InsertItem( g_hwndListBuddy,&item);
			
			//BUG!!
			if(strlen(pBI->szServerName)>0)
			{
				ZeroMemory(&item, sizeof(LVITEM));
				item.mask = LVIF_IMAGE | LVIF_TEXT; 
				colorfilter(pBI->szServerName,cf,99);
				//ListView_SetItemText(g_hwndListBuddy,item.iItem ,1,cf);
				item.iItem = i;
				item.iSubItem = 1;
				item.pszText = cf;
				item.cchTextMax = (int)strlen(cf);
				item.iImage = Get_GameIcon(pBI->cGAMEINDEX);
				if(pBI->pSERVER!=NULL)
				{
//				
				}
				ListView_SetItem(g_hwndListBuddy,&item);
				ListView_SetItemText(g_hwndListBuddy,item.iItem ,2,pBI->szIPaddress);
			}
			
			
			i++;
		}	
		pBI = pBI->pNext;
		
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
	pEditBuddy = Buddy_GetBuddyInfoBySelection();
	pEditBuddy->bRemove = true;
	Buddy_UpdateList(g_pBIStart);

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
						pEditBuddy = Buddy_GetBuddyInfoBySelection();
						if(pEditBuddy!=NULL)
							DialogBox(g_hInst, (LPCTSTR)IDD_DLG_ADD_BUDDY, g_hWnd, (DLGPROC)Buddy_AddBuddyProc);
						
						bEditBuddyname=false;
						Buddy_UpdateList(g_pBIStart);
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
						char szIP[40];
						n = ListView_GetSelectionMark(g_hwndListBuddy);
						ListView_GetItemText(g_hwndListBuddy,n,2,szIP,sizeof(szIP)-1);
						if(n!=-1)
						{
							EditCopy(szIP);
						}
						else
							MessageBox(hwnd,lang.GetString("ErrorServerCopyToClipBoard"),NULL,MB_OK);
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
				
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_CONNECT,"&Connect");				
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_DELETE,"&Remove");
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_EDIT_BUDDY,"Edit");
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_COPYIP,"Copy &IP");
			}
						
				InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_ADD,"&Add");			
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

