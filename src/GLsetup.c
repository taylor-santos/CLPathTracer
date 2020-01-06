#define WINDOW_NAME "OpenCL Path Tracer"

#include "GLsetup.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

struct GLData {
    GLFWwindow *window;
};

static void
error_callback(int error, const char *msg) {
    fprintf(stderr, "GLFW Error %d (via error_callback): %s\n", error, msg);
}

static void
init_glfw(void) {
    if (glfwInit() != GLFW_TRUE) {
        fprintf(stderr, "GLFW failed to initialize\n");
        exit(EXIT_FAILURE);
    }
}

static GLFWmonitor *
get_monitor(void) {
    GLFWmonitor **monitors;
    int monitor_count;

    monitors = glfwGetMonitors(&monitor_count);
    printf("There %s %d monitor%s available:\n", monitor_count == 1
                                                 ? "is"
                                                 : "are", monitor_count,
        monitor_count == 1
        ? ""
        : "s");
    for (int i = 0; i < monitor_count; i++) {
        const char *name = glfwGetMonitorName(monitors[i]);
        printf("\t%d) ", i + 1);
        if (i == 0) {
            printf("(Primary) ");
        }
        printf("%s ", name);
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[i]);
        printf("[%dx%d]\n", mode->width, mode->height);
    }
    if (monitor_count == 1) {
        return monitors[0];
    }
    long int index;
    do {
        printf("Select a monitor (1-%d):\n", monitor_count);
        printf("> ");
        char buf[64];
        fgets(buf, 64, stdin);
        index = strtol(buf, NULL, 10);
        if (index < 1 || index > (long)monitor_count) {
            printf("Invalid input. ");
        }
    } while (index < 1 || index > (long)monitor_count);
    return monitors[index - 1];
}

static GLFWwindow *
create_window(GLFWmonitor *monitor) {
    GLFWwindow *window;
    const GLFWvidmode *mode;

    mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    window = glfwCreateWindow(mode->width, mode->height, WINDOW_NAME, monitor,
        NULL);
    return window;
}

static void
delete_GLState(GLState *this) {
    glfwTerminate();
    free(this->data);
}

GLState
GLsetup(void) {
    GLFWmonitor *monitor;
    GLFWwindow *window;
    GLData *data;

    glfwSetErrorCallback(error_callback);
    init_glfw();
    monitor = get_monitor();
    window = create_window(monitor);
    glfwMakeContextCurrent(window);
    data = malloc(sizeof(*data));
    if (data == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *data = (GLData){
        window
    };

    return (GLState){
        data,
        delete_GLState
    };
}
