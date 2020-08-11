#include "../../../include/c/nn.h"  /* NeuralNetwork */

int main(void) {
    register NeuralNetwork *nn = neural_network_new_with_args(2, 4, 1);

    register NeuralNetwork *temp = neural_network_new_with_nn(nn);

    neural_network_free(temp);
    neural_network_free(nn);
    return 0;
}
