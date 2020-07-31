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

#include "config.h"

#include "json_tokener.h"
#include "json_object.h"
#include "printbuf.h"

#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <locale.h>

enum json_tokener_state {
    json_tokener_state_eatws,
    json_tokener_state_start,
    json_tokener_state_finish,
    json_tokener_state_null,
    json_tokener_state_comment_start,
    json_tokener_state_comment,
    json_tokener_state_comment_eol,
    json_tokener_state_comment_end,
    json_tokener_state_string,
    json_tokener_state_string_escape,
    json_tokener_state_escape_unicode,
    json_tokener_state_escape_unicode_need_escape,
    json_tokener_state_escape_unicode_need_u,
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
    json_tokener_state_inf
};

struct json_tokener_srec {
    enum json_tokener_state state, saved_state;

    char *obj_field_name;

    json_object *current;
};

/**
 * Internal state of the json parser.
 * Do not access any fields of this structure directly.
 * Its definition is published due to historical limitations
 * in the json tokener API, and will be changed to be an opaque
 * type in the future.
 */
struct _Json_Tokener {
    enum json_tokener_error err;

    unsigned int ucs_char, high_surrogate;

    char quote_char;

    int char_offset;

    int flags;

    int max_depth, depth, is_double, st_pos;

    char* str;

    printbuf *pb;
    struct json_tokener_srec *stack;
};


#ifdef _WIN32
#  define strdup _strdup
#  define strncasecmp _strnicmp
#endif

#define json_min(a, b) ((a) < (b) ? (a) : (b))

#define jt_hexdigit(x) (((unsigned char)(x) <= (unsigned char)'9') ? (unsigned char)(x) - (unsigned char)'0' : ((unsigned char)(x) & 7U) + 9U)

static const char json_nan_str[3] = "NaN";
static const int json_nan_str_len = 2;

static int _json_parse_int64(const char* buf, long long *retval) {
	errno = 0;

    char *end = NULL;
	register const long long val = strtoll(buf, &end, 10);
	if (end != buf)
		*retval = val;

	return ((val == 0 && errno != 0) || (end == buf)) ? 1 : 0;
}

static int _json_parse_uint64(const char* buf, unsigned long long *retval) {
	errno = 0;
	while (*buf == ' ')
		buf++;

	if (*buf == '-')
		return 1; /* error: uint cannot be negative */

	char *end = NULL;
	register const unsigned long long val = strtoull(buf, &end, 10);
	if (end != buf)
		*retval = val;

	return ((val == 0 && errno != 0) || (end == buf)) ? 1 : 0;
}


/**
 * validate the utf-8 string in strict model.
 * if not utf-8 format, return err.
 */
static int json_tokener_validate_utf8(const char c, unsigned int *nBytes) {
    register const unsigned char chr = (unsigned char)c;
    if (*nBytes == 0) {
        if (chr >= 0x80) {
            if ((chr & (unsigned)0xe0) == 0xc0)
                *nBytes = 1;
            else if ((chr & (unsigned)0xf0) == 0xe0)
                *nBytes = 2;
            else if ((chr & (unsigned)0xf8) == 0xf0)
                *nBytes = 3;
            else
                return 0;
        }
    } else {
        if ((chr & (unsigned)0xC0) != 0x80)
            return 0;

        (*nBytes)--;
    }
    return 1;
}

static int json_tokener_parse_double(const char* buf, const int len, double *retval) {
    char *end = NULL;
    *retval = strtod(buf, &end);

    register const char* _expr = buf + len;
    register int result = 1;
    if (_expr == end)
        result = 0; /* It worked */

    return result;
}

/* Stuff for decoding unicode sequences */
#define IS_HIGH_SURROGATE(uc) (((uc) & (unsigned)0xFC00) == 0xD800)
#define IS_LOW_SURROGATE(uc) (((uc) & (unsigned)0xFC00) == 0xDC00)
#define DECODE_SURROGATE_PAIR(hi, lo) ((((hi) & (unsigned)0x3FF) << 10U) + ((lo) & (unsigned)0x3FF) + 0x10000)
static unsigned char utf8_replacement_char[3] = { (unsigned char)0xEF, (unsigned char)0xBF, (unsigned char)0xBD };

