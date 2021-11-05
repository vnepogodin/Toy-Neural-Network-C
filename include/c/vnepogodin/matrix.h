/* Matrix lib */
#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <vnepogodin/third_party/json-c/json.h> /* json_object */

typedef struct _Matrix Matrix;

#ifdef __cplusplus
extern "C" {
#endif

/* Constructors */

Matrix* matrix_new_with_args(const int, const int);
Matrix* matrix_new_with_matrix(const Matrix *const);

/* Destructor */

void matrix_free(register Matrix *);

/* Operators */

void matrix_add_matrix(register Matrix *, const Matrix *const);
void matrix_add_float(register Matrix *, const float);
void matrix_multiply(register Matrix *, const Matrix *const);
void matrix_multiply_scalar(register Matrix *, const float);

/* Functions */

float* matrix_toArray(const Matrix *const);
void matrix_randomize(register Matrix *);
void matrix_map(register Matrix *, float (*const)(float));
void matrix_print(const Matrix *const);
json_object* matrix_serialize(const Matrix *const);
int matrix_equal(const Matrix *__restrict const, const Matrix *__restrict const);
float* matrix_data(Matrix *);

/* Static functions */

Matrix* matrix_fromArray(const float* __restrict const, const int);
Matrix* matrix_transpose_static(const Matrix *const);
Matrix* matrix_multiply_static(const Matrix *__restrict const, const Matrix *__restrict const);
Matrix* matrix_subtract_static(const Matrix *const, const Matrix *const);
Matrix* matrix_map_static(const Matrix *const, float (*const)(float));
Matrix* matrix_deserialize(const json_object *__restrict const);
Matrix** matrix_deserialize_many(const json_object *__restrict const, int*);
Matrix* matrix_deserialize_file(const char*);
Matrix** matrix_deserialize_file_many(const char*, int*);

#ifdef __cplusplus
}
#endif

#endif /* __MATRIX_H__ */
