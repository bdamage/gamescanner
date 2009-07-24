#include "StdAfx.h"
#include "ScriptEngine.h"
extern HWND g_hWnd;

#pragma warning(disable : 4996)

bool Sort_Filter(FILTER_SET fsA, FILTER_SET fsB)
{
	return (fsA.iFilterType>fsB.iFilterType);		
}

bool Sort_Filter_By_GroupName(FILTER_SET fsA, FILTER_SET fsB)
{
	return strcmp(fsA.sGroupName.c_str(),fsB.sGroupName.c_str())>0;		
}


CScriptEngine::CScriptEngine(void)
{
}

CScriptEngine::~CScriptEngine(void)
{
}

int CScriptEngine::Get_OPCode(char *szInOp)
{
	if(szInOp==NULL)
		return 0;

	else if(strcmp(szInOp,"==")==0) //equal to
		return 1;
	else if(strcmp(szInOp,"!=")==0) //NOT equal to
		return 2;
	else if(strcmp(szInOp,"~==")==0)  //strings match (case insensitive)
		return 3;
	else if(strcmp(szInOp,"~!=")==0)  //strings don't match (case insensitive)
		return 4;
	else if(strcmp(szInOp,"<")==0)  //less than
		return 5;
	else if(strcmp(szInOp,"<=")==0)  //less than or equal
		return 6;
	else if(strcmp(szInOp,">")==0)  //greater than 
		return 7;
	else if(strcmp(szInOp,">=")==0)  //greater than or equal 
		return 8;
	return 0;

}
int CScriptEngine::Do_Compare(int OP,const char *szA, const char *szB,BOOL bCompareMode)
{


	switch(OP)
	{
		case 1: //==
			{
				if((szA==NULL) || (szB==NULL))
					return 0;
				if(bCompareMode)  //substring compare?
					return wildcmp(szA, szB); //(strstr(szA,szB)!=NULL);  //strstr substring compare				
				else
					return ( strcmp(szA,szB)==0);
			}
		case 2: //!= NOT equal to
			{
				if((szA==NULL) || (szB==NULL))
					return 1;
				if(bCompareMode)  //substring compare?
					return (wildcmp(szA, szB)==0); //(strstr(szA,szB)==NULL); //NOT strstr
				else
					return (strcmp(szA,szB)!=0);  //exact compare
			}
		case 3: //~== case insensitive compare
			{
				if((szA==NULL) || (szB==NULL))
					return 0;

				if(bCompareMode)  //substring compare
					return wildicmp(szA, szB); //(stristr((TCHAR*)szA,szB)!=NULL); //substring compare
				else
					return (_stricmp(szA,szB)==0);  //exact compare
			}
		case 4:  //~!=
			{	
				if((szA==NULL) || (szB==NULL))
					return 1;
				if(bCompareMode)  //substring compare NOT
					return (wildicmp(szA, szB)==0); //(stristr((TCHAR*)szA,szB)==NULL);
				else
					return (_stricmp(szA,szB)!=0);   //exact compare
			}
		case 5: 
			{
				if((szA==NULL) || (szB==NULL))
					return 0;
				return (atoi(szA)<atoi(szB));
			}
		case 6: 
			{
				if((szA==NULL) || (szB==NULL))
					return 0;			
				return (atoi(szA)<=atoi(szB));
			}
		case 7: {
				if((szA==NULL) || (szB==NULL))
					return 0;
				return (atoi(szA)>atoi(szB));
				}
		case 8: {
				if((szA==NULL) || (szB==NULL))
					return 0;

				return (atoi(szA)>=atoi(szB));
				}
	}
	return 0;
}

