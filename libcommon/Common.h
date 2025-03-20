#pragma once

#include <functional>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <string>
#include <memory>
#include <regex>

#pragma warning(push)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma warning(pop)
#pragma GCC diagnostic pop

namespace archer 
{
namespace common 
{
bool isAbsolutePath(std::string const& path);
void createDirectories(std::string const& path);
std::string getCurrentPath();
bool fileExists(const std::string& filename);
std::string getNowTime();
int getUint8sFromHex(std::string const& hex, uint8_t* bytes);
std::string getHexFromUint8s(const uint8_t* bytes, size_t bytes_len);
std::string randomString();
bool isIpAddress(std::string& ipstr);
}
}

