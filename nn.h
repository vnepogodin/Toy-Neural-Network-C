/* Other techniques for learning */
#ifndef __NN_H__
#define __NN_H__

#include "matrix.h"

typedef struct _NeuralNetwork NeuralNetwork;

struct _NeuralNetwork {
    // Variables
    int input_nodes, hidden_nodes, output_nodes;

    float learning_rate;

    Matrix *weights_ih, *weights_ho, *bias_h, *bias_o;

    float (*activation_function)(float);
};

/* Constructors */
NeuralNetwork* neural_network_new_with_nn(const NeuralNetwork *a, const int hidden_nodes, const int output_nodes);

NeuralNetwork* neural_network_new_with_args(const int input_nodes, const int hidden_nodes, const int output_nodes);

/* Deconstructor */
void neural_network_free(NeuralNetwork *nn);

/* Functions */
const float* neural_network_predict(const NeuralNetwork *nn, const float *input_array);

void neural_network_setLearningRate(NeuralNetwork *nn, const float lr);

void neural_network_setActivationFunction(NeuralNetwork *nn, float (*func)(float));

void neural_network_train(NeuralNetwork *nn, const float *input_array, const float *target_array);

NeuralNetwork* neural_network_copy(const NeuralNetwork *nn);

#endif /* __NN_H__ */
