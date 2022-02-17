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
 #include "string.h"

 /* boost C++ lib headers */
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>

 /* local C++ headers */
#include "DataProcess.h"
#include "../log/Logger.h"
#include "../conn/ConnectionManager.h"
#include "../conn/AsyncClient.h"

#define USE_MSG_BROKER true

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

std::string DataProcess::ConstructMessage(const MessageBroker::T& id, std::string& message) {

    namespace pt = boost::property_tree;
    pt::ptree ptree;

    ptree.put("user id", id);
    ptree.put("message", message);
    std::ostringstream oss;
    boost::property_tree::json_parser::write_json(oss, ptree);
    std::string res = jsonHandler->ConvertToString(ptree);
    return res;
}

/* structure of users list response message
{
    "message_identifier" : users_list_message // details (JsonHandler::json_req_t)
    "users_amount" : 1 ... N, // size of comtainer in UsersPool class (size_t)
    "users_list" : [ "user1_id", "user2_id", ... "userN_id" ],
}
*/

/* structure of different request messages 
{
    "message_identifier" : authentication_message | user_message | group_users_message
    "src user id" : async connection ID
    "dst user id" : [] async connection IDs (optional, depending from message type)
    "message" : " ... ",
    "timestamp" : system datetime
    "hash" : sha512 | sha256
}
*/

std::string DataProcess::GetUsersListInJson(std::string& usersList, const size_t usersCount) {

    namespace pt = boost::property_tree;
    pt::ptree ptree;

    auto identifer = static_cast<uint32_t>(JsonHandler::json_req_t::users_list_message);

    ptree.put(JsonHandler::usersListJsonHeader, identifer);
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
    auto userId = jsonHandler->ParseTreeParam<std::string>(tree, "user id");
    auto userMsg = jsonHandler->ParseTreeParam<std::string>(tree, "message");
    auto id = boost::lexical_cast<MessageBroker::T>(userId);
    msgBroker->PushMessage(id, userMsg);
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
        auto userId = jsonHandler->ParseTreeParam<std::string>(tree, std::move("user id"));
        connMan_.SendUsersListToUser(boost::lexical_cast<uint32_t>(userId));
    }
    catch (std::exception& ex) {
        ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
    }
}

void DataProcess::ProcessNewMessage() const noexcept {

    try {
        std::string msg{ PullNewMessage() };

        // TODO: keep msg in NoSQL DB

        namespace pt = boost::property_tree;
        pt::ptree tree = jsonHandler->ConstructTree(msg);

        // read new message ID (identification of message in hierarchy)
        auto msgId = jsonHandler->ParseTreeParam<std::string>(tree, "message_identifier");
        auto identifer = boost::lexical_cast<uint32_t>(msgId);

        switch(static_cast<JsonHandler::json_req_t>(identifer)) {
            case JsonHandler::json_req_t::users_list_message: {
                /* app client can send such messages to synch list of users */
                ProcessUsersListRequest(tree);
                break;
            }
            case JsonHandler::json_req_t::authentication_message: {
                /* user connection message */
                ProcessAuthMessage(tree);
                break;
            }
            case JsonHandler::json_req_t::user_message: {
                /* usual users messages */
                ProcessUserMessage(tree);
                break;
            }
            case JsonHandler::json_req_t::group_users_message: {
                /* user can send such messages in bounded group of users */
                ProcessGroupMessage(tree);
                break;
            }
            default: {
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
    return std::move(res);
}

DataProcess dataProcessor;