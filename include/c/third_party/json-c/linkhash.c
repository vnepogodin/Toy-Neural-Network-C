/*
 * $Id: linkhash.c,v 1.4 2006/01/26 02:16:28 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */
#include "linkhash.h"
#include "json_object.h"

#include <stdlib.h> /* arc4random, rand_r */
#include <string.h> /* strlen */
#include <limits.h> /* INT_MAX, LONG_MAX.. */

#ifdef __linux__
# include <fcntl.h> /* openat, O_RDONLY */
# include <unistd.h> /* pread, close */
#elif _WIN32
# include <windows.h>
# include <bcrypt.h> /* BCryptGenRandom */
#endif


struct _LH_Entry {
    /*
     * A flag for users of linkhash to know whether or not they
     * need to free k.
     */
    unsigned char k_is_constant;

    const void *k;
    const void *v;

    /*
     * The next entry
     */
    lh_entry *next;
};

struct _LH_Table {
    /**
     * Size of our hash.
     */
    int size;
    /**
     * Numbers of entries.
     */
    int count;

    /**
     * A pointer onto the function responsible for freeing an entry.
     */
    void (*free_fn)(lh_entry *);
    unsigned long (*hash_fn)(const void*);
    int (*equal_fn)(const void*, const void*);

    /**
     * The first entry.
     */
    lh_entry *head;

    /**
     * The last entry.
     */
    lh_entry *tail;

    lh_entry *table;
};


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h> /* Get InterlockedCompareExchange */
#endif

#ifdef __UNCONST
#define _LH_UNCONST(a) __UNCONST(a)
#else
#define _LH_UNCONST(a) ((void *)(unsigned long)(const void *)(a))
#endif

static lh_table* lh_table_new(const int size,
                              void (*const free_fn)(lh_entry *),
                              unsigned long (*const hash_fn)(const void*),
                              int (*const equal_fn)(const void*, const void*)) {
#ifdef DEBUG
    lh_table *t = NULL;

    /* assumed 0.004MB page sizes */
    posix_memalign((void **)&t, 4096UL, 4096UL);
#else
    lh_table *t = (lh_table *)calloc(1, sizeof(lh_table));
#endif

    if (t == NULL)
        return NULL;

    t->count = 0;
    t->size = size;

#ifdef DEBUG
    t->table = NULL;

    posix_memalign((void **)&t->table, 2048UL, (unsigned long)size);
#else
    t->table = (lh_entry *)calloc((unsigned long)size, sizeof(lh_entry));
#endif

    if (t->table == NULL) {
        free(t);
        return NULL;
    }

    t->free_fn = free_fn;
    t->hash_fn = hash_fn;
    t->equal_fn = equal_fn;

    register int i = 0;
    while (i < size) {
        t->table[i].k = LH_EMPTY;
        ++i;
    }

    return t;
}


inline unsigned char lh_entry_getConstant(lh_entry *__entry_param) {
    return __entry_param->k_is_constant;
}

inline lh_entry* lh_entry_getNext(lh_entry *__entry_param) {
    return __entry_param->next;
}

/**
 * Return a non-const version of lh_entry.k.
 *
 * lh_entry.k is const to indicate and help ensure that linkhash itself doesn't modify
 * it, but callers are allowed to do what they want with it.
 * See also lh_entry.k_is_constant
 */
inline const void* lh_entry_getK(const lh_entry *__entry_param) {
    return _LH_UNCONST(__entry_param->k);
}

/**
 * Return a non-const version of lh_entry.v.
 *
 * v is const to indicate and help ensure that linkhash itself doesn't modify
 * it, but callers are allowed to do what they want with it.
 */
inline const void* lh_entry_getV(const lh_entry *__entry_param) {
    return _LH_UNCONST(__entry_param->v);
}


inline void lh_entry_setV(lh_entry *__entry_param, const void* __value_param) {
    __entry_param->v = __value_param;
}


inline lh_entry* lh_table_getHead(const lh_table *const __table_param) {
    return __table_param->head;
}

inline unsigned long lh_get_hash(const lh_table *__table_param, const void *k) {
    return __table_param->hash_fn(k);
}


