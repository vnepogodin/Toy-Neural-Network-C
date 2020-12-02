/*
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */
#include "json_object.h"
#include "arraylist.h"
#include "linkhash.h"
#include "printbuf.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h> /* LLONG_MAX */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* supported object types */
enum json_type {
    json_type_null,
    json_type_double,
    json_type_int,
    json_type_object,
    json_type_array,
};

enum json_object_int_type {
    json_object_int64,
    json_object_uint64
};

/**
 * A structure to use with json_object_object_foreachC() loops.
 * Contains key, val and entry members.
 */
struct json_object_iter {
    char* key;
    json_object* val;
    lh_entry* entry;
};

struct _Json_Object {
    enum json_type o_type;

    unsigned _ref_count;

    void* _userdata;

    int (*_to_json_string)(json_object*, printbuf*, const int, const int);
    void (*_user_delete)(json_object*, void*);

    printbuf* _pb;
    /* Actually longer, always malloc'd as some more-specific type.
    * The rest of a struct json_object_${o_type} follows
    */
};

struct json_object_object {
    json_object base;

    lh_table* c_object;
};
struct json_object_array {
    json_object base;

    array_list* c_array;
};
struct json_object_double {
    json_object base;

    double c_double;
};
struct json_object_int {
    json_object base;

    enum json_object_int_type cint_type;
    union {
        long long c_int64;
        unsigned long long c_uint64;
    } cint;
};

#ifdef _WIN32
# define strdup _strdup
# define UNUSED
#else
# define UNUSED __attribute__((unused))
#endif

#define JC_CONCAT(a, b) a##b
#define JC_CONCAT3(a, b, c) a##b##c

#define JSON_OBJECT_NEW(jtype)                               \
    (struct JC_CONCAT(json_object_, jtype)*)json_object_new( \
        JC_CONCAT(json_type_, jtype),                        \
        sizeof(struct JC_CONCAT(json_object_, jtype)),       \
        (int (*)(json_object*, printbuf*, const int, const int)) & JC_CONCAT3(json_object_, jtype, _to_json_string))

/** Iterate through all keys and values of an object (ANSI C Safe)
 * @param obj the json_object instance
 * @param iter the object iterator, use type json_object_iter
 */
#define json_object_object_foreachC(obj, iter)                          \
    for (iter.entry = json_object_get_object(obj)->head;                \
         (iter.entry ? (iter.key = (char*)lh_entry_k(iter.entry),       \
                       iter.val = (json_object*)lh_entry_v(iter.entry), \
                       iter.entry)                                      \
                     : 0);                                              \
         iter.entry = iter.entry->next)

/* generic object construction and destruction parts */
static inline void json_object_generic_delete(json_object* jso) {
    printbuf_free(jso->_pb);
    free(jso);
}

/* string escaping */
static int json_escape_str(printbuf* pb, const char* str, unsigned long len, const int flags) {
    static const char json_hex_chars[22] = "0123456789abcdefABCDEF";

    register unsigned long pos = 0UL;
    register unsigned long start_offset = 0UL;

    while (len--) {
        register unsigned char c = (unsigned char)str[pos];
        register unsigned long buf_l = pos - start_offset;

        switch (c) {
            case '\b':
            case '\n':
            case '\r':
            case '\t':
            case '\f':
            case '"':
            case '\\':
            case '/':
                if (((unsigned)flags & JSON_C_TO_STRING_NOSLASHESCAPE) && (c == '/')) {
                    ++pos;
                    break;
                }

                if (buf_l > 0)
                    printbuf_memappend(pb, str + start_offset, buf_l);

                if (c == '\b')
                    printbuf_memappend(pb, "\\b", 2);
                else if (c == '\n')
                    printbuf_memappend(pb, "\\n", 2);
                else if (c == '\r')
                    printbuf_memappend(pb, "\\r", 2);
                else if (c == '\t')
                    printbuf_memappend(pb, "\\t", 2);
                else if (c == '\f')
                    printbuf_memappend(pb, "\\f", 2);
                else if (c == '"')
                    printbuf_memappend(pb, "\\\"", 2);
                else if (c == '\\')
                    printbuf_memappend(pb, "\\\\", 2);

                ++pos;
                start_offset = pos;
                break;
            default:
                if (c < ' ') {
                    if (buf_l > 0)
                        printbuf_memappend(pb, str + start_offset, buf_l);

                    char sbuf[7] = {0};
                    snprintf(sbuf, 7, "\\u00%c%c", json_hex_chars[c >> 4U], json_hex_chars[c & (unsigned)0xf]);
                    printbuf_memappend_fast(pb, sbuf, 6);

                    ++pos;
                    start_offset = pos;
                } else
                    pos++;
        }
    }
    if ((pos - start_offset) > 0)
        printbuf_memappend(pb, str + start_offset, pos - start_offset);

    return 0;
}

