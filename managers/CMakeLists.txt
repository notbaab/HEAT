list(APPEND managers_src
    PacketManager.cpp
    NetworkManager.cpp
    NetworkManagerServer.cpp
)

add_library(managers SHARED ${managers_src})
target_include_directories(packets PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(managers io packets networking)
