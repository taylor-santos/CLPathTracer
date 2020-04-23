#include <stdlib.h>
#include <math.h>

#include "camera.h"

static Matrix
camera_transform(Camera cam) {
    Vector3 forward = cam.Forward;
    Vector3 left = {
        forward.z,
        0,
        -forward.x
    };
    vec_normalize(&left);
    Vector3 up = vec_cross(forward, left);
    Vector3 pos = vec_negated(cam.Position);
    return (Matrix){
        {
            left.x,
            left.y,
            left.z,
            vec_dot(left, pos), // 1st row

            up.x,
            up.y,
            up.z,
            vec_dot(up, pos), // 2nd row

            forward.x,
            forward.y,
            forward.z,
            vec_dot(forward, pos), // 3rd row

            0,
            0,
            0,
            1 // 4th row
        }
    };
}

static Matrix
projection_transform(Camera cam) {
    Matrix mat = { 0 };
    double c, near, far, fov;

    near = cam.Near;
    far = cam.Far;
    fov = cam.FOV;
    c = 1.0 / tan(fov / 2.0);
    mat_set(&mat, 0, 0, c);
    mat_set(&mat, 1, 1, c);
    mat_set(&mat, 2, 2, -(far + near) / (near - far));
    mat_set(&mat, 3, 2, (2.0 * far * near) / (near - far));
    mat_set(&mat, 2, 3, 1.0);
    return mat;
}

static Matrix
device_transform(int height) {
    Matrix ret = { 0 };
    mat_set(&ret, 0, 0, height / 2.0);
    mat_set(&ret, 1, 1, height / 2.0);
    mat_set(&ret, 2, 2, 1.0);
    mat_set(&ret, 3, 3, 1.0);
    return ret;
}

Matrix
cam_matrix(Camera cam, int height) {
    Matrix device = device_transform(height);
    Matrix projection = projection_transform(cam);
    Matrix camera = camera_transform(cam);
    Matrix mat = mat_multiply(device, projection);
    mat = mat_multiply(mat, camera);
    return mat_inverse(mat, NULL);
}
