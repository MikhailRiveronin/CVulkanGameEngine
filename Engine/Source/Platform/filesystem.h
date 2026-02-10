#pragma once

#include "defines.h"

typedef struct File_Handle
{
    FILE* handle;
} File_Handle;

typedef enum File_Access_Mode
{
    FILE_ACCESS_MODE_READ,
    FILE_ACCESS_MODE_WRITE,
    FILE_ACCESS_MODE_READ_WRITE,
    FILE_ACCESS_MODE_APPEND,
    FILE_ACCESS_MODE_READ_BINARY,
    FILE_ACCESS_MODE_WRITE_BINARY,
    FILE_ACCESS_MODE_READ_WRITE_BINARY,
    FILE_ACCESS_MODE_APPEND_BINARY
} File_Access_Mode;

/**
 * Attempt to open file located at path
 * @param path The path of the file to be opened
 * @param mode Mode flags for the file when opened (read/write). See file_modes enum in filesystem.h
 * @param file A pointer to a file handle  which holds the handle information
 * @return TRUE if opened successfully; otherwise FALSE
 */
LIB_API bool filesystem_open(char const* path, File_Access_Mode mode, File_Handle* file);

/**
 * Closes the provided handle to a file
 * @param file A pointer to a file_handle structure which holds the handle to be closed
 */
LIB_API void filesystem_close(File_Handle* file);

/**
 * @brief Attempts to read the size of the file to which handle is attached
 * 
 * @param file A pointer to a file_handle structure
 * @return File size.
 */
LIB_API u32 filesystem_size(File_Handle* file);

/** 
 * Reads all bytes of data into buffer
 * @param file A pointer to a file_handle structure.
 * @param buffer A byte array which will be populated by this method
 * @param bytes_read A pointer to a number which will be populated with the number of bytes actually read from the file.
 * @return TRUE if successful; otherwise FALSE
 */
LIB_API bool filesystem_read(File_Handle* file, void* buffer, u32 size);

/**
 * Writes provided data to the file
 * @param file A pointer to a file_handle structure
 * @param size_in_bytes The size of the data in bytes
 * @param data The data to be written.
 * @param bytes_written A pointer to a number which will be populated with the number of bytes actually written to the file.
 * @return TRUE if successful; otherwise FALSE.
 */
LIB_API bool filesystem_write(File_Handle* file, u32 size, void const* data);
