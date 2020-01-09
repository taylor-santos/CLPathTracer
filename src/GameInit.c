#include "GameInit.h"
#include "GLInit.h"
#include "CLInit.h"
#include "vector3.h"
#include "camera.h"
#include <stdio.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define CLAMP(x, min, max) ((x) < (min) ? (min) : (x) > (max) ? (max) : (x))

gameprops GameProperties = {
    {
        2,
        2
    },
    5,
    50
};

static double currTime;
static int prevScreenPos[2], prevScreenSize[2];
static GL GLState;
static Camera camera;

static Vector3 position;
static Vector3 viewDir;
static Vector3 velocity;

static struct {
    double x, y;
} lookRotation;
static struct {
    double x, y;
} prevMousePos;
static struct {
    int forward, left, back, right, sprint;
} moveKeyState;

double
update_time(void) {
    // Update the currTime value to the current wall time. Returns the time
    // difference from the previous call to update_time().
    double prev_time;

    prev_time = currTime;
    currTime = glfwGetTime();
    return currTime - prev_time;
}

void
close_window(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }
    glfwSetWindowShouldClose(window, 1);
}

void
toggle_fullscreen(GLFWwindow *window, int key, int scancode, int action,
    int mods) {
    GLFWmonitor *monitor;
    const GLFWvidmode *mode;

    if (action != GLFW_PRESS) {
        return;
    }
    if (glfwGetWindowMonitor(window)) {
        glfwSetWindowMonitor(window,
            NULL,
            prevScreenPos[0],
            prevScreenPos[1],
            prevScreenSize[0],
            prevScreenSize[1],
            GLFW_DONT_CARE);
        return;
    }
    monitor = glfwGetPrimaryMonitor();
    if (monitor == NULL) {
        return;
    }
    mode = glfwGetVideoMode(monitor);
    if (mode == NULL) {
        return;
    }
    GLState.GetWindowPos(&GLState, &prevScreenPos[0], &prevScreenPos[1]);
    GLState.GetWindowSize(&GLState, &prevScreenSize[0], &prevScreenSize[1]);
    glfwSetWindowMonitor(window,
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
        moveKeyState.forward = 1;
    } else if (action == GLFW_RELEASE) {
        moveKeyState.forward = 0;
    }
}

void
right_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        moveKeyState.right = 1;
    } else if (action == GLFW_RELEASE) {
        moveKeyState.right = 0;
    }
}

void
back_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        moveKeyState.back = 1;
    } else if (action == GLFW_RELEASE) {
        moveKeyState.back = 0;
    }
}

void
left_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        moveKeyState.left = 1;
    } else if (action == GLFW_RELEASE) {
        moveKeyState.left = 0;
    }
}

void
sprint(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        moveKeyState.sprint = 1;
    } else if (action == GLFW_RELEASE) {
        moveKeyState.sprint = 0;
    }
}

void
GameTerminate(void) {
    GLState.Terminate(&GLState);
    camera.delete(&camera);
}

static void
step_physics(void) {
    double dT, speed;
    Vector3 disp, up, right, forward;

    dT = update_time();
    speed = moveKeyState.sprint
            ? GameProperties.sprintSpeed
            : GameProperties.movementSpeed;
    disp = velocity.scaled(&velocity, dT * speed);
    position = position.plus(&position, &disp);
    up = new_Vector3(0, 1, 0);
    right = up.cross(&up, &viewDir);
    right.normalize(&right);
    //forward = right.cross(&right, &up);
    forward = viewDir;
    forward.normalize(&forward);
    right.scale(&right, moveKeyState.right - moveKeyState.left);
    forward.scale(&forward, moveKeyState.forward - moveKeyState.back);
    velocity = right.plus(&right, &forward);
}

static void
mouse_handler(GLFWwindow *window, double x, double y) {
    double dx, dy;

    dx = prevMousePos.x - x;
    dy = prevMousePos.y - y;
    prevMousePos.x = x;
    prevMousePos.y = y;
    dx /= 1000.0;
    dy /= 1000.0;
    lookRotation.x += dx * GameProperties.mouseSensitivity.x;
    lookRotation.y += dy * GameProperties.mouseSensitivity.y;

    lookRotation.y = CLAMP(lookRotation.y, -M_PI / 2, M_PI / 2);
    lookRotation.x = fmod(fmod(lookRotation.x, 2 * M_PI) + 2 * M_PI, 2 * M_PI);

    viewDir = new_Vector3(-cos(lookRotation.y) * sin(lookRotation.x),
        sin(lookRotation.y),
        cos(lookRotation.x) * cos(lookRotation.y));
}

static void
update_camera(void) {
    Matrix matrix;
    cl_float4 arr[4];
    int height;

    camera.position = position;
    camera.forward = viewDir;
    GLState.GetWindowSize(&GLState, NULL, &height);
    matrix = camera.GetMatrix(&camera, height);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            arr[y].s[x] = matrix.values[4 * y + x];
        }
    }
    GLState.SetCameraMatrix(&GLState, arr);
}

void
StartGameLoop(void) {
    while (GLState.Render(&GLState)) {
        step_physics();
        update_camera();
    }
}

void
GameInit(const char *kernel_filename, const char *kernel_name) {
    GLState = GLInit(kernel_filename, kernel_name);
    GLState.RegisterKey(&GLState, GLFW_KEY_ESCAPE, close_window);
    GLState.RegisterKey(&GLState, GLFW_KEY_F, toggle_fullscreen);
    GLState.RegisterKey(&GLState, GLFW_KEY_W, forward_key);
    GLState.RegisterKey(&GLState, GLFW_KEY_D, right_key);
    GLState.RegisterKey(&GLState, GLFW_KEY_S, back_key);
    GLState.RegisterKey(&GLState, GLFW_KEY_A, left_key);
    GLState.RegisterKey(&GLState, GLFW_KEY_LEFT_SHIFT, sprint);
    GLState.RegisterMouseFunction(&GLState, mouse_handler);
    GLState.GetWindowPos(&GLState, &prevScreenPos[0], &prevScreenPos[1]);
    GLState.GetWindowSize(&GLState, &prevScreenSize[0], &prevScreenSize[1]);
    GLState.GetMousePos(&GLState, &prevMousePos.x, &prevMousePos.y);
    position = new_Vector3(0, 0, 0);
    viewDir = new_Vector3(0, 0, 1);
    velocity = new_Vector3(0, 0, 0);
    camera = new_Camera(0.1, 1, M_PI / 3);
}
