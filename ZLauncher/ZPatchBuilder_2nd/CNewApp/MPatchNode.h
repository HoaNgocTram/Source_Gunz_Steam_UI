#pragma once

#include <windows.h>
#include <list>
#include <string>
using namespace std;

class MPatchNode {
protected:
    FILETIME        m_tmWrite;    // Thời gian chỉnh sửa file
    unsigned long   m_nSize;      // Kích thước file (byte)
    char            m_szName[_MAX_DIR]; // Đường dẫn file
    string          m_sChecksum;  // Giá trị checksum (MD5)
    bool            m_bValidate;  // Cờ kiểm tra tính hợp lệ

public:
    MPatchNode(const char* pszName, unsigned long nSize, FILETIME tmWrite, string sChecksum);
    virtual ~MPatchNode();

    FILETIME GetWriteTime() { return m_tmWrite; }
    unsigned long GetSize() { return m_nSize; }
    const char* GetName() { return m_szName; }

    string GetChecksum() { return m_sChecksum; }
    void MakeChecksum(); // Tính checksum cho file

    bool IsValid() { return m_bValidate; }
    bool CheckValid(CString* pstrErrorMsg);
    void ForcedSetValid(bool bVal) { m_bValidate = bVal; }
};

class MPatchList : public list<MPatchNode*> {};