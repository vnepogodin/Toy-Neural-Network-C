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

/**
 * @file
 * @brief Methods to parse an input string into a tree of json_object objects.
 */
#ifndef __JSON_TOKENER_H__
#define __JSON_TOKENER_H__

#include "json_object.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Parse a json_object out of the string `str`.
 *
 */
json_object* json_tokener_parse(const char*);

#ifdef __cplusplus
}
#endif

#endif /* __JSON_TOKENER_H__ */
