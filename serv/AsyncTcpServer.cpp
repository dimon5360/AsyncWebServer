/*********************************************
 *
 *
 */

/* std C++ lib headers */
#include <iostream>
#include <unordered_map>
#include <memory>
#include <utility>
#include <algorithm>
#include <cstdint>
#include <thread>

/* boost C++ lib headers */
#include <boost/format.hpp>
#include <boost/asio.hpp> 
#include <boost/array.hpp> 
#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/date_time.hpp>

/* local C++ headers */
#include "AsyncTcpServer.h"
#include "../log/Logger.h"
#include "../conn/ConnectionManager.h"

ConnectionManager connMan_;

ConsoleLogger serverLogger;

/***********************************************************************************
 *  @brief  Callback-handler of async accepting process
 *  @param  new_connection Shared pointer to new connection of client
 *  @param  error Boost system error object reference
 *  @return None
 */
void AsyncTcpServer::HandleAccept(AsyncTcpConnection::connection_ptr& new_connection,
    const boost::system::error_code& error)
{
    if (!error)
    {
        serverLogger.Write("New connection accepted. Start reading data.\n");
        new_connection->StartAuth();
        connMan_.CreateNewConnection(new_connection->GetId(), new_connection);
        StartAccept();
    }
    else {
        new_connection->socket().close();
    }
}

/***********************************************************************************
 *  @brief  Start async assepting process in socket
 *  @return None
 */
void AsyncTcpServer::StartAccept() {

    uint64_t connId = connMan_.GetFreeId();

    AsyncTcpConnection::connection_ptr new_connection =
        AsyncTcpConnection::create(io_service, context_, connId);

    std::cout << "Current thread ID = " << std::this_thread::get_id();
    serverLogger.Write(boost::str(boost::format("Current thread ID = %1% \n") % 
        std::this_thread::get_id()));

    acceptor_.async_accept(new_connection->socket(),
        std::bind(&AsyncTcpServer::HandleAccept, this, new_connection,
            std::placeholders::_1));
}

/***********************************************************************************
 *  @brief  Async server constructor
 */
AsyncTcpServer::AsyncTcpServer(boost::asio::io_service&& io_service, uint16_t port) :
    io_service(std::ref(io_service)),
    acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    context_(boost::asio::ssl::context::tlsv13)
{
    serverLogger.Write("AsyncTcpServer constructor\n");

    std::cout << "AsyncTcpServer constructor\n";

    serverLogger.Write(boost::str(boost::format("Start listening to %1% port\n") % port));

    context_.set_options(boost::asio::ssl::context::default_workarounds |
        boost::asio::ssl::context::no_sslv2|
        boost::asio::ssl::context::no_sslv3);

    context_.use_certificate_chain_file("user.crt");
    context_.use_private_key_file("user.key", boost::asio::ssl::context::pem);

    StartAccept();
    io_service.run();
}

static IConfig scfg;

/***********************************************************************************
 *  @brief  Config and start async TCP server on "host:port"
 *  @return None
 */
void AsyncTcpServer::StartTcpServer(boost::asio::io_service &ios) {

    try {
        /* open db config file */
        scfg.Open("server.ini");
        uint16_t port = boost::lexical_cast<uint16_t>(scfg.GetRecordByKey("port"));
        std::make_unique<AsyncTcpServer>(std::move(ios), port);
    }
    catch (std::exception& ex) {
        serverLogger.Write(boost::str(boost::format("StartTcpServer exception: %1%\n") % ex.what()));
    }
}

/***********************************************************************************
 *  @brief  Close all connections and stop async TCP server io service
 *  @return None
 */
void AsyncTcpServer::StopTcpServer(boost::asio::io_service& ios) {
    connMan_.CloseAllConnections();
}
