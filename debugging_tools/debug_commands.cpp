#include <iostream>
#include <unordered_map>

#include "debug_commands.h"
#include "logger/Logger.h"

static std::unordered_map<std::string, DebugCommand> commands;

void add_command(std::string command, DebugCommand callback)
{
    commands.emplace(command, callback);
    INFO("Adding {}", command);
}

bool tryExecuteCommand(std::string command, std::vector<std::string> args, std::string* outInfo)
{
    if (args.size() != 0)
    {
        INFO("Executing {} with param {}", command, args[0]);
    }
    if (commands.find(command) != commands.end())
    {
        try
        {
            *outInfo = commands[command](args);
            return true;
        }
        // catch (const std::exception& e)
        catch (const std::runtime_error& e)
        {
            ERROR("Exception thrown {}", e.what());
            *outInfo = e.what();
            return false;
        }
    }
    WARN("Command {} not found", command);

    for (const auto& kv : commands)
    {
        WARN("{}", kv.first);
    }

    return false;
}

std::string GetAllCommands(std::vector<std::string>)
{
    INFO("Adding commands");
    std::vector<std::string> commandVec;
    commandVec.reserve(commands.size());

    for (const auto& kv : commands)
    {
        commandVec.push_back(kv.first);
    }

    std::string commandStr;
    for (const auto& cmd : commandVec)
    {
        commandStr += cmd;
        commandStr += "\n";
    }
    // commandStr.pop_back();

    return commandStr;
}

void addGetAllCommands() { add_command("get-commands", GetAllCommands); }
