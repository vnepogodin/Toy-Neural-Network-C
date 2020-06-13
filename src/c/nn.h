/* Other techniques for learning */
#ifndef __NN_H__
#define __NN_H__

#include <json.h> /* json_object */

typedef struct _NeuralNetwork NeuralNetwork;

/* Constructors */
NeuralNetwork* neural_network_new_with_nn(const NeuralNetwork *__restrict const);
NeuralNetwork* neural_network_new_with_args(const int, const int, const int);

/* Destructor */
void neural_network_free(register NeuralNetwork *__restrict);

/* Functions */
const float* neural_network_predict(const NeuralNetwork *const, const float* __restrict const);

void neural_network_train(register NeuralNetwork *, const float* __restrict const, const float* __restrict const);

void neural_network_setLearningRate(register NeuralNetwork *__restrict, const float);
void neural_network_setActivationFunction(register NeuralNetwork *__restrict, float (*const)(float));

void* neural_network_getActivationFunction(const NeuralNetwork *__restrict const);

const json_object* neural_network_serialize(const NeuralNetwork *__restrict const);
NeuralNetwork* neural_network_copy(const NeuralNetwork *__restrict const);
NeuralNetwork* neural_network_deserialize(const json_object *__restrict const);

#endif /* __NN_H__ */
