#ifndef VECTOR3_H
#define VECTOR3_H

typedef struct Vector3 Vector3;

struct Vector3 {
    double x, y, z;
};

extern const Vector3 Vector3_zero, Vector3_up, Vector3_forward;

double
vec_dot(Vector3, Vector3);
double
vec_length_squared(Vector3);
double
vec_length(Vector3);
Vector3
vec_normalized(Vector3);
Vector3 *
vec_normalize(Vector3 *);
Vector3
vec_add(Vector3, Vector3);
Vector3
vec_subtract(Vector3, Vector3);
Vector3
vec_cross(Vector3, Vector3);
Vector3
vec_negated(Vector3);
Vector3 *
vec_negate(Vector3 *);
Vector3
vec_scaled(Vector3, double);
Vector3 *
vec_scale(Vector3 *, double);

#endif//VECTOR3_H
