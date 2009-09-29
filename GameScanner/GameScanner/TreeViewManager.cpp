#include "StdAfx.h"
#include "TreeViewManager.h"


//extern int Script_CompileFilter(vFILTER_SETS &vFS,const char *szInFilterText,const char *szCharFilterName,const char *szGroupName);
extern APP_SETTINGS_NEW AppCFG;
extern int g_currentGameIdx;

CTreeViewManager::CTreeViewManager(CLogger & logger, CGameManager & _gm, CScriptEngine &_se) : 
	log(logger), 
	gm(_gm),
	m_se(_se),
	g_bMouseMenuClick(FALSE),
	m_bShiftKeyPressed(FALSE)
{
	g_save_counter = 0;
	hwndTreeCtrl = NULL;
	m_bTREELOADED = FALSE;
	m_hwndMain = NULL;
}

CTreeViewManager::~CTreeViewManager(void)
{
}

BOOL CTreeViewManager::DeleteByHTI(HTREEITEM hItemtree)
{
	for(UINT i=0;i<vTI.size();i++)
	{
		if(vTI.at(i).hTreeItem == hItemtree)
		{	
			vTI.erase(vTI.begin()+i);
			return TRUE;
		}
	}
	return FALSE;
}

HTREEITEM CTreeViewManager::GetHTIByItemGame(int game)
{
	
	for(UINT i=0;i<vTI.size();i++)
	{
		//Will return first occurens of the game index (= game root)
		if(vTI.at(i).cGAMEINDEX == game)
			return vTI.at(i).hTreeItem;
	}
	return NULL;
}

HTREEITEM CTreeViewManager::GetTIByType(DWORD dwType)
{
	for(UINT i=0;i<vTI.size();i++)
	{
		if(vTI.at(i).dwType == dwType)
			return vTI.at(i).hTreeItem;
	}
	return NULL;
}

void CTreeViewManager::CleanUp()
{
	dbg_print("TreeView_cleanup");
	for(int i=0;i<vTI.size();i++)
	{
		vTI.at(i).sElementName.clear();
		vTI.at(i).sName.clear();
		vTI.at(i).sScript.clear();
		vTI.at(i).pGI = NULL;
	}

	vTI.clear();
}


int CTreeViewManager::GetIndex(HTREEITEM hItemtree)
{
	for(UINT i=0;i<vTI.size();i++)
	{
		if(vTI.at(i).hTreeItem == hItemtree)
			return i;
	}
	return -1;
}

void CTreeViewManager::ReIndex()
{
	for(int i=0; i<vTI.size();i++) //reindex
	{					
			vTI.at(i).dwIndex = i;
			TVITEM  tvitem;
			ZeroMemory(&tvitem,sizeof(TVITEM));
			tvitem.hItem = vTI.at(i).hTreeItem;
			tvitem.mask = TVIF_PARAM ;
			tvitem.lParam = vTI.at(i).dwIndex;
			TreeView_SetItem(hwndTreeCtrl,&tvitem);

			/*char text[256];
			sprintf(text,"%s %d",vTI.at(i).sName.c_str(),vTI.at(i).dwIndex);
			TreeView_SetItemText(vTI.at(i).hTreeItem,text);
			*/
	}
}

void CTreeViewManager::SetItemText(HTREEITEM hTI, const char *szText)
{
	TVITEM  tvitem;
	if(szText==NULL)
		return;
	memset(&tvitem,0,sizeof(TVITEM));
	tvitem.hItem = hTI;
	tvitem.mask = TVIF_TEXT ;
	tvitem.cchTextMax = strlen(szText);
	tvitem.pszText = (LPSTR)szText;
	TreeView_SetItem(hwndTreeCtrl, &tvitem );
}


BOOL CTreeViewManager::SetCheckBoxState(int iSel,DWORD dwState)
{
	TVITEM  tvitem;
	ZeroMemory(&tvitem,sizeof(TVITEM));
	tvitem.hItem = vTI.at(iSel).hTreeItem;
	tvitem.mask = TVIF_SELECTEDIMAGE |  TVIF_IMAGE;

	if(dwState) //Is it checked
		tvitem.iSelectedImage =	tvitem.iImage = CHECKED_ICON;  //Checked image
	else
		tvitem.iSelectedImage =	tvitem.iImage = UNCHECKED_ICON;  //Unchecked image
	
	vTI.at(iSel).dwState = tvitem.iSelectedImage - UNCHECKED_ICON ; //set the value
	return TreeView_SetItem(hwndTreeCtrl, &tvitem );	
}


int CTreeViewManager::SetAllChildItemExpand(int startIdx, bool expand)
{
	DWORD dwLevel=0;
	dwLevel = vTI.at(startIdx).dwLevel;
	for(UINT i=startIdx;i<vTI.size();i++)
	{
		vTI.at(i).bExpanded = expand;
		if(vTI.at(i).dwLevel <dwLevel)
			break;
			
	}
	return -1;
}


DWORD CTreeViewManager::Save_all_by_level(TiXmlElement *pElemRoot,DWORD dwlevel,BOOL bIgnoreGlobalFilter)
{
	if(g_save_counter>vTI.size())
		return 0;
	BOOL bSkip=FALSE;

	if(pElemRoot == NULL)
		return 0;

	while(dwlevel==vTI.at(g_save_counter).dwLevel)
	{			
		int iSel = g_save_counter;
		
		bSkip=FALSE;
		if(bIgnoreGlobalFilter && vTI.at(iSel).cGAMEINDEX==-1) //reached to global filter
			bSkip=TRUE;

		if(vTI.at(iSel).bDelete)
			bSkip =TRUE;

		TiXmlElement * elem = NULL;
		if(bSkip==FALSE)
		{

    		elem = new TiXmlElement( vTI.at(iSel).sElementName.c_str());  
/*
	#ifdef _DEBUG	
			char padding[40];
			padding[0]=0;
			for(int i=0;i<dwlevel;i++)
				strcat(padding," ");
			dbg_print("%s %d %d %s %s Action %d level:%d",padding,iSel,dwlevel,tvmgr.vTI.at(iSel).sElementName.c_str(),tvmgr.vTI.at(iSel).sName.c_str(),tvmgr.vTI.at(iSel).dwType,tvmgr.vTI.at(iSel).dwLevel);
	#endif
*/
			elem->SetAttribute("name",vTI.at(iSel).sName.c_str());
			elem->SetAttribute("icon",vTI.at(iSel).iIconIndex);
			elem->SetAttribute("expand",(UINT)vTI.at(iSel).bExpanded);

			elem->SetAttribute("state",vTI.at(iSel).dwState);
			if(vTI.at(iSel).dwType==1)
				elem->SetAttribute("game",vTI.at(iSel).cGAMEINDEX);

			elem->SetAttribute("type",vTI.at(iSel).dwType);	
			elem->LinkEndChild(new TiXmlText(vTI.at(iSel).sScript.c_str()));
		
				
			pElemRoot->LinkEndChild( elem ); 
		}

		g_save_counter++;
		if(g_save_counter>=vTI.size())
			return 0;

		 
		DWORD nextlevel = vTI.at(g_save_counter).dwLevel;
		DWORD lvl=0;
		if(nextlevel>dwlevel)
		{	
			lvl = Save_all_by_level(elem,nextlevel,bIgnoreGlobalFilter);
			if(lvl!=dwlevel)
				return lvl;
		}		
		if(g_save_counter>=vTI.size())
			return 0;
	}
	return vTI.at(g_save_counter).dwLevel;
}


