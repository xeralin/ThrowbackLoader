#include "Uplay.h"

DLLEXPORT const char* UPC_ErrorToString(int error)
{
    switch (error)
    {
    case -1:
        return "Declined";
    case -2:
        return "Invalid arguments";
    case -3:
        return "Uninitialized subsystem";
    case -4:
        return "Communication error";
    case -5:
        return "Memory error";
    case -6:
        return "Not found";
    case -7:
        return "End of file";
    case -8:
        return "Limit reached";
    case -9:
        return "Unauthorized action";
    case -10:
        return "Internal error";
    case -11:
        return "Operation aborted";
    case -13:
        return "Failed precondition";
    case -14:
        return "Unavailable";
    default:
        return "Unknown error";
    }
}

DLLEXPORT int UPC_ApplicationIdGet(ContextData* context, const char** out_app_id)
{
    if (!context || !out_app_id)
    {
        return -2;
    }

    *out_app_id = CONSTS::UPLAY::DEFAULT_GAME_ID;
    return 0;
}

DLLEXPORT int UPC_AvatarFree()
{
    return 0;
}

DLLEXPORT int UPC_AvatarGet()
{
    return 0;
}

DLLEXPORT int UPC_BlacklistAdd()
{
    return 0;
}

DLLEXPORT int UPC_BlacklistHas()
{
    return 0;
}

DLLEXPORT int UPC_BlacklistHas_Extended()
{
    return 0;
}

DLLEXPORT void UPC_Cancel()
{
}

DLLEXPORT ContextData* UPC_ContextCreate()
{
    return new ContextData();
}

DLLEXPORT int UPC_ContextFree(ContextData* context)
{
    if (!context)
    {
        return -2;
    }

    delete context;
    return 0;
}

DLLEXPORT int UPC_CPUScoreGet(ContextData* context, unsigned int* out_score)
{
    if (!context || !out_score)
    {
        return -2;
    }

    *out_score = 0x1000;
    return 0;
}

DLLEXPORT const char* UPC_EmailGet()
{
    return CONSTS::UPLAY::DEFAULT_EMAIL;
}

DLLEXPORT int UPC_EmailGet_Extended(ContextData* context, const char** out_email)
{
    if (!context || !out_email)
    {
        return -2;
    }

    *out_email = CONSTS::UPLAY::DEFAULT_EMAIL;
    return 0;
}

DLLEXPORT int UPC_EventNextPeek()
{
    return 0;
}

DLLEXPORT int UPC_EventNextPoll()
{
    return -6;
}

DLLEXPORT int UPC_EventRegisterHandler()
{
    return 0;
}

DLLEXPORT int UPC_EventUnregisterHandler()
{
    return 0;
}

DLLEXPORT int UPC_GPUScoreGet(ContextData* context, unsigned* out_score, float* out_confidence_level)
{
    if (!context || !out_score || !out_confidence_level)
    {
        return -2;
    }

    *out_score = 0x1000;
    *out_confidence_level = 1.0f;
    return 0;
}

DLLEXPORT const char* UPC_IdGet()
{
    return g_uplay_config.GetUserId().c_str();
}

DLLEXPORT int UPC_IdGet_Extended(ContextData* context, const char** out_id)
{
    if (!context || !out_id)
    {
        return -2;
    }

    *out_id = g_uplay_config.GetUserId().c_str();
    return 0;
}

DLLEXPORT int UPC_Init(unsigned in_version, int appid)
{
    g_uplay_config.Initialize();
    g_uplay_config.SetAppId(appid);
    return 0;
}

DLLEXPORT const char* UPC_InstallLanguageGet()
{
    return CONSTS::UPLAY::DEFAULT_GAME_LANGUAGE;
}

DLLEXPORT int UPC_InstallLanguageGet_Extended(ContextData* context, const char** out_install_language)
{
    if (!context || !out_install_language)
    {
        return -2;
    }

    *out_install_language = CONSTS::UPLAY::DEFAULT_GAME_LANGUAGE;
    return 0;
}

DLLEXPORT int UPC_LaunchApp()
{
    return 0;
}

DLLEXPORT const char* UPC_NameGet()
{
    return g_uplay_config.GetUsername().c_str();
}

DLLEXPORT int UPC_NameGet_Extended(ContextData* context, const char** out_name)
{
    if (!context || !out_name)
    {
        return -2;
    }

    *out_name = g_uplay_config.GetUsername().c_str();
    return 0;
}

DLLEXPORT int UPC_RichPresenceSet()
{
    return 0;
}

DLLEXPORT int UPC_RichPresenceSet_Extended()
{
    return 0;
}

DLLEXPORT int UPC_ShowBrowserUrl()
{
    return 0;
}

DLLEXPORT const char* UPC_TicketGet()
{
    return CONSTS::UPLAY::DEFAULT_TICKETID;
}

DLLEXPORT int UPC_TicketGet_Extended(ContextData* context, const char** out_ticket)
{
    if (!context || !out_ticket)
    {
        return -2;
    }

    *out_ticket = CONSTS::UPLAY::DEFAULT_TICKETID;
    return 0;
}

DLLEXPORT void UPC_Uninit()
{
    arm_quit_guard();
}

DLLEXPORT int UPC_Update(ContextData* context)
{
    if (!context)
    {
        return -2;
    }

    while (true)
    {
        CallbackData cb(nullptr, nullptr, 0);
        {
            std::lock_guard<std::mutex> lock(context->callbacks_mutex);
            if (context->callbacks.empty())
            {
                break;
            }
            cb = context->callbacks.front();
            context->callbacks.pop();
        }
        cb.callback(cb.result, cb.context_data);
    }

    return 0;
}

DLLEXPORT int UPC_UserAccountCountryGet()
{
    return 0;
}

DLLEXPORT int UPC_UserFree()
{
    return 0;
}

DLLEXPORT int UPC_UserGet()
{
    return 0;
}

DLLEXPORT int UPC_UserPlayedWithAdd()
{
    return 0;
}

DLLEXPORT int UPC_UserPlayedWithAdd_Extended()
{
    return 0;
}
