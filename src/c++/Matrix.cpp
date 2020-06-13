// Matrix lib

#include "Matrix.hpp"
#include <cstdlib> // malloc
#include <ctime> // rand_r, timeval64

// Constructors
Matrix::Matrix(const int rows, const int columns) : rows(rows), columns(columns) {
    this->allocSpace();

    float *ptr = &this->data[0][0];

    int end = this->rows * this->columns;
    PTR_START(end)
        *ptr = 0;

        ++ptr;
    PTR_END

    this->len = i;
}

Matrix::Matrix(void) : rows(1), columns(1) {
    this->data = new float *[this->rows];
    this->data[0] = new float [this->columns];

    this->data[0][0] = 0;

    this->len = 1;
}

Matrix::Matrix(const Matrix &m) : rows(m.rows), columns(m.columns) {
    this->allocSpace();

    float *ptr     = &this->data[0][0];
    const float *ref_ptr = &m.data[0][0];

    PTR_START(m.len)
        *ptr = *ref_ptr;

        ++ptr;
        ++ref_ptr;
    PTR_END

    this->len = i;
}

// Destructors
Matrix::~Matrix(void) {
}

void Matrix::free(void) {
    int i = 0;
    while (i < this->rows) {
        delete[] this->data[i];
        ++i;
    }
    delete[] this->data;
}

// Operators
void Matrix::matrix_equal(const Matrix &m) {
    this->free();

    this->rows = m.rows;
    this->columns = m.columns;

    this->allocSpace();

    float *ptr     = &this->data[0][0];
    const float *m_ptr   = &m.data[0][0];

    PTR_START(m.len)
        *ptr = *m_ptr;
            
        ++ptr;
        ++m_ptr;
    PTR_END
        
    this->len = i;
}

Matrix& Matrix::operator-=(const Matrix &m) {
    float *ptr   = &this->data[0][0];
    const float *m_ptr = &m.data[0][0];

    PTR_START(this->len)
        *ptr -= *m_ptr;

        ++ptr;
        ++m_ptr;
    PTR_END

    return *this;
}

Matrix& Matrix::operator*=(const Matrix &m) {
    if (this->columns <= m.rows) {
        this->rows = m.rows;
        this->columns = m.columns;

        this->allocSpace();

        int i = 0;
        while (i < this->rows) {
            int j = 0;
            while (j < m.columns) {
                int k = 0;
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
        const float *m_ptr  = &m.data[0][0];

        PTR_START(this->len)
            *ptr *= *m_ptr;

            ++ptr;
            ++m_ptr;
        PTR_END
    }

    return *this;
}

// Functions
Matrix Matrix::fromArray(const float* const arr) {
    Matrix t(2, 1);

    float *ptr = &t.data[0][0];

    PTR_START(t.len)
        *ptr = arr[i];

        ++ptr;
    PTR_END

    return t;
}

const float* Matrix::toArray(void) {
    // Array[2]
    float* arr = new float [2];

    // pointer to Matrix.data
    const float *ptr = &this->data[0][0];

    PTR_START(this->len)
        arr[i] = *ptr;

        ++ptr;
    PTR_END

    return arr;
}

void Matrix::randomize(void) {
    float *ptr = &this->data[0][0];

    struct timeval ts;

    unsigned int seed = (unsigned int)(ts.tv_sec);

    PTR_START(this->len)
        *ptr = 0.f + (rand_r(&seed) * (1.f - 0.f) / RAND_MAX);

        ++ptr;
    PTR_END
}

void Matrix::add(const Matrix &a) {
    float *ptr	    = &this->data[0][0];
    const float *ref_ptr  = &a.data[0][0];

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

void Matrix::add(const float n) {
    float *ptr = &this->data[0][0];

    PTR_START(this->len)
        *ptr += n;

        ++ptr;
    PTR_END
}

void Matrix::multiply(const float n) {
    // Scalar product
    float *ptr = &this->data[0][0];

    PTR_START(this->len)
        *ptr *= n;

        ++ptr;
    PTR_END
}

void Matrix::map(float (*const func)(float)) {
    // Apply a function to every element of matrix
    float *ptr = &this->data[0][0];

    PTR_START(this->len)
        *ptr = (*func)(*ptr);

        ++ptr;
    PTR_END
}

void Matrix::print(void) {
    const float *ptr = &this->data[0][0];

    int cout = 0;
    PTR_START(this->len)
        printf("%f ", *ptr);
        ++ptr;
        cout++;

        if(cout == this->columns) {
            cout = 0;
            printf("\n");
        }
    PTR_END
}

const nlohmann::json Matrix::serialize(const Matrix &m) const {
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


// Private function
void Matrix::allocSpace(void) {
    this->data = (float **)new float* [this->rows];
        
    int i = 0;
    while (i < this->rows) {
        this->data[i] = (float *)new float [this->columns];
        ++i;
    }
}

// Non Member Operator
Matrix operator*(const Matrix &a, const Matrix &b) {
    Matrix t(a);

    return t *= b;
}
