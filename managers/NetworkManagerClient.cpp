#include "NetworkManagerClient.h"
#include "PacketManager.h"
#include "logger/Logger.h"
#include "messages/ClientConnectionChallengeMessage.h"
#include "messages/ClientConnectionChallengeResponseMessage.h"
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
    logger::InitLog(logger::level::DEBUG, "Network Manager Client");
    DEBUG("Made NetworkManagerClient");
    this->serverAddress = SocketAddressFactory::CreateIPv4FromString(serverAddressString);
    this->connectionState = CurrentConnectionState::CREATED;
}

void NetworkManagerClient::StartServerHandshake()
{
    // create a hello message and queue it to send in the next tick
    this->clientSalt = GenerateSalt();
    auto msg = std::make_unique<ClientConnectionRequestMessage>(this->clientSalt);
    this->packetManager.SendMessage(std::move(msg));
    DEBUG("Sending handshake");
    this->connectionState = CurrentConnectionState::CONNECTING;
}

// read the packet according to the state the client is in into the packet
// manager
void NetworkManagerClient::dataRecievedCallback(SocketAddress fromAddress,
                                                std::unique_ptr<std::vector<uint8_t>> data)
{
    // De serialize raw byte data
    auto packets = packetSerializer->ReadPackets(std::move(data));
    for (auto const& packet : packets)
    {
        bool handled;
        switch (connectionState)
        {
        case CurrentConnectionState::CREATED:
        case CurrentConnectionState::CONNECTING:
        case CurrentConnectionState::CHALLENGED:
            handled = HandleUnauthenticatedPacket(packet);
            break;
        case CurrentConnectionState::AUTHENTICATED:
            handled = HandleAuthenticatedPacket(packet);
            break;
        }

        if (!handled)
        {
            ERROR("Couldn't read packet");
        }
    }
}

// Casts the packet to an UnauthenticatedPacket. If it works read the messages
// with the packet manager
bool NetworkManagerClient::HandleUnauthenticatedPacket(const std::shared_ptr<Packet> packet)
{
    auto cast = std::dynamic_pointer_cast<UnauthenticatedPacket>(packet);
    TRACE("Reading unauthed packet");
    if (cast == nullptr)
    {
        ERROR("Couldn't read unauthed packet")
        return false;
    }

    // correctly cast, read it
    return packetManager.ReadPacket(cast);
}

// Casts the packet to an AuthenticatedPacket. If it works read the messages
// with the packet manager
bool NetworkManagerClient::HandleAuthenticatedPacket(const std::shared_ptr<Packet> packet)
{
    auto cast = std::dynamic_pointer_cast<AuthenticatedPacket>(packet);
    TRACE("Reading authed packet");
    if (cast == nullptr)
    {
        ERROR("Couldn't read authed packet")
        return false;
    }

    // correctly cast, read it
    return packetManager.ReadPacket(cast);
}

void NetworkManagerClient::ProcessMessages()
{
    // DEBUG("Processing Messages");
    messageBuf.clear();
    packetManager.ReceiveMessages(messageBuf);
    TRACE("Processing {} messages", messageBuf.size());
    for (auto const& message : messageBuf)
    {
        switch (message->GetIdentifier())
        {
        case ClientConnectionChallengeMessage::CLASS_ID:
            ReadChallengeMessage(message);
            break;
        default:
            ERROR("Didn't handle message of type {}, Raw id {}", message->IdentifierToString(),
                  message->GetIdentifier());
        }
    }
}

bool NetworkManagerClient::ReadChallengeMessage(const std::shared_ptr<Message> message)
{
    // Unsafe cause we should have checked before we called this
    auto cast = std::static_pointer_cast<ClientConnectionChallengeMessage>(message);
    this->serverSalt = cast->serverSalt;
    this->xOrSalt = this->clientSalt ^ cast->serverSalt;

    // queue up a challenge response message and switch to authenticated
    connectionState = CurrentConnectionState::AUTHENTICATED;
    INFO("Authenticated, sending only authed packets now");

    auto challengeResponse =
        std::make_unique<ClientConnectionChallengeResponseMessage>(this->xOrSalt);

    TRACE("Queueing challenge response");
    this->packetManager.SendMessage(std::move(challengeResponse));
    return true;
}

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
        return;
    }
    TRACE("Packet with messages {} sent", packet->messages->size());

    // ship it into the ether
    socketManager.SendTo(stream.GetBufferPtr()->data(), stream.GetByteLength(), *serverAddress);
}
