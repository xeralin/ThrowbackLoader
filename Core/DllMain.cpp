#include "Core.h"

BOOL APIENTRY DllMain(HMODULE, DWORD call_reason, LPVOID)
{
    if (call_reason == DLL_PROCESS_ATTACH)
    {
        reset_streaminginstall();
        remove_sen_client();
        start_close_guard();
        start_bridge();
    }
    return TRUE;
}
