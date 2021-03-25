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

    /* file output stream object */
    std::ofstream log_file;
    /* dump file name */
    std::stringstream filename;
    /* mutex object to avoid data race */
    std::mutex _m;

#endif /* FILE_LOGGER */

    /* open log file */
    int open() noexcept {
        log_file.open(filename.str());
        if (!log_file.is_open())
        {
            std::cout << "Log file opening is failed\n\n";
            return 1;
        }
        write(boost::str(boost::format("Log file \'%1%\' is opened.\n") % filename.str()));
    }

    /* close log file */
    void close() noexcept {
#if FILE_LOGGER
        std::lock_guard<std::mutex> lk(this->_m);
        if (log_file.is_open()) {
            log_file.close();
        }
#endif /* FILE_LOGGER */
    }

    /* get time code */
    uint64_t GetCurrTimeMs() {
        const auto systick_now = std::chrono::system_clock::now();
        const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(systick_now.time_since_epoch());
        return nowMs.count();
    }

public:

    /* constructor */
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

    /* destructor */
    ~file_logger() {
        close();
    }

    /* write log string */
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

class file_dump {

private:

    const uint64_t dump_interval_ms = 20000; // 20 seconds

    /* file output stream object */
    std::ofstream log_dump;
    /* dump file name */
    std::stringstream filename;
    /* mutex object to avoid data race */
    std::mutex _m;

    /* open dump file */
    int open() noexcept {
        log_dump.open(filename.str(), std::ios::out | std::ios::binary);
        if (!log_dump.is_open())
        {
            std::cout << "Dump file opening is failed\n\n";
            return 1;
        }
        std::cout << "Dump file \'" << filename.str() << "\' is opened.\n";
    }

    /* close dump file */
    void close() noexcept {
        if (log_dump.is_open()) {
            log_dump.close();
        }
    }

    /* to make dump of data set (container) */
    void make_dump() noexcept {
        using namespace boost::posix_time;
        using namespace boost::gregorian;
        ptime now = second_clock::local_time();

        log_dump << "Dump: " << to_simple_string(now) << ":\n";
        log_dump << _gset.Dump();
        log_dump << "\n\n";
    }

    /* get time code */
    uint64_t GetCurrTimeMs() {
        const auto systick_now = std::chrono::system_clock::now();
        const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(systick_now.time_since_epoch());
        return nowMs.count();
    }

public:

    /* constructor */
    file_dump() {
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        ptime now = second_clock::local_time();
        filename << "web_server_dump " << GetCurrTimeMs() << ".log";
        open();

        std::thread t(&file_dump::handle, this);
        t.detach();
    }

    /* destructor */
    ~file_dump() {
        close();
    }

    /* main handler for dump processor */
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

    /* alias for shared pointer to tcp connectio class */
    typedef boost::shared_ptr<tcp_connection> connection_ptr;

    /***********************************************************************************
     *  @brief  Static func to create new connection tcp object and return its reference
     *  @param  io_service  Reference to boost io_service object
     *  @param  id New client id
     *  @return Reference to tcp connection object  
     */
    static connection_ptr create(boost::asio::io_service& io_service, uint32_t id)
    {
        return connection_ptr(new tcp_connection(io_service, id));
    }


    /***********************************************************************************
     *  @brief  Getter for tcp connection socket reference
     *  @return Reference to tcp connection socket
     */
    boost::asio::ip::tcp::socket& socket()
    {
        return socket_;
    }

