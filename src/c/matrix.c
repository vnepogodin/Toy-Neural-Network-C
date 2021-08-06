/* Matrix lib */

#include <vnepogodin/matrix.h>

#include <stdio.h>  /* printf */
#include <stdlib.h> /* malloc, posix_memalign, arc4random */
#include <string.h> /* strtof, strtok_r, strtok_s */
#ifdef __linux__
# include <fcntl.h>  /* openat, O_RDONLY */
# include <unistd.h> /* pread, close */
#elif _WIN32
# include <windows.h>
# include <bcrypt.h> /* BCryptGenRandom */
# pragma comment(lib, "bcrypt.lib")
#endif

struct _Matrix {
    /* Variables */
    int len;

    int rows;
    int columns;

    float* data;
};

/**
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define STMT_START do

/**
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define STMT_END while (0)

/**
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define PTR_START(end)  \
    register int i = 0; \
    while (i < (end)) {
/**
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define PTR_END \
    ++ptr;      \
    ++i;        \
    }

#ifdef _WIN32
# define posix_memalign(p, a, s) (((*(p)) = _aligned_malloc((s), (a))), *(p) ? 0 : errno)
# define posix_memalign_free _aligned_free
#else
# define posix_memalign_free free
#endif /* _WIN32 */

/* Non member functions */
#define length_str(string, size) STMT_START{ \
    register const char* buf_str = (string); \
    while (*buf_str != '\0') {               \
        ++(size);                            \
        ++buf_str;                           \
    }                                        \
}STMT_END

#define allocSpace(data_base, r, cols) STMT_START{                                            \
    (data_base) = (float*)malloc((unsigned long)(r) * (unsigned long)(cols) * sizeof(float)); \
}STMT_END

static void json_strsplit(register float* result, const char* _str, const int columns) {
    const char delim[2] = {","};

    register int size = 0;

    length_str(_str, size);

    register char* tmp = (char*)malloc((unsigned long)size - 1UL);

    /*   slice_str   */
    register int i = 2;
    register int j = 0;

    size -= 2;
    while (i < size) {
        tmp[j] = _str[i];

        ++j;
        ++i;
    }

    char* save_token = NULL;
#ifdef _WIN32
    register char* token = strtok_s(tmp, delim, &save_token);
#else
    register char* token = strtok_r(tmp, delim, &save_token);
#endif /* _WIN32 */

    i = 0;
    while (i < columns) {
        result[i] = strtof(token, NULL);

#ifdef _WIN32
        token = strtok_s(NULL, delim, &save_token);
#else
        token = strtok_r(NULL, delim, &save_token);
#endif /* _WIN32 */

        ++i;
    }

    free(tmp);
}

/**
 * Creates a new #Matrix with an all data = 0.
 * @param rows The rows of matrix.
 * @param columns The columns of matrix.
 * @returns The new #Matrix
 * @example
 *        2 rows, 3 columns
 *
 *           [0][0][0]
 *           [0][0][0]
 *
 */
Matrix* matrix_new_with_args(const int rows, const int columns) {
    Matrix* __matrix_m = NULL;

    /* assumed 0.001MB page sizes */
    posix_memalign((void**)&__matrix_m, 1024UL, 1024UL);

    if (__matrix_m != NULL) {
        __matrix_m->rows = rows;
        __matrix_m->columns = columns;

        allocSpace(__matrix_m->data, rows, columns);

        register float* ptr = &__matrix_m->data[0];

        register int end = rows * columns;
        PTR_START(end)
            *ptr = 0.F;
        PTR_END

        __matrix_m->len = i;
    }

    return __matrix_m;
}

/**
 * Creates a new #Matrix with reference rows and columns,
 * and with a reference data.
 * @param matrix The const #Matrix.
 * @returns The new #Matrix
 * @example
 *        2 rows, 1 columns  ->  2 rows, 1 columns
 *
 *            [232]          ->        [232]
 *            [21]           ->        [21]
 *
 */
Matrix* matrix_new_with_matrix(const Matrix* const __matrix_param) {
    Matrix* __matrix_m = matrix_new_with_args(__matrix_param->rows,
                                              __matrix_param->columns);

    if (__matrix_m != NULL) {
        register float* ptr = &__matrix_m->data[0];
        register const float* ref_ptr = &__matrix_param->data[0];

        PTR_START(__matrix_m->len)
            *ptr = *ref_ptr;

            ++ref_ptr;
        PTR_END
    }

    return __matrix_m;
}

