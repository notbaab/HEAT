#include <iostream>
#include <unordered_map>

#include "debug_commands.h"
#include "logger/Logger.h"

static std::unordered_map<std::string, DebugCommand> commands;
static std::unordered_map<std::string, std::vector<std::string>> hints_map;

void add_command(std::string command, DebugCommand callback)
{
    commands.emplace(command, callback);
    TRACE("Adding {}", command);
}

void add_hint(std::string commandToHint, std::string valueToHint)
{
    TRACE("Adding hint for {}, '{}'", valueToHint);
    if (hints_map.count(commandToHint) == 0)
    {
        hints_map[commandToHint] = std::vector<std::string>();
    }

    hints_map.at(commandToHint).emplace_back(valueToHint);
}

bool tryExecuteCommand(std::string command, std::vector<std::string> args, std::string* outInfo)
{
    if (args.size() != 0)
    {
        // TODO: Join string
        INFO("Executing {} with param {}", command, args[0]);
    }
    if (commands.find(command) != commands.end())
    {
        try
        {
            *outInfo = commands[command](args);
            TRACE("Returning '{}'", *outInfo);
            return true;
        }
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

    return commandStr;
}

std::string GetAllHints(std::vector<std::string>)
{
    INFO("Getting all hints");

    std::string commandStr;

    for (auto element : hints_map)
    {
        std::string word = element.first;

        commandStr += word + " ";
        for (const auto& cmd : element.second)
        {
            commandStr += cmd + " ";
        }
        commandStr += "\n";
    }

    commandStr += "\n";
    return commandStr;
}

void addGetAllCommands() { add_command("get-commands", GetAllCommands); }
void addGetAllHints() { add_command("get-command-hints", GetAllHints); }
