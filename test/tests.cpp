/******************************************************
 *  @file       test.cpp
 *  @brief      Unit tests implementations
 *
 *  @author     Kalmykov Dmitry
 *  @date       19.08.2021
 *  @modified   19.08.2021
 *  @version    1.0
 */

 /* local C++ headers */
#include "tests.h"

/* std C++ lib headers */
#include <memory>
#include <string>

/* external C++ libs headers -------------------------------- */
/* spdlog C++ lib */
#include <spdlog/spdlog.h>

#if UNIT_TEST  

using testcase_t = enum class Testcase {
    test_RSACryptoAlg = 1,
    test_DHCryptoAlg,
    test_JsonParser,
    test_MongoDbConnect,
};

/* tests declarations --------------------------------------- */

#if TEST_RSA_CRYPTO
static int test_rsa_enc_dec();
#endif /* USER_RSA_CRYPTO */

#if TEST_DH_CRYPTO
static int test_dh_alg();
#endif /* USER_DH_CRYPTO */

#if TEST_PARSE_JSON
static void test_parse_json();
#endif /* TEST_PARSE_JSON */


static void tests_start(testcase_t testcase);


/***************************************************************
 *  @brief  Function calls defined unit tests
 *  @return Unit tests code result
 ***************************************************************/

unittest_code_t init_unit_tests() {

    unittest_code_t ret = UnitestCode::unittest_ok;
    tests_start(Testcase::test_MongoDbConnect);
    return ret;
}

/* tests implementations ---------------------------------- */

#ifdef TEST_MONGO_DB_CONNECT

#include "../db/MongoProcess.h"

#include <cstdint>
#include <iostream>
#include <vector>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>


using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

static int test_mongo_connect() {

    std::shared_ptr<MongoProcessor> mongo = std::make_shared<MongoProcessor>("mongo.ini");
    mongo->InsertNewMessage("{ \"user id\" : \"1234\", \"message\" : \"hello world\"}");

    return 0;
}
#endif // TEST_MONGO_DB_CONNECT

#if TEST_RSA_CRYPTO
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

#if TEST_DH_CRYPTO

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

#if TEST_JTHREAD
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

#if TEST_PARSE_JSON
#include "../utils/json.h"
#include <memory>

static void test_parse_json() {
    std::unique_ptr jsonParser = std::make_unique<JsonParser>();
    jsonParser->HandleRequest("{ \"usermail\" : \"test@test.com\", \
        \"username\" : \"test\", \"password\" : \"testPASS2#$\", \"active\" : true }\r\n",
        JsonParser::json_req_t::authentication_request);
}
#endif /* TEST_PARSE_JSON */

/* ----------------------------------- */
static void tests_start(testcase_t testcase) {

    switch (testcase) {

#if TEST_RSA_CRYPTO
    case Testcase::test_RSACryptoAlg: test_rsa_enc_dec(); break;
#endif /* USER_RSA_CRYPTO */

#if TEST_DH_CRYPTO
    case Testcase::test_DHCryptoAlg: test_dh_alg(); break;
#endif /* USER_DH_CRYPTO */

#if TEST_PARSE_JSON
    case Testcase::test_JsonParser: test_parse_json(); break;
#endif /* TEST_PARSE_JSON */

    case Testcase::test_MongoDbConnect: test_mongo_connect(); break;

    default: spdlog::error("Undefined test case");
    }
}

#endif /* UNIT_TEST */