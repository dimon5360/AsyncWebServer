/*****************************************************************
 *  @file       MongoProcess.cpp
 *  @brief      MongoDB connection handler implementation
 *  @author     Kalmykov Dmitry
 *  @date       15.02.2022
 *  @version    0.1
 */

#include "MongoProcess.h"

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

// connecting string "mongodb://localhost:27017"

void MongoProcessor::InitializeConnection() const noexcept {
    mongocxx::instance instance{}; // This should be done only once.
    //mongoclient_ = std::make_shared<mongocxx::client>(mongocxx::uri{connectingString_});
}

MongoProcessor::MongoProcessor(std::string&& connectingString) :
    connectingString_(std::move(connectingString)) {
    Log("MongoProcess class constructor\n");  
    InitializeConnection();
}

MongoProcessor::MongoProcessor(const std::string& connectingString) :
    connectingString_(connectingString) {
    Log("MongoProcess class constructor\n");  
    InitializeConnection();
}

MongoProcessor::~MongoProcessor() {
    Log("MongoProcess class destructor\n");    
}


void MongoProcessor::testInsert(std::string&& dbs, std::string&& table) const noexcept {
    mongocxx::client client{mongocxx::uri{connectingString_}};
    mongocxx::database db = client[dbs];
    mongocxx::collection collection = db[table];
    bsoncxx::stdx::optional<mongocxx::result::insert_one> result =
        collection.insert_one(bsoncxx::builder::stream::document{} << "i" << 10 << finalize);
}


void MongoProcessor::testRequest(std::string&& dbs, std::string&& table) const noexcept {
    mongocxx::client client{mongocxx::uri{connectingString_}};
    mongocxx::database db = client[dbs];
    mongocxx::collection collection = db[table];
    bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result =
    collection.find_one(document{} << "i" << 10 << finalize);
    if(maybe_result) {
        std::cout << bsoncxx::to_json(*maybe_result) << "\n";
    }
}

void MongoProcessor::Log(std::string&& log) {
    if(MongoProcessor::config_ == MongoProcessor::ConfigClass::debug) {
        std::cout << log;
    }    
}

