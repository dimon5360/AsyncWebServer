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

class MongoProcessor {

    enum class ConfigClass {
        debug = 0,
        release,
    };

    // private fields
    static constexpr ConfigClass config_ = ConfigClass::release; 
    std::string connectingString_;  

    mutable std::shared_ptr<mongocxx::client> mongoclient_;

    // private member-functions
    void InitializeConnection() const noexcept;
        
    // static private member-functions
    static void Log(std::string&& log);

public:

    void testInsert(std::string&& dbs, std::string&& table) const noexcept;
    void testRequest(std::string&& dbs, std::string&& table) const noexcept;

    MongoProcessor() = delete;
    MongoProcessor(std::string&& connectingString);
    MongoProcessor(const std::string& connectingString);
    
    ~MongoProcessor();
};