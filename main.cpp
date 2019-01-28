#include "networking/SocketManager.h"
#include <iostream>
#include <string>
#include <thread>

#define EXIT "exit"

void callback(std::shared_ptr<std::vector<uint8_t>> data)
{
    std::cout << "Calling" << std::endl;
    for (auto i = (*data).begin(); i != (*data).end(); ++i)
    {
        std::cout << unsigned(*i) << '|';
    }

    std::cout << std::endl;
}

const char** __argv;
int __argc;
int main(int argc, const char* argv[])
{

    // Logger::InitLog(spdlog::level::info, "server");
    auto socketManager = SocketManager(4500, callback);
    usleep(10 * 1000 * 1000);
    socketManager.Stop();
}
