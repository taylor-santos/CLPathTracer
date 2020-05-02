#ifndef MODEL_H
#define MODEL_H

#include <CL/cl_gl.h>

#include "vector3.h"

typedef struct Model Model;

Model *
new_Model(void);
void
delete_Model(Model *);
void
append_Model_vert(Model *, Vector4);
void
append_Model_tri(Model *, cl_int3);
Vector4 *
Model_verts(Model *);
cl_int3 *
Model_tris(Model *);
int
LoadModel(const char *filename, Model *model);

#endif//MODEL_H
