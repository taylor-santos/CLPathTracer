#ifndef VECTOR3_H
#define VECTOR3_H

typedef struct Vector3 Vector3;

struct Vector3 {
    double x, y, z;
    double (*dot)(const Vector3 *this, const Vector3 *other);
    double (*sqrMagnitue)(const Vector3 *this);
    double (*magnitude)(const Vector3 *this);
    Vector3 (*normalized)(const Vector3 *this);
    Vector3 (*normalize)(Vector3 *this);
    Vector3 (*plus)(const Vector3 *this, const Vector3 *other);
    Vector3 (*minus)(const Vector3 *this, const Vector3 *other);
    Vector3 (*cross)(const Vector3 *this, const Vector3 *other);
    Vector3 (*negated)(const Vector3 *this);
    Vector3 (*negate)(Vector3 *this);
    Vector3 (*scaled)(const Vector3 *this, double factor);
    Vector3 (*scale)(Vector3 *this, double factor);
};

Vector3
new_Vector3(double x, double y, double z);

#endif//VECTOR3_H
