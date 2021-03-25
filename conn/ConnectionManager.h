/*********************************************
 *
 *
 */
#pragma once

 /* std C++ lib headers */
#include <unordered_map>

 /* local C++ headers */
#include "AsyncTcpConnection.h"

template <class K>
class ConnectionManager {

private:
    /* hash map to keep clients connection pointers */
    std::unordered_map<K, async_tcp_connection::connection_ptr> clientsMap_;
    /* mutex object to avoid data race */
    std::mutex _m;

    const K DEFAULT_ID = 10;
public:

    K GetFreeId() {
        std::lock_guard<std::mutex> lk(this->_m);
        static K connId = DEFAULT_ID;

        /* if currIdConn is overloaded and there are free ids */
        while (clientsMap_.contains(connId) || connId < DEFAULT_ID) {
            connId++;
        }

        return connId;
    }

    /***********************************************************************************
     *  @brief  Static func to create new connection tcp object and return its reference
     *  @param  io_service  Reference to boost io_service object
     *  @param  id New client id
     *  @return Reference to tcp connection object
     */
    K CreateNewConnection(K connId, async_tcp_connection::connection_ptr connPtr)
    {
        std::lock_guard<std::mutex> lk(this->_m);
        clientsMap_.insert({ connId, connPtr });
    }

    /***********************************************************************************
     *  @brief  Static func to create new connection tcp object and return its reference
     *  @param  io_service  Reference to boost io_service object
     *  @param  id New client id
     *  @return Reference to tcp connection object
     */
    void RemoveConnection(K connId)
    {
        std::lock_guard<std::mutex> lk(this->_m);
        clientsMap_.erase(connId);
    }
};

extern ConnectionManager<uint64_t> connMan_;