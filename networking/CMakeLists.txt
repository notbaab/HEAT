list(APPEND network_src
   SocketAddress.cpp
   SocketAddressFactory.cpp
   SocketUtil.cpp
   UDPSocket.cpp
   SocketManager.cpp
)

add_library(networking SHARED ${network_src})
target_link_libraries(networking logger)
target_include_directories(networking
        PUBLIC
        ${CMAKE_SOURCE_DIR}
)