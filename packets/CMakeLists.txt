list(APPEND packet_src
    PacketSerializer.cpp
    MessageSerializer.cpp
)

add_library(packets SHARED ${packet_src})
target_include_directories(packets PUBLIC ${CMAKE_SOURCE_DIR}/IO)
target_link_libraries(packets io)
