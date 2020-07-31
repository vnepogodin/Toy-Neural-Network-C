/*
 * $Id: arraylist.h,v 1.4 2006/01/26 02:16:28 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

/**
 * @file
 * @brief Internal methods for working with json_type_array objects.
 *        Although this is exposed by the json_object_get_array() method,
 *        it is not recommended for direct use.
 */
#ifndef __ARRAYLIST_H__
#define __ARRAYLIST_H__

#include <stddef.h>

typedef struct _Array_List array_list;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate an array_list of the desired size.
 *
 * If possible, the size should be chosen to closely match
 * the actual number of elements expected to be used.
 * If the exact size is unknown, there are tradeoffs to be made:
 * - too small - the array_list code will need to call realloc() more
 *   often (which might incur an additional memory copy).
 * - too large - will waste memory, but that can be mitigated
 *   by calling array_list_shrink() once the final size is known.
 *
 * @see array_list_shrink
 */
extern array_list* array_list_new(void(*)(void*), const int);

extern void array_list_free(array_list *);

extern void *array_list_get_idx(array_list *, const unsigned long);

extern int array_list_add(array_list *, const void* const);

extern unsigned long array_list_length(const array_list *);

/**
 * Shrink the array list to just enough to fit the number of elements in it,
 * plus empty_slots.
 */
extern int array_list_shrink(array_list *, const unsigned long);

#ifdef __cplusplus
}
#endif

#endif /* __ARRAYLIST_H__ */
