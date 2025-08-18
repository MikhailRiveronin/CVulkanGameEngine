#pragma once

#include "defines.h"

u64 stringLength(char const* str);
char const* stringCopy(char const* str);

// Case-sensitive
b8 string_equal(char const* str0, char const* str1);

// Case-insensitive
b8 string_equali(char const* str0, char const* str1);

API void string_format(char* str, char const* format, ...);
