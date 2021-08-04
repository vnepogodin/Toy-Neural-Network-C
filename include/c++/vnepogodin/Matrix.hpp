// Matrix lib
#ifndef MATRIX_HPP_
#define MATRIX_HPP_

#include <vnepogodin/helper.hpp>                     // tnn_really_inline, tnn_likely, tnn_unlikely, vector
#include <vnepogodin/third_party/json/simdjson.hpp>  // simdjson::dom::element

#include <iosfwd>     // std::ostream
#include <iterator>   // std::reverse_iterator
#include <stdexcept>  // std::overflow_error
#include <string>     // std::to_string

namespace vnepogodin {
class Matrix {
 public:
    using value_type             = double;
    using pointer                = value_type*;
    using const_pointer          = const value_type*;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using iterator               = value_type*;
    using const_iterator         = const value_type*;
    using reverse_iterator       = typename std::reverse_iterator<pointer>;
    using const_reverse_iterator = typename std::reverse_iterator<const_pointer>;

    using function_t         = double (*)(const double&);
    using initializer_list_t = std::initializer_list<std::initializer_list<value_type>>;

    // Constructors.
    constexpr Matrix() noexcept                 = default;
    tnn_really_inline Matrix(Matrix&&) noexcept = default;
    tnn_really_inline Matrix(const std::uint32_t& _r, const std::uint32_t& _c) noexcept
      : len(_r * _c),
        rows(_r),
        columns(_c),
        elem(new double[_r * _c]) {
        std::memset(data(), 0, len);
    }
    tnn_really_inline explicit Matrix(initializer_list_t&& i) noexcept
      : len(std::distance(i.begin()->begin(), i.begin()->end()) * i.size()),
        rows(i.size()),
        columns(len / rows),
        elem(new double[rows * columns]) {
        auto* ptr = begin();
        for (const auto& init_list : i) {
            for (const auto& iter : init_list) { *ptr = iter, ++ptr; }
        }
    }
    tnn_really_inline static auto copy(const Matrix& m) noexcept -> Matrix {
        Matrix tmp(m.rows, m.columns);
        std::copy(m.cbegin(), m.cend(), tmp.begin());

        return tmp;
    }

    // Destructor.
    tnn_really_inline ~Matrix() noexcept = default;

    // Operators.
    auto operator+=(const Matrix&) noexcept -> Matrix&;
    auto operator*=(const Matrix&) noexcept -> Matrix&;

    /* clang-format off */

    // Compare.
    constexpr bool operator==(const Matrix& _x) const noexcept {
        return (size() == _x.size()) && (std::equal(begin(), end(), _x.begin()));
    }
    constexpr bool operator!=(const Matrix& _x) const noexcept {
        return !(*this == _x);
    }

    /* clang-format on */

    tnn_really_inline auto operator=(Matrix&&) noexcept -> Matrix& = default;
    tnn_really_inline auto operator+=(const double& num) noexcept -> Matrix& {
        std::for_each(begin(), end(), [&](double& el) {
            el += num;
        });
        return *this;
    }
    tnn_really_inline auto operator*=(const double& num) & noexcept -> Matrix& {
        // Scalar product
        std::for_each(begin(), end(), [&](double& el) {
            el *= num;
        });
        return *this;
    }

    // Non member operator.
    friend auto operator<<(std::ostream&, const Matrix&) noexcept -> std::ostream&;

    /* clang-format off */

    // Iterators.
    tnn_really_inline auto begin() noexcept -> iterator
    { return iterator(data()); }

    tnn_really_inline auto begin() const noexcept -> const_iterator
    { return const_iterator(data()); }

    tnn_really_inline auto end() noexcept -> iterator
    { return iterator(data() + len); }

    tnn_really_inline auto end() const noexcept -> const_iterator
    { return const_iterator(data() + len); }

    tnn_really_inline auto rbegin() noexcept -> reverse_iterator
    { return reverse_iterator(end()); }

    tnn_really_inline auto rbegin() const noexcept -> const_reverse_iterator
    { return const_reverse_iterator(end()); }

    tnn_really_inline auto rend() noexcept -> reverse_iterator
    { return reverse_iterator(begin()); }

    tnn_really_inline auto rend() const noexcept -> const_reverse_iterator
    { return const_reverse_iterator(begin()); }

