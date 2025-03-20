#include "ProxyService.h"

#include <stdio.h>
#include <sys/file.h>

using namespace archer::service;
using namespace archer::database;

void ProxyService::proxyServiceSendResponse(HttpResponse *res, const char *body) {
    proxyServiceSendResponse(res, body, strlen(body));
}

void ProxyService::proxyServiceSendResponse(HttpResponse *res, const char *body, size_t len) {
    http_response_set_status(res, 200);
    http_response_set_content_type(res, "application/json");
    http_response_send_all(res, body, len);
}


void ProxyService::listAllProxy(HttpResponse *res) {
    std::string list = DataBase::instance().listAllProxy();
    if(list.empty()) {
        const char *error = "{\"success\":false,\"error\":\"system error\"}";
        proxyServiceSendResponse(res, error, strlen(error));
        return ;
    }
    Json::Value jsonList;
    m_jsonReader.parse(list, jsonList);
    bool found;
    for(int i = 0; i < jsonList.size(); i++) {
        found = false;
        for(auto it = m_proxies.begin(); it != m_proxies.end(); it++) {
            if((*it)->getHost() == jsonList[i]["address"].asString() && (*it)->getPort() == jsonList[i]["port"].asInt()) {
                jsonList[i]["status"] = (*it)->isActive() ? "AVAILABLE":"UNAVAILABLE";
                found = true;
            }
        }
        if(!found) {
            jsonList[i]["status"] = "UNAVAILABLE";
        }
    }
    list = m_jsonWriter.write(jsonList);
    std::string body = "{\"success\":true,\"data\":" + list + "}";
    proxyServiceSendResponse(res, body.c_str(), body.length());
}

