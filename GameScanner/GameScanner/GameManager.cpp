#include "StdAfx.h"
#include "GameManager.h"


extern TCHAR EXE_PATH[_MAX_PATH+_MAX_FNAME];
extern int LoadIconIntoImageList(char*szFilename);
extern LRESULT Draw_ColorEncodedText(RECT rc, LPNMLVCUSTOMDRAW pListDraw , TCHAR *pszText);
extern LRESULT Draw_ColorEncodedTextQ4(RECT rc, LPNMLVCUSTOMDRAW pListDraw , TCHAR *pszText);
extern LRESULT Draw_ColorEncodedTextQW(RECT rc, LPNMLVCUSTOMDRAW pListDraw , TCHAR *pszText);
extern LRESULT Draw_ColorEncodedTextNexuiz(RECT rc, LPNMLVCUSTOMDRAW pListDraw , TCHAR *pszText);
extern LRESULT Draw_ColorEncodedTextUNICODE(RECT rc, LPNMLVCUSTOMDRAW pListDraw , TCHAR *pszText);
extern LRESULT Draw_UTF8Text(RECT rc, LPNMLVCUSTOMDRAW pListDraw , char *pszText);
extern HWND g_hWnd;
extern APP_SETTINGS_NEW AppCFG;
extern long UpdatePlayerList(LPPLAYERDATA pPlayers);
extern long UpdateRulesList(LPSERVER_RULES pServerRules);
extern SERVER_INFO *g_CurrentSRV;
extern _WINDOW_CONTAINER WNDCONT[15];

CGameManager::CGameManager(CLogger & logger) : 
	m_log(logger),
	m_iGameCounter (0)
{
}

CGameManager::~CGameManager(void)
{
}


