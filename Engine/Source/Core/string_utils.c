#include "string_utils.h"

#include "memory.h"
#include <string.h>
#include <stdarg.h>

u64 stringLength(char const* str)
{
    return strlen(str);
}

char const* stringCopy(char const* str)
{
    u64 length = stringLength(str);
    char* copy = memory_allocate(length + 1, MEMORY_TAG_STRING);
    memory_copy(copy, str, length + 1);
    return copy;
}

b8 string_equal(char const* str0, char const* str1)
{
    return strcmp(str0, str1) == 0;
}

b8 string_equali(char const* str0, char const* str1)
{
#ifdef _MSC_VER
    return _strcmpi(str0, str1);
#elif
    return FALSE;
#endif
}

void string_format(char* str, char const* format, ...)
{
    va_list vaList;
    va_start(vaList, format);
    vsprintf(str, format, vaList);
    va_end(vaList);
}
