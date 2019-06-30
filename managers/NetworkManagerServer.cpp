#include "NetworkManagerServer.h"

void NetworkManagerServer::StaticInit(uint16_t port, std::shared_ptr<PacketManager> packetManager)
{
    sInstance.reset(new NetworkManagerServer(port, packetManager));
}

NetworkManagerServer::NetworkManagerServer(uint16_t port,
                                           std::shared_ptr<PacketManager> packetManager)
    : NetworkManager(port, packetManager)
{
}
