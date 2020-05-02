#ifndef CAMERA_H
#define CAMERA_H

#include "matrix.h"

typedef struct Camera {
    VEC_TYPE Near;
    VEC_TYPE Far;
    VEC_TYPE FOV;
    Vector3 Position;
    Vector3 Forward;
} Camera;

Matrix
cam_matrix(Camera, int height);

#endif//CAMERA_H
