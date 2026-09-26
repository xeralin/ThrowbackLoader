#include "Core.h"

DLLEXPORT const char* ThrowbackLoader_GetVersion()
{
    return get_version_string();
}

BOOL APIENTRY DllMain(HMODULE, DWORD call_reason, LPVOID)
{
    handle_windows_calls(call_reason);
    if (call_reason == DLL_PROCESS_ATTACH)
    {
        reset_streaminginstall();
        remove_sen_client();
        start_close_guard();
    }
    return TRUE;
}
