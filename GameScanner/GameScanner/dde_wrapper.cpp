
//HDDEDATA CALLBACK DDE_DdeCallback(UINT uType, UINT uFmt,HCONV hconv, HSZ hsz1, HSZ hsz2,HDDEDATA hdata,DWORD dwData1,DWORD dwData2) ;
#include "stdafx.h"
#include "dde_wrapper.h"

DWORD g_DDE_idInst = 0;
HCONV hConv=NULL;         // conversation handle 
HSZ hszServName;     // service name string handle 
HSZ hszSysTopic;     // System topic string handle 
HSZ hszCommand;

BOOL DDE_Send(TCHAR * szMsg)
{
  
   // DdeSend(hConv,szMsg,strlen(szMsg)+1); //"/join #test123"
	
	hszCommand = DdeCreateStringHandle(g_DDE_idInst,szMsg,CP_WINANSI);
	if(hszCommand==0)
	{ 
		MessageBox(NULL, "DdeCreateStringHandle error", (LPSTR) NULL, MB_OK); 
		return FALSE; 
	} 

	DWORD dwRes=0;

	DdeClientTransaction((LPBYTE)szMsg,strlen(szMsg)+1,hConv,hszCommand,CF_TEXT,XTYP_POKE,1000,&dwRes);

	if(dwRes>0)
	{

	}

/*		UINT ret = DdeGetLastError(g_DDE_idInst);
		if(DMLERR_INVALIDPARAMETER==ret)
			MessageBox(NULL, "DMLERR_INVALIDPARAMETER error", (LPSTR) NULL, MB_OK); 
		else if(DMLERR_ADVACKTIMEOUT==ret)
			MessageBox(NULL, "DMLERR_ADVACKTIMEOUT error", (LPSTR) NULL, MB_OK); 
		else if(DMLERR_BUSY==ret)
			MessageBox(NULL, "DMLERR_BUSY error", (LPSTR) NULL, MB_OK); 
		else if(DMLERR_DATAACKTIMEOUT==ret)
			MessageBox(NULL, "DMLERR_DATAACKTIMEOUT error", (LPSTR) NULL, MB_OK); 
		else if(DMLERR_DLL_NOT_INITIALIZED==ret)
			MessageBox(NULL, "DMLERR_DLL_NOT_INITIALIZED error", (LPSTR) NULL, MB_OK); 
		else if(DMLERR_EXECACKTIMEOUT==ret)
			MessageBox(NULL, "DMLERR_EXECACKTIMEOUT error", (LPSTR) NULL, MB_OK); 
		else if(DMLERR_NO_CONV_ESTABLISHED==ret)
			MessageBox(NULL, "DMLERR_NO_CONV_ESTABLISHED error", (LPSTR) NULL, MB_OK); 
		else if(DMLERR_POKEACKTIMEOUT==ret)
			MessageBox(NULL, "DMLERR_POKEACKTIMEOUT error", (LPSTR) NULL, MB_OK); 
		else if(DMLERR_NO_ERROR==ret)
			MessageBox(NULL, "DMLERR_NO_ERROR error", (LPSTR) NULL, MB_OK); 
		else if(DMLERR_POSTMSG_FAILED  ==ret)
			MessageBox(NULL, "DMLERR_POSTMSG_FAILED   error", (LPSTR) NULL, MB_OK); 
		else if(DMLERR_SERVER_DIED  ==ret)
			MessageBox(NULL, "DMLERR_SERVER_DIED   error", (LPSTR) NULL, MB_OK); 

		else if(DMLERR_UNADVACKTIMEOUT==ret)
			MessageBox(NULL, "DMLERR_UNADVACKTIMEOUT error", (LPSTR) NULL, MB_OK); 
		else if(DMLERR_NOTPROCESSED==ret)
			MessageBox(NULL, "DMLERR_NOTPROCESSED error", (LPSTR) NULL, MB_OK); 
		*/

	return TRUE;
}

BOOL DDE_Init()
{

	DdeInitialize(&g_DDE_idInst,         // receives instance identifier 
		(PFNCALLBACK) NULL, // pointer to callback function 
		APPCMD_CLIENTONLY,0); 


hszServName = DdeCreateStringHandle( 
    g_DDE_idInst,         // instance identifier 
    "mIRC",     // string to register 
    CP_WINANSI);    // Windows ANSI code page 
 
hszSysTopic = DdeCreateStringHandle( 
    g_DDE_idInst,         // instance identifier 
    "COMMAND", // System topic 
    CP_WINANSI);    // Windows ANSI code page 
    

	hConv = DdeConnect( 
		g_DDE_idInst,               // instance identifier 
		hszServName,          // service name string handle 
		hszSysTopic,          // System topic string handle 
		(PCONVCONTEXT) NULL); // use default context 
	 
	if (hConv == NULL) 
	{ 
		//MessageBox(NULL, "mIRC is unavailable.", (LPSTR) NULL, MB_OK); 
		return FALSE; 
	} 
	
	return TRUE;
}

void DDE_DeInit()
{
	DdeFreeStringHandle(g_DDE_idInst,hszServName);    
    DdeFreeStringHandle(g_DDE_idInst,hszSysTopic);    

	DdeDisconnect(hConv);
	hConv = NULL;
	DdeUninitialize(g_DDE_idInst);
	g_DDE_idInst = 0;
}

/*
HDDEDATA CALLBACK DDE_DdeCallback(UINT uType, UINT uFmt,HCONV hconv, HSZ hsz1, HSZ hsz2,HDDEDATA hdata,DWORD dwData1,DWORD dwData2) 
{ 
    switch (uType) 
    { 
 
        case XTYP_DISCONNECT: 
            
            //    
			MessageBox(NULL,(LPSTR) "DDE Callback receiving XTYP_DISCONNECT",
             "ERROR",MB_OK|MB_ICONINFORMATION|MB_TASKMODAL);
            
            return (HDDEDATA) NULL; 
 
        default: 
            return (HDDEDATA) NULL; 
    } 
} 
*/