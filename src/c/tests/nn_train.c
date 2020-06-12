#include "../nn.h"

#include <stdio.h>

int main() {
    register NeuralNetwork *nn = neural_network_new_with_args(5, 25, 1);

    const float input[] = { 1.f, 0.f };

    neural_network_train(nn, input, neural_network_predict(nn, input));

    neural_network_free(nn);
    return 0;
}
