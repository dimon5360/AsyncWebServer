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
#include <boost/lexical_cast.hpp>

 /* local C++ headers */
#include "../data/DataProcess.h"
#include "../log/Logger.h"
#include "../conn/ConnectionManager.h"
#include "../conn/AsyncClient.h"


#define USE_MSG_BROKER true

void DataProcess::StartDataProcessor() {
    std::thread{ [&]() {
        HandleInOutMessages();
    }}.join();
}

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

void DataProcess::HandleInOutMessages() const noexcept {
    while (connMan_.ManagerIsActive()) {
        if (msgInQueue > 0) {
            ProcessNewMessage();
        }
        if (!msgBroker->IsQueueEmpty()) {
            SendLastMessage();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}

void DataProcess::ProcessNewMessage() const noexcept {

    try {
        std::string msg{ PullNewMessage() };

        if (msg.starts_with(tech_msg_header)) {  // todo: further use the json parser 
            MessageBroker::T id;
            auto user_id_offset = msg.find(",") - tech_msg_header.size();
            auto sId = msg.substr(tech_msg_header.size(), user_id_offset);
            id = boost::lexical_cast<MessageBroker::T>(sId);
            auto message_start_offset = msg.find(tech_req_msg) + tech_req_msg.size();
            auto messageItself = msg.substr(message_start_offset, msg.size() - message_start_offset);

            msgBroker->PushMessage(id, std::move(messageItself));
        }
        else {
            ConsoleLogger::Error("Undefined message header");
        }
    }
    catch (std::exception& ex) {
        ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
    }
}

void DataProcess::SendLastMessage() const noexcept {
    try {
        auto msg = msgBroker->PullMessage();
        connMan_.ResendUserMessage(msg.first, msg.second);
    }
    catch (std::exception& ex) {
        ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
    }
}

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