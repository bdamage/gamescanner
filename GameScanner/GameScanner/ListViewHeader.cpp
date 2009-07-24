#include "StdAfx.h"
#include "ListViewHeader.h"

#define LVCOL_VER 0x1000 

CListViewHeader::CListViewHeader(HWND hwndLV,char *szHeaderName, CLanguage &lang) :
	m_hwndListView (hwndLV),
	m_hwndListViewHeader(NULL),
	m_iOrder(NULL),
	m_iItems(0),
	m_lang(lang)
{
	m_hwndListViewHeader = ListView_GetHeader(m_hwndListView);
	m_szHeaderName = szHeaderName;
}

CListViewHeader::~CListViewHeader(void)
{
	Save();
	if(m_iOrder!=NULL)
		delete [] m_iOrder;
}


BOOL CListViewHeader::Save()
{
	FILE *pf=NULL;
	DWORD dwVersion=LVCOL_VER;
	if( (pf = fopen( m_szHeaderName, "wb" )) != NULL ) 
	{
	
	   fwrite(&dwVersion,sizeof(DWORD),1,pf);
	   for(int i=0; i<vCC.size();i++)
	   {
			fwrite(&m_iOrder[i],sizeof(int),1,pf);
			fwrite(&vCC.at(i).lvColumn.cx,sizeof(int),1,pf);
			fwrite(&vCC.at(i).bActive,sizeof(int),1,pf);
			fwrite(&vCC.at(i).bSortAsc,sizeof(int),1,pf);
	   }

       fclose( pf );
	   dbg_print("Saved column order state.");	
	   return TRUE;
	}
	return FALSE;
}

BOOL CListViewHeader::Load()
{
	FILE *pf=NULL;
	if( (pf = fopen( m_szHeaderName, "rb" )) != NULL ) 
	{
	/*   long length = 0;
	   fseek( pf, 0, SEEK_END );
	   length = ftell( pf );
	   fseek( pf, 0, SEEK_SET );
	   m_iItems = (int)length / sizeof(int);
	   m_iOrder =(int*) malloc(length);
	   fread(m_iOrder,sizeof(BYTE),length,pf);	   
	   */
	   
	   DWORD dwVersion=0;
	   fread(&dwVersion,sizeof(DWORD),1,pf);
	   if(dwVersion!=LVCOL_VER)
	   {
		   fclose(pf);
		   return FALSE;
	   }

	   int i=0;
	   int bytes;
	   while(!feof(pf))
	   {
			int val;
			bytes = fread(&m_iOrder[i],sizeof(int),1,pf);
			if(bytes==0)
				break;

			bytes = fread(&val,sizeof(int),1,pf);
			vCC.at(i).lvColumn.cx = val;
			bytes = fread(&val,sizeof(int),1,pf);
			vCC.at(i).bActive = val;
			fread(&val,sizeof(int),1,pf);
			vCC.at(i).bSortAsc = val;
			i++;
	   }
       fclose( pf );
	   dbg_print("Load column order state successfully.");	
	   return TRUE;
	}
	return FALSE;
}

void CListViewHeader::Update()
{
	if((m_iItems = SendMessage(m_hwndListViewHeader, HDM_GETITEMCOUNT, 0,0))!=-1)
	{
		if(m_iOrder!=NULL)
			delete [] m_iOrder;

		m_iOrder = new int[m_iItems];
		ListView_GetColumnOrderArray(m_hwndListView,m_iItems,m_iOrder);

	}	
}

int CListViewHeader::GetSize()
{
	return vCC.size();
}

BOOL CListViewHeader::IsActive(int id)
{
	if(id>m_iItems)
		throw(2);

	return vCC.at(id).bActive;
}
void CListViewHeader::SetActive(int id,BOOL bActive)
{
	if(id>m_iItems)
		throw(2);
	 vCC.at(id).bActive = bActive;
	if(bActive)
		ListView_SetColumnWidth(m_hwndListView,id, vCC.at(id).lvColumn.cx);
	else
		ListView_SetColumnWidth(m_hwndListView,id,0);

}
void CListViewHeader::DeleteAllColumns()
{
	for(int i = 0;i<m_iItems;i++)
		ListView_DeleteColumn(m_hwndListView, 0 );
}

