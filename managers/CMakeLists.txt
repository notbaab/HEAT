list(APPEND managers_src
    PacketManager.cpp
)

add_library(managers SHARED ${managers_src})
target_include_directories(packets PUBLIC ${CMAKE_SOURCE_DIR}/IO ${CMAKE_SOURCE_DIR}/packets)
target_link_libraries(managers io packets)
