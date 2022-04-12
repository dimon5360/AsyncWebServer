/*****************************************************************
 *  @file       DataProcessor.h
 *  @brief      Singleton class is responsible for processing all 
 *              data between server and clients
 *  @author     Kalmykov Dmitry
 *  @date       04.09.2021
 *  @modified   04.09.2021
 *  @version    0.1
 */
#pragma once

 /* std C++ lib headers */
#include <string>
#include <queue>
#include <atomic>
#include <shared_mutex>
#include <memory>
#include <chrono>
#include <thread>
#include <future>
#include <functional>

#include <boost/asio.hpp> 

#include "../conn/MessageBroker.h"
#include "../db/MongoProcess.h"
#include "../db/PostgresProcessor.h"

#include "../format/json.h"
#include "../log/Logger.h"

class DataProcess { 

public:

    using record_t = std::pair<const MessageBroker::T, const std::string>;

    void StartDataProcessor();
    void PushNewMessage(const MessageBroker::T id, std::string&& msg) const noexcept;


    std::string GetUsersListInJson(std::string& usersList, const size_t usersCount);
    
    std::string ConstructMessage(const MessageBroker::T& id, const std::string& message, JsonHandler::json_req_t&& json_msg_type);
    std::string ConstructMessage(const MessageBroker::T& id, std::string&& message, JsonHandler::json_req_t&& json_msg_type);
    
    DataProcess(const DataProcess& dp) = delete;
    DataProcess& operator=(const DataProcess& dp) = delete;
 
    DataProcess()
    {
        ConsoleLogger::Debug("Construct Data processor class");
        jsonHandler = std::make_shared<JsonHandler>();
        mongoUserMessagesStorage = std::make_unique<MongoProcessor>("mongo.ini");
        postgresConnectionManager = std::make_unique<PostgresProcessor>();
        StartDataProcessor();
    }

    ~DataProcess() {
        ConsoleLogger::Debug("Destruct Data processor class");
    }

    static const std::shared_ptr<DataProcess>& GetInstance() {
        if(!dp_) {
            dp_ = std::make_shared<DataProcess>(); 
        }
        return dp_;
    } 
    
private:

    const uint32_t delay = 10; // ms
    mutable std::queue<record_t> ioq_; // in and out queue for messages
    mutable std::shared_mutex mutex_;
    mutable std::atomic_int16_t msgInQueue;

    std::shared_ptr<JsonHandler> jsonHandler;

    std::unique_ptr<MongoProcessor> mongoUserMessagesStorage;
    std::unique_ptr<PostgresProcessor> postgresConnectionManager;

    static std::shared_ptr<DataProcess> dp_;


    void ProcessUserMessage(const boost::property_tree::ptree& tree) const noexcept;
    void ProcessAuthMessage(const MessageBroker::T& id, const boost::property_tree::ptree& tree) const noexcept;
    void ProcessGroupMessage(const boost::property_tree::ptree& tree) const noexcept;
    void ProcessUsersListRequest(const boost::property_tree::ptree& tree) const noexcept;
  
    void SendLastMessage() const noexcept;
    void ProcessNewMessage() const noexcept;
    void HandleInOutMessages() const noexcept;
    std::pair<const MessageBroker::T, const std::string> PullNewMessage() const;
};