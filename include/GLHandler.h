#ifndef GLHANDLER_H
#define GLHANDLER_H

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

void
GLInitGLFW(void);
GLFWmonitor *
GLGetMonitor(void);
GLFWwindow *
GLCreateWindow(GLFWmonitor *monitor);
void
GLInitGL3W(void);
GLuint
GLBuildShader(void);
GLuint
GLCreateTexture(int width, int height);
void
GLResizeTexture(GLuint *texture, int width, int height);
GLuint
GLSetupRender(void);

#endif//CLHANDLER