/**
 * Frees #Matrix.
 * @param matrix The #Matrix.
 *
 */
void matrix_free(register Matrix* __matrix_param) {
    free(__matrix_param->data);
    __matrix_param->data = NULL;

    posix_memalign_free(__matrix_param);
}

/**
 * Add `num` to `matrix`->data.
 * @param matrix The #Matrix.
 * @param num The reference const float number.
 * @example
 *        2 rows, 1 columns
 *
 *             [321]         +       3.3
 *             [74]          +       3.3
 *
 */
void matrix_add_float(register Matrix* a_param, const float num_param) {
    register float* ptr = &a_param->data[0];

    PTR_START(a_param->len)
        *ptr += num_param;
    PTR_END
}

/**
 * Add `b`->data to `a`->data.
 * @param a The #Matrix.
 * @param b The reference #Matrix.
 * @example
 *  2 rows, 1 columns   2 rows, 1 columns
 *
 *      [321]        +       [0.1]
 *      [74]         +       [0.78]
 *
 */
void matrix_add_matrix(register Matrix* a_param, const Matrix* const b_param) {
    if ((a_param->rows == b_param->rows) || (a_param->columns == b_param->columns)) {
        register float* ptr = &a_param->data[0];
        register const float* ref_ptr = &b_param->data[0];

        PTR_START(a_param->len)
            *ptr += *ref_ptr;

            ++ref_ptr;
        PTR_END
    } else
        printf("Columns and Rows of A must match Columns and Rows of B.\n");
}

/**
 * Hadamard product.
 * @param a The #Matrix.
 * @param b The reference #Matrix.
 * @see https://en.wikipedia.org/wiki/Hadamard_product_(matrices)
 * @example:
 *        3 rows, 1 columns       2 rows, 1 columns
 *
 *             [64]
 *             [87]          *         [232]
 *             [21]                    [21]
 *
 */
void matrix_multiply(register Matrix* a_param, const Matrix* const b_param) {
    if ((a_param->rows == b_param->rows) || (a_param->columns == b_param->columns)) {
        register float* ptr = &a_param->data[0];
        register const float* b_ptr = &b_param->data[0];

        PTR_START(a_param->len)
            *ptr *= *b_ptr;

            ++b_ptr;
        PTR_END
    } else
        printf("Columns and Rows of A must match Columns and Rows of B.\n");
}

/**
 * Add number to `matrix`->data.
 * @param matrix The #Matrix.
 * @param num The reference floating-point number.
 * @example
 *        2 rows, 1 columns
 *
 *             [321]         *        3.3
 *             [74]          *        3.3
 *
 */
void matrix_multiply_scalar(register Matrix* m_param, const float num_param) {
    /* Scalar product */
    register float* ptr = &m_param->data[0];

    PTR_START(m_param->len)
        *ptr *= num_param;
    PTR_END
}

/**
 * Create array by #Matrix.
 * @param matrix The reference #Matrix.
 * @returns The new const float array
 * @example
 *        2 rows, 1 columns
 *
 *             [321]      ->      [321]
 *             [74]       ->      [74]
 *
 */
float* matrix_toArray(const Matrix* const m_param) {
    float* arr = (float*)malloc((unsigned long)m_param->len * sizeof(float));

    /* pointer to Matrix.data in CPU register */
    register const float* ptr = &m_param->data[0];

    PTR_START(m_param->len)
        arr[i] = *ptr;
    PTR_END

    return arr;
}

/**
 * Randomize `matrix`->data (from 0 to 2) - 1.
 * @param matrix The #Matrix.
 * @example
 *  2 rows, 1 columns   2 rows, 1 columns
 *
 *       [321]       ->      [0.1]
 *       [74]        ->      [0.78]
 *
 */
void matrix_randomize(register Matrix* m_param) {
    register float* ptr = &m_param->data[0];

#ifdef __linux__
    register int fd = openat(0, "/dev/urandom", O_RDONLY, 0);
    unsigned char buf[1] = {0U};

    if (fd != -1) {
        pread(fd, buf, 1, 0);
        close(fd);
    }

    unsigned __random = buf[0];

    PTR_START(m_param->len)
        *ptr = ((float)(rand_r(&__random) * 2.F / (float)RAND_MAX)) - 1.F;
#elif _WIN32
    UINT __random = 0U;

    BCryptGenRandom(NULL, (BYTE*)&__random, sizeof(UINT), BCRYPT_USE_SYSTEM_PREFERRED_RNG);

    PTR_START(m_param->len)
        *ptr = ((float)(__random * 2.F / UINT_MAX)) - 1.F;
#else
    unsigned __random = arc4random();

    PTR_START(m_param->len)
        *ptr = ((float)rand_r(&__random) * 2.F / (float)RAND_MAX) - 1.F;
#endif

    PTR_END
}

