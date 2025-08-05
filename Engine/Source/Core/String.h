#pragma once

#include "defines.h"

u64 stringLength(char const* str);
char const* stringCopy(char const* str);
b8 stringEqual(char const* str0, char const* str1);

API void string_format(char* str, const char* format, ...);
