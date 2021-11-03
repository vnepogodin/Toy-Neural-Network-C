#include <unity.h>
#include <vnepogodin/nn.h> /* NeuralNetwork */

NeuralNetwork* nn   = NULL;
NeuralNetwork* temp = NULL;

void setUp(void) { }
void tearDown(void) {
    /* clang-format off */
    if (nn) { neural_network_free(nn); nn = NULL; }
    if (temp) { neural_network_free(temp); nn = NULL; }
    /* clang-format on */
}

void test_copy_basic_behavior(void) {
    nn   = neural_network_new_with_args(2, 4, 1);
    temp = neural_network_new_with_nn(nn);

    TEST_IGNORE();
}

int main(void) {
    UnityBegin("unit-nn_copy.c");
    RUN_TEST(test_copy_basic_behavior);
    return UNITY_END();
}
