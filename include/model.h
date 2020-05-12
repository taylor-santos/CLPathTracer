#ifndef MODEL_H
#define MODEL_H

#include <CL/cl_gl.h>

#include "vector3.h"
#include "kd_tree.h"

typedef struct Model Model;

Model *
new_Model(void);
void
delete_Model(Model *);
void
append_Model_vert(Model *, Vector4);
void
append_Model_tri(Model *, cl_int3);
const Vector4 *
Model_verts(Model *);
const cl_int3 *
Model_tris(Model *);
Vector3
Model_min(Model *);
Vector3
Model_max(Model *);
const char *
Model_path(Model *);
int
LoadModel(const char *filename, kd *model);

#endif//MODEL_H
