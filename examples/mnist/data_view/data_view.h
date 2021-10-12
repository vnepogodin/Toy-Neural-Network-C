// Copyright (c) 2019 The DataView Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEW_DATA_VIEW_H_
#define DATA_VIEW_DATA_VIEW_H_

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <type_traits>

#include "arch.h"
#include "export.h"

namespace data_view {

namespace internal {

template <typename, typename = void>
struct is_byte_ordered_integral : std::false_type {};

template <typename T>
struct is_byte_ordered_integral<
    T, std::enable_if_t<std::is_integral<T>::value && (sizeof(T) > 1)>>
    : std::true_type {};

template <typename, typename = void>
struct is_1byte_integral : std::false_type {};

template <typename T>
struct is_1byte_integral<
    T, std::enable_if_t<std::is_integral<T>::value && (sizeof(T) == 1)>>
    : std::true_type {};

template <typename T>
struct FloatingPointTraits;

template <>
struct FloatingPointTraits<float> {
  union value_type {
    float fpvalue;
    uint32_t ivalue;
  };
};

template <>
struct FloatingPointTraits<double> {
  union value_type {
    double fpvalue;
    uint64_t ivalue;
  };
};

}  // namespace internal

#ifdef ARCH_CPU_LITTLE_ENDIAN

template <
    typename T,
    std::enable_if_t<internal::is_byte_ordered_integral<T>::value>* = nullptr>
inline void ReadBigEndian(const char* buf, T* out) {
  *out = buf[0];
  for (size_t i = 1; i < sizeof(T); ++i) {
    *out <<= 8;
    *out |= static_cast<uint8_t>(buf[i]);
  }
}

template <
    typename T,
    std::enable_if_t<internal::is_byte_ordered_integral<T>::value>* = nullptr>
inline void WriteBigEndian(char* buf, T val) {
  for (size_t i = 0; i < sizeof(T); ++i) {
    buf[sizeof(T) - i - 1] = static_cast<char>(val & 0xFF);
    val >>= 8;
  }
}

template <typename T,
          std::enable_if_t<std::is_floating_point<T>::value>* = nullptr>
inline void ReadBigEndian(const char* buf, T* out) {
  typedef typename internal::FloatingPointTraits<T>::value_type value_type;
  value_type value;
  ReadBigEndian(buf, &value.ivalue);
  *out = value.fpvalue;
}

template <typename T,
          std::enable_if_t<std::is_floating_point<T>::value>* = nullptr>
inline void WriteBigEndian(char* buf, T val) {
  typedef typename internal::FloatingPointTraits<T>::value_type value_type;
  value_type value;
  value.fpvalue = val;
  WriteBigEndian(buf, value.ivalue);
}

template <typename T,
          std::enable_if_t<internal::is_1byte_integral<T>::value>* = nullptr>
inline void ReadBigEndian(const char* buf, T* out) {
  *out = buf[0];
}

template <typename T,
          std::enable_if_t<internal::is_1byte_integral<T>::value>* = nullptr>
inline void WriteBigEndian(char* buf, T val) {
  buf[0] = static_cast<char>(val);
}

template <typename T>
inline void ReadLittleEndian(const char* buf, T* out) {
  memcpy(out, buf, sizeof(T));
}

template <typename T>
inline void WriteLittleEndian(char* buf, T val) {
  memcpy(buf, static_cast<const T*>(&val), sizeof(T));
}

#else

template <
    typename T,
    std::enable_if_t<internal::is_byte_ordered_integral<T>::value>* = nullptr>
inline void ReadLittleEndian(const char* buf, T* out) {
  *out = buf[sizeof(T) - 1];
  for (size_t i = 1; i < sizeof(T); ++i) {
    *out <<= 8;
    *out |= static_cast<uint8_t>(buf[sizeof(T) - i - 1]);
  }
}

template <
    typename T,
    std::enable_if_t<internal::is_byte_ordered_integral<T>::value>* = nullptr>
inline void WriteLittleEndian(char* buf, T val) {
  for (size_t i = 0; i < sizeof(T); ++i) {
    buf[i] = static_cast<char>(val & 0xFF);
    val >>= 8;
  }
}

template <typename T,
          std::enable_if_t<internal::is_1byte_integral<T>::value>* = nullptr>
inline void ReadLittleEndian(const char* buf, T* out) {
  *out = buf[0];
}

template <typename T,
          std::enable_if_t<internal::is_1byte_integral<T>::value>* = nullptr>
inline void WriteLittleEndian(char* buf, T val) {
  buf[0] = static_cast<char>(val);
}

template <typename T,
          std::enable_if_t<std::is_floating_point<T>::value>* = nullptr>
inline void ReadLittleEndian(const char* buf, T* out) {
  typedef typename internal::FloatingPointTraits<T>::value_type value_type;
  value_type value;
  ReadLittleEndian(buf, &value.ivalue);
  *out = value.fpvalue;
}

template <typename T,
          std::enable_if_t<std::is_floating_point<T>::value>* = nullptr>
inline void WriteLittleEndian(char* buf, T val) {
  typedef typename internal::FloatingPointTraits<T>::value_type value_type;
  value_type value;
  value.fpvalue = val;
  WriteLittleEndian(buf, value.ivalue);
}

template <typename T>
inline void ReadBigEndian(const char* buf, T* out) {
  memcpy(out, buf, sizeof(T));
}

template <typename T>
inline void WriteBigEndian(char* buf, T val) {
  memcpy(out, static_cast<const T*>(&val), sizeof(T));
}

#endif

namespace internal {

template <typename T>
bool Read(const char* data, size_t length, size_t offset, T* value,
          bool little_endian) {
  if (offset + sizeof(T) > length) return false;

  const char* ptr = data + offset;
  if (little_endian) {
    ReadLittleEndian(ptr, value);
  } else {
    ReadBigEndian(ptr, value);
  }
  return true;
}

}  // namespace internal

class DATA_VIEW_EXPORT ConstDataView {
 public:
  ConstDataView(const char* data, size_t length);
  ConstDataView(const ConstDataView& other) noexcept;
  ConstDataView& operator=(const ConstDataView& other) noexcept;

