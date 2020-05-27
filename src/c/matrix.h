/* Matrix lib */
#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <json.h> /* json_object */

typedef struct _Matrix Matrix;

struct _Matrix {
    /* Variables */
    int len;

	int rows, columns;
	
	float **data;
};

/* Constructors */
Matrix* matrix_new_with_args(const int, const int);
Matrix* matrix_new(void);
Matrix* matrix_new_with_matrix(const Matrix *);

/* Deconstructor */
void matrix_free(register Matrix *__restrict);

/* Operators */
void matrix_equal(register Matrix *, const Matrix *);
void matrix_subtract(register Matrix *, const Matrix *);
void matrix_multiply(register Matrix *, const Matrix *);

/* Functions */
Matrix* matrix_fromArray(const float* __restrict);
const float* matrix_toArray(const Matrix *);
void matrix_randomize(register Matrix *);
void matrix_add_matrix(register Matrix *, const Matrix *);
void matrix_add_float(register Matrix *, const float);
void matrix_multiply_scalar(register Matrix *, const float);
void matrix_map(register Matrix *, float (*)(float));
void matrix_print(const Matrix *);
const json_object* matrix_serialize(const Matrix *);

/* Static functions */
Matrix* matrix_transpose_static(const Matrix *);
Matrix* matrix_multiply_static(const Matrix *__restrict, const Matrix *__restrict);
Matrix* matrix_subtract_static(const Matrix *, const Matrix *);
Matrix* matrix_map_static(const Matrix *, float (*)(float));
Matrix* matrix_deserialize(const json_object *__restrict);

#endif /* __MATRIX_H__ */
