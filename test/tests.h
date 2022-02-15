/******************************************************
 *  @file   test.h
 *  @brief  Unit tests declarations
 *
 *  @author Kalmykov Dmitry
 *  @date   19.08.2021
 */

#pragma once

#define UNIT_TEST true // false or true

#if UNIT_TEST     

using unittest_code_t = enum class UnitestCode {
    unittest_ok = 0,
};

/* RSA crypto alg tests */
#define TEST_RSA_CRYPTO         0
/* Diffie-Hellman crypto alg tests */
#define TEST_DH_CRYPTO          0
/* jthread tests */
#define TEST_JTHREAD            0
/* coroutines tests */
#define TEST_COROUTINES         0
/* json parser tests */
#define TEST_PARSE_JSON         0

#define TEST_MONGO_DB_CONNECT   1

extern unittest_code_t init_unit_tests();

#endif /* UNIT_TEST */