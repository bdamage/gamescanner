#include "stdafx.h"
#include "scanner.h"
#include "utilz.h"
#include "structs_defines.h"
#include "logger.h"
#include "GameManager.h"

#define MAX_THREADS 256

extern APP_SETTINGS_NEW AppCFG;
extern HWND g_hwndProgressBar;
extern HWND g_hWnd;
extern CLanguage g_lang;
extern long UpdateServerItem(DWORD index);
extern CLogger log;
extern CGameManager gm;

LPSERVERINFO		SCANNER_pSI_rescan = NULL;
extern CRITICAL_SECTION	SCANNER_cs,SCANNER_CSthreadcounter; 
DWORD				SCANNER_dwThreadCounter=0;
HANDLE SCAN_hContinueEvent;  //This here to help to close all events in a gracefull way

BOOL SCANNER_bCloseApp=FALSE;
BOOL SCANNER_bScanningInProgress=FALSE;



long (*SCANNER_UpdateServerListView)(DWORD idx);

DWORD (*Get_ServerStatus)(SERVER_INFO *pSI,long (*UpdatePlayerListView)(LPPLAYERDATA pPlayers),long (*UpdateRulesListView)(LPSERVER_RULES pServerRules));

//bool (*SCAN_FilterServerItem)(SERVER_INFO *lp,GAME_INFO *pGI, vFILTER_SETS *vFilterSets);


void SCAN_Set_CALLBACKS(DWORD (*_Get_ServerStatus)(SERVER_INFO *pSI, long (*UpdatePlayerListView)(LPPLAYERDATA pPlayers),long (*UpdateRulesListView)(LPSERVER_RULES pServerRules)),
				   long (*_UpdateServerListView)(DWORD idx)
				   )
{
	Get_ServerStatus = _Get_ServerStatus;

}


extern bool Sort_Filter_By_GroupName(FILTER_SET fsA, FILTER_SET fsB);

