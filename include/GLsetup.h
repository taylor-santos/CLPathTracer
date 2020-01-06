#ifndef GLSETUP_H
#define GLSETUP_H

typedef struct GLState GLState;
typedef struct GLData GLData;

struct GLState {
    GLData *data;
    void (*terminate)(GLState *this);
};

GLState
GLsetup();

#endif//GLSETUP_H
