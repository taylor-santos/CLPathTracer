#ifndef MODEL_H
#define MODEL_H

#include <CL/cl_gl.h>

typedef struct Model Model;

Model
new_Model(void);

void
append_Model_vert(Model *, cl_double4);

void
append_Model_tri(Model *, cl_int3);

int
LoadModel(const char *filename);

#endif//MODEL_H
