list(APPEND debugging_tools_source
    debug_socket.cpp
    debug_commands.cpp
)

add_library(debugging_tools SHARED ${debugging_tools_source})
