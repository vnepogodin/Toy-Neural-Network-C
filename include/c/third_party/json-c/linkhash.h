/*
 * $Id: linkhash.h,v 1.6 2006/01/30 23:07:57 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */
#ifndef __LINKHASH_H__
#define __LINKHASH_H__

/**
 * An entry in the hash table
 */
typedef struct _LH_Entry lh_entry;

/**
 * The hash table structure.
 */
typedef struct _LH_Table lh_table;


/**
 * golden prime used in hash functions
 */
#define LH_PRIME 0x9e370001UL

/**
 * The fraction of filled hash buckets until an insert will cause the table
 * to be resized.
 * This can range from just above 0 up to 1.0.
 */
#define LH_LOAD_FACTOR 0.66

/**
 * sentinel pointer value for empty slots
 */
#define LH_EMPTY (void *)-1

/**
 * sentinel pointer value for freed slots
 */
#define LH_FREED (void *)-2

/**
 * default string hash function
 */
#define JSON_C_STR_HASH_DFLT 0

/**
 * perl-like string hash function
 */
#define JSON_C_STR_HASH_PERLLIKE 1

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char lh_entry_getConstant(lh_entry *);

extern lh_entry* lh_entry_getNext(lh_entry *);

extern const void* lh_entry_getK(const lh_entry *);
extern const void* lh_entry_getV(const lh_entry *);

extern void lh_entry_setV(lh_entry *, const void*);

/**
 * Convenience function to create a new linkhash table with char keys.
 *
 * @param size initial table size.
 * @param free_fn callback function used to free memory for entries.
 * @return On success, a pointer to the new linkhash table is returned.
 * 	On error, a null pointer is returned.
 */
extern lh_table* lh_kchar_table_new(const int, void (*const)(lh_entry *));

/**
 * Free a linkhash table.
 *
 * If a lh_entry_free_fn callback free function was provided then it is
 * called for all entries in the table.
 *
 * @param t table to free.
 */
extern void lh_table_free(lh_table *);

/**
 * Insert a record into the table using a precalculated key hash.
 *
 * The hash h, which should be calculated with lh_get_hash() on k, is provided by
 *  the caller, to allow for optimization when multiple operations with the same
 *  key are known to be needed.
 *
 * @param t The table to insert into.
 * @param k a pointer to the key to insert.
 * @param v a pointer to the value to insert.
 * @param h hash value of the key to insert
 * @param opts if set to JSON_C_OBJECT_KEY_IS_CONSTANT, sets lh_entry.k_is_constant
 *             so t's free function knows to avoid freeing the key.
 */
extern int lh_table_insert_w_hash(lh_table *, const void*, const void*,
                                  const unsigned long, const unsigned char);

/**
 * Lookup a record in the table using a precalculated key hash.
 *
 * The hash h, which should be calculated with lh_get_hash() on k, is provided by
 *  the caller, to allow for optimization when multiple operations with the same
 *  key are known to be needed.
 *
 * @param t The table to lookup
 * @param k a pointer to the key to lookup
 * @param h hash value of the key to lookup
 * @return a pointer to the record structure of the value or NULL if it does not exist.
 */
extern lh_entry* lh_table_lookup_entry_w_hash(const lh_table *, const void*, const unsigned long);

/**
 * Lookup a record in the table.
 *
 * @param t The table to lookup
 * @param k a pointer to the key to lookup
 * @param v a pointer to a where to store the found value (set to NULL if it doesn't exist).
 * @return whether or not the key was found
 */
extern unsigned char lh_table_lookup_ex(const lh_table *, const void*, void **);


extern lh_entry* lh_table_getHead(const lh_table *const);

/**
 * Calculate the hash of a key for a given table.
 *
 * This is an extension to support functions that need to calculate
 * the hash several times and allows them to do it just once and then pass
 * in the hash to all utility functions. Depending on use case, this can be a
 * considerable performance improvement.
 * @param t The table (used to obtain hash function)
 * @param k a pointer to the key to lookup
 * @return the key's hash
 */
extern unsigned long lh_get_hash(const lh_table *const, const void* const);

#ifdef __cplusplus
}
#endif

#endif /* __LINKHASH_H__ */
