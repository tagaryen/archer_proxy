#pragma once

#include "archer_net.h"

#include <libcommon/Common.h>
#include <libcommon/GlobalConfig.h>
#include <libdatabase/DataBase.h>
#include <libserver/ProxyServer.h>

namespace archer 
{
namespace service 
{
class ProxyService
{

typedef std::shared_ptr<server::ProxyServer> ProxyServerPtr;

public:

    static ProxyService& instance() {
        static ProxyService instance;
        return instance;
    }

    ProxyService(const ProxyService&) = delete;
    ProxyService& operator=(const ProxyService&) = delete;
    
    ~ProxyService() {}

    void listAllProxy(HttpResponse *res);

    void addProxy(HttpResponse *res, Json::Value &val);

    void delProxy(HttpResponse *res, Json::Value &val);
    
    void addLocation(HttpResponse *res, Json::Value &val);

    void delLocation(HttpResponse *res, Json::Value &val);
    
    void addBackend(HttpResponse *res, Json::Value &val);
    
    void delBackend(HttpResponse *res, Json::Value &val);

    void proxyServiceSendResponse(HttpResponse *res, const char *body, size_t len);

    void proxyServiceSendResponse(HttpResponse *res, const char *body);

    void initLoad();
    
private:

    ProxyService() {}

    Json::FastWriter                    m_jsonWriter;
    Json::Reader                        m_jsonReader;
    std::vector<ProxyServerPtr>         m_proxies;
};
}
}