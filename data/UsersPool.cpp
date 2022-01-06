

#include "UsersPool.h"
#include "DataProcess.h"

#include <boost/format.hpp>


const uint32_t UsersPool::BROADCAST_ID = std::numeric_limits<T>::max();


void UsersPool::StoreNewClient(const T& id, AsyncClient::client_ptr& ptr) const noexcept {
    try{
        std::unique_lock lk(mutex_);
        clients.insert({ id, ptr });
        usersIdsAsStrings.insert(boost::str(boost::format("%1%") % id));
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
}

const AsyncClient::client_ptr& UsersPool::GetClient(const T& id) const noexcept {
    AsyncClient::client_ptr ptr = nullptr;
    try {
        std::shared_lock lk(mutex_);
        ptr = clients.at(id);
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
    return ptr;
}

void UsersPool::RemoveExistedClient(const T& id) const noexcept {
    try {
        std::unique_lock lk(mutex_);
        clients.erase(id);
        usersIdsAsStrings.erase(boost::str(boost::format("%1%") % id));
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
}

bool UsersPool::IsThereSuchClient(const T& id) const noexcept {
    std::unique_lock lk(mutex_);
    return clients.contains(id);
}

void UsersPool::SendoutUsersList() const noexcept {

    try {
        std::unique_lock lk(mutex_);
        auto users = PrepareUsersIdsList();
        std::string json = dataProcessor.GetUsersListInJson(users, clients.size());
        std::string message = dataProcessor.ConstructMessage(UsersPool::BROADCAST_ID, json);
        dataProcessor.PushNewMessage(message);
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
}

void UsersPool::DisconnectAllClients() const noexcept {
    try {
        std::unique_lock lk(mutex_);
        for (auto& client : clients) {
            client.second->DisconnectClient();
        }
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
}

const size_t UsersPool::GetUsersAmount() const noexcept {
    try {
        std::unique_lock lk(mutex_);
        return clients.size();
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
}

std::string UsersPool::PrepareUsersIdsList() const {
    std::string usersIds = "";
    for (auto& sid : usersIdsAsStrings) {
        usersIds += sid;
        usersIds += " ";
    }
    return usersIds;
}
