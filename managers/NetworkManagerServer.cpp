#include "NetworkManagerServer.h"
#include "PacketManager.h"
#include "logger/Logger.h"
#include "messages/ClientConnectionChallengeMessage.h"
#include "messages/ClientWelcomeMessage.h"
#include "packets/AuthenticatedPacket.h"
#include "packets/Message.h"
#include "packets/UnauthenticatedPacket.h"

void NetworkManagerServer::StaticInit(uint16_t port,
                                      std::shared_ptr<PacketSerializer> packetSerializer)
{
    sInstance.reset(new NetworkManagerServer(port, packetSerializer));
}

NetworkManagerServer::NetworkManagerServer(uint16_t port,
                                           std::shared_ptr<PacketSerializer> packetSerializer)
    : NetworkManager(port, packetSerializer)
{
    DEBUG("erer");
}

void NetworkManagerServer::dataRecievedCallback(SocketAddress fromAddress,
                                                std::unique_ptr<std::vector<uint8_t>> data)
{
    // Deserialize raw byte data
    auto packets = packetSerializer->ReadPackets(std::move(data));

    uint64_t key = fromAddress.GetIPPortKey();
    auto cDataPair = cData.find(key);

    if (cDataPair == cData.end())
    {
        // no packet manager found for this guy, create one and starting trying to
        // get this guy in the game
        DEBUG("Creating new manager for {} ", fromAddress.GetIPPortKey());
        // Forward everything because our packet manager disallows copying
        cData.emplace(std::piecewise_construct, std::forward_as_tuple(key),
                      std::forward_as_tuple(packetSerializer, fromAddress));
    }

    ClientData& client = cData.at(key);

    // Loop over packets and depending on the state of the client, determine
    // if they are allowed to be sending those types of packets
    for (auto const& packet : packets)
    {
        switch (client.state)
        {
        case ClientConnectionState::CONNECTING:
            // Client just connected, move to challenge and queue up sending a challenge
            HandleNewClient(client, packet);
            break;
        case ClientConnectionState::CHALLENGED:
            break;
        case ClientConnectionState::AUTHENTICATED:
            break;
        case ClientConnectionState::DISCONNECTED:
            break;
        }

        // Should I guard this with some sort of class check?
        // auto cast = std::static_pointer_cast<ReliableOrderedPacket>(packet);
        // auto cast = std::static_pointer_cast<AuthenticatedPacket>(packet);
        // bool weGood = clientManager.ReadPacket(cast);

        // if (!weGood)
        // {
        //     std::cout << "well fuck me" << std::endl;
        // }
        // else
        // {
        //     std::cout << "Damn son, we got that shit" << std::endl;
        // }
    }
}

// New client means we first need to to check if they sent us an unauthenticated
// packet with the connection request message. If it's all good, send
// a challenge with a random salt
bool NetworkManagerServer::HandleNewClient(ClientData& client, const std::shared_ptr<Packet> packet)
{
    auto cast = std::dynamic_pointer_cast<UnauthenticatedPacket>(packet);
    if (cast == nullptr)
    {
        return false;
    }

    // got a correct packet, read it and queue up a salt exchange
    client.packetManager.ReadPacket(cast);
    client.clientSalt = cast->clientSalt;
    client.serverSalt = GenerateSalt();
    client.state = ClientConnectionState::CHALLENGED;

    auto challenge =
        std::make_unique<ClientConnectionChallengeMessage>(client.clientSalt, client.serverSalt);
    client.packetManager.SendMessage(std::move(challenge));
    return true;
}

void NetworkManagerServer::ProcessMessages()
{
    // loop over all the packet managers, receive the messages then process them
    // It may be possible in the future we want some sort of ordering of
    // which messages to handle so we aren't just going in a set order each time
    // clear our message buf so we can read the latest and greatest
    // messageBuf.clear();

    // packetManager->ReceiveMessages(messageBuf);

    // for (auto message : messageBuf)
    // {
    //     // Dispatch the message based on type to various components
    //     switch (message->GetIdentifier())
    //     {
    //     case ClientWelcomeMessage::ID:
    //         // HandleWelcomeMessage(message);
    //         break;
    //     default:
    //         break;
    //     }
    //     std::cout << "Got message with id" << message->GetId() << std::endl;
    // }
}

void NetworkManagerServer::SendOutgoingPackets() {}
