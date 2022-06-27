/*****************************************************************
 *  @file       main.cpp
 *  @brief      Entry point of program
 *  @author     Kalmykov Dmitry
 *  @date       28.04.2021
 *  @version    1.0
 */

#include "core/AsyncTcpServer.h"
#include "db/PostgresProcessor.h"
#include "test/tests.h"

#include "db/KafkaProcess.h"

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <string>
#include <thread>

#include <boost/format.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <boost/asio/thread_pool.hpp>

#include <spdlog/spdlog.h>

/* Build v.0.0.27 from 27.06.2022 */
const uint32_t PATCH = 27;
const uint32_t MINOR = 0;
const uint32_t MAJOR = 0;

int main()
{
#if UNIT_TEST    
    return static_cast<int32_t>(init_unit_tests());
#else 
    
    const std::string shello = "Hello. Application version is %1%.%2%.%3%\n";
    spdlog::info(boost::str(boost::format(shello) % MAJOR % MINOR % PATCH));

    try
    {
        /* separate thread to start tcp server */
        boost::asio::io_service ios;
        boost::thread_group threads;
        boost::asio::io_context::work work(ios);
        boost::asio::signal_set signals(work.get_io_context(), SIGINT, SIGTERM);

        for (unsigned int i = 0; i < boost::thread::hardware_concurrency(); ++i)
        {
            threads.create_thread([&]() {
                work.get_io_context().run();
            });
        }

        boost::asio::post(work.get_io_context(), [&]() {
            AsyncTcpServer::StartTcpServer(work.get_io_context());
        });

        /* asynchronous wait for Ctrl + C signal to occur */
        signals.async_wait([&](const boost::system::error_code& error, int signal_number) {
            AsyncTcpServer::StopTcpServer(work.get_io_context());
            work.get_io_context().stop();
        });

        threads.join_all();
    }
    catch (std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
    }
    return 0;
#endif /* UNIT_TEST */
}
