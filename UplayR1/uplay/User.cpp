#include "Uplay.h"

DLLEXPORT int UPLAY_USER_ClearGameSession()
{
    return 1;
}

DLLEXPORT int UPLAY_USER_ConsumeItem()
{
    return 1;
}

DLLEXPORT int UPLAY_USER_GetAccountId()
{
    return 0;
}

DLLEXPORT const char* UPLAY_USER_GetAccountIdUtf8()
{
    return g_uplay_config.GetUserId().c_str();
}

DLLEXPORT int UPLAY_USER_GetCPUScore()
{
    return 0;
}

DLLEXPORT const char* UPLAY_USER_GetCdKeyUtf8()
{
    return CONSTS::UPLAY::DEFAULT_CD_KEY;
}

DLLEXPORT int UPLAY_USER_GetCdKeys()
{
    return 0;
}

DLLEXPORT int UPLAY_USER_GetConsumableItems()
{
    return 0;
}

DLLEXPORT int UPLAY_USER_GetCredentials()
{
    return 0;
}

DLLEXPORT int UPLAY_USER_GetEmail()
{
    return 0;
}

DLLEXPORT const char* UPLAY_USER_GetEmailUtf8()
{
    return CONSTS::UPLAY::DEFAULT_EMAIL;
}

DLLEXPORT int UPLAY_USER_GetGPUScore()
{
    return 0;
}

DLLEXPORT int UPLAY_USER_GetGPUScoreConfidenceLevel()
{
    return 0;
}

DLLEXPORT const char* UPLAY_USER_GetNameUtf8()
{
    return g_uplay_config.GetUsername().c_str();
}

DLLEXPORT int UPLAY_USER_GetPassword()
{
    return 0;
}

DLLEXPORT const char* UPLAY_USER_GetPasswordUtf8()
{
    return CONSTS::UPLAY::DEFAULT_PASSWORD;
}

DLLEXPORT int UPLAY_USER_GetProfile()
{
    return 0;
}

DLLEXPORT const char* UPLAY_USER_GetTicketUtf8()
{
    return CONSTS::UPLAY::DEFAULT_TICKETID;
}

DLLEXPORT int UPLAY_USER_GetUsername()
{
    return 0;
}

DLLEXPORT const char* UPLAY_USER_GetUsernameUtf8()
{
    return g_uplay_config.GetUsername().c_str();
}

DLLEXPORT int UPLAY_USER_IsConnected()
{
    return 1;
}

DLLEXPORT int UPLAY_USER_IsInOfflineMode()
{
    return 1;
}

DLLEXPORT int UPLAY_USER_IsOwned()
{
    return 1;
}

DLLEXPORT int UPLAY_USER_ReleaseCdKeyList()
{
    return 1;
}

DLLEXPORT int UPLAY_USER_ReleaseConsumeItemResult()
{
    return 1;
}

DLLEXPORT int UPLAY_USER_ReleaseProfile()
{
    return 0;
}

DLLEXPORT int UPLAY_USER_SetGameSession()
{
    return 0;
}
