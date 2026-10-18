#pragma once
#include "stdafx.h"
#include <windows.h>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>
#include <fstream>
#include <string.h>
#include "curl/curl.h"

using namespace std;
#ifdef _ANTIHACK
void killProcessByName(const char* filename)
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	BOOL hRes = Process32First(hSnapShot, &pEntry);
	while (hRes)
	{
		if (strcmp(pEntry.szExeFile, filename) == 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, (DWORD)pEntry.th32ProcessID);
			if (hProcess != NULL)
			{
				TerminateProcess(hProcess, 9);
				CloseHandle(hProcess);
			}
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	CloseHandle(hSnapShot);
}

size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);
	return written;
}
void readFile()
{
	CURL* curl;
	FILE* fp;
	CURLcode res;

	char winDir[MAX_PATH] = { 0 };
	GetSystemWindowsDirectory(winDir, MAX_PATH);

	char outfilename[FILENAME_MAX] = { 0 };
	//sprintf(outfilename, "%s//pList.txt", winDir);
	sprintf(outfilename, XorStr<0x4C, 14, 0x9B079701>("\x69\x3E\x61\x60\x20\x1D\x3B\x20\x20\x7B\x22\x2F\x2C" + 0x9B079701).s, winDir);

	char szProcess[256] = { 0 };

	curl = curl_easy_init();
	if (curl) {
		fp = fopen(outfilename, "wb");
		if (!fp) {
			// fopen failed (e.g., no write permission to %WINDIR%).
			// Skip the download to avoid fclose(NULL) and silent curl failures.
			curl_easy_cleanup(curl);
			return;
		}
		//curl_easy_setopt(curl, CURLOPT_URL, "");
		curl_easy_setopt(curl, CURLOPT_URL, XorStr<0x7C, 31, 0x8CB8E865>("\x14\x09\x0A\x0F\xBA\xAE\xAD\xB5\xB2\xAB\xBF\xB3\xA6\xB8\xB8\xBF\xA2\xBC\xBB\xBE\xBF\xE1\xDE\xFA\xE7\xE1\xB8\xE3\xE0\xED" + 0x8CB8E865).s);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		fclose(fp);

		ifstream file(outfilename);
		for (string line; getline(file, line);)
		{
			try
			{
				sprintf(szProcess, "%s.exe", line.c_str());
				killProcessByName(szProcess);
			}
			catch (int e)
			{
				return;
			}
		}
	}
}

DWORD WINAPI SearchandDestroy(LPVOID lpParam)
{
	UNREFERENCED_PARAMETER(lpParam);
	while (true)
	{
		readFile();
		Sleep(3000);
	}
	return 0;
}
#endif