#pragma once
#include <functional>
#include <thread>
#include <vector>

#include "UDPSocket.h"

using RecieveCallback = std::function<void(std::shared_ptr<std::vector<uint8_t>>)>;

class SocketManager
{
  public:
    SocketManager(uint16_t port, RecieveCallback receiveCallback);
    void Stop();

  private:
    const int kMaxPacketSize = 1500;

    void receiveLoop();
    bool stopFlag;
    RecieveCallback receiveCallback;
    UDPSocketPtr mSocket;
    std::thread receiveThread;
};
