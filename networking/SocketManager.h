#pragma once

#include <functional>
#include <thread>
#include <vector>

#include "UDPSocket.h"

using RecieveCallback = std::function<void(std::unique_ptr<std::vector<uint8_t>>)>;

namespace networking
{
void printData(std::shared_ptr<std::vector<uint8_t>> data);
void printCallback(std::unique_ptr<std::vector<uint8_t>> data);

} // namespace networking

class SocketManager
{
  public:
    SocketManager(uint16_t port, RecieveCallback receiveCallback);
    ~SocketManager();
    int SendTo(const void* data, int length, const SocketAddress& toAddress);
    void Stop();

  private:
    static inline const int kMaxPacketSize = 1500;

    void receiveLoop();
    bool stopFlag;
    RecieveCallback receiveCallback;
    UDPSocketPtr mSocket;
    std::thread receiveThread;
};
