#pragma once
// class that holds all relevant client data on for the server. Holds the client state,
// packet manager, salt and challenge data. Just container class so everything
// is public for easy access
#include "PacketManager.h"
#include "sockets/SocketAddress.h"

enum ClientConnectionState
{
    CONNECTING,    // Just created
    CHALLENGED,    // They are unauthenticated and we have sent the challenge request
    AUTHENTICATED, // They have gone through the whole handshake and are now authenticated
    LOGGED_IN,     // Assigned a game Id
    DISCONNECTED,  // They are dead to us
};

class ClientData
{
  public:
    ClientData(std::shared_ptr<PacketSerializer> packetSerializer, SocketAddress socketAddress)
        : packetManager(PacketManager(packetSerializer)), socketAddress(socketAddress),
          state(ClientConnectionState::CONNECTING)
    {
    }

    uint32_t serverSalt;
    uint32_t clientSalt;
    uint32_t xOrSalt;
    uint32_t gameId;

    // last time we heard from this client
    uint32_t lastHeardFrom;

    std::string userName;

    SocketAddress socketAddress;
    PacketManager packetManager;
    ClientConnectionState state;
};
