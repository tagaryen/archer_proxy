#include "Common.h"
#include "Icon.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>

static const int HEX_MAP[] = {127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 127, 127, 127, 127, 127, 127, 127, 10, 11, 12, 13, 14, 15, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 10, 11, 12, 13, 14, 15, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127};
static const char BYTE_MAP[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
static unsigned char SEED = 1;

static int checkHex(std::string const& hex) {
    if(hex.length() != 128 && hex.length() != 130) {
        return -1;
    }
    if(hex.length() == 130){
        if(hex[0] != '0' || (hex[1] != 'x' && hex[1] != 'X')) {
            return -1;
        }
        return 2;
    }
    return 0;
}


static void createDirectory(const char *path) {
    if(access(path, 0)) {
#ifdef _WIN32
        mkdir(path);
#else
        mkdir(path, S_IRWXU);
#endif
    }
}

void archer::common::createDirectories(std::string const& pathStr) {
    const char *path = pathStr.c_str();
    size_t len = pathStr.length();
    char *dir = (char *) malloc(len + 1);
    memcpy(dir, path, len);
    dir[len] = '\0';
    if(dir[len - 1] == 92 || dir[len - 1] == 47) {
        dir[len - 1] = '\0';
        --len;
    }
    for(int i = 1; i < len; i++) {
        if(dir[i] == 47 || dir[i] == 92) {
            dir[i] = 0;
            createDirectory(dir);
            dir[i] = '/';
        }
    }
    createDirectory(dir);
}



bool archer::common::isAbsolutePath(std::string const& path) {
    if(path.empty()) {
        return false;
    }
#ifdef __WIN32
    return (isalpha(path[0]) && path[1] == ':') || (path[0] == 92 && path[1] == 92);
#else
    return (path[0] == '/');
#endif
}

std::string archer::common::getCurrentPath() {
    char path[1024];
    if(getcwd(path, 1024)) {}
    size_t len = strlen(path);
    return std::string(path, len);
}

bool archer::common::fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

std::string archer::common::getNowTime() {
    std::time_t now = std::time(nullptr);
    now += 8 * 3600;//时区问题
    std::tm *localTime = std::localtime(&now);
    char timestr[TIME_FORMAT_LEN];
    std::strftime(timestr, TIME_FORMAT_LEN, "%Y-%m-%d %H:%M:%S", localTime);
    std::string nowStr(timestr, TIME_FORMAT_LEN);
    return std::move(nowStr);
}

int archer::common::getUint8sFromHex(std::string const& hex, uint8_t* bytes) {
    for(int i = 0 ; i < hex.length(); i+=2) {
        uint8_t h1 = hex[i], h2 = hex[i+1];
        if(HEX_MAP[h1] == 127 || HEX_MAP[h2] == 127) {
            return 0;
        }
        bytes[i>>1] = (HEX_MAP[h1] << 4) | HEX_MAP[h2];
    }
    return 1;
}

std::string archer::common::getHexFromUint8s(const uint8_t* bytes, size_t bytes_len) {
    std::string hex;
    for(int i = 0 ; i < bytes_len; i++) {
        hex += BYTE_MAP[(bytes[i] >> 4) & 0xf];
        hex += BYTE_MAP[bytes[i] & 0xf];
    }
    return hex;
}

std::string archer::common::randomString() {
    unsigned char random[16];
    SEED++;
    if(SEED == 0) {
        SEED++;
    }
    for(int i = 0; i < 16; i++) {
        random[i] = SEED ^ random[i];
    }
    return getHexFromUint8s(random, 16);
}

bool archer::common::isIpAddress(std::string& ipstr) {
    std::regex pattern("((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)");
    std::smatch res;
    if(std::regex_match(ipstr, res, pattern)){
        return true;
    }
    return false;
}
