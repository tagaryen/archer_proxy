#pragma once

#include <libcommon/Common.h>
#include <json/json.h>
#include <unordered_map>

#include "archer_net.h"

namespace archer 
{
namespace handler 
{

typedef std::function<void(HttpResponse*,Json::Value&)> handlerFunction;

class HttpHandler
{
public:
    HttpHandler() {};

    virtual ~HttpHandler(){};

    HttpHandler(const HttpHandler&) = delete;
    HttpHandler& operator=(const HttpHandler&) = delete;

    virtual std::unordered_map<std::string, handlerFunction> getHandlerFunctions() = 0;
    virtual std::unordered_map<std::string, handlerFunction> postHandlerFunctions() = 0;
};
}
}