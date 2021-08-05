/*
 * $Id: json_tokener.c,v 1.20 2006/07/25 03:24:50 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 *
 * Copyright (c) 2008-2009 Yahoo! Inc.  All rights reserved.
 * The copyrights to the contents of this file are licensed under the MIT License
 * (http://www.opensource.org/licenses/mit-license.php)
 */
// #include <vnepogodin/third_party/json-c/json_tokener.h>
/*
 * $Id: json_tokener.h,v 1.10 2006/07/25 03:24:50 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */
#ifndef __JSON_TOKENER_H__
#define __JSON_TOKENER_H__

// #include <vnepogodin/third_party/json-c/json_object.h>
/*
 * $Id: json_object.h,v 1.12 2006/01/30 23:07:57 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */
#ifndef __JSON_OBJECT_H__
#define __JSON_OBJECT_H__

typedef struct _Json_Object json_object;

#define JSON_OBJECT_DEF_HASH_ENTRIES 16

/**
 * A flag for the json_object_to_json_string_ext() function
 * which causes the output to have
 * minimal whitespace inserted to make things slightly more readable.
 */
#define JSON_C_TO_STRING_SPACED 0x01U
/**
 * A flag for the json_object_to_json_string_ext() and
 * json_object_to_file_ext() functions which causes
 * the output to be formatted.
 *
 * See the "Two Space Tab" option at http://jsonformatter.curiousconcept.com/
 * for an example of the format.
 */
#define JSON_C_TO_STRING_PRETTY 0x02U
/**
 * A flag for the json_object_to_json_string_ext() and
 * json_object_to_file_ext() functions which causes
 * the output to be formatted.
 *
 * Instead of a "Two Space Tab" this gives a single tab character.
 */
#define JSON_C_TO_STRING_PRETTY_TAB 0x08U
/**
 * A flag to drop trailing zero for float values
 */
#define JSON_C_TO_STRING_NOZERO 0x04U

/**
 * Don't escape forward slashes.
 */
#define JSON_C_TO_STRING_NOSLASHESCAPE 0x16U

/**
 * A flag for the json_object_object_add_ex function which
 * causes the value to be added without a check if it already exists.
 * Note: it is the responsibility of the caller to ensure that no
 * key is added multiple times. If this is done, results are
 * unpredictable. While this option is somewhat dangerous, it
 * permits potentially large performance savings in code that
 * knows for sure the key values are unique (e.g. because the
 * code adds a well-known set of constant key values).
 */
#define JSON_C_OBJECT_ADD_KEY_IS_NEW 0x02U
/**
 * A flag for the json_object_object_add_ex function which
 * flags the key as being constant memory. This means that
 * the key will NOT be copied via strdup(), resulting in a
 * potentially huge performance win (malloc, strdup and
 * free are usually performance hogs). It is acceptable to
 * use this flag for keys in non-constant memory blocks if
 * the caller ensure that the memory holding the key lives
 * longer than the corresponding json object. However, this
 * is somewhat dangerous and should only be done if really
 * justified.
 * The general use-case for this flag is cases where the
 * key is given as a real constant value in the function
 * call, e.g. as in
 *   json_object_object_add_ex(obj, "ip", json,
 *       JSON_C_OBJECT_KEY_IS_CONSTANT);
 */
#define JSON_C_OBJECT_KEY_IS_CONSTANT 0x04U


#ifdef __cplusplus
extern "C" {
#endif

/* reference counting functions */
/**
 * Increment the reference count of json_object, thereby taking ownership of it.
 *
 * Cases where you might need to increase the refcount include:
 * - Using an object field or array index (retrieved through
 *    `json_object_object_get()` or `json_object_array_get_idx()`)
 *    beyond the lifetime of the parent object.
 * - Detaching an object field or array index from its parent object
 *    (using `json_object_object_del()` or `json_object_array_del_idx()`)
 * - Sharing a json_object with multiple (not necesarily parallel) threads
 *    of execution that all expect to free it (with `json_object_put()`) when
 *    they're done.
 *
 * @param obj The json_object instance
 * @see json_object_put()
 * @see json_object_object_get()
 * @see json_object_array_get_idx()
 */
extern json_object* json_object_get(json_object *);

/**
 * Decrement the reference count of json_object and free if it reaches zero.
 *
 * You must have ownership of obj prior to doing this or you will cause an
 * imbalance in the reference count, leading to a classic use-after-free bug.
 * In particular, you normally do not need to call `json_object_put()` on the
 * json_object returned by `json_object_object_get()` or `json_object_array_get_idx()`.
 *
 * Just like after calling `free()` on a block of memory, you must not use
 * `obj` after calling `json_object_put()` on it or any object that it
 * is a member of (unless you know you've called `json_object_get(obj)` to
 * explicitly increment the refcount).
 *
 * NULL may be passed, which in case this is a no-op.
 *
 * @param obj The json_object instance
 * @returns 1 if the object was freed.
 * @see json_object_get()
 */
int json_object_put(json_object *);

/** Stringify object to json format
 * @see json_object_to_json_string() for details on how to free string.
 * @param obj The json_object instance
 * @param flags formatting options, see JSON_C_TO_STRING_PRETTY and other constants
 * @returns a string in JSON format
 */
extern const char* json_object_to_json_string_ext(json_object *, const int);

/* object type methods */

/** Create a new empty object with a reference count of 1.  The caller of
 * this object initially has sole ownership.  Remember, when using
 * json_object_object_add or json_object_array_put_idx, ownership will
 * transfer to the object/array.  Call json_object_get if you want to maintain
 * shared ownership or also add this object as a child of multiple objects or
 * arrays.  Any ownerships you acquired but did not transfer must be released
 * through json_object_put.
 *
 * @returns a json_object of type json_type_object
 */
extern json_object* json_object_new_object(void);

/** Add an object field to a json_object of type json_type_object
 *
 * The semantics are identical to json_object_object_add, except that an
 * additional flag fields gives you more control over some detail aspects
 * of processing. See the description of JSON_C_OBJECT_ADD_* flags for more
 * details.
 *
 * @param obj The json_object instance
 * @param key The object field name (a private copy will be duplicated)
 * @param val a json_object or NULL member to associate with the given field
 * @param opts process-modifying options. To specify multiple options, use
 *             (OPT1|OPT2)
 */
int json_object_object_add_ex(json_object *, const char* const,
                              json_object *const, const unsigned char);

/** Get the json_object associated with a given object field.
 *
 * This returns true if the key is found, false in all other cases (including
 * if obj isn't a json_type_object).
 *
 * *No* reference counts will be changed.  There is no need to manually adjust
 * reference counts through the json_object_put/json_object_get methods unless
 * you need to have the child (value) reference maintain a different lifetime
 * than the owning parent (obj).  Ownership of value is retained by obj.
 *
 * @param obj The json_object instance
 * @param key The object field name
 * @param value a pointer where to store a reference to the json_object
 *              associated with the given field name.
 *
 *              It is safe to pass a NULL value.
 * @returns whether or not the key exists
 */
extern unsigned char json_object_object_get_ex(const json_object *, const char*,
                                              json_object **);

/* Array type methods */

/** Create a new empty json_object of type json_type_array
 * with the desired number of slots allocated.
 * @see json_object_array_shrink()
 * @param initial_size The number of slots to allocate
 * @returns a json_object of type json_type_array
 */
extern json_object *json_object_new_array_ext(const int);


/** Add an element to the end of a json_object of type json_type_array
 *
 * The reference count will *not* be incremented. This is to make adding
 * fields to objects in code more compact. If you want to retain a reference
 * to an added object you must wrap the passed object with json_object_get
 *
 * @param obj The json_object instance
 * @param val The json_object to be added
 */
extern int json_object_array_add(json_object *, json_object *);

/** Get the element at specified index of array `obj` (which must be a json_object of type json_type_array)
 *
 * *No* reference counts will be changed, and ownership of the returned
 * object remains with `obj`.  See json_object_object_get() for additional
 * implications of this behavior.
 *
 * Calling this with anything other than a json_type_array will trigger
 * an assert.
 *
 * @param obj The json_object instance
 * @param idx The index to get the element at
 * @returns The json_object at the specified index (or NULL)
 */
extern json_object* json_object_array_get_idx(const json_object *, unsigned long);

/**
 * Shrink the internal memory allocation of the array to just
 * enough to fit the number of elements in it, plus empty_slots.
 *
 * @param jso The json_object instance, must be json_type_array
 * @param empty_slots The number of empty slots to leave allocated
 */
extern int json_object_array_shrink(json_object *, const int);

/* int type methods */

/** Create a new empty json_object of type json_type_int
 * Note that values are stored as 64-bit values internally.
 * To ensure the full range is maintained, use json_object_new_int64 instead.
 * @param i The integer
 * @returns a json_object of type json_type_int
 */
extern json_object* json_object_new_int(const int);

/** Create a new empty json_object of type json_type_int
 * @param i The integer
 * @returns a json_object of type json_type_int
 */
extern json_object* json_object_new_int64(const long long);

/** Create a new empty json_object of type json_type_uint
 * @param i The integer
 * @returns a json_object of type json_type_uint
 */
extern json_object* json_object_new_uint64(const unsigned long long);

/** Get the int value of a json_object
 *
 * The type is coerced to a int if the passed object is not a int.
 * double objects will return their integer conversion. Strings will be
 * parsed as an integer. If no conversion exists then 0 is returned
 * and errno is set to EINVAL. null is equivalent to 0 (no error values set)
 *
 * Note that integers are stored internally as 64-bit values.
 * If the value of too big or too small to fit into 32-bit, INT32_MAX or
 * INT32_MIN are returned, respectively.
 *
 * @param obj The json_object instance
 * @returns an int
 */
int json_object_get_int(const json_object *);

extern json_object* json_find(const json_object* __restrict const, const char* __restrict);

/* double type methods */

/** Create a new empty json_object of type json_type_double
 *
 * @see json_object_double_to_json_string() for how to set a custom format string.
 *
 * @param d The double
 * @returns a json_object of type json_type_double
 */
extern json_object* json_object_new_double(const double);

/**
 * Create a new json_object of type json_type_double, using
 * the exact serialized representation of the value.
 *
 * This allows for numbers that would otherwise get displayed
 * inefficiently (e.g. 12.3 => "12.300000000000001") to be
 * serialized with the more convenient form.
 *
 * Notes:
 *
 * This is used by json_tokener_parse_ex() to allow for
 * an exact re-serialization of a parsed object.
 *
 * The userdata field is used to store the string representation, so it
 * can't be used for other data if this function is used.
 *
 * A roughly equivalent sequence of calls, with the difference being that
 *  the serialization function won't be reset by json_object_set_double(), is:
 * @code
 *   jso = json_object_new_double(d);
 *   json_object_set_serializer(jso, json_object_userdata_to_json_string,
 *       strdup(ds), json_object_free_userdata);
 * @endcode
 *
 * @param d The numeric value of the double.
 * @param ds The string representation of the double.  This will be copied.
 */
extern json_object* json_object_new_double_s(const double, const char*);

/** Get the floating point value of a json_object
 *
 * @param obj The json_object instance
 * @returns a floating point number
 */
float json_object_get_float(const json_object *);

#ifdef __cplusplus
}
#endif

#endif


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Parse a json_object out of the string `str`.
 */
json_object* json_tokener_parse(const char*);

#ifdef __cplusplus
}
#endif

#endif  /* __JSON_TOKENER_H__ */

// #include <vnepogodin/third_party/json-c/printbuf.h>
/*
 * $Id: printbuf.h,v 1.4 2006/01/26 02:16:28 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 *
 * Copyright (c) 2008-2009 Yahoo! Inc.  All rights reserved.
 * The copyrights to the contents of this file are licensed under the MIT License
 * (http://www.opensource.org/licenses/mit-license.php)
 */

/**
 * @file
 * @brief Internal string buffer handing.
 *
 */
#ifndef __PRINTBUF_H__
#define __PRINTBUF_H__

typedef struct _Printbuf printbuf;

