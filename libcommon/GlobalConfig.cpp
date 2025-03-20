#include "GlobalConfig.h"

using namespace archer::common;

// static GlobalConfig *globalConfigInstance = static_cast<GlobalConfig *>(std::malloc(sizeof(GlobalConfig)));


void GlobalConfig::parseConfig(std::string const & configPath) {
    m_curPath = archer::common::getCurrentPath();
    std::string abConfigPath = configPath;
    if(!archer::common::isAbsolutePath(configPath)) {
        abConfigPath = m_curPath + "/" + configPath;
    }
    if(!archer::common::fileExists(abConfigPath)) {
        console_out("Can not read %s, file not exists, using default configs", abConfigPath.c_str());
        m_logPath = "logs";
        m_logLevel = LOG_LEVEL_INFO;
        m_httpServerAddress = "127.0.0.1";
        m_httpServerPort = 9617;
        m_dbPath = "/opt/archer-file/database/";
        m_dbReaders = 1;
        m_dbMemory = 1024 * 1024 * 8;
        
        if(!archer::common::fileExists(m_dbPath)) {
            archer::common::createDirectories(m_dbPath);
        }
        
        console_out("Log path = %s", m_logPath.c_str());
        console_out("Log level = INFO");
        console_out("Database path = %s", m_dbPath.c_str());
        console_out("Database memory size = %u", m_dbMemory);
        console_out("HTTP Server host = %s", m_httpServerAddress.c_str());
        console_out("HTTP Server port = %d", m_httpServerPort);

        return ;
    }
    
    console_out("Using config file %s", configPath.c_str());

    std::ifstream file(configPath);
    if(!file.is_open()) {
        console_error("Can not open file %s", configPath.c_str());
        exit(0);
    }
    m_root = Json::Value(Json::stringValue);
    try {
        file >> m_root;
    } catch(std::exception& ex) {
        console_error("Parse json file %s failed, due to %s", configPath.c_str(), ex.what());
        file.close();
        exit(0);
    }
    file.close();

    console_out("Parse log configs");
    std::string level;
    if(m_root.isMember("log")) {
        if(m_root["log"].isMember("path")) {
            m_logPath = std::string(m_root["log"]["path"].asCString());
        } else {
            m_logPath = "logs";
        }

        if(m_root["log"].isMember("level")) {
            level = std::string(m_root["log"]["level"].asCString());
            if(level == "TRACE") {
                m_logLevel = LOG_LEVEL_TRACE;
            } else if(level == "DEBUG") {
                m_logLevel = LOG_LEVEL_DEBUG;
            } else if(level == "INFO") {
                m_logLevel = LOG_LEVEL_INFO;
            } else if(level == "WARN") {
                m_logLevel = LOG_LEVEL_WARN;
            } else if(level == "ERROR") {
                m_logLevel = LOG_LEVEL_ERROR;
            } else if(level == "FATAL") {
                m_logLevel = LOG_LEVEL_FATAL;
            } else {
                level = "INFO";
                m_logLevel = LOG_LEVEL_INFO;
            }
        } else {
            m_logLevel = LOG_LEVEL_INFO;
        }
    } else {
        m_logPath = "logs";
        m_logLevel = LOG_LEVEL_INFO;
    }

    console_out("Log path = %s", m_logPath.c_str());
    console_out("Log level = %s", level.c_str());
    
    Logger::getDefault().setPath(m_logPath);
    Logger::getDefault().setLevel(m_logLevel);

    console_out("Parse database configs");
    if(m_root.isMember("database") && m_root["database"].isMember("path")) {
        m_dbPath = std::string(m_root["database"]["path"].asCString());
        if(!archer::common::isAbsolutePath(m_dbPath)) {
            if(m_dbPath[0] == '/' || m_dbPath[0] == '\\') {
                m_dbPath = m_curPath + m_dbPath;
            } else {
                m_dbPath = m_curPath + "/" + m_dbPath;
            }
            if(m_dbPath[m_dbPath.length()-1] != '/' && m_dbPath[m_dbPath.length()-1] != '\\') {
                m_dbPath += '/';
            }
        }
    } else {
        m_dbPath = m_curPath + "/database/";
    }
    m_dbReaders = 4;
    m_dbMemory = 1024 * 1024 * 8;

    console_out("Database path = %s", m_dbPath.c_str());
    console_out("Database readers = %d", m_dbReaders);
    console_out("Database memory size = %u", m_dbMemory);

    console_out("Parse http server configs");
    if(m_root.isMember("http") && m_root["http"].isMember("host")) {
        m_httpServerAddress = std::string(m_root["http"]["host"].asCString());
    } else {
        m_httpServerAddress = "127.0.0.1";
    }
    if(m_root.isMember("http") && m_root["http"].isMember("port")) {
        uint32_t port = m_root["http"]["port"].asUInt();
        if(port > 65535) {
            m_httpServerPort = 9607;
        } else {
            m_httpServerPort = port;
        }
    } else {
        m_httpServerPort = 9607;
    }
    console_out("HTTP Server host = %s", m_httpServerAddress.c_str());
    console_out("HTTP Server port = %d", m_httpServerPort);

    if(!archer::common::fileExists(m_dbPath)) {
        archer::common::createDirectories(m_dbPath);
    }
}

