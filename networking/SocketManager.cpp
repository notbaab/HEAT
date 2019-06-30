#include "SocketManager.h"
#include "SocketUtil.h"

namespace networking
{

void printData(std::shared_ptr<std::vector<uint8_t>> data)
{
    for (auto i = (*data).begin(); i != (*data).end(); ++i)
    {
        std::cout << std::hex << unsigned(*i) << '|';
    }

    std::cout << std::endl;
}

void printCallback(std::unique_ptr<std::vector<uint8_t>> data)
{
    for (auto i = (*data).begin(); i != (*data).end(); ++i)
    {
        std::cout << std::hex << unsigned(*i) << '|';
    }

    std::cout << std::endl;
}
} // namespace networking

SocketManager::SocketManager(uint16_t port, RecieveCallback receiveCallback)
    : receiveCallback(receiveCallback), stopFlag(false)
{
    // Create the underlaying socket for this port
    mSocket = SocketUtil::CreateUDPSocket(INET);

    // Create a wildcard input address to bind to
    SocketAddress ownAddress(INADDR_ANY, port);
    mSocket->Bind(ownAddress);
    // did we bind okay?
    if (mSocket == nullptr)
    {
        throw std::runtime_error("Failed binding socket");
    }

    receiveThread = std::thread(&SocketManager::receiveLoop, this);
}

SocketManager::~SocketManager()
{
    // stop the thread so we can die gracefully
    Stop();
}

void SocketManager::Stop()
{
    stopFlag = true;
    mSocket->Shutdown();
    receiveThread.join();
}

// TODO: Does this need to wrap the socket send to method?
int SocketManager::SendTo(const void* data, int length, const SocketAddress& toAddress)
{
    int sentByteCount = mSocket->SendTo(data, length, toAddress);
    return sentByteCount;
}

void SocketManager::receiveLoop()
{
    SocketAddress fromAddress;
    while (true)
    {
        // TODO: Pool memory?
        auto packetMem = std::make_unique<std::vector<uint8_t>>(SocketManager::kMaxPacketSize);
        int readByteCount =
            mSocket->ReceiveFrom(packetMem->data(), SocketManager::kMaxPacketSize, fromAddress);

        // if we were stopped, don't call the callback
        if (stopFlag)
        {
            return;
        }

        packetMem->resize(readByteCount);
        receiveCallback(std::move(packetMem));
    }
}
