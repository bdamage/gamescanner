#pragma once
#include "XmlFile.h"

#include <string>
#include <map>
using namespace std;

typedef std::map<std::string,std::string> MessageMap;


class CLanguage
{
	MessageMap DefaultLanguage;
	MessageMap CurrentLanguage;
	const char *m_pszDirectory;
	CXmlFile xmlFile;
public:
	char m_szError[200];
	CLanguage(void);
	~CLanguage(void);
	int loadFile(const char *filename);
	void SetPath(const char*pszDir);
	const char* GetString(const char* szXmlTagName);
};
