/*
MIT License

Copyright (c) 2007 Kjell Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


*/

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
