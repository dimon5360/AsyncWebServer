/*********************************************
 *
 *
 */
#pragma once

 /* std C++ lib headers */
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

/* boost C++ lib headers */
#include <boost/bind/placeholders.hpp>
#include <boost/thread.hpp>

 /* local C++ headers */
#include "AsyncTcpConnection.h"

template <class K>
class ConnectionManager {

private:
    /* hash map to keep clients connection pointers */
    std::unordered_map<K, AsyncTcpConnection::connection_ptr> clientsMap_;
    /* mutex object to avoid data race */
    mutable std::shared_mutex mutex_;

    const K DEFAULT_ID = 10;
public:

    K GetFreeId() {
        std::shared_lock lk(mutex_);
        static K connId = DEFAULT_ID;

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
    void CreateNewConnection(const K& connId, AsyncTcpConnection::connection_ptr connPtr)
    {        
        std::unique_lock lk(mutex_);
        clientsMap_.insert({ connId, connPtr });
    }

    /***********************************************************************************
     *  @brief  Func to remove connection tcp object by connection id
     *  @param  connId Client id to remove connection
     *  @return None
     */
    void RemoveConnection(K connId)
    {
        std::unique_lock lk(mutex_);
        clientsMap_.erase(connId);
    }

    /***********************************************************************************
     *  @brief  Func to remove connection tcp object by connection id
     *  @param  connId Client id to remove connection
     *  @return None
     */
    bool Contains(K connId)
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

extern ConnectionManager<uint64_t> connMan_;