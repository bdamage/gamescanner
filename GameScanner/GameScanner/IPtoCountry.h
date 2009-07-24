#pragma once

struct IPCOUNTRY
{
	DWORD startIP;
	DWORD endIP;
	char COUNTRYNAME_SHORT[4];

};
struct LOOKUPTABLE
{
	DWORD dwIndex;
	IPCOUNTRY ipc;
};


class CIPCountry
{
public:
	DWORD dwStartIP;
	DWORD dwEndIP;
	char COUNTRYNAME_SHORT[4];
	CIPCountry& operator = (const CIPCountry &b)
	{
		dwStartIP = b.dwStartIP;
		dwEndIP = b.dwEndIP;
		strcpy_s(COUNTRYNAME_SHORT,sizeof(COUNTRYNAME_SHORT),b.COUNTRYNAME_SHORT);
		return *this;
	}
	CIPCountry& operator = (const IPCOUNTRY &other)
	{
		dwStartIP = other.startIP;
		dwEndIP = other.endIP;
		strcpy_s(COUNTRYNAME_SHORT,sizeof(COUNTRYNAME_SHORT),other.COUNTRYNAME_SHORT);
	
		return *this;
	}	
};

typedef vector<CIPCountry> vecIPC;

class CIPtoCountry
{
	vecIPC vIPC;
	const char *m_pszPath;
public:
	CIPtoCountry(void);
	~CIPtoCountry(void);
	void SetPath(const char *pszPath);
	void LoadTable();
	char * IPtoCountry(DWORD IP, char *szShortName);
	int ConvertDatabase(void);
};
