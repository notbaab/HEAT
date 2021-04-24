list(APPEND managers_src
    PacketManager.cpp
    NetworkManagerServer.cpp
    NetworkManagerClient.cpp
)

add_library(managers SHARED ${managers_src})
target_include_directories(packets PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/logger/lib/spdlog/include)
target_link_libraries(managers io packets sockets logger events gameobjects holistic)