int CScriptEngine::Get_Constant(const char *szVarIn)
{
	if(szVarIn==NULL)
		return 0;

	if(stricmp(szVarIn,"hostname")==0) //
		return 1;
	else if(stricmp(szVarIn,"game")==0)  //
		return 2;
	else if(stricmp(szVarIn,"ping")==0)  //
		return 3;
	else if(stricmp(szVarIn,"country")==0)  //
		return 4;
	else if(stricmp(szVarIn,"ip")==0)  //
		return 5;
	else if(stricmp(szVarIn,"port")==0)  //
		return 6;
	else if(stricmp(szVarIn,"address")==0)  //ip:port
		return 7;
	else if(stricmp(szVarIn,"map")==0)  //
		return 8;
	else if(stricmp(szVarIn,"players")==0)  //
		return 9;
	else if(stricmp(szVarIn,"maxplayers")==0)  //
		return 10;
	else if(stricmp(szVarIn,"modname")==0)  //
		return 11;
	else if(stricmp(szVarIn,"favorite")==0)  //
		return 12;
	else if(stricmp(szVarIn,"version")==0)  //
		return 13;
	else if(stricmp(szVarIn,"private")==0)  //
		return 14;
	else if(stricmp(szVarIn,"anticheat")==0)  //
		return 15;
	else if(stricmp(szVarIn,"bots")==0)  //
		return 16;
	return 0;
}

int Script_Get_Command(char *szVarIn)
{
	if(szVarIn==NULL)
		return 0;

	if(stricmp(szVarIn,"if")==0) // if statement
		return 1;
	else if(stricmp(szVarIn,"goto")==0)  //
		return 2;
	else if(stricmp(szVarIn,"remove")==0)  //
		return 3;
	else if(stricmp(szVarIn,"keep")==0)  //
		return 4;
	else if(stricmp(szVarIn,"run")==0)  //
		return 5;
	else if(stricmp(szVarIn,"norun")==0)  //
		return 6;
	return 0;
}

#define SCRIPT_EMPTY 100
#define SCRIPT_SYNTAX_ERROR 101
#define SCRIPT_MISSING_LINE_NO 102
#define SCRIPT_OUT_OF_MEMORY 103
#define SCRIPT_MISSING_VAR1 104
#define SCRIPT_MISSING_VAR2 105
#define SCRIPT_MISSING_OP 106
#define SCRIPT_MISSING_END_COMMAND 107
#define SCRIPT_MISSING_COMMAND 108
#define SCRIPT_LOOP_DETECTED 109
#define SCRIPT_GAME_INFO_MISSING 110
#define SCRIPT_QUOTE_MISSING 111
#define SCRIPT_OK 200


BOOL CScriptEngine::Display_Result(int iResult,BOOL bShowOK)
{
	char szMessage[200];
	switch(LOWORD(iResult))
	{
		case SCRIPT_EMPTY: MessageBox(NULL,"Script is empty.","Script",MB_OK); break;
		case SCRIPT_SYNTAX_ERROR: MessageBox(NULL,"Syntax error.","Script",MB_OK); break;
		case SCRIPT_MISSING_LINE_NO: 
			sprintf_s(szMessage,"Missing line number. (Line: %2.2d)",HIWORD(iResult));
			MessageBox(NULL,szMessage,"Script",MB_OK); 
			break;
		case SCRIPT_OUT_OF_MEMORY: MessageBox(NULL,"Out of memory","Script",MB_OK); break;
		case SCRIPT_MISSING_VAR1: 
			sprintf_s(szMessage,"Missing first expected variable. (Line: %2.2d)",HIWORD(iResult));
			MessageBox(NULL,szMessage,"Script",MB_OK); 
			break;
		case SCRIPT_MISSING_VAR2:
			sprintf_s(szMessage,"Missing secont expected variable. (Line: %2.2d)",HIWORD(iResult));
			MessageBox(NULL,szMessage,"Script",MB_OK); 
			break;

		case SCRIPT_MISSING_OP: 
			sprintf_s(szMessage,"Missing operator. (Line: %2.2d)",HIWORD(iResult));
			MessageBox(NULL,szMessage,"Script",MB_OK); 
			break;
		case SCRIPT_MISSING_END_COMMAND: MessageBox(NULL,"Missing end command.","Script",MB_OK); break;
		case SCRIPT_MISSING_COMMAND: 
			sprintf_s(szMessage,"No command detected. (Line: %2.2d)",HIWORD(iResult));
			MessageBox(NULL,szMessage,"Script",MB_OK); 	
			break;
		case SCRIPT_LOOP_DETECTED: 
				sprintf_s(szMessage,"Loops is not allowed. (Line: %2.2d)",HIWORD(iResult));	
				MessageBox(NULL,szMessage,"Script",MB_OK); 
			break;
		case SCRIPT_GAME_INFO_MISSING:
			    sprintf_s(szMessage,"Warning: Game info is missing...\nHowever this filter may still be valid.");	
				MessageBox(NULL,szMessage,"Script",MB_OK); 
			break;
		case SCRIPT_QUOTE_MISSING:
				sprintf_s(szMessage,"Warning: Quote is missing. (Line: %2.2d)...\nHowever this filter may still be valid.",HIWORD(iResult));	
				MessageBox(NULL,szMessage,"Script",MB_OK); 
			break;

		case SCRIPT_OK: 
			if(bShowOK)
				MessageBox(g_hWnd,"Script is ok.","Script",MB_OK);
			return TRUE;


		default: MessageBox(g_hWnd,"Unknown result!","Script",MB_OK); break;
	}
	return FALSE;
}


