#include "NetworkManagerServer.h"
#include "dvr/PacketReceivedEvent.h"
#include "events/CreatePlayerOwnedObject.h"
#include "events/EventManager.h"
#include "events/RemoveClientOwnedGameObjectsEvent.h"
#include "gameobjects/World.h"
#include "holistic/Configurator.h"
#include "messages/ClientConnectionChallengeMessage.h"
#include "messages/ClientConnectionChallengeResponseMessage.h"
#include "messages/ClientConnectionRequestMessage.h"
#include "messages/ClientLoginMessage.h"
#include "messages/ClientLoginResponse.h"
#include "packets/AuthenticatedPacket.h"
#include "packets/UnauthenticatedPacket.h"

void NetworkManagerServer::StaticInit(uint16_t port, std::shared_ptr<PacketSerializer> packetSerializer)
{
    sInstance.reset(new NetworkManagerServer(port, packetSerializer));
}

NetworkManagerServer::NetworkManagerServer(uint16_t port, std::shared_ptr<PacketSerializer> packetSerializer)
    : HNetworkManager(port, packetSerializer), playingBack(false)
{
    SetupConfigVars();
}

void NetworkManagerServer::AddPacketToQueue(ReceivedPacket& packet) { packetQueue.push(packet); }

void NetworkManagerServer::DataReceivedCallback(SocketAddress fromAddress, std::unique_ptr<std::vector<uint8_t>> data)
{
    // Deserialize raw byte data
    auto packets = packetSerializer->ReadPackets(std::move(data));

    // Loop over packets and depending on the state of the client, determine
    // if they are allowed to be sending those types of packets
    for (auto const& packet : packets)
    {
        auto receivedPacket = ReceivedPacket();
        // receivedPacket.timeRecieved = currentTime + 100;
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

// challenged clients will send authenticated packets but we only send it
// unauthenticated packets since we don't know if it' got through yet
bool NetworkManagerServer::HandleChallengedClient(ClientData& client, const std::shared_ptr<Packet> packet)
{
    // A challenged client will send authed packets, if it's sending unauthed,
    // ignore. Maybe? That does seem kinda weird to enforce
    auto cast = std::dynamic_pointer_cast<AuthenticatedPacket>(packet);
    if (cast == nullptr)
    {
        WARN("Didn't get a authenticated packet from a challenged client. Maybe challenge hasn't "
             "got to the client?");
        return false;
    }
    if (!playingBack && cast->expectedSalt != client.xOrSalt)
    {
        WARN("xor'd salt doesn't match client, expecting {} got {} ignoring packet", cast->expectedSalt,
             client.xOrSalt);
        return false;
    }

    // passed stuff, let the packet manager read the packet now
    client.packetManager.ReadPacket(cast);
    return true;
}

// New client means we first need to to check if they sent us an unauthenticated
// packet with the connection request message. If it's all good, send
// a challenge with a random salt
bool NetworkManagerServer::HandleNewClient(ClientData& client, const std::shared_ptr<Packet> packet)
{
    auto cast = std::dynamic_pointer_cast<UnauthenticatedPacket>(packet);
    if (cast == nullptr)
    {
        ERROR("Couldn't cast Packet to unauthenticated packet");
        return false;
    }

    // got a correct packet, read it and queue up a salt exchange
    client.packetManager.ReadPacket(cast);

    // TODO: Since this is in it's own thread, it shouldn't look into the
    // messages of the packet and check if it isn't including other messages.
    // We handle that by only reading the messages that the client state allows
    // us too but I kind of want a mechanism in the packet serialization layer
    // that says only messages of these types can be in this packet. Maybe too
    // much overhead though. For now return true and the the ProcessMessages
    // handle the message

    return true;
}

// Handles the passed in message
void NetworkManagerServer::HandleMessage(uint64_t clientKey, std::shared_ptr<Message> message)
{
    auto& client = cData.at(clientKey);

    // Game events all handled via the event manager
    if (message->GetTypeIdentifier() == Event::TYPE_ID)
    {
        auto evt = std::static_pointer_cast<Event>(message);
        EventManager::sInstance->QueueEvent(evt);
        return;
    }

    // The rest is for connection related messages
    switch (message->GetClassIdentifier())
    {
    case ClientConnectionRequestMessage::CLASS_ID:
        ReadConnectionRequestMessage(client, message);
        break;
    case ClientConnectionChallengeResponseMessage::CLASS_ID:
        ReadChallengeResponseMessage(client, message);
        break;
    case ClientLoginMessage::CLASS_ID:
        ReadLoginMessage(client, message);
        break;
    default:
        ERROR("Didn't handle message of type {}, Raw id {}", message->IdentifierToString(),
              message->GetClassIdentifier());
    }
}

void NetworkManagerServer::ProcessMessages()
{
    // loop over all the packet managers, receive the messages then process them
    // It may be possible in the future we want some sort of ordering of
    // which messages to handle so we aren't just going in a set order each time
    // clear our message buf so we can read the latest and greatest

    for (auto& cDataElement : cData)
    {
        messageBuf.clear();
        auto& client = cDataElement.second;
        client.packetManager.ReceiveMessages(messageBuf);

        for (auto const& message : messageBuf)
        {
            HandleMessage(cDataElement.first, message);
        }
    }
}

bool NetworkManagerServer::ReadChallengeResponseMessage(ClientData& client, const std::shared_ptr<Message> message)
{
    if (client.state != ClientConnectionState::CHALLENGED)
    {
        WARN("Client {} not in challenged state but got a challenged response message, ignoring",
             client.socketAddress.ToString());
        return false;
    }

    INFO("Reading CCRP from client {}", client.socketAddress.ToString());
    auto castMsg = std::static_pointer_cast<ClientConnectionChallengeResponseMessage>(message);

    // successfully got the response, move to authenticated client
    client.state = ClientConnectionState::AUTHENTICATED;
    return true;
}
bool NetworkManagerServer::ReadLoginMessage(ClientData& client, const std::shared_ptr<Message> message)
{
    auto castMsg = std::static_pointer_cast<ClientLoginMessage>(message);

    // successfully got the response, move to authenticated client
    client.userName = castMsg->userName;
    uint32_t id = holistic::GenerateSalt();
    client.gameId = id;
    client.state = ClientConnectionState::LOGGED_IN;
    INFO("Logging in {} with id {}", client.userName, client.gameId);

    std::vector<std::tuple<uint32_t, std::string>> loggedInClients;

    // PIMP save this so we don't create it each time?
    for (auto& element : cData)
    {
        auto& c = element.second;
        loggedInClients.emplace_back(c.gameId, c.userName);
    }

    // PIMP will copy the whole vector. Probably fine?
    auto resp = std::make_unique<ClientLoginResponse>(client.gameId, loggedInClients);
    // respond to this client only
    client.packetManager.SendMessage(std::move(resp));

    // queue event to create a player owned player object
    auto createPlayer = gameobjects::World::sInstance->CreatePlayerCreationEvent(client.gameId);
    EventManager::sInstance->QueueEvent(createPlayer);

    // Add the rest of the world objects to the packet
    auto welcomeEvents = gameobjects::World::sInstance->CreateWelcomeStateEvents();
    for (auto& evt : welcomeEvents)
    {
        client.packetManager.SendMessage(evt);
    }

    auto snapShot = gameobjects::World::sInstance->CreateWorldSnapshot();
    for (auto& evt : snapShot)
    {
        client.packetManager.SendMessage(evt);
    }

    return true;
}

// bool NetworkManagerServer::QueueBroadCastMessage(){}

bool NetworkManagerServer::ReadConnectionRequestMessage(ClientData& client, const std::shared_ptr<Message> message)
{
    if (client.state != ClientConnectionState::CONNECTING)
    {
        WARN("Client {} not in connecting state but got a connection request message, ignoring",
             client.socketAddress.ToString());
        return false;
    }

    // assume the message is a CCRM and go from there
    INFO("Reading CCRM from client {}", client.socketAddress.ToString());
    auto castMsg = std::static_pointer_cast<ClientConnectionRequestMessage>(message);

    // Take the salt from the client
    client.clientSalt = castMsg->salt;

    // Tell it our salt
    client.serverSalt = holistic::GenerateSalt();
    client.xOrSalt = client.clientSalt ^ client.serverSalt;

    // It has been challenged
    client.state = ClientConnectionState::CHALLENGED;
    auto challenge = std::make_unique<ClientConnectionChallengeMessage>(client.serverSalt);

    DEBUG("Queuing challenge messages")
    client.packetManager.SendMessage(std::move(challenge));

    return true;
}

void NetworkManagerServer::SendOutgoingPackets()
{
    for (auto& element : cData)
    {
        auto& client = element.second;

        std::shared_ptr<ReliableOrderedPacket> packet;

        if (client.state == ClientConnectionState::AUTHENTICATED || client.state == ClientConnectionState::LOGGED_IN)
        {
            // This is stupid. Why do I have to cast it to reach and and set it?
            packet = client.packetManager.WritePacket(AuthenticatedPacket::CLASS_ID);
            auto cast = std::static_pointer_cast<AuthenticatedPacket>(packet);
            cast->expectedSalt = client.xOrSalt;
            TRACE("Sending authed packet with {} messages", packet->messages->size());
        }
        else
        {
            packet = client.packetManager.WritePacket(UnauthenticatedPacket::CLASS_ID);
            TRACE("Sending unauthed packet with {} messages", packet->messages->size());
        }

        const uint8_t* outData;
        uint32_t outSize;
        auto good = packetSerializer->WritePacket(packet, &outData, &outSize);
        if (!good)
        {
            ERROR("AHAHAHAH");
            return;
        }
        // ship it into the ether
        socketManager.SendTo(outData, outSize, client.socketAddress);
    }
}

// Callback that can be attached to events that need to be sent out to the connected
// clients as messages
void NetworkManagerServer::EventForwarder(std::shared_ptr<Event> evt)
{
    TRACE("Forwarding event");
    BroadcastMessage(evt);
}

void NetworkManagerServer::SetupConfigVars()
{
    Configurator::sInstance->CreateConfigVar("logoutTime", logoutTimeMs);
    Configurator::sInstance->CreateConfigVar("running", logoutTimeMs);

    // Configurator::sInstance->SetConfigVar("logoutTime", 10000);
}

void NetworkManagerServer::BroadcastMessage(std::shared_ptr<Message> msg)
{
    // For now, create a copy for every client. Oh well.
    for (auto& element : cData)
    {
        auto& client = element.second;
        if (client.state != LOGGED_IN)
        {
            continue;
        }

        client.packetManager.SendMessage(msg);
    }
}

bool NetworkManagerServer::LogoutClient(uint64_t clientKey)
{
    INFO("Logging out client {}", clientKey);
    if (cData.count(clientKey) == 0)
    {
        ERROR("No client found at {}", clientKey);
        return false;
    }

    auto& c = cData.at(clientKey);

    cData.erase(clientKey);

    auto evt = std::make_shared<RemoveClientOwnedGameObjectsEvent>(c.gameId);
    EventManager::sInstance->QueueEvent(evt);

    return true;
}

void NetworkManagerServer::HandleReceivedPacket(ReceivedPacket& receivedPacket)
{
    auto fromAddress = receivedPacket.fromAddress;
    uint64_t key = fromAddress.GetIPPortKey();
    auto cDataPair = cData.find(key);
    std::shared_ptr<Packet> packet = receivedPacket.packet;

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
    client.lastHeardFrom = this->currentTime;

    switch (client.state)
    {
    case ClientConnectionState::CONNECTING:
        // Client just connected, move to challenge and queue up sending a challenge
        HandleNewClient(client, packet);
        break;
    case ClientConnectionState::CHALLENGED:
    case ClientConnectionState::AUTHENTICATED:
    case ClientConnectionState::LOGGED_IN:
        HandleChallengedClient(client, packet);
        break;
    case ClientConnectionState::DISCONNECTED:
        break;
    }
}

void NetworkManagerServer::Tick(uint32_t currentTime)
{
    this->currentTime = currentTime;

    // Read all the queued packets and do stuff with them
    std::shared_ptr<ReceivedPacket> packet = std::make_shared<ReceivedPacket>();
    bool hasMore = this->packetQueue.peek(*packet);
    while (hasMore && packet->timeRecieved <= currentTime)
    {
        HandleReceivedPacket(*packet);
        this->packetQueue.pop();
        hasMore = this->packetQueue.peek(*packet);
    }

    // Packets have been read, read the

    // PIMP: Only removes removes one client once per tick.
    // I'm not sure how much we actually care about that.
    uint64_t removeClient = 0;

    for (auto it = cData.begin(); it != cData.end(); it++ /* not hoisted */ /* no increment */)
    {
        auto& client = it->second;
        if (currentTime - client.lastHeardFrom > logoutTimeMs)
        {
            INFO("Client {}:{} not heard from since {}, it's now {}, logging out", client.userName,
                 client.socketAddress.ToString(), client.lastHeardFrom, currentTime);
            // We could remove it with the iterator...
            removeClient = it->first;
            continue;
        }

        client.packetManager.SetTime(currentTime);
    }

    if (removeClient != 0)
    {
        LogoutClient(removeClient);
    }
}
