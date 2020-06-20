// Matrix lib

#include "Matrix.hpp" // class Matrix
#include <random>

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
#define PTR_END ++ptr; ++i; }

class random_in_range {
    std::mt19937 rng;
public:
    random_in_range() : rng(std::random_device()()) {}
    float operator()(int low, int high) {
        std::uniform_real_distribution<float> uni(low, high);
        return uni(rng);
    }
};

// Constructors
Matrix::Matrix(const int rows, const int columns) : rows(rows), columns(columns) {
    this->allocSpace();

    float *ptr = &this->data[0][0];

    int end = this->rows * this->columns;
    PTR_START(end)
        *ptr = 0;
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

        ++ref_ptr;
    PTR_END

    this->len = i;
}

// Destructors
Matrix::~Matrix(void) {
    this->data = nullptr;

    delete[] this->data;
}

void Matrix::matrix_free(void) {
    delete[] this->data;
}

// Operators
void Matrix::matrix_equal(const Matrix &m) {
    delete this;

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
    PTR_END

    return arr;
}

void Matrix::randomize(void) {
    float *ptr = &this->data[0][0];

    random_in_range r;

    PTR_START(this->len)
        *ptr = r(0, 1);
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
    PTR_END
}

void Matrix::multiply(const float n) {
    // Scalar product
    float *ptr = &this->data[0][0];

    PTR_START(this->len)
        *ptr *= n;
    PTR_END
}

void Matrix::map(float (*const func)(float)) {
    // Apply a function to every element of matrix
    float *ptr = &this->data[0][0];

    PTR_START(this->len)
        *ptr = (*func)(*ptr);
    PTR_END
}

void Matrix::print(void) {
    const float *ptr = &this->data[0][0];

    int cout = 0;
    int i = 0;
    while (i < this->len) {
        printf("%f ", *ptr);
        ++ptr;
        cout++;

        if(cout == this->columns) {
            cout = 0;
            printf("\n");
        }
        ++i;
    }
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

// Static functions
Matrix Matrix::transpose(const Matrix &m) {
    Matrix t(m.rows, m.columns);

    float *ptr	 = &t.data[0][0];
    const float *m_ptr = &m.data[0][0];

    PTR_START(t.len)
        *ptr = *m_ptr;

        ++m_ptr;
    PTR_END

    return t;
}


// Private function
void Matrix::allocSpace(void) {
    this->data = new float*[this->rows];
        
    int i = 0;
    while (i < this->rows) {
        this->data[i] = new float[this->columns];
        ++i;
    }
}

// Non Member Operator
Matrix operator*(const Matrix &a, const Matrix &b) {
    Matrix t(a);

    return t *= b;
}
