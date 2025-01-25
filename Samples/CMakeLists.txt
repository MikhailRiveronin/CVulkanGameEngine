file(GLOB_RECURSE sources *.c)
add_executable(Main ${sources})

target_link_libraries(Main PRIVATE Engine)
