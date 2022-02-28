/*********************************************
 *
 *
 */
#pragma once

#include "ConnectionManager.h"

#include <shared_mutex>
#include <queue>
#include <memory>

#include <spdlog/spdlog.h>

class MessageBroker {
    friend class DataProcess;

    static std::shared_ptr<MessageBroker> mb_;

public:
    using T = AsyncTcpConnection::id_t;
    
    using record_t = std::pair<T, const std::string>;

    // to avoid copying and creating any one instance
    MessageBroker(const MessageBroker& mb) = delete;
    MessageBroker& operator=(const MessageBroker& md) = delete;

    MessageBroker() {
        spdlog::info("Construct MessageBroker class");
    }

    ~MessageBroker() {
        spdlog::info("Destruct MessageBroker class");
    }

    static const std::shared_ptr<MessageBroker>& GetInstance() {
        if(!mb_) {
            mb_ = std::make_shared<MessageBroker>();
        }
        return mb_;
    }

    void PushMessage(const T& connId, std::string&& msg) {
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