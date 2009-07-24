#pragma once

#include <tinyxml.h>

#define XMLFILE_SUCCESS_LOADING 0
#define XMLFILE_ERROR_LOADING 1

class CXmlFile
{
private:
	TiXmlDocument *m_pDocument;
	TiXmlHandle *m_pHandleDoc;  //Document handler
	const char *m_pszDirectory;
	

public:
	TiXmlElement *m_pRootElement;  //This will point to the first root tag <>
	CXmlFile(void);
	~CXmlFile(void);
	TiXmlHandle GetHandle();
	int load(const char* pszFilename);	
	TiXmlElement * CXmlFile::GetElementSafe(TiXmlElement *pElement,const char *szElementName);
	int GetText(TiXmlElement *pInElement,const char * szElementName, char *pszOut,DWORD dwBufferLen);
	int GetInteger(TiXmlElement *pElm,char * pszElementName,long *lOut);
	void SetPath(const char *pszDir);
	char *GetAttribute(TiXmlElement* pNode, char *szParamName,char *szOutputBuffer,int iBuffSize);
	char * GetCustomAttribute(TiXmlElement* pNode, char *szAttributeName,char *szOutputBuffer,int iBuffSize);
	const char * GetByName(TiXmlElement* pNode,char *szOutput, DWORD maxBytes);
};
