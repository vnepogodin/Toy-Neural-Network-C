// Other techniques for learning
#include "Matrix.hpp"
#include "nn.hpp"

#include <cmath>  // exp

// Non member functions
static auto sigmoid(float x) -> float {
    return 1.F / (1.F + exp(-x));
}

static auto dsigmoid(float y) -> float {
    // return sigmoid(x) * (1 - sigmoid(x));
    return y * (1.F - y);
}

// Constructors
NeuralNetwork::NeuralNetwork(const NeuralNetwork &a)
    : input_nodes(a.input_nodes), hidden_nodes(a.hidden_nodes), output_nodes(a.output_nodes) {
    this->weights_ih = Matrix(a.weights_ih);
    this->weights_ho = Matrix(a.weights_ho);

    this->bias_h = Matrix(a.bias_h);
    this->bias_o = Matrix(a.bias_o);

    // TODO: copy these as well
    this->learning_rate = a.learning_rate;
    this->activation_function = a.activation_function;
}

NeuralNetwork::NeuralNetwork(const int input_nodes, const int hidden_nodes, const int output_nodes)
    : input_nodes(input_nodes), hidden_nodes(hidden_nodes), output_nodes(output_nodes) {
    this->weights_ih = Matrix(this->hidden_nodes, this->input_nodes);
    this->weights_ho = Matrix(this->output_nodes, this->hidden_nodes);
    this->weights_ih.randomize();
    this->weights_ho.randomize();

    this->bias_h = Matrix(this->hidden_nodes, 1);
    this->bias_o = Matrix(this->output_nodes, 1);
    this->bias_h.randomize();
    this->bias_o.randomize();

    // TODO: copy these as well
    this->learning_rate = 0.1F;
    this->activation_function = &sigmoid;
}

// Destructor
NeuralNetwork::~NeuralNetwork() {
    this->activation_function = nullptr;

    this->weights_ih.clear();
    this->weights_ho.clear();
    this->bias_h.clear();
    this->bias_o.clear();
}

// Functions
auto NeuralNetwork::predict(const float* const input_array) -> const float* {
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

void NeuralNetwork::setLearningRate(const float lr) {
    this->learning_rate = lr;
}

void NeuralNetwork::setActivationFunction(float (*const func)(float)) {
    this->activation_function = *func;
}

void NeuralNetwork::train(const float* const input_array, const float* const target_array) {
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

auto NeuralNetwork::serialize(const NeuralNetwork &nn) -> const nlohmann::json {
    nlohmann::json t;
    t["input_nodes"] = nn.input_nodes;
    t["hidden_nodes"] = nn.hidden_nodes;
    t["output_nodes"] = nn.output_nodes;

    t["weights_ih"] = nn.weights_ih.serialize(weights_ih);
    t["weights_ho"] = nn.weights_ih.serialize(weights_ho);
    t["bias_h"] = nn.weights_ih.serialize(bias_h);
    t["bias_o"] = nn.weights_ih.serialize(bias_o);

    t["learning_rate"] = nn.learning_rate;

    return t.dump();
}

auto NeuralNetwork::copy() -> NeuralNetwork {
    NeuralNetwork t = NeuralNetwork(this->input_nodes, this->hidden_nodes, this->output_nodes);

    t.weights_ih = Matrix(this->weights_ih);
    t.weights_ho = Matrix(this->weights_ho);
    t.bias_h = Matrix(this->bias_h);
    t.bias_o = Matrix(this->bias_o);
       
    t.setLearningRate(this->learning_rate);

    return t;
}

auto NeuralNetwork::deserialize(const nlohmann::json &t) -> NeuralNetwork {
    NeuralNetwork nn = NeuralNetwork(t["input_nodes"].get<int>(), t["hidden_nodes"].get<int>(), t["output_nodes"].get<int>());

    nn.weights_ih = Matrix::deserialize(t["weights_ih"]);
    nn.weights_ho = Matrix::deserialize(t["weights_ho"]);
    nn.bias_h = Matrix::deserialize(t["bias_h"]);
    nn.bias_o = Matrix::deserialize(t["bias_o"]);

    nn.setLearningRate(t["learning_rate"].get<float>());

    return nn;
}
