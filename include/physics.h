#ifndef PHYSICS_H
#define PHYSICS_H

#include "vector3.h"

void
AddPhysObject(Vector3 *position, Vector3 *velocity);
void
PhysStep(double stepSize);
void
PhysTerminate(void);

#endif//PHYSICS_H
