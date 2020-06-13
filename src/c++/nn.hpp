// Other techniques for learning
#ifndef __NN_HPP__
#define __NN_HPP__

#include "Matrix.hpp"
#include <cmath> // exp

class NeuralNetwork {
public:
    // Constructors
    NeuralNetwork(const NeuralNetwork &);
    NeuralNetwork(const int, const int, const int);

    // Destructor
    virtual ~NeuralNetwork(void);

    // Functions
    const float* predict(const float* const);
    void setLearningRate(const float);
    void setActivationFunction(float (*const)(float));
    void train(const float* const, const float* const);
    const nlohmann::json serialize(const NeuralNetwork &);
    NeuralNetwork copy(void);

    // Static function
    static NeuralNetwork deserialize(const nlohmann::json &t) {
        NeuralNetwork nn = *new NeuralNetwork(t["input_nodes"].get<int>(), t["hidden_nodes"].get<int>(), t["output_nodes"].get<int>());

        nn.weights_ih = Matrix::deserialize(t["weights_ih"]);
        nn.weights_ho = Matrix::deserialize(t["weights_ho"]);
        nn.bias_h = Matrix::deserialize(t["bias_h"]);
        nn.bias_o = Matrix::deserialize(t["bias_o"]);

        nn.setLearningRate(t["learning_rate"].get<float>());

        return nn;
    }

private:
    // Variables
    int input_nodes, hidden_nodes, output_nodes;

    float learning_rate;

    float (*activation_function)(float);

    Matrix weights_ih, weights_ho, bias_h, bias_o;
};

#endif // __NN_HPP__
