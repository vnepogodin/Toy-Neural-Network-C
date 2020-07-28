#include "../../../single_include/c++/nn.hpp"

#include <iostream>

auto main() -> int {
    NeuralNetwork nn(5, 25, 1);

    const float input[] = { 1.F, 0.F };

    const float* output = nn.predict(input);

    std::cout << output[0] << "\n";

    return 0;
}
