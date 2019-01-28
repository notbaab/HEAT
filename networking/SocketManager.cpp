#include "SocketManager.h"
#include "SocketUtil.h"

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

void SocketManager::Stop()
{
    stopFlag = true;
    mSocket->Shutdown();
    receiveThread.join();
}

void SocketManager::receiveLoop()
{
    SocketAddress fromAddress;
    while (true)
    {
        // TODO: Pool memory?
        auto packetMem = std::make_shared<std::vector<uint8_t>>(kMaxPacketSize);
        int readByteCount = mSocket->ReceiveFrom(packetMem->data(), kMaxPacketSize, fromAddress);

        // if we were stopped, don't call the callback
        if (stopFlag)
        {
            return;
        }

        packetMem->resize(readByteCount);
        receiveCallback(packetMem);
    }
}
