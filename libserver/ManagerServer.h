#pragma once

#include <libcommon/Common.h>
#include <libcommon/GlobalConfig.h>
#include <libcommon/Logger.h>
#include <libcommon/Icon.h>
#include <libhandler/HttpHandler.h>

#include <vector>

#include "archer_net.h"

namespace archer 
{
namespace server 
{
class ManagerServer
{
public:

    ManagerServer();
    ~ManagerServer();
    
    ManagerServer(const ManagerServer&) = delete;
    ManagerServer& operator=(const ManagerServer&) = delete;

    void listen(std::string const& host, std::uint16_t port);
    
    void onMessage(HttpRequest *req, HttpResponse *res, char *chunk, size_t chunk_len);

    void close();

    void sendNotFound(HttpResponse *res);

    void sendRequestError(HttpResponse *res, int httpStatus, const char *msg);
    
    void registerHandler(handler::HttpHandler& handler);

private:
    void sendIcon(HttpResponse *res);

    Json::Reader                       m_jsonReader;
    HttpServer                        *m_http = NULL;

    std::unordered_map<std::string, handler::handlerFunction> m_getHandlers;
    std::unordered_map<std::string, handler::handlerFunction> m_postHandlers;
};
}
}