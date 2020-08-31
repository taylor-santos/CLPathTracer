#include "matrix.h"

#define index(M, i) M.rows[i / 4].s[i % 4]

void
mat_set(Matrix *mat, unsigned int n, unsigned int m, vec_t value) {
    mat->rows[m].s[n] = value;
}

vec_t
mat_get(Matrix mat, unsigned int n, unsigned int m) {
    return mat.rows[m].s[n];
}

Matrix
mat_add(Matrix a, Matrix b) {
    Matrix ret = {0};
    for (int m = 0; m < 4; m++) {
        for (int n = 0; n < 4; n++) {
            ret.rows[m].s[n] = a.rows[m].s[n] + b.rows[m].s[n];
        }
    }
    return ret;
}

Matrix
mat_multiply(Matrix a, Matrix b) {
    Matrix ret = {0};
    for (unsigned int i = 0; i < 4; i++) {
        for (unsigned int j = 0; j < 4; j++) {
            for (unsigned int k = 0; k < 4; k++) {
                ret.rows[i].s[j] += a.rows[i].s[k] * b.rows[k].s[j];
            }
        }
    }
    return ret;
}

Matrix *
mat_scale(Matrix *mat, vec_t factor) {
    for (int m = 0; m < 4; m++) {
        for (int n = 0; n < 4; n++) { mat->rows[m].s[n] *= factor; }
    }
    return mat;
}

Matrix
mat_scaled(Matrix mat, vec_t factor) {
    return *mat_scale(&mat, factor);
}

