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

#define SECURE 1
#if SECURE
#include <boost/asio/ssl.hpp>
#endif /* SECURE */

class AsyncTcpConnection
{
public:
#if SECURE
    /* alias for ssl stream to tcp socket */
    using ssl_socket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
#endif /* SECURE */
    /* alias for shared pointer to tcp connectio class */
    using connection_ptr = std::shared_ptr<AsyncTcpConnection>;

    /***********************************************************************************
     *  @brief  Static func to create new connection tcp object and return its reference
     *  @param  io_service  Reference to boost io_service object
     *  @param  id New client id
     *  @return Reference to tcp connection object
     */
#if SECURE
    static connection_ptr create(boost::asio::io_service& io_service,
        boost::asio::ssl::context& context, uint64_t id)
    {
        return connection_ptr(new AsyncTcpConnection(io_service, context, id));
    }
#else 
    static connection_ptr create(boost::asio::io_service& io_service, uint64_t id)
    {
        return connection_ptr(new async_tcp_connection(io_service, id));
    }
#endif /* SECURE */


    /***********************************************************************************
     *  @brief  Getter for tcp connection socket reference
     *  @return Reference to tcp connection socket
     */
#if SECURE
    ssl_socket::lowest_layer_type& socket();
#else 
    boost::asio::ip::tcp::socket& socket();
#endif /* SECURE */

    /***********************************************************************************
     *  @brief  Start process authentication of client
     *  @return None
     */
    void StartAuth();

#if SECURE
    AsyncTcpConnection(boost::asio::io_service& io_service_,
        boost::asio::ssl::context& context_, uint64_t id)
        : socket_(io_service_, context_),
        id_(id)
#else 
    async_tcp_connection(boost::asio::io_service& io_service_, uint64_t id)
        : socket_(io_service_),
        id_(id)
#endif /* SECURE */
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


#if SECURE
    /***********************************************************************************
     *  @brief  Callback-handler of async handshake process
     *  @param  error Boost system error object reference
     *  @return None
     */
    void HandleHandshake(const boost::system::error_code& error);
#endif /* SECURE */


    void to_lower(std::string& str) {
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
#if SECURE
    ssl_socket socket_;
#else 
    boost::asio::ip::tcp::socket socket_;
#endif /* SECURE */

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
