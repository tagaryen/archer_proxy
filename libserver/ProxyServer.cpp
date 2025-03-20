#include "ProxyServer.h"

using namespace archer::server;

inline static void sendRequestError(HttpResponse *res) {
    const char *body = "<!DOCTYPE html><html><head><title>APROXY SERVER</title></head><body><h3>APROXY SERVER INTERNAL ERROR</h3></body></html>";
    http_response_set_status(res, 500);
    http_response_set_content_type(res, "text/html");
    http_response_send_all(res, body, strlen(body));
}

static void httpRequestMessage(HttpManager *mgr, HttpRequest *req, HttpResponse *res, char *chunk, size_t chunk_len) {
    ProxyServer *proxy = static_cast<ProxyServer *>(http_manager_get_arg(mgr));
    proxy->onRequest(req, res, chunk, chunk_len);
}

static void httpResponseMessage(HttpManager *mgr, HttpResponse *res, char *chunk, size_t chunk_len) {
    ProxyServer *proxy = static_cast<ProxyServer *>(http_manager_get_arg(mgr));
    proxy->onResponse(res, chunk, chunk_len);
}

static void httpOnError(HttpRequest *req, HttpResponse *res, const char *error) {
    LOG_warn("http request error, %s", error);
    sendRequestError(res);
}

static void subChannelOnError(HttpManager *mgr, const char *host, int port, const char *error) {
    LOG_warn("peer connection %s:%d error, %s", host, port, error);
}

static void subChannelOnClose(HttpManager *mgr, const char *host, int port) {
    LOG_warn("peer connection %s:%d closed", host, port);
}


ProxyServer::ProxyServer(std::string const& host, std::uint16_t port) {
    m_host = host;
    m_port = port;
    m_httpManager = http_manager_new();
}

ProxyServer::~ProxyServer() {
    close();
    http_manager_free(m_httpManager);
}

void ProxyServer::close() {
    if(m_httpManager) {
        http_manager_close(m_httpManager);
    }
}

void ProxyServer::startAsync() {
    std::thread asyncListen(&ProxyServer::doStart, this);
    asyncListen.detach();
}

void ProxyServer::addPeer(std::string const& host, int port) {
    if(m_httpManager) {

        LOG_info("Proxy Server %s:%d add peer %s:%d", m_host.c_str(), m_port, host.c_str(), port);
        std::lock_guard<std::mutex> lock(m_peerMutex);
        for(int i = 0; i < m_peers.size(); i++) {
            if(m_peers[i].host == host && m_peers[i].port == port) {
                return ;
            }
        }
        http_manager_add_sub_connection(m_httpManager, host.c_str(), port, NULL);
        DstPeer peer{host, port};
        m_peers.push_back(peer);
    }
}

void ProxyServer::delPeer(std::string const& host, int port) {
    if(m_httpManager) {
        LOG_info("Proxy Server %s:%d delete peer %s:%d", m_host.c_str(), m_port, host.c_str(), port);
        http_manager_del_sub_connection(m_httpManager, host.c_str(), port);
        std::lock_guard<std::mutex> lock(m_peerMutex);
        int idx = 0;
        for(; idx < m_peers.size(); idx++) {
            if(m_peers[idx].host == host && m_peers[idx].port == port) {
                break;
            }
        }
        if(idx < m_peers.size()) {
            m_peers.erase(m_peers.begin() + idx);
        }
    }
}

void ProxyServer::doStart() {
    http_manager_set_threads(m_httpManager, m_threads);
    http_manager_set_arg(m_httpManager, this);
    LOG_info("Start Proxy on %s:%d", m_host.c_str(), m_port);
    m_active = true;
    if(!http_manager_listen(m_httpManager, m_host.c_str(), m_port, httpRequestMessage, httpResponseMessage, httpOnError, subChannelOnError, subChannelOnClose)) {
        const char *errstr = http_manager_get_error_str(m_httpManager);
        LOG_error("Proxy Server listen on %s:%d error, %s", m_host.c_str(), m_port, errstr);
    }
    m_active = false;
}

void ProxyServer::onRequest(HttpRequest *req, HttpResponse *res, char *chunk, size_t chunk_len) {
    std::string uri(http_request_get_uri(req)), newUri;
    LOG_trace("Proxy Server access %s", uri.c_str());
    bool found = false;
    {
        std::lock_guard<std::mutex> lock(m_uriMutex);
        for(int i = 0; i < m_locations.size(); i++) {
            if(uri.length() >= m_locations[i].src.length() && uri.substr(0, m_locations[i].src.length()) == m_locations[i].src) {
                newUri = m_locations[i].dst + uri.substr(m_locations[i].src.length(), uri.length() - m_locations[i].src.length());
                http_request_set_uri(req, newUri.c_str());
                found = true;
                break ;
            }
        }
    }
    if(found) {
        sendRequsetToPeer(req, res, chunk, chunk_len);
    } else {
        sendNotFound(req, res);
    }
}

void ProxyServer::onResponse(HttpResponse *res, char *chunk, size_t chunk_len) {
    http_response_send_some(res, chunk, chunk_len);
}

void ProxyServer::sendRequsetToPeer(HttpRequest *req, HttpResponse *res, char *chunk, size_t len) {
    if(m_peers.empty()) {
        sendNotFound(req, res);
        return ;
    }
    int idx = 0;
    {
        std::lock_guard<std::mutex> lock(m_peerMutex);
        idx = m_peerIdx++;
        if(m_peerIdx >= m_peers.size()) {
            m_peerIdx = 0;
        }
    }
    http_request_set_header(req, "Host", m_peers[idx].host.c_str());
    LOG_trace("Proxy Server send to %s:%d", m_peers[idx].host.c_str(), m_peers[idx].port);
    http_manager_write_to(m_httpManager, m_peers[idx].host.c_str(), m_peers[idx].port, req, chunk, len);
}


void ProxyServer::sendNotFound(HttpRequest *req, HttpResponse *res) {
    http_response_set_status(res, 404);
    http_response_set_content_type(res, "text/html");
    const char *body = "<!DOCTYPE html><html><head><title>APROXY SERVER</title></head><body><h3>APROXY SERVER 404 NotFound</h3></body></html>";
    http_response_send_all(res, body, strlen(body));
}