int CTreeViewManager::Save()
{
	char szFilePath[_MAX_PATH+_MAX_FNAME];
	dbg_print("Saving treeview state in progress...");
	TiXmlDocument doc;  
 	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );  
	doc.LinkEndChild( decl );  
 
	TiXmlElement * root = new TiXmlElement( "TreeViewCFG" );  //TreeView config
	root->SetAttribute("version",TREEVIEW_VERSION);
	TiXmlComment * comment = new TiXmlComment();
	comment->SetValue("Settings for Game Scanner treeview." );  
	root->LinkEndChild( comment );  
	TiXmlElement * TreeItems = new TiXmlElement( "TreeItems" );  
		
	g_save_counter=0;
	if(!vTI.empty())
		Save_all_by_level(TreeItems,vTI.at(g_save_counter).dwLevel,TRUE);

	root->LinkEndChild( TreeItems );  
	doc.LinkEndChild( root );  

	sprintf_s(szFilePath,"%s%s",m_USER_SAVE_PATH,"treeviewcfg.xml");
	SetCurrentDirectory(m_USER_SAVE_PATH);
	if(!doc.SaveFile(szFilePath))
		log.AddLogInfo(GS_LOG_WARNING,"Error saving Treeview XML file!");

	TiXmlDocument GlobalFilterDoc;  
	 TiXmlDeclaration* decl2 = new TiXmlDeclaration( "1.0", "", "" );   
	GlobalFilterDoc.LinkEndChild( decl2 );  
 
	TiXmlElement * GFroot = new TiXmlElement( "Globalfilter" );  //TreeView config
	GFroot->SetAttribute("version",TREEVIEW_GLOBAL_FILTER_VERSION);
	
	TiXmlComment * comment2 = new TiXmlComment();
	comment2->SetValue("Global filter settings for Game Scanner treeview." );  
	GFroot->LinkEndChild( comment2 );  
	TiXmlElement * TreeItems2 = new TiXmlElement( "TreeItems" );  
	dbg_print("Saving ---------- Global Filter -------------- ");
	g_save_counter--;
	if(!vTI.empty() && (g_save_counter<vTI.size()))
		Save_all_by_level(TreeItems2,vTI.at(g_save_counter).dwLevel,FALSE);

	GFroot->LinkEndChild( TreeItems2 );  
	GlobalFilterDoc.LinkEndChild( GFroot );  

	sprintf_s(szFilePath,"%s%s",m_USER_SAVE_PATH,"globalfilter.xml");
	SetCurrentDirectory(m_USER_SAVE_PATH);
	if(!GlobalFilterDoc.SaveFile(szFilePath))
		log.AddLogInfo(GS_LOG_WARNING,"Error saving Treeview XML file!");

	return 0;
}



