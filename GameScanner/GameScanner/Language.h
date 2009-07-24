#pragma once
#include "XmlFile.h"
#include "Logger.h"
#include <string>
#include <map>
using namespace std;

typedef std::map<std::string,std::string> MessageMap;



class CLanguage
{
	CLogger & log;
	MessageMap DefaultLanguage;

	const char *m_pszDirectory;
	CXmlFile xmlFile;
	int AddFile(const char *filename);
public:
	string m_strCurrentLang;
	MessageMap m_Languages;
	TCHAR m_szError[512];	
	CLanguage(CLogger & logger);
	~CLanguage(void);
	int loadFile(const char *filename);
	void SetPath(const char*pszDir);
	const TCHAR* GetString(const char* szXmlTagName);
	int EnumerateLanguage(void);
};
