#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "NetworkManager.h"
#include "PacketManager.h"

// This is all very similar to client data. How should it be consolidated?
enum CurrentConnectionState
{
    CREATED,
    CONNECTING,    // Just created
    CHALLENGED,    // They are unauthenticated and we have sent the challenge request
    AUTHENTICATED, // They have gone through the whole handshake and are now authenticated
    LOGGED_IN,     // Assigned a game Id
};

class NetworkManagerClient : public NetworkManager
{
  public:
    static inline std::unique_ptr<NetworkManagerClient> sInstance;
    NetworkManagerClient(std::string serverAddress,
                         std::shared_ptr<PacketSerializer> packetSerializer, std::string userName);

    virtual void ProcessMessages() override;
    virtual void SendOutgoingPackets() override;
    static void StaticInit(std::string serverAddress,
                           std::shared_ptr<PacketSerializer> packetSerializer,
                           std::string userName);

    virtual void dataRecievedCallback(SocketAddress fromAddressKey,
                                      std::unique_ptr<std::vector<uint8_t>> data) override;

    void StartServerHandshake();
    static uint32_t GetClientId() { return sInstance->clientId; }
    void Update();
    bool HandleUnauthenticatedPacket(const std::shared_ptr<Packet> packet);
    bool HandleAuthenticatedPacket(const std::shared_ptr<Packet> packet);
    // message reading functions
    bool ReadChallengeMessage(const std::shared_ptr<Message> message);
    bool ReadLoginMessage(const std::shared_ptr<Message> message);

    void QueueMessage(std::shared_ptr<Message> messageToQueue);
    PacketManager packetManager;

  protected:
    uint32_t clientSalt;
    uint32_t serverSalt;
    uint32_t xOrSalt;
    uint32_t clientId;
    std::string userName;

    CurrentConnectionState connectionState;
    SocketAddressPtr serverAddress;
};
