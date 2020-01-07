#include "logger/Logger.h"
namespace gameobjects
{
void SetupLogger(logger::level level) { logger::InitLog(level, "GameObjects"); }
} // namespace gameobjects
