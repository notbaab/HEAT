#pragma once

#include <functional>
#include <thread>
#include <vector>

#include "SocketAddress.h"
#include "UDPSocket.h"

using ReceiveCallback = std::function<void(SocketAddress fromAddress, std::unique_ptr<std::vector<uint8_t>>)>;

namespace networking
{
void printData(SocketAddress fromAddress, std::shared_ptr<std::vector<uint8_t>> data);
void printCallback(SocketAddress fromAddress, std::unique_ptr<std::vector<uint8_t>> data);

} // namespace networking

class SocketManager
{
  public:
    SocketManager(ReceiveCallback receiveCallback);
    SocketManager(uint16_t port, ReceiveCallback receiveCallback);
    ~SocketManager();
    int SendTo(const void* data, uint32_t length, const SocketAddress& toAddress);
    void Stop();

  private:
    static inline const int kMaxPacketSize = 1500;

    void receiveLoop();
    void bindSocket(uint16_t port);
    void startReceiveThread();
    bool stopFlag;
    ReceiveCallback receiveCallback;
    // Receiving socket
    UDPSocketPtr mSocket;
    std::thread receiveThread;
};
