
#include "structs_defines.h"


//RCON functions 
LRESULT APIENTRY RCON_EditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK RCON_Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK RCON_AskPasswordProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
DWORD RCON_Connect(SERVER_INFO *pServer);
DWORD RCON_SendCmd(TCHAR *szPassword,TCHAR *szCmd);
DWORD RCON_Read();
void RCON_Disconnect();