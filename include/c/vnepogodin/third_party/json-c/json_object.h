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
