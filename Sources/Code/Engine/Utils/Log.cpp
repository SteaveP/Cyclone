#include "Log.h"

#include "Engine/Core/Types.h"
#include "Engine/Utils/Config.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <spdlog/logger.h>

namespace Cyclone
{

Ptr<spdlog::logger> GLoggerPtr;
spdlog::logger* GLogger() { return GLoggerPtr.get(); }

C_STATUS GInitLogging(String Filename, String GreetingsString)
{
    bool EnableLogging = true;
    EnableLogging = GET_CONFIG()["Log"].value("EnableLogging", EnableLogging);
    GStartupArguments()->GetParameter("-EnableLogging", EnableLogging);

    if (EnableLogging)
    {
        auto ConsoleLoggerSink = MakeShared<spdlog::sinks::stdout_color_sink_mt>();
        auto FileLoggerSink = MakeShared<spdlog::sinks::basic_file_sink_mt>(Filename, true);

        GLoggerPtr = MakeShared<spdlog::logger>("Logger", spdlog::sinks_init_list{ ConsoleLoggerSink, FileLoggerSink });

        spdlog::set_default_logger(GLoggerPtr);

        // Set Level
        {
            String LevelStr = "Info";
            LevelStr = GET_CONFIG()["Log"].value("Level", LevelStr);
            GStartupArguments()->GetParameter("-LogLevel", LevelStr);

            spdlog::level::level_enum Level = spdlog::level::info;
            if (LevelStr == "Trace")
                Level = spdlog::level::trace;

            GLogger()->info(GreetingsString);
            GLogger()->info("Start Logging with {} level to {} file", spdlog::level::to_string_view(Level), Filename);

            spdlog::set_level(Level);
        }

        uint32 FlushIntervalInSeconds = 1;
        GET_CONFIG()["Log"].value("FileFlushIntervalInSeconds", FlushIntervalInSeconds);

        spdlog::flush_every(std::chrono::seconds(FlushIntervalInSeconds));
    }
    else
    {
        spdlog::set_default_logger(nullptr);
    }

    return C_STATUS::C_STATUS_OK;
}

void GDeInitLogging()
{
    if (GLoggerPtr)
    {
        GLoggerPtr->flush();
    }

    GLoggerPtr.reset();
}

} // namespace Cyclone