json_tokener* json_tokener_new_ex(const int depth) {
    json_tokener *tok = (json_tokener *)calloc(1UL, sizeof(json_tokener));
	if (tok == NULL)
		return NULL;

	tok->stack = (struct json_tokener_srec *)calloc((unsigned long)depth, sizeof(struct json_tokener_srec));
	if (tok->stack == NULL) {
		free(tok);
		return NULL;
	}

	tok->pb = printbuf_new();
	tok->max_depth = depth;
	json_tokener_reset(tok);
	return tok;
}

void json_tokener_free(json_tokener *tok) {
	json_tokener_reset(tok);
	if (tok->pb != NULL)
		printbuf_free(tok->pb);

	free(tok->stack);
	free(tok);
}

static void json_tokener_reset_level(json_tokener *tok, const int depth) {
	tok->stack[depth].state = json_tokener_state_eatws;
	tok->stack[depth].saved_state = json_tokener_state_start;

	json_object_put(tok->stack[depth].current);
	tok->stack[depth].current = NULL;

	free(tok->stack[depth].obj_field_name);
	tok->stack[depth].obj_field_name = NULL;
}

void json_tokener_reset(json_tokener *tok) {
	if (tok == NULL)
		return;

    register int i = tok->depth;
    while (i >= 0) {
        json_tokener_reset_level(tok, i);
        --i;
    }

	tok->depth = 0;
	tok->err = json_tokener_success;
}

json_object* json_tokener_parse(const char* str) {
	enum json_tokener_error jerr_ignored;

    json_object *obj = json_tokener_parse_verbose(str, &jerr_ignored);
	return obj;
}

json_object* json_tokener_parse_verbose(const char* str, enum json_tokener_error *error) {
    json_tokener *tok = json_tokener_new_ex(JSON_TOKENER_DEFAULT_DEPTH);
	if (tok == NULL)
		return NULL;

    json_object *obj = json_tokener_parse_ex(tok, str, -1);
	*error = tok->err;
	if (tok->err != json_tokener_success
#if 0
		/* This would be a more sensible default, and cause parsing
		 * things like "null123" to fail when the caller can't know
		 * where the parsing left off, but starting to fail would
		 * be a notable behaviour change.  Save for a 1.0 release.
		 */
	    || json_tokener_get_parse_end(tok) != strlen(str)
#endif
	)

	{
		if (obj != NULL)
			json_object_put(obj);

		obj = NULL;
	}

	json_tokener_free(tok);
	return obj;
}

#define saved_state tok->stack[tok->depth].saved_state
#define current tok->stack[tok->depth].current
#define obj_field_name tok->stack[tok->depth].obj_field_name

/* Optimization:
 * json_tokener_parse_ex() consumed a lot of CPU in its main loop,
 * iterating character-by character.  A large performance boost is
 * achieved by using tighter loops to locally handle units such as
 * comments and strings.  Loops that handle an entire token within
 * their scope also gather entire strings and pass them to
 * printbuf_memappend() in a single call, rather than calling
 * printbuf_memappend() one char at a time.
 *
 * PEEK_CHAR() and ADVANCE_CHAR() macros are used for code that is
 * common to both the main loop and the tighter loops.
 */

/* PEEK_CHAR(dest, tok) macro:
 *  Peeks at the current char and stores it in dest.
 *  Returns 1 on success, sets tok->err and returns 0 if no more chars.
 *  Implicit inputs:  str, len, nBytesp variables
 */
#define PEEK_CHAR(dest, tok)                                                                      \
	(((tok)->char_offset == len)                                                                  \
	     ? ((((tok)->depth == 0) && (tok->stack[tok->depth].state == json_tokener_state_eatws) && \
             (saved_state == json_tokener_state_finish))                                          \
	            ? (((tok)->err = json_tokener_success), 0)                                        \
	            : (((tok)->err = json_tokener_continue), 0))                                      \
	     : ((((unsigned)tok->flags & JSON_TOKENER_VALIDATE_UTF8) &&                               \
	         (!json_tokener_validate_utf8(*str, nBytesp)))                                        \
	            ? ((tok->err = json_tokener_error_parse_utf8_string), 0)                          \
	            : (((dest) = *str), 1)))

/* ADVANCE_CHAR() macro:
 *   Increments str & tok->char_offset.
 *   For convenience of existing conditionals, returns the old value of c (0 on eof)
 *   Implicit inputs:  c var
 */
