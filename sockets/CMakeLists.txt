list(APPEND network_src
   SocketAddress.cpp
   SocketAddressFactory.cpp
   SocketUtil.cpp
   UDPSocket.cpp
   SocketManager.cpp
)

add_library(sockets SHARED ${network_src})
target_link_libraries(sockets logger)
target_include_directories(sockets
        PUBLIC
        ${CMAKE_SOURCE_DIR}
)
