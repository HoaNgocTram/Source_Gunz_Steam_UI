#ifndef MZFileSYSTEM_H
#define MZFileSYSTEM_H

#include <list>
#include <map>
#include <string>
#include "MZip.h"

using namespace std;

// File Descrition ( by Zip File System )
struct MZFILEDESC{
	char	m_szFileName[_MAX_PATH];		// ÆÄÀÏ¸í ( ½ÇÁ¦ ÆÄÀÏÀÎ °æ¿ìµµ ÀÖ°í, zip ÆÄÀÏ¾È¿¡ ¼ÓÇØ ÀÖ´Â °æ¿ìµµ ÀÖ´Ù. )
	char	m_szZFileName[_MAX_PATH];		// "" ÀÌ¸é ½ÇÁ¦ ÆÄÀÏÀ» ÀÇ¹ÌÇÑ´Ù.
	int		m_iSize;
	unsigned int	m_crc32;
	unsigned long	m_modTime;	// zip ÆÄÀÏ ¾È¿¡ ÀÖ´Â ÆÄÀÏµéÀ» À§ÇØ dos time À¸·Î ÀúÀåÇÑ´Ù
};

typedef map<string,MZFILEDESC*>	ZFLIST;
typedef ZFLIST::iterator				ZFLISTITOR;

// iterator ¸¦ °¡Áö°íÀÖÀ¸¹Ç·Î create µÈ ÀÌÈÄ¿¡ ÆÄÀÏ¸ñ·ÏÀÌ Ãß°¡µÇ¸é °ï¶õÇÏ´Ù. ±×·±°æ¿ì ÄÚµå ¼öÁ¤ÀÌ ÇÊ¿ä

void GetRefineFilename(char *szRefine, const char *szSource);

// ÆÄÀÏ¸íÀ¸·Î »ç¿ëÇÒ ¼ö ¾ø´Â ¹®ÀÚ°¡ Æ÷ÇÔµÇ¾î ÀÖÀ¸¸é ´ëÃ¼¹®ÀÚ·Î ±³Ã¼ÇÑ´Ù. À©µµ¿ì ÆÄÀÏ¸í ±Ô¾àÀº »ç¿ëºÒ°¡¹®ÀÚ ¿Ü¿¡µµ ¿©·¯°¡Áö°¡ ´õ ÀÖÀ¸¹Ç·Î
// ÀÌ ÇÔ¼ö¸¦ °ÅÃÄµµ ÆÄÀÏ¸í ±Ô¾à¿¡ ¿ÏÀüÈ÷ ºÎÇÕÇÑ´Ù´Â º¸ÀåÀº ¾ø´Ù´Â °ÍÀ» ÁÖÀÇ
void ValidateFilename(char* szOut, const char* szSrc, char substitute);

unsigned MGetCRC32(const char *data, int nLength);
class MZFileCheckList;

class MZFileSystem{
	char		m_szBasePath[256];
	ZFLIST		m_ZFileList;
	ZFLISTITOR	m_iterator;
	int			m_nIndex;

	char		m_szUpdateName[256];

	MZFileCheckList *m_pCheckList;

protected:
	bool AddItem(MZFILEDESC*);

	void RemoveFileList(void);
	// RefreshFileList Pure File List
	void RefreshFileList(const char* szBasePath);
	// Expand Z File List From File List
//	void ExpandZFileList(ZFLIST* pZFList);
	// '\' -> '/'

	int GetUpdatePackageNumber(const char *szPackageFileName);

public:
	static MZFileSystem* GetInstance();

	// THÊM DÒNG NÀY: Để kiểm tra file có tồn tại hay không
	bool IsExist(const char* szFileName);

	MZFileSystem(void);
	virtual ~MZFileSystem(void);

	bool Create(const char* szBasePath,const char* szUpdateName=NULL);
	void Destroy(void);

	int GetFileCount(void) const;
	const char* GetFileName(int i);
	const MZFILEDESC* GetFileDesc(int i);

	const char *GetBasePath() { return m_szBasePath; }

	MZFILEDESC* GetFileDesc(const char* szFileName);

	unsigned int GetCRC32(const char* szFileName);
	unsigned int GetTotalCRC();

	int GetFileLength(const char* szFileName);
	int GetFileLength(int i);

