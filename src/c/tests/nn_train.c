#include "../nn.h"

int main(void) {
    register NeuralNetwork *nn = neural_network_new_with_args(5, 25, 1);

    const float input[2] = { 1.F, 0.F };

    neural_network_train(nn, input, neural_network_predict(nn, input));

    neural_network_free(nn);
    return 0;
}
