#ifndef _ARCHER_NET_H_
#define _ARCHER_NET_H_

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>

#ifndef TLS1_VERSION
#define	TLS1_VERSION      0x0301
#endif

#ifndef TLS1_1_VERSION
#define	TLS1_1_VERSION    0x0302
#endif

#ifndef TLS1_2_VERSION
#define	TLS1_2_VERSION    0x0303
#endif

#ifndef TLS1_3_VERSION
#define	TLS1_3_VERSION    0x0304
#endif

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN   65
#endif


#ifdef __cplusplus
extern "C" {
#endif

struct ChannelBase_st;
struct Channel_st;
struct ServerChannel_st;
struct SSLOption_st;
struct HttpServer_st;
struct HttpRequest_st;
struct HttpResponse_st;
struct ChannelManager_st;
struct HttpManager_st;


typedef struct ChannelBase_st        ChannelBase;
typedef struct Channel_st            Channel;
typedef struct ChannelBase_st        ChannelBase;
typedef struct ServerChannel_st      ServerChannel;
typedef struct SSLOption_st          SSLOption;
typedef struct HttpServer_st         HttpServer;
typedef struct HttpRequest_st        HttpRequest;
typedef struct HttpResponse_st       HttpResponse;
typedef struct ChannelManager_st     ChannelManager;
typedef struct HttpManager_st HttpManager;


typedef struct FairLock_st FairLock;
typedef struct BaseHandler_st BaseHandler;

typedef void (*server_channel_on_accept)(Channel *channel);
typedef void (*channel_on_connect)(Channel *channel);
typedef void (*channel_on_close)(Channel *channel);
typedef void (*channel_on_read)(Channel *channel, char *data, size_t data_len);
typedef void (*channel_on_error)(Channel *channel, const char *err_msg);
typedef void (*channel_on_peer_certificate)(Channel *channel, const char *crt, const size_t crt_len);


typedef void (*channel_manager_on_connect)(ChannelManager *mgr, Channel *channel);
typedef void (*channel_manager_on_close)(ChannelManager *mgr, Channel *channel);
typedef void (*channel_manager_on_read)(ChannelManager *mgr, Channel *channel, char *data, size_t data_len);
typedef void (*channel_manager_on_error)(ChannelManager *mgr, Channel *channel, const char *err_msg);

typedef void (*http_chunked_message)(HttpRequest *req, HttpResponse *res, char *chunk, size_t chunk_len);
typedef void (*http_err_message)(HttpRequest *req, HttpResponse *res, const char *error_msg);
typedef void (*http_header_foreach_cb)(const char *key, const char *val);
typedef void (*http_manager_on_request)(HttpManager *mgr, HttpRequest *req, HttpResponse *res, char *chunk, size_t chunk_len);
typedef void (*http_manager_on_response)(HttpManager *mgr, HttpResponse *res, char *chunk, size_t chunk_len);
typedef void (*remote_on_error)(HttpManager *mgr, const char *host, int port, const char *err_msg);
typedef void (*remote_on_close)(HttpManager *mgr, const char *host, int port);

//ssl
extern SSLOption * ssl_option_new(int is_client_mode, int is_verify_peer);
extern void ssl_option_free(SSLOption *opt);
extern int ssl_option_set_version(SSLOption *opt, int max_version, int min_version);
extern int ssl_option_authonrized_hostname(SSLOption *opt, const char *hostname);
extern int ssl_option_set_named_curves(SSLOption *opt, const char *named_curves);
extern int ssl_option_set_trust_ca(SSLOption *opt, const char *ca, const size_t ca_len);
extern int ssl_option_set_certificate_and_key(SSLOption *opt, const char *crt, const size_t crt_len, const char *key, const size_t key_len);
extern int ssl_option_set_encrypt_certificate_and_key(SSLOption *opt, const char *crt, const size_t crt_len, const char *key, const size_t key_len);
extern const char * ssl_option_get_errstr(SSLOption * opt);


extern ChannelBase * channel_base_new();
extern void channel_base_stop(ChannelBase *base);
extern void channel_base_free(ChannelBase *base);
extern void channel_base_start_event_loop(ChannelBase *base);

// channel 
extern Channel * channel_new(void);
extern void channel_close(Channel *channel);
extern void channel_free(Channel *channel);
extern const char * channel_get_host(Channel *channel);
extern int channel_get_port(Channel *channel);
extern const char * channel_get_errstr(Channel *channel);
extern void * channel_get_arg(Channel *channel);
extern SSLOption * channel_get_ssl_option(Channel *channel);
extern int channel_is_client_side(Channel *channel);

extern void channel_set_channel_on_connect(Channel *channel, channel_on_connect);
extern void channel_set_channel_on_read(Channel *channel, channel_on_read);
extern void channel_set_channel_on_error(Channel *channel, channel_on_error);
extern void channel_set_channel_on_close(Channel *channel, channel_on_close);
extern void channel_set_channel_on_peer_cerificate(Channel *channel, channel_on_peer_certificate);
extern void channel_set_channel_ssl_option(Channel *channel, SSLOption *ssl_opt);
extern void channel_set_channel_arg(Channel *channel, void *arg);
extern int channel_write(Channel *channel, const char *data, const size_t data_len);
extern int channel_connect_to(Channel *channel, const char *host, const int port);
extern int channel_connect_to_with_base(Channel *channel, const char *host, const int port, ChannelBase *channel_base);


// server channel 
extern ServerChannel * server_channel_new(void);
extern ServerChannel * server_channel_new_with_ssl(SSLOption *ssl_opt);
extern void server_channel_close(ServerChannel *server);
extern void server_channel_free(ServerChannel *server);
extern const char * server_channel_get_errstr(ServerChannel *server);
extern void * server_channel_get_arg(ServerChannel *server);
extern SSLOption * server_channel_get_ssl_option(ServerChannel *server);

extern int server_channel_listen(ServerChannel *server, const char *host, const int port);
extern int server_channel_listen_ipv6(ServerChannel *server, const char *host, const int port);
extern void server_channel_set_eventloop_threads(ServerChannel *server, uint16_t thread_num);
extern void server_channel_set_read_threads(ServerChannel *server, uint16_t thread_num, size_t memory_size);
extern void server_channel_set_on_accept(ServerChannel *server, server_channel_on_accept);
extern void server_channel_set_channel_on_connect(ServerChannel *server, channel_on_connect);
extern void server_channel_set_channel_on_read(ServerChannel *server, channel_on_read);
extern void server_channel_set_channel_on_error(ServerChannel *server, channel_on_error);
extern void server_channel_set_channel_on_close(ServerChannel *server, channel_on_close);
extern void server_channel_set_channel_on_peer_cerificate(ServerChannel *server, channel_on_peer_certificate);
extern void server_channel_set_channel_ssl_option(ServerChannel *server, SSLOption *ssl_opt);
extern void server_channel_set_arg(ServerChannel *server, void *arg);


//http

extern void http_response_set_status(HttpResponse *res, int code);
extern void http_response_set_content_length(HttpResponse *res, size_t len);
extern void http_response_set_content_type(HttpResponse *res, const char *value);
extern void http_response_set_header(HttpResponse *res, const char *key, const char *value);
extern int http_response_get_status(HttpResponse *res);
extern void * http_response_get_arg(HttpResponse *res);
extern const char * http_response_get_header(HttpResponse *res, const char *key);
extern uint16_t http_response_headers_count(HttpResponse *res);
extern void http_response_headers_foreach(HttpResponse *res, http_header_foreach_cb cb);
extern void http_response_send_head(HttpResponse *res);
extern void http_response_send_body(HttpResponse *res, const char *data, const size_t data_len);
extern void http_response_send_some(HttpResponse *res, const char *data, const size_t data_len);
extern void http_response_send_all(HttpResponse *res, const char *data, const size_t data_len);
extern void http_response_parse(HttpResponse *res, char *msg, size_t *msg_len);

extern const char * http_request_get_method(HttpRequest *req);
extern const char * http_request_get_uri(HttpRequest *req);
extern size_t http_request_get_content_length(HttpRequest *req);
extern const char * http_request_get_content_type(HttpRequest *req);
extern const char * http_request_get_header(HttpRequest *req, const char *key);
extern void * http_request_get_arg(HttpRequest *req);
extern void http_request_set_uri(HttpRequest *req, const char *uri);
extern void http_request_set_header(HttpRequest *req, const char *key, const char *value);
extern uint16_t http_request_get_headers_count(HttpRequest *req);
extern void http_request_get_headers_foreach(HttpRequest *req, http_header_foreach_cb cb);
extern const char * http_request_get_query_param(HttpRequest *req, const char *key);
extern int http_request_is_finished(HttpRequest *req);
extern void http_request_to_string(HttpRequest *req, char **out, size_t *out_len);

extern HttpServer * http_server_new();
extern HttpServer * http_server_new_with_ssl(SSLOption *opt);
extern void http_server_close(HttpServer * server);
extern void http_server_free(HttpServer * server);
extern void http_server_set_ssl_option(HttpServer *server, SSLOption *ssl_opt);
extern void http_server_set_arg(HttpServer *server, void *arg);
extern void * http_server_get_arg(HttpServer *server);
extern SSLOption * http_server_get_ssl_option(HttpServer *server);
extern int http_server_listen(HttpServer *server, const char *host, const uint16_t port);
extern int http_server_listen_ipv6(HttpServer *server, const char *host, const uint16_t port);
extern const char * http_server_get_errstr(HttpServer *server);
extern void http_server_set_message_handler(HttpServer *server, http_chunked_message on_chunked);
extern void http_server_set_error_handler(HttpServer *server, http_err_message on_error);
extern void http_server_set_eventloop_threads(HttpServer *server, uint16_t thread_num);
extern void http_server_set_read_threads(HttpServer *server, uint16_t thread_num, size_t memory_size);


extern FairLock * fair_lock_new();
extern void fair_lock_acquire(FairLock *lock);
extern void fair_lock_release(FairLock *lock);
extern void fair_lock_destroy(FairLock* lock);


extern BaseHandler * base_handler_new();
extern void base_handler_free(BaseHandler *handler);
extern void base_handler_set_max_msg_size(BaseHandler *handler, size_t);
extern void base_handler_set_on_accept(BaseHandler *handler, server_channel_on_accept);
extern void base_handler_set_channel_on_connect(BaseHandler *handler, channel_on_connect);
extern void base_handler_set_channel_on_read(BaseHandler *handler, channel_on_read);
extern void base_handler_set_channel_on_error(BaseHandler *handler, channel_on_error);
extern void base_handler_set_channel_on_close(BaseHandler *handler, channel_on_close);
extern void base_handler_handle_channel(BaseHandler *handler, Channel *channel);
extern void base_handler_handle_server_channel(BaseHandler *handler, ServerChannel *server);
extern int base_handler_write(Channel *channel, const char *data, const size_t data_len);



extern ChannelManager * channel_manager_new();
extern void channel_manager_free(ChannelManager *manager);
extern void channel_manager_close(ChannelManager *manager);
extern void channel_manager_set_threads(ChannelManager *manager, int threads);
extern void channel_manager_set_arg(ChannelManager *manager, void *arg);
extern void * channel_manager_get_arg(ChannelManager *manager);
extern int channel_manager_listen(ChannelManager *manager, const char *host, uint16_t port, 
            channel_manager_on_connect on_connect, channel_manager_on_read on_read, channel_manager_on_error on_error, channel_manager_on_close on_close,
            remote_on_error sub_on_error, remote_on_close sub_on_close);
extern int channel_manager_add_sub_connection(ChannelManager *manager, const char *host, uint16_t port, SSLOption *ssl_opt);
extern void channel_manager_del_sub_connection(ChannelManager *manager, const char *host, uint16_t port);
extern int channel_manager_write_to(ChannelManager *manager, const char *host, uint16_t port, Channel *channel, const char *data, const size_t data_len);
extern const char * channel_manager_get_error_str(ChannelManager *manager);




extern HttpManager * http_manager_new();
extern void http_manager_free(HttpManager *manager);
extern void http_manager_close(HttpManager *manager);
extern void http_manager_set_threads(HttpManager *manager, int threads);
extern void http_manager_set_arg(HttpManager *manager, void *arg);
extern void * http_manager_get_arg(HttpManager *manager);
extern int http_manager_listen(HttpManager *manager, const char *host, uint16_t port, http_manager_on_request on_req, http_manager_on_response on_res, http_err_message on_err,
            remote_on_error sub_on_error, remote_on_close sub_on_close);
extern int http_manager_add_sub_connection(HttpManager *manager, const char *host, uint16_t port, SSLOption *ssl_opt);
extern void http_manager_del_sub_connection(HttpManager *manager, const char *host, uint16_t port);
extern int http_manager_write_to(HttpManager *mgr, const char *host, uint16_t port, HttpRequest *req, const char *chunk, const size_t chunk_len);
extern const char * http_manager_get_error_str(HttpManager *manager);



#ifdef __cplusplus
}
#endif

#endif



