/* std C++ lib headers */
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <utility>
#include <algorithm>

/* boost C++ lib headers */
#include <boost/asio.hpp> 
#include <boost/array.hpp> 
#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>

/* deployment definitions */
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

guarded_set<uint64_t> _set;

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
        std::cout << "Close current connection\n";
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
            std::string hello_msg{ buf.data(), bytes_transferred };

#if PRINT_MESSAGE
            std::cout << "Read " << bytes_transferred << " bytes: \"";
            std::cout << hello_msg << "\"\n";
#endif /* PRINT_MESSAGE */ 

            std::stringstream resp;
            resp << hello_msg << id_;

#if PRINT_MESSAGE
            std::cout << "Write " << resp.str().size() << " bytes: \"";
            std::cout << resp.str() << "\"\n";
#endif /* PRINT_MESSAGE */ 

            boost::asio::async_write(socket_, boost::asio::buffer(resp.str()),
                boost::bind(&tcp_connection::handle_write, this,
                    boost::asio::placeholders::error));
        }
        else {
            std::cout << "Authentication error: " << error.message() << "\n";
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
            std::string msg{ buf.data(), bytes_transferred };

#if PRINT_MESSAGE
            std::cout << "Read " << bytes_transferred << " bytes: \"";
            std::cout << msg << "\"\n";
#endif /* PRINT_MESSAGE */        

            /* support of different register */
            to_lower(msg);

            /* check that auth msg corresponds to default value */
            if (msg.substr(0, tech_req_msg.size()).compare(tech_req_msg) == 0) {
                auto number = msg.substr(tech_req_msg.size());

                std::stringstream int_conv(number);

                int value;
                int_conv >> value;
                uint64_t summ = _set.GetAverage(value);
                start_write(summ);
            }
        }
        else {
            std::cout << "Reading error: " << error.message() << "\n";
        }
    }

    void start_write(uint64_t value)
    {    
        std::stringstream resp;
        resp << tech_resp_msg << value;
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
            std::cout << "Writing error: " << error.message() << std::endl;
        }
    }


    boost::asio::ip::tcp::socket socket_;

    const std::string hello_msg = std::string("hello user id=");
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
            std::cout << "New connection accepted. Start reading data.\n";
            new_connection->start_auth();
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