char *CScriptEngine::Get_ConstantValue(SERVER_INFO *pSI,GAME_INFO *pGI,int n,char *szConstTemp)
{
	char *pRuleValue=NULL;
	switch(n)
	{
		case 1:  pRuleValue = pSI->szServerName; break;
		case 2:  pRuleValue = pGI->szWebProtocolName; break;
		case 3:  pRuleValue = _itoa(pSI->dwPing,szConstTemp,10); break;								
		case 4:  pRuleValue = pSI->szShortCountryName; break;
		case 5:  pRuleValue = pSI->szIPaddress; break;							
		case 6:  pRuleValue = _itoa(pSI->usPort,szConstTemp,10); break;
		case 7:  
			sprintf(szConstTemp,"%s:%d",pSI->szIPaddress,pSI->usPort); 
			pRuleValue = szConstTemp;
			break;
		case 8:  pRuleValue = pSI->szMap; break;
		case 9:  pRuleValue = _itoa(pSI->nPlayers,szConstTemp,10); break;
		case 10:  pRuleValue = _itoa(pSI->nMaxPlayers,szConstTemp,10); break;	
		case 11:  pRuleValue = pSI->szMod; break;
		case 12:  pRuleValue = _itoa(pSI->cFavorite,szConstTemp,10); break;
		case 13:  pRuleValue = pSI->szVersion; break;
		case 14:  pRuleValue = _itoa(pSI->bPrivate,szConstTemp,10); break;
		case 15:  pRuleValue = _itoa(pSI->bPunkbuster,szConstTemp,10); break;
		case 16:  pRuleValue = _itoa(pSI->cBots,szConstTemp,10); break;
	}
	return pRuleValue;
}

/********************************
ex1:
if sv_punkbuster == "1"

else

ex2:
1 if hostname == "Test server"

*********************************/