int  CTreeViewManager::Load(char *ExePath,char *UserPath)
{
	m_EXE_PATH = ExePath;			
	m_USER_SAVE_PATH = UserPath;  

	CXmlFile xml;
	CXmlFile xmlUpdate;
	CXmlFile xmlGlobalFilters;
	TiXmlHandle hRoot(0);

	char szFilePath[_MAX_PATH+_MAX_FNAME];
	ZeroMemory(szFilePath,sizeof(szFilePath));
	
	strncpy(szFilePath,m_USER_SAVE_PATH,strlen(m_USER_SAVE_PATH));
	strcat_s(szFilePath,"treeviewcfg.xml");
	log.AddLogInfo(GS_LOG_INFO,"Trying to load %s",szFilePath);
	SetCurrentDirectory(m_USER_SAVE_PATH);
	if (xml.load(szFilePath)== XMLFILE_ERROR_LOADING)
	{
		ZeroMemory(szFilePath,sizeof(szFilePath));
		strncpy(szFilePath,m_EXE_PATH,strlen(m_EXE_PATH));
		strcat_s(szFilePath,"\\updated_treeviewcfg.xml");
		SetCurrentDirectory(m_EXE_PATH);
			
		if (xml.load(szFilePath)== XMLFILE_ERROR_LOADING) //(!doc.LoadFile(szFilePath)) 
		{
			log.AddLogInfo(GS_LOG_ERROR,"Error loading default TreeView file from m_EXE_PATH (%s)",szFilePath);
			return 1;
		}
	}

	hRoot = xml.GetHandle();
	m_bTREELOADED = TRUE;


	ZeroMemory(TREEVIEW_VERSION,sizeof(TREEVIEW_VERSION));
	xml.GetCustomAttribute(xml.m_pRootElement,"version",TREEVIEW_VERSION,sizeof(TREEVIEW_VERSION)-1);

	if(strlen(TREEVIEW_VERSION)>0)
		log.AddLogInfo(GS_LOG_INFO,"Current TreeView CFG file version is %s.",TREEVIEW_VERSION);
		
	if(CheckForUpdate(TREEVIEW_VERSION)==0)
	{
		int retMB = MessageBox(m_hwndMain,"A new version of treeview (filters & game selection) file has detected.\nWould you like to upgrade?\nNOTE: An upgrade will cause custom filters to be lost!","Game Scanner",MB_YESNO|MB_ICONINFORMATION);
		if(retMB==IDYES)
		{

			//new version detected
			ZeroMemory(szFilePath,sizeof(szFilePath));
			strncpy(szFilePath,m_EXE_PATH,strlen(m_EXE_PATH));
			strcat_s(szFilePath,"\\updated_treeviewcfg.xml");
			SetCurrentDirectory(m_EXE_PATH);		
			if(xmlUpdate.load(szFilePath)==XMLFILE_ERROR_LOADING ) //!docNew.LoadFile(szFilePath))
			{
				log.AddLogInfo(GS_LOG_ERROR,"Error loading NEW treeviewcfg file.");
				//continue with the old one
			}else
			{
				ZeroMemory(TREEVIEW_VERSION,sizeof(TREEVIEW_VERSION));
				xmlUpdate.GetCustomAttribute(xmlUpdate.m_pRootElement,"version",TREEVIEW_VERSION,sizeof(TREEVIEW_VERSION)-1);

				hRoot = xmlUpdate.GetHandle();
			}			
		}
	}
	vTI.clear();
	
	TiXmlElement* child = hRoot.FirstChild( "TreeItems" ).ToElement();

	DWORD TreeLevel=1;
	for( child; child; child=child->NextSiblingElement() )
	{
		ParseChilds((DWORD*)&TreeLevel,child->FirstChildElement(),TVI_ROOT,-2);
	}	


	sprintf_s(szFilePath,"%s%s",m_USER_SAVE_PATH,"globalfilter.xml");	
	log.AddLogInfo(GS_LOG_INFO,"Trying to load %s",szFilePath);
	SetCurrentDirectory(m_USER_SAVE_PATH);
	TiXmlDocument GlobalFilterDoc(szFilePath);
	if (xmlGlobalFilters.load(szFilePath)==XMLFILE_ERROR_LOADING) //!GlobalFilterDoc.LoadFile()) 
	{
		log.AddLogInfo(GS_LOG_ERROR,"Error loading config file for globalfilter.xml from m_USER_SAVE_PATH (%s)",szFilePath);
		AppCFG.filter.bHideOfflineServers = 0;
		sprintf_s(szFilePath,"%s\\%s",m_EXE_PATH,"globalfilter.xml");
		SetCurrentDirectory(m_EXE_PATH);
		if (xmlGlobalFilters.load(szFilePath)==XMLFILE_ERROR_LOADING) 
		{
			log.AddLogInfo(GS_LOG_ERROR,"Error loading default globalfilter.xml file from m_EXE_PATH (%s)",szFilePath);
			return 1;
		}
	}


	ZeroMemory(TREEVIEW_GLOBAL_FILTER_VERSION,sizeof(TREEVIEW_GLOBAL_FILTER_VERSION));
	xml.GetCustomAttribute(xml.m_pRootElement,"version",TREEVIEW_GLOBAL_FILTER_VERSION,sizeof(TREEVIEW_GLOBAL_FILTER_VERSION)-1);
	if(CheckForUpdateGlobalFilters(TREEVIEW_GLOBAL_FILTER_VERSION)==0)
	{
		int retMB = MessageBox(m_hwndMain,"A new version of global filters file has detected.\nWould you like to upgrade?\nNOTE: An upgrade will cause custom filters to be lost!","Game Scanner",MB_YESNO|MB_ICONINFORMATION);
		if(retMB==IDYES)
		{
			AppCFG.filter.bHideOfflineServers = 0;
			sprintf_s(szFilePath,"%s\\%s",m_EXE_PATH,"globalfilter.xml");
			SetCurrentDirectory(m_EXE_PATH);
			if (xmlGlobalFilters.load(szFilePath)==XMLFILE_ERROR_LOADING) 
			{
				log.AddLogInfo(GS_LOG_ERROR,"Error loading default globalfilter.xml file from m_EXE_PATH (%s)",szFilePath);
				return 1;
			}
		}
	}

	ZeroMemory(TREEVIEW_GLOBAL_FILTER_VERSION,sizeof(TREEVIEW_GLOBAL_FILTER_VERSION));
	xmlGlobalFilters.GetCustomAttribute(xmlGlobalFilters.m_pRootElement,"version",TREEVIEW_GLOBAL_FILTER_VERSION,sizeof(TREEVIEW_GLOBAL_FILTER_VERSION)-1);

	child = xmlGlobalFilters.m_pRootElement->FirstChildElement("TreeItems");
	for( child; child; child=child->NextSiblingElement() )
	{	
		ParseChilds((DWORD*)&TreeLevel,child->FirstChildElement(),TVI_ROOT,-1);
	}	

	char szBuffer[200];

	//Let's do some resync values, this will help to ensure after an upgrade of the treeview structure to display correct values and states.
	for(int i=0; i<gm.GamesInfo.size();i++)
	{
	
		gm.SetHTREEITEM(i, GetHTIByItemGame(i));

		sprintf_s(szBuffer,"%s (%d)",gm.GamesInfo[i].szGAME_NAME,gm.GamesInfo[i].dwTotalServers);
		if(gm.GamesInfo[i].hTI!=NULL)
		{
			SetItemText(gm.GamesInfo[i].hTI,szBuffer);
			TreeView_SetItemState(hwndTreeCtrl,gm.GamesInfo[i].hTI,TVIS_BOLD ,TVIS_BOLD);

			//TreeView_SetFilterGroupCheckState(i,FILTER_REGION,gm.GamesInfo[i].filter.dwRegion);
			//TreeView_SetFilterGroupCheckState(i,FILTER_REGION,gm.GamesInfo[i].filter.dwRegion);
		}
	}



	SetFocus(hwndTreeCtrl);
	return 0;
}

//Returns 0 if a new version has detected otherwise non zero.
int  CTreeViewManager::CheckForUpdate(const char *szCurrentVersion)
{
	CXmlFile xml;
	char szFilePath[_MAX_PATH+_MAX_FNAME];
	ZeroMemory(szFilePath,sizeof(szFilePath));
	strncpy(szFilePath,m_EXE_PATH,strlen(m_EXE_PATH));
	strcat_s(szFilePath,"\\updated_treeviewcfg.xml");
	log.AddLogInfo(GS_LOG_INFO,"Trying to load %s",szFilePath);
	SetCurrentDirectory(m_EXE_PATH);

	if (xml.load(szFilePath)== XMLFILE_ERROR_LOADING)//!doc.LoadFile(szFilePath)) 
	{
		log.AddLogInfo(GS_LOG_ERROR,"Error loading %s",szFilePath);
		return 1;
	}

	char  szVersion[20];
	ZeroMemory(szVersion,sizeof(szVersion));
	xml.GetCustomAttribute(xml.m_pRootElement,"version",szVersion,sizeof(szVersion)-1);

	log.AddLogInfo(GS_LOG_INFO,"Detected treeview.NEW cfg file version is %s ",szVersion);
	if(szVersion!=NULL)
	{
		if(szCurrentVersion!=NULL)
		{
			if(strcmp(szVersion,szCurrentVersion)>0)
			{
			
				log.AddLogInfo(GS_LOG_INFO,"New version detected %s != %s",szVersion,szCurrentVersion);
		//		strcpy_s(TREEVIEW_VERSION,szVersion);
			}
			else
				return 2;
		}else
		{
			log.AddLogInfo(GS_LOG_INFO,"New version detected",szVersion);
		//	strcpy_s(TREEVIEW_VERSION,szVersion);
		}
		
		return 0; //New version detected
	}
	return 1;

}

