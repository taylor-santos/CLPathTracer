#ifndef CAMERA_H
#define CAMERA_H

#include "matrix.h"

typedef struct Camera {
    vec_t Near;
    vec_t Far;
    vec_t FOV;
    Vector3 Position;
    Vector3 Forward;
} Camera;

Matrix
cam_matrix(Camera, int height);

#endif//CAMERA_H
