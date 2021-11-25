/*********************************************
 *
 *
 */
#pragma once

 /* std C++ lib headers */
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <random>
#include <limits>
#include <queue>
#include <set>

/* boost C++ lib headers */
#include <boost/bind/placeholders.hpp>
#include <boost/thread.hpp>

 /* local C++ headers */
#include "AsyncTcpConnection.h"
#include "AsyncClient.h"
#include "MessageBroker.h"
#include "../log/Logger.h"
#include "../data/UsersPool.h"

#define DATA_PROCESS

class ConnectionManager {

    friend class DataProcess;

private:

    using T = AsyncTcpConnection::id_t;

    class CustomRandomGen {
    private:
        std::random_device r;
        std::mt19937 e1;
        std::uniform_int_distribution<T> uniform_dist;

    public:

        CustomRandomGen(const T& min, const T& max) :
            e1(r()),
            uniform_dist(min, max)
        {
            std::cout << "Construct new random numbers generator class\n";
        }

        /* destructor */
        ~CustomRandomGen() {
            std::cout << "Destruct random generator class\n";
        }

        T GenRandomNumber() noexcept {
            return static_cast<T>(uniform_dist(e1));
        }
    };

    std::unique_ptr<CustomRandomGen> randEngine;

 private:

    bool isAlive = true;

    std::unique_ptr<UsersPool> users;
    std::priority_queue<T> vacatedIds_;

    mutable std::shared_mutex mutex_;

    const T INVALID_ID = 0;
    const uint32_t RESERVED_USERS_POOL_SIZE = 100;

    const T GetFreeId() noexcept {
        T connId = INVALID_ID;

        try {
            if (!vacatedIds_.empty()) {
                connId = vacatedIds_.top();
                vacatedIds_.pop();
            }
            else {
                connId = randEngine->GenRandomNumber();
                while (Contains(connId)) {
                    connId = randEngine->GenRandomNumber();
                }
            }
        }
        catch (std::exception& ex) {
            ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
        }
        return connId;
    }

public:

    ConnectionManager() {
        std::cout << "Construct connection manager\n";
        randEngine = std::make_unique<CustomRandomGen>(1, UsersPool::BROADCAST_ID-1);
        users = std::make_unique<UsersPool>(RESERVED_USERS_POOL_SIZE);
    }

    ~ConnectionManager() {
        std::cout << "Destruct connection manager\n";
    }

    bool ManagerIsActive() {
        return isAlive;
    }
    void DeactivateManager() {
        isAlive = false;
    }

    AsyncClient::client_ptr CreateNewClient(boost::asio::io_service& io_service,
        boost::asio::ssl::context& context)
    {
        std::unique_lock lk(mutex_);
        T freeId = GetFreeId();
        auto pnewClient = std::make_shared<AsyncClient>(io_service, context, freeId);
        return pnewClient;
    }

    void AddConnection(AsyncClient::client_ptr& pnewClient) {
        users->StoreNewClient(pnewClient->GetClientId(), pnewClient);
    }

    void RemoveConnection(const T& connId)
    {
        try {
            vacatedIds_.push(connId);
            users->RemoveExistedClient(connId);
        }
        catch (std::exception& ex) {
            ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
        }
    }

    bool Contains(const T& connId) const noexcept
    {
        return users->IsThereSuchClient(connId);
    }

    void CloseAllConnections()
    {
        users->DisconnectAllClients();
        std::cout << "All connections are closed\n";
    }

    void SendUsersListToEveryone() 
    {
        users->SendoutUsersList();
    }

protected:

    void ResendUserMessage(const T& connId, const std::string& user_msg) const {
        try {
            if (connId == UsersPool::BROADCAST_ID) {
                for (auto it = users->begin(); it != users->end(); ++it) {
                    auto client = users->GetClient(it->first);
                    client->ResendMessage(user_msg);
                    std::this_thread::sleep_for(std::chrono::microseconds(20));
                }
            }
            else if (Contains(connId)) {
                users->GetClient(connId)->ResendMessage(user_msg);
                std::cout << "Message for user #" << connId << " sended\n";
            }
            else {
                std::cout << "User #" << connId << " not found\n";
            }
        }
        catch (std::exception& ex) {
            ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
        }
    }
};

extern ConnectionManager connMan_;
