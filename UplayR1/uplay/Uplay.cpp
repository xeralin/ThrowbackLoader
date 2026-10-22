#include "Uplay.h"

DLLEXPORT int UPLAY_ClearGameSession()
{
    return 0;
}

DLLEXPORT int UPLAY_GetLastError()
{
    return 0;
}

DLLEXPORT int UPLAY_GetNextEvent()
{
    return 0;
}

DLLEXPORT int UPLAY_GetOverlappedOperationResult(UplayOverlapped* overlapped, int* out_result)
{
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
    g_uplay_config.Initialize();
    return 1;
}

DLLEXPORT int UPLAY_PeekNextEvent()
{
    return 1;
}

DLLEXPORT int UPLAY_Quit()
{
    arm_quit_guard();
    return 1;
}

DLLEXPORT int UPLAY_Release()
{
    return 1;
}

DLLEXPORT int UPLAY_SetGameSession()
{
    return 1;
}

DLLEXPORT int UPLAY_Update()
{
    return 1;
}

DLLEXPORT int UPLAY_SetLanguage()
{
    return 1;
}

DLLEXPORT int UPLAY_Start()
{
    UPLAY_Init();
    return 0;
}

DLLEXPORT int UPLAY_StartCN()
{
    return UPLAY_Start();
}

DLLEXPORT int UPLAY_Startup()
{
    return UPLAY_Start();
}

DLLEXPORT int UPLAY_METADATA_ClearContinuousTag()
{
    return 1;
}

DLLEXPORT int UPLAY_METADATA_SetContinuousTag()
{
    return 1;
}

DLLEXPORT int UPLAY_METADATA_SetSingleEventTag()
{
    return 1;
}

DLLEXPORT int UPLAY_PRODUCT_GetProductList()
{
    return 1;
}

DLLEXPORT int UPLAY_PRODUCT_ReleaseProductList()
{
    return 1;
}
