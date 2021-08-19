/*********************************************
 *
 *
 */
#pragma once

 /* local C++ headers */
#include "AsyncTcpConnection.h"

class AsyncClient {
public:

    /* alias for shared pointer to tcp client class */
    using client_ptr = std::shared_ptr<AsyncClient>;

    /***********************************************************************************
     *  @brief  Getter for tcp connection socket reference
     *  @return Reference to tcp connection socket
     */
    decltype(auto) socket() {
        return conn->socket();
    }

    static client_ptr CreateNewClient(boost::asio::io_service& io_service,
        boost::asio::ssl::context& context);

    AsyncClient(boost::asio::io_service& io_service,
        boost::asio::ssl::context& context, const uint64_t connId)
        : id_(connId)
    {
        conn = AsyncTcpConnection::create(io_service, context, id_);
        std::cout << "New client constructor\n";
    }

    ~AsyncClient() {
        std::cout << "Destruct existed client\n";
    }

    void HandleAccept();

    void DisconnectClient();

    const uint64_t GetClientId() const noexcept;

private:

    AsyncTcpConnection::connection_ptr conn;
    uint64_t id_;
};