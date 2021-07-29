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
#include "AsyncClient.h"

class MessageBroker {

public:
    using record_t = std::pair<const uint64_t, const std::string>;

    /***********************************************************************************
     *  @brief  Push info about new message {msg} for user {connId} to queue
     *  @param  connId  User ID who must receive message
     *  @param  msg Message itself
     *  @return None
     */
    void PushMessage(const uint64_t& connId, const std::string&& msg) {
        std::unique_lock lk(m_);
        msgQueue.emplace(std::make_pair(connId, msg));
        msgNum++;
    }

    /***********************************************************************************
     *  @brief  Check queue is empty
     *  @return Check result, true if queue is empty
     */
    bool IsQueueEmpty() const noexcept {
        std::unique_lock lk(m_);
        return msgNum == 0;
    }

protected:

    /***********************************************************************************
     *  @brief  Pull fisrt message from queue
     *  @return Message info
     */
    const record_t PullMessage() {
        std::unique_lock lk(m_);
        record_t msg{ msgQueue.front() };
        msgQueue.pop();
        msgNum--;
        return msg;
    }

private:

    friend class ConnectionManager;
    std::queue<record_t> msgQueue;
    mutable std::shared_mutex m_;
    std::atomic_size_t msgNum;
};

extern MessageBroker msgBroker;

class ConnectionManager {

public:

    using id_t = uint64_t;

 private:

    /* hash map to keep clients connection pointers */
    std::unordered_map<id_t, AsyncTcpConnection::connection_ptr> clientsMap_;
    std::unordered_map<id_t, AsyncClient::client_ptr> clients_;
    /* mutex object to avoid data race */
    mutable std::shared_mutex mutex_;

    const id_t DEFAULT_ID = 10;
    std::atomic_size_t connId = DEFAULT_ID;
    const long long default_delay = 5;

public:

    /***********************************************************************************
     *  @brief  Operator to start handler of queue in separate thread
     *  @param  None
     *  @return None
     */
    void operator()(void) { 
        handle(); 
    }

    ConnectionManager() {
        std::cout << "Construct connection manager\n"; 
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
        clients_.insert({ connId, AsyncClient::AddNewClient(connId, connPtr) });
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
        clients_.erase(connId);
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

private:

#if CHAT
    /***********************************************************************************
     *  @brief  Public function to initiate retransmit message to another user
     *  @param  dstUserId Destiny user ID
     *  @param  msg Message string which must be sended
     *  @return None
     */
    void ResendUserMessage(const id_t& conn_user_id, const std::string& user_msg) const {
        std::unique_lock lk(mutex_);
        if (clientsMap_.contains(conn_user_id)) {
            clientsMap_.at(conn_user_id)->StartWriteMessage(user_msg);
            std::cout << "Message for user #" << conn_user_id << " sended\n";
        }
        else {
            std::cout << "User #" << conn_user_id << " not found\n";
        }
    }
#endif /* CHAT */

    /***********************************************************************************
     *  @brief  Handler of queue in separate thread
     *  @param  dstUserId Destiny user ID
     *  @param  msg Message string which must be sended
     *  @return None
     */
    void handle() {
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
