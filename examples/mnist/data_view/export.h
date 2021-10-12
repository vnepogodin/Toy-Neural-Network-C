// Copyright (c) 2019 The DataView Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEW_EXPORT_H_
#define DATA_VIEW_EXPORT_H_

#if defined(DATA_VIEW_COMPONENT_BUILD)

#if defined(_WIN32)
#ifdef DATA_VIEW_COMPILE_LIBRARY
#define DATA_VIEW_EXPORT __declspec(dllexport)
#else
#define DATA_VIEW_EXPORT __declspec(dllimport)
#endif  // defined(DATA_VIEW_COMPILE_LIBRARY)

#else
#ifdef DATA_VIEW_COMPILE_LIBRARY
#define DATA_VIEW_EXPORT __attribute__((visibility("default")))
#else
#define DATA_VIEW_EXPORT
#endif  // defined(DATA_VIEW_COMPILE_LIBRARY)
#endif  // defined(_WIN32)

#else
#define DATA_VIEW_EXPORT
#endif  // defined(DATA_VIEW_COMPONENT_BUILD)

#endif  // DATA_VIEW_EXPORT_H_