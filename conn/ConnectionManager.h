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

/* boost C++ lib headers */
#include <boost/bind/placeholders.hpp>
#include <boost/thread.hpp>

 /* local C++ headers */
#include "AsyncTcpConnection.h"
#include "AsyncClient.h"
#include "MessageBroker.h"
#include "../log/Logger.h"

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

        CustomRandomGen(const T&& min, const T&& max) :
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
    std::unordered_map<T, AsyncClient::client_ptr> clientsMap_;
    std::priority_queue<T> vacatedIds_;
    mutable std::shared_mutex mutex_;

    const T INVALID_ID = 0;

    const T GetFreeId() noexcept {
        T connId = INVALID_ID;

        try {
            //std::unique_lock lk(mutex_);
            if (!vacatedIds_.empty()) {
                connId = vacatedIds_.top();
                vacatedIds_.pop();
            }
            else {
                connId = randEngine->GenRandomNumber();
                /* if currIdConn is overloaded and there are free ids */
                while (clientsMap_.contains(connId)) {
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

    bool ManagerIsActive() {
        return isAlive;
    }
    void DeactivateManager() {
        isAlive = false;
    }

    ConnectionManager() {
        std::cout << "Construct connection manager\n";
        randEngine = std::make_unique<CustomRandomGen>(1, std::move(std::numeric_limits<T>::max()));
    }

    ~ConnectionManager() {
        std::cout << "Destruct connection manager\n";
    }
    
    AsyncClient::client_ptr CreateNewClient(boost::asio::io_service& io_service,
        boost::asio::ssl::context& context)
    {
        std::unique_lock lk(mutex_);
        T freeId = GetFreeId();
        auto newClient = std::make_shared<AsyncClient>(io_service, context, freeId);
        clientsMap_.insert({ freeId, newClient });
        return newClient;
    }

    void RemoveConnection(const T& connId)
    {
        try {
            std::unique_lock lk(mutex_);
            vacatedIds_.push(connId);
            clientsMap_.erase(connId);
        }
        catch (std::exception& ex) {
            ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
        }
    }

    bool Contains(const T& connId)
    {
        //std::shared_lock lk(mutex_);
        return clientsMap_.contains(connId);
    }

    void CloseAllConnections()
    {
        for (auto& v : clientsMap_) {
            v.second->DisconnectClient();
        }
        std::cout << "All connections are closed\n";
    }

protected:

    void ResendUserMessage(const T& conn_user_id, const std::string& user_msg) const {
        try {
            std::unique_lock lk(mutex_);
            if (clientsMap_.contains(conn_user_id)) {
                clientsMap_.at(conn_user_id)->ResendMessage(user_msg);
                std::cout << "Message for user #" << conn_user_id << " sended\n";
            }
            else {
                std::cout << "User #" << conn_user_id << " not found\n";
            }
        }
        catch (std::exception& ex) {
            ConsoleLogger::Error(boost::str(boost::format("Exception %1%: %2%\n") % __FUNCTION__ % ex.what()));
        }
    }
    };

extern ConnectionManager connMan_;