//Returns 0 if a new version has detected otherwise non zero.
int  CTreeViewManager::CheckForUpdateGlobalFilters(const char *szCurrentVersion)
{
	CXmlFile xml;
	char szFilePath[_MAX_PATH+_MAX_FNAME];
	ZeroMemory(szFilePath,sizeof(szFilePath));
	strncpy(szFilePath,m_EXE_PATH,strlen(m_EXE_PATH));
	strcat_s(szFilePath,"\\globalfilter.xml");
	log.AddLogInfo(GS_LOG_INFO,"Trying to load %s",szFilePath);
	SetCurrentDirectory(m_EXE_PATH);

	if (xml.load(szFilePath)== XMLFILE_ERROR_LOADING)//!doc.LoadFile(szFilePath)) 
	{
		log.AddLogInfo(GS_LOG_ERROR,"Error loading %s",szFilePath);
		return 1;
	}

	char  szVersion[20];
	ZeroMemory(szVersion,sizeof(szVersion));
	xml.GetCustomAttribute(xml.m_pRootElement,"version",szVersion,sizeof(szVersion)-1);

	log.AddLogInfo(GS_LOG_INFO,"Detected globalfilter file version is %s ",szVersion);
	if(szVersion!=NULL)
	{
		if(szCurrentVersion!=NULL)
		{
			if(strcmp(szVersion,szCurrentVersion)>0)
			{
			
				log.AddLogInfo(GS_LOG_INFO,"New global filter version detected %s != %s",szVersion,szCurrentVersion);
		//		strcpy_s(TREEVIEW_GLOBAL_FILTER_VERSION,szVersion);
			}
			else
				return 2;
		}else
		{
			log.AddLogInfo(GS_LOG_INFO,"New global filter version detected",szVersion);
		//	strcpy_s(TREEVIEW_GLOBAL_FILTER_VERSION,szVersion);
		}
		
		return 0; //New version detected
	}
	return 1;

}

int CTreeViewManager::ParseChilds(DWORD *TreeLevel,TiXmlElement* childItem, HTREEITEM hTreeItem, int iGameIdx)
{
	_MYTREEITEM ti;
	if(childItem==NULL)
		return 0;

	*TreeLevel = *TreeLevel +1;

	for( childItem; childItem; childItem=childItem->NextSiblingElement() )
	{			
		ti.dwType = 0;
		ti.dwState = 0;
		ti.iIconIndex = 0;
		ti.hTreeItem = NULL;
		ti.sElementName.clear();
		ti.pGI = NULL;
		ti.bVisible = true;
		ti.bExpanded = false;
		ti.bDelete = false;
		ti.sScript.clear();


		ti.sElementName = childItem->Value();
		ti.sName = childItem->Attribute("name");
		if(iGameIdx==-2)
			childItem->Attribute("game",&ti.cGAMEINDEX);
		else
			ti.cGAMEINDEX = iGameIdx;

		childItem->Attribute("type",(int*)&ti.dwType);
		childItem->Attribute("state",(int*)&ti.dwState);
		childItem->Attribute("expand",&ti.bExpanded);


	
		const TCHAR *szTxt = childItem->GetText();
		if(szTxt!=NULL)
			ti.sScript = szTxt;

		if((ti.dwType==1) && (strcmp(ti.sElementName.c_str(),"Game")==0))
			ti.iIconIndex = gm.Get_GameIcon(ti.cGAMEINDEX);
		else  if((ti.dwType==1) && (strcmp(ti.sName.c_str(),"Internet servers")==0))
			ti.iIconIndex = 12;
		else
			childItem->Attribute("icon",&ti.iIconIndex);//XML_GetTreeItemInt(childItem,"icon");


		ti.dwLevel = *TreeLevel;
		ti.dwIndex = vTI.size();

		if((ti.dwState==1) && (ti.cGAMEINDEX==-1) && (ti.dwType>=13) && (ti.dwType<20))
			m_se.CompileFilter(gm.GetFilterSet(GLOBAL_FILTER),ti.sScript.c_str(), ti.sName.c_str(),ti.sElementName.c_str());
		else if((ti.dwState==1) && (ti.dwType>=13) && (ti.dwType<20))
			m_se.CompileFilter(gm.GetFilterSet(ti.cGAMEINDEX),ti.sScript.c_str(), ti.sName.c_str(),ti.sElementName.c_str());

		vTI.push_back(ti);


		ParseChilds(TreeLevel,childItem->FirstChildElement(),ti.hTreeItem,ti.cGAMEINDEX);		
		
	}	

	*TreeLevel = *TreeLevel - 1;

	return 0;
}




DWORD CTreeViewManager::SwapDWCheckState(TVITEM  *pTVitem, DWORD &dwValue)
{
	if(pTVitem->iImage == CHECKED_ICON) //Is it checked
	{
		pTVitem->iSelectedImage =	pTVitem->iImage = UNCHECKED_ICON;  //Unchecked image
		dwValue = 0;		
	}
	else
	{
		pTVitem->iSelectedImage =	pTVitem->iImage = CHECKED_ICON;  //Checked image
		dwValue = 1;
	}			
	
	TreeView_SetItem(hwndTreeCtrl, pTVitem );
	return dwValue;
}

