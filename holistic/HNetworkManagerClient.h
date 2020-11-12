#pragma once

#include "HNetworkManager.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace holistic
{

class HNetworkManagerClient : public HNetworkManager
{
  public:
    HNetworkManagerClient(std::shared_ptr<PacketSerializer> packetSerializer) : HNetworkManager(packetSerializer){};
    HNetworkManagerClient(uint16_t port, std::shared_ptr<PacketSerializer> packetSerializer)
        : HNetworkManager(port, packetSerializer){};
    // Queue message to be sent in send outgoing packets
    virtual void QueueMessage(std::shared_ptr<Message> message) = 0;
    virtual void StartServerHandshake() = 0;
    virtual uint32_t GetClientId() = 0;
};
} // namespace holistic
