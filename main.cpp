/* std C++ lib headers */
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <utility>
#include <algorithm>
#include <chrono>
#include <thread>

/* boost C++ lib headers */
#include <boost/format.hpp>
#include <boost/asio.hpp> 
#include <boost/array.hpp> 
#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>

/* deployment definitions */
#define CONSOLE_LOGGER      1
#define FILE_LOGGER         1

#if FILE_LOGGER
#include <boost/date_time.hpp>
#include <fstream>
#endif /* FILE_LOGGER */

class file_logger {

private:

#if FILE_LOGGER
    std::ofstream log_file;
    std::stringstream filename;
    std::mutex _m;
#endif /* FILE_LOGGER */

    int open() noexcept {
        log_file.open(filename.str());
        if (!log_file.is_open())
        {
            std::cout << "Log file opening is failed\n\n";
            return 1;
        }
        write(boost::str(boost::format("Log file \'%1%\' is opened.\n") % filename.str()));
    }

    void close() noexcept {
#if FILE_LOGGER
        std::lock_guard<std::mutex> lk(this->_m);
        if (log_file.is_open()) {
            log_file.close();
        }
#endif /* FILE_LOGGER */
    }

    uint64_t GetCurrTimeMs() {
        const auto systick_now = std::chrono::system_clock::now();
        const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(systick_now.time_since_epoch());
        return nowMs.count();
    }

public:
    file_logger() {

#if FILE_LOGGER
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        ptime now = second_clock::local_time();
        filename << "web_server_log " << GetCurrTimeMs() << ".log";
        open();

        write(boost::str(boost::format("Start time \'%1%\' is opened.\n") % to_simple_string(now)));
#endif /* FILE_LOGGER */
    }

    ~file_logger() {
        close();
    }

    void write(std::string log) noexcept {
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        ptime now = second_clock::local_time();
        std::string time = boost::str(boost::format("%1%: ") % to_simple_string(now));
#if CONSOLE_LOGGER
        std::cout << time << log;
#endif /* CONSOLE_LOGGER */
#if FILE_LOGGER
        std::lock_guard<std::mutex> lk(this->_m);
        log_file << time << log;
#endif /* FILE_LOGGER */
    }
};

static file_logger logger;