int CTreeViewManager::DoAction(int iSel, UINT  flags,TVITEM  *tvitem, BOOL bRedraw)
{
	switch(vTI.at(iSel).dwType)
	{	
		case DO_REDRAW_SERVERLIST: 
			if(flags & TVHT_ONITEM)
				return SetCurrentActiveGame(vTI.at(iSel).cGAMEINDEX); 
			return DO_NOTHING;
		case DO_HISTORY : SetCurrentActiveGame(vTI.at(iSel).cGAMEINDEX); return SHOW_HISTORY;
		case DO_MONITOR : SetCurrentActiveGame(vTI.at(iSel).cGAMEINDEX); return SHOW_MONITOR;
		case DO_HIDE_OFFLINE_SERVERS :  
			if(flags & TVHT_ONITEMICON)
				AppCFG.filter.bHideOfflineServers = SwapDWCheckState(tvitem,vTI.at(iSel).dwState); 
			else
				return DO_NOTHING;
			break;
		case DO_CUSTOM_FAVORITE_FILTER:	
			if(vTI.at(iSel).cGAMEINDEX!=GLOBAL_FILTER)
			{
				Filter_custom_favorite_filter(&gm.GamesInfo[vTI.at(iSel).cGAMEINDEX],vTI.at(iSel), tvitem);
				SetCurrentActiveGame(vTI.at(iSel).cGAMEINDEX);			
			}
			return SHOW_CUSTOM_FAVORITE_FILTER;

		case DO_CUSTOM_MODIFIER:
			if(flags & TVHT_ONITEMICON)
			{
				if(vTI.at(iSel).cGAMEINDEX!=GLOBAL_FILTER)
				{
					vTI.at(iSel).dwState = Filter_change(&gm.GamesInfo[vTI.at(iSel).cGAMEINDEX],vTI.at(iSel), tvitem);
					SetCurrentActiveGame(vTI.at(iSel).cGAMEINDEX);
				} else  //Global filter
				{
					vTI.at(iSel).dwState = Filter_change(NULL,vTI.at(iSel), tvitem);
				}
				if(bRedraw)
					Initialize_RedrawServerListThread();
			}
			return DO_NOTHING;
		case DO_CUSTOM_GROUP_FILTER:
			if(flags & TVHT_ONITEMICON)
			{
				if(vTI.at(iSel).cGAMEINDEX!=GLOBAL_FILTER)
				{
					vTI.at(iSel).dwState = Filter_change_group(&gm.GamesInfo[vTI.at(iSel).cGAMEINDEX],vTI.at(iSel), tvitem);
					SetCurrentActiveGame(vTI.at(iSel).cGAMEINDEX);
				} else
				{
					vTI.at(iSel).dwState = Filter_change_group(NULL,vTI.at(iSel), tvitem);
				}
				if(bRedraw)
					Initialize_RedrawServerListThread();
			}
			return DO_NOTHING;
		case DO_COUNTRY_FILTER:
			{
				if(flags & TVHT_ONITEMICON)
				{
					HTREEITEM hChild = TreeView_GetNextItem(hwndTreeCtrl, tvitem->hItem, TVGN_CHILD);
					if(vTI.at(iSel).dwState)
					{   //Is it checked
						tvitem->iSelectedImage =	tvitem->iImage = UNCHECKED_ICON;  //Unchecked image
						Select_all_childs(hChild,false);
					}
					else
					{
						tvitem->iSelectedImage =	tvitem->iImage = CHECKED_ICON;  //Checked image
						Select_all_childs(hChild,true);
					}
					vTI.at(iSel).dwState = tvitem->iSelectedImage - UNCHECKED_ICON ; //set the state
					TreeView_SetItem(hwndTreeCtrl, tvitem );
					Initialize_CountryFilter();
				} else
					return DO_NOTHING; 

			break;
		case DO_REGION_SELECTION:
			{
			
				int val = atoi(vTI.at(iSel).sScript.c_str());
				if(val==255)
				{ //force uncheck all others than ROW
					for(UINT i=0;i<vTI.size();i++)
					{						
						if(vTI.at(i).dwType==DO_REGION_SELECTION)
						{
							int val = atoi(vTI.at(i).sScript.c_str());
							if(val<255)
							{
								TVITEM  _tvitem;
								memset(&_tvitem,0,sizeof(TVITEM));
								_tvitem.hItem = vTI.at(i).hTreeItem;
								_tvitem.mask =  TVIF_SELECTEDIMAGE| TVIF_IMAGE;
								TreeView_GetItem(hwndTreeCtrl, &_tvitem);
								_tvitem.iImage = CHECKED_ICON;
								vTI.at(i).dwState = 1;
								SwapDWCheckState(&_tvitem,vTI.at(i).dwState);
							}
						}
					}
				} else
				{ //force uncheck of ROW
					for(UINT i=0;i<vTI.size();i++)
					{						
						if(vTI.at(i).dwType==DO_REGION_SELECTION)
						{
							int val = atoi(vTI.at(i).sScript.c_str());
							if((val==255) && (vTI.at(i).dwState==1))
							{
								TVITEM  _tvitem;
								memset(&_tvitem,0,sizeof(TVITEM));
								_tvitem.hItem = vTI.at(i).hTreeItem;
								_tvitem.mask =  TVIF_SELECTEDIMAGE| TVIF_IMAGE;
								TreeView_GetItem(hwndTreeCtrl, &_tvitem);
								_tvitem.iImage = CHECKED_ICON;
								vTI.at(i).dwState = 1;
								gm.GamesInfo[vTI.at(iSel).cGAMEINDEX].filter.dwRegion = 0;
								SwapDWCheckState(&_tvitem,vTI.at(i).dwState);
							}
						}
					}
				}
				BYTE newVal = (BYTE)gm.GamesInfo[vTI.at(iSel).cGAMEINDEX].filter.dwRegion;
				if(SwapDWCheckState(tvitem,vTI.at(iSel).dwState))
					newVal |= val;
				else
					newVal ^= val;

				gm.GamesInfo[vTI.at(iSel).cGAMEINDEX].filter.dwRegion =newVal;
				dbg_print("Region %X",gm.GamesInfo[vTI.at(iSel).cGAMEINDEX].filter.dwRegion);
				return DO_NOTHING; 
			}
		default:
		case DO_NOTHING_: 
			return DO_NOTHING; 
		}
	}

	TreeView_SelectItem(hwndTreeCtrl,NULL);	
	SetCurrentActiveGame(g_currentGameIdx);
	Initialize_RedrawServerListThread();
	return DO_NOTHING;
}


