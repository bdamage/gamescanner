// IPtoCountry.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "IPtoCountry.h"

//#include <fcntl.h>     /* for _O_TEXT and _O_BINARY */
//#include <errno.h>     /* for EINVAL */

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

// This is an example of an exported variable
IPTOCOUNTRY_API int nIPtoCountry=0;

//00000000011111111112222222222333333333344444444445
//12345678901234567890123456789012345678901234567890
//THE DEMOCRATIC REPUBLIC OF THE CONGO
//THE FORMER YUGOSLAV REPUBLIC OF MACEDONIA
struct IPCOUNTRY
{
	DWORD startIP;
	DWORD endIP;
	char COUNTRYNAME_MID[4];
	char COUNTRYNAME_SHORT[4];
	char COUNTRYNAME[45];
};
struct LOOKUPTABLE
{
	DWORD dwIndex;
	IPCOUNTRY ipc;
};

#define MAX_LUT 10
LOOKUPTABLE LUT[MAX_LUT];

class CIPCountry
{
public:
	DWORD dwStartIP;
	DWORD dwEndIP;
	char COUNTRYNAME_SHORT[4];
	char COUNTRYNAME_MID[4];
	char COUNTRYNAME[45];
	std::string strCountry;
	CIPCountry& operator = (const CIPCountry &b)
	{
		dwStartIP = b.dwStartIP;
		dwEndIP = b.dwEndIP;
		strcpy_s(COUNTRYNAME,sizeof(COUNTRYNAME),b.COUNTRYNAME);
		strcpy_s(COUNTRYNAME_SHORT,sizeof(COUNTRYNAME_SHORT),b.COUNTRYNAME_SHORT);
		strcpy_s(COUNTRYNAME_MID,sizeof(COUNTRYNAME_MID),b.COUNTRYNAME_MID);
		return *this;
	}
	CIPCountry& operator = (const IPCOUNTRY &other)
	{
		dwStartIP = other.startIP;
		dwEndIP = other.endIP;
		strcpy_s(COUNTRYNAME,sizeof(COUNTRYNAME),other.COUNTRYNAME);
		strCountry = other.COUNTRYNAME;
		strcpy_s(COUNTRYNAME_SHORT,sizeof(COUNTRYNAME_SHORT),other.COUNTRYNAME_SHORT);
		strcpy_s(COUNTRYNAME_MID,sizeof(COUNTRYNAME_MID),other.COUNTRYNAME_MID);
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
	 
		//MessageBox(NULL, szBuf, TEXT("Error"), MB_OK); 

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
						
						token3 = strtok_s( NULL, seps, &next_token1);  //short name 2
						
						token4 = strtok_s( NULL, seps, &next_token1);  //longname name 
						
						strcpy_s(structIP.COUNTRYNAME_SHORT,sizeof(structIP.COUNTRYNAME_SHORT),token3a);
						strcpy_s(structIP.COUNTRYNAME_MID,sizeof(structIP.COUNTRYNAME_MID),token3);
						strcpy_s(structIP.COUNTRYNAME,sizeof(structIP.COUNTRYNAME),token4);

						fwrite( &structIP, sizeof( IPCOUNTRY ), 1, fpNew );

						
						//reset and begin on next line
						//memset(&buffer,0,sizeof(buffer));						
						//i=-1;
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




//typedef vector<IPCOUNTRY> vecIPC;
typedef vector<CIPCountry> vecIPC;
vecIPC vIPC;
typedef map <DWORD, IPCOUNTRY> mIPC;
mIPC mIPcountry;
 

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
				strcpy_s(tmpipc.COUNTRYNAME,sizeof(tmpipc.COUNTRYNAME),g_structIP.COUNTRYNAME);
				strcpy_s(tmpipc.COUNTRYNAME_SHORT,sizeof(tmpipc.COUNTRYNAME_SHORT),g_structIP.COUNTRYNAME_SHORT);
				strcpy_s(tmpipc.COUNTRYNAME_MID,sizeof(tmpipc.COUNTRYNAME_MID),g_structIP.COUNTRYNAME_MID);
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
	DWORD dwHalf = dwSize/2;
	DWORD dwQuarter = dwHalf/2;
	DWORD dwHalfQuarter = dwQuarter/2;


	CIPCountry tmpIPC;
	DWORD dwIdx = dwSize;
	for(int i=0;i<6; i++)
	{
		dwIdx-=dwHalfQuarter;
		tmpIPC  = vIPC.at(dwIdx);

		LUT[i].dwIndex = dwIdx;
		LUT[i].ipc.startIP = tmpIPC.dwStartIP;
		tmpIPC.strCountry.clear();
	}
	return 0;
}
DWORD fnIPtoCountryDeInit()
{
	vIPC.clear();
	return 0;
}


char * fnIPtoCountry2(DWORD IP, DWORD *pSHORTNAME, char *country,char *szShortName)
{
	*pSHORTNAME = 7;
	strcpy(country,"unknown");
	strcpy(szShortName,"zz");
	
	vecIPC::iterator vIPCiter;

	CIPCountry tmpIPC;
	tmpIPC.dwStartIP = IP;
	tmpIPC.dwEndIP = IP;
	IPCOUNTRY inIP;
	inIP.startIP = IP;
	inIP.endIP = IP;

	DWORD dwStartIdx = 0;
	for(int i=0;i<6; i++)
	{		
		if(IP>=LUT[i].ipc.startIP)
		{
			dwStartIdx = LUT[i].dwIndex;
			break;
		}

	}
	

	vIPCiter  = std::find(vIPC.begin()+dwStartIdx,vIPC.end(),&tmpIPC);
	// search_n(vIPC.begin(),vIPC.end(),1,inIP,insideiprange);
	//vIPCiter = upper_bound(vIPC.begin(),vIPC.end(),inIP,insideiprange);

	if(vIPCiter != vIPC.end())
	{	

		tmpIPC =  *vIPCiter;
		//IPCOUNTRY ipc = *vIPCiter;
		strcpy(country,tmpIPC.COUNTRYNAME);
		strcpy(szShortName,tmpIPC.COUNTRYNAME_SHORT);
		//*pSHORTNAME = fnConvertShortNameImageListIndex(tmpIPC.COUNTRYNAME_MID);
		return country;	
	}

	return country;
}