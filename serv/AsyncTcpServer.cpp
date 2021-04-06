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

ConnectionManager<uint64_t> connMan_;

ConsoleLogger serverLogger;

/***********************************************************************************
 *  @brief  Callback-handler of async accepting process
 *  @param  new_connection Shared pointer to new connection of client
 *  @param  error Boost system error object reference
 *  @return None
 */
void AsyncTcpServer::handle_accept(AsyncTcpConnection::connection_ptr new_connection,
    boost::system::error_code error)
{
    if (!error)
    {
        serverLogger.Write("New connection accepted. Start reading data.\n");
        new_connection->StartAuth();
    }
    else {
        shutdown(boost::asio::ip::tcp::socket::shutdown_send, error.value());
    }
    start_accept();
}

/***********************************************************************************
 *  @brief  Start async assepting process in socket
 *  @return None
 */
void AsyncTcpServer::start_accept() {

    uint64_t connId = connMan_.GetFreeId();

#if SECURE
    AsyncTcpConnection::connection_ptr new_connection =
        AsyncTcpConnection::create(io_service_, context_, connId);
#else 
    async_tcp_connection::connection_ptr new_connection =
        async_tcp_connection::create(io_service_, connId);
#endif /* SECURE */
    
    connMan_.CreateNewConnection(connId, new_connection);

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&AsyncTcpServer::handle_accept, this, new_connection,
            boost::asio::placeholders::error));
}

/***********************************************************************************
 *  @brief  Async server constructor
 */
AsyncTcpServer::AsyncTcpServer(boost::asio::io_service& io_service, uint16_t port) :
    io_service_(io_service),
#if SECURE
    acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    context_(boost::asio::ssl::context::sslv23)
#else
    acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
#endif /* SECURE */
{
    std::cout << "Construct AsyncTcpServer class\n";
    serverLogger.Write(boost::str(boost::format("Start listening to %1% port\n") % port));

#if SECURE
    context_.set_options(boost::asio::ssl::context::default_workarounds |
        boost::asio::ssl::context::no_sslv2 |
        boost::asio::ssl::context::single_dh_use);

    //context_.set_password_callback(boost::bind(&async_tcp_server::get_password, this));
    context_.use_certificate_chain_file("user.crt");
    context_.use_private_key_file("user.key", boost::asio::ssl::context::pem);
    context_.use_tmp_dh_file("dh2048.pem");
#endif /* SECURE */

    start_accept();
}

static IConfig scfg;

/***********************************************************************************
 *  @brief  Config and start async TCP server on "host:port"
 *  @return None
 */
void AsyncTcpServer::StartTcpServer() {

    try {
        /* open db config file */
        scfg.Open("server.ini");
        std::stringstream sport(scfg.GetRecordByKey("port"));
        uint16_t port;
        sport >> port;

        /* start tcp server */
        boost::asio::io_service ios;
        //std::unique_ptr<AsyncTcpServer> serv = std::make_unique<AsyncTcpServer>(ios, port);
        AsyncTcpServer serv(ios, port);
        ios.run();
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
}

/***********************************************************************************
 *  @brief  Config and start async TCP server on "host:port"
 *  @return None
 */
void AsyncTcpServer::StopTcpServer() {
    connMan_.CloseAllConnection();
}
