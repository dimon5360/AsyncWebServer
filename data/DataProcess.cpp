/*****************************************************************
 *  @file       DataProcessor.h
 *  @brief      Singleton class is responsible for processing all
 *              data between server and clients
 *  @author     Kalmykov Dmitry
 *  @date       04.09.2021
 *  @version    0.1
 */

 #include "string.h"

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "DataProcess.h"
#include "../conn/ConnectionManager.h"
#include "../conn/MessageBroker.h"
#include "../conn/AsyncClient.h"
#include "../format/json.h"

void DataProcess::StartDataProcessor() {
    std::thread{ [&]() {
        HandleInOutMessages();
    }}.detach();
}

void DataProcess::PushNewMessage(const MessageBroker::T id, std::string&& msg) const noexcept {

    try {
        std::unique_lock lk(mutex_);
        ioq_.push(std::make_pair<const MessageBroker::T, std::string>(std::move(id), std::move(msg)));
        msgInQueue++;
    }
    catch (std::exception& ex) {
        ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
    }
}

std::string DataProcess::ConstructMessage(const MessageBroker::T& id, const std::string& message, JsonHandler::json_req_t&& json_msg_type) {

    namespace pt = boost::property_tree;
    pt::ptree ptree;

    ptree.put(JsonHandler::msg_identificator_token, static_cast<uint32_t>(json_msg_type));
    ptree.put(JsonHandler::dst_user_msg_token, id);
    ptree.put(JsonHandler::user_msg_token, message);
    std::string res = jsonHandler->ConvertToString(ptree);
    return res;
}

std::string DataProcess::ConstructMessage(const MessageBroker::T& id, std::string&& message, JsonHandler::json_req_t&& json_msg_type) {

    namespace pt = boost::property_tree;
    pt::ptree ptree;

    ptree.put(JsonHandler::msg_identificator_token, static_cast<uint32_t>(json_msg_type));
    ptree.put(JsonHandler::dst_user_msg_token, id);
    ptree.put(JsonHandler::user_msg_token, std::move(message));
    std::string res = jsonHandler->ConvertToString(ptree);
    return res;
}

std::string DataProcess::GetUsersListInJson(std::string& usersList, const size_t usersCount) {

    namespace pt = boost::property_tree;
    pt::ptree ptree;

    ptree.put(JsonHandler::users_amount_token, usersCount);
    ptree.put(JsonHandler::users_list_token, usersList);

    std::ostringstream oss;
    boost::property_tree::json_parser::write_json(oss, ptree);
    std::string res = jsonHandler->ConvertToString(ptree);
    return res;
}

void DataProcess::HandleInOutMessages() const noexcept {
    
    while (ConnectionManager::GetInstance()->ManagerIsActive()) {
        if (msgInQueue > 0) {
            ProcessNewMessage();
        }
        if (!MessageBroker::GetInstance()->IsQueueEmpty()) {
            SendLastMessage();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}


// @brief correct logic, need to retransmit received message to another user (also keep in db)
void DataProcess::ProcessUserMessage(const boost::property_tree::ptree& tree) const noexcept {

    try {
        mongoUserMessagesStorage->InsertNewMessage(jsonHandler->ConvertToString(tree));

        std::string userId = jsonHandler->ParseTreeParam<std::string>(tree, const_cast<const std::string&>(JsonHandler::dst_user_msg_token));
        std::string userMsg = jsonHandler->ParseTreeParam<std::string>(tree, JsonHandler::user_msg_token);
        uint32_t id = boost::lexical_cast<MessageBroker::T>(userId);
        MessageBroker::GetInstance()->PushMessage(id, std::move(userMsg));
    }
    catch (std::exception& ex) {
        ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
    }
}

void DataProcess::ProcessGroupMessage(const boost::property_tree::ptree& tree) const noexcept {
    // TODO:
    (void)tree;
}

// @brief correct logic, need to implement processing and validating user personal data 
void DataProcess::ProcessAuthMessage(const MessageBroker::T& id, const boost::property_tree::ptree& tree) const noexcept {

    try
    {
        // TODO: process user authentication data

        // std::string userId = jsonHandler->ParseTreeParam<std::string>(tree, JsonHandler::dst_user_msg_token);
        // std::string timestamp = jsonHandler->ParseTreeParam<std::string>(tree, JsonHandler::msg_timestamp_token);
    
        // here must send request to DB service to validate user data
        namespace pt = boost::property_tree;
        pt::ptree ptree;

        ptree.put(JsonHandler::msg_identificator_token, static_cast<uint32_t>(JsonHandler::json_req_t::authentication_message));
        ptree.put(JsonHandler::dst_user_msg_token, id);
        ptree.put(JsonHandler::auth_status_token, std::move("approved"));
        ptree.put(JsonHandler::user_msg_token, std::move(""));
        
        std::string outmsg = jsonHandler->ConvertToString(ptree);

        MessageBroker::GetInstance()->PushMessage(id, std::move(outmsg));
    }
    catch (std::exception &ex)
    {
        ConsoleLogger::Error(ex.what());
    }
}

// @brief need to process request and transmit the user list
void DataProcess::ProcessUsersListRequest(const boost::property_tree::ptree& tree) const noexcept {

    try {
        mongoUserMessagesStorage->InsertNewMessage(jsonHandler->ConvertToString(tree));

        std::string userId = jsonHandler->ParseTreeParam<std::string>(tree, JsonHandler::src_user_msg_token);
        std::string datetime = jsonHandler->ParseTreeParam<std::string>(tree, JsonHandler::msg_timestamp_token);
        std::string hash = jsonHandler->ParseTreeParam<std::string>(tree, JsonHandler::msg_hash_token);
        uint32_t id = boost::lexical_cast<MessageBroker::T>(userId);
        ConnectionManager::GetInstance()->SendUsersListToUser(id); 

    } catch (std::exception& ex) {
        ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
    }
}

void DataProcess::ProcessNewMessage() const noexcept {

    try {
        decltype(auto) msg{ PullNewMessage() };

        namespace pt = boost::property_tree;
        pt::ptree tree = jsonHandler->ConstructTree(msg.second);

        auto identifer = boost::lexical_cast<uint32_t>(jsonHandler->ParseTreeParam<std::string>(tree, 
                                                                JsonHandler::msg_identificator_token));

        switch(static_cast<JsonHandler::json_req_t>(identifer)) {
            case JsonHandler::json_req_t::users_list_message: {
                ProcessUsersListRequest(tree);
                break;
            }
            case JsonHandler::json_req_t::user_message: {
                ProcessUserMessage(tree);
                break;
            }
            case JsonHandler::json_req_t::authentication_message: {
                ProcessAuthMessage(msg.first, tree);
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
        auto msg = MessageBroker::GetInstance()->PullMessage();
        ConnectionManager::GetInstance()->ResendUserMessage(msg.first, msg.second);
    }
    catch (std::exception& ex) {
        ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
    }
}

std::pair<const MessageBroker::T, const std::string> DataProcess::PullNewMessage() const {
    std::unique_lock lk(mutex_);
    auto[id, msg] {ioq_.front()};
    ioq_.pop();
    msgInQueue--;
    return std::make_pair(std::move(id), std::move(msg));
}

std::shared_ptr<DataProcess> DataProcess::dp_ = nullptr;