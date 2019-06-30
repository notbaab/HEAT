#pragma once

#include "NetworkManager.h"
#include <cstdint>
#include <memory>
#include <vector>

class NetworkManagerServer : public NetworkManager
{
  public:
    static inline std::unique_ptr<NetworkManagerServer> sInstance;
    NetworkManagerServer(uint16_t port, std::shared_ptr<PacketManager> packetManager);

    static void StaticInit(uint16_t port, std::shared_ptr<PacketManager> packetManager);

  protected:
};
