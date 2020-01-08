#ifndef GLSETUP_H
#define GLSETUP_H

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

GLuint
GLGetTexture(void);

void
GLRender(void);

void
GLTerminate(void);

void
GLSetup();

#endif//GLSETUP_H
