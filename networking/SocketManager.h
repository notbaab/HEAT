#pragma once
#include <functional>
#include <thread>
#include <vector>

#include "UDPSocket.h"

using RecieveCallback = std::function<void(std::shared_ptr<std::vector<uint8_t>>)>;

namespace networking
{
void printCallback(std::shared_ptr<std::vector<uint8_t>> data)
{
    for (auto i = (*data).begin(); i != (*data).end(); ++i)
    {
        std::cout << std::hex << unsigned(*i) << '|';
    }

    std::cout << std::endl;
}
} // namespace networking

class SocketManager
{
  public:
    SocketManager(uint16_t port, RecieveCallback receiveCallback);
    int SendTo(const void* data, int length, const SocketAddress& toAddress);
    void Stop();

  private:
    const int kMaxPacketSize = 1500;

    void receiveLoop();
    bool stopFlag;
    RecieveCallback receiveCallback;
    UDPSocketPtr mSocket;
    std::thread receiveThread;
};
