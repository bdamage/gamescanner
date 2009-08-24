
#include "stdafx.h"
#include "rcon.h"
#include "utilz.h"
#include "logger.h"

#pragma warning(disable : 4995)
#pragma warning(disable : 4996)

#define WM_WSAASYNC (WM_USER+1)
#define MAX_BACKLOG 20

SOCKET RCON_ConnectSocket=NULL;
SERVER_INFO *g_RCONServer;
int dwRCONLOG=0;  //index
int dwRCONLOGStepper=0;  //index
LONG_PTR wpOrigEditProc=NULL;

HWND g_hwndRCONCmd=NULL,g_hwndRCONOut=NULL,g_hRCONDlg=NULL;
char cGAMEINDEX=0;
extern HINSTANCE g_hInst;
extern SERVER_INFO *g_CurrentSRV;
extern GamesMap GamesInfo;
//extern GAME_INFO GamesInfo[GamesInfo.size()+1];
extern CLanguage g_lang;
extern CLogger log;

list<string> RCONList;
char szRCON_CMD_BACKLOG[MAX_BACKLOG][80];

void LoadRCONCommandList()
{
	RCONList.clear();
	RCONList.push_back("kick");
	RCONList.push_back("config");
	RCONList.push_back("ref");
	RCONList.push_back("map_restart");
	RCONList.push_back("cls");
	RCONList.push_back("cvarlist");
	RCONList.push_back("gamemod");
	RCONList.push_back("modversion");
	RCONList.push_back("timeinfo");
	RCONList.push_back("cmdlist");
	RCONList.push_back("serverinfo");
	RCONList.push_back("status");
	RCONList.push_back("map");
	RCONList.push_back("say");
	RCONList.push_back("vstr");
	RCONList.push_back("exec");
	RCONList.push_back("set");
	RCONList.push_back("seta");
	RCONList.push_back("setu");
	RCONList.push_back("sets");
	RCONList.push_back("devmap");
	RCONList.push_back("sectorlist");
	RCONList.push_back("dumpuser");
	RCONList.push_back("systeminfo");
	RCONList.push_back("heartbeat");
	RCONList.push_back("vminfo");
	RCONList.push_back("midiinfo");
	RCONList.push_back("writeconfig");
	RCONList.push_back("changeVectors");
	RCONList.push_back("meminfo");
	RCONList.push_back("bind");
	RCONList.push_back("unbindall");
	RCONList.push_back("touchFile");
	RCONList.push_back("dir");
	RCONList.push_back("path");
	RCONList.push_back("bindlist");
	RCONList.push_back("unbind");
	RCONList.push_back("reset");
	RCONList.push_back("toggle");
	RCONList.push_back("wait");
	RCONList.push_back("echo");
}

