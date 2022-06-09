/*****************************************************************
 *  @file       json.h
 *  @brief      JSON handler class declaration
 *  @author     Kalmykov Dmitry
 *  @date       19.08.2021
 *  @version    0.2
 */
#pragma once

#include <boost/property_tree/ptree.hpp>

#include <iostream>
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

    static std::string msg_identificator_token;
    static std::string dst_user_msg_token;
    static std::string src_user_msg_token;
    static std::string user_msg_token;
    static std::string msg_timestamp_token;
    static std::string msg_hash_token;  

    static std::string users_amount_token;
    static std::string users_list_token;

    static std::string auth_status_token;

private:

    void PrintTree(boost::property_tree::ptree& tree);

public:

    JsonHandler() {
        std::cout << "Construct JsonHandler class\n";
    }
    ~JsonHandler() {
        std::cout << "Destruct JsonHandler class\n";
    }

    boost::property_tree::ptree ConstructTree(const std::string& jsonString);
    boost::property_tree::ptree ConstructTree(std::string&& jsonString);

    template<typename T>
    T ParseJsonParam(const std::string& jsonReq, const std::string& param) {
        auto tree = ConstructTree(jsonReq);
        return tree.get<T>(param);
    }

    template<typename T>
    T ParseTreeParam(const boost::property_tree::ptree& jsonTree, const std::string& param) {
        return jsonTree.get<T>(param);
    }

    std::string ConvertToString(const boost::property_tree::ptree& jsonTree) const noexcept;

private:
    mutable std::queue<std::string> msgQueue;
    mutable std::shared_mutex mtx_;
};