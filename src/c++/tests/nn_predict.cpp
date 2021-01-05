#include <vnepogodin/nn.hpp>  // class NeuralNetwork

#include <iostream>  // std::cout

using vnepogodin::NeuralNetwork;

auto main() -> int32_t {
    NeuralNetwork nn(2, 4, 1);

    constexpr double input[2] { 1.F, 0.F };
    const auto output = nn.predict(input);

    std::cout << output[0] << '\n';

    delete[] output;
}
