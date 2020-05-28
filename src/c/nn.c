/* Other techniques for learning */

#include <stdlib.h> /* malloc */
#include <math.h> /* exp */

#include "nn.h"

/* Non member functions */
static float sigmoid(float x) {
    return 1.f / (1.f + exp(-x));
}

static float dsigmoid(float y) {
    /* return sigmoid(x) * (1 - sigmoid(x)); */
    return y * (1.f - y);
}

static json_object* json_find(const json_object *__restrict j, const char* __restrict key) {
	json_object *t;

    json_object_object_get_ex(j, key, &t);

    return t;
}

/**
 * neural_network_new_with_nn:
 * @a: a reference #NeuralNetwork.
 *
 * Creates a new #NeuralNetwork with data of @a.
 *
 * Returns: the new #NeuralNetwork
 */
NeuralNetwork* neural_network_new_with_nn(const NeuralNetwork *a) {
    register NeuralNetwork *nn = (NeuralNetwork *)malloc(sizeof(NeuralNetwork));

    nn->input_nodes = a->input_nodes;
    nn->hidden_nodes = a->hidden_nodes;
    nn->output_nodes = a->output_nodes;

    matrix_equal(nn->weights_ih, a->weights_ih);
    matrix_equal(nn->weights_ho, a->weights_ho);

	matrix_equal(nn->bias_h, a->bias_h);
    matrix_equal(nn->bias_o, a->bias_o);

    /* TODO: copy these as well */
    neural_network_setLearningRate(nn, 0.1);
	neural_network_setActivationFunction(nn, sigmoid);

    return nn;
}

/**
 * neural_network_new_with_args:
 * @input_nodes: a const num input.
 * @hidden_nodes: a const num hidden.
 * @output_nodes: a const num output.
 *
 * Creates a new #NeuralNetwork with random data of #Matrix.
 *
 * Returns: the new #NeuralNetwork
 */
NeuralNetwork* neural_network_new_with_args(const int input_nodes, const int hidden_nodes, const int output_nodes) {
    register NeuralNetwork *nn = (NeuralNetwork *)malloc(sizeof(NeuralNetwork));
    
    nn->input_nodes = input_nodes;
    nn->hidden_nodes = hidden_nodes;
    nn->output_nodes = output_nodes;

    nn->weights_ih = matrix_new_with_args(nn->hidden_nodes, nn->input_nodes);
    nn->weights_ho = matrix_new_with_args(nn->output_nodes, nn->hidden_nodes);
    matrix_randomize(nn->weights_ih);
    matrix_randomize(nn->weights_ho);

    nn->bias_h = matrix_new_with_args(nn->hidden_nodes, 1);
    nn->bias_o = matrix_new_with_args(nn->output_nodes, 1);
    matrix_randomize(nn->bias_h);
    matrix_randomize(nn->bias_o);

    /* TODO: copy these as well */
    neural_network_setLearningRate(nn, 0.1);
	neural_network_setActivationFunction(nn, sigmoid);
    
    return nn;
}

/**
 * neural_network_free:
 * @nn: a #NeuralNetwork.
 *
 * Frees #NeuralNetwork.
 */
void neural_network_free(register NeuralNetwork *__restrict nn) {
	nn->activation_function = NULL;
	
	matrix_free(nn->weights_ih);
	matrix_free(nn->weights_ho);
	matrix_free(nn->bias_h);
	matrix_free(nn->bias_o);
}

/**
 * neural_network_predict:
 * @nn: a reference #NeuralNetwork.
 * @input_array: a input data.
 *
 * Output of #NeuralNetwork.
 *
 * Returns: float array
 */
const float* neural_network_predict(const NeuralNetwork *nn, const float* __restrict input_array) {
	/* Generating the Hidden Outputs */
	register Matrix *inputs = matrix_fromArray(input_array);
    register Matrix *hidden = matrix_multiply_static(nn->weights_ih, inputs);
    matrix_add_matrix(hidden, nn->bias_h);

    /* Activation function! */
    matrix_map(hidden, nn->activation_function);

    /* Generating the output'Ms output! */
	register Matrix *output = matrix_multiply_static(nn->weights_ho, hidden);
	matrix_add_matrix(output, nn->bias_o);
	matrix_map(output, nn->activation_function);

	/* Sending back to the caller! */
    return matrix_toArray(output);
}

/**
 * neural_network_setLearningRate:
 * @nn: a #NeuralNetwork.
 * @lr: a learning rate of neural network.
 *
 * Setting learning rate.
 */
void neural_network_setLearningRate(register NeuralNetwork *__restrict nn, const float lr) {
	nn->learning_rate = lr;
}

/**
 * neural_network_setActivationFunction:
 * @nn: a #NeuralNetwork.
 * @func: a aome function.
 *
 * Setting function.
 */
void neural_network_setActivationFunction(register NeuralNetwork *__restrict nn, float (*func)(float)) {
	nn->activation_function = *func;
}

/**
 * neural_network_train:
 * @nn: a #NeuralNetwork.
 * @input_array: a input data.
 * @target_array: a output data.
 *
 * Trains a neural network.
 */
