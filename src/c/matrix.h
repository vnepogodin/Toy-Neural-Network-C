/* Matrix lib */
#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <json-c/json_object.h> /* json_object */

typedef struct _Matrix Matrix;

#ifdef __cplusplus
    #define MATRIX_API extern "C"
#else
    #define MATRIX_API extern
#endif

/* Constructors */
MATRIX_API Matrix* matrix_new_with_args(const int, const int);
MATRIX_API Matrix* matrix_new(void);
MATRIX_API Matrix* matrix_new_with_matrix(const Matrix *const);

/* Destructor */
MATRIX_API void matrix_free(register Matrix *);

/* Operators */
MATRIX_API void matrix_subtract(register Matrix *, const Matrix *const);
MATRIX_API void matrix_multiply(register Matrix *, const Matrix *const);

/* Functions */
MATRIX_API Matrix* matrix_fromArray(const float* __restrict const, const int);
MATRIX_API void matrix_toArray(float* __restrict, const Matrix *const);
MATRIX_API void matrix_randomize(register Matrix *);
MATRIX_API void matrix_add_matrix(register Matrix *, const Matrix *const);
MATRIX_API void matrix_add_float(register Matrix *, const float);
MATRIX_API void matrix_multiply_scalar(register Matrix *, const float);
MATRIX_API void matrix_map(register Matrix *, float (*const)(float));
MATRIX_API void matrix_print(const Matrix *const);
MATRIX_API json_object* matrix_serialize(const Matrix *const);

/* Static functions */
MATRIX_API Matrix* matrix_transpose_static(const Matrix *const);
MATRIX_API Matrix* matrix_multiply_static(const Matrix *__restrict const, const Matrix *__restrict const);
MATRIX_API Matrix* matrix_subtract_static(const Matrix *const, const Matrix *const);
MATRIX_API Matrix* matrix_map_static(const Matrix *const, float (*const)(float));
MATRIX_API Matrix* matrix_deserialize(const json_object *__restrict const);

#undef MATRIX_API

#endif /* __MATRIX_H__ */
