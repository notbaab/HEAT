#include <iostream>
#include <sstream>
// Linux socket stuff
#include <sys/socket.h>

#include "IO/InputMemoryBitStream.h"
#include "IO/JsonInputMemoryStream.h"
#include "IO/JsonOutputMemoryStream.h"
#include "IO/OutputMemoryBitStream.h"
#include "debugging_tools/debug_commands.h"
#include "debugging_tools/debug_socket.h"
#include "debugging_tools/debug_tools.h"
#include "dvr/DVR.h"
#include "engine/Engine.h"
#include "events/CreatePlayerOwnedObject.h"
#include "events/EventManager.h"
#include "events/EventRouter.h"
#include "events/PhysicsComponentUpdate.h"
#include "events/PlayerInputEvent.h"
#include "events/RemoveClientOwnedGameObjectsEvent.h"
#include "events/RemoveGameObjectEvent.h"
#include "gameobjects/PlayerServer.h"
#include "gameobjects/Registry.h"
#include "gameobjects/World.h"
#include "holistic/Configurator.h"
#include "holistic/SetupSerializers.h"
#include "managers/NetworkManagerServer.h"
#include "messages/ClientConnectionChallengeResponseMessage.h"
#include "messages/ClientConnectionRequestMessage.h"
#include "messages/ClientLoginMessage.h"
#include "messages/ClientLoginResponse.h"
#include "messages/ClientWelcomeMessage.h"
#include "messages/LogoutMessage.h"
#include "messages/PlayerMessage.h"
#include "packets/AuthenticatedPacket.h"
#include "packets/UnauthenticatedPacket.h"

#define EXIT "exit"

// Global thing until I figure out how I want to do management stuffs.
// Maybe like a engine or something
std::shared_ptr<PacketManager> packetManager;
std::vector<std::shared_ptr<Message>> messagesToProcess;
std::shared_ptr<spdlog::logger> the_log;

std::string PrintPlayerInfo(std::vector<std::string> args) { return args[0]; }

static std::shared_ptr<Engine> serverInstance;

std::string DebugSetEngineTick(std::vector<std::string> args)
{
    if (args.size() != 1)
    {
        throw std::runtime_error("Must supply one argument");
    }
    int fps = stoi(args[0]);
    INFO("Setting server to {} fps", fps);
    serverInstance->SetTicksPerSecond(fps);

    std::stringstream s;
    s << "Set engine tick to " << fps;

    return s.str();
}

std::string SetConfigVar(std::vector<std::string> args)
{
    if (args.size() != 2)
    {
        throw std::runtime_error("Must supply 2 arguments, a varible and a value");
    }

    std::string varibleName = args[0];
    std::string value = args[1];
    bool success = Configurator::sInstance->SetConfigVar(varibleName, value);

    std::stringstream s;
    if (success)
    {
        s << "Success, set to " << value;
        return s.str();
    }
    else
    {
        return "Failed ";
    }
}

std::string SetTime(std::vector<std::string> args)
{
    if (args.size() != 1)
    {
        return "Must give one argument";
    }

    int timeStamp = stoi(args[0]);

    INFO("Setting server to {} timeStamp", timeStamp);
    serverInstance->SetCurrentTime(timeStamp);

    std::stringstream s;
    s << "Set engine timestamp to  to " << timeStamp;

    return s.str();
}

std::string DVRReplayPackets(std::vector<std::string> args)
{
    if (args.size() != 1)
    {
        return "Must give one argument";
    }

    int timeStamp = stoi(args[0]);

    INFO("Setting server to {} timeStamp", timeStamp);
    serverInstance->SetCurrentTime(timeStamp);

    auto& nms = NetworkManagerServer::sInstance;
    nms->playingBack = true;

    const uint32_t max = 2400;
    PacketInfo outPackets[max];
    uint32_t count = DVR::sInstance->GetRecvPackets(max, outPackets);

    for (int i = 0; i < count; ++i)
    {
        NetworkManagerServer::sInstance->AddPacketToQueue(outPackets[i]);
    }
    return "Did something";
}

std::string DVRWriteMessages(std::vector<std::string> args)
{
    if (args.size() != 1)
    {
        throw std::runtime_error("Must supply 1 arguments, a file location ");
    }

    std::string fileLoc = args[0];

    DVR::sInstance->WriteReceivedPacketsToFile(fileLoc);
    DVR::sInstance->ReadReceivedPacketsFromFile(fileLoc);
    return "\n\0";
}

std::string DVRGetMessages(std::vector<std::string> args)
{
    auto receivedMessages = DVR::sInstance->GetRecvMessages(0, 4000);

    for (auto& message : receivedMessages)
    {
        INFO("message id {}, got at {}", message.message->GetId(), message.time);
    }

    return "\n\0";
}

std::string DVRGetPackets(std::vector<std::string> args)
{
    const uint32_t max = 2400;
    PacketInfo outPackets[max];
    uint32_t count = DVR::sInstance->GetRecvPackets(max, outPackets);

    for (int i = 0; i < count; ++i)
    {
        outPackets[i].packet->GetClassIdentifier();
    }

    return "\n\0";
}

