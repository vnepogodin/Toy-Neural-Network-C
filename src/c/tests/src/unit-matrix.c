#include <vnepogodin/nn.h> /* NeuralNetwork */
#include <stdlib.h> /* free */
#include <string.h> /* memcpy */
#include <unity.h>

Matrix* m    = NULL;
Matrix* n    = NULL;
Matrix* test = NULL;

json_object* object1 = NULL;
json_object* object2 = NULL;

char* sbuffer = NULL;

float test_map(const float elem) {
    return elem * 10;
}

float test_map2(const float elem) {
    return elem - 1;
}

void setUp(void) { }
void tearDown(void) {
    /* clang-format off */
    if (m) { matrix_free(m); m = NULL; }
    if (n) { matrix_free(n); n = NULL; }
    if (test) { matrix_free(test); test = NULL; }

    if (sbuffer) { free(sbuffer); sbuffer = NULL; }
    if (object1) { json_object_put(object1); object1 = NULL; }
    if (object2) { json_object_put(object2); object2 = NULL; }
    /* clang-format on */
}

void test_add_scalar_to_matrix(void) {
    const float m_arr[] = {
        1.0, 2.0, 3.0,
        4.0, 5.0, 6.0,
        7.0, 8.0, 9.0};
    const float test_arr[] = {
        2.0, 3.0, 4.0,
        5.0, 6.0, 7.0,
        8.0, 9.0, 10.0};

    const int size = 9;
    m              = matrix_new_with_args(3, 3);
    test           = matrix_new_with_args(3, 3);
    memcpy(matrix_data(m), m_arr, size * sizeof(float));
    memcpy(matrix_data(test), test_arr, size * sizeof(float));
    matrix_add_float(m, 1);

    TEST_ASSERT(matrix_equal(m, test));
}

void test_add_matrix_to_matrix(void) {
    const float m_arr[] = {
        1.0, 2.0,
        3.0, 4.0};
    const float n_arr[] = {
        10.0, 11.0,
        12.0, 13.0};

    const float test_arr[] = {
        11.0, 13.0,
        15.0, 17.0};

    const int size = 4;
    m              = matrix_new_with_args(2, 2);
    n              = matrix_new_with_args(2, 2);
    test           = matrix_new_with_args(2, 2);
    memcpy(matrix_data(m), m_arr, size * sizeof(float));
    memcpy(matrix_data(n), n_arr, size * sizeof(float));
    memcpy(matrix_data(test), test_arr, size * sizeof(float));
    matrix_add_matrix(m, n);

    TEST_ASSERT(matrix_equal(m, test));
}

void test_sub_matrix_from_matrix(void) {
    const float m_arr[] = {
        10.0, 11.0,
        12.0, 13.0};
    const float n_arr[] = {
        1.0, 2.0,
        3.0, 4.0};

    const float test_arr[] = {
        9.0, 9.0,
        9.0, 9.0};

    const int size = 4;
    m              = matrix_new_with_args(2, 2);
    n              = matrix_new_with_args(2, 2);
    test           = matrix_new_with_args(2, 2);
    memcpy(matrix_data(m), m_arr, size * sizeof(float));
    memcpy(matrix_data(n), n_arr, size * sizeof(float));
    memcpy(matrix_data(test), test_arr, size * sizeof(float));
    Matrix* mMinusN = matrix_subtract_static(m, n);
    matrix_free(m);
    m = mMinusN;

    TEST_ASSERT(matrix_equal(m, test));
}

void test_matrix_product(void) {
    const float m_arr[] = {
        1.0, 2.0, 3.0,
        4.0, 5.0, 6.0};
    const float n_arr[] = {
        7.0, 8.0,
        9.0, 10.0,
        11.0, 12.0};

    const float test_arr[] = {
        58.0, 64.0,
        139.0, 154.0};

    const int size = 6;
    m              = matrix_new_with_args(2, 3);
    n              = matrix_new_with_args(3, 2);
    test           = matrix_new_with_args(2, 2);
    memcpy(matrix_data(m), m_arr, size * sizeof(float));
    memcpy(matrix_data(n), n_arr, size * sizeof(float));
    memcpy(matrix_data(test), test_arr, 4 * sizeof(float));
    Matrix* mn = matrix_multiply_static(m, n);
    matrix_free(m);
    m = mn;

    TEST_ASSERT(matrix_equal(m, test));
}