int CScriptEngine::Execute(SERVER_INFO *pSI,GAME_INFO *pGI,vFILTER_SETS *vFS)
{
	int iReturn = -1;  //no script has run
	int iCodeReturn = FALSE;  //defaulting to FALSE to fall through the first loop
	if(vFS->size()<1)
		return TRUE;

	int iLastFilterType=-1;
	string sLastGroup = "??";

	for(UINT iSet=0; iSet<vFS->size();iSet++)
	{	  
		// Go through filter set	
		FILTER_SET fset = vFS->at(iSet);
	
		
		if(iLastFilterType!=-1)  //check if last set returned FALSE? this works as an AND operator
			if((stricmp(fset.sGroupName.c_str(),sLastGroup.c_str())!=0) && (iCodeReturn==FALSE))
				return FALSE;

		if((stricmp(fset.sGroupName.c_str(),sLastGroup.c_str())!=0) || (iCodeReturn==FALSE))
		{
			sLastGroup = fset.sGroupName;
			iLastFilterType = 1;

			UINT codeSize = fset.vecFilter_Codes.size();
			for(UINT i=0; i<codeSize;i++)
			{
				//Now go through byte codes

				FILTER_CODE fs = fset.vecFilter_Codes.at(i);

				int iResult = 0;
				iCodeReturn = -1;
				switch(fs.CMD)
				{
					case 1:
						{
						
							char szTemp1[20];
							char szTemp2[20];
							const char *pRuleValue1 = NULL;
							const char *pRuleValue2 = NULL;
							//int n = Script_Get_Constant(fs.sValue1.c_str());
							
							if(fs.constant1==0)
								pRuleValue1 = Get_RuleValue(fs.sValue1.c_str(),pSI->pServerRules);
							else
								pRuleValue1 = Get_ConstantValue(pSI,pGI,fs.constant1,szTemp1);
							
							//n = Script_Get_Constant(fs.sValue2.c_str());
							if(fs.constant2==0)
								pRuleValue2 = fs.sValue2.c_str();
							else
								pRuleValue2 = Get_ConstantValue(pSI,pGI,fs.constant2,szTemp2);


							iResult = Do_Compare(fs.OP,pRuleValue1,pRuleValue2,fs.bCompareMode);
							
							if((fs.secCMD==4) && iResult) //keep
								iCodeReturn = TRUE;
							else if((fs.secCMD==3) && iResult) //remove
								iCodeReturn = FALSE;
							else if((fs.secCMD==2) && iResult) //goto command
								i = fs.nextLN-2;
							else if((fs.secCMD==5) && iResult) //run
								iCodeReturn = TRUE;
							else if((fs.secCMD==6) && iResult) //norun
								iCodeReturn = FALSE;

							break;
						}
					case 2: //goto
						{
							i = fs.nextLN-2;
							break;
						}
					case 3: //remove
					case 6: //norun
						{		
							iCodeReturn = FALSE;
							break;			
						}
					case 4: //keep
					case 5: //run

						{
							iCodeReturn = TRUE;
							break;
						}
				} //end switch
				if((iCodeReturn==TRUE) || (iCodeReturn==FALSE))
					break;	

			} //end for of filter codes
		}//End if type

		iReturn = iCodeReturn;
	} //end of filter sets
	if(iReturn==-1)
		iReturn=TRUE;
		
	return iReturn;
}



