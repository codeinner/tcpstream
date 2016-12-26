#include "test_vector.h"

#include <vector.h>

#include <string.h>

bool test_vector()
{
  const unsigned char data[] = {1, 2, 3, 4, 5, 6, 7, 8};
  const unsigned char data2[] = {9, 10, 11, 12, 13, 14};
  unsigned char data3[sizeof(data) + sizeof(data2)];
  Vector* vector;
  const void* oldBuffer;

  vector = VectorInitialize(0);
  
  test_assert_neq(vector, NULL);
  test_assert_eq(VectorGetSize(vector), 0);
  
  VectorUninitialize(vector);

  vector = VectorInitialize(sizeof(data));
  
  test_assert_neq(vector, NULL);
  
  oldBuffer = VectorGetBuffer(vector);

  test_assert_neq(oldBuffer, NULL);
  test_assert_eq(VectorGetSize(vector), 0);

  test_assert_eq(VectorAppend(vector, data, sizeof(data)), true);
  test_assert_eq(VectorGetBuffer(vector), oldBuffer);
  test_assert_eq(VectorGetSize(vector), sizeof(data));
  test_assert_eq(memcmp(VectorGetBuffer(vector), data, sizeof(data)), 0);

  test_assert_eq(VectorAppend(vector, data2, sizeof(data2)), true);
  test_assert_neq(VectorGetBuffer(vector), oldBuffer);
  test_assert_eq(VectorGetSize(vector), sizeof(data) + sizeof(data2));
  test_assert_eq(memcmp(VectorGetBuffer(vector), data, sizeof(data)), 0);
  test_assert_eq(memcmp((char*)VectorGetBuffer(vector) + sizeof(data), data2, sizeof(data2)), 0);

  memset(data3, 0, sizeof(data3));
  
  test_assert_eq(VectorCopyData(vector, data3, VectorGetSize(vector)), true);

  test_assert_eq(memcmp(data3, data, sizeof(data)), 0);
  test_assert_eq(memcmp((char*)data3 + sizeof(data), data2, sizeof(data2)), 0);

  oldBuffer = VectorGetBuffer(vector);

  VectorClear(vector);

  test_assert_eq(VectorGetBuffer(vector), oldBuffer);
  test_assert_eq(VectorGetSize(vector), 0);

  test_assert_eq(VectorAppend(vector, data, sizeof(data)), true);
  test_assert_eq(VectorGetSize(vector), sizeof(data));
  test_assert_eq(memcmp(VectorGetBuffer(vector), data, sizeof(data)), 0);

  VectorUninitialize(vector);

  return true;
}