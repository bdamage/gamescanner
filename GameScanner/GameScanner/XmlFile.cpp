#include "StdAfx.h"
#include "XmlFile.h"


CXmlFile::CXmlFile(void)
{
	m_pDocument = new TiXmlDocument("What evah");
	m_pHandleDoc = new TiXmlHandle(m_pDocument);
}

CXmlFile::~CXmlFile(void)
{
	delete m_pDocument;
	delete m_pHandleDoc;
}
void CXmlFile::SetPath(const char *pszDir)
{
	m_pszDirectory = pszDir;
}
int CXmlFile::load(const char* pszFilename)
{
	m_pDocument->SetCondenseWhiteSpace(false);	
	SetCurrentDirectory(m_pszDirectory);
	if (m_pDocument->LoadFile(pszFilename)==false) 
		return XMLFILE_ERROR_LOADING;

	m_pRootElement=m_pHandleDoc->FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (m_pRootElement==NULL) 
		return XMLFILE_ERROR_LOADING;
	

	return XMLFILE_SUCCESS_LOADING;
}


TiXmlElement * CXmlFile::GetElementSafe(TiXmlElement *pElement,const char *szElementName)
{
	if(pElement!=NULL)
	{
		TiXmlNode *pNode=NULL;
		if(strcmp(pElement->Value(),szElementName)==0)
			return pElement;
		pNode = pElement->FirstChild(szElementName);
		if(pNode!=NULL)
		{
			 return pNode->ToElement();
		}
	}
//	OutputDebugString("Could not find tag <%S> or in pElement is NULL value.",szElementName);
	return NULL;
}

int CXmlFile::GetText(TiXmlElement *pInElement,const char * szElementName, char *pszOut,DWORD dwBufferLen)
{

	TiXmlElement *pElement=NULL;
	if(szElementName!=NULL)
	{
		pElement = GetElementSafe(pInElement,szElementName);
		if(pElement!=NULL)
		{
			const char *szTxt = pElement->GetText();
			if(szTxt!=NULL)
			{
				//if(strlen(szTxt)+1 <= dwBufferLen)
				//	dwBufferLen = strlen(szTxt)+1;
	//			else
	//				OutputDebugString("Tag <%S> value length is overrided, max length is %d - <br>Part of the value will only be readed.",szElementName,dwBufferLen);
				strcpy_s(pszOut, dwBufferLen,szTxt);	
			}
			return TIXML_SUCCESS;	
		}
	}
//	OutputDebugString("Could not find tag <%S> or pInElement is NULL.",szElementName);
	return TIXML_NO_ATTRIBUTE;
}

/***************************
GetInteger Returns
  0 Successfull parse
 -1 Unsuccessfull parse


****************************/
int CXmlFile::GetInteger(TiXmlElement *pElm,char * pszElementName,long *lOut)
{
	TiXmlNode* pNode=NULL;
	pNode = pElm->FirstChild(pszElementName);
	if(pNode!=NULL)
	{		
		TiXmlElement *pElement= pNode->ToElement();
		if(pElement)
		{
			const char *szTxt = pElement->GetText();
			if(szTxt!=NULL)
				*lOut = atol(szTxt);
			return 0;
		}
	}
	//OutputDebugString("Could not find tag %S",elementname);
	return -1;
}


char * CXmlFile::GetAttribute(TiXmlElement* pNode, char *szParamName,char *szOutputBuffer,int iBuffSize)
{
	if(szOutputBuffer==NULL)
		return NULL;
	ZeroMemory(szOutputBuffer,iBuffSize);
	for( pNode; pNode; pNode=pNode->NextSiblingElement())
	{
		
		const char *pName=pNode->Attribute("name");
		if(pName==NULL)
			continue;
		
		if(strcmp(szParamName,pName)==0)
		{
			const char *pValue = pNode->Attribute("value"); // If this fails, original value is left as-is
			//int len = strlen(pValue);
			//strcpy(szOutputBuffer,pValue);
			strncpy(szOutputBuffer,pValue,iBuffSize);
			return szOutputBuffer;					
		}
	}
	//AddLogInfo(0,"Error reading XML tag %s",szParamName);
	return NULL;
}