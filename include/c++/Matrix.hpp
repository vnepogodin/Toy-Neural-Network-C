// Matrix lib
#ifndef MATRIX_HPP_
#define MATRIX_HPP_

#include "third_party/json/simdjson.h"  // simdjson::

using std::to_string;
using std::string;

class Matrix {
 public:
    using value_type = float;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = typename std::reverse_iterator<pointer>;
    using const_reverse_iterator = typename std::reverse_iterator<const_pointer>;

    // Constructors.
    inline constexpr Matrix() = default;
    Matrix(Matrix&&) = default;
    Matrix(const uint32_t&, const uint32_t&);
    Matrix(const Matrix&);

    // Destructor.
    inline ~Matrix() = default;

    // Operators.
    auto operator=(Matrix&&) -> Matrix& = default;
    auto operator+=(const Matrix&) -> Matrix&;
    auto operator+=(const float&) -> Matrix&;
    auto operator*=(const Matrix&) -> Matrix&;
    auto operator*=(const float&) -> Matrix&;


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

    // Element access.
    constexpr auto data() noexcept -> pointer
    { return Matrix::S_ptr(elem); }
    constexpr auto data() const noexcept -> const_pointer
    { return Matrix::S_ptr(elem); }

    constexpr auto
    operator()(const size_t& x, const size_t& y) noexcept -> reference
    { return Matrix::S_ref(elem, this->pos(x, y)); }

    constexpr auto
    operator()(const size_t& x, const size_t& y) const noexcept -> const_reference
    { return Matrix::S_ref(elem, this->pos(x, y)); }


    // Functions.
    auto toArray() const noexcept -> float*;
    void randomize();
    void map(float (*const &)(float));
    auto serialize() const noexcept -> string;

    // Static functions.
    static auto fromArray(const float* const&, const uint32_t&) -> Matrix;
    static auto transpose(const Matrix&) -> Matrix;
    static auto multiply(const Matrix&, const Matrix&) -> Matrix;
    static auto subtract(const Matrix&, const Matrix&) -> Matrix;
    static auto map(const Matrix&, float (*const &)(float)) -> Matrix;
    static auto deserialize(const simdjson::dom::element&) -> Matrix;

    // Delete.
    auto operator=(const Matrix&) -> Matrix& = delete;

 private:
    // Variables.
    uint32_t len {};
    uint32_t rows {};
    uint32_t columns {};

    float* elem {nullptr};

    constexpr auto
    operator[](const size_t& idx) noexcept -> reference
    { return Matrix::S_ref(elem, idx); }

    constexpr auto
    operator[](const size_t& idx) const noexcept -> const_reference
    { return Matrix::S_ref(elem, idx); }


    // Function.
    void allocSpace();

    constexpr auto
    pos(const uint32_t& x, const uint32_t& y) const -> uint32_t {
        return ((x < rows) && (y < columns)) ? ((columns * x) + y)
            : (throw std::overflow_error("out of bounds"), 0);
    }

    static constexpr auto
    S_ref(const pointer& _t, const size_t& _n) noexcept -> reference
    { return static_cast<reference>(_t[_n]); }

    static constexpr auto
    S_ptr(const pointer& _t) noexcept -> pointer
    { return static_cast<pointer>(_t); }
};

#endif  // MATRIX_HPP_
