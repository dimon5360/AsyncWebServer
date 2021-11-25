/*********************************************
 *
 *
 */
#pragma once

 /* local C++ headers */
#include "ConnectionManager.h"

/* std C++ lib headers */
#include <shared_mutex>
#include <queue>

class MessageBroker {
    friend class DataProcess;
    using T = AsyncTcpConnection::id_t;

public:
    using record_t = std::pair<T, const std::string>;

    void PushMessage(const T& connId, const std::string& msg) {
        std::unique_lock lk(m_);
        msgQueue.emplace(std::make_pair(connId, msg));
        msgNum++;
    }

    bool IsQueueEmpty() noexcept {
        std::shared_lock lk(m_);
        return msgNum == 0;
    }

protected:

    const record_t PullMessage() {
        std::unique_lock lk(m_);
        record_t msg{ msgQueue.front() };
        msgQueue.pop();
        msgNum--;
        return msg;
    }

private:

    std::queue<record_t> msgQueue;
    std::shared_mutex m_;
    std::atomic_size_t msgNum;
};

extern MessageBroker msgBroker;