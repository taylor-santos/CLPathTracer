#ifndef ERROR_H
#define ERROR_H
#define HANDLE_ERR(err) { cl_int errID = err; if (err < 0) { fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); handle_err(errID);}}

#include "CL/cl.h"

const char*
err_string(cl_int error);

void
handle_err(cl_int err);

#endif//ERROR_H
