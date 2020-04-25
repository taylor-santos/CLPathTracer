#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GLHandler.h"

#define WINDOW_NAME "OpenCL Path Tracer"
#define VSYNC 1

static void
error_callback(int error, const char *msg) {
    fprintf(stderr, "GLFW Error %d (via error_callback): %s\n", error, msg);
}

void
GLInitGLFW(void) {
    glfwSetErrorCallback(error_callback);
    if (glfwInit() != GLFW_TRUE) {
        fprintf(stderr, "GLFW failed to initialize\n");
        exit(EXIT_FAILURE);
    }
}

GLFWmonitor *
GLGetMonitor(void) {
    GLFWmonitor **monitors;
    int monitor_count;

    monitors = glfwGetMonitors(&monitor_count);
    printf("There %s %d monitor%s available:\n",
        monitor_count == 1
            ? "is"
            : "are",
        monitor_count,
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
    return monitors[0];
}

GLFWwindow *
GLCreateWindow(GLFWmonitor *monitor) {
    const GLFWvidmode *mode;

    mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    return glfwCreateWindow(mode->width / 2,
        mode->height / 2,
        WINDOW_NAME,
        NULL,
        NULL);
}

void
GLInitGL3W(void) {
    if (gl3wInit()) {
        fprintf(stderr, "OpenGL failed to initialize\n");
        exit(EXIT_FAILURE);
    }
}

static GLuint
compile_shader(const char *source, GLenum type) {
    GLuint shader;
    GLint len;

    len = strlen(source);
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, &len);
    glCompileShader(shader);
    return shader;
}

GLuint
GLBuildShader(void) {
    GLuint shader, vertex, fragment;

    const char *vertex_source = "#version 330\n"
                                "layout(location = 0) in vec4 vposition;\n"
                                "layout(location = 1) in vec2 vtexcoord;\n"
                                "out vec2 ftexcoord;\n"
                                "void main() {\n"
                                "    ftexcoord = vtexcoord;\n"
                                "    gl_Position = vposition;\n"
                                "}\n";

    const char *fragment_source = "#version 330\n"
                                  "uniform sampler2D tex;\n"
                                  "in vec2 ftexcoord;\n"
                                  "layout(location = 0) out vec4 fcolor;\n"
                                  "void main() {\n"
                                  "    fcolor = texture(tex, ftexcoord);\n"
                                  "}\n";
    vertex = compile_shader(vertex_source, GL_VERTEX_SHADER);
    fragment = compile_shader(fragment_source, GL_FRAGMENT_SHADER);
    shader = glCreateProgram();
    glAttachShader(shader, vertex);
    glAttachShader(shader, fragment);
    glLinkProgram(shader);

    return shader;
}

GLuint
GLSetupRender(void) {
    GLuint vao, vbo, ibo;

    glfwSwapInterval(VSYNC);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    GLfloat vertexData[] = { // {x,y,z,u,v}
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f, // vertex 0
        -1.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f, // vertex 1
        1.0f,
        -1.0f,
        0.0f,
        1.0f,
        0.0f, // vertex 2
        -1.0f,
        -1.0f,
        0.0f,
        0.0f,
        0.0f, // vertex 3
    };
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(vertexData),
        vertexData,
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
        3,
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(GLfloat),
        (char *)(0 + 0 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
        2,
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(GLfloat),
        (char *)(0 + 3 * sizeof(GLfloat)));
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    GLuint indexData[] = {
        0,
        1,
        2, // triangle 0
        2,
        1,
        3 // triangle 1
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        sizeof(indexData),
        indexData,
        GL_STATIC_DRAW);
    glBindVertexArray(0);
    return vao;
}

void
GLResizeTexture(GLuint *texture, int width, int height) {
    glDeleteTextures(1, texture);
    glViewport(0, 0, width, height);

    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        NULL);
}

GLuint
GLCreateTexture(int width, int height) {
    GLuint texture;
    glViewport(0, 0, width, height);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        NULL);
    return texture;
}
