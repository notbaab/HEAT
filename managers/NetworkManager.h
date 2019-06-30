#pragma once

#include "networking/SocketManager.h"
#include <cstdint>
#include <memory>
#include <vector>

class PacketManager;

class NetworkManager
{
  public:
    NetworkManager(uint16_t port, std::shared_ptr<PacketManager> packetManger);

  protected:
    virtual void dataRecievedCallback(std::unique_ptr<std::vector<uint8_t>> data);
    SocketManager socketManager;
    std::shared_ptr<PacketManager> packetManager;
};
