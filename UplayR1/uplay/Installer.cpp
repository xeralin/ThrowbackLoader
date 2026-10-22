#include "Uplay.h"

DLLEXPORT int UPLAY_INSTALLER_AreChunksInstalled()
{
    return 1;
}

DLLEXPORT int UPLAY_INSTALLER_GetChunkIdsFromTag()
{
    return 0;
}

DLLEXPORT int UPLAY_INSTALLER_GetChunks()
{
    return 1;
}

DLLEXPORT const char* UPLAY_INSTALLER_GetLanguageUtf8()
{
    return CONSTS::UPLAY::DEFAULT_GAME_LANGUAGE;
}

DLLEXPORT int UPLAY_INSTALLER_Init()
{
    return 1;
}

DLLEXPORT int UPLAY_INSTALLER_ReleaseChunkIdList()
{
    return 1;
}

DLLEXPORT int UPLAY_INSTALLER_UpdateInstallOrder()
{
    return 1;
}