template <class K> 
class guarded_set {
private:
    std::unordered_set<K> _set;
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
     *  @param  newRandNum Last received random number from client
     *  @return Average of numbers squares summ
     */
    K GetAverage(K value) {
        std::lock_guard<std::mutex> lk(this->_m);
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

class file_dump {

private:

    const uint64_t dump_interval_ms = 20000;// *60 * 5; // 5 minutes

    std::ofstream log_dump;
    std::stringstream filename;
    std::mutex _m;

    int open() noexcept {
        log_dump.open(filename.str(), std::ios::out | std::ios::binary);
        if (!log_dump.is_open())
        {
            std::cout << "Dump file opening is failed\n\n";
            return 1;
        }
        std::cout << "Dump file \'" << filename.str() << "\' is opened.\n";
    }

    void close() noexcept {
        if (log_dump.is_open()) {
            log_dump.close();
        }
    }

    void make_dump() noexcept {
        using namespace boost::posix_time;
        using namespace boost::gregorian;
        ptime now = second_clock::local_time();

        log_dump << "Dump: " << to_simple_string(now) << ":\n";
        log_dump << _gset.Dump();
        log_dump << "\n\n";
    }

    uint64_t GetCurrTimeMs() {
        const auto systick_now = std::chrono::system_clock::now();
        const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(systick_now.time_since_epoch());
        return nowMs.count();
    }

public:

    file_dump() {
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        ptime now = second_clock::local_time();
        filename << "web_server_dump " << GetCurrTimeMs() << ".log";
        open();

        std::thread t(&file_dump::handle, this);
        t.detach();
    }

    ~file_dump() {
        close();
    }

    void handle() {

        uint64_t prevDumpTime = GetCurrTimeMs();

        for (;;) {

            /* if uint64_t overload */
            if (GetCurrTimeMs() < prevDumpTime) {
                prevDumpTime = GetCurrTimeMs();
            }
            else if (GetCurrTimeMs() - prevDumpTime > dump_interval_ms){
                make_dump();
                prevDumpTime = GetCurrTimeMs();
            }

            std::chrono::system_clock::time_point timePoint =
                std::chrono::system_clock::now() + std::chrono::seconds(10);
            std::this_thread::sleep_until(timePoint);
        }
    }
};

class tcp_connection
{
public:

    typedef boost::shared_ptr<tcp_connection> connection_ptr;
    static connection_ptr create(boost::asio::io_service& io_service, uint32_t id)
    {
        return connection_ptr(new tcp_connection(io_service, id));
    }

    boost::asio::ip::tcp::socket& socket()
    {
        return socket_;
    }

    void start_auth()
    {  
        socket_.async_read_some(boost::asio::buffer(buf),
            boost::bind(&tcp_connection::handle_auth, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    tcp_connection(boost::asio::io_service& io_service_, uint32_t id)
        : socket_(io_service_),
        id_(id)
    {
        /* ... */
    }

    ~tcp_connection() {
        logger.write("Close current connection\n");
    };

private:

    void to_lower(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }

    void handle_auth(const boost::system::error_code& error,
        std::size_t bytes_transferred)
    {
        if (!error)
        {
            std::string in_hello_msg{ buf.data(), bytes_transferred };

            {
                std::stringstream log;

                log << "<< "<< "\"" << in_hello_msg << "\" [" << bytes_transferred << "]\n";
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
                boost::bind(&tcp_connection::handle_write, this,
                    boost::asio::placeholders::error));
        }
        else {
            shutdown(boost::asio::ip::tcp::socket::shutdown_send, error.value());
            logger.write(boost::str(boost::format(" %1% \n") % error.message()));
        }
    }

    void start_read()
    {    
        socket_.async_read_some(boost::asio::buffer(buf),
            boost::bind(&tcp_connection::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    void handle_read(const boost::system::error_code& error,
        std::size_t bytes_transferred)
    {
        if (!error)
        {
            std::string in_msg{ buf.data(), bytes_transferred };

            {
                std::stringstream log;
                log << "<< " << "\"" << in_msg << "\" [" << bytes_transferred << "]\n";
                logger.write(log.str());
            }

            /* support of different register */
            to_lower(in_msg);

            /* check that auth msg corresponds to default value */
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
            shutdown(boost::asio::ip::tcp::socket::shutdown_send, error.value());
            logger.write(boost::str(boost::format(" %1% \n") % error.message()));
        }
    }

    void start_write(uint64_t value)
    {
        std::stringstream resp;
        resp << tech_resp_msg << value;

        {
            std::stringstream log;
            log << ">> " << "\"" << resp.str() << "\" [" << resp.str().size() << "]\n";
            logger.write(log.str());
        }

        boost::asio::async_write(socket_, boost::asio::buffer(resp.str()),
            boost::bind(&tcp_connection::handle_write, this,
            boost::asio::placeholders::error));
    }

    void handle_write(const boost::system::error_code& error)
    {      
        if (!error)
        {
            start_read();
        }
        else {
            shutdown(boost::asio::ip::tcp::socket::shutdown_send, error.value());
            logger.write(boost::str(boost::format(" %1% \n") % error.message()));
        }
    }

    boost::asio::ip::tcp::socket socket_;

    const std::string hello_msg = std::string("hello user id=");
    const std::string tech_msg_header = std::string("user id=");
    const std::string tech_req_msg = std::string("number=");
    const std::string tech_resp_msg = std::string("summ=");
    uint32_t id_;

    enum { max_length = 1024 };
    boost::array<char, max_length> buf = { { 0 } };
};

class async_tcp_server {

private:    

    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::unordered_map<uint32_t, tcp_connection::connection_ptr> clientMap;

    void handle_accept(tcp_connection::connection_ptr new_connection,
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


    void start_accept() {
        static uint32_t currIdConn = 1;

        tcp_connection::connection_ptr new_connection =
            tcp_connection::create(io_service_, currIdConn);

        logger.write(boost::str(boost::format("Start listening to %1% port \n") % 4059));

        if (!clientMap.contains(currIdConn)) {
            clientMap.insert({ currIdConn, new_connection });
            currIdConn++;
        }

        acceptor_.async_accept(new_connection->socket(),
            boost::bind(&async_tcp_server::handle_accept, this, new_connection,
                boost::asio::placeholders::error));
    }

public:

    /* constructor */
    async_tcp_server(boost::asio::io_service& io_service) :
        io_service_(io_service),
        acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 4059))
    {
        start_accept();
    }

    /* destructor */
    ~async_tcp_server() { /**/ };
};

#include <windows.h>

static void EscapeWait() {
    while (GetAsyncKeyState(VK_SPACE) == 0) {
        Sleep(10);
    }
    exit(0);
}

int main() {

    SetConsoleOutputCP(1251);
    logger.write("Press SPACE to exit...\n");
    std::thread ext(&EscapeWait);
    std::unique_ptr<file_dump> dump = std::make_unique<file_dump>();
    try
    {
        boost::asio::io_service ios;
        async_tcp_server serv(ios);
        ios.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}


/***************************************************************
 *  @brief  Check that new number from client is unique
 *  @param  newRandNum Last received random number from client
 *  @return Average of numbers squares summ
 */
/*int GetAverageNumbersSquares(int newRandNum) {
    static int squaresNumsSumm = 0;
    if (cont.contains(newRandNum)) {
        return squaresNumsSumm / cont.size();
    }
    else {
        cont.insert(newRandNum);
        squaresNumsSumm += (newRandNum * newRandNum);
        return squaresNumsSumm / cont.size();
    }
}*/


/*#include <iostream>
#include <pqxx/pqxx>

#include <boost/format.hpp>
#include <spdlog/spdlog.h>

#include "db/PostgresProcessor.h"

const uint32_t PATCH = 0;
const uint32_t MINOR = 0;
const uint32_t MAJOR = 0;

int main()
{
    std::cout << boost::format("Hello. Application version is %1%.%2%.%3%\n") % MAJOR % MINOR % PATCH;
    std::unique_ptr<Postgres::PostgresProcessor> db = std::make_unique<Postgres::PostgresProcessor>();
    std::cout << (int)db->InitializeDatabaseConnection() << std::endl;
    return 0;
}*/