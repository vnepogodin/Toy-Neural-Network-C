/* Matrix lib */

#include <stdio.h> /* printf */
#include <stdlib.h> /* malloc */
#include <time.h> /* rand, srand */

#include "matrix.h"

/**
 * STMT_START:
 *
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define STMT_START do

/**
 * STMT_END:
 *
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define STMT_END while(0)

#define allocSpace(matrix) STMT_START{				\
    (matrix)->data = (float **)malloc((matrix)->rows);		\
								\
    register int i = 0;						\
    while (i < (matrix)->rows) {				\
    	(matrix)->data[i] = (float *)malloc((matrix)->columns); \
	    i++;						\
    }								\
}STMT_END

/**
 * matrix_new_with_args:
 * @example:
 *				2 rows, 3 columns
 *
 *				   [0][0][0]
 *				   [0][0][0]
 *
 * Creates a new #Matrix with an all data = 0.
 *
 * Returns: the new #Matrix
 */
Matrix* matrix_new_with_args(const int rows, const int columns) {
	register Matrix *m = malloc(sizeof(Matrix));
    
    m->rows = rows;
    m->columns = columns;

    allocSpace(m);
	
	register float *ptr = &m->data[0][0];
    
    register int i = 0, end = m->rows * m->columns;
    while (i < end) {
		*ptr++ = 0;
        i++;
    }

    m->len = i;

    return m;
}

/**
 * matrix_new:
 * @example:
 *				1 rows, 1 columns
 *
 *				      [0]
 *
 * Creates a new #Matrix with 1 rows and 1 columns,
 * and with an all data = 0.
 *
 * Returns: the new #Matrix
 */
Matrix* matrix_new(void) {
    register Matrix *m = malloc(sizeof(Matrix));

    m->rows = 1;
    m->columns = 1;

    allocSpace(m);

    m->len = 1;

    return m;
}

/**
 * matrix_new_with_matrix:
 * @m: a #Matrix.
 * @example:
 *		
 *		2 rows, 1 columns  ->	2 rows, 1 columns
 *
 *			  [232]		   ->		 [232]
 *			  [21]		   ->		 [21]
 *
 * Creates a new #Matrix with reference rows and columns,
 * and with a reference data.
 *
 * Returns: the new #Matrix
 */
Matrix* matrix_new_with_matrix(const Matrix *m) {
    register Matrix *t = malloc(sizeof(Matrix));
    
    t->rows = m->rows;
    t->columns = m->columns;
    
    allocSpace(t);

    register float *ptr     = &t->data[0][0],
				   *ref_ptr = &m->data[0][0];
    
    register int i = 0;
    while (i < m->len) {
		*ptr++ = *ref_ptr++;
        i++;
    }

    t->len = i;

    return t;
}

/* Deconstructor */
void matrix_free(register Matrix *m)  {
    free(m->data);
    m->data = NULL;
}

/**
 * matrix_equal:
 * @a: a #Matrix.
 * @b: a reference #Matrix.
 * @example:
 *		
 *		2 rows, 2 columns      ->	2 rows, 1 columns
 *
 *			  [64][2]		   ->		 [232]
 *			  [21][546]		   ->		 [21]
 *
 * #Matrix with reference rows and columns,
 * and with a reference data
 *
 */
void matrix_equal(register Matrix *a, const Matrix *b) {
    matrix_free(a);
	
	a->rows = b->rows;
    a->columns = b->columns;
        
	allocSpace(a);

	register float *ptr		= &a->data[0][0],
				   *ref_ptr = &b->data[0][0];
	
	register int i = 0;
    while (i < b->len) {
		*ptr++ = *ref_ptr++;
        i++;
    }

	a->len = i;
}

/**
 * matrix_subtract:
 * @a: a #Matrix.
 * @b: a reference #Matrix.
 * @example:
 *		
 *		3 rows, 1 columns   	2 rows, 1 columns
 *
 *			  [64]
 *			  [87]			-		 [232]
 *			  [21]		   		     [21]
 *
 * Subtract data of a #Matrix
 *
 */
void matrix_subtract(register Matrix *a, const Matrix *b) {
    register float *ptr		= &a->data[0][0],
				   *ref_ptr = &b->data[0][0];
	
	register int i = 0;
	while (i < a->len) {
		*ptr++ -= *ref_ptr++;
		i++;
    }
}

