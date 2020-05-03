#ifndef VECTOR3_H
#define VECTOR3_H

#include <CL/cl_gl.h>

#define vec_t float

#define CONCAT2(a, b, c) a ## b ## c
#define CONCAT(a, b, c) CONCAT2(a, b, c)
#define VEC(dim) CONCAT(cl_, vec_t, dim)

typedef VEC(3) Vector3;
typedef VEC(4) Vector4;

#define Vector3(x, y, z) (Vector3){ {x, y, z} }
#define Vector4(x, y, z, w) (Vector4){ {x, y, z, w} }
#define vec_x(v) ((v).s[0])
#define vec_y(v) ((v).s[1])
#define vec_z(v) ((v).s[2])

#define Vector3_zero Vector3(0, 0, 0)
#define Vector4_zero Vector4(0, 0, 0, 0)
#define Vector3_forward Vector3(0, 0, 1)
#define Vector3_up Vector3(0, 1, 0)

vec_t
vec_dot(Vector3, Vector3);
vec_t
vec_length_squared(Vector3);
vec_t
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
vec_scaled(Vector3, vec_t);
Vector3 *
vec_scale(Vector3 *, vec_t);
Vector3
vec_min(Vector3, Vector3);
Vector3
vec_max(Vector3, Vector3);

#endif//VECTOR3_H
