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

ConsoleLogger connectionLogger;
MessageBroker msgBroker;

template <class K>
class GuardedSet {
private:

    /* set (container) to keep unique random numbers from clients */
    std::unordered_set<K> _set;
    /* mutex object to avoid data race */
    mutable std::shared_mutex mutex_;

    K squaresSumm = 0; // keep last value of squares summ

    /* setter */
    void Set(const K& value) {
        std::unique_lock lock(mutex_);
        _set.insert(value);
    }

    /* check contains */
    bool IsContain(const K& value) {
        std::shared_lock lock(mutex_);
        return _set.contains(value);
    }

public:

    /***************************************************************
     *  @brief  Check that new number from client is unique
     *  @param  value Received random number from client
     *  @return Average of numbers squares summ
     */
    const K& GetAverage(const K&& value) noexcept {

        /* New random value is already in container
         * We don't need to calculate new average of numbers' squares */
        if (IsContain(value)) {
            return static_cast<const K&>(squaresSumm / _set.size());
        }
        /* New random value is unique
         * We need to calculate it */
        else {
            Set(value);
            squaresSumm += (value * value);
            return static_cast<const K&>(squaresSumm / _set.size());
        }
    }

    /* return dump of set data */
    const std::string& Dump() const noexcept {
        std::stringstream ss;

        std::shared_lock lock(mutex_);
        for (auto& v : _set) {
            ss << v << "\n";
        }

        return ss.str();
    }
};

GuardedSet<uint64_t> _gset;

/***********************************************************************************
*  @brief  Getter for tcp connection socket reference
*  @return Reference to tcp connection socket
*/
AsyncTcpConnection::ssl_socket::lowest_layer_type& AsyncTcpConnection::socket() {

    return socket_.lowest_layer();
}

/***********************************************************************************
 *  @brief  Start process authentication of client
 *  @return None
 */
void AsyncTcpConnection::StartAuth() {

    socket_.async_handshake(boost::asio::ssl::stream_base::server,
        [&](const boost::system::error_code& error) {
            HandleHandshake(error);
        });
}

/***********************************************************************************
 *  @brief  Callback-handler of async handshake process
 *  @param  error Boost system error object reference
 *  @return None
 */
void AsyncTcpConnection::HandleHandshake(const boost::system::error_code& error) {
    if (!error)
    {
        socket_.async_read_some(boost::asio::buffer(buf),
            [&](const boost::system::error_code& error,
                std::size_t recvBytes) {
                    HandleAuth(error, recvBytes);
            });
    }
    else
    {
        connectionLogger.Write(boost::str(boost::format(
            "HandleHandshake error user: %1% \"%2%\"\n") % id_ % error.message()));
        Shutdown();
    }
}

/***********************************************************************************
*  @brief  Send shutdown request (close notify)
*  @return None
*/
void AsyncTcpConnection::Shutdown() {
    socket_.async_shutdown([&](const boost::system::error_code& error) {
        Close(error);
        });
}

/***********************************************************************************
 *  @brief  Close tcp connection and call destructor
 *  @param  error Boost system error object reference
 *  @return None
 */
void AsyncTcpConnection::Close(const boost::system::error_code& error) {
    if (connMan_.Contains(id_))
    {
        connectionLogger.Write(boost::str(boost::format("Close connection user: %1% \n") % id_));
        socket_.next_layer().close();
        connMan_.RemoveConnection(id_);
    }
}

/***********************************************************************************
 *  @brief  Callback-handler of async authentication process
 *  @param  error Boost system error object reference
 *  @param  recvBytes Amount of bytes received from connection
 *  @return None
 */
void AsyncTcpConnection::HandleAuth(const boost::system::error_code& error,
    std::size_t recvBytes)
{
    if (!error)
    {
        std::string_view in_hello_msg{ buf.data(), recvBytes };
        connectionLogger.Write(boost::str(boost::format("<< \"%1%\" [%2%]\n") % std::string{ buf.data(), recvBytes } % recvBytes));

        if (in_hello_msg.starts_with("hello server")) {

            std::string resp{ boost::str(boost::format("%1%%2%") % hello_msg % id_) };
            connectionLogger.Write(boost::str(boost::format(">> \"%1%\" [%2%]\n") % resp % resp.size()));

            socket_.async_write_some(boost::asio::buffer(resp),
                [&](const boost::system::error_code& error,
                    std::size_t bytes_transferred) {
                        HandleWrite(error);
                });
        }
        else {
            connectionLogger.Write(boost::str(boost::format(
                "Invalid hello message from user %1%: \"%2%\"\n") % id_ % in_hello_msg));
            Shutdown();
        }
    }
    else {
        connectionLogger.Write(boost::str(boost::format(
            "Handle authentication error user: %1% \"%2%\"\n") % id_ % error.message()));
        Shutdown();
    }
}