void CGameManager::Default_GameSettings()
{
	GAME_INSTALLATIONS gi;
	//g_log.AddLogInfo(GS_LOG_INFO,"Enter Default_GameSettings");

	for(UINT i=0; i<GamesInfo.size(); i++)
	{

		GamesInfo[i].nMasterServers = 0;
		GamesInfo[i].dwViewFlags = 0;
		GamesInfo[i].vGAME_INST.clear();
		GamesInfo[i].bLockServerList = FALSE;
		GamesInfo[i].filter.dwRegion = 0;
		GamesInfo[i].hTI = NULL;
	}

	CXmlFile xml;
	xml.SetPath(EXE_PATH);	
	if(xml.load("gamedefaults.xml")==XMLFILE_ERROR_LOADING)
	{
		m_log.AddLogInfo(GS_LOG_ERROR,"Error reading gamedefaults.xml.");
		return;
	}
	//TixmlElement *ptempElement = xml.GetElementSafe(xml.m_pRootElement,"Version");
	char szVersion[10];
	xml.GetText(xml.m_pRootElement,"Version",szVersion,sizeof(szVersion));
	
	m_log.AddLogInfo(GS_LOG_INFO,"Default game settings version %s",szVersion);

	TiXmlElement *pGame = xml.GetElementSafe(xml.m_pRootElement,"Game");

	while(pGame!=NULL)
	{
		char szTemp[100];
		GAME_INFO gameinfo;
		gameinfo.bLockServerList = FALSE;
		gameinfo.bActive=0;
		gameinfo.dwTotalServers=0;
		gameinfo.dwScanIdx = 0;
		gameinfo.dwViewFlags = 0;
		gameinfo.nMasterServers = 0;
		gameinfo.filter.dwRegion = 0;
		gameinfo.hTI = NULL;
		
		ZeroMemory(&gameinfo.filter,sizeof(gameinfo.filter));
		ZeroMemory(&gameinfo.szGAME_NAME,sizeof(gameinfo.szGAME_NAME));
		ZeroMemory(&gameinfo.szMasterQueryString,sizeof(gameinfo.szMasterQueryString));
		ZeroMemory(&gameinfo.szMasterServerIP[0],sizeof(gameinfo.szMasterServerIP[0]));
		ZeroMemory(&gameinfo.szMAP_YAWN_PATH,sizeof(gameinfo.szMAP_YAWN_PATH));
		
		for(int x=0; x<MAX_MASTERS_SERVERS; x++)
		{
			gameinfo.bUseHTTPServerList[x] = FALSE;
			ZeroMemory(gameinfo.szMasterServerIP[x],sizeof(gameinfo.szMasterServerIP[x]));
			ZeroMemory(gameinfo.szGameProtocol[x],sizeof(gameinfo.szGameProtocol[x]));
		}



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
		xml.GetText(pGame,"WebProtocol",gameinfo.szWebProtocolName,sizeof(gameinfo.szWebProtocolName)-1);
		xml.GetInteger(pGame,"ServerDefaultPort",(long*)&gameinfo.dwDefaultPort);
		char szColorEnc[40];
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
		} else if(strcmp(szColorEnc,"NEXUIZ")==0)
		{
			gameinfo.colorfilter = &colorfilterNEXUIZ;
			gameinfo.Draw_ColorEncodedText = &Draw_ColorEncodedTextNexuiz;
		}else if(strcmp(szColorEnc,"JEDIKNIGHT3")==0)
		{
			gameinfo.colorfilter = &colorfilterJK3;
			gameinfo.Draw_ColorEncodedText = &Draw_ColorEncodedTextJK3;
		}
		


		xml.GetText(pGame,"MapPreviewPath",gameinfo.szMAP_MAPPREVIEW_PATH,sizeof(gameinfo.szMAP_MAPPREVIEW_PATH)-1);
		xml.GetText(pGame,"ServerInfoQuery",gameinfo.szServerRequestInfo,sizeof(gameinfo.szServerRequestInfo)-1);
		xml.GetText(pGame,"MasterQueryString",gameinfo.szMasterQueryString,sizeof(gameinfo.szMasterQueryString)-1);


		TiXmlElement *ptempMasters = xml.GetElementSafe(pGame,"MasterServers");
		//xml.GetInteger(ptempMasters,"UseHTTP",(long*)&gameinfo.bUseHTTPServerList);
		
		TiXmlElement* pMaster = xml.GetElementSafe(ptempMasters,"MasterServer"); 
		int idx=0;
		while(pMaster!=NULL)
		{
			if(xml.GetText(pMaster,"MasterServer",gameinfo.szMasterServerIP[idx],sizeof(gameinfo.szMasterServerIP[idx])-1)==TIXML_SUCCESS)
			{				
				if(xml.GetCustomAttribute(pMaster,"http",szTemp,sizeof(szTemp)-1)!=NULL)
					gameinfo.bUseHTTPServerList[idx] = atoi(szTemp);
			
				xml.GetCustomAttribute(pMaster,"protocol",gameinfo.szGameProtocol[idx],sizeof(gameinfo.szGameProtocol[idx])-1);		
			
				idx++;
				gameinfo.nMasterServers++;
			}
			if(idx>MAX_MASTERS_SERVERS)
				break;
	
			pMaster = pMaster->NextSiblingElement();
		}

		//if(gameinfo.bUseHTTPServerList[0]==FALSE)
		//	SplitIPandPORT(gameinfo.szMasterServerIP[0],gameinfo.dwMasterServerPORT);

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
		gameinfo.bActive = FALSE;
		int len = strlen(szPath);
		int len2 = strlen(szInstallSuffix);
		if((len>0) && (len2>0)) //should we add suffix into game path?
		{
			gameinfo.bActive = TRUE;
			strcat_s(szPath,sizeof(szPath),szInstallSuffix);
		}else if(len>0)
			gameinfo.bActive = TRUE;

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
		/*
			xml.GetAttribute(pInstall,"LaunchByMod",szTemp,sizeof(szTemp)-1);
			gi.sMod = szTemp;
			xml.GetAttribute(pInstall,"LaunchByVer",szTemp,sizeof(szTemp)-1);
			gi.sVersion = szTemp;
			*/
			if(gameinfo.bActive)
				gameinfo.vGAME_INST.push_back(gi);

			gameinfo.vGAME_INST_DEFAULT.push_back(gi); //This is used for config

			ptempInstalls = ptempInstalls->NextSiblingElement();
			if(ptempInstalls==NULL)
				break;		
		}

		TiXmlElement *ptempCols = xml.GetElementSafe(pGame,"Columns");

		while(ptempCols!=NULL)
		{
		
			GAME_SPECIFIC_COLUMNS gsc;

			TiXmlElement *pCol = xml.GetElementSafe(ptempCols,"Column");
			xml.GetText(pCol,"Column",szTemp,sizeof(szTemp)-1);
			gsc.sRuleValue = szTemp;
			xml.GetAttribute(pCol,"idx",szTemp,sizeof(szTemp)-1);
			gsc.iColumnIdx = atoi(szTemp);

			gameinfo.vGAME_SPEC_COL.push_back(gsc); //This is used for config

			ptempCols = pCol->NextSiblingElement();
			if(ptempCols==NULL)
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
			case COD4_ENGINE:
				{
					gameinfo.GetServersFromMasterServer = &Q3_ConnectToMasterServer;
					gameinfo.GetServerStatus = &COD4_Get_ServerStatus;
					break;
				}
			
			case VALVE_ENGINE:
				{
					gameinfo.colorfilter = colorfilterUTF8;
					gameinfo.Draw_ColorEncodedText = &Draw_UTF8Text;
					gameinfo.GetServersFromMasterServer = &STEAM_ConnectToMasterServer;
					gameinfo.GetServerStatus = &STEAM_Get_ServerStatus;
					break;
				}
			case GAMESPYv4_ENGINE:
				{
					
					gameinfo.Draw_ColorEncodedText = &Draw_ColorEncodedText;
					gameinfo.GetServersFromMasterServer = NULL;
					gameinfo.GetServerStatus = &UT_Get_ServerStatus;
				}
				break;
			case WOLF_ENGINE:
				{
				
					gameinfo.Draw_ColorEncodedText = &Draw_ColorEncodedText;
					gameinfo.GetServersFromMasterServer = &GSC_ConnectToMasterServer;
					gameinfo.GetServerStatus = &Q4_Get_ServerStatus;
					break;
				}
			case CODWW_ENGINE:
				{
					gameinfo.Draw_ColorEncodedText = &Draw_ColorEncodedText;				
					gameinfo.GetServersFromMasterServer = &GSC_ConnectToMasterServer;
					gameinfo.GetServerStatus = &Q3_Get_ServerStatus;
					break;
				}
			case BFBC2_ENGINE:
				{
					gameinfo.Draw_ColorEncodedText = &Draw_ColorEncodedText;				
					gameinfo.GetServersFromMasterServer = &EA_ConnectToMasterServer;
					gameinfo.GetServerStatus = &BFBC2_Get_ServerStatus;
					break;
				}
		}
		GamesInfo[gameinfo.cGAMEINDEX] = gameinfo;
		pGame = pGame->NextSiblingElement();
		if(pGame==NULL)
			break;		
	}



}

