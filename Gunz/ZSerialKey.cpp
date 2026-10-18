#include "stdafx.h"
#include "ZSerialKey.h"
#ifdef _SERIALCUSTOM
bool check_patch(const char* filename)
{
    FILE* SerialKey = fopen(filename, "r");
    bool is_Key = false;
    if (SerialKey != NULL)
    {
        is_Key = true;
        fclose(SerialKey);
    }
    return is_Key;
}

int SerialPatch()
{
    char* Patch = "update";
    if (check_patch(Patch))
    {
        mlog("Gunz has been successfully updated!\n"); 
    }
    else
    {
        mlog("Serialkey error : Update not found (By Desperate)\n");
        ExitProcess(NULL);
    }
    return 0;
}

void Serial::SerialKey()
{
    SerialPatch();
}

#endif