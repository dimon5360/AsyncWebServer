/*********************************************
 *
 *
 */
#pragma once

 /* local C++ headers */
#include "AsyncTcpConnection.h"

class AsyncClient {
    using T = uint64_t;
public:

    /* alias for shared pointer to tcp client class */
    using client_ptr = std::shared_ptr<AsyncClient>;

    /***********************************************************************************
     *  @brief  Getter for tcp connection socket reference
     *  @return Reference to tcp connection socket
     */
    decltype(auto) socket() const noexcept {
        return conn->socket();
    }

    AsyncClient(boost::asio::io_service& io_service,
        boost::asio::ssl::context& context, const T& connId)
        : id_(connId)
    {
        std::cout << "New client constructor\n";
        conn = AsyncTcpConnection::create(io_service, context, id_);
    }

    ~AsyncClient() {
        std::cout << "Destruct existed client #" << id_ << std::endl;
    }

    void HandleAccept() const noexcept;
    void DisconnectClient() const noexcept;
    const T GetClientId() const noexcept;
    void ResendMessage(const std::string & msg) const noexcept;
private:

    AsyncTcpConnection::connection_ptr conn;
    T id_;
};