int CTreeViewManager::OnSelection(LPARAM lParam)
{
	HTREEITEM hTreeItem;
	if(g_bMouseMenuClick)
		return DO_NOTHING;
	/*= TreeView_GetSelection(hwndTreeCtrl);
	if(hTreeItem==NULL)
	{
		log.AddLogInfo(GS_LOG_DEBUG,  "Inside TreeView_GetSelection2 hTreeItem=NULL");
		return DO_NOTHING;
	}*/
	TVHITTESTINFO tvHTTI;
	POINT lpClickPoint;
	lpClickPoint.x =  LOWORD(lParam);
	lpClickPoint.y =  HIWORD(lParam);
	GetCursorPos(&lpClickPoint);
	ScreenToClient(hwndTreeCtrl,&lpClickPoint);
	ZeroMemory(&tvHTTI,sizeof(TVHITTESTINFO));
	tvHTTI.pt = lpClickPoint;
	dbg_print("point: x:%d  y:%d",tvHTTI.pt.x,tvHTTI.pt.y);
	hTreeItem = TreeView_HitTest(hwndTreeCtrl,&tvHTTI);


	if(tvHTTI.flags & TVHT_NOWHERE)
		return DO_NOTHING;
	if(tvHTTI.flags & TVHT_ONITEM)
		dbg_print("TVHT_ONITEM");
	if(tvHTTI.flags & TVHT_ONITEMICON)
		dbg_print("TVHT_ONITEMICON");
	if(tvHTTI.flags & TVHT_ONITEMINDENT)
		dbg_print("TVHT_ONITEMINDENT");
	if(tvHTTI.flags & TVHT_ONITEMSTATEICON)
		dbg_print("TVHT_ONITEMSTATEICON");
	if(tvHTTI.flags & TVHT_TORIGHT)
		dbg_print("TVHT_TORIGHT");
	if(tvHTTI.flags & TVHT_BELOW)
		dbg_print("TVHT_BELOW");

	if(tvHTTI.flags & TVHT_ABOVE)
		dbg_print("TVHT_ABOVE");	
	if(tvHTTI.flags & TVHT_TOLEFT)
		dbg_print("TVHT_TOLEFT");		

//	if(tvHTTI.hItem!=hTreeItem)
//		return DO_NOTHING;

	USHORT sKey = GetAsyncKeyState(VK_SHIFT);
	if(sKey==0x8001)
		m_bShiftKeyPressed = TRUE;
	else
		m_bShiftKeyPressed = FALSE;

	TVITEM  tvitem;
	ZeroMemory(&tvitem,sizeof(TVITEM));
	char szBuffer[100];
	tvitem.hItem = hTreeItem;
	tvitem.cchTextMax = sizeof(szBuffer);
	tvitem.pszText = szBuffer;
	tvitem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_SELECTEDIMAGE |  TVIF_IMAGE;
	TreeView_GetItem(hwndTreeCtrl, &tvitem );

	int iSel = (int)tvitem.lParam;
	
	if(m_bShiftKeyPressed)
	{
		dbg_print("SHIFT DOWN");

			int ret;
			m_iSecondClickIdx = iSel;
			int lowestIdx,high;

			if(m_iFirstClickIdx<m_iSecondClickIdx)
				lowestIdx = m_iFirstClickIdx;
			else
				lowestIdx = m_iSecondClickIdx;

			if(m_iFirstClickIdx>m_iSecondClickIdx)
				high = m_iFirstClickIdx;
			else
				high = m_iSecondClickIdx;


			for(UINT i=lowestIdx; i<=high;i++)
			{

				ZeroMemory(&tvitem,sizeof(TVITEM));
				tvitem.hItem = vTI.at(i).hTreeItem;
				tvitem.mask = TVIF_PARAM | TVIF_SELECTEDIMAGE |  TVIF_IMAGE;
				TreeView_GetItem(hwndTreeCtrl, &tvitem );

				ret =  DoAction(i,TVHT_ONITEMICON,&tvitem,FALSE);

			}
			Initialize_RedrawServerListThread();
			return ret;

	}
	else
	{
		m_iFirstClickIdx = iSel;
		m_bFirstShiftClick = FALSE;
	}


	//log.AddLogInfo(GS_LOG_DEBUG,"%d %s Action %d",iSel,szBuffer,vTI.at(iSel).dwType);

	return DoAction(iSel, tvHTTI.flags,&tvitem,TRUE);
}



void CTreeViewManager::Filter_Remove(vFILTER_SETS & vFS,const char *szFiltername)
{
	
	for(int i=0; i<vFS.size();i++)
	{
		if(strcmp(vFS.at(i).sFilterName.c_str(),szFiltername)==0)
			vFS.erase(vFS.begin()+i);
	}
	
}
void CTreeViewManager::Filter_Remove_Group(vFILTER_SETS &vFS,_MYTREEITEM ti)
{
	for(int i=0; i<vFS.size();i++)
	{		
		if((strcmp(vFS.at(i).sGroupName.c_str(),ti.sElementName.c_str())==0))
			Filter_Remove(vFS,vFS.at(i).sFilterName.c_str());
	}	
}


int CTreeViewManager::Filter_update(GAME_INFO *pGI,_MYTREEITEM ti,const char *szOldFilterName)
{	
	if(ti.dwState==1)
	{
		vFILTER_SETS &pfs = gm.GetFilterSet(ti.cGAMEINDEX);
		Filter_Remove(pfs,szOldFilterName);
		m_se.CompileFilter(pfs,ti.sScript.c_str(), ti.sName.c_str(),ti.sElementName.c_str());		
	}
	return ti.dwState;
}


int CTreeViewManager::Filter_change(GAME_INFO *pGI,_MYTREEITEM ti, TVITEM *tvi)
{
	SwapDWCheckState(tvi,ti.dwState);

	vFILTER_SETS & pfs = gm.GetFilterSet(ti.cGAMEINDEX);

	if(ti.dwState==1)
	{
		m_se.CompileFilter(pfs,ti.sScript.c_str(), ti.sName.c_str(),ti.sElementName.c_str());		
	}
	else
	{
		Filter_Remove(pfs,ti.sName.c_str());
	}				
	return ti.dwState;
}

