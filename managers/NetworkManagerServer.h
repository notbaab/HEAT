#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "ClientData.h"
#include "PacketManager.h"
#include "holistic/HNetworkManager.h"

class Event;
class Message;

class NetworkManagerServer : public holistic::HNetworkManager
{
  public:
    static inline std::unique_ptr<NetworkManagerServer> sInstance;
    static void StaticInit(uint16_t port, std::shared_ptr<PacketSerializer> packetSerializer);

    NetworkManagerServer(uint16_t port, std::shared_ptr<PacketSerializer> packetSerializer);

    virtual void ProcessMessages() override;
    virtual void SendOutgoingPackets() override;
    virtual void Tick(uint32_t timeStep) override;
    virtual void DataReceivedCallback(SocketAddress fromAddressKey,
                                      std::unique_ptr<std::vector<uint8_t>> data) override;

    void EventForwarder(std::shared_ptr<Event> evt);
    void BroadcastMessage(std::shared_ptr<Message> msg);

    bool HandleNewClient(ClientData& client, const std::shared_ptr<Packet> packet);
    bool HandleChallenedClient(ClientData& client, const std::shared_ptr<Packet> packet);

    // Message handler functions
    bool ReadConnectionRequestMessage(ClientData& client, const std::shared_ptr<Message> message);
    bool ReadChallengeResponseMessage(ClientData& client, const std::shared_ptr<Message> message);
    bool ReadLoginMessage(ClientData& client, const std::shared_ptr<Message> message);

  protected:
    // Each client gets a packet manager as the manager is the thing that
    // manages the reliability of each message
    std::unordered_map<uint64_t, ClientData> cData;
    uint32_t currentTime;
};
