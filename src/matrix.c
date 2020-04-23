#include "matrix.h"

void
mat_set(Matrix *mat, unsigned int n, unsigned int m, double value) {
    mat->values[4 * m + n] = value;
}

double
mat_get(Matrix mat, unsigned int n, unsigned int m) {
    return mat.values[4 * m + n];
}

Matrix
mat_add(Matrix a, Matrix b) {
    Matrix ret = { 0 };
    for (unsigned int i = 0; i < 4 * 4; i++) {
        ret.values[i] = a.values[i] + b.values[i];
    }
    return ret;
}

Matrix
mat_multiply(Matrix a, Matrix b) {
    Matrix ret = { 0 };
    for (unsigned int i = 0; i < 4; i++) {
        for (unsigned int j = 0; j < 4; j++) {
            for (unsigned int k = 0; k < 4; k++) {
                ret.values[4 * i + j] +=
                    a.values[4 * i + k] * b.values[4 * k + j];
            }
        }
    }
    return ret;
}

Matrix *
mat_scale(Matrix *mat, double factor) {
    for (unsigned int i = 0; i < 4 * 4; i++) {
        mat->values[i] *= factor;
    }
    return mat;
}

Matrix
mat_scaled(Matrix mat, double factor) {
    return *mat_scale(&mat, factor);
}

Matrix
mat_inverse(Matrix m, int *err) {
    double det;
    Matrix inv = {
        {
            m.values[5] * m.values[10] * m.values[15] -
                m.values[5] * m.values[11] * m.values[14] -
                m.values[9] * m.values[6] * m.values[15] +
                m.values[9] * m.values[7] * m.values[14] +
                m.values[13] * m.values[6] * m.values[11] -
                m.values[13] * m.values[7] * m.values[10],
            -m.values[1] * m.values[10] * m.values[15] +
                m.values[1] * m.values[11] * m.values[14] +
                m.values[9] * m.values[2] * m.values[15] -
                m.values[9] * m.values[3] * m.values[14] -
                m.values[13] * m.values[2] * m.values[11] +
                m.values[13] * m.values[3] * m.values[10],
            m.values[1] * m.values[6] * m.values[15] -
                m.values[1] * m.values[7] * m.values[14] -
                m.values[5] * m.values[2] * m.values[15] +
                m.values[5] * m.values[3] * m.values[14] +
                m.values[13] * m.values[2] * m.values[7] -
                m.values[13] * m.values[3] * m.values[6],
            -m.values[1] * m.values[6] * m.values[11] +
                m.values[1] * m.values[7] * m.values[10] +
                m.values[5] * m.values[2] * m.values[11] -
                m.values[5] * m.values[3] * m.values[10] -
                m.values[9] * m.values[2] * m.values[7] +
                m.values[9] * m.values[3] * m.values[6],
            -m.values[4] * m.values[10] * m.values[15] +
                m.values[4] * m.values[11] * m.values[14] +
                m.values[8] * m.values[6] * m.values[15] -
                m.values[8] * m.values[7] * m.values[14] -
                m.values[12] * m.values[6] * m.values[11] +
                m.values[12] * m.values[7] * m.values[10],
            m.values[0] * m.values[10] * m.values[15] -
                m.values[0] * m.values[11] * m.values[14] -
                m.values[8] * m.values[2] * m.values[15] +
                m.values[8] * m.values[3] * m.values[14] +
                m.values[12] * m.values[2] * m.values[11] -
                m.values[12] * m.values[3] * m.values[10],
            -m.values[0] * m.values[6] * m.values[15] +
                m.values[0] * m.values[7] * m.values[14] +
                m.values[4] * m.values[2] * m.values[15] -
                m.values[4] * m.values[3] * m.values[14] -
                m.values[12] * m.values[2] * m.values[7] +
                m.values[12] * m.values[3] * m.values[6],
            m.values[0] * m.values[6] * m.values[11] -
                m.values[0] * m.values[7] * m.values[10] -
                m.values[4] * m.values[2] * m.values[11] +
                m.values[4] * m.values[3] * m.values[10] +
                m.values[8] * m.values[2] * m.values[7] -
                m.values[8] * m.values[3] * m.values[6],
            m.values[4] * m.values[9] * m.values[15] -
                m.values[4] * m.values[11] * m.values[13] -
                m.values[8] * m.values[5] * m.values[15] +
                m.values[8] * m.values[7] * m.values[13] +
                m.values[12] * m.values[5] * m.values[11] -
                m.values[12] * m.values[7] * m.values[9],
            -m.values[0] * m.values[9] * m.values[15] +
                m.values[0] * m.values[11] * m.values[13] +
                m.values[8] * m.values[1] * m.values[15] -
                m.values[8] * m.values[3] * m.values[13] -
                m.values[12] * m.values[1] * m.values[11] +
                m.values[12] * m.values[3] * m.values[9],
            m.values[0] * m.values[5] * m.values[15] -
                m.values[0] * m.values[7] * m.values[13] -
                m.values[4] * m.values[1] * m.values[15] +
                m.values[4] * m.values[3] * m.values[13] +
                m.values[12] * m.values[1] * m.values[7] -
                m.values[12] * m.values[3] * m.values[5],
            -m.values[0] * m.values[5] * m.values[11] +
                m.values[0] * m.values[7] * m.values[9] +
                m.values[4] * m.values[1] * m.values[11] -
                m.values[4] * m.values[3] * m.values[9] -
                m.values[8] * m.values[1] * m.values[7] +
                m.values[8] * m.values[3] * m.values[5],
            -m.values[4] * m.values[9] * m.values[14] +
                m.values[4] * m.values[10] * m.values[13] +
                m.values[8] * m.values[5] * m.values[14] -
                m.values[8] * m.values[6] * m.values[13] -
                m.values[12] * m.values[5] * m.values[10] +
                m.values[12] * m.values[6] * m.values[9],
            m.values[0] * m.values[9] * m.values[14] -
                m.values[0] * m.values[10] * m.values[13] -
                m.values[8] * m.values[1] * m.values[14] +
                m.values[8] * m.values[2] * m.values[13] +
                m.values[12] * m.values[1] * m.values[10] -
                m.values[12] * m.values[2] * m.values[9],
            -m.values[0] * m.values[5] * m.values[14] +
                m.values[0] * m.values[6] * m.values[13] +
                m.values[4] * m.values[1] * m.values[14] -
                m.values[4] * m.values[2] * m.values[13] -
                m.values[12] * m.values[1] * m.values[6] +
                m.values[12] * m.values[2] * m.values[5],
            m.values[0] * m.values[5] * m.values[10] -
                m.values[0] * m.values[6] * m.values[9] -
                m.values[4] * m.values[1] * m.values[10] +
                m.values[4] * m.values[2] * m.values[9] +
                m.values[8] * m.values[1] * m.values[6] -
                m.values[8] * m.values[2] * m.values[5]
        }
    };

    det = m.values[0] * inv.values[0] + m.values[1] * inv.values[4] +
        m.values[2] * inv.values[8] + m.values[3] * inv.values[12];
    if (det == 0) {
        if (err) {
            *err = 1;
        }
        return (Matrix){ 0 };
    }
    det = 1.0 / det;
    mat_scale(&inv, det);
    return inv;
}
