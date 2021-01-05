// Other techniques for learning
#ifndef NN_HPP_
#define NN_HPP_

#include <vnepogodin/Matrix.hpp>  // class Matrix

namespace vnepogodin {
enum class Function : std::uint8_t {
    sigmoid = 1,
    dsigmoid = 2,
};

class NeuralNetwork {
  public:
    // Constructors.
    inline NeuralNetwork() = default;
    inline NeuralNetwork(const NeuralNetwork&) = default;
    inline NeuralNetwork(NeuralNetwork&&) = default;
    NeuralNetwork(const std::uint32_t&, const std::uint32_t&, const std::uint32_t&);

    // Destructor.
    virtual ~NeuralNetwork() = default;

    // Operator.
    auto operator=(NeuralNetwork&&) -> NeuralNetwork& = default;

    // Functions
    auto predict(Matrix::const_pointer const&) const noexcept -> Matrix::pointer;
    constexpr void setLearningRate(const double& lr)
    { this->learning_rate = lr; }
    void setActivationFunction(const Function&);
    void train(Matrix::const_pointer const&, Matrix::const_pointer const&);
    auto serialize() const noexcept -> string;

    // Static function
    static auto deserialize(const simdjson::dom::element&) -> NeuralNetwork;

    // Delete.
    auto operator=(const NeuralNetwork&) -> NeuralNetwork& = delete;

  private:
    // Variables
    std::uint32_t input_nodes{};
    std::uint32_t hidden_nodes{};
    std::uint32_t output_nodes{};

    double learning_rate{};

    Matrix::function_t activation_function{};

    Matrix weights_ih{};
    Matrix weights_ho{};
    Matrix bias_h{};
    Matrix bias_o{};
};
};  // namespace vnepogodin

#endif  // NN_HPP_
