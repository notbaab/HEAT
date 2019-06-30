#include "managers/NetworkManagerServer.h"
#include "managers/PacketManager.h"
#include "messages/PlayerMessage.h"
#include "networking/SocketAddress.h"
#include "networking/SocketAddressFactory.h"
#include "networking/SocketManager.h"

// Something somewhere better have declared this.
extern std::shared_ptr<PacketManager> packetManager;

namespace holistic
{

// Technically setup server networking...
void SetupNetworking()
{
    std::string destination = "127.0.0.1:4500";
    SocketAddressPtr serverAddress = SocketAddressFactory::CreateIPv4FromString(destination);

    auto messageSerializer = std::make_shared<MessageSerializer>();
    AddMessageCtor(messageSerializer, PlayerMessage);
    auto packetSerializer = std::make_shared<PacketSerializer>(messageSerializer);
    AddPacketCtor(packetSerializer, ReliableOrderedPacket);
    packetManager = std::make_shared<PacketManager>(packetSerializer);

    // Init our singleton
    NetworkManagerServer::StaticInit(4500, packetManager);
}

} // namespace holistic
