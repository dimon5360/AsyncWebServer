/*****************************************************************
 *  @file       DataProcessor.h
 *  @brief      Singleton class is responsible for processing all
 *              data between server and clients
 *  @author     Kalmykov Dmitry
 *  @date       04.09.2021
 *  @modified   04.09.2021
 *  @version    0.1
 */

 /* std C++ lib headers */

 /* boost C++ lib headers */

 /* local C++ headers */
#include "../data/DataProcess.h"
#include "../log/Logger.h"
#include "../conn/ConnectionManager.h"
#include "../conn/AsyncClient.h"

#define USE_MSG_BROKER true

void DataProcess::StartDataProcessor() {
    th_ = std::thread{ [&]() {
        Handle1();
    } };
}

// push new message from async connection class in queue to process
void DataProcess::PushNewMessage(std::string&& msg) const noexcept {

    try {
        std::unique_lock lk(mutex_);
        ioq_.push(msg);
        msgInQueue++;
    }
    catch (std::exception& ex) {
        ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
    }
}

void DataProcess::Handle1() const noexcept {
    while (true) {
        if (msgInQueue > 0) {
            ProcessNewMessage();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}

void DataProcess::Handle2() const noexcept {
    while (true) {
        if (msgInQueue > 0) {
            ProcessNewMessage();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}


// process new message to define destiny user ID or session ID
void DataProcess::ProcessNewMessage() const noexcept {

    try {
        std::string msg{ PullNewMessage() };

        // template of process msg (JSON format)
        AsyncTcpConnection::id_t tempId = 1;
        std::string tempMsg = "Hello";

        msgBroker->PushMessage(tempId, std::move(tempMsg));
        SendLastMessage();
    }
    catch (std::exception& ex) {
        ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
    }
}

// put message for defined user in connection manager class to resend
void DataProcess::SendLastMessage() const noexcept {
    try {
        auto msg = msgBroker->PullMessage();
        connMan_.ResendUserMessage(msg.first, msg.second);
    }
    catch (std::exception& ex) {
        ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
    }
}

// pull new message from queue to process
std::string DataProcess::PullNewMessage() const noexcept {
    std::string res;
    try {
        std::unique_lock lk(mutex_);
        res = ioq_.front();
        ioq_.pop();
        msgInQueue--;
    }
    catch (std::exception& ex) {
        ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
    }
    return res;
}

DataProcess dataProcessor;