#pragma once

class CGameManager
{
	int m_iGameCounter;
	CLogger & log;
	vFILTER_SETS vFilterSetsGlobal;


public:
	char g_currServerIP[128];
	char g_szMapName[MAX_PATH];

	GamesMap GamesInfo;
	CGameManager(CLogger &logger);
	~CGameManager(void);
	void Default_GameSettings();
	int GetNetEngine(char *szName);
	char *Registry_GetGamePath(HKEY hkey,char *pszRegPath,char *pszRegKey,char *pszOutputString,DWORD *dwSizeOfBuffer);
	void ClearAllGameServer();
	void ClearServerList(int i);
	vFILTER_SETS & GetFilterSet(int GameIdx);
	void ValidateGameIndex(int GameIdx);
	bool SetHTREEITEM(int GameIdx, HTREEITEM hTI);
	UINT Get_GameIcon(UINT GameIndex);
	SERVER_INFO *Get_ServerInfoByIndex(GAME_INFO *pGI,int index);
	void GetServerInfo(int gametype,SERVER_INFO *pSrvInf);
};