void CListViewHeader::Reset()
{
	vCC.clear();
}


void CListViewHeader::AddColumn(int cx,char *szColumnName,UINT mask,BOOL bActive)
{
	_CUSTOM_COLUMN CC;

	ZeroMemory(&CC.lvColumn,sizeof(CC.lvColumn));

	CC.id = vCC.size();
	CC.lvColumn.mask =  mask;
	CC.lvColumn.cx = cx;
	CC.sColumnName = szColumnName;
	CC.columnIdx = vCC.size();
	CC.bActive = bActive;
	CC.bSortAsc = TRUE;
	vCC.push_back(CC);

	//Insert the column
	LV_COLUMN   lvColumn;
	ZeroMemory(&lvColumn,sizeof(LV_COLUMN));
	lvColumn.pszText = (LPSTR) m_lang.GetString(CC.sColumnName.c_str());
	lvColumn.cx = CC.lvColumn.cx;
	lvColumn.fmt =  CC.lvColumn.fmt;
	lvColumn.mask =  CC.lvColumn.mask;
	ListView_InsertColumn(m_hwndListView,CC.id, &lvColumn);	
	if(bActive==FALSE)
		ListView_SetColumnWidth(m_hwndListView,CC.id,0);
}

int CListViewHeader::GetId(int iSubItem)
{
	if(m_iOrder==NULL)
		throw(1);
	if(iSubItem>m_iItems)
		throw(2);

	return iSubItem;// m_iOrder[iSubItem];
}
int CListViewHeader::GetColumnIdx(int iSubItem)
{
	return iSubItem;// m_iOrder[iSubItem]; //vCC.at(colname).columnIdx;
}

BOOL CListViewHeader::GetColumnSortOrder(int colname)
{
	return vCC.at(colname).bSortAsc;
}

void CListViewHeader::SwapSortAsc(int iSubItem)
{

	if(iSubItem>m_iItems)
		throw(2);

	BOOL val = vCC.at(iSubItem).bSortAsc;
	vCC.at(iSubItem).bSortAsc = ! val;
}

void CListViewHeader::ResetColumnOrder()
{
	int *order = new int[vCC.size()];
	for(int i=0; i<vCC.size();i++)
	{
		order[i]=i;
		vCC.at(i).bActive = TRUE;
	}

	if(ListView_SetColumnOrderArray(m_hwndListView, vCC.size(), order)==FALSE)
		DebugBreak();

	delete order;
}

void CListViewHeader::UpdateColumnOrder()
{

	if(ListView_SetColumnOrderArray(m_hwndListView, vCC.size(), m_iOrder)==FALSE)
		DebugBreak();

	for(int i=0; i<vCC.size();i++)
	{
		if(vCC.at(i).bActive==FALSE)
			ListView_SetColumnWidth(m_hwndListView,i,0);
		else
			ListView_SetColumnWidth(m_hwndListView,i,vCC.at(i).lvColumn.cx);
	}

}

const char *CListViewHeader::GetColumnName(int id)
{
	return vCC[id].sColumnName.c_str();
}



void CListViewHeader::OnEndDrag()
{
	PostMessage(m_hwndListView,WM_SAVE_SERVERLIST_COLUMN_STATE,0,0);

}

BOOL CListViewHeader::OnBeginTrack(LPARAM lParam)
{
	LPNMHEADER phdn;
	phdn = (LPNMHEADER) lParam; 
	
	if(vCC.at(phdn->iItem).bActive==FALSE)
		return TRUE;
	
	return FALSE;
}
void CListViewHeader::OnEndTrack(LPARAM lParam)
{	
	NMHEADER *pNMheader = (LPNMHEADER)lParam;
	HDITEM *pHDitem = pNMheader->pitem;

	vCC.at(pNMheader->iItem).lvColumn.cx = pNMheader->pitem->cxy;
	Save();
	DUMP_COL_ORDER();
}

void CListViewHeader::DUMP_COL_ORDER()
{

dbg_print(" ");	
		for(int i=0; i<m_iItems;i++)
			dbg_printNoLF("%d  ",i);	
dbg_print("");
		for(int i=0; i<m_iItems;i++)
			dbg_printNoLF("%d, ",m_iOrder[i]);	

	dbg_print("\n");	
}
