#ifndef GAMEINIT_H
#define GAMEINIT_H

typedef struct {
    struct {
        double x, y;
    } mouseSensitivity;
    double movementSpeed;
    double sprintSpeed;
} gameprops;

extern gameprops GameProperties;

void
GameInit(const char *kernel_filename, const char *kernel_name);

void
StartGameLoop(void);

void
GameTerminate(void);

#endif//GAMEINIT_H
