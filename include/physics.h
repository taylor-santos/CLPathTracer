#ifndef PHYSICS_H
#define PHYSICS_H

#include "vector3.h"

void
AddPhysObject(Vector3 *position, Vector3 *velocity);
void
AddPhysPtr(void *pos_base, void *pos_ptr, void *vel_base, void *vel_ptr);
void
PhysStep(double stepSize);
void
PhysTerminate(void);

#endif//PHYSICS_H
