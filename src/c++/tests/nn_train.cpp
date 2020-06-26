#include "../nn.hpp"

auto main() -> int {
    NeuralNetwork nn(5, 25, 1);

    const float input[] = { 1.F, 0.F };

    nn.train(input, nn.predict(input));

    return 0;
}
