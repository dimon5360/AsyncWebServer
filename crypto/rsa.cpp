/*********************************************
 *
 *
 */
 /* std C++ lib headers */
#include <iostream>
#include <random>
#include <cmath>
#include <vector>

/* local C++ headers */
#include "rsa.h"

/* getter for new random number to send in server */
int32_t RandomGen::GenRandomNumber() {
    std::default_random_engine e1(r());
    std::uniform_int_distribution<int32_t> uniform_dist(MIN, MAX);

    int value = uniform_dist(e1);
    return static_cast<int32_t>(value);
}

bool RSA_Crypto::isPrime(int32_t prime)
{
    int32_t i, j;

    j = (int32_t)sqrt((long double)prime);

    for (i = 2; i <= j; i++) {
        if (prime % i == 0) {
            return false;
        }
    }

    return true;
}

int32_t RSA_Crypto::calculateE(int32_t t)
{
    int64_t e;

    for (e = 2; e < t; e++)
    {
        if (greatestCommonDivisor(e, t) == 1)
        {
            return e;
        }
    }

    return -1;
}

int32_t RSA_Crypto::greatestCommonDivisor(int32_t e, int32_t t)
{
    while (e > 0)
    {
        int32_t myTemp;

        myTemp = e;
        e = t % e;
        t = myTemp;
    }

    return t;
}

int32_t RSA_Crypto::calculateD(int32_t e, int32_t t)
{
    int64_t d;
    int64_t k = 1;

    while (1)
    {
        k = k + t;

        if (k % e == 0)
        {
            d = (k / e);
            return d;
        }
    }

}

std::vector<int32_t> RSA_Crypto::encrypt(std::string msg_)
{
    std::vector<int32_t> res(0, msg_.size());
    for (int32_t i = 0; i < msg_.length(); i++) {
        res.push_back(EncryptChar(msg_[i], e, n));
    }
    return res;
}

std::string RSA_Crypto::decrypt(std::vector<int32_t> msg_)
{    
    std::string str;
    for (int32_t i = 0; i < msg_.size(); i++) {
        //std::cout << msg_[i] << " ";
        str += DecryptChar(msg_[i], d, n);
    }
    //std::cout << "\n";
    return str;
}

int32_t RSA_Crypto::EncryptChar(int32_t i, int32_t e, int32_t n)
{
    int64_t current, result;

    current = i - 97;
    result = 1;

    for (int32_t j = 0; j < e; j++)
    {
        result = result * current;
        result = result % n;
    }

    return result;
}

int32_t RSA_Crypto::DecryptChar(int32_t i, int32_t d, int32_t n)
{
    int64_t current, result;

    current = i;
    result = 1;

    for (int32_t j = 0; j < d; j++)
    {
        result = result * current;
        result = result % n;
    }

    return result + 97;
}


RSA_Crypto::RSA_Crypto() {
    std::cout << "Construct new RSA crypto class\n";

    /* construct random generator */
    gen_ = std::make_unique<RandomGen>(2500, 3700);

    /* Initialize two random prime numbers: p, q */
    p = gen_->GenRandomNumber();
    while (!isPrime(p)) {
        p = gen_->GenRandomNumber();
    }

    q = gen_->GenRandomNumber();
    while (!isPrime(q) && q != p) {
        q = gen_->GenRandomNumber();
    }

    //std::cout << "p = " << p << ", q = " << q << std::endl;

    n = p * q;
    //std::cout << "\nResult of computing n = p*q = " << n << std::endl;
    t = (p - 1) * (q - 1);
    //std::cout << "Result of computing Euler's totient function:\t t = " << t << std::endl;
    e = calculateE(t);
    d = calculateD(e, t);
    public_key = std::make_pair(n, e);
    private_key = std::make_pair(n, d);
}

RSA_Crypto::~RSA_Crypto() {

    std::cout << "Destruct RSA crypto class\n";
}