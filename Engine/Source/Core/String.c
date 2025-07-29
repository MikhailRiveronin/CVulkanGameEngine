#include "String.h"
#include "Memory.h"
#include <string.h>

u64 stringLength(char const* str)
{
    return strlen(str);
}

char const* stringCopy(char const* str)
{
    u64 length = stringLength(str);
    char* copy = memoryAllocate(length + 1, MEMORY_TAG_STRING);
    memoryCopy(copy, str, length + 1);
    return copy;
}

b8 stringEqual(char const* str0, char const* str1)
{
    return strcmp(str0, str1) == 0;
}
