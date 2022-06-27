/*****************************************************************
 *  @file       MongoProcess.cpp
 *  @brief      MongoDB connection handler implementation
 *  @author     Kalmykov Dmitry
 *  @date       15.02.2022
 *  @version    0.1
 */

#include "MongoProcess.h"

#include "../format/json.h"
#include "../log/Logger.h"

#include <boost/date_time.hpp>
#include <boost/format.hpp>

#include <spdlog/spdlog.h>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

void MongoProcessor::InitializeConnection(std::string&& config) noexcept {
    mongocxx::instance instance{};
    try
    {
        dbcfg = std::make_shared<IConfig>();
        dbcfg->Open(std::move(config));
        connectingString_ = dbcfg->GetConfigValueByKey("connstr");
    } catch (std::exception const& ex) {
        MongoError(boost::str(boost::format("%1% %2%") % "Initialize MongDB connection error: " % ex.what()));
    }
}

MongoProcessor::MongoProcessor(std::string&& connectingConfig) {
    std::cout << "Construct MongoProcessor class\n";
    InitializeConnection(std::move(connectingConfig));
}

MongoProcessor::~MongoProcessor() {
    std::cout << "Destruct MongoProcessor class\n";
}

void MongoProcessor::InsertNewMessage(const boost::property_tree::ptree& tree) noexcept {
    try {
        mongocxx::client client{mongocxx::uri{connectingString_}};
        mongocxx::database db = client["msgdb"];
        auto collection = std::make_unique<mongocxx::collection>(db["msgtable"]);
        Insert(std::move(collection), std::cref(tree));
    } catch (std::exception &ex) {
        MongoError(boost::str(boost::format("%1% %2%") % "Insert message error: " % ex.what()));
    }
}

#include <boost/date_time/posix_time/posix_time.hpp>

void MongoProcessor::Insert(std::unique_ptr<mongocxx::collection> collection, const boost::property_tree::ptree& tree) noexcept {
    auto builder = bsoncxx::builder::stream::document{};

    std::unique_ptr<JsonHandler> handle = std::make_unique<JsonHandler>();
    auto toId = handle->ParseTreeParam<std::string>(tree, JsonHandler::dst_user_msg_token);
    auto fromId = handle->ParseTreeParam<std::string>(tree, JsonHandler::src_user_msg_token);
    auto message = handle->ParseTreeParam<std::string>(tree, JsonHandler::user_msg_token);
    auto timestamp = handle->ParseTreeParam<std::string>(tree, JsonHandler::msg_timestamp_token);

    bsoncxx::document::value doc_value = builder
    << JsonHandler::dst_user_msg_token << toId 
    << JsonHandler::src_user_msg_token << fromId
    << JsonHandler::user_msg_token << message
    << JsonHandler::msg_timestamp_token << timestamp
    << bsoncxx::builder::stream::finalize;

    bsoncxx::document::view view = doc_value.view();
    
    bsoncxx::stdx::optional<mongocxx::result::insert_one> result = collection->insert_one(view);
    if(!result) {
        spdlog::error("MongoDB insert function is failed");
        // TODO:
    }
}

void MongoProcessor::MongoLog(std::string&& logMsg) {
    if(MongoProcessor::config_ == MongoProcessor::ConfigClass::debug) {
        ConsoleLogger::Info(std::move(logMsg));
    }    
}

void MongoProcessor::MongoError(std::string&& errMsg) {
    ConsoleLogger::Error(std::move(errMsg));
}
