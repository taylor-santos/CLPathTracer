#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "GLState.h"
#include "camera.h"
#include "physics.h"
#include "object.h"
#include "list.h"
#include "kd_tree.h"
#include "model.h"
#include "voxel.h"

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif
#define CLAMP(x, min, max) ((x) < (min) ? (min) : (x) > (max) ? (max) : (x))

static struct {
    struct {
        double x, y;
    } mouseSensitivity;
    double movementSpeed;
    double sprintModifier;
    double walkModifier;
} GameProperties = {{2, 2}, 20, 3, 0.3};
static struct {
    double time;
    struct {
        double x, y;
    } lookRotation;
    struct {
        double x, y;
    } mousePos;
    struct {
        int forward, left, back, right, up, down, sprint, walk;
    } moveKey;
    Camera  camera;
    Vector3 camVel;
} State;
static Object *vec_objects;
static kd *    vec_models;
static int     prevScreenPos[2], prevScreenSize[2];

double
update_time(void) {
    // Update the currTime value to the current wall time. Returns the time
    // difference from the previous call to update_time().
    double prev_time;

    prev_time  = State.time;
    State.time = glfwGetTime();
    return State.time - prev_time;
}

void
close_window(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) { return; }
    glfwSetWindowShouldClose(window, 1);
}

void
toggle_fullscreen(
    GLFWwindow *window,
    int         key,
    int         scancode,
    int         action,
    int         mods) {
    GLFWmonitor *      monitor;
    const GLFWvidmode *mode;

    if (action != GLFW_PRESS) { return; }
    if (glfwGetWindowMonitor(window)) {
        glfwSetWindowMonitor(
            window,
            NULL,
            prevScreenPos[0],
            prevScreenPos[1],
            prevScreenSize[0],
            prevScreenSize[1],
            GLFW_DONT_CARE);
        return;
    }
    monitor = glfwGetPrimaryMonitor();
    if (monitor == NULL) { return; }
    mode = glfwGetVideoMode(monitor);
    if (mode == NULL) { return; }
    GLGetWindowPos(&prevScreenPos[0], &prevScreenPos[1]);
    GLGetWindowSize(&prevScreenSize[0], &prevScreenSize[1]);
    glfwSetWindowMonitor(
        window,
        monitor,
        0,
        0,
        mode->width,
        mode->height,
        mode->refreshRate);
}

void
forward_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        State.moveKey.forward = 1;
    } else if (action == GLFW_RELEASE) {
        State.moveKey.forward = 0;
    }
}

void
right_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        State.moveKey.right = 1;
    } else if (action == GLFW_RELEASE) {
        State.moveKey.right = 0;
    }
}

void
back_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        State.moveKey.back = 1;
    } else if (action == GLFW_RELEASE) {
        State.moveKey.back = 0;
    }
}

void
left_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        State.moveKey.left = 1;
    } else if (action == GLFW_RELEASE) {
        State.moveKey.left = 0;
    }
}

void
up_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        State.moveKey.up = 1;
    } else if (action == GLFW_RELEASE) {
        State.moveKey.up = 0;
    }
}

void
down_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        State.moveKey.down = 1;
    } else if (action == GLFW_RELEASE) {
        State.moveKey.down = 0;
    }
}

void
sprint(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        State.moveKey.sprint = 1;
    } else if (action == GLFW_RELEASE) {
        State.moveKey.sprint = 0;
    }
}

void
walk(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        State.moveKey.walk = 1;
    } else if (action == GLFW_RELEASE) {
        State.moveKey.walk = 0;
    }
}

static void
change_fov(GLFWwindow *window, double xoffset, double yoffset) {
    double fov    = State.camera.FOV / M_PI;
    fov           = -fov / (fov - 1);
    double factor = pow(0.9, yoffset);
    GameProperties.mouseSensitivity.x *= factor;
    GameProperties.mouseSensitivity.y *= factor;
    fov *= factor;
    State.camera.FOV = M_PI * fov / (1 + fov);
}

void
GameTerminate(void) {
    GLTerminate();
    PhysTerminate();
    delete_list(vec_objects);
    delete_list(vec_models);
}

