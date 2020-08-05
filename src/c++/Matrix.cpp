// Matrix lib

#include "../../include/c++/Matrix.hpp"  // class Matrix

#include <random>  // std::mt19937, std::uniform_real_distribution, std::random_device
#include <atomic>  // std::atomic<int32_t>, std::memory_order_release

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

#ifdef _WIN32
#  define posix_memalign(p, a, s) (((*(p)) = _aligned_malloc((s), (a))), *(p) ? 0 : errno)
#endif  // _WIN32


class random_in_range {
    std::mt19937 rng;

 public:
    random_in_range()
	: rng(std::random_device()()) {}

    auto operator()() -> float_t {
        std::uniform_real_distribution<float_t> _realDistribution(0.0, 1.0);

        return _realDistribution(rng);
    }
};

// Constructors
Matrix::Matrix(const int32_t r, const int32_t cols)
    : rows(r), columns(cols) {
    this->allocSpace();

    float_t *ptr = &this->data[0][0];

    int32_t end = this->rows * this->columns;
    PTR_START(end)
        *ptr = 0;
    PTR_END

    this->iterator = ptr;

    this->len = i.load(std::memory_order_consume);
}

// Default constructor
//
Matrix::Matrix()
    : len(1),
      rows(1), columns(1) {
    this->data = new float_t* [this->rows];
    this->data[0] = new float_t [this->columns];

    this->data[0][0] = 0;

    this->iterator = &this->data[0][0];
}

// Copy constructor
//
Matrix::Matrix(const Matrix &m)
    : rows(m.rows), columns(m.columns) {
    this->allocSpace();

    float_t *ptr           = &this->data[0][0];
    const float_t *ref_ptr = &m.data[0][0];

    PTR_START(m.len)
        *ptr = *ref_ptr;

        ++ref_ptr;
    PTR_END

    this->iterator = ptr;

    this->len = i.load(std::memory_order_consume);
}

// Operators
auto Matrix::operator-=(const Matrix &m) -> Matrix& {
    float_t *ptr         = &this->data[0][0];
    const float_t *m_ptr = &m.data[0][0];

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

        int32_t i = 0;
        while (i < this->rows) {
            int32_t j = 0;
            while (j < m.columns) {
                int32_t k = 0;
                while (k < m.columns) {
                    this->data[i][j] += this->data[i][k] * m.data[k][j];
                    ++k;
                }
                ++j;
            }
            ++i;
        }
    } else {
        float_t *ptr         = &this->data[0][0];
        const float_t *m_ptr = &m.data[0][0];

        PTR_START(this->len)
            *ptr *= *m_ptr;

            ++m_ptr;
        PTR_END
    }

    return *this;
}

// Non member operator
auto operator<<(std::ostream& stream, const Matrix& m) -> std::ostream& {
    int32_t counter = 0;
    for (auto i : m) {
        stream << i << " ";

        counter++;
        if (counter == m.columns) {
            counter = 0;

            if (&i != m.end())
                stream << "\n";
        }
    }

    return stream;
}

// Functions
auto Matrix::toArray() const noexcept -> const float_t* {
    // Array[2]
    auto* arr = new float_t[2];

    // pointer to Matrix.data
    const float_t *ptr = &this->data[0][0];

    PTR_START(this->len)
        arr[i] = *ptr;
    PTR_END

    return arr;
}

void Matrix::randomize() {
    float_t *ptr = &this->data[0][0];

    random_in_range r;

    PTR_START(this->len)
        *ptr = r();
    PTR_END
}

void Matrix::add(const Matrix &a) {
    float_t *ptr           = &this->data[0][0];
    const float_t *ref_ptr = &a.data[0][0];

    int32_t i = 0;

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

void Matrix::add(const float_t &num) {
    float_t *ptr = &this->data[0][0];

    PTR_START(this->len)
        *ptr += num;
    PTR_END
}

void Matrix::multiply(const float_t &num) {
    // Scalar product
    float_t *ptr = &this->data[0][0];

    PTR_START(this->len)
        *ptr *= num;
    PTR_END
}

void Matrix::map(float_t (*const func)(float)) {
    // Apply a function to every element of matrix
    float_t *ptr = &this->data[0][0];

    PTR_START(this->len)
        *ptr = (*func)(*ptr);
    PTR_END
}

auto Matrix::serialize() const noexcept -> const nlohmann::json {
    nlohmann::json t = nlohmann::json::object({ {"rows", this->rows}, {"columns", this->columns}, {"data", nlohmann::json::array()} });

    nlohmann::json temp_arr = nlohmann::json::array();

    int32_t counter = 0;
    for (auto i : *this) {
        temp_arr += i;
        counter++;

        if (counter == this->columns) {
            t["data"] += temp_arr;

            temp_arr = nlohmann::json::array();
            counter = 0;
        }
    }

    return t;
}


// Static functions
auto Matrix::fromArray(const float_t* const &arr) -> Matrix {
    Matrix t(2, 1);

    float_t *ptr = &t.data[0][0];

    PTR_START(t.len)
    *ptr = arr[i];
    PTR_END

    return t;
}

auto Matrix::transpose(const Matrix &m) -> Matrix {
    Matrix t(m.rows, m.columns);

    float_t *ptr         = &t.data[0][0];
    const float_t *m_ptr = &m.data[0][0];

    PTR_START(t.len)
        *ptr = *m_ptr;

        ++m_ptr;
    PTR_END

    return t;
}

auto Matrix::multiply(const Matrix &a, const Matrix &b) -> Matrix {
    Matrix t;

    // Matrix product
    if (a.columns != b.rows) {
        t = Matrix(b.rows, b.columns);

        int32_t i = 0;
        while (i < t.rows) {
            int32_t j = 0;
            while (j < t.columns) {
                t.data[i][j] = 0;
                int32_t k = 0;
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

        int32_t i = 0;
        while (i < t.rows) {
            int32_t j = 0;
            while (j < t.columns) {
                int32_t k = 0;
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

    if (a.columns >= b.rows) {
        t = Matrix(b.rows, b.columns);
    } else {
        t = Matrix(a.rows, b.columns);
    }

    float_t *ptr          = &t.data[0][0];
    const float_t *a_ptr  = &a.data[0][0];
    const float_t *b_ptr  = &b.data[0][0];

    PTR_START(t.len)
        *ptr = *a_ptr - *b_ptr;

        ++a_ptr;
        ++b_ptr;
    PTR_END

    return t;
}

auto Matrix::map(const Matrix &m, float_t (*const func)(float)) -> Matrix {
    Matrix t(m.rows, m.columns);

    float_t *ptr          = &t.data[0][0];
    const float_t *m_ptr  = &m.data[0][0];

    PTR_START(t.len)
        *ptr = (*func)(*m_ptr);

        ++m_ptr;
    PTR_END

    return t;
}

auto Matrix::deserialize(const nlohmann::json &t) -> Matrix {
    Matrix m(t["rows"].get<int32_t>(),
             t["columns"].get<int32_t>());
 
    float_t *ptr = &m.data[0][0];

    int32_t counter = 0;
    PTR_START(m.rows)
        *ptr = t["data"][static_cast<unsigned long>(i.load(std::memory_order_consume))][static_cast<unsigned long>(counter)].get<float_t>();
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
    posix_memalign(reinterpret_cast<void **>(&this->data), 1024UL, static_cast<size_t>(this->rows));

    int32_t i = 0;
    while (i < this->rows) {
        posix_memalign(reinterpret_cast<void **>(&this->data[i]), 1024UL, static_cast<size_t>(this->columns));
        ++i;
    }
}
