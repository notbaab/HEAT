#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

#include "debug_socket.h"

class SocketThreadContainer
{
  public:
    static std::unique_ptr<SocketThreadContainer> BuildContainer(const char* socketPath,
                                                                 ReadCallback callback)
    {
        int fd;
        struct sockaddr_un addr;

        if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
        {
            perror("socket error");
            return nullptr;
        }
        std::cout << fd << "1" << std::endl;

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;

        strncpy(addr.sun_path, socketPath, sizeof(addr.sun_path) - 1);
        unlink(socketPath);

        if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        {
            perror("bind error");
            return nullptr;
        }
        std::cout << fd << "1" << std::endl;

        if (listen(fd, 5) == -1)
        {
            perror("listen error");
            return nullptr;
        }

        return std::make_unique<SocketThreadContainer>(socketPath, fd, callback);
    }

    void readThread()
    {
        uint8_t buf[100];
        int readCount, acceptedFd;

        while (1)
        {
            if ((acceptedFd = accept(socketFd, NULL, NULL)) == -1)
            {
                perror("accept error");
                return;
            }
            while ((readCount = read(acceptedFd, buf, sizeof(buf))) > 0)
            {
                callback(buf, readCount);
            }
            if (readCount == -1)
            {
                perror("Unexpected error while reading");
                return;
            }
            else if (readCount == 0)
            {
                close(acceptedFd);
            }
        }
    }

    void start() { socketThread = std::thread(&SocketThreadContainer::readThread, this); }
    void stop()
    {
        close(socketFd);
        socketThread.join();
    }

    SocketThreadContainer(const char* socketPath, int socketFd, ReadCallback callback)
        : socketPath(socketPath), socketFd(socketFd), callback(callback)
    {
    }

  private:
    const char* socketPath;
    ReadCallback callback;
    int socketFd;
    std::thread socketThread;
};

static std::unordered_map<const char*, std::unique_ptr<SocketThreadContainer>> socketMap;

void StopSocketThreads()
{
    for (auto&& i : socketMap)
    {
        i.second->stop();
    }
}

bool SpawnSocket(const char* socketPath, ReadCallback callback)
{
    if (socketMap.find(socketPath) != socketMap.end())
    {
        // already exists
        return false;
    }

    std::unique_ptr<SocketThreadContainer> container =
        SocketThreadContainer::BuildContainer(socketPath, callback);
    container->start();
    socketMap[socketPath] = std::move(container);

    return true;
}