static void
mouse_handler(GLFWwindow *window, double x, double y) {
    double dx, dy;

    dx               = State.mousePos.x - x;
    dy               = State.mousePos.y - y;
    State.mousePos.x = x;
    State.mousePos.y = y;
    dx /= 1000.0;
    dy /= 1000.0;
    State.lookRotation.x += dx * GameProperties.mouseSensitivity.x;
    State.lookRotation.y += dy * GameProperties.mouseSensitivity.y;

    State.lookRotation.y = CLAMP(State.lookRotation.y, -M_PI / 2, M_PI / 2);
    State.lookRotation.x =
        fmod(fmod(State.lookRotation.x, 2 * M_PI) + 2 * M_PI, 2 * M_PI);

    State.camera.Forward = Vector3(
        -cos(State.lookRotation.y) * sin(State.lookRotation.x),
        sin(State.lookRotation.y),
        cos(State.lookRotation.x) * cos(State.lookRotation.y));
}

static void
update_camera(void) {
    Matrix matrix;
    int    height;

    GLGetWindowSize(NULL, &height);
    matrix = cam_matrix(State.camera, height);
    GLSetCameraMatrix(matrix);
}

static void
update_objects(void) {
    GLSetObjects(vec_objects, list_size(vec_objects));
}

void
StartGameLoop(void) {
    vec_t   speed;
    Vector3 up, right, forward;
    FILE *  save;

    // if ((save = fopen("save", "rb")) != NULL) {
    //    fread(&State, sizeof(State), 1, save);
    //    fclose(save);
    //}
    while (GLRender()) {
        speed = GameProperties.movementSpeed;
        if (State.moveKey.sprint) { speed *= GameProperties.sprintModifier; }
        if (State.moveKey.walk) { speed *= GameProperties.walkModifier; }
        up    = Vector3_up;
        right = vec_cross(up, State.camera.Forward);
        vec_normalize(&right);
        forward = State.camera.Forward;
        vec_normalize(&forward);
        vec_scale(&right, State.moveKey.right - State.moveKey.left);
        vec_scale(&forward, State.moveKey.forward - State.moveKey.back);
        vec_scale(&up, State.moveKey.up - State.moveKey.down);
        State.camVel = vec_scaled(vec_add(vec_add(right, forward), up), speed);
        update_camera();
        update_objects();

        PhysStep(update_time());
    }
    if ((save = fopen("save", "wb")) == NULL) {
        perror("save");
        exit(EXIT_FAILURE);
    }
    fwrite(&State, sizeof(State), 1, save);
    fclose(save);
}

void
GameInit(const char *kernel_filename, const char *kernel_name, CLArg *args) {
    GLInit(kernel_filename, kernel_name, args);
    GLRegisterKey(GLFW_KEY_ESCAPE, close_window);
    GLRegisterKey(GLFW_KEY_F, toggle_fullscreen);
    GLRegisterKey(GLFW_KEY_W, forward_key);
    GLRegisterKey(GLFW_KEY_D, right_key);
    GLRegisterKey(GLFW_KEY_S, back_key);
    GLRegisterKey(GLFW_KEY_A, left_key);
    GLRegisterKey(GLFW_KEY_E, up_key);
    GLRegisterKey(GLFW_KEY_Q, down_key);
    GLRegisterKey(GLFW_KEY_LEFT_SHIFT, sprint);
    GLRegisterKey(GLFW_KEY_LEFT_CONTROL, walk);
    GLRegisterScroll(change_fov);
    GLRegisterMouseFunction(mouse_handler);
    GLGetWindowPos(&prevScreenPos[0], &prevScreenPos[1]);
    GLGetWindowSize(&prevScreenSize[0], &prevScreenSize[1]);
    GLGetMousePos(&State.mousePos.x, &State.mousePos.y);
    State.camVel = Vector3_zero;
    State.camera =
        (Camera){0.1f, 1, M_PI / 3, Vector3(0, 0, -2), Vector3_forward};
    AddPhysObject(&State.camera.Position, &State.camVel);
    vec_objects = new_list(0);
}