LRESULT APIENTRY RCON_EditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
	static list <string>::iterator sIter;
	static char szCmdAuto[256];

	if(uMsg == WM_KEYDOWN)
	 {
	 	if(wParam==VK_UP)
			return TRUE;
	 	if(wParam==VK_DOWN)
			return TRUE;
		if(wParam==VK_TAB)
			return TRUE;
	 }

 	if(uMsg == WM_KEYUP)
	{			
			if(wParam>=0x41 && wParam<=0x5A)
			{
				
				GetDlgItemText(g_hRCONDlg,IDC_EDIT_CMD,szCmdAuto,sizeof(szCmdAuto)-1);
				for(sIter = RCONList.begin(); sIter!=RCONList.end();)
				{
					string s = *sIter;
					if(strncmp(s.c_str(),szCmdAuto,strlen(szCmdAuto))==0)
					{
						SetDlgItemText(g_hRCONDlg,IDC_EDIT_CMD,s.c_str());
						PostMessage(GetDlgItem(g_hRCONDlg,IDC_EDIT_CMD),EM_SETSEL,strlen(szCmdAuto),s.size());
						break;
					}
					sIter++;
				}
				
			}
			if(wParam==VK_TAB)
			{
				if(sIter==RCONList.end())
					sIter = RCONList.begin();
				else
					sIter++;
				while(sIter!=RCONList.end())
				{

					string s = *sIter;
					if(strncmp(s.c_str(),szCmdAuto,strlen(szCmdAuto))==0)
					{
						SetDlgItemText(g_hRCONDlg,IDC_EDIT_CMD,s.c_str());
						PostMessage(GetDlgItem(g_hRCONDlg,IDC_EDIT_CMD),EM_SETSEL,strlen(szCmdAuto),s.size());
						break;
					}
					sIter++;
				}

			}
			if(wParam==VK_F1)
			{

				char szCmd[128],sztempCmd[128];
				GetDlgItemText(g_hRCONDlg,IDC_EDIT_CMD,szCmd,sizeof(szCmd)-1);
				sprintf_s(sztempCmd,sizeof(sztempCmd),"%s\t",szCmd);
				RCON_SendCmd(g_RCONServer,g_RCONServer->szRCONPASS,sztempCmd);
				return TRUE;
			}
			if(wParam==VK_RETURN)
			{
				char szCmd[128];
				GetDlgItemText(g_hRCONDlg,IDC_EDIT_CMD,szCmd,sizeof(szCmd)-1);


				SetDlgItemText(g_hRCONDlg,IDC_EDIT_CMD,"");
				if((szCmd!=NULL) && (strlen(szCmd)>0))
				{
					strcpy(szRCON_CMD_BACKLOG[dwRCONLOGStepper],szCmd);
					dwRCONLOG = dwRCONLOGStepper++;
					dwRCONLOG++;
					
					if(dwRCONLOGStepper>MAX_BACKLOG)
						dwRCONLOGStepper=0;

					RCON_SendCmd(g_RCONServer,g_RCONServer->szRCONPASS,szCmd);
				}
				return TRUE;

			}
			if(wParam==VK_UP)
			{
				dwRCONLOG--;
				if(dwRCONLOG<0)
					dwRCONLOG=0;
				if(szRCON_CMD_BACKLOG[dwRCONLOG]!=NULL)
					SetDlgItemText(g_hRCONDlg,IDC_EDIT_CMD,szRCON_CMD_BACKLOG[dwRCONLOG]);
		
				PostMessage(hwnd,EM_SETSEL,strlen(szRCON_CMD_BACKLOG[dwRCONLOG]),-1);
				//PostMessage(hwnd,EM_SETSEL,1,3);
				return TRUE;
			} else if(wParam==VK_DOWN)
			{
				dwRCONLOG++;
				if(dwRCONLOG>dwRCONLOGStepper)
					dwRCONLOG=dwRCONLOGStepper;

				if(szRCON_CMD_BACKLOG[dwRCONLOG]!=NULL)
				{
					SetDlgItemText(g_hRCONDlg,IDC_EDIT_CMD,szRCON_CMD_BACKLOG[dwRCONLOG]);
					PostMessage(hwnd,EM_SETSEL,strlen(szRCON_CMD_BACKLOG[dwRCONLOG]),-1);
				}
				else
				{
					dwRCONLOG--;
					if(dwRCONLOG<0)
						dwRCONLOG=0;
					SetDlgItemText(g_hRCONDlg,IDC_EDIT_CMD,"");
					
				}
				return TRUE;
			}
			
		}				
    return CallWindowProc((WNDPROC)wpOrigEditProc, hwnd, uMsg,  wParam, lParam); 
}


