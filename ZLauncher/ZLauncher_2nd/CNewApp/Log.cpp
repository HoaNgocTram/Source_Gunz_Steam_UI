#include "stdafx.h"
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>
#include <windows.h>

void InitLog()
{
    // nếu bạn từng disable log bằng return; thì bỏ dòng return đó đi

    // Nếu file đã tồn tại, xóa nó trước khi tạo mới
    const char* filename = "patchlog.txt";
    if (GetFileAttributesA(filename) != INVALID_FILE_ATTRIBUTES) {
        // file tồn tại -> xóa
        DeleteFileA(filename);
    }

    FILE* pFile = fopen(filename, "w");
    if (pFile == NULL) {
        // không thể tạo file -> thôi
        return;
    }

    __time64_t ltime;
    _time64(&ltime);
    // _ctime64 trả về chuỗi có newline ở cuối
    fprintf(pFile, "Log start : %s", _ctime64(&ltime));

    char szDir[512];
    GetCurrentDirectoryA(sizeof(szDir), szDir);
    fprintf(pFile, "Current Directory : %s\n", szDir);

    fclose(pFile);
}

void PutLog(const char* pszLog)
{
    if (pszLog == NULL) return;

    const char* filename = "patchlog.txt";
    FILE* pFile = fopen(filename, "a");
    if (pFile == NULL) return;

    char szBuff[16];
    _strtime(szBuff);
    struct __timeb64 tstruct;
    _ftime64(&tstruct);
    // in thời gian dạng hh:mm:ss:ms
    fprintf(pFile, "%s:%03d   ", szBuff, tstruct.millitm);

    // An toàn: in chuỗi bằng format "%s" để tránh vulnerabilities
    fprintf(pFile, "%s", pszLog);
    fprintf(pFile, "\n");

    fclose(pFile);
}
