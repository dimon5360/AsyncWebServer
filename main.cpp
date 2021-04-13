/*********************************************
 *
 *
 */

 /* std C++ lib headers */
#include <iostream>

 /* boost C++ lib headers */
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>

 /* local C++ headers */
#include "serv/AsyncTcpServer.h"
#include "conn/ConnectionManager.h"
#include "db/PostgresProcessor.h"
#include "test/tests.h"

/* Build v.0.0.9 from 13.04.2021 */
const uint32_t PATCH = 9;
const uint32_t MINOR = 0;
const uint32_t MAJOR = 0;


int main()
{
#if UNIT_TEST    
    return tests();
#else 

    /* for corrent output boost error messages */
    SetConsoleOutputCP(1251);
    std::cout << boost::format("Hello. Application version is %1%.%2%.%3%\n") % MAJOR % MINOR % PATCH;
    std::cout << "Press SPACE to exit...\n";

    try
    {
        /* conctruct db class */
        std::unique_ptr<PostgresProcessor> db = std::make_unique<PostgresProcessor>();


        /* separate thread to start tcp server */
        boost::asio::io_service ios;
        boost::asio::signal_set signals(ios, SIGINT);

        boost::thread_group threads;
        boost::asio::io_service::work work(ios);

        for (int i = 0; i < boost::thread::hardware_concurrency(); ++i)
        {
            threads.create_thread(boost::bind(&boost::asio::io_service::run, &ios));
        }
        ios.post(boost::bind(&AsyncTcpServer::StartTcpServer, std::ref(ios)));


        /* asynchronous wait for Ctrl + C signal to occur */
        signals.async_wait([&](const boost::system::error_code& error, int signal_number) {
            AsyncTcpServer::StopTcpServer(std::ref(ios));
            ios.stop();
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