	bool IsZipFile(const char* szFileName);

	bool ReadFile(const char* szFileName, void* pData, int nMaxSize);

	void SetFileCheckList(MZFileCheckList *pCheckList) { m_pCheckList = pCheckList; }
	MZFileCheckList *GetFileCheckList()	{ return m_pCheckList; }


	void SetPrivateKey( const unsigned char* pPrivateKey, size_t length);
};


class MZFile{
public:
	enum SeekPos { begin = 0x0, current = 0x1, end = 0x2 };

protected:
	FILE*	m_fp;
	bool	m_IsZipFile;
	bool	m_IsBufferd;
	bool	m_IsEncrypted;

	MZip	m_Zip;

	char*	m_pData;
	unsigned long	m_nFileSize;
	unsigned long	m_nEncryptFileSize;

	int		m_nPos;

	int		m_nIndexInZip;
	unsigned int	m_crc32;

	char	m_FileName[256];
	char	m_ZipFileName[256];

	static  unsigned long m_dwReadMode;

public:

	MZFile();
	virtual ~MZFile();

	bool Create();

//	bool Open(const char* szFileName);		// ÀÌ ¹æ¹ýÀº ±×³É ÆÄÀÏ¿¡¼­ ¿ÀÇÂ ÇÏ´Â ¹æ¹ýÀ¸·Î ¾Æ·¡ Æã¼Ç¿¡ ÇÕÄ£´Ù.
	bool Open(const char* szFileName, MZFileSystem* pZFS = NULL);
	bool Open(const char* szFileName, const char* szZipFileName, bool bFileCheck = false , unsigned int crc32 = 0);

	bool Seek(long off,int mode);

	void Close(void);

	static void SetReadMode(unsigned long mode) {	m_dwReadMode = mode; }
	static unsigned long GetReadMode(void)  { return m_dwReadMode; }
	static bool isMode(unsigned long mode ) { return (m_dwReadMode & mode) ? true : false ; }

//	bool OpenZip(char* szZipFileName);
//	void CloseZip(void);

	unsigned long GetLength(void);
	bool Read(void* pBuffer, int nMaxSize);

	// ÇÑ¹ø¿¡ ¹Ù·Î ÀÐ¾î¿Â´Ù. ÆÄÀÏ³»ÀÇ seek À§Ä¡¿Í´Â »ó°üÀÌ ¾ø´Ù
//	bool ReadAll(void* pBuffer, int nBufferSize);
};

class MZFileCheckList
{
	unsigned int				m_crc32;		// ÆÄÀÏ ÀÚÃ¼ÀÇ crc32
	map<string,unsigned int>	m_fileList;

public:
	bool Open(const char *szFileName, MZFileSystem *pfs = NULL);

	unsigned int GetCRC32(const char *szFileName);	// Æ¯Á¤ÆÄÀÏÀÇ crc32¸¦ ¸®ÅÏ
	unsigned int GetCRC32()	{ return m_crc32; }		// ½º½º·ÎÀÇ crc32¸¦ ¸®ÅÏ
};

#endif

/*
	////	°£´Ü »ç¿ë¹ý		////

	MZFileSystem fs;

	fs.Create("./");

	char buffer[1000];

	MZFile mzf;

	mzf.Open("6.txt");

	int size = mzf.GetLength();
	mzf.Read(buffer,size);

	mzf.Close();

	mzf.Open("5.txt","test2.zip");
//	mzf.Read(buffer,mzf.GetLength());
	mzf.Read(buffer,4);
	mzf.Read((buffer+4),6);

	mzf.Seek(10,MZFile::begin);
	mzf.Read(buffer,4);

	mzf.Seek(2,MZFile::current);
	mzf.Read(buffer,4);

	mzf.Seek(-2,MZFile::end);
	mzf.Read(buffer,4);

	mzf.Open("4.txt","test2.zip");
	mzf.Read(buffer,mzf.GetLength());

	mzf.Close();

	mzf.Open("5.txt",&fs);
	mzf.Read(buffer,mzf.GetLength());

	mzf.Open("4.txt",&fs);
	mzf.Read(buffer,mzf.GetLength());

	mzf.Open("6.txt",&fs);
	mzf.Read(buffer,mzf.GetLength());

	mzf.Close();

	fs.Destroy();
	*/