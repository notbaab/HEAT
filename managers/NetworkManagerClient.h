#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "NetworkManager.h"
#include "PacketManager.h"

class NetworkManagerClient : public NetworkManager
{
  public:
    static inline std::unique_ptr<NetworkManagerClient> sInstance;
    NetworkManagerClient(std::string serverAddress,
                         std::shared_ptr<PacketSerializer> packetSerializer);

    virtual void ProcessMessages() override;
    virtual void SendOutgoingPackets() override;
    static void StaticInit(std::string serverAddress,
                           std::shared_ptr<PacketSerializer> packetSerializer);

    virtual void dataRecievedCallback(SocketAddress fromAddressKey,
                                      std::unique_ptr<std::vector<uint8_t>> data) override;

    void StartServerHandshake();

  protected:
    uint64_t clientSalt;
    uint64_t serverSalt;
    PacketManager packetManager;
    SocketAddressPtr serverAddress;
};
