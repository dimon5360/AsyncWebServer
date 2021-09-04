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

class DataProcess {   

public:
    using data_process_t = std::unique_ptr<DataProcess>;

    void StartDataProcessor();
    // push new message from async connection class in queue to process
    void PushNewMessage(std::string&& msg) const noexcept;
    // process new message to define destiny user ID or session ID
    void ProcessNewMessage() const noexcept;
    // put message for defined user in connection manager class to resend
    void SendLastMessage() const noexcept;

    void Handle2() const noexcept;
    void Handle1() const noexcept;
    
    DataProcess()/* :
        th_(std::thread{ [&]() {
            Handle1();
        }}),
        task(&DataProcess::Handle2)*/
    {
        std::cout << "Construct Data processor class\n";
        msgBroker = std::make_unique<MessageBroker>();
        StartDataProcessor();
    }

    ~DataProcess() {
        std::cout << "Destruct Data processor class\n";
        th_.detach();
    }

private:

    // pull new message from queue to process
    std::string PullNewMessage() const noexcept;

    std::packaged_task<void()> task;
    std::thread th_;
    const uint32_t delay = 10; // ms
    mutable std::unique_ptr<MessageBroker> msgBroker;
    mutable std::queue<std::string> ioq_; // in and out queue for messages
    mutable std::shared_mutex mutex_;
    mutable std::atomic_int16_t msgInQueue;
};

extern DataProcess dataProcessor;