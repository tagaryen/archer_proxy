#include "ProxyApi.h"

using namespace archer::api;
using namespace archer::service;

std::unordered_map<std::string, archer::handler::handlerFunction> ProxyApi::getHandlerFunctions()  {
    std::unordered_map<std::string, archer::handler::handlerFunction> retMap;
    retMap["/aproxy/list"] = std::bind(&ProxyApi::listAllProxy, this, std::placeholders::_1, std::placeholders::_2); 
    return retMap;
}

std::unordered_map<std::string, archer::handler::handlerFunction> ProxyApi::postHandlerFunctions() {
    std::unordered_map<std::string, archer::handler::handlerFunction> retMap;
    retMap["/aproxy/add"] = std::bind(&ProxyApi::addProxy, this, std::placeholders::_1, std::placeholders::_2); 
    retMap["/aproxy/delete"] = std::bind(&ProxyApi::delProxy, this, std::placeholders::_1, std::placeholders::_2); 
    retMap["/aproxy/location/add"] = std::bind(&ProxyApi::addLocation, this, std::placeholders::_1, std::placeholders::_2); 
    retMap["/aproxy/location/delete"] = std::bind(&ProxyApi::delLocation, this, std::placeholders::_1, std::placeholders::_2); 
    retMap["/aproxy/backend/add"] = std::bind(&ProxyApi::addBackend, this, std::placeholders::_1, std::placeholders::_2); 
    retMap["/aproxy/backend/delete"] = std::bind(&ProxyApi::delBackend, this, std::placeholders::_1, std::placeholders::_2); 
    return retMap;
}

void ProxyApi::listAllProxy(HttpResponse *res, Json::Value &val) {
    ProxyService::instance().listAllProxy(res);
}

/**
 * {
 *   "address": "0.0.0.0"
 *   "port":8080,
 *   "threads": 2,
 *   "backends": [
 *     {
 *       "protocol": "https",
 *       "host": "www.baidu.com",
 *       "port": 443
 *     }
 *   ]
 *   "locations": [
 *     {
 *       "order": 0
 *       "src":"/",
 *       "dst":"/"
 *     }
 *   ]
 * }
*/
void ProxyApi::addProxy(HttpResponse *res, Json::Value &val) {
    if(!baseCheck(res, val)) {
        return ;
    }
    if(!val.isMember("backends") || !val["backends"].isArray()) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"backends is require and must be an array\"}");
        return ;
    } else {
        for(int i = 0; i < val["backends"].size(); i++) {
            if(!backendCheck(res, val["backends"][i])) {
                return ;
            }
        }
    }

    if(!val.isMember("locations") || !val["locations"].isArray()) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"locations is require and must be an array\"}");
        return ;
    } else {
        for(int i = 0; i < val["locations"].size(); i++) {
            if(!locationCheck(res, val["locations"][i])) {
                return ;
            }
        }
    }
    ProxyService::instance().addProxy(res, val);
}

/**
 * {
 *   "id": "",
 *   "address": "127.0.0.1",
 *   "port": 9607
 * }
 * 
*/
void ProxyApi::delProxy(HttpResponse *res, Json::Value &val) {
    if(!val.isMember("id") || !val["id"].isString()) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"id is require and must be a string\"}");
        return ;
    }
    if(!baseCheck(res, val)) {
        return ;
    }
    ProxyService::instance().delProxy(res, val);
}

/**
 * {
 *   "id": "",
 *   "address": "127.0.0.1",
 *   "port": 9607,
 *   "location": {
 *     "order": 0,
 *     "src": "",
 *     "dst": ""
 *   }
 * }
 * 
*/
void ProxyApi::addLocation(HttpResponse *res, Json::Value &val) {
    if(!val.isMember("id") || !val["id"].isString()) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"id is require and must be a string\"}");
        return ;
    }
    if(!baseCheck(res, val)) {
        return ;
    }
    if(!val.isMember("location")) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"location is require and must be an object\"}");
        return ;
    }
    if(!locationCheck(res, val["location"])) {
        return ;
    }
    ProxyService::instance().addLocation(res, val);
}

/**
 * {
 *   "id": "",
 *   "address": "127.0.0.1",
 *   "port": 9607,
 *   "location": {
 *     "order": 0,
 *     "src": "",
 *     "dst": ""
 *   }
 * }
 * 
*/
void ProxyApi::delLocation(HttpResponse *res, Json::Value &val) {
    if(!val.isMember("id") || !val["id"].isString()) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"id is require and must be a string\"}");
        return ;
    }
    if(!baseCheck(res, val)) {
        return ;
    }
    if(!val.isMember("location")) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"location is require and must be an object\"}");
        return ;
    }
    if(!locationCheck(res, val["location"])) {
        return ;
    }
    ProxyService::instance().delLocation(res, val);
}
/**
 * {
 *   "id": "",
 *   "address": "127.0.0.1",
 *   "port": 9607,
 *   "backend": {
 *     "protocol": "https",
 *     "host": "www.baidu.com",
 *     "port": 443
 *   }
 * }
 * 
*/
void ProxyApi::addBackend(HttpResponse *res, Json::Value &val) {
    if(!val.isMember("id") || !val["id"].isString()) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"id is require and must be a string\"}");
        return ;
    }
    if(!baseCheck(res, val)) {
        return ;
    }
    if(!val.isMember("backend")) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"backend is require and must be an object\"}");
        return ;
    }
    if(!backendCheck(res, val["backend"])) {
        return ;
    }
    ProxyService::instance().addLocation(res, val);
}
/**
 * {
 *   "id": "",
 *   "address": "127.0.0.1",
 *   "port": 9607,
 *   "backend": {
 *     "host": "www.baidu.com",
 *     "port": 443
 *   }
 * }
 * 
*/
void ProxyApi::delBackend(HttpResponse *res, Json::Value &val) {
    if(!val.isMember("id") || !val["id"].isString()) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"id is require and must be a string\"}");
        return ;
    }
    if(!baseCheck(res, val)) {
        return ;
    }
    if(!val.isMember("backend")) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"backend is require and must be an object\"}");
        return ;
    }
    if(!backendCheck(res, val["backend"])) {
        return ;
    }
    ProxyService::instance().delLocation(res, val);
}

bool ProxyApi::baseCheck(HttpResponse *res, Json::Value &val) {
    if(!val.isMember("address") || !val["address"].isString()) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"address is require and must be a string\"}");
        return false;
    }
    
    if(!val.isMember("port") || !val["port"].isInt()) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"port is require and must be a int\"}");
        return false;
    }

    return true;
}


bool ProxyApi::backendCheck(HttpResponse *res, Json::Value &val) {
    if(!val.isMember("host") || !val["host"].isString()) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"backend item host is require and must be a string\"}");
        return false;
    }
    if(!val.isMember("port") || !val["port"].isInt()) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"backend item port is require and must be a int\"}");
        return false;
    }
    return true;
}

bool ProxyApi::locationCheck(HttpResponse *res, Json::Value &val) {
    if(!val.isMember("src") || !val["src"].isString()) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"location item src is require and must be a string\"}");
        return false;
    }
    if(!val.isMember("dst") || !val["dst"].isString()) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"location dst host is require and must be a string\"}");
        return false;
    }
    if(!val.isMember("order") || !val["order"].isInt()) {
        ProxyService::instance().proxyServiceSendResponse(res, "{\"success\":false,\"error\":\"location item order is require and must be a int\"}");
        return false;
    }
    return true;
}