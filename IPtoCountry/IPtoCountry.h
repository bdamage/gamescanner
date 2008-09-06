// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the IPTOCOUNTRY_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// IPTOCOUNTRY_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef IPTOCOUNTRY_EXPORTS
#define IPTOCOUNTRY_API __declspec(dllexport)
#else
#define IPTOCOUNTRY_API __declspec(dllimport)
#endif


extern IPTOCOUNTRY_API int nIPtoCountry;

IPTOCOUNTRY_API int fnConvertDatabase(void);
IPTOCOUNTRY_API bool IPC_SetPath(char *szPath);
IPTOCOUNTRY_API DWORD fnIPtoCountryDeInit();
IPTOCOUNTRY_API DWORD fnIPtoCountryInit();
IPTOCOUNTRY_API char * fnIPtoCountry2(DWORD IP, char *country,char *szShortName);
