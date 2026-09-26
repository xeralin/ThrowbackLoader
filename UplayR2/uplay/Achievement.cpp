#include "Uplay.h"

DLLEXPORT int UPC_AchievementImageFree()
{
    LOGGER_INFO(__FUNCTION__);
    return 0;
}

DLLEXPORT int UPC_AchievementImageGet()
{
    LOGGER_INFO(__FUNCTION__);
    return 0;
}

DLLEXPORT int UPC_AchievementListFree()
{
    LOGGER_INFO(__FUNCTION__);
    return 0;
}

DLLEXPORT int UPC_AchievementListGet(ContextData* context, void* in_opt_user_id_utf8, unsigned in_filter,
                                     void* out_achievement_list, void* in_callback, void* in_callback_data)
{
    LOGGER_INFO(__FUNCTION__);

    if (!context)
    {
        return -2;
    }

    {
        std::lock_guard<std::mutex> lock(context->callbacks_mutex);
        context->callbacks.push(CallbackData(in_callback, in_callback_data, -4));
    }
    return 0x2000;
}

DLLEXPORT int UPC_AchievementUnlock()
{
    LOGGER_INFO(__FUNCTION__);
    return 0;
}