//Create scanning threads
void Initialize_Rescan2(GAME_INFO *pGI, bool (*filterServerItem)(SERVER_INFO *lp,GAME_INFO *pGI, vFILTER_SETS *vFilterSets))
{
	log.AddLogInfo(GS_LOG_DEBUG,"Entering Initialize_Rescan2 function.");
	//SCAN_FilterServerItem = filterServerItem;

	vFILTER_SETS vFS;
	vFS = pGI->vFilterSets;
	vFS.insert(vFS.end(),gm.GetFilterSet(GLOBAL_FILTER).begin(),gm.GetFilterSet(GLOBAL_FILTER).end());
	sort(vFS.begin(),vFS.end(),Sort_Filter_By_GroupName);

	vSRV_INF::iterator  iLst;
	pGI->vRefScanSI.clear();

	pGI->dwViewFlags  |= SCAN_SERVERLIST;

	for ( iLst = pGI->vSI.begin( ); iLst != pGI->vSI.end( ); iLst++ )
	{	
		SERVER_INFO *pSI = *iLst;
		REF_SERVER_INFO refSI;
		refSI.pServerInfo = pSI;
		if(filterServerItem!=NULL)
		{
			if(filterServerItem(pSI,pGI,&vFS))
			{
				pGI->vRefScanSI.push_back(refSI);		
			}
		}
		else
		{
			pGI->vRefScanSI.push_back(refSI);  		
		}
	}
	if(pGI->dwViewFlags & SCAN_SERVERLIST)
		pGI->dwViewFlags ^= SCAN_SERVERLIST;

	log.AddLogInfo(GS_LOG_INFO,"Preparing to scan %d servers of a total %d.\n",pGI->vRefScanSI.size(),pGI->vSI.size());

	if(pGI->dwViewFlags & FORCE_SCAN_FILTERED)
		pGI->dwViewFlags = 0;

	if(g_hwndProgressBar!=NULL)
	{
		//Initililze Progressbar
		SendMessage(g_hwndProgressBar, PBM_SETSTEP, (WPARAM) 1, 0); 
		SendMessage(g_hwndProgressBar, PBM_SETRANGE, (WPARAM) 0,MAKELPARAM(0,pGI->vRefScanSI.size())); 
		SendMessage(g_hwndProgressBar, PBM_SETPOS, (WPARAM) 0, 0); 
	}

	HANDLE hThreadIndex[MAX_THREADS];
	DWORD dwThreadId[MAX_THREADS];
	
	for(DWORD i=0; i<AppCFG.dwThreads;i++)
		hThreadIndex[i]=NULL;

	SCANNER_dwThreadCounter=0;
	pGI->dwScanIdx = 0;
	
	//Create and setup Continue Event, this is important! Otherwise ETSV can crash.
	SCAN_hContinueEvent = CreateEvent(NULL,TRUE,TRUE,"ScanContinueEvent"); 
    if (SCAN_hContinueEvent == NULL) 
        log.AddLogInfo(GS_LOG_DEBUG,"CreateEvent failed (%d)\n", GetLastError());

	if (! ResetEvent(SCAN_hContinueEvent) ) 
        log.AddLogInfo(GS_LOG_DEBUG,"ResetEvent failed (%d)\n", GetLastError());

	DWORD dwMaxThreads = (AppCFG.dwThreads>pGI->vRefScanSI.size())?pGI->vRefScanSI.size():AppCFG.dwThreads;

	//---------------------------------
	//Multi thread scanning code
	//---------------------------------
	//Startup the threads!!!
	for (DWORD i=0; i<dwMaxThreads;i++)
	{
		hThreadIndex[SCANNER_dwThreadCounter] = CreateThread( NULL, 0, &Get_ServerStatusThread2, (LPVOID) pGI ,0, &dwThreadId[SCANNER_dwThreadCounter]);  
	
		if (hThreadIndex[SCANNER_dwThreadCounter] == NULL) 
		{
			dbg_print("Error creating thread!\n");			
		}
		else 
		{	
			//log.AddLogInfo(GS_LOG_INFO,"Thread created! %d",i);					
			EnterCriticalSection( &SCANNER_CSthreadcounter ); 
			SCANNER_dwThreadCounter++;  
			LeaveCriticalSection( &SCANNER_CSthreadcounter ); 
			//SetThreadName(dwThreadId[SCANNER_dwThreadCounter], "Get_ServerStatusThread2");
		} 
	}
		
	//All threads created in graceful way and counter increased properly
	if (! SetEvent(SCAN_hContinueEvent) ) 
		dbg_print("SetEvent failed\n");
	//After this this the thread counter can decrease properly with a noncorrupted handle
	DWORD dwStartTick = GetTickCount();
	DWORD iWaitIndex = 0;
	DWORD i=0;
	//Wait for all threads to finish...
	//log.AddLogInfo(GS_LOG_DEBUG,"AppCFG.dwThreads %d",AppCFG.dwThreads);

	while(iWaitIndex<dwMaxThreads)
	{
	
		DWORD max = ((dwMaxThreads-iWaitIndex)<MAXIMUM_WAIT_OBJECTS)?(dwMaxThreads-iWaitIndex):MAXIMUM_WAIT_OBJECTS;		

	//	log.AddLogInfo(GS_LOG_DEBUG,"iWaitIndex: %d, iWaitIndex+max: %d, dwMaxThreads: %d,  max:%d",iWaitIndex,iWaitIndex+max,dwMaxThreads,max);
		DWORD dwEvent = WaitForMultipleObjects(max, &hThreadIndex[iWaitIndex], TRUE, INFINITE);
		//log.AddLogInfo(GS_LOG_DEBUG,">iWaitIndex: %d, iWaitIndex+max: %d, dwMaxThreads: %d, nThreads: %d, max:%d",iWaitIndex,iWaitIndex+max,dwMaxThreads, nThreads,max);
	
		switch (dwEvent) 
		{
			case WAIT_OBJECT_0: 
				//dbg_print("First event was signaled.\n");
				break; 
			case WAIT_TIMEOUT:
				log.AddLogInfo(GS_LOG_DEBUG,"Wait timed out. @ %s",__FUNCTION__);	
				break;
			// Return value is invalid.
			default: 
				{
					log.AddLogInfo(GS_LOG_DEBUG,"Error at waiting threads! %s",__FUNCTION__);				
				}         
		}


		// Close all thread handles upon completion.
		for(i=iWaitIndex; i<iWaitIndex+max; i++)
		{			
			if(hThreadIndex[i]!=NULL)
			{
				CloseHandle(hThreadIndex[i]);
				hThreadIndex[i]=NULL;
			}
		}
		iWaitIndex+=MAXIMUM_WAIT_OBJECTS;
	}
	DWORD dwEndTick = GetTickCount();
	log.AddLogInfo(0,"All servers is now scanned...%d sec",(dwEndTick-dwStartTick)/1000);
	pGI->vRefScanSI.clear();
	CloseHandle(SCAN_hContinueEvent);
}



