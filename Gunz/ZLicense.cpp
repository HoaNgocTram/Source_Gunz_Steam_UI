#include "stdafx.h"
#include "ZLicense.h"
#include "HMAC_SHA1.h"
#ifdef _LICENSE

bool Checks::CheckByteSha1(char* fileName, BYTE* SHA1_Value){
	BYTE digest[20];
	BYTE Key[GUNZ_HMAC_KEY_LENGTH];

	memset(Key, 0, 20);
	memcpy(Key, GUNZ_HMAC_KEY, strlen(GUNZ_HMAC_KEY));

	CHMAC_SHA1 HMAC_SHA1;
	HMAC_SHA1.HMAC_SHA1_file(fileName, Key, GUNZ_HMAC_KEY_LENGTH, digest);

	if (memcmp(digest, SHA1_Value, 20) == 0) 
	{
		return true;
	}

	return false;
}

void Checks::LicenseSystem()
{
	BYTE SHA_License[20] = { BYTE_LICENSE };
	if (!Checks::CheckByteSha1("license", SHA_License))
	{
		char Text[64];
		sprintf(Text, "The license key is wrong.");
		MessageBox(g_hWnd, Text, "Gunz License", MB_OK);
		exit(0);
	}
	else 
	{
		mlog("Gunz: the license key is correct and it worked: " __DATE__ " " __TIME__ "\n");
	}
}
#endif