static int get_random_seed(void) {
#ifdef __linux__
    register int fd = openat(0, "/dev/urandom", O_RDONLY, 0);
    unsigned char buf[1] = { 0U };

    if (fd != -1) {
        pread(fd, buf, 1, 0);
        close(fd);
    }

    unsigned int __random = buf[0];

    return rand_r(&__random);
#elif _WIN32
    UINT __random = 0U;

    BCryptGenRandom(NULL, (BYTE*)&__random, sizeof(UINT), BCRYPT_USE_SYSTEM_PREFERRED_RNG);

    return (int)__random;
#else
    return (int)arc4random();
#endif
}

/* hash functions */
static unsigned long lh_char_hash(const void *k);

/* comparison functions */
static int lh_char_equal(const void *k1, const void *k2) {
    return strcmp((const char *)k1, (const char *)k2) == 0;
}


static int lh_table_resize(lh_table *t, const int new_size) {
    lh_table *new_t = lh_table_new(new_size, NULL, t->hash_fn, t->equal_fn);
    if (new_t == NULL)
        return -1;

    lh_entry *ent = t->head;
    while (ent != NULL) {
        register unsigned long h = lh_get_hash(new_t, ent->k);
        register unsigned char opts = 0;
        if (ent->k_is_constant)
            opts = JSON_C_OBJECT_KEY_IS_CONSTANT;

        if (lh_table_insert_w_hash(new_t, ent->k, ent->v, h, opts) != 0) {
            lh_table_free(new_t);
            return -1;
        }

        ent = ent->next;
    }
    free(t->table);
    t->table = new_t->table;
    t->size = new_size;
    t->head = new_t->head;
    t->tail = new_t->tail;
    free(new_t);

    return 1;
}


/*
 * hashlittle from lookup3.c, by Bob Jenkins, May 2006, Public Domain.
 * http://burtleburtle.net/bob/c/lookup3.c
 * minor modifications to make functions static so no symbols are exported
 * minor mofifications to compile with -Werror
 */

/*
-------------------------------------------------------------------------------
lookup3.c, by Bob Jenkins, May 2006, Public Domain.

These are functions for producing 32-bit hashes for hash table lookup.
hashword(), hashlittle(), hashlittle2(), hashbig(), mix(), and final()
are externally useful functions.  Routines to test the hash are included
if SELF_TEST is defined.  You can use this free for any purpose.  It's in
the public domain.  It has no warranty.

You probably want to use hashlittle().  hashlittle() and hashbig()
hash byte arrays.  hashlittle() is is faster than hashbig() on
little-endian machines.  Intel and AMD are little-endian machines.
On second thought, you probably want hashlittle2(), which is identical to
hashlittle() except it returns two 32-bit hashes for the price of one.
You could implement hashbig2() if you wanted but I haven't bothered here.

If you want to find a hash of, say, exactly 7 integers, do
  a = i1;  b = i2;  c = i3;
  mix(a,b,c);
  a += i4; b += i5; c += i6;
  mix(a,b,c);
  a += i7;
  final(a,b,c);
then use c as the hash value.  If you have a variable length array of
4-byte integers to hash, use hashword().  If you have a byte array (like
a character string), use hashlittle().  If you have several byte arrays, or
a mix of things, see the comments above hashlittle().

Why is this so big?  I read 12 bytes at a time into 3 4-byte integers,
then mix those integers.  This is fast (you can do a lot more thorough
mixing with 12*3 instructions on 3 integers than you can with 3 instructions
on 1 byte), but shoehorning those bytes into integers efficiently is messy.
-------------------------------------------------------------------------------
*/

/*
 * My best guess at if you are big-endian or little-endian.  This may
 * need adjustment.
 */
#if (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && __BYTE_ORDER == __LITTLE_ENDIAN) || \
    (defined(i386) || defined(__i386__) || defined(__i486__) || defined(__i586__) ||          \
     defined(__i686__) || defined(vax) || defined(MIPSEL))
#define HASH_LITTLE_ENDIAN 1
#define HASH_BIG_ENDIAN 0
#elif (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && __BYTE_ORDER == __BIG_ENDIAN) || \
    (defined(sparc) || defined(POWERPC) || defined(mc68000) || defined(sel))
