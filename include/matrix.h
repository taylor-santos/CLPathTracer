#ifndef MATRIX_H
#define MATRIX_H

typedef struct Matrix Matrix;

struct Matrix {
    double values[4 * 4];
};

void
mat_set(Matrix *, unsigned int n, unsigned int m, double value);
double
mat_get(Matrix, unsigned int n, unsigned int m);
Matrix
mat_add(Matrix, Matrix);
Matrix
mat_multiply(Matrix, Matrix);
Matrix
mat_scaled(Matrix, double);
Matrix *
mat_scale(Matrix *, double);
Matrix
mat_inverse(Matrix, int *err);

#endif//MATRIX_H