#define ADVANCE_CHAR(str, tok) (++(str), ((tok)->char_offset)++, c)

/* End optimization macro defs */
json_object* json_tokener_parse_ex(json_tokener *tok, const char* str, const int len) {
	tok->char_offset = 0;
	tok->err = json_tokener_success;

	/* this interface is presently not 64-bit clean due to the int len argument
	 * and the internal printbuf interface that takes 32-bit int len arguments
	 * so the function limits the maximum string size to INT32_MAX (2GB).
	 * If the function is called with len == -1 then strlen is called to check
	 * the string length is less than INT32_MAX (2GB)
	 */
	if ((len < -1) || (len == -1 && strlen(str) > INT32_MAX)) {
		tok->err = json_tokener_error_size;
		return NULL;
	}

    char *oldlocale = NULL;
	{
		char *tmplocale;
		tmplocale = setlocale(LC_NUMERIC, NULL);
		if (tmplocale)
			oldlocale = strdup(tmplocale);
		setlocale(LC_NUMERIC, "C");
	}

    json_object *obj = NULL;
    char c = '\1';
    unsigned int nBytes = 0;
    unsigned int *nBytesp = &nBytes;

	while (PEEK_CHAR(c, tok)) { /* Note: c might be '\0' ! */


	redo_char:
		switch (tok->stack[tok->depth].state) {
		case json_tokener_state_eatws:
			/* Advance until we change state */
			while (isspace((unsigned char)c)) {
				if ((!ADVANCE_CHAR(str, tok)) || (!PEEK_CHAR(c, tok)))
					goto out;
			}
			if (c == '/' && !((unsigned)tok->flags & JSON_TOKENER_STRICT)) {
				printbuf_reset(tok->pb);
				printbuf_memappend_fast(tok->pb, &c, 1);
                tok->stack[tok->depth].state = json_tokener_state_comment_start;
			}
			else {
                tok->stack[tok->depth].state = saved_state;
				goto redo_char;
			}
			break;

		case json_tokener_state_start:
			switch (c) {
			case '{':
                tok->stack[tok->depth].state = json_tokener_state_eatws;
				saved_state = json_tokener_state_object_field_start;
				current = json_object_new_object();
				if (current == NULL)
					goto out;

				break;
			case '[':
                tok->stack[tok->depth].state = json_tokener_state_eatws;
				saved_state = json_tokener_state_array;
				current = json_object_new_array();
				if (current == NULL)
					goto out;
				break;
			case 'I':
			case 'i':
                tok->stack[tok->depth].state = json_tokener_state_inf;
				printbuf_reset(tok->pb);
				tok->st_pos = 0;
				goto redo_char;
			case 'N':
			case 'n':
                tok->stack[tok->depth].state = json_tokener_state_null; // or NaN
				printbuf_reset(tok->pb);
				tok->st_pos = 0;
				goto redo_char;
			case '\'':
				if ((unsigned)tok->flags & JSON_TOKENER_STRICT) {
					/* in STRICT mode only double-quote are allowed */
					tok->err = json_tokener_error_parse_unexpected;
					goto out;
				}
				/* FALLTHRU */
			case '"':
                tok->stack[tok->depth].state = json_tokener_state_string;
				printbuf_reset(tok->pb);
				tok->quote_char = c;
				break;
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
				tok->is_double = 0;
				goto redo_char;
			default:
                tok->err = json_tokener_error_parse_unexpected;
                goto out;
			}
			break;

		case json_tokener_state_finish:
			if (tok->depth == 0)
				goto out;

			obj = json_object_get(current);
			json_tokener_reset_level(tok, tok->depth);
			tok->depth--;
			goto redo_char;

		case json_tokener_state_inf: { /* aka starts with 'i' (or 'I', or "-i", or "-I") */
			/* If we were guaranteed to have len set, then we could (usually) handle
			 * the entire "Infinity" check in a single strncmp (strncasecmp), but
			 * since len might be -1 (i.e. "read until \0"), we need to check it
			 * a character at a time.
			 * Trying to handle it both ways would make this code considerably more
			 * complicated with likely little performance benefit.
			 */
			const char* _json_inf_str = "Infinity";
			if (!((unsigned)tok->flags & JSON_TOKENER_STRICT))
				_json_inf_str = "infinity";

			/* Note: tok->st_pos must be 0 when state is set to json_tokener_state_inf */
			while (tok->st_pos < 7) {
				char inf_char = *str;
				if (!((unsigned)tok->flags & JSON_TOKENER_STRICT))
					inf_char = (char)tolower((int)*str);

				if (inf_char != _json_inf_str[tok->st_pos]) {
					tok->err = json_tokener_error_parse_unexpected;
					goto out;
				}
				tok->st_pos++;
				(void)ADVANCE_CHAR(str, tok);

                /* out of input chars, for now at least */
				if (!PEEK_CHAR(c, tok))
					goto out;
			}
			/* We checked the full length of "Infinity", so create the object.
			 * When handling -Infinity, the number parsing code will have dropped
			 * the "-" into tok->pb for us, so check it now.
			 */
            if ((printbuf_getPos(tok->pb) > 0) && (*printbuf_getBuf(tok->pb) == '-'))
                current = json_object_new_double((double)-INFINITY);
            else
                current = json_object_new_double((double)INFINITY);

			if (current == NULL)
				goto out;

			saved_state = json_tokener_state_finish;
            tok->stack[tok->depth].state = json_tokener_state_eatws;
			goto redo_char;
		}
		break;
		case json_tokener_state_null: { /* aka starts with 'n' */
			printbuf_memappend_fast(tok->pb, &c, 1);

            register int size = json_min(tok->st_pos + 1, 3);
            register int size_nan = json_min(tok->st_pos + 1, json_nan_str_len);
			if ((!((unsigned)tok->flags & JSON_TOKENER_STRICT) &&
			     strncasecmp("null", printbuf_getBuf(tok->pb), (unsigned long)size) == 0) ||
			    (strncmp("null", printbuf_getBuf(tok->pb), (unsigned long)size) == 0)) {
				if (tok->st_pos == 3) {
					current = NULL;
					saved_state = json_tokener_state_finish;
                    tok->stack[tok->depth].state = json_tokener_state_eatws;
					goto redo_char;
				}
			}
			else if ((!((unsigned)tok->flags & JSON_TOKENER_STRICT) &&
			          strncasecmp(json_nan_str, printbuf_getBuf(tok->pb), (unsigned long)size_nan) == 0) ||
			         (strncmp(json_nan_str, printbuf_getBuf(tok->pb), (unsigned long)size_nan) == 0)) {
				if (tok->st_pos == json_nan_str_len) {
					current = json_object_new_double((double)NAN);
					if (current == NULL)
						goto out;

					saved_state = json_tokener_state_finish;
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
				goto out;
			}
			printbuf_memappend_fast(tok->pb, &c, 1);
			break;

		case json_tokener_state_comment: {
			/* Advance until we change state */
			const char* case_start = str;
			while (c != '*') {
				if (!ADVANCE_CHAR(str, tok) || !PEEK_CHAR(c, tok)) {
					printbuf_memappend_fast(tok->pb, case_start,
					                        str - case_start);
					goto out;
				}
			}
			printbuf_memappend_fast(tok->pb, case_start, 1 + str - case_start);
            tok->stack[tok->depth].state = json_tokener_state_comment_end;
		}
		break;

		case json_tokener_state_comment_eol: {
			/* Advance until we change state */
			const char *case_start = str;
			while (c != '\n') {
				if (!ADVANCE_CHAR(str, tok) || !PEEK_CHAR(c, tok)) {
					printbuf_memappend_fast(tok->pb, case_start,
					                        str - case_start);
					goto out;
				}
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

		case json_tokener_state_string: {
			/* Advance until we change state */
			const char *case_start = str;
			while (1) {
				if (c == tok->quote_char) {
					printbuf_memappend_fast(tok->pb, case_start,
					                        str - case_start);
					current = json_object_new_string_len(printbuf_getBuf(tok->pb), printbuf_getPos(tok->pb));
					if (current == NULL)
						goto out;
					saved_state = json_tokener_state_finish;
                    tok->stack[tok->depth].state = json_tokener_state_eatws;
					break;
				} else if (c == '\\') {
					printbuf_memappend_fast(tok->pb, case_start,
					                        str - case_start);
					saved_state = json_tokener_state_string;
                    tok->stack[tok->depth].state = json_tokener_state_string_escape;
					break;
				}
				if (!ADVANCE_CHAR(str, tok) || !PEEK_CHAR(c, tok)) {
					printbuf_memappend_fast(tok->pb, case_start,
					                        str - case_start);
					goto out;
				}
			}
		}
		break;

		case json_tokener_state_string_escape:
			switch (c) {
			case '"':
			case '\\':
			case '/':
				printbuf_memappend_fast(tok->pb, &c, 1);
				tok->stack[tok->depth].state = saved_state;
				break;
			case 'b':
			case 'n':
			case 'r':
			case 't':
			case 'f':
				if (c == 'b')
					printbuf_memappend_fast(tok->pb, "\b", 1);
				else if (c == 'n')
					printbuf_memappend_fast(tok->pb, "\n", 1);
				else if (c == 'r')
					printbuf_memappend_fast(tok->pb, "\r", 1);
				else if (c == 't')
					printbuf_memappend_fast(tok->pb, "\t", 1);
				else if (c == 'f')
					printbuf_memappend_fast(tok->pb, "\f", 1);
				tok->stack[tok->depth].state = saved_state;
				break;
			case 'u':
				tok->ucs_char = 0;
				tok->st_pos = 0;
				tok->stack[tok->depth].state = json_tokener_state_escape_unicode;
				break;
			default: tok->err = json_tokener_error_parse_string; goto out;
			}
			break;

			// ===================================================

		case json_tokener_state_escape_unicode: {
            static const char json_hex_chars[22] = "0123456789abcdefABCDEF";

			/* Handle a 4-byte \uNNNN sequence, or two sequences if a surrogate pair */
			while (1) {
				if (!c || !strchr(json_hex_chars, c)) {
					tok->err = json_tokener_error_parse_string;
					goto out;
				}
				tok->ucs_char |= ((unsigned)jt_hexdigit(c) << ((3U - (unsigned)tok->st_pos) * 4U));
				tok->st_pos++;
				if (tok->st_pos >= 4)
					break;

				(void)ADVANCE_CHAR(str, tok);
				if (!PEEK_CHAR(c, tok)) {
					/*
					 * We're out of characters in the current call to
					 * json_tokener_parse(), but a subsequent call might
					 * provide us with more, so leave our current state
					 * as-is (including tok->high_surrogate) and return.
					 */
					goto out;
				}
			}
			tok->st_pos = 0;

			/* Now, we have a full \uNNNN sequence in tok->ucs_char */

			/* If the *previous* sequence was a high surrogate ... */
			if (tok->high_surrogate) {
				if (IS_LOW_SURROGATE(tok->ucs_char)) {
					/* Recalculate the ucs_char, then fall thru to process normally */
					tok->ucs_char = DECODE_SURROGATE_PAIR(tok->high_surrogate,
					                                      tok->ucs_char);
				} else {
				    /* High surrogate was not followed by a low surrogate
					 * Replace the high and process the rest normally
					 */
				    printbuf_memappend_fast(tok->pb, (char *)utf8_replacement_char, 3);
				}
				tok->high_surrogate = 0;
			}

			if (tok->ucs_char < 0x80) {
				unsigned char unescaped_utf[1] = { (unsigned char)tok->ucs_char };

				printbuf_memappend_fast(tok->pb, (char *)unescaped_utf, 1);
			} else if (tok->ucs_char < 0x800) {
			    unsigned char unescaped_utf[2];
				unescaped_utf[0] = 0xc0U | (unsigned char)(tok->ucs_char >> 6U);
				unescaped_utf[1] = 0x80U | (tok->ucs_char & 0x3fU);

				printbuf_memappend_fast(tok->pb, (char *)unescaped_utf, 2);
			} else if (IS_HIGH_SURROGATE(tok->ucs_char)) {
				/*
				 * The next two characters should be \u, HOWEVER,
				 * we can't simply peek ahead here, because the
				 * characters we need might not be passed to us
				 * until a subsequent call to json_tokener_parse.
				 * Instead, transition throug a couple of states.
				 * (now):
				 *   _escape_unicode => _unicode_need_escape
				 * (see a '\\' char):
				 *   _unicode_need_escape => _unicode_need_u
				 * (see a 'u' char):
				 *   _unicode_need_u => _escape_unicode
				 *      ...and we'll end up back around here.
				 */
				tok->high_surrogate = tok->ucs_char;
				tok->ucs_char = 0;
                tok->stack[tok->depth].state = json_tokener_state_escape_unicode_need_escape;
				break;
			}
			else if (IS_LOW_SURROGATE(tok->ucs_char)) {
				/* Got a low surrogate not preceded by a high */
				printbuf_memappend_fast(tok->pb, (char *)utf8_replacement_char, 3);
			} else if (tok->ucs_char < 0x10000) {
			    unsigned char unescaped_utf[3];
				unescaped_utf[0] = 0xe0U | ((unsigned char)tok->ucs_char >> 12U);
				unescaped_utf[1] = 0x80U | ((tok->ucs_char >> 6U) & 0x3fU);
				unescaped_utf[2] = 0x80U | (tok->ucs_char & 0x3fU);

				printbuf_memappend_fast(tok->pb, (char *)unescaped_utf, 3);
			} else if (tok->ucs_char < 0x110000) {
				unsigned char unescaped_utf[4];
				unescaped_utf[0] = 0xf0U | ((tok->ucs_char >> 18U) & 0x07U);
				unescaped_utf[1] = 0x80U | ((tok->ucs_char >> 12U) & 0x3fU);
				unescaped_utf[2] = 0x80U | ((tok->ucs_char >> 6U) & 0x3fU);
				unescaped_utf[3] = 0x80U | (tok->ucs_char & 0x3fU);

				printbuf_memappend_fast(tok->pb, (char *)unescaped_utf, 4);
			} else {
				/* Don't know what we got--insert the replacement char */
				printbuf_memappend_fast(tok->pb, (char *)utf8_replacement_char, 3);
			}
            tok->stack[tok->depth].state = saved_state; // i.e. _state_string or _state_object_field
		}
		break;

		case json_tokener_state_escape_unicode_need_escape:
			/* We get here after processing a high_surrogate
			 * require a '\\' char
			 */
			if (!c || c != '\\') {
				/* Got a high surrogate without another sequence following
				 * it.  Put a replacement char in for the high surrogate
				 * and pop back up to _state_string or _state_object_field.
				 */
				printbuf_memappend_fast(tok->pb, (char *)utf8_replacement_char, 3);
				tok->high_surrogate = 0;
				tok->ucs_char = 0;
				tok->st_pos = 0;
                tok->stack[tok->depth].state = saved_state;
				goto redo_char;
			}
			tok->stack[tok->depth].state = json_tokener_state_escape_unicode_need_u;
			break;

		case json_tokener_state_escape_unicode_need_u:
			/* We already had a \ char, check that it's \u */
			if (!c || c != 'u') {
				/* Got a high surrogate with some non-unicode escape
				 * sequence following it.
				 * Put a replacement char in for the high surrogate
				 * and handle the escape sequence normally.
				 */
				printbuf_memappend_fast(tok->pb, (char *)utf8_replacement_char, 3);
				tok->high_surrogate = 0;
				tok->ucs_char = 0;
				tok->st_pos = 0;
                tok->stack[tok->depth].state = json_tokener_state_string_escape;
				goto redo_char;
			}
			tok->stack[tok->depth].state = json_tokener_state_escape_unicode;
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
					char *last_saved_char = printbuf_get_posBufInPos(tok->pb, printbuf_getPos(tok->pb) - 1);
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

			while (c && ((c >= '0' && c <= '9') ||
			             (!is_exponent && (c == 'e' || c == 'E')) ||
			             (neg_sign_ok && c == '-') || (pos_sign_ok && c == '+') ||
			             (!tok->is_double && c == '.'))) {
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
					tok->is_double = 1;
					pos_sign_ok = 1U;
					neg_sign_ok = 1U;
					break;
				case 'e': /* FALLTHRU */
				case 'E':
					is_exponent = 1U;
					tok->is_double = 1;
					/* the exponent part can begin with a negative sign */
					pos_sign_ok = neg_sign_ok = 1U;
					break;
				default:
				    break;
				}

				if (!ADVANCE_CHAR(str, tok) || !PEEK_CHAR(c, tok)) {
					printbuf_memappend_fast(tok->pb, case_start, case_len);
					goto out;
				}
			}
			/*
				Now we know c isn't a valid number char, but check whether
				it might have been intended to be, and return a potentially
				more understandable error right away.
				However, if we're at the top-level, use the number as-is
			    because c can be part of a new object to parse on the
				next call to json_tokener_parse().
			 */
			if (tok->depth > 0 && c != ',' && c != ']' && c != '}' && c != '/' &&
			    c != 'I' && c != 'i' && !isspace((unsigned char)c)) {
				tok->err = json_tokener_error_parse_number;
				goto out;
			}
			if (case_len > 0)
				printbuf_memappend_fast(tok->pb, case_start, case_len);

			/* Check for -Infinity */
			if ((printbuf_getBuf(tok->pb)[0] == '-') && (case_len <= 1) && (c == 'i' || c == 'I')) {
                tok->stack[tok->depth].state = json_tokener_state_inf;
				tok->st_pos = 0;
				goto redo_char;
			}
			if (tok->is_double && !((unsigned)tok->flags & JSON_TOKENER_STRICT)) {
				/* Trim some chars off the end, to allow things
				   like "123e+" to parse ok. */
				while (printbuf_getPos(tok->pb) > 1) {
					char last_char = printbuf_getBuf(tok->pb)[printbuf_getPos(tok->pb) - 1];
					if (last_char != 'e' && last_char != 'E' &&
					    last_char != '-' && last_char != '+') {
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
				if (!tok->is_double && (printbuf_getBuf(tok->pb)[0] == '-') &&
				    _json_parse_int64(printbuf_getBuf(tok->pb), &num64) == 0) {
					current = json_object_new_int64(num64);
					if (current == NULL)
						goto out;
				}
				else if ((!tok->is_double) && (printbuf_getBuf(tok->pb)[0] != '-') && (_json_parse_uint64(printbuf_getBuf(tok->pb), &numuint64) == 0)) {
					if (numuint64 && (printbuf_getBuf(tok->pb)[0] == '0') && ((unsigned)tok->flags & JSON_TOKENER_STRICT)) {
						tok->err = json_tokener_error_parse_number;
						goto out;
					}
					if (numuint64 <= INT64_MAX) {
						num64 = (long long)numuint64;
						current = json_object_new_int64(num64);
						if (current == NULL)
							goto out;
					} else {
						current = json_object_new_uint64(numuint64);
						if (current == NULL)
							goto out;
					}
				}
				else if ((tok->is_double) && (json_tokener_parse_double(printbuf_getBuf(tok->pb), printbuf_getPos(tok->pb), &numd) == 0)) {
					current = json_object_new_double_s(numd, printbuf_getBuf(tok->pb));
					if (current == NULL)
						goto out;
				} else {
					tok->err = json_tokener_error_parse_number;
					goto out;
				}
				saved_state = json_tokener_state_finish;
                tok->stack[tok->depth].state = json_tokener_state_eatws;
				goto redo_char;
			}
			break;

		case json_tokener_state_array_after_sep:
		case json_tokener_state_array:
			if (c == ']') {
				// Minimize memory usage; assume parsed objs are unlikely to be changed
				json_object_array_shrink(current, 0);

				if (tok->stack[tok->depth].state == json_tokener_state_array_after_sep &&
				    ((unsigned)tok->flags & JSON_TOKENER_STRICT)) {
					tok->err = json_tokener_error_parse_unexpected;
					goto out;
				}
				saved_state = json_tokener_state_finish;
                tok->stack[tok->depth].state = json_tokener_state_eatws;
			} else {
				if (tok->depth >= tok->max_depth - 1) {
					tok->err = json_tokener_error_depth;
					goto out;
				}
                tok->stack[tok->depth].state = json_tokener_state_array_add;
				tok->depth++;
				json_tokener_reset_level(tok, tok->depth);
				goto redo_char;
			}
			break;

		case json_tokener_state_array_add:
			if (json_object_array_add(current, obj) != 0)
				goto out;

			saved_state = json_tokener_state_array_sep;
			tok->stack[tok->depth].state = json_tokener_state_eatws;
			goto redo_char;

		case json_tokener_state_array_sep:
			if (c == ']') {
				// Minimize memory usage; assume parsed objs are unlikely to be changed
				json_object_array_shrink(current, 0);

				saved_state = json_tokener_state_finish;
                tok->stack[tok->depth].state = json_tokener_state_eatws;
			} else if (c == ',') {
				saved_state = json_tokener_state_array_after_sep;
                tok->stack[tok->depth].state = json_tokener_state_eatws;
			} else {
				tok->err = json_tokener_error_parse_array;
				goto out;
			}
			break;

		case json_tokener_state_object_field_start:
		case json_tokener_state_object_field_start_after_sep:
			if (c == '}') {
				if (tok->stack[tok->depth].state == json_tokener_state_object_field_start_after_sep &&
				    ((unsigned)tok->flags & JSON_TOKENER_STRICT)) {
					tok->err = json_tokener_error_parse_unexpected;
					goto out;
				}
				saved_state = json_tokener_state_finish;
                tok->stack[tok->depth].state = json_tokener_state_eatws;
			} else if (c == '"' || c == '\'') {
				tok->quote_char = c;
				printbuf_reset(tok->pb);
                tok->stack[tok->depth].state = json_tokener_state_object_field;
			} else {
				tok->err = json_tokener_error_parse_object_key_name;
				goto out;
			}
			break;

		case json_tokener_state_object_field: {
			/* Advance until we change state */
			const char *case_start = str;
			while (1) {
				if (c == tok->quote_char) {
					printbuf_memappend_fast(tok->pb, case_start,
					                        str - case_start);

					obj_field_name = strdup(printbuf_getBuf(tok->pb));
					saved_state = json_tokener_state_object_field_end;
                    tok->stack[tok->depth].state = json_tokener_state_eatws;
					break;
				} else if (c == '\\') {
					printbuf_memappend_fast(tok->pb, case_start,
					                        str - case_start);

					saved_state = json_tokener_state_object_field;
                    tok->stack[tok->depth].state = json_tokener_state_string_escape;
					break;
				}
				if (!ADVANCE_CHAR(str, tok) || !PEEK_CHAR(c, tok)) {
					printbuf_memappend_fast(tok->pb, case_start,
					                        str - case_start);
					goto out;
				}
			}
		}
		break;

		case json_tokener_state_object_field_end:
			if (c == ':') {
				saved_state = json_tokener_state_object_value;
                tok->stack[tok->depth].state = json_tokener_state_eatws;
			} else {
				tok->err = json_tokener_error_parse_object_key_sep;
				goto out;
			}
			break;

		case json_tokener_state_object_value:
			if (tok->depth >= tok->max_depth - 1) {
				tok->err = json_tokener_error_depth;
				goto out;
			}
			tok->stack[tok->depth].state = json_tokener_state_object_value_add;
			tok->depth++;
			json_tokener_reset_level(tok, tok->depth);
			goto redo_char;

		case json_tokener_state_object_value_add:
			json_object_object_add(current, obj_field_name, obj);
			free(obj_field_name);
			obj_field_name = NULL;
			saved_state = json_tokener_state_object_sep;
			tok->stack[tok->depth].state = json_tokener_state_eatws;
			goto redo_char;

		case json_tokener_state_object_sep:
			/* { */
			if (c == '}') {
				saved_state = json_tokener_state_finish;
                tok->stack[tok->depth].state = json_tokener_state_eatws;
			} else if (c == ',') {
				saved_state = json_tokener_state_object_field_start_after_sep;
                tok->stack[tok->depth].state = json_tokener_state_eatws;
			} else {
				tok->err = json_tokener_error_parse_object_value_sep;
				goto out;
			}
			break;
		}
		(void)ADVANCE_CHAR(str, tok);
		if (!c) // This is the char *before* advancing
			break;
	} /* while(PEEK_CHAR) */

out:
	if (((unsigned)tok->flags & JSON_TOKENER_VALIDATE_UTF8) && (nBytes != 0)) {
		tok->err = json_tokener_error_parse_utf8_string;
	}
	if (c && (tok->stack[tok->depth].state == json_tokener_state_finish) && (tok->depth == 0) &&
	    ((unsigned)tok->flags & (JSON_TOKENER_STRICT | JSON_TOKENER_ALLOW_TRAILING_CHARS)) ==
	        JSON_TOKENER_STRICT) {
		/* unexpected char after JSON data */
		tok->err = json_tokener_error_parse_unexpected;
	}
	if (!c) {
		/* We hit an eof char (0) */
		if (tok->stack[tok->depth].state != json_tokener_state_finish && saved_state != json_tokener_state_finish)
			tok->err = json_tokener_error_parse_eof;
	}

	setlocale(LC_NUMERIC, oldlocale);
	free(oldlocale);

	if (tok->err == json_tokener_success) {
		json_object *ret = json_object_get(current);

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
