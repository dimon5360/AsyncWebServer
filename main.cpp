/* std C++ lib headers */
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <utility>

/* boost C++ lib headers */
#include <boost/asio.hpp> 
#include <boost/array.hpp> 
#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>

#include <spdlog/spdlog.h>

#define LOG_FUNC        0
#define PRINT_MESSAGE   1

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
        if (_set.empty()) {
            return static_cast<K>(0);
        }
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
}; 

guarded_set<int> _set;

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

    void start_read()
    {
#if LOG_FUNC
        std::cout << __func__ << "()\n";
#endif /* LOG_FUNC */        
        socket_.async_read_some(boost::asio::buffer(buf),
            boost::bind(&tcp_connection::handle_read, this, 
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    void start_write()
    {
#if LOG_FUNC
        std::cout << __func__ << "()\n";
#endif /* LOG_FUNC */        
        std::stringstream resp;
        resp << "hello user id=" << id_;
        std::string msg_ = "hello user id=" + id_;
        boost::asio::async_write(socket_, boost::asio::buffer(resp.str()),
            boost::bind(&tcp_connection::handle_write, this,
                boost::asio::placeholders::error));
    }

    tcp_connection(boost::asio::io_service& io_service_, uint32_t id)
        : socket_(io_service_),
        id_(id)
    {
    }

    ~tcp_connection() { 
        std::cout << "Close current connection\n";
    };

private:

    void handle_write(const boost::system::error_code& error)
    {
#if LOG_FUNC
        std::cout << __func__ << "()\n";
#endif /* LOG_FUNC */        
        if (!error)
        {
            start_read();
        }
        else {
            std::cout << "Writing error: " << error.message() << std::endl;
        }
    }

    void handle_read(const boost::system::error_code& error,
        std::size_t bytes_transferred)
    {
#if LOG_FUNC
        std::cout << __func__ << "()\n";
#endif /* LOG_FUNC */        
        if (!error)
        {
#if PRINT_MESSAGE
            std::cout << "Read " << bytes_transferred << " bytes: \"";
            std::cout.write(buf.data(), bytes_transferred) << "\"\n";
#endif /* PRINT_MESSAGE */        
            // TODO: parse input data to get new random number
            /*int k = 0;
            int summ = _set.GetAverage(k);*/
            start_write();
        }
        else {
            std::cout << "Reading error: " << error.message() << "\n";
        }
    }

    uint32_t id_;
    boost::asio::ip::tcp::socket socket_;
    enum { max_length = 1024 };
    boost::array<char, max_length> buf = { { 0 } };
    char data_[max_length] = { 0 };
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
            std::cout << "New connection accepted. Start reading data.\n";
            new_connection->start_read();
        }
        start_accept();
    }


    void start_accept() {
        static uint32_t currIdConn = 1;

#if LOG_FUNC
        std::cout << __func__ << "()\n";
#endif /* LOG_FUNC */        
        std::cout << "Wait new connection ... \n";
        tcp_connection::connection_ptr new_connection =
            tcp_connection::create(io_service_, currIdConn);

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


int main() {

    std::cout << "Hello, user." << std::endl;
    //boost::asio::io_context io_context;
    boost::asio::io_service ios;
    async_tcp_server serv(ios);
    ios.run();
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