// Other techniques for learning
#include <vnepogodin/nn.hpp>  // class NeuralNetwork

#include <cmath>  // std::exp

using namespace vnepogodin;

namespace {
// Non member functions
inline auto sigmoid(double x) -> double {
    return 1.0 / (1.0 + std::exp(-x));
}

inline auto dsigmoid(double y) -> double {
    // return sigmoid(x) * (1 - sigmoid(x));
    return y * (1.0 - y);
}

constexpr auto convert_ActivationFunction(const Matrix::function_t& func) -> std::uint8_t {
    return (tnn_unlikely(*func == dsigmoid)) ? 2U : 1U;
}

constexpr double default_lr = 0.1;
};  // namespace

namespace vnepogodin {
// Constructor
NeuralNetwork::NeuralNetwork(const std::uint32_t& input,
                             const std::uint32_t& hidden,
                             const std::uint32_t& output)
  : input_nodes(input)
  , hidden_nodes(hidden)
  , output_nodes(output)
  ,
  // Rate
  learning_rate(default_lr)
  ,
  // Function
  activation_function(&sigmoid) {
    this->matrices[0] = new Matrix(hidden, input);
    this->matrices[1] = new Matrix(output, hidden);
    this->matrices[2] = new Matrix(hidden, 1);
    this->matrices[3] = new Matrix(output, 1);

    for (auto& mat : this->matrices) {
        mat->randomize();
    }
}

// Functions
auto NeuralNetwork::predict(Matrix::const_pointer input_array) const noexcept -> Matrix::pointer {
    // Generating the Hidden Outputs
    const Matrix& inputs = Matrix::fromArray(input_array, this->input_nodes);
    Matrix hidden = Matrix::multiply(*this->matrices[0], inputs);
    hidden += *this->matrices[2];

    // activation function!
    hidden.map(this->activation_function);

    // Generating the output's output!
    Matrix output = Matrix::multiply(*this->matrices[1], hidden);
    output += *this->matrices[3];
    output.map(this->activation_function);

    // Sending back to the caller!
    return output.toArray();
}

// Setting function
//
void NeuralNetwork::setActivationFunction(const Function& flag) noexcept {
    this->activation_function = nullptr;

    if (tnn_likely(flag == Function::sigmoid)) {
        this->activation_function = sigmoid;
    } else if (tnn_unlikely(flag == Function::dsigmoid)) {
        this->activation_function = dsigmoid;
    }
}

// Training neural network
//
void NeuralNetwork::train(Matrix::const_pointer input_array,
                          Matrix::const_pointer target_array) noexcept {
    // Generating the Hidden Outputs
    const Matrix& inputs = Matrix::fromArray(input_array, this->input_nodes);
    Matrix hidden = Matrix::multiply(*this->matrices[0], inputs);
    hidden += *this->matrices[2];
    // activation function!
    hidden.map(sigmoid);

    // Generating the output's output!
    Matrix outputs = Matrix::multiply(*this->matrices[1], hidden);
    outputs += *this->matrices[3];
    outputs.map(sigmoid);

    // Convert array to matrix object
    const Matrix& targets = Matrix::fromArray(target_array, this->output_nodes);

    // Calculate the error
    // ERROR = TARGETS - OUTPUTS
    const Matrix& output_errors = Matrix::subtract(targets, outputs);

    // let gradient = outputs * (1 - outputs);
    // Calculate gradient
    Matrix gradients = Matrix::map(outputs, dsigmoid);
    gradients *= output_errors;
    gradients *= this->learning_rate;

    // Calculate deltas
    const Matrix& hidden_T = Matrix::transpose(hidden);
    const Matrix& weight_ho_deltas = Matrix::multiply(gradients, hidden_T);

    // Adjust the weights by deltas
    *this->matrices[1] += weight_ho_deltas;
    // Adjust the bias by its deltas (which is just the gradients)
    *this->matrices[3] += gradients;

    // Calculate the hidden layer errors
    const Matrix& who_t = Matrix::transpose(*this->matrices[1]);
    const Matrix& hidden_errors = Matrix::multiply(who_t, output_errors);

    // Calculate hidden gradient
    Matrix hidden_gradient = Matrix::map(hidden, dsigmoid);
    hidden_gradient *= hidden_errors;
    hidden_gradient *= this->learning_rate;

    // Calcuate input->hidden deltas
    const Matrix& inputs_T = Matrix::transpose(inputs);
    const Matrix& weight_ih_deltas = Matrix::multiply(hidden_gradient, inputs_T);

    *this->matrices[0] += weight_ih_deltas;
    // Adjust the bias by its deltas (which is just the gradients)
    *this->matrices[2] += hidden_gradient;
}

// Serialize to JSON
// TODO: Refactor
//
auto NeuralNetwork::dumps() const noexcept -> std::string {
    constexpr int resLen = 500;
    auto _str = std::string("{\"activation_function\":") + std::to_string(convert_ActivationFunction(this->activation_function));

    _str.reserve(resLen);

    _str += std::string(",\"bias_h\":") + this->matrices[2]->dumps() + std::string(",\"bias_o\":") + this->matrices[3]->dumps() + std::string(",\"input_nodes\":") + std::to_string(this->input_nodes) + std::string(",\"hidden_nodes\":") + std::to_string(this->hidden_nodes) + std::string(",\"output_nodes\":") + std::to_string(this->output_nodes) + std::string(",\"weights_ih\":") + this->matrices[0]->dumps() + std::string(",\"weights_ho\":") + this->matrices[1]->dumps() + std::string(",\"learning_rate\":") + std::to_string(this->learning_rate) + '}';

    _str.shrink_to_fit();
    return _str;
}

// Copy factory
//
auto NeuralNetwork::copy(const NeuralNetwork& nn) noexcept -> NeuralNetwork {
    const std::uint32_t input = nn.input_nodes;
    const std::uint32_t hidden = nn.hidden_nodes;
    const std::uint32_t output = nn.output_nodes;

    NeuralNetwork tmp;
    tmp.input_nodes = input;
    tmp.hidden_nodes = hidden;
    tmp.output_nodes = output;

    tmp.learning_rate = nn.learning_rate;
    tmp.activation_function = nn.activation_function;

    tmp.matrices[0] = new Matrix(hidden, input);
    tmp.matrices[1] = new Matrix(output, hidden);
    tmp.matrices[2] = new Matrix(hidden, 1);
    tmp.matrices[3] = new Matrix(output, 1);

    for (int i = 0; i < nn.matrices.size(); ++i) {
        std::memcpy(tmp.matrices[i]->data(), nn.matrices[i]->data(), nn.matrices[i]->size());
    }

    return tmp;
}

// Deserialize from JSON
//
auto NeuralNetwork::parse(const simdjson::dom::object& obj) noexcept -> NeuralNetwork {
    const std::uint32_t& in_nodes = static_cast<std::uint64_t>(obj["input_nodes"]);
    const std::uint32_t& h_nodes = static_cast<std::uint64_t>(obj["hidden_nodes"]);
    const std::uint32_t& out_nodes = static_cast<std::uint64_t>(obj["output_nodes"]);

    const auto& func = static_cast<Function>(static_cast<std::uint64_t>(obj["activation_function"]));

    NeuralNetwork nn;
    nn.input_nodes = in_nodes;
    nn.hidden_nodes = h_nodes;
    nn.output_nodes = out_nodes;

    nn.learning_rate = obj["learning_rate"];
    nn.setActivationFunction(func);

    for (auto& mat : nn.matrices) {
        mat = new Matrix;
    }

    *nn.matrices[0] = Matrix::parse(obj["weights_ih"]);
    *nn.matrices[1] = Matrix::parse(obj["weights_ho"]);
    *nn.matrices[2] = Matrix::parse(obj["bias_h"]);
    *nn.matrices[3] = Matrix::parse(obj["bias_o"]);

    return nn;
}
};  // namespace vnepogodin
