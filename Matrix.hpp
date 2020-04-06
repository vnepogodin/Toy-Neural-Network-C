// Matrix lib
#pragma once
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <ctime>
#include <json.hpp>
using namespace std;
typedef float (* vFunctionCall)(float args);

class Matrix {
private:
	// Variables
	int rows, colums;
    float **data;

    int l = sizeof(*data) / sizeof(**data);

    // Function
    void allocSpace() {
        this->data = new float *[this->rows];

        int i = 0;
        while (i < this->rows) {
            this->data[i] = new float [this->colums];
            i++;
        }
    }
public:
	// Constructors
    Matrix(const int rows, const int colums) {
        this->rows = rows;
        this->colums = colums;

        this->allocSpace();

        int i = 0;
        while (i < this->rows) {
            int j = 0;
            while (j < this->colums) {
                this->data[i][j] = 0;
                j++;
            }
            i++;
        }
    }
    Matrix() {
        this->rows = 1;
        this->colums = 1;
        this->allocSpace();
        this->data[0][0] = 0;
    }
    Matrix(const Matrix& m) {
        this->rows = m.rows;
        this->colums = m.colums;

        this->allocSpace();
        int i = 0;
        while (i < this->rows) {
            int j = 0;
            while (j < this->colums) {
                this->data[i][j] = m.data[i][j];
                j++;
            }
            i++;
        }
    }

    // Deconstructor
    virtual ~Matrix() {
        int i = 0;
        while (i < this->rows) {
            delete[] this->data[i];
            i++;
        }
        delete[] this->data;
    }

    // Operators
    Matrix& operator=(const Matrix& m) {
        if (this == &m) {
            return *this;
        }

        if (this->rows != m.rows || this->colums != m.colums) {
            int i = 0;
            while (i < this->rows) {
                delete[] this->data[i];
                i++;
            }
            delete[] this->data;

            this->rows = m.rows;
            this->colums = m.colums;
            this->allocSpace();
        }

        int i = 0;
        while (i < this->rows) {
            int j = 0;
            while (j < this->colums) {
                this->data[i][j] = m.data[i][j];
                j++;
            }
            i++;
        }

        return *this;
    }
    Matrix& operator-=(const Matrix& m) {
        int i = 0;
        while (i < this->rows) {
            int j = 0;
            while (j < this->colums) {
                this->data[i][j] -= m.data[i][j];
                j++;
            }
            i++;
        }
        return *this;
    }
    Matrix& operator*=(const Matrix& m) {
        Matrix temp(this->rows, m.colums);
        int i = 0;
        while (i < temp.rows) {
            int j = 0;
            while (j < temp.colums) {
                int k = 0;
                while (k < this->colums) {
                    temp.data[i][j] += (this->data[i][k] * m.data[k][j]);
                    k++;
                }
                j++;
            }
            i++;
        }
        return *this = temp;
    }

    // Functions
    Matrix copy(const Matrix& m) {
        this->rows = m.rows;
        this->colums = m.colums;

        this->allocSpace();

        int i = 0;
        while (i < this->rows) {
            int j = 0;
            while (j < this->colums) {
                this->data[i][j] = m.data[i][j];
                j++;
            }
            i++;
        }
        return m;
    }
    static Matrix fromArray(const float* arr) {
        Matrix t(2, 1);

        int i = 0;
        while (i < t.rows) {
            int j = 0;
            while (j < t.colums) {
                t.data[i][j] = arr[i];
                j++;
            }
            i++;
        }
        return t;
    }
    const float* toArray() {
        float *arr = new float [2];

        int i = 0;
        while (i < this->rows) {
            int j = 0;
            while (j < this->colums) {
                arr[i] = this->data[i][j];
                j++;
            }
            i++;
        }
        return arr;
    }
    void randomize() {
        int i = 0;
        while (i < this->rows) {
            int j = 0;
            while (j < this->colums) {
                this->data[i][j] = 0.f + (rand() * (1.f - 0.f) / RAND_MAX);
                j++;
            }
            i++;
        }
    }
    void add(const Matrix& a) {
        if(a.rows > this->rows) {
            int i = 0;
            while (i < this->rows) {
                int j = 0;
                while (j < this->colums) {
                    this->data[i][j] += a.data[i][j];
                    j++;
                }
                i++;
            }
        } else {
            int i = 0;
            while (i < a.rows) {
                int j = 0;
                while (j < a.colums) {
                    this->data[i][j] += a.data[i][j];
                    j++;
                }
                i++;
            }
        }
    }
    void add(const float n) {
        int i = 0;
        while (i < this->rows) {
            int j = 0;
            while (j < this->colums) {
                this->data[i][j] += n;
                j++;
            }
            i++;
        }
    }
    void multiply(const Matrix& a) {
        if (this->colums <= a.rows) {
            this->rows = a.rows;
            this->colums = a.colums;

            this->allocSpace();

            int i = 0;
            while (i < this->rows){
                int j = 0;
                while (j < a.colums) {
                    int k = 0;
                    while (k < a.colums) {
                        this->data[i][j] += this->data[j][k] * a.data[i][j];
                        k++;
                    }
                    j++;
                }
                i++;
            }
        } else {
            // hadamard product
            int i = 0;
            while (i < this->rows) {
                int j = 0;
                while (j < this->rows) {
                    this->data[i][j] *= a.data[i][j];
                    j++;
                }
                i++;
            }
        }
    }
    void multiply(const float n) {
        // Scalar product
        int i = 0;
        while (i < this->rows) {
            int j = 0;
            while (j < this->colums) {
                this->data[i][j] *= n;
                j++;
            }
            i++;
        }
    }
    void map(vFunctionCall func) {
        // Apply a function to every element of matrix
        int i = 0;
        while (i < this->rows) {
            int j = 0;
            while (j < this->colums) {
                this->data[i][j] = func(this->data[i][j]);
                j++;
            }
            i++;
        }
    }
    void print() {
        int i = 0;
        while (i < this->rows) {
            int j = 0;
            while (j < this->colums) {
                cout << this->data[i][j] << " ";
                j++;
            }
            cout << endl;
            i++;
        }
    }
    const nlohmann::json serialize(const Matrix& m) const {
        nlohmann::json t;
        t["rows"] = m.rows;
        t["colums"] = m.colums;

        int i = 0;
        while (i < m.rows) {
            int j = 0;
            while (j < m.colums) {
                t["data"] += m.data[i][j];
                j++;
            }
            i++;
        }

        return t.dump();
    }