void neural_network_train(register NeuralNetwork *nn, const float* __restrict input_array, const float* __restrict target_array) {
	/* Generating the Hidden Outputs */
	register Matrix *inputs = matrix_fromArray(input_array);
    register Matrix *hidden = matrix_multiply_static(nn->weights_ih, inputs);
    matrix_add_matrix(hidden, nn->bias_h);
    /* Activation function! */
    matrix_map(hidden, nn->activation_function);

    /* Generating the output's output! */
    register Matrix *outputs = matrix_multiply_static(nn->weights_ho, hidden);
    matrix_add_matrix(outputs, nn->bias_o);
    matrix_map(outputs, nn->activation_function);

	/* Convert array to matrix object */
    register Matrix *targets = matrix_fromArray(target_array);

	/* Calculate the error
	 * ERROR = TARGETS - OUTPUTS */
    register Matrix *output_errors = matrix_subtract_static(targets, outputs);

	/* let gradient = outputs * (1 - outputs);
	 * Calculate gradient */
    register Matrix *gradients = matrix_map_static(outputs, nn->activation_function);
	matrix_multiply(gradients, output_errors);
	matrix_multiply_scalar(gradients, nn->learning_rate);


    /* Calculate deltas */
    register Matrix *hidden_T = matrix_transpose_static(hidden);
	register Matrix *weight_ho_deltas = matrix_multiply_static(gradients, hidden_T);

    /* Adjust the weights by deltas */
    matrix_add_matrix(nn->weights_ho, weight_ho_deltas);
    /* Adjust the bias by its deltas (which is just the gradients) */
    matrix_add_matrix(nn->bias_o, gradients);

    /* Calculate the hidden layer errors */
    register Matrix *who_t = matrix_transpose_static(nn->weights_ho);
    register Matrix *hidden_errors = matrix_multiply_static(who_t, output_errors);

    /* Calculate hidden gradient */
    register Matrix *hidden_gradient = matrix_map_static(hidden, nn->activation_function);
    matrix_add_matrix(hidden_gradient, hidden_errors);
    matrix_add_float(hidden_gradient, nn->learning_rate);

    /* Calcuate input->hidden deltas */
    register Matrix *inputs_T = matrix_transpose_static(inputs);
    register Matrix *weight_ih_deltas = matrix_multiply_static(hidden_gradient, inputs_T);

	matrix_add_matrix(nn->weights_ih, weight_ih_deltas);
    /* Adjust the bias by its deltas (which is just the gradients) */
    matrix_add_matrix(nn->bias_h, hidden_gradient);
}

/**
 * neural_network_serialize:
 * @nn: a reference #NeuralNetwork.
 *
 * Serialize #Matrix to JSON.
 *
 * Returns: the new #json_object
 */
const json_object* neural_network_serialize(const NeuralNetwork *__restrict nn) {
	register json_object *t = json_object_new_object();

	json_object_object_add(t, "input_nodes", json_object_new_int(nn->input_nodes));
	json_object_object_add(t, "hidden_nodes", json_object_new_int(nn->hidden_nodes));
	json_object_object_add(t, "output_nodes", json_object_new_int(nn->output_nodes));

	json_object_object_add(t, "weights_ih", matrix_serialize(nn->weights_ih));
	json_object_object_add(t, "weights_ho", matrix_serialize(nn->weights_ho));

	json_object_object_add(t, "bias_h", matrix_serialize(nn->bias_h));
	json_object_object_add(t, "bias_o", matrix_serialize(nn->bias_o));

	json_object_object_add(t, "learning_rate", json_object_new_double(nn->learning_rate));

	return t;
}

/**
 * neural_network_copy:
 * @nn: a reference #NeuralNetwork.
 *
 * Copy @nn.
 *
 * Returns: the new #NeuralNetwork
 */
NeuralNetwork* neural_network_copy(const NeuralNetwork *__restrict nn) {
	register NeuralNetwork *t = (NeuralNetwork *)malloc(sizeof(NeuralNetwork));

	matrix_equal(t->weights_ih, nn->weights_ih);
	matrix_equal(t->weights_ho, nn->weights_ho);
	matrix_equal(t->bias_h, nn->bias_h);
	matrix_equal(t->bias_o, nn->bias_o);

	neural_network_setLearningRate(t, nn->learning_rate);
	neural_network_setActivationFunction(t, nn->activation_function);
	
	return t;
}

/**
 * neural_network_deserialize:
 * @t: a reference #json_object.
 *
 * Deserialize JSON.
 *
 * Returns: the new #NeuralNetwork
 */
NeuralNetwork* neural_network_deserialize(const json_object *__restrict t) {
	register NeuralNetwork *nn = neural_network_new_with_args(json_object_get_int(json_object_object_get(t, "input_nodes")), json_object_get_int(json_object_object_get(t, "hidden_nodes")), json_object_get_int(json_object_object_get(t, "output_nodes")));

	nn->weights_ih = matrix_deserialize(json_find(t, "weights_ih"));
	nn->weights_ho = matrix_deserialize(json_find(t, "weights_ho"));
	nn->bias_h = matrix_deserialize(json_find(t, "bias_h"));
	nn->bias_o = matrix_deserialize(json_find(t, "bias_o"));
	
	neural_network_setLearningRate(nn, (float)json_object_get_double(json_object_object_get(t, "learning_rate")));
	
	return nn;
}