int CTreeViewManager::Filter_change_group(GAME_INFO *pGI,_MYTREEITEM ti, TVITEM *tvi)
{
	int GameIdx = ti.cGAMEINDEX; 
	Filter_Remove_Group(gm.GetFilterSet(GLOBAL_FILTER) ,ti);
	if(GameIdx==GLOBAL_FILTER) //ensure we have atleast the active game to deselect
		GameIdx = gm.GamesInfo[g_currentGameIdx].cGAMEINDEX;
	Filter_Remove_Group(gm.GetFilterSet(GameIdx),ti);
	const char * szGroup = ti.sElementName.c_str();

	for(UINT i=0;i<vTI.size();i++)
	{
		const char *Elementname = vTI.at(i).sElementName.c_str();
		if((strcmp(Elementname,szGroup)==0) && (vTI.at(i).dwState==1) && ((vTI.at(i).cGAMEINDEX==GameIdx) || ((vTI.at(i).cGAMEINDEX==-1))))
		{
			TVITEM  tvitem;
			memset(&tvitem,0,sizeof(TVITEM));
			tvitem.hItem = vTI.at(i).hTreeItem;
			tvitem.mask =  TVIF_SELECTEDIMAGE| TVIF_IMAGE;
			TreeView_GetItem(hwndTreeCtrl, &tvitem);
			SwapDWCheckState(&tvitem,vTI.at(i).dwState);
		}
	}
	SwapDWCheckState(tvi,ti.dwState);
	if(ti.dwState==1)
	{
		if(ti.cGAMEINDEX != GLOBAL_FILTER) 
			m_se.CompileFilter(gm.GetFilterSet(GameIdx),ti.sScript.c_str(), ti.sName.c_str(),ti.sElementName.c_str());		
		else
			m_se.CompileFilter(gm.GetFilterSet(GLOBAL_FILTER),ti.sScript.c_str(), ti.sName.c_str(),ti.sElementName.c_str());		
	}
	
	return ti.dwState;
}


int CTreeViewManager::Filter_custom_favorite_filter(GAME_INFO *pGI,_MYTREEITEM ti, TVITEM *tvi)
{
	if(pGI!=NULL)
	{
		pGI->vFilterSetsFavorites.clear();		
		m_se.CompileFilter(pGI->vFilterSetsFavorites,ti.sScript.c_str(), ti.sName.c_str(),ti.sElementName.c_str());
		TreeView_SetItemState(hwndTreeCtrl,gm.GamesInfo[ti.cGAMEINDEX].hTI,TVIS_SELECTED, TVIS_SELECTED);				
	}
	return ti.dwState;
}


void CTreeViewManager::Select_all_childs(HTREEITEM hRoot, bool selected)
{
	TVITEM  tvitem;
	if(hRoot==NULL)
		return;
	char szBuffer[100];
	memset(&tvitem,0,sizeof(TVITEM));
	tvitem.hItem = hRoot;
	tvitem.cchTextMax = sizeof(szBuffer);
	tvitem.pszText = szBuffer;
	tvitem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_SELECTEDIMAGE |  TVIF_IMAGE;

	while(hRoot!=NULL)
	{
		tvitem.hItem = hRoot;

		TreeView_GetItem(hwndTreeCtrl, &tvitem);
		int iSel = (int)tvitem.lParam;
		//log.AddLogInfo(GS_LOG_DEBUG,"%d %s Action %d",iSel,szBuffer,vTI.at(iSel).dwType);

		if(selected==false)
			tvitem.iSelectedImage =	tvitem.iImage = UNCHECKED_ICON;  //Unchecked image
		else
			tvitem.iSelectedImage =	tvitem.iImage = CHECKED_ICON;  //Checked image
		
		vTI.at(iSel).dwState = tvitem.iSelectedImage - UNCHECKED_ICON; //Set value
		TreeView_SetItem(hwndTreeCtrl, &tvitem );
		Select_all_childs(TreeView_GetNextItem( hwndTreeCtrl, hRoot, TVGN_CHILD),selected);
		hRoot =TreeView_GetNextItem( hwndTreeCtrl, hRoot, TVGN_NEXT);			
		
	}
}


void  CTreeViewManager::Initialize_CountryFilter()
{
	CountryFilter.counter=0;
	TVITEM  tvitem;
	ZeroMemory(&tvitem,sizeof(TVITEM));
	hRootCountryFiltersItem = GetTIByType(10);
	if(hRootCountryFiltersItem!=NULL)
	{

		tvitem.hItem = hRootCountryFiltersItem;
		tvitem.mask = TVIF_SELECTEDIMAGE |  TVIF_IMAGE;
		TreeView_GetItem(hwndTreeCtrl, &tvitem );

		if(	tvitem.iImage == UNCHECKED_ICON) //Unchecked image
			AppCFG.bUseCountryFilter = FALSE;
		else
			AppCFG.bUseCountryFilter = TRUE;

		Build_CountryFilter(hRootCountryFiltersItem);
	}
}

DWORD CTreeViewManager::Build_CountryFilter(HTREEITEM hRoot)
{
	
	HTREEITEM hCurrent=NULL;
	HTREEITEM hChild=NULL;
	char szBuffer[55];
	TVITEM  tvitem;
	DWORD dwReturnFromChild = 0;
	DWORD dwReturnToParent = 0;
	memset(&tvitem,0,sizeof(TVITEM));
	tvitem.hItem = hRoot;
	tvitem.pszText = szBuffer;
	tvitem.cchTextMax = sizeof(szBuffer);
	tvitem.mask = TVIF_SELECTEDIMAGE |  TVIF_IMAGE | TVIF_TEXT | TVIF_PARAM;
	hCurrent = hRoot;
	hChild = TreeView_GetNextItem( hwndTreeCtrl, hRoot, TVGN_CHILD);
	if(hChild!=NULL)
	{
		dwReturnFromChild = Build_CountryFilter(hChild);
		if(dwReturnFromChild>0)
		{
			dwReturnToParent = 1;
			tvitem.hItem = hRoot;
			TreeView_GetItem(hwndTreeCtrl, &tvitem );
			if(	tvitem.iImage != CHECKED_ICON)
			{
				tvitem.iImage  = GRAY_CHECKED_ICON;
				TreeView_SetItem(hwndTreeCtrl, &tvitem );
			}
		} else
		{
			tvitem.hItem = hRoot;
			TreeView_GetItem(hwndTreeCtrl, &tvitem );
			if(	tvitem.iImage == GRAY_CHECKED_ICON)
			{
				tvitem.iImage  = UNCHECKED_ICON;
				TreeView_SetItem(hwndTreeCtrl, &tvitem );
			}

		}
	} 

	while(hCurrent!=NULL)
	{

		tvitem.hItem = hCurrent;
		TreeView_GetItem(hwndTreeCtrl, &tvitem );
/*#ifdef _DEBUG
		char szDebugTxt[100];
		sprintf(szDebugTxt,"%d %s - (%d) dwReturnFromChild = %d  dwReturnToRoot = %d",tvitem.iImage,tvitem.pszText,tvitem.lParam,dwReturnFromChild, dwReturnToParent);
		dbg_print(szDebugTxt);
#endif*/
		if(	tvitem.iImage == CHECKED_ICON) //Checked country
		{

			dwReturnToParent = 1;
			int iSel = (int)tvitem.lParam;
			strncpy(CountryFilter.szShortCountryName[CountryFilter.counter],vTI.at(iSel).sScript.c_str(),4);
			CountryFilter.counter++;
		}
		
		hCurrent =TreeView_GetNextItem( hwndTreeCtrl, hCurrent, TVGN_NEXT);
		if(hCurrent!=NULL)
		{
			hChild = TreeView_GetNextItem( hwndTreeCtrl, hCurrent, TVGN_CHILD);
			if(hChild!=NULL)
			{
				dwReturnFromChild = Build_CountryFilter(hChild);
				if(dwReturnFromChild>0)
				{		
					dwReturnToParent = 1;
					tvitem.hItem = hCurrent;
					TreeView_GetItem(hwndTreeCtrl, &tvitem );
					if(	tvitem.iImage != CHECKED_ICON)
					{
						tvitem.iImage  = GRAY_CHECKED_ICON;
						TreeView_SetItem(hwndTreeCtrl, &tvitem );
					} 

				}else
				{
					tvitem.hItem = hCurrent;
					TreeView_GetItem(hwndTreeCtrl, &tvitem );
					if(	tvitem.iImage == GRAY_CHECKED_ICON)
					{
						tvitem.iImage  = UNCHECKED_ICON;
						TreeView_SetItem(hwndTreeCtrl, &tvitem );
					}

				}
			}
		}
		
	}
	return dwReturnToParent;
}


