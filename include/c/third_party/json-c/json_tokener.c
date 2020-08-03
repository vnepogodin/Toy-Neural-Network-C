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
#include "json_tokener.h"
#include "json_object.h"
#include "printbuf.h"

#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <limits.h> /* ULONG_MAX, INT_MAX.. */

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

    json_object *current;
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

    printbuf *pb;
    struct json_tokener_srec *stack;
};


#ifdef _WIN32
#  define strdup _strdup
#  define strncasecmp _strnicmp
#endif

#define json_min(a, b) ((a) < (b) ? (a) : (b))


/* PARSING */
static int json_parse_uint64(const char* buf, unsigned long long *return_value) {
	while (*buf == ' ')
		buf++;

	if (*buf == '-')
		return 1; /* error: uint cannot be negative */

	char *end = NULL;
	register const unsigned long long val = strtoull(buf, &end, 10);
	if (end != buf)
		*return_value = val;

	return ((val == 0) || (end == buf)) ? 1 : 0;
}

static int json_tokener_parse_double(const char* buf, const int len, double *return_value) {
    char *end = NULL;
    *return_value = strtod(buf, &end);

    register const char* _expr = buf + len;
    register int result = 1;
    if (_expr == end)
        result = 0; /* It worked */

    return result;
}

/* RESETTING */
static inline void json_tokener_reset_level(struct json_tokener *tok, const int depth) {
    tok->stack[depth].state = json_tokener_state_eatws;
    tok->stack[depth].saved_state = json_tokener_state_start;

    json_object_put(tok->stack[depth].current);
    tok->stack[depth].current = NULL;

    free(tok->stack[depth].obj_field_name);
    tok->stack[depth].obj_field_name = NULL;
}

/**
 * Reset the state of a json_tokener, to prepare to parse a
 * brand new JSON object.
 */
static inline void json_tokener_reset(struct json_tokener *tok) {
    if (tok != NULL) {
        register int i = tok->depth;
        while (i >= 0) {
            json_tokener_reset_level(tok, i);
            --i;
        }

        tok->depth = 0;
        tok->err = json_tokener_success;
    }
}

/* FREEING */
static inline void json_tokener_free(struct json_tokener *tok) {
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
#ifdef DEBUG
    struct json_tokener *tok = NULL;

    /* assumed 0.004MB page sizes */
    posix_memalign((void **)&tok, 4096UL, 4096UL);
#else
    struct json_tokener *tok = (struct json_tokener *)calloc(1UL, sizeof(struct json_tokener));
#endif
	if (tok == NULL)
		return NULL;

#ifdef DEBUG
    tok->stack = NULL;

    /* assumed 0.001MB page sizes */
    posix_memalign((void **)&tok->stack, 1024UL, 1024UL);
#else
    tok->stack = (struct json_tokener_srec *)calloc((unsigned long)depth, sizeof(struct json_tokener_srec));
#endif

	if (tok->stack == NULL) {
		free(tok);
		return NULL;
	}

	tok->pb = printbuf_new();
	tok->max_depth = depth;
	json_tokener_reset(tok);
	return tok;
}

static json_object* json_tokener_parse_ex(struct json_tokener *tok, const char* str, const int len) {
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

    json_object *obj = NULL;
    char c = 1;

    while (1) { /* Note: c never might not be 0! */
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
            case json_tokener_state_null: { /* aka starts with 'n' */
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
            }
                break;

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
            }
                break;

            case json_tokener_state_comment_eol: {
                /* Advance until we change state */
                const char *case_start = str;
                while (c != '\n') {
                    ++str;
                    tok->char_offset++;
                    c = *str;
                }
                printbuf_memappend_fast(tok->pb, case_start, str - case_start);
                tok->stack[tok->depth].state = json_tokener_state_eatws;
            }
                break;

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
                const char *case_start = str;
                register int case_len = 0;
                register unsigned char neg_sign_ok = 1U;
                register unsigned char pos_sign_ok = 0U;
                register unsigned char is_exponent = 0U;
                if (printbuf_getPos(tok->pb) > 0) {
                    /* We don't save all state from the previous incremental parse
                     * so we need to re-generate it based on the saved string so far.
                     */
                    char *e_loc = strchr(printbuf_getBuf(tok->pb), 'e');
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
                        if (numuint64 <= ULONG_MAX) {
                            num64 = (long long)numuint64;
                            tok->stack[tok->depth].current = json_object_new_int64(num64);
                            if (tok->stack[tok->depth].current == NULL)
                                return NULL;
                        } else {
                            tok->stack[tok->depth].current = json_object_new_uint64(numuint64);
                            if (tok->stack[tok->depth].current == NULL)
                                return NULL;
                        }
                    }
                    else if ((tok->is_double) && (json_tokener_parse_double(printbuf_getBuf(tok->pb), printbuf_getPos(tok->pb), &numd) == 0)) {
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
                const char *case_start = str;
                while (1) {
                    if (c == tok->quote_char) {
                        printbuf_memappend_fast(tok->pb, case_start,
                                                str - case_start);

                        tok->stack[tok->depth].obj_field_name = strdup(printbuf_getBuf(tok->pb));
                        tok->stack[tok->depth].saved_state = json_tokener_state_object_field_end;
                        tok->stack[tok->depth].state = json_tokener_state_eatws;
                        break;
                    }

                    ++str;
                    tok->char_offset++;
                    c = *str;
                }
            }
                break;

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
        json_object *ret = json_object_get(tok->stack[tok->depth].current);

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
    register json_object *result = NULL;
    struct json_tokener *tok = json_tokener_new_ex(32);

    if (tok != NULL) {
        register json_object *obj = json_tokener_parse_ex(tok, str, -1);

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
