/*****************************************************************
 *  @file       json.h
 *  @brief      JSON handler class declaration
 *  @author     Kalmykov Dmitry
 *  @date       19.08.2021
 *  @modified   03.09.2021
 *  @version    0.2
 */
#pragma once

 /* boost C++ lib headers */
#include <boost/property_tree/ptree.hpp>

/* std �++ lib headers */
#include <queue>
#include <shared_mutex>

class JsonHandler {

public:
    using json_req_t = enum class req_t : uint32_t {
        users_list_message = 1,
        authentication_message,
        user_message,
        group_users_message,
    };

    static std::string usersListJsonHeader;
    static std::string usersCountJsonField;
    static std::string usersListJsonField;
    
    static std::string msg_identificator_token;
    static std::string dst_user_msg_token;
    static std::string src_user_msg_token;
    static std::string user_msg_token;
    static std::string msg_timestamp_token;
    static std::string msg_hash_token;  

private:

    void PrintTree(boost::property_tree::ptree& tree);
    void HandleAuthJson(std::string&& authJson) noexcept;

public:
    boost::property_tree::ptree ConstructTree(const std::string& jsonString);

    void PushRequest(std::string&& inReq) const noexcept;
    std::string PullRequest() const noexcept;

    void HandleRequest(std::string&& json, const json_req_t& type) noexcept;

    template<typename T>
    T ParseJsonParam(const std::string& jsonReq, const std::string& param);
    template<typename T>
    T ParseTreeParam(const boost::property_tree::ptree& jsonTree, const std::string& param);

    std::string ConvertToString(const boost::property_tree::ptree& jsonTree) const noexcept;

private:
    mutable std::queue<std::string> msgQueue;
    mutable std::shared_mutex mtx_;
};