UINT CGameManager::Get_GameIcon(UINT GameIndex)
{
	return GamesInfo[GameIndex].iIconIndex;
}

/***************************************************
	Set up default settings for each game.
****************************************************/
int CGameManager::GetNetEngine(char *szName)
{
	if(strcmp("Q3",szName)==0)
		return Q3_ENGINE;
	else if(strcmp("Q4",szName)==0)
		return Q4_ENGINE;
	else if(strcmp("VALVE",szName)==0)
		return VALVE_ENGINE;
	else if(strcmp("GS4",szName)==0)
		return GAMESPYv4_ENGINE;
	else if(strcmp("COD4",szName)==0)
		return COD4_ENGINE;
	else if(strcmp("WOLF",szName)==0)
		return WOLF_ENGINE;
	else if(strcmp("CODWW",szName)==0)
		return CODWW_ENGINE;	
	else if(strcmp("BFBC2",szName)==0)
		return BFBC2_ENGINE;

	return Q3_ENGINE;  //Q3 as default
}


/************************************************
 Will output the result into pszOutputString and size in bytes
 Unsuccefull result equals to NULL
**************************************************/
char *CGameManager::Registry_GetGamePath(HKEY hkey,char *pszRegPath,char *pszRegKey,char *pszOutputString,DWORD *dwSizeOfBuffer)
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