int CTreeViewManager::ReBuildListChild(HTREEITEM hTreeItemParent,int idx,int ParentLevel)
{
	HTREEITEM hTreeItem=NULL;
	int i = idx;
	int level;

	for (i; i<vTI.size();i++)
	{
		_MYTREEITEM ti;
		ti = vTI.at(i);
		
		bool active=true;
		if(ti.cGAMEINDEX!=-1)
		{
			if(ti.cGAMEINDEX<gm.GamesInfo.size())
				active = gm.GamesInfo[ti.cGAMEINDEX].bActive;
		}
		if((ParentLevel==vTI.at(i).dwLevel) && (vTI.at(i).bDelete==FALSE))
		{
			vTI.at(i).hTreeItem = AddItem(&ti,hTreeItemParent, active);
			level = vTI.at(i).dwLevel ;
			hTreeItem = vTI.at(i).hTreeItem;
		} else if (ParentLevel<vTI.at(i).dwLevel)
		{
			i = ReBuildListChild(hTreeItem,i,vTI.at(i).dwLevel);
			hTreeItem = NULL;
			if(i<vTI.size())
			{
				if (vTI.at(i).dwLevel==ParentLevel)
					i--;
				else
					return i;
			}
			else
				return i;

		} else if (ParentLevel>vTI.at(i).dwLevel)
			return i;
		
	}
	return i;
}

void  CTreeViewManager::BuildList(HWND hWndParent,char *ExePath,char*UserPath)
{   
	g_tvIndex = 0;	
	TreeView_DeleteAllItems(hwndTreeCtrl);
	Load(ExePath,UserPath);
	ReBuildList();
	PostMessage(hWndParent,WM_REINIT_COUNTRYFILTER,0,0);

	return;
}

void CTreeViewManager::ReBuildList()
{   
	g_tvIndex = 0;	
	int level=2;
	TreeView_DeleteAllItems(hwndTreeCtrl);
	HTREEITEM hTreeItem=NULL;
	for (int i=0; i<vTI.size();i++)
	{
		if(vTI.at(i).bDelete)
		{
			vTI.erase(vTI.begin()+i);
			i--;
		}

	}
	for (int i=0; i<vTI.size();i++)
	{
		i = ReBuildListChild(hTreeItem,i, level);	
	}
	char szBuffer[256];
	for(int i=0; i<gm.GamesInfo.size();i++)
	{
		gm.GamesInfo[i].hTI = GetHTIByItemGame(i);
		sprintf(szBuffer,"%s (%d)",gm.GamesInfo[i].szGAME_NAME,gm.GamesInfo[i].dwTotalServers);
		if(gm.GamesInfo[i].hTI!=NULL)
		{
			SetItemText(gm.GamesInfo[i].hTI,szBuffer);
			TreeView_SetItemState(hwndTreeCtrl,gm.GamesInfo[i].hTI,TVIS_BOLD ,TVIS_BOLD);
		}

	}
}



HTREEITEM CTreeViewManager::AddItem(_MYTREEITEM *ti,HTREEITEM hCurrent,bool active)
							
{
	 int iImageIndex = ti->iIconIndex+ti->dwState;
	 bool expand = ti->bExpanded;
	
	 char text[256];
	 sprintf(text,"%s",ti->sName.c_str());

	if(active==false)
	{
		g_tvIndex++;
		return NULL;
	}
		
	TVINSERTSTRUCT tvs;
	memset(&tvs,0,sizeof(TVINSERTSTRUCT));
	tvs.item.mask                   = TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
/*	if(ti->dwType==13)
	{
		tvs.item.mask                   = TVIF_STATE|TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
		tvs.item.state = INDEXTOSTATEIMAGEMASK (ti->dwState+1);
		tvs.item.stateMask = TVIS_STATEIMAGEMASK;
	}
*/

	tvs.hParent = hCurrent;
	tvs.item.pszText            = (LPSTR)text;
	
	tvs.item.lParam = g_tvIndex++;
	tvs.item.cchTextMax             = lstrlen(tvs.item.pszText) + 1;

	//tvs.hInsertAfter = TVI_LAST;
	tvs.item.iImage                 = iImageIndex;
	tvs.item.iSelectedImage         = iImageIndex;//62;
	HTREEITEM hNewItem = TreeView_InsertItem(hwndTreeCtrl, &tvs);
	//TreeView_SetCheckState(g_hwndMainTreeCtrl,hNewItem,TRUE);
    
	if (hCurrent && expand)
		TreeView_Expand(hwndTreeCtrl, hCurrent, TVE_EXPAND);
	return hNewItem;
}