#include "filesystem.h"

#include "core/logger.h"
#include "systems/memory_system.h"

bool filesystem_open(char const* path, File_Access_Mode mode, File_Handle* file)
{
    char const* access_mode;
    switch (mode)
    {
        case FILE_ACCESS_MODE_READ:
            access_mode = "r";
            break;

        case FILE_ACCESS_MODE_WRITE:
            access_mode = "w";
            break;

        case FILE_ACCESS_MODE_READ_WRITE:
            access_mode = "r+";
            break;

        case FILE_ACCESS_MODE_APPEND:
            access_mode = "a";
            break;

        case FILE_ACCESS_MODE_READ_BINARY:
            access_mode = "rb";
            break;

        case FILE_ACCESS_MODE_WRITE_BINARY:
            access_mode = "wb";
            break;

        case FILE_ACCESS_MODE_READ_WRITE_BINARY:
            access_mode = "rb+";
            break;

        case FILE_ACCESS_MODE_APPEND_BINARY:
            access_mode = "ab";
            break;
        
        default:
            break;
    }

    file->handle = fopen(path, access_mode);
    if (!file->handle)
    {
        LOG_FATAL("filesystem_open: Failed to open file %s", path);
        return false;
    }

    return true;
}

void filesystem_close(File_Handle* file)
{
    if (file->handle)
    {
        fclose(file->handle);
    }
}

u32 filesystem_size(File_Handle* file)
{
    fseek(file->handle, 0, SEEK_END);
    u32 size = ftell(file->handle);
    rewind(file->handle);
    return size;
}

bool filesystem_read(File_Handle* file, void* buffer, u32 size)
{
        size_t bytes_read = fread(buffer, 1, size, file->handle);
        return bytes_read == size;
}

bool filesystem_write(File_Handle* file, u32 size, void const* data)
{
        size_t bytes_written = fwrite(data, 1, size, file->handle);
        fflush(file->handle);
        return bytes_written == size;
}
