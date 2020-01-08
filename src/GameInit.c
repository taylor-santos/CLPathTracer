#include "GameInit.h"
#include "GLInit.h"
#include "CLInit.h"

static int prevPos[2], prevSize[2];
static GL GLState;

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
    GLState.GetWindowPos(&GLState, &prevPos[0], &prevPos[1]);
    GLState.GetWindowSize(&GLState, &prevSize[0], &prevSize[1]);
    glfwSetWindowMonitor(window,
        monitor,
        0,
        0,
        mode->width,
        mode->height,
        mode->refreshRate);
}

void
GameInit(const char *kernel_filename, const char *kernel_name) {
    GLState = GLInit(kernel_filename, kernel_name);
    GLState.RegisterKey(&GLState, GLFW_KEY_ESCAPE, close_window);
    GLState.RegisterKey(&GLState, GLFW_KEY_F, toggle_fullscreen);
    GLState.GetWindowPos(&GLState, &prevPos[0], &prevPos[1]);
    GLState.GetWindowSize(&GLState, &prevSize[0], &prevSize[1]);
}

void
GameTerminate(void) {
    GLState.Terminate(&GLState);
}

void
StartGameLoop(void) {
    GLState.Render(&GLState);
}