#define HASH_LITTLE_ENDIAN 0
#define HASH_BIG_ENDIAN 1
#else
#define HASH_LITTLE_ENDIAN 0
#define HASH_BIG_ENDIAN 0
#endif

#define rot(x, k) (((x) << (k)) | ((x) >> (32U - (k))))

/*
-------------------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly.

This is reversible, so any information in (a,b,c) before mix() is
still in (a,b,c) after mix().

If four pairs of (a,b,c) inputs are run through mix(), or through
mix() in reverse, there are at least 32 bits of the output that
are sometimes the same for one pair and different for another pair.
This was tested for:
* pairs that differed by one bit, by two bits, in any combination
  of top bits of (a,b,c), or in any combination of bottom bits of
  (a,b,c).
* "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed
  the output delta to a Gray code (a^(a>>1)) so a string of 1's (as
  is commonly produced by subtraction) look like a single 1-bit
  difference.
* the base values were pseudorandom, all zero but one bit set, or
  all zero plus a counter that starts at zero.

Some k values for my "a-=c; a^=rot(c,k); c+=b;" arrangement that
satisfy this are
    4  6  8 16 19  4
    9 15  3 18 27 15
   14  9  3  7 17  3
Well, "9 15 3 18 27 15" didn't quite get 32 bits diffing
for "differ" defined as + with a one-bit base and a two-bit delta.  I
used http://burtleburtle.net/bob/hash/avalanche.html to choose
the operations, constants, and arrangements of the variables.

This does not achieve avalanche.  There are input bits of (a,b,c)
that fail to affect some output bits of (a,b,c), especially of a.  The
most thoroughly mixed value is c, but it doesn't really even achieve
avalanche in c.

This allows some parallelism.  Read-after-writes are good at doubling
the number of bits affected, so the goal of mixing pulls in the opposite
direction as the goal of parallelism.  I did what I could.  Rotates
seem to cost as much as shifts on every machine I could lay my hands
on, and rotates are much kinder to the top and bottom bits, so I used
rotates.
-------------------------------------------------------------------------------
*/
/* clang-format off */
#define mix(a, b, c) do{                \
    a -= c;  a ^= rot(c, 4U);  c += b;  \
    b -= a;  b ^= rot(a, 6U);  a += c;  \
    c -= b;  c ^= rot(b, 8U);  b += a;  \
    a -= c;  a ^= rot(c, 16U); c += b;  \
    b -= a;  b ^= rot(a, 19U); a += c;  \
    c -= b;  c ^= rot(b, 4U);  b += a;  \
}while(0)
/* clang-format on */

/*
-------------------------------------------------------------------------------
final -- final mixing of 3 32-bit values (a,b,c) into c

Pairs of (a,b,c) values differing in only a few bits will usually
produce values of c that look totally different.  This was tested for
* pairs that differed by one bit, by two bits, in any combination
  of top bits of (a,b,c), or in any combination of bottom bits of
  (a,b,c).
* "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed
  the output delta to a Gray code (a^(a>>1)) so a string of 1's (as
  is commonly produced by subtraction) look like a single 1-bit
  difference.
* the base values were pseudorandom, all zero but one bit set, or
  all zero plus a counter that starts at zero.

These constants passed:
 14 11 25 16 4 14 24
 12 14 25 16 4 14 24
and these came close:
  4  8 15 26 3 22 24
 10  8 15 26 3 22 24
 11  8 15 26 3 22 24
-------------------------------------------------------------------------------
*/
/* clang-format off */
#define final(a, b, c) {      \
    c ^= b; c -= rot(b, 14U); \
    a ^= c; a -= rot(c, 11U); \
    b ^= a; b -= rot(a, 25U); \
    c ^= b; c -= rot(b, 16U); \
    a ^= c; a -= rot(c, 4U);  \
    b ^= a; b -= rot(a, 14U); \
    c ^= b; c -= rot(b, 24U); \
}
/* clang-format on */

