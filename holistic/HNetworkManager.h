#pragma once

#include "networking/SocketManager.h"
#include <cstdint>
#include <memory>
#include <vector>

class PacketSerializer;
class Message;

namespace holistic
{

// TODO: No. Put somewhere more better
uint64_t GenerateSalt();

class HNetworkManager
{
  public:
    HNetworkManager(std::shared_ptr<PacketSerializer> packetSerializer);
    HNetworkManager(uint16_t port, std::shared_ptr<PacketSerializer> packetSerializer);
    // Read any messages that have come through
    virtual void ProcessMessages() = 0;
    // Send any new packets it needs to send.
    virtual void SendOutgoingPackets() = 0;
    virtual void Tick(uint32_t timeStep) = 0;
    virtual void DataReceivedCallback(SocketAddress fromAddress, std::unique_ptr<std::vector<uint8_t>> data) = 0;
    virtual void SetupConfigVars(){};

  protected:
    SocketManager socketManager;
    std::shared_ptr<PacketSerializer> packetSerializer;
    std::vector<std::shared_ptr<Message>> messageBuf;
};

} // namespace holistic
