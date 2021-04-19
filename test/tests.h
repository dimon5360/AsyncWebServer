
#pragma once

#define UNIT_TEST 0

#if UNIT_TEST     

#define USER_RSA_CRYPTO 0
#define USER_DH_CRYPTO  0
#define USE_JTHREAD     0
#define USE_COROUTINES  0

extern int tests();

#endif /* UNIT_TEST */