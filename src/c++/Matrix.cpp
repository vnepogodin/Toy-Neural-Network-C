// Matrix lib

#include <vnepogodin/Matrix.hpp>  // class Matrix

#include <algorithm>  // std::fill
#include <atomic>     // std::atomic<std::uint32_t>, std::memory_order_release
#include <iostream>   // std::cerr
#include <random>     // std::mt19937, std::uniform_real_distribution, std::random_device

namespace {
/**
 * PTR_START(end):
 *
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define PTR_START(end)               \
    std::atomic<std::uint32_t> i(0); \
    while (i < (end)) {
/**
 * PTR_END:
 *
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define PTR_END                                \
    i.fetch_add(1, std::memory_order_release); \
    }

class random_in_range {
 public:
    random_in_range()
        : rng(std::random_device()()) {}

    virtual ~random_in_range() = default;

    auto get() -> double {
        std::uniform_real_distribution<double> _realDistribution(start, end);

        return _realDistribution(rng);
    }

    // Delete.
    random_in_range(random_in_range&&) = delete;
    random_in_range(const random_in_range&) = delete;
    auto operator=(random_in_range&&) -> random_in_range& = delete;
    auto operator=(const random_in_range&) -> random_in_range& = delete;

 private:
    std::mt19937 rng;
    static constexpr double start = 0.0;
    static constexpr double end = 2.0;
};
};

namespace vnepogodin {
// Constructors
Matrix::Matrix(const std::uint32_t& _r, const std::uint32_t& _c)
    : len(_r * _c),
      rows(_r), columns(_c),
      elem(static_cast<pointer>(::operator new(_r * _c * sizeof(value_type))))
{
    std::fill(begin(), end(), 0);
}


// Copy constructor
//
Matrix::Matrix(const Matrix& mat)
    : len(mat.len),
      rows(mat.rows), columns(mat.columns),
      elem(static_cast<pointer>(::operator new(rows * columns * sizeof(value_type))))
{
    std::copy(mat.cbegin(), mat.cend(), begin());
}

// Operators
auto Matrix::operator+=(const_reference num) -> Matrix& {
    for (auto& el : *this)
        el += num;

    return *this;
}

auto Matrix::operator+=(const Matrix& mat_a) -> Matrix& {
    if ((this->rows != mat_a.rows) || (this->columns != mat_a.columns)) {
        std::cerr << "Columns and Rows of A must match Columns and Rows of B.\n";
        return *this;
    }

    std::atomic<std::uint32_t> i(0);
    for (auto& iter : *this) {
        iter += mat_a[i.load(std::memory_order_consume)];
    PTR_END

    return *this;
}

auto Matrix::operator*=(const Matrix& mat) -> Matrix& {
    if ((this->rows != mat.rows) || (this->columns != mat.columns)) {
        std::cerr << "Columns and Rows of A must match Columns and Rows of B.\n";
        return *this;
    }

    // hadamard product
    // @see https://en.wikipedia.org/wiki/Hadamard_product_(matrices)
    //
    std::atomic<std::uint32_t> i(0);
    for (auto& iter : *this) {
        iter *= mat[i.load(std::memory_order_consume)];
    PTR_END

    return *this;
}

auto Matrix::operator*=(const double& num) -> Matrix& {
    // Scalar product
    for(auto& iter : *this)
        iter *= num;

    return *this;
}


// Non member operator
auto operator<<(std::ostream& stream, const Matrix& mat) -> std::ostream& {
    std::atomic<std::uint32_t> counter(0);
    for (const auto& i : mat) {
        stream << i << ' ';

        counter.fetch_add(1, std::memory_order_release);
        if (counter == mat.columns) {
            counter = 0;

            if ((&i + 1) != mat.end())
                stream << "\n";
        }
    }

    return stream;
}

// Functions
auto Matrix::toArray() const noexcept -> pointer {
    auto *tmp = new double[len];

    std::atomic<std::uint32_t> i(0);
    for (const auto& iter : *this) {
        tmp[i.load(std::memory_order_consume)] = iter;
    PTR_END

    return tmp;
}

void Matrix::randomize() {
    random_in_range r;
    for (auto& el : *this)
        el = r.get() - 1;
}

// Serialize to JSON
// TODO: Refactor
//
auto Matrix::serialize() const noexcept -> string {
    auto _str = string("{\"rows\":") + to_string(rows)
              + string(",\"columns\":") + to_string(columns)
              + string(",\"data\":[");
    string temp_arr = "[";

    _str.reserve(120);
    temp_arr.reserve(3 + (this->columns * 11));

    std::uint32_t counter = 0;
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

    _str.shrink_to_fit();
    return _str;
}


// Static functions
auto Matrix::fromArray(const_pointer const& arr, const std::uint32_t& len) -> Matrix {
    Matrix t(len, 1);

    std::atomic<std::uint32_t> i(0);
    for (auto& iter : t) {
        iter = arr[i.load(std::memory_order_consume)];
    PTR_END

    return t;
}

// @see https://en.wikipedia.org/wiki/Transpose
auto Matrix::transpose(const Matrix& m) -> Matrix {
    Matrix t(m.columns, m.rows);

    std::atomic<std::uint32_t> counter(0);
    PTR_START(t.rows)
        std::atomic<std::uint32_t> j(0);
        while (j < t.columns) {
            t[counter.load(std::memory_order_consume)] =
                m[j.load(std::memory_order_consume) * t.rows + i.load(std::memory_order_consume)];

            counter.fetch_add(1, std::memory_order_release);
            j.fetch_add(1, std::memory_order_release);
        }
    PTR_END

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

    std::atomic<std::uint32_t> counter(0);
    PTR_START(t.rows)
        std::atomic<std::uint32_t> j(0);
        while (j < t.columns) {
            std::atomic<std::uint32_t> k(0);
            double sum = 0.0;
            while (k < a.columns) {
                sum +=
                    a[i.load(std::memory_order_consume) * a.columns + k.load(std::memory_order_consume)]
                    * b[k.load(std::memory_order_consume) * t.rows + j.load(std::memory_order_consume)];

                k.fetch_add(1, std::memory_order_release);
            }
            t[counter.load(std::memory_order_consume)] = sum;

            counter.fetch_add(1, std::memory_order_release);
            j.fetch_add(1, std::memory_order_release);
        }
    PTR_END

    return t;
}

auto Matrix::subtract(const Matrix& a, const Matrix& b) -> Matrix {
    if ((a.rows != b.rows) || (a.columns != b.columns)) {
        std::cerr << "Columns and Rows of A must match Columns and Rows of B.\n";
        return Matrix();
    }

    Matrix t(a.rows, b.columns);
    std::atomic<std::uint32_t> i(0);
    for (auto& iter : t) {
        iter = a[i.load(std::memory_order_consume)]
               - b[i.load(std::memory_order_consume)];
    PTR_END

    return t;
}

auto Matrix::map(const Matrix& m, const function_t& func) -> Matrix {
    Matrix t(m);

    t.map(func);
    return t;
}

// Deserialize from JSON
//
auto Matrix::deserialize(const simdjson::dom::element& t) -> Matrix {
    const std::uint64_t& rows = t["rows"];
    const std::uint64_t& cols = t["columns"];

    Matrix m(rows, cols);
    auto *ptr = m.begin();

    constexpr const auto& _str = "/data/";

    std::uint32_t counter = 0;
    PTR_START(m.rows)
        const auto& buf_s = _str + to_string(i.load(std::memory_order_consume)) + '/' + to_string(counter);

        *ptr = t.at_pointer(buf_s);
        ++ptr;
        counter++;

        if (counter == m.columns) {
            counter = 0;

            i.fetch_add(1, std::memory_order_release);
        }
    }

    return m;
}
};  // namespace vnepogodin