/***********************************************************************************
 *  @brief  Start async reading process from socket
 *  @param  None
 *  @return None
 */
void AsyncTcpConnection::StartRead()
{
    socket_.async_read_some(boost::asio::buffer(buf),
        [&](const boost::system::error_code& error,
            std::size_t recvBytes) {
                HandleRead(error, recvBytes);
        });
}

/***********************************************************************************
 *  @brief  Callback-handler of async reading process
 *  @param  error Boost system error object reference
 *  @param  recvBytes Amount of bytes received from connection
 *  @return None
 */
void AsyncTcpConnection::HandleRead(const boost::system::error_code& error,
    std::size_t recvBytes)
{
    if (!error)
    {
#if CHAT
        std::string_view in_msg{ buf.data(), recvBytes };
        connectionLogger.Write(boost::str(boost::format("<< \"%1%\" [%2%]\n") % std::string{ buf.data(), recvBytes } % recvBytes));

        to_lower(std::move(in_msg.data()));

        if (in_msg.starts_with(tech_msg_header)) {
            auto item = in_msg.find(tech_req_msg);

            auto dstUserId = boost::lexical_cast<uint64_t>(in_msg.substr(tech_msg_header.size(), in_msg.find(",") - tech_msg_header.size()));
            std::cout << "Message from user #" << id_ << " for user #" << dstUserId << std::endl;

            std::string msg{ in_msg.substr(item + tech_req_msg.size()) };
            msgBroker.PushMessage(dstUserId, std::move(msg));
            //ResendMessage(dstUserId, msg);
            StartRead();
        }
#else 
        std::string_view in_msg{ buf.data(), recvBytes };
        connectionLogger.Write(boost::str(boost::format("<< \"%1%\" [%2%]\n") % std::string{ buf.data(), recvBytes } % recvBytes));

        to_lower(std::move(in_msg.data()));

        if (in_msg.starts_with(tech_msg_header)) {
            auto item = in_msg.find(tech_req_msg);
            int value = boost::lexical_cast<int>(in_msg.substr(item + tech_req_msg.size()));
            StartWrite(_gset.GetAverage(std::move(value)));
        }
#endif /* CHAT */
    }
    else {
        connectionLogger.Write(boost::str(boost::format(
            "HandleRead error user: %1% \"%2%\"\n") % id_ % error.message()));
        Shutdown();
    }
}

#if CHAT
/***********************************************************************************
 *  @brief  Trigger to send the message from current user to destiny user
 *  @note   Function passes message and user ID to the connection manager
 *  @param  dstUserId Destiny user ID
 *  @param  msg Message string which must be sended
 *  @return None
 */
/*void AsyncTcpConnection::ResendMessage(uint64_t dstUserId, const std::string& msg) noexcept {
    connMan_.ResendUserMessage(dstUserId, msg);
}*/

/***********************************************************************************
 *  @brief  Public function to initiate retransmit message to another user
 *  @note   Function has no callback
 *  @param  msg Message string which must be sended
 *  @return None
 */
void AsyncTcpConnection::StartWriteMessage(const std::string& msg)
{
    socket_.async_write_some(boost::asio::buffer(msg),
        [&](const boost::system::error_code& error,
            std::size_t bytes_transferred) {
                std::cout << "Message sended\n";
        });
}
#endif /* CHAT */

/***********************************************************************************
 *  @brief  Start async writing process from socket
 *  @param  value Average of squares summ from set (container)
 *  @return None
 */
void AsyncTcpConnection::StartWrite(uint64_t value)
{
    std::string resp{ boost::str(boost::format("%1%%2%,%3%%4%")
        % tech_msg_header % id_ % tech_resp_msg % value) };
    connectionLogger.Write(boost::str(boost::format(">> \"%1%\" [%2%]\n") % resp % resp.size()));

    socket_.async_write_some(boost::asio::buffer(resp),
        [&](const boost::system::error_code& error,
            std::size_t bytes_transferred) {
                HandleWrite(error);
        });
}

/***********************************************************************************
 *  @brief  Callback-handler of async writing process
 *  @param  error Boost system error object reference
 *  @return None
 */
void AsyncTcpConnection::HandleWrite(const boost::system::error_code& error)
{
    if (!error)
    {
        StartRead();
    }
    else {
        connectionLogger.Write(boost::str(boost::format(
            "HandleWrite error user: %1% \"%2%\"\n") % id_ % error.message()));
        Shutdown();
    }
}
