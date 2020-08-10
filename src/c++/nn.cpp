// Other techniques for learning
#include "../../include/c++/Matrix.hpp"
#include "../../include/c++/nn.hpp"

#include <cmath>  // std::exp

using nn_function = float_t (*const)(float_t);  // Function alias

// Non member functions
static inline auto sigmoid(float_t x) -> float_t {
    return 1.F / (1.F + std::exp(-x));
}

static inline auto dsigmoid(float_t y) -> float_t {
    // return sigmoid(x) * (1 - sigmoid(x));
    return y * (1.F - y);
}

static inline int32_t convert_ActivationFunction(nn_function &func) {
    return (*(func) == dsigmoid) ? 2 : 1;
}


// Constructor
NeuralNetwork::NeuralNetwork(const int32_t &input, const int32_t &hidden, const int32_t &output)
    : input_nodes(input), hidden_nodes(hidden), output_nodes(output),
      // Rate
      learning_rate(0.1F),
      // Function
      activation_function(&sigmoid),
      // Matrix
      weights_ih(hidden, input),
      weights_ho(output, hidden),
      bias_h(hidden, 1),
      bias_o(output, 1)
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


// Functions
auto NeuralNetwork::predict(const float_t* const &input_array) const noexcept -> float_t* {
    // Generating the Hidden Outputs
    const Matrix inputs = Matrix::fromArray(input_array, this->input_nodes);
    Matrix hidden = Matrix::multiply(this->weights_ih, inputs);
    hidden += this->bias_h;

    // activation function!
    hidden.map(this->activation_function);

    // Generating the output's output!
    Matrix output = Matrix::multiply(this->weights_ho, hidden);
    output += this->bias_o;
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
void NeuralNetwork::setActivationFunction(const uint8_t &flag) {
    this->activation_function = nullptr;

    if (flag & FUNC_SIGMOID)
        this->activation_function = sigmoid;
    else if (flag & FUNC_DSIGMOID)
        this->activation_function = dsigmoid;
}


// Training neural network
//
void NeuralNetwork::train(const float_t* const &input_array, const float_t* const& target_array) {
    // Generating the Hidden Outputs
    const Matrix inputs = Matrix::fromArray(input_array, this->input_nodes);
    Matrix hidden = Matrix::multiply(this->weights_ih, inputs);
    hidden += this->bias_h;
    // activation function!
    hidden.map(sigmoid);

    // Generating the output's output!
    Matrix outputs = Matrix::multiply(this->weights_ho, hidden);
    outputs += this->bias_o;
    outputs.map(sigmoid);

    // Convert array to matrix object
    const Matrix targets = Matrix::fromArray(target_array, this->output_nodes);

    // Calculate the error
    // ERROR = TARGETS - OUTPUTS
    const Matrix output_errors = Matrix::subtract(targets, outputs);

    // let gradient = outputs * (1 - outputs);
    // Calculate gradient
    Matrix gradients = Matrix::map(outputs, dsigmoid);
    gradients *= output_errors;
    gradients *= this->learning_rate;


    // Calculate deltas
    const Matrix hidden_T = Matrix::transpose(hidden);
    const Matrix weight_ho_deltas = Matrix::multiply(gradients, hidden_T);

    // Adjust the weights by deltas
    this->weights_ho += weight_ho_deltas;
    // Adjust the bias by its deltas (which is just the gradients)
    this->bias_o += gradients;

    // Calculate the hidden layer errors
    const Matrix who_t = Matrix::transpose(this->weights_ho);
    const Matrix hidden_errors = Matrix::multiply(who_t, output_errors);

    // Calculate hidden gradient
    Matrix hidden_gradient = Matrix::map(hidden, dsigmoid);
    hidden_gradient *= hidden_errors;
    hidden_gradient *= this->learning_rate;

    // Calcuate input->hidden deltas
    const Matrix inputs_T = Matrix::transpose(inputs);
    const Matrix weight_ih_deltas = Matrix::multiply(hidden_gradient, inputs_T);

    this->weights_ih += weight_ih_deltas;
    // Adjust the bias by its deltas (which is just the gradients)
    this->bias_h += hidden_gradient;
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
    NeuralNetwork nn(t["input_nodes"].get<int32_t>(),
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
