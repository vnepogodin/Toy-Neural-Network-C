/* Matrix lib */
#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <json-c/json_object.h> /* json_object */

typedef struct _Matrix Matrix;

/* Constructors */
extern Matrix* matrix_new_with_args(const int, const int);
extern Matrix* matrix_new(void);
extern Matrix* matrix_new_with_matrix(const Matrix *const);

/* Destructor */
extern void matrix_free(register Matrix *__restrict);

/* Operators */
extern void matrix_subtract(register Matrix *, const Matrix *const);
extern void matrix_multiply(register Matrix *, const Matrix *const);

/* Functions */
extern Matrix* matrix_fromArray(const float* __restrict const);
extern const float* matrix_toArray(const Matrix *const);
extern void matrix_randomize(register Matrix *);
extern void matrix_add_matrix(register Matrix *, const Matrix *const);
extern void matrix_add_float(register Matrix *, const float);
extern void matrix_multiply_scalar(register Matrix *, const float);
extern void matrix_map(register Matrix *, float (*const)(float));
extern void matrix_print(const Matrix *const);
extern json_object* matrix_serialize(const Matrix *const);

/* Static functions */
extern Matrix* matrix_transpose_static(const Matrix *const);
extern Matrix* matrix_multiply_static(const Matrix *__restrict const, const Matrix *__restrict const);
extern Matrix* matrix_subtract_static(const Matrix *const, const Matrix *const);
extern Matrix* matrix_map_static(const Matrix *const, float (*const)(float));
extern Matrix* matrix_deserialize(const json_object *__restrict const);

#endif /* __MATRIX_H__ */
