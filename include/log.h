#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>

#define LOG_CHECK(expression) (!expression ? (printf("Check failed: %s\n", #expression), expression) : expression)
#define LOG_DEBUG(a, ...) printf("Debug: "a"\n", ##__VA_ARGS__)
#define LOG(a, ...) printf(a"\n", ##__VA_ARGS__)
#define LOG_ERROR(a, ...) fprintf(stderr, "Error: "a"\n", ##__VA_ARGS__)

#endif