void CGameManager::ClearAllGameServer()
{
	OutputDebugString(" - Cleaning up serverlist, playerlist and hashes etc.\n");

	for(int i=0;i<GamesInfo.size();i++)
		ClearServerList(i);	
}


long CGameManager::GetIndexByHashValue(int GameIdx, int hash,DWORD dwIP, DWORD dwPort)
{
	hash_multimap <int, int>::iterator hmp_Iter;
	
	hmp_Iter = GamesInfo[GameIdx].shash.find(hash);
	while(hmp_Iter!= GamesInfo[GameIdx].shash.end())
	{
		Int_Pair idx = *hmp_Iter;		
		SERVER_INFO  *pSI = (SERVER_INFO*) GamesInfo[GameIdx].vSI.at(idx.second);
		if((dwIP == pSI->dwIP) && (dwPort == pSI->usQueryPort))
			return idx.second;
		hmp_Iter++;
	}
	return -1;
}


/************************************************************ 
	Check if the server exsist in the current view list.

	This function is not multithread safe!!
*************************************************************/
long CGameManager::CheckForDuplicateServer(int GameIdx, SERVER_INFO *pSI)
{
	vSRV_INF::iterator  iResult;
	SERVER_INFO *pSrv = NULL;
	int hash = pSI->dwIP + pSI->usQueryPort;

	return GetIndexByHashValue(GameIdx,  hash,pSI->dwIP,pSI->usQueryPort);
}

/*
  returns 0xFFFFFFFF (-1), if IP is not satisfied 
  if adding a IP and Favorite=false then if exsistent return 0xFFFFFFFF otherwise the new index
  if adding a IP and favorite=true and it exisist set server as favorite and return the current index

 */

long CGameManager::AddServer(int GameIdx, char *szIP, unsigned short usPort,bool bFavorite)
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
	

	pSI->cGAMEINDEX = GameIdx;
	strcpy(pSI->szIPaddress,szIP);
	pSI->dwIP = NetworkNameToIP(szIP,_itoa(usPort,destPort,10));
	pSI->usPort = usPort;
	pSI->usQueryPort = usPort;

	int iResult = CheckForDuplicateServer(GameIdx,pSI);
	if(iResult!=-1) //did we get an exsisting server?
	{
		 //If yes then set that server to a favorite
		if(bFavorite)
			GamesInfo[GameIdx].vSI[iResult]->cFavorite = 1;
		else
		{
			free(pSI);
			return 0xFFFFFFFF;
		}
		free(pSI);
		return GamesInfo[GameIdx].vSI[iResult]->dwIndex;
	}
		
	//Add a new server into current list!
	InitializeCriticalSection(&pSI->csLock);
	pSI->dwPing = 9999;
	strcpy(pSI->szShortCountryName,"zz");
	pSI->bUpdated = 0;
	
	pSI->dwIndex = GamesInfo[GameIdx].vSI.size();
	if(bFavorite)
		pSI->cFavorite = 1;
	
	int hash = pSI->dwIP + pSI->usPort;
	GamesInfo[GameIdx].shash.insert(Int_Pair(hash,pSI->dwIndex));
	GamesInfo[GameIdx].vSI.push_back(pSI);
	
	

	InsertServerItem(&GamesInfo[GameIdx],pSI);
		
	return pSI->dwIndex;
}


