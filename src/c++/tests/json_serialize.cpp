#include "../../../include/c++/nn.hpp"  // class NeuralNetwork

#include <iostream>  // std::cout

auto main() -> int {
    NeuralNetwork nn(2, 4, 1);

    std::cout << nn.serialize().dump(2) << "\n";

    return 0;
}
