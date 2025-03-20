
#pragma once

#include <libcommon/Common.h>
#include <libcommon/Logger.h>
#include <libhandler/HttpHandler.h>
#include <libservice/ProxyService.h>

#include "archer_net.h"

namespace archer 
{
namespace api 
{
class ProxyApi : public handler::HttpHandler
{
public:

    static ProxyApi& instance() {
        static ProxyApi instance;
        return instance;
    }
    
    ~ProxyApi() {}

    ProxyApi(const ProxyApi&) = delete;
    ProxyApi& operator=(const ProxyApi&) = delete;

    std::unordered_map<std::string, handler::handlerFunction> getHandlerFunctions() override;
    
    std::unordered_map<std::string, handler::handlerFunction> postHandlerFunctions() override;

    void listAllProxy(HttpResponse *res, Json::Value &val);

    void addProxy(HttpResponse *res, Json::Value &val);

    void delProxy(HttpResponse *res, Json::Value &val);
    
    void addLocation(HttpResponse *res, Json::Value &val);

    void delLocation(HttpResponse *res, Json::Value &val);
    
    void addBackend(HttpResponse *res, Json::Value &val);
    
    void delBackend(HttpResponse *res, Json::Value &val);

    bool baseCheck(HttpResponse *res, Json::Value &val);

    bool backendCheck(HttpResponse *res, Json::Value &val);

    bool locationCheck(HttpResponse *res, Json::Value &val);

private:
    
    ProxyApi() {}
};
}
}