void test_hadamard_product(void) {
    const float m_arr[] = {
        1.0, 2.0,
        3.0, 4.0,
        5.0, 6.0};
    const float n_arr[] = {
        7.0, 8.0,
        9.0, 10.0,
        11.0, 12.0};

    const float test_arr[] = {
        7.0, 16.0,
        27.0, 40.0,
        55.0, 72.0};

    const int size = 6;
    m              = matrix_new_with_args(3, 2);
    n              = matrix_new_with_args(3, 2);
    test           = matrix_new_with_args(3, 2);
    memcpy(matrix_data(m), m_arr, size * sizeof(float));
    memcpy(matrix_data(n), n_arr, size * sizeof(float));
    memcpy(matrix_data(test), test_arr, size * sizeof(float));
    matrix_multiply(m, n);

    TEST_ASSERT(matrix_equal(m, test));
}

void test_scalar_product(void) {
    const float m_arr[] = {
        1.0, 2.0,
        3.0, 4.0,
        5.0, 6.0};

    const float test_arr[] = {
        7.0, 14.0,
        21.0, 28.0,
        35.0, 42.0};

    const int size = 6;
    m              = matrix_new_with_args(3, 2);
    test           = matrix_new_with_args(3, 2);
    memcpy(matrix_data(m), m_arr, size * sizeof(float));
    memcpy(matrix_data(test), test_arr, size * sizeof(float));
    matrix_multiply_scalar(m, 7);

    TEST_ASSERT(matrix_equal(m, test));
}

void test_matrix_transpose_11(void) {
    const float m_arr[]    = {1.0};
    const float test_arr[] = {1.0};

    const int size = 1;
    m              = matrix_new_with_args(1, 1);
    test           = matrix_new_with_args(1, 1);
    memcpy(matrix_data(m), m_arr, size * sizeof(float));
    memcpy(matrix_data(test), test_arr, size * sizeof(float));
    Matrix* mt = matrix_transpose_static(m);
    matrix_free(m);
    m = mt;

    TEST_ASSERT(matrix_equal(m, test));
}

void test_matrix_transpose_23_to_32(void) {
    const float m_arr[] = {
        1.0, 2.0, 3.0,
        4.0, 5.0, 6.0};
    const float test_arr[] = {
        1.0, 4.0,
        2.0, 5.0,
        3.0, 6.0};

    const int size = 6;
    m              = matrix_new_with_args(2, 3);
    test           = matrix_new_with_args(3, 2);
    memcpy(matrix_data(m), m_arr, size * sizeof(float));
    memcpy(matrix_data(test), test_arr, size * sizeof(float));
    Matrix* mt = matrix_transpose_static(m);
    matrix_free(m);
    m = mt;

    TEST_ASSERT(matrix_equal(m, test));
}

void test_matrix_transpose_32_to_23(void) {
    const float m_arr[] = {
        1.0, 2.0,
        3.0, 4.0,
        5.0, 6.0};
    const float test_arr[] = {
        1.0, 3.0, 5.0,
        2.0, 4.0, 6.0};

    const int size = 6;
    m              = matrix_new_with_args(3, 2);
    test           = matrix_new_with_args(2, 3);
    memcpy(matrix_data(m), m_arr, size * sizeof(float));
    memcpy(matrix_data(test), test_arr, size * sizeof(float));
    Matrix* mt = matrix_transpose_static(m);
    matrix_free(m);
    m = mt;

    TEST_ASSERT(matrix_equal(m, test));
}

void test_matrix_transpose_15_to_51(void) {
    const float m_arr[]    = {1.0, 2.0, 3.0, 4.0, 5.0};
    const float test_arr[] = {
        1.0,
        2.0,
        3.0,
        4.0,
        5.0};

    const int size = 5;
    m              = matrix_new_with_args(1, 5);
    test           = matrix_new_with_args(5, 1);
    memcpy(matrix_data(m), m_arr, size * sizeof(float));
    memcpy(matrix_data(test), test_arr, size * sizeof(float));
    Matrix* mt = matrix_transpose_static(m);
    matrix_free(m);
    m = mt;

    TEST_ASSERT(matrix_equal(m, test));
}

void test_matrix_transpose_51_to_15(void) {
    const float m_arr[] = {
        1.0,
        2.0,
        3.0,
        4.0,
        5.0};
    const float test_arr[] = {1.0, 2.0, 3.0, 4.0, 5.0};

    const int size = 5;
    m              = matrix_new_with_args(1, 5);
    test           = matrix_new_with_args(5, 1);
    memcpy(matrix_data(m), m_arr, size * sizeof(float));
    memcpy(matrix_data(test), test_arr, size * sizeof(float));
    Matrix* mt = matrix_transpose_static(m);
    matrix_free(m);
    m = mt;

    TEST_ASSERT(matrix_equal(m, test));
}

