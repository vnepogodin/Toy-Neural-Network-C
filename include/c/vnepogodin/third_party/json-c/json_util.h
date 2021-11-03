/*
 * $Id: json_util.h,v 1.4 2006/01/30 23:07:57 mclark Exp $
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
 * @brief Miscllaneous utility functions and macros.
 */

#ifndef __JSON_UTIL_H__
#define __JSON_UTIL_H__

#include <vnepogodin/third_party/json-c/json_object.h>

#ifdef __cplusplus
extern "C" {
#endif

/* utility functions */
/**
 * Read the full contents of the given file, then convert it to a
 * json_object using json_tokener_parse().
 *
 * Returns NULL on failure.
 */
extern json_object* json_object_from_file(const char*);

/**
 * Create a JSON object from already opened file descriptor.
 *
 * This function can be helpful, when you opened the file already,
 * e.g. when you have a temp file.
 * Note, that the fd must be readable at the actual position, i.e.
 * use lseek(fd, 0, SEEK_SET) before.
 *
 * The depth argument specifies the maximum object depth to pass to
 * json_tokener_new_ex().  When depth == -1, JSON_TOKENER_DEFAULT_DEPTH
 * is used instead.
 *
 * Returns NULL on failure.
 */
json_object* json_object_from_fd_ex(const int, const int);

/**
 * Create a JSON object from an already opened file descriptor, using
 * the default maximum object depth. (JSON_TOKENER_DEFAULT_DEPTH)
 *
 * See json_object_from_fd_ex() for details.
 */
extern json_object* json_object_from_fd(const int);


extern json_object* json_object_from_fd_many(const int, const int);
extern json_object* json_object_from_file_many(const char*);

#ifdef __cplusplus
}
#endif

#endif