static inline void indent(printbuf* pb, const int level, const int flags) {
    if ((unsigned)flags & JSON_C_TO_STRING_PRETTY) {
        if ((unsigned)flags & JSON_C_TO_STRING_PRETTY_TAB)
            printbuf_memset(pb, -1, '\t', level);
        else
            printbuf_memset(pb, -1, ' ', level * 2);
    }
}

/* SETTING DATA */
static inline void json_object_set_userdata(json_object* jso, void* userdata, void (*user_delete)(json_object*, void*)) {
    /* First, clean up any previously existing user info */
    if (jso->_user_delete != NULL)
        jso->_user_delete(jso, jso->_userdata);

    jso->_userdata = userdata;
    jso->_user_delete = user_delete;
}

static inline void json_object_set_serializer(json_object* jso, int (*to_string_func)(json_object*, printbuf*, int, int), void* userdata, void (*user_delete)(json_object*, void*)) {
    json_object_set_userdata(jso, userdata, user_delete);

    jso->_to_json_string = to_string_func;
}

/*
 * Helper functions to more safely cast to a particular type of json_object
 */
static inline struct json_object_object* JC_OBJECT(json_object* jso) {
    return (void*)jso;
}
static inline const struct json_object_object* JC_OBJECT_C(const json_object* jso) {
    return (const void*)jso;
}
static inline struct json_object_array* JC_ARRAY(json_object* jso) {
    return (void*)jso;
}
static inline const struct json_object_array* JC_ARRAY_C(const json_object* jso) {
    return (const void*)jso;
}
static inline struct json_object_double* JC_DOUBLE(json_object* jso) {
    return (void*)jso;
}
static inline const struct json_object_double* JC_DOUBLE_C(const json_object* jso) {
    return (const void*)jso;
}
static inline struct json_object_int* JC_INT(json_object* jso) {
    return (void*)jso;
}
static inline const struct json_object_int* JC_INT_C(const json_object* jso) {
    return (const void*)jso;
}

/** Get the hashtable of a json_object of type json_type_object
 * @param obj The json_object instance
 * @returns a linkhash
 */
static inline lh_table* json_object_get_object(const json_object* jso) {
    register lh_table* result = NULL;
    if (jso != NULL) {
        result = JC_OBJECT_C(jso)->c_object;
    }

    return result;
}

/* type checking function */
//UNUSED static inline enum json_type json_object_get_type(const json_object *jso) {
//    if (jso == NULL)
//        return json_type_null;

//    return jso->o_type;
//}

/** Get the length of a json_object of type json_type_array
 * @param obj the json_object instance
 * @returns an int
 */
static unsigned long json_object_array_length(const json_object* jso) {
    return array_list_length(JC_ARRAY_C(jso)->c_array);
}

static inline json_object* json_object_new(enum json_type o_type, const unsigned long alloc_size, int (*to_json_string)(json_object*, printbuf*, const int, const int)) {
    json_object* result = NULL;

    register json_object* jso = (json_object*)malloc(alloc_size);

    if (jso != NULL) {
        jso->o_type = o_type;
        jso->_ref_count = 1;
        jso->_to_json_string = to_json_string;
        jso->_pb = NULL;
        jso->_user_delete = NULL;
        jso->_userdata = NULL;
        /* @code jso->...
         * Type-specific fields must be set by caller
         */
        result = jso;
    }

    return result;
}

