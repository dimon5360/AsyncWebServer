/*********************************************
 *
 *
 */

/* std C++ lib headers */
#include <iostream>
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
#include <boost/date_time.hpp>

/* local C++ headers */
#include "AsyncTcpServer.h"
#include "../log/Logger.h"
#include "../conn/ConnectionManager.h"

ConnectionManager<uint64_t> connMan_;

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

/***********************************************************************************
    *  @brief  Callback-handler of async accepting process
    *  @param  new_connection Shared pointer to new connection of client
    *  @param  error Boost system error object reference
    *  @return None
    */
void async_tcp_server::handle_accept(async_tcp_connection::connection_ptr new_connection,
    boost::system::error_code error)
{
    if (!error)
    {
        logger.write("New connection accepted. Start reading data.\n");
        new_connection->start_auth();
    }
    else {
        shutdown(boost::asio::ip::tcp::socket::shutdown_send, error.value());
    }
    start_accept();
}

/***********************************************************************************
    *  @brief  Start async assepting process in socket
    *  @return None
    */
void async_tcp_server::start_accept() {

    logger.write(boost::str(boost::format("Start listening new connection to %1% port \n") % 4059));

    uint64_t connId = connMan_.GetFreeId();

    async_tcp_connection::connection_ptr new_connection =
        async_tcp_connection::create(io_service_, connId);
    
    connMan_.CreateNewConnection(connId, new_connection);

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&async_tcp_server::handle_accept, this, new_connection,
            boost::asio::placeholders::error));
}