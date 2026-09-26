#include "Core.h"

DLLEXPORT char const* GetDefaultArgsString()
{
    return "/uplayproductid:635 /belaunch /nologo";
}

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID)
{
    return TRUE;
}
