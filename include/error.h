#ifndef ERROR_H
#define ERROR_H
#define HANDLE_ERR(err) handle_err(err, __FILE__, __LINE__)

#include "CL/cl.h"

const char *
err_string(cl_int error);
void
handle_err(cl_int err, const char *file, int line);

#endif//ERROR_H
