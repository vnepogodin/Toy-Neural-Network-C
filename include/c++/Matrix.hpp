// Matrix lib
#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include "third_party/json/simdjson.h"  // simdjson::

class Matrix {
 public:
    // Constructors
    Matrix(const int32_t&, const int32_t&);
    inline Matrix();
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

    // Non member operator
    friend auto operator<<(std::ostream&, const Matrix&) -> std::ostream&;

    // Functions
    inline constexpr auto begin() const noexcept -> float * {
        return &this->data[0];
    }
    inline constexpr auto end() const noexcept -> float * {
        return this->iterator;
    }
    inline constexpr auto cbegin() const noexcept -> const float * {
        return static_cast<const float*>(&this->data[0]);
    }
    inline auto cend() const noexcept -> const float * {
        return static_cast<const float*>(this->iterator);
    }
    auto toArray() const noexcept -> float*;
    void randomize();
    void map(float (*const &)(float));
    auto serialize() const noexcept -> const std::string_view;

    // Static functions
    static auto fromArray(const float* const&, const int&) -> Matrix;
    static auto transpose(const Matrix&) -> Matrix;
    static auto multiply(const Matrix&, const Matrix&) -> Matrix;
    static auto subtract(const Matrix&, const Matrix&) -> Matrix;
    static auto map(const Matrix&, float (*const &)(float)) -> Matrix;
    static auto deserialize(const simdjson::dom::object&) -> Matrix;

 public:
    float *iterator;
    // Variables
    int32_t len;

    int32_t rows, columns;

    float* data;

    // Function
    void allocSpace();
};

#endif  // __MATRIX_HPP__
