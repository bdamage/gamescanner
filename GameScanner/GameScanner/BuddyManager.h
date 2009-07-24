#pragma once

#include "logger.h"
#include "gamemanager.h"
#include "language.h"

bool Buddy_Sort_Name(BUDDY_INFO BIa, BUDDY_INFO BIb);
bool Buddy_Sort_ServerName(BUDDY_INFO BIa, BUDDY_INFO BIb);
long Buddy_CheckForBuddies(PLAYERDATA *pPlayers, SERVER_INFO *pServerInfo);
LRESULT APIENTRY Buddy_ListViewSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


class CBuddyManager
{

	CLanguage		&m_lang;
	CLogger			&log;
	CGameManager	&gm;
public:
	
	vecBI BuddyList;

	CBuddyManager(CLogger & logger, CGameManager & _gm, CLanguage &_lang);
	~CBuddyManager(void);
	void Clear();	
	void OnAddSelectedPlayerToBuddyList();
	DWORD GetBuddyIDBySelection();
	vecBI::iterator FindBuddyInfoByID(DWORD dwID);
	BOOL Add(char *szName,SERVER_INFO *pServer);
	void Remove();
	void Buddy_Add(HINSTANCE hInst,HWND hWnd, bool manually);
	void UpdateList();
	void OnBuddySelected();
	void Save();
	int  Load();
	void NotifyBuddyIsOnline(BUDDY_INFO *pBI, SERVER_INFO *pServerInfo);
	void NotifyBuddyIsOffline(BUDDY_INFO *pBI, SERVER_INFO *pServerInfo);
};
