#ifndef CL_SETUP_H
#define CL_SETUP_H

#include <stddef.h>
#include <CL/cl.h>
#include "GLsetup.h"

void
CLCreateImage(void);

void
CLExecute(int width, int height);

void
CLTerminate(void);

void
CLSetup(const char *filename, const char *kernel_name);

#endif//CL_SETUP_H
