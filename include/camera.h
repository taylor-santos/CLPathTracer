#ifndef CAMERA_H
#define CAMERA_H

#include "matrix.h"
#include "vector3.h"

typedef struct Camera Camera;
typedef struct CamData CamData;

struct Camera {
    CamData *data;
    void (*set_position)(const Camera *this, Vector3 pos);
    void (*set_forward)(const Camera *this, Vector3 forward);
    Matrix (*camera_transform)(const Camera *this);
    Matrix (*projection_transform)(const Camera *this);
    Matrix (*device_transform)(const Camera *this, int width, int height);
    void (*delete)(Camera *this);
};

Camera
new_Camera(double near, double far, double fov);

#endif//CAMERA_H
