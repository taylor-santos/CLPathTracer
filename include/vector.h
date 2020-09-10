#ifndef VECTOR_H
#define VECTOR_H

#include <CL/cl_gl.h>

#define vec_t double

#define CONCAT2(a, b, c) a##b##c
#define CONCAT(a, b, c)  CONCAT2(a, b, c)
#define VEC(dim)         CONCAT(cl_, vec_t, dim)

typedef VEC(3) Vector3;
typedef VEC(4) Vector4;

#define Vector3(x, y, z) \
    (Vector3) {          \
        { x, y, z }      \
    }
#define Vector4(x, y, z, w) \
    (Vector4) {             \
        { x, y, z, w }      \
    }
#define vec_x(v) ((v).s[0])
#define vec_y(v) ((v).s[1])
#define vec_z(v) ((v).s[2])

#define Vector3_zero    Vector3(0, 0, 0)
#define Vector3_forward Vector3(0, 0, 1)
#define Vector3_up      Vector3(0, 1, 0)

vec_t
vec_dot(Vector3 a, Vector3 b);
vec_t
vec_length_squared(Vector3 v);
vec_t
vec_length(Vector3 v);
Vector3
vec_normalized(Vector3 v);
Vector3 *
vec_normalize(Vector3 *pv);
Vector3
vec_add(Vector3 a, Vector3 b);
Vector3
vec_subtract(Vector3 a, Vector3 b);
Vector3
vec_cross(Vector3 a, Vector3 b);
Vector3
vec_negated(Vector3 v);
Vector3 *
vec_negate(Vector3 *pv);
Vector3
vec_scaled(Vector3 v, vec_t factor);
Vector3 *
vec_scale(Vector3 *pv, vec_t factor);
Vector3
vec_divide(Vector3 a, Vector3 b);
Vector3
vec_min(Vector3 a, Vector3 b);
Vector3
vec_max(Vector3 a, Vector3 b);

#endif // VECTOR_H
