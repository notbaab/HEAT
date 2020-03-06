#include <iostream>
#include <sstream>
#include <string>

// Linux socket stuff
#include <sys/socket.h>
#include <sys/un.h>

#include "debugging_tools/debug_commands.h"
#include "debugging_tools/debug_socket.h"
#include "engine/Engine.h"
#include "events/EventManager.h"
#include "gameobjects/World.h"
#include "holistic/SetupFuncs.h"
#include "logger/Logger.h"
#include "managers/NetworkManagerServer.h"
#include "managers/PacketManager.h"
#include "messages/PlayerMessage.h"
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
    int fps = stoi(args[0]);
    INFO("Setting server to {} fps", fps);
    serverInstance->SetTicksPerSecond(fps);
    return "";
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

void DebugCommandHandler(uint8_t* data, size_t size)
{
    // Accepting commands of the form <action> <args>
    std::string command;
    std::string out;
    std::vector<std::string> args;
    SplitCommandString(data, &command, &args);

    if (!tryExecuteCommand(command, args, &out))
    {
        ERROR("failed executing command {}", command);
    }
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

void initStuffs()
{
    holistic::SetupNetworking();
    holistic::SetupWorld();
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

    add_command("tick", DebugSetEngineTick);
    // holistic::SetupNetworking();
    messagesToProcess.reserve(30);
    DEBUG("Starting")

    // Use a promise to not spool.
    serverInstance.reset(new Engine(initStuffs, tick));
    serverInstance->Run();
}
