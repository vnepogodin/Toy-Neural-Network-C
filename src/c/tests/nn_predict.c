#include "../nn.h"

#include <stdio.h>

int main() {
    register NeuralNetwork *nn = neural_network_new_with_args(5, 25, 1);

    const float input[] = { 1.f, 0.f };

    const float* output = neural_network_predict(nn, input);
    printf("%f\n", output[0]);

    neural_network_free(nn);
    return 0;
}
