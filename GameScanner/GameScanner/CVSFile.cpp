#include "StdAfx.h"
#include "CVSFile.h"

CCVSFile::CCVSFile(void) : 
	m_file(NULL),
	m_pBuffer(NULL),
	m_pPointer(NULL)
{
	m_uiBufferSize = 3000;
	AllocateBuffer(m_uiBufferSize);
}

CCVSFile::~CCVSFile(void)
{
	FreeBuffer();
	Close();
}

int CCVSFile::SetFilename(const char * szFilename)
{
	return 0;
}

int CCVSFile::NextRecordSet(void)
{
	return ReadRecordSet();
}

int CCVSFile::ReadRecord(char* &pszString)
{
//	if(pszString==NULL)
//		return  -1;

	char *pStart = (char*)m_pPointer;
	if(pStart==NULL)
		return -1;

	while(((char*)m_pPointer-pStart)<m_uiRecordLen)
	{
		if((m_pPointer[0]==0x09) || (m_pPointer[0]==0x0A))
		break;
		m_pPointer++;	
	}
	LPBYTE TrimEnd = m_pPointer;
	TrimEnd--; //skip end quote
	TrimEnd[0]=0;
	m_pPointer[0]=0;
	m_pPointer++;
	pStart++; //skip start quote for strings

	pszString = _strdup(pStart);

	return 0;
}


int CCVSFile::ReadRecordString(char* pszString)
{
	if(pszString==NULL)
		return  -1;

	char *pStart = (char*)m_pPointer;
	if(pStart==NULL)
		return -1;

	while(((char*)m_pPointer-pStart)<m_uiRecordLen)
	{
		if((m_pPointer[0]==0x09) || (m_pPointer[0]==0x0A))
		break;
		m_pPointer++;	
	}
	LPBYTE TrimEnd = m_pPointer;
	TrimEnd--; //skip end quote
	TrimEnd[0]=0;
	m_pPointer[0]=0;
	m_pPointer++;
	pStart++; //skip start quote for strings

	strcpy(pszString,pStart);

	return 0;
}

int CCVSFile::ReadRecord(long* lValue)
{
	char *pStart = (char*)m_pPointer;
	if(pStart==NULL)
		return -1;


	while(!EofRecordSet()) //((char*)m_pPointer-pStart)<m_uiRecordLen)
	{
		if((m_pPointer[0]==0x09) || (m_pPointer[0]==0x0A))
		break;
		m_pPointer++;	
	}
	m_pPointer[0]=0;
	m_pPointer++;


	*lValue = atoi(pStart);

	return 0;
}

int CCVSFile::ReadRecord(short* lValue)
{
	char *pStart = (char*)m_pPointer;
	if(pStart==NULL)
		return -1;


	while(!EofRecordSet()) //((char*)m_pPointer-pStart)<m_uiRecordLen)
	{
		if((m_pPointer[0]==0x09) || (m_pPointer[0]==0x0A))
		break;
		m_pPointer++;	
	}
	m_pPointer[0]=0;
	m_pPointer++;


	*lValue = (short)atoi(pStart);

	return 0;
}
int CCVSFile::ReadRecord(int* lValue)
{
	char *pStart = (char*)m_pPointer;
	if(pStart==NULL)
		return -1;


	while(!EofRecordSet()) //((char*)m_pPointer-pStart)<m_uiRecordLen)
	{
		if((m_pPointer[0]==0x09) || (m_pPointer[0]==0x0A))
		break;
		m_pPointer++;	
	}
	m_pPointer[0]=0;
	m_pPointer++;


	*lValue = (int)atoi(pStart);

	return 0;
}
BOOL CCVSFile::EofRecordSet()
{
	if(m_uiRecordLen==0)
		return TRUE;
	return (m_pPointer>=m_pBuffer+m_uiRecordLen);
}

int CCVSFile::AllocateBuffer(unsigned int size)
{
	FreeBuffer();
	m_pBuffer =(LPBYTE)calloc(1,size);
	return size;
}
int CCVSFile::FreeBuffer()
{
	if(m_pBuffer)
		free((LPVOID)m_pBuffer);
	return 0;
}

int CCVSFile::ReadRecordSet()
{
	m_uiRecordLen = 0;
	while(!feof( m_file ) )
	{
		memset(m_pBuffer,0,m_uiBufferSize);
		for(int i=0; i<m_uiBufferSize;i++)
		{	
			if(fread(&m_pBuffer[i], 1, 1, m_file)!=0)
			{
				if(m_pBuffer[i]==0x0A)//Read to end of line
				{				
					m_uiRecordLen=i;
					m_pPointer = &m_pBuffer[0];
					return 1;
				}
			} else
			{
			//	OutputDebugString("Error reading from file!\n");
				return 0;
			}
		}
		OutputDebugString("Error buffer to small!\n");
	}
	return 0;
}

int CCVSFile::Open(const char *szFilename, BOOL bReadOnly)
{
	m_file = NULL;
	errno_t err=0;
	if(bReadOnly)
		err = fopen_s(&m_file,szFilename, "rb");
	else
		err = fopen_s(&m_file,szFilename, "wb");

	return err;
}

BOOL CCVSFile::Close()
{
	if(m_file!=NULL)
		fclose(m_file);
	return TRUE;
}



char * CCVSFile::RemoveStringQuote(char *szIn)
{	
	if(szIn==NULL)
		return NULL;
	char *p=NULL;
	char *s=NULL;
	s = strchr(szIn,'"');	
	p = strrchr(szIn,'"');	
	if(s!=p)
	{
		if(p!=NULL)
			p[0] = 0;
		if(s!=NULL)
			s++;

	}else
	{
		if(s!=NULL)
			s++;
		else
			s = szIn;
	}
	return s;
}

char * CCVSFile::ReplaceIllegalchars(char *szIn)
{
	char *p;
	if(szIn==NULL)
		return NULL;
	int i = 0;
	p = szIn;
	while(p[i]!=0)
	{
		if(p[i]==0x0a)
			p[i] = 0x20;
		else if(p[i]==0x0d)
			p[i] = 0x20;
		else if(p[i]==0x09)
			p[i] = 0x20;
		i++;
	}
	return szIn;
}
