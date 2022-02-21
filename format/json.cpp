/*****************************************************************
 *  @file       json.cpp
 *  @brief      JSON handler class implementation
 *
 *  @author     Kalmykov Dmitry
 *  @date       19.08.2021
 *  @version    0.2
 */

#include "json.h"
#include "../log/Logger.h"

#include <fstream>
#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/format.hpp>
#include <boost/iostreams/stream.hpp>

#include <spdlog/spdlog.h>

std::string JsonHandler::usersListJsonHeader = "message_identifier";
std::string JsonHandler::usersCountJsonField = "users_amount";
std::string JsonHandler::usersListJsonField = "users_list";

std::string JsonHandler::msg_identificator_token{ "message_identifier" };
std::string JsonHandler::dst_user_msg_token{ "dst_user_id" };
std::string JsonHandler::src_user_msg_token{ "src_user_id" };
std::string JsonHandler::user_msg_token{ "user_message" };
std::string JsonHandler::msg_timestamp_token{ "message_timestamp" };
std::string JsonHandler::msg_hash_token{ "message_hash" };

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
    "src_user_id" : async connection ID
    "dst_user_id" : [] async connection IDs (optional, depending from message type)
    "user_message" : " ... ",
    "message_timestamp" : system datetime
    "message_hash" : sha512 | sha256
}
*/

void JsonHandler::PushRequest(std::string&& inReq) const noexcept {
    std::unique_lock lk(mtx_);
    msgQueue.push(std::move(inReq));
}

std::string JsonHandler::PullRequest() const noexcept {
    std::unique_lock lk(mtx_);
    std::string outMsg{ "" };
    if (!msgQueue.empty()) {
        outMsg = msgQueue.front();
        msgQueue.pop();
    }
    return outMsg;
}


void JsonHandler::HandleAuthJson(std::string&& authJson) noexcept {

    namespace pt = boost::property_tree;

    pt::ptree tree = ConstructTree(authJson);
    auto usermail = ParseTreeParam<std::string>(tree, "usermail");
    ConsoleLogger::Debug(boost::str(boost::format("%1%") % usermail));
    auto username = ParseTreeParam<std::string>(tree, "username");
    ConsoleLogger::Debug(boost::str(boost::format("%1%") % username));
    auto password = ParseTreeParam<std::string>(tree, "password");
    ConsoleLogger::Debug(boost::str(boost::format("%1%") % password));

    // TODO: send to postgres processor
}

void JsonHandler::HandleRequest(std::string&& json, const json_req_t& type) noexcept {

    try {
        switch (type) {
            case json_req_t::authentication_message:
            {
                HandleAuthJson(std::move(json));
                break;
            }
            case json_req_t::user_message:
            {
                HandleAuthJson(std::move(json));
                break;
            }
            default: 
            {
                //throw std::string{boost::str(boost::format("Invalid request type %1%") % type)};
                throw "Invalid request type " + std::to_string(static_cast<double>(type));
            }
        }
    }
    catch (std::exception& ex) {
        ConsoleLogger::Error(std::string{ ex.what() });
    }
}

boost::property_tree::ptree JsonHandler::ConstructTree(const std::string& jsonString) {
    namespace pt = boost::property_tree;
    pt::ptree ptree;
    boost::iostreams::array_source as(jsonString.data(), jsonString.size());
    boost::iostreams::stream<boost::iostreams::array_source> is(as);
    pt::read_json(is, ptree);
    return ptree;
}

void JsonHandler::PrintTree(boost::property_tree::ptree& tree) {

    std::string msg;

    msg = tree.get<std::string>("usermail");
    ConsoleLogger::Debug(boost::str(boost::format("%1%") % msg));
    msg = tree.get<std::string>("username");
    ConsoleLogger::Debug(boost::str(boost::format("%1%") % msg));
    msg = tree.get<std::string>("password");
    ConsoleLogger::Debug(boost::str(boost::format("%1%") % msg));

    /* traversal the tree through 'foreach' */
    for (auto& item : tree) {
        auto key = item.first;
        auto value = item.second.data();
        ConsoleLogger::Debug(boost::str(boost::format("%1% : %2%") % key % value));
    }
}

template<typename T>
T JsonHandler::ParseJsonParam(const std::string& jsonReq, const std::string& param) {
    auto tree = ConstructTree(jsonReq);
    return tree.get<T>(param);
}

template<typename T>
T JsonHandler::ParseTreeParam(const boost::property_tree::ptree& jsonTree, const std::string& param) {
    return jsonTree.get<T>(param);
}

std::string JsonHandler::ConvertToString(const boost::property_tree::ptree& jsonTree) const noexcept {

    std::ostringstream oss;
    boost::property_tree::json_parser::write_json(oss, jsonTree);
    return oss.str();
}


