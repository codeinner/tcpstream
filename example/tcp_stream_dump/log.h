#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#define LOG(a, ...) printf(a"\n", ##__VA_ARGS__)
#define LOG_ERROR(a, ...) fprintf(stderr, "Error: "a"\n", ##__VA_ARGS__)

#endif