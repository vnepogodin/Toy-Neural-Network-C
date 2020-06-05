#include "../nn.h"

#include <stdio.h>

int main() {
    register NeuralNetwork *nn = neural_network_new_with_args(5, 25, 1);

    const float input[2] = { 1, 0 };
    const float* output =  neural_network_predict(nn, input);
    printf("%f", output[0]);

    neural_network_free(nn);
    return 0;
}
