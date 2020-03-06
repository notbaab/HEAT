list(APPEND debugging_tools_source
    debug_socket.cpp
    debug_commands.cpp
)

add_library(debugging_tools SHARED ${debugging_tools_source})
target_link_libraries(debugging_tools logger)
target_include_directories(debugging_tools
        PUBLIC
        ${CMAKE_SOURCE_DIR}
)