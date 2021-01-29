#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "ClientData.h"
#include "PacketManager.h"
#include "datastructures/ThreadSafeQueue.h"
#include "holistic/HNetworkManager.h"
#include "networking/ReceivedMessage.h"
#include "networking/ReceivedPacket.h"

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

    void AddPacketToQueue(ReceivedPacket& packet);
    void HandleMessage(uint64_t clientKey, std::shared_ptr<Message> message);
    virtual void SetupConfigVars() override;

    void EventForwarder(std::shared_ptr<Event> evt);
    void BroadcastMessage(std::shared_ptr<Message> msg);

    bool HandleNewClient(ClientData& client, const std::shared_ptr<Packet> packet);
    bool HandleChallengedClient(ClientData& client, const std::shared_ptr<Packet> packet);
    void HandleReceivedPacket(ReceivedPacket& recievedPacket);

    // Message handler functions
    bool ReadConnectionRequestMessage(ClientData& client, const std::shared_ptr<Message> message);
    bool ReadChallengeResponseMessage(ClientData& client, const std::shared_ptr<Message> message);
    bool ReadLoginMessage(ClientData& client, const std::shared_ptr<Message> message);

    bool LogoutClient(uint64_t clientKey);

    // If we are playing back packets, some things need to be taken into account,
    // like don't check the salt on the authed packets
    bool playingBack;

  protected:
    // Each client gets a packet manager as the manager is the thing that
    // manages the reliability of each message
    std::unordered_map<uint64_t, ClientData> cData;
    uint32_t currentTime;

    ThreadSafeQueue<ReceivedPacket> packetQueue;
    ThreadSafeQueue<ReceivedMessage> messageQueue;

    // time that we log out a client if they haven't been heard from in this time
    uint32_t logoutTimeMs = 1000;

    bool recording;
};
