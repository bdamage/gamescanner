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
#include "Language.h"
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <time.h>
#include "utilz.h"


CLanguage::CLanguage(CLogger & logger) : m_log(logger)
{
	
}

CLanguage::~CLanguage(void)
{
	DefaultLanguage.clear();
}
void CLanguage::SetPath(const char *pszDir)
{
	m_pszDirectory = pszDir;
}

int CLanguage::loadFile(const char *filename)
{

	char sztemp[512];
	ZeroMemory(sztemp,sizeof(sztemp));
	SetCurrentDirectory(m_pszDirectory);	
	if(xmlFile.load(filename)!=XMLFILE_ERROR_LOADING)
	{
		
		TiXmlElement *pElement = xmlFile.m_pRootElement;  
		const char* pLangTag = pElement->Value();
		pElement= pElement->FirstChildElement();
		m_strCurrentLang = pElement->FirstChild()->Value();
		pElement= pElement->NextSiblingElement();
		const char* pVersion = pElement->Value();
		pElement= pElement->NextSiblingElement();
		while(pElement!=NULL)
		{
			xmlFile.GetText(pElement, pElement->Value(),sztemp,sizeof(sztemp)-1);
			DefaultLanguage[pElement->Value()] = sztemp;
			pElement= pElement->NextSiblingElement();			
		}
	}
	return 0;
}

int CLanguage::AddFile(const char *filename)
{
	char sztemp[200];
	ZeroMemory(sztemp,sizeof(sztemp));
	SetCurrentDirectory(m_pszDirectory);
	if(xmlFile.load(filename)!=XMLFILE_ERROR_LOADING)
	{
		TiXmlElement *pElement = xmlFile.m_pRootElement;  
		const char* pLangTag = pElement->Value();
		pElement= pElement->FirstChildElement();
		const char* pName = pElement->FirstChild()->Value();
		m_Languages[pName] = filename;
		pElement= pElement->NextSiblingElement();
		const char* pVersion = pElement->Value();
		pElement= pElement->NextSiblingElement();
		return 1;
	}
	return 0;
}

const TCHAR* CLanguage::GetString(const char* szXmlTagName)
{
	if(DefaultLanguage.size()>0)
	{
		if(DefaultLanguage[szXmlTagName].length()>0)
		{
		//	ZeroMemory(m_szError,sizeof(m_szError));
		//	StringCchPrintf(m_szError,sizeof(m_szError)/sizeof(TCHAR),_T("%s"),DefaultLanguage[szXmlTagName].c_str());
			return DefaultLanguage[szXmlTagName].c_str();
		}
	}
	StringCchPrintf(m_szError,sizeof(m_szError)/sizeof(TCHAR),_T("Missing Tag: %s"),szXmlTagName);
	return m_szError;
}


int CLanguage::EnumerateLanguage(void)
{
	m_Languages.clear();
   struct _finddata_t lang_file;
   intptr_t hFile;
	SetCurrentDirectory(m_pszDirectory);
   if( (hFile = _findfirst( "lang*.xml", &lang_file )) == -1L )
      m_log.AddLogInfo(0, "No lang*.xml files in current directory!" );
   else
   {
      do {
		// AddLogInfo(0, "Detected translation file %s ",lang_file.name );
		 if(AddFile(lang_file.name)==0)
			m_log.AddLogInfo(0, "Error reading translation file %s ",lang_file.name );
      } while( _findnext( hFile, &lang_file ) == 0 );
      _findclose( hFile );
   }


	return 0;
}
