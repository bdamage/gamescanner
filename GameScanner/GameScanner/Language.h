#pragma once
#include "XmlFile.h"

class CLanguage
{
	const char *m_pszDirectory;
	CXmlFile xmlFile;
public:
	CLanguage(void);
	~CLanguage(void);
	int loadFile(const char *filename);
	void SetPath(const char*pszDir);
};
