
#include "tests.h"

#if UNIT_TEST     

#if USE_COROUTINES
static void test_start_server();
#endif /* USE_COROUTINES */

#if USER_RSA_CRYPTO
static int test_rsa_enc_dec();
#endif /* USER_RSA_CRYPTO */

#if USER_DH_CRYPTO
static int test_dh_alg();
#endif /* USER_DH_CRYPTO */

int tests() {

#if USE_COROUTINES
    test_start_server();
#endif /* USE_COROUTINES */

#if USER_RSA_CRYPTO
    test_rsa_enc_dec();
#endif /* USER_RSA_CRYPTO */

#if USER_DH_CRYPTO
    test_dh_alg();
#endif /* USER_DH_CRYPTO */
    return 0;
}

#if USER_RSA_CRYPTO
#include "../crypto/rsa.h"

static int test_rsa_enc_dec() {

    RSA_Crypto rsa;
    std::string msg = "hello server";

    std::cout << "\nThe original message is: " << msg << std::endl;
    auto encrypted_msg = rsa.Encrypt(msg);
    std::string decrypted_msg = rsa.Decrypt(encrypted_msg);
    std::cout << "\nThe decrypted message is: " << decrypted_msg << std::endl;

    return 0;
}
#endif /* USER_RSA_CRYPTO */

#if USER_DH_CRYPTO

#include "../crypto/dh.h"
static int test_dh_alg() {

    // Generate first prime numbers
    int32_t p = DH_Crypto::GetRandomPrimeNum(100, 150);
    int32_t g = DH_Crypto::GetRandomPrimeNum(4, 10);

    /* construct random generator */
    std::unique_ptr<DH_Crypto> dh = std::make_unique<DH_Crypto>(p, g);

    int32_t private_key = dh->GetRandomPrimeNum(100, 150);
    int32_t A = dh->GetPublicKey();
    int32_t B = dh->Calc(g, private_key, p);
    dh->SetPublicKey(B);
    int32_t common_secret_key = dh->Calc(A, private_key, p);
    std::cout << "Client common secret key: " << common_secret_key << std::endl;
    return 0;
}
#endif /* USER_DH_CRYPTO */

#if USE_JTHREAD
#include <coroutine>
#include <iostream>
#include <stdexcept>
#include <thread>

auto switch_to_new_thread(std::jthread& out) {
    struct awaitable {
        std::jthread* p_out;
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> h) {
            std::jthread& out = *p_out;
            if (out.joinable())
                throw std::runtime_error("Output jthread parameter not empty");
            out = std::jthread([h] { h.resume(); });
            // Potential undefined behavior: accessing potentially destroyed *this
            // std::cout << "New thread ID: " << p_out->get_id() << '\n';
            std::cout << "New thread ID: " << out.get_id() << '\n'; // this is OK
        }
        void await_resume() {}
    };
    return awaitable{ &out };
}

struct task {
    struct promise_type {
        task get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

task resuming_on_new_thread(std::jthread& out) {
    std::cout << "Coroutine started on thread: " << std::this_thread::get_id() << '\n';
    co_await switch_to_new_thread(out);
    // awaiter destroyed here
    std::cout << "Coroutine resumed on thread: " << std::this_thread::get_id() << '\n';
}
#endif /* USE_JTHREAD */


#if USE_COROUTINES

#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::redirect_error;
using boost::asio::use_awaitable;

//----------------------------------------------------------------------

class chat_participant
{
public:
    virtual ~chat_participant() {}
    virtual void deliver(const std::string& msg) = 0;
};


//----------------------------------------------------------------------

class chat_room
{
public:
    using chat_participant_ptr = std::shared_ptr<chat_participant>;

    void join(chat_participant_ptr participant)
    {
        participants_.insert(participant);
        for (auto msg : recent_msgs_)
            participant->deliver(msg);
    }

    void leave(chat_participant_ptr participant)
    {
        participants_.erase(participant);
    }

    void deliver(const std::string& msg)
    {
        recent_msgs_.push_back(msg);
        while (recent_msgs_.size() > max_recent_msgs)
            recent_msgs_.pop_front();

        for (auto participant : participants_)
            participant->deliver(msg);
    }

private:
    std::set<chat_participant_ptr> participants_;
    enum { max_recent_msgs = 100 };
    std::deque<std::string> recent_msgs_;
};

//----------------------------------------------------------------------

class chat_session
    : public chat_participant,
    public std::enable_shared_from_this<chat_session>
{
public:
    chat_session(tcp::socket socket, chat_room& room)
        : socket_(std::move(socket)),
        timer_(socket_.get_executor()),
        room_(room)
    {
        timer_.expires_at(std::chrono::steady_clock::time_point::max());
    }

    void start()
    {
        std::cout << "Start\n";
        room_.join(shared_from_this());

        co_spawn(socket_.get_executor(),
            [self = shared_from_this()]{ return self->reader(); },
            detached);

        co_spawn(socket_.get_executor(),
            [self = shared_from_this()]{ return self->writer(); },
            detached);
    }

    void deliver(const std::string& msg)
    {
        std::cout << msg << std::endl;
        write_msgs_.push_back("HTTP/1.1 OK 200");
        timer_.cancel_one();
    }

private:

    enum { max_length = 1024 };
    std::array<char, max_length> buf = { { 0 } };

    awaitable<void> reader()
    {
        try
        {
            for (std::string read_msg;;)
            {
                size_t recvBytes = co_await socket_.async_read_some(boost::asio::buffer(buf), use_awaitable);

                std::cout << "read " << recvBytes << " bytes\n";

                room_.deliver({ buf.data(), recvBytes });
            }
        }
        catch (std::exception&)
        {
            stop();
        }
    }

    awaitable<void> writer()
    {
        try
        {
            while (socket_.is_open())
            {
                if (write_msgs_.empty())
                {
                    boost::system::error_code ec;
                    co_await timer_.async_wait(redirect_error(use_awaitable, ec));
                }
                else
                {
                    co_await boost::asio::async_write(socket_,
                        boost::asio::buffer(write_msgs_.front()), use_awaitable);
                    std::cout << write_msgs_.front() << std::endl;
                    write_msgs_.pop_front();
                }
            }
        }
        catch (std::exception&)
        {
            stop();
        }
    }

    void stop()
    {
        room_.leave(shared_from_this());
        socket_.close();
        timer_.cancel();
    }

    tcp::socket socket_;
    boost::asio::steady_timer timer_;
    chat_room& room_;
    std::deque<std::string> write_msgs_;
};

//----------------------------------------------------------------------

awaitable<void> listener(tcp::acceptor acceptor)
{
    chat_room room;

    for (;;)
    {
        std::make_shared<chat_session>(
            co_await acceptor.async_accept(use_awaitable),
            room
            )->start();
    }
}

static void test_start_server() {

    try
    {
        boost::asio::io_context io_context(1);

        for (int i = 1; i < 2; ++i)
        {
            unsigned short port = std::atoi("4059");
            co_spawn(io_context,
                listener(tcp::acceptor(io_context, { tcp::v4(), 4059 })),
                detached);
        }

        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { io_context.stop(); });

        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

#endif /* USE_COROUTINES */

#endif /* UNIT_TEST */