#ifdef __cplusplus
extern "C" {
#endif

extern void printbuf_memcpy(printbuf *, const void*, const unsigned long);

extern int   printbuf_getSize(printbuf *);
extern int   printbuf_getPos(printbuf *);
extern char* printbuf_getBuf(printbuf *);
extern char* printbuf_get_posBufInPos(printbuf *, const int);

extern void printbuf_addPos(printbuf *, const int);
extern void printbuf_subPos(printbuf *, const int);

extern void printbuf_setBufInPos(printbuf *, const int, const char);

extern printbuf* printbuf_new(void);

/* As an optimization, printbuf_memappend_fast() is defined as a macro
 * that handles copying data if the buffer is large enough; otherwise
 * it invokes printbuf_memappend() which performs the heavy
 * lifting of reallocting the buffer and copying data.
 *
 * Your code should not use printbuf_memappend() directly unless it
 * checks the return code. Use printbuf_memappend_fast() instead.
 */
extern int printbuf_memappend(printbuf *, const char*, const unsigned long);

#define printbuf_memappend_fast(p, bufptr, bufsize) do{                \
		if ((printbuf_getSize(p) - printbuf_getPos(p)) > bufsize) {    \
			printbuf_memcpy(p, (bufptr), (unsigned long)(bufsize));    \
			printbuf_addPos(p, (int)(bufsize));                        \
			printbuf_setBufInPos(p, printbuf_getPos(p), '\0');         \
		} else                                                         \
			printbuf_memappend(p, (bufptr), (unsigned long)(bufsize)); \
}while(0)

/**
 * Results in a compile error if the argument is not a string literal.
 */
#define _printbuf_check_literal(mystr) ("" mystr)

/**
 * This is an optimization wrapper around printbuf_memappend() that is useful
 * for appending string literals. Since the size of string constants is known
 * at compile time, using this macro can avoid a costly strlen() call. This is
 * especially helpful when a constant string must be appended many times. If
 * you got here because of a compilation error caused by passing something
 * other than a string literal, use printbuf_memappend_fast() in conjunction
 * with strlen().
 *
 * See also:
 *   printbuf_memappend_fast()
 *   printbuf_memappend()
 *   sprintbuf()
 */
#define printbuf_strappend(pb, str) \
	printbuf_memappend((pb), _printbuf_check_literal(str), sizeof(str) - 1)

/**
 * Set len bytes of the buffer to charvalue, starting at offset offset.
 * Similar to calling memset(x, charvalue, len);
 *
 * The memory allocated for the buffer is extended as necessary.
 *
 * If offset is -1, this starts at the end of the current data in the buffer.
 */
int printbuf_memset(printbuf *, int, const int, const int);

extern void printbuf_reset(printbuf *p);

extern void printbuf_free(printbuf *p);

#ifdef __cplusplus
}
#endif

#endif /* __PRINTBUF_H__ */


#include <ctype.h>
#include <limits.h> /* LLONG_MAX, INT_MAX.. */
#include <math.h>
#include <stdlib.h>
#include <string.h>

enum json_tokener_state {
    json_tokener_state_eatws,
    json_tokener_state_start,
    json_tokener_state_finish,
    json_tokener_state_null,
    json_tokener_state_comment_start,
    json_tokener_state_comment,
    json_tokener_state_comment_eol,
    json_tokener_state_comment_end,
    json_tokener_state_number,
    json_tokener_state_array,
    json_tokener_state_array_add,
    json_tokener_state_array_sep,
    json_tokener_state_object_field_start,
    json_tokener_state_object_field,
    json_tokener_state_object_field_end,
    json_tokener_state_object_value,
    json_tokener_state_object_value_add,
    json_tokener_state_object_sep,
    json_tokener_state_array_after_sep,
    json_tokener_state_object_field_start_after_sep,
};

enum json_tokener_error {
    json_tokener_success,
    json_tokener_error_depth,
    json_tokener_error_parse_unexpected,
    json_tokener_error_parse_null,
    json_tokener_error_parse_number,
    json_tokener_error_parse_array,
    json_tokener_error_parse_object_key_name,
    json_tokener_error_parse_object_key_sep,
    json_tokener_error_parse_object_value_sep,
    json_tokener_error_parse_comment,
    json_tokener_error_size
};

struct json_tokener_srec {
    enum json_tokener_state state, saved_state;

    char* obj_field_name;

    json_object* current;
};

/**
 * Internal state of the json parser.
 * Do not access any fields of this structure directly.
 * Its definition is published due to historical limitations
 * in the json tokener API, and will be changed to be an opaque
 * type in the future.
 */
struct json_tokener {
    enum json_tokener_error err;

    unsigned int is_double;

    char quote_char;

    int char_offset;

    int max_depth, depth, st_pos;

    printbuf* pb;
    struct json_tokener_srec* stack;
};

#ifdef _WIN32
# define strdup _strdup
# define strncasecmp _strnicmp
#endif

#define json_min(a, b) (((a) < (b)) ? (a) : (b))

/* PARSING */
static inline
int json_parse_uint64(const char* buf, unsigned long long* return_value) {
    while (*buf == ' ')
        buf++;

    if (*buf == '-')
        return 1; /* error: uint cannot be negative */

    char* end = NULL;
    register const unsigned long long val = strtoull(buf, &end, 10);
    if (end != buf)
        *return_value = val;

    return ((val == 0) || (end == buf)) ? 1 : 0;
}

static inline
unsigned char json_tokener_parse_double(const char* buf, const int len, double* return_value) {
    char* end = NULL;
    *return_value = strtod(buf, &end);

    register const char* _expr = buf + len;
    register unsigned char result = 1U;
    if (_expr == end)
        result = 0U; /* It worked */

    return result;
}

/* RESETTING */
static inline
void json_tokener_reset_level(struct json_tokener* tok, const int depth) {
    tok->stack[depth].state = json_tokener_state_eatws;
    tok->stack[depth].saved_state = json_tokener_state_start;

    json_object_put(tok->stack[depth].current);
    tok->stack[depth].current = NULL;

    free(tok->stack[depth].obj_field_name);
    tok->stack[depth].obj_field_name = NULL;
}

/**
 * Reset the state of a json_tokener, to prepare
 * to parse a brand new JSON object.
 */
static inline
void json_tokener_reset(struct json_tokener* tok) {
    if (tok != NULL) {
        while (tok->depth >= 0) {
            json_tokener_reset_level(tok, tok->depth);
            --tok->depth;
        }

        tok->depth = 0;
        tok->err = json_tokener_success;
    }
}

/* FREEING */
static inline
void json_tokener_free(struct json_tokener* tok) {
    json_tokener_reset(tok);
    if (tok->pb != NULL)
        printbuf_free(tok->pb);

    free(tok->stack);
    free(tok);
}

/**
 * Allocate a new json_tokener with a custom max nesting depth.
 * When done using that to parse objects, free it with json_tokener_free().
 * @see JSON_TOKENER_DEFAULT_DEPTH
 */
static struct json_tokener* json_tokener_new_ex(const int depth) {
    struct json_tokener* tok = (struct json_tokener*)calloc(1UL, sizeof(struct json_tokener));
    if (tok == NULL)
        return NULL;

    tok->stack = (struct json_tokener_srec*)calloc((unsigned long)depth, sizeof(struct json_tokener_srec));
    if (tok->stack == NULL) {
        free(tok);
        return NULL;
    }

    tok->pb = printbuf_new();
    if (tok->pb == NULL) {
        free(tok);
        free(tok->stack);
        return NULL;
    }

    tok->max_depth = depth;
    json_tokener_reset(tok);
    return tok;
}

