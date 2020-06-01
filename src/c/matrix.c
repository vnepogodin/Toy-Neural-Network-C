/* Matrix lib */

#include <stdio.h> /* printf */
#include <stdlib.h> /* malloc */
#include <string.h> /* strtof, strtok */
#include <time.h> /* rand_r, timeval64 */

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

#define PTR_START(end) register int i = 0; while (i < (end)) {

#define PTR_END ++i; }

#define length_str(string, size) STMT_START{   \
    const register char* buf_str = (string);   \
    while (*(buf_str) != '\0') {               \
        ++(size);                              \
        ++buf_str;                             \
    }                                          \
}STMT_END

static float* json_strsplit(const char* _str, const char _delim, const int columns) {
    const register char delim[2] = { _delim, '\0' };
    
    register int size = 0;

    length_str(_str, size);
    
    register char* tmp = (char *)malloc(size - 1);

    /*   slice_str   */
    register int i = 2;
    register int j = 0;

    size -= 2;
    while (i < size) {
        tmp[j] = _str[i];
        
        ++j;
        ++i;
    }
    tmp[j] = '\0';

    register float* result = (float *)malloc(columns);

    register char* token = strtok(tmp, delim);

    i = 0;
    while (i < columns) {
        result[i] = strtof(token, NULL);

        token = strtok(NULL, delim);
       
        ++i;
    }

    return result;
}

static json_object* json_find(const json_object *__restrict j, const char* __restrict key) {
	json_object *t;

    json_object_object_get_ex(j, key, &t);

    return t;
}

#define allocSpace(matrix) STMT_START{                          \
    (matrix)->data = (float **)malloc((matrix)->rows);          \
                                                                \
    register int i = 0;                                         \
    while (i < (matrix)->rows) {                                \
        (matrix)->data[i] = (float *)malloc((matrix)->columns); \
        ++i;                                                    \
    }                                                           \
}STMT_END

