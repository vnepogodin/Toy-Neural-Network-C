// Matrix lib

#include "Matrix.hpp"  // class Matrix

#include <random>  // std::mt19937, std::uniform_real_distribution, std::random_device

/**
 * PTR_START(end):
 *
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define PTR_START(end) int i = 0; while (i < (end)) {

/**
 * PTR_END:
 *
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define PTR_END ++ptr; ++i; }

class random_in_range {
    std::mt19937 rng{};
 public:
    random_in_range() : rng(std::random_device()()) {}
    auto operator()(int low, int high) -> float {
        std::uniform_real_distribution<float> uni(low, high);
        return uni(rng);
    }
};

// Constructors
Matrix::Matrix(const int rows, const int columns)
    : rows(rows), columns(columns) {
    this->allocSpace();

    float *ptr = &this->data[0][0];

    int end = this->rows * this->columns;
    PTR_START(end)
        *ptr = 0;
    PTR_END

    this->len = i;
}

Matrix::Matrix()
    : rows(1), columns(1) {
    this->data = new float *[this->rows];
    this->data[0] = new float[this->columns];

    this->data[0][0] = 0;

    this->len = 1;
}

Matrix::Matrix(const Matrix &m)
    : rows(m.rows), columns(m.columns) {
    this->allocSpace();

    float *ptr     = &this->data[0][0];
    const float *ref_ptr = &m.data[0][0];

    PTR_START(m.len)
        *ptr = *ref_ptr;

        ++ref_ptr;
    PTR_END

    this->len = i;
}

// Destructors
Matrix::~Matrix() {
}

void Matrix::clear() {
    int i = 0;
    while (i < this->rows) {
        delete[] this->data[i];
        ++i;
    }

    delete[] this->data;

    this->data = nullptr;
}

// Operators
auto Matrix::operator-=(const Matrix &m) -> Matrix& {
    float *ptr         = &this->data[0][0];
    const float *m_ptr = &m.data[0][0];

    PTR_START(this->len)
        *ptr -= *m_ptr;

        ++m_ptr;
    PTR_END

    return *this;
}

auto Matrix::operator*=(const Matrix &m) -> Matrix& {
    if (this->columns <= m.rows) {
        this->rows    = m.rows;
        this->columns = m.columns;

        this->allocSpace();

        int i = 0;
        while (i < this->rows) {
            int j = 0;
            while (j < m.columns) {
                int k = 0;
                while (k < m.columns) {
                    this->data[i][j] += this->data[i][k] * m.data[k][j];
                    ++k;
                }
                ++j;
            }
            ++i;
        }
    } else {
        float *ptr         = &this->data[0][0];
        const float *m_ptr = &m.data[0][0];

        PTR_START(this->len)
            *ptr *= *m_ptr;

            ++m_ptr;
        PTR_END
    }

    return *this;
}

// Functions
auto Matrix::fromArray(const float* const arr) -> Matrix {
    Matrix t(2, 1);

    float *ptr = &t.data[0][0];

    PTR_START(t.len)
        *ptr = arr[i];
    PTR_END

    return t;
}

auto Matrix::toArray() -> const float* {
    // Array[2]
    auto* arr = new float[2];

    // pointer to Matrix.data
    const float *ptr = &this->data[0][0];

    PTR_START(this->len)
        arr[i] = *ptr;
    PTR_END

    return arr;
}

void Matrix::randomize() {
    float *ptr = &this->data[0][0];

    random_in_range r;

    PTR_START(this->len)
        *ptr = r(0, 1);
    PTR_END
}

void Matrix::add(const Matrix &a) {
    float *ptr           = &this->data[0][0];
    const float *ref_ptr = &a.data[0][0];

    int i = 0;

    if (a.rows > this->rows) {
        while (i < this->len) {
            *ptr += *ref_ptr;

            ++ptr;
            ++ref_ptr;
            ++i;
        }
    } else {
        while (i < a.len) {
            *ptr += *ref_ptr;

            ++ptr;
            ++ref_ptr;
            ++i;
        }
    }
}

void Matrix::add(const float n) {
    float *ptr = &this->data[0][0];

    PTR_START(this->len)
        *ptr += n;
    PTR_END
}

void Matrix::multiply(const float n) {
    // Scalar product
    float *ptr = &this->data[0][0];

    PTR_START(this->len)
        *ptr *= n;
    PTR_END
}

void Matrix::map(float (*const func)(float)) {
    // Apply a function to every element of matrix
    float *ptr = &this->data[0][0];

    PTR_START(this->len)
        *ptr = (*func)(*ptr);
    PTR_END
}

void Matrix::print() {
    const float *ptr = &this->data[0][0];

    int cout = 0;
    int i = 0;
    while (i < this->len) {
        std::printf("%f ", *ptr);
        ++ptr;
        cout++;

        if (cout == this->columns) {
            cout = 0;
            std::printf("\n");
        }
        ++i;
    }
}

auto Matrix::serialize(const Matrix &m) const -> const nlohmann::json {
    nlohmann::json t;
    t["rows"] = m.rows;
    t["columns"] = m.columns;

    int i = 0;
    while (i < m.rows) {
        int j = 0;
        while (j < m.columns) {
            t["data"] += m.data[i][j];
            j++;
        }
        ++i;
    }

    return t.dump();
}

// Static functions
auto Matrix::transpose(const Matrix &m) -> Matrix {
    Matrix t(m.rows, m.columns);

    float *ptr         = &t.data[0][0];
    const float *m_ptr = &m.data[0][0];

    PTR_START(t.len)
        *ptr = *m_ptr;

        ++m_ptr;
    PTR_END

    return t;
}

auto Matrix::multiply(const Matrix &a, const Matrix &b) -> Matrix {
    Matrix t;
    t.clear();

    // Matrix product
    if (a.columns != b.rows) {
        t = Matrix(b.rows, b.columns);

        int i = 0;
        while (i < t.rows) {
            int j = 0;
            while (j < t.columns) {
                t.data[i][j] = 0;
                int k = 0;
                while (k < t.columns) {
                    t.data[i][j] += a.data[j][k] * b.data[i][j];
                    ++k;
                }
                ++j;
            }
            ++i;
        }
    } else {
        // Dot product of values in column
        t = Matrix(a.rows, b.columns);

        int i = 0;
        while (i < t.rows) {
            int j = 0;
            while (j < t.columns) {
                int k = 0;
                while (k < a.columns) {
                    t.data[i][j] += a.data[i][k] * b.data[k][j];
                    ++k;
                }
                ++j;
            }
            ++i;
        }
    }

    return t;
}

auto Matrix::subtract(const Matrix &a, const Matrix &b) -> Matrix {
    Matrix t;
    t.clear();

    if (a.columns >= b.rows) {
        t = Matrix(b.rows, b.columns);
    } else {
        t = Matrix(a.rows, b.columns);
    }

    float *ptr          = &t.data[0][0];
    const float *a_ptr  = &a.data[0][0];
    const float *b_ptr  = &b.data[0][0];

    PTR_START(t.len)
        *ptr = *a_ptr - *b_ptr;

        ++a_ptr;
        ++b_ptr;
    PTR_END

    return t;
}

auto Matrix::map(const Matrix &m, float (*const func)(float)) -> Matrix {
    Matrix t(m.rows, m.columns);

    float *ptr          = &t.data[0][0];
    const float *m_ptr  = &m.data[0][0];

    PTR_START(t.len)
        *ptr = (*func)(*m_ptr);

        ++m_ptr;
    PTR_END

    return t;
}

auto Matrix::deserialize(const nlohmann::json &t) -> Matrix {
    Matrix m = Matrix(t["rows"].get<int>(), t["columns"].get<int>());

    int i = 0;
    while (i < m.rows) {
        int j = 0;
        while (j < m.columns) {
            m.data[i][j] = t["data"][j].get<float>();
            j++;
        }
        i++;
    }
    return m;
}


// Private function
void Matrix::allocSpace() {
    posix_memalign(reinterpret_cast<void **>(&this->data), 1024UL, this->rows);

    int i = 0;
    while (i < this->rows) {
        posix_memalign(reinterpret_cast<void **>(&this->data[i]), 1024UL, this->columns);
        ++i;
    }
}

// Non Member Operator
auto operator*(const Matrix &a, const Matrix &b) -> Matrix {
    Matrix t(a);

    return t *= b;
}
