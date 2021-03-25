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

class async_tcp_connection
{
public:

    /* alias for shared pointer to tcp connectio class */
    typedef boost::shared_ptr<async_tcp_connection> connection_ptr;

    /***********************************************************************************
     *  @brief  Static func to create new connection tcp object and return its reference
     *  @param  io_service  Reference to boost io_service object
     *  @param  id New client id
     *  @return Reference to tcp connection object
     */
    static connection_ptr create(boost::asio::io_service& io_service, uint64_t id)
    {
        return connection_ptr(new async_tcp_connection(io_service, id));
    }


    /***********************************************************************************
     *  @brief  Getter for tcp connection socket reference
     *  @return Reference to tcp connection socket
     */
    boost::asio::ip::tcp::socket& socket();

    /***********************************************************************************
     *  @brief  Start process authentication of client
     *  @return None
     */
    void start_auth();

    /* constructor */
    async_tcp_connection(boost::asio::io_service& io_service_, uint64_t id)
        : socket_(io_service_),
        id_(id)
    {

    }

    /* destructor */
    ~async_tcp_connection() {
        //logger.write("Close current connection\n");
    }

private:

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
    boost::asio::ip::tcp::socket socket_;

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
