/*********************************************
 *
 *
 */

 /* std C++ lib headers */
#include <iostream>

 /* boost C++ lib headers */
#include <boost/format.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <spdlog/spdlog.h>

 /* local C++ headers */
#include "serv/AsyncTcpServer.h"
#include "conn/ConnectionManager.h"
#include "db/PostgresProcessor.h"
#include "test/tests.h"

/* Build v.0.0.14 from 29.07.2021 */
const uint32_t PATCH = 14;
const uint32_t MINOR = 0;
const uint32_t MAJOR = 0;

/**********************************************************
 *  @brief  entry point 
 */
int main()
{
#if UNIT_TEST    
    return tests();
#else 

    /* for corrent output boost error messages */
    SetConsoleOutputCP(1251);
    spdlog::info(boost::str(boost::format("Hello. Application version is %1%.%2%.%3%\n") % MAJOR % MINOR % PATCH));

    try
    {
        /* conctruct db class */
        std::unique_ptr<PostgresProcessor> db = std::make_unique<PostgresProcessor>();

        /* separate thread to start tcp server */
        boost::asio::io_service ios;

        boost::thread_group threads;
        boost::asio::io_context::work work(ios);
        boost::asio::signal_set signals(work.get_io_context(), SIGINT);

        for (int i = 0; i < boost::thread::hardware_concurrency(); ++i)
        {
            threads.create_thread([&]() {
                work.get_io_context().run();
            });
        }
        boost::asio::post(work.get_io_context(), [&]() {
            AsyncTcpServer::StartTcpServer(std::ref(work.get_io_context()));
        });

        /* asynchronous wait for Ctrl + C signal to occur */
        signals.async_wait([&](const boost::system::error_code& error, int signal_number) {
            AsyncTcpServer::StopTcpServer(std::ref(work.get_io_context()));
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
