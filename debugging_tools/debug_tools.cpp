#include "debug_tools.h"
#include "debug_commands.h"
#include "logger/Logger.h"

void InitDebugingTools()
{
    DEBUG("Initing debugging tools");
    addGetAllCommands();
    addGetAllHints();
}
