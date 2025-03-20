#include "DataBase.h"

using namespace archer::database;

void DataBase::init(std::string const& dbPath, unsigned int readerNum, size_t maxMemorySize) {
    openDataBase(dbPath, readerNum, maxMemorySize);
    initData();
}


void DataBase::openDataBase(std::string const& dbPath, unsigned int readerNum, size_t maxMemorySize) {
    if(doError(mdb_env_create(&m_env))) {
        console_error("Can not create lmdb file database environment. Exit(0)\n");
        LOG_error("Can not create lmdb file database environment. Exit(0)\n");
        exit(0);
    }
    if(doError(mdb_env_set_maxdbs(m_env, 4))) {
        console_error("Can not set lmdb file database max db nums. Exit(0)");
        LOG_error("Can not set lmdb file database max db nums. Exit(0)");
        exit(0);
    }
    if(doError(mdb_env_set_maxreaders(m_env, readerNum))) {
        console_error("Can not set lmdb file database readers. Exit(0)");
        LOG_error("Can not set lmdb file database readers. Exit(0)");
        exit(0);
    }
    if(doError(mdb_env_set_mapsize(m_env, maxMemorySize))) {
        console_error("Can not set lmdb file database max memory size. Exit(0)");
        LOG_error("Can not set lmdb file database max memory size. Exit(0)");
        exit(0);
    }

    archer::common::createDirectories(dbPath);
    if(doError(mdb_env_open(m_env, dbPath.c_str(), 0, 0664))) {
        console_error("Can not open lmdb file database dir '%s'. Exit(0)", dbPath.c_str());
        LOG_error("Can not open lmdb file database dir '%s'. Exit(0)", dbPath.c_str());
        exit(0);
    }

    MDB_txn *txn = NULL;
    if(doError(mdb_txn_begin(m_env, NULL, 0, &txn))) {
        console_error("Begin init file transaction failed. Exit(0)");
        LOG_error("Begin init file transaction failed. Exit(0)");
        exit(0);
    }

    if(doError(mdb_dbi_open(txn, "aproxy", MDB_CREATE, &m_dbi))) {
        console_error("Open file database failed. Exit(0)");
        LOG_error("Open file database failed. Exit(0)");
        mdb_txn_abort(txn);
        exit(0);
    }
    if(doError(mdb_txn_commit(txn))) {
        console_error("Open file database commit transaction failed. Exit(0)");
        LOG_error("Open file database commit transaction failed. Exit(0)");
        mdb_txn_abort(txn);
        exit(0);
    }

    console_out("Create lmdb file database success");
    LOG_info("Create lmdb file database success");
}

void DataBase::initData() {
    MDB_txn *txn = NULL;
    if(doError(mdb_txn_begin(m_env, NULL, 0, &txn))) {
        console_error("Begin initialize data transaction failed. Exit(0)");
        LOG_error("Begin initialize data transaction failed. Exit(0)");
        exit(0);
    }

    int rc = 0;
    MDB_val listValue;
    if((rc = mdb_get(txn, m_dbi, &m_key, &listValue))) {
        LOG_info("Database fetch rc %d, data len %d", rc, listValue.mv_size);
        if(MDB_NOTFOUND == rc || listValue.mv_size == 0) {
            listValue.mv_data = (void*)"[]";
            listValue.mv_size = 2;
            if((rc = mdb_put(txn, m_dbi, &m_key, &listValue, 0))) {
                LOG_error("Database Writting data failed, due to %s", mdb_strerror(rc));
                mdb_txn_abort(txn);
                exit(0);
            }
        } else {
            console_error("Database data initialize failed, due to %s", mdb_strerror(rc));
            LOG_error("Database data initialize failed, due to %s", mdb_strerror(rc));
            mdb_txn_abort(txn); 
            exit(0);  
        }
    }
    
    if(doError(mdb_txn_commit(txn))) {
        console_error("Database data initialize commit transaction failed. Exit(0)");
        LOG_error("Database data initialize commit transaction failed. Exit(0)");
        mdb_txn_abort(txn);
        exit(0);
    }
}

