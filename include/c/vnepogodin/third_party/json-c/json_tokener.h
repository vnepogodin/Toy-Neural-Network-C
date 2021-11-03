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

#include <vnepogodin/third_party/json-c/json_object.h>

typedef struct _Json_Tokener json_tokener;

#define JSON_TOKENER_DEFAULT_DEPTH 32

/**
 * Be strict when parsing JSON input.  Use caution with
 * this flag as what is considered valid may become more
 * restrictive from one release to the next, causing your
 * code to fail on previously working input.
 *
 * Note that setting this will also effectively disable parsing
 * of multiple json objects in a single character stream
 * (e.g. {"foo":123}{"bar":234}); if you want to allow that
 * also set JSON_TOKENER_ALLOW_TRAILING_CHARS
 *
 * This flag is not set by default.
 *
 * @see json_tokener_set_flags()
 */
#define JSON_TOKENER_STRICT 0x01

/**
 * Use with JSON_TOKENER_STRICT to allow trailing characters after the
 * first parsed object.
 *
 * @see json_tokener_set_flags()
 */
#define JSON_TOKENER_ALLOW_TRAILING_CHARS 0x02

/**
 * Cause json_tokener_parse_ex() to validate that input is UTF8.
 * If this flag is specified and validation fails, then
 * json_tokener_get_error(tok) will return
 * json_tokener_error_parse_utf8_string
 *
 * This flag is not set by default.
 *
 * @see json_tokener_set_flags()
 */
#define JSON_TOKENER_VALIDATE_UTF8 0x10

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate a new json_tokener with a custom max nesting depth.
 * @see JSON_TOKENER_DEFAULT_DEPTH
 */
json_tokener* json_tokener_new_ex(const int);

/**
 * Free a json_tokener previously allocated with json_tokener_new().
 */
extern void json_tokener_free(json_tokener *);

/**
 * Parse a json_object out of the string `str`.
 */
json_object* json_tokener_parse(const char*);


/**
 * Parse a string and return a non-NULL json_object if a valid JSON value
 * is found.  The string does not need to be a JSON object or array;
 * it can also be a string, number or boolean value.
 *
 * A partial JSON string can be parsed.  If the parsing is incomplete,
 * NULL will be returned and json_tokener_get_error() will return
 * json_tokener_continue.
 * json_tokener_parse_ex() can then be called with additional bytes in str
 * to continue the parsing.
 *
 * If json_tokener_parse_ex() returns NULL and the error is anything other than
 * json_tokener_continue, a fatal error has occurred and parsing must be
 * halted.  Then, the tok object must not be reused until json_tokener_reset()
 * is called.
 *
 * When a valid JSON value is parsed, a non-NULL json_object will be
 * returned, with a reference count of one which belongs to the caller.  Also,
 * json_tokener_get_error() will return json_tokener_success. Be sure to check
 * the type with json_object_is_type() or json_object_get_type() before using
 * the object.
 *
 * Trailing characters after the parsed value do not automatically cause an
 * error.  It is up to the caller to decide whether to treat this as an
 * error or to handle the additional characters, perhaps by parsing another
 * json value starting from that point.
 *
 * If the caller knows that they are at the end of their input, the length
 * passed MUST include the final '\0' character, so values with no inherent
 * end (i.e. numbers) can be properly parsed, rather than just returning
 * json_tokener_continue.
 *
 * Extra characters can be detected by comparing the value returned by
 * json_tokener_get_parse_end() against
 * the length of the last len parameter passed in.
 *
 * The tokener does \b not maintain an internal buffer so the caller is
 * responsible for a subsequent call to json_tokener_parse_ex with an
 * appropriate str parameter starting with the extra characters.
 *
 * This interface is presently not 64-bit clean due to the int len argument
 * so the function limits the maximum string size to INT32_MAX (2GB).
 * If the function is called with len == -1 then strlen is called to check
 * the string length is less than INT32_MAX (2GB)
 *
 * Example:
 * @code
json_object *jobj = NULL;
const char *mystring = NULL;
int stringlen = 0;
enum json_tokener_error jerr;
do {
    mystring = ...  // get JSON string, e.g. read from file, etc...
    stringlen = strlen(mystring);
    if (end_of_input)
        stringlen++;  // Include the '\0' if we know we're at the end of input
    jobj = json_tokener_parse_ex(tok, mystring, stringlen);
} while ((jerr = json_tokener_get_error(tok)) == json_tokener_continue);
if (jerr != json_tokener_success)
{
    fprintf(stderr, "Error: %s\n", json_tokener_error_desc(jerr));
    // Handle errors, as appropriate for your application.
}
if (json_tokener_get_parse_end(tok) < stringlen)
{
    // Handle extra characters after parsed object as desired.
    // e.g. issue an error, parse another object from that point, etc...
}
// Success, use jobj here.
@endcode
 *
 * @param tok a json_tokener previously allocated with json_tokener_new()
 * @param str an string with any valid JSON expression, or portion of.  This does not need to be null terminated.
 * @param len the length of str
 */
json_object* json_tokener_parse_ex(json_tokener*, const char*, const int);

#ifdef __cplusplus
}
#endif

#endif  /* __JSON_TOKENER_H__ */
