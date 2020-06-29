#include "../nn.h"

#include <stdio.h>

int main(void) {
    register NeuralNetwork *nn = neural_network_new_with_args(5, 25, 1);

    const float input[5] = { 1.F, 0.F, 0.F, 0.F, 0.F };

    float output[1] = { 0 };

    neural_network_predict(output, nn, input);

    printf("%f\n", output[0]);

    neural_network_free(nn);
    return 0;
}
