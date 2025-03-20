#pragma once

#include <libcommon/Common.h>
#include <libcommon/GlobalConfig.h>
#include <libcommon/Logger.h>
#include <libhandler/HttpHandler.h>

#include <atomic>
#include <vector>

#include "archer_net.h"

namespace archer 
{
namespace server 
{
class ProxyServer
{
typedef struct {
    std::string host;
    int port;
} DstPeer;
typedef struct {
    int         order;
    std::string src;
    std::string dst;
} Location;
public:

    ProxyServer(std::string const& host, std::uint16_t port);
    ~ProxyServer();
    
    ProxyServer(const ProxyServer&) = delete;
    ProxyServer& operator=(const ProxyServer&) = delete;

    void addPeer(std::string const& host, int port);

    void delPeer(std::string const& host, int port);

    void startAsync();

    void close();
    
    void onRequest(HttpRequest *req, HttpResponse *res, char *chunk, size_t chunk_len);
    
    void onResponse(HttpResponse *res, char *chunk, size_t chunk_len);

    void sendNotFound(HttpRequest *req, HttpResponse *res);

    bool isActive() {return m_active;}

    void setThreads(uint16_t threadNum) {
        m_threads = threadNum;
    }

    void addLocation(int order, std::string const& src, std::string const& dst) {
        Location loc{order, src, dst};
        std::lock_guard<std::mutex> lock(m_uriMutex);
        for(int i = 0; i < m_locations.size(); i++) {
            if(src == m_locations[i].src) {
                return ;
            }
        }
        LOG_info("Proxy Server %s:%d add location %s:%s", m_host.c_str(), m_port, src.c_str(), dst.c_str());
        m_locations.push_back(loc);
        std::sort(m_locations.begin(), m_locations.end(), [](const Location& s1, const Location& s2) { return s1.order < s2.order;});
    }

    void delLocation(std::string const& src, std::string const& dst) {
        int idx = 0;
        std::lock_guard<std::mutex> lock(m_uriMutex);
        for(; idx < m_locations.size(); idx++) {
            if(src == m_locations[idx].src && dst == m_locations[idx].dst) {
                break ;
            }
        }
        LOG_info("Proxy Server %s:%d delete location %s:%s", m_host.c_str(), m_port, src.c_str(), dst.c_str());
        if(idx < m_locations.size()) {
            m_locations.erase(m_locations.begin() + idx);
        }
    }

    std::string& getHost() {
        return m_host;
    }

    int getPort() {
        return m_port;
    }

private:

    void sendRequsetToPeer(HttpRequest *req, HttpResponse *res, char *chunked, size_t len);

    void doStart();

    HttpManager                 *m_httpManager;
    uint16_t                     m_threads = 0;

    std::string                  m_host  = "";
    int                          m_port = 0;

    bool                         m_active = false;
    Json::Reader                 m_jsonReader;

    std::mutex                   m_peerMutex;
    int                          m_peerIdx = 0;
    std::vector<DstPeer>         m_peers;

    std::mutex                   m_uriMutex;
    std::vector<Location>        m_locations;
    // std::vector<std::string>     m_srcUris;
    // std::vector<std::string>     m_dstUris;
};
}
}