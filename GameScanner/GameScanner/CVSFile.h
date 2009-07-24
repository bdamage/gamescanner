#pragma once

class CCVSFile 
{
	CLogger log;
	FILE *m_file;
	LPBYTE m_pBuffer;
	LPBYTE m_pPointer;
	UINT m_uiBufferSize;
	UINT m_uiRecordLen;
	
public:
	CCVSFile(void);
	~CCVSFile(void);
	int SetFilename(const char * szFilename);
	int NextRecordSet(void);
	int Open(const char *szFIlename, BOOL bReadOnly);
	BOOL Close();
	int ReadRecordString(char* pszString);
	int ReadRecord(char *&pszString);
	int ReadRecord(long* lValue);
	int ReadRecord(int* lValue);
	int ReadRecord(short* lValue);
	int AllocateBuffer(unsigned int size);
	int FreeBuffer();
	int ReadRecordSet();
	char * RemoveStringQuote(char *szIn);
	char * ReplaceIllegalchars(char *szIn);

	BOOL EofRecordSet();

};