LRESULT CALLBACK RCON_Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_WSAASYNC:
		{			
			switch(WSAGETSELECTEVENT(lParam))
			{
				case FD_CLOSE :
				{
					RCON_Disconnect();
					return(0);
				}
				case FD_READ:
				{
					DWORD errCode = WSAGETSELECTERROR(lParam);
					if(errCode == WSAENETDOWN)
						return (FALSE);
				
					RCON_Read(g_RCONServer);
					  return(0);
				}
			}
		}
	case WM_SIZE:
		{
			RECT rc,item;
			GetClientRect(hDlg,&rc);
			GetClientRect(g_hwndRCONCmd,&item);
			MoveWindow(g_hwndRCONOut,0,0,rc.right,rc.bottom-50,TRUE);
			int x = (int)(rc.right * 0.7);
			MoveWindow(g_hwndRCONCmd,0,rc.bottom-50,x,20,TRUE);

			GetClientRect(GetDlgItem(hDlg, IDC_STATIC_RCON_INFO),&item);
			MoveWindow(GetDlgItem(hDlg, IDC_STATIC_RCON_INFO),0,rc.bottom-30,x,50,TRUE);
			GetClientRect(GetDlgItem(hDlg, IDOK),&item);
			MoveWindow(GetDlgItem(hDlg, IDOK),x+10,rc.bottom-50,item.right,item.bottom,TRUE);
			GetClientRect(GetDlgItem(hDlg, ID_RCON_CONNECT),&item);
			MoveWindow(GetDlgItem(hDlg, ID_RCON_CONNECT),x+10,rc.bottom-20,item.right,item.bottom,TRUE);
			GetClientRect(GetDlgItem(hDlg, IDC_CHECK_COLFILTER),&item);
			MoveWindow(GetDlgItem(hDlg, IDC_CHECK_COLFILTER),x+100,rc.bottom-30,item.right,item.bottom,TRUE);
			return TRUE;
		}
			break;
	case WM_INITDIALOG:
		{
			LoadRCONCommandList();
			dwRCONLOG=0;
			g_hRCONDlg = hDlg;
			g_hwndRCONCmd = GetDlgItem(hDlg,IDC_EDIT_CMD);
			g_hwndRCONOut = GetDlgItem(hDlg,IDC_LIST_RCON);
			EnableWindow( GetDlgItem(hDlg, IDOK),FALSE); 
			wpOrigEditProc =  SetWindowLongPtr(g_hwndRCONCmd, GWLP_WNDPROC, (LONG_PTR) RCON_EditSubclassProc); 
			SetWindowText(GetDlgItem(hDlg, ID_RCON_CONNECT),"Connect");
			//return TRUE;
		}
		break;


	case WM_CLOSE:
		RCON_Disconnect();
		SetWindowLong(g_hwndRCONCmd, GWLP_WNDPROC, (LONG) wpOrigEditProc); 
		break;

	case WM_COMMAND:
		{

			
			switch(LOWORD(wParam))
			{
				case ID_RCON_CONNECT:
					{

						char szWinName[40];
						GetWindowText(GetDlgItem(hDlg, ID_RCON_CONNECT),szWinName,39);
//						if(strcmp(szWinName,"Connect")==0)
//						{

							if(g_CurrentSRV == NULL)
							{
								MessageBox(hDlg,g_lang.GetString("SelectServerAtConnectionRCON"),"Error",MB_OK);
								return TRUE;
							}
							g_RCONServer = g_CurrentSRV;
							int ret =(int) DialogBox(g_hInst, (LPCTSTR)IDD_DLG_RCON_PASS, hDlg, (DLGPROC)RCON_AskPasswordProc);					

							if(ret == IDOK)
							{
							//	GamesInfo[g_RCONServer->cGAMEINDEX].vSI.at(g_RCONServer->dwIndex) = g_CurrentSelServer;
								RCON_Connect(g_RCONServer);
								SetFocus(GetDlgItem(hDlg,IDC_EDIT_CMD));
								RCON_SendCmd(g_RCONServer,g_RCONServer->szRCONPASS,"status"); 
								EnableWindow( GetDlgItem(hDlg, IDOK),TRUE);  //Join button
								SetWindowText(GetDlgItem(hDlg, ID_RCON_CONNECT),"Disconnect");
							}
/*							}
					else
						{
							EnableWindow( GetDlgItem(hDlg, IDOK),FALSE); 
							SetWindowText(GetDlgItem(hDlg, ID_RCON_CONNECT),"Connect");
							RCON_Disconnect();
						}
*/
					break;
					}
				case IDOK:
				{
					char szCmd[128];
					GetDlgItemText(hDlg,IDC_EDIT_CMD,szCmd,sizeof(szCmd)-1);
					strcpy(szRCON_CMD_BACKLOG[dwRCONLOG],szCmd);
					dwRCONLOG++;
					if(dwRCONLOG>9)
						dwRCONLOG=0;

					SetDlgItemText(hDlg,IDC_EDIT_CMD,"");
					if((szCmd!=NULL) && (strlen(szCmd)>0))
						RCON_SendCmd(g_RCONServer,g_RCONServer->szRCONPASS,szCmd);
					return TRUE;
				}	
				
				case IDCANCEL:
					{
						EndDialog(hDlg, LOWORD(wParam));
						return TRUE;
					}
				break;
			}
		break;
		}
	}
	return FALSE;
}


LRESULT CALLBACK RCON_AskPasswordProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		{
		CenterWindow(hDlg);
		g_hwndRCONCmd = GetDlgItem(hDlg,IDC_EDIT_PASSWORD);
		SetDlgItemText(hDlg,IDC_EDIT_PASSWORD,g_RCONServer->szRCONPASS);
		
		SetFocus(GetDlgItem(hDlg,IDC_EDIT_PASSWORD));
		SetWindowText(hDlg,g_RCONServer->szServerName);
		if(g_RCONServer!=NULL)
			if(g_RCONServer->szRCONPASS!=NULL)
				PostMessage(GetDlgItem(hDlg,IDC_EDIT_PASSWORD),EM_SETSEL,0,strlen(g_RCONServer->szRCONPASS));

		//return TRUE;
		}
		break;
	case WM_COMMAND:

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			if(LOWORD(wParam) == IDOK)
			{
				GetDlgItemText(hDlg,IDC_EDIT_PASSWORD,g_RCONServer->szRCONPASS,sizeof(g_RCONServer->szRCONPASS)-1);
			}

			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}


DWORD RCON_Connect(SERVER_INFO *pServer)
{
	if(pServer==NULL)
	{
		log.AddLogInfo(0,"RCON_Connect @ pServer==NULL");
		return 1;
	}

	if(RCON_ConnectSocket!=NULL)
		RCON_Disconnect();

	RCON_ConnectSocket = getsockudp(pServer->szIPaddress ,(unsigned short)pServer->usPort); 
 
	if(INVALID_SOCKET==RCON_ConnectSocket)
	{
		log.AddLogInfo(0,"RCON_Connect @ INVALID SOCKET");
		return 2;
	}

	WSAAsyncSelect(RCON_ConnectSocket, g_hRCONDlg, WM_WSAASYNC,  FD_READ | FD_CLOSE);


	return 0;
}





