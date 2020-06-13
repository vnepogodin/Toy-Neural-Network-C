// Matrix lib
#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include "json.hpp" // json

/**
 * PTR_START(end):
 *
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define PTR_START(end) int i = 0; while (i < (end)) {

/**
 * PTR_END:
 *
 * Used within multi-statement macros so that they can be used in places
 * where only one statement is expected by the compiler.
 */
#define PTR_END ++i; }

class Matrix {
public:
    // Constructors
    Matrix(const int rows, const int columns);
    Matrix(void);
    Matrix(const Matrix &);

    // Destructors
    virtual ~Matrix(void);
    void free(void);

    // Operators
    void matrix_equal(const Matrix &);
    Matrix& operator-=(const Matrix &);
    Matrix& operator*=(const Matrix &);

    // Functions
    static Matrix fromArray(const float* const arr);
    const float* toArray(void);
    void randomize(void);
    void add(const Matrix &);
    void add(const float);
    void multiply(const float);
    void map(float (*const)(float));
    void print(void);
    const nlohmann::json serialize(const Matrix &) const;

    // Static functions
    static Matrix transpose(const Matrix &m) {
        Matrix t(m.rows, m.columns);

        float *ptr	 = &t.data[0][0];
        const float *m_ptr = &m.data[0][0];

        PTR_START(t.len)
            *ptr = *m_ptr;

            ++ptr;
            ++m_ptr;
        PTR_END

        return t;
    }

    static Matrix multiply(const Matrix &a, const Matrix &b) {
        Matrix t;

        // Matrix product
        if (a.columns != b.rows) {
            t = *new Matrix(b.rows, b.columns);

            int i = 0;
            while (i < t.rows) {
                int j = 0;
                while (j < t.columns) {
                    t.data[i][j] = 0;
                    int k = 0;
                    while (k < t.columns) {
                        t.data[i][j] += a.data[j][k] * b.data[i][j];
                        ++k;
                    }
                    ++j;
                }
                ++i;
            }
        } else {
            // Dot product of values in column
            t = *new Matrix(a.rows, b.columns);

            int i = 0;
            while (i < t.rows) {
                int j = 0;
                while (j < t.columns) {
                    int k = 0;
                    while (k < a.columns) {
                        t.data[i][j] += a.data[i][k] * b.data[k][j];
                        ++k;
                    }
                    ++j;
                }
                ++i;
            }
        }

        return t;
    }

    static Matrix subtract(const Matrix &a, const Matrix &b) {
        Matrix t;
        if (a.columns >= b.rows)
            t = *new Matrix(b.rows, b.columns);
        else
            t = *new Matrix(a.rows, b.columns);

        float *ptr	 = &t.data[0][0];
        const float *a_ptr = &a.data[0][0];
        const float *b_ptr = &b.data[0][0];

        PTR_START(t.len)
            *ptr = *a_ptr - *b_ptr;
            
            ++ptr;
            ++a_ptr;
            ++b_ptr;
        PTR_END

        return t;
    }

    static Matrix map(const Matrix &m, float (*const func)(float)) {
        Matrix t(m.rows, m.columns);

        float *ptr   = &t.data[0][0];
        const float *m_ptr = &m.data[0][0];

        PTR_START(t.len)
            *ptr = (*func)(*m_ptr);
            
            ++ptr;
            ++m_ptr;
        PTR_END

        return t;
    }

    static Matrix deserialize(const nlohmann::json &t) {
        Matrix m = *new Matrix(t["rows"].get<int>(), t["columns"].get<int>());

        int i = 0;
        while (i < m.rows) {
            int j = 0;
            while (j < m.columns) {
                m.data[i][j] = t["data"][j].get<float>();
                j++;
            }
            i++;
        }
        return m;
    }

private:
    // Variables
    int len;

    int rows, columns;

    float **data;

    // Function
    void allocSpace(void);
};

#endif // __MATRIX_HPP__
