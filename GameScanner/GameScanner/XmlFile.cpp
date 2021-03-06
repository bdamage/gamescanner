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

#include "StdAfx.h"
#include "XmlFile.h"


CXmlFile::CXmlFile(void)
{
	m_pDocument = new TiXmlDocument("What evah");
	m_pHandleDoc = new TiXmlHandle(m_pDocument);
	m_pszDirectory = NULL;
	m_pRootElement = NULL;
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

TiXmlHandle CXmlFile::GetHandle()
{
	return TiXmlHandle(m_pRootElement);
}

int CXmlFile::load(const char* pszFilename)
{
	m_pDocument->SetCondenseWhiteSpace(false);	
	
#ifdef UNICODE
	SetCurrentDirectory(m_pwszDirectory);
#else
	if(m_pszDirectory!=NULL)
		SetCurrentDirectory(m_pszDirectory);
#endif
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

char * CXmlFile::GetCustomAttribute(TiXmlElement* pNode, char *szAttributeName,char *szOutputBuffer,int iBuffSize)
{
	if(szOutputBuffer==NULL)
		return NULL;
	ZeroMemory(szOutputBuffer,iBuffSize);
	for( pNode; pNode; pNode=pNode->NextSiblingElement())
	{

		const char *pValue = pNode->Attribute(szAttributeName); 
		if(pValue!=NULL)
		{		
			strncpy(szOutputBuffer,pValue,iBuffSize);
			return szOutputBuffer;		
		}
		
	}
	return NULL;		
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

