
#ifndef __BUDDY_H__
#define __BUDDY_H__

#include "structs_defines.h"

//Buddy functions
void Buddy_Add(bool manually);					
BOOL Buddy_AddToList(TCHAR *szName,SERVER_INFO *pServer);
void Buddy_UpdateList();
void Buddy_Remove();
long Buddy_CheckForBuddies(PLAYERDATA *pPlayers, SERVER_INFO *pServerInfo);
void Buddy_InsertToList(TCHAR* szPlayerName,TCHAR *szPlayerColorEncoded);
void Buddy_Save();
int Buddy_Load();
void Buddy_Clear();
LRESULT APIENTRY Buddy_ListViewSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Buddy_AddBuddyProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void OnAddSelectedPlayerToBuddyList();
vecBI::iterator Buddy_FindBuddyInfoByID(DWORD dwID);
bool Buddy_Sort_Name(BUDDY_INFO BIa, BUDDY_INFO BIb);
bool Buddy_Sort_ServerName(BUDDY_INFO BIa, BUDDY_INFO BIb);

#endif