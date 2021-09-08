#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "arraylist.h"


/*
typedef struct ArrayList {
    char *arr;
    size_t elementSize;
    ssize_t size;
    ssize_t capacity;
    //sem_t safe;
} ArrayList;
*/

ArrayList *ArrayList_init(ssize_t elementSize) {

    ArrayList *list = NULL;
    if (elementSize <= 0) {
        return NULL;
    }

    list = (ArrayList*) malloc(sizeof(ArrayList));
    if (!list) {
        return NULL;
    }

    if (sem_init(&(list->safe), 0, 1)) {
        free(list);
        return NULL;
    }

    list->arr = calloc(INITIAL_CAPACITY, elementSize);
    if (!(list->arr)) {
        free(list);
        if (sem_destroy(&(list->safe)) && errno != EINVAL) {}
        return NULL;
    }

    list->size = 0;
    list->capacity = INITIAL_CAPACITY;
    list->elementSize = elementSize;
    return list;
}

void ArrayList_destroy(ArrayList *list) {
    if (list) {
        if (sem_destroy(&(list->safe)) && errno != EINVAL) {}
        free(list->arr);
        free(list);
    }
}

size_t ArrayList_size(ArrayList *list) {

    int size;
    if (sem_wait(&(list->safe))) {
        return -1;
    }

    if (!list) {
        if (sem_post(&(list->safe))) {
            return -1;
        }
        return -1;
    }
    size = list->size;

    if (sem_post(&(list->safe))) {
        return -1;
    }

    return size;
}

int ArrayList_isEmpty(ArrayList *list) {

    int isEmpty;
    if (sem_wait(&(list->safe))) {
        return -1;
    }

    if (!list) {
        if (sem_post(&(list->safe))) {
            return -1;
        }
        return -1;
    }

    isEmpty = (list->size == 0);

    if (sem_post(&(list->safe))) {
        return -1;
    }

    return isEmpty;
}

// do not call from outside
int ArrayList_ensureCapacity(ArrayList* list, ssize_t capacity) {

    char *tempArr = NULL;

    if (!list) {
        return -1;
    }

    ssize_t newCapacity = list->capacity;
    while (newCapacity < capacity) {
        newCapacity *= 2;

        tempArr = realloc(list->arr, newCapacity*list->elementSize);
        if (!tempArr) {
            return -1;
        }
        list->capacity = newCapacity;
        list->arr = tempArr;
    }

    return 0;
}

int ArrayList_append(ArrayList *list, const void *element) {
    if (!list || !element) {
        return -1;
    }

    return ArrayList_add(list, list->size, element);
}

int ArrayList_add(ArrayList *list, ssize_t index, const void *element) {

    char *source = NULL;
    char *dest = NULL;

    if (sem_wait(&(list->safe))) {
        return -1;
    }

    if (!list || index > list->size || !element) {
        if (sem_post(&(list->safe))) {
            return -1;
        }
        return -1;
    }

    if (ArrayList_ensureCapacity(list, list->size+1)) {
        if (sem_post(&(list->safe))) {
            return -1;
        }
        return -1;
    }

    source = list->arr + index*list->elementSize;
    dest = source + list->elementSize;

    memmove(dest, source, list->elementSize * (list->size - index));
    memcpy(source, element, list->elementSize);
    ++(list->size);

    if (sem_post(&(list->safe))) {
        return -1;
    }

    return 0;
}

int ArrayList_remove(ArrayList *list, ssize_t index) {

    char *source = NULL;
    char *dest = NULL;

    if (sem_wait(&(list->safe))) {
        return -1;
    }

    if (!list || index >= list->size) {
        if (sem_post(&(list->safe))) {
            return -1;
        }
        return -1;
    }

    source = list->arr + (index+1) * list->elementSize;
    dest = list->arr + index*list->elementSize;
    size_t bytes = (list->size-(index+1)) * list->elementSize;

    memmove(dest, source, bytes);
    --(list->size);

    if (sem_post(&(list->safe))) {
        return -1;
    }

    return 0;    
}

void ArrayList_get(ArrayList *list, ssize_t index, void *ret) {
    
    if (sem_wait(&(list->safe))) {
        ret = NULL;
        return;
    }

    if (!list || index >= list->size) {
        ret = NULL;
        if (sem_post(&(list->safe))) {
            return;
        }
        return;
    }

    memcpy(ret, (list->arr + (index * list->elementSize)), list->elementSize);

    if (sem_post(&(list->safe))) {
        ret = NULL;
        return;
    }
}