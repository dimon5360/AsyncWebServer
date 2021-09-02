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

/* boost C++ lib headers */
#include <boost/bind/placeholders.hpp>
#include <boost/thread.hpp>

 /* local C++ headers */
#include "AsyncTcpConnection.h"
#include "AsyncClient.h"
#include "MessageBroker.h"

#define USE_CLIENT_CLASS 1

class ConnectionManager {

private:

    using T = uint64_t;

    template<class type>
    class CustomRandomGen {
    private:
        /* random number generator object */
        std::random_device r;
        std::mt19937 e1;
        std::uniform_int_distribution<type> uniform_dist;

    public:

        CustomRandomGen(type&& min, type&& max) :
            e1(r()),
            uniform_dist(min, max)
        {
            std::cout << "Construct new random numbers generator class\n";
        }

        /* destructor */
        ~CustomRandomGen() {
            std::cout << "Destruct random generator class\n";
        }

        /* getter for new random number to send in server */
        type GenRandomNumber() noexcept {
            return static_cast<type>(uniform_dist(e1));
        }
    };

    std::unique_ptr<CustomRandomGen<uint32_t>> randEngine;

 private:

    /* hash map to keep clients connection pointers */
    std::unordered_map<T, AsyncTcpConnection::connection_ptr> clientsMap_;
    /* queue to keep vacated IDs, default queue is empty */
    std::priority_queue<T> vacatedIds_;
    /* mutex object to avoid data race */
    mutable std::shared_mutex mutex_;

    const T INVALID_ID = 0;
    const uint32_t default_delay = 5;

public:

    ConnectionManager() {
        std::cout << "Construct connection manager\n";
        randEngine = std::make_unique<CustomRandomGen<uint32_t>>(1, UINT64_MAX);
    }

    ~ConnectionManager() {
        std::cout << "Destruct connection manager\n";
    }

    const T GetFreeId() noexcept {
        std::shared_lock lk(mutex_);
        T connId = INVALID_ID;

        if (!vacatedIds_.empty()) {
            connId = vacatedIds_.top();
            vacatedIds_.pop();
        } else {
            connId = randEngine->GenRandomNumber();
            /* if currIdConn is overloaded and there are free ids */
            while (clientsMap_.contains(connId)) {
                connId = randEngine->GenRandomNumber();
            }
        }
        return connId;
    }

    /***********************************************************************************
     *  @brief  Operator to start handler of queue in separate thread
     *  @param  None
     *  @return None
     */
    void operator()(void) { 
        handle(); 
    }

    /***********************************************************************************
     *  @brief  Func to add new connection tcp object to map
     *  @param  id New client id
     *  @param  connPtr Reference to async tcp connection class
     *  @return None
     */
    void CreateNewConnection(const T& connId, AsyncTcpConnection::connection_ptr connPtr)
    {
        std::unique_lock lk(mutex_);
        clientsMap_.insert({ connId, connPtr });
    }

    /***********************************************************************************
     *  @brief  Func to remove connection tcp object by connection id
     *  @param  connId Client id to remove connection
     *  @return None
     */
    void RemoveConnection(const T& connId)
    {
        std::unique_lock lk(mutex_);
        vacatedIds_.push(connId);
        clientsMap_.erase(connId);
    }

    /***********************************************************************************
     *  @brief  Func to remove connection tcp object by connection id
     *  @param  connId Client id to remove connection
     *  @return None
     */
    bool Contains(const T& connId)
    {
        std::shared_lock lk(mutex_);
        return clientsMap_.contains(connId);
    }

    /***********************************************************************************
     *  @brief  Func to close and remove all connections
     *  @return None
     */
    void CloseAllConnections()
    {
        for (auto& v : clientsMap_) {
            v.second->socket().close();
        }
        while (clientsMap_.size() > 0) {
            boost::this_thread::sleep(1);
        }
    }

private:

    /***********************************************************************************
     *  @brief  Public function to initiate retransmit message to another user
     *  @param  dstUserId Destiny user ID
     *  @param  msg Message string which must be sended
     *  @return None
     */
    void ResendUserMessage(const T& conn_user_id, const std::string& user_msg) const {
        std::unique_lock lk(mutex_);
        if (clientsMap_.contains(conn_user_id)) {
            clientsMap_.at(conn_user_id)->StartWriteMessage(user_msg);
            std::cout << "Message for user #" << conn_user_id << " sended\n";
        }
        else {
            std::cout << "User #" << conn_user_id << " not found\n";
        }
    }

    /***********************************************************************************
     *  @brief  Handler of queue in separate thread
     *  @param  dstUserId Destiny user ID
     *  @param  msg Message string which must be sended
     *  @return None
     */
    void handle() { // TODO: remake, dummy
        std::thread{ [&]() {
            while (true) {
                if (!msgBroker.IsQueueEmpty()) {
                    auto rec = msgBroker.PullMessage();
                    ResendUserMessage(rec.first, rec.second);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(default_delay));
            }}
        }.detach();
    }
};

extern ConnectionManager connMan_;
