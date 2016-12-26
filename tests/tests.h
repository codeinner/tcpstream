#ifndef TESTS_H
#define TESTS_H

#include <stdbool.h>
#include <stdio.h>

#define LOG_ERROR(a, ...) fprintf(stderr, "Error: "a"\n", ##__VA_ARGS__)

#define test_assert_eq(a, b) if (a != b) { \
  LOG_ERROR("%s: check failed %s", __FUNCTION__, #a" != "#b); \
  return false; \
}

#define test_assert_neq(a, b) if (a == b) { \
  LOG_ERROR("%s: check failed %s", __FUNCTION__, #a" == "#b); \
  return false; \
}

#endif