static inline void json_object_object_delete(json_object* jso_base) {
    lh_table_free(JC_OBJECT(jso_base)->c_object);
    json_object_generic_delete(jso_base);
}

static inline void json_object_array_delete(json_object* jso) {
    array_list_free(JC_ARRAY(jso)->c_array);
    json_object_generic_delete(jso);
}

static int json_object_double_to_json_string_format(json_object* jso,
                                                    printbuf* pb,
                                                    UNUSED const int level,
                                                    const int flags) {
    struct json_object_double* jsodbl = JC_DOUBLE(jso);
    char buf[128];
    register int size = 0;
    /* Although JSON RFC does not support
     * NaN or Infinity as numeric values
     * ECMA 262 section 9.8.1 defines
     * how to handle these cases as strings
     */
    if (isnan(jsodbl->c_double)) {
        size = snprintf(buf, 128, "NaN");
    } else if (isinf(jsodbl->c_double)) {
        if (jsodbl->c_double > 0)
            size = snprintf(buf, 128, "Infinity");
        else
            size = snprintf(buf, 128, "-Infinity");
    } else {
        size = snprintf(buf, 128UL, "%.17g", jsodbl->c_double);

        if (size < 0)
            return -1;

        char* p = strchr(buf, ',');
        if (p != NULL)
            *p = '.';
        else
            p = strchr(buf, '.');

        /* Looks like *some* kind of number */
        register unsigned char looks_numeric = isdigit((int)buf[0]) || ((size > 1) && (buf[0] == '-') && (isdigit((int)buf[1])));

        /* Ensure it looks like a float, even if snprintf didn't,
         * unless a custom format is set to omit the decimal.
         */
        if ((size < 126) && looks_numeric && (p == NULL) &&
            (strchr(buf, 'e') == NULL)) {
#ifdef _WIN32
            strcat_s
#else
            strncat
#endif
            (buf, ".0", 3UL);
            size += 2;
        }

        if ((p != NULL) && ((unsigned)flags & JSON_C_TO_STRING_NOZERO)) {
            /* last useful digit, always keep 1 zero */
            p++;
            char* q = p;
            while (*q != '\0') {
                if (*q != '0')
                    p = q;
                ++q;
            }
            /* drop trailing zeroes */
            if (*p != 0) {
                ++p;
                *p = 0;
            }

            size = (int)(p - buf);
        }
    }
    /* although unlikely, snprintf can fail */
    if (size < 0)
        return -1;

    /* The standard formats are guaranteed not to overrun the buffer,
     * but if a custom one happens to do so, just silently truncate.
     */
    if (size >= 128)
        size = 127;

    printbuf_memappend(pb, buf, (unsigned long)size);
    return size;
}

static inline int json_object_double_to_json_string(json_object* jso, printbuf* pb, UNUSED const int level, const int flags) {
    return json_object_double_to_json_string_format(jso, pb, 0, flags);
}

static int json_object_object_to_json_string(json_object* jso, printbuf* pb, const int level, const int flags) {
    printbuf_strappend(pb, "{" /*}*/);
    if ((unsigned)flags & JSON_C_TO_STRING_PRETTY)
        printbuf_strappend(pb, "\n");

    register unsigned char had_children = 0U;
    struct json_object_iter iter;

    json_object_object_foreachC(jso, iter) {
        if (had_children) {
            printbuf_strappend(pb, ",");
            if ((unsigned)flags & JSON_C_TO_STRING_PRETTY)
                printbuf_strappend(pb, "\n");
        }

        had_children = 1U;
        if ((unsigned)flags & JSON_C_TO_STRING_SPACED && !((unsigned)flags & JSON_C_TO_STRING_PRETTY))
            printbuf_strappend(pb, " ");

        indent(pb, level + 1, flags);
        printbuf_strappend(pb, "\"");
        json_escape_str(pb, iter.key, strlen(iter.key), flags);

        if ((unsigned)flags & JSON_C_TO_STRING_SPACED)
            printbuf_strappend(pb, "\": ");
        else
            printbuf_strappend(pb, "\":");

        if (iter.val == NULL)
            printbuf_strappend(pb, "null");
        else if (iter.val->_to_json_string(iter.val, pb, level + 1, flags) < 0)
            return -1;
    }
    if ((unsigned)flags & JSON_C_TO_STRING_PRETTY) {
        if (had_children)
            printbuf_strappend(pb, "\n");

        indent(pb, level, flags);
    }
    if ((unsigned)flags & JSON_C_TO_STRING_SPACED && !((unsigned)flags & JSON_C_TO_STRING_PRETTY))
        return printbuf_strappend(pb, /*{*/ " }");
    else
        return printbuf_strappend(pb, /*{*/ "}");
}

