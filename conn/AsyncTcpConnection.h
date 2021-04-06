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
// openssl
#include <boost/asio/ssl.hpp>
#endif /* SECURE */

class async_tcp_connection
{
public:
#if SECURE
    /* alias for ssl stream to tcp socket */
    using ssl_socket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
#endif /* SECURE */
    /* alias for shared pointer to tcp connectio class */
    using connection_ptr = boost::shared_ptr<async_tcp_connection>;

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
        return connection_ptr(new async_tcp_connection(io_service, context, id));
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
    void start_auth();

#if SECURE
    async_tcp_connection(boost::asio::io_service& io_service_,
        boost::asio::ssl::context& context_, uint64_t id)
        : socket_(io_service_, context_),
        id_(id)
#else 
    async_tcp_connection(boost::asio::io_service& io_service_, uint64_t id)
        : socket_(io_service_),
        id_(id)
#endif /* SECURE */
    {

    }

    ~async_tcp_connection() {
        /* ... */
    }

private:


#if SECURE
    /***********************************************************************************
     *  @brief  Callback-handler of async handshake process
     *  @param  error Boost system error object reference
     *  @return None
     */
    void handle_handshake(const boost::system::error_code& error);
#endif /* SECURE */

    /***********************************************************************************
    *  @brief  Close tcp connection and call destructor
    *  @param  error Boost system error object reference
    *  @return None
    */
    void close(const boost::system::error_code& error);


    void to_lower(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }

    /***********************************************************************************
    *  @brief  Callback-handler of async authentication process
    *  @param  error Boost system error object reference
    *  @param  recvBytes Amount of bytes received from connection
    *  @return None
    */
    void handle_auth(const boost::system::error_code& error,
    std::size_t recvBytes);

    /***********************************************************************************
    *  @brief  Start async reading process from socket
    *  @param  None
    *  @return None
    */
    void start_read();

    /***********************************************************************************
    *  @brief  Callback-handler of async reading process
    *  @param  error Boost system error object reference
    *  @param  recvBytes Amount of bytes received from connection
    *  @return None
    */
    void handle_read(const boost::system::error_code& error,
    std::size_t recvBytes);

    /***********************************************************************************
    *  @brief  Start async writing process from socket
    *  @param  value Average of squares summ from set (container)
    *  @return None
    */
    void start_write(uint64_t value);

    /***********************************************************************************
    *  @brief  Callback-handler of async writing process
    *  @param  error Boost system error object reference
    *  @return None
    */
    void handle_write(const boost::system::error_code& error);

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
    boost::array<char, max_length> buf = { { 0 } };
};
