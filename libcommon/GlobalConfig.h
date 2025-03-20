#pragma once

#include <fstream>
#include <json/json.h>

#include "Common.h"
// #include "Log.h"
#include "Logger.h"

namespace archer 
{
namespace common 
{

class GlobalConfig 
{
public:
    static GlobalConfig& instance() {
        static GlobalConfig instance;
        return instance;
    }

    ~GlobalConfig() {};

    GlobalConfig(const GlobalConfig&) = delete;
    GlobalConfig& operator=(const GlobalConfig&) = delete;

    void parseConfig(std::string const & configPath);
    
    std::string const& fetchCurPath()  {return m_curPath;}

    std::string const& fetchLogPath()  {return m_logPath;}
    
    uint16_t fetchLogLevel()  {return m_logLevel;}
    
    std::string const& fetchDatabasePath()  {return m_dbPath;}
    
    uint16_t fetchDatabaseReaders()  {return m_dbReaders;}
    
    uint32_t fetchDatabaseMemory()  {return m_dbMemory;}

    std::string const& fetchHttpServerAddress()  {return m_httpServerAddress;}
    
    uint16_t fetchHttpServerPort()  {return m_httpServerPort;}

private:

    GlobalConfig() {};

    std::string m_curPath;
    std::string m_logPath;
    uint16_t    m_logLevel;
    std::string m_dbPath;
    uint16_t    m_dbReaders;
    uint32_t    m_dbMemory;
    std::string m_httpServerAddress;
    uint16_t    m_httpServerPort;
    Json::Value m_root;
};
}
}