static inline int json_object_int_to_json_string(json_object* jso,
                                                 printbuf* pb,
                                                 UNUSED const int level,
                                                 UNUSED const int flags) {
    /* room for 19 digits, the sign char, and a null term */
    char sbuf[21];
    if (JC_INT(jso)->cint_type == json_object_int64)
        snprintf(sbuf, 21, "%lld", JC_INT(jso)->cint.c_int64);
    else
        snprintf(sbuf, 21, "%llu", JC_INT(jso)->cint.c_uint64);

    return printbuf_memappend(pb, sbuf, strlen(sbuf));
}

static int json_object_array_to_json_string(json_object* jso,
                                            printbuf* pb,
                                            const int level,
                                            const int flags) {
    printbuf_strappend(pb, "[");
    if ((unsigned)flags & JSON_C_TO_STRING_PRETTY)
        printbuf_strappend(pb, "\n");

    register unsigned char had_children = 0U;
    register unsigned long ii = 0UL;
    while (ii < json_object_array_length(jso)) {
        if (had_children) {
            printbuf_strappend(pb, ",");
            if ((unsigned)flags & JSON_C_TO_STRING_PRETTY)
                printbuf_strappend(pb, "\n");
        }
        had_children = 1U;
        if ((unsigned)flags & JSON_C_TO_STRING_SPACED && !((unsigned)flags & JSON_C_TO_STRING_PRETTY))
            printbuf_strappend(pb, " ");

        indent(pb, level + 1, flags);
        register json_object* val = json_object_array_get_idx(jso, ii);

        if (val == NULL)
            printbuf_strappend(pb, "null");
        else if (val->_to_json_string(val, pb, level + 1, flags) < 0)
            return -1;

        ++ii;
    }

    if ((unsigned)flags & JSON_C_TO_STRING_PRETTY) {
        if (had_children)
            printbuf_strappend(pb, "\n");

        indent(pb, level, flags);
    }

    if ((unsigned)flags & JSON_C_TO_STRING_SPACED && !((unsigned)flags & JSON_C_TO_STRING_PRETTY))
        return printbuf_strappend(pb, " ]");

    return printbuf_strappend(pb, "]");
}

/*
 * Copy the jso->_userdata string over to pb as-is.
 *
 * A wrapper around json_object_userdata_to_json_string() used only
 * by json_object_new_double_s() just so json_object_set_double() can
 * detect when it needs to reset the serializer to the default.
 *
 * @param jso The object whose _userdata is used.
 * @param pb The destination buffer.
 */
static inline int json_object_userdata_to_json_string(json_object* jso,
                                                      printbuf* pb,
                                                      UNUSED const int level,
                                                      UNUSED const int flags) {
    register int userdata_len = (int)strlen((const char*)jso->_userdata);
    printbuf_memappend(pb, (const char*)jso->_userdata, (unsigned long)userdata_len);

    return userdata_len;
}

/* FREEZING */
static inline void json_object_free_userdata(UNUSED json_object* jso,
                                             void* data) {
    free(data);
}

static inline void json_object_array_entry_free(void* data) {
    json_object_put((json_object*)data);
}

static inline void json_object_lh_entry_free(lh_entry* ent) {
    if (!ent->k_is_constant)
        free((void*)lh_entry_k(ent));
    json_object_put((json_object*)lh_entry_v(ent));
}

/* reference counting */
inline json_object* json_object_get(json_object* jso) {
    if (jso == NULL)
        return NULL;

    /* Don't overflow the reference counter. */
    assert(jso->_ref_count < UINT_MAX);

#ifdef _WIN32
    ++jso->_ref_count;
#else
    __sync_add_and_fetch(&jso->_ref_count, 1);
#endif

    return jso;
}

