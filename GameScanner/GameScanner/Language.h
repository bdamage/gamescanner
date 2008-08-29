#pragma once
#include "XmlFile.h"

#include <string>
#include <map>
using namespace std;

typedef std::map<std::string,std::string> MessageMap;



class CLanguage
{
	MessageMap DefaultLanguage;

	const char *m_pszDirectory;
	CXmlFile xmlFile;
	int AddFile(const char *filename);
public:
	string m_strCurrentLang;
	MessageMap m_Languages;
	char m_szError[200];
	CLanguage(void);
	~CLanguage(void);
	int loadFile(const char *filename);
	void SetPath(const char*pszDir);
	const char* GetString(const char* szXmlTagName);
	int EnumerateLanguage(void);
};
