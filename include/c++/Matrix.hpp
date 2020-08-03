// Matrix lib
#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include "third_party/json/json.hpp"  // nlohmann::json

class Matrix {
 public:
    // Constructors
    Matrix(const int32_t, const int32_t);
    Matrix();
    Matrix(const Matrix &);

    // Destructors
    ~Matrix() = default;
    void clear();

    // Operators
    auto operator=(const Matrix &) -> Matrix& = default;
    auto operator-=(const Matrix &) -> Matrix&;
    auto operator*=(const Matrix &) -> Matrix&;

    // Non member operator
    friend auto operator<<(std::ostream&, const Matrix&) -> std::ostream&;

    // Functions
    auto begin() const noexcept -> float_t* {
        return &this->data[0][0];
    }
    auto end() const noexcept -> float_t* {
        return this->iterator;
    }
    auto toArray() const noexcept -> const float_t*;
    void randomize();
    void add(const Matrix &);
    void add(const float_t &);
    void multiply(const float_t &);
    void map(float_t (*const)(float_t));
    auto serialize() const noexcept -> const nlohmann::json;

    // Static functions
    static auto fromArray(const float_t* const &) -> Matrix;
    static auto transpose(const Matrix &) -> Matrix;
    static auto multiply(const Matrix &, const Matrix &) -> Matrix;
    static auto subtract(const Matrix &, const Matrix &) -> Matrix;
    static auto map(const Matrix &, float_t (*const)(float_t)) -> Matrix;
    static auto deserialize(const nlohmann::json &) -> Matrix;

 private:
    float_t *iterator;
    // Variables
    int32_t len;

    int32_t rows, columns;

    float_t** data;

    // Function
    void allocSpace();
};

#endif  // __MATRIX_HPP__
