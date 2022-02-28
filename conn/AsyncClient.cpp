/*********************************************
 *
 *
 */

 /* local C++ headers */
#include "AsyncClient.h"
#include "ConnectionManager.h"

std::shared_ptr<ConnectionManager> ConnectionManager::cm_ = nullptr;
std::shared_ptr<MessageBroker> MessageBroker::mb_ = nullptr;

void AsyncClient::HandleAccept() const noexcept {
    conn->StartAuth();
}

void AsyncClient::DisconnectClient() const noexcept  {
    conn->socket().close();
    ConnectionManager::GetInstance()->RemoveConnection(id_);
}

void AsyncClient::ResendMessage(const std::string & msg) const noexcept {
    conn->StartWriteMessage(msg);
}

const AsyncClient::T AsyncClient::GetClientId() const noexcept  {
    return id_;
}