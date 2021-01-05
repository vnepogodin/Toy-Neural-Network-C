// Other techniques for learning
#include <vnepogodin/nn.hpp>  // class NeuralNetwork

#include <cmath>  // std::exp

namespace {
// Non member functions
static inline auto sigmoid(double x) -> double {
    return 1.F / (1.F + std::exp(-x));
}

static inline auto dsigmoid(double y) -> double {
    // return sigmoid(x) * (1 - sigmoid(x));
    return y * (1.F - y);
}

static constexpr auto convert_ActivationFunction(const vnepogodin::Matrix::function_t& func) -> std::uint8_t {
    return (*func == dsigmoid) ? 2U : 1U;
}

static constexpr auto default_lr = 0.1;
};

namespace vnepogodin {
// Constructor
NeuralNetwork::NeuralNetwork(const std::uint32_t& input,
                             const std::uint32_t& hidden,
                             const std::uint32_t& output)
    : input_nodes(input),
      hidden_nodes(hidden),
      output_nodes(output),
      // Rate
      learning_rate(default_lr),
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

// Functions
auto NeuralNetwork::predict(Matrix::const_pointer const& input_array) const noexcept -> Matrix::pointer {
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

// Setting function
//
void NeuralNetwork::setActivationFunction(const Function& flag) {
    this->activation_function = nullptr;

    if (flag == Function::sigmoid) {
        this->activation_function = sigmoid;
    } else if (flag == Function::dsigmoid) {
        this->activation_function = dsigmoid;
    }
}

// Training neural network
//
void NeuralNetwork::train(Matrix::const_pointer const& input_array,
                          Matrix::const_pointer const& target_array) {
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
// TODO: Refactor
//
auto NeuralNetwork::serialize() const noexcept -> string {
    constexpr auto resLen = 500;
    auto _str = string("{\"activation_function\":")
              + to_string(convert_ActivationFunction(this->activation_function));

    _str.reserve(resLen);

    _str += string(",\"bias_h\":") + this->bias_h.serialize()
         + string(",\"bias_o\":") + this->bias_o.serialize()
         + string(",\"input_nodes\":") + to_string(this->input_nodes)
         + string(",\"hidden_nodes\":") + to_string(this->hidden_nodes)
         + string(",\"output_nodes\":") + to_string(this->output_nodes)
         + string(",\"weights_ih\":") + this->weights_ih.serialize()
         + string(",\"weights_ho\":") + this->weights_ho.serialize()
         + string(",\"learning_rate\":") + to_string(this->learning_rate)
         + '}';

    _str.shrink_to_fit();
    return _str;
}

// Deserialize from JSON
//
auto NeuralNetwork::deserialize(const simdjson::dom::element& t) -> NeuralNetwork {
    const std::uint32_t& in_nodes = static_cast<std::uint64_t>(t["input_nodes"]);
    const std::uint32_t& h_nodes = static_cast<std::uint64_t>(t["hidden_nodes"]);
    const std::uint32_t& out_nodes = static_cast<std::uint64_t>(t["output_nodes"]);

    const auto& func = static_cast<Function>(static_cast<std::uint64_t>(t["activation_function"]));

    NeuralNetwork nn(in_nodes, h_nodes, out_nodes);

    nn.weights_ih = Matrix::deserialize(t["weights_ih"]);
    nn.weights_ho = Matrix::deserialize(t["weights_ho"]);
    nn.bias_h = Matrix::deserialize(t["bias_h"]);
    nn.bias_o = Matrix::deserialize(t["bias_o"]);

    nn.learning_rate = t["learning_rate"];
    nn.setActivationFunction(func);

    return nn;
}
};  // namespace vnepogodin
