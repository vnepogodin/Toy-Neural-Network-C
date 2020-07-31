/*
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#include "config.h"

#include "json_object.h"

#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

struct _Json_Object {
    enum json_type o_type;

    unsigned int _ref_count;

    void *_userdata;

    int (*_to_json_string)(json_object *, printbuf *, int, int);
    void (*_user_delete)(json_object *, void*);

    printbuf *_pb;
    /* Actually longer, always malloc'd as some more-specific type.
    * The rest of a struct json_object_${o_type} follows
    */
};

typedef enum json_object_int_type {
    json_object_int_type_int64,
    json_object_int_type_uint64
} json_object_int_type;

struct json_object_object {
    json_object base;
    lh_table *c_object;
};
struct json_object_array {
    json_object base;
    array_list *c_array;
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
struct json_object_string {
    long len; /* Signed b/c negative lengths indicate data is a pointer */
    /* Consider adding an "alloc" field here, if json_object_set_string calls
     * to expand the length of a string are common operations to perform.
     */
    union {
        char idata[1]; // Immediate data.  Actually longer
        char *pdata;   // Only when len < 0
    } c_string;

    json_object base;
};

#ifdef _WIN32
#  define strdup _strdup
#endif

/* generic object construction and destruction parts */
static void json_object_generic_delete(json_object *jso) {
    printbuf_free(jso->_pb);
    free(jso);
}

static int _json_parse_int64(const char* buf, long long *retval) {
    errno = 0;

    char *end = NULL;
	register const long long val = strtoll(buf, &end, 10);
	if (end != buf)
		*retval = val;

	return (((val == 0) && (errno != 0)) || (end == buf)) ? 1 : 0;
}

/*
 * Helper functions to more safely cast to a particular type of json_object
 */
static inline struct json_object_object *JC_OBJECT(json_object *jso) {
	return (void *)jso;
}
static inline const struct json_object_object *JC_OBJECT_C(const json_object *jso) {
	return (const void *)jso;
}
static inline struct json_object_array *JC_ARRAY(json_object *jso) {
	return (void *)jso;
}
static inline const struct json_object_array *JC_ARRAY_C(const json_object *jso) {
	return (const void *)jso;
}
static inline struct json_object_double *JC_DOUBLE(json_object *jso) {
	return (void *)jso;
}
static inline const struct json_object_double *JC_DOUBLE_C(const json_object *jso) {
	return (const void *)jso;
}
static inline struct json_object_int *JC_INT(json_object *jso) {
	return (void *)jso;
}
static inline const struct json_object_int *JC_INT_C(const json_object *jso) {
	return (const void *)jso;
}
static inline struct json_object_string *JC_STRING(json_object *jso) {
	return (void *)jso;
}
static inline const struct json_object_string *JC_STRING_C(const json_object *jso) {
	return (const void *)jso;
}

#define JC_CONCAT(a, b) a##b
#define JC_CONCAT3(a, b, c) a##b##c

#define JSON_OBJECT_NEW(jtype)                                                       \
	(struct JC_CONCAT(json_object_, jtype) *)json_object_new(                        \
	    JC_CONCAT(json_type_, jtype), sizeof(struct JC_CONCAT(json_object_, jtype)), \
	    &JC_CONCAT3(json_object_, jtype, _to_json_string))

static inline json_object* json_object_new(enum json_type o_type, const unsigned long alloc_size, json_object_to_json_string_fn *to_json_string) {
    json_object *jso = (json_object *)malloc(alloc_size);
    if (jso == NULL)
        return NULL;

    jso->o_type = o_type;
    jso->_ref_count = 1;
    jso->_to_json_string = to_json_string;
    jso->_pb = NULL;
    jso->_user_delete = NULL;
    jso->_userdata = NULL;
    /* @code jso->...
     * Type-specific fields must be set by caller
     */

    return jso;
}

static void json_object_object_delete(json_object *jso_base) {
    lh_table_free(JC_OBJECT(jso_base)->c_object);
    json_object_generic_delete(jso_base);
}

static void json_object_string_delete(json_object *jso) {
    if (JC_STRING(jso)->len < 0)
        free(JC_STRING(jso)->c_string.pdata);

    json_object_generic_delete(jso);
}

static void json_object_array_delete(json_object *jso) {
    array_list_free(JC_ARRAY(jso)->c_array);
    json_object_generic_delete(jso);
}

static json_object_to_json_string_fn json_object_object_to_json_string;
static json_object_to_json_string_fn json_object_double_to_json_string_default;
static json_object_to_json_string_fn json_object_int_to_json_string;
static json_object_to_json_string_fn json_object_string_to_json_string;
static json_object_to_json_string_fn json_object_array_to_json_string;
static json_object_to_json_string_fn _json_object_userdata_to_json_string;

#ifndef JSON_NORETURN
#if defined(_MSC_VER)
#define JSON_NORETURN __declspec(noreturn)
#elif defined(__OS400__)
#define JSON_NORETURN
#else
/* 'cold' attribute is for optimization, telling the computer this code
 * path is unlikely.
 */
#define JSON_NORETURN __attribute__((noreturn, cold))
#endif
#endif
/**
 * Abort and optionally print a message on standard error.
 * This should be used rather than assert() for unconditional abortion
 * (in particular for code paths which are never supposed to be run).
 * */
JSON_NORETURN static void json_abort(const char* message) {
    if (message != NULL)
        fprintf(stderr, "json-c aborts with error: %s\n", message);

    abort();
}

/* helper for accessing the optimized string data component in json_object
 */
static inline char* get_string_component_mutable(json_object *jso) {
    /* Due to json_object_set_string(), we might have a pointer */
	if (JC_STRING_C(jso)->len < 0)
        return JC_STRING(jso)->c_string.pdata;

	return JC_STRING(jso)->c_string.idata;
}
static inline const char* get_string_component(const json_object *jso) {
	return get_string_component_mutable((void *)(uintptr_t)(const void *)jso);
}

/* string escaping */
static int json_escape_str(printbuf *pb, const char *str, unsigned long len, const int flags) {
    static const char json_hex_chars[22] = "0123456789abcdefABCDEF";

	register int pos = 0;
    register int start_offset = 0;

	register unsigned char c = 0U;
	while (len--) {
		c = (unsigned char)str[pos];

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

			if ((pos - start_offset) > 0)
				printbuf_memappend(pb, str + start_offset, pos - start_offset);

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
			else if (c == '/')
				printbuf_memappend(pb, "\\/", 2);

            ++pos;
			start_offset = pos;
			break;
		default:
			if (c < ' ') {
				if (pos - start_offset > 0)
					printbuf_memappend(pb, str + start_offset, pos - start_offset);

				char sbuf[7] = { 0 };
				snprintf(sbuf, 7, "\\u00%c%c", json_hex_chars[c >> 4U], json_hex_chars[c & (unsigned)0xf]);
				printbuf_memappend_fast(pb, sbuf, 6);
                ++pos;
				start_offset = pos;
			}
			else
				pos++;
		}
	}
	if ((pos - start_offset) > 0)
		printbuf_memappend(pb, str + start_offset, pos - start_offset);

