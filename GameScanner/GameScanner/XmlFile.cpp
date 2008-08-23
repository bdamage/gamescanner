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

int CXmlFile::load(const char* pszFilename)
{
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

/*
	XML file example:
	<IPv4>
	  <IPAddress>10.216.140.74</IPAddress>
	  <Subnet>255.255.248.0</Subnet>
	  <Gateway>10.216.140.1</Gateway>
	</IPv4>

	C code usage:

	char szIPV4Address[MAX_IP_ADDRESS_LENGTH];

	TiXmlElement *pElmIPv4 = GetElementSafe(pElmProf,"IPv4");
	if(pElmIPv4)
	{
		GetText(pElmIPv4,szIPV4Address,"IPAddress",MAX_IP_ADDRESS_LENGTH);
	}

	or
		GetText(GetElementSafe(pElmProf,"IPv4"),szIPV4Address,"IPAddress",MAX_IP_ADDRESS_LENGTH);

*/

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
				if(strlen(szTxt)+1 <= dwBufferLen)
					dwBufferLen = strlen(szTxt)+1;
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
int CXmlFile::GetInteger(TiXmlElement *pElm,long *lOut,char * pszElementName)
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