Matrix
mat_inverse(Matrix m, int *err) {
    vec_t  det;
    Matrix inv = Matrix(
        // First Row
        index(m, 5) * index(m, 10) * index(m, 15) -
            index(m, 5) * index(m, 11) * index(m, 14) -
            index(m, 9) * index(m, 6) * index(m, 15) +
            index(m, 9) * index(m, 7) * index(m, 14) +
            index(m, 13) * index(m, 6) * index(m, 11) -
            index(m, 13) * index(m, 7) * index(m, 10),
        -index(m, 1) * index(m, 10) * index(m, 15) +
            index(m, 1) * index(m, 11) * index(m, 14) +
            index(m, 9) * index(m, 2) * index(m, 15) -
            index(m, 9) * index(m, 3) * index(m, 14) -
            index(m, 13) * index(m, 2) * index(m, 11) +
            index(m, 13) * index(m, 3) * index(m, 10),
        index(m, 1) * index(m, 6) * index(m, 15) -
            index(m, 1) * index(m, 7) * index(m, 14) -
            index(m, 5) * index(m, 2) * index(m, 15) +
            index(m, 5) * index(m, 3) * index(m, 14) +
            index(m, 13) * index(m, 2) * index(m, 7) -
            index(m, 13) * index(m, 3) * index(m, 6),
        -index(m, 1) * index(m, 6) * index(m, 11) +
            index(m, 1) * index(m, 7) * index(m, 10) +
            index(m, 5) * index(m, 2) * index(m, 11) -
            index(m, 5) * index(m, 3) * index(m, 10) -
            index(m, 9) * index(m, 2) * index(m, 7) +
            index(m, 9) * index(m, 3) * index(m, 6),
        // Second Row
        -index(m, 4) * index(m, 10) * index(m, 15) +
            index(m, 4) * index(m, 11) * index(m, 14) +
            index(m, 8) * index(m, 6) * index(m, 15) -
            index(m, 8) * index(m, 7) * index(m, 14) -
            index(m, 12) * index(m, 6) * index(m, 11) +
            index(m, 12) * index(m, 7) * index(m, 10),
        index(m, 0) * index(m, 10) * index(m, 15) -
            index(m, 0) * index(m, 11) * index(m, 14) -
            index(m, 8) * index(m, 2) * index(m, 15) +
            index(m, 8) * index(m, 3) * index(m, 14) +
            index(m, 12) * index(m, 2) * index(m, 11) -
            index(m, 12) * index(m, 3) * index(m, 10),
        -index(m, 0) * index(m, 6) * index(m, 15) +
            index(m, 0) * index(m, 7) * index(m, 14) +
            index(m, 4) * index(m, 2) * index(m, 15) -
            index(m, 4) * index(m, 3) * index(m, 14) -
            index(m, 12) * index(m, 2) * index(m, 7) +
            index(m, 12) * index(m, 3) * index(m, 6),
        index(m, 0) * index(m, 6) * index(m, 11) -
            index(m, 0) * index(m, 7) * index(m, 10) -
            index(m, 4) * index(m, 2) * index(m, 11) +
            index(m, 4) * index(m, 3) * index(m, 10) +
            index(m, 8) * index(m, 2) * index(m, 7) -
            index(m, 8) * index(m, 3) * index(m, 6),
        // Third Row
        index(m, 4) * index(m, 9) * index(m, 15) -
            index(m, 4) * index(m, 11) * index(m, 13) -
            index(m, 8) * index(m, 5) * index(m, 15) +
            index(m, 8) * index(m, 7) * index(m, 13) +
            index(m, 12) * index(m, 5) * index(m, 11) -
            index(m, 12) * index(m, 7) * index(m, 9),
        -index(m, 0) * index(m, 9) * index(m, 15) +
            index(m, 0) * index(m, 11) * index(m, 13) +
            index(m, 8) * index(m, 1) * index(m, 15) -
            index(m, 8) * index(m, 3) * index(m, 13) -
            index(m, 12) * index(m, 1) * index(m, 11) +
            index(m, 12) * index(m, 3) * index(m, 9),
        index(m, 0) * index(m, 5) * index(m, 15) -
            index(m, 0) * index(m, 7) * index(m, 13) -
            index(m, 4) * index(m, 1) * index(m, 15) +
            index(m, 4) * index(m, 3) * index(m, 13) +
            index(m, 12) * index(m, 1) * index(m, 7) -
            index(m, 12) * index(m, 3) * index(m, 5),
        -index(m, 0) * index(m, 5) * index(m, 11) +
            index(m, 0) * index(m, 7) * index(m, 9) +
            index(m, 4) * index(m, 1) * index(m, 11) -
            index(m, 4) * index(m, 3) * index(m, 9) -
            index(m, 8) * index(m, 1) * index(m, 7) +
            index(m, 8) * index(m, 3) * index(m, 5),
        // Fourth Row
        -index(m, 4) * index(m, 9) * index(m, 14) +
            index(m, 4) * index(m, 10) * index(m, 13) +
            index(m, 8) * index(m, 5) * index(m, 14) -
            index(m, 8) * index(m, 6) * index(m, 13) -
            index(m, 12) * index(m, 5) * index(m, 10) +
            index(m, 12) * index(m, 6) * index(m, 9),
        index(m, 0) * index(m, 9) * index(m, 14) -
            index(m, 0) * index(m, 10) * index(m, 13) -
            index(m, 8) * index(m, 1) * index(m, 14) +
            index(m, 8) * index(m, 2) * index(m, 13) +
            index(m, 12) * index(m, 1) * index(m, 10) -
            index(m, 12) * index(m, 2) * index(m, 9),
        -index(m, 0) * index(m, 5) * index(m, 14) +
            index(m, 0) * index(m, 6) * index(m, 13) +
            index(m, 4) * index(m, 1) * index(m, 14) -
            index(m, 4) * index(m, 2) * index(m, 13) -
            index(m, 12) * index(m, 1) * index(m, 6) +
            index(m, 12) * index(m, 2) * index(m, 5),
        index(m, 0) * index(m, 5) * index(m, 10) -
            index(m, 0) * index(m, 6) * index(m, 9) -
            index(m, 4) * index(m, 1) * index(m, 10) +
            index(m, 4) * index(m, 2) * index(m, 9) +
            index(m, 8) * index(m, 1) * index(m, 6) -
            index(m, 8) * index(m, 2) * index(m, 5));

    det = index(m, 0) * index(inv, 0) + index(m, 1) * index(inv, 4) +
          index(m, 2) * index(inv, 8) + index(m, 3) * index(inv, 12);
    if (det == 0) {
        if (err) { *err = 1; }
        return (Matrix){0};
    }
    det = 1 / det;
    mat_scale(&inv, det);
    return inv;
}
