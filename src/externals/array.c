#include "gr-base/externals/array.h"

array* array_create(size_t num, size_t size)
{
    array* arr = (array*)malloc(sizeof(array) + (num * size));
    if (arr == NULL)
        return NULL;

    arr->length = num;

    return arr;
}

void array_free(array* arr)
{
    free(arr);
}

array* array_resize(array* arr, size_t num, size_t size)
{
    if (arr->length > num)
        return arr;

    arr = realloc(arr, sizeof(array) + (num * size));
    arr->length = num;
    return arr;
}


