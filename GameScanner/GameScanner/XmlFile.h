#pragma once

#include "..\..\tinyxml\tinyxml.h"

#define XMLFILE_SUCCESS_LOADING 0
#define XMLFILE_ERROR_LOADING 1

class CXmlFile
{
private:
	TiXmlDocument *m_pDocument;
	TiXmlHandle *m_pHandleDoc;  //Document handler
	const TCHAR *m_pszDirectory;
	

public:
	TiXmlElement *m_pRootElement;  //This will point to the first root tag <>
	CXmlFile(void);
	~CXmlFile(void);
	int load(const TCHAR* pszFilename);	
	TiXmlElement * CXmlFile::GetElementSafe(TiXmlElement *pElement,const TCHAR *szElementName);
	int GetText(TiXmlElement *pInElement,const TCHAR * szElementName, TCHAR *pszOut,DWORD dwBufferLen);
	int GetInteger(TiXmlElement *pElm,TCHAR * pszElementName,long *lOut);
	void SetPath(const TCHAR *pszDir);
	TCHAR *GetAttribute(TiXmlElement* pNode, TCHAR *szParamName,TCHAR *szOutputBuffer,int iBuffSize);
	TCHAR * CXmlFile::GetCustomAttribute(TiXmlElement* pNode, TCHAR *szAttributeName,TCHAR *szOutputBuffer,int iBuffSize);
};
