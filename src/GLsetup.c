#define WINDOW_NAME "OpenCL Path Tracer"
#define VSYNC 0

#include "GLsetup.h"
#include "CLsetup.h"
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

static GLFWmonitor *monitor;
static GLFWwindow *window;
static GLuint shader_program;
static GLint tex_loc;
static int width, height;
static GLuint vao, texture;

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

static void
get_monitor(void) {
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
    monitor = monitors[0];
}

static void
create_window() {
    const GLFWvidmode *mode;

    mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    window = glfwCreateWindow(mode->width,
        mode->height,
        WINDOW_NAME,
        monitor/*monitor*/,
        NULL);
}

static void
init_gl3w(void) {
    if (gl3wInit()) {
        fprintf(stderr, "OpenGL failed to initialize\n");
        exit(EXIT_FAILURE);
    }
}

static GLuint
build_shaders() {
    // shader source code
    const char *vertex_source = "#version 330\n"
                                "layout(location = 0) in vec4 vposition;\n"
                                "layout(location = 1) in vec2 vtexcoord;\n"
                                "out vec2 ftexcoord;\n"
                                "void main() {\n"
                                "   ftexcoord = vtexcoord;\n"
                                "   gl_Position = vposition;\n"
                                "}\n";

    const char *fragment_source = "#version 330\n"
                                  "uniform sampler2D tex;\n"
                                  "in vec2 ftexcoord;\n"
                                  "layout(location = 0) out vec4 FragColor;\n"
                                  "void main() {\n"
                                  "   FragColor = texture(tex, ftexcoord);\n"
                                  "}\n";

    // program and shader handles
    GLuint vertex_shader, fragment_shader;

    // we need these to properly pass the strings
    const char *source;
    GLint length;

    // create and compiler vertex shader
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    source = vertex_source;
    length = strlen(vertex_source);
    glShaderSource(vertex_shader, 1, &source, &length);
    glCompileShader(vertex_shader);

    // create and compiler fragment shader
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    source = fragment_source;
    length = strlen(fragment_source);
    glShaderSource(fragment_shader, 1, &source, &length);
    glCompileShader(fragment_shader);

    // create program
    shader_program = glCreateProgram();

    // attach shaders
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);

    // link the program and check for errors
    glLinkProgram(shader_program);

    return shader_program;
}

static void
create_texture(void) {
    glViewport(0, 0, width, height);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set texture parameters
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

static void
resize_callback(GLFWwindow *wind, int new_width, int new_height) {
    width = max(new_width, 1);
    height = max(new_height, 1);
    create_texture();
    CLCreateImage();
}

GLuint
GLGetTexture(void) {
    return texture;
}

void
GLRender(void) {
    int frame = 0;
    double time = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader_program);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(tex_loc, 0);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            fprintf(stderr, "OpenGL Error: %d\n", error);
            break;
        }
        glfwSwapBuffers(window);
        CLExecute(width, height);
        frame++;
    }
    time = glfwGetTime() - time;
    printf("%f fps\n", frame / time);
}

void
GLTerminate(void) {
    glfwTerminate();
}

void
GLSetup(void) {
    glfwSetErrorCallback(error_callback);
    init_glfw();
    get_monitor();
    create_window();
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, resize_callback);
    init_gl3w();
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSwapInterval(VSYNC);

    shader_program = build_shaders();
    tex_loc = glGetUniformLocation(shader_program, "tex");
    GLuint vbo, ibo;
    // generate and bind the vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // generate and bind the buffer object
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // data for a fullscreen quad
    GLfloat vertexData[] = {
        //  X     Y     Z           U     V
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
    }; // 4 vertices with 5 components (floats) each

    // fill with data
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(vertexData),
        vertexData,
        GL_STATIC_DRAW);

    // set up generic attrib pointers
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
        2, // first triangle
        2,
        1,
        3 // second triangle
    };

    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        sizeof(indexData),
        indexData,
        GL_STATIC_DRAW);

    //unbind vao
    glBindVertexArray(0);

    create_texture();
}
