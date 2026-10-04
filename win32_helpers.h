#ifndef WIN_HELPERS_H
#define WIN_HELPERS_H
#include <stdio.h>

void hide_taskbar_icon(void* hwnd);
void* get_workerw(void);
void reparent_to_workerw(void* hwnd_ptr, int screenX, int screenY, int width, int height);

int is_escape_held(void);
void start_keyboard_hook(void);

#endif