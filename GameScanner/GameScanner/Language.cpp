#include "StdAfx.h"
#include "Language.h"

CLanguage::CLanguage(void)
{
}

CLanguage::~CLanguage(void)
{
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
	if(xmlFile.load("en_lang.xml")!=XMLFILE_ERROR_LOADING)
	{
		TiXmlElement *pElement = xmlFile.m_pRootElement;  
		const char* pLangTag = pElement->Value();
		pElement= pElement->FirstChildElement();
		const char* pName = pElement->Value();
		pElement= pElement->NextSiblingElement();
		const char* pVersion = pElement->Value();
		pElement= pElement->NextSiblingElement();


		while(pElement!=NULL)
		{
			xmlFile.GetText(pElement, pElement->Value(),sztemp,sizeof(sztemp)-1);
			OutputDebugString(pElement->Value());
			OutputDebugString("=");
			OutputDebugString(sztemp);
			OutputDebugString("\n");
			DefaultLanguage[pElement->Value()] = sztemp;
			pElement= pElement->NextSiblingElement();			
		}
			
		

	}


	return 0;
}

const char* CLanguage::GetString(const char* szXmlTagName)
{
	if(DefaultLanguage[szXmlTagName].length()>0)
		return DefaultLanguage[szXmlTagName].c_str();
	char szTemp[200];
	sprintf("Missing String Value!\nTagname: %s",szXmlTagName);
	return szTemp;
}
