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

#include <boost/format.hpp>
#include <spdlog/spdlog.h>

/* local C++ headers */
#include "config.h"

void IConfig::Read() const noexcept {
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

void IConfig::PrintContain() const noexcept {
    config_record record;
    for (; !config_.eof();) {
        std::getline(config_, record, '\n');
        std::cout << record << std::endl;
    }
}

void IConfig::Close() const noexcept {
    config_.close();
}

void IConfig::Open(std::string&& configname) const {
    config_.open(configname);
    if (!config_) {
        throw std::runtime_error("Config file is invalid"); 
    }
    Read();
}

IConfig::config_record IConfig::GetConfigValueByKey(IConfig::config_record&& key) const noexcept {

    config_record ret;
    try {
        ret = mcfg.at(key);
    }
    catch (std::exception& ex) {
        spdlog::error(boost::str(boost::format("%1% %2%") % "Condfig get value error: " % ex.what()));
    }
    return ret;
}
