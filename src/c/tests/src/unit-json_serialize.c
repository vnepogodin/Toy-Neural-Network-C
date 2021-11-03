#include <unity.h>
#include <vnepogodin/nn.h> /* NeuralNetwork */

json_object* object1 = NULL;
json_object* object2 = NULL;

NeuralNetwork* nn   = NULL;
NeuralNetwork* copy = NULL;

void setUp(void) { }
void tearDown(void) {
    /* clang-format off */
    if (object1) { json_object_put(object1); object1 = NULL; }
    if (object2) { json_object_put(object2); object2 = NULL; }

    if (nn) { neural_network_free(nn); nn = NULL; }
    if (copy) { neural_network_free(copy); copy = NULL; }
    /* clang-format on */
}

void test_serialize_basic_behavior(void) {
    nn                = neural_network_new_with_args(2, 4, 1);
    object1           = neural_network_serialize(nn);
    const char* first = json_object_to_json_string_ext(object1, 0);

    copy               = neural_network_deserialize(object1);
    object2            = neural_network_serialize(copy);
    const char* second = json_object_to_json_string_ext(object2, 0);

    TEST_ASSERT_EQUAL_STRING(first, second);
}

int main(void) {
    UnityBegin("unit-json_serialize.c");
    RUN_TEST(test_serialize_basic_behavior);
    return UNITY_END();
}
