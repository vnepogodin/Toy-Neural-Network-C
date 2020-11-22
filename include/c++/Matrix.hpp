// Matrix lib
#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include "third_party/json/simdjson.h"  // simdjson::

using std::to_string;
using std::string;

class Matrix {
 public:
    using iterator = float*;
    using const_iterator = const float*;

    typedef std::reverse_iterator<const_iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;


    // Constructors
    Matrix(const int32_t&, const int32_t&);
    constexpr Matrix();
    Matrix(const Matrix&);

    // Destructor
    inline ~Matrix() = default;

    // Operators
    auto operator=(Matrix&&) -> Matrix& = default;
    auto operator=(const Matrix&) -> Matrix& = delete;
    auto operator+=(const Matrix&) -> Matrix&;
    auto operator+=(const float&) -> Matrix&;
    auto operator*=(const Matrix&) -> Matrix&;
    auto operator*=(const float&) -> Matrix&;
    auto operator[](const size_t& idx) -> iterator;

    // Non member operator
    friend auto operator<<(std::ostream&, const Matrix&) -> std::ostream&;

    // Functions
    constexpr auto begin() const noexcept -> iterator {
        return &this->data[0];
    }
    constexpr auto end() const noexcept -> iterator {
        return this->iter;
    }
    constexpr auto cbegin() const noexcept -> const_iterator {
        return static_cast<const_iterator>(&this->data[0]);
    }
    constexpr auto cend() const noexcept -> const_iterator {
        return static_cast<const_iterator>(this->iter);
    }

    constexpr auto rbegin() const noexcept -> const_reverse_iterator {
        return const_reverse_iterator(this->end());
    }
    constexpr auto rend() const noexcept -> const_reverse_iterator {
        return const_reverse_iterator(this->begin());
    }
    constexpr auto crbegin() const noexcept -> const_reverse_iterator {
        return rbegin();
    }
    constexpr auto crend() const noexcept -> const_reverse_iterator {
        return rend();
    }

    auto toArray() const noexcept -> float*;
    void randomize();
    void map(float (*const &)(float));
    auto serialize() const noexcept -> const string;

    // Static functions
    static auto fromArray(const float* const&, const int&) -> Matrix;
    static auto transpose(const Matrix&) -> Matrix;
    static auto multiply(const Matrix&, const Matrix&) -> Matrix;
    static auto subtract(const Matrix&, const Matrix&) -> Matrix;
    static auto map(const Matrix&, float (*const &)(float)) -> Matrix;
    static auto deserialize(const simdjson::dom::element&) -> Matrix;

 private:
    iterator iter;
    // Variables
    int32_t len;

    int32_t rows, columns;

    float* data;

    // Function
    void allocSpace();
};

#endif  // __MATRIX_HPP__
