

#include "UsersPool.h"
#include "DataProcess.h"

#include <boost/format.hpp>

const uint32_t UsersPool::BROADCAST_ID = std::numeric_limits<T>::max();

void UsersPool::StoreNewClient(const T &id, AsyncClient::client_ptr &ptr) const noexcept
{
    try
    {
        std::unique_lock lk(mutex_);
        clients.insert({id, ptr});
        usersIdsAsStrings.insert(boost::str(boost::format("%1%") % id));
    }
    catch (std::exception &ex)
    {
        ConsoleLogger::Error(ex.what());
    }
}

const AsyncClient::client_ptr UsersPool::GetClient(const T &id) const noexcept
{
    AsyncClient::client_ptr ptr = nullptr;
    try
    {
        std::shared_lock lk(mutex_);
        ptr = clients.at(id);
    }
    catch (std::exception &ex)
    {
        ConsoleLogger::Error(ex.what());
    }
    return ptr;
}

void UsersPool::RemoveExistedClient(const T &id) const noexcept
{
    try
    {
        std::unique_lock lk(mutex_);
        clients.erase(id);
        usersIdsAsStrings.erase(boost::str(boost::format("%1%") % id));
    }
    catch (std::exception &ex)
    {
        ConsoleLogger::Error(ex.what());
    }
}

bool UsersPool::IsThereSuchClient(const T &id) const noexcept
{
    std::unique_lock lk(mutex_);
    return clients.contains(id);
}

void UsersPool::SendUsersListToUser(const T& id) const noexcept
{
    try
    {
        std::unique_lock lk(mutex_);
        std::string users{PrepareUsersIdsList()};
        std::string json = DataProcess::GetInstance()->GetUsersListInJson(users, clients.size());
        std::string message = DataProcess::GetInstance()->ConstructMessage(id, json, JsonHandler::json_req_t::users_list_message);
        DataProcess::GetInstance()->PushNewMessage(id, std::move(message));
    }
    catch (std::exception &ex)
    {
        ConsoleLogger::Error(ex.what());
    }
}

void UsersPool::DisconnectAllClients() const noexcept
{
    try
    {
        std::unique_lock lk(mutex_);
        for (auto &client : clients)
        {
            client.second->DisconnectClient();
        }
    }
    catch (std::exception &ex)
    {
        ConsoleLogger::Error(ex.what());
    }
}

const size_t UsersPool::GetUsersAmount() const noexcept
{
    try
    {
        std::unique_lock lk(mutex_);
        return clients.size();
    }
    catch (std::exception &ex)
    {
        ConsoleLogger::Error(ex.what());
    }
    return 0;
}

std::string UsersPool::PrepareUsersIdsList() const
{
    std::string usersIds = "";
    for (auto &sid : usersIdsAsStrings)
    {
        usersIds += sid;
        usersIds += " ";
    }
    return usersIds;
}
