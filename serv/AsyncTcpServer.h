/*****************************************************************
 *  @file       AsyncTcpServer.h
 *  @brief      Async TCP server class declaration
 *  @author     Kalmykov Dmitry
 *  @date       28.04.2021
 *  @modified   19.08.2021
 *  @version    1.0
 */
#pragma once

/* std C++ lib headers */
#include <iostream>
#include <unordered_set>
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
#include <boost/asio/ssl.hpp>

/* local C++ headers */
#include "../config/config.h"
#include "../conn/AsyncClient.h"
#include "../conn/AsyncTcpConnection.h"

class AsyncTcpServer {

private:

    boost::asio::io_service& io_service;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ssl::context context_;

    void HandleAccept(AsyncClient::client_ptr& client,
        const boost::system::error_code& error);
    void StartAccept();

public:

    AsyncTcpServer() = delete;
    AsyncTcpServer(const AsyncTcpServer&) = delete;
    AsyncTcpServer(const AsyncTcpServer&&) = delete;
    AsyncTcpServer& operator=(const AsyncTcpServer&) = delete;
    AsyncTcpServer&& operator=(const AsyncTcpServer&&) = delete;
    AsyncTcpServer(boost::asio::io_service&& io_service, uint16_t port);

    ~AsyncTcpServer() { std::cout << "AsyncTcpServer destructor\n"; }

    static void StartTcpServer(boost::asio::io_service& ios);
    static void StopTcpServer(boost::asio::io_service& ios);
};