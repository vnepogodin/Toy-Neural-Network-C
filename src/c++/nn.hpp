// Other techniques for learning
#ifndef __NN_HPP__
#define __NN_HPP__

#include "Matrix.hpp"

class NeuralNetwork {
 public:
    // Constructors
    NeuralNetwork(const NeuralNetwork &);
    NeuralNetwork(const int, const int, const int);

    // Destructor
    virtual ~NeuralNetwork();

    // Functions
    auto predict(const float* const) -> const float*;
    void setLearningRate(const float);
    void setActivationFunction(float (*const)(float));
    void train(const float* const, const float* const);
    static auto serialize(const NeuralNetwork &) -> std::string;
    auto copy() -> NeuralNetwork;

    // Static function
    static auto deserialize(const nlohmann::json &t) -> NeuralNetwork;

 private:
    // Variables
    int input_nodes, hidden_nodes, output_nodes;

    float learning_rate;

    float (*activation_function)(float);

    Matrix weights_ih, weights_ho, bias_h, bias_o;
};

#endif // __NN_HPP__