/**
 * matrix_new_with_args:
 * @rows: a const rows of matrix.
 * @columns: a const columns of matrix.
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
	register Matrix *m = (Matrix *)malloc(sizeof(Matrix));
    
    m->rows = rows;
    m->columns = columns;

    allocSpace(m);
	
	register float *ptr = &m->data[0][0];
    
    register int end = m->rows * m->columns;
    PTR_START(end)
		*ptr = 0;

        ++ptr;
    PTR_END

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
    register Matrix *m = (Matrix *)malloc(sizeof(Matrix));

    m->rows = 1;
    m->columns = 1;

    m->data = (float **)malloc(m->rows);
    m->data[0] = (float *)malloc(m->columns);

    m->data[0][0] = 0;

    m->len = 1;

    return m;
}

/**
 * matrix_new_with_matrix:
 * @m: a const #Matrix.
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
    register Matrix *t = (Matrix *)malloc(sizeof(Matrix));
    
    t->rows = m->rows;
    t->columns = m->columns;
    
    allocSpace(t);

    register float *ptr     = &t->data[0][0];
	register float *ref_ptr = &m->data[0][0];
    
    PTR_START(m->len)
		*ptr = *ref_ptr;

        ++ptr;
        ++ref_ptr;
    PTR_END

    t->len = i;

    return t;
}

/* Deconstructor */
void matrix_free(register Matrix *__restrict m)  {
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
	
	a = matrix_new_with_matrix(b);
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
    register float *ptr     = &a->data[0][0];
    register float *b_ptr   = &b->data[0][0];
	
	PTR_START(a->len)
		*ptr -= *b_ptr;

        ++ptr;
        ++b_ptr;
	PTR_END
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
					++k;
				}
				++j;
			}
			++i;
		}
	} else {
		register float *ptr    = &a->data[0][0];
		register float *b_ptr  = &b->data[0][0];

		PTR_START(a->len)
			*ptr *= *b_ptr;

            ++ptr;
            ++b_ptr;
		PTR_END
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
Matrix* matrix_fromArray(const float* __restrict arr) {
	register Matrix *t = matrix_new_with_args(2, 1);

	register float *ptr = &t->data[0][0];

	PTR_START(t->len)
		*ptr = arr[i];

        ++ptr;
	PTR_END

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

    PTR_START(m->len)
		arr[i] = *ptr;

        ++ptr;
	PTR_END

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

	register struct timeval64 ts;
    
    unsigned int seed = (unsigned int)(ts.tv_sec ^ ts.tv_usec);

	PTR_START(m->len)
        *ptr = 0.f + (rand_r(&seed) * (1.f - 0.f) / RAND_MAX);

        ++ptr;
    PTR_END
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
    register float *ptr     = &a->data[0][0];
    register float *ref_ptr = &b->data[0][0];

    register int i = 0;

    if(b->rows >= a->rows) {
		while (i < a->len) {
			*ptr += *ref_ptr;

            ++ptr;
            ++ref_ptr;
            ++i;
        }
    } else {
        while (i < b->len) {
			*ptr += *ref_ptr;

            ++ptr;
            ++ref_ptr;
            ++i;
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
    
	PTR_START(a->len)
        *ptr += n;

        ++ptr;
    PTR_END
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

	PTR_START(m->len)
	    *ptr *= n;

        ++ptr;
	PTR_END
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

	PTR_START(m->len)
		*ptr = (*func)(*ptr);

        ++ptr;
	PTR_END
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
    
    register int cout = 0;
    PTR_START(m->len)
        printf("%f ", *ptr);
        ++ptr;
        cout++;

        if(cout == m->columns) {
            cout = 0;
            printf("\n");
        }
    PTR_END
}

const json_object* matrix_serialize(const Matrix *m) {
	register json_object *t = json_object_new_object();

	json_object_object_add(t, "rows", json_object_new_int(m->rows));
	json_object_object_add(t, "columns", json_object_new_int(m->columns));
	json_object_object_add(t, "data", json_object_new_array());
    
    register json_object *temp_arr = json_object_new_array();
    
    register float *ptr = &m->data[0][0];
        
    register int i = 0;
    register int cout = 0;
    while (i < m->rows) {
		json_object_array_add(temp_arr, json_object_new_double(*ptr));
        ++ptr;
        cout++;
		
        if(cout == m->columns) {
            json_object_array_add(json_find(t, "data"), temp_arr);
            temp_arr = json_object_new_array();
            cout = 0;
            ++i;
        }
    }

	return t;
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

	register float *ptr	  = &t->data[0][0];
	register float *m_ptr = &m->data[0][0];
    
	PTR_START(t->len)
	    *ptr = *m_ptr;

        ++ptr;
        ++m_ptr;
	PTR_END

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
Matrix* matrix_multiply_static(const Matrix *__restrict a, const Matrix *__restrict b) {
    register Matrix *t;
    /* Matrix product */
	if (a->columns != b->rows) {
		t = matrix_new_with_args(b->rows, b->columns);
		
		register int i = 0;
		while (i < t->rows) {
			register int j = 0;
			while (j < t->columns) {
				register int k = 0;
				while (k < t->columns) {
					t->data[i][j] += a->data[j][k] * b->data[i][j];
					++k;
				}
				++j;
			}
			++i;
		} 
    } else {
        /* Dot product of values in columns */
        t = matrix_new_with_args(a->rows, b->columns);

        register int i = 0;
        while (i < t->rows) {
            register int j = 0;
            while (j < t->columns) {
                register int k = 0;
                while (k < a->columns) {
                    t->data[i][j] += a->data[i][k] * b->data[k][j];
                    ++k;
                }
                ++j;
            }
            ++i;
        }
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
	
	register float *ptr	  = &t->data[0][0];
	register float *a_ptr = &a->data[0][0];
	register float *b_ptr = &b->data[0][0];	   
    
    PTR_START(t->len)
        *ptr = *a_ptr - *b_ptr;

        ++ptr;
        ++a_ptr;
        ++b_ptr;
    PTR_END

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
	
	register float *ptr   = &t->data[0][0];
    register float *m_ptr = &m->data[0][0];
    
    PTR_START(t->len)
        *ptr = (*func)(*m_ptr);

        ++ptr;
        ++m_ptr;
    PTR_END

    return t;
}

/**
 * matrix_deserialize:
 * @t: a const #json_object.
 * @example:
 *
 * {"rows":10,"columns":4,"data":[[0.19060910095479766,-0.7044789872907934,0.12916417175926737,-0.4493542740188917],[-0.8856888126717002,-0.9031315595837806,-0.25989472073659314,0.5503610418820337],[0.015814800116810357,0.20458699223751559,0.4144319562653632,-0.44217425771287694],[-0.6479931730467494,1.0466013336675943,0.34151503310804115,-0.9019454688191391],[0.6360219561553282,-0.7970648255401476,0.6967338500765156,1.026139659585225],[0.7070906013077707,-0.029736810492719402,0.4291704886090242,-0.5162258314269367],[0.5127199375296518,0.9105995373130602,-0.016193872134645272,-0.6064552608044859],[-0.37527682605966417,0.6967128785525135,-0.19384958454126475,0.7929244831790743],[0.12271433171587676,-0.42020640380104357,0.3119476057291891,-0.29433625893968235],[0.974302294354497,0.5381564104252675,0.7234688874658266,0.6823117502912057]]}
 * or
 * cities.json
 *
 * Create new #Matrix by json file or string.
 *
 * Returns: the new #Matrix
 */
Matrix* matrix_deserialize(const json_object *__restrict t) {
	register Matrix *m = matrix_new_with_args(json_object_get_int(json_object_object_get(t, "rows")),
											  json_object_get_int(json_object_object_get(t, "columns")));
    
    register float *ptr = &m->data[0][0];
    
    register float* buf = json_strsplit(json_object_get_string(json_object_array_get_idx(json_find(t, "data"), 0)), ',', m->columns);
        
    register int i = 0;
    register int cout = 0;
    while (i < m->rows) {
        *ptr = buf[cout];

        ++ptr;
        cout++;
        if(cout == m->columns) {
            cout = 0;
            ++i;
            
            if (i != m->rows)
                buf = json_strsplit(json_object_get_string(json_object_array_get_idx(json_find(t, "data"), i)), ',', m->columns);
        }
    }

	return m;
}
