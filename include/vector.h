#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stddef.h>

typedef struct _Vector Vector;

Vector* VectorInitialize(size_t initialCapacity);
void VectorUninitialize(Vector* vector);
void VectorClear(Vector* vector);
void* VectorGetBuffer(Vector* vector);
size_t VectorGetSize(Vector* vector);
bool VectorAppend(Vector* vector, const void* buffer, size_t size);
bool VectorCopyData(Vector* vector, void* buffer, size_t size);

#endif