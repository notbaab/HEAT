#pragma once

#include "networking/SocketManager.h"
#include "packets/Message.h"
#include <cstdint>
#include <memory>
#include <vector>

class PacketManager;

class NetworkManager
{
  public:
    NetworkManager(uint16_t port, std::shared_ptr<PacketManager> packetManger);
    virtual void ProcessMessages() = 0;

  protected:
    virtual void dataRecievedCallback(uint64_t fromAddressKey,
                                      std::unique_ptr<std::vector<uint8_t>> data);
    // where the client and server start processing messages

    SocketManager socketManager;
    std::shared_ptr<PacketManager> packetManager;
    std::vector<std::shared_ptr<Message>> messageBuf;
};
