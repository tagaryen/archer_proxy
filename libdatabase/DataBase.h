#pragma once

#include <libcommon/Common.h>
#include <libcommon/GlobalConfig.h>
#include <libcommon/Logger.h>
#include "lmdb.h"

#include <mutex>
#include <array>
#include <vector>

namespace archer 
{
namespace database 
{
class DataBase 
{

public:

    static DataBase& instance() {
        static DataBase instance;
        return instance;
    }


    DataBase(const DataBase&) = delete;
    DataBase& operator=(const DataBase&) = delete;
    
    ~DataBase() {
        mdb_env_close(m_env);
        free(m_key.mv_data);
    }

    void init(std::string const& dbPath, unsigned int readerNum, size_t maxMemorySize);

    std::string listAllProxy();
    
    bool saveAllProxy(Json::Value &val);

    std::string addProxy(Json::Value &val);

    bool delProxy(Json::Value &val);

private:
    DataBase() {
        m_listKey = "archer-proxy-list-bxzchjahcadcds";
        m_key.mv_data = malloc(33);
        memcpy(m_key.mv_data, m_listKey.c_str(), 32);
        m_key.mv_size = m_listKey.length();
    }

    bool doError(int rc) {
        if(rc) {
            char *msg = mdb_strerror(rc);
            console_error("%s", msg);
            LOG_error("%s", msg);
        }
        return rc;
    }

    
    void openDataBase(std::string const& dbPath, unsigned int readerNum, size_t maxMemorySize);
    
    void initData();

    Json::Reader     m_jsonReader;
    Json::FastWriter m_jsonWriter;
    std::string      m_listKey;
    
    MDB_val          m_key;
    MDB_env         *m_env;
    MDB_dbi          m_dbi;
};
}
}
