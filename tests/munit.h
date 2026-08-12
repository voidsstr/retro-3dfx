/* munit.h - minimal single-header C test framework for the driver-logic tests.
 *
 * Each test file defines tests with TEST(name){...} and lists them in a
 * RUN_TESTS(...) main(). Assertions: CHECK(cond, msg), CHECK_EQ_U(a,b),
 * CHECK_EQ_I(a,b). A failing assertion prints file:line and marks the test
 * failed but keeps going to the next test. main() returns non-zero if any
 * test failed, so run_native.sh / CI can gate on it.
 *
 * These are PURE-LOGIC tests: they replicate or #include the exact arithmetic
 * of a shipped driver fix and assert the invariant the fix established, with a
 * comment pointing at the source file:function. If someone regresses the fix,
 * the invariant breaks here at build/test time instead of on the Voodoo.
 */
#ifndef MUNIT_H
#define MUNIT_H
#include <stdio.h>
#include <stdint.h>

static int  munit_fails = 0;     /* failures in the current test            */
static int  munit_total_fails = 0;
static int  munit_total_tests = 0;

#define TEST(name) static void name(void)

#define CHECK(cond, msg) do {                                               \
    if (!(cond)) {                                                          \
        munit_fails++;                                                      \
        fprintf(stderr, "    FAIL %s:%d: %s\n", __FILE__, __LINE__, (msg)); \
    }                                                                       \
} while (0)

#define CHECK_EQ_U(a, b) do {                                              \
    unsigned long long _a = (unsigned long long)(a);                       \
    unsigned long long _b = (unsigned long long)(b);                       \
    if (_a != _b) {                                                         \
        munit_fails++;                                                      \
        fprintf(stderr, "    FAIL %s:%d: %s (%llu) != %s (%llu)\n",         \
                __FILE__, __LINE__, #a, _a, #b, _b);                        \
    }                                                                       \
} while (0)

#define CHECK_EQ_I(a, b) do {                                              \
    long long _a = (long long)(a);                                         \
    long long _b = (long long)(b);                                         \
    if (_a != _b) {                                                         \
        munit_fails++;                                                      \
        fprintf(stderr, "    FAIL %s:%d: %s (%lld) != %s (%lld)\n",         \
                __FILE__, __LINE__, #a, _a, #b, _b);                        \
    }                                                                       \
} while (0)

#define RUN(test) do {                                                     \
    munit_fails = 0;                                                        \
    munit_total_tests++;                                                    \
    test();                                                                 \
    if (munit_fails) { munit_total_fails++; printf("  [FAIL] %s\n", #test);}\
    else             { printf("  [ ok ] %s\n", #test); }                    \
} while (0)

#define MUNIT_MAIN(suite_name, body)                                       \
    int main(void) {                                                       \
        printf("== %s ==\n", suite_name);                                  \
        body                                                               \
        printf("-- %s: %d/%d tests passed --\n", suite_name,               \
               munit_total_tests - munit_total_fails, munit_total_tests);  \
        return munit_total_fails ? 1 : 0;                                  \
    }

#endif /* MUNIT_H */
