#include <math.h>

#include "vector.h"

vec_t
vec_dot(Vector3 a, Vector3 b) {
    return vec_x(a) * vec_x(b) + vec_y(a) * vec_y(b) + vec_z(a) * vec_z(b);
}

vec_t
vec_length_squared(Vector3 v) {
    return vec_x(v) * vec_x(v) + vec_y(v) * vec_y(v) + vec_z(v) * vec_z(v);
}

vec_t
vec_length(Vector3 v) {
    return (vec_t)sqrt((double)vec_length_squared(v));
}

Vector3 *
vec_normalize(Vector3 *pv) {
    vec_t length = vec_length(*pv);
    vec_x(*pv) /= length;
    vec_y(*pv) /= length;
    vec_z(*pv) /= length;
    return pv;
}

Vector3
vec_normalized(Vector3 v) {
    return *vec_normalize(&v);
}

Vector3
vec_add(Vector3 a, Vector3 b) {
    return Vector3(
        vec_x(a) + vec_x(b),
        vec_y(a) + vec_y(b),
        vec_z(a) + vec_z(b));
}

Vector3
vec_subtract(Vector3 a, Vector3 b) {
    return Vector3(
        vec_x(a) - vec_x(b),
        vec_y(a) - vec_y(b),
        vec_z(a) - vec_z(b));
}

Vector3
vec_cross(Vector3 a, Vector3 b) {
    vec_t x = vec_y(a) * vec_z(b) - vec_z(a) * vec_y(b);
    vec_t y = vec_z(a) * vec_x(b) - vec_x(a) * vec_z(b);
    vec_t z = vec_x(a) * vec_y(b) - vec_y(a) * vec_x(b);
    return Vector3(x, y, z);
}

Vector3 *
vec_negate(Vector3 *pv) {
    vec_x(*pv) = -vec_x(*pv);
    vec_y(*pv) = -vec_y(*pv);
    vec_z(*pv) = -vec_z(*pv);
    return pv;
}

Vector3
vec_negated(Vector3 v) {
    return *vec_negate(&v);
}

Vector3 *
vec_scale(Vector3 *pv, vec_t factor) {
    vec_x(*pv) *= factor;
    vec_y(*pv) *= factor;
    vec_z(*pv) *= factor;
    return pv;
}

Vector3
vec_scaled(Vector3 v, vec_t factor) {
    return *vec_scale(&v, factor);
}

Vector3
vec_divide(Vector3 a, Vector3 b) {
    return Vector3(
        vec_x(a) / vec_x(b),
        vec_y(a) / vec_y(b),
        vec_z(a) / vec_z(b));
}

Vector3
vec_min(Vector3 a, Vector3 b) {
    return Vector3(
        vec_x(a) < vec_x(b) ? vec_x(a) : vec_x(b),
        vec_y(a) < vec_y(b) ? vec_y(a) : vec_y(b),
        vec_z(a) < vec_z(b) ? vec_z(a) : vec_z(b));
}

Vector3
vec_max(Vector3 a, Vector3 b) {
    return Vector3(
        vec_x(a) > vec_x(b) ? vec_x(a) : vec_x(b),
        vec_y(a) > vec_y(b) ? vec_y(a) : vec_y(b),
        vec_z(a) > vec_z(b) ? vec_z(a) : vec_z(b));
}
