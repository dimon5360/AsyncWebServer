

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

    static const T BROADCAST_ID;
    
    auto begin() const { return clients.begin(); }
    auto end() const { return clients.end(); }

    UsersPool() = delete;
    UsersPool& operator=(UsersPool& pool) = delete;

    UsersPool(uint32_t maxUserCount) {
        std::cout << "Construct UsersPool class\n";
        clients.reserve(maxUserCount);
    }
    ~UsersPool() {
        std::cout << "Destruct UsersPool class\n";
    }

    void StoreNewClient(const T& id, AsyncClient::client_ptr& ptr) const noexcept;
    const AsyncClient::client_ptr GetClient(const T& id) const noexcept;
    void RemoveExistedClient(const T& id) const noexcept;
    bool IsThereSuchClient(const T& id) const noexcept;
    void DisconnectAllClients() const noexcept;
    const size_t GetUsersAmount() const noexcept;
    void SendUsersListToUser(const T& id) const noexcept;
};
