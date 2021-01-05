/* Other techniques for learning */
#ifndef __NN_H__
#define __NN_H__

#include <vnepogodin/matrix.h>  /* Matrix */

typedef struct _NeuralNetwork NeuralNetwork;

/**
 * Flag to sigmoid float function.
 * @see nn.c 21-23.
 *
 */
#define FUNC_SIGMOID 0x01U

/**
 * Flag to dsigmoid float function.
 * @see nn.c 25-28.
 *
 */
#define FUNC_DSIGMOID 0x02U

#ifdef __cplusplus
extern "C" {
#endif

/* Constructors */

NeuralNetwork* neural_network_new_with_nn(const NeuralNetwork *const);
NeuralNetwork* neural_network_new_with_args(const int, const int, const int);

/* Destructor */

void neural_network_free(register NeuralNetwork *__restrict);

/* Functions */

float* neural_network_predict(const NeuralNetwork *const, const float* __restrict const);
void neural_network_train(register NeuralNetwork *, const float* __restrict const, const float* __restrict const);
void neural_network_setLearningRate(register NeuralNetwork *__restrict, const float);
void neural_network_setActivationFunction(register NeuralNetwork *__restrict, const unsigned char);
json_object* neural_network_serialize(const NeuralNetwork *__restrict const);

/* Static functions */

NeuralNetwork* neural_network_deserialize(const json_object *__restrict const);

#ifdef __cplusplus
}
#endif

#endif /* __NN_H__ */
