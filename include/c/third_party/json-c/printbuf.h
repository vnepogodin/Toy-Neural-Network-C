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


#ifndef JSON_EXPORT
#if defined(_MSC_VER)
#define JSON_EXPORT __declspec(dllexport)
#else
#define JSON_EXPORT extern
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

JSON_EXPORT void printbuf_memcpy(printbuf *, const void*, const unsigned long);

JSON_EXPORT int   printbuf_getSize(printbuf *);
JSON_EXPORT int   printbuf_getPos(printbuf *);
JSON_EXPORT char* printbuf_getBuf(printbuf *);
JSON_EXPORT char* printbuf_get_posBufInPos(printbuf *, const int);

JSON_EXPORT void printbuf_addPos(printbuf *, const int);
JSON_EXPORT void printbuf_subPos(printbuf *, const int);

JSON_EXPORT void printbuf_setBufInPos(printbuf *, const int, const char);

printbuf* printbuf_new(void);

/* As an optimization, printbuf_memappend_fast() is defined as a macro
 * that handles copying data if the buffer is large enough; otherwise
 * it invokes printbuf_memappend() which performs the heavy
 * lifting of reallocting the buffer and copying data.
 *
 * Your code should not use printbuf_memappend() directly unless it
 * checks the return code. Use printbuf_memappend_fast() instead.
 */
JSON_EXPORT int printbuf_memappend(printbuf *, const char*, const unsigned long);

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

JSON_EXPORT void printbuf_reset(printbuf *p);

JSON_EXPORT void printbuf_free(printbuf *p);

#ifdef __cplusplus
}
#endif

#endif /* __PRINTBUF_H__ */
