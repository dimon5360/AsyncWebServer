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

template <class K>
class GuardedSet {
private:

    /* set (container) to keep unique random numbers from clients */
    std::unordered_set<K> _set;
    /* mutex object to avoid data race */
    mutable std::shared_mutex mutex_;

    K squaresSumm = 0; // keep last value of squares summ

    /* setter */
    void Set(K& value) {
        std::unique_lock lock(mutex_);
        this->_set.insert(value);
    }

    /* check contains */
    bool IsContain(K& value) {
        std::shared_lock lock(mutex_);
        return _set.contains(value);
    }

public:

    /***************************************************************
     *  @brief  Check that new number from client is unique
     *  @param  value Received random number from client
     *  @return Average of numbers squares summ
     */
    K GetAverage(K value) {
        /* New random value is already in container
         * We don't need to calculate new average of numbers' squares */
        if (IsContain(value)) {
            return static_cast<K>(squaresSumm / _set.size());
        }
        /* New random value is unique
         * We need to calculate it */
        else {
            Set(value);
            squaresSumm += (value * value);
            return static_cast<K>(squaresSumm / _set.size());
        }
    }

    /* return dump of set data */
    std::string Dump() {
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
#if SECURE
AsyncTcpConnection::ssl_socket::lowest_layer_type& AsyncTcpConnection::socket() {

    return socket_.lowest_layer();
}
#else 
boost::asio::ip::tcp::socket& async_tcp_connection::socket()
{
    return socket_;
}
#endif /* SECURE */

/***********************************************************************************
 *  @brief  Start process authentication of client
 *  @return None
 */
void AsyncTcpConnection::StartAuth() {

#if SECURE
    socket_.async_handshake(boost::asio::ssl::stream_base::server,
    boost::bind(&AsyncTcpConnection::HandleHandshake, this,
        boost::asio::placeholders::error));
#else 
    socket_.async_read_some(boost::asio::buffer(buf),
        boost::bind(&async_tcp_connection::handle_auth, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
#endif /* SECURE */
}

#if SECURE
/***********************************************************************************
 *  @brief  Callback-handler of async handshake process
 *  @param  error Boost system error object reference
 *  @return None
 */
void AsyncTcpConnection::HandleHandshake(const boost::system::error_code& error) {
    if (!error)
    {
        socket_.async_read_some(boost::asio::buffer(buf),
            boost::bind(&AsyncTcpConnection::HandleAuth, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        Shutdown();
    }
}
#endif /* SECURE */

/***********************************************************************************
*  @brief  Send shutdown request (close notify)
*  @return None
*/
void AsyncTcpConnection::Shutdown() {
    std::cout << "Shutdown id " << id_ << std::endl;
    socket_.async_shutdown(boost::bind(&AsyncTcpConnection::Close, this, boost::asio::placeholders::error));
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
        std::string in_hello_msg{ buf.data(), recvBytes };

        {
            std::stringstream log;

            log << "<< " << "\"" << in_hello_msg << "\" [" << recvBytes << "]\n";
            connectionLogger.Write(log.str());
        }

        std::stringstream resp;
        resp << hello_msg << id_;

        {
            std::stringstream log;
            log << ">> " << "\"" << resp.str() << "\" [" << resp.str().size() << "]\n";
            connectionLogger.Write(log.str());
        }

        boost::asio::async_write(socket_, boost::asio::buffer(resp.str()),
            boost::bind(&AsyncTcpConnection::HandleWrite, this,
                boost::asio::placeholders::error));
    }
    else {
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
        boost::bind(&AsyncTcpConnection::HandleRead, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
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
        std::string_view in_msg{ buf.data(), recvBytes };

        {
            std::stringstream log;
            log << "<< " << "\"" << in_msg << "\" [" << recvBytes << "]\n";
            connectionLogger.Write(log.str());
        }

        //to_lower(in_msg);

        if (in_msg.starts_with(tech_msg_header)) {
            auto item = in_msg.find(tech_req_msg);
            int value = boost::lexical_cast<int>(in_msg.substr(item + tech_req_msg.size()));
            StartWrite(_gset.GetAverage(value));
        }
    }
    else {
        Shutdown();
    }
}

/***********************************************************************************
 *  @brief  Start async writing process from socket
 *  @param  value Average of squares summ from set (container)
 *  @return None
 */
void AsyncTcpConnection::StartWrite(uint64_t value)
{
    std::stringstream resp;
    resp << tech_msg_header << id_ << ", " << tech_resp_msg << value;

    {
        std::stringstream log;
        log << ">> " << "\"" << resp.str() << "\" [" << resp.str().size() << "]\n";
        connectionLogger.Write(log.str());
    }

    boost::asio::async_write(socket_, boost::asio::buffer(resp.str()),
        boost::bind(&AsyncTcpConnection::HandleWrite, this,
            boost::asio::placeholders::error));
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
        Shutdown();
    }
}