    /***********************************************************************************
     *  @brief  Start process authentication of client
     *  @return None
     */
    void start_auth()
    {  
        socket_.async_read_some(boost::asio::buffer(buf),
            boost::bind(&tcp_connection::handle_auth, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    /* constructor */
    tcp_connection(boost::asio::io_service& io_service_, uint32_t id)
        : socket_(io_service_),
        id_(id)
    {
        /* ... */
    }

    /* destructor */
    ~tcp_connection() {
        logger.write("Close current connection\n");
    };

private:

    /***********************************************************************************
     *  @brief  Close tcp connection and call destructor
     *  @param  error Boost system error object reference
     *  @return None
     */
    void close(const boost::system::error_code& error) {
        shutdown(boost::asio::ip::tcp::socket::shutdown_send, error.value());
        logger.write(boost::str(boost::format("Close connection user=%1% error: %2% \n") % id_ % error.message())); 
        this->~tcp_connection();
    }

    void to_lower(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }

    /***********************************************************************************
     *  @brief  Callback-handler of async authentication process
     *  @param  error Boost system error object reference
     *  @param  recvBytes Amount of bytes received from connection
     *  @return None
     */
    void handle_auth(const boost::system::error_code& error,
        std::size_t recvBytes)
    {
        if (!error)
        {
            std::string in_hello_msg{ buf.data(), recvBytes };

            {
                std::stringstream log;

                log << "<< "<< "\"" << in_hello_msg << "\" [" << recvBytes << "]\n";
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
            close(error);
        }
    }

    /***********************************************************************************
     *  @brief  Start async reading process from socket
     *  @param  None
     *  @return None
     */
    void start_read()
    {    
        socket_.async_read_some(boost::asio::buffer(buf),
            boost::bind(&tcp_connection::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    /***********************************************************************************
     *  @brief  Callback-handler of async reading process
     *  @param  error Boost system error object reference
     *  @param  recvBytes Amount of bytes received from connection 
     *  @return None
     */
    void handle_read(const boost::system::error_code& error,
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
            close(error);
        }
    }

    /***********************************************************************************
     *  @brief  Start async writing process from socket
     *  @param  value Average of squares summ from set (container)
     *  @return None
     */
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

    /***********************************************************************************
     *  @brief  Callback-handler of async writing process
     *  @param  error Boost system error object reference
     *  @return None
     */
    void handle_write(const boost::system::error_code& error)
    {      
        if (!error)
        {
            start_read();
        }
        else {
            close(error);
        }
    }

    /* tcp socket object */
    boost::asio::ip::tcp::socket socket_;
    /* msgs headers to exchange with clients */
    const std::string hello_msg = std::string("hello user id=");
    const std::string tech_msg_header = std::string("user id=");
    const std::string tech_req_msg = std::string("number=");
    const std::string tech_resp_msg = std::string("summ=");

    /* unique id of client */
    uint32_t id_;

    /* exchange data buffer */
    enum { max_length = 1024 };
    boost::array<char, max_length> buf = { { 0 } };
};

class async_tcp_server {

private:    
    /* boost io_service object reference */
    boost::asio::io_service& io_service_;
    /* boost acceptor object */
    boost::asio::ip::tcp::acceptor acceptor_;
    /* hash map to keep clients connection pointers */
    std::unordered_map<uint32_t, tcp_connection::connection_ptr> clientMap;

    /***********************************************************************************
     *  @brief  Callback-handler of async accepting process
     *  @param  new_connection Shared pointer to new connection of client 
     *  @param  error Boost system error object reference
     *  @return None
     */
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

    /***********************************************************************************
     *  @brief  Start async assepting process in socket
     *  @return None
     */
    void start_accept() {
        static uint32_t currIdConn = 1;

        tcp_connection::connection_ptr new_connection =
            tcp_connection::create(io_service_, currIdConn);

        logger.write(boost::str(boost::format("Start listening to %1% port \n") % 4059));
         
        /* if currIdConn is overloaded and there are free ids */
        while (clientMap.contains(currIdConn)) {
            currIdConn++;
        }

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

/* separate thread for processing of SPACE key press (to close application) */
static void EscapeWait() {
    while (GetAsyncKeyState(VK_SPACE) == 0) {
        Sleep(10);
    }
    exit(0);
}

int main() {
    /* for corrent output boost error messages */
    SetConsoleOutputCP(1251);
    logger.write("Press SPACE to exit...\n");

    try
    {
        /* separate thread to monitor SPACE key pressing */
        std::thread ext(&EscapeWait);
        /* unique pointer to dump processor object (in separate thread) */
        std::unique_ptr<file_dump> dump = std::make_unique<file_dump>();
        /* start tcp server */
        boost::asio::io_service ios;
        async_tcp_server serv(ios);
        ios.run();
        ext.join();
    }
    catch (std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
    }

    return 0;
}
