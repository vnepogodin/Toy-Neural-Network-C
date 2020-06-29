#include "../nn.h"

int main(void) {
    register NeuralNetwork *nn = neural_network_new_with_args(5, 25, 1);

    const float input[5] = { 1.F, 0.F, 0.F, 0.F, 0.F };

    register float output[1] = { 0 };

    neural_network_predict(output, nn, input);

    neural_network_train(nn, input, output);

    neural_network_free(nn);
    return 0;
}
