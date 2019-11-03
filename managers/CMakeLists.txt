list(APPEND managers_src
    PacketManager.cpp
    NetworkManager.cpp
    NetworkManagerServer.cpp
    NetworkManagerClient.cpp
)

add_library(managers SHARED ${managers_src})
target_include_directories(packets PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/logger/lib/spdlog/include)
target_link_libraries(managers io packets networking)
target_link_libraries(managers io packets networking logger)
