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
    // std::string log_file;
    // std::thread t;
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

    // Logger::InitLog(spdlog::level::info, "server");
    auto socketManager = SocketManager(4500, callback);
    usleep(10 * 1000 * 1000);
    socketManager.Stop();
}
