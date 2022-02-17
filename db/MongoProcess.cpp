/*****************************************************************
 *  @file       MongoProcess.cpp
 *  @brief      MongoDB connection handler implementation
 *  @author     Kalmykov Dmitry
 *  @date       15.02.2022
 *  @version    0.1
 */

#include "MongoProcess.h"

#include "../format/json.h"

#include <boost/date_time.hpp>
#include <boost/format.hpp>

#include <spdlog/spdlog.h>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

void MongoProcessor::InitializeConnection(const std::string config) noexcept {
    mongocxx::instance instance{};
    try
    {
        dbcfg = std::make_shared<IConfig>();
        dbcfg->Open("mongo.ini");
        connectingString_ = dbcfg->GetConfigValueByKey("connstr");
    } catch (std::exception const& ex) {
        MongoError(boost::str(boost::format("%1% %2%") % "Initialize MongDB connection error: " % ex.what()));
    }
}

MongoProcessor::MongoProcessor(std::string&& connectingConfig) :
    connectingConfig_(std::move(connectingConfig))
{
    MongoLog("Construct MongoProcess class\n");  
    InitializeConnection(connectingConfig_);
}

MongoProcessor::~MongoProcessor() {
    MongoLog("Destruct MongoProcess class\n");    
}

void MongoProcessor::InsertNewMessage(std::string&& msg) noexcept {
    try {
        mongocxx::client client{mongocxx::uri{connectingString_}};
        mongocxx::database db = client[dbcfg->GetConfigValueByKey("msgdb")];
        auto collection = std::make_unique<mongocxx::collection>(db[dbcfg->GetConfigValueByKey("msgtable")]);
        Insert(std::move(collection), std::cref(msg));
    } catch (std::exception &ex) {
        MongoError(boost::str(boost::format("%1% %2%") % "Insert message error: " % ex.what()));
    }
}

#include <boost/date_time/posix_time/posix_time.hpp>

void MongoProcessor::Insert(std::unique_ptr<mongocxx::collection> collection, const std::string& json) noexcept {
    auto builder = bsoncxx::builder::stream::document{};

    std::unique_ptr<JsonHandler> handle = std::make_unique<JsonHandler>();
    auto tree = handle->ConstructTree(json);

    auto userId = handle->ParseTreeParam<std::string>(tree, "user id");
    auto message = handle->ParseTreeParam<std::string>(tree, "message");

    bsoncxx::document::value doc_value = builder 
    << "user_id" << userId 
    << "message" << message
    << "timestamp" << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time())
    << bsoncxx::builder::stream::finalize;

    bsoncxx::document::view view = doc_value.view();
    
    bsoncxx::stdx::optional<mongocxx::result::insert_one> result = collection->insert_one(view);
    if(!result) {
        // TODO:
    }
}

void MongoProcessor::Insert(std::unique_ptr<mongocxx::collection> collection, const boost::property_tree::ptree& tree) noexcept {
    auto builder = bsoncxx::builder::stream::document{};

    std::unique_ptr<JsonHandler> handle = std::make_unique<JsonHandler>();
    auto userId = handle->ParseTreeParam<std::string>(tree, "user id");
    auto message = handle->ParseTreeParam<std::string>(tree, "message");

    bsoncxx::document::value doc_value = builder 
    << "user_id" << userId 
    << "message" << message 
    << bsoncxx::builder::stream::finalize;

    bsoncxx::document::view view = doc_value.view();
    
    bsoncxx::stdx::optional<mongocxx::result::insert_one> result = collection->insert_one(view);
    if(!result) {
        // TODO:
    }
}

// void MongoProcessor::testRequest(std::string&& dbs, std::string&& table) noexcept {
//     mongocxx::client client{mongocxx::uri{connectingString_}};
//     mongocxx::database db = client[dbs];
//     auto collection = std::make_unique<mongocxx::collection>(db[table]);
//     bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result =
//     collection->find_one(document{} << "i" << 10 << finalize);
//     if(maybe_result) {
//         std::cout << bsoncxx::to_json(*maybe_result) << "\n";
//     }
// }

void MongoProcessor::MongoLog(std::string&& logMsg) {
    if(MongoProcessor::config_ == MongoProcessor::ConfigClass::debug) {
        spdlog::info(logMsg);
    }    
}

void MongoProcessor::MongoError(std::string&& errMsg) {
    spdlog::error(errMsg);
    // TODO: add file logger record
}
