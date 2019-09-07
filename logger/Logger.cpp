#include "Logger.h"
#include <fstream>
#include <iostream>

namespace logger
{

std::shared_ptr<spdlog::logger> the_log;

void InitLog(level level, std::string name)
{
    // spdlog::set_async_mode(4096);
    the_log = spdlog::stdout_color_mt(name);
    SetLevel(level);
}

void InitLog(level level, std::string name, std::string log_file)
{
    // spdlog::set_async_mode(4096);
    the_log = spdlog::basic_logger_mt(name, log_file);
    SetLevel(level);
}

void SetLevel(level level)
{
    auto spdLevel = convertToSpdlog(level);
    the_log->set_level(spdLevel);
}

spdlog::level::level_enum convertToSpdlog(level level)
{
    switch (level)
    {
    case TRACE:
        return spdlog::level::trace;
    case DEBUG:
        return spdlog::level::debug;
    case INFO:
        return spdlog::level::info;
    case WARN:
        return spdlog::level::warn;
    case ERR:
        return spdlog::level::err;
    case CRITICAL:
        return spdlog::level::critical;
    case OFF:
        return spdlog::level::off;
    default:
        return spdlog::level::debug;
    }
}

std::shared_ptr<spdlog::logger> GetSpdLogger() { return the_log; }

} // namespace logger
