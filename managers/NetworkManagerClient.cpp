#include "NetworkManagerClient.h"
#include "PacketManager.h"
#include "logger/Logger.h"
#include "messages/ClientConnectionChallengeMessage.h"
#include "messages/ClientConnectionRequestMessage.h"
#include "messages/ClientWelcomeMessage.h"
#include "networking/SocketAddressFactory.h"
#include "packets/AuthenticatedPacket.h"
#include "packets/Message.h"
#include "packets/UnauthenticatedPacket.h"

void NetworkManagerClient::StaticInit(std::string serverAddress,
                                      std::shared_ptr<PacketSerializer> packetSerializer)
{
    sInstance.reset(new NetworkManagerClient(serverAddress, packetSerializer));
}

NetworkManagerClient::NetworkManagerClient(std::string serverAddressString,
                                           std::shared_ptr<PacketSerializer> packetSerializer)
    : packetManager(PacketManager(packetSerializer)), NetworkManager(packetSerializer)
{
    DEBUG("Made NetworkManagerClient");
    this->serverAddress = SocketAddressFactory::CreateIPv4FromString(serverAddressString);
}

void NetworkManagerClient::StartServerHandshake()
{
    // create a hello message and queue it to send in the next tick
    auto msg = std::make_unique<ClientConnectionRequestMessage>();
    this->packetManager.SendMessage(std::move(msg));
}

void NetworkManagerClient::dataRecievedCallback(SocketAddress fromAddress,
                                                std::unique_ptr<std::vector<uint8_t>> data)
{
    // De serialize raw byte data
    auto packets = packetSerializer->ReadPackets(std::move(data));
}

void NetworkManagerClient::ProcessMessages() {}

// Create an output stream and write out our outgoing packet
void NetworkManagerClient::SendOutgoingPackets()
{
    std::shared_ptr<ReliableOrderedPacket> packet;
    if (connectionState == CurrentConnectionState::AUTHENTICATED)
    {
        // TODO: We should just have the packet manager know if it's writing
        // an unreliable or reliable packet
        packet = packetManager.WritePacket(AuthenticatedPacket::CLASS_ID);
        auto cast = std::static_pointer_cast<AuthenticatedPacket>(packet);
        cast->expectedSalt = this->xOrSalt;
    }
    else
    {
        packet = packetManager.WritePacket(UnauthenticatedPacket::CLASS_ID);
    }
    auto stream = OutputMemoryBitStream();
    auto good = packetSerializer->WritePacket(packet, stream);
    if (!good)
    {
        ERROR("AHAHAHAH");
    }

    // ship it into the either
    socketManager.SendTo(stream.GetBufferPtr()->data(), stream.GetByteLength(), *serverAddress);
}
