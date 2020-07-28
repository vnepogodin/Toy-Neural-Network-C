#include "../../../single_include/c++/nn.hpp"

auto main() -> int {
    NeuralNetwork nn(5, 25, 1);

    NeuralNetwork temp(nn);

    return 0;
}
