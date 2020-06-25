// Other techniques for learning
#include "Matrix.hpp"
#include "nn.hpp"

#include <cmath>  // std::exp

namespace nn {
    using function = float_t (*const)(float_t);  // Function alias
};

// Non member functions
[[maybe_unused]] static inline auto sigmoid(float_t x) -> float_t {
    return 1.F / (1.F + std::exp(-x));
}

[[maybe_unused]] static inline auto dsigmoid(float_t y) -> float_t {
    // return sigmoid(x) * (1 - sigmoid(x));
    return y * (1.F - y);
}

static inline int32_t convert_ActivationFunction(nn::function &func) {
    return (*(func) == dsigmoid) ? 2 : 1;
}


// Constructor
NeuralNetwork::NeuralNetwork(const int32_t input_nodes, const int32_t hidden_nodes, const int32_t output_nodes)
    : input_nodes(input_nodes), hidden_nodes(hidden_nodes), output_nodes(output_nodes),
      // Rate
      learning_rate(0.1F),
      // Function
      activation_function(&sigmoid),
      // Matrix
      weights_ih(hidden_nodes, input_nodes),
      weights_ho(output_nodes, hidden_nodes),
      bias_h(hidden_nodes, 1),
      bias_o(output_nodes, 1)
{
    this->weights_ih.randomize();
    this->weights_ho.randomize();

    this->bias_h.randomize();
    this->bias_o.randomize();
}

// Copy constructor
NeuralNetwork::NeuralNetwork(const NeuralNetwork &a)
    : input_nodes(a.input_nodes), hidden_nodes(a.hidden_nodes), output_nodes(a.output_nodes),
      // Rate
      learning_rate(a.learning_rate),
      // Function
      activation_function(*(a.activation_function)),
      // Matrix
      weights_ih(a.weights_ih),
      weights_ho(a.weights_ho),
      bias_h(a.bias_h),
      bias_o(a.bias_o) {}

// Destructor
NeuralNetwork::~NeuralNetwork() {
    this->activation_function = nullptr;

    this->weights_ih.clear();
    this->weights_ho.clear();
    this->bias_h.clear();
    this->bias_o.clear();
}

// Functions
auto NeuralNetwork::predict(const float_t* const &input_array) const -> const float_t* const {
    // Generating the Hidden Outputs
    Matrix inputs = Matrix::fromArray(input_array);
    Matrix hidden = Matrix::multiply(this->weights_ih, inputs);
    hidden.add(this->bias_h);

    // activation function!
    hidden.map(this->activation_function);

    // Generating the output's output!
    Matrix output = Matrix::multiply(this->weights_ho, hidden);
    output.add(this->bias_o);
    output.map(this->activation_function);

    // Sending back to the caller!
    return output.toArray();
}


// Setting learning rate
//
void NeuralNetwork::setLearningRate(const float_t &lr) {
    this->learning_rate = lr;
}

// Setting function
//
void NeuralNetwork::setActivationFunction(const int32_t &flag) {
    this->activation_function = nullptr;
    
    if (flag == FUNC_SIGMOID)
        this->activation_function = sigmoid;
    else if (flag == FUNC_DSIGMOID)
        this->activation_function = dsigmoid;
}


// Training neural network
//
void NeuralNetwork::train(const float_t* const &input_array, const float* const &target_array) {
    // Generating the Hidden Outputs
    Matrix inputs = Matrix::fromArray(input_array);
    Matrix hidden = Matrix::multiply(this->weights_ih, inputs);
    hidden.add(this->bias_h);
    // activation function!
    hidden.map(this->activation_function);

    // Generating the output's output!
    Matrix outputs = Matrix::multiply(this->weights_ho, hidden);
    outputs.add(this->bias_o);
    outputs.map(this->activation_function);

    // Convert array to matrix object
    Matrix targets = Matrix::fromArray(target_array);

    // Calculate the error
    // ERROR = TARGETS - OUTPUTS
    Matrix output_errors = Matrix::subtract(targets, outputs);

    // let gradient = outputs * (1 - outputs);
    // Calculate gradient
    Matrix gradients = Matrix::map(outputs, this->activation_function);
    gradients *= output_errors;
    gradients.multiply(this->learning_rate);


    // Calculate deltas
    Matrix hidden_T = Matrix::transpose(hidden);
    Matrix weight_ho_deltas = Matrix::multiply(gradients, hidden_T);

    // Adjust the weights by deltas
    this->weights_ho.add(weight_ho_deltas);
    // Adjust the bias by its deltas (which is just the gradients)
    this->bias_o.add(gradients);

    // Calculate the hidden layer errors
    Matrix who_t = Matrix::transpose(this->weights_ho);
    Matrix hidden_errors = Matrix::multiply(who_t, output_errors);

    // Calculate hidden gradient
    Matrix hidden_gradient = Matrix::map(hidden, this->activation_function);

    // FIXME
    hidden_gradient.add(hidden_errors);
    hidden_gradient.add(this->learning_rate);

    // Calcuate input->hidden deltas
    Matrix inputs_T = Matrix::transpose(inputs);
    Matrix weight_ih_deltas = Matrix::multiply(hidden_gradient, inputs_T);

    this->weights_ih.add(weight_ih_deltas);
    // Adjust the bias by its deltas (which is just the gradients)
    this->bias_h.add(hidden_gradient);
}


// Serialize to JSON
//
auto NeuralNetwork::serialize() const -> const nlohmann::json {
    nlohmann::json t;
    t["input_nodes"] = this->input_nodes;
    t["hidden_nodes"] = this->hidden_nodes;
    t["output_nodes"] = this->output_nodes;

    t["weights_ih"] = this->weights_ih.serialize();
    t["weights_ho"] = this->weights_ho.serialize();
    t["bias_h"] = this->bias_h.serialize();
    t["bias_o"] = this->bias_o.serialize();

    t["learning_rate"] = this->learning_rate;
    t["activation_function"] = convert_ActivationFunction(this->activation_function);
    
    return t;
}


// Deserialize from JSON
//
auto NeuralNetwork::deserialize(const nlohmann::json &t) -> NeuralNetwork {
    NeuralNetwork nn = NeuralNetwork(t["input_nodes"].get<int32_t>(),
                                     t["hidden_nodes"].get<int32_t>(),
                                     t["output_nodes"].get<int32_t>());

    nn.weights_ih = Matrix::deserialize(t["weights_ih"]);
    nn.weights_ho = Matrix::deserialize(t["weights_ho"]);
    nn.bias_h = Matrix::deserialize(t["bias_h"]);
    nn.bias_o = Matrix::deserialize(t["bias_o"]);

    nn.setLearningRate(t["learning_rate"].get<float_t>());
    nn.setActivationFunction(t["activation_function"].get<int32_t>());

    return nn;
}
