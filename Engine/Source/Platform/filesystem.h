#pragma once

#include "defines.h"

typedef struct File_Handle {
    void* stream;
    b8 is_valid;
} File_Handle;

typedef enum access_mode {
    ACCESS_MODE_READ = 0x1,
    ACCESS_MODE_WRITE = 0x2,
    ACCESS_MODE_BINARY = 0x4
} access_mode;

/**
 * Checks if a file with the given path exists
 * @param path The path of the file to be checked
 * @return TRUE if exists; otherwise FALSE
 */
LIB_API b8 filesystem_exists(char const* path);

/**
 * Attempt to open file located at path
 * @param path The path of the file to be opened
 * @param mode Mode flags for the file when opened (read/write). See file_modes enum in filesystem.h
 * @param file A pointer to a file_handle structure which holds the handle information
 * @return TRUE if opened successfully; otherwise FALSE
 */
LIB_API b8 filesystem_open(char const* path, access_mode mode, File_Handle* file);

/**
 * Closes the provided handle to a file
 * @param file A pointer to a file_handle structure which holds the handle to be closed
 */
LIB_API void filesystem_close(File_Handle* file);

/**
 * @brief Attempts to read the size of the file to which handle is attached
 * 
 * @param file A pointer to a file_handle structure
 * @param size_in_bytes A pointer to hold the file size.
 * @return TRUE if attempted successfully; otherwise FALSE
 */
LIB_API b8 filesystem_size(File_Handle* file, u64* size_in_bytes);

/**
 * Reads up to a newline or EOF
 * @param file A pointer to a file_handle structure
 * @param max_length The maximum length to be read from the line
 * @param buffer A pointer to a character array populated by this method. Must already be allocated
 * @param length A pointer to hold the line length read from the file
 * @return TRUE if successful; otherwise FALSE
 */
LIB_API b8 filesystem_read_line(File_Handle* file, u64 max_length, char** buffer, u64* length);

/**
 * Writes text to the provided file, appending a '\n' afterward
 * @param file A pointer to a file_handle structure
 * @param text The text to be written
 * @return TRUE if successful; otherwise FALSE
 */
LIB_API b8 filesystem_write_line(File_Handle* file, char const* line);

/**
 * Reads up to size_in_bytes bytes into buffer
 * @param file A pointer to a file_handle structure
 * @param size_in_bytes The number of bytes to read
 * @param buffer A pointer to a block of memory to be populated by this method
 * @param bytes_read A pointer to a number which will be populated with the number of bytes actually read from the file
 * @return TRUE if successful; otherwise FALSE
 */
LIB_API b8 filesystem_read(File_Handle* file, u64 size_in_bytes, void* buffer, u64* bytes_read);

/** 
 * Reads all bytes of data into buffer
 * @param file A pointer to a file_handle structure.
 * @param buffer A byte array which will be populated by this method
 * @param bytes_read A pointer to a number which will be populated with the number of bytes actually read from the file.
 * @return TRUE if successful; otherwise FALSE
 */
LIB_API b8 filesystem_read_all(File_Handle* file, void* buffer, u64* bytes_read);

/**
 * Writes provided data to the file
 * @param file A pointer to a file_handle structure
 * @param size_in_bytes The size of the data in bytes
 * @param data The data to be written.
 * @param bytes_written A pointer to a number which will be populated with the number of bytes actually written to the file.
 * @return TRUE if successful; otherwise FALSE.
 */
LIB_API b8 filesystem_write(File_Handle* file, u64 size_in_bytes, void const* data, u64* bytes_written);
