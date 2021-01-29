#include "NetworkManagerClient.h"
#include "dvr/PacketReceivedEvent.h"
#include "events/Event.h"
#include "events/EventManager.h"
#include "events/LoggedIn.h"
#include "messages/ClientConnectionChallengeMessage.h"
#include "messages/ClientConnectionChallengeResponseMessage.h"
#include "messages/ClientConnectionRequestMessage.h"
#include "messages/ClientLoginMessage.h"
#include "messages/ClientLoginResponse.h"
#include "packets/AuthenticatedPacket.h"
#include "packets/UnauthenticatedPacket.h"
#include "sockets/SocketAddressFactory.h"

void NetworkManagerClient::StaticInit(std::string serverAddress, std::shared_ptr<PacketSerializer> packetSerializer,
                                      std::string userName)
{
    sInstance.reset(new NetworkManagerClient(serverAddress, packetSerializer, userName));
}

NetworkManagerClient::NetworkManagerClient(std::string serverAddressString,
                                           std::shared_ptr<PacketSerializer> packetSerializer, std::string userName)
    : packetManager(PacketManager(packetSerializer)), HNetworkManagerClient(packetSerializer), userName(userName),
      playingBack(false), currentTime(0)
{
    logger::InitLog(logger::level::TRACE, "Network Manager Client");
    DEBUG("Made NetworkManagerClient");
    this->serverAddress = SocketAddressFactory::CreateIPv4FromString(serverAddressString);
    this->connectionState = CurrentConnectionState::CREATED;
}

void NetworkManagerClient::StartServerHandshake()
{
    INFO("Sending handshake");
    // create a hello message and queue it to send in the next tick
    this->clientSalt = holistic::GenerateSalt();
    auto msg = std::make_unique<ClientConnectionRequestMessage>(this->clientSalt);
    this->packetManager.SendMessage(std::move(msg));
    this->connectionState = CurrentConnectionState::CONNECTING;
    INFO("Sent handshake");
}

void NetworkManagerClient::AddPacketToQueue(ReceivedPacket& packet)
{
    TRACE("Pushing packet to queue");
    packetQueue.push(packet);
}

void NetworkManagerClient::DataReceivedCallback(SocketAddress fromAddress, std::unique_ptr<std::vector<uint8_t>> data)
{
    // If playing back something right now, drop packet
    // TODO: Make a "head" to the server for playback maybe?
    if (playingBack)
    {
        TRACE("Playing back, don't listen to packet");
        return;
    }

    // Deserialize raw byte data
    auto packets = packetSerializer->ReadPackets(std::move(data));
    TRACE("Read {} packets", packets.size());

    // Loop over packets and depending on the state of the client, determine
    // if they are allowed to be sending those types of packets
    for (auto const& packet : packets)
    {
        auto receivedPacket = ReceivedPacket();
        receivedPacket.timeRecieved = currentTime;
        // TODO: Need a frame
        receivedPacket.frameRecieved = 0;
        receivedPacket.fromAddress = fromAddress;
        receivedPacket.packet = packet;
        AddPacketToQueue(receivedPacket);

        // Also queue up an event for anything that cares

        auto packetReceived = std::make_shared<PacketReceivedEvent>();
        packetReceived->packet = receivedPacket;
        EventManager::sInstance->QueueEvent(packetReceived);
    }
}