/*
-------------------------------------------------------------------------------
hashlittle() -- hash a variable-length key into a 32-bit value
  k       : the key (the unaligned variable-length array of bytes)
  length  : the length of the key, counting by bytes
  initval : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Two keys differing by one or two bits will have
totally different hash values.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (unsigned char **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hashlittle( k[i], len[i], h);

By Bob Jenkins, 2006.  bob_jenkins@burtleburtle.net.  You may use this
code any way you wish, private, educational, or commercial.  It's free.

Use for hash table lookup, or anything where one collision in 2^^32 is
acceptable.  Do NOT use for cryptographic purposes.
-------------------------------------------------------------------------------
*/

/* clang-format off */
static unsigned hashlittle(const void *key, unsigned long length, unsigned initval) {
    /* internal state */
    unsigned a;
    unsigned b;
    unsigned c;
    union {
        const void *ptr;
        unsigned long i;
    } u; /* needed for Mac Powerbook G4 */

    /* Set up the internal state */
    a = b = c = 0xdeadbeef + ((unsigned)length) + initval;

    u.ptr = key;
    if (HASH_LITTLE_ENDIAN && ((u.i & 0x3) == 0)) {
        const unsigned *k = (const unsigned *)key; /* read 32-bit chunks */

        /*------ all but last block: aligned reads and affect 32 bits of (a, b, c) */
        while (length > 12) {
            a += k[0];
            b += k[1];
            c += k[2];

            mix(a, b, c);

            length -= 12;
            k += 3;
        }
    } else if (HASH_LITTLE_ENDIAN && ((u.i & 0x1) == 0)) {
        const unsigned short *k = (const unsigned short *)key; /* read 16-bit chunks */

        /*--------------- all but last block: aligned reads and different mixing */
        while (length > 12) {
            a += k[0] + ((unsigned)k[1] << 16U);
            b += k[2] + ((unsigned)k[3] << 16U);
            c += k[4] + ((unsigned)k[5] << 16U);

            mix(a, b, c);

            length -= 12;
            k += 6;
        }

        /*----------------------------- handle the last (probably partial) block */
        const unsigned char *k8 = (const unsigned char *)k;
        switch(length) {
        case 12:
            c += k[4] + ((unsigned)k[5] << 16U);
            b += k[2] + ((unsigned)k[3] << 16U);
            a += k[0] + ((unsigned)k[1] << 16U);
            break;
        case 11:
            c += (unsigned)k8[10] << 16U;     /* fall through */
        case 10:
            c += k[4];
            b += k[2] + ((unsigned)k[3] << 16U);
            a += k[0] + ((unsigned)k[1] << 16U);
            break;
        case 9:
            c += k8[8];                      /* fall through */
        case 8:
            b += k[2] + ((unsigned)k[3] << 16U);
            a += k[0] + ((unsigned)k[1] << 16U);
            break;
        case 7:
            b += (unsigned)k8[6] << 16U;      /* fall through */
        case 6:
            b += k[2];
            a += k[0] + ((unsigned)k[1] << 16U);
            break;
        case 5:
            b += k8[4];                      /* fall through */
        case 4:
            a += k[0] + ((unsigned)k[1] << 16U);
            break;
        case 3:
            a += ((unsigned)k8[2]) << 16U;      /* fall through */
        case 2:
            a += k[0];
            break;
        case 1:
            a += k8[0];
            break;
        case 0:
            return c;                     /* zero length requires no mixing */
        }
    } else {
        /* need to read the key one byte at a time */
        const unsigned char *k = (const unsigned char *)key;

        /*--------------- all but the last block: affect some 32 bits of (a,b,c) */
        while (length > 12) {
            a += k[0];
            a += (unsigned)k[1] << 8U;
            a += (unsigned)k[2] << 16U;
            a += (unsigned)k[3] << 24U;
            b += k[4];
            b += (unsigned)k[5] << 8U;
            b += (unsigned)k[6] << 16U;
            b += (unsigned)k[7] << 24U;
            c += k[8];
            c += (unsigned)k[9] << 8U;
            c += (unsigned)k[10] << 16U;
            c += (unsigned)k[11] << 24U;

            mix(a, b, c);

            length -= 12;
            k += 12;
        }

        /*-------------------------------- last block: affect all 32 bits of (c) */
        switch(length) { /* all the case statements fall through */
        case 12:
            c += (unsigned)k[11] << 24U; /* FALLTHRU */
        case 11:
            c += (unsigned)k[10] << 16U; /* FALLTHRU */
        case 10:
            c += (unsigned)k[9] << 8U; /* FALLTHRU */
        case 9:
            c += k[8]; /* FALLTHRU */
        case 8:
            b += (unsigned)k[7] << 24U; /* FALLTHRU */
        case 7:
            b += (unsigned)k[6] << 16U; /* FALLTHRU */
        case 6:
            b += (unsigned)k[5] << 8U; /* FALLTHRU */
        case 5:
            b += k[4]; /* FALLTHRU */
        case 4:
            a += (unsigned)k[3] << 24U; /* FALLTHRU */
        case 3:
            a += (unsigned)k[2] << 16U; /* FALLTHRU */
        case 2:
            a += (unsigned)k[1] << 8U; /* FALLTHRU */
        case 1:
            a += k[0];
            break;
        case 0:
            return c;
        }
    }

    final(a, b, c)
    return c;
}
/* clang-format on */

