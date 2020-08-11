/* Other techniques for learning */

#include "../../include/c/matrix.h"
#include "../../include/c/nn.h"

#include <stdlib.h> /* malloc */
#include <math.h> /* expf */

struct _NeuralNetwork {
    /* Variables */
    int input_nodes, hidden_nodes, output_nodes;

    float learning_rate;

    float (*activation_function)(float);

    Matrix *weights_ih, *weights_ho, *bias_h, *bias_o;
};


/* Non member functions */
static inline float sigmoid(float x) {
    return 1.F / (1.F + expf(-x));
}

static inline float dsigmoid(float y) {
    /* return sigmoid(x) * (1 - sigmoid(x)); */
    return y * (1.F - y);
}

static json_object* json_find(const json_object *__restrict const j,
                              const char* __restrict key) {
    json_object *temp_json;

    json_object_object_get_ex(j, key, &temp_json);

    return temp_json;
}

#define convert_ActivationFunction(func) \
    (*(func) == dsigmoid) ? 2 : 1


/**
 * neural_network_new_with_nn:
 * @a: a reference #NeuralNetwork.
 *
 * Creates a new #NeuralNetwork with data of @a.
 *
 * Returns: the new #NeuralNetwork
 */
NeuralNetwork* neural_network_new_with_nn(const NeuralNetwork *const __nn_param) {
    register NeuralNetwork *__nn_temp = (NeuralNetwork *)malloc(56UL);

    __nn_temp->input_nodes = __nn_param->input_nodes;
    __nn_temp->hidden_nodes = __nn_param->hidden_nodes;
    __nn_temp->output_nodes = __nn_param->output_nodes;

    __nn_temp->weights_ih = matrix_new_with_matrix(__nn_param->weights_ih);
    __nn_temp->weights_ho = matrix_new_with_matrix(__nn_param->weights_ho);
    __nn_temp->bias_h = matrix_new_with_matrix(__nn_param->bias_h);
    __nn_temp->bias_o = matrix_new_with_matrix(__nn_param->bias_o);

    /* TODO: copy these as well */
    neural_network_setLearningRate(__nn_temp, __nn_param->learning_rate);

    register int flag = convert_ActivationFunction(__nn_param->activation_function);
    neural_network_setActivationFunction(__nn_temp, flag);

    return __nn_temp;
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
NeuralNetwork* neural_network_new_with_args(const int input_nodes,
                                            const int hidden_nodes,
                                            const int output_nodes) {
    register NeuralNetwork *nn = (NeuralNetwork *)malloc(56UL);

    nn->input_nodes = input_nodes;
    nn->hidden_nodes = hidden_nodes;
    nn->output_nodes = output_nodes;

    nn->weights_ih = matrix_new_with_args(hidden_nodes, input_nodes);
    nn->weights_ho = matrix_new_with_args(output_nodes, hidden_nodes);
    matrix_randomize(nn->weights_ih);
    matrix_randomize(nn->weights_ho);

    nn->bias_h = matrix_new_with_args(hidden_nodes, 1);
    nn->bias_o = matrix_new_with_args(output_nodes, 1);
    matrix_randomize(nn->bias_h);
    matrix_randomize(nn->bias_o);

    /* TODO: copy these as well */
    neural_network_setLearningRate(nn, 0.1F);
    neural_network_setActivationFunction(nn, FUNC_SIGMOID);

    return nn;
}

/**
 * neural_network_free:
 * @nn: a #NeuralNetwork.
 *
 * Frees #NeuralNetwork.
 *
 */
void neural_network_free(register NeuralNetwork *__restrict __nn_param) {
    __nn_param->activation_function = NULL;

    matrix_free(__nn_param->weights_ih);
    matrix_free(__nn_param->weights_ho);
    matrix_free(__nn_param->bias_h);
    matrix_free(__nn_param->bias_o);

    free(__nn_param);
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
float* neural_network_predict(const NeuralNetwork *const nn,
                              const float* __restrict const input_array) {
    /* Generating the Hidden Outputs */
    register Matrix *input = matrix_fromArray(input_array, nn->input_nodes);
    register Matrix *hidden = matrix_multiply_static(nn->weights_ih, input);
    matrix_free(input);
    matrix_add_matrix(hidden, nn->bias_h);

    /* Activation function! */
    matrix_map(hidden, nn->activation_function);

    /* Generating the output's output! */
    register Matrix *output = matrix_multiply_static(nn->weights_ho, hidden);
    matrix_free(hidden);
    matrix_add_matrix(output, nn->bias_o);
    matrix_map(output, nn->activation_function);

    /* Sending back to the caller! */
    float* result = matrix_toArray(output);
    matrix_free(output);

    return result;
}

/**
 * neural_network_setLearningRate:
 * @nn: a #NeuralNetwork.
 * @lr: a learning rate of neural network.
 *
 * Setting learning rate.
 *
 */
void neural_network_setLearningRate(register NeuralNetwork *__restrict nn,
                                    const float lr) {
    nn->learning_rate = lr;
}

/**
 * neural_network_setActivationFunction:
 * @nn: a #NeuralNetwork.
 * @flag: a some function.
 *
 * Setting function.
 *
 */
void neural_network_setActivationFunction(register NeuralNetwork *__restrict nn,
                                          const unsigned char flag) {
    nn->activation_function = NULL;

    if (flag & FUNC_SIGMOID)
        nn->activation_function = sigmoid;
    else if (flag & FUNC_DSIGMOID)
        nn->activation_function = dsigmoid;
}

/**
 * neural_network_train:
 * @nn: a #NeuralNetwork.
 * @input_array: a input data.
 * @target_array: a output data.
 *
 * Trains a neural network.
 *
 */
void neural_network_train(register NeuralNetwork *nn,
                          const float* __restrict const input_array,
                          const float* __restrict const target_array) {
    /* Generating the Hidden Outputs */
    register Matrix *inputs = matrix_fromArray(input_array, nn->input_nodes);
    register Matrix *hidden = matrix_multiply_static(nn->weights_ih, inputs);
    matrix_free(inputs);
    matrix_add_matrix(hidden, nn->bias_h);
    /*  Activation function!  */
    matrix_map(hidden, sigmoid);

    /* Generating the output's output! */
    register Matrix *outputs = matrix_multiply_static(nn->weights_ho, hidden);
    matrix_add_matrix(outputs, nn->bias_o);
    matrix_map(outputs, sigmoid);

    /* Convert array to matrix object */
    register Matrix *targets = matrix_fromArray(target_array, nn->output_nodes);

    /* Calculate the error
     * ERROR = TARGETS - OUTPUTS */
    register Matrix *output_errors = matrix_subtract_static(targets, outputs);
    matrix_free(targets);

    /* let gradient = outputs * (1 - outputs);
     * Calculate gradient */
    register Matrix *gradients = matrix_map_static(outputs, dsigmoid);
    matrix_free(outputs);
    matrix_multiply(gradients, output_errors);
    matrix_multiply_scalar(gradients, nn->learning_rate);


    /* Calculate deltas */
    register Matrix *hidden_T = matrix_transpose_static(hidden);
    register Matrix *weight_ho_deltas = matrix_multiply_static(gradients, hidden_T);
    matrix_free(hidden_T);

    /* Adjust the weights by deltas */
    matrix_add_matrix(nn->weights_ho, weight_ho_deltas);
    matrix_free(weight_ho_deltas);

    /* Adjust the bias by its deltas (which is just the gradients) */
    matrix_add_matrix(nn->bias_o, gradients);
    matrix_free(gradients);

    /* Calculate the hidden layer errors */
    register Matrix *who_t = matrix_transpose_static(nn->weights_ho);
    register Matrix *hidden_errors = matrix_multiply_static(who_t, output_errors);
    matrix_free(output_errors);
    matrix_free(who_t);

    /* Calculate hidden gradient */
    register Matrix *hidden_gradient = matrix_map_static(hidden, dsigmoid);
    matrix_free(hidden);
    matrix_multiply(hidden_gradient, hidden_errors);
    matrix_free(hidden_errors);
    matrix_multiply_scalar(hidden_gradient, nn->learning_rate);

    /* Calcuate input->hidden deltas */
    inputs = matrix_fromArray(input_array, nn->input_nodes);
    register Matrix *inputs_T = matrix_transpose_static(inputs);
    register Matrix *weight_ih_deltas = matrix_multiply_static(hidden_gradient, inputs_T);
    matrix_free(inputs);
    matrix_free(inputs_T);

    matrix_add_matrix(nn->weights_ih, weight_ih_deltas);
    matrix_free(weight_ih_deltas);
    /* Adjust the bias by its deltas (which is just the gradients) */
    matrix_add_matrix(nn->bias_h, hidden_gradient);
    matrix_free(hidden_gradient);
}


/**
 * neural_network_serialize:
 * @nn: a reference #NeuralNetwork.
 *
 * Serialize #Matrix to JSON.
 *
 * Returns: the new #json_object
 */
json_object* neural_network_serialize(const NeuralNetwork *__restrict const __nn_param) {
    register json_object *t = json_object_new_object();

    json_object_object_add_ex(t, "input_nodes", json_object_new_int(__nn_param->input_nodes), 0U);
    json_object_object_add_ex(t, "hidden_nodes", json_object_new_int(__nn_param->hidden_nodes), 0U);
    json_object_object_add_ex(t, "output_nodes", json_object_new_int(__nn_param->output_nodes), 0U);


    json_object_object_add_ex(t, "weights_ih", matrix_serialize(__nn_param->weights_ih), 0U);
    json_object_object_add_ex(t, "weights_ho", matrix_serialize(__nn_param->weights_ho), 0U);
    json_object_object_add_ex(t, "bias_h", matrix_serialize(__nn_param->bias_h), 0U);
    json_object_object_add_ex(t, "bias_o", matrix_serialize(__nn_param->bias_o), 0U);


    json_object_object_add_ex(t, "learning_rate", json_object_new_double((double)__nn_param->learning_rate), 0U);

    register int flag = convert_ActivationFunction(__nn_param->activation_function);
    json_object_object_add_ex(t, "activation_function", json_object_new_int(flag), 0U);

    return t;
}

/**
 * neural_network_deserialize:
 * @t: The reference #json_object.
 *
 * Deserialize JSON.
 *
 * Returns: the new #NeuralNetwork
 */
NeuralNetwork* neural_network_deserialize(const json_object *__restrict const __json_param) {
    register NeuralNetwork *nn = (NeuralNetwork *)malloc(57);

    nn->input_nodes = json_object_get_int(json_find(__json_param, "input_nodes"));
    nn->hidden_nodes = json_object_get_int(json_find(__json_param, "hidden_nodes"));
    nn->output_nodes = json_object_get_int(json_find(__json_param, "output_nodes"));

    nn->weights_ih = matrix_deserialize(json_find(__json_param, "weights_ih"));
    nn->weights_ho = matrix_deserialize(json_find(__json_param, "weights_ho"));
    nn->bias_h = matrix_deserialize(json_find(__json_param, "bias_h"));
    nn->bias_o = matrix_deserialize(json_find(__json_param, "bias_o"));

#ifdef DEBUG
    matrix_print(nn->weights_ih);
#endif

    neural_network_setLearningRate(nn, json_object_get_float(json_find(__json_param, "learning_rate")));
    neural_network_setActivationFunction(nn, json_object_get_int(json_find(__json_param, "activation_function")));

    return nn;
}
