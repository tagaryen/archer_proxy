#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <libcommon/GlobalConfig.h>
#include <libapi/ProxyApi.h>
#include <libdatabase/DataBase.h>
#include <libserver/ManagerServer.h>

using namespace archer::common;
using namespace archer::database;
using namespace archer::server;
using namespace archer::service;
using namespace archer::api;



int main() {
    std::string configPath = "config.json";
    GlobalConfig::instance().parseConfig(configPath);

    DataBase::instance().init(GlobalConfig::instance().fetchDatabasePath(), 
                GlobalConfig::instance().fetchDatabaseReaders(), GlobalConfig::instance().fetchDatabaseMemory());

    ProxyService::instance().initLoad();

    ManagerServer server;
    server.registerHandler(ProxyApi::instance());
    server.listen(GlobalConfig::instance().fetchHttpServerAddress(), GlobalConfig::instance().fetchHttpServerPort());

    LOG_info("archer-proxy exit");

    return 0;
}