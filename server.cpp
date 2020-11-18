#include <iostream>
#include <sstream>
#include <string>

// Linux socket stuff
#include <sys/socket.h>
#include <sys/un.h>

#include "debugging_tools/debug_commands.h"
#include "debugging_tools/debug_socket.h"
#include "debugging_tools/debug_tools.h"
#include "engine/Engine.h"
#include "events/CreatePlayerOwnedObject.h"
#include "events/EventManager.h"
#include "events/EventRouter.h"
#include "events/PhysicsComponentUpdate.h"
#include "events/PlayerInputEvent.h"
#include "gameobjects/PlayerServer.h"
#include "gameobjects/Registry.h"
#include "gameobjects/World.h"
#include "logger/Logger.h"
#include "managers/NetworkManagerServer.h"
#include "managers/PacketManager.h"
#include "messages/ClientConnectionChallengeResponseMessage.h"
#include "messages/ClientConnectionRequestMessage.h"
#include "messages/ClientLoginMessage.h"
#include "messages/ClientLoginResponse.h"
#include "messages/ClientWelcomeMessage.h"
#include "messages/PlayerMessage.h"
#include "packets/AuthenticatedPacket.h"
#include "packets/Message.h"
#include "packets/MessageSerializer.h"
#include "packets/Packet.h"
#include "packets/PacketSerializer.h"
#include "packets/ReliableOrderedPacket.h"
#include "packets/UnauthenticatedPacket.h"

#include "networking/SocketManager.h"

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

void SplitCommandString(uint8_t* data, std::string* outCommand, std::vector<std::string>* outArgs)
{
    std::string tmp;
    std::stringstream stringStream;
    stringStream << (char*)data;

    getline(stringStream, tmp, ' ');
    *outCommand = tmp;
    while (getline(stringStream, tmp, ' '))
    {
        outArgs->push_back(tmp);
    }
}

std::string DebugCommandHandler(uint8_t* data, size_t size)
{
    // Accepting commands of the form <action> <args>
    std::string command;
    std::string out;
    std::vector<std::string> args;
    SplitCommandString(data, &command, &args);

    if (!tryExecuteCommand(command, args, &out))
    {
        ERROR("failed executing command {}", command);
        return "error\n\0";
    }
    return out;
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

void SetupDebugTools()
{
    InitDebugingTools();
    add_command("tick", DebugSetEngineTick);
}

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

    // Event constructors. Also messages
    AddMessageCtor(messageSerializer, CreatePlayerOwnedObject);
    AddMessageCtor(messageSerializer, PlayerInputEvent);

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
    EventManager::sInstance->AddListener(evtForwarder, CreatePlayerOwnedObject::EVENT_TYPE);
    EventManager::sInstance->AddListener(evtForwarder, PhysicsComponentUpdate::EVENT_TYPE);

    // World listens for requests to add objects
    auto addObject = CREATE_DELEGATE(&gameobjects::World::OnAddObject, gameobjects::World::sInstance);
    EventManager::sInstance->AddListener(addObject, CreatePlayerOwnedObject::EVENT_TYPE);

    EventRouter<PlayerInputEvent>::StaticInit();
    auto playerInputRouter = CREATE_DELEGATE_LAMBDA((EventRouter<PlayerInputEvent>::sInstance->RouteEvent));
    EventManager::sInstance->AddListener(playerInputRouter, PlayerInputEvent::EVENT_TYPE);
}

void initStuffs()
{
    SetupNetworking();
    SetupWorld();
    SetupDebugTools();
}

const char** __argv;
int __argc;
int main(int argc, const char* argv[])
{
    logger::InitLog(logger::DEBUG, "Main");
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

    // Use a promise to not spool.
    serverInstance.reset(new Engine(initStuffs, tick));
    serverInstance->Run();
}
