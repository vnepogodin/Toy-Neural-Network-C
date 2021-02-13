#include <vnepogodin/nn.h>  /* NeuralNetwork */

#include <stdio.h>  /* printf */
#include <stdlib.h>  /* free */

int main(void) {
    register NeuralNetwork *nn = neural_network_new_with_args(2, 4, 1);

    const float input[2] = { 1.F, 0.F };

    float* const output = neural_network_predict(nn, input);

    printf("%f\n", (double)output[0]);

    free(output);

    neural_network_free(nn);
    return 0;
}
