/* Other techniques for learning */

#include <stdlib.h>
#include <math.h> // exp

#include "nn.h"

/* Non member functions */
static inline float sigmoid(float x) {
    return 1 / (1 + exp(-x));
}

static inline float dsigmoid(float y) {
    /* return sigmoid(x) * (1 - sigmoid(x)); */
    return y * (1 - y);
}

// Constructors
NeuralNetwork* neural_network_new_with_nn(const NeuralNetwork *a, const int hidden_nodes, const int output_nodes) {
    NeuralNetwork *nn = malloc(sizeof(NeuralNetwork));

    nn->input_nodes = a->input_nodes;
    nn->hidden_nodes = a->hidden_nodes;
    nn->output_nodes = a->output_nodes;

    nn->weights_ih = a->weights_ih;
    nn->weights_ho = a->weights_ho;

    nn->bias_h = a->bias_h;
    nn->bias_o = a->bias_o;

    // TODO: copy these as well
    nn->learning_rate = 0.1;
    //nn->activation_function = sigmoid;

    return nn;
}

NeuralNetwork* neural_network_new_with_args(const int input_nodes, const int hidden_nodes, const int output_nodes) {
    NeuralNetwork *nn = malloc(sizeof(NeuralNetwork));
    
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

    // TODO: copy these as well
    nn->learning_rate = 0.1;
    //nn->activation_function = sigmoid;
    
    return nn;
}

/* Deconstructor */
void neural_network_free(NeuralNetwork *nn) {
	matrix_free(nn->weights_ih);
	matrix_free(nn->weights_ho);
	matrix_free(nn->bias_h);
	matrix_free(nn->bias_o);
}

/* Functions */
const float* neural_network_predict(const NeuralNetwork *nn, const float *input_array) {
	// Generating the Hidden Outputs
	Matrix *inputs = matrix_fromArray(input_array);
    Matrix *hidden = matrix_multiply_static(nn->weights_ih, inputs);
    matrix_add_matrix(hidden, nn->bias_h);

    // activation function!
    matrix_map(hidden, nn->activation_function);

    // Generating the output's output!
	Matrix *output = matrix_multiply_static(nn->weights_ho, hidden);
	matrix_add_matrix(output, nn->bias_o);
	matrix_map(output, nn->activation_function);

	// Sending back to the caller!
    return matrix_toArray(output);
}

void neural_network_setLearningRate(NeuralNetwork *nn, const float lr) {
	nn->learning_rate = lr;
}

void neural_network_setActivationFunction(NeuralNetwork *nn, float (*func)(float)) {
	nn->activation_function = *func;
}

void neural_network_train(NeuralNetwork *nn, const float *input_array, const float *target_array) {
	// Generating the Hidden Outputs
	Matrix *inputs = matrix_fromArray(input_array);
    Matrix *hidden = matrix_multiply_static(nn->weights_ih, inputs);
    matrix_add_matrix(hidden, nn->bias_h);
    // activation function!
    matrix_map(hidden, nn->activation_function);

    // Generating the output's output!
    Matrix *outputs = matrix_multiply_static(nn->weights_ho, hidden);
    matrix_add_matrix(outputs, nn->bias_o);
    matrix_map(outputs, nn->activation_function);

	// Convert array to matrix object
    Matrix *targets = matrix_fromArray(target_array);

	// Calculate the error
	// ERROR = TARGETS - OUTPUTS
    Matrix *output_errors = matrix_subtract_static(targets, outputs);

	// let gradient = outputs * (1 - outputs);
	// Calculate gradient
    Matrix *gradients = matrix_map_static(outputs, nn->activation_function);
	matrix_multiply(gradients, output_errors);
	matrix_multiply_scalar(gradients, nn->learning_rate);


    // Calculate deltas
    Matrix *hidden_T = matrix_transpose_static(hidden);
	Matrix *weight_ho_deltas = matrix_multiply_static(gradients, hidden_T);

    // Adjust the weights by deltas
    matrix_add_matrix(nn->weights_ho, weight_ho_deltas);
    // Adjust the bias by its deltas (which is just the gradients)
    matrix_add_matrix(nn->bias_o, gradients);

    // Calculate the hidden layer errors
    Matrix *who_t = matrix_transpose_static(nn->weights_ho);
    Matrix *hidden_errors = matrix_multiply_static(who_t, output_errors);

    // Calculate hidden gradient
    Matrix *hidden_gradient = matrix_map_static(hidden, nn->activation_function);
    matrix_add_matrix(hidden_gradient, hidden_errors);
    matrix_add_float(hidden_gradient, nn->learning_rate);

    // Calcuate input->hidden deltas
    Matrix *inputs_T = matrix_transpose_static(inputs);
    Matrix *weight_ih_deltas = matrix_multiply_static(hidden_gradient, inputs_T);

	matrix_add_matrix(nn->weights_ih, weight_ih_deltas);
    // Adjust the bias by its deltas (which is just the gradients)
    matrix_add_matrix(nn->bias_h, hidden_gradient);
}
    
NeuralNetwork* copy(const NeuralNetwork *nn) {
	NeuralNetwork *t = malloc(sizeof(NeuralNetwork));

	matrix_equal(t->weights_ih, nn->weights_ih);
	matrix_equal(t->weights_ho, nn->weights_ho);
	matrix_equal(t->bias_h, nn->bias_h);
	matrix_equal(t->bias_o, nn->bias_o);

	t->learning_rate = nn->learning_rate;

	return t;
}