static json_object* json_tokener_parse_ex(struct json_tokener* tok, const char* str, const int len) {
    tok->char_offset = 0;
    tok->err = json_tokener_success;

    /* this interface is presently not 64-bit clean due to the int len argument
     * and the internal printbuf interface that takes 32-bit int len arguments
     * so the function limits the maximum string size to INT32_MAX (2GB).
     * If the function is called with len == -1 then strlen is called to check
     * the string length is less than INT32_MAX (2GB)
     */
    if ((len < -1) || ((len == -1) && (strlen(str) > INT_MAX))) {
        tok->err = json_tokener_error_size;
        return NULL;
    }

    json_object* obj = NULL;
    char c = 1;

    while (1) {
        c = *str;

redo_char:
        switch (tok->stack[tok->depth].state) {
            case json_tokener_state_eatws:
                /* Advance until we change state */
                while (isspace((int)c)) {
                    ++str;
                    tok->char_offset++;

                    c = *str;
                }

                if (c == '/') {
                    printbuf_reset(tok->pb);
                    printbuf_memappend_fast(tok->pb, &c, 1);
                    tok->stack[tok->depth].state = json_tokener_state_comment_start;
                } else {
                    tok->stack[tok->depth].state = tok->stack[tok->depth].saved_state;
                    goto redo_char;
                }
                break;

            case json_tokener_state_start:
                switch (c) {
                    case '{':
                        tok->stack[tok->depth].state = json_tokener_state_eatws;
                        tok->stack[tok->depth].saved_state = json_tokener_state_object_field_start;
                        tok->stack[tok->depth].current = json_object_new_object();
                        if (tok->stack[tok->depth].current == NULL)
                            goto out;

                        break;
                    case '[':
                        tok->stack[tok->depth].state = json_tokener_state_eatws;
                        tok->stack[tok->depth].saved_state = json_tokener_state_array;
                        tok->stack[tok->depth].current = json_object_new_array_ext(32);
                        if (tok->stack[tok->depth].current == NULL)
                            goto out;
                        break;
                    case 'N':
                    case 'n':
                        tok->stack[tok->depth].state = json_tokener_state_null; /* or NaN */
                        printbuf_reset(tok->pb);
                        tok->st_pos = 0;
                        goto redo_char;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '-':
                        tok->stack[tok->depth].state = json_tokener_state_number;
                        printbuf_reset(tok->pb);

                        tok->is_double = 0U;
                        goto redo_char;
                    default:
                        tok->err = json_tokener_error_parse_unexpected;
                        return NULL;
                }
                break;

            case json_tokener_state_finish:
                if (tok->depth == 0)
                    goto out;

                obj = json_object_get(tok->stack[tok->depth].current);
                json_tokener_reset_level(tok, tok->depth);
                tok->depth--;
                goto redo_char;
            case json_tokener_state_null: { /* starts with 'n' */
                printbuf_memappend_fast(tok->pb, &c, 1);

                register const unsigned long size = json_min((unsigned long)tok->st_pos + 1UL, 3UL);
                register const unsigned long size_nan = json_min((unsigned long)tok->st_pos + 1UL, 3UL);
                if (strncasecmp("null", printbuf_getBuf(tok->pb), size) == 0) {
                    if (tok->st_pos == 3) {
                        tok->stack[tok->depth].current = NULL;
                        tok->stack[tok->depth].saved_state = json_tokener_state_finish;
                        tok->stack[tok->depth].state = json_tokener_state_eatws;
                        goto redo_char;
                    }
                } else if (strncasecmp("NaN", printbuf_getBuf(tok->pb), size_nan) == 0) {
                    if (tok->st_pos == 2) {
                        tok->stack[tok->depth].current = json_object_new_double((double)NAN);
                        if (tok->stack[tok->depth].current == NULL)
                            return NULL;

                        tok->stack[tok->depth].saved_state = json_tokener_state_finish;
                        tok->stack[tok->depth].state = json_tokener_state_eatws;
                        goto redo_char;
                    }
                } else {
                    tok->err = json_tokener_error_parse_null;
                    goto out;
                }
                tok->st_pos++;
            } break;

            case json_tokener_state_comment_start:
                if (c == '*')
                    tok->stack[tok->depth].state = json_tokener_state_comment;
                else if (c == '/')
                    tok->stack[tok->depth].state = json_tokener_state_comment_eol;
                else {
                    tok->err = json_tokener_error_parse_comment;
                    return NULL;
                }
                printbuf_memappend_fast(tok->pb, &c, 1);
                break;

            case json_tokener_state_comment: {
                /* Advance until we change state */
                const char* case_start = str;
                while (c != '*') {
                    ++str;
                    tok->char_offset++;
                    c = *str;
                }
                printbuf_memappend_fast(tok->pb, case_start, 1 + str - case_start);
                tok->stack[tok->depth].state = json_tokener_state_comment_end;
            } break;

            case json_tokener_state_comment_eol: {
                /* Advance until we change state */
                const char* case_start = str;
                while (c != '\n') {
                    ++str;
                    tok->char_offset++;
                    c = *str;
                }
                printbuf_memappend_fast(tok->pb, case_start, str - case_start);
                tok->stack[tok->depth].state = json_tokener_state_eatws;
            } break;

            case json_tokener_state_comment_end:
                printbuf_memappend_fast(tok->pb, &c, 1);
                if (c == '/')
                    tok->stack[tok->depth].state = json_tokener_state_eatws;
                else
                    tok->stack[tok->depth].state = json_tokener_state_comment;
                break;

                // ===================================================

            case json_tokener_state_number: {
                /* Advance until we change state */
                const char* case_start = str;
                register int case_len = 0;
                register unsigned char neg_sign_ok = 1U;
                register unsigned char pos_sign_ok = 0U;
                register unsigned char is_exponent = 0U;
                if (printbuf_getPos(tok->pb) > 0) {
                    /* We don't save all state from the previous incremental parse
                     * so we need to re-generate it based on the saved string so far.
                     */
                    char* e_loc = strchr(printbuf_getBuf(tok->pb), 'e');
                    if (e_loc == NULL)
                        e_loc = strchr(printbuf_getBuf(tok->pb), 'E');

                    if (e_loc != NULL) {
                        const char* last_saved_char = printbuf_get_posBufInPos(tok->pb, printbuf_getPos(tok->pb) - 1);
                        is_exponent = 1U;
                        pos_sign_ok = neg_sign_ok = 1U;
                        /* If the "e" isn't at the end, we can't start with a '-' */
                        if (e_loc != last_saved_char) {
                            neg_sign_ok = 0U;
                            pos_sign_ok = 0U;
                        }
                        /* else leave it set to 1, i.e. start of the new input */
                    }
                }

                while ((c) && (((c >= '0') && (c <= '9')) ||
                               ((!is_exponent) && ((c == 'e') || (c == 'E'))) ||
                               ((neg_sign_ok) && (c == '-')) || ((pos_sign_ok) && (c == '+')) ||
                               ((!tok->is_double) && (c == '.')))) {
                    pos_sign_ok = neg_sign_ok = 0U;
                    ++case_len;

                    /* non-digit characters checks */
                    /* note: since the main loop condition to get here was
                     * an input starting with 0-9 or '-', we are
                     * protected from input starting with '.' or
                     * e/E.
                     */
                    switch (c) {
                        case '.':
                            tok->is_double = 1U;
                            pos_sign_ok = 1U;
                            neg_sign_ok = 1U;
                            break;
                        case 'e': /* FALLTHRU */
                        case 'E':
                            is_exponent = 1U;
                            tok->is_double = 1U;
                            /* the exponent part can begin with a negative sign */
                            pos_sign_ok = neg_sign_ok = 1U;
                            break;
                        default:
                            break;
                    }

                    ++str;
                    tok->char_offset++;
                    c = *str;
                }
                /*
                 *	Now we know c isn't a valid number char, but check whether
                 *	it might have been intended to be, and return a potentially
                 *	more understandable error right away.
                 *	However, if we're at the top-level, use the number as-is
                 *  because c can be part of a new object to parse on the
                 *	next call to json_tokener_parse().
                 */
                if ((tok->depth > 0) && (c != ',') && (c != ']') && (c != '}') && (c != '/') &&
                    (c != 'I') && (c != 'i') && (!isspace((int)c))) {
                    tok->err = json_tokener_error_parse_number;
                    return NULL;
                }
                if (case_len > 0)
                    printbuf_memappend_fast(tok->pb, case_start, case_len);

                if (tok->is_double) {
                    /* Trim some chars off the end, to allow things
                     *  like "123e+" to parse ok.
                     */
                    while (printbuf_getPos(tok->pb) > 1) {
                        register const char last_char = printbuf_getBuf(tok->pb)[printbuf_getPos(tok->pb) - 1];
                        if ((last_char != 'e') && (last_char != 'E') &&
                            (last_char != '-') && (last_char != '+')) {
                            break;
                        }

                        printbuf_setBufInPos(tok->pb, printbuf_getPos(tok->pb) - 1, '\0');
                        printbuf_subPos(tok->pb, 1);
                    }
                }
            }
                {
                    long long num64 = 0LL;
                    unsigned long long numuint64 = 0ULL;
                    double numd = 0.0;
                    if ((!tok->is_double) && (printbuf_getBuf(tok->pb)[0] != '-') && (json_parse_uint64(printbuf_getBuf(tok->pb), &numuint64) == 0)) {
                        if (numuint64 <= LLONG_MAX) {
                            num64 = (long long)numuint64;
                            tok->stack[tok->depth].current = json_object_new_int64(num64);
                            if (tok->stack[tok->depth].current == NULL)
                                return NULL;
                        } else {
                            tok->stack[tok->depth].current = json_object_new_uint64(numuint64);
                            if (tok->stack[tok->depth].current == NULL)
                                return NULL;
                        }
                    } else if ((tok->is_double) && (json_tokener_parse_double(printbuf_getBuf(tok->pb), printbuf_getPos(tok->pb), &numd) == 0)) {
                        tok->stack[tok->depth].current = json_object_new_double_s(numd, printbuf_getBuf(tok->pb));
                        if (tok->stack[tok->depth].current == NULL)
                            return NULL;
                    } else {
                        tok->err = json_tokener_error_parse_number;
                        return NULL;
                    }
                    tok->stack[tok->depth].saved_state = json_tokener_state_finish;
                    tok->stack[tok->depth].state = json_tokener_state_eatws;

                    goto redo_char;
                }

            case json_tokener_state_array_after_sep:
            case json_tokener_state_array:
                if (c == ']') {
                    /* Minimize memory usage; assume parsed objs are unlikely to be changed */
                    json_object_array_shrink(tok->stack[tok->depth].current, 0);

                    tok->stack[tok->depth].saved_state = json_tokener_state_finish;
                    tok->stack[tok->depth].state = json_tokener_state_eatws;
                } else {
                    if (tok->depth >= tok->max_depth - 1) {
                        tok->err = json_tokener_error_depth;
                        return NULL;
                    }
                    tok->stack[tok->depth].state = json_tokener_state_array_add;
                    tok->depth++;
                    json_tokener_reset_level(tok, tok->depth);
                    goto redo_char;
                }
                break;

            case json_tokener_state_array_add:
                if (json_object_array_add(tok->stack[tok->depth].current, obj) != 0)
                    goto out;

                tok->stack[tok->depth].saved_state = json_tokener_state_array_sep;
                tok->stack[tok->depth].state = json_tokener_state_eatws;
                goto redo_char;

            case json_tokener_state_array_sep:
                if (c == ']') {
                    /* Minimize memory usage; assume parsed objects are unlikely to be changed */
                    json_object_array_shrink(tok->stack[tok->depth].current, 0);

                    tok->stack[tok->depth].saved_state = json_tokener_state_finish;
                    tok->stack[tok->depth].state = json_tokener_state_eatws;
                } else if (c == ',') {
                    tok->stack[tok->depth].saved_state = json_tokener_state_array_after_sep;
                    tok->stack[tok->depth].state = json_tokener_state_eatws;
                } else {
                    tok->err = json_tokener_error_parse_array;
                    return NULL;
                }
                break;

            case json_tokener_state_object_field_start:
            case json_tokener_state_object_field_start_after_sep:
                if (c == '}') {
                    tok->stack[tok->depth].saved_state = json_tokener_state_finish;
                    tok->stack[tok->depth].state = json_tokener_state_eatws;
                } else if ((c == '"') || (c == '\'')) {
                    tok->quote_char = c;
                    printbuf_reset(tok->pb);
                    tok->stack[tok->depth].state = json_tokener_state_object_field;
                } else {
                    tok->err = json_tokener_error_parse_object_key_name;
                    return NULL;
                }
                break;

            case json_tokener_state_object_field: {
                /* Advance until we change state */
                const char* case_start = str;
                while (1) {
                    if (c == tok->quote_char) {
                        printbuf_memappend_fast(tok->pb, case_start, str - case_start);

                        tok->stack[tok->depth].obj_field_name = strdup(printbuf_getBuf(tok->pb));
                        tok->stack[tok->depth].saved_state = json_tokener_state_object_field_end;
                        tok->stack[tok->depth].state = json_tokener_state_eatws;
                        break;
                    }

                    ++str;
                    tok->char_offset++;
                    c = *str;
                }
            } break;

            case json_tokener_state_object_field_end:
                if (c == ':') {
                    tok->stack[tok->depth].saved_state = json_tokener_state_object_value;
                    tok->stack[tok->depth].state = json_tokener_state_eatws;
                } else {
                    tok->err = json_tokener_error_parse_object_key_sep;
                    return NULL;
                }
                break;

            case json_tokener_state_object_value:
                if (tok->depth >= tok->max_depth - 1) {
                    tok->err = json_tokener_error_depth;
                    return NULL;
                }

                tok->stack[tok->depth].state = json_tokener_state_object_value_add;
                tok->depth++;
                json_tokener_reset_level(tok, tok->depth);
                goto redo_char;

            case json_tokener_state_object_value_add:
                json_object_object_add_ex(tok->stack[tok->depth].current, tok->stack[tok->depth].obj_field_name, obj, 0);

                free(tok->stack[tok->depth].obj_field_name);
                tok->stack[tok->depth].obj_field_name = NULL;

                tok->stack[tok->depth].saved_state = json_tokener_state_object_sep;
                tok->stack[tok->depth].state = json_tokener_state_eatws;
                goto redo_char;

            case json_tokener_state_object_sep: /* { */
                if (c == '}') {
                    tok->stack[tok->depth].saved_state = json_tokener_state_finish;
                    tok->stack[tok->depth].state = json_tokener_state_eatws;
                } else if (c == ',') {
                    tok->stack[tok->depth].saved_state = json_tokener_state_object_field_start_after_sep;
                    tok->stack[tok->depth].state = json_tokener_state_eatws;
                } else {
                    tok->err = json_tokener_error_parse_object_value_sep;
                    return NULL;
                }
                break;
        }

        ++str;
        tok->char_offset++;
    } /* while(1) */

out:
    if (tok->err == json_tokener_success) {
        json_object* ret = json_object_get(tok->stack[tok->depth].current);

        /* Partially reset, so we parse additional objects on subsequent calls. */
        register int ii = tok->depth;
        while (ii >= 0) {
            json_tokener_reset_level(tok, ii);
            --ii;
        }
        return ret;
    }

    return NULL;
}

json_object* json_tokener_parse(const char* str) {
    register json_object* result = NULL;
    struct json_tokener* tok = json_tokener_new_ex(32);

    if (tok != NULL) {
        register json_object* obj = json_tokener_parse_ex(tok, str, -1);

        if (tok->err != json_tokener_success) {
            if (obj != NULL)
                json_object_put(obj);

            obj = NULL;
        }

        json_tokener_free(tok);

        result = obj;
    }

    return result;
}
// #include <vnepogodin/third_party/json-c/arraylist.h>
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
array_list* array_list_new(void (*)(void*), const int);

extern void array_list_free(array_list*);

extern void* array_list_get_idx(array_list*, const unsigned long);

extern int array_list_add(array_list*, const void* const);

extern unsigned long array_list_length(const array_list*);

/**
 * Shrink the array list to just enough to fit the number of elements in it,
 * plus empty_slots.
 */
extern int array_list_shrink(array_list*, const unsigned long);

#ifdef __cplusplus
}
#endif

#endif /* __ARRAYLIST_H__ */


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

inline
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

inline
void* array_list_get_idx(array_list* arr, const unsigned long i) {
    register void* result = NULL;
    if (i < arr->length)
        result = arr->array[i];

    return result;
}

inline
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
inline
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

inline
unsigned long array_list_length(const array_list* arr) {
    return arr->length;
}
/*
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */
// #include <vnepogodin/third_party/json-c/json_object.h>

// #include <vnepogodin/third_party/json-c/arraylist.h>

// #include <vnepogodin/third_party/json-c/linkhash.h>
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

/**
 * @file
 * @brief Internal methods for working with json_type_object objects.  Although
 *        this is exposed by the json_object_get_object() function and within the
 *        json_object_iter type, it is not recommended for direct use.
 */
#ifndef _linkhash_h_
#define _linkhash_h_

typedef struct _LH_Entry lh_entry;
typedef struct _LH_Table lh_table;

/**
 * callback function prototypes
 */
typedef void (lh_entry_free_fn)(lh_entry *e);
/**
 * callback function prototypes
 */
typedef unsigned long (lh_hash_fn)(const void *k);
/**
 * callback function prototypes
 */
typedef int (lh_equal_fn)(const void *k1, const void *k2);


/**
 * An entry in the hash table
 */
