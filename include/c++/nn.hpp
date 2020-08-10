// Other techniques for learning
#ifndef __NN_HPP__
#define __NN_HPP__

#include "Matrix.hpp"

/**
 * FUNC_SIGMOID:
 * @see nn.cpp 21-23.
 *
 * Flag to sigmoid float_t function.
 */
#define FUNC_SIGMOID 0x01U

/**
 * FUNC_DSIGMOID:
 * @see nn.cpp 25-28.
 *
 * Flag to dsigmoid float_t function.
 */
#define FUNC_DSIGMOID 0x02U

class NeuralNetwork {
 public:
    // Constructors
    NeuralNetwork(const NeuralNetwork &);
    NeuralNetwork(const int32_t &input_nodes, const int32_t &hidden_nodes, const int32_t &output_nodes);

    // Destructor
    virtual ~NeuralNetwork() = default;

    // Functions
    auto predict(const float_t* const &) const noexcept -> float_t*;
    void setLearningRate(const float_t&);
    void setActivationFunction(const uint8_t&);
    void train(const float_t* const&, const float_t* const&);
    auto serialize() const -> const nlohmann::json;

    // Static function
    static auto deserialize(const nlohmann::json &t) -> NeuralNetwork;

 private:
    // Variables
    int32_t input_nodes, hidden_nodes, output_nodes;

    float_t learning_rate;

    float_t (*activation_function)(float_t);

    Matrix weights_ih, weights_ho, bias_h, bias_o;
};

#endif  // __NN_HPP__
