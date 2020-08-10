#include "../../../include/c++/nn.hpp"  // class NeuralNetwork

auto main() -> int {
    NeuralNetwork nn(5, 25, 1);

    const float_t input[5] { 1.F, 0.F, 0.F, 0.F, 0.F };

    const auto output = nn.predict(input);

    nn.train(input, output);

    delete[] output;

    return 0;
}