struct _LH_Entry {
	/**
     * The key.  Use lh_entry_k() instead of accessing this directly.
     */
    const void *k;
	/**
	 * A flag for users of linkhash to know whether or not they
	 * need to free k.
	 */
	int k_is_constant;
	/**
	 * The value.  Use lh_entry_v() instead of accessing this directly.
	 */
	const void *v;
	/**
	 * The next entry
	 */
	lh_entry *next;
	/**
	 * The previous entry.
	 */
	lh_entry *prev;
};

/**
 * The hash table structure.
 */
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
	 * The first entry.
	 */
	lh_entry *head;

	/**
	 * The last entry.
	 */
    lh_entry *tail;
	lh_entry *table;

	/**
	 * A pointer onto the function responsible for freeing an entry.
	 */
	lh_entry_free_fn *free_fn;
	lh_hash_fn *hash_fn;
	lh_equal_fn *equal_fn;
};


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

#ifdef __UNCONST
# define _LH_UNCONST(a) __UNCONST(a)
#else
# define _LH_UNCONST(a) ((void *)(unsigned long)(const void *)(a))
#endif

/**
 * Convenience list iterator.
 */
#define lh_foreach(table, entry) for (entry = table->head; entry; entry = entry->next)

/**
 * lh_foreach_safe allows calling of deletion routine while iterating.
 *
 * @param table a lh_table* to iterate over
 * @param entry a lh_entry* variable to hold each element
 * @param tmp a lh_entry* variable to hold a temporary pointer to the next element
 */
#define lh_foreach_safe(table, entry, tmp) \
	for (entry = table->head; entry && ((tmp = entry->next) || 1); entry = tmp)

/**
 * Return a non-const version of lh_entry.k.
 *
 * lh_entry.k is const to indicate and help ensure that linkhash itself doesn't modify
 * it, but callers are allowed to do what they want with it.
 * See also lh_entry.k_is_constant
 */
#define lh_entry_k(entry) _LH_UNCONST((entry)->k)

/**
 * Return a non-const version of lh_entry.v.
 *
 * v is const to indicate and help ensure that linkhash itself doesn't modify
 * it, but callers are allowed to do what they want with it.
 */
#define lh_entry_v(entry) _LH_UNCONST((entry)->v)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This function sets the hash function to be used for strings.
 * Must be one of the JSON_C_STR_HASH_* values.
 * @returns 0 - ok, -1 if parameter was invalid
 */
extern int json_global_set_string_hash(const int h);


/**
 * Create a new linkhash table.
 *
 * @param size initial table size. The table is automatically resized
 * although this incurs a performance penalty.
 * @param free_fn callback function used to free memory for entries
 * when lh_table_free or lh_table_delete is called.
 * If NULL is provided, then memory for keys and values
 * must be freed by the caller.
 * @param hash_fn  function used to hash keys. 2 standard ones are defined:
 * lh_ptr_hash and lh_char_hash for hashing pointer values
 * and C strings respectively.
 * @param equal_fn comparison function to compare keys. 2 standard ones defined:
 * lh_ptr_hash and lh_char_hash for comparing pointer values
 * and C strings respectively.
 * @return On success, a pointer to the new linkhash table is returned.
 * 	On error, a null pointer is returned.
 */
extern lh_table* lh_table_new(int size, lh_entry_free_fn *free_fn, lh_hash_fn *hash_fn,
                                     lh_equal_fn *equal_fn);

/**
 * Convenience function to create a new linkhash table with char keys.
 *
 * @param size initial table size.
 * @param free_fn callback function used to free memory for entries.
 * @return On success, a pointer to the new linkhash table is returned.
 * 	On error, a null pointer is returned.
 */
extern lh_table* lh_kchar_table_new(int size, lh_entry_free_fn *free_fn);

/**
 * Convenience function to create a new linkhash table with ptr keys.
 *
 * @param size initial table size.
 * @param free_fn callback function used to free memory for entries.
 * @return On success, a pointer to the new linkhash table is returned.
 * 	On error, a null pointer is returned.
 */
extern lh_table* lh_kptr_table_new(int size, lh_entry_free_fn *free_fn);

/**
 * Free a linkhash table.
 *
 * If a lh_entry_free_fn callback free function was provided then it is
 * called for all entries in the table.
 *
 * @param t table to free.
 */
extern void lh_table_free(lh_table *t);

/**
 * Insert a record into the table.
 *
 * @param t the table to insert into.
 * @param k a pointer to the key to insert.
 * @param v a pointer to the value to insert.
 *
 * @return On success, <code>0</code> is returned.
 * 	On error, a negative value is returned.
 */
extern int lh_table_insert(lh_table *t, const void *k, const void *v);

/**
 * Insert a record into the table using a precalculated key hash.
 *
 * The hash h, which should be calculated with lh_get_hash() on k, is provided by
 *  the caller, to allow for optimization when multiple operations with the same
 *  key are known to be needed.
 *
 * @param t the table to insert into.
 * @param k a pointer to the key to insert.
 * @param v a pointer to the value to insert.
 * @param h hash value of the key to insert
 * @param opts if set to JSON_C_OBJECT_KEY_IS_CONSTANT, sets lh_entry.k_is_constant
 *             so t's free function knows to avoid freeing the key.
 */
extern int lh_table_insert_w_hash(lh_table *t, const void *k, const void *v,
                                  const unsigned long h, const unsigned opts);

/**
 * Lookup a record in the table.
 *
 * @param t the table to lookup
 * @param k a pointer to the key to lookup
 * @return a pointer to the record structure of the value or NULL if it does not exist.
 */
extern lh_entry* lh_table_lookup_entry(lh_table *t, const void *k);

/**
 * Lookup a record in the table using a precalculated key hash.
 *
 * The hash h, which should be calculated with lh_get_hash() on k, is provided by
 *  the caller, to allow for optimization when multiple operations with the same
 *  key are known to be needed.
 *
 * @param t the table to lookup
 * @param k a pointer to the key to lookup
 * @param h hash value of the key to lookup
 * @return a pointer to the record structure of the value or NULL if it does not exist.
 */
extern lh_entry *lh_table_lookup_entry_w_hash(lh_table *t, const void *k,
                                                     const unsigned long h);

/**
 * Lookup a record in the table.
 *
 * @param t the table to lookup
 * @param k a pointer to the key to lookup
 * @param v a pointer to a where to store the found value (set to NULL if it doesn't exist).
 * @return whether or not the key was found
 */
extern unsigned char lh_table_lookup_ex(lh_table *t, const void *k, void **v);

/**
 * Delete a record from the table.
 *
 * If a callback free function is provided then it is called for the
 * for the item being deleted.
 * @param t the table to delete from.
 * @param e a pointer to the entry to delete.
 * @return 0 if the item was deleted.
 * @return -1 if it was not found.
 */
extern int lh_table_delete_entry(lh_table *t, lh_entry *e);

/**
 * Delete a record from the table.
 *
 * If a callback free function is provided then it is called for the
 * for the item being deleted.
 * @param t the table to delete from.
 * @param k a pointer to the key to delete.
 * @return 0 if the item was deleted.
 * @return -1 if it was not found.
 */
extern int lh_table_delete(lh_table *t, const void *k);

extern int lh_table_length(lh_table *t);

/**
 * Resizes the specified table.
 *
 * @param t Pointer to table to resize.
 * @param new_size New table size. Must be positive.
 *
 * @return On success, <code>0</code> is returned.
 * 	On error, a negative value is returned.
 */
int lh_table_resize(lh_table *t, int new_size);

/**
 * Calculate the hash of a key for a given table.
 *
 * This is an exension to support functions that need to calculate
 * the hash several times and allows them to do it just once and then pass
 * in the hash to all utility functions. Depending on use case, this can be a
 * considerable performance improvement.
 * @param t the table (used to obtain hash function)
 * @param k a pointer to the key to lookup
 * @return the key's hash
 */
static inline unsigned long lh_get_hash(const lh_table *t, const void *k) {
	return t->hash_fn(k);
}

#ifdef __cplusplus
}
#endif

#endif

// #include <vnepogodin/third_party/json-c/printbuf.h>


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

/** Get the length of a json_object of type json_type_array
 * @param obj the json_object instance
 * @returns an int
 */
static inline unsigned long json_object_array_length(const json_object* jso) {
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

inline json_object* json_find(const json_object* __restrict const j, const char* __restrict key) {
    json_object* temp_json = NULL;

    json_object_object_get_ex(j, key, &temp_json);

    return temp_json;
}
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
// #include <vnepogodin/third_party/json-c/linkhash.h>

// #include <vnepogodin/third_party/json-c/json_object.h>


#include <limits.h> /* INT_MAX, LONG_MAX.. */
#include <stddef.h>
#include <stdlib.h> /* arc4random, rand_r */
#include <string.h> /* strlen */

#ifdef __linux__
# include <fcntl.h>  /* openat, O_RDONLY */
# include <unistd.h> /* pread, close */
#elif _WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <bcrypt.h> /* BCryptGenRandom */
# pragma comment(lib, "bcrypt.lib")
#endif

/* hash functions */
static inline unsigned long lh_char_hash(const void* k);
static inline unsigned long lh_perllike_str_hash(const void* k) {
    const char* rkey = (const char*)k;
    unsigned hashval = 1U;

    while (*rkey != '\0')
        hashval = hashval * 33U + (unsigned)*rkey++;

    return hashval;
}
static lh_hash_fn* char_hash_fn = lh_char_hash;

/* comparison functions */
static inline int lh_char_equal(const void* k1, const void* k2) {
    return (strcmp((const char*)k1, (const char*)k2) == 0);
}

static inline int lh_ptr_equal(const void* k1, const void* k2) {
    return (k1 == k2);
}

inline int json_global_set_string_hash(const int h) {
    switch (h) {
        case JSON_C_STR_HASH_DFLT:
            char_hash_fn = lh_char_hash;
            break;
        case JSON_C_STR_HASH_PERLLIKE:
            char_hash_fn = lh_perllike_str_hash;
            break;
        default:
            return -1;
    }
    return 0;
}

static inline unsigned long lh_ptr_hash(const void* k) {
    /* CAW: refactored to be 64bit nice */
    return (unsigned long)((((ptrdiff_t)k * (ptrdiff_t)LH_PRIME) >> 4UL) & (ptrdiff_t)ULONG_MAX);
}

static int get_random_seed(void) {
#ifdef __linux__
    register int fd = openat(0, "/dev/urandom", O_RDONLY, 0);
    unsigned char buf[1] = {0U};

    if (fd != -1) {
        pread(fd, buf, 1, 0);
        close(fd);
    }

    unsigned __random = buf[0];

    return rand_r(&__random);
#elif _WIN32
    UINT __random = 0U;

    BCryptGenRandom(NULL, (BYTE*)&__random, sizeof(UINT), BCRYPT_USE_SYSTEM_PREFERRED_RNG);

    return (int)__random;
#else
    return (int)arc4random();
#endif
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
#if (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) &&       \
     __BYTE_ORDER == __LITTLE_ENDIAN) ||                        \
    (defined(i386) || defined(__i386__) || defined(__i486__) || \
     defined(__i586__) || defined(__i686__) || defined(vax) ||  \
     defined(MIPSEL))
    #define HASH_LITTLE_ENDIAN 1
    #define HASH_BIG_ENDIAN 0
#elif (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && \
       __BYTE_ORDER == __BIG_ENDIAN) ||                  \
    (defined(sparc) || defined(POWERPC) || defined(mc68000) || defined(sel))
    #define HASH_LITTLE_ENDIAN 0
    #define HASH_BIG_ENDIAN 1
#else
    #define HASH_LITTLE_ENDIAN 0
    #define HASH_BIG_ENDIAN 0
#endif

