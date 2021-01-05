#include <vnepogodin/nn.hpp>  // class NeuralNetwork

using vnepogodin::NeuralNetwork;

#include <iostream>  // std::cout

auto main() -> int32_t {
    NeuralNetwork nn(2, 4, 1);

    std::cout << nn.serialize() << '\n';
}
