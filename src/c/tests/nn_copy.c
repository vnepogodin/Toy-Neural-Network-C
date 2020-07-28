#include "../../../include/c/nn.h"

int main(void) {
    register NeuralNetwork *nn = neural_network_new_with_args(5, 25, 1);

    register NeuralNetwork *temp = neural_network_new_with_nn(nn);

    neural_network_free(temp);
    neural_network_free(nn);
    return 0;
}
