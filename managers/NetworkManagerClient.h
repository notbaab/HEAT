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
};

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
    bool HandleUnauthenticatedPacket(const std::shared_ptr<Packet> packet);
    bool HandleAuthenticatedPacket(const std::shared_ptr<Packet> packet);
    bool ReadChallengeMessage(const std::shared_ptr<Message> message);
    PacketManager packetManager;

  protected:
    uint32_t clientSalt;
    uint32_t serverSalt;
    uint32_t xOrSalt;
    CurrentConnectionState connectionState;
    // PacketManager packetManager;
    SocketAddressPtr serverAddress;
};
