#ifndef CAMERA_H
#define CAMERA_H

#include "matrix.h"
#include "vector3.h"

typedef struct Camera {
    double Near;
    double Far;
    double FOV;
    Vector3 Position;
    Vector3 Forward;
} Camera;

Matrix
cam_matrix(Camera, int height);

#endif//CAMERA_H
