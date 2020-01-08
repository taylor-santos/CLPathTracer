#include "GameInit.h"
#include "GLInit.h"

static int prevPos[2], prevSize[2];

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
            prevPos[0],
            prevPos[1],
            prevSize[0],
            prevSize[1],
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
    GLGetWindowPos(&prevPos[0], &prevPos[1]);
    GLGetWindowSize(&prevSize[0], &prevSize[1]);
    glfwSetWindowMonitor(window,
        monitor,
        0,
        0,
        mode->width,
        mode->height,
        mode->refreshRate);
}

void
GameInit(void) {
    GLRegisterKey(GLFW_KEY_ESCAPE, close_window);
    GLRegisterKey(GLFW_KEY_F, toggle_fullscreen);
    GLGetWindowPos(&prevPos[0], &prevPos[1]);
    GLGetWindowSize(&prevSize[0], &prevSize[1]);
}

