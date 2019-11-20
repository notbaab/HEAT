#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "ClientData.h"
#include "NetworkManager.h"
#include "PacketManager.h"

class NetworkManagerServer : public NetworkManager
{
  public:
    static inline std::unique_ptr<NetworkManagerServer> sInstance;
    NetworkManagerServer(uint16_t port, std::shared_ptr<PacketSerializer> packetSerializer);

    virtual void ProcessMessages() override;
    virtual void SendOutgoingPackets() override;
    static void StaticInit(uint16_t port, std::shared_ptr<PacketSerializer> packetSerializer);

    virtual void dataRecievedCallback(SocketAddress fromAddressKey,
                                      std::unique_ptr<std::vector<uint8_t>> data) override;

    bool HandleNewClient(ClientData& client, const std::shared_ptr<Packet> packet);
    bool HandleChallenedClient(ClientData& client, const std::shared_ptr<Packet> packet);
    bool ReadConnectionRequestMessage(ClientData& client, const std::shared_ptr<Message> message);
    bool ReadChallengeResponseMessage(ClientData& client, const std::shared_ptr<Message> message);
    void Tick(double timeStep);

  protected:
    // Each client gets a packet manager as the manager is the thing that
    // manages the reliability of each message
    std::unordered_map<uint64_t, ClientData> cData;
};
