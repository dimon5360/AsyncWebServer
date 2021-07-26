/*********************************************
 *
 *
 */

 /* local C++ headers */
#include "AsyncTcpConnection.h"

class AsyncClient {

public:

    using client_ptr = std::shared_ptr<AsyncClient>;

    static client_ptr AddNewClient() {
        return std::make_shared<AsyncClient>();
    }

    AsyncClient() {
        std::cout << "New client constructor\n";
    }

    ~AsyncClient() {
        std::cout << "Destruct existed client\n";
    }


};