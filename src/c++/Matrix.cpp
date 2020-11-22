// Matrix lib

#include "../../include/c++/Matrix.hpp"  // class Matrix

#include <random>  // std::mt19937, std::uniform_real_distribution, std::random_device
#include <atomic>  // std::atomic<int32_t>, std::memory_order_release
#include <iostream>  // std::cerr

/**
 * PTR_START(end):
 *
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define PTR_START(end) std::atomic<int32_t> i(0); while (i < (end)) {

/**
 * PTR_END:
 *
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define PTR_END ++ptr; i.fetch_add(1, std::memory_order_release); }


using matrix_function = float_t (*const)(float_t);  // Function alias


class random_in_range {
    std::mt19937 rng;

 public:
    random_in_range()
        : rng(std::random_device()()) {}

    virtual ~random_in_range() = default;

    auto get() -> float_t {
        std::uniform_real_distribution<float_t> _realDistribution(0.0, 2.0);

        return _realDistribution(rng);
    }
};

// Constructors
Matrix::Matrix(const int32_t& r, const int32_t& cols)
    : rows(r), columns(cols) {
    this->allocSpace();

    float_t *ptr = &this->data[0];

    int32_t end = this->rows * this->columns;

    PTR_START(end)
        *ptr = 0;
    PTR_END

    this->iter = ptr;

    this->len = i.load(std::memory_order_consume);
}

// Default constructor
//
Matrix::Matrix()
    : len(1),
      rows(1), columns(1) {
    this->data = new float_t[this->len];

    this->data[0] = 0;

    this->iter = &this->data[0] + 1;
}

// Copy constructor
//
Matrix::Matrix(const Matrix& m)
    : rows(m.rows), columns(m.columns) {
    this->allocSpace();

    float_t *ptr           = &this->data[0];
    const float_t *ref_ptr = &m.data[0];

    PTR_START(m.len)
        *ptr = *ref_ptr;

        ++ref_ptr;
    PTR_END

    this->iter = ptr;

    this->len = i.load(std::memory_order_consume);
}

// Operators
auto Matrix::operator+=(const float_t& num) -> Matrix& {
    float_t *ptr = &this->data[0];

    PTR_START(this->len)
        *ptr += num;
    PTR_END

    return *this;
}

auto Matrix::operator+=(const Matrix& a) -> Matrix& {
    if ((this->rows != a.rows) || (this->columns != a.columns)) {
        std::cerr << "Columns and Rows of A must match Columns and Rows of B.\n";
        return *this;
    }

    float_t *ptr           = &this->data[0];
    const float_t *ref_ptr = &a.data[0];

    PTR_START(this->len)
        *ptr += *ref_ptr;

        ++ref_ptr;
    PTR_END

    return *this;
}

auto Matrix::operator*=(const Matrix& m) -> Matrix& {
    if ((this->rows != m.rows) || (this->columns != m.columns)) {
        std::cerr << "Columns and Rows of A must match Columns and Rows of B.\n";
        return *this;
    }

    float_t *ptr         = &this->data[0];
    const float_t *m_ptr = &m.data[0];

    // hadamard product
    // @see https://en.wikipedia.org/wiki/Hadamard_product_(matrices)
    //
    PTR_START(this->len)
        *ptr *= *m_ptr;

        ++m_ptr;
    PTR_END

    return *this;
}

auto Matrix::operator*=(const float_t& num) -> Matrix& {
    // Scalar product
    float_t *ptr = &this->data[0];

    PTR_START(this->len)
        *ptr *= num;
    PTR_END

    return *this;
}

auto Matrix::operator[](const size_t& idx) -> iterator {
    if ((idx < 0) || (idx >= this->len)) {
        throw std::overflow_error("idx overload");
        return nullptr;
    }
    return this->data + (this->columns * idx);
}
auto Matrix::operator[](const size_t& idx) const noexcept -> const_iterator {
    if ((idx < 0) || (idx >= this->len)) {
        return nullptr;
    }
    return static_cast<const_iterator>(&this->data[idx] + (this->columns * idx));
}

// Non member operator
auto operator<<(std::ostream& stream, const Matrix& m) -> std::ostream& {
    int32_t counter = 0;
    for (const auto& i : m) {
        stream << i << ' ';

        ++counter;
        if (counter == m.columns) {
            counter = 0;

            if ((&i + 1) != m.end())
                stream << "\n";
        }
    }

    return stream;
}

// Functions
auto Matrix::toArray() const noexcept -> float_t* {
    auto arr = new float_t[this->len];

    // pointer to Matrix.data
    const float_t *ptr = &this->data[0];

    PTR_START(this->len)
        arr[i] = *ptr;
    PTR_END

    return arr;
}

void Matrix::randomize() {
    float_t *ptr = &this->data[0];

    random_in_range r;

    PTR_START(this->len)
        *ptr = r.get() - 1;
    PTR_END
}

void Matrix::map(matrix_function &func) {
    // Apply a function to every element of matrix
    float_t *ptr = &this->data[0];

    PTR_START(this->len)
        *ptr = (*func)(*ptr);
    PTR_END
}

// TODO: Refactor
//
auto Matrix::serialize() const noexcept -> const string {
    auto _str =   string("{\"rows\":") + to_string(this->rows)
                + string(",\"columns\":") + to_string(this->columns)
                + string(",\"data\":[");
    string temp_arr = "[";

    _str.reserve(120);
    temp_arr.reserve(3 + this->columns);

    int32_t counter = 0;
    for (const auto& i : *this) {
        temp_arr += to_string(i);

        ++counter;
        if (counter != this->columns) {
            temp_arr += ',';
        } else {
            counter = 0;
            _str += temp_arr + ']';

            temp_arr = '[';
            if ((&i + 1) != this->end())
                _str += ',';
        }
    }
    _str += "]}";

    const auto& len = _str.size();
    std::unique_ptr<char[]> res{new char[len]};
    std::copy(_str.cbegin(), _str.cend() + 1, res.get());

    return res.get();
}


// Static functions
auto Matrix::fromArray(const float_t* const& arr, const int& len) -> Matrix {
    Matrix t(len, 1);

    float_t *ptr = &t.data[0];

    PTR_START(len)
        *ptr = arr[i];
    PTR_END

    return t;
}

// @see https://en.wikipedia.org/wiki/Transpose
auto Matrix::transpose(const Matrix& m) -> Matrix {
    Matrix t(m.columns, m.rows);

    float_t *ptr         = &t.data[0];
    const float_t *m_ptr = &m.data[0];

    int32_t counter = 0;
    PTR_START(t.rows)
        std::atomic<int32_t> j(0);
        while(j < t.columns) {
            ptr[counter] = m_ptr[j.load(std::memory_order_consume) * t.rows + i.load(std::memory_order_consume)];

            ++counter;
            j.fetch_add(1, std::memory_order_release);
        }
        i.fetch_add(1, std::memory_order_release);
    }

    return t;
}

auto Matrix::multiply(const Matrix& a, const Matrix& b) -> Matrix {
    // Matrix product
    if (a.columns != b.rows) {
        std::cerr << "Columns of A must match rows of B.\n";
        return Matrix();
    }

    // Dot product of values in column
    Matrix t(a.rows, b.columns);

    float_t *ptr          = &t.data[0];
    const float_t *a_ptr  = &a.data[0];
    const float_t *b_ptr  = &b.data[0];

    std::atomic<int32_t> counter(0);
    PTR_START(t.rows)
        std::atomic<int32_t> j(0);
        while (j < t.columns) {
            std::atomic<int32_t> k(0);
            float_t sum = 0;
            while (k < a.columns) {
                sum += a_ptr[i.load(std::memory_order_consume) * a.columns + k.load(std::memory_order_consume)] * b_ptr[k.load(std::memory_order_consume) * t.rows + j.load(std::memory_order_consume)];

                k.fetch_add(1, std::memory_order_release);
            }
            ptr[counter.load(std::memory_order_consume)] = sum;

            counter.fetch_add(1, std::memory_order_release);
            j.fetch_add(1, std::memory_order_release);
        }
        i.fetch_add(1, std::memory_order_release);
    }

    return t;
}

auto Matrix::subtract(const Matrix& a, const Matrix& b) -> Matrix {
    if ((a.rows != b.rows) || (a.columns != b.columns)) {
        std::cerr << "Columns and Rows of A must match Columns and Rows of B.\n";
        return Matrix();
    }

    Matrix t(a.rows, b.columns);

    float_t *ptr          = &t.data[0];
    const float_t *a_ptr  = &a.data[0];
    const float_t *b_ptr  = &b.data[0];

    PTR_START(t.len)
        *ptr = *a_ptr - *b_ptr;

        ++a_ptr;
        ++b_ptr;
    PTR_END

    return t;
}

auto Matrix::map(const Matrix& m, matrix_function &func) -> Matrix {
    Matrix t(m);

    float_t *ptr = &t.data[0];

    PTR_START(t.len)
        *ptr = (*func)(*ptr);
    PTR_END

    return t;
}

auto Matrix::deserialize(const simdjson::dom::element& t) -> Matrix {
    const uint64_t& rows = t["rows"];
    const uint64_t& cols = t["columns"];

    Matrix m(rows, cols);
    float *ptr = &m.data[0];

    constexpr auto& _str = "/data/";

    int32_t counter = 0;
    PTR_START(m.rows)
        const auto& buf_s = _str + to_string(i.load(std::memory_order_consume)) + '/' + to_string(counter);

        *ptr = static_cast<float>(static_cast<double>(t.at_pointer(buf_s)));
        ++ptr;
        counter++;

        if (counter == m.columns) {
            counter = 0;

            i.fetch_add(1, std::memory_order_release);
        }
    }

    return m;
}


// Private function
void Matrix::allocSpace() {
    this->data = static_cast<float_t *>(::operator new(this->rows * this->columns * sizeof(float_t)));
}
