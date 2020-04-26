#ifndef MATRIX_H
#define MATRIX_H

#include <CL/cl_gl.h>

typedef struct Matrix Matrix;

struct Matrix {
    cl_float4 rows[4];
};

#define Matrix(\
    m00, m01, m02, m03, \
    m10, m11, m12, m13, \
    m20, m21, m22, m23, \
    m30, m31, m32, m33) \
        (Matrix){ { \
           {{ m00, m01, m02, m03 }}, \
           {{ m10, m11, m12, m13 }}, \
           {{ m20, m21, m22, m23 }}, \
           {{ m30, m31, m32, m33 }} \
        } }

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
