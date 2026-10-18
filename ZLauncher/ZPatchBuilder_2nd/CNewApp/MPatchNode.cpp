#include "StdAfx.h"
#include "MPatchNode.h"
#include "MMD5.h"
#include "FileInfo.h"
#include "Log.h"
#include <io.h>
#include <string>

// MPatchNode
MPatchNode::MPatchNode(const char* pszName, unsigned long nSize, FILETIME tmWrite, string sChecksum)
{
    if (strlen(pszName) >= _MAX_DIR)
    {
        PutLog("[MPatchNode] ERROR: File path too long");
        throw std::exception("File path too long");
    }
    strcpy(m_szName, pszName);
    m_nSize = nSize;
    m_tmWrite = tmWrite;
    m_sChecksum = sChecksum;
    m_bValidate = false;

    char szMsg[512];
    sprintf(szMsg, "[MPatchNode] Created node for file: %s, size: %u", m_szName, m_nSize);
    PutLog(szMsg);
}

MPatchNode::~MPatchNode()
{
}

void MPatchNode::MakeChecksum()
{
    MMD5 m;
    if (m.md5_file(m_szName, m_sChecksum) != 0)
    {
        char szMsg[512];
        sprintf(szMsg, "[MPatchNode] Failed to calculate checksum for %s", m_szName);
        PutLog(szMsg);
    }
    else
    {
        char szMsg[512];
        sprintf(szMsg, "[MPatchNode] Calculated checksum for %s: %s", m_szName, m_sChecksum.c_str());
        PutLog(szMsg);
    }
}

std::string GetCRC(const char* szFileName, CString* pstrErrorMsg)
{
    MMD5 m;
    std::string sMd5;
    if (m.md5_file((char*)szFileName, sMd5) != 0)
    {
        char szMsg[512];
        sprintf(szMsg, "[MPatchNode] Failed to calculate CRC for %s", szFileName);
        PutLog(szMsg);
        return "";
    }
    return sMd5;
}

bool MPatchNode::CheckValid(CString* pstrErrorMsg)
{
    m_bValidate = false;

    CString filePath = GetName();
    if (filePath.Left(2) == _T("./"))
        filePath = filePath.Mid(2);

    WIN32_FIND_DATA FindData;
    HANDLE hFind;
    if ((hFind = FindFirstFile(filePath, &FindData)) == INVALID_HANDLE_VALUE)
    {
        char szMsg[512];
        sprintf(szMsg, "[MPatchNode] File not found: %s", filePath.GetString());
        PutLog(szMsg);
        return false;
    }

    if (GetSize() != FindData.nFileSizeLow || CompareFileTime(&m_tmWrite, &FindData.ftLastWriteTime) != 0)
    {
        char szMsg[512];
        sprintf(szMsg, "[MPatchNode] Size or timestamp mismatch for %s", filePath.GetString());
        PutLog(szMsg);
        FindClose(hFind);
        return false;
    }

    FindClose(hFind);

    string sMd5 = GetCRC(filePath.GetString(), pstrErrorMsg);
    if (stricmp(sMd5.c_str(), GetChecksum().c_str()))
    {
        char szMsg[512];
        sprintf(szMsg, "[MPatchNode] Checksum mismatch: Expected %s, Got %s for %s", GetChecksum().c_str(), sMd5.c_str(), filePath.GetString());
        PutLog(szMsg);
        return false;
    }

    m_bValidate = true;
    char szMsg[512];
    sprintf(szMsg, "[MPatchNode] File validated: %s", filePath.GetString());
    PutLog(szMsg);

    return m_bValidate;
}