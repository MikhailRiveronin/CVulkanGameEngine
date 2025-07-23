# file(GLOB_RECURSE sources *.c)

set(sources
    Core/Logger.c
    Core/Application.c
    Core/Memory.c
    Core/Events.c
    Core/Input.c
    Containers/DynamicArray.c)

if(WIN32)
    list(APPEND sources
        Platform/PlatformWin32.c)
endif()

add_library(Engine SHARED ${sources})

target_compile_definitions(Engine PRIVATE EXPORT)

# Remove /RTC1.
set(CMAKE_C_FLAGS_DEBUG "/Ob0 /Od")

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
