#include "StdAfx.h"
#include "Language.h"
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <time.h>
#include "utilz.h"

CLanguage::CLanguage(void)
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

	char sztemp[200];
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
	
	}
	return 0;
}
const char* CLanguage::GetString(const char* szXmlTagName)
{
	if(DefaultLanguage.size()>0)
		if(DefaultLanguage[szXmlTagName].length()>0)
			return DefaultLanguage[szXmlTagName].c_str();
	
	sprintf(m_szError,"Missing Tag: %s",szXmlTagName);
	return m_szError;
}

int CLanguage::EnumerateLanguage(void)
{
	m_Languages.clear();
   struct _finddata_t lang_file;
   intptr_t hFile;
	SetCurrentDirectory(m_pszDirectory);
   if( (hFile = _findfirst( "lang*.xml", &lang_file )) == -1L )
      AddLogInfo(0, "No lang*.xml files in current directory!" );
   else
   {
      do {
		 AddFile(lang_file.name);
      } while( _findnext( hFile, &lang_file ) == 0 );
      _findclose( hFile );
   }


	return 0;
}
