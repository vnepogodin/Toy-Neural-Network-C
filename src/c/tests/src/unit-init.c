#include <vnepogodin/nn.h> /* NeuralNetwork */
#include <unity.h>

NeuralNetwork* nn = NULL;

void setUp(void) { }
void tearDown(void) {
    /* clang-format off */
    if (nn) { neural_network_free(nn); nn = NULL; }
    /* clang-format on */
}

void test_construction_basic_behavior(void) {
    nn = neural_network_new_with_args(2, 4, 1);
    TEST_ASSERT(nn != NULL);
}

int main(void) {
    UnityBegin("unit-init.c");
    RUN_TEST(test_construction_basic_behavior);
    return UNITY_END();
}
