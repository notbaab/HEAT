#pragma once

// Set log level to debug
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <spdlog/common.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <stdio.h>
#include <string>

namespace logger
{

enum level
{
    TRACE = spdlog::level::trace,
    DEBUG = spdlog::level::debug,
    INFO = spdlog::level::info,
    WARN = spdlog::level::warn,
    ERR = spdlog::level::err,
    CRITICAL = spdlog::level::critical,
    OFF = spdlog::level::off,
};

void InitLog(level level, std::string loggerName);
void InitLog(level level, std::string loggerName, std::string filename);
void SetLevel(level level);
spdlog::level::level_enum convertToSpdlog(level level);
std::shared_ptr<spdlog::logger> GetSpdLogger();

} // namespace logger

#define TRACE(fmt, ...) SPDLOG_LOGGER_TRACE(logger::GetSpdLogger(), fmt, ##__VA_ARGS__);
#define DEBUG(fmt, ...) SPDLOG_LOGGER_DEBUG(logger::GetSpdLogger(), fmt, ##__VA_ARGS__);
#define INFO(fmt, ...) logger::GetSpdLogger()->info(fmt, ##__VA_ARGS__);
#define WARN(fmt, ...) logger::GetSpdLogger()->warn(fmt, ##__VA_ARGS__);
#define ERROR(fmt, ...) logger::GetSpdLogger()->error(fmt, ##__VA_ARGS__);
#define CRITICAL(fmt, ...) logger::GetSpdLogger()->critical(fmt, ##__VA_ARGS__);
