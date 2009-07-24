#pragma once

typedef vector<_CUSTOM_COLUMN> vec_CUSTOM_COLUMN;  

class CListViewHeader
{
	vec_CUSTOM_COLUMN vCC;

	CLanguage &m_lang;
	HWND m_hwndListView;
	HWND m_hwndListViewHeader;
	int *m_iOrder;
	int m_iItems;
	char *m_szHeaderName;
public:
	CListViewHeader(HWND hwndLV,char *szHeaderName, CLanguage &lang);
	~CListViewHeader(void);
	BOOL Save();
	BOOL Load();
	void Update();
	void DeleteAllColumns();
	void Reset();
	void ResetColumnOrder();
	void UpdateColumnOrder();
	void AddColumn(int cx,char *szColumnName,UINT mask,BOOL bActive=TRUE);
	BOOL OnBeginTrack(LPARAM lParam);
	void OnEndDrag();
	void OnEndTrack(LPARAM lParam);
	int GetColumnIdx(int colname);
	int GetId(int iSubItem);
	void SwapSortAsc(int iSubItem);
	BOOL GetColumnSortOrder(int colname);
	const char *GetColumnName(int id);
	int GetSize();
	void SetActive(int id,BOOL bActive);
	BOOL IsActive(int id);
	//Debug methods
	void DUMP_COL_ORDER();
};
