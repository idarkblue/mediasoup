#define PMS_CLASS "pingos::Log"

#include <iostream>
#include "Log.hpp"

namespace pingos {

std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> Log::consolSink;
std::shared_ptr<spdlog::sinks::daily_file_sink_mt> Log::fileSink;

void Log::ClassInit(std::string filename, std::string fileLevel, std::string consolLevel)
{
    try
    {
        consolSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consolSink->set_level(spdlog::level::from_str(fileLevel));
//        consolSink->set_pattern("[multi_sink_example] [%^%l%$] %v");
//        consolSink->set_pattern("[thread %t] [%Y-%m-%d %H:%M:%S.%e] [%^%l%$] : %v");

        fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(filename, 2, 30);
        fileSink->set_level(spdlog::level::from_str(consolLevel));

        AddLogger(PMS_LOGGER, spdlog::level::level_enum::trace);
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }
}

void Log::AddLogger(std::string name, spdlog::level::level_enum level)
{
    std::vector<spdlog::sink_ptr> sinks {consolSink, fileSink};
    auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
    logger->set_level(level);

    logger->flush_on(spdlog::level::trace);

    spdlog::register_logger(logger);
    logger->info("Logger [{}] added", name);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [master %t] [%^%l%$] %v");
}

}
