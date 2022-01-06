/*********************************************
 *
 *
 */
/* std C++ lib headers */
#include <iostream>
#include <istream>
#include <ctime>
#include <utility>
#include <string>
#include <unordered_map>
#include <sstream>

/* local C++ headers */
#include "config.h"

void IConfig::Read() {
    std::string record;
    std::string key, symbol, value;

    for (; !config_.eof(); ) {

        std::getline(config_, record, '\n');
        if (record.compare("#") == 0) {
            /* this is a comment */
            continue;
        }

        std::stringstream ss(record);
        ss >> key >> symbol >> value;
        mcfg[key] = value;
    }
}

void IConfig::PrintContain() noexcept {
    config_record record;
    for (; !config_.eof();) {
        std::getline(config_, record, '\n');
        std::cout << record << std::endl;
    }
}

void IConfig::Close() noexcept {
    config_.close();
}

void IConfig::Open(const std::string&& configname) {
    config_.open(configname); // launch under debug from VS
    if (!config_) {
        throw std::runtime_error("Config TCP server file is invalid"); 
    }

    Read();
}

const IConfig::config_record IConfig::GetRecordByKey(const IConfig::config_record&& key) const {

    config_record ret;
    try {
        ret = mcfg.at(key);
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
    return ret;
}
