#include "vector.h"

#include <stdlib.h>
#include <string.h>

typedef struct _Vector
{
  size_t Capacity;
  size_t Size;
  void* Buffer;
} Vector;

Vector* VectorInitialize(size_t initialCapacity)
{
  Vector* vector = malloc(sizeof(Vector));
  if (vector == NULL)
  {
    return NULL;
  }

  vector->Size = 0;
  vector->Capacity = initialCapacity;

  if (initialCapacity == 0)
  {
    vector->Buffer = NULL;
    return vector;
  }

  void* buffer = malloc(initialCapacity);
  if (buffer != NULL)
  {
    vector->Buffer = buffer;
    return vector;
  }

  free(vector);
  return NULL;
}

void VectorUninitialize(Vector* vector)
{
  free(vector->Buffer);
}

void VectorClear(Vector* vector)
{
  vector->Size = 0;
}

size_t VectorGetSize(Vector* vector)
{
  return vector->Size;
}

void* VectorGetBuffer(Vector* vector)
{
  return vector->Buffer;
}

bool VectorAppend(Vector* vector, const void* buffer, size_t size)
{
  size_t newSize = vector->Size + size;
  if (newSize > vector->Capacity)
  {
    void* buffer = malloc(newSize);
    if (buffer == NULL)
    {
      return false;
    }
    vector->Capacity = newSize;
    memcpy(buffer, vector->Buffer, vector->Size);
    
    free(vector->Buffer);    
    vector->Buffer = buffer;
  }

  memcpy((char*)vector->Buffer + vector->Size, buffer, size);
  vector->Size = newSize;

  return true;
}

bool VectorCopyData(Vector* vector, void* buffer, size_t size)
{
  if (size > vector->Size)
  {
    return false;
  }
  
  memcpy(buffer, vector->Buffer, size);
  
  return true;
}