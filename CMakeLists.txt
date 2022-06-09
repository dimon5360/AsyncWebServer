cmake_minimum_required(VERSION 3.10)

project(asyncWebServer)

set(CMAKE_CXX_STANDARD 20)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost REQUIRED COMPONENTS chrono filesystem thread system regex date_time)
find_package(spdlog REQUIRED)

link_directories(/usr/local/lib)
include_directories(/usr/local/include)
include_directories(${Boost_INCLUDE_DIRS})

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

# find_package(RdKafka)
find_package(CppKafka REQUIRED)
find_package(mongocxx REQUIRED)

find_library(PQXX_LIB pqxx)
find_library(PQ_LIB pq)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lboost_date_time -pthread -std=c++2a -lpqxx -lpq -lmongocxx -lbsoncxx")

set(SRC_LIST main.cpp 
    config/config.cpp 
    conn/AsyncClient.cpp 
    conn/AsyncTcpConnection.cpp 
    crypto/dh.cpp 
    crypto/rsa.cpp 
    data/DataProcess.cpp 
    data/UsersPool.cpp 
    db/PostgresProcess.cpp 
    db/MongoProcess.cpp 
    db/KafkaProcess.cpp
    log/Logger.cpp 
    serv/AsyncTcpServer.cpp 
    test/tests.cpp 
    format/json.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ${Boost_DATE_TIME_LIBRARY} OpenSSL::Crypto OpenSSL::SSL ${PQXX_LIB} ${PQ_LIB} mongo::mongocxx_shared CppKafka::cppkafka)