static unsigned long lh_char_hash(const void *k) {
#ifdef _WIN32
#define RANDOM_SEED_TYPE LONG
#else
#define RANDOM_SEED_TYPE int
#endif
    static volatile RANDOM_SEED_TYPE random_seed = -1;

    if (random_seed == -1) {
        register const int seed = get_random_seed();

#ifdef _WIN32
        InterlockedCompareExchange(&random_seed, seed, -1);
#else
        (void)__sync_val_compare_and_swap(&random_seed, -1, seed);
#endif
    }

    return hashlittle((const char *)k, strlen((const char *)k), (unsigned)random_seed);
}

lh_table* lh_kchar_table_new(const int size, void(*const free_fn)(lh_entry *)) {
    return lh_table_new(size, free_fn, lh_char_hash, lh_char_equal);
}

void lh_table_free(lh_table *t) {
    if (t->free_fn != NULL) {
        register lh_entry *c = t->head;
        while (c != NULL) {
            t->free_fn(c);

            c = c->next;
        }
    }
    free(t->table);
    free(t);
}

int lh_table_insert_w_hash(lh_table *t, const void *k, const void *v, const unsigned long h, const unsigned char opts) {
    if (t->count >= t->size * LH_LOAD_FACTOR) {
        /* Avoid signed integer overflow with large tables. */
        register const int new_size = (t->size > (INT_MAX / 2)) ? INT_MAX : t->size * 2;
        if ((t->size == INT_MAX) || (!lh_table_resize(t, new_size)))
            return -1;
    }

    register unsigned long n = h % (unsigned long)t->size;

    while (1) {
        if ((t->table[n].k == LH_EMPTY) || (t->table[n].k == LH_FREED))
            break;

        ++n;
        if ((int)n == t->size)
            n = 0;
    }

    t->table[n].k = k;
    t->table[n].k_is_constant = opts & JSON_C_OBJECT_KEY_IS_CONSTANT;
    t->table[n].v = v;
    t->count++;

    if (t->head == NULL) {
        t->head = t->tail = &t->table[n];
        t->table[n].next = NULL;
    } else {
        t->tail->next = &t->table[n];
        t->table[n].next = NULL;
        t->tail = &t->table[n];
    }

    return 0;
}


lh_entry* lh_table_lookup_entry_w_hash(const lh_table *t, const void *k, const unsigned long h) {
    register unsigned long n = h % (unsigned long)t->size;

    register int i = 0;
    while (i < t->size) {
        if (t->table[n].k == LH_EMPTY)
            return NULL;

        if ((t->table[n].k != LH_FREED) && (t->equal_fn(t->table[n].k, k)))
            return &t->table[n];

        ++n;
        if ((int)n == t->size)
            n = 0;

        ++i;
    }
    return NULL;
}

unsigned int lh_table_lookup_ex(const lh_table *t, const void *k, void **v) {
    register const lh_entry *e = lh_table_lookup_entry_w_hash(t, k, lh_get_hash(t, k));
    register unsigned int result = 0U;
    if (e != NULL) {
        if (v != NULL)
            *v = (void *)lh_entry_getV(e);

        result = 1U; /* key found */
    }

    return result; /* key not found */
}
