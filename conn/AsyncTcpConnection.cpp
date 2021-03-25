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
#include <cstdint>

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

class console_logger : ILogger {

private:

    /* get time code */
    uint64_t GetCurrTimeMs() override {
        const auto systick_now = std::chrono::system_clock::now();
        const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(systick_now.time_since_epoch());
        return nowMs.count();
    }

public:

    /* open log file */
    void open() noexcept override { /* ... */ }

    /* close log file */
    void close() noexcept override { /* ... */ }

    /* write log string */
    void write(std::string log) noexcept override {
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        ptime now = second_clock::local_time();
        std::string time = boost::str(boost::format("%1%: ") % to_simple_string(now));
        std::cout << time << log;
    }
};

static console_logger logger;

#define DIFFICULT_WAY 1
#if DIFFICULT_WAY
#define SIMPLE_WAY 0
#else
#define SIMPLE_WAY 1
#endif /* DIFFICULT_WAY */

template <class K>
class guarded_set {
private:

    /* set (container) to keep unique random numbers from clients */
    std::unordered_set<K> _set;
    /* mutex object to avoid data race */
    std::mutex _m;

    K squaresSumm = 0; // keep last value of squares summ

    /* setter */
    void Set(K& value) {
        this->_set.insert(value);
    }

    /* check contains */
    bool IsContain(K& value) {
        return _set.contains(value);
    }

public:

    /***************************************************************
     *  @brief  Check that new number from client is unique
     *  @param  value Received random number from client
     *  @return Average of numbers squares summ
     */
    K GetAverage(K value) {
        std::lock_guard<std::mutex> lk(this->_m);
#if DIFFICULT_WAY // O(N)
        if (!IsContain(value)) {
            Set(value);
        }
        squaresSumm = 0;
        for (auto& v : _set) {
            squaresSumm += (v * v);
        }

        return static_cast<K>(squaresSumm / _set.size());
#elif SIMPLE_WAY // O(1)
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
#endif /* DIFFICULT_WAY */
    }

    /* return dump of set data */
    std::string Dump() {
        std::stringstream ss;

        std::lock_guard<std::mutex> lk(this->_m);
        for (auto& v : _set) {
            ss << v << "\n";
        }

        return ss.str();
    }
};

static guarded_set<uint64_t> _gset;

/***********************************************************************************
*  @brief  Getter for tcp connection socket reference
*  @return Reference to tcp connection socket
*/
boost::asio::ip::tcp::socket& async_tcp_connection::socket()
{
    return socket_;
}

/***********************************************************************************
 *  @brief  Start process authentication of client
 *  @return None
 */
void async_tcp_connection::start_auth() {
    socket_.async_read_some(boost::asio::buffer(buf),
        boost::bind(&async_tcp_connection::handle_auth, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

/***********************************************************************************
 *  @brief  Close tcp connection and call destructor
 *  @param  error Boost system error object reference
 *  @return None
 */
void async_tcp_connection::close(const boost::system::error_code& error) {
    logger.write(boost::str(boost::format("Close connection request user=%1% error: %2% \n") % id_ % error.message()));
    shutdown(boost::asio::ip::tcp::socket::shutdown_send, error.value());
    connMan_.RemoveConnection(id_);
}

/***********************************************************************************
 *  @brief  Callback-handler of async authentication process
 *  @param  error Boost system error object reference
 *  @param  recvBytes Amount of bytes received from connection
 *  @return None
 */
void async_tcp_connection::handle_auth(const boost::system::error_code& error,
    std::size_t recvBytes)
{
    if (!error)
    {
        std::string in_hello_msg{ buf.data(), recvBytes };

        {
            std::stringstream log;

            log << "<< " << "\"" << in_hello_msg << "\" [" << recvBytes << "]\n";
            logger.write(log.str());
        }

        std::stringstream resp;
        resp << hello_msg << id_;

        {
            std::stringstream log;
            log << ">> " << "\"" << resp.str() << "\" [" << resp.str().size() << "]\n";
            logger.write(log.str());
        }

        boost::asio::async_write(socket_, boost::asio::buffer(resp.str()),
            boost::bind(&async_tcp_connection::handle_write, this,
                boost::asio::placeholders::error));
    }
    else {
        close(error);
    }
}

/***********************************************************************************
 *  @brief  Start async reading process from socket
 *  @param  None
 *  @return None
 */
void async_tcp_connection::start_read()
{
    socket_.async_read_some(boost::asio::buffer(buf),
        boost::bind(&async_tcp_connection::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

/***********************************************************************************
 *  @brief  Callback-handler of async reading process
 *  @param  error Boost system error object reference
 *  @param  recvBytes Amount of bytes received from connection
 *  @return None
 */
void async_tcp_connection::handle_read(const boost::system::error_code& error,
    std::size_t recvBytes)
{
    if (!error)
    {
        std::string in_msg{ buf.data(), recvBytes };

        {
            std::stringstream log;
            log << "<< " << "\"" << in_msg << "\" [" << recvBytes << "]\n";
            logger.write(log.str());
        }

        to_lower(in_msg);

        if (in_msg.substr(0, tech_req_msg.size()).compare(tech_req_msg) == 0) {
            auto number = in_msg.substr(tech_req_msg.size());

            std::stringstream int_conv(number);

            int value;
            int_conv >> value;
            uint64_t summ = _gset.GetAverage(value);
            start_write(summ);
        }
    }
    else {
        close(error);
    }
}

/***********************************************************************************
 *  @brief  Start async writing process from socket
 *  @param  value Average of squares summ from set (container)
 *  @return None
 */
void async_tcp_connection::start_write(uint64_t value)
{
    std::stringstream resp;
    resp << tech_resp_msg << value;

    {
        std::stringstream log;
        log << ">> " << "\"" << resp.str() << "\" [" << resp.str().size() << "]\n";
        logger.write(log.str());
    }

    boost::asio::async_write(socket_, boost::asio::buffer(resp.str()),
        boost::bind(&async_tcp_connection::handle_write, this,
            boost::asio::placeholders::error));
}

/***********************************************************************************
 *  @brief  Callback-handler of async writing process
 *  @param  error Boost system error object reference
 *  @return None
 */
void async_tcp_connection::handle_write(const boost::system::error_code& error)
{
    if (!error)
    {
        start_read();
    }
    else {
        close(error);
    }
}