/**
 * Data of `matrix` equal, return value of `func`.
 * @param matrix The #Matrix.
 * @param func The float function.
 * @code
 * float func(float num) {
 *  return num * 2;
 * }
 * @endcode
 *
 */
void matrix_map(register Matrix* m_param, float (*const func_param)(float)) {
    register float* ptr = &m_param->data[0];

    PTR_START(m_param->len)
        *ptr = (*func_param)(*ptr);
    PTR_END
}

/**
 * Print elements of `matrix`->data.
 * @param matrix The const #Matrix.
 *
 */
void matrix_print(const Matrix* const m_param) {
    register const float* ptr = &m_param->data[0];

    register int counter = 0;
    register int i = 0;
    while (i < m_param->len) {
        printf("%f ", (double)*ptr);

        ++ptr;
        ++counter;
        if (counter == m_param->columns) {
            counter = 0;

            if ((i + 1) != m_param->len)
                printf("\n");
        }
        ++i;
    }
}

/**
 * Serialize `matrix` to JSON.
 * @param matrix The const #Matrix.
 * @returns The new #json_object
 *
 */
json_object* matrix_serialize(const Matrix* const m_param) {
    register json_object* t = json_object_new_object();

    json_object_object_add_ex(t, "rows", json_object_new_int(m_param->rows), 0);
    json_object_object_add_ex(t, "columns", json_object_new_int(m_param->columns), 0);
    json_object_object_add_ex(t, "data", json_object_new_array_ext(m_param->rows), 0);

    register json_object* temp_arr = json_object_new_array_ext(m_param->columns);

    register const float* ptr = &m_param->data[0];

    register int counter = 0;
    register int i = 0;
    while (i < m_param->rows) {
        json_object_array_add(temp_arr, json_object_new_double((double)*ptr));

        ++ptr;
        ++counter;
        if (counter == m_param->columns) {
            json_object_array_add(json_find(t, "data"), temp_arr);
            temp_arr = json_object_new_array_ext(m_param->columns);

            counter = 0;
            ++i;
        }
    }

    return t;
}

/**
 * Create #Matrix by `arr`.
 * @param arr The reference float array.
 * @returns The new #Matrix
 * @example
 *        2 rows, 1 columns
 *
 *             [321]       ->    [321]
 *             [74]        ->    [74]
 *
 */
Matrix* matrix_fromArray(const float* __restrict const arr_param, const int len_param) {
    register Matrix* t = matrix_new_with_args(len_param, 1);

    register float* ptr = &t->data[0];

    PTR_START(len_param)
        *ptr = arr_param[i];
    PTR_END

    return t;
}

/**
 * Transpositing `matrix`->data.
 * @param matrix The const #Matrix.
 * @see https://en.wikipedia.org/wiki/Transpose
 * @returns The new #Matrix
 *
 */
Matrix* matrix_transpose_static(const Matrix* const m_param) {
    register Matrix* t = matrix_new_with_args(m_param->columns, m_param->rows);

    register float* ptr = &t->data[0];
    register const float* m_ptr = &m_param->data[0];

    register unsigned counter = 0U;
    PTR_START(t->rows)
        register int j = 0;
        while (j < t->columns) {
            ptr[counter] = m_ptr[(j * t->rows) + i];

            ++counter;
            ++j;
        }
        ++i;
    }

    return t;
}

/**
 * Add to new #Matrix, multiply of `a`->data and `b`->data.
 * @param a The const #Matrix.
 * @param b The const #Matrix.
 * @returns The new #Matrix
 *
 */
Matrix* matrix_multiply_static(const Matrix* __restrict const a_param, const Matrix* __restrict const b_param) {
    Matrix* result = NULL;

    /* Matrix product */
    if (a_param->columns != b_param->rows) {
        printf("Columns of A must match rows of B.\n");
    } else {
        register Matrix* t = matrix_new_with_args(a_param->rows, b_param->columns);

        register float* ptr = &t->data[0];
        register const float* a_ptr = &a_param->data[0];
        register const float* b_ptr = &b_param->data[0];

        register unsigned counter = 0U;

        PTR_START(t->rows)
            register int j = 0;
            while (j < t->columns) {
                register int k = 0;
                register float sum = 0.F;
                while (k < a_param->columns) {
                    sum += a_ptr[(i * a_param->columns) + k] * b_ptr[(k * b_param->columns) + j];

                    ++k;
                }
                ptr[counter] = sum;

                ++counter;
                ++j;
            }
            ++i;
        }

        result = t;
    }

    return result;
}

