#ifndef GLSTATE_H
#define GLSTATE_H

#include "matrix.h"
#include "object.h"
#include "kd_tree.h"

void
GLInit(const char *kernel_filename, const char *kernel_name);
void
GLGetWindowPos(int *x, int *y);
void
GLGetWindowSize(int *x, int *y);
void
GLSetCameraMatrix(Matrix matrix);
void
GLSetObjects(Object *vec_objects, size_t size);
void
GLSetMeshes(kd *models);
void
GLRegisterKey(int key, GLFWkeyfun function);
void
GLRegisterScroll(GLFWscrollfun callback);
void
GLRegisterMouseFunction(GLFWcursorposfun function);
void
GLGetMousePos(double *x, double *y);
int
GLRender(void);
void
GLTerminate(void);
#endif//GLSTATE_H
