#include <vnepogodin/nn.hpp>  // class NeuralNetwork

using vnepogodin::NeuralNetwork;

auto main() -> int {
    const NeuralNetwork nn(2, 4, 1);
    const NeuralNetwork temp(nn);
}
