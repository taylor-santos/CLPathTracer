#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "CLState.h"
#include "GLHandler.h"

static struct {
    GLFWmonitor *monitor;
    GLFWwindow *window;
    GLuint shaderProgram, vao, texture;
    GLint texLoc;
    int width, height;
    GLFWkeyfun keyHandlers[GLFW_KEY_LAST + 1];
} State;

static void
resize_callback(GLFWwindow *wind, int new_width, int new_height) {
    State.width = new_width >= 1
        ? new_width
        : 1;
    State.height = new_height >= 1
        ? new_height
        : 1;
    State.texture = GLCreateTexture(State.width, State.height);
    CLDeleteImage();
    CLCreateImage(State.texture);
}

void
GLGetWindowPos(int *x, int *y) {
    glfwGetWindowPos(State.window, x, y);
}

void
GLGetWindowSize(int *x, int *y) {
    glfwGetWindowSize(State.window, x, y);
}

static void
key_callback(GLFWwindow *wind, int key, int scancode, int action, int mods) {
    if (State.keyHandlers[key] == NULL) {
        return;
    }
    State.keyHandlers[key](wind, key, scancode, action, mods);
}

static void
set_key_callback(GLFWwindow *wind) {
    glfwSetKeyCallback(wind, key_callback);
}

void
GLRegisterKey(int key, GLFWkeyfun function) {
    State.keyHandlers[key] = function;
}

void
GLRegisterMouseFunction(GLFWcursorposfun function) {
    glfwSetCursorPosCallback(State.window, function);
}

void
GLGetMousePos(double *x, double *y) {
    glfwGetCursorPos(State.window, x, y);
}

void
GLSetCameraMatrix(Matrix matrix) {
    CLSetCameraMatrix(matrix);
}

int
GLRender(void) {
    // Renders the next frame to screen, then returns 1 if the window is
    // still open, or 0 if the window has been closed.
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(State.shaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, State.texture);
    glUniform1i(State.texLoc, 0);
    glBindVertexArray(State.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGL Error: %d\n", error);
        exit(EXIT_FAILURE);
    }
    glfwSwapBuffers(State.window);
    CLExecute(State.width, State.height);
    return !glfwWindowShouldClose(State.window);
}

void
GLTerminate(void) {
    CLTerminate();
    glfwTerminate();
}

void
GLInit(const char *kernel_filename, const char *kernel_name) {
    GLInitGLFW();
    State.monitor = GLGetMonitor();
    State.window = GLCreateWindow(State.monitor);
    glfwMakeContextCurrent(State.window);
    glfwSetWindowSizeCallback(State.window, resize_callback);
    set_key_callback(State.window);
    GLInitGL3W();
    glfwGetFramebufferSize(State.window, &State.width, &State.height);

    glfwSetInputMode(State.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(State.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    State.shaderProgram = GLBuildShader();
    State.vao = GLSetupRender();
    State.texLoc = glGetUniformLocation(State.shaderProgram, "tex");
    State.texture = GLCreateTexture(State.width, State.height);
    CLInit(kernel_filename, kernel_name);
    CLCreateImage(State.texture);
}
