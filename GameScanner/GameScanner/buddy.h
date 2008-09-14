
#ifndef __BUDDY_H__
#define __BUDDY_H__

#include "structs_defines.h"

//Buddy functions
void Buddy_Add(bool manually);					
BUDDY_INFO *Buddy_AddToList(LPBUDDY_INFO &pBI,TCHAR *szName,SERVER_INFO *pServer);
void Buddy_UpdateList(BUDDY_INFO *pBI);
void Buddy_Remove();
void Buddy_ChangeName(TCHAR *newname);
long Buddy_CheckForBuddies(PLAYERDATA *pPlayers, SERVER_INFO *pServerInfo);
void Buddy_InsertToList(TCHAR* szPlayerName,TCHAR *szPlayerColorEncoded);
void Buddy_Save(BUDDY_INFO *pBI);
int Buddy_Load(LPBUDDY_INFO &pBI);
void Buddy_Clear(LPBUDDY_INFO pBI);
LRESULT APIENTRY Buddy_ListViewSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Buddy_AddBuddyProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void OnAddSelectedPlayerToBuddyList();

#endif