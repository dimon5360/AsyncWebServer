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
    std::ifstream config_;
    std::unordered_map<config_record, config_record> mcfg;

public:

    void Read();
    void PrintContain() noexcept;
    void Close() noexcept;
    void Open(const std::string&& configname);
    const IConfig::config_record GetRecordByKey(const config_record&& key) const;

    /* constructor */
    IConfig() {
        std::cout << "Construct config class\n";
    }
    /* destructor */
    ~IConfig() {
        std::cout << "Destruct config class\n";
    }
};