/**
 * Subtract `a`->data and `b`->data.
 * @param a The const #Matrix.
 * @param b The const #Matrix.
 * @returns The new #Matrix
 * @example
 *        2 rows, 1 columns   2 rows, 1 columns
 *
 *             [321]        -      [3.3]
 *             [74]         -      [3.3]
 *
 *                         | |
 *                         \ /
 *
 *                        [317.7]
 *                        [70.7]
 *
 */
Matrix* matrix_subtract_static(const Matrix* const a_param, const Matrix* const b_param) {
    Matrix* result = NULL;

    if ((a_param->rows != b_param->rows) || (a_param->columns != b_param->columns)) {
        printf("Columns and Rows of A must match Columns and Rows of B.\n");
    } else {
        register Matrix* t = matrix_new_with_args(a_param->rows, b_param->columns);

        register float* ptr = &t->data[0];
        register const float* a_ptr = &a_param->data[0];
        register const float* b_ptr = &b_param->data[0];

        PTR_START(t->len)
            *ptr = *a_ptr - *b_ptr;

            ++a_ptr;
            ++b_ptr;
        PTR_END

        result = t;
    }

    return result;
}

/**
 * Apply `func` to every element of `matrix`.
 * @param matrix The const #Matrix.
 * @param func The float function.
 * @returns The new #Matrix
 *
 */
Matrix* matrix_map_static(const Matrix* const m_param, float (*const func_param)(float)) {
    register Matrix* t = matrix_new_with_matrix(m_param);

    register float* ptr = &t->data[0];

    PTR_START(t->len)
        *ptr = (*func_param)(*ptr);
    PTR_END

    return t;
}

/**
 * Create new #Matrix by json file or string.
 * @param json The const #json_object.
 * @returns The new #Matrix
 * @example
 *
 * {"rows":10,"columns":4,"data":[[0.19060910095479766,-0.7044789872907934,0.12916417175926737,-0.4493542740188917],[-0.8856888126717002,-0.9031315595837806,-0.25989472073659314,0.5503610418820337],[0.015814800116810357,0.20458699223751559,0.4144319562653632,-0.44217425771287694],[-0.6479931730467494,1.0466013336675943,0.34151503310804115,-0.9019454688191391],[0.6360219561553282,-0.7970648255401476,0.6967338500765156,1.026139659585225],[0.7070906013077707,-0.029736810492719402,0.4291704886090242,-0.5162258314269367],[0.5127199375296518,0.9105995373130602,-0.016193872134645272,-0.6064552608044859],[-0.37527682605966417,0.6967128785525135,-0.19384958454126475,0.7929244831790743],[0.12271433171587676,-0.42020640380104357,0.3119476057291891,-0.29433625893968235],[0.974302294354497,0.5381564104252675,0.7234688874658266,0.6823117502912057]]}
 * or
 * cities.json
 *
 */
Matrix* matrix_deserialize(const json_object* __restrict const t_param) {
    register Matrix* __matrix_m = matrix_new_with_args(json_object_get_int(json_find(t_param, "rows")),
                                                       json_object_get_int(json_find(t_param, "columns")));

    register float* ptr = &__matrix_m->data[0];

    register float* buf = (float*)malloc((unsigned long)__matrix_m->columns);
    json_strsplit(buf, json_object_to_json_string_ext(json_object_array_get_idx(json_find(t_param, "data"), 0), JSON_C_TO_STRING_SPACED), __matrix_m->columns);

    register int i = 0;
    register int counter = 0;
    while (i < __matrix_m->rows) {
        *ptr = buf[counter];
        ++ptr;
        counter++;

        if (counter == __matrix_m->columns) {
            counter = 0;
            ++i;

            if (i != __matrix_m->rows)
                json_strsplit(buf, json_object_to_json_string_ext(json_object_array_get_idx(json_find(t_param, "data"), (unsigned long)i), JSON_C_TO_STRING_SPACED), __matrix_m->columns);
        }
    }

    free(buf);

    return __matrix_m;
}
