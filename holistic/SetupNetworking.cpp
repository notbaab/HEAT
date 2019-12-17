#include "managers/NetworkManagerServer.h"
#include "managers/PacketManager.h"
#include "messages/ClientConnectionChallengeMessage.h"
#include "messages/ClientConnectionChallengeResponseMessage.h"
#include "messages/ClientConnectionRequestMessage.h"
#include "messages/ClientLoginMessage.h"
#include "messages/ClientLoginResponse.h"
#include "messages/ClientWelcomeMessage.h"
#include "messages/PlayerMessage.h"
#include "networking/SocketAddress.h"
#include "networking/SocketAddressFactory.h"
#include "networking/SocketManager.h"
#include "packets/AuthenticatedPacket.h"
#include "packets/Message.h"
#include "packets/MessageSerializer.h"
#include "packets/Packet.h"
#include "packets/PacketSerializer.h"
#include "packets/ReliableOrderedPacket.h"
#include "packets/UnauthenticatedPacket.h"

// Something somewhere better have declared this.
// extern std::shared_ptr<PacketManager> packetManager;

namespace holistic
{

// Technically setup server networking...
void SetupNetworking()
{
    std::string destination = "127.0.0.1:4500";

    auto messageSerializer = std::make_shared<MessageSerializer>();

    // Message constructors
    AddMessageCtor(messageSerializer, PlayerMessage);
    AddMessageCtor(messageSerializer, ClientWelcomeMessage);
    AddMessageCtor(messageSerializer, ClientConnectionChallengeResponseMessage);
    AddMessageCtor(messageSerializer, ClientConnectionRequestMessage);
    AddMessageCtor(messageSerializer, ClientLoginMessage);
    AddMessageCtor(messageSerializer, ClientLoginResponse);
    // AddMessageCtor(messageSerializer, ClientWelcomeMessage);

    auto packetSerializer = std::make_shared<PacketSerializer>(messageSerializer);

    // TODO: Do we ever want a raw ROP?
    AddPacketCtor(packetSerializer, ReliableOrderedPacket);
    AddPacketCtor(packetSerializer, UnauthenticatedPacket);
    AddPacketCtor(packetSerializer, AuthenticatedPacket);
    // TODO: So this is really what we want. I vote to rename packetManager to
    // something that's more clear like, reliablePacketManager or something. Basically we will
    // need a packet manager for every client/server pair. So the network manager should take
    // a already setup serializer and spawn new managers for each connection that comes in.
    // packetManager = std::make_shared<PacketManager>(packetSerializer);

    // Init our singleton
    NetworkManagerServer::StaticInit(4500, packetSerializer);
}

} // namespace holistic
