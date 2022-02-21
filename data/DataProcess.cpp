/*****************************************************************
 *  @file       DataProcessor.h
 *  @brief      Singleton class is responsible for processing all
 *              data between server and clients
 *  @author     Kalmykov Dmitry
 *  @date       04.09.2021
 *  @version    0.1
 */

 /* std C++ lib headers */
 #include "string.h"

 /* boost C++ lib headers */
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>

 /* local C++ headers */
#include "DataProcess.h"
#include "../log/Logger.h"
#include "../conn/ConnectionManager.h"
#include "../conn/AsyncClient.h"

void DataProcess::StartDataProcessor() {
    std::thread{ [&]() {
        HandleInOutMessages();
    }}.detach();
}

void DataProcess::PushNewMessage(std::string& msg) const noexcept {

    try {
        std::unique_lock lk(mutex_);
        ioq_.push(msg);
        msgInQueue++;
    }
    catch (std::exception& ex) {
        ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
    }
}

std::string DataProcess::ConstructMessage(const MessageBroker::T& id, std::string& message, JsonHandler::json_req_t&& json_msg_type) {

    namespace pt = boost::property_tree;
    pt::ptree ptree;

    ptree.put(JsonHandler::usersListJsonHeader, static_cast<uint32_t>(json_msg_type));
    ptree.put(JsonHandler::dst_user_msg_token, id);
    ptree.put(JsonHandler::user_msg_token, message);
    std::ostringstream oss;
    boost::property_tree::json_parser::write_json(oss, ptree);
    std::string res = jsonHandler->ConvertToString(ptree);
    return res;
}

std::string DataProcess::GetUsersListInJson(std::string& usersList, const size_t usersCount) {

    namespace pt = boost::property_tree;
    pt::ptree ptree;

    ptree.put(JsonHandler::usersCountJsonField, usersCount);
    ptree.put(JsonHandler::usersListJsonField, usersList);

    std::ostringstream oss;
    boost::property_tree::json_parser::write_json(oss, ptree);
    std::string res = jsonHandler->ConvertToString(ptree);
    return res;
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

void DataProcess::ProcessUserMessage(const boost::property_tree::ptree& tree) const noexcept {

    try {
        mongoUserMessagesStorage->InsertNewMessage(jsonHandler->ConvertToString(tree));

        std::string userId = jsonHandler->ParseTreeParam<std::string>(tree, JsonHandler::dst_user_msg_token);
        std::string userMsg = jsonHandler->ParseTreeParam<std::string>(tree, JsonHandler::user_msg_token);
        uint32_t id = boost::lexical_cast<MessageBroker::T>(userId);
        msgBroker->PushMessage(id, std::move(userMsg));
    }
    catch (std::exception& ex) {
        ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
    }
}

void DataProcess::ProcessGroupMessage(const boost::property_tree::ptree& tree) const noexcept {
    // TODO:
    (void)tree;
}

void DataProcess::ProcessAuthMessage(const boost::property_tree::ptree& tree) const noexcept {
    // TODO: process user authentication data
    // here must send request to DB service to validate user data
}

void DataProcess::ProcessUsersListRequest(const boost::property_tree::ptree& tree) const noexcept {

    try {
        mongoUserMessagesStorage->InsertNewMessage(jsonHandler->ConvertToString(tree));

        std::string userId = jsonHandler->ParseTreeParam<std::string>(tree, JsonHandler::dst_user_msg_token);
        std::string userMsg = jsonHandler->ParseTreeParam<std::string>(tree, JsonHandler::user_msg_token);
        uint32_t id = boost::lexical_cast<MessageBroker::T>(userId);
        msgBroker->PushMessage(id, std::move(userMsg));
    }
    catch (std::exception& ex) {
        ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
    }
}

void DataProcess::ProcessNewMessage() const noexcept {

    try {
        std::string msg{ PullNewMessage() };

        namespace pt = boost::property_tree;
        pt::ptree tree = jsonHandler->ConstructTree(msg);

        auto identifer = boost::lexical_cast<uint32_t>(jsonHandler->ParseTreeParam<std::string>(tree, JsonHandler::usersListJsonHeader));

        switch(static_cast<JsonHandler::json_req_t>(identifer)) {
            case JsonHandler::json_req_t::users_list_message: {
                ProcessUsersListRequest(tree);
                break;
            }
            case JsonHandler::json_req_t::authentication_message: {
                ProcessAuthMessage(tree);
                break;
            }
            case JsonHandler::json_req_t::user_message: {
                ProcessUserMessage(tree);
                break;
            }
            case JsonHandler::json_req_t::group_users_message: {
                ProcessGroupMessage(tree);
                break;
            }
            default: {
                ConsoleLogger::Error("Undefined message identifier.");
                break;
            }
        }
    }
    catch (std::exception& ex) {
        ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
    }
}

void DataProcess::SendLastMessage() const noexcept {
    try {
        auto msg = msgBroker->PullMessage();
        std::cout << msg.first << " " << msg.second << std::endl;
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