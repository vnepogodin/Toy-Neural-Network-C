#include "../../../include/c/nn.h"  /* NeuralNetwork */

#include <stdlib.h>  /* free */

int main(void) {
    register NeuralNetwork *nn = neural_network_new_with_args(5, 25, 1);

    const float input[5] = { 1.F, 0.F, 0.F, 0.F, 0.F };

    float* const output = neural_network_predict(nn, input);

    neural_network_train(nn, input, output);

    free(output);

    neural_network_free(nn);
    return 0;
}
