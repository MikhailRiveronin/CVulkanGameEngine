#include <core/logger.h>
#include <math/kmath.h>

/**
 * @brief Expects expected to be equal to actual.
 */
#define expect_should_be(expected, actual)                                                              \
    if (actual != expected) {                                                                           \
        LOG_ERROR("--> Expected %lld, but got: %lld. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
        return FALSE;                                                                                   \
    }

/**
 * @brief Expects expected to NOT be equal to actual.
 */
#define expect_should_not_be(expected, actual)                                                                   \
    if (actual == expected) {                                                                                    \
        LOG_ERROR("--> Expected %d != %d, but they are equal. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
        return FALSE;                                                                                            \
    }

/**
 * @brief Expects expected to be actual given a tolerance of K_FLOAT_EPSILON.
 */
#define expect_float_to_be(expected, actual)                                                        \
    if (kabs(expected - actual) > 0.001f) {                                                         \
        LOG_ERROR("--> Expected %f, but got: %f. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
        return FALSE;                                                                               \
    }

/**
 * @brief Expects actual to be TRUE.
 */
#define expect_to_be_true(actual)                                                      \
    if (actual != TRUE) {                                                              \
        LOG_ERROR("--> Expected TRUE, but got: FALSE. File: %s:%d.", __FILE__, __LINE__); \
        return FALSE;                                                                  \
    }

/**
 * @brief Expects actual to be FALSE.
 */
#define expect_to_be_false(actual)                                                     \
    if (actual != FALSE) {                                                             \
        LOG_ERROR("--> Expected FALSE, but got: TRUE. File: %s:%d.", __FILE__, __LINE__); \
        return FALSE;                                                                  \
    }
