#include "camera.h"
#include "matrix.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <vector3.h>

struct CamData {
    double near, far, fov;
    Vector3 position, forward;
};

static void
set_position(const Camera *this, Vector3 position) {
    this->data->position = position;
}

static void
set_forward(const Camera *this, Vector3 forward) {
    this->data->forward = forward;
}

static Matrix
camera_transform(const Camera *this) {
    Vector3 forward = this->data->forward;
    Vector3 left = new_Vector3(forward.z, 0, -forward.x);
    left.normalize(&left);
    Vector3 up = forward.cross(&forward, &left);
    Vector3 pos = this->data->position.negate(&this->data->position);
    return new_Matrix((double[16]){
        left.x,
        left.y,
        left.z,
        left.dot(&left, &pos), // 1st row

        up.x,
        up.y,
        up.z,
        up.dot(&up, &pos), // 2nd row

        forward.x,
        forward.y,
        forward.z,
        forward.dot(&forward, &pos), // 3rd row

        0,
        0,
        0,
        1 // 4th row
    });
}

static Matrix
projection_transform(const Camera *this) {
    Matrix mat = new_Matrix(NULL);
    double c, near, far, fov;

    near = this->data->near;
    far = this->data->far;
    fov = this->data->fov;
    c = 1.0 / tan(fov / 2.0);
    mat.set(&mat, 0, 0, c);
    mat.set(&mat, 1, 1, c);
    mat.set(&mat, 2, 2, -(far + near) / (near - far));
    mat.set(&mat, 3, 2, (2.0 * far * near) / (near - far));
    mat.set(&mat, 2, 3, 1.0);
    return mat;
}

static Matrix
device_transform(const Camera *this, int width, int height) {
    Matrix ret = new_Matrix(NULL);
    ret.set(&ret, 0, 0, height / 2.0);
    ret.set(&ret, 1, 1, height / 2.0);
    ret.set(&ret, 2, 2, 1.0);
    ret.set(&ret, 3, 3, 1.0);
    return ret;
}

static void
delete(Camera *this) {
    free(this->data);
}

Camera
new_Camera(double near, double far, double fov) {
    CamData *data;

    data = malloc(sizeof(*data));
    if (data == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *data = (CamData){
        near,
        far,
        fov,
        new_Vector3(0, 0, 0),
        new_Vector3(0, 0, 1)
    };
    return (Camera){
        data,
        set_position,
        set_forward,
        camera_transform,
        projection_transform,
        device_transform,
        delete
    };
}
