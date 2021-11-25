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

/* std Ñ++ lib headers */
#include <queue>
#include <shared_mutex>

class JsonHandler {

public:
    using json_req_t = enum class req_t : uint32_t {
        users_list_broadcast_message = 1,
        authentication_request,
        user_message,
    };

    static std::string usersListJsonHeader;
    static std::string usersCountJsonField;
    static std::string usersListJsonField;

private:

    void PrintTree(boost::property_tree::ptree& tree);

    void HandleAuthJson(std::string&& authJson) noexcept;

public:
    boost::property_tree::ptree ConstructTree(const std::string& jsonString);

    void PushRequest(std::string&& inReq) const noexcept;
    std::string PullRequest() const noexcept;

    void HandleRequest(std::string&& json, const json_req_t& type) noexcept;

    template<typename T>
    T ParseJsonParam(const std::string& jsonReq, std::string&& sparam);

    template<typename T>
    T ParseTreeParam(const boost::property_tree::ptree& jsonTree, std::string&& sparam);

    std::string ConvertToString(const boost::property_tree::ptree& jsonTree) const noexcept;

private:
    mutable std::queue<std::string> msgQueue;
    mutable std::shared_mutex mtx_;
};