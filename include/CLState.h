#ifndef CLSTATE_H
#define CLSTATE_H

#include <stddef.h>
#include <CL/cl.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "matrix.h"

void
CLInit(const char *kernel_filename, const char *kernel_name);
void
CLTerminate(void);
void
CLSetCameraMatrix(Matrix matrix);
void
CLDeleteImage(void);
void
CLCreateImage(GLuint texture);
void
CLExecute(int width, int height);

#endif//CL_SETUP_H
