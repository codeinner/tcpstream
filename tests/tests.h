#include "log.h"

#include <stdbool.h>

#define test_assert_eq(a, b) if (a != b) { \
  LOG_ERROR("%s: check failed %s", __FUNCTION__, #a" != "#b); \
  return false; \
}

#define test_assert_neq(a, b) if (a == b) { \
  LOG_ERROR("%s: check failed %s", __FUNCTION__, #a" == "#b); \
  return false; \
}
