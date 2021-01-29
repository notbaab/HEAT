#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "PacketManager.h"
#include "datastructures/ThreadSafeQueue.h"
#include "holistic/HNetworkManagerClient.h"
#include "networking/ReceivedPacket.h"

// This is all very similar to client data. How should it be consolidated?
enum CurrentConnectionState
{
    CREATED,
    CONNECTING,    // Just created
    CHALLENGED,    // They are unauthenticated and we have sent the challenge request
    AUTHENTICATED, // They have gone through the whole handshake and are now authenticated
    LOGGED_IN,     // Assigned a game Id
};

class NetworkManagerClient : public holistic::HNetworkManagerClient
{
  public:
    static inline std::unique_ptr<NetworkManagerClient> sInstance;
    // TODO: This presents a weird problem for us now that we are holistic
    uint32_t GetClientId() override { return clientId; }

    NetworkManagerClient(std::string serverAddress, std::shared_ptr<PacketSerializer> packetSerializer,
                         std::string userName);

    virtual void ProcessMessages() override;
    virtual void SendOutgoingPackets() override;
    static void StaticInit(std::string serverAddress, std::shared_ptr<PacketSerializer> packetSerializer,
                           std::string userName);

    virtual void DataReceivedCallback(SocketAddress fromAddressKey,
                                      std::unique_ptr<std::vector<uint8_t>> data) override;

    void AddPacketToQueue(ReceivedPacket& packet);
    void HandleReceivedPacket(ReceivedPacket& recievedPacket);
    void HandleMessage(std::shared_ptr<Message> message);

    // If we are playing back packets, some things need to be taken into account,
    // like don't check the salt on the authed packets
    bool playingBack;

    virtual void StartServerHandshake() override;
    void Tick(uint32_t timeStep) override;
    bool HandleUnauthenticatedPacket(const std::shared_ptr<Packet> packet);
    bool HandleAuthenticatedPacket(const std::shared_ptr<Packet> packet);
    // message reading functions
    bool ReadChallengeMessage(const std::shared_ptr<Message> message);
    bool ReadLoginMessage(const std::shared_ptr<Message> message);

    void QueueMessage(std::shared_ptr<Message> messageToQueue) override;
    PacketManager packetManager;

  protected:
    uint32_t clientSalt;
    uint32_t serverSalt;
    uint32_t xOrSalt;
    uint32_t clientId;
    uint32_t currentTime;
    std::string userName;

    CurrentConnectionState connectionState;
    SocketAddressPtr serverAddress;

    ThreadSafeQueue<ReceivedPacket> packetQueue;
};