void CGameManager::ClearServerList(int GameIdx)
{
	SERVER_INFO *pSrv = NULL;
	GamesInfo[GameIdx].vRefListSI.clear();
	GamesInfo[GameIdx].vRefScanSI.clear();
	for(long x=0; x<GamesInfo[GameIdx].vSI.size();x++)
	{
		CleanUp_PlayerList(GamesInfo[GameIdx].vSI.at(x)->pPlayerData);
		CleanUp_ServerRules(GamesInfo[GameIdx].vSI.at(x)->pServerRules);
		DeleteCriticalSection(&GamesInfo[GameIdx].vSI.at(x)->csLock);
		pSrv = GamesInfo[GameIdx].vSI.at(x);
		if(pSrv!=NULL)
			free(pSrv);
		pSrv = NULL;
	}

	GamesInfo[GameIdx].dwTotalServers = 0;
	GamesInfo[GameIdx].vSI.clear();

	GamesInfo[GameIdx].shash.clear();
}

vFILTER_SETS & CGameManager::GetFilterSet(int GameIdx)
{
	if(GameIdx==GLOBAL_FILTER)
		return vFilterSetsGlobal;

	try
	{
		ValidateGameIndex(GameIdx);
	}catch(int a)
	{
		MessageBox(NULL,"Game idx out of bounds!","Error",MB_OK);
		DebugBreak();
		throw 2;
	}

	return GamesInfo[GameIdx].vFilterSets;
}

void CGameManager::ValidateGameIndex(int GameIdx)
{
	if(GameIdx<0)
		throw 1;
	if(GameIdx>GamesInfo.size())
		throw 2;
}
bool CGameManager::SetHTREEITEM(int GameIdx, HTREEITEM hTI)
{
	try
	{
		ValidateGameIndex(GameIdx);
	}catch(int a)
	{
		MessageBox(NULL,"Game idx out of bounds!","Error",MB_OK);
		DebugBreak();
		throw 2;
	}
	GamesInfo[GameIdx].hTI = hTI;
	return false;
}

SERVER_INFO *CGameManager::Get_ServerInfoByIndex(GAME_INFO *pGI,int index)
{	
	SERVER_INFO *srv;

	__try{

		srv = pGI->vSI.at(index);
	}
	__except(EXCEPTION_ACCESS_VIOLATION == GetExceptionCode())
	{
		OutputDebugString("Access violation...@ Get_ServerInfoByListViewIndex");		
		return NULL;
	}

	return srv;
}

/*************************
	Retrieve server info
***************************/
void CGameManager::GetServerInfo(int gametype,SERVER_INFO *pSrvInf)
{
	g_CurrentSRV = NULL;
	if(pSrvInf==NULL)
		return;

	strcpy_s(g_currServerIP,pSrvInf->szIPaddress);
	if(pSrvInf->szMap!=NULL)
	{
		strncpy_s(g_szMapName,sizeof(g_szMapName),pSrvInf->szMap,_TRUNCATE);
		RECT rc;
		SetRect(&rc,  WNDCONT[WIN_MAPPREVIEW].rSize.left, WNDCONT[WIN_MAPPREVIEW].rSize.top, WNDCONT[WIN_MAPPREVIEW].rSize.left+WNDCONT[WIN_MAPPREVIEW].rSize.right, WNDCONT[WIN_MAPPREVIEW].rSize.top+WNDCONT[WIN_MAPPREVIEW].rSize.bottom);
		InvalidateRect(g_hWnd,&rc, TRUE);

	}
	//dwCurrPort = pSrvInf->usPort;	
	g_CurrentSRV = pSrvInf;
	
	if(pSrvInf->cGAMEINDEX==-1)  //safety check since v1.25
		pSrvInf->cGAMEINDEX = 0;


	GamesInfo[gametype].GetServerStatus(pSrvInf,&UpdatePlayerList,&UpdateRulesList);
}
