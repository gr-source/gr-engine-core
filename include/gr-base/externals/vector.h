#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

typedef struct vector_header
{
    size_t capacity;
    size_t length;
} vector_header;

#define vector_push(vec, e) \
    do { \
        if ((vec) == NULL) \
        { \
            vector_header* header = (vector_header*)malloc(sizeof(*(vec)) * 2 + sizeof(vector_header)); \
            assert(header != NULL && strerror(errno)); \
            header->capacity = 2; \
            header->length = 0; \
            (vec) = (__typeof__(vec))((vector_header*)(header) + 1); \
        } \
        vector_header* header = (vector_header*)(vec) - 1; \
        if (header->length >= header->capacity) \
        { \
            size_t new_capacity = header->capacity * 2; \
            vector_header* new_header = (vector_header*)realloc(header, sizeof(*(vec)) * new_capacity + sizeof(vector_header)); \
            assert(new_header != NULL && strerror(errno)); \
            new_header->capacity = new_capacity; \
            (vec) = (__typeof__(vec))((vector_header*)new_header + 1); \
            header = new_header; \
        } \
        (vec)[header->length++] = (e); \
    } while (0)

#define vector_length(arr) ((vector_header*)arr - 1)->length

#define vector_free(arr) free(((vector_header*)arr - 1))

#endif // !VECTOR_H
