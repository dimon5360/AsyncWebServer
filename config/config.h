/*********************************************
 *
 *
 */
#pragma once

 /* std C++ lib headers */
#include <iostream>
#include <fstream>
#include <unordered_map>

/* abstract class to work with config files */
class IConfig {

private:
    using config_record = std::string;
    mutable std::ifstream config_;
    mutable std::unordered_map<config_record, config_record> mcfg;

public:

    void Read() const noexcept;
    void PrintContain() const noexcept;
    void Close() const noexcept;
    void Open(std::string&& configname) const;

    IConfig::config_record GetConfigValueByKey(config_record&& key) const noexcept;
};