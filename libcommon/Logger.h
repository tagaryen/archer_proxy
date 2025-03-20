#pragma once

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <ctime>
#include <chrono>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

enum { LOG_LEVEL_NONE, LOG_LEVEL_TRACE, LOG_LEVEL_DEBUG, LOG_LEVEL_INFO, LOG_LEVEL_WARN, LOG_LEVEL_ERROR, LOG_LEVEL_FATAL };

namespace archer 
{
namespace common 
{

class Logger 
{

typedef struct {
    int lv;
    const char *fileName;
    int line;
    char *logMsg;
    size_t msgLen;
} LogEvent;

public:

    static Logger& getDefault() {
        static Logger instance("logs", "log", LOG_LEVEL_INFO);
        return instance;
    }

    Logger(std::string const& logPath, std::string const& logFilePath, const int logLevel);
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    
    void log(const int lv, const char *fileName, const int line, const char *fmt, ...);
    void console(const int lv, const char *fmt, ...);

    void setLevel(const int lv) {
        m_logLevel = lv;
    }
    void setPath(std::string const& path) {
        m_logPath = path;
    }

private:

    void logAppendThread();

    std::string                  m_logPath;
    std::string                  m_logFileName;
    int                          m_logLevel;
    std::mutex                   m_logMutex;
    std::queue<LogEvent *>       m_logEvents;
    std::condition_variable      m_cv;
    std::string                  m_curDate;
    FILE                        *m_logFile;
};
}
}

#define console_out(...)   archer::common::Logger::getDefault().console(LOG_LEVEL_INFO, __VA_ARGS__)
#define console_error(...) archer::common::Logger::getDefault().console(LOG_LEVEL_ERROR, __VA_ARGS__)

#define LOG_trace(...) archer::common::Logger::getDefault().log(LOG_LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_debug(...) archer::common::Logger::getDefault().log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_info(...)  archer::common::Logger::getDefault().log(LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_warn(...)  archer::common::Logger::getDefault().log(LOG_LEVEL_WARN, __FILE__, __LINE__,  __VA_ARGS__)
#define LOG_error(...) archer::common::Logger::getDefault().log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_fatal(...) archer::common::Logger::getDefault().log(LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)


#define LOGGER_trace(logger, ...) logger.log(LOG_LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define LOGGER_debug(logger,...)  logger.log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOGGER_info(logger, ...)  logger.log(LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOGGER_warn(logger, ...)  logger.log(LOG_LEVEL_WARN, __FILE__, __LINE__,  __VA_ARGS__)
#define LOGGER_error(logger, ...) logger.log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOGGER_fatal(logger, ...) logger.log(LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)
