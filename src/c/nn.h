/* Other techniques for learning */
#ifndef __NN_H__
#define __NN_H__

#include "matrix.h"

typedef struct _NeuralNetwork NeuralNetwork;

struct _NeuralNetwork {
    /* Variables */
    int input_nodes, hidden_nodes, output_nodes;

    float learning_rate;

	float (*activation_function)(float);

    Matrix *weights_ih, *weights_ho, *bias_h, *bias_o;    
};

/* Constructors */
NeuralNetwork* neural_network_new_with_nn(const NeuralNetwork *);
NeuralNetwork* neural_network_new_with_args(const int, const int, const int);

/* Deconstructor */
void neural_network_free(register NeuralNetwork *__restrict);

/* Functions */
const float* neural_network_predict(const NeuralNetwork *, const float* __restrict);
void neural_network_setLearningRate(register NeuralNetwork *__restrict, const float);
void neural_network_setActivationFunction(register NeuralNetwork *__restrict, float (*)(float));
void neural_network_train(register NeuralNetwork *, const float* __restrict, const float* __restrict);
const json_object* neural_network_serialize(const NeuralNetwork *__restrict);
NeuralNetwork* neural_network_copy(const NeuralNetwork *__restrict);
NeuralNetwork* neural_network_deserialize(const json_object *__restrict);

#endif /* __NN_H__ */
