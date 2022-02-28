/*********************************************
 *
 *
 */

 /* std C++ lib headers */
#include <iostream>
#include <unordered_set>
#include <memory>
#include <utility>
#include <algorithm>
#include <mutex>
#include <shared_mutex>

/* boost C++ lib headers */
#include <boost/format.hpp>
#include <boost/asio.hpp> 
#include <boost/array.hpp> 
#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/date_time.hpp>

/* local C++ headers */
#include "AsyncTcpConnection.h"
#include "../log/Logger.h"
#include "../conn/ConnectionManager.h"
#include "../data/DataProcess.h"

AsyncTcpConnection::ssl_socket::lowest_layer_type& AsyncTcpConnection::socket() {
    return socket_.lowest_layer();
}

void AsyncTcpConnection::StartAuth() {

    socket_.async_handshake(boost::asio::ssl::stream_base::server,
        [&](const boost::system::error_code& error) {
            HandleHandshake(error);
        });
}

void AsyncTcpConnection::HandleHandshake(const boost::system::error_code& error) {

    std::cout << buf.data() << std::endl;

    if (!error) {
        socket_.async_read_some(boost::asio::buffer(buf),
            [&](const boost::system::error_code& error,
                std::size_t recvBytes) {
                    HandleAuth(error, recvBytes);
            });
    } else {
        ConsoleLogger::Info(boost::str(boost::format(
            "HandleHandshake error user: %1% \"%2%\"\n") % id_ % error.message()));
        Shutdown();
    }
}

void AsyncTcpConnection::HandleAuth(const boost::system::error_code& error,
    std::size_t recvBytes)
{
    if (!error)
    {
        std::string auth_message{ buf.data(), recvBytes };
        ConsoleLogger::Info(boost::str(boost::format("<< \"%1%\" [%2%]\n") % std::string{ buf.data(), recvBytes } % recvBytes));

        DataProcess::GetInstance()->PushNewMessage(std::cref(id_), std::move(auth_message));
        StartRead();
    }
    else {
        ConsoleLogger::Info(boost::str(boost::format(
            "Handle authentication error user: %1% \"%2%\"\n") % id_ % error.message()));
        Shutdown();
    }
}

void AsyncTcpConnection::StartRead()
{
    socket_.async_read_some(boost::asio::buffer(buf),
        [&](const boost::system::error_code& error,
            std::size_t recvBytes) {
                HandleRead(error, recvBytes);
        });
}

void AsyncTcpConnection::HandleRead(const boost::system::error_code& error,
    std::size_t recvBytes)
{
    if (!error)
    {
        std::string in_msg{ buf.data(), recvBytes };
        ConsoleLogger::Info(boost::str(boost::format("<< \"%1%\" [%2%]\n") % std::string{ buf.data(), recvBytes } % recvBytes));

        to_lower(std::move(in_msg.data()));

        DataProcess::GetInstance()->PushNewMessage(std::cref(id_), std::move(in_msg));
        StartRead();
    }
    else {
        ConsoleLogger::Info(boost::str(boost::format(
            "HandleRead error user: %1% \"%2%\"\n") % id_ % error.message()));
        Shutdown();
    }
}

void AsyncTcpConnection::StartWriteMessage(const std::string& msg)
{
    socket_.async_write_some(boost::asio::buffer(msg),
        [&](const boost::system::error_code& error,
            std::size_t bytes_transferred) {
                std::cout << "Message sended\n";
        });
}

void AsyncTcpConnection::Shutdown() {
    socket_.async_shutdown(
        [&](const boost::system::error_code& error) {
            Close(error);
        });
}

void AsyncTcpConnection::Close(const boost::system::error_code& error) {
    ConsoleLogger::Info(boost::str(boost::format("Close connection user: %1% \n") % id_));
    socket_.next_layer().close();
    ConnectionManager::GetInstance()->RemoveConnection(id_);
}