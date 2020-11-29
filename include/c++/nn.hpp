// Other techniques for learning
#ifndef NN_HPP_
#define NN_HPP_

#include "Matrix.hpp"

namespace TNN {
    enum class Function: uint8_t {
        sigmoid = 1,
        dsigmoid = 2,
    };
};

class NeuralNetwork {
 public:
    // Constructors.
    inline NeuralNetwork() = default;
    inline NeuralNetwork(const NeuralNetwork &) = default;
    inline NeuralNetwork(NeuralNetwork&&) = default;
    NeuralNetwork(const uint32_t&, const uint32_t&, const uint32_t&);

    // Destructor.
    virtual ~NeuralNetwork() = default;

    // Operator.
    auto operator=(NeuralNetwork&&) -> NeuralNetwork& = default;

    // Functions
    auto predict(const float* const &) const noexcept -> float*;
    constexpr void setLearningRate(const float &lr)
    { this->learning_rate = lr; }
    void setActivationFunction(const TNN::Function&);
    void train(const float* const&, const float* const&);
    auto serialize() noexcept -> string;

    // Static function
    static auto deserialize(const simdjson::dom::element&) -> NeuralNetwork;

    // Delete.
    auto operator=(const NeuralNetwork&) -> NeuralNetwork& = delete;

 private:
    // Variables
    uint32_t input_nodes{};
    uint32_t hidden_nodes{};
    uint32_t output_nodes{};

    float learning_rate{};

    float (*activation_function)(float){};

    Matrix weights_ih{};
    Matrix weights_ho{};
    Matrix bias_h{};
    Matrix bias_o{};
};

#endif  // NN_HPP_
