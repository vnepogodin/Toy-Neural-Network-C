/* Other techniques for learning */
#ifndef __NN_H__
#define __NN_H__

#include <json-c/json_object.h> /* json_object */

typedef struct _NeuralNetwork NeuralNetwork;

/**
 * FUNC_SIGMOID:
 * @see nn.c 21-23.
 *
 * Flag to sigmoid float function.
 */
#define FUNC_SIGMOID 1

/**
 * FUNC_DSIGMOID:
 * @see nn.c 25-28.
 *
 * Flag to dsigmoid float function.
 */
#define FUNC_DSIGMOID 2

#ifdef __cplusplus
    #define NN_API extern "C"
#else
    #define NN_API extern
#endif

/* Constructors */
NN_API NeuralNetwork* neural_network_new_with_nn(const NeuralNetwork *const);
NN_API NeuralNetwork* neural_network_new_with_args(const int, const int, const int);
 
/* Destructor */
NN_API void neural_network_free(register NeuralNetwork *__restrict);

/* Functions */
NN_API const float* neural_network_predict(const NeuralNetwork *const, const float* __restrict const);

NN_API void neural_network_train(register NeuralNetwork *, const float* __restrict const, const float* __restrict const);

NN_API void neural_network_setLearningRate(register NeuralNetwork *__restrict, const float);
NN_API void neural_network_setActivationFunction(register NeuralNetwork *__restrict, const int);

NN_API json_object* neural_network_serialize(const NeuralNetwork *__restrict const);
NN_API NeuralNetwork* neural_network_deserialize(const json_object *__restrict const);

#undef NN_API

#endif /* __NN_H__ */
