#ifndef GAME_H
#define GAME_H

#include "CLState.h"
void
GameInit(const char *kernel_filename, const char *kernel_name, CLArg *args);
void
StartGameLoop(void);
void
GameTerminate(void);

#endif // GAME_H
