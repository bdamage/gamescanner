#pragma once

#include "logger.h"
#include "gamemanager.h"
#include "ScriptEngine.h"

//#include "structs_defines.h"
typedef vector<_MYTREEITEM> _vecTI;

class CTreeViewManager
{


	CXmlFile	xml;
	TCHAR *m_EXE_PATH;			//Don't write anything to this path
	TCHAR *m_USER_SAVE_PATH;    //Path to save settings and server lists
	CLogger & log;
	CGameManager &gm;
	CScriptEngine &m_se;
	
//HTREEITEM hRootItem=NULL,hRootFiltersItem=NULL,
	HTREEITEM hRootCountryFiltersItem;

//HTREEITEM hRootEuropeItem=NULL,hFilterPingItem=NULL,hFilterGameTypeItem=NULL;


public:
	
	_CountryFilter CountryFilter;
	BOOL g_bMouseMenuClick;
	BOOL m_bShiftKeyPressed;
	BOOL m_bFirstShiftClick;
	UINT m_iFirstClickIdx;
	UINT m_iSecondClickIdx;
	UINT g_save_counter;
	BOOL m_bTREELOADED;
	HWND hwndTreeCtrl;
	char TREEVIEW_VERSION[20];
	char TREEVIEW_GLOBAL_FILTER_VERSION[20];
	_vecTI vTI; //Vector Tree Item
public:
	CTreeViewManager(CLogger & logger,	CGameManager & _gm, CScriptEngine &_se);
	~CTreeViewManager(void);
	BOOL DeleteByHTI(HTREEITEM hItemtree);
	HTREEITEM GetHTIByItemGame(int game);
	HTREEITEM GetTIByType(DWORD dwType);
	void CleanUp();
	int GetIndex(HTREEITEM hItemtree);
	void ReIndex();
	int ParseChilds(DWORD *TreeLevel,TiXmlElement* childItem, HTREEITEM hTreeItem, int iGameIdx);
	void SetItemText(HTREEITEM hTI, const char *szText);
	BOOL SetCheckBoxState(int iSel,DWORD dwState);
	int SetAllChildItemExpand(int startIdx, bool expand);
	DWORD Save_all_by_level(TiXmlElement *pElemRoot,DWORD dwlevel,BOOL bIgnoreGlobalFilter);
	int Save();
	int  Load(char *ExePath,char *UserPath);
	int  CheckForUpdate(const char *szCurrentVersion);
	int  CheckForUpdateGlobalFilters(const char *szCurrentVersion);
	DWORD SwapDWCheckState(TVITEM  *pTVitem, DWORD &dwValue);
	int DoAction(int iSel, UINT flags,TVITEM  *ptvitem,BOOL bRedraw);
	int OnSelection(LPARAM lParam);
	void Filter_Remove(vFILTER_SETS & vFS,const char *szFiltername);
	void Filter_Remove_Group(vFILTER_SETS &vFS,_MYTREEITEM ti);	
	int Filter_update(GAME_INFO *pGI,_MYTREEITEM ti,const char *szOldFilterName);
	int Filter_change(GAME_INFO *pGI,_MYTREEITEM ti, TVITEM *tvi);
	int Filter_change_group(GAME_INFO *pGI,_MYTREEITEM ti, TVITEM *tvi);
	int Filter_custom_favorite_filter(GAME_INFO *pGI,_MYTREEITEM ti, TVITEM *tvi);
	void Select_all_childs(HTREEITEM hRoot, bool selected);
	void Initialize_CountryFilter();
	DWORD Build_CountryFilter(HTREEITEM hRoot);
};
