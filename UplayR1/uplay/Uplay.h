#ifndef UPLAY_R1_H
#define UPLAY_R1_H

#include "Core.h"

inline UplayConfig g_uplay_config;

struct UplayOverlapped
{
    void* Result;
    int IsOperationCompleted;
    int Reserved;

    void set_result(int result = 0)
    {
        *this = {nullptr, 1, result};
    }
    void set_result(void* out_result)
    {
        *this = {out_result, 1, 0};
    }
};

#endif
