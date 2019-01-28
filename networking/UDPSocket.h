#pragma once

#include <memory>
#include <sys/socket.h>
#include <unistd.h>

#include "SocketAddress.h"
#include "SocketConsts.h"

class UDPSocket
{
  public:
    UDPSocket();
    ~UDPSocket();

    int Bind(const SocketAddress& inToAddress);
    int SendTo(const void* inToSend, int inLength, const SocketAddress& inToAddress);
    int ReceiveFrom(void* inToReceive, int inMaxLength, SocketAddress& outFromAddress);
    void Shutdown();

    /*
    int SendTo( const MemoryOutputStream& inMOS, const SocketAddress&
    inToAddress );
    int ReceiveFrom( MemoryInputStream& inMIS, SocketAddress& outFromAddress );
    */

    int SetNonBlockingMode(bool inShouldBeNonBlocking);

  private:
    friend class SocketUtil;
    UDPSocket(SOCKET inSocket) : mSocket(inSocket) {}
    SOCKET mSocket;
    fd_set read_fds;
    int close_pipe[2];
};

typedef std::shared_ptr<UDPSocket> UDPSocketPtr;
