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

class AsyncTcpSession {
    
};

class AsyncTcpConnection
{
public:

    using id_t = uint32_t;
    using ssl_socket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
    using connection_ptr = std::unique_ptr<AsyncTcpConnection>;

    static connection_ptr create(boost::asio::io_service& io_service,
        boost::asio::ssl::context& context, const id_t& id)
    {
        return std::make_unique<AsyncTcpConnection>(io_service, context, id);
    }


    ssl_socket::lowest_layer_type& socket();
    void StartAuth();

    const id_t GetId() const noexcept {
        return id_;
    }

    AsyncTcpConnection() = delete;
    AsyncTcpConnection(const AsyncTcpConnection&) = delete;
    AsyncTcpConnection& operator=(const AsyncTcpConnection&) = delete;
    AsyncTcpConnection(const AsyncTcpConnection&&) = delete;
    AsyncTcpConnection&& operator=(const AsyncTcpConnection&&) = delete;

    AsyncTcpConnection(boost::asio::io_service& io_service,
        boost::asio::ssl::context& context_, const id_t& id)
        : socket_(io_service, context_),
        id_(id)
    {
        std::cout << "Construct AsyncTcpConnection class for user ID = " << id_ << "\n";
    }

    ~AsyncTcpConnection() {
        std::cout << "Destruct AsyncTcpConnection class for user ID = " << id_ << "\n";
    }

    void StartWriteMessage(const std::string& msg);

private:

    void Close(const boost::system::error_code& error);
    void Shutdown();
    void HandleHandshake(const boost::system::error_code& error);
    void HandleAuth(const boost::system::error_code& error,  std::size_t recvBytes);
    void StartRead();
    void HandleRead(const boost::system::error_code& error, std::size_t recvBytes);
        
    void to_lower(std::string&& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }

    const std::string hello_msg{ "hello user id=" };
    const std::string hello_msg_header{ "hello server" };

    ssl_socket socket_;
    id_t id_;

    enum { max_length = 1024 };
    std::array<char, max_length> buf = { { 0 } };
};
