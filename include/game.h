#ifndef GAME_H
#define GAME_H

void
GameInit(
    const char *       kernel_filename,
    const char *       kernel_name,
    const char *const *models);
void
StartGameLoop(void);
void
GameTerminate(void);

#endif // GAME_H