void NetworkManagerClient::HandleReceivedPacket(ReceivedPacket& packet)
{
    bool handled = false;
    TRACE("Handling packet")
    switch (connectionState)
    {
    case CurrentConnectionState::CREATED:
    case CurrentConnectionState::CONNECTING:
    case CurrentConnectionState::CHALLENGED:
        handled = HandleUnauthenticatedPacket(packet.packet);
        break;
    case CurrentConnectionState::AUTHENTICATED:
    case CurrentConnectionState::LOGGED_IN:
        handled = HandleAuthenticatedPacket(packet.packet);
        break;
    }

    if (!handled)
    {
        ERROR("Couldn't read packet");
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

void NetworkManagerClient::HandleMessage(std::shared_ptr<Message> message)
{
    // If a game event, feed into the event manager cause we don't
    // need to do anything
    if (message->GetTypeIdentifier() == Event::TYPE_ID)
    {
        auto evt = std::static_pointer_cast<Event>(message);
        TRACE("Queuing event {}", message->IdentifierToString());
        EventManager::sInstance->QueueEvent(evt);
        return;
    }

    // Not an event, maybe we handle it?
    switch (message->GetClassIdentifier())
    {
    case ClientConnectionChallengeMessage::CLASS_ID:
        ReadChallengeMessage(message);
        break;
    case ClientLoginResponse::CLASS_ID:
        ReadLoginMessage(message);
        break;
    default:
        ERROR("Didn't handle message of type {}, Raw id {}", message->IdentifierToString(),
              message->GetClassIdentifier());
    }
}

void NetworkManagerClient::ProcessMessages()
{
    messageBuf.clear();
    packetManager.ReceiveMessages(messageBuf);
    TRACE("Processing {} messages", messageBuf.size());

    for (auto const& message : messageBuf)
    {
        HandleMessage(message);
    }
}

bool NetworkManagerClient::ReadLoginMessage(const std::shared_ptr<Message> message)
{
    auto cast = std::static_pointer_cast<ClientLoginResponse>(message);
    this->clientId = cast->clientId;
    INFO("Logged with id {}", cast->clientId)
    this->connectionState = CurrentConnectionState::LOGGED_IN;
    for (auto& idNameTup : cast->currentClients)
    {
        INFO("Current clients and ids are {} Id {}", std::get<0>(idNameTup), std::get<1>(idNameTup));
    }

    // Some people care about the fact we logged in, fire a loggedIn event
    auto loggedInEvt = std::make_shared<LoggedIn>(this->clientId);
    EventManager::sInstance->QueueEvent(loggedInEvt);

    return true;
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

    auto challengeResponse = std::make_unique<ClientConnectionChallengeResponseMessage>(this->xOrSalt);

    TRACE("Queuing challenge response");
    this->QueueMessage(std::move(challengeResponse));

    // Login? I'm not sure if these in the right spot to do this but
    auto loginRequest = std::make_unique<ClientLoginMessage>(this->userName);
    this->QueueMessage(std::move(loginRequest));

    return true;
}

void NetworkManagerClient::Tick(uint32_t timeStamp)
{
    this->currentTime = timeStamp;

    // Read all the queued packets and do stuff with them
    std::shared_ptr<ReceivedPacket> packet = std::make_shared<ReceivedPacket>();
    bool hasMore = this->packetQueue.peek(*packet);
    TRACE("Current time {}, hasMore {}, packetTime {}", currentTime, hasMore, packet->timeRecieved);
    while (hasMore && packet->timeRecieved <= currentTime)
    {
        TRACE("Handling ")
        HandleReceivedPacket(*packet);
        this->packetQueue.pop();
        hasMore = this->packetQueue.peek(*packet);
    }

    packetManager.SetTime(currentTime);
}

void NetworkManagerClient::QueueMessage(std::shared_ptr<Message> messageToQueue)
{
    this->packetManager.SendMessage(std::move(messageToQueue));
}

// Create an output stream and write out our outgoing packet
void NetworkManagerClient::SendOutgoingPackets()
{
    TRACE("Sending Outgoing packets");
    std::shared_ptr<ReliableOrderedPacket> packet;
    if (connectionState == CurrentConnectionState::AUTHENTICATED ||
        connectionState == CurrentConnectionState::LOGGED_IN)
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

    // auto stream = OutputMemoryBitStream();
    const uint8_t* outData;
    uint32_t outSize;
    auto good = packetSerializer->WritePacket(packet, &outData, &outSize);

    if (!good)
    {
        ERROR("AHAHAHAH");
        return;
    }
    TRACE("Packet with messages {} sent", packet->messages->size());

    // ship it into the ether
    socketManager.SendTo(outData, outSize, *serverAddress);

    // TODO: send it to the debug port as well. From there it can be recorded or discarded
}
