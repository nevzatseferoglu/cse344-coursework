#ifndef _ARRAYLIST
#define _ARRAYLIST

#include <semaphore.h>
#define INITIAL_CAPACITY 10

typedef struct ArrayList {
    char *arr;
    size_t elementSize;
    ssize_t size;
    ssize_t capacity;
    sem_t safe;
} ArrayList;

ArrayList *ArrayList_init(ssize_t elementSize);
void ArrayList_destroy(ArrayList *list);
size_t ArrayList_size(ArrayList *list);
int ArrayList_isEmpty(ArrayList *list);
int ArrayList_append(ArrayList *list, const void *element);
int ArrayList_add(ArrayList *list, ssize_t index, const void *element);
int ArrayList_remove(ArrayList *list, ssize_t index);
void ArrayList_get(ArrayList *list, ssize_t index, void *ret);
int ArrayList_ensureCapacity(ArrayList* list, ssize_t capacity);

#endif /* _ARRAYLIST */