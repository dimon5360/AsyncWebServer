/*********************************************
 *
 *
 */
#pragma once

 /* std C++ lib headers */
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <queue>

/* boost C++ lib headers */
#include <boost/bind/placeholders.hpp>
#include <boost/thread.hpp>

 /* local C++ headers */
#include "AsyncTcpConnection.h"

class MessageBroker {

public:
    using record_t = std::pair<const uint64_t, const std::string>;

    void PushMessage(const uint64_t& connId, const std::string&& msg) {
        std::unique_lock lk(m_);
        msgQueue.emplace(std::make_pair(connId, msg));
        msgNum++;
    }

    bool IsQueueEmpty() const noexcept {
        std::unique_lock lk(m_);
        return msgNum == 0;
    }

protected:

    const record_t PullMessage() {
        std::unique_lock lk(m_);
        msgNum--;
        return msgQueue.front();
    }

private:

    friend class ConnectionManager;
    std::queue<record_t> msgQueue;
    mutable std::shared_mutex m_;
    std::atomic_size_t msgNum;
};

extern MessageBroker msgBroker;

class ConnectionManager {

private:

    using id_t = uint64_t;

    /* hash map to keep clients connection pointers */
    std::unordered_map<id_t, AsyncTcpConnection::connection_ptr> clientsMap_;
    /* mutex object to avoid data race */
    mutable std::shared_mutex mutex_;

    const id_t DEFAULT_ID = 10;
    std::atomic_size_t connId = DEFAULT_ID;

#if CHAT
    /***********************************************************************************
     *  @brief  Public function to initiate retransmit message to another user
 *  @param  dstUserId Destiny user ID
     *  @param  msg Message string which must be sended
     *  @return None
     */
    void ResendUserMessage(const id_t& connId, const std::string& msg) const {
        std::unique_lock lk(mutex_);
        if (clientsMap_.contains(connId)) {
            clientsMap_.at(connId)->StartWriteMessage(msg);
            std::cout << "Message for user #" << connId << " sended\n";
        }
        else {
            std::cout << "User #" << connId << " not found\n";
        }
    }
#endif /* CHAT */

    void handle() {
        while (true) {
            if (!msgBroker.IsQueueEmpty()) {
                auto rec = msgBroker.PullMessage();
                ResendUserMessage(rec.first, rec.second);
            }
            //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

public:

    ConnectionManager() {
        std::cout << "Construct connection manager\n";
        std::thread{ &ConnectionManager::handle, this }.detach();
    }

    ~ConnectionManager() {
        std::cout << "Destruct connection manager\n";
    }

    id_t GetFreeId() {
        std::shared_lock lk(mutex_);

        /* if currIdConn is overloaded and there are free ids */
        while (clientsMap_.contains(connId) || connId < DEFAULT_ID) {
            connId++;
        }

        return connId;
    }

    /***********************************************************************************
     *  @brief  Func to add new connection tcp object to map
     *  @param  id New client id
     *  @param  connPtr Reference to async tcp connection class
     *  @return None
     */
    void CreateNewConnection(const id_t& connId, AsyncTcpConnection::connection_ptr connPtr)
    {
        std::unique_lock lk(mutex_);
        clientsMap_.insert({ connId, connPtr });
    }

    /***********************************************************************************
     *  @brief  Func to remove connection tcp object by connection id
     *  @param  connId Client id to remove connection
     *  @return None
     */
    void RemoveConnection(id_t connId)
    {
        std::unique_lock lk(mutex_);
        clientsMap_.erase(connId);
    }

    /***********************************************************************************
     *  @brief  Func to remove connection tcp object by connection id
     *  @param  connId Client id to remove connection
     *  @return None
     */
    bool Contains(const id_t& connId)
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
};

extern ConnectionManager connMan_;