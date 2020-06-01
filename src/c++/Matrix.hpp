// Matrix lib
#pragma once

#include <cstdlib> // malloc
#include <ctime> // rand_r, timeval64
#include <json.hpp> // json

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

using namespace std;

typedef float (* vFunctionCall)(float args);

class Matrix {
public:
    // Constructors
    Matrix(const int rows, const int columns) {
        this->rows = rows;
        this->columns = columns;

        this->allocSpace();

        float *ptr = &this->data[0][0];

        int end = this->rows * this->columns;
        PTR_START(end)
            *ptr = 0;

            ++ptr;
        PTR_END

        this->len = i;
    }
    Matrix(void) {
        this->rows = 1;
        this->columns = 1;

        this->data = new float *[this->rows];
        this->data[0] = new float [this->columns];

        this->data[0][0] = 0;

        this->len = 1;
    }
    Matrix(const Matrix& m) {
        this->rows = m.rows;
        this->columns = m.columns;

        this->allocSpace();

        float *ptr     = &this->data[0][0];
        float *ref_ptr = &m.data[0][0];

        PTR_START(m.len)
            *ptr = *ref_ptr;

            ++ptr;
            ++ref_ptr;
        PTR_END

        this->len = i;
    }

    // Deconstructor
    virtual ~Matrix(void) {
        int i = 0;
        while (i < this->rows) {
            delete[] this->data[i];
            ++i;
        }
        delete[] this->data;
    }

    // Operators
    Matrix& operator=(const Matrix& m) {
        this = *new Matrix(m);

        return *this;
    }
    Matrix& operator-=(const Matrix& m) {
        float *ptr   = &this->data[0][0];
        float *m_ptr = &m.data[0][0];

        PTR_START(this->len)
            *ptr -= *m_ptr;

            ++ptr;
            ++m_ptr;
        PTR_END

        return *this;
    }
    Matrix& operator*=(const Matrix& m) {
        if (this->columns <= m.rows) {
            this->rows = m->rows;
            this->columns = m.columns;

            this->allocSpace();

            int i = 0;
            while (i < this->rows) {
                register int j = 0;
                while (j < m.columns) {
                    register int k = 0;
                    while (k < m.columns) {
                        this->data[i][j] += this->data[i][k] * m.data[k][j];
                        ++k;
                    }
                    ++j;
                }
                ++i;
            }
        } else {
            float *ptr    = &this->data[0][0];
            float *m_ptr  = &m.data[0][0];

            PTR_START(this->len)
                *ptr *= *m_ptr;

                ++ptr;
                ++m_ptr;
            PTR_END
        }

        return *this;
    }

    // Functions
    static Matrix fromArray(const float* arr) {
        Matrix t(2, 1);

        float *ptr = &t.data[0][0];

        PTR_START(t.len)
            *ptr = arr[i];

            ++ptr;
        PTR_END

        return t;
    }
    const float* toArray(void) {
        // Array[2]
        float* arr = new float [2];

        // pointer to Matrix.data
        float *ptr = &this->data[0][0];

        PTR_START(this->len)
            arr[i] = *ptr;

            ++ptr;
        PTR_END

        return arr;
    }
    void randomize(void) {
        float *ptr = &this->data[0][0];

        struct timeval64 ts;

        unsigned int seed = (unsigned int)(ts.tv_sec ^ ts.tv_usec);

        PTR_START(this->len)
            *ptr = 0.f + (rand_r(&seed) * (1.f - 0.f) / RAND_MAX);

            ++ptr;
        PTR_END
    }
    void add(const Matrix& a) {
        float *ptr	    = &this->data[0][0];
        float *ref_ptr  = &a.data[0][0];

        int i = 0;

        if(a.rows > this->rows) {
            while (i < this->len) {
                *ptr += *ref_ptr;

                ++ptr;
                ++ref_ptr;
                ++i;
            }
        } else {
            while (i < a.len) {
                *ptr += *ref_ptr;

                ++ptr;
                ++ref_ptr;
                ++i;
            }
        }
    }
    void add(const float n) {
        float *ptr = &this->data[0][0];

        PTR_START(this->len)
            *ptr += n;

            ++ptr;
        PTR_END
    }
    void multiply(const float n) {
        // Scalar product
        float *ptr = &this->data[0][0];

        PTR_START(this->len)
            *ptr *= n;

            ++ptr;
        PTR_END
    }
    void map(vFunctionCall func) {
        // Apply a function to every element of matrix
        float *ptr = &this->data[0][0];

        PTR_START(this->len)
            *ptr = func(*ptr);

            ++ptr;
        PTR_END
    }
    void print(void) {
        float *ptr = &this->data[0][0];

        int cout = 0;
        PTR_START(this->len)
            cout << *ptr << " ";
            ++ptr;
            cout++;

            if(cout == this->columns) {
                cout = 0;
                cout << endl;
            }
        PTR_END
    }
    const nlohmann::json serialize(const Matrix& m) const {
        nlohmann::json t;
        t["rows"] = m.rows;
        t["columns"] = m.columns;

        int i = 0;
        while (i < m.rows) {
            int j = 0;
            while (j < m.columns) {
                t["data"] += m.data[i][j];
                j++;
            }
            ++i;
        }

        return t.dump();
    }

    // Static functions
    static Matrix transpose(const Matrix& m) {
        Matrix t(m.rows, m.columns);

        float *ptr	 = &t.data[0][0];
        float *m_ptr = &m.data[0][0];

        PTR_START(t.len)
            *ptr = *m_ptr;

            ++ptr;
            ++m_ptr;
        PTR_END

        return t;
    }
    static Matrix multiply(const Matrix& a, const Matrix& b) {
        Matrix t;

        // Matrix product
        if (a.columns != b.rows) {
            t(b.rows, b.columns);

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
            t(a.rows, b.columns);

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
    static Matrix subtract(const Matrix& a, const Matrix& b) {
        Matrix t;
        if (a.columns >= b.rows)
            t(b.rows, b.columns);
        else
            t(a.rows, b.columns);

        float *ptr	 = &t.data[0][0];
        float *a_ptr = &a.data[0][0];
        float *b_ptr = &b.data[0][0];

        PTR_START(t.len)
            *ptr = *a_ptr - *b_ptr;
            
            ++ptr;
            ++a_ptr;
            ++b_ptr;
        PTR_END

        return t;
    }
    static Matrix map(const Matrix& m, vFunctionCall func) {
        Matrix t(m.rows, m.columns);

        float *ptr   = &t.data[0][0];
        float *m_ptr = &m.data[0][0];

        PTR_START(t.len)
            *ptr = func(*m_ptr);
            
            ++ptr;
            ++m_ptr;
        PTR_END

        return t;
    }
    static Matrix deserialize(const nlohmann::json& t) {
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
    void allocSpace() {
        this->data = new float *[this->rows];

        int i = 0;
        while (i < this->rows) {
            this->data[i] = new float [this->columns];
            ++i;
        }
    }
};
