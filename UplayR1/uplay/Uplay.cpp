#include "Uplay.h"

DLLEXPORT int UPLAY_ClearGameSession()
{
    LOGGER_INFO(__FUNCTION__);
    return 0;
}

DLLEXPORT int UPLAY_GetLastError()
{
    LOGGER_INFO(__FUNCTION__);
    return 0;
}

DLLEXPORT int UPLAY_GetNextEvent()
{
    return 0;
}

DLLEXPORT int UPLAY_GetOverlappedOperationResult(UplayOverlapped* overlapped, int* out_result)
{
    LOGGER_INFO(__FUNCTION__);
    if (overlapped && out_result && overlapped->IsOperationCompleted)
    {
        *out_result = overlapped->Reserved;
        return 1;
    }

    return 0;
}

DLLEXPORT int UPLAY_HasOverlappedOperationCompleted(UplayOverlapped* overlapped)
{
    return overlapped ? overlapped->IsOperationCompleted : 0;
}

DLLEXPORT int UPLAY_Init()
{
    LOGGER_INFO(__FUNCTION__);
    g_uplay_config.Initialize();
    return 1;
}

DLLEXPORT int UPLAY_PeekNextEvent()
{
    LOGGER_INFO(__FUNCTION__);
    return 1;
}

DLLEXPORT int UPLAY_Quit()
{
    LOGGER_INFO(__FUNCTION__);
    arm_quit_guard();
    return 1;
}

DLLEXPORT int UPLAY_Release()
{
    LOGGER_INFO(__FUNCTION__);
    return 1;
}

DLLEXPORT int UPLAY_SetGameSession()
{
    LOGGER_INFO(__FUNCTION__);
    return 1;
}

DLLEXPORT int UPLAY_Update()
{
    return 1;
}

DLLEXPORT int UPLAY_SetLanguage(const char* language)
{
    LOGGER_INFO(__FUNCTION__);
    return 1;
}

DLLEXPORT int UPLAY_Start()
{
    LOGGER_INFO(__FUNCTION__);
    UPLAY_Init();
    return 0;
}

DLLEXPORT int UPLAY_StartCN()
{
    LOGGER_INFO(__FUNCTION__);
    return UPLAY_Start();
}

DLLEXPORT int UPLAY_Startup()
{
    LOGGER_INFO(__FUNCTION__);
    return UPLAY_Start();
}

DLLEXPORT int UPLAY_METADATA_ClearContinuousTag()
{
    LOGGER_INFO(__FUNCTION__);
    return 1;
}

DLLEXPORT int UPLAY_METADATA_SetContinuousTag()
{
    LOGGER_INFO(__FUNCTION__);
    return 1;
}

DLLEXPORT int UPLAY_METADATA_SetSingleEventTag()
{
    LOGGER_INFO(__FUNCTION__);
    return 1;
}

DLLEXPORT int UPLAY_PRODUCT_GetProductList()
{
    LOGGER_INFO(__FUNCTION__);
    return 1;
}

DLLEXPORT int UPLAY_PRODUCT_ReleaseProductList()
{
    LOGGER_INFO(__FUNCTION__);
    return 1;
}
