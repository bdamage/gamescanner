/*
MIT License

Copyright (c) 2007 Kjell Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


*/


#include "StdAfx.h"
#include "IPtoCountry.h"



#define MAX_LUT 55
LOOKUPTABLE LUT[MAX_LUT];


inline bool operator == (CIPCountry const &a, CIPCountry const &b ) 
{
	return ((b.dwStartIP >= a.dwStartIP) && (b.dwEndIP<= a.dwEndIP));
}

inline bool operator == (CIPCountry  a, CIPCountry * b   ) 
{
	return ((b->dwStartIP >= a.dwStartIP) && (b->dwEndIP<= a.dwEndIP));
}





CIPtoCountry::CIPtoCountry(void):
	m_pszPath(NULL)
{
}

CIPtoCountry::~CIPtoCountry(void)
{
	vIPC.clear();
}

void CIPtoCountry::SetPath(const char *pszPath)
{
	m_pszPath = pszPath;
}

void CIPtoCountry::LoadTable()
{
	CIPCountry tmpipc;
	IPCOUNTRY g_structIP;
	FILE *fp=NULL;
	
	SetCurrentDirectory(m_pszPath);
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
		return;
	}

	//Create a look up table for optimiziation
	DWORD dwSize = vIPC.size();
	DWORD dwPartition = dwSize/100;

	CIPCountry tmpIPC;
	DWORD dwIdx = dwSize;
	for(int i=0;i<100; i++)
	{
		dwIdx-=dwPartition;  //dwHalfQuarter
		tmpIPC  = vIPC.at(dwIdx);

		LUT[i].dwIndex = dwIdx;
		LUT[i].ipc.startIP = tmpIPC.dwStartIP;
	}
}





char * CIPtoCountry::IPtoCountry(DWORD IP, char *szShortName)
{
	vecIPC::iterator vIPCiter;		
	CIPCountry tmpIPC;
	tmpIPC.dwStartIP = IP;
	tmpIPC.dwEndIP = IP;

	DWORD dwStartIdx = 0;
	for(int i=0;i<50; i++)
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
		strcpy(szShortName,"zz");
	}
	return szShortName;
}



int CIPtoCountry::ConvertDatabase(void)
{
	char buffer[512];
	IPCOUNTRY structIP;
	DWORD lines=0;
	DWORD start=0,stop=0;
	int i=0;
    char  *stopstring;

	char seps[]   = ",\t\n\"";
	
	char *token1, *token2,  *token3,  *token4,*token3a,  *next_token1;
	
	SetCurrentDirectory(m_pszPath);

	FILE *fp=NULL;
	fopen_s(&fp,"IpToCountry.csv", "rb");  //"new-ip-to-country.csv"
	if(fp==NULL)
	{

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

	//	swprintf_s(szBuf,sizeof(szBuf)/sizeof(wchar_t),_T(" failed with error %d: %s"), dw, lpMsgBuf); 
	 
		LocalFree(lpMsgBuf);
	//	MessageBox(NULL,TEXT("Error open new country CSV file! \nThis file may be placed in debug/release folder!"),NULL,MB_OK);
		return -1;
	} 
		
	
	FILE *fpNew=NULL;
	FILE *fpNew2=NULL;
	fopen_s(&fpNew,"ipcountry.dat", "wb");
	fopen_s(&fpNew2,"ipcountry.newdat", "wb");
	if(fp!=NULL)
	{						
		char out[200];
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

						sprintf_s(out,"%d %d %s\n",structIP.startIP,structIP.endIP,token3a);
						fwrite( &out, strlen(out), 1, fpNew2 );
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
		fclose(fpNew2);
		OutputDebugString(TEXT("Converting Country database DONE!\n"));
	}
	return 0;
}

