#include "ManagerServer.h"

using namespace archer::server;

static std::function<void(HttpRequest *req, HttpResponse *res, char *chunk, size_t chunk_len)> proxyServerOnRequestCallback;

static void httpChunkedMessage(HttpRequest *req, HttpResponse *res, char *chunk, size_t chunk_len) {
    proxyServerOnRequestCallback(req, res, chunk, chunk_len);
}

ManagerServer::ManagerServer() {}

ManagerServer::~ManagerServer() {
    close();
}

void ManagerServer::listen(std::string const& host, std::uint16_t port) {

    m_http = http_server_new();
    proxyServerOnRequestCallback = std::bind(&ManagerServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    http_server_set_message_handler(m_http, httpChunkedMessage);

    console_out("Manager Server listenning on %s:%d", host.c_str(), port);
    LOG_info("Manager Server listenning on %s:%d", host.c_str(), port);
    if(!http_server_listen(m_http, host.c_str(), port)) {
        const char *errstr = http_server_get_errstr(m_http);
        console_error("Manager Server listen on %s:%d error, %s", host.c_str(), port, errstr);
        LOG_error("Manager Server listen on %s:%d error, %s", host.c_str(), port, errstr);
    }
    http_server_free(m_http);
    m_http = NULL;
}


void ManagerServer::onMessage(HttpRequest *req, HttpResponse *res, char *chunk, size_t chunk_len) {
    if(!http_request_is_finished(req)) {
        sendRequestError(res, 413, "{\"success\":false,\"error\":\"413 Body Too Large\"}");
        return ;
    }
    std::string uri(http_request_get_uri(req));
    LOG_info("Manager Server access %s", uri.c_str());
    if(uri == "/favicon.ico") {
        sendIcon(res);
        return ;
    }
    Json::Value val;
    if(chunk_len > 0 && !m_jsonReader.parse(std::string(chunk, chunk_len), val)) {
        sendRequestError(res, 400, "{\"success\":false,\"error\":\"400 Body Not A Valid JSON\"}");
        return ;
    }
    std::string method(http_request_get_method(req));
    if("GET" == method) {
        auto it = m_getHandlers.find(uri);
        if(it == m_getHandlers.end()) {
            sendNotFound(res);
        } else {
            m_getHandlers[uri](res, val);
        }
        return ;
    }
    if("POST" == method) {
        auto it = m_postHandlers.find(uri);
        if(it == m_postHandlers.end()) {
            sendNotFound(res);
        } else {
            m_postHandlers[uri](res, val);
        }
        return ;
    }
    sendNotFound(res);
}


void ManagerServer::registerHandler(handler::HttpHandler& handler) {
    std::unordered_map<std::string, archer::handler::handlerFunction> getHandlers = handler.getHandlerFunctions();
    for(auto it = getHandlers.begin(); it != getHandlers.end(); it++) {
        m_getHandlers[it->first] = it->second;
    }
    std::unordered_map<std::string, archer::handler::handlerFunction> postHandlers = handler.postHandlerFunctions();
    for(auto it = postHandlers.begin(); it != postHandlers.end(); it++) {
        m_postHandlers[it->first] = it->second;
    }
}

void ManagerServer::close() {
    if(m_http) {
        http_server_close(m_http);
    }
}


void ManagerServer::sendNotFound(HttpResponse *res) {
    http_response_set_status(res, 404);
    http_response_set_content_type(res, "application/json");
    const char *body = "{\"success\":false,\"error\":\"404 NotFound\"}";
    http_response_send_all(res, body, strlen(body));
}

void ManagerServer::sendRequestError(HttpResponse *res, int httpStatus, const char *msg) {
    http_response_set_status(res, httpStatus);
    http_response_set_content_type(res, "application/json");
    http_response_send_all(res, msg, strlen(msg));
}

void ManagerServer::sendIcon(HttpResponse *res) {
    http_response_set_status(res, 200);
    http_response_set_content_type(res, "image/icon");
    const char *body = common::ICON_SRC;
    size_t len = common::ICON_SRC_LEN;
    http_response_send_all(res, body, len);
}