DWORD RCON_WriteToConsole(int index, const char *szMessage)
{
	if(szMessage==NULL)
	{
		//SendMessage(g_hwndRCONOut, LB_ADDSTRING, (WPARAM) index, (LPARAM) "");
		return 1;
	}

	if(IsDlgButtonChecked(g_hRCONDlg,IDC_CHECK_COLFILTER)==BST_CHECKED)	
	{
		char* colorfiltered = (char*) calloc(1,strlen(szMessage)+1);

		SendMessage(g_hwndRCONOut, LB_ADDSTRING, (WPARAM) index, (LPARAM) colorfilter(szMessage,colorfiltered,sizeof(colorfiltered)-1));
		free(colorfiltered);
	}
	else
		SendMessage(g_hwndRCONOut, LB_ADDSTRING, (WPARAM) index, (LPARAM) szMessage);

	SendMessage(g_hwndRCONOut, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN,0),NULL); 
	
	return 0;	
}


DWORD RCON_Parse(SERVER_INFO *pServer,unsigned char *Buff, size_t dwPacketLen)
{
		 char *pEndAddress = (char*)Buff+dwPacketLen;
		//Buff[dwPacketLen-1]=0; //add string termination
		int start =0 ;

		char * pBuf2;
		
		int i =0;
		while(Buff[i]==0xFF) //Trim yyyy (char 255)
			i++;

		pBuf2 = (char*)&Buff[i];

		if(pServer!=NULL)
		{
			switch(pServer->cGAMEINDEX)
			{
				
				case Q4_SERVERLIST :
				case ETQW_SERVERLIST:
					pBuf2 =&pBuf2[10];  
				break;
				case COD_SERVERLIST:
				case COD2_SERVERLIST:
				case COD4_SERVERLIST:
					//nada
					break;
				case ET_SERVERLIST:
					pBuf2 =&pBuf2[5]; 
					break;
			}
		}
		pBuf2 = strtok( (char*)pBuf2, "\\\n" );
            
		RCON_WriteToConsole(start++, (const char *)pBuf2);

       while( pBuf2 != NULL ) {

            pBuf2 = strtok( NULL, "\\\n" );
            if (pBuf2 == NULL)
                break;

			if(pBuf2>pEndAddress)
				break;
			

			RCON_WriteToConsole(start++, pBuf2);

	   }
	   
	   return 0;
}

DWORD RCON_Read(SERVER_INFO *pServer)
{
	unsigned char* pckt = NULL;
	size_t packetlen=0;
	
	pckt = (unsigned char*)getpacket(RCON_ConnectSocket, &packetlen);
	
	if(pckt!=NULL)
	{
		//AddLogInfo(0,"Recv packl %d", packetlen);
		RCON_Parse(pServer,pckt,packetlen);
		free(pckt);
	}	
	return 0;
}




DWORD RCON_SendCmd(SERVER_INFO* pSI,char *szPassword,char *szCmd)
{
	size_t packetlen=0;
	char sendbuf[512];

	if(strcmp(szCmd,"cls")==0)
	{
		SendMessage(g_hwndRCONOut, LB_RESETCONTENT, (WPARAM) NULL, (LPARAM) NULL);
		return 0;
	}

	//net_clientRemoteConsolePassword

	if(pSI==NULL)
		return 2;

	if(pSI->cGAMEINDEX==1)
		sprintf_s(sendbuf,sizeof(sendbuf), "\xFF\xFFrcon\xFF%s\xFF%s\xFF",szPassword,szCmd); //ETQW (might work for Doom 3)
	else
		sprintf_s(sendbuf,sizeof(sendbuf), "\xFF\xFF\xFF\xFFrcon %c%s%c %s",'"',szPassword,'"',szCmd); //Tested on ET and Cod4 but should work any Quake 3 servers
	
	RCON_WriteToConsole(0, szCmd);
	//AddLogInfo("Sending :%s",sendbuf);
	int len = strlen(sendbuf)+1;

	if(send(RCON_ConnectSocket, sendbuf, (int)len, 0)==SOCKET_ERROR) 
	{
		RCON_Disconnect();
		return 2;
	}

	return 0;
}

void RCON_Disconnect()
{
	//SendMessage(g_hwndRCONOut, LB_ADDSTRING, (WPARAM) 0, (LPARAM) "Disconnected!");
	//AddLogInfo(0,"RCON_Disconnect()");
	if(RCON_ConnectSocket!=NULL)
		closesocket(RCON_ConnectSocket);
	RCON_ConnectSocket=NULL;
}