DWORD WINAPI  Get_ServerStatusThread2(LPVOID lpParam)
{
	GAME_INFO *pGI = (GAME_INFO *)lpParam;
	SERVER_INFO *pSI=NULL;
	DWORD idx=0;
	DWORD size = pGI->vRefScanSI.size();

	char szScanStatus[256];  //cache local language status text
	strcpy(szScanStatus,g_lang.GetString("ScanStatus"));

	while(pGI->dwScanIdx<size)
	{			
		if(SCANNER_bCloseApp)
		{
			dbg_print("Stop scanning SIGNALED!");
			break;
		}
		
		pSI=NULL;

		EnterCriticalSection(&SCANNER_cs);
		if(pGI->dwScanIdx<size)
		{
			SetStatusText(pGI->iIconIndex,szScanStatus,pGI->dwScanIdx,size);
			pSI = pGI->vRefScanSI.at(pGI->dwScanIdx).pServerInfo; //pGI->vSI.at(pGI->vRefScanSI.at(pGI->dwScanIdx).dwIndex);
			pGI->dwScanIdx++;
		}
		LeaveCriticalSection(&SCANNER_cs);
		
		//Is there any more server to scan?
		if(pSI==NULL) //if(pSI->usPort==0) //if the port is zero then no equal empty SERVER_INFO structure, (Ugly hack but  it works :))
		{
			//OutputDebugString(">>>>ERROR? Breaked scanning thread\n");
			break;
		}

		//Do non-filtered scan of all servers
		Get_ServerStatus(pSI,NULL,NULL);			

		//if(SCANNER_UpdateServerListView!=NULL)
			UpdateServerItem(pSI->dwLVIndex);

		if(g_hwndProgressBar!=NULL)
				SendMessage(g_hwndProgressBar, PBM_STEPIT, (WPARAM) 0, 0);
			
		Sleep(AppCFG.dwSleep);				
	}

	SetStatusText(pGI->iIconIndex, g_lang.GetString("ScanWaitingForThreads") );
	
	//This ensures that all threads has been created properly and thread count critical sections works correctly
	//dbg_print("Waiting for all threads to finish the loop!\n");
	DWORD dwWaitResult = WaitForSingleObject(SCAN_hContinueEvent,INFINITE);    // infinite wait
	switch (dwWaitResult) 
	{
		// Both event objects were signaled.
		case WAIT_OBJECT_0: 
		//	dbg_print("WAIT_OBJECT_0:\n"); 
			break; 
		case WAIT_ABANDONED:
			dbg_print("WAIT_ABANDONED: \n"); 
			break;
		case WAIT_TIMEOUT:
			dbg_print("WAIT_TIMEOUT:\n"); 
			break;
		case WAIT_FAILED:
			dbg_print("WAIT_FAILED:\n"); 
			break;

		// An error occurred.
		default: 
			dbg_print("WaitForSingleObject error: \n");        
	}

	DWORD id=0;
	//Decrease thread counter....
	EnterCriticalSection( &SCANNER_CSthreadcounter ); 
	id = SCANNER_dwThreadCounter;
	SCANNER_dwThreadCounter--;  
	LeaveCriticalSection( &SCANNER_CSthreadcounter ); 
	
//	ExitThread(id);

	return id;
}


