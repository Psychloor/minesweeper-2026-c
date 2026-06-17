#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define VECTOR_DEFINE(Type, Name)                                                   \
typedef struct Name {                                                               \
    Type *data;                                                                     \
    int size;                                                                       \
    int capacity;                                                                   \
} Name;                                                                             \
                                                                                    \
static inline bool Name##_create(Name *vector, int initialCapacity) {               \
    if (initialCapacity <= 0) {                                                     \
        initialCapacity = 4;                                                        \
    }                                                                               \
                                                                                    \
    vector->data = (Type *)malloc(sizeof(Type) * initialCapacity);                  \
    if (!vector->data) {                                                            \
        vector->size = 0;                                                           \
        vector->capacity = 0;                                                       \
        return false;                                                               \
    }                                                                               \
                                                                                    \
    vector->size = 0;                                                               \
    vector->capacity = initialCapacity;                                             \
    return true;                                                                    \
}                                                                                   \
                                                                                    \
static inline void Name##_destroy(Name *vector) {                                   \
    if (!vector) {                                                                  \
        return;                                                                     \
    }                                                                               \
                                                                                    \
    free(vector->data);                                                             \
    vector->data = NULL;                                                            \
    vector->size = 0;                                                               \
    vector->capacity = 0;                                                           \
}                                                                                   \
                                                                                    \
static inline bool Name##_resize(Name *vector, int newCapacity) {                   \
    if (newCapacity <= 0) {                                                         \
        newCapacity = 1;                                                            \
    }                                                                               \
                                                                                    \
    Type *newData = (Type *)realloc(vector->data, sizeof(Type) * newCapacity);      \
    if (!newData) {                                                                 \
        return false;                                                               \
    }                                                                               \
                                                                                    \
    vector->data = newData;                                                         \
    vector->capacity = newCapacity;                                                 \
                                                                                    \
    if (vector->size > vector->capacity) {                                          \
        vector->size = vector->capacity;                                            \
    }                                                                               \
                                                                                    \
    return true;                                                                    \
}                                                                                   \
                                                                                    \
static inline bool Name##_push(Name *vector, Type value) {                          \
    if (vector->size >= vector->capacity) {                                         \
        int newCapacity = vector->capacity == 0 ? 4 : vector->capacity * 2;         \
        if (!Name##_resize(vector, newCapacity)) {                                  \
            return false;                                                           \
        }                                                                           \
    }                                                                               \
                                                                                    \
    vector->data[vector->size++] = value;                                           \
    return true;                                                                    \
}                                                                                   \
                                                                                    \
static inline bool Name##_pop(Name *vector, Type *outValue) {                       \
    if (vector->size <= 0) {                                                        \
        return false;                                                               \
    }                                                                               \
                                                                                    \
    vector->size--;                                                                 \
                                                                                    \
    if (outValue) {                                                                 \
        *outValue = vector->data[vector->size];                                     \
    }                                                                               \
                                                                                    \
    return true;                                                                    \
}                                                                                   \
                                                                                    \
static inline Type *Name##_get(Name *vector, int index) {                           \
    if (index < 0 || index >= vector->size) {                                       \
        return NULL;                                                                \
    }                                                                               \
                                                                                    \
    return &vector->data[index];                                                    \
}                                                                                   \
                                                                                    \
static inline const Type *Name##_get_const(const Name *vector, int index) {         \
    if (index < 0 || index >= vector->size) {                                       \
        return NULL;                                                                \
    }                                                                               \
                                                                                    \
    return &vector->data[index];                                                    \
}                                                                                   \
                                                                                    \
static inline bool Name##_set(Name *vector, int index, Type value) {                \
    if (index < 0 || index >= vector->size) {                                       \
        return false;                                                               \
    }                                                                               \
                                                                                    \
    vector->data[index] = value;                                                    \
    return true;                                                                    \
}                                                                                   \
                                                                                    \
static inline void Name##_clear(Name *vector) {                                     \
    vector->size = 0;                                                               \
}                                                                                   \
                                                                                    \
static inline bool Name##_is_empty(const Name *vector) {                            \
    return vector->size == 0;                                                       \
}

#endif
