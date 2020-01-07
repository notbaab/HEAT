
SET(SPDLOG_BUILD_EXAMPLES OFF CACHE BOOL "don't build spdlog example")

add_subdirectory(lib/spdlog)

add_library(logger SHARED Logger.cpp)
target_include_directories(logger PUBLIC lib/spdlog/include)
spdlog_enable_warnings(logger)

target_link_libraries(logger PUBLIC spdlog::spdlog)
