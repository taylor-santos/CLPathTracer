#ifndef VECTOR3_H
#define VECTOR3_H

#include <CL/cl_gl.h>

typedef cl_float4 Vector3;
#define Vector3(x, y, z) (Vector3){ {x, y, z, 0} }
#define vec_x(v) (v).s[0]
#define vec_y(v) (v).s[1]
#define vec_z(v) (v).s[2]

#define Vector3_zero Vector3(0, 0, 0)
#define Vector3_forward Vector3(0, 0, 1)
#define Vector3_up Vector3(0, 1, 0)

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
