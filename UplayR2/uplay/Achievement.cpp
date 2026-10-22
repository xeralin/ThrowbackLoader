#include "Uplay.h"

DLLEXPORT int UPC_AchievementImageFree()
{
    return 0;
}

DLLEXPORT int UPC_AchievementImageGet()
{
    return 0;
}

DLLEXPORT int UPC_AchievementListFree()
{
    return 0;
}

DLLEXPORT int UPC_AchievementListGet(ContextData* context, void* in_opt_user_id_utf8, unsigned in_filter,
                                     void* out_achievement_list, void* in_callback, void* in_callback_data)
{
    if (!context)
    {
        return -2;
    }

    context->push_callback(in_callback, in_callback_data, -4);
    return 0x2000;
}

DLLEXPORT int UPC_AchievementUnlock()
{
    return 0;
}
