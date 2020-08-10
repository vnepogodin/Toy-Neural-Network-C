#include "../../../include/c++/nn.hpp"  // class NeuralNetwork

auto main() -> int {
    NeuralNetwork nn(2, 4, 1);

    NeuralNetwork temp(nn);

    return 0;
}
