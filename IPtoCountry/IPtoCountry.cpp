// IPtoCountry.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "IPtoCountry.h"
#include <list>

using namespace std;

#ifdef _MANAGED
#pragma managed(push, off)
#endif

char IPC_g_szPath[MAX_PATH+_MAX_FNAME];

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			fnIPtoCountryDeInit();
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

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



#define MAX_LUT 35
LOOKUPTABLE LUT[MAX_LUT];

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

inline bool operator == (CIPCountry const &a, CIPCountry const &b ) 
{
	return ((b.dwStartIP >= a.dwStartIP) && (b.dwEndIP<= a.dwEndIP));
}

inline bool operator == (CIPCountry  a, CIPCountry * b   ) 
{
	return ((b->dwStartIP >= a.dwStartIP) && (b->dwEndIP<= a.dwEndIP));
}


typedef vector<CIPCountry> vecIPC;
vecIPC vIPC;

IPTOCOUNTRY_API bool IPC_SetPath(char *szPath)
{
	memset(IPC_g_szPath,0,sizeof(IPC_g_szPath));
	int len = strlen(szPath);
	if(len>=sizeof(IPC_g_szPath))
	{
		OutputDebugString("Error setting path in IP country dll!\n");
		return false;
		
	}

	strncpy_s(IPC_g_szPath,sizeof(IPC_g_szPath),szPath,len);
	OutputDebugString("IP Country Path: ");
	OutputDebugString(IPC_g_szPath);
	OutputDebugString("\n\n");
	return true;

}



IPTOCOUNTRY_API int fnConvertDatabase(void)
{

//char country[256];
//"2.6.190.56","2.6.190.63","33996344","33996351","GB","United Kingdom"
//char * IPtoCountry(DWORD IP)
//{
/*
"33996344","33996351","GB","GBR","UNITED KINGDOM"
"50331648","69956103","US","USA","UNITED STATES"
"69956104","69956111","BM","BMU","BERMUDA"

IP Number = A x (256*256*256) + B x (256*256) + C x 256 + D
Which is the same as:

IP Number = A x 16777216 + B x 65536 + C x 256 + D
213 132		125 56

In PHP 4 you can use the following to convert a dotted IP Address to its corresponding IP Number:

$ip_number = sprintf("%u", ip2long($dotted_ip_address));

and this to convert IP Number to its corresponding dotted IP Address:

$dotted_ip_address = long2ip($ip_number);
*/
//   IP = (213*16777216)+(132*65536)+(125*256)+56;

	char buffer[512];
	IPCOUNTRY structIP;
	DWORD lines=0;
	DWORD start=0,stop=0;
	int i=0;
    char  *stopstring;

	char seps[]   = ",\t\n\"";
	
	char *token1, *token2,  *token3,  *token4,*token3a,  *next_token1;
	
	SetCurrentDirectory(IPC_g_szPath);

	FILE *fp=NULL;
	fopen_s(&fp,"IpToCountry.csv", "rb");  //"new-ip-to-country.csv"
	if(fp==NULL)
	{
		TCHAR szBuf[200]; 
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError(); 

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

		wsprintf(szBuf,TEXT(" failed with error %d: %s"), dw, lpMsgBuf); 
	 
		LocalFree(lpMsgBuf);
	//	MessageBox(NULL,TEXT("Error open new country CSV file! \nThis file may be placed in debug/release folder!"),NULL,MB_OK);
		return -1;
	} 
		
	
	FILE *fpNew=NULL;
	fopen_s(&fpNew,"ipcountry.dat", "wb");

	if(fp!=NULL)
	{
		while( !feof( fp ) )
		{
			//memset(&buffer,0,sizeof(buffer));
			for(i=0; i<511;i++)
			{
				if(fread(&buffer[i], 1, 1, fp)!=0)
				{

					if(buffer[i]==10)  //read until line is done
					{
						if(buffer[0]==10 || buffer[0]=='#')
							break;
						memset(&structIP,0,sizeof(structIP));
						token1=NULL;
						token2=NULL;
						token3=NULL;
						token4=NULL;
						token1 = strtok_s( buffer, seps, &next_token1);
						structIP.startIP = (DWORD)strtoul(token1,&stopstring,10); //startIP
						token1 = strtok_s( NULL, seps, &next_token1);
						structIP.endIP = (DWORD)strtoul(token1,&stopstring,10);  //endIP

						token2 = strtok_s( NULL, seps, &next_token1);  //
						
						token2 = strtok_s( NULL, seps, &next_token1);  //ASSIGN

						token3a = strtok_s( NULL, seps, &next_token1);  //short name 1
						
						strcpy_s(structIP.COUNTRYNAME_SHORT,sizeof(structIP.COUNTRYNAME_SHORT),token3a);
						fwrite( &structIP, sizeof( IPCOUNTRY ), 1, fpNew );
						lines++;
						break;  //break the for loop
					}
				} else
					break;
	
				
			}			
		}
		fclose(fp);
		fclose(fpNew);
		OutputDebugString(TEXT("Converting Country database DONE!\n"));
	}
	return 0;
}



DWORD fnIPtoCountryInit()
{

	CIPCountry tmpipc;
	IPCOUNTRY g_structIP;
	FILE *fp=NULL;
	
	SetCurrentDirectory(IPC_g_szPath);
	fopen_s(&fp,"ipcountry.dat", "rb");

	if(fp!=NULL)
	{
		while( !feof( fp ) )
		{
			if(fread(&g_structIP, sizeof(g_structIP), 1, fp)!=0)
			{
				strcpy_s(tmpipc.COUNTRYNAME_SHORT,sizeof(tmpipc.COUNTRYNAME_SHORT),g_structIP.COUNTRYNAME_SHORT);
				tmpipc.dwEndIP = g_structIP.endIP;
				tmpipc.dwStartIP = g_structIP.startIP;
				vIPC.push_back(tmpipc);
			}
		}
	
		fclose(fp);
	} else
	{
		MessageBox(NULL,TEXT("Couldn't load Country database!"),TEXT("Error"),MB_OK);
		return 0;
	}

	//Create a look up table for optimiziation
	DWORD dwSize = vIPC.size();
	DWORD dwPartition = dwSize/30;

	CIPCountry tmpIPC;
	DWORD dwIdx = dwSize;
	for(int i=0;i<30; i++)
	{
		dwIdx-=dwPartition;  //dwHalfQuarter
		tmpIPC  = vIPC.at(dwIdx);

		LUT[i].dwIndex = dwIdx;
		LUT[i].ipc.startIP = tmpIPC.dwStartIP;
	}
	return 0;
}

DWORD fnIPtoCountryDeInit()
{
	vIPC.clear();
	return 0;
}


char * fnIPtoCountry(DWORD IP, char *szShortName)
{
	vecIPC::iterator vIPCiter;		
	CIPCountry tmpIPC;
	tmpIPC.dwStartIP = IP;
	tmpIPC.dwEndIP = IP;

	DWORD dwStartIdx = 0;
	for(int i=0;i<30; i++)
	{		
		if(IP>=LUT[i].ipc.startIP)
		{
			dwStartIdx = LUT[i].dwIndex;
			break;
		}

	}
	vIPCiter  = std::find(vIPC.begin()+dwStartIdx,vIPC.end(),&tmpIPC);
	if(vIPCiter != vIPC.end())
	{	
		tmpIPC =  *vIPCiter;
		strcpy(szShortName,tmpIPC.COUNTRYNAME_SHORT);
		return szShortName;	
	} else
	{
		strcpy(szShortName,"ZZ");
	}
	return szShortName;
}