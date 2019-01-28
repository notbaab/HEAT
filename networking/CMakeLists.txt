list(APPEND network_src
   SocketAddress.cpp
   SocketAddressFactory.cpp
   SocketUtil.cpp
   UDPSocket.cpp
   SocketManager.cpp
)

add_library(networking SHARED ${network_src})