std::string DataBase::listAllProxy() {
    int rc = 0;

    MDB_txn *txn = NULL;

    LOG_info("List all proxies");

    if((rc = mdb_txn_begin(m_env, NULL, 0, &txn))) {
        LOG_error("database Begin read transaction failed, due to %s", mdb_strerror(rc));
        return "";
    }

    MDB_val listValue;
    if((rc = mdb_get(txn, m_dbi, &m_key, &listValue))) {
        LOG_error("database Readding key %s failed, due to %s", m_listKey.c_str(), mdb_strerror(rc));
        mdb_txn_abort(txn);
        return "";
    }

    if((rc = mdb_txn_commit(txn))) {
        LOG_error("database Commit transaction failed, due to %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return "";
    }
    std::string val((char *)listValue.mv_data, listValue.mv_size);
    return val;
}


bool DataBase::saveAllProxy(Json::Value &val) {
    
    int rc = 0;

    MDB_cursor *cursor = NULL;
    MDB_txn *txn;

    LOG_info("Save all proxies");

    if((rc = mdb_txn_begin(m_env, NULL, 0, &txn))) {
        LOG_error("database Begin read transaction failed, due to %s", mdb_strerror(rc));
        return false;
    }

    LOG_trace("Begin to save all proxies transaction with key = '%s'", m_listKey.c_str());

    std::string list = m_jsonWriter.write(val);
    
    MDB_val listValue;
    listValue.mv_data = (void *) list.c_str();
    listValue.mv_size = list.length();

    if((rc = mdb_put(txn, m_dbi, &m_key, &listValue, 0))) {
        LOG_error("database Readding key %s failed, due to %s", m_listKey.c_str(), mdb_strerror(rc));
        mdb_txn_abort(txn);
        return false;
    }

    if((rc = mdb_txn_commit(txn))) {
        LOG_error("database Commit transaction failed, due to %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return false;
    }
    return true;
}
/**
 * {
 *   "id": "",
 *   "address": "0.0.0.0"
 *   "port":8080,
 *   "threads": 2,
 *   "proxies": [
 *     {
 *       "protocol": "https",
 *       "host": "www.baidu.com",
 *       "port": 443
 *     }
 *   ]
 *   "locations": [
 *     {
 *       "src":"/",
 *       "dst":"/"
 *     }
 *   ]
 * }
*/
std::string DataBase::addProxy(Json::Value &val) {
    int rc = 0;

    // MDB_dbi dbi;
    MDB_txn *txn;
    
    LOG_info("Add a proxy to database");

    if((rc = mdb_txn_begin(m_env, NULL, 0, &txn))) {
        LOG_error("database Begin write transaction failed, due to %s", mdb_strerror(rc));
        return "";
    }

    MDB_val listValue;
    if((rc = mdb_get(txn, m_dbi, &m_key, &listValue))) {
        LOG_error("database Readding key %s failed, due to %s", m_listKey.c_str(), mdb_strerror(rc));
        mdb_txn_abort(txn);
        return "";
    }
    std::string listStr((char *)listValue.mv_data, listValue.mv_size);
    Json::Value list;
    if(!m_jsonReader.parse(listStr, list)) {
        LOG_error("database JSON parse key %s value %s failed", m_listKey.c_str(), listStr);
        return "";
    }
    for(int i = 0; i < list.size(); i++) {
        if(val["port"].asInt() ==  list[i]["port"].asInt()) {
            return "duplicated port " + val["port"].asString();
        }
    }

    std::string key = archer::common::randomString();
    val["id"] = key;
    list.append(val);
    std::string jsonVal = m_jsonWriter.write(list);

    LOG_trace("Begin to save proxy write transaction, Key = %s", key.c_str());

    MDB_val dbValue;
    dbValue.mv_size = jsonVal.length();
    dbValue.mv_data = (void *)jsonVal.c_str();

    if((rc = mdb_put(txn, m_dbi, &m_key, &dbValue, 0))) {
        LOG_error("database Writting data failed, due to %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return "";
    }
    if((rc = mdb_txn_commit(txn))) {
        LOG_error("database Commit transaction failed, due to %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return "";
    }
    return key;
}
/**
 * {
 *   "id": "",
 *   "address": "127.0.0.1",
 *   "port": 9607
 * }
 * 
*/
bool DataBase::delProxy(Json::Value &val) {
    int rc = 0;
    MDB_txn *txn;
    
    LOG_info("Delete a proxy");

    if((rc = mdb_txn_begin(m_env, NULL, 0, &txn))) {
        LOG_error("database Begin write transaction failed, due to %s", mdb_strerror(rc));
        return false;
    }

    MDB_val listValue;
    if((rc = mdb_get(txn, m_dbi, &m_key, &listValue))) {
        LOG_error("database Readding key %s failed, due to %s", m_listKey.c_str(), mdb_strerror(rc));
        mdb_txn_abort(txn);
        return false;
    }
    std::string listStr((char *)listValue.mv_data, listValue.mv_size);
    Json::Value list;
    if(!m_jsonReader.parse(listStr, list)) {
        LOG_error("database JSON parse key %s value %s failed", m_listKey.c_str(), listStr);
        return false;
    }
    int idx = 0;
    for(; idx < list.size(); idx++) {
        if(val["id"].asString() ==  list[idx]["id"].asString()) {
            break ;
        }
    }
    if(idx < list.size()) {
        list.removeIndex(idx, NULL);
    }
    
    LOG_trace("Begin to save deleted proxy write transaction, Key = %s", val["id"].asString().c_str());

    std::string jsonVal = m_jsonWriter.write(list);
    MDB_val dbValue;
    dbValue.mv_size = jsonVal.length();
    dbValue.mv_data = (void *)jsonVal.c_str();

    if((rc = mdb_put(txn, m_dbi, &m_key, &dbValue, 0))) {
        LOG_error("database Writting data failed, due to %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return false;
    }
    if((rc = mdb_txn_commit(txn))) {
        LOG_error("database Commit transaction failed, due to %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return false;
    }
    return true;
}

