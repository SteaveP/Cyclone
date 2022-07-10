#pragma once

#include "Engine/Core/Types.h"
#include "Engine/EngineModule.h"

// #todo_log add as static lib to decrease compile time
// #define SPDLOG_COMPILED_LIB
#define SPDLOG_NO_EXCEPTIONS
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>

namespace Cyclone
{

ENGINE_API spdlog::logger* GLogger();

#define LOG_INFO(...) SPDLOG_LOGGER_INFO(Cyclone::GLogger(), __VA_ARGS__)
#define LOG_TRACE(...) SPDLOG_LOGGER_TRACE(Cyclone::GLogger(), __VA_ARGS__)
#define LOG_WARN(...) SPDLOG_LOGGER_WARN(Cyclone::GLogger(), __VA_ARGS__)
#define LOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(Cyclone::GLogger(), __VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_LOGGER_ERROR(Cyclone::GLogger(), __VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(Cyclone::GLogger(), __VA_ARGS__)

C_STATUS ENGINE_API GInitLogging(String Filename, String GreetingsString);
void ENGINE_API GDeInitLogging();

} // namespace Cyclone
