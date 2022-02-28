/******************************************************
 *  @file       test.cpp
 *  @brief      Unit tests implementations
 *
 *  @author     Kalmykov Dmitry
 *  @date       19.08.2021
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
    test_AsyncTask,
};

static void tests_start(testcase_t testcase, unittest_code_t& ret);

unittest_code_t init_unit_tests() {

    unittest_code_t ret = UnitestCode::unittest_ok;
    tests_start(Testcase::test_AsyncTask, ret);
    return ret;
}

/* tests implementations ---------------------------------- */

#if TEST_RSA_CRYPTO
static int test_rsa_enc_dec();
#endif // TEST_RSA_CRYPTO
#if TEST_DH_CRYPTO
static int test_dh_alg();
#endif // TEST_DH_CRYPTO
#if TEST_PARSE_JSON
static void test_parse_json();
#endif // TEST_PARSE_JSON
#if TEST_MONGO_DB_CONNECT
static int test_mongo_connect();
#endif // TEST_MONGO_DB_CONNECT
#if TEST_ASYNC_TASK
static int test_async_task();
#endif // TEST_ASYNC_TASK

/* ----------------------------------- */
static void tests_start(testcase_t testcase, unittest_code_t& ret) {

    switch (testcase) {
#if TEST_RSA_CRYPTO
    case Testcase::test_RSACryptoAlg: ret = (unittest_code_t)test_rsa_enc_dec(); break;
#elif TEST_DH_CRYPTO
    case Testcase::test_DHCryptoAlg: ret = (unittest_code_t)test_dh_alg(); break;
#elif TEST_PARSE_JSON
    case Testcase::test_JsonParser: ret = (unittest_code_t)test_parse_json(); break;
#elif TEST_MONGO_DB_CONNECT
    case Testcase::test_MongoDbConnect: ret = (unittest_code_t)test_mongo_connect(); break;
#elif TEST_ASYNC_TASK
    case Testcase::test_AsyncTask: ret = (unittest_code_t)test_async_task(); break;
#endif 
    default: spdlog::error("Undefined test case");
    }
}

#ifdef TEST_ASYNC_TASK
#include <iostream>     // std::cout
#include <future>       // std::packaged_task, std::future
#include <chrono>       // std::chrono::seconds
#include <thread>       // std::thread, std::this_thread::sleep_for

// count down taking a second for each value:
int countdown (int from, int to) {
  for (int i=from; i!=to; --i) {
    std::cout << i << '\n';
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  std::cout << "Lift off!\n";
  return from-to;
}

static int test_async_task() {

  std::packaged_task<int(int,int)> tsk (countdown);   // set up packaged_task
  std::future<int> ret = tsk.get_future();            // get future

  std::thread th (std::move(tsk),10,0);   // spawn thread to count down from 10 to 0

  // ...

  int value = ret.get();                  // wait for the task to finish and get result

  std::cout << "The countdown lasted for " << value << " seconds.\n";

  th.join();
    return 0;
}
#endif // TEST_ASYNC_TASK

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
#endif // USER_RSA_CRYPTO

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
#endif // USER_DH_CRYPTO

#if TEST_PARSE_JSON
#include "../utils/json.h"
#include <memory>

static void test_parse_json() {
    std::unique_ptr jsonParser = std::make_unique<JsonParser>();
    jsonParser->HandleRequest("{ \"usermail\" : \"test@test.com\", \
        \"username\" : \"test\", \"password\" : \"testPASS2#$\", \"active\" : true }\r\n",
        JsonParser::json_req_t::authentication_request);
}
#endif // TEST_PARSE_JSON
#endif // UNIT_TEST