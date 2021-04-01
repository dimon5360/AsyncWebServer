/*********************************************
 *
 *
 */

 /* std C++ lib headers */
#include <iostream>

 /* boost C++ lib headers */
#include <boost/format.hpp>

 /* local C++ headers */
#include "serv/AsyncTcpServer.h"
#include "conn/ConnectionManager.h"
#include "db/PostgresProcessor.h"

/* Build v.0.0.4 from 02.04.2021 */
const uint32_t PATCH = 4;
const uint32_t MINOR = 0;
const uint32_t MAJOR = 0;

#include <windows.h>

/* separate thread for processing of SPACE key press (to close application) */
static void EscapeWait() {
    while (GetAsyncKeyState(VK_SPACE) == 0) {
        Sleep(10);
    }
    exit(0);
}

#define UNIT_TEST 1
#if UNIT_TEST
static int test_rsa_enc_dec();
static int test_dh_alg();
#endif /* UNIT_TEST */

int main()
{
#if UNIT_TEST
    return test_dh_alg();
#endif /* UNIT_TEST */

    /* for corrent output boost error messages */
    SetConsoleOutputCP(1251);
    std::cout << boost::format("Hello. Application version is %1%.%2%.%3%\n") % MAJOR % MINOR % PATCH;
    std::cout << "Press SPACE to exit...\n";

    try
    {
        /* conctruct db class */
        std::unique_ptr<PostgresProcessor> db = std::make_unique<PostgresProcessor>();
        /* separate thread to monitor SPACE key pressing */
        std::thread ext(&EscapeWait);
        /* start tcp server */
        async_tcp_server::StartTcpServer();
        ext.join();
    }
    catch (std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
    }
    return 0;
}

#if UNIT_TEST
#include "crypto/rsa.h"

static int test_rsa_enc_dec() {

    RSA_Crypto rsa;
    std::string msg = "hello server";

    std::cout << "\nThe original message is: " << msg << std::endl;
    auto encrypted_msg = rsa.Encrypt(msg);
    std::string decrypted_msg = rsa.Decrypt(encrypted_msg);
    std::cout << "\nThe decrypted message is: " << decrypted_msg << std::endl;

    return 0;
}

#include "crypto/dh.h"
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
#endif /* UNIT_TEST */