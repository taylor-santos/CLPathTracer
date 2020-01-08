#ifndef MATRIX_H
#define MATRIX_H

typedef struct Matrix Matrix;

struct Matrix {
    void (*set)(Matrix *this, unsigned int n, unsigned int m, double value);
    double (*get)(const Matrix *this, unsigned int n, unsigned int m);
    Matrix (*plus)(const Matrix *this, const Matrix *other);
    Matrix (*times)(const Matrix *this, const Matrix *other);
    Matrix (*scale)(const Matrix *this, double factor);
    Matrix (*inverse)(const Matrix *this, int *err);
    double values[4 * 4];
};

Matrix
new_Matrix(const double *values);

#endif//MATRIX_H
