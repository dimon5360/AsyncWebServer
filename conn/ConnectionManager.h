/*********************************************
 *
 *
 */
#pragma once

#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <random>
#include <limits>
#include <queue>
#include <set>

#include <boost/bind/placeholders.hpp>
#include <boost/thread.hpp>

#include <spdlog/spdlog.h>

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
            ConsoleLogger::Debug("Construct new random numbers generator class");
        }

        ~CustomRandomGen() {
            ConsoleLogger::Debug("Destruct random generator class");
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

    static std::shared_ptr<ConnectionManager> cm_;

public:

    // to avoid copying and creating any one instance
    ConnectionManager(const ConnectionManager& mb) = delete;
    ConnectionManager& operator=(const ConnectionManager& md) = delete;

    ConnectionManager() {
        ConsoleLogger::Debug("Construct connection manager");
        randEngine = std::make_unique<CustomRandomGen>(1, UsersPool::BROADCAST_ID-1);
        users = std::make_unique<UsersPool>(RESERVED_USERS_POOL_SIZE);
    }

    ~ConnectionManager() {
        ConsoleLogger::Debug("Destruct connection manager");
    }

    static const std::shared_ptr<ConnectionManager>& GetInstance() {
        if(!cm_) {
            cm_ = std::make_shared<ConnectionManager>();
        }
        return cm_;
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
            ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%") % __FUNCTION__ % ex.what()));
        }
    }

    bool Contains(const T& connId) const noexcept
    {
        return users->IsThereSuchClient(connId);
    }

    void CloseAllConnections()
    {
        users->DisconnectAllClients();
        ConsoleLogger::Debug("All connections are closed");
    }

    void SendUsersListToUser(const T& id) 
    {
        users->SendUsersListToUser(id);
    }

protected:

    void ResendUserMessage(const T& connId, const std::string& user_msg) const {
        try {
            if (Contains(connId)) {
                users->GetClient(connId)->ResendMessage(user_msg);
                ConsoleLogger::Debug(boost::str(boost::format("%1%%2%%3%") % "Message for user #" % connId % " sended"));
            }
            else {
                std::cout << "User #" << connId << " not found\n";
            }
        }
        catch (std::exception& ex) {
            ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%") % __FUNCTION__ % ex.what()));
        }
    }
};

