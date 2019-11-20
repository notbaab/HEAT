#include <iostream>
#include <string>
#include <thread>

#include "engine/Engine.h"
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

bool tick()
{
    // Read messages in a loop
    NetworkManagerServer::sInstance->ProcessMessages();
    NetworkManagerServer::sInstance->Tick(0.03);
    // Send out any new messages we need to send.
    // This will actually be after game logic. I think? Idk
    NetworkManagerServer::sInstance->SendOutgoingPackets();
    return true;
}

void initStuffs() { holistic::SetupNetworking(); }

const char** __argv;
int __argc;
int main(int argc, const char* argv[])
{
    logger::InitLog(logger::DEBUG, "Main");
    // holistic::SetupNetworking();
    messagesToProcess.reserve(30);
    DEBUG("Starting")

    // Use a promise to not spool.
    Engine engine = Engine(initStuffs, tick);
    engine.Run();
}
