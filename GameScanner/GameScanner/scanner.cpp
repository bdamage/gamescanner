#include "stdafx.h"
#include "scanner.h"
#include "utilz.h"

#define MAX_THREADS 256

extern APP_SETTINGS_NEW AppCFG;
extern HWND g_hwndProgressBar;
extern HWND g_hWnd;

LPSERVERINFO		SCANNER_pSI_rescan = NULL;
extern CRITICAL_SECTION	SCANNER_cs,SCANNER_CSthreadcounter; 
DWORD				SCANNER_dwThreadCounter=0;
HANDLE SCAN_hContinueEvent;  //This here to help to close all events in a gracefull way

BOOL SCANNER_bCloseApp=FALSE;
BOOL SCANNER_bScanningInProgress=FALSE;


long (*SCANNER_UpdateServerListView)(DWORD idx);

DWORD (*Get_ServerStatus)(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *Q3players),long (*UpdateRulesListView)(SERVER_RULES *pServerRules));

bool (*SCAN_FilterServerItem)(LPARAM *lp,GAME_INFO *pGI);


void SCAN_Set_CALLBACKS(DWORD (*_Get_ServerStatus)(SERVER_INFO *pSI, long (*UpdatePlayerListView)(PLAYERDATA *Q3players),long (*UpdateRulesListView)(SERVER_RULES *pServerRules)),
				   long (*_UpdateServerListView)(DWORD idx)
				   )
{
	Get_ServerStatus = _Get_ServerStatus;
	SCANNER_UpdateServerListView = _UpdateServerListView;
}

GAME_INFO *ppGI;
void PollForNewServers()
{
	vSRV_INF::iterator  iLst;
	DWORD dwSize = ppGI->pSC->vRefScanSI.size();

	for ( iLst = ppGI->pSC->vSI.begin()+dwSize; iLst != ppGI->pSC->vSI.end( ); iLst++ )
	{
	
		SERVER_INFO pSI = *iLst;//currCV->vSI.at((int)pLVItem->iItem);
		REF_SERVER_INFO refSI;
		refSI.dwIndex = pSI.dwIndex;
		refSI.cGAMEINDEX = pSI.cGAMEINDEX;

		//pSrvInf = &pSI;
		if(SCAN_FilterServerItem!=NULL)
		{
			if(SCAN_FilterServerItem((LPARAM*)&pSI,ppGI))
			{


				ppGI->pSC->vRefScanSI.push_back(refSI);
			}
		}
		else
			ppGI->pSC->vRefScanSI.push_back(refSI);  

	}
	dbg_print("Created scan serverlist!\n");

	if(g_hwndProgressBar!=NULL)
	{
		//ReInitililze Progressbar
		SendMessage(g_hwndProgressBar, PBM_SETRANGE, (WPARAM) 0,MAKELPARAM(0,ppGI->pSC->vRefScanSI.size())); 
	}
	
}

