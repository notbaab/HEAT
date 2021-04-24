#pragma once
#include <functional>
#include <string>
#include <vector>

using DebugCommand = std::function<std::string(std::vector<std::string>)>;
void add_command(std::string command, DebugCommand function);
bool tryExecuteCommand(std::string command, std::vector<std::string> args, std::string* outInfo);
void addGetAllCommands();
void addGetAllHints();
void add_hint(std::string commandToHint, std::string valueToHint);

void SplitCommandString(std::string data, std::string* outCommand, std::vector<std::string>* outArgs);
std::string DebugCommandHandler(uint8_t* data, size_t size);
