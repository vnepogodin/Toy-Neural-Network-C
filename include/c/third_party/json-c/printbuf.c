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
#include "printbuf.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

struct _Printbuf {
    int bpos;
    int size;

    char* buf;
};

void printbuf_memcpy(printbuf *__pb_param, const void* __src_param, const unsigned long __size_param) {
    memcpy(__pb_param->buf + __pb_param->bpos, __src_param, __size_param);
}

inline int printbuf_getSize(printbuf *__pb_param) {
    return __pb_param->size;
}
inline int printbuf_getPos(printbuf *__pb_param) {
    return __pb_param->bpos;
}
inline char* printbuf_getBuf(printbuf *__pb_param) {
    return __pb_param->buf;
}
inline char* printbuf_get_posBufInPos(printbuf *__pb_param, const int __pos_param) {
    return &__pb_param->buf[__pos_param];
}

inline void printbuf_addPos(printbuf *__pb_param, const int __value_param) {
    __pb_param->bpos += __value_param;
}
inline void printbuf_subPos(printbuf *__pb_param, const int __value_param) {
    __pb_param->bpos -= __value_param;
}

inline void printbuf_setBufInPos(printbuf *__pb_param, const int __pos_param, const char __value_param) {
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
static int printbuf_extend(printbuf *p, const int min_size) {
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

    const char* t = (char *)realloc(p->buf, (unsigned long)new_size);
    if (t == NULL)
        return -1;

    p->size = new_size;
    p->buf = (char *)t;
    return 0;
}

printbuf* printbuf_new(void) {
#ifdef DEBUG
    printbuf *p = NULL;

    /* assumed 0.0005MB page sizes */
    posix_memalign((void **)&p, 512UL, 512UL);
#else
    printbuf *p = (printbuf *)calloc(1UL, sizeof(printbuf));
#endif

	if (p == NULL)
		return NULL;

	p->size = 32;
	p->bpos = 0;
    p->buf = (char *)malloc((unsigned long)p->size);
	if (p->buf == NULL) {
		free(p);
		return NULL;
	}

	p->buf[0] = '\0';
	return p;
}

inline int printbuf_memappend(printbuf *p, const char* buf, const int size) {
	/* Prevent signed integer overflows with large buffers. */
	if (size > (INT_MAX - p->bpos - 1))
		return -1;

	if (p->size <= (p->bpos + size + 1)) {
		if (printbuf_extend(p, p->bpos + size + 1) < 0)
			return -1;
	}

	memcpy(p->buf + p->bpos, buf, (unsigned long)size);
	p->bpos += size;
	p->buf[p->bpos] = '\0';
	return size;
}

int printbuf_memset(printbuf *pb, int offset, const int charvalue, const int len) {
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

	memset(pb->buf + offset, charvalue, (unsigned long)len);
	if (pb->bpos < size_needed)
		pb->bpos = size_needed;

	return 0;
}

inline void printbuf_reset(printbuf *p) {
	p->buf[0] = '\0';
	p->bpos = 0;
}

inline void printbuf_free(printbuf *p) {
	if (p != NULL) {
		free(p->buf);
		free(p);
	}
}
