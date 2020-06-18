#pragma once

#include "networking/SocketManager.h"
#include "packets/Message.h"
#include <cstdint>
#include <memory>
#include <vector>

class PacketSerializer;

uint64_t GenerateSalt();

class NetworkManager
{
  public:
    NetworkManager(std::shared_ptr<PacketSerializer> packetSerializer);
    NetworkManager(uint16_t port, std::shared_ptr<PacketSerializer> packetSerializer);
    // Read any messages that have come through
    virtual void ProcessMessages() = 0;

    // Send any new packets it needs to send.
    virtual void SendOutgoingPackets() = 0;

  protected:
    virtual void dataRecievedCallback(SocketAddress fromAddress, std::unique_ptr<std::vector<uint8_t>> data);
    // where the client and server start processing messages

    SocketManager socketManager;
    std::shared_ptr<PacketSerializer> packetSerializer;
    std::vector<std::shared_ptr<Message>> messageBuf;
};
