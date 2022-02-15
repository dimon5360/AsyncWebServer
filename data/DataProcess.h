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

 /* boost C++ lib headers */
#include <boost/asio.hpp> 

 /* local C++ headers */
#include "../conn/MessageBroker.h"
#include "../utils/json.h"

class DataProcess {   

public:

    void StartDataProcessor();
    void PushNewMessage(std::string& msg) const noexcept;
    std::string GetUsersListInJson(std::string& usersList, const size_t usersCount);
    std::string ConstructMessage(const MessageBroker::T& id, std::string& message);
    
    DataProcess()
    {
        std::cout << "Construct Data processor class\n";
        msgBroker = std::make_unique<MessageBroker>();
        jsonHandler = std::make_unique<JsonHandler>();
        StartDataProcessor();
    }

    ~DataProcess() {
        std::cout << "Destruct Data processor class\n";
    }

private:

    void ProcessUserMessage(const boost::property_tree::ptree& tree) const noexcept;
    void ProcessAuthMessage(const boost::property_tree::ptree& tree) const noexcept;
    void ProcessGroupMessage(const boost::property_tree::ptree& tree) const noexcept;
    void ProcessUsersListRequest(const boost::property_tree::ptree& tree) const noexcept;

    void SendLastMessage() const noexcept;
    void ProcessNewMessage() const noexcept;
    void HandleInOutMessages() const noexcept;
    std::string PullNewMessage() const noexcept;

    const uint32_t delay = 10; // ms
    mutable std::unique_ptr<MessageBroker> msgBroker;
    mutable std::queue<std::string> ioq_; // in and out queue for messages
    mutable std::shared_mutex mutex_;
    mutable std::atomic_int16_t msgInQueue;

    std::unique_ptr<JsonHandler> jsonHandler;

    const std::string tech_msg_header{ "user id=" };
    const std::string tech_pub_key_msg{ "key=" };
    const std::string tech_req_msg{ "message=" };
    const std::string tech_resp_msg{ "summ=" };
};

extern DataProcess dataProcessor;