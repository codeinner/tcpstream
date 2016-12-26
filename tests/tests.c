#include "errors.h"

#include "tests.h"
#include "test_packet_processing.h"
#include "test_vector.h"

#define test(a, b, c) (a() ? ++b : ++c);

int main(int argc, char* argv[])
{
  int passed = 0;
  int failed = 0;

  test(test_vector, passed, failed);
  test(test_packet_processing, passed, failed);

  printf("Passed: %d\n"
         "Failed: %d\n", passed, failed);

  return failed;
}