	return 0;
}

/* reference counting */
json_object* json_object_get(json_object *jso) {
	if (jso == NULL)
		return jso;

	/* Don't overflow the refcounter. */
	assert(jso->_ref_count < UINT_MAX);

#ifdef _WIN32
	++jso->_ref_count;
#else
	__sync_add_and_fetch(&jso->_ref_count, 1);
#endif

	return jso;
}

int json_object_put(json_object *jso) {
	if (jso == NULL)
		return 0;

	/* Avoid invalid free and crash explicitly instead of (silently)
	 * segfaulting.
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
	case json_type_string:
        json_object_string_delete(jso);
        break;
	default:
        json_object_generic_delete(jso);
        break;
	}
	return 1;
}

/* type checking functions */
enum json_type json_object_get_type(const json_object *jso) {
	if (jso == NULL)
		return json_type_null;

	return jso->o_type;
}

void json_object_set_userdata(json_object *jso, void* userdata, void(*user_delete)(json_object *, void*)) {
	/* Can't return failure, so abort if we can't perform the operation. */
	assert(jso != NULL);

	/* First, clean up any previously existing user info */
	if (jso->_user_delete != NULL)
		jso->_user_delete(jso, jso->_userdata);

	jso->_userdata = userdata;
	jso->_user_delete = user_delete;
}

/* set a custom conversion to string */
void json_object_set_serializer(json_object *jso, int (*to_string_func)(json_object *, printbuf *, int, int), void* userdata, void(*user_delete)(json_object *, void*)) {
	json_object_set_userdata(jso, userdata, user_delete);

	if (to_string_func == NULL) {
		/* Reset to the standard serialization function */
		switch (jso->o_type) {
		case json_type_null:
            jso->_to_json_string = NULL;
            break;
		case json_type_double:
			jso->_to_json_string = &json_object_double_to_json_string_default;
			break;
		case json_type_int:
            jso->_to_json_string = &json_object_int_to_json_string;
            break;
		case json_type_object:
			jso->_to_json_string = &json_object_object_to_json_string;
			break;
		case json_type_array:
			jso->_to_json_string = &json_object_array_to_json_string;
			break;
		case json_type_string:
			jso->_to_json_string = &json_object_string_to_json_string;
			break;
		}
		return;
	}

	jso->_to_json_string = to_string_func;
}

/* extended conversion to string */
const char* json_object_to_json_string_length(json_object *jso, const int flags, unsigned long *length) {
	const char *r = NULL;
    unsigned long s = 0;

	if (jso == NULL) {
		s = 4;
		r = "null";
	} else if ((jso->_pb) || (jso->_pb = printbuf_new())) {
		printbuf_reset(jso->_pb);

		if (jso->_to_json_string(jso, jso->_pb, 0, flags) >= 0) {
			s = (unsigned long)printbuf_getPos(jso->_pb);
			r = printbuf_getBuf(jso->_pb);
		}
	}

	if (length)
		*length = s;

	return r;
}

const char* json_object_to_json_string_ext(json_object *jso, const int flags) {
	return json_object_to_json_string_length(jso, flags, NULL);
}

/* backwards-compatible conversion to string */
const char* json_object_to_json_string(json_object *jso) {
	return json_object_to_json_string_ext(jso, JSON_C_TO_STRING_SPACED);
}

static void indent(printbuf *pb, int level, int flags) {
	if ((unsigned)flags & JSON_C_TO_STRING_PRETTY) {
		if ((unsigned)flags & JSON_C_TO_STRING_PRETTY_TAB)
			printbuf_memset(pb, -1, '\t', level);
		else
			printbuf_memset(pb, -1, ' ', level * 2);
	}
}

/* json_object_object */
static int json_object_object_to_json_string(json_object *jso, printbuf *pb, int level, int flags) {
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

static void json_object_lh_entry_free(lh_entry *ent) {
	if (!lh_entry_getConstant(ent))
		free((void *)lh_entry_getK(ent));

	json_object_put((json_object *)lh_entry_getV(ent));
}

json_object* json_object_new_object(void) {
	struct json_object_object *jso = JSON_OBJECT_NEW(object);
	if (jso == NULL)
		return NULL;

	jso->c_object = lh_kchar_table_new(JSON_OBJECT_DEF_HASH_ENTRIES, &json_object_lh_entry_free);
	if (jso->c_object == NULL) {
		json_object_generic_delete(&jso->base);
		errno = ENOMEM;
		return NULL;
	}
	return &jso->base;
}

lh_table* json_object_get_object(const json_object *jso) {
	if (jso == NULL)
		return NULL;

	switch (jso->o_type) {
	case json_type_object:
        return JC_OBJECT_C(jso)->c_object;
	default:
        return NULL;
	}
}

int json_object_object_add_ex(json_object *jso, const char* const key, json_object *const val, const unsigned opts) {
	assert(json_object_get_type(jso) == json_type_object);

	/* We lookup the entry and replace the value, rather than just deleting
	 * and re-adding it, so the existing key remains valid.
	 */
	register const unsigned long hash = lh_get_hash(JC_OBJECT(jso)->c_object, (const void *)key);
	lh_entry *existing_entry = (opts & JSON_C_OBJECT_ADD_KEY_IS_NEW)
        ? NULL
        : lh_table_lookup_entry_w_hash(JC_OBJECT(jso)->c_object, (const void *)key, hash);

	/* The caller must avoid creating loops in the object tree, but do a
	 * quick check anyway to make sure we're not creating a trivial loop.
	 */
	if (jso == val)
		return -1;

	if (existing_entry == NULL) {
		const void *const k = (opts & JSON_C_OBJECT_KEY_IS_CONSTANT) ? (const void *)key : strdup(key);
		if (k == NULL)
			return -1;

		return lh_table_insert_w_hash(JC_OBJECT(jso)->c_object, k, val, hash, opts);
	}

	json_object *existing_value = (json_object *)lh_entry_getV(existing_entry);
	if (existing_value != NULL)
		json_object_put(existing_value);

    lh_entry_setV(existing_entry, val);
	return 0;
}

int json_object_object_add(json_object *jso, const char* key, json_object *val) {
	return json_object_object_add_ex(jso, key, val, 0);
}

int json_object_object_get_ex(const json_object *jso, const char* key, json_object **value) {
	if (value != NULL)
		*value = NULL;

	if (jso == NULL)
		return 0;

	switch (jso->o_type) {
	case json_type_object:
		return lh_table_lookup_ex(JC_OBJECT_C(jso)->c_object, (const void *)key, (void **)value);
	default:
		if (value != NULL)
			*value = NULL;

		return 0;
	}
}

/* json_object_int */
static int json_object_int_to_json_string(json_object *jso, printbuf *pb, int level, int flags) {
	/* room for 19 digits, the sign char, and a null term */
	char sbuf[21];
	if (JC_INT(jso)->cint_type == json_object_int_type_int64)
		snprintf(sbuf, 21, "%lld", JC_INT(jso)->cint.c_int64);
	else
		snprintf(sbuf, 21, "%llu", JC_INT(jso)->cint.c_uint64);

	return printbuf_memappend(pb, sbuf, (int)strlen(sbuf));
}

json_object* json_object_new_int(const int i) {
	return json_object_new_int64(i);
}

int json_object_get_int(const json_object *jso) {
    if (jso == NULL)
		return 0;

	long long cint64 = 0LL;
	double cdouble = 0.0;
	enum json_type o_type = jso->o_type;
	if (o_type == json_type_int) {
		const struct json_object_int *jsoint = JC_INT_C(jso);
		if (jsoint->cint_type == json_object_int_type_int64)
			cint64 = jsoint->cint.c_int64;
        else {
			if (jsoint->cint.c_uint64 >= ULLONG_MAX)
				cint64 = INT64_MAX;
			else
				cint64 = (long long)jsoint->cint.c_uint64;
		}
	} else if (o_type == json_type_string) {
		/*
		 * Parse strings into 64-bit numbers, then use the
		 * 64-to-32-bit number handling below.
		 */
		if (_json_parse_int64(get_string_component(jso), &cint64) != 0)
			return 0; /* whoops, it didn't work. */

		o_type = json_type_int;
	}

	switch (o_type) {
	case json_type_int:
		/* Make sure we return the correct values for out of range numbers. */
		if (cint64 <= INT_MIN)
			return INT_MIN;

		if (cint64 >= INT_MAX)
			return INT_MAX;

		return (int)cint64;
	case json_type_double:
		cdouble = JC_DOUBLE_C(jso)->c_double;
		if (cdouble <= INT_MIN)
			return INT_MIN;

		if (cdouble >= INT_MAX)
			return INT_MAX;

		return (int)cdouble;
	default:
        return 0;
	}
}

json_object* json_object_new_int64(const long long i) {
	struct json_object_int *jso = JSON_OBJECT_NEW(int);
	if (jso == NULL)
		return NULL;

	jso->cint.c_int64 = i;
	jso->cint_type = json_object_int_type_int64;
	return &jso->base;
}

json_object* json_object_new_uint64(const unsigned long long i) {
	struct json_object_int *jso = JSON_OBJECT_NEW(int);
	if (jso == NULL)
		return NULL;

	jso->cint.c_uint64 = i;
	jso->cint_type = json_object_int_type_uint64;
	return &jso->base;
}

/* json_object_double */
static int json_object_double_to_json_string_format(json_object *jso, printbuf *pb, int level, int flags, const char* format) {
	struct json_object_double *jsodbl = JC_DOUBLE(jso);
	char buf[128];
	register int size = 0;
	/* Although JSON RFC does not support
	 * NaN or Infinity as numeric values
	 * ECMA 262 section 9.8.1 defines
	 * how to handle these cases as strings
	 */
	if (isnan(jsodbl->c_double))
		size = snprintf(buf, 128, "NaN");
	else if (isinf(jsodbl->c_double)) {
		if (jsodbl->c_double > 0)
			size = snprintf(buf, 128, "Infinity");
		else
			size = snprintf(buf, 128, "-Infinity");
	} else {
		const char *std_format = "%.17g";
		if (format == NULL)
            format = std_format;
		size = snprintf(buf, 128UL, format, jsodbl->c_double);

		if (size < 0)
			return -1;

        char* p = strchr(buf, ',');
		if (p != NULL)
			*p = '.';
		else
			p = strchr(buf, '.');

        /* Looks like *some* kind of number */
        register int looks_numeric = isdigit((unsigned char)buf[0]) || ((size > 1) && (buf[0] == '-') && isdigit((unsigned char)buf[1]));

		if ((size < 126) && looks_numeric && (p == NULL) && /* Has no decimal point */
		    (strchr(buf, 'e') == NULL)) { /* Not scientific notation */
			/* Ensure it looks like a float, even if snprintf didn't,
			 *  unless a custom format is set to omit the decimal.
             */
			strcat(buf, ".0");
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

	printbuf_memappend(pb, buf, size);
	return size;
}

static int json_object_double_to_json_string_default(json_object *jso, printbuf *pb, int level, int flags) {
	return json_object_double_to_json_string_format(jso, pb, level, flags, NULL);
}

json_object* json_object_new_double(const double d) {
	struct json_object_double *jso = (struct json_object_double *)json_object_new(json_type_double, sizeof(struct json_object_double), &json_object_double_to_json_string_default);
	if (jso == NULL)
		return NULL;

	jso->c_double = d;
	return &jso->base;
}

json_object* json_object_new_double_s(const double d, const char* ds) {
    register json_object *result = NULL;
    json_object *jso = json_object_new_double(d);

	if (jso != NULL) {
        char* new_ds = strdup(ds);

        if (new_ds != NULL) {
            json_object_set_serializer(jso, _json_object_userdata_to_json_string, new_ds, json_object_free_userdata);
            result = jso;
        } else
            json_object_generic_delete(jso);
	}

	return result;
}

static int _json_object_userdata_to_json_string(json_object *jso, printbuf *pb, int level, int flags) {
	return json_object_userdata_to_json_string(jso, pb, level, flags);
}

/*
 * A wrapper around json_object_userdata_to_json_string() used only
 * by json_object_new_double_s() just so json_object_set_double() can
 * detect when it needs to reset the serializer to the default.
 */
int json_object_userdata_to_json_string(json_object *jso, printbuf *pb, int level, int flags) {
	register int userdata_len = (int)strlen((const char *)jso->_userdata);
	printbuf_memappend(pb, (const char *)jso->_userdata, userdata_len);
	return userdata_len;
}

void json_object_free_userdata(json_object *jso, void* userdata) {
	free(userdata);
}

double json_object_get_double(const json_object *jso) {
	if (jso == NULL)
		return 0.0;

    double cdouble = 0.0;
    char *errPtr = NULL;
	switch (jso->o_type) {
	case json_type_double: return JC_DOUBLE_C(jso)->c_double;
	case json_type_int:
		switch (JC_INT_C(jso)->cint_type) {
		case json_object_int_type_int64:
		    return JC_INT_C(jso)->cint.c_int64;
		case json_object_int_type_uint64:
		    return JC_INT_C(jso)->cint.c_uint64;
		default:
		    json_abort("invalid cint_type");
		}
	case json_type_string:
		errno = 0;
		cdouble = strtod(get_string_component(jso), &errPtr);

		/* if conversion stopped at the first character, return 0.0 */
		if (errPtr == get_string_component(jso)) {
			errno = EINVAL;
			return 0.0;
		}

		/*
		 * Check that the conversion terminated on something sensible
		 *
		 * For example, { "pay" : 123AB } would parse as 123.
		 */
		if (*errPtr != '\0') {
			errno = EINVAL;
			return 0.0;
		}

		/*
		 * If strtod encounters a string which would exceed the
		 * capacity of a double, it returns +/- HUGE_VAL and sets
		 * errno to ERANGE. But +/- HUGE_VAL is also a valid result
		 * from a conversion, so we need to check errno.
		 *
		 * Underflow also sets errno to ERANGE, but it returns 0 in
		 * that case, which is what we will return anyway.
		 *
		 * See CERT guideline ERR30-C
		 */
		if ((HUGE_VAL == cdouble || -HUGE_VAL == cdouble) && (ERANGE == errno))
			cdouble = 0.0;

		return cdouble;
	default:
	    errno = EINVAL;
	    return 0.0;
	}
}

/* json_object_string */
static int json_object_string_to_json_string(json_object *jso, printbuf *pb, int level, int flags) {
    register long len = JC_STRING(jso)->len;

	printbuf_strappend(pb, "\"");

    if (len < 0)
        json_escape_str(pb, get_string_component(jso), (unsigned long)-len, flags);
    else
        json_escape_str(pb, get_string_component(jso), (unsigned long)len, flags);

	printbuf_strappend(pb, "\"");

	return 0;
}

static json_object* _json_object_new_string(const char* s, const unsigned long len) {
	/*
     * Structures           Actual memory layout
     * -------------------  --------------------
	 * [json_object_string  [json_object_string
	 *  [json_object]        [json_object]
     *  ...other fields...   ...other fields...
	 *  c_string]            len
     *                       bytes
	 *                       of
	 *                       string
	 *                       data
     *                       \0]
	 */
    struct json_object_string *jso = NULL;

	if (len > (LONG_MAX - (sizeof(*jso) - sizeof(jso->c_string)) - 1))
		return NULL;

	register unsigned long objsize = (sizeof(*jso) - sizeof(jso->c_string)) + len + 1;
    /* We need a minimum size to support json_object_set_string() mutability
	 * so we can stuff a pointer into pdata :(
     */
	if (len < sizeof(void *))
        objsize += sizeof(void *) - len;

	jso = (struct json_object_string *)json_object_new(json_type_string, objsize, &json_object_string_to_json_string);

	if (jso == NULL)
		return NULL;

	jso->len = (long)len;
	memcpy(jso->c_string.idata, s, len);
	jso->c_string.idata[len] = '\0';
	return &jso->base;
}

json_object* json_object_new_string_len(const char* s, const int len) {
	return _json_object_new_string(s, (unsigned long)len);
}

const char* json_object_get_string(json_object *jso) {
	if (jso == NULL)
		return NULL;

	switch (jso->o_type) {
	case json_type_string:
        return get_string_component(jso);
	default:
        return json_object_to_json_string(jso);
	}
}

/* json_object_array */
static int json_object_array_to_json_string(json_object *jso, printbuf *pb, int level, int flags) {
	printbuf_strappend(pb, "[");
	if ((unsigned)flags & JSON_C_TO_STRING_PRETTY)
		printbuf_strappend(pb, "\n");

    register unsigned char had_children = 0U;
	unsigned long ii = 0UL;
	for (ii = 0UL; ii < json_object_array_length(jso); ii++) {
	    json_object *val;
		if (had_children) {
			printbuf_strappend(pb, ",");
			if ((unsigned)flags & JSON_C_TO_STRING_PRETTY)
				printbuf_strappend(pb, "\n");
		}
		had_children = 1U;
		if ((unsigned)flags & JSON_C_TO_STRING_SPACED && !((unsigned)flags & JSON_C_TO_STRING_PRETTY))
			printbuf_strappend(pb, " ");

		indent(pb, level + 1, flags);
		val = json_object_array_get_idx(jso, ii);

		if (val == NULL)
			printbuf_strappend(pb, "null");
		else if (val->_to_json_string(val, pb, level + 1, flags) < 0)
			return -1;
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

static void json_object_array_entry_free(void* data) {
	json_object_put((json_object *)data);
}

json_object* json_object_new_array(void) {
	return json_object_new_array_ext(32);
}

json_object* json_object_new_array_ext(const int initial_size) {
	struct json_object_array *jso = JSON_OBJECT_NEW(array);
	if (jso == NULL)
		return NULL;

	jso->c_array = array_list_new(&json_object_array_entry_free, initial_size);
	if (jso->c_array == NULL) {
		free(jso);
		return NULL;
	}

	return &jso->base;
}

unsigned long json_object_array_length(const json_object *jso) {
	assert(json_object_get_type(jso) == json_type_array);

	return array_list_length(JC_ARRAY_C(jso)->c_array);
}

int json_object_array_add(json_object *jso, json_object *val) {
	assert(json_object_get_type(jso) == json_type_array);

	return array_list_add(JC_ARRAY(jso)->c_array, val);
}

json_object* json_object_array_get_idx(const json_object *jso, unsigned long idx) {
	assert(json_object_get_type(jso) == json_type_array);

	return (json_object *)array_list_get_idx(JC_ARRAY_C(jso)->c_array, idx);
}

int json_object_array_shrink(json_object *jso, const int empty_slots) {
	if (empty_slots < 0)
		json_abort("json_object_array_shrink called with negative empty_slots");

	return array_list_shrink(JC_ARRAY(jso)->c_array, (unsigned long)empty_slots);
}
