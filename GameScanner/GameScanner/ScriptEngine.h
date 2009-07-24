#pragma once

bool Sort_Filter(FILTER_SET fsA, FILTER_SET fsB);
bool Sort_Filter_By_GroupName(FILTER_SET fsA, FILTER_SET fsB);

class CScriptEngine
{
public:
	CScriptEngine(void);
	~CScriptEngine(void);
	int Execute(SERVER_INFO *pSI,GAME_INFO *pGI,vFILTER_SETS *vFS);
	int Get_OPCode(char *szInOp);
	int Do_Compare(int OP,const char *szA, const char *szB,BOOL bCompareMode);
	int Get_Constant(const char *szVarIn);
	int Get_Command(char *szVarIn);
	BOOL Display_Result(int iResult,BOOL bShowOK=FALSE);
	char *Get_ConstantValue(SERVER_INFO *pSI,GAME_INFO *pGI,int n,char *szConstTemp);
	int CompileFilter(vFILTER_SETS &vFS,const char *szInFilterText,const char *szCharFilterName,const char *szGroupName);
};
