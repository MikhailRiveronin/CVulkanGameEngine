#include "string_utils.h"

#include "memory_utils.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

u64 string_length(char const* str)
{
    return strlen(str);
}

char const* string_duplicate(char const* str)
{
    u64 length = string_length(str);
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

char* string_copy(char* dest, char const* source) {
    return strcpy(dest, source);
}

char* string_ncopy(char* dest, char const* source, i64 length) {
    return strncpy(dest, source, length);
}

char* string_trim(char* str) {
    while (isspace((unsigned char)*str)) {
        str++;
    }
    if (*str) {
        char* p = str;
        while (*p) {
            p++;
        }
        while (isspace((unsigned char)*(--p)))
            ;

        p[1] = '\0';
    }

    return str;
}

void string_mid(char* dest, char const* source, i32 start, i32 length) {
    if (length == 0) {
        return;
    }
    u64 src_length = string_length(source);
    if (start >= src_length) {
        dest[0] = 0;
        return;
    }
    if (length > 0) {
        for (u64 i = start, j = 0; j < length && source[i]; ++i, ++j) {
            dest[j] = source[i];
        }
        dest[start + length] = 0;
    } else {
        // If a negative value is passed, proceed to the end of the string.
        u64 j = 0;
        for (u64 i = start; source[i]; ++i, ++j) {
            dest[j] = source[i];
        }
        dest[start + j] = 0;
    }
}

i32 string_index_of(char* str, char c) {
    if (!str) {
        return -1;
    }
    u32 length = string_length(str);
    if (length > 0) {
        for (u32 i = 0; i < length; ++i) {
            if (str[i] == c) {
                return i;
            }
        }
    }

    return -1;
}

b8 string_to_vec4(char* str, vec4* out_vector) {
    if (!str) {
        return FALSE;
    }

    memory_zero(out_vector, sizeof(vec4));
    i32 result = sscanf(str, "%f %f %f %f", &out_vector->x, &out_vector->y, &out_vector->z, &out_vector->w);
    return result != -1;
}

b8 string_to_vec3(char* str, vec3* out_vector) {
    if (!str) {
        return FALSE;
    }

    memory_zero(out_vector, sizeof(vec3));
    i32 result = sscanf(str, "%f %f %f", &out_vector->x, &out_vector->y, &out_vector->z);
    return result != -1;
}

b8 string_to_vec2(char* str, vec2* out_vector) {
    if (!str) {
        return FALSE;
    }

    memory_zero(out_vector, sizeof(vec2));
    i32 result = sscanf(str, "%f %f", &out_vector->x, &out_vector->y);
    return result != -1;
}

b8 string_to_f32(char* str, f32* f) {
    if (!str) {
        return FALSE;
    }

    *f = 0;
    i32 result = sscanf(str, "%f", f);
    return result != -1;
}

b8 string_to_f64(char* str, f64* f) {
    if (!str) {
        return FALSE;
    }

    *f = 0;
    i32 result = sscanf(str, "%lf", f);
    return result != -1;
}

b8 string_to_i8(char* str, i8* i) {
    if (!str) {
        return FALSE;
    }

    *i = 0;
    i32 result = sscanf(str, "%hhi", i);
    return result != -1;
}

b8 string_to_i16(char* str, i16* i) {
    if (!str) {
        return FALSE;
    }

    *i = 0;
    i32 result = sscanf(str, "%hi", i);
    return result != -1;
}

b8 string_to_i32(char* str, i32* i) {
    if (!str) {
        return FALSE;
    }

    *i = 0;
    i32 result = sscanf(str, "%i", i);
    return result != -1;
}

b8 string_to_i64(char* str, i64* i) {
    if (!str) {
        return FALSE;
    }

    *i = 0;
    i32 result = sscanf(str, "%lli", i);
    return result != -1;
}

b8 string_to_u8(char* str, u8* u) {
    if (!str) {
        return FALSE;
    }

    *u = 0;
    i32 result = sscanf(str, "%hhu", u);
    return result != -1;
}

b8 string_to_u16(char* str, u16* u) {
    if (!str) {
        return FALSE;
    }

    *u = 0;
    i32 result = sscanf(str, "%hu", u);
    return result != -1;
}

b8 string_to_u32(char* str, u32* u) {
    if (!str) {
        return FALSE;
    }

    *u = 0;
    i32 result = sscanf(str, "%u", u);
    return result != -1;
}

b8 string_to_u64(char* str, u64* u) {
    if (!str) {
        return FALSE;
    }

    *u = 0;
    i32 result = sscanf(str, "%llu", u);
    return result != -1;
}

b8 string_to_bool(char* str, b8* b) {
    if (!str) {
        return FALSE;
    }

    if (string_equal(str, "1") || string_equali(str, "true")) {
        *b = TRUE;
        return TRUE;
    }

    if (string_equal(str, "0") || string_equali(str, "false")) {
        *b = FALSE;
        return TRUE;
    }

    return FALSE;
}
