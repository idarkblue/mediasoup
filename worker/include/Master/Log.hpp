#ifndef _LOG_HPP_INCLUDE_
#define _LOG_HPP_INCLUDE_

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
//#include "spdlog/sinks/rotating_file_sink.h"
//#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"

#define PMS_LOGGER "pms"
#define PWS_LOGGER "pms"

#ifdef MS_LOG_FILE_LINE
    #define _PMS_LOG_STR "{}:{} | {}::{}()"
    #define _PMS_LOG_STR_DESC _PMS_LOG_STR " | "
    #define _PMS_FILE (std::strchr(__FILE__, '/') ? std::strchr(__FILE__, '/') + 1 : __FILE__)
    #define _PMS_LOG_ARG _PMS_FILE, __LINE__, PMS_CLASS, __FUNCTION__
#else
    #define _PMS_LOG_STR "{}::{}()"
    #define _PMS_LOG_STR_DESC _PMS_LOG_STR " | "
    #define _PMS_LOG_ARG PMS_CLASS, __FUNCTION__
#endif

namespace pingos {

class Log {

public:
    static void ClassInit(std::string filename,
        spdlog::level::level_enum consolLevel = spdlog::level::level_enum::info,
        spdlog::level::level_enum fileLevel = spdlog::level::level_enum::info);

    static void AddLogger(std::string name, spdlog::level::level_enum level);

    static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> consolSink;
    static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> fileSink;
};

#define PMS_TRACE(desc, ...) \
    spdlog::get(PMS_LOGGER)->trace( _PMS_LOG_STR_DESC desc, _PMS_LOG_ARG, ##__VA_ARGS__);
#define PMS_DEBUG(desc, ...) \
    spdlog::get(PMS_LOGGER)->debug( _PMS_LOG_STR_DESC desc, _PMS_LOG_ARG, ##__VA_ARGS__);
#define PMS_INFO(desc, ...) \
    spdlog::get(PMS_LOGGER)->info( _PMS_LOG_STR_DESC desc, _PMS_LOG_ARG, ##__VA_ARGS__);
#define PMS_WARN(desc, ...) \
    spdlog::get(PMS_LOGGER)->warn( _PMS_LOG_STR_DESC desc, _PMS_LOG_ARG, ##__VA_ARGS__);
#define PMS_ERROR(desc, ...) \
    spdlog::get(PMS_LOGGER)->error( _PMS_LOG_STR_DESC desc, _PMS_LOG_ARG, ##__VA_ARGS__);

#define PWS_TRACE(desc, ...) \
    spdlog::get(PWS_LOGGER)->trace(desc, ##__VA_ARGS__);
#define PWS_DEBUG(desc, ...) \
    spdlog::get(PWS_LOGGER)->debug(desc, ##__VA_ARGS__);
#define PWS_INFO(desc, ...) \
    spdlog::get(PWS_LOGGER)->info(desc, ##__VA_ARGS__);
#define PWS_WARN(desc, ...) \
    spdlog::get(PWS_LOGGER)->warn(desc, ##__VA_ARGS__);
#define PWS_ERROR(desc, ...) \
    spdlog::get(PWS_LOGGER)->error(desc, ##__VA_ARGS__);

}

#endif