  size_t length() const;

  // Returns true if succeeded to read. (offset + sizeof(T) <= |length_|)
  template <typename T>
  bool Read(size_t offset, T* value, bool little_endian) const;

 private:
  const char* data_;
  size_t length_;
};

class DATA_VIEW_EXPORT DataView {
 public:
  DataView(char* data, size_t length);
  DataView(const DataView& other) noexcept;
  DataView& operator=(const DataView& other) noexcept;

  size_t length() const;

  // Returns true if succeeded to read. (offset + sizeof(T) <= |length_|)
  template <typename T>
  bool Read(size_t offset, T* value, bool little_endian) const;

  // Returns true if succeeded to write. (offset + sizeof(T) <= |length_|)
  template <typename T>
  bool Write(size_t offset, T value, bool little_endian);

 private:
  char* data_;
  size_t length_;
};

template <typename T>
bool ConstDataView::Read(size_t offset, T* value, bool little_endian) const {
  return internal::Read(data_, length_, offset, value, little_endian);
}

template <typename T>
bool DataView::Read(size_t offset, T* value, bool little_endian) const {
  return internal::Read(data_, length_, offset, value, little_endian);
}

template <typename T>
bool DataView::Write(size_t offset, T value, bool little_endian) {
  if (offset + sizeof(T) > length_) return false;

  char* ptr = data_ + offset;
  if (little_endian) {
    WriteLittleEndian(ptr, value);
  } else {
    WriteBigEndian(ptr, value);
  }
  return true;
}

}  // namespace data_view

#endif  // DATA_VIEW_DATA_VIEW_H_