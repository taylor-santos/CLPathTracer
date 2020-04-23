#include "vector3.h"
#include <math.h>

const Vector3 Vector3_zero = {
    0,
    0,
    0
};
const Vector3 Vector3_up = {
    0,
    1,
    0
};
const Vector3 Vector3_forward = {
    0,
    0,
    1
};

double
vec_dot(Vector3 a, Vector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double
vec_length_squared(Vector3 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

double
vec_length(Vector3 v) {
    return sqrt(vec_length_squared(v));
}

Vector3 *
vec_normalize(Vector3 *v) {
    double length = vec_length(*v);
    v->x /= length;
    v->y /= length;
    v->z /= length;
    return v;
}

Vector3
vec_normalized(Vector3 v) {
    return *vec_normalize(&v);
}

Vector3
vec_add(Vector3 a, Vector3 b) {
    return (Vector3){
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    };
}

Vector3
vec_subtract(Vector3 a, Vector3 b) {
    return (Vector3){
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };
}

Vector3
vec_cross(Vector3 a, Vector3 b) {
    return (Vector3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

Vector3 *
vec_negate(Vector3 *v) {
    v->x = -v->x;
    v->y = -v->y;
    v->z = -v->z;
    return v;
}

Vector3
vec_negated(Vector3 v) {
    return *vec_negate(&v);
}

Vector3 *
vec_scale(Vector3 *v, double factor) {
    v->x *= factor;
    v->y *= factor;
    v->z *= factor;
    return v;
}

Vector3
vec_scaled(Vector3 v, double factor) {
    return *vec_scale(&v, factor);
}
