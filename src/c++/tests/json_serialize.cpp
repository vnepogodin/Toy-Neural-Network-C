#include "../../../include/c++/nn.hpp"

#include <iostream>

auto main() -> int {
    NeuralNetwork nn(5, 25, 1);

    std::cout << nn.serialize().dump(2) << "\n";

    return 0;
}

