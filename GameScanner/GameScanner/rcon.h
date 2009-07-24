
#include "structs_defines.h"


//RCON functions 
LRESULT APIENTRY RCON_EditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK RCON_Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK RCON_AskPasswordProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
DWORD RCON_Connect(SERVER_INFO *pServer);
DWORD RCON_SendCmd(SERVER_INFO* pSI,char *szPassword,char *szCmd);
DWORD RCON_Read(SERVER_INFO *pServer);
void RCON_Disconnect();