/**
 * {
 *   "id": "",
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
void ProxyService::addProxy(HttpResponse *res, Json::Value &val) {

    std::string host = val["address"].asString();
    int port = val["port"].asInt();
    ProxyServerPtr proxy = std::make_shared<server::ProxyServer>(host, port);
    m_proxies.push_back(proxy);

    Json::Value backends = val["backends"];
    for(int i = 0; i < backends.size(); i++) {
        proxy->addPeer(backends[i]["host"].asString(), backends[i]["port"].asInt());
    }

    Json::Value locations = val["locations"];
    for(int i = 0; i < locations.size(); i++) {
        proxy->addLocation(locations[i]["order"].asInt(), locations[i]["src"].asString(), locations[i]["dst"].asString());
    }

    if(val.isMember("threads") && val["threads"].isInt()) {
        proxy->setThreads(val["threads"].asInt());  
    }
    proxy->startAsync();

    std::string id = DataBase::instance().addProxy(val);
    if(id.length() != 32) {
        if(id.empty()) {
            const char *error = "{\"success\":false,\"error\":\"system error\"}";
            proxyServiceSendResponse(res, error, strlen(error));
            return ;
        }
        std::string error = "{\"success\":false,\"error\":\"" + id + "\"}";
        proxyServiceSendResponse(res, error.c_str(), error.length());
        return ;
    }
    
    std::string body = "{\"success\":true,\"data\":\"" + id + "\"}";
    proxyServiceSendResponse(res, body.c_str(), body.length());
}

/**
 * {
 *   "id": "",
 *   "address": "127.0.0.1",
 *   "port": 9607
 * }
 * 
*/
void ProxyService::delProxy(HttpResponse *res, Json::Value &val) {
    std::string list = DataBase::instance().listAllProxy();
    if(list.empty()) {
        const char *error = "{\"success\":false,\"error\":\"system error\"}";
        proxyServiceSendResponse(res, error, strlen(error));
        return ;
    }
    Json::Value listJson;
    m_jsonReader.parse(list, listJson);
    bool found = false;
    for(int i = 0; i < listJson.size(); i++) {
        if(listJson[i]["id"].asString() == val["id"].asString() && 
           listJson[i]["address"].asString() == val["address"].asString() && 
           listJson[i]["port"].asInt() == val["port"].asInt()) {

            found = true;
        }
    }
    if(!found) {
        const char *error = "{\"success\":false,\"error\":\"proxy server not found\"}";
        proxyServiceSendResponse(res, error, strlen(error));
        return ;
    }


    int idx = 0;
    for(; idx < m_proxies.size(); idx++) {
        if(m_proxies[idx]->getHost() == val["address"].asString() && m_proxies[idx]->getPort() == val["port"].asInt()) {
            break ;
        }
    }
    if(idx < m_proxies.size()) {
        m_proxies[idx]->close();
        m_proxies.erase(m_proxies.begin()+idx);
    }
    DataBase::instance().delProxy(val);

    const char *data = "{\"success\":true,\"data\":null}";
    proxyServiceSendResponse(res, data, strlen(data));
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
void ProxyService::addLocation(HttpResponse *res, Json::Value &val) {
    
    for(int i = 0; i < m_proxies.size(); i++) {
        if(m_proxies[i]->getHost() == val["address"].asString() && m_proxies[i]->getPort() == val["port"].asInt()) {
            m_proxies[i]->addLocation(val["location"]["order"].asInt(), val["location"]["src"].asString(), val["location"]["dst"].asString());
        }
    }

    std::string list = DataBase::instance().listAllProxy();
    if(list.empty()) {
        const char *error = "{\"success\":false,\"error\":\"system error\"}";
        proxyServiceSendResponse(res, error, strlen(error));
        return ;
    }
    Json::Value listJson;
    m_jsonReader.parse(list, listJson);
    for(int i = 0; i < listJson.size(); i++) {
        if(listJson[i]["id"].asString() == val["id"].asString() && 
           listJson[i]["address"].asString() == val["address"].asString() && 
           listJson[i]["port"].asInt() == val["port"].asInt()) {
            for(int j = 0; j < listJson[i]["locations"].size(); j++) {
                if(listJson[i]["locations"][j]["src"].asString() == val["location"]["src"].asString()) {

                    const char *error = "{\"success\":false,\"error\":\"duplicated location.src\"}";
                    proxyServiceSendResponse(res, error, strlen(error));
                    return ;
                }
            }
            listJson[i]["locations"].append(val["location"]);
        }
    }
    DataBase::instance().saveAllProxy(listJson);
    const char *data = "{\"success\":true,\"data\":null}";
    proxyServiceSendResponse(res, data, strlen(data));
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
void ProxyService::delLocation(HttpResponse *res, Json::Value &val) {
    
    for(int i = 0; i < m_proxies.size(); i++) {
        if(m_proxies[i]->getHost() == val["address"].asString() && m_proxies[i]->getPort() == val["port"].asInt()) {
            m_proxies[i]->delLocation(val["location"]["src"].asString(), val["location"]["dst"].asString());
        }
    }

    std::string list = DataBase::instance().listAllProxy();
    if(list.empty()) {
        const char *error = "{\"success\":false,\"error\":\"system error\"}";
        proxyServiceSendResponse(res, error, strlen(error));
        return ;
    }
    Json::Value listJson;
    m_jsonReader.parse(list, listJson);
    for(int i = 0; i < listJson.size(); i++) {
        if(listJson[i]["id"].asString() == val["id"].asString() && 
           listJson[i]["address"].asString() == val["address"].asString() && 
           listJson[i]["port"].asInt() == val["port"].asInt()) {

            int j = 0;
            for(; j < listJson[i]["locations"].size(); j++) {
                if(listJson[i]["locations"][j]["src"].asString() == val["location"]["src"].asString() && 
                   listJson[i]["locations"][j]["dst"].asString() == val["location"]["dst"].asString()) {
                    break;
                }
            }
            if(j < listJson[i]["locations"].size()) {
                listJson[i]["locations"].removeIndex(j, NULL);
                DataBase::instance().saveAllProxy(listJson);
                const char *data = "{\"success\":true,\"data\":null}";
                proxyServiceSendResponse(res, data, strlen(data));
                return ;
            }
        }
    }
    const char *error = "{\"success\":false,\"error\":\"can not found the location.src\"}";
    proxyServiceSendResponse(res, error, strlen(error));
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
void ProxyService::addBackend(HttpResponse *res, Json::Value &val) {
    
    for(int i = 0; i < m_proxies.size(); i++) {
        if(m_proxies[i]->getHost() == val["address"].asString() && m_proxies[i]->getPort() == val["port"].asInt()) {
            m_proxies[i]->addPeer(val["host"].asString(), val["port"].asInt());
        }
    }

    std::string list = DataBase::instance().listAllProxy();
    if(list.empty()) {
        const char *error = "{\"success\":false,\"error\":\"system error\"}";
        proxyServiceSendResponse(res, error, strlen(error));
        return ;
    }
    Json::Value listJson;
    m_jsonReader.parse(list, listJson);
    for(int i = 0; i < listJson.size(); i++) {
        if(listJson[i]["id"].asString() == val["id"].asString() && 
           listJson[i]["address"].asString() == val["address"].asString() && 
           listJson[i]["port"].asInt() == val["port"].asInt()) {

            for(int j = 0; j < listJson[i]["backends"].size(); j++) {
                if(listJson[i]["backends"][j]["host"].asString() == val["backend"]["host"].asString() &&
                   listJson[i]["backends"][j]["port"].asInt() == val["backend"]["port"].asInt()) {

                    const char *error = "{\"success\":false,\"error\":\"duplicated backend\"}";
                    proxyServiceSendResponse(res, error, strlen(error));
                    return ;
                }
            }
            listJson[i]["backends"].append(val["backend"]);
        }
    }
    DataBase::instance().saveAllProxy(listJson);
    const char *data = "{\"success\":true,\"data\":null}";
    proxyServiceSendResponse(res, data, strlen(data));
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
void ProxyService::delBackend(HttpResponse *res, Json::Value &val) {
    
    for(int i = 0; i < m_proxies.size(); i++) {
        if(m_proxies[i]->getHost() == val["address"].asString() && m_proxies[i]->getPort() == val["port"].asInt()) {
            m_proxies[i]->delPeer(val["host"].asString(), val["port"].asInt());
        }
    }

    std::string list = DataBase::instance().listAllProxy();
    if(list.empty()) {
        const char *error = "{\"success\":false,\"error\":\"system error\"}";
        proxyServiceSendResponse(res, error, strlen(error));
        return ;
    }
    Json::Value listJson;
    m_jsonReader.parse(list, listJson);
    for(int i = 0; i < listJson.size(); i++) {
        if(listJson[i]["id"].asString() == val["id"].asString() && 
           listJson[i]["address"].asString() == val["address"].asString() && 
           listJson[i]["port"].asInt() == val["port"].asInt()) {


            int j = 0;
            for(; j < listJson[i]["backends"].size(); j++) {
                if(listJson[i]["backends"][j]["host"].asString() == val["backends"]["host"].asString() && 
                   listJson[i]["backends"][j]["port"].asInt() == val["backends"]["port"].asInt()) {
                    break;
                }
            }
            if(j < listJson[i]["backends"].size()) {
                listJson[i]["backends"].removeIndex(j, NULL);
                DataBase::instance().saveAllProxy(listJson);
                const char *data = "{\"success\":true,\"data\":null}";
                proxyServiceSendResponse(res, data, strlen(data));
                return ;
            }
        }
    }
    const char *error = "{\"success\":false,\"error\":\"can not found the backends\"}";
    proxyServiceSendResponse(res, error, strlen(error));
}



void ProxyService::initLoad() {

    std::string list = DataBase::instance().listAllProxy();
    if(list.empty()) {
        console_error("Proxy Service can not load init data from database, Exit(0)");
        exit(0);
        return;
    }
    
    Json::Value listJson, val;
    m_jsonReader.parse(list, listJson);
    for(int i = 0; i < listJson.size(); i++) {
        val = listJson[i];
        std::string host = val["address"].asString();
        int port = val["port"].asInt();
        ProxyServerPtr proxy = std::make_shared<server::ProxyServer>(host, port);
        m_proxies.push_back(proxy);

        Json::Value backends = val["backends"];
        for(int i = 0; i < backends.size(); i++) {
            proxy->addPeer(backends[i]["host"].asString(), backends[i]["port"].asInt());
        }

        Json::Value locations = val["locations"];
        for(int i = 0; i < locations.size(); i++) {
            proxy->addLocation(locations[i]["order"].asInt(), locations[i]["src"].asString(), locations[i]["dst"].asString());
        }

        if(val.isMember("threads") && val["threads"].isInt()) {
            proxy->setThreads(val["threads"].asInt());  
        }
        proxy->startAsync();
    }
}