bool tick(uint32_t currentTime)
{
    // Read messages in a loop. messages can queue events
    NetworkManagerServer::sInstance->ProcessMessages();

    // TODO: Add timing
    EventManager::sInstance->FireEvents(currentTime);

    // Event manager fire events the the client has sent over and any that were in
    // the old queue?
    gameobjects::World::sInstance->Update(currentTime);
    // World update stuff. Queues events that it needs to notify other players and
    // system

    // Send out server world state event or individual objects can send out their
    // state over an event Unclear how do.
    // This will actually be after game logic. I think? Idk
    NetworkManagerServer::sInstance->SendOutgoingPackets();

    NetworkManagerServer::sInstance->Tick(currentTime);
    return true;
}

void VariableAdded(std::string variable) { add_hint("set-var", variable); }

void SetupDebugTools()
{
    InitDebugingTools();
    Configurator::StaticInit(VariableAdded);

    add_command("tick", DebugSetEngineTick);
    add_command("set-var", SetConfigVar);
    add_command("dvr-get-packets", DVRGetPackets);
    add_command("dvr-get-messages", DVRGetMessages);
    add_command("dvr-write-messages", DVRWriteMessages);
    add_command("dvr-replay-packets", DVRReplayPackets);
    add_command("set-engine-time", SetTime);
}

void SetupNetworking()
{
    std::string destination = "127.0.0.1:4500";

    auto messageSerializer = std::make_shared<MessageSerializer>();

    holistic::SetupMessageSerializer(messageSerializer);

    auto bitReader = std::make_unique<InputMemoryBitStream>();
    auto bitWriter = std::make_unique<OutputMemoryBitStream>();
    auto packetReader = std::make_unique<StructuredDataReader>(std::move(bitReader));
    auto packetWriter = std::make_unique<StructuredDataWriter>(std::move(bitWriter));

    auto packetSerializer =
        std::make_shared<PacketSerializer>(messageSerializer, std::move(packetReader), std::move(packetWriter));
    holistic::SetupPacketSerializer(packetSerializer);

    // Init our singleton
    NetworkManagerServer::StaticInit(4500, packetSerializer);
    DVR::StaticInit();
}

// Function that is called to create the registry with all the
// create functions
void SetupWorld()
{
    EventManager::StaticInit();

    gameobjects::World::StaticInit();

    // create registry and add all the creation functions we know about
    gameobjects::Registry::StaticInit(gameobjects::World::StaticAddGameObject);
    gameobjects::Registry::sInstance->RegisterCreationFunction(gameobjects::PLAYER_ID,
                                                               gameobjects::PlayerServer::StaticCreate);

    // Event forwarder takes events and pushes them to clients
    auto evtForwarder = CREATE_DELEGATE(&NetworkManagerServer::EventForwarder, NetworkManagerServer::sInstance);
    EventManager::sInstance->AddListener(evtForwarder, PhysicsComponentUpdate::EVENT_TYPE);

    // World listens for requests to add objects
    auto addObject = CREATE_DELEGATE(&gameobjects::World::OnAddObject, gameobjects::World::sInstance);
    EventManager::sInstance->AddListener(addObject, CreatePlayerOwnedObject::EVENT_TYPE);
    EventManager::sInstance->AddListener(evtForwarder, CreatePlayerOwnedObject::EVENT_TYPE);

    auto removeObject = CREATE_DELEGATE(&gameobjects::World::OnRemoveObject, gameobjects::World::sInstance);
    EventManager::sInstance->AddListener(removeObject, RemoveGameObjectEvent::EVENT_TYPE);
    EventManager::sInstance->AddListener(evtForwarder, RemoveGameObjectEvent::EVENT_TYPE);

    auto removeClientObjects =
        CREATE_DELEGATE(&gameobjects::World::OnRemoveClientOwnedObjects, gameobjects::World::sInstance);
    EventManager::sInstance->AddListener(removeClientObjects, RemoveClientOwnedGameObjectsEvent::EVENT_TYPE);
    EventManager::sInstance->AddListener(evtForwarder, RemoveClientOwnedGameObjectsEvent::EVENT_TYPE);

    EventRouter<PlayerInputEvent>::StaticInit();
    auto playerInputRouter = CREATE_DELEGATE_LAMBDA((EventRouter<PlayerInputEvent>::sInstance->RouteEvent));
    EventManager::sInstance->AddListener(playerInputRouter, PlayerInputEvent::EVENT_TYPE);

    // DVR Recording
    auto recievedPacket = CREATE_DELEGATE(&DVR::PacketReceived, DVR::sInstance);
    EventManager::sInstance->AddListener(recievedPacket, PacketReceivedEvent::EVENT_TYPE);
}

void initStuffs()
{
    SetupDebugTools();
    SetupNetworking();
    SetupWorld();
}

int main(int argc, const char* argv[])
{
    logger::InitLog(logger::TRACE, "Main");
    while (argc > 1)
    {
        argc--;
        argv++;
        if (!strcmp(*argv, "--debug-socket"))
        {
            argv++;
            argc--;
            const char* socketPath = *argv;
            INFO("Starting a debug socket at {}", socketPath);
            SpawnSocket(socketPath, DebugCommandHandler);
        }
    }

    messagesToProcess.reserve(30);
    DEBUG("Starting")

    // PIMP: Use a promise to not spool.
    serverInstance.reset(new Engine(initStuffs, tick));
    serverInstance->Run();
}