/**
 * matrix_multiply:
 * @a: a #Matrix.
 * @b: a reference #Matrix.
 * @example:
 *		
 *		3 rows, 1 columns   	2 rows, 1 columns
 *
 *			  [64]
 *			  [87]			*		 [232]
 *			  [21]		   		     [21]
 *
 *							or
 *		
 *		3 rows, 1 columns		2 rows, 1 columns
 *
 *			  [64]
 *			  [87]			+*		[232]
 *			  [21]					[21]
 *
 *					 [] += 64 * 232
 *
 * Hadamard product, 
 * or multiply data of a #Matrix
 *
 */
void matrix_multiply(register Matrix *a, const Matrix *b) {
    if (a->columns <= b->rows) {
        a->rows = b->rows;
        a->columns = b->columns;

		allocSpace(a);

    	register int i = 0;
    	while (i < a->rows) {
			register int j = 0;
			while (j < b->columns) {
				register int k = 0;
				while (k < b->columns) {
					a->data[i][j] += a->data[i][k] * b->data[k][j];
				   	k++;
				}
				j++;
			}
    	    i++;
		}
    } else {
		register float *ptr		= &a->data[0][0],
				       *ref_ptr = &b->data[0][0];

        register int i = 0;
		while (i < a->len) {
			*ptr++ *= *ref_ptr++;
			i++;
		}
    }
}

/**
 * matrix_fromArray:
 * @arr: a reference float array.
 * @example:
 *					2 rows, 1 columns
 *
 *		[321]	->		 [321]
 *		[74]	->		 [74]
 * 
 * Create #Matrix by array
 *
 * Returns: the new #Matrix
 */
Matrix* matrix_fromArray(const float* arr) {
    register Matrix *t = matrix_new_with_args(2, 1);

	register float *ptr = &t->data[0][0];

    register int i = 0;
    while (i < t->len) {
        *ptr++ = arr[i];
        i++;
    }
    
	return t;
}

/**
 * matrix_toArray:
 * @m: a reference #Matrix.
 * @example:
 *	
 *	2 rows, 1 columns
 *
 *		[321]	->		[321]
 *		[74]	->		[74]
 * 
 * Create array by #Matrix
 *
 * Returns: the new const float array
 */
const float* matrix_toArray(const Matrix *m) {
	/* Array[2] */
	register float* arr = (float *)malloc(2);
	
	/* pointer to Matrix.data in CPU register */
	register float *ptr = &m->data[0][0];

    register int i = 0;
    while (i < m->len) {
		arr[i] = *ptr++;
		i++;
    }

    return arr;
}

/**
 * matrix_randomize:
 * @m: a #Matrix.
 * @example:
 *	
 *	2 rows, 1 columns	2 rows, 1 columns
 *
 *		[321]	->		[0.1]
 *		[74]	->		[0.78]
 * 
 * Randomize @m data from 0 to 1
 *
 */
void matrix_randomize(register Matrix *m) {
    register float *ptr = &m->data[0][0];
    
    srand(time(NULL));

	register int i = 0;
    while (i < m->len) {
        *ptr++ = 0.f + (rand() * (1.f - 0.f) / RAND_MAX);
        i++;
    }
}

/**
 * matrix_add_matrix:
 * @a: a #Matrix.
 * @b: a reference #Matrix.
 * @example:
 *	
 *	2 rows, 1 columns	2 rows, 1 columns
 *
 *		[321]	+		[0.1]
 *		[74]	+		[0.78]
 *
 * Add @b data to @a data, by @b len,
 * or if @b rows larger than @a rows,
 * add @b data to @a data, by @a len
 *
 */
void matrix_add_matrix(register Matrix *a, const Matrix *b) {
    if(b->rows > a->rows) {
		register float *ptr		= &a->data[0][0],
					   *ref_ptr = &b->data[0][0];
		
		register int i = 0;
		while (i < a->len) {
			*ptr++ += *ref_ptr++;
            i++;
        }
    } else {
		register float *ptr		= &a->data[0][0],
					   *ref_ptr = &b->data[0][0];

		register int i = 0;
        while (i < b->len) {
			*ptr++ += *ref_ptr;
            i++;
		}
    }
}

/**
 * matrix_add_float:
 * @a: a #Matrix.
 * @n: a reference const float num.
 * @example:
 *	
 *	2 rows, 1 columns
 *
 *		[321]	+		3.3
 *		[74]	+	    3.3
 *
 * Add @n to @a data
 *
 */
void matrix_add_float(register Matrix *a, const float n) {
    register float *ptr = &a->data[0][0];
    
	register int i = 0;
    while (i < a->len) {
        *ptr++ += n;
        i++;
    }
}

/**
 * matrix_multiply_scalar:
 * @m: a #Matrix.
 * @n: a reference const float num.
 * @example:
 *	
 *	2 rows, 1 columns
 *
 *		[321]	*		3.3
 *		[74]	*	    3.3
 *
 * Add @n to @a data
 *
 */
