// Other techniques for learning
#ifndef __NN_HPP__
#define __NN_HPP__

#include "Matrix.hpp"

/**
 * FUNC_SIGMOID:
 * @see nn.cpp 10-12.
 *
 * Flag to sigmoid float_t function.
 */
#define FUNC_SIGMOID 0x01U

/**
 * FUNC_DSIGMOID:
 * @see nn.cpp 14-17.
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
    auto predict(const float* const &) const noexcept -> float*;
    void setLearningRate(const float&);
    void setActivationFunction(const uint8_t&);
    void train(const float* const&, const float* const&);
    auto serialize() const noexcept -> const string;

    // Static function
    static auto deserialize(const simdjson::dom::element&) -> NeuralNetwork;

 private:
    // Variables
    int32_t input_nodes;
    int32_t hidden_nodes;
    int32_t output_nodes;

    float learning_rate;

    float (*activation_function)(float);

    Matrix weights_ih;
    Matrix weights_ho;
    Matrix bias_h;
    Matrix bias_o;
};

#endif  // __NN_HPP__
