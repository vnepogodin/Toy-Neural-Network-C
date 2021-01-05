// Matrix lib
#ifndef MATRIX_HPP_
#define MATRIX_HPP_

#include <vnepogodin/third_party/json/simdjson.hpp>  // simdjson::dom::element

#include <iosfwd>     // std::ostream
#include <iterator>   // std::reverse_iterator
#include <stdexcept>  // std::overflow_error
#include <string>     // std::string, std::to_string

namespace vnepogodin {
using std::string;
using std::to_string;

class Matrix {
 public:
    using value_type = double;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = value_type*;
    using const_iterator = const value_type*;
    using reverse_iterator = typename std::reverse_iterator<pointer>;
    using const_reverse_iterator = typename std::reverse_iterator<const_pointer>;

    using function_t = double (*)(double);

    // Constructors.
    constexpr Matrix() = default;
    Matrix(Matrix&&) = default;
    Matrix(const std::uint32_t&, const std::uint32_t&);
    Matrix(const Matrix&);

    // Destructor.
    inline ~Matrix() = default;

    // Operators.
    auto operator=(Matrix&&) -> Matrix& = default;
    auto operator+=(const Matrix&) -> Matrix&;
    auto operator+=(const_reference) -> Matrix&;
    auto operator*=(const Matrix&) -> Matrix&;
    auto operator*=(const_reference) -> Matrix&;

    // Non member operator.
    friend auto operator<<(std::ostream&, const Matrix&) -> std::ostream&;

    // Iterators.
    constexpr auto begin() noexcept -> iterator
    { return iterator(data()); }

    constexpr auto begin() const noexcept -> const_iterator
    { return const_iterator(data()); }

    constexpr auto end() noexcept -> iterator
    { return iterator(data() + len); }

    constexpr auto end() const noexcept -> const_iterator
    { return const_iterator(data() + len); }

    auto rbegin() noexcept -> reverse_iterator
    { return reverse_iterator(end()); }

    auto rbegin() const noexcept -> const_reverse_iterator
    { return const_reverse_iterator(end()); }

    auto rend() noexcept -> reverse_iterator
    { return reverse_iterator(begin()); }

    auto rend() const noexcept -> const_reverse_iterator
    { return const_reverse_iterator(begin()); }

    constexpr auto cbegin() const noexcept -> const_iterator
    { return const_iterator(begin()); }

    constexpr auto cend() const noexcept -> const_iterator
    { return const_iterator(end()); }

    auto crbegin() const noexcept -> const_reverse_iterator
    { return const_reverse_iterator(end()); }

    auto crend() const noexcept -> const_reverse_iterator
    { return const_reverse_iterator(begin()); }

    // Capacity.
    constexpr auto
    size() const noexcept -> std::uint32_t { return len; }

    constexpr auto
    max_size() const noexcept -> std::uint32_t { return len; }

    constexpr auto
    empty() const noexcept -> bool { return size() == 0; }

    // Element access.
    constexpr auto data() noexcept -> pointer
    { return Matrix::S_ptr(elem); }
    constexpr auto data() const noexcept -> const_pointer
    { return Matrix::S_ptr(elem); }

    constexpr auto
    operator()(const std::uint32_t& x, const std::uint32_t& y) noexcept -> reference
    { return Matrix::S_ref(elem, pos(x, y)); }

    constexpr auto
    operator()(const std::uint32_t& x, const std::uint32_t& y) const noexcept -> const_reference
    { return Matrix::S_ref(elem, pos(x, y)); }

    constexpr auto
    at(const std::uint32_t& x, const std::uint32_t& y) -> reference {
        if ((x >= rows) && (y >= columns)) {
            throw std::overflow_error("out of bounds");
        }
        return Matrix::S_ref(elem, pos(x, y));
    }

    constexpr auto
    at(const std::uint32_t& x, const std::uint32_t& y) const -> const_reference {
        return ((x < rows) && (y < columns)) ? Matrix::S_ref(elem, pos(x, y))
                                             : (throw std::overflow_error("out of bounds")),
               Matrix::S_ref(elem, 0);
    }

    // Functions.
    auto toArray() const noexcept -> pointer;
    void randomize();
    constexpr void map(const function_t& func) {
        // Apply a function to every element of matrix
        for (auto& el : *this) {
            el = func(el);
        }
    }
    auto serialize() const noexcept -> string;

    // Static functions.
    static auto fromArray(const_pointer const&, const std::uint32_t&) -> Matrix;
    static auto transpose(const Matrix&) -> Matrix;
    static auto multiply(const Matrix&, const Matrix&) -> Matrix;
    static auto subtract(const Matrix&, const Matrix&) -> Matrix;
    static auto map(const Matrix&, const function_t&) -> Matrix;
    static auto deserialize(const simdjson::dom::element&) -> Matrix;

    // Delete.
    auto operator=(const Matrix&) -> Matrix& = delete;

 private:
    // Variables.
    std::uint32_t len{};
    std::uint32_t rows{};
    std::uint32_t columns{};

    pointer elem{nullptr};

    constexpr auto
    operator[](const std::uint32_t& idx) noexcept -> reference
    { return Matrix::S_ref(elem, idx); }

    constexpr auto
    operator[](const std::uint32_t& idx) const noexcept -> const_reference
    { return Matrix::S_ref(elem, idx); }

    // Functions.
    constexpr auto
    pos(const std::uint32_t& x, const std::uint32_t& y) const noexcept -> std::uint32_t
    { return (columns * x) + y; }

    static constexpr auto
    S_ref(const pointer& _t, const std::uint32_t& _n) noexcept -> reference
    { return static_cast<reference>(_t[_n]); }

    static constexpr auto
    S_ptr(const pointer& _t) noexcept -> pointer
    { return static_cast<pointer>(_t); }
};
};  // namespace vnepogodin

#endif  // MATRIX_HPP_
