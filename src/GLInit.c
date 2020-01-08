#define WINDOW_NAME "OpenCL Path Tracer"
#define VSYNC 0

#include "GLInit.h"
#include "CLInit.h"
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

struct GLData {
    CL CL;
    GLFWmonitor *monitor;
    GLFWwindow *window;
    GLuint shader_program;
    GLint tex_loc;
    int width, height;
    GLuint vao, texture;
    GLFWkeyfun KeyHandlers[GLFW_KEY_LAST + 1];
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
get_monitor(void) __attribute__ ((warn_unused_result));

static GLFWmonitor *
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
    return monitors[0];
}

static GLFWwindow *
create_window(GLFWmonitor *monitor) __attribute__ ((warn_unused_result));

static GLFWwindow *
create_window(GLFWmonitor *monitor) {
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

static void
init_gl3w(void) {
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

static GLuint
build_shaders(void) {
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

static void
create_texture(GLuint *texture, int width, int height) {
    glViewport(0, 0, width, height);

    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

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
    GLData *GL = glfwGetWindowUserPointer(wind);
    GL->width = max(new_width, 1);
    GL->height = max(new_height, 1);
    create_texture(&GL->texture, GL->width, GL->height);
    GL->CL.DeleteImage(&GL->CL);
    GL->CL.CreateImage(&GL->CL, GL->texture);
}

static void
get_window_pos(GL *this, int *x, int *y) {
    glfwGetWindowPos(this->data->window, x, y);
}

static void
get_window_size(GL *this, int *x, int *y) {
    glfwGetWindowSize(this->data->window, x, y);
}

static void
key_callback(GLFWwindow *wind, int key, int scancode, int action, int mods) {
    GLData *GL = glfwGetWindowUserPointer(wind);

    if (GL->KeyHandlers[key] == NULL) {
        return;
    }
    GL->KeyHandlers[key](GL->window, key, scancode, action, mods);
}

static void
set_key_callback(GLFWwindow *window) {
    glfwSetKeyCallback(window, key_callback);
}

static void
register_key(GL *this, int key, GLFWkeyfun function) {
    this->data->KeyHandlers[key] = function;
}

static void
render(GL *this) {
    int frame = 0;
    double time = glfwGetTime();
    while (!glfwWindowShouldClose(this->data->window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(this->data->shader_program);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->data->texture);
        glUniform1i(this->data->tex_loc, 0);
        glBindVertexArray(this->data->vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            fprintf(stderr, "OpenGL Error: %d\n", error);
            break;
        }
        glfwSwapBuffers(this->data->window);
        this->data->CL
            .Execute(&this->data->CL, this->data->width, this->data->height);
        frame++;
    }
    time = glfwGetTime() - time;
    printf("%f fps\n", frame / time);
}

static void
terminate(GL *this) {
    this->data->CL.Terminate(&this->data->CL);
    free(this->data);
    glfwTerminate();
}

GL
GLInit(const char *kernel_filename, const char *kernel_name) {
    GLData *data;
    GLuint vbo, ibo;

    data = malloc(sizeof(*data));
    if (data == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    glfwSetErrorCallback(error_callback);
    init_glfw();
    data->monitor = get_monitor();
    data->window = create_window(data->monitor);
    glfwSetWindowUserPointer(data->window, data);
    glfwMakeContextCurrent(data->window);
    glfwSetWindowSizeCallback(data->window, resize_callback);
    set_key_callback(data->window);
    init_gl3w();
    glfwGetFramebufferSize(data->window, &data->width, &data->height);
    glViewport(0, 0, data->width, data->height);
    glfwSwapInterval(VSYNC);

    data->shader_program = build_shaders();
    data->tex_loc = glGetUniformLocation(data->shader_program, "tex");

    glGenVertexArrays(1, &data->vao);
    glBindVertexArray(data->vao);
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

    create_texture(&data->texture, data->width, data->height);
    data->CL = CLInit(kernel_filename, kernel_name);
    data->CL.CreateImage(&data->CL, data->texture);
    return (GL){
        data,
        get_window_pos,
        get_window_size,
        register_key,
        render,
        terminate
    };
}
