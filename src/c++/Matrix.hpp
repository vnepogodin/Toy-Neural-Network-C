// Matrix lib
#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include "json.hpp"  // nlohmann::json

class Matrix {
 public:
    // Constructors
    Matrix(const int rows, const int columns);
    Matrix();
    Matrix(const Matrix &);

    // Destructors
    ~Matrix();
    void clear();

    // Operators
    auto operator-=(const Matrix &) -> Matrix&;
    auto operator*=(const Matrix &) -> Matrix&;

    // Functions
    static auto fromArray(const float* const arr) -> Matrix;
    auto toArray() -> const float*;
    void randomize();
    void add(const Matrix &);
    void add(const float);
    void multiply(const float);
    void map(float (*const)(float));
    void print();
    auto serialize(const Matrix &) const -> const nlohmann::json;

    // Static functions
    static auto transpose(const Matrix &) -> Matrix;
    static auto multiply(const Matrix &, const Matrix &) -> Matrix;
    static auto subtract(const Matrix &, const Matrix &) -> Matrix;
    static auto map(const Matrix &, float (*const)(float)) -> Matrix;
    static auto deserialize(const nlohmann::json &) -> Matrix;

 private:
    // Variables
    int len;

    int rows, columns;

    float **data;

    // Function
    void allocSpace();
};

#endif  // __MATRIX_HPP__
