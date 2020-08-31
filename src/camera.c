#include <math.h>

#include "camera.h"

static Matrix
camera_transform(Camera cam) {
    Vector3 forward = cam.Forward;
    Vector3 left    = Vector3(vec_z(forward), 0, -vec_x(forward));
    vec_normalize(&left);
    Vector3 up  = vec_cross(forward, left);
    Vector3 pos = vec_negated(cam.Position);
    return Matrix(
        // 1st row
        vec_x(left),
        vec_y(left),
        vec_z(left),
        vec_dot(left, pos),
        // 2nd row
        vec_x(up),
        vec_y(up),
        vec_z(up),
        vec_dot(up, pos),
        // 3rd row
        vec_x(forward),
        vec_y(forward),
        vec_z(forward),
        vec_dot(forward, pos),
        // 4th row
        0,
        0,
        0,
        1);
}

static Matrix
projection_transform(Camera cam) {
    Matrix mat = {0};
    vec_t  c, near, far, fov;

    near = cam.Near;
    far  = cam.Far;
    fov  = cam.FOV;
    c    = 1 / (vec_t)tan((double)fov / 2);
    mat_set(&mat, 0, 0, c);
    mat_set(&mat, 1, 1, c);
    mat_set(&mat, 2, 2, -(far + near) / (near - far));
    mat_set(&mat, 3, 2, (2 * far * near) / (near - far));
    mat_set(&mat, 2, 3, 1);
    return mat;
}

static Matrix
device_transform(int height) {
    Matrix ret = {0};
    mat_set(&ret, 0, 0, (vec_t)height / 2);
    mat_set(&ret, 1, 1, (vec_t)height / 2);
    mat_set(&ret, 2, 2, 1);
    mat_set(&ret, 3, 3, 1);
    return ret;
}

Matrix
cam_matrix(Camera cam, int height) {
    Matrix device     = device_transform(height);
    Matrix projection = projection_transform(cam);
    Matrix camera     = camera_transform(cam);
    Matrix mat        = mat_multiply(device, projection);
    mat               = mat_multiply(mat, camera);
    return mat_inverse(mat, NULL);
}
