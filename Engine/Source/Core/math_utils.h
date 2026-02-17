#pragma once

#include "defines.h"

inline u32 round_up_to_next_pow2(u32 value)
{
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value++;
    return value;
}

inline u32 DJB2_hash(unsigned char* str)
{
    u32 hash = 5381;
    int c;
    while (c = *str++)
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}
