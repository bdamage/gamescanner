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
	if(xmlFile.load("se_lang.xml")!=XMLFILE_ERROR_LOADING)
	{

		xmlFile.GetText(xmlFile.m_pRootElement,"NewVersion",sztemp,sizeof(sztemp)-1);
	}


	return 0;
}
