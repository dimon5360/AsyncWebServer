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
    typedef std::string config_record;
    std::ifstream config_;
    std::unordered_map<config_record, config_record> mcfg;

public:

    void Read();
    void PrintContain() noexcept;
    void Close() noexcept;
    void Open(std::string configname);
    config_record GetRecordByKey(config_record key) const;

    /* constructor */
    IConfig() {
        std::cout << "Construct config class\n";
    }
    /* destructor */
    ~IConfig() {
        std::cout << "Destruct config class\n";
    }
};