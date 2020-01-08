#ifndef GLINIT_H
#define GLINIT_H

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

GLuint
GLGetTexture(void);

void
GLGetWindowPos(int *x, int *y);

void
GLGetWindowSize(int *x, int *y);

void
GLRegisterKey(int key, GLFWkeyfun function);

void
GLRender(void);

void
GLTerminate(void);

void
GLInit();

#endif//GLINIT_H
