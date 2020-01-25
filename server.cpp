#include <iostream>
#include <string>
#include <thread>

// Linux socket stuff
#include <sys/socket.h>
#include <sys/un.h>

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

bool tick(uint32_t currentTime)
{
    // Read messages in a loop. messages can queue events
    NetworkManagerServer::sInstance->ProcessMessages();

    // TODO: Add timing
    EventManager::sInstance->FireEvents(10);

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

    // struct sockaddr_un addr;
    // char buf[100];
    // int fd, cl, rc;

    // if (argc > 1)
    //     socket_path = argv[1];

    // if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    // {
    //     perror("socket error");
    //     exit(-1);
    // }

    // memset(&addr, 0, sizeof(addr));
    // addr.sun_family = AF_UNIX;
    // if (*socket_path == '\0')
    // {
    //     *addr.sun_path = '\0';
    //     strncpy(addr.sun_path + 1, socket_path + 1, sizeof(addr.sun_path) - 2);
    // }
    // else
    // {
    //     strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    //     unlink(socket_path);
    // }

    // if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    // {
    //     perror("bind error");
    //     exit(-1);
    // }

    // if (listen(fd, 5) == -1)
    // {
    //     perror("listen error");
    //     exit(-1);
    // }

    // while (1)
    // {
    //     if ((cl = accept(fd, NULL, NULL)) == -1)
    //     {
    //         perror("accept error");
    //         continue;
    //     }

    //     while ((rc = read(cl, buf, sizeof(buf))) > 0)
    //     {
    //         printf("read %u bytes: %.*s\n", rc, rc, buf);
    //     }
    //     if (rc == -1)
    //     {
    //         perror("read");
    //         exit(-1);
    //     }
    //     else if (rc == 0)
    //     {
    //         printf("EOF\n");
    //         close(cl);
    //     }
    // }

    // unix_socket = socket(AF_UNIX, type, 0);
    // while (argc > 1)
    // {
    //     argc--;
    //     argv++;
    //     if (!strcmp(*argv, "--console"))
    //     {
    //         std::cout << "Starting a shell" << std::endl;
    //         t = std::thread(&interactive_console);
    //     }
    //     else if (!strcmp(*argv, "--log-file"))
    //     {
    //         argv++;
    //         argc--;
    //         log_file = *argv;
    //     }
    // }

    logger::InitLog(logger::DEBUG, "Main");
    // holistic::SetupNetworking();
    messagesToProcess.reserve(30);
    DEBUG("Starting")

    // Use a promise to not spool.
    Engine engine = Engine(initStuffs, tick);
    engine.Run();
}