    // Static functions
    static Matrix transpose(const Matrix& m) {
        Matrix t(m.rows, m.colums);
        t.allocSpace();

        int i = 0;
        while (i < t.rows) {
            int j = 0;
            while (j < t.rows) {
                t.data[i][j] = m.data[j][i];
                j++;
            }
            i++;
        }
        return t;
    }
    static Matrix multiply(const Matrix& a, const Matrix& b) {
        // Matrix product
        if (a.colums != b.rows) {
            Matrix t(b.rows, b.colums);

            int i = 0;
            while (i < t.rows) {
                int j = 0;
                while (j < t.colums) {
                    t.data[i][j] = 0;
                    int k = 0;
                    while (k < t.colums) {
                        t.data[i][j] += a.data[j][k] * b.data[i][j];
                        k++;
                    }
                    j++;
                }
                i++;
            }
            return t;
        } else {
            // Dot product of values in column
            Matrix t(a.rows, b.colums);

            int i = 0;
            while (i < t.rows) {
                int j = 0;
                while (j < t.colums) {
                    int k = 0;
                    while (k < a.colums) {
                        t.data[i][j] += a.data[i][k] * b.data[k][j];
                        k++;
                    }
                    j++;
                }
                i++;
            }
            return t;
        }
    }
    static Matrix subtract(const Matrix& a, const Matrix& b) {
        // Return a new Matrix(a - b)
        if (a.colums >= b.rows) {
            Matrix t(b.rows, b.colums);
            int i = 0;
            while (i < t.rows) {
                int j = 0;
                while (j < t.colums) {
                    t.data[i][j] = a.data[i][j] - b.data[i][j];
                    j++;
                }
                i++;
            }
            return t;
        } else {
            Matrix t(a.rows, b.colums);
            int i = 0;
            while (i < t.rows) {
                int j = 0;
                while (j < t.colums) {
                    t.data[i][j] = a.data[i][j] - b.data[i][j];
                    j++;
                }
                i++;
            }
            return t;
        }
    }
    static Matrix map(const Matrix& m, vFunctionCall func) {
        Matrix t(m.rows, m.colums);

        // Apply a function to every element of matrix
        int i = 0;
        while (i < m.rows) {
            int j = 0;
            while (j < m.colums) {
                t.data[i][j] = func(t.data[i][j]);
                j++;
            }
            i++;
        }

        return t;
    }
    static Matrix deserialize(const nlohmann::json& t) {
        Matrix m = *new Matrix(t["rows"].get<int>(), t["colums"].get<int>());

        int i = 0;
        while (i < m.rows) {
            int j = 0;
            while (j < m.colums) {
                m.data[i][j] = t["data"][j].get<float>();
                j++;
            }
            i++;
        }
        return m;
    }
};

// Operators
Matrix operator*(const Matrix& a, const Matrix& b) {
    Matrix t(a);
    return t *= b;
}
