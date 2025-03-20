#include "Logger.h"

using namespace archer::common;

static const char *levelFormat[] = {
  "[NONE]", "[TRACE]", "[DEBUG]", "[INFO]", "[WARN]", "[ERROR]", "[FATAL]"
};

static bool logThreadRuning = false;

inline static std::string simplifyFilename(const char *filename) {
    size_t len = strlen(filename);
    long off = len - 1;
    while(off >= 0 && filename[off] != 47 && filename[off] != 92) {
        --off;
    }
    char *new_name =  (char *)malloc(len - off);
    memcpy(new_name, filename + off + 1, len - off - 1);
    new_name[len - off - 1] = '\0';
    return new_name;
}

inline static std::string formatNowTime() {
    char buffer[20];
    time_t t = time(0);
    struct tm* timeinfo = localtime(&t);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}

inline static std::string formatNowDate() {
    char buffer[20];
    time_t t = time(0);
    struct tm* timeinfo = localtime(&t);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
    return std::string(buffer);
}

inline static void doMkdir(const char *path) {
    if(access(path, 0)) {
#ifdef _WIN32
        mkdir(path);
#else
        mkdir(path, S_IRWXU);
#endif
    }
}

inline static void doMkdirs(std::string &path) {
    size_t len = path.length();
    char dir[1024];
    memcpy(dir, path.c_str(), len);
    dir[len] = 0;
    for(int i = 1; i < len; i++) {
        if(dir[i] == '/' || dir[i] == '\\') {
            dir[i] = 0;
            doMkdir(dir);
            dir[i] = '/';
        }
    }
    doMkdir(dir);
}

inline static bool isAbsolutePath(const char *path) {
#ifdef __WIN32
    return (isalpha(path[0]) && path[1] == ':') || (path[0] == 92 && path[1] == 92);
#else
    return (path[0] == '/');
#endif
}

Logger::Logger(std::string const& logPath, std::string const& logFileName, const int logLevel) {
    m_logPath = logPath;
    m_logLevel = logLevel;
    m_logFileName = logFileName;
    m_logFile = NULL;
    
    size_t pathLen = m_logPath.length();
    if(!isAbsolutePath(m_logPath.c_str())) {
        char rootPathChars[512] = {0};
        if(getcwd(rootPathChars, 512)) {}
        std::string rootPath(rootPathChars);
        m_logPath = rootPath + '/' + m_logPath;
    }
    if(m_logPath[pathLen - 1] == '\\' || m_logPath[pathLen - 1] == '/') {
        m_logPath.pop_back();
    }
    doMkdirs(m_logPath);

    if(!logThreadRuning) {
        std::lock_guard<std::mutex> lock(m_logMutex);
        if(!logThreadRuning) {
            logThreadRuning = true;
            std::thread run(&Logger::logAppendThread, this);
            run.detach();
        }
    }
}

Logger::~Logger() {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_cv.notify_one();
}

void Logger::log(const int lv, const char *fileName, const int line, const char *fmt, ...) {
    if(lv < m_logLevel || lv > LOG_LEVEL_FATAL) {
        return ;
    }
    
    LogEvent *event = (LogEvent *) malloc(sizeof(LogEvent));
    event->lv = lv;
    event->fileName = fileName;
    event->line = line;

    va_list args;
    va_start(args, fmt);

    event->msgLen = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    event->logMsg = (char*)malloc(event->msgLen);
    va_start(args, fmt);
    vsnprintf(event->logMsg, event->msgLen + 1, fmt, args);
    va_end(args);

    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        m_logEvents.push(event);
        m_cv.notify_one();
    }
}

void Logger::console(const int lv, const char *fmt, ...) {
    if(lv < m_logLevel || lv > LOG_LEVEL_FATAL) {
        return ;
    }
    
    std::string color;
    if(lv >= LOG_LEVEL_WARN) {
        color = "\033[31m";
    }
    std::string level(levelFormat[lv]);
    std::string time = formatNowTime();
    std::string meta = color + level + ' ' + time + ' ';
    
    fprintf(stdout, "%s", meta.c_str());
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
    if(lv >= LOG_LEVEL_WARN) {
        fprintf(stdout, "%s", "\033[0m");
    }
    fflush(stdout);
}

void Logger::logAppendThread() {
    while(logThreadRuning) {
        LogEvent *event = NULL;
        {
            std::unique_lock<std::mutex> lock(m_logMutex);
            if(m_logEvents.empty()) {
                m_cv.wait(lock);
                continue;
            }
            event = m_logEvents.front();
            m_logEvents.pop();

            std::string today = formatNowDate();
            if(m_curDate != today) {
                if(m_logFile) {
                    fclose(m_logFile);
                }
                m_curDate = today;
                std::string filePath = m_logPath + '/' + m_logFileName + '-' + today + ".log";
                m_logFile = fopen(filePath.c_str(), "ab");
            }
        }
        if(event == NULL) {
            continue;
        }
        std::string level(levelFormat[event->lv]);
        std::string time = formatNowTime();
        std::string fileName = simplifyFilename(event->fileName);
        std::string logMsg(event->logMsg, event->msgLen);
        logMsg = level + ' ' + time + ' ' + fileName + ':' + std::to_string(event->line) + ' ' + logMsg + '\n';

        fprintf(m_logFile, logMsg.c_str());
        fflush(m_logFile);

        free(event->logMsg);
        free(event);
    }
}