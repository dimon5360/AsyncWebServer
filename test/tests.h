/******************************************************
 *  @file   test.h
 *  @brief  Unit tests declarations
 *
 *  @author Kalmykov Dmitry
 *  @date   19.08.2021
 */

#pragma once

#define UNIT_TEST false // false or true

#if UNIT_TEST     

using unittest_code_t = enum class UnitestCode {
    unittest_ok = 0,
};

#define TEST_RSA_CRYPTO         0
#define TEST_DH_CRYPTO          0
#define TEST_PARSE_JSON         0
#define TEST_ASYNC_TASK         1
#define TEST_MONGO_DB_CONNECT   0

extern unittest_code_t init_unit_tests();

#endif /* UNIT_TEST */