void test_mapping_with_static_map(void) {
    const float m_arr[] = {
        1.0, 2.0, 3.0,
        4.0, 5.0, 6.0,
        7.0, 8.0, 9.0};
    const float test_arr[] = {
        10.0, 20.0, 30.0,
        40.0, 50.0, 60.0,
        70.0, 80.0, 90.0};

    const int size = 9;
    m              = matrix_new_with_args(3, 3);
    test           = matrix_new_with_args(3, 3);
    memcpy(matrix_data(m), m_arr, size * sizeof(float));
    memcpy(matrix_data(test), test_arr, size * sizeof(float));
    Matrix* mt = matrix_map_static(m, test_map);
    matrix_free(m);
    m = mt;

    TEST_ASSERT(matrix_equal(m, test));
}

void test_mapping_with_instance_map(void) {
    const float m_arr[] = {
        1.0, 2.0, 3.0,
        4.0, 5.0, 6.0,
        7.0, 8.0, 9.0};
    const float test_arr[] = {
        10.0, 20.0, 30.0,
        40.0, 50.0, 60.0,
        70.0, 80.0, 90.0};

    const int size = 9;
    m              = matrix_new_with_args(3, 3);
    test           = matrix_new_with_args(3, 3);
    memcpy(matrix_data(m), m_arr, size * sizeof(float));
    memcpy(matrix_data(test), test_arr, size * sizeof(float));
    matrix_map(m, test_map);

    TEST_ASSERT(matrix_equal(m, test));
}

#ifndef _WIN32

void error_handling_of_addition_when_colr_ofA_dont_match_ofB(void) {
    fflush(stderr);
    /* save stderr */
    FILE* sbuf = stderr;

    /* Redirect stderr to our string buffer */
    size_t size;
    stderr = open_memstream(&sbuffer, &size);

    m    = matrix_new_with_args(1, 2);
    test = matrix_new_with_args(3, 4);
    matrix_add_matrix(m, test);

    /* Cleanup */
    fflush(stderr);
    fclose(stderr);

    /* When done redirect stderr to its old self */
    stderr = sbuf;
    setbuf(stderr, NULL);

    static const char* expected = "Columns and Rows of A must match Columns and Rows of B.\n";
    TEST_ASSERT_EQUAL_STRING(expected, sbuffer);
}

void error_handling_of_static_sub_when_colr_ofA_dont_match_ofB(void) {
    fflush(stderr);
    /* save stderr */
    FILE* sbuf = stderr;

    /* Redirect stderr to our string buffer */
    size_t size;
    stderr = open_memstream(&sbuffer, &size);

    m    = matrix_new_with_args(1, 2);
    test = matrix_new_with_args(3, 4);
    matrix_subtract_static(m, test);

    /* Cleanup */
    fflush(stderr);
    fclose(stderr);

    /* When done redirect stderr to its old self */
    stderr = sbuf;
    setbuf(stderr, NULL);

    static const char* expected = "Columns and Rows of A must match Columns and Rows of B.\n";
    TEST_ASSERT_EQUAL_STRING(expected, sbuffer);
}

void error_handling_of_hadamard_product_when_colr_ofA_dont_match_ofB(void) {
    fflush(stderr);
    /* save stderr */
    FILE* sbuf = stderr;

    /* Redirect stderr to our string buffer */
    size_t size;
    stderr = open_memstream(&sbuffer, &size);

    m    = matrix_new_with_args(1, 2);
    test = matrix_new_with_args(3, 4);
    matrix_multiply(m, test);

    /* Cleanup */
    fflush(stderr);
    fclose(stderr);

    /* When done redirect stderr to its old self */
    stderr = sbuf;
    setbuf(stderr, NULL);

    static const char* expected = "Columns and Rows of A must match Columns and Rows of B.\n";
    TEST_ASSERT_EQUAL_STRING(expected, sbuffer);
}

void error_handling_of_matrix_product_when_col_ofA_dont_match_r_ofB(void) {
    fflush(stderr);
    /* save stderr */
    FILE* sbuf = stderr;

    /* Redirect stderr to our string buffer */
    size_t size;
    stderr = open_memstream(&sbuffer, &size);

    m    = matrix_new_with_args(1, 2);
    test = matrix_new_with_args(3, 4);
    matrix_multiply_static(m, test);

    /* Cleanup */
    fflush(stderr);
    fclose(stderr);

    /* When done redirect stderr to its old self */
    stderr = sbuf;
    setbuf(stderr, NULL);

    static const char* expected = "Columns of A must match rows of B.\n";
    TEST_ASSERT_EQUAL_STRING(expected, sbuffer);
}