#define hashsize(n) (1U << (n))
#define hashmask(n) (hashsize(n) - 1U)
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
#define mix(a,b,c) { \
	a -= c;  a ^= rot(c, 4);  c += b; \
	b -= a;  b ^= rot(a, 6);  a += c; \
	c -= b;  c ^= rot(b, 8);  b += a; \
	a -= c;  a ^= rot(c, 16); c += b; \
	b -= a;  b ^= rot(a, 19); a += c; \
	c -= b;  c ^= rot(b, 4);  b += a; \
}
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
#define final(a,b,c) { \
	c ^= b; c -= rot(b, 14); \
	a ^= c; a -= rot(c, 11); \
	b ^= a; b -= rot(a, 25); \
	c ^= b; c -= rot(b, 16); \
	a ^= c; a -= rot(c, 4);  \
	b ^= a; b -= rot(a, 14); \
	c ^= b; c -= rot(b, 24); \
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
	unsigned a, b, c; /* internal state */
	union {
		const void *ptr;
		unsigned long i;
	} u; /* needed for Mac Powerbook G4 */

	/* Set up the internal state */
	a = b = c = 0xdeadbeef + ((unsigned)length) + initval;

	u.ptr = key;
	if (HASH_LITTLE_ENDIAN && ((u.i & 0x3U) == 0)) {
		const unsigned* k = (const unsigned *)key; /* read 32-bit chunks */

		/*------ all but last block: aligned reads and affect 32 bits of (a,b,c) */
		while (length > 12) {
			a += k[0];
			b += k[1];
			c += k[2];
			mix(a,b,c);
			length -= 12;
			k += 3;
		}

        /*----------------------------- handle the last (probably partial) block */
		/*
         * "k[2]&0xffffff" actually reads beyond the end of the string, but
         * then masks off the part it's not allowed to read.  Because the
         * string is aligned, the masked-off tail is in the same word as the
         * rest of the string.  Every machine with memory protection I've seen
         * does it on word boundaries, so is OK with this.  But VALGRIND will
         * still catch it and complain.  The masking trick does make the hash
         * noticably faster for short strings (like English words).
         * AddressSanitizer is similarly picky about overrunning
         * the buffer. (http://clang.llvm.org/docs/AddressSanitizer.html
         */
#ifdef VALGRIND
# define PRECISE_MEMORY_ACCESS 1
#elif defined(__SANITIZE_ADDRESS__) /* GCC's ASAN */
# define PRECISE_MEMORY_ACCESS 1
#elif defined(__has_feature)
#if __has_feature(address_sanitizer) /* Clang's ASAN */
# define PRECISE_MEMORY_ACCESS 1
#endif
#endif
#ifndef PRECISE_MEMORY_ACCESS

		switch(length) {
		case 12: c += k[2]; b += k[1]; a += k[0]; break;
		case 11: c += k[2] & 0xffffffU; b += k[1]; a += k[0]; break;
		case 10: c += k[2] & 0xffffU; b += k[1]; a += k[0]; break;
		case 9 : c += k[2] & 0xffU; b += k[1]; a += k[0]; break;
		case 8 : b += k[1]; a += k[0]; break;
		case 7 : b += k[1] & 0xffffffU; a += k[0]; break;
		case 6 : b += k[1] & 0xffffU; a += k[0]; break;
		case 5 : b += k[1] & 0xffU; a += k[0]; break;
		case 4 : a += k[0]; break;
		case 3 : a += k[0] & 0xffffffU; break;
		case 2 : a += k[0] & 0xffffU; break;
		case 1 : a += k[0] & 0xffU; break;
		case 0 : return c; /* zero length strings require no mixing */
		}

#else /* make valgrind happy */

		const unsigned char* k8 = (const unsigned char *)k;
		switch(length) {
		case 12: c += k[2]; b += k[1]; a += k[0]; break;
		case 11: c += ((unsigned)k8[10]) << 16U;  /* fall through */
		case 10: c += ((unsigned)k8[9]) << 8U;    /* fall through */
		case 9 : c += k8[8];                   /* fall through */
		case 8 : b += k[1]; a += k[0]; break;
		case 7 : b += ((unsigned)k8[6]) << 16U;   /* fall through */
		case 6 : b += ((unsigned)k8[5]) << 8U;    /* fall through */
		case 5 : b += k8[4];                   /* fall through */
		case 4 : a += k[0]; break;
		case 3 : a += ((unsigned)k8[2]) << 16U;   /* fall through */
		case 2 : a += ((unsigned)k8[1]) << 8U;    /* fall through */
		case 1 : a += k8[0]; break;
		case 0 : return c;
		}

#endif /* !valgrind */

	}
	else if (HASH_LITTLE_ENDIAN && ((u.i & 0x1U) == 0)) {
		const unsigned short* k = (const unsigned short *)key; /* read 16-bit chunks */

		/*--------------- all but last block: aligned reads and different mixing */
		while (length > 12) {
			a += k[0] + (((unsigned)k[1]) << 16U);
			b += k[2] + (((unsigned)k[3]) << 16U);
			c += k[4] + (((unsigned)k[5]) << 16U);
			mix(a,b,c);
			length -= 12;
			k += 6;
		}

		/*----------------------------- handle the last (probably partial) block */
        const unsigned char* k8 = (const unsigned char *)k;
		switch(length) {
		case 12:
            c += k[4] + (((unsigned)k[5]) << 16U);
            b += k[2] + (((unsigned)k[3]) << 16U);
            a += k[0] + (((unsigned)k[1]) << 16U);
			break;
		case 11: c += ((unsigned)k8[10]) << 16U;     /* fall through */
		case 10:
            c += k[4];
            b += k[2] + (((unsigned)k[3]) << 16U);
            a += k[0] + (((unsigned)k[1]) << 16U);
			break;
		case 9 : c += k8[8];                      /* fall through */
		case 8 :
            b += k[2] + (((unsigned)k[3]) << 16U);
            a += k[0] + (((unsigned)k[1]) << 16U);
            break;
		case 7 : b += ((unsigned)k8[6]) << 16U;      /* fall through */
		case 6 :
            b += k[2];
            a += k[0] + (((unsigned)k[1]) << 16U);
            break;
        case 5 : b += k8[4];                      /* fall through */
        case 4 : a += k[0] + (((unsigned)k[1]) << 16U);
			break;
        case 3 : a += ((unsigned)k8[2]) << 16U;      /* fall through */
        case 2 :
            a += k[0];
			break;
        case 1 :
            a += k8[0];
            break;
        case 0 : return c;                     /* zero length requires no mixing */
        }

    } else {
        /* need to read the key one byte at a time */
        const unsigned char *k = (const unsigned char *)key;

        /*--------------- all but the last block: affect some 32 bits of (a,b,c) */
		while (length > 12) {
			a += k[0];
			a += ((unsigned)k[1])<<8;
			a += ((unsigned)k[2])<<16;
			a += ((unsigned)k[3])<<24;
			b += k[4];
			b += ((unsigned)k[5])<<8;
			b += ((unsigned)k[6])<<16;
			b += ((unsigned)k[7])<<24;
			c += k[8];
			c += ((unsigned)k[9])<<8;
			c += ((unsigned)k[10])<<16;
			c += ((unsigned)k[11])<<24;
			mix(a,b,c);
			length -= 12;
			k += 12;
		}

		/*-------------------------------- last block: affect all 32 bits of (c) */
		switch(length) { /* all the case statements fall through */
		case 12: c+=((unsigned)k[11])<<24; /* FALLTHRU */
		case 11: c+=((unsigned)k[10])<<16; /* FALLTHRU */
		case 10: c+=((unsigned)k[9])<<8; /* FALLTHRU */
		case 9 : c+=k[8]; /* FALLTHRU */
		case 8 : b+=((unsigned)k[7])<<24; /* FALLTHRU */
		case 7 : b+=((unsigned)k[6])<<16; /* FALLTHRU */
		case 6 : b+=((unsigned)k[5])<<8; /* FALLTHRU */
		case 5 : b+=k[4]; /* FALLTHRU */
		case 4 : a+=((unsigned)k[3])<<24; /* FALLTHRU */
		case 3 : a+=((unsigned)k[2])<<16; /* FALLTHRU */
		case 2 : a+=((unsigned)k[1])<<8; /* FALLTHRU */
		case 1 : a+=k[0];
                 break;
		case 0 : return c;
		}
	}

	final(a,b,c);
	return c;
}
/* clang-format on */

static unsigned long lh_char_hash(const void* k) {
#ifdef _WIN32
    #define RANDOM_SEED_TYPE LONG
#else
    #define RANDOM_SEED_TYPE int
#endif
    static volatile RANDOM_SEED_TYPE random_seed = -1;

    if (random_seed == -1) {
        RANDOM_SEED_TYPE seed = get_random_seed();

#ifdef _WIN32
        InterlockedCompareExchange(&random_seed, seed, -1);
#else
        (void)__sync_val_compare_and_swap(&random_seed, -1, seed);
#endif
    }

    return hashlittle((const char*)k, strlen((const char*)k), (unsigned)random_seed);
}

lh_table* lh_table_new(int size, lh_entry_free_fn* free_fn, lh_hash_fn* hash_fn, lh_equal_fn* equal_fn) {
    lh_table* t = (lh_table*)calloc(1UL, sizeof(lh_table));
    if (t == NULL)
        return NULL;

    t->count = 0;
    t->size = size;
    t->table = (lh_entry*)calloc((size_t)size, sizeof(lh_entry));
    if (t->table == NULL) {
        free(t);
        return NULL;
    }

    t->free_fn = free_fn;
    t->hash_fn = hash_fn;
    t->equal_fn = equal_fn;

    for (int i = 0; i < size; i++)
        t->table[i].k = LH_EMPTY;
    return t;
}

inline lh_table* lh_kchar_table_new(int size,
                                    lh_entry_free_fn* free_fn) {
    return lh_table_new(size, free_fn, char_hash_fn, lh_char_equal);
}

inline lh_table* lh_kptr_table_new(int size, lh_entry_free_fn* free_fn) {
    return lh_table_new(size, free_fn, lh_ptr_hash, lh_ptr_equal);
}

int lh_table_resize(lh_table* t, int new_size) {
    lh_table* new_t = lh_table_new(new_size, NULL, t->hash_fn, t->equal_fn);
    if (new_t == NULL)
        return -1;

    lh_entry* ent = t->head;
    for (; ent != NULL; ent = ent->next) {
        unsigned long h = lh_get_hash(new_t, ent->k);
        unsigned opts = 0U;
        if (ent->k_is_constant)
            opts = JSON_C_OBJECT_KEY_IS_CONSTANT;

        if (lh_table_insert_w_hash(new_t, ent->k, ent->v, h, opts) != 0) {
            lh_table_free(new_t);
            return -1;
        }
    }
    free(t->table);
    t->table = new_t->table;
    t->size = new_size;
    t->head = new_t->head;
    t->tail = new_t->tail;
    free(new_t);

    return 0;
}

inline void lh_table_free(lh_table* t) {
    if (t->free_fn) {
        register lh_entry* c = t->head;
        for (; c != NULL; c = c->next)
            t->free_fn(c);
    }
    free(t->table);
    free(t);
}

int lh_table_insert_w_hash(lh_table* t, const void* k, const void* v, const unsigned long h, const unsigned opts) {
    if (t->count >= t->size * LH_LOAD_FACTOR) {
        /* Avoid signed integer overflow with large tables. */
        int new_size = (t->size > INT_MAX / 2) ? INT_MAX : (t->size * 2);
        if (t->size == INT_MAX || lh_table_resize(t, new_size) != 0)
            return -1;
    }
    size_t n = h % (size_t)t->size;

    while (1) {
        if (t->table[n].k == LH_EMPTY || t->table[n].k == LH_FREED)
            break;
        if ((int)++n == t->size)
            n = 0;
    }

    t->table[n].k = k;
    t->table[n].k_is_constant = (opts & JSON_C_OBJECT_KEY_IS_CONSTANT);
    t->table[n].v = v;
    t->count++;

    if (t->head == NULL) {
        t->head = t->tail = &t->table[n];
        t->table[n].next = t->table[n].prev = NULL;
    } else {
        t->tail->next = &t->table[n];
        t->table[n].prev = t->tail;
        t->table[n].next = NULL;
        t->tail = &t->table[n];
    }

    return 0;
}
inline int lh_table_insert(lh_table* t, const void* k, const void* v) {
    return lh_table_insert_w_hash(t, k, v, lh_get_hash(t, k), 0);
}

inline lh_entry* lh_table_lookup_entry_w_hash(lh_table* t, const void* k, const unsigned long h) {
    size_t n = h % (size_t)t->size;

    register int count = 0;
    while (count < t->size) {
        if (t->table[n].k == LH_EMPTY)
            return NULL;
        if (t->table[n].k != LH_FREED && t->equal_fn(t->table[n].k, k))
            return &t->table[n];
        if ((int)++n == t->size)
            n = 0;
        count++;
    }
    return NULL;
}

inline lh_entry* lh_table_lookup_entry(lh_table* t, const void* k) {
    return lh_table_lookup_entry_w_hash(t, k, lh_get_hash(t, k));
}

inline unsigned char lh_table_lookup_ex(lh_table* t, const void* k, void** v) {
    lh_entry* e = lh_table_lookup_entry(t, k);
    if (e != NULL) {
        if (v != NULL)
            *v = lh_entry_v(e);
        return 1; /* key found */
    }
    if (v != NULL)
        *v = NULL;
    return 0; /* key not found */
}

