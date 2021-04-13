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
#include <boost/asio/ssl.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>

class AsyncTcpConnection
{
public:
    /* alias for ssl stream to tcp socket */
    using ssl_socket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
    /* alias for shared pointer to tcp connectio class */
    using connection_ptr = std::shared_ptr<AsyncTcpConnection>;

    /***********************************************************************************
     *  @brief  Static func to create new connection tcp object and return its reference
     *  @param  io_service  Reference to boost io_service object
     *  @param  id New client id
     *  @return Reference to tcp connection object
     */
    static connection_ptr create(boost::asio::io_service& io_service,
        boost::asio::ssl::context& context, uint64_t id)
    {
        return connection_ptr(new AsyncTcpConnection(io_service, context, id));
    }


    /***********************************************************************************
     *  @brief  Getter for tcp connection socket reference
     *  @return Reference to tcp connection socket
     */
    ssl_socket::lowest_layer_type& socket();

    /***********************************************************************************
     *  @brief  Start process authentication of client
     *  @return None
     */
    void StartAuth();

    AsyncTcpConnection(boost::asio::io_service& io_service_,
        boost::asio::ssl::context& context_, uint64_t id)
        : socket_(io_service_, context_),
        id_(id)
    {
        std::cout << "Construct AsyncTcpConnection class\n";
    }

    ~AsyncTcpConnection() {
        std::cout << "Destruct AsyncTcpConnection class for user ID = " << id_ << "\n";
    }

    uint64_t GetId() const {
        return id_;
    }

private:

    /***********************************************************************************
    *  @brief  Close tcp connection and call destructor
    *  @param  error Boost system error object reference
    *  @return None
    */
    void Close(const boost::system::error_code& error);

    /***********************************************************************************
    *  @brief  Send shutdown request (close notify)
    *  @return None
    */
    void Shutdown();


    /***********************************************************************************
     *  @brief  Callback-handler of async handshake process
     *  @param  error Boost system error object reference
     *  @return None
     */
    void HandleHandshake(const boost::system::error_code& error);


    void to_lower(std::string&& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }

    /***********************************************************************************
    *  @brief  Callback-handler of async authentication process
    *  @param  error Boost system error object reference
    *  @param  recvBytes Amount of bytes received from connection
    *  @return None
    */
    void HandleAuth(const boost::system::error_code& error,
        std::size_t recvBytes);

    /***********************************************************************************
    *  @brief  Start async reading process from socket
    *  @param  None
    *  @return None
    */
    void StartRead();

    /***********************************************************************************
    *  @brief  Callback-handler of async reading process
    *  @param  error Boost system error object reference
    *  @param  recvBytes Amount of bytes received from connection
    *  @return None
    */
    void HandleRead(const boost::system::error_code& error,
        std::size_t recvBytes);

    /***********************************************************************************
    *  @brief  Start async writing process from socket
    *  @param  value Average of squares summ from set (container)
    *  @return None
    */
    void StartWrite(uint64_t value);

    /***********************************************************************************
    *  @brief  Callback-handler of async writing process
    *  @param  error Boost system error object reference
    *  @return None
    */
    void HandleWrite(const boost::system::error_code& error);

    /* tcp socket object */
    ssl_socket socket_;

    /* msgs headers to exchange with clients */
    const std::string hello_msg = std::string("hello user id=");
    const std::string tech_msg_header = std::string("user id=");
    const std::string tech_req_msg = std::string("number=");
    const std::string tech_resp_msg = std::string("summ=");

    /* unique id of client */
    uint64_t id_;

    /* exchange data buffer */
    enum { max_length = 1024 };
    std::array<char, max_length> buf = { { 0 } };
};