void test_chanining_matrix_methods(void) {
    const float m_arr[] = {
        1.0, 2.0, 3.0,
        4.0, 5.0, 6.0,
        7.0, 8.0, 9.0};
    const float test_arr[] = {
        6.0, 16.0, 26.0,
        36.0, 46.0, 56.0,
        66.0, 76.0, 86.0};

    const int size = 9;
    m              = matrix_new_with_args(3, 3);
    test           = matrix_new_with_args(3, 3);
    memcpy(matrix_data(m), m_arr, size * sizeof(float));
    memcpy(matrix_data(test), test_arr, size * sizeof(float));

    matrix_map(m, test_map2);
    matrix_multiply_scalar(m, 10);
    matrix_add_float(m, 6);

    fflush(stdout);
    /* save stdout */
    FILE* sbuf = stdout;

    /* Redirect stdout to our string buffer */
    size_t buf_size;
    stdout = open_memstream(&sbuffer, &buf_size);

    matrix_print(m);

    /* Cleanup */
    fflush(stdout);
    fclose(stdout);

    /* When done redirect stdout to its old self */
    stdout = sbuf;

    TEST_ASSERT(matrix_equal(m, test));
}

#endif

void test_matrix_from_array(void) {
    const float array[]    = {1, 2, 3};
    const float test_arr[] = {
        1.0,
        2.0,
        3.0};

    const int size = 3;
    m              = matrix_fromArray(array, size);
    test           = matrix_new_with_args(3, 1);
    memcpy(matrix_data(test), test_arr, size * sizeof(float));

    TEST_ASSERT(matrix_equal(m, test));
}

void test_matrix_to_array(void) {
    const float m_arr[] = {
        1.0, 2.0, 3.0,
        4.0, 5.0, 6.0,
        7.0, 8.0, 9.0};

    const int size = 9;
    m              = matrix_new_with_args(3, 3);
    memcpy(matrix_data(m), m_arr, size * sizeof(float));
    float* test_arr = matrix_toArray(m);
    int is_equal    = 1;
    float* ptr      = matrix_data(m);
    for (int i = 0; i < size; ++i) {
        if (ptr[i] != test_arr[i]) {
            is_equal = 0;
            break;
        }
    }
    free(test_arr);
    TEST_ASSERT(is_equal);
}

void test_matrix_de_serialization(void) {
    m = matrix_new_with_args(5, 5);
    matrix_randomize(m);

    object1           = matrix_serialize(m);
    const char* first = json_object_to_json_string_ext(object1, 0);

    n                  = matrix_deserialize(object1);
    object2            = matrix_serialize(n);
    const char* second = json_object_to_json_string_ext(object2, 0);

    TEST_ASSERT_EQUAL_STRING(first, second);
}

void test_matrix_copy(void) {
    m = matrix_new_with_args(5, 5);
    matrix_randomize(m);
    n = matrix_new_with_matrix(m);

    TEST_ASSERT(matrix_equal(m, n));
}

int main(void) {
    UnityBegin("unit-matrix.c");
    RUN_TEST(test_add_scalar_to_matrix);
    RUN_TEST(test_add_matrix_to_matrix);
    RUN_TEST(test_sub_matrix_from_matrix);
    RUN_TEST(test_matrix_product);
    RUN_TEST(test_hadamard_product);
    RUN_TEST(test_scalar_product);
    RUN_TEST(test_matrix_transpose_11);
    RUN_TEST(test_matrix_transpose_23_to_32);
    RUN_TEST(test_matrix_transpose_32_to_23);
    RUN_TEST(test_matrix_transpose_15_to_51);
    RUN_TEST(test_matrix_transpose_51_to_15);
    RUN_TEST(test_mapping_with_static_map);
    RUN_TEST(test_mapping_with_instance_map);
#ifndef _WIN32
    RUN_TEST(error_handling_of_addition_when_colr_ofA_dont_match_ofB);
    RUN_TEST(error_handling_of_static_sub_when_colr_ofA_dont_match_ofB);
    RUN_TEST(error_handling_of_hadamard_product_when_colr_ofA_dont_match_ofB);
    RUN_TEST(error_handling_of_matrix_product_when_col_ofA_dont_match_r_ofB);
    RUN_TEST(test_chanining_matrix_methods);
#endif
    RUN_TEST(test_matrix_from_array);
    RUN_TEST(test_matrix_to_array);
    RUN_TEST(test_matrix_de_serialization);
    RUN_TEST(test_matrix_copy);
    return UNITY_END();
}
