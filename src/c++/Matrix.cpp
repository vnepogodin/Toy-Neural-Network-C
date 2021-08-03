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
    while (i < (end))

/**
 * PTR_END:
 *
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define PTR_END \
    i.fetch_add(1, std::memory_order_release);

class random_in_range {
 public:
    tnn_really_inline random_in_range()
      : rng(std::random_device()()) {}

    virtual ~random_in_range() = default;

    tnn_really_inline auto get() noexcept -> double {
        std::uniform_real_distribution<double> _realDistribution(start, end);

        return _realDistribution(rng);
    }

    // Delete.
    random_in_range(random_in_range&&)      = delete;
    random_in_range(const random_in_range&) = delete;
    auto operator=(random_in_range&&) -> random_in_range& = delete;
    auto operator=(const random_in_range&) -> random_in_range& = delete;

 private:
    std::mt19937 rng;
    static constexpr double start = 0.0;
    static constexpr double end   = 2.0;
};
};  // namespace

namespace vnepogodin {

// Operators
auto Matrix::operator+=(const Matrix& mat_a) noexcept -> Matrix& {
    if ((this->rows != mat_a.rows) || (this->columns != mat_a.columns)) {
        std::cerr << "Columns and Rows of A must match Columns and Rows of B.\n";
        return *this;
    }

    std::atomic<std::uint32_t> i(0);
    for (auto& iter : *this) {
        iter += mat_a[i.load(std::memory_order_consume)];
        PTR_END
    }

    return *this;
}

auto Matrix::operator*=(const Matrix& mat) noexcept -> Matrix& {
    if (tnn_unlikely((this->rows != mat.rows) || (this->columns != mat.columns))) {
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
    }

    return *this;
}

// Non member operator
auto operator<<(std::ostream& stream, const Matrix& mat) noexcept -> std::ostream& {
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
    }
    return tmp;
}

void Matrix::randomize() noexcept {
    random_in_range r;
    std::generate(begin(), end(), [&](){ return r.get() - 1; });
}

// Serialize to JSON
// TODO: Refactor
//
auto Matrix::dumps() const noexcept -> std::string {
    static constexpr int resLen = 125;
    /* clang-format off */
    std::string _str = std::string("{")
                     + std::string("\"rows\":") + std::to_string(rows)
                     + std::string(",\"columns\":") + std::to_string(columns)
                     + std::string(",\"data\":[");
    /* clang-format on */
    _str.reserve(resLen);

    std::string temp_arr = "[";
    temp_arr.reserve(3 + (this->columns * 11));

    std::uint32_t counter = 0;
    for (const auto& i : *this) {
        temp_arr += std::to_string(i);

        ++counter;
        if (tnn_likely(counter != this->columns)) {
            temp_arr += ',';
        } else {
            counter = 0;
            _str += temp_arr + ']';

            temp_arr = '[';
            if (tnn_likely((&i + 1) != this->end()))
                _str += ',';
        }
    }
    _str += "]}";

    _str.shrink_to_fit();
    return _str;
}

// Static functions
auto Matrix::fromArray(const_pointer arr, const std::uint32_t& len) noexcept -> Matrix {
    Matrix t(len, 1);

    std::atomic<std::uint32_t> i(0);
    for (auto& iter : t) {
        iter = arr[i.load(std::memory_order_consume)];
        PTR_END
    }

    return t;
}

// @see https://en.wikipedia.org/wiki/Transpose
auto Matrix::transpose(const Matrix& m) noexcept -> Matrix {
    Matrix t(m.columns, m.rows);

    std::atomic<std::uint32_t> counter(0);
    PTR_START(t.rows) {
        std::atomic<std::uint32_t> j(0);
        while (j < t.columns) {
            t[counter.load(std::memory_order_consume)] =
                m[j.load(std::memory_order_consume) * t.rows + i.load(std::memory_order_consume)];

            counter.fetch_add(1, std::memory_order_release);
            j.fetch_add(1, std::memory_order_release);
        }
        PTR_END
    }

    return t;
}

auto Matrix::multiply(const Matrix& a, const Matrix& b) noexcept -> Matrix {
    // Matrix product
    if (tnn_unlikely(a.columns != b.rows)) {
        std::cerr << "Columns of A must match rows of B.\n";
        return Matrix();
    }

    // Dot product of values in column
    Matrix t(a.rows, b.columns);

    std::atomic<std::uint32_t> counter(0);
    PTR_START(t.rows) {
        std::atomic<std::uint32_t> j(0);
        while (j < t.columns) {
            std::atomic<std::uint32_t> k(0);
            double sum = 0.0;
            while (k < a.columns) {
                sum += a[i.load(std::memory_order_consume) * a.columns + k.load(std::memory_order_consume)]
                     * b[k.load(std::memory_order_consume) * b.columns + j.load(std::memory_order_consume)];

                k.fetch_add(1, std::memory_order_release);
            }
            t[counter.load(std::memory_order_consume)] = sum;

            counter.fetch_add(1, std::memory_order_release);
            j.fetch_add(1, std::memory_order_release);
        }
        PTR_END
    }
    return t;
}

auto Matrix::subtract(const Matrix& a, const Matrix& b) noexcept -> Matrix {
    if (tnn_unlikely((a.rows != b.rows) || (a.columns != b.columns))) {
        std::cerr << "Columns and Rows of A must match Columns and Rows of B.\n";
        return Matrix();
    }

    Matrix t(a.rows, b.columns);
    std::atomic<std::uint32_t> i(0);
    for (auto& iter : t) {
        iter = a[i.load(std::memory_order_consume)]
             - b[i.load(std::memory_order_consume)];
        PTR_END
    }
    return t;
}

auto Matrix::map(const Matrix& m, const function_t& func) noexcept -> Matrix {
    Matrix t = Matrix::copy(m);

    t.map(func);
    return t;
}

// Deserialize from JSON
//
auto Matrix::parse(const simdjson::dom::object& obj) noexcept -> Matrix {
    const std::uint64_t& rows = obj["rows"];
    const std::uint64_t& cols = obj["columns"];

    Matrix m(rows, cols);
    auto *ptr = m.begin();

    const auto& data = obj["data"];

    std::uint32_t counter = 0;
    PTR_START(m.rows) {
        const auto& buf_s = '/' + std::to_string(i.load(std::memory_order_consume)) + '/' + std::to_string(counter);
        *ptr = data.at_pointer(buf_s);
        ++ptr;
        counter++;

        if (tnn_unlikely(counter == m.columns)) {
            counter = 0;

            i.fetch_add(1, std::memory_order_release);
        }
    }

    return m;
}
};  // namespace vnepogodin
