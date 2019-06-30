#include <iostream>
#include <string>
#include <thread>

#include "holistic/SetupFuncs.h"

#include "managers/NetworkManagerServer.h"
#include "managers/PacketManager.h"
#include "messages/PlayerMessage.h"
#include "networking/SocketManager.h"

#define EXIT "exit"

// Global thing until I figure out how I want to do management stuffs.
// Maybe like a engine or something
std::shared_ptr<PacketManager> packetManager;

const char** __argv;
int __argc;
int main(int argc, const char* argv[])
{
    holistic::SetupNetworking();

    usleep(10 * 1000 * 1000);
}
