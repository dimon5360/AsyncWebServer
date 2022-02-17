/*****************************************************************
 *  @file       MongoProcess.h
 *  @brief      Postgres connection handler declaration
 *  @author     Kalmykov Dmitry
 *  @date       15.02.2021
 *  @version    0.1
 */
#pragma once

#include <string>
#include <iostream>
#include <memory>

#include <cstdint>
#include <iostream>
#include <vector>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

#include "../config/config.h"

#include <boost/property_tree/json_parser.hpp>

class MongoProcessor {

    enum class ConfigClass {
        debug = 0,
        release,
    };

    static constexpr ConfigClass config_ = ConfigClass::debug; 

    mutable std::string connectingConfig_, connectingString_;  
    mutable std::shared_ptr<IConfig> dbcfg;
    mutable std::shared_ptr<mongocxx::client> mongoclient_;

    static void MongoLog(std::string&& logMsg);
    static void MongoError(std::string&& errMsg);
    
    void Insert(std::unique_ptr<mongocxx::collection> collection, const std::string& json) noexcept;
    void Insert(std::unique_ptr<mongocxx::collection> collection, const boost::property_tree::ptree& tree) noexcept;
    void InitializeConnection(const std::string config) noexcept;
    
public:


    MongoProcessor() = delete;
    MongoProcessor(std::string&& connectingConfig);
    ~MongoProcessor();

    void InsertNewMessage(std::string&& msg) noexcept;
};