int lh_table_delete_entry(lh_table* t, lh_entry* e) {
    /* CAW: fixed to be 64bit nice, still need the crazy negative case... */
    ptrdiff_t n = (ptrdiff_t)(e - t->table);

    /* CAW: this is bad, really bad, maybe stack goes other direction on this
   * machine... */
    if (n < 0) {
        return -2;
    }

    if (t->table[n].k == LH_EMPTY || t->table[n].k == LH_FREED)
        return -1;
    t->count--;
    if (t->free_fn)
        t->free_fn(e);
    t->table[n].v = NULL;
    t->table[n].k = LH_FREED;
    if (t->tail == &t->table[n] && t->head == &t->table[n]) {
        t->head = t->tail = NULL;
    } else if (t->head == &t->table[n]) {
        t->head->next->prev = NULL;
        t->head = t->head->next;
    } else if (t->tail == &t->table[n]) {
        t->tail->prev->next = NULL;
        t->tail = t->tail->prev;
    } else {
        t->table[n].prev->next = t->table[n].next;
        t->table[n].next->prev = t->table[n].prev;
    }
    t->table[n].next = t->table[n].prev = NULL;
    return 0;
}

inline int lh_table_delete(lh_table* t, const void* k) {
    register int result = -1;
    lh_entry* e = lh_table_lookup_entry(t, k);
    if (e != NULL)
        result = lh_table_delete_entry(t, e);

    return result;
}

inline int lh_table_length(lh_table* t) {
    return t->count;
}
/*
 * $Id: printbuf.c,v 1.5 2006/01/26 02:16:28 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 *
 * Copyright (c) 2008-2009 Yahoo! Inc.  All rights reserved.
 * The copyrights to the contents of this file are licensed under the MIT License
 * (http://www.opensource.org/licenses/mit-license.php)
 */
// #include <vnepogodin/third_party/json-c/printbuf.h>


#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _Printbuf {
    int bpos;
    int size;

    char* buf;
};

void printbuf_memcpy(printbuf* __pb_param, const void* __src_param, const unsigned long __size_param) {
    memcpy(__pb_param->buf + __pb_param->bpos, __src_param, __size_param);
}

inline int printbuf_getSize(printbuf* __pb_param) {
    return __pb_param->size;
}
inline int printbuf_getPos(printbuf* __pb_param) {
    return __pb_param->bpos;
}
inline char* printbuf_getBuf(printbuf* __pb_param) {
    return __pb_param->buf;
}
inline char* printbuf_get_posBufInPos(printbuf* __pb_param, const int __pos_param) {
    return &__pb_param->buf[__pos_param];
}

inline void printbuf_addPos(printbuf* __pb_param, const int __value_param) {
    __pb_param->bpos += __value_param;
}
inline void printbuf_subPos(printbuf* __pb_param, const int __value_param) {
    __pb_param->bpos -= __value_param;
}

inline void printbuf_setBufInPos(printbuf* __pb_param, const int __pos_param, const char __value_param) {
    __pb_param->buf[__pos_param] = __value_param;
}

/**
 * Extend the buffer p so it has a size of at least min_size.
 *
 * If the current size is large enough, nothing is changed.
 *
 * Note: this does not check the available space!  The caller
 *  is responsible for performing those calculations.
 */
static int printbuf_extend(printbuf* p, const int min_size) {
    if (p->size >= min_size)
        return 0;

    /* Prevent signed integer overflows with large buffers. */
    if (min_size > INT_MAX - 8)
        return -1;

    register int new_size = 0;

    if (p->size > (INT_MAX / 2))
        new_size = min_size + 8;
    else {
        new_size = p->size * 2;
        if (new_size < min_size + 8)
            new_size = min_size + 8;
    }

    const char* t = (char*)realloc(p->buf, (unsigned long)new_size);
    if (t == NULL)
        return -1;

    p->size = new_size;
    p->buf = (char*)t;
    return 0;
}

inline printbuf* printbuf_new(void) {
    printbuf* p = (printbuf*)calloc(1UL, sizeof(printbuf));
    if (p == NULL)
        return NULL;

    p->size = 32;
    p->bpos = 0;
    p->buf = (char*)malloc((unsigned long)p->size);
    if (p->buf == NULL) {
        free(p);
        return NULL;
    }

    p->buf[0] = '\0';
    return p;
}

inline int printbuf_memappend(printbuf* p, const char* buf, const unsigned long size) {
    /* Prevent signed integer overflows with large buffers. */
    if ((int)size > (INT_MAX - p->bpos - 1))
        return -1;

    if (p->size <= (p->bpos + (int)size + 1)) {
        if (printbuf_extend(p, p->bpos + (int)size + 1) < 0)
            return -1;
    }

    memcpy(p->buf + p->bpos, buf, size);
    p->bpos += (int)size;
    p->buf[p->bpos] = '\0';
    return (int)size;
}

int printbuf_memset(printbuf* pb, int offset, const int value, const int len) {
    if (offset == -1)
        offset = pb->bpos;

    /* Prevent signed integer overflows with large buffers. */
    if (len > (INT_MAX - offset))
        return -1;

    register const int size_needed = offset + len;
    if (pb->size < size_needed) {
        if (printbuf_extend(pb, size_needed) < 0)
            return -1;
    }

    memset(pb->buf + offset, value, (unsigned long)len);
    if (pb->bpos < size_needed)
        pb->bpos = size_needed;

    return 0;
}

inline void printbuf_reset(printbuf* p) {
    p->buf[0] = '\0';
    p->bpos = 0;
}

inline void printbuf_free(printbuf* p) {
    if (p != NULL) {
        free(p->buf);
        free(p);
    }
}
/* Matrix lib */

// #include <vnepogodin/matrix.h>
/* Matrix lib */
#ifndef __MATRIX_H__
#define __MATRIX_H__

// #include <vnepogodin/third_party/json-c/json.h>
#ifndef __JSON_H__
#define __JSON_H__

// #include <vnepogodin/third_party/json-c/json_object.h>

// #include <vnepogodin/third_party/json-c/json_tokener.h>


#endif /* __JSON_H__ */
 /* json_object */

typedef struct _Matrix Matrix;

#ifdef __cplusplus
extern "C" {
#endif

/* Constructors */

Matrix* matrix_new_with_args(const int, const int);
Matrix* matrix_new_with_matrix(const Matrix *const);

/* Destructor */

void matrix_free(register Matrix *);

/* Operators */

void matrix_add_matrix(register Matrix *, const Matrix *const);
void matrix_add_float(register Matrix *, const float);
void matrix_multiply(register Matrix *, const Matrix *const);
void matrix_multiply_scalar(register Matrix *, const float);

/* Functions */

float* matrix_toArray(const Matrix *const);
void matrix_randomize(register Matrix *);
void matrix_map(register Matrix *, float (*const)(float));
void matrix_print(const Matrix *const);
json_object* matrix_serialize(const Matrix *const);

/* Static functions */

Matrix* matrix_fromArray(const float* __restrict const, const int);
Matrix* matrix_transpose_static(const Matrix *const);
Matrix* matrix_multiply_static(const Matrix *__restrict const, const Matrix *__restrict const);
Matrix* matrix_subtract_static(const Matrix *const, const Matrix *const);
Matrix* matrix_map_static(const Matrix *const, float (*const)(float));
Matrix* matrix_deserialize(const json_object *__restrict const);

#ifdef __cplusplus
}
#endif

#endif /* __MATRIX_H__ */


#include <stdio.h>  /* printf */
#include <stdlib.h> /* malloc, posix_memalign, arc4random */
#include <string.h> /* strtof, strtok_r, strtok_s */
#ifdef __linux__
# include <fcntl.h>  /* openat, O_RDONLY */
# include <unistd.h> /* pread, close */
#elif _WIN32
# include <windows.h>
# include <bcrypt.h> /* BCryptGenRandom */
# pragma comment(lib, "bcrypt.lib")
#endif

struct _Matrix {
    /* Variables */
    int len;

    int rows;
    int columns;

    float* data;
};

/**
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define STMT_START do

/**
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define STMT_END while (0)

/**
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define PTR_START(end)  \
    register int i = 0; \
    while (i < (end)) {
/**
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define PTR_END \
    ++ptr;      \
    ++i;        \
    }

#ifdef _WIN32
# define posix_memalign(p, a, s) (((*(p)) = _aligned_malloc((s), (a))), *(p) ? 0 : errno)
# define posix_memalign_free _aligned_free
#else
# define posix_memalign_free free
#endif /* _WIN32 */

/* Non member functions */
#define length_str(string, size) STMT_START{ \
    register const char* buf_str = (string); \
    while (*buf_str != '\0') {               \
        ++(size);                            \
        ++buf_str;                           \
    }                                        \
}STMT_END

#define allocSpace(data_base, r, cols) STMT_START{                                            \
    (data_base) = (float*)malloc((unsigned long)(r) * (unsigned long)(cols) * sizeof(float)); \
}STMT_END

static void json_strsplit(register float* result, const char* _str, const int columns) {
    const char delim[2] = {","};

    register int size = 0;

    length_str(_str, size);

    register char* tmp = (char*)malloc((unsigned long)size - 1UL);

    /*   slice_str   */
    register int i = 2;
    register int j = 0;

    size -= 2;
    while (i < size) {
        tmp[j] = _str[i];

        ++j;
        ++i;
    }

    char* save_token = NULL;
#ifdef _WIN32
    register char* token = strtok_s(tmp, delim, &save_token);
#else
    register char* token = strtok_r(tmp, delim, &save_token);
#endif /* _WIN32 */

    i = 0;
    while (i < columns) {
        result[i] = strtof(token, NULL);

#ifdef _WIN32
        token = strtok_s(NULL, delim, &save_token);
#else
        token = strtok_r(NULL, delim, &save_token);
#endif /* _WIN32 */

        ++i;
    }

    free(tmp);
}

/**
 * Creates a new #Matrix with an all data = 0.
 * @param rows The rows of matrix.
 * @param columns The columns of matrix.
 * @returns The new #Matrix
 * @example
 *        2 rows, 3 columns
 *
 *           [0][0][0]
 *           [0][0][0]
 *
 */
Matrix* matrix_new_with_args(const int rows, const int columns) {
    Matrix* __matrix_m = NULL;

    /* assumed 0.001MB page sizes */
    posix_memalign((void**)&__matrix_m, 1024UL, 1024UL);

    if (__matrix_m != NULL) {
        __matrix_m->rows = rows;
        __matrix_m->columns = columns;

        allocSpace(__matrix_m->data, rows, columns);

        register float* ptr = &__matrix_m->data[0];

        register int end = rows * columns;
        PTR_START(end)
            *ptr = 0.F;
        PTR_END

        __matrix_m->len = i;
    }

    return __matrix_m;
}

/**
 * Creates a new #Matrix with reference rows and columns,
 * and with a reference data.
 * @param matrix The const #Matrix.
 * @returns The new #Matrix
 * @example
 *        2 rows, 1 columns  ->  2 rows, 1 columns
 *
 *            [232]          ->        [232]
 *            [21]           ->        [21]
 *
 */
Matrix* matrix_new_with_matrix(const Matrix* const __matrix_param) {
    Matrix* __matrix_m = matrix_new_with_args(__matrix_param->rows,
                                              __matrix_param->columns);

    if (__matrix_m != NULL) {
        register float* ptr = &__matrix_m->data[0];
        register const float* ref_ptr = &__matrix_param->data[0];

        PTR_START(__matrix_m->len)
            *ptr = *ref_ptr;

            ++ref_ptr;
        PTR_END
    }

    return __matrix_m;
}

/**
 * Frees #Matrix.
 * @param matrix The #Matrix.
 *
 */
void matrix_free(register Matrix* __matrix_param) {
    free(__matrix_param->data);
    __matrix_param->data = NULL;

    posix_memalign_free(__matrix_param);
}

/**
 * Add `num` to `matrix`->data.
 * @param matrix The #Matrix.
 * @param num The reference const float number.
 * @example
 *	2 rows, 1 columns
 *
 *		[321]	+		3.3
 *		[74]	+	    3.3
 *
 */
void matrix_add_float(register Matrix* a_param, const float num_param) {
    register float* ptr = &a_param->data[0];

    PTR_START(a_param->len)
        *ptr += num_param;
    PTR_END
}

