#ifndef CAMERA_H
#define CAMERA_H

#include "matrix.h"
#include "vector3.h"

typedef struct Camera Camera;
typedef struct CamData CamData;

struct Camera {
    CamData *data;
    Vector3 position;
    Vector3 forward;
    Matrix (*GetMatrix)(const Camera *this, int height);
    void (*delete)(Camera *this);
};

Camera
new_Camera(double near, double far, double fov);

#endif//CAMERA_H
