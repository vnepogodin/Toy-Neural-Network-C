/*
 * $Id: json_util.c,v 1.4 2006/01/30 23:07:57 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#include <vnepogodin/third_party/json-c/json_util.h>
#include <vnepogodin/third_party/json-c/json_object.h>
#include <vnepogodin/third_party/json-c/json_tokener.h>
#include <vnepogodin/third_party/json-c/printbuf.h>

#include <stdio.h>
#include <string.h>

#include <fcntl.h>

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# define open _open
# include <io.h>
# include <windows.h>
#else
# include <unistd.h>
#endif /* _WIN32 */

#define JSON_FILE_BUF_SIZE 4096

inline json_object* json_object_from_fd(const int fd) {
    return json_object_from_fd_ex(fd, -1);
}

static printbuf* printbuf_from_fd_ex(const int fd) {
    printbuf* pb;
    if (!(pb = printbuf_new())) {
        return NULL;
    }

    int ret = 0;
    char buf[JSON_FILE_BUF_SIZE] = {0};
    while ((ret = read(fd, buf, JSON_FILE_BUF_SIZE)) > 0) {
        printbuf_memappend(pb, buf, ret);
    }
    if (ret < 0) {
        printbuf_free(pb);
        return NULL;
    }

    return pb;
}

json_object* json_object_from_fd_ex(const int fd, const int in_depth) {
    printbuf* pb = printbuf_from_fd_ex(fd);
    if (!pb) {
        return NULL;
    }

    int depth = JSON_TOKENER_DEFAULT_DEPTH;
    if (in_depth != -1) {
        depth = in_depth;
    }
    json_tokener* tok = json_tokener_new_ex(depth);
    if (!tok) {
        printbuf_free(pb);
        return NULL;
    }

    json_object* obj = json_tokener_parse_ex(tok, printbuf_getBuf(pb), printbuf_getPos(pb));

    json_tokener_free(tok);
    printbuf_free(pb);
    return obj;
}

json_object* json_object_from_fd_many(const int fd, const int in_depth) {
    static const char* delim = "\n";
    printbuf* pb = printbuf_from_fd_ex(fd);
    if (!pb) {
        return NULL;
    }

    int depth = JSON_TOKENER_DEFAULT_DEPTH;
    if (in_depth != -1) {
        depth = in_depth;
    }
    json_tokener* tok = json_tokener_new_ex(depth);
    if (!tok) {
        printbuf_free(pb);
        return NULL;
    }

    int newlines = 0;
    char* c = printbuf_getBuf(pb);
    while (*c != '\0') {
        if (*c == '\n') { ++newlines; }
        ++c;
    }

    char* save_token = NULL;
    json_object* nj = json_object_new_array_ext(newlines);

#ifdef _WIN32
    char* token = strtok_s(printbuf_getBuf(pb), delim, &save_token);
#else
    char* token = strtok_r(printbuf_getBuf(pb), delim, &save_token);
#endif /* _WIN32 */

    for (int i = 0; i < newlines; ++i) {
        json_object* j = json_tokener_parse_ex(tok, token, strlen(token));
        json_object_array_add(nj, j);

#ifdef _WIN32
        token = strtok_s(NULL, delim, &save_token);
#else
        token = strtok_r(NULL, delim, &save_token);
#endif /* _WIN32 */
    }

    json_tokener_free(tok);
    printbuf_free(pb);
    return nj;
}

inline json_object* json_object_from_file(const char* filename) {
    int fd = 0;
    if ((fd = open(filename, O_RDONLY)) < 0) {
        return NULL;
    }
    json_object* obj = json_object_from_fd(fd);
    close(fd);
    return obj;
}

inline json_object* json_object_from_file_many(const char* filename) {
    int fd = 0;
    if ((fd = open(filename, O_RDONLY)) < 0) {
        return NULL;
    }
    json_object* obj = json_object_from_fd_many(fd, -1);
    close(fd);
    return obj;
}