void matrix_multiply_scalar(register Matrix *m, const float n) {
    /* Scalar product */
    register float *ptr = &m->data[0][0];

	register int i = 0;
    while (i < m->len) {
	    *ptr++ *= n;
		i++;
    }
}

/**
 * matrix_map:
 * @m: a #Matrix.
 * @func: a float function.
 * @example:
 * 
 * float func(float num) {
 *	return num * 2 
 * } 
 *
 * @m data equal, return value of @func
 *
 */
void matrix_map(register Matrix *m,  float (*func)(float)) {
    register float *ptr = &m->data[0][0];

	register int i = 0;
    while (i < m->len) {
		*ptr++ = (*func)(*ptr);
		i++;
    }
}

/**
 * matrix_print:
 * @m: a const #Matrix.
 *
 * Print elements of @m data
 *
 */
void matrix_print(const Matrix *m) {
	register float *ptr = &m->data[0][0];
    
    register int i = 0, cout = 0;
    while (i < m->len) {
        printf("%f ", *ptr++);
        cout++;
        if(cout == m->columns) {
            cout = 0;
            printf("\n");
        }
        i++;
    }
}

/**
 * matrix_transpose_static:
 * @m: a const #Matrix.
 *
 * Transpositing @m data
 *
 * Returns: the new #Matrix
 */
Matrix* matrix_transpose_static(const Matrix *m) {
	register Matrix *t = matrix_new_with_args(m->rows, m->columns);   

	register float *ptr	  = &t->data[0][0],
				   *m_ptr = &m->data[0][0];
    
	register int i = 0;
    while (i < t->len) {
	    *ptr++ = *m_ptr++;
		i++;
    }

    return t;
}

/**
 * matrix_multiply_static:
 * @a: a const #Matrix.
 * @b: a const #Matrix.
 *
 * Add to new #Matrix, multiply of @a data and @b data
 *
 * Returns: the new #Matrix
 */
Matrix* matrix_multiply_static(const Matrix *a, const Matrix *b) {
    /* Matrix product */
	if (a->columns != b->rows) {
		register Matrix *t = matrix_new_with_args(b->rows, b->columns);
		
		register int i = 0;
		while (i < t->rows) {
			register int j = 0;
			while (j < t->columns) {
				register int k = 0;
				while (k < t->columns) {
					t->data[i][j] += a->data[j][k] * b->data[i][j];
					k++;
				}
				j++;
			}
			i++;
		} 
		return t;
	}
	
	/* Dot product of values in columns */
	register Matrix *t = matrix_new_with_args(a->rows, b->columns);

	register int i = 0;
	while (i < t->rows) {
	    register int j = 0;
		while (j < t->columns) {
			register int k = 0;
			while (k < a->columns) {
				t->data[i][j] += a->data[i][k] * b->data[k][j];
				k++;
			}
			j++;
	    }
	    i++;
	}
	
	return t;
}

/**
 * matrix_subtract_static:
 * @a: a const #Matrix.
 * @b: a const #Matrix.
 * @example:
 *	
 *	2 rows, 1 columns	2 rows, 1 columns
 *
 *		[321]		  -		 [3.3]
 *		[74]		  -		 [3.3]
 *
 *					 | |
 *					 \ /
 *
 *				   [317.7]
 *				   [70.7]
 *
 * Subtract @a data and @b data.
 *
 * Returns: the new #Matrix
 */
Matrix* matrix_subtract_static(const Matrix *a, const Matrix *b) {
    register Matrix *t;
	if (a->columns >= b->rows) 
		t = matrix_new_with_args(b->rows, b->columns);
	else
		t = matrix_new_with_args(a->rows, b->columns);
	
	register float *ptr	  = &t->data[0][0],
				   *a_ptr = &a->data[0][0],
				   *b_ptr = &b->data[0][0];	   
    
    register int i = 0;
    while (i < t->len) {
        *ptr++ = (*a_ptr++) - (*b_ptr++);
        i++;
    }

	return t;	
}

/**
 * matrix_map_static:
 * @m: a const #Matrix.
 * @func: a float func.
 *
 *
 *
 * Apply a function to every element of matrix.
 *
 * Returns: the new #Matrix
 */
Matrix* matrix_map_static(const Matrix *m, float (*func)(float)) {
    register Matrix *t = matrix_new_with_args(m->rows, m->columns);
	
	register float *ptr   = &t->data[0][0],
				   *m_ptr = &m->data[0][0];
    
    register int i = 0;
    while (i < t->len) {
        *ptr++ = (*func)(*m_ptr++);
        i++;
    }

    return t;
}
