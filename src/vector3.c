#include "vector3.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static double
dot(const Vector3 *this, const Vector3 *other) {
    return this->x * other->x + this->y * other->y + this->z * other->z;
}

static double
sqrMagnitue(const Vector3 *this) {
    return this->x * this->x + this->y * this->y + this->z * this->z;
}

static double
magnitude(const Vector3 *this) {
    return sqrt(sqrMagnitue(this));
}

static Vector3
normalized(const Vector3 *this) {
    double mag = magnitude(this);
    return new_Vector3(this->x / mag, this->y / mag, this->z / mag);
}

static Vector3
normalize(Vector3 *this) {
    double mag = magnitude(this);
    this->x /= mag;
    this->y /= mag;
    this->z /= mag;
    return *this;
}

static Vector3
plus(const Vector3 *this, const Vector3 *other) {
    return new_Vector3(this->x + other->x,
        this->y + other->y,
        this->z + other->z);
}

static Vector3
minus(const Vector3 *this, const Vector3 *other) {
    return new_Vector3(this->x - other->x,
        this->y - other->y,
        this->z - other->z);
}

static Vector3
cross(const Vector3 *this, const Vector3 *other) {
    return new_Vector3(this->y * other->z - this->z * other->y,
        this->z * other->x - this->x * other->z,
        this->x * other->y - this->y * other->x);
}

static Vector3
negated(const Vector3 *this) {
    return new_Vector3(-this->x, -this->y, -this->z);
}

static Vector3
negate(Vector3 *this) {
    this->x = -this->x;
    this->y = -this->y;
    this->z = -this->z;
    return *this;
}

static Vector3
scaled(const Vector3 *this, double factor) {
    return new_Vector3(this->x * factor, this->y * factor, this->z * factor);
}


static Vector3
scale(Vector3 *this, double factor) {
    this->x *= factor;
    this->y *= factor;
    this->z *= factor;
    return *this;
}

Vector3
new_Vector3(double x, double y, double z) {
    return (Vector3){
        x,
        y,
        z,
        dot,
        sqrMagnitue,
        magnitude,
        normalized,
        normalize,
        plus,
        minus,
        cross,
        negated,
        negate,
        scaled,
        scale
    };
}
