/*********************************************
 *
 *
 */
#pragma once

 /* std C++ lib headers */
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <iomanip>
#include <random>
#include <cmath>

class RandomGen {
private:
    /* random number generator object */
    std::random_device r;
    /* borders of random numbers */
    int32_t MIN = 0, MAX = 8192;

public:

    /* constructor */
    RandomGen(int32_t min, int32_t max) :
        MIN(min),
        MAX(max)
    {
        std::cout << "Construct new random numbers generator class\n";
    }

    /* destructor */
    ~RandomGen() {
        std::cout << "Destruct random generator class\n";
    }

    /* getter for new random number to send in server */
    int32_t GenRandomNumber();
};

class RSA_Crypto {

private:

    std::pair<int32_t, int32_t> private_key, public_key;
    std::string decripted_message, encripted_message;

    int32_t p, q, n, t, e, d;

    std::unique_ptr<RandomGen> gen_;

    bool isPrime(int32_t prime);
    int32_t calculateE(int32_t t);
    int32_t greatestCommonDivisor(int32_t e, int32_t t);
    int32_t calculateD(int32_t e, int32_t t);

    int32_t EncryptChar(int32_t i, int32_t e, int32_t n);
    int32_t DecryptChar(int32_t i, int32_t d, int32_t n);

public:

    /* constructor */
    RSA_Crypto();
    /* destructor */
    ~RSA_Crypto();

    std::vector<int32_t> encrypt(std::string msg_);
    std::string decrypt(std::vector<int32_t> msg_);
};