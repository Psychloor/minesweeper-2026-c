#ifndef SET_H
#define SET_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define SET_EMPTY 0
#define SET_OCCUPIED 1
#define SET_DELETED 2

#define SET_DEFINE(Type, Name, HashFn, EqualsFn)                                    \
typedef struct Name {                                                               \
    Type *data;                                                                     \
    unsigned char *states;                                                          \
    int size;                                                                       \
    int capacity;                                                                   \
} Name;                                                                             \
                                                                                    \
static inline bool Name##_create(Name *set, int initialCapacity) {                  \
    if (initialCapacity <= 0) {                                                     \
        initialCapacity = 16;                                                       \
    }                                                                               \
                                                                                    \
    set->data = (Type *)malloc(sizeof(Type) * initialCapacity);                     \
    set->states = (unsigned char *)malloc(sizeof(unsigned char) * initialCapacity); \
                                                                                    \
    if (!set->data || !set->states) {                                               \
        free(set->data);                                                            \
        free(set->states);                                                          \
        set->data = NULL;                                                           \
        set->states = NULL;                                                         \
        set->size = 0;                                                              \
        set->capacity = 0;                                                          \
        return false;                                                               \
    }                                                                               \
                                                                                    \
    memset(set->states, SET_EMPTY, sizeof(unsigned char) * initialCapacity);        \
    set->size = 0;                                                                  \
    set->capacity = initialCapacity;                                                \
    return true;                                                                    \
}                                                                                   \
                                                                                    \
static inline void Name##_destroy(Name *set) {                                      \
    if (!set) {                                                                     \
        return;                                                                     \
    }                                                                               \
                                                                                    \
    free(set->data);                                                                \
    free(set->states);                                                              \
    set->data = NULL;                                                               \
    set->states = NULL;                                                             \
    set->size = 0;                                                                  \
    set->capacity = 0;                                                              \
}                                                                                   \
                                                                                    \
static inline bool Name##_contains(const Name *set, Type value) {                   \
    if (!set || set->capacity <= 0) {                                               \
        return false;                                                               \
    }                                                                               \
                                                                                    \
    uint32_t hash = HashFn(value);                                                  \
    int index = (int)(hash % (uint32_t)set->capacity);                              \
                                                                                    \
    for (int i = 0; i < set->capacity; ++i) {                                       \
        int probe = (index + i) % set->capacity;                                    \
                                                                                    \
        if (set->states[probe] == SET_EMPTY) {                                      \
            return false;                                                           \
        }                                                                           \
                                                                                    \
        if (set->states[probe] == SET_OCCUPIED && EqualsFn(set->data[probe], value)) { \
            return true;                                                            \
        }                                                                           \
    }                                                                               \
                                                                                    \
    return false;                                                                   \
}                                                                                   \
                                                                                    \
static inline bool Name##_resize(Name *set, int newCapacity) {                      \
    Name newSet;                                                                    \
    if (!Name##_create(&newSet, newCapacity)) {                                     \
        return false;                                                               \
    }                                                                               \
                                                                                    \
    for (int i = 0; i < set->capacity; ++i) {                                       \
        if (set->states[i] == SET_OCCUPIED) {                                      \
            Type value = set->data[i];                                              \
            uint32_t hash = HashFn(value);                                          \
            int index = (int)(hash % (uint32_t)newSet.capacity);                    \
                                                                                    \
            for (int j = 0; j < newSet.capacity; ++j) {                             \
                int probe = (index + j) % newSet.capacity;                          \
                if (newSet.states[probe] != SET_OCCUPIED) {                         \
                    newSet.data[probe] = value;                                     \
                    newSet.states[probe] = SET_OCCUPIED;                            \
                    newSet.size++;                                                  \
                    break;                                                          \
                }                                                                   \
            }                                                                       \
        }                                                                           \
    }                                                                               \
                                                                                    \
    free(set->data);                                                                \
    free(set->states);                                                              \
    *set = newSet;                                                                  \
    return true;                                                                    \
}                                                                                   \
                                                                                    \
static inline bool Name##_insert(Name *set, Type value) {                           \
    if ((set->size + 1) * 100 / set->capacity > 70) {                               \
        if (!Name##_resize(set, set->capacity * 2)) {                               \
            return false;                                                           \
        }                                                                           \
    }                                                                               \
                                                                                    \
    uint32_t hash = HashFn(value);                                                  \
    int index = (int)(hash % (uint32_t)set->capacity);                              \
    int firstDeleted = -1;                                                          \
                                                                                    \
    for (int i = 0; i < set->capacity; ++i) {                                       \
        int probe = (index + i) % set->capacity;                                    \
                                                                                    \
        if (set->states[probe] == SET_OCCUPIED) {                                  \
            if (EqualsFn(set->data[probe], value)) {                                \
                return true;                                                        \
            }                                                                       \
        } else if (set->states[probe] == SET_DELETED) {                             \
            if (firstDeleted == -1) {                                               \
                firstDeleted = probe;                                               \
            }                                                                       \
        } else {                                                                    \
            int target = firstDeleted != -1 ? firstDeleted : probe;                 \
            set->data[target] = value;                                              \
            set->states[target] = SET_OCCUPIED;                                     \
            set->size++;                                                            \
            return true;                                                            \
        }                                                                           \
    }                                                                               \
                                                                                    \
    if (firstDeleted != -1) {                                                       \
        set->data[firstDeleted] = value;                                            \
        set->states[firstDeleted] = SET_OCCUPIED;                                   \
        set->size++;                                                                \
        return true;                                                                \
    }                                                                               \
                                                                                    \
    return false;                                                                   \
}                                                                                   \
                                                                                    \
static inline bool Name##_remove(Name *set, Type value) {                           \
    if (!set || set->capacity <= 0) {                                               \
        return false;                                                               \
    }                                                                               \
                                                                                    \
    uint32_t hash = HashFn(value);                                                  \
    int index = (int)(hash % (uint32_t)set->capacity);                              \
                                                                                    \
    for (int i = 0; i < set->capacity; ++i) {                                       \
        int probe = (index + i) % set->capacity;                                    \
                                                                                    \
        if (set->states[probe] == SET_EMPTY) {                                      \
            return false;                                                           \
        }                                                                           \
                                                                                    \
        if (set->states[probe] == SET_OCCUPIED && EqualsFn(set->data[probe], value)) { \
            set->states[probe] = SET_DELETED;                                       \
            set->size--;                                                            \
            return true;                                                            \
        }                                                                           \
    }                                                                               \
                                                                                    \
    return false;                                                                   \
}                                                                                   \
                                                                                    \
static inline void Name##_clear(Name *set) {                                        \
    memset(set->states, SET_EMPTY, sizeof(unsigned char) * set->capacity);          \
    set->size = 0;                                                                  \
}                                                                                   \
                                                                                    \
static inline bool Name##_is_empty(const Name *set) {                               \
    return set->size == 0;                                                          \
}

#endif
