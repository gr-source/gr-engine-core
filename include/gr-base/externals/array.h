#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>

typedef struct array
{
    size_t length;
    void* vector[];
} array;

array* array_create(size_t num, size_t size);

void array_free(array* arr);

array* array_resize(array* arr, size_t num, size_t size);

#endif // !ARRAY_H


