#include <math.h>

#include "vector3.h"

#define vec_x(v) (v).s[0]
#define vec_y(v) (v).s[1]
#define vec_z(v) (v).s[2]

VEC_TYPE
vec_dot(Vector3 a, Vector3 b) {
    return vec_x(a) * vec_x(b) + vec_y(a) * vec_y(b) + vec_z(a) * vec_z(b);
}

VEC_TYPE
vec_length_squared(Vector3 v) {
    return vec_x(v) * vec_x(v) + vec_y(v) * vec_y(v) + vec_z(v) * vec_z(v);
}

VEC_TYPE
vec_length(Vector3 v) {
    return (VEC_TYPE)sqrt((double)vec_length_squared(v));
}

Vector3 *
vec_normalize(Vector3 *v) {
    VEC_TYPE length = vec_length(*v);
    vec_x(*v) /= length;
    vec_y(*v) /= length;
    vec_z(*v) /= length;
    return v;
}

Vector3
vec_normalized(Vector3 v) {
    return *vec_normalize(&v);
}

Vector3
vec_add(Vector3 a, Vector3 b) {
    return Vector3(vec_x(a) + vec_x(b),
        vec_y(a) + vec_y(b),
        vec_z(a) + vec_z(b));
}

Vector3
vec_subtract(Vector3 a, Vector3 b) {
    return Vector3(vec_x(a) - vec_x(b),
        vec_y(a) - vec_y(b),
        vec_z(a) - vec_z(b));
}

Vector3
vec_cross(Vector3 a, Vector3 b) {
    VEC_TYPE x = vec_y(a) * vec_z(b) - vec_z(a) * vec_y(b);
    VEC_TYPE y = vec_z(a) * vec_x(b) - vec_x(a) * vec_z(b);
    VEC_TYPE z = vec_x(a) * vec_y(b) - vec_y(a) * vec_x(b);
    return Vector3(x, y, z);
}

Vector3 *
vec_negate(Vector3 *v) {
    vec_x(*v) = -vec_x(*v);
    vec_y(*v) = -vec_y(*v);
    vec_z(*v) = -vec_z(*v);
    return v;
}

Vector3
vec_negated(Vector3 v) {
    return *vec_negate(&v);
}

Vector3 *
vec_scale(Vector3 *v, VEC_TYPE factor) {
    vec_x(*v) *= factor;
    vec_y(*v) *= factor;
    vec_z(*v) *= factor;
    return v;
}

Vector3
vec_scaled(Vector3 v, VEC_TYPE factor) {
    return *vec_scale(&v, factor);
}
