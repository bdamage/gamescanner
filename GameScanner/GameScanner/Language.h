#pragma once
#include "XmlFile.h"

#include <string>
#include <map>
using namespace std;

typedef std::map<std::string,std::string> MessageMap;



class CLanguage
{
	MessageMap DefaultLanguage;

	const TCHAR *m_pszDirectory;
	CXmlFile xmlFile;
	int AddFile(const TCHAR *filename);
public:
	string m_strCurrentLang;
	MessageMap m_Languages;
	TCHAR m_szError[200];
	CLanguage(void);
	~CLanguage(void);
	int loadFile(const TCHAR *filename);
	void SetPath(const TCHAR*pszDir);
	const TCHAR* GetString(const TCHAR* szXmlTagName);
	//const TCHAR* GetString(const TCHAR* szXmlTagName);
	int EnumerateLanguage(void);
};
