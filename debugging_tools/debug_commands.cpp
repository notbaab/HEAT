#include <iostream>
#include <unordered_map>

#include "debug_commands.h"
#include "logger/Logger.h"

static std::unordered_map<std::string, DebugCommand> commands;

void add_command(std::string command, DebugCommand callback)
{
    commands.emplace(command, callback);
}

bool tryExecuteCommand(std::string command, std::vector<std::string> args, std::string* outInfo)
{
    if (commands.find(command) != commands.end())
    {
        try
        {
            *outInfo = commands[command](args);
            return true;
        }
        catch (const std::exception& e)
        {
            ERROR("Exception thrown {}", e.what());
            return false;
        }
    }
    return false;
}

std::vector<std::string> GetAllCommands()
{
    std::vector<std::string> keys;
    keys.reserve(commands.size());

    for (auto kv : commands)
    {
        keys.push_back(kv.first);
    }
    return keys;
}
