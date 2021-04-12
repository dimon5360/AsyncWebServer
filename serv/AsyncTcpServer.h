/*********************************************
 *
 *
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
// openssl
#include <boost/asio/ssl.hpp>

/* local C++ headers */
#include "../conn/AsyncTcpConnection.h"
#include "../config/config.h"

class AsyncTcpServer {

private:


    /* boost io_service object reference */
    boost::asio::io_service& io_service_;
    /* boost acceptor object */
    boost::asio::ip::tcp::acceptor acceptor_;
#if SECURE
    /* boost ssl context */
    boost::asio::ssl::context context_;
#endif /* SECURE */
    /* hash map to keep clients connection pointers */
    mutable std::unordered_map<uint32_t, AsyncTcpConnection::connection_ptr> clientMap;


    /***********************************************************************************
     *  @brief  Callback-handler of async accepting process
     *  @param  new_connection Shared pointer to new connection of client
     *  @param  error Boost system error object reference
     *  @return None
     */
    void HandleAccept(AsyncTcpConnection::connection_ptr new_connection,
        const boost::system::error_code error);

    /***********************************************************************************
     *  @brief  Start async assepting process in socket
     *  @return None
     */
    void StartAccept();

    AsyncTcpServer();

public:

    static void StartTcpServer(boost::asio::io_service& ios);
    static void StopTcpServer(boost::asio::io_service& ios);

    AsyncTcpServer(boost::asio::io_service& io_service, uint16_t port);
    ~AsyncTcpServer() {
        std::cout << "Destruct AsyncTcpServer class\n";
    };
};