#pragma once

#include "logger.h"
#include "gamemanager.h"
#include "language.h"



struct BUDDY_INFO
{
	DWORD dwID;
	char szPlayerName[MAX_NAME_LEN];
	char szClan[MAX_NAME_LEN];
	char szServerName[MAX_NAME_LEN];
	char szIPaddress[MAX_IP_LEN];
	int cMatchExact;
	int cMatchOnColorEncoded;
	char cGAMEINDEX;
	char cUnused1;
	int sIndex;
	bool bRemove;
	SERVER_INFO *pSI;
	char szLastSeenServerName[MAX_NAME_LEN];
	char szLastSeenIPaddress[MAX_IP_LEN];
	bool bXMPP;
	bool operator == (DWORD id) {return dwID == id;}

};

typedef BUDDY_INFO* LPBUDDY_INFO;

typedef vector<BUDDY_INFO> vecBI; 

bool Buddy_Sort_Name(BUDDY_INFO BIa, BUDDY_INFO BIb);
bool Buddy_Sort_ServerName(BUDDY_INFO BIa, BUDDY_INFO BIb);
long Buddy_CheckForBuddies(PLAYERDATA *pPlayers, SERVER_INFO *pServerInfo);
LRESULT APIENTRY Buddy_ListViewSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


class CBuddyManager
{

	CLanguage		&m_lang;
	CLogger			&m_log;
	CGameManager	&gm;
public:
	
	vecBI BuddyList;

	CBuddyManager(CLogger & logger, CGameManager & _gm, CLanguage &_lang);
	~CBuddyManager(void);
	void Clear();	
	void OnAddSelectedPlayerToBuddyList();
	DWORD GetBuddyIDBySelection();
	vecBI::iterator FindBuddyInfoByID(DWORD dwID);
	BOOL Add(const char *szName,SERVER_INFO *pServer, bool bIsXMPPBuddy=false);
	void Remove();
	void Buddy_Add(HINSTANCE hInst,HWND hWnd, bool manually);
	void UpdateList();
	void OnBuddySelected();
	void Save();
	int  Load();
	void NotifyBuddyIsOnline(BUDDY_INFO *pBI, SERVER_INFO *pServerInfo);
	void NotifyBuddyIsOffline(BUDDY_INFO *pBI, SERVER_INFO *pServerInfo);
};