int json_object_put(json_object* jso) {
    if (jso == NULL)
        return 0;

    /* Avoid invalid free and crash explicitly instead of (silently)
     * seg_faulting.
     */
    assert(jso->_ref_count > 0);

    /* Note: this only allow the refcount to remain correct
     * when multiple threads are adjusting it.  It is still an error
     * for a thread to decrement the refcount if it doesn't "own" it,
     * as that can result in the thread that loses the race to 0
     * operating on an already-freed object.
     */
#ifdef _WIN32
    if (--jso->_ref_count > 0)
        return 0;
#else
    if (__sync_sub_and_fetch(&jso->_ref_count, 1) > 0)
        return 0;
#endif

    if (jso->_user_delete != NULL)
        jso->_user_delete(jso, jso->_userdata);

    switch (jso->o_type) {
        case json_type_object:
            json_object_object_delete(jso);
            break;
        case json_type_array:
            json_object_array_delete(jso);
            break;
        default:
            json_object_generic_delete(jso);
            break;
    }
    return 1;
}

/* extended conversion to string */
inline const char* json_object_to_json_string_ext(json_object* jso,
                                                  const int flags) {
    const char* r = "null";

    if ((jso->_pb != NULL) || (jso->_pb = printbuf_new())) {
        printbuf_reset(jso->_pb);

        if (jso->_to_json_string(jso, jso->_pb, 0, flags) >= 0) {
            r = printbuf_getBuf(jso->_pb);
        }
    }

    return r;
}

/* json_object_object */
inline json_object* json_object_new_object(void) {
    json_object* result = NULL;
    struct json_object_object* jso = JSON_OBJECT_NEW(object);

    if (jso != NULL) {
        jso->c_object = lh_kchar_table_new(JSON_OBJECT_DEF_HASH_ENTRIES, &json_object_lh_entry_free);

        if (jso->c_object != NULL)
            result = &jso->base;
        else
            json_object_generic_delete(result);
    }
    return result;
}

int json_object_object_add_ex(json_object* jso, const char* const key, json_object* const val, const unsigned char opts) {
    assert(jso->o_type == json_type_object);

    /* We lookup the entry and replace the value, rather than just deleting
     * and re-adding it, so the existing key remains valid.
     */
    register const unsigned long hash = lh_get_hash(JC_OBJECT(jso)->c_object, (const void*)key);
    lh_entry* existing_entry = (opts & JSON_C_OBJECT_ADD_KEY_IS_NEW)
                                   ? NULL
                                   : lh_table_lookup_entry_w_hash(JC_OBJECT(jso)->c_object, (const void*)key, hash);

    /* The caller must avoid creating loops in the object tree, but do a
     * quick check anyway to make sure we're not creating a trivial loop.
     */
    if (jso == val)
        return -1;

    if (existing_entry == NULL) {
        const void* const k = (opts & JSON_C_OBJECT_KEY_IS_CONSTANT) ? (const void*)key : strdup(key);
        if (k == NULL)
            return -1;

        return lh_table_insert_w_hash(JC_OBJECT(jso)->c_object, k, val, hash, opts);
    }

    json_object* existing_value = (json_object*)lh_entry_v(existing_entry);
    if (existing_value != NULL)
        json_object_put(existing_value);

    existing_entry->v = val;

    return 0;
}

inline unsigned char json_object_object_get_ex(const json_object* jso, const char* key, json_object** value) {
    if (value != NULL)
        *value = NULL;

    if (NULL == jso)
        return 0;

    assert(jso->o_type == json_type_object);

    return lh_table_lookup_ex(JC_OBJECT_C(jso)->c_object, (const void*)key, (void**)value);
}

/* json_object_int */
inline json_object* json_object_new_int(const int i) {
    return json_object_new_int64(i);
}