//Create scanning threads
void Initialize_Rescan2(GAME_INFO *pGI, bool (*filterServerItem)(LPARAM *lp,GAME_INFO *pGI))
{
	AddLogInfo(ETSV_DEBUG,"Entering Initialize_Rescan2 function.");
	SCAN_FilterServerItem = filterServerItem;

	vSRV_INF::iterator  iLst;
	pGI->pSC->vRefScanSI.clear();

	for ( iLst = pGI->pSC->vSI.begin( ); iLst != pGI->pSC->vSI.end( ); iLst++ )
	{
	
		SERVER_INFO pSI = *iLst;//currCV->vSI.at((int)pLVItem->iItem);
		REF_SERVER_INFO refSI;
		refSI.dwIndex = pSI.dwIndex;
		refSI.cGAMEINDEX = pSI.cGAMEINDEX;


		if(filterServerItem!=NULL)
		{
			if(filterServerItem((LPARAM*)&pSI,pGI))
			{
		
				pGI->pSC->vRefScanSI.push_back(refSI);
		
			}
		}
		else
		{
			pGI->pSC->vRefScanSI.push_back(refSI);  
		
		}

	}
	AddLogInfo(ETSV_INFO,"Preparing to scan %d servers of a total %d.\n",pGI->pSC->vRefScanSI.size(),pGI->pSC->vSI.size());

	if(pGI->dwViewFlags & FORCE_SCAN_FILTERED)
		pGI->dwViewFlags = 0;

	if(g_hwndProgressBar!=NULL)
	{
		//Initililze Progressbar
		SendMessage(g_hwndProgressBar, PBM_SETSTEP, (WPARAM) 1, 0); 
		SendMessage(g_hwndProgressBar, PBM_SETRANGE, (WPARAM) 0,MAKELPARAM(0,pGI->pSC->vRefScanSI.size())); 
		SendMessage(g_hwndProgressBar, PBM_SETPOS, (WPARAM) 0, 0); 
	}
	



	HANDLE hThreadIndex[MAX_THREADS];
	DWORD dwThreadId[MAX_THREADS];
	
	for(DWORD i=0; i<AppCFG.dwThreads;i++)
		hThreadIndex[i]=NULL;

	SCANNER_dwThreadCounter=0;
	pGI->dwScanIdx = 0;
	


	//Create and setup Continue Event, this is important! Otherwise ETSV can crash due.
	SCAN_hContinueEvent = CreateEvent(NULL,TRUE,TRUE,"ScanContinueEvent"); 
    if (SCAN_hContinueEvent == NULL) 
        AddLogInfo(ETSV_DEBUG,"CreateEvent failed (%d)\n", GetLastError());

	if (! ResetEvent(SCAN_hContinueEvent) ) 
        AddLogInfo(ETSV_DEBUG,"ResetEvent failed (%d)\n", GetLastError());

	DWORD dwMaxThreads = (AppCFG.dwThreads>pGI->pSC->vRefScanSI.size())?pGI->pSC->vRefScanSI.size():AppCFG.dwThreads;

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
			//AddLogInfo(ETSV_INFO,"Thread created! %d",i);					
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


	DWORD iWaitIndex = 0;
	int i=0;
	//Wait for all threads to finish...
	AddLogInfo(ETSV_DEBUG,"AppCFG.dwThreads %d",AppCFG.dwThreads);
	while(iWaitIndex<dwMaxThreads)
	{
	
		DWORD max = ((dwMaxThreads-iWaitIndex)<MAXIMUM_WAIT_OBJECTS)?(dwMaxThreads-iWaitIndex):MAXIMUM_WAIT_OBJECTS;		

		AddLogInfo(ETSV_DEBUG,"iWaitIndex: %d, iWaitIndex+max: %d, dwMaxThreads: %d,  max:%d",iWaitIndex,iWaitIndex+max,dwMaxThreads,max);
		DWORD dwEvent = WaitForMultipleObjects(max, &hThreadIndex[iWaitIndex], TRUE, INFINITE);
		//AddLogInfo(ETSV_DEBUG,">iWaitIndex: %d, iWaitIndex+max: %d, dwMaxThreads: %d, nThreads: %d, max:%d",iWaitIndex,iWaitIndex+max,dwMaxThreads, nThreads,max);
	
		switch (dwEvent) 
		{
			case WAIT_OBJECT_0: 
				//dbg_print("First event was signaled.\n");
				break; 
			case WAIT_TIMEOUT:
				dbg_print("Wait timed out.\n");
				break;
			// Return value is invalid.
			default: 
				{
					DebugBreak();
					Show_ErrorDetails("Error at waiting threads!");
					dbg_print("Wait error\n"); 
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

	AddLogInfo(0,"All servers is now scanned...");
	
	pGI->pSC->vRefScanSI.clear();


	CloseHandle(SCAN_hContinueEvent);
}



DWORD WINAPI  Get_ServerStatusThread2(LPVOID lpParam)
{


	GAME_INFO *pGI = (GAME_INFO *)lpParam;
	ppGI = pGI;
	SERVER_INFO pSI;
	memset(&pSI,0,sizeof(SERVER_INFO));
	
	DWORD idx=0;
	DWORD size = pGI->pSC->vRefScanSI.size();
//	char szText[100];

	while(pGI->dwScanIdx<size)
	{	
		
		if(SCANNER_bCloseApp)
		{
			dbg_print("Closing down SIGNALED!\n");
			break;
		}
		
		memset(&pSI,0,sizeof(SERVER_INFO));
		
		EnterCriticalSection(&SCANNER_cs);
		if(pGI->dwScanIdx<size)
		{
			SetStatusText(pGI->iIconIndex, "Scanning server %d of %d",pGI->dwScanIdx,size);
			idx = pGI->dwScanIdx;
			REF_SERVER_INFO refSI;
			refSI = pGI->pSC->vRefScanSI.at(idx);
			refSI.cGAMEINDEX = pSI.cGAMEINDEX;
			
			pSI = pGI->pSC->vSI.at(refSI.dwIndex);
			
			//sprintf(szText,"idx: %d scanid %d %d\n",pSI.dwIndex,pGI->dwScanIdx,pSI.dwPing);
		//	dbg_print(szText);		
			pGI->dwScanIdx++;
		}
		LeaveCriticalSection(&SCANNER_cs);
		
		if(pSI.dwPort==0)
		{
			//OutputDebugString(">>>>ERROR? Breaked scanning thread\n");
			break;
		}

		if(SCAN_FilterServerItem!=NULL)
		{
			//Only scan filtered servers
			 if(SCAN_FilterServerItem((LPARAM *)&pSI,pGI))
			 {
				 Get_ServerStatus(&pSI,NULL,NULL);	
				 pGI->pSC->vSI.at((int)pSI.dwIndex) = pSI;
			 }
		}
		else
		{
			//Do non-filtered scan of all servers
			Get_ServerStatus(&pSI,NULL,NULL);			
			pGI->pSC->vSI.at((int)pSI.dwIndex) = pSI;
		}
			
		if(SCANNER_UpdateServerListView!=NULL)
			SCANNER_UpdateServerListView(pSI.dwLVIndex);

		if(g_hwndProgressBar!=NULL)
				SendMessage(g_hwndProgressBar, PBM_STEPIT, (WPARAM) 0, 0);
		
	
		Sleep(10);		
	//	PollForNewServers();
	//	size = pGI->pSC->vRefScanSI.size();
		
	}

	SetStatusText(pGI->iIconIndex, "Waiting for threads to die!");
	
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