/**
 * Add `b`->data to `a`->data.
 * @param a The #Matrix.
 * @param b The reference #Matrix.
 * @example
 *  2 rows, 1 columns   2 rows, 1 columns
 *
 *      [321]        +       [0.1]
 *      [74]         +       [0.78]
 *
 */
void matrix_add_matrix(register Matrix* a_param, const Matrix* const b_param) {
    if ((a_param->rows == b_param->rows) || (a_param->columns == b_param->columns)) {
        register float* ptr = &a_param->data[0];
        register const float* ref_ptr = &b_param->data[0];

        PTR_START(a_param->len)
            *ptr += *ref_ptr;

            ++ref_ptr;
        PTR_END
    } else
        printf("Columns and Rows of A must match Columns and Rows of B.\n");
}

/**
 * Hadamard product.
 * @param a The #Matrix.
 * @param b The reference #Matrix.
 * @see https://en.wikipedia.org/wiki/Hadamard_product_(matrices)
 * @example:
 *        3 rows, 1 columns       2 rows, 1 columns
 *
 *             [64]
 *             [87]          *         [232]
 *             [21]                    [21]
 *
 */
void matrix_multiply(register Matrix* a_param, const Matrix* const b_param) {
    if ((a_param->rows == b_param->rows) || (a_param->columns == b_param->columns)) {
        register float* ptr = &a_param->data[0];
        register const float* b_ptr = &b_param->data[0];

        PTR_START(a_param->len)
            *ptr *= *b_ptr;

            ++b_ptr;
        PTR_END
    } else
        printf("Columns and Rows of A must match Columns and Rows of B.\n");
}

/**
 * Add number to `matrix`->data.
 * @param matrix The #Matrix.
 * @param num The reference floating-point number.
 * @example
 *	      2 rows, 1 columns
 *
 *             [321]         *        3.3
 *             [74]          *        3.3
 *
 */
void matrix_multiply_scalar(register Matrix* m_param, const float num_param) {
    /* Scalar product */
    register float* ptr = &m_param->data[0];

    PTR_START(m_param->len)
        *ptr *= num_param;
    PTR_END
}

/**
 * Create array by #Matrix.
 * @param matrix The reference #Matrix.
 * @returns The new const float array
 * @example
 *        2 rows, 1 columns
 *
 *             [321]      ->      [321]
 *             [74]       ->      [74]
 *
 */
float* matrix_toArray(const Matrix* const m_param) {
    float* arr = (float*)malloc((unsigned long)m_param->len * sizeof(float));

    /* pointer to Matrix.data in CPU register */
    register const float* ptr = &m_param->data[0];

    PTR_START(m_param->len)
        arr[i] = *ptr;
    PTR_END

    return arr;
}

/**
 * Randomize `matrix`->data (from 0 to 2) - 1.
 * @param matrix The #Matrix.
 * @example
 *	2 rows, 1 columns   2 rows, 1 columns
 *
 *       [321]       ->      [0.1]
 *       [74]        ->      [0.78]
 *
 */
void matrix_randomize(register Matrix* m_param) {
    register float* ptr = &m_param->data[0];

#ifdef __linux__
    register int fd = openat(0, "/dev/urandom", O_RDONLY, 0);
    unsigned char buf[1] = {0U};

    if (fd != -1) {
        pread(fd, buf, 1, 0);
        close(fd);
    }

    unsigned __random = buf[0];

    PTR_START(m_param->len)
        *ptr = ((float)(rand_r(&__random) * 2.F / (float)RAND_MAX)) - 1.F;
#elif _WIN32
    UINT __random = 0U;

    BCryptGenRandom(NULL, (BYTE*)&__random, sizeof(UINT), BCRYPT_USE_SYSTEM_PREFERRED_RNG);

    PTR_START(m_param->len)
        *ptr = ((float)(__random * 2.F / UINT_MAX)) - 1.F;
#else
    unsigned __random = arc4random();

    PTR_START(m_param->len)
        *ptr = ((float)rand_r(&__random) * 2.F / (float)RAND_MAX) - 1.F;
#endif

    PTR_END
}

/**
 * Data of `matrix` equal, return value of `func`.
 * @param matrix The #Matrix.
 * @param func The float function.
 * @code
 * float func(float num) {
 *	return num * 2;
 * }
 * @endcode
 *
 */
void matrix_map(register Matrix* m_param, float (*const func_param)(float)) {
    register float* ptr = &m_param->data[0];

    PTR_START(m_param->len)
        *ptr = (*func_param)(*ptr);
    PTR_END
}

/**
 * Print elements of `matrix`->data.
 * @param matrix The const #Matrix.
 *
 */
void matrix_print(const Matrix* const m_param) {
    register const float* ptr = &m_param->data[0];

    register int counter = 0;
    register int i = 0;
    while (i < m_param->len) {
        printf("%f ", (double)*ptr);

        ++ptr;
        ++counter;
        if (counter == m_param->columns) {
            counter = 0;

            if ((i + 1) != m_param->len)
                printf("\n");
        }
        ++i;
    }
}

/**
 * Serialize `matrix` to JSON.
 * @param matrix The const #Matrix.
 * @returns The new #json_object
 *
 */
json_object* matrix_serialize(const Matrix* const m_param) {
    register json_object* t = json_object_new_object();

    json_object_object_add_ex(t, "rows", json_object_new_int(m_param->rows), 0);
    json_object_object_add_ex(t, "columns", json_object_new_int(m_param->columns), 0);
    json_object_object_add_ex(t, "data", json_object_new_array_ext(m_param->rows), 0);

    register json_object* temp_arr = json_object_new_array_ext(m_param->columns);

    register const float* ptr = &m_param->data[0];

    register int counter = 0;
    register int i = 0;
    while (i < m_param->rows) {
        json_object_array_add(temp_arr, json_object_new_double((double)*ptr));

        ++ptr;
        ++counter;
        if (counter == m_param->columns) {
            json_object_array_add(json_find(t, "data"), temp_arr);
            temp_arr = json_object_new_array_ext(m_param->columns);

            counter = 0;
            ++i;
        }
    }

    return t;
}

/**
 * Create #Matrix by `arr`.
 * @param arr The reference float array.
 * @returns The new #Matrix
 * @example
 *        2 rows, 1 columns
 *
 *             [321]       ->    [321]
 *             [74]        ->    [74]
 *
 */
Matrix* matrix_fromArray(const float* __restrict const arr_param, const int len_param) {
    register Matrix* t = matrix_new_with_args(len_param, 1);

    register float* ptr = &t->data[0];

    PTR_START(len_param)
        *ptr = arr_param[i];
    PTR_END

    return t;
}

/**
 * Transpositing `matrix`->data.
 * @param matrix The const #Matrix.
 * @see https://en.wikipedia.org/wiki/Transpose
 * @returns The new #Matrix
 *
 */
Matrix* matrix_transpose_static(const Matrix* const m_param) {
    register Matrix* t = matrix_new_with_args(m_param->columns, m_param->rows);

    register float* ptr = &t->data[0];
    register const float* m_ptr = &m_param->data[0];

    register unsigned counter = 0U;
    PTR_START(t->rows)
        register int j = 0;
        while (j < t->columns) {
            ptr[counter] = m_ptr[(j * t->rows) + i];

            ++counter;
            ++j;
        }
        ++i;
    }

    return t;
}

/**
 * Add to new #Matrix, multiply of `a`->data and `b`->data.
 * @param a The const #Matrix.
 * @param b The const #Matrix.
 * @returns The new #Matrix
 *
 */
Matrix* matrix_multiply_static(const Matrix* __restrict const a_param, const Matrix* __restrict const b_param) {
    Matrix* result = NULL;

    /* Matrix product */
    if (a_param->columns != b_param->rows) {
        printf("Columns of A must match rows of B.\n");
    } else {
        register Matrix* t = matrix_new_with_args(a_param->rows, b_param->columns);

        register float* ptr = &t->data[0];
        register const float* a_ptr = &a_param->data[0];
        register const float* b_ptr = &b_param->data[0];

        register unsigned counter = 0U;

        PTR_START(t->rows)
            register int j = 0;
            while (j < t->columns) {
                register int k = 0;
                register float sum = 0.F;
                while (k < a_param->columns) {
                    sum += a_ptr[(i * a_param->columns) + k] * b_ptr[(k * t->rows) + j];

                    ++k;
                }
                ptr[counter] = sum;

                ++counter;
                ++j;
            }
            ++i;
        }

        result = t;
    }

    return result;
}

/**
 * Subtract `a`->data and `b`->data.
 * @param a The const #Matrix.
 * @param b The const #Matrix.
 * @returns The new #Matrix
 * @example
 *        2 rows, 1 columns   2 rows, 1 columns
 *
 *             [321]        -      [3.3]
 *             [74]         -      [3.3]
 *
 *                         | |
 *                         \ /
 *
 *                        [317.7]
 *                        [70.7]
 *
 */
Matrix* matrix_subtract_static(const Matrix* const a_param, const Matrix* const b_param) {
    Matrix* result = NULL;

    if ((a_param->rows != b_param->rows) || (a_param->columns != b_param->columns)) {
        printf("Columns and Rows of A must match Columns and Rows of B.\n");
    } else {
        register Matrix* t = matrix_new_with_args(a_param->rows, b_param->columns);

        register float* ptr = &t->data[0];
        register const float* a_ptr = &a_param->data[0];
        register const float* b_ptr = &b_param->data[0];

        PTR_START(t->len)
            *ptr = *a_ptr - *b_ptr;

            ++a_ptr;
            ++b_ptr;
        PTR_END

        result = t;
    }

    return result;
}

/**
 * Apply `func` to every element of `matrix`.
 * @param matrix The const #Matrix.
 * @param func The float function.
 * @returns The new #Matrix
 *
 */
Matrix* matrix_map_static(const Matrix* const m_param, float (*const func_param)(float)) {
    register Matrix* t = matrix_new_with_matrix(m_param);

    register float* ptr = &t->data[0];

    PTR_START(t->len)
        *ptr = (*func_param)(*ptr);
    PTR_END

    return t;
}

/**
 * Create new #Matrix by json file or string.
 * @param json The const #json_object.
 * @returns The new #Matrix
 * @example
 *
 * {"rows":10,"columns":4,"data":[[0.19060910095479766,-0.7044789872907934,0.12916417175926737,-0.4493542740188917],[-0.8856888126717002,-0.9031315595837806,-0.25989472073659314,0.5503610418820337],[0.015814800116810357,0.20458699223751559,0.4144319562653632,-0.44217425771287694],[-0.6479931730467494,1.0466013336675943,0.34151503310804115,-0.9019454688191391],[0.6360219561553282,-0.7970648255401476,0.6967338500765156,1.026139659585225],[0.7070906013077707,-0.029736810492719402,0.4291704886090242,-0.5162258314269367],[0.5127199375296518,0.9105995373130602,-0.016193872134645272,-0.6064552608044859],[-0.37527682605966417,0.6967128785525135,-0.19384958454126475,0.7929244831790743],[0.12271433171587676,-0.42020640380104357,0.3119476057291891,-0.29433625893968235],[0.974302294354497,0.5381564104252675,0.7234688874658266,0.6823117502912057]]}
 * or
 * cities.json
 *
 */
Matrix* matrix_deserialize(const json_object* __restrict const t_param) {
    register Matrix* __matrix_m = matrix_new_with_args(json_object_get_int(json_find(t_param, "rows")),
                                                       json_object_get_int(json_find(t_param, "columns")));

    register float* ptr = &__matrix_m->data[0];

    register float* buf = (float*)malloc((unsigned long)__matrix_m->columns);
    json_strsplit(buf, json_object_to_json_string_ext(json_object_array_get_idx(json_find(t_param, "data"), 0), JSON_C_TO_STRING_SPACED), __matrix_m->columns);

    register int i = 0;
    register int counter = 0;
    while (i < __matrix_m->rows) {
        *ptr = buf[counter];
        ++ptr;
        counter++;

        if (counter == __matrix_m->columns) {
            counter = 0;
            ++i;

            if (i != __matrix_m->rows)
                json_strsplit(buf, json_object_to_json_string_ext(json_object_array_get_idx(json_find(t_param, "data"), (unsigned long)i), JSON_C_TO_STRING_SPACED), __matrix_m->columns);
        }
    }

    free(buf);

    return __matrix_m;
}
/* Other techniques for learning */

// #include <vnepogodin/nn.h>
/* Other techniques for learning */
#ifndef __NN_H__
#define __NN_H__