int json_object_get_int(const json_object* jso) {
    register int result = 0;
    if (jso != NULL) {
        switch (jso->o_type) {
            case json_type_int: {
                register const struct json_object_int* json_int = JC_INT_C(jso);
                register long long cint64 = 0LL;

                if (json_int->cint_type == json_object_int64) {
                    cint64 = json_int->cint.c_int64;
                } else {
                    if (json_int->cint.c_uint64 >= ULLONG_MAX)
                        cint64 = LLONG_MAX;
                    else
                        cint64 = (long long)json_int->cint.c_uint64;
                }
                /* Make sure we return the correct values for out of range numbers. */
                result = (int)cint64;

                if (cint64 <= INT_MIN)
                    result = INT_MIN;

                if (cint64 >= INT_MAX)
                    result = INT_MAX;
                break;
            }
            case json_type_double: {
                register const double cdouble = JC_DOUBLE_C(jso)->c_double;
                result = (int)cdouble;

                if (cdouble <= INT_MIN)
                    result = INT_MIN;

                if (cdouble >= INT_MAX)
                    result = INT_MAX;
                break;
            }
            default:
                break;
        }
    }

    return result;
}

inline json_object* json_object_new_int64(const long long i) {
    json_object* result = NULL;

    struct json_object_int* jso = JSON_OBJECT_NEW(int);
    if (jso != NULL) {
        jso->cint.c_int64 = i;
        jso->cint_type = json_object_int64;

        result = &jso->base;
    }

    return result;
}

inline json_object* json_object_new_uint64(const unsigned long long i) {
    json_object* result = NULL;

    struct json_object_int* jso = JSON_OBJECT_NEW(int);
    if (jso != NULL) {
        jso->cint.c_uint64 = i;
        jso->cint_type = json_object_uint64;

        result = &jso->base;
    }

    return result;
}

/* json_object_double */
inline json_object* json_object_new_double(const double d) {
    json_object* result = NULL;

    struct json_object_double* jso = JSON_OBJECT_NEW(double);
    if (jso != NULL) {
        jso->c_double = d;

        result = &jso->base;
    }

    return result;
}

inline json_object* json_object_new_double_s(const double d, const char* ds) {
    register json_object* result = NULL;
    json_object* jso = json_object_new_double(d);

    if (jso != NULL) {
        register const char* new_ds = strdup(ds);

        if (new_ds != NULL) {
            json_object_set_serializer(jso, (int (*)(json_object*, printbuf*, int, int))json_object_userdata_to_json_string, (void*)new_ds, (void (*)(json_object*, void*))json_object_free_userdata);
            result = jso;
        } else
            json_object_generic_delete(jso);
    }

    return result;
}

float json_object_get_float(const json_object* jso) {
    register float result = 0.F;

    if (jso != NULL) {
        assert((jso->o_type == json_type_double) || (jso->o_type == json_type_int));

        switch (jso->o_type) {
            case json_type_double:
                result = (float)JC_DOUBLE_C(jso)->c_double;
                break;
            case json_type_int:
                switch (JC_INT_C(jso)->cint_type) {
                    case json_object_int64:
                        result = (float)JC_INT_C(jso)->cint.c_int64;
                        break;
                    case json_object_uint64:
                        result = (float)JC_INT_C(jso)->cint.c_uint64;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    return result;
}

/* json_object_array */
inline json_object* json_object_new_array_ext(const int initial_size) {
    json_object* result = NULL;
    register struct json_object_array* jso = JSON_OBJECT_NEW(array);

    if (jso != NULL) {
        jso->c_array = array_list_new(&json_object_array_entry_free, initial_size);

        if (jso->c_array != NULL)
            result = &jso->base;
        else
            free(jso);
    }

    return result;
}

inline int json_object_array_add(json_object* jso, json_object* val) {
    assert(jso->o_type == json_type_array);

    return array_list_add(JC_ARRAY(jso)->c_array, val);
}

inline json_object* json_object_array_get_idx(const json_object* jso, unsigned long idx) {
    assert(jso->o_type == json_type_array);

    return (json_object*)array_list_get_idx(JC_ARRAY_C(jso)->c_array, idx);
}

inline int json_object_array_shrink(json_object* jso, const int empty_slots) {
    assert(empty_slots >= 0);

    return array_list_shrink(JC_ARRAY(jso)->c_array, (unsigned long)empty_slots);
}
