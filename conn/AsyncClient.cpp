/*********************************************
 *
 *
 */

 /* local C++ headers */
#include "AsyncClient.h"
#include "ConnectionManager.h"

void AsyncClient::HandleAccept() const noexcept {
    conn->StartAuth();
}

void AsyncClient::DisconnectClient() const noexcept  {
    conn->socket().close();
    connMan_.RemoveConnection(id_);
}

void AsyncClient::ResendMessage(const std::string & msg) const noexcept {
    conn->StartWriteMessage(msg);
}

const AsyncClient::T AsyncClient::GetClientId() const noexcept  {
    return id_;
}