    tnn_really_inline auto cbegin() const noexcept -> const_iterator
    {return const_iterator(begin()); }

    tnn_really_inline auto cend() const noexcept -> const_iterator
    { return const_iterator(end()); }

    tnn_really_inline auto crbegin() const noexcept -> const_reverse_iterator
    { return const_reverse_iterator(end()); }

    tnn_really_inline auto crend() const noexcept -> const_reverse_iterator
    { return const_reverse_iterator(begin()); }

    // Capacity.
    constexpr auto
    size() const noexcept -> std::uint32_t { return len; }

    constexpr auto
    max_size() const noexcept -> std::uint32_t { return len; }

    constexpr auto
    empty() const noexcept -> bool { return size() == 0; }

    // Element access.
    tnn_really_inline auto data() noexcept -> pointer
    { return Matrix::S_ptr(&elem[0]); }
    tnn_really_inline auto data() const noexcept -> const_pointer
    { return Matrix::S_ptr(&elem[0]); }

    tnn_really_inline auto
    operator()(const std::uint32_t& x, const std::uint32_t& y) noexcept -> reference
    { return Matrix::S_ref(elem.get(), pos(x, y)); }

    tnn_really_inline auto
    operator()(const std::uint32_t& x, const std::uint32_t& y) const noexcept -> const_reference
    { return Matrix::S_ref(elem.get(), pos(x, y)); }
    /* clang-format on */

    tnn_really_inline auto
    at(const std::uint32_t& x, const std::uint32_t& y) noexcept(false) -> reference {
        if (tnn_unlikely((x >= rows) && (y >= columns))) {
            throw std::overflow_error("out of bounds");
        }
        return Matrix::S_ref(elem.get(), pos(x, y));
    }

    tnn_really_inline auto
    at(const std::uint32_t& x, const std::uint32_t& y) const noexcept(false) -> const_reference {
        return (tnn_likely((x < rows) && (y < columns))) ? Matrix::S_ref(elem.get(), pos(x, y))
                                                         : (throw std::overflow_error("out of bounds")),
               Matrix::S_ref(elem.get(), 0);
    }

    // Functions.
    auto toArray() const noexcept -> pointer;
    void randomize() noexcept;
    tnn_really_inline void map(const function_t& func) noexcept {
        // Apply a function to every element of matrix
        for (auto& el : *this) { el = func(el); }
    }
    auto dumps() const noexcept -> std::string;

    // Static functions.
    static auto fromArray(const_pointer, const std::uint32_t&) noexcept -> Matrix;
    static auto transpose(const Matrix&) noexcept -> Matrix;
    static auto multiply(const Matrix&, const Matrix&) noexcept -> Matrix;
    static auto subtract(const Matrix&, const Matrix&) noexcept -> Matrix;
    static auto map(const Matrix&, const function_t&) noexcept -> Matrix;

    tnn_really_inline static auto load(const std::string&& path) noexcept -> Matrix {
        simdjson::dom::parser p;
        simdjson::dom::object obj = p.load(path);
        return Matrix::parse(obj);
    }

    tnn_really_inline static vnepogodin::vector<Matrix> load_many(const std::string&& path, std::size_t batch_size = simdjson::dom::DEFAULT_BATCH_SIZE) noexcept {
        simdjson::dom::parser p;
        simdjson::dom::document_stream docs = p.load_many(path, batch_size);

        vnepogodin::vector<Matrix> res;
        for (simdjson::dom::element doc : docs) {
            res.emplace_back(Matrix::parse(doc));
        }
        return res;
    }

    static auto parse(const simdjson::dom::object& obj) noexcept -> Matrix;
    /** @overload parse(const simdjson::dom::element& obj) */
    tnn_really_inline static auto parse(const uint8_t* buf, std::size_t len, bool realloc_if_needed) noexcept -> Matrix {
        simdjson::dom::parser p;
        const simdjson::dom::object& obj = p.parse(buf, len, realloc_if_needed);
        return parse(obj);
    }
    /* clang-format off */
    /** @overload parse(const uint8_t *buf, std::size_t len, bool realloc_if_needed) */
    tnn_really_inline static auto parse(const char* buf, std::size_t len, bool realloc_if_needed) noexcept -> Matrix {
        return parse((const uint8_t*)buf, len, realloc_if_needed);
    }
    /** @overload parse(const uint8_t *buf, std::size_t len, bool realloc_if_needed) */
    tnn_really_inline static auto parse(const std::string& s) noexcept -> Matrix {
        return parse(s.data(), s.length(), s.capacity() - s.length() < simdjson::SIMDJSON_PADDING);
    }
    /** @overload parse(const uint8_t *buf, std::size_t len, bool realloc_if_needed) */
    tnn_really_inline static auto parse(const simdjson::padded_string& s) noexcept -> Matrix {
        return parse(s.data(), s.length(), false);
    }
    /* clang-format on */

