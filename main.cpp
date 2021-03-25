/*********************************************
 *
 *
 */

 /* std C++ lib headers */
#include <iostream>

 /* boost C++ lib headers */
#include <boost/format.hpp>

 /* local C++ headers */
#include "serv/AsyncTcpServer.h"
#include "conn/ConnectionManager.h"

/* Build v.0.0.1 from 26.03.2021 */
const uint32_t PATCH = 1;
const uint32_t MINOR = 0;
const uint32_t MAJOR = 0;

#include <windows.h>

/* separate thread for processing of SPACE key press (to close application) */
static void EscapeWait() {
    while (GetAsyncKeyState(VK_SPACE) == 0) {
        Sleep(10);
    }
    exit(0);
}


int main()
{
    /* for corrent output boost error messages */
    SetConsoleOutputCP(1251);
    std::cout << boost::format("Hello. Application version is %1%.%2%.%3%\n") % MAJOR % MINOR % PATCH;
    std::cout << "Press SPACE to exit...\n";

    try
    {
        /* separate thread to monitor SPACE key pressing */
        std::thread ext(&EscapeWait);
        /* start tcp server */
        boost::asio::io_service ios;
        async_tcp_server serv(ios);
        ios.run();
        ext.join();
    }
    catch (std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
    }
    //std::unique_ptr<PostgresProcessor> db = std::make_unique<PostgresProcessor>();
    //std::cout << (int)db->InitializeDatabaseConnection() << std::endl;
    return 0;
}