// Other techniques for learning
#ifndef NN_HPP_
#define NN_HPP_

#include <vnepogodin/Matrix.hpp>  // class Matrix

#include <array>

namespace vnepogodin {
enum class Function : std::uint8_t {
    sigmoid = 1,
    dsigmoid = 2,
};

class NeuralNetwork {
  public:
    // Constructors.
    constexpr NeuralNetwork() noexcept = default;
    tnn_really_inline NeuralNetwork(NeuralNetwork&&) noexcept = default;
    NeuralNetwork(const std::uint32_t&, const std::uint32_t&, const std::uint32_t&);

    // Destructor.
    virtual ~NeuralNetwork() noexcept = default;

    // Operator.
    auto operator=(NeuralNetwork&&) noexcept -> NeuralNetwork& = default;

    // Functions
    auto predict(Matrix::const_pointer) const noexcept -> Matrix::pointer;
    constexpr void setLearningRate(const double& lr) noexcept
    { this->learning_rate = lr; }
    void setActivationFunction(const Function&) noexcept;
    void train(Matrix::const_pointer, Matrix::const_pointer) noexcept;
    auto dumps() const noexcept -> std::string;

    // Static function
    static auto copy(const NeuralNetwork& nn) noexcept -> NeuralNetwork;

    tnn_really_inline static auto load(const std::string&& path) noexcept -> NeuralNetwork {
        simdjson::dom::parser p;
        simdjson::dom::object obj = p.load(path);
        return NeuralNetwork::parse(obj);
    }

    tnn_really_inline static std::pmr::vector<NeuralNetwork> load_many(const std::string&& path, std::size_t batch_size = simdjson::dom::DEFAULT_BATCH_SIZE) noexcept {
        simdjson::dom::parser p;
        simdjson::dom::document_stream docs = p.load_many(path, batch_size);

        std::pmr::vector<NeuralNetwork> res;
        for (simdjson::dom::element doc : docs) {
            res.emplace_back(NeuralNetwork::parse(doc));
        }
        return res;
    }

    static auto parse(const simdjson::dom::object& obj) noexcept -> NeuralNetwork;
    /** @overload parse(const simdjson::dom::element& obj) */
    tnn_really_inline static auto parse(const uint8_t *buf, size_t len, bool realloc_if_needed) noexcept -> NeuralNetwork {
        simdjson::dom::parser p;
        simdjson::dom::element obj = p.parse(buf, len, realloc_if_needed);
        return parse(obj);
    }
    /* clang-format off */
    /** @overload parse(const uint8_t *buf, size_t len, bool realloc_if_needed) */
    tnn_really_inline static auto parse(const char *buf, size_t len, bool realloc_if_needed) noexcept -> NeuralNetwork
    { return parse((const uint8_t *)buf, len, realloc_if_needed); }
    /** @overload parse(const uint8_t *buf, size_t len, bool realloc_if_needed) */
    tnn_really_inline static auto parse(const std::string& s) noexcept -> NeuralNetwork
    { return parse(s.data(), s.length(), s.capacity() - s.length() < simdjson::SIMDJSON_PADDING); }
    /** @overload parse(const uint8_t *buf, size_t len, bool realloc_if_needed) */
    tnn_really_inline static auto parse(const simdjson::padded_string& s) noexcept -> NeuralNetwork
    { return parse(s.data(), s.length(), false); }
    /* clang-format on */

    tnn_really_inline static std::pmr::vector<NeuralNetwork> parse_many(const uint8_t *buf, std::size_t len, std::size_t batch_size = simdjson::dom::DEFAULT_BATCH_SIZE) noexcept {
        simdjson::dom::parser p;
        std::pmr::vector<NeuralNetwork> res;
        for (simdjson::dom::object obj : p.parse_many(buf, len, batch_size)) {
            res.emplace_back(NeuralNetwork::parse(obj));
        }
        return res;
    }
    /* clang-format off */
    /** @overload parse_many(const uint8_t *buf, std::size_t len, bool realloc_if_needed) */
    tnn_really_inline static std::pmr::vector<NeuralNetwork> parse_many(const char *buf, std::size_t len, std::size_t batch_size = simdjson::dom::DEFAULT_BATCH_SIZE) noexcept
    { return parse_many((const uint8_t *)buf, len, batch_size); }
    /** @overload parse_many(const uint8_t *buf, std::size_t len, bool realloc_if_needed) */
    tnn_really_inline static std::pmr::vector<NeuralNetwork> parse_many(const std::string& s, std::size_t batch_size = simdjson::dom::DEFAULT_BATCH_SIZE) noexcept
    { return parse_many(s.data(), s.length(), batch_size); }
    /** @overload parse_many(const uint8_t *buf, std::size_t len, bool realloc_if_needed) */
    tnn_really_inline static std::pmr::vector<NeuralNetwork> parse_many(const simdjson::padded_string& s, std::size_t batch_size = simdjson::dom::DEFAULT_BATCH_SIZE) noexcept
    { return parse_many(s.data(), s.length(), batch_size); }
    /* clang-format on */

    // Delete.
    constexpr NeuralNetwork(const NeuralNetwork&) = delete;
    constexpr auto operator=(const NeuralNetwork&) -> NeuralNetwork& = delete;

    /** @private We do not want to allow implicit conversion from C string to std::string. */
    tnn_really_inline static auto parse(const char *buf) noexcept -> NeuralNetwork = delete;

    tnn_really_inline static std::pmr::vector<NeuralNetwork> parse_many(const std::string &&s, std::size_t batch_size) = delete;// unsafe
    tnn_really_inline static std::pmr::vector<NeuralNetwork> parse_many(const simdjson::padded_string &&s, std::size_t batch_size) = delete;// unsafe
    /** @private We do not want to allow implicit conversion from C string to std::string. */
    tnn_really_inline static std::pmr::vector<NeuralNetwork> parse_many(const char *buf, std::size_t batch_size = simdjson::dom::DEFAULT_BATCH_SIZE) noexcept = delete;

  private:
    // Variables
    std::uint32_t input_nodes{};
    std::uint32_t hidden_nodes{};
    std::uint32_t output_nodes{};

    double learning_rate{};

    Matrix::function_t activation_function{};

    /*
     * [0] = weights_ih
     * [1] = weights_ho
     * [2] = bias_h
     * [3] = bias_o
     */
    std::array<Matrix, 4> matrices{};
};
};  // namespace vnepogodin

#endif  // NN_HPP_