    tnn_really_inline static vnepogodin::vector<Matrix> parse_many(const uint8_t* buf, std::size_t len, std::size_t batch_size = simdjson::dom::DEFAULT_BATCH_SIZE) noexcept {
        simdjson::dom::parser p;
        simdjson::dom::document_stream docs = p.parse_many(buf, len, batch_size);

        vnepogodin::vector<Matrix> res;
        for (simdjson::dom::element doc : docs) {
            res.emplace_back(Matrix::parse(doc));
        }
        return res;
    }

    /* clang-format off */
    /** @overload parse_many(const uint8_t *buf, size_t len, size_t batch_size) */
    tnn_really_inline static vnepogodin::vector<Matrix> parse_many(const char* buf, size_t len, std::size_t batch_size = simdjson::dom::DEFAULT_BATCH_SIZE) noexcept
    { return parse_many((const uint8_t*)buf, len, batch_size); }
    /** @overload parse_many(const uint8_t *buf, size_t len, size_t batch_size) */
    tnn_really_inline static vnepogodin::vector<Matrix> parse_many(const std::string& s, std::size_t batch_size = simdjson::dom::DEFAULT_BATCH_SIZE) noexcept
    { return parse_many(s.data(), s.length(), batch_size); }
    /** @overload parse_many(const uint8_t *buf, size_t len, size_t batch_size) */
    tnn_really_inline static vnepogodin::vector<Matrix> parse_many(const simdjson::padded_string& s, std::size_t batch_size = simdjson::dom::DEFAULT_BATCH_SIZE) noexcept
    { return parse_many(s.data(), s.length(), batch_size); }
    /* clang-format on */

    // Delete.
    constexpr Matrix(const Matrix&) noexcept = delete;
    constexpr auto operator=(const Matrix&) -> Matrix& = delete;

    /** @private We do not want to allow implicit conversion from C string to std::string. */
    tnn_really_inline static auto parse(const char* buf) noexcept -> Matrix = delete;

    tnn_really_inline static vnepogodin::vector<Matrix> parse_many(const std::string&& s, std::size_t batch_size)             = delete;  // unsafe
    tnn_really_inline static vnepogodin::vector<Matrix> parse_many(const simdjson::padded_string&& s, std::size_t batch_size) = delete;  // unsafe
    /** @private We do not want to allow implicit conversion from C string to std::string. */
    tnn_really_inline static vnepogodin::vector<Matrix> parse_many(const char* buf, std::size_t batch_size = simdjson::dom::DEFAULT_BATCH_SIZE) noexcept = delete;

 private:
    // Variables.
    std::uint32_t len{};
    std::uint32_t rows{};
    std::uint32_t columns{};

    std::unique_ptr<double[]> elem{nullptr};

    /* clang-format off */

    tnn_really_inline auto
    operator[](const std::uint32_t& idx) noexcept -> reference
    { return Matrix::S_ref(elem.get(), idx); }

    tnn_really_inline auto
    operator[](const std::uint32_t& idx) const noexcept -> const_reference
    { return Matrix::S_ref(elem.get(), idx); }

    // Functions.
    constexpr auto
    pos(const std::uint32_t& x, const std::uint32_t& y) const noexcept -> std::uint32_t
    { return (columns * x) + y; }

    static constexpr auto
    S_ref(const pointer _t, const std::uint32_t& _n) noexcept -> reference
    { return static_cast<reference>(_t[_n]); }

    static constexpr auto
    S_ptr(const pointer _t) noexcept -> pointer
    { return static_cast<pointer>(_t); }
    /* clang-format on */
};
};  // namespace vnepogodin

#endif  // MATRIX_HPP_
