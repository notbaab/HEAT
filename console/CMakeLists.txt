list(APPEND console_src
    console.cpp
    linenoise/linenoise.c
)

add_executable(console ${console_src})
target_include_directories(console PUBLIC
    ${CMAKE_SOURCE_DIR}
)
target_link_libraries(console datastructures)
