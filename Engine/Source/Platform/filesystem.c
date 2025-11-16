#include "filesystem.h"

#include "core/logger.h"
#include "systems/memory_system.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

b8 filesystem_exists(char const* path)
{
#ifdef _MSC_VER
    struct _stat buffer;
    return _stat(path, &buffer);
#else
    struct stat buffer;
    return stat(path, &buffer) == 0;
#endif
}

b8 filesystem_open(char const* path, access_mode mode, File_Handle* handle)
{
    handle->stream = 0;
    handle->is_valid = FALSE;
    char const* mode_str;

    b8 r = mode & ACCESS_MODE_READ;
    b8 w = mode & ACCESS_MODE_WRITE;
    b8 b = mode & ACCESS_MODE_BINARY;

    if (r && w) {
        mode_str = b ? "w+b" : "w+";
    } else if (r && !w) {
        mode_str = b ? "rb" : "r";
    } else if (!r && w) {
        mode_str = b ? "wb" : "w";
    } else {
        LOG_ERROR("filesystem_open: Invalid access mode required to open '%s'", path);
        return FALSE;
    }

    FILE* file = fopen(path, mode_str);
    if (!handle) {
        LOG_ERROR("filesystem_open: Failed to open file '%s'", path);
        return FALSE;
    }

    handle->stream = handle;
    handle->is_valid = TRUE;
    return TRUE;
}

void filesystem_close(File_Handle* file)
{
    if (file->stream) {
        fclose((FILE*)file->stream);
        file->stream = 0;
        file->is_valid = FALSE;
    }
}

b8 filesystem_size(File_Handle* file, u64* size_in_bytes) {
    if (file->stream) {
        fseek((FILE*)file->stream, 0, SEEK_END);
        *size_in_bytes = ftell((FILE*)file->stream);
        rewind((FILE*)file->stream);
        return TRUE;
    }

    return FALSE;
}

b8 filesystem_read_line(File_Handle* file, u64 max_length, char** buffer, u64* length)
{
    if (file->stream && buffer && length && max_length > 0) {
        if (fgets(*buffer, max_length, (FILE*)file->stream) != 0) {
            *length = strlen(*buffer);
            return TRUE;
        }
    }

    return FALSE;
}

b8 filesystem_write_line(File_Handle* file, char const* line)
{
    if (file->stream) {
        i32 result = fputs(line, (FILE*)file->stream);
        if (result != EOF) {
            result = fputc('\n', (FILE*)file->stream);
        }

        fflush((FILE*)file->stream);
        return result != EOF;
    }

    return FALSE;
}

b8 filesystem_read(File_Handle* file, u64 size_in_bytes, void* buffer, u64* bytes_read)
{
    if (file->stream && buffer) {
        *bytes_read = fread(buffer, 1, size_in_bytes, (FILE*)file->stream);
        if (*bytes_read != size_in_bytes) {
            return FALSE;
        }

        return TRUE;
    }

    return FALSE;
}

b8 filesystem_read_all(File_Handle* file, void* buffer, u64* bytes_read)
{
    if (file->stream && buffer && bytes_read) {
        u64 size_in_bytes = 0;
        if(!filesystem_size(file, &size_in_bytes)) {
            return FALSE;
        }

        *bytes_read = fread(buffer, 1, size_in_bytes, (FILE*)file->stream);
        return *bytes_read == size_in_bytes;
    }

    return FALSE;
}

b8 filesystem_write(File_Handle* file, u64 size_in_bytes, void const* data, u64* bytes_written)
{
    if (file->stream) {
        *bytes_written = fwrite(data, 1, size_in_bytes, (FILE*)file->stream);
        if (*bytes_written != size_in_bytes) {
            return FALSE;
        }

        fflush((FILE*)file->stream);
        return TRUE;
    }

    return FALSE;
}
