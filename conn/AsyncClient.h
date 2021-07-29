/*********************************************
 *
 *
 */

 /* local C++ headers */
#include "AsyncTcpConnection.h"
#include "ConnectionManager.h"

class AsyncClient {

public:

    using client_ptr = std::shared_ptr<AsyncClient>;
    AsyncTcpConnection::connection_ptr conn;
    uint64_t id_;

    static client_ptr AddNewClient(const uint64_t& connId, const AsyncTcpConnection::connection_ptr connPtr) {
        return std::make_shared<AsyncClient>(connId, connPtr);
    }

    AsyncClient(const uint64_t& connId, const AsyncTcpConnection::connection_ptr connPtr)
        : id_(connId), 
        conn(connPtr)
    {
        std::cout << "New client constructor\n";
    }

    ~AsyncClient() {
        std::cout << "Destruct existed client\n";
    }


};