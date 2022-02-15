/*****************************************************************
 *  @file       AsyncTcpServer.cpp
 *  @brief      Async TCP server class implementation
 *  @author     Kalmykov Dmitry
 *  @date       28.04.2021
 *  @modified   19.08.2021
 *  @version    1.0
 */

#include <iostream>
#include <unordered_map>
#include <memory>
#include <utility>
#include <algorithm>
#include <cstdint>
#include <thread>

#include <boost/format.hpp>
#include <boost/asio.hpp> 
#include <boost/array.hpp> 
#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/date_time.hpp>

#include "AsyncTcpServer.h"
#include "../log/Logger.h"
#include "../conn/ConnectionManager.h"
#include "../conn/AsyncClient.h"

ConnectionManager connMan_;

void AsyncTcpServer::HandleAccept(AsyncClient::client_ptr& client,
    const boost::system::error_code& error)
{
    if (!error)
    {
        ConsoleLogger::Info("New connection accepted. Start reading data.\n");
        connMan_.AddConnection(std::ref(client));
        client->HandleAccept();
        StartAccept();
    }
    else {
        client->DisconnectClient();
    }
}

void AsyncTcpServer::StartAccept() {
        
    auto client = connMan_.CreateNewClient(io_service, context_);

    std::cout << "Current thread ID = " << std::this_thread::get_id();
    ConsoleLogger::Info(boost::str(boost::format("Current thread ID = %1% \n") %
        std::this_thread::get_id()));

    acceptor_.async_accept(client->socket(),
        std::bind(&AsyncTcpServer::HandleAccept, this, client,
            std::placeholders::_1));
}

AsyncTcpServer::AsyncTcpServer(boost::asio::io_service&& io_service, uint16_t port) :
    io_service(std::ref(io_service)),
    acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    context_(boost::asio::ssl::context::tlsv13)
{
    ConsoleLogger::Info("AsyncTcpServer constructor");

    ConsoleLogger::Info(boost::str(boost::format("Start listening to %1% port") % port));

    context_.set_options(boost::asio::ssl::context::default_workarounds |
        boost::asio::ssl::context::no_sslv2|
        boost::asio::ssl::context::no_sslv3);

    context_.use_certificate_chain_file("user.crt");
    context_.use_private_key_file("user.key", boost::asio::ssl::context::pem);

    StartAccept();
    io_service.run();
}

void AsyncTcpServer::StartTcpServer(boost::asio::io_service &ios) {

    try {
        /* open db config file */
        auto scfg = std::make_shared<IConfig>();
        scfg->Open("./server.ini");
        auto sport = scfg->GetRecordByKey("port");
        uint16_t port = std::atoi(sport.c_str());
        std::cout << "Start TCP server...\n";
        std::make_unique<AsyncTcpServer>(std::move(ios), port);
    }
    catch (std::exception& ex) {
        ConsoleLogger::Info(boost::str(boost::format("StartTcpServer exception: %1%\n") % ex.what()));
    }
}

void AsyncTcpServer::StopTcpServer(boost::asio::io_service& ios) {
    connMan_.DeactivateManager();
    connMan_.CloseAllConnections();
}
