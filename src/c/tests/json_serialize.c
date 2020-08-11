#include "../../../include/c/nn.h"  /* NeuralNetwork */

#include "../../../include/c/third_party/json-c/json_object.h"  /* json_object */

#include <stdio.h>  /* printf */

int main(void) {
    register NeuralNetwork *nn = neural_network_new_with_args(2, 4, 1);

    register json_object *j = neural_network_serialize(nn);

    register const char* data = json_object_to_json_string_ext(j, JSON_C_TO_STRING_PRETTY);
    printf("%s\n", data);

    neural_network_free(nn);
    return 0;
}