int CScriptEngine::CompileFilter(vFILTER_SETS &vFS,const char *szInFilterText,const char *szCharFilterName,const char *szGroupName)
{
	int iResult = SCRIPT_OK;
	int iLNo=1;
	char seps[]   = " \n\t\r";
	char seps2[]   = " \"\n\t\r";
	char seps3[]   = "\"\t\r";


	if(szInFilterText==NULL)
		return SCRIPT_EMPTY;

	int size = strlen(szInFilterText);

	if(size==0)
		return SCRIPT_EMPTY;


	char *szScript = (char*)calloc(sizeof(char),size+1);

	if(szScript==NULL)
		return SCRIPT_OUT_OF_MEMORY;
	strcpy(szScript,szInFilterText);

	char  *next_token = NULL;
	FILTER_SET fset;
	fset.sFilterName = szCharFilterName;
	//fset.iFilterType = nType;
	fset.sGroupName = szGroupName;
	
	FILTER_CODE fs;	


//loop
	next_token = NULL;

	char *szCmd = strtok_s( szScript, seps, &next_token);
	if(szCmd==NULL)
		{ iResult = SCRIPT_MISSING_COMMAND; goto exitScript;}		

	char *szSecCmd = NULL;
	int lineNo = 1;
	while(next_token!=NULL)
	{
		
		fs.constant1=0;
		fs.constant2=0;
		fs.sValue1 = "";
		fs.sValue2 = "";
		fs.OP = 0;
		szSecCmd = NULL;
		fs.secCMD = 0;
		fs.nextLN = 0;
		fs.LN = lineNo++; // = atoi(szLNo);
		fs.bCompareMode = FALSE;
		fs.CMD = Script_Get_Command(szCmd);

	/*
	if expected syntax
	-------------------
	*/
		switch(fs.CMD)
		{
			case 1: //if  example: if sv_punkbuster == "1"
			{
				fs.sValue1 = strtok_s( NULL, seps2, &next_token);
					if(fs.sValue1.size()==0)
						{ iResult = SCRIPT_MISSING_VAR1; goto exitScript;}				
				
				fs.constant1 = Get_Constant(fs.sValue1.c_str());
			

				if(next_token!=NULL)
					fs.OP = Get_OPCode( strtok_s( NULL, seps, &next_token));
				else
					{ iResult = SCRIPT_MISSING_OP; goto exitScript;}
				if(fs.OP==0)
					{ iResult = SCRIPT_MISSING_OP; goto exitScript;}

				if(next_token!=NULL)
				{
					char *Q1 = strchr(next_token,'\"');
					char *Q2=NULL;
					if(Q1!=NULL)
						Q2 = strchr(++Q1,'\"');

					char *szValue2 = NULL;
					if(Q1!=NULL || Q2!=NULL)
					{
						//it's a string
						szValue2 = strtok_s( NULL, seps3, &next_token);
					} else //assume it is a constant
					{
						szValue2 = strtok_s( NULL, seps2, &next_token);
					}
						
					
					if(szValue2==NULL)
						{ iResult = SCRIPT_MISSING_VAR2; goto exitScript;}

					char *pWildAsterix = strchr(szValue2,'*');
					char *pWildQuestion = strrchr(szValue2,'?');
					if((pWildAsterix!=NULL) || (pWildQuestion!=NULL))
						fs.bCompareMode = TRUE;
						
					fs.sValue2 = szValue2;	
					fs.constant2 = Get_Constant(fs.sValue2.c_str());
				}
				else
					{ iResult = SCRIPT_MISSING_VAR2; goto exitScript;}
				
				if((next_token!=NULL) && (next_token[0]!=0x0A))
				{	

					szSecCmd = strtok_s( NULL, seps, &next_token);
					
					if(szSecCmd!=NULL)
					{	
						fs.secCMD =  Script_Get_Command(szSecCmd);

						if(fs.secCMD==2)  //goto command?
						{
							fs.nextLN = atoi(strtok_s( NULL, seps2, &next_token));
							if(fs.nextLN<=fs.LN)
								{ iResult = SCRIPT_LOOP_DETECTED; goto exitScript;}
						}
					}
				}
			
				break;
			}
			case 2: //goto  example: goto 2
			{
				fs.nextLN = atoi(strtok_s( NULL, seps2, &next_token));
				if(fs.nextLN<=fs.LN)
					{ iResult = SCRIPT_LOOP_DETECTED; goto exitScript;}
				break;
			}
			case 3: //remove
			case 4: //keep			
			case 5: //run
			case 6: //norun
				{
					break;
				}
			default:
				{ iResult = SCRIPT_MISSING_COMMAND; goto exitScript;}
				break;
		}
		if(fs.CMD!=0)
			fset.vecFilter_Codes.push_back(fs);		

	
		szCmd = strtok_s( NULL, seps, &next_token);
		if(szCmd==NULL)
			{ iResult = SCRIPT_OK; goto exitScript;}	
			
	}
	

exitScript:
	if(iResult == SCRIPT_OK)  
	{		
		vFS.push_back(fset);
		sort(vFS.begin(),vFS.end(),Sort_Filter_By_GroupName);
	}

	iResult = MAKELONG(iResult,fs.LN);
	//clean up
	if(szScript!=NULL)
		free(szScript);
		
	return iResult;
}
