/*
 * $Id: arraylist.c,v 1.4 2006/01/26 02:16:28 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */
#include "arraylist.h"

#include <limits.h> /* ULONG_MAX */
#include <stdlib.h> /* malloc, calloc */

struct _Array_List {
    unsigned long length;
    unsigned long size;

    void (*free_fn)(void*);

    void** array;
};

static int array_list_expand_internal(array_list* arr, const unsigned long max) {
    register int result = 0;

    if (max >= arr->size) {
        register unsigned long new_size = 0UL;
        /* Avoid undefined behaviour on unsigned long overflow */
        if (arr->size >= ULONG_MAX / 2)
            new_size = max;
        else {
            new_size = arr->size << 1UL;
            if (new_size < max)
                new_size = max;
        }
        result = -1;
        if (new_size <= (~((unsigned long)0)) / sizeof(void*)) {
            register void* t = realloc(arr->array, new_size * sizeof(void*));
            if (t != NULL) {
                arr->array = (void**)t;
                arr->size = new_size;
                result = 0;
            }
        }
    }

    return result;
}

array_list* array_list_new(void (*free_fn)(void*), const int initial_size) {
    array_list* result = NULL;

    if (!((initial_size < 0) || ((unsigned long)initial_size >= (ULONG_MAX / sizeof(void*))))) {
        array_list* arr = (array_list*)malloc(sizeof(array_list));

        if (arr != NULL) {
            arr->size = (unsigned long)initial_size;
            arr->length = 0;
            arr->free_fn = free_fn;
            arr->array = (void**)calloc((unsigned long)initial_size, 8UL);

            if (arr->array != NULL)
                result = arr;
            else
                free(arr);
        }
    }

    return result;
}

void array_list_free(array_list* arr) {
    register unsigned long i = 0UL;
    while (i < arr->length) {
        if (arr->array[i] != NULL)
            arr->free_fn(arr->array[i]);

        ++i;
    }
    free(arr->array);
    free(arr);
}

void* array_list_get_idx(array_list* arr, const unsigned long i) {
    register void* result = NULL;
    if (i < arr->length)
        result = arr->array[i];

    return result;
}

int array_list_shrink(array_list* arr, const unsigned long empty_slots) {
    if (empty_slots >= (ULONG_MAX / (sizeof(void*) - arr->length)))
        return -1;

    register unsigned long new_size = arr->length + empty_slots;
    if (new_size == arr->size)
        return 0;

    if (new_size > arr->size)
        return array_list_expand_internal(arr, new_size);

    if (new_size == 0)
        new_size = 1;

    register void* t = realloc(arr->array, new_size * sizeof(void*));
    if (t == NULL)
        return -1;

    arr->array = (void**)t;
    arr->size = new_size;
    return 0;
}

/* Repeat some of array_list_put_idx() so we can skip several
 * checks that we know are unnecessary when appending at the end
 */
int array_list_add(array_list* arr, const void* const data) {
    register unsigned long idx = arr->length;
    register int result = -1;

    if ((idx <= (ULONG_MAX - 1)) && (array_list_expand_internal(arr, idx + 1) != 1)) {
        arr->array[idx] = (void*)data;
        arr->length++;

        result = 0;
    }
    return result;
}

inline unsigned long array_list_length(const array_list* arr) {
    return arr->length;
}
