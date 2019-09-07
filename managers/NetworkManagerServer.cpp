#include "NetworkManagerServer.h"
#include "PacketManager.h"
#include "logger/Logger.h"
#include "messages/ClientWelcomeMessage.h"
#include "packets/Message.h"

void NetworkManagerServer::StaticInit(uint16_t port, std::shared_ptr<PacketManager> packetManager)
{
    sInstance.reset(new NetworkManagerServer(port, packetManager));
}

NetworkManagerServer::NetworkManagerServer(uint16_t port,
                                           std::shared_ptr<PacketManager> packetManager)
    : NetworkManager(port, packetManager)
{
}

void NetworkManagerServer::ProcessMessages()
{
    // clear our message buf so we can read the latest and greatest
    messageBuf.clear();

    packetManager->ReceiveMessages(messageBuf);

    for (auto message : messageBuf)
    {
        // Dispatch the message based on type to various components
        switch (message->GetIdentifier())
        {
        case ClientWelcomeMessage::ID:
            // HandleWelcomeMessage(message);
            break;
        default:
            break;
        }
        std::cout << "Got message with id" << message->GetId() << std::endl;
    }
}
