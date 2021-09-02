/*********************************************
 *
 *
 */

 /* local C++ headers */
#include "AsyncClient.h"
#include "ConnectionManager.h"

AsyncClient::client_ptr AsyncClient::CreateNewClient(boost::asio::io_service& io_service,
    boost::asio::ssl::context& context) 
{
    return std::make_shared<AsyncClient>(io_service, context, connMan_.GetFreeId());
}

void AsyncClient::HandleAccept() const noexcept {
    conn->StartAuth();
    connMan_.CreateNewConnection(id_, conn);
}

void AsyncClient::DisconnectClient() const noexcept  {
    conn->socket().close();
}

const uint64_t AsyncClient::GetClientId() const noexcept  {
    return id_;
}