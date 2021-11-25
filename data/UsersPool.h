

#pragma once


#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <set>

#include "../conn/AsyncClient.h"
#include "../conn/AsyncTcpConnection.h"
#include "../log/Logger.h"

#define USE_USERS_POOL

class UsersPool {

private:
    using T = AsyncTcpConnection::id_t;
    mutable std::unordered_map<T, AsyncClient::client_ptr> clients;
    mutable std::set<std::string> usersIdsAsStrings;

    std::string PrepareUsersIdsList() const;

    mutable std::shared_mutex mutex_;

public:

    static const uint32_t BROADCAST_ID = std::numeric_limits<T>::max();

    auto begin() const { return clients.begin(); }
    auto end() const { return clients.end(); }

    UsersPool() = delete;
    UsersPool& operator=(UsersPool& pool) = delete;

    UsersPool(uint32_t maxUserCount) {
        std::cout << "UsersPool class constructor\n";
        clients.reserve(maxUserCount);
    }
    ~UsersPool() {
        std::cout << "UsersPool class destructor\n";
    }

    void StoreNewClient(const T& id, AsyncClient::client_ptr& ptr) const noexcept;
    const AsyncClient::client_ptr& GetClient(const T& id) const noexcept;
    void RemoveExistedClient(const T& id) const noexcept;
    bool IsThereSuchClient(const T& id) const noexcept;
    void DisconnectAllClients() const noexcept;
    const size_t GetUsersAmount() const noexcept;
    void SendoutUsersList() const noexcept;
};