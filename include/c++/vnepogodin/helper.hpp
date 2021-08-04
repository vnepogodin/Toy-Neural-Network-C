// Helper macroses
#ifndef HELPER_HPP_
#define HELPER_HPP_

#if __has_include(<memory_resource>) && __cplusplus >= 201703L
#include <memory_resource>  // std::pmr::vector
#endif

#include <vector>  // std::vector

namespace vnepogodin {
#ifdef _MSC_VER
#define tnn_really_inline __forceinline

#ifndef tnn_likely
#define tnn_likely(x) x
#endif
#ifndef tnn_unlikely
#define tnn_unlikely(x) x
#endif
#else
#define tnn_really_inline inline __attribute__((always_inline))

#ifndef tnn_likely
#define tnn_likely(x) __builtin_expect(!!(x), 1)
#endif
#ifndef tnn_unlikely
#define tnn_unlikely(x) __builtin_expect(!!(x), 0)
#endif
#endif  // _MSC_VER

#if __has_include(<memory_resource>) && __cplusplus >= 201703L
template <class T>
using vector = std::vector<T, std::pmr::polymorphic_allocator<T>>;
#else
template <class T>
using vector = std::vector<T>;
#endif
}  // namespace vnepogodin

#endif  // HELPER_HPP_