// #include <vnepogodin/matrix.h>
  /* Matrix */

typedef struct _NeuralNetwork NeuralNetwork;

/**
 * Flag to sigmoid float function.
 * @see nn.c 21-23.
 *
 */
#define FUNC_SIGMOID 0x01U

/**
 * Flag to dsigmoid float function.
 * @see nn.c 25-28.
 *
 */
#define FUNC_DSIGMOID 0x02U

#ifdef __cplusplus
extern "C" {
#endif

/* Constructors */

NeuralNetwork* neural_network_new_with_nn(const NeuralNetwork *const);
NeuralNetwork* neural_network_new_with_args(const int, const int, const int);

/* Destructor */

void neural_network_free(register NeuralNetwork *__restrict);

/* Functions */

float* neural_network_predict(const NeuralNetwork *const, const float* __restrict const);
void neural_network_train(register NeuralNetwork *, const float* __restrict const, const float* __restrict const);
void neural_network_setLearningRate(register NeuralNetwork *__restrict, const float);
void neural_network_setActivationFunction(register NeuralNetwork *__restrict, const unsigned char);
json_object* neural_network_serialize(const NeuralNetwork *__restrict const);

/* Static functions */

NeuralNetwork* neural_network_deserialize(const json_object *__restrict const);

#ifdef __cplusplus
}
#endif

#endif /* __NN_H__ */


#include <math.h>   /* expf */
#include <stdlib.h> /* malloc */

struct _NeuralNetwork {
    /* Variables */
    int input_nodes;
    int hidden_nodes;
    int output_nodes;

    float learning_rate;

    float (*activation_function)(float);

    Matrix* weights_ih;
    Matrix* weights_ho;
    Matrix* bias_h;
    Matrix* bias_o;
};

/* Non member functions */
static inline float sigmoid(float x) {
    return 1.F / (1.F + expf(-x));
}

static inline float dsigmoid(float y) {
    /* return sigmoid(x) * (1 - sigmoid(x)); */
    return y * (1.F - y);
}

#define convert_ActivationFunction(func) \
    (*(func) == dsigmoid) ? 2 : 1

/**
 * Creates new #NeuralNetwork with data of `nn`.
 * @param nn The reference #NeuralNetwork.
 * @returns The new #NeuralNetwork
 *
 */
NeuralNetwork* neural_network_new_with_nn(const NeuralNetwork* const __nn_param) {
    register NeuralNetwork* __nn_temp = (NeuralNetwork*)malloc(56UL);

    __nn_temp->input_nodes = __nn_param->input_nodes;
    __nn_temp->hidden_nodes = __nn_param->hidden_nodes;
    __nn_temp->output_nodes = __nn_param->output_nodes;

    __nn_temp->weights_ih = matrix_new_with_matrix(__nn_param->weights_ih);
    __nn_temp->weights_ho = matrix_new_with_matrix(__nn_param->weights_ho);
    __nn_temp->bias_h = matrix_new_with_matrix(__nn_param->bias_h);
    __nn_temp->bias_o = matrix_new_with_matrix(__nn_param->bias_o);

    /* TODO: copy these as well */
    neural_network_setLearningRate(__nn_temp, __nn_param->learning_rate);

    register const unsigned char flag = convert_ActivationFunction(__nn_param->activation_function);
    neural_network_setActivationFunction(__nn_temp, flag);

    return __nn_temp;
}

/**
 * Creates a new #NeuralNetwork with random data.
 * @param input_nodes The input.
 * @param hidden_nodes The hidden.
 * @param output_nodes The output.
 * @returns The new #NeuralNetwork
 *
 */
NeuralNetwork* neural_network_new_with_args(const int input_nodes,
                                            const int hidden_nodes,
                                            const int output_nodes) {
    register NeuralNetwork* nn = (NeuralNetwork*)malloc(56UL);

    nn->input_nodes = input_nodes;
    nn->hidden_nodes = hidden_nodes;
    nn->output_nodes = output_nodes;

    nn->weights_ih = matrix_new_with_args(hidden_nodes, input_nodes);
    nn->weights_ho = matrix_new_with_args(output_nodes, hidden_nodes);
    matrix_randomize(nn->weights_ih);
    matrix_randomize(nn->weights_ho);

    nn->bias_h = matrix_new_with_args(hidden_nodes, 1);
    nn->bias_o = matrix_new_with_args(output_nodes, 1);
    matrix_randomize(nn->bias_h);
    matrix_randomize(nn->bias_o);

    /* TODO: copy these as well */
    neural_network_setLearningRate(nn, 0.1F);
    neural_network_setActivationFunction(nn, FUNC_SIGMOID);

    return nn;
}

/**
 * Frees #NeuralNetwork.
 * @param nn The #NeuralNetwork.
 *
 */
void neural_network_free(register NeuralNetwork* __restrict __nn_param) {
    __nn_param->activation_function = NULL;

    matrix_free(__nn_param->weights_ih);
    matrix_free(__nn_param->weights_ho);
    matrix_free(__nn_param->bias_h);
    matrix_free(__nn_param->bias_o);

    free(__nn_param);
}

/**
 * Output of #NeuralNetwork.
 * @param nn The reference #NeuralNetwork.
 * @param input_array The input data.
 * @returns float array
 *
 */
float* neural_network_predict(const NeuralNetwork* const nn,
                              const float* __restrict const input_array) {
    /* Generating the Hidden Outputs */
    register Matrix* input = matrix_fromArray(input_array, nn->input_nodes);
    register Matrix* hidden = matrix_multiply_static(nn->weights_ih, input);
    matrix_free(input);
    matrix_add_matrix(hidden, nn->bias_h);

    /* Activation function! */
    matrix_map(hidden, nn->activation_function);

    /* Generating the output's output! */
    register Matrix* output = matrix_multiply_static(nn->weights_ho, hidden);
    matrix_free(hidden);
    matrix_add_matrix(output, nn->bias_o);
    matrix_map(output, nn->activation_function);

    /* Sending back to the caller! */
    float* result = matrix_toArray(output);
    matrix_free(output);

    return result;
}

/**
 * Setting learning rate.
 * @param nn The #NeuralNetwork.
 * @param lr The learning rate of neural network.
 *
 */
void neural_network_setLearningRate(register NeuralNetwork* __restrict nn,
                                    const float lr) {
    nn->learning_rate = lr;
}

/**
 * Setting function.
 * @param nn The #NeuralNetwork.
 * @param flag The some function.
 *
 */
void neural_network_setActivationFunction(register NeuralNetwork* __restrict nn,
                                          const unsigned char flag) {
    nn->activation_function = NULL;

    if (flag == FUNC_SIGMOID)
        nn->activation_function = sigmoid;
    else if (flag == FUNC_DSIGMOID)
        nn->activation_function = dsigmoid;
}

/**
 * Trains neural network.
 * @param nn The #NeuralNetwork.
 * @param input_array The input data.
 * @param target_array The output data.
 *
 */
void neural_network_train(register NeuralNetwork* nn,
                          const float* __restrict const input_array,
                          const float* __restrict const target_array) {
    /* Generating the Hidden Outputs */
    register Matrix* inputs = matrix_fromArray(input_array, nn->input_nodes);
    register Matrix* hidden = matrix_multiply_static(nn->weights_ih, inputs);
    matrix_free(inputs);
    matrix_add_matrix(hidden, nn->bias_h);
    /*  Activation function!  */
    matrix_map(hidden, sigmoid);

    /* Generating the output's output! */
    register Matrix* outputs = matrix_multiply_static(nn->weights_ho, hidden);
    matrix_add_matrix(outputs, nn->bias_o);
    matrix_map(outputs, sigmoid);

    /* Convert array to matrix object */
    register Matrix* targets = matrix_fromArray(target_array, nn->output_nodes);

    /* Calculate the error
     * ERROR = TARGETS - OUTPUTS */
    register Matrix* output_errors = matrix_subtract_static(targets, outputs);
    matrix_free(targets);

    /* let gradient = outputs * (1 - outputs);
     * Calculate gradient */
    register Matrix* gradients = matrix_map_static(outputs, dsigmoid);
    matrix_free(outputs);
    matrix_multiply(gradients, output_errors);
    matrix_multiply_scalar(gradients, nn->learning_rate);

    /* Calculate deltas */
    register Matrix* hidden_T = matrix_transpose_static(hidden);
    register Matrix* weight_ho_deltas = matrix_multiply_static(gradients, hidden_T);
    matrix_free(hidden_T);

    /* Adjust the weights by deltas */
    matrix_add_matrix(nn->weights_ho, weight_ho_deltas);
    matrix_free(weight_ho_deltas);

    /* Adjust the bias by its deltas (which is just the gradients) */
    matrix_add_matrix(nn->bias_o, gradients);
    matrix_free(gradients);

    /* Calculate the hidden layer errors */
    register Matrix* who_t = matrix_transpose_static(nn->weights_ho);
    register Matrix* hidden_errors = matrix_multiply_static(who_t, output_errors);
    matrix_free(output_errors);
    matrix_free(who_t);

    /* Calculate hidden gradient */
    register Matrix* hidden_gradient = matrix_map_static(hidden, dsigmoid);
    matrix_free(hidden);
    matrix_multiply(hidden_gradient, hidden_errors);
    matrix_free(hidden_errors);
    matrix_multiply_scalar(hidden_gradient, nn->learning_rate);

    /* Calcuate input->hidden deltas */
    inputs = matrix_fromArray(input_array, nn->input_nodes);
    register Matrix* inputs_T = matrix_transpose_static(inputs);
    register Matrix* weight_ih_deltas = matrix_multiply_static(hidden_gradient, inputs_T);
    matrix_free(inputs);
    matrix_free(inputs_T);

    matrix_add_matrix(nn->weights_ih, weight_ih_deltas);
    matrix_free(weight_ih_deltas);
    /* Adjust the bias by its deltas (which is just the gradients) */
    matrix_add_matrix(nn->bias_h, hidden_gradient);
    matrix_free(hidden_gradient);
}

/**
 * Serialize #Matrix to JSON.
 * @param nn The reference #NeuralNetwork.
 * @returns The new #json_object
 *
 */
json_object* neural_network_serialize(const NeuralNetwork* __restrict const __nn_param) {
    register json_object* t = json_object_new_object();

    json_object_object_add_ex(t, "input_nodes", json_object_new_int(__nn_param->input_nodes), 0U);
    json_object_object_add_ex(t, "hidden_nodes", json_object_new_int(__nn_param->hidden_nodes), 0U);
    json_object_object_add_ex(t, "output_nodes", json_object_new_int(__nn_param->output_nodes), 0U);

    json_object_object_add_ex(t, "weights_ih", matrix_serialize(__nn_param->weights_ih), 0U);
    json_object_object_add_ex(t, "weights_ho", matrix_serialize(__nn_param->weights_ho), 0U);
    json_object_object_add_ex(t, "bias_h", matrix_serialize(__nn_param->bias_h), 0U);
    json_object_object_add_ex(t, "bias_o", matrix_serialize(__nn_param->bias_o), 0U);

    json_object_object_add_ex(t, "learning_rate", json_object_new_double((double)__nn_param->learning_rate), 0U);

    register const int flag = convert_ActivationFunction(__nn_param->activation_function);
    json_object_object_add_ex(t, "activation_function", json_object_new_int(flag), 0U);

    return t;
}

/**
 * Deserialize JSON.
 * @param json The reference #json_object.
 * @returns The new #NeuralNetwork
 *
 */
NeuralNetwork* neural_network_deserialize(const json_object* __restrict const __json_param) {
    register NeuralNetwork* nn = (NeuralNetwork*)malloc(56UL);

    nn->input_nodes = json_object_get_int(json_find(__json_param, "input_nodes"));
    nn->hidden_nodes = json_object_get_int(json_find(__json_param, "hidden_nodes"));
    nn->output_nodes = json_object_get_int(json_find(__json_param, "output_nodes"));

    nn->weights_ih = matrix_deserialize(json_find(__json_param, "weights_ih"));
    nn->weights_ho = matrix_deserialize(json_find(__json_param, "weights_ho"));
    nn->bias_h = matrix_deserialize(json_find(__json_param, "bias_h"));
    nn->bias_o = matrix_deserialize(json_find(__json_param, "bias_o"));

    neural_network_setLearningRate(nn, json_object_get_float(json_find(__json_param, "learning_rate")));
    neural_network_setActivationFunction(nn, (unsigned char)json_object_get_int(json_find(__json_param